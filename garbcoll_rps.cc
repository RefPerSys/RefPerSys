/****************************************************************
 * file garbcoll_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      Garbage collector support.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io>
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "refpersys.hh"

std::mutex Rps_MemoryBlock::_bl_lock_;

std::map<intptr_t,Rps_MemoryBlock*> Rps_MemoryBlock::_bl_blocksmap_;

std::vector<Rps_MemoryBlock*> Rps_MemoryBlock::_bl_blocksvect_;
std::mutex Rps_MarkedMemoryBlock::_glob_mablock_mtx_;
Rps_MarkedMemoryBlock* Rps_MarkedMemoryBlock::_glo_markedblock_;
std::set<Rps_MarkedMemoryBlock*> Rps_MarkedMemoryBlock::_glob_set_mablocks;

std::atomic<bool> Rps_GarbageCollector::_gc_wanted;

thread_local Rps_GarbageCollector::thread_allocation_data*
Rps_GarbageCollector::_gc_thralloc_;

Rps_GarbageCollector::global_allocation_data
Rps_GarbageCollector::_gc_globalloc_;
std::atomic<unsigned long> Rps_GarbageCollector::_gc_count;



void*
Rps_MemoryBlock::operator new(size_t size)
{
  RPS_ASSERTPRINTF (size % RPS_SMALL_BLOCK_SIZE == 0, "size=%zd", size);
  void* ad = mmap(nullptr, size,  //
                  PROT_READ | PROT_WRITE, //
                  MAP_ANON | MAP_PRIVATE | MAP_HUGETLB, //
                  -1, 0);
  if (ad == MAP_FAILED)
    ad = mmap(nullptr, size,  //
              PROT_READ | PROT_WRITE, //
              MAP_ANON | MAP_PRIVATE, //
              -1, 0);
  if (ad == MAP_FAILED)
    RPS_FATAL("failed to get memory block of %zd Mbytes (%m)",
              size >> 20);
  if ((uintptr_t)ad % RPS_SMALL_BLOCK_SIZE != 0)
    {
      // misaligned block, allocate something bigger, and munmap the
      // useless parts
      if (munmap(ad, size) < 0)
        RPS_FATAL("failed to munmap memory block of %zd Mbytes @ %p (%m)",
                  size >> 20, ad);
      ad = mmap(nullptr, size+RPS_SMALL_BLOCK_SIZE, //
                PROT_READ | PROT_WRITE, //
                MAP_ANON | MAP_SHARED | MAP_HUGETLB, //
                -1, 0);
      if (ad == MAP_FAILED)
        // no huge pages, so try normal pages...
        ad = mmap(nullptr, size+RPS_SMALL_BLOCK_SIZE, //
                  PROT_READ | PROT_WRITE, //
                  MAP_ANON | MAP_SHARED, //
                  -1, 0);
      if (ad == MAP_FAILED)
        RPS_FATAL("failed to get memory block of %zd Mbytes (%m)",
                  (size+RPS_SMALL_BLOCK_SIZE) >> 20);
      if ((uintptr_t)ad % RPS_SMALL_BLOCK_SIZE != 0)
        {
          void* endad = (char*)ad + size+RPS_SMALL_BLOCK_SIZE;
          void* oldad = ad;
          uintptr_t nextad = ((uintptr_t)ad + RPS_SMALL_BLOCK_SIZE) & ~(RPS_SMALL_BLOCK_SIZE-1);
          if (nextad > (uintptr_t)ad  && munmap(ad, nextad - (uintptr_t)ad))
            RPS_FATAL("failed to munmap @%p (%m)", ad);
          ad = (void*)nextad;
          if ((char*)ad + size < endad
              && munmap((char*)ad+size, (char*)endad - ((char*)ad + size)))
            RPS_FATAL("failed to munmap @%p (%m)", (void*) ((char*) ad+size));
        }
    };
  return ad;
} // end Rps_MemoryBlock::operator new



Rps_MemoryBlock::Rps_MemoryBlock(std::mutex&mtx,
                                 unsigned kindnum,
                                 Rps_BlockIndex ix,
                                 Rps_MemoryBlock::size_tag,
                                 size_t size,
                                 std::function<void(Rps_MemoryBlock*)> before,
                                 std::function<void(Rps_MemoryBlock*)> after) :
  _bl_kindnum(kindnum), _bl_ix(0), _bl_curptr(this->_bl_data),
  _bl_endptr(((char*)this)+size-sizeof(intptr_t)),
  _bl_next(nullptr), _bl_prev(nullptr)
{
  RPS_ASSERTPRINTF (size >= RPS_SMALL_BLOCK_SIZE && size <= RPS_LARGE_BLOCK_SIZE, "size=%zd", size);
  RPS_ASSERTPRINTF (size % RPS_SMALL_BLOCK_SIZE == 0, "size=%zd", size);
  RPS_ASSERTPRINTF ((intptr_t)this % RPS_SMALL_BLOCK_SIZE == 0, "this@%p", (void*)this);
  intptr_t ad = (intptr_t) this;
  RPS_ASSERT ((const char*)_bl_endptr > (const char*)this);
  RPS_ASSERT ((const char*)_bl_curptr > (const char*)this);
  RPS_ASSERT ((const char*)_bl_curptr < (const char*)_bl_endptr);
  std::lock_guard gu(mtx);
  RPS_ASSERT (_bl_blocksmap_.find(ad) == _bl_blocksmap_.end());
  if (before)
    before(this);
  _bl_ix = ix;
  auto itb = _bl_blocksmap_.lower_bound((intptr_t)((intptr_t*)ad-2));
  auto ita = _bl_blocksmap_.upper_bound((intptr_t)((intptr_t*)ad+2));
  if (itb != _bl_blocksmap_.end())
    {
      _bl_prev = itb->second;
      RPS_ASSERT ((char*)_bl_prev < (char*)this);
    }
  if (ita != _bl_blocksmap_.end())
    {
      _bl_next = ita->second;
      RPS_ASSERT ((char*)_bl_next > (char*)this);
    }
  _bl_blocksmap_.insert({ad,this});
  int altix = 0;
  if (ix <= 0)   // in particular, when ix is _no_index_
    {
      if (_bl_prev && (altix=_bl_prev->_bl_ix)>0)
        {
          ix = altix;
        }
      else if (_bl_next && (altix=_bl_next->_bl_ix)>1)
        {
          ix = altix-1;
        }
      else
        {
          RPS_ASSERT (!_bl_prev && !_bl_next);
          ix = 1;
        }
    };
  RPS_ASSERT (ix > 0);
  if (_bl_blocksvect_.size() + 1 <= ix)
    _bl_blocksvect_.resize(rps_prime_above((17*ix)/16+1));
  _bl_blocksvect_.insert(_bl_blocksvect_.begin()+ix, this);
  auto nbbl = _bl_blocksvect_.size();
  for (int nix=ix; nix<nbbl; nix++)
    {
      auto nbl = _bl_blocksvect_[nix];
      if (!nbl) continue;
      nbl->_bl_ix = nix;
    }
  if (after)
    after(this);
} // end Rps_MemoryBlock::Rps_MemoryBlock




void
Rps_MemoryBlock::remove_block(Rps_MemoryBlock*bl)
{
  RPS_ASSERT (bl != nullptr && dynamic_cast<Rps_MemoryBlock*>(bl) != nullptr);
  intptr_t ad = (intptr_t) bl;
  Rps_BlockIndex ix = bl->_bl_ix;
  std::lock_guard<std::mutex> guard(_bl_lock_);
  _bl_blocksmap_.erase(ad);
  RPS_ASSERTPRINTF (ix < _bl_blocksvect_.size(), "ix=%u", (unsigned)ix);
  _bl_blocksvect_[ix] = nullptr;
} // end Rps_MemoryBlock::remove_block


void
Rps_MemoryBlock::operator delete(void*ptr)
{
#warning unimplemented Rps_MemoryBlock::operator delete
  RPS_FATAL("Rps_MemoryBlock::operator delete\n");
} // end Rps_MemoryBlock::operator delete

void*
Rps_MemoryBlock::allocate_aligned_zone(size_t size, size_t align)
{
#warning unimplemented Rps_MemoryBlock::allocate_aligned_zone
  RPS_FATAL("Rps_MemoryBlock::allocate_aligned_zone(size %zd, align %zd)\n", size, align);
} // end Rps_MemoryBlock::operator delete

////////////////

Rps_BirthMemoryBlock*
Rps_BirthMemoryBlock::make(void)
{
#warning unimplemented Rps_BirthMemoryBlock::make
  RPS_FATAL("Rps_BirthMemoryBlock::make unimplemented");
} // end of Rps_BirthMemoryBlock::make


Rps_SmallOldMemoryBlock*
Rps_SmallOldMemoryBlock::make(void)
{
#warning unimplemented Rps_SmallOldMemoryBlock::make
  RPS_FATAL("Rps_SmallOldMemoryBlock::make unimplemented");
} // end of Rps_SmallOldMemoryBlock::make


Rps_LargeNewMemoryBlock*
Rps_LargeNewMemoryBlock::make(void)
{
#warning unimplemented Rps_LargeNewMemoryBlock::make
  RPS_FATAL("Rps_LargeNewMemoryBlock::make unimplemented");
} // end of Rps_LargeNewMemoryBlock::make



Rps_LargeOldMemoryBlock*
Rps_LargeOldMemoryBlock::make(void)
{
#warning unimplemented Rps_LargeOldMemoryBlock::make
  RPS_FATAL("Rps_LargeOldMemoryBlock::make unimplemented");
} // end of Rps_LargeOldMemoryBlock::make


Rps_MarkedMemoryBlock*
Rps_MarkedMemoryBlock::make(void)
{
  Rps_MarkedMemoryBlock* bl =
    new Rps_MarkedMemoryBlock(Rps_MemoryBlock::_no_index_);
  return bl;
} // end of Rps_MarkedMemoryBlock::make



////////////////
void
Rps_GarbageCollector::run_garbcoll(Rps_CallFrameZone*callingfra)
{
  _gc_wanted.store(true);
  // we need to synchronize with other worker threads
  if (callingfra)
    scan_call_stack(callingfra);
#warning unimplemented Rps_MemoryBlock::run_garbcoll
  RPS_FATAL("Rps_MemoryBlock::run_garbcoll unimplemented");
} // end Rps_GarbageCollector::run_garbcoll


void
Rps_GarbageCollector::scan_call_stack(Rps_CallFrameZone*callingfra)
{
  RPS_ASSERT (callingfra != nullptr);
  /// we should scan and forward all the pointers on the call stack,
  /// starting with the topmost callingfra
  for (Rps_CallFrameZone*curfram = callingfra; curfram != nullptr;
       curfram = curfram->prev())
    {
      int siz = curfram->size();
      curfram->descr().scan_objectref(callingfra);
      for (int ix=0; ix<siz; ix++)
        {
          auto curqvalptr = curfram->unsafe_at(ix);
          if (!curqvalptr)
            continue;
          Rps_Type curtyp = Rps_ZoneValue::get_type(curqvalptr);
          if (!rps_is_type_of_quasi_value(curtyp))
            continue;
          Rps_ZoneValue::scan_quasivalue(curqvalptr,callingfra);
        }
    }
} // end Rps_GarbageCollector::scan_call_stack


void
Rps_GarbageCollector::run_write_barrier(Rps_CallFrameZone*callingfra, Rps_ZoneValue*zva)
{
#warning unimplemented Rps_GarbageCollector::run_write_barrier
  RPS_FATAL("Rps_GarbageCollector::run_write_barrier unimplemented callingfra@%p zva@%p",
            (void*)callingfra, (void*)zva);
} // end Rps_GarbageCollector::run_write_barrier


void*
Rps_GarbageCollector::allocate_marked_maybe_gc(size_t size, Rps_CallFrameZone*callingfra)
{
  void* ad = nullptr;
  if (RPS_UNLIKELY(size % rps_allocation_unit == 0))
    size = (size | (rps_allocation_unit-1))+1;
  RPS_ASSERT (size < RPS_SMALL_BLOCK_SIZE - Rps_MarkedMemoryBlock::_remain_threshold_ - 4*sizeof(void*));
  maybe_garbcoll(callingfra);
  {
    std::lock_guard<std::mutex>
    _gu(Rps_MarkedMemoryBlock::_glob_mablock_mtx_);
    if (Rps_MarkedMemoryBlock::_glo_markedblock_ == nullptr
        || (Rps_MarkedMemoryBlock::_glo_markedblock_->remaining_bytes()
            < size))
      {
        /// should allocate that _glo_markedblock_
        Rps_MarkedMemoryBlock::_glo_markedblock_ =
          Rps_MarkedMemoryBlock::make();
      };
    ad = Rps_MarkedMemoryBlock::_glo_markedblock_->allocate_zone(size);
  };
  RPS_ASSERT (ad != nullptr);
  return ad;
} // end Rps_GarbageCollector::allocate_marked_maybe_gc

////////////////////////////////////////////////////////////////
std::mutex Rps_MutatorThread::_mthr_mtx_;
std::set<Rps_MutatorThread*> Rps_MutatorThread::_mthr_thread_set_;
////////////////
Rps_MutatorThread::Rps_MutatorThread()
  : std::thread(), _mthr_prefix(), _mthr_gc_count(0)
{
  std::lock_guard<std::mutex> gu(_mthr_mtx_);
  _mthr_thread_set_.insert(this);
} // end Rps_MutatorThread::Rps_MutatorThread()

Rps_MutatorThread::Rps_MutatorThread(const char*name)
  : Rps_MutatorThread()
{
  if (name && name[0])
    _mthr_prefix = std::string(name);
} // end of Rps_MutatorThread::Rps_MutatorThread(const char*name)


Rps_MutatorThread::~Rps_MutatorThread()
{
  std::lock_guard<std::mutex> gu(_mthr_mtx_);
  _mthr_thread_set_.erase(this);
#warning unimplemented Rps_MutatorThread::~Rps_MutatorThread()
  RPS_FATAL("Rps_MutatorThread::~Rps_MutatorThread unimplemented");
} // end Rps_MutatorThread::~Rps_MutatorThread()


void
Rps_MutatorThread::disable_garbage_collector(void)
{
#warning unimplemented Rps_MutatorThread::disable_garbage_collector
  RPS_FATAL("unimplemented Rps_MutatorThread::disable_garbage_collector");
} // end Rps_MutatorThread::disable_garbage_collector

void
Rps_MutatorThread::enable_garbage_collector(void)
{
#warning unimplemented Rps_MutatorThread::enable_garbage_collector
  RPS_FATAL("unimplemented Rps_MutatorThread::enable_garbage_collector");
} // end Rps_MutatorThread::enable_garbage_collector

/// end of file garbcoll_rps.cc

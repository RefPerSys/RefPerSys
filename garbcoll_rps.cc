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
 *      Niklas Rosencrantz <niklasro@gmail.com>
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

void*
Rps_MemoryBlock::operator new(size_t size)
{
  assert (size % RPS_SMALL_BLOCK_SIZE == 0);
  void* ad = mmap(nullptr, size,  //
                  PROT_READ | PROT_WRITE, //
                  MAP_ANON | MAP_SHARED | MAP_HUGETLB, //
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
          if ((char*)ad + size < endad && munmap((char*)ad+size, (char*)endad - ((char*)ad + size)))
            RPS_FATAL("failed to munmap @%p (%m)", (void*) ((char*) ad+size));
        }
    };
  return ad;
} // end Rps_MemoryBlock::operator new



Rps_MemoryBlock::Rps_MemoryBlock(unsigned kindnum, Rps_BlockIndex ix, size_t size) :
  _bl_kindnum(kindnum), _bl_ix(ix), _bl_curptr(_bl_data),
  _bl_endptr((char*)this+size),
  _bl_next(nullptr), _bl_prev(nullptr)
{
} // end Rps_MemoryBlock::Rps_MemoryBlock

void
Rps_MemoryBlock::operator delete(void*)
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



std::atomic<bool> Rps_GarbageCollector::_gc_wanted;

thread_local Rps_GarbageCollector::thread_allocation_data*
Rps_GarbageCollector::_gc_thralloc_;

Rps_GarbageCollector::global_allocation_data
Rps_GarbageCollector::_gc_globalloc_;

void
Rps_GarbageCollector::run_garbcoll(Rps_CallFrameZone*callfram)
{
  _gc_wanted.store(true);
  if (callfram)
    scan_call_stack(callfram);
  // we need to synchronize with other worker threads
#warning unimplemented Rps_MemoryBlock::run_garbcoll
  RPS_FATAL("Rps_MemoryBlock::run_garbcoll unimplemented");
} // end Rps_GarbageCollector::run_garbcoll


void
Rps_GarbageCollector::scan_call_stack(Rps_CallFrameZone*callfram)
{
  assert (callfram != nullptr);
  /// we should scan and forward all the pointers on the call stack,
  /// starting with the topmost callfram
#warning unimplemented Rps_MemoryBlock::scan_call_stack
  RPS_FATAL("Rps_MemoryBlock::scan_call_stack unimplemented");
} // end Rps_GarbageCollector::scan_call_stack

/// end of file garbcoll_rps.cc

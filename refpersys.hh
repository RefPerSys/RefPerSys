/****************************************************************
 * file refpersys.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its only C++ header file.
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



#ifndef REFPERSYS_INCLUDED
#define REFPERSYS_INCLUDED

// A name containing `unsafe` refers to something which should be used
// with great caution.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#if __cplusplus < 201412L
#error expecting C++17 standard
#endif

#include <set>
#include <map>
#include <variant>
#include <unordered_map>
#include <new>
#include <random>
#include <iostream>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <mutex>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <functional>

#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdio>

#include <argp.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

#include "unistr.h"


// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define RPS_UNLIKELY(P) __builtin_expect(!!(P),0)
#define RPS_LIKELY(P) !__builtin_expect(!(P),0)
#define RPS_UNUSED __attribute__((unused))
#else
#define RPS_UNLIKELY(P) (P)
#define RPS_LIKELY(P) (P)
#define RPS_UNUSED
#endif


// generated in _timestamp_rps.cc
extern "C" const char timestamp_rps[];
extern "C" unsigned long timenum_rps;
extern "C" const char gitid_rps[];
extern "C" const char md5sum_rps[];
extern "C" const char cwd_rps[];
extern "C" const char lastgitcommit_rps[];
extern "C" const char cxxcompiler_rps[];
extern "C" const char buildhost_rps[];
extern "C" const char sourcefiles_rps[];
extern "C" const char headerfiles_rps[];


// see http://man7.org/linux/man-pages/man2/clock_gettime.2.html
static inline double
rps_monotonic_real_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_monotonic_real_time


static inline double
rps_process_cpu_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_process_cpu_time


static inline double
rps_thread_cpu_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_thread_cpu_time

/***
 * any makeconst C++ metaprogram would need to use only the code
 * compiled with -DRPS_ONLY_ID_CODE, in particular any future
 * adaptation of
 * https://github.com/bstarynk/bismon/blob/master/BM_makeconst.cc
 ***/

#ifndef RPS_ONLY_ID_CODE
extern void print_types_info(void);
class Rps_CallFrameZone;
class Rps_GarbageCollector;

#define RPS_FLEXIBLE_DIM 0	/* for flexible array members */

/// In rare occasions (some kind of array hash table, perhaps) we may
/// need a pointer value which is non null but still morally "empty":
/// this is rarely useful, and any code using that should be carefully
/// written.
#define RPS_EMPTYSLOT   ((const void*)(((intptr_t*)nullptr)+1))

// size of blocks, in bytes
#define RPS_SMALL_BLOCK_SIZE (8<<20)
#define RPS_LARGE_BLOCK_SIZE (8*RPS_SMALL_BLOCK_SIZE)

static_assert(RPS_SMALL_BLOCK_SIZE & (~ (RPS_SMALL_BLOCK_SIZE-1)),
              "RPS_SMALL_BLOCK_SIZE should be some power of two");

// give, using some a table of primes, some prime number above or below a
// given integer, and reasonably close to it (e.g. less than 20% from
// it).
extern "C" int64_t rps_prime_above (int64_t n);
extern "C" int64_t rps_prime_below (int64_t n);

extern "C" void rps_fatal_stop_at (const char *, int) __attribute__((noreturn));

#define RPS_FATAL_AT_BIS(Fil,Lin,Fmt,...) do {			\
    fprintf(stderr,						\
	    "RefPerSys FATAL:%s:%d: <%s>\n " Fmt "\n\n",	\
            Fil, Lin, __func__, ##__VA_ARGS__);			\
    rps_fatal_stop_at (Fil,Lin); } while(0)

#define RPS_FATAL_AT(Fil,Lin,Fmt,...) RPS_FATAL_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)

#define RPS_FATAL(Fmt,...) RPS_FATAL_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

typedef uint32_t Rps_BlockIndex;

// blocks are always dynamically allocated, and are using mmap-ed
// memory
class  alignas(RPS_SMALL_BLOCK_SIZE) Rps_MemoryBlock
{
  friend class Rps_GarbageCollector;
public:
  static constexpr size_t _bl_minsize_ = 4*sizeof(void*);
  enum BlockKind_en
  {
    blk_none=0,
    blk_birth,
    blk_largenew,
    blk_marked,
  };
protected:
  const uint32_t _bl_kindnum;
  const Rps_BlockIndex _bl_ix;
  static constexpr unsigned _bl_metasize_ = 8;
private:
  void* _bl_curptr;
  void* const _bl_endptr;
  Rps_MemoryBlock* _bl_next;
  Rps_MemoryBlock* _bl_prev;
  static std::mutex _bl_lock_;
  static std::map<intptr_t,Rps_MemoryBlock*> _bl_blocksmap_;
  static std::vector<Rps_MemoryBlock*> _bl_blocksvect_;
protected:
  static void remove_block(Rps_MemoryBlock*bl);
  virtual ~Rps_MemoryBlock()
  {
    remove_block(this);
    _bl_curptr = nullptr;
    const_cast<void*&>(_bl_endptr) = nullptr;
    _bl_next = nullptr;
    _bl_prev = 0;
    const_cast<Rps_BlockIndex&>(_bl_ix) = 0;
  };
  void* operator new(size_t size);
  void operator delete(void*);
  intptr_t _bl_meta[_bl_metasize_];
  intptr_t _bl_data[RPS_FLEXIBLE_DIM];
  Rps_MemoryBlock(std::mutex&mtx,
                  unsigned kindnum, Rps_BlockIndex ix, size_t size,
                  std::function<void(Rps_MemoryBlock*)> before=nullptr,
                  std::function<void(Rps_MemoryBlock*)> after=nullptr);
  struct unlocked_tag {};
  Rps_MemoryBlock(unlocked_tag,
                  unsigned kindnum, Rps_BlockIndex ix, size_t size,
                  std::function<void(Rps_MemoryBlock*)> before=nullptr,
                  std::function<void(Rps_MemoryBlock*)> after=nullptr)
    : Rps_MemoryBlock(_bl_lock_, kindnum, ix, size,
                      before, after) {};
  void* allocate_zone(size_t size)
  {
    assert(size % (2*alignof(void*)) == 0);
    assert(size >= _bl_minsize_);
    void*ptr = _bl_curptr;
    _bl_curptr = (char*)_bl_curptr + size;
    assert (_bl_curptr <= _bl_endptr);
    return ptr;
  }
  void* allocate_aligned_zone (size_t size, size_t align);
  template <class Meta> Meta& raw_metadata()
  {
    static_assert(sizeof(Meta) <= sizeof(_bl_meta));
    static_assert(alignof(Meta) <= alignof(_bl_meta));
    return *reinterpret_cast<Meta*>(_bl_meta);
  };
  template <class BlockClass>
  static BlockClass* make_block(size_t size)
  {
  }
public:
  unsigned remaining_bytes() const
  {
    assert ((char*)_bl_curptr <= (char*)_bl_endptr);
    return (char*)_bl_endptr - (char*)_bl_curptr;
  };
  static Rps_MemoryBlock*block_at_addr(const void*ad)
  {
    if (!ad || ad ==  RPS_EMPTYSLOT) return nullptr;
    std::lock_guard<std::mutex> guard(_bl_lock_);
    auto it = _bl_blocksmap_.find((intptr_t)ad
                                  & (~ (RPS_SMALL_BLOCK_SIZE-1)));
    if (it == _bl_blocksmap_.end())
      return nullptr;
    else
      return it->second;
  };				// end block_at_addr
};				// end Rps_MemoryBlock


class Rps_BirthMemoryBlock : public Rps_MemoryBlock
{
  friend class Rps_GarbageCollector;
  struct birthmetadata_st
  {
  };
  struct birthmetadata_st& metadata()
  {
    return raw_metadata<birthmetadata_st>();
  };
  Rps_BirthMemoryBlock(Rps_BlockIndex ix,
                       std::function<void(Rps_MemoryBlock*)> before=nullptr,
                       std::function<void(Rps_MemoryBlock*)> after=nullptr) :
    Rps_MemoryBlock(Rps_MemoryBlock::unlocked_tag{},
                    blk_birth, ix,
                    RPS_SMALL_BLOCK_SIZE - sizeof(Rps_BirthMemoryBlock),
                    before, after) {};
  ~Rps_BirthMemoryBlock() {};
public:
  static constexpr unsigned _remain_threshold_ = RPS_SMALL_BLOCK_SIZE/5;
  bool almost_full() const
  {
    return remaining_bytes() < _remain_threshold_;
  };
};				// end Rps_BirthMemoryBlock



class  alignas(RPS_LARGE_BLOCK_SIZE) Rps_LargeNewMemoryBlock
  : public Rps_MemoryBlock
{
  friend class Rps_GarbageCollector;
  struct largenewmetadata_st
  {
  };
  struct largenewmetadata_st& metadata()
  {
    return raw_metadata<largenewmetadata_st>();
  };
  Rps_LargeNewMemoryBlock(Rps_BlockIndex ix,
                          std::function<void(Rps_MemoryBlock*)> before=nullptr,
                          std::function<void(Rps_MemoryBlock*)> after=nullptr) :
    Rps_MemoryBlock(Rps_MemoryBlock::unlocked_tag{},
                    blk_largenew, ix,
                    RPS_LARGE_BLOCK_SIZE - sizeof(Rps_LargeNewMemoryBlock),
                    before, after) {};
  ~Rps_LargeNewMemoryBlock() {};
public:
  static constexpr unsigned _remain_threshold_ = RPS_LARGE_BLOCK_SIZE/5;
  bool almost_full() const
  {
    return remaining_bytes() < _remain_threshold_;
  };
};				// end Rps_LargeNewMemoryBlock


class  alignas(RPS_SMALL_BLOCK_SIZE) Rps_MarkedMemoryBlock
  : public Rps_MemoryBlock
{
  friend class Rps_GarbageCollector;
  static std::mutex _glob_mablock_mtx_; // global mutex for marked blocks
  static Rps_MarkedMemoryBlock* _glo_markedblock_; // the global marked block
  struct markedmetadata_st
  {
  };
  struct markedmetadata_st& metadata()
  {
    return raw_metadata<markedmetadata_st>();
  };
  Rps_MarkedMemoryBlock(Rps_BlockIndex ix,
                        std::function<void(Rps_MemoryBlock*)> before=nullptr,
                        std::function<void(Rps_MemoryBlock*)> after=nullptr	) :
    Rps_MemoryBlock(Rps_MemoryBlock::unlocked_tag{},
                    blk_marked, ix,
                    RPS_SMALL_BLOCK_SIZE - sizeof(Rps_MarkedMemoryBlock),
                    before, after) {};
  ~Rps_MarkedMemoryBlock() {};
public:
  static constexpr unsigned _remain_threshold_ = RPS_SMALL_BLOCK_SIZE/5;
  bool almost_full() const
  {
    return remaining_bytes() < _remain_threshold_;
  };
};				// end Rps_MarkedMemoryBlock





class Rps_ZoneValue;
class Rps_ObjectZone;
class Rps_ObjectRef
{
  friend class Rps_GarbageCollector;
  Rps_ObjectZone*_optr;
protected:
  inline void scan_objectref(Rps_CallFrameZone* callingfra) const;
public:
  Rps_ObjectZone* optr() const
  {
    return _optr;
  };
  const Rps_ObjectZone* const_optr() const
  {
    return const_cast<const Rps_ObjectZone*>(_optr);
  };
  bool is_empty() const
  {
    return _optr == nullptr || _optr == (Rps_ObjectZone*)RPS_EMPTYSLOT;
  }
  // rule of five
  Rps_ObjectRef(Rps_ObjectZone*oz = nullptr) : _optr(oz)
  {
    if (RPS_UNLIKELY((oz == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
  };
  ~Rps_ObjectRef()
  {
    _optr = nullptr;
  };
  Rps_ObjectRef(const Rps_ObjectRef&oth)
  {
    if (RPS_UNLIKELY((oth._optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
    else
      _optr = oth._optr;
  };
  Rps_ObjectRef(Rps_ObjectRef&&oth) : _optr(std::exchange(oth._optr, nullptr))
  {
    if (RPS_UNLIKELY((_optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
  };
  Rps_ObjectRef& operator = (const Rps_ObjectRef& oth)
  {
    _optr = oth._optr;
    if (RPS_UNLIKELY((_optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
    return *this;
  }
  Rps_ObjectRef& operator = (Rps_ObjectRef&& oth)
  {
    std::swap(_optr, oth._optr);
    if (RPS_UNLIKELY((_optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
    return *this;
  }
  const Rps_ObjectZone& operator * (void) const
  {
    assert(_optr != nullptr);
    return *_optr;
  };
  bool operator ! () const
  {
    return _optr == nullptr;
  };
  operator const Rps_ObjectZone* () const
  {
    return _optr;
  };
  operator Rps_ObjectZone* ()
  {
    return _optr;
  };
  operator bool () const
  {
    return _optr != nullptr;
  };
  const Rps_ObjectZone* operator -> (void) const
  {
    assert(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone* operator * (void)
  {
    assert(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone* operator -> (void)
  {
    assert(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone*obptr() const
  {
    return _optr;
  };
  void set_obptr(Rps_ObjectZone*zob)
  {
    if (RPS_UNLIKELY(zob == (Rps_ObjectZone*)RPS_EMPTYSLOT))
      zob = nullptr;
    _optr = zob;
  };
  inline bool operator == (const Rps_ObjectRef& oth) const;
  inline bool operator != (const Rps_ObjectRef& oth) const;
  inline bool operator <= (const Rps_ObjectRef& oth) const;
  inline bool operator < (const Rps_ObjectRef& oth) const;
  inline bool operator > (const Rps_ObjectRef& oth) const;
  inline bool operator >= (const Rps_ObjectRef& oth) const;
  static inline Rps_ObjectRef make(Rps_CallFrameZone*callingfra);
  static void tiny_benchmark_1(Rps_CallFrameZone* callingfra, unsigned count);
};				// end class Rps_ObjectRef


////////////////////////////////////////////////////////////////
typedef uint32_t Rps_HashInt;
enum Rps_Type
{
  /// non-value types (or quasi-values)
  ///
  /// these are GC-scanned, but not GC-allocated, just stack allocated:
  Rps_TyDumper = -32,
  Rps_TyLoader = -31,
  Rps_TyCallFrame = -30,
  /// these are GC-allocated quasivalues:
  Rps_TyPayload = -5,
  ///
  ///
  /// Quasi-values are indeed garbage collected, so GC-scanned and
  /// GC-allocated, but not "first class citizens" as genuine values
  /// are... So they don't directly go inside RpsValue-s.
  ///
  /// but see also, and keep in sync with, rps_ty_min_quasi below.
  Rps_TyQuasiToken = -4,
  Rps_TyQuasiAttributeArray = -3,
  Rps_TyQuasiComponentVector = -2,
  ///
  /// Values that could go into Rps_Value.
  Rps_TyInt = -1, // for tagged integers
  Rps_TyNone = 0, // for nil
  ///
  /// Boxed genuine values, are "first class citizens" that could be
  /// in Rps_Value's data. Of course they are both GC-allocated and
  /// GC-scanned.
  Rps_TyString,
  Rps_TyDouble,
  Rps_TySet,
  Rps_TyTuple,
  Rps_TyObject,
};

static constexpr Rps_Type rps_ty_min_quasi = Rps_TyQuasiToken;

////////////////////////////////////////////////////////////////




class Rps_Random
{
  static thread_local Rps_Random _rand_thr_;
  /// the thread local random state
  unsigned long _rand_count;
  std::mt19937 _rand_generator;
  /// we could need very quick and poor small random numbers on just 4
  /// bits. For these, we care less about the random quality, but even
  /// more about speed. So we keep one 32 bits of random number in
  /// advance, and a count of the remaining random bits in it.
  uint32_t _rand_advance;
  uint8_t _rand_remainbits;
  /// private constructor
  Rps_Random () : _rand_count(0), _rand_generator(), _rand_advance(0), _rand_remainbits(0) {};
  ///
  uint32_t generate_32u(void)
  {
    if (RPS_UNLIKELY(_rand_count++ % 65536 == 0))
      {
        std::random_device randev;
        auto s1=randev(), s2=randev(), s3=randev(), s4=randev(),
             s5=randev(), s6=randev(), s7=randev();
        std::seed_seq seq {s1,s2,s3,s4,s5,s6,s7};
        _rand_generator.seed(seq);
      }
    return _rand_generator();
  };
  uint32_t generate_nonzero_32u(void)
  {
    uint32_t r = 0;
    do
      {
        r = generate_32u();
      }
    while (RPS_UNLIKELY(r==0));
    return r;
  };
  uint64_t generate_64u(void)
  {
    return (static_cast<uint64_t>(generate_32u())<<32) | static_cast<uint64_t>(generate_32u());
  };
  uint8_t generate_quickly_4bits()
  {
    if (RPS_UNLIKELY(_rand_remainbits < 4))
      {
        _rand_advance = generate_32u();
        _rand_remainbits = 32;
      }
    uint8_t res = _rand_remainbits & 0xf;
    _rand_remainbits -= 4;
    return res;
  };
public:
  static uint32_t random_32u(void)
  {
    return _rand_thr_.generate_32u();
  };
  static uint64_t random_64u(void)
  {
    return _rand_thr_.generate_64u();
  };
  static uint32_t random_nonzero_32u(void)
  {
    return _rand_thr_.generate_nonzero_32u();
  };
  static uint8_t random_quickly_4bits()
  {
    return _rand_thr_.generate_quickly_4bits();
  };
};				// end class Rps_Random
#endif /*no RPS_ONLY_ID_CODE*/


////////////////////////////////////////////////////////////////

class Rps_Id
{
#ifndef RPS_ONLY_ID_CODE
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
#endif /*RPS_ONLY_ID_CODE*/
  uint64_t _id_hi;
  uint64_t _id_lo;
public:
  static constexpr const char b62digits[] =
    "0123456789"                          \
    "abcdefghijklmnopqrstuvwxyz"          \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr unsigned buflen = 24;
  static constexpr unsigned base = sizeof(b62digits)-1;
  static constexpr uint64_t min_hi = 62*62*62;
  static constexpr uint64_t max_hi = /// 8392993658683402240, about 8.392994e+18
    (uint64_t)10 * 62 * (62 * 62 * 62) * (62 * 62 * 62) * (62 * 62 * 62);
  static constexpr unsigned nbdigits_hi = 11;
  static constexpr uint64_t delta_hi = max_hi - min_hi;
  static constexpr uint64_t min_lo = 62*62;
  static constexpr uint64_t max_lo = (uint64_t)62 * (62L * 62 * 62) * (62 * 62 * 62); /// about 3.52161e+12
  static constexpr uint64_t delta_lo = max_lo - min_lo;
  static constexpr unsigned nbdigits_lo = 7;
  static constexpr unsigned maxbuckets = 10*62;
  static Rps_HashInt hash(uint64_t hi, uint64_t lo)
  {
    return (hi % 2147473837) + ((hi >> 32) ^ (lo * 17 + 201151));
  }
  Rps_HashInt hash() const
  {
    return hash(_id_hi, _id_lo);
  };
  static unsigned bucket_num(uint64_t hi)
  {
    unsigned b = hi / (max_hi / maxbuckets);
    assert(b<=maxbuckets);
    return b;
  };
  unsigned bucket_num(void) const
  {
    return bucket_num(_id_hi);
  };
  bool empty() const
  {
    return _id_hi == 0 && _id_lo == 0;
  };
  bool valid() const
  {
    return _id_hi >= min_hi && _id_hi < max_hi
           && _id_lo >= min_lo && _id_lo < max_lo
           && hash() != 0;
  }
  operator bool () const
  {
    return !empty();
  };
  bool operator ! () const
  {
    return empty();
  };
  bool operator == (const Rps_Id&oth) const
  {
    return _id_hi == oth._id_hi && _id_lo == oth._id_lo;
  };
  bool operator < (const Rps_Id&oth) const
  {
    if (_id_hi > oth._id_hi) return false;
    if (_id_hi < oth._id_hi) return true;
    return _id_lo < oth._id_lo;
  }
  bool operator <= (const Rps_Id&oth) const
  {
    if (_id_hi > oth._id_hi) return false;
    if (_id_hi < oth._id_hi) return true;
    return _id_lo <= oth._id_lo;
  }
  bool operator != (const Rps_Id&oth) const
  {
    return !(oth == *this);
  };
  bool operator > (const Rps_Id&oth) const
  {
    return oth < *this;
  };
  bool operator >= (const Rps_Id&oth) const
  {
    return oth <= *this;
  };
  uint64_t hi() const
  {
    return _id_hi;
  };
  uint64_t lo() const
  {
    return _id_lo;
  };
  Rps_Id(uint64_t h, uint32_t l=0) : _id_hi(h), _id_lo(l)
  {
    assert((h==0 && l==0) || hash() != 0);
  };
#ifndef RPS_ONLY_ID_CODE
  static Rps_Id random()
  {
    for(;;)
      {
        auto hi = Rps_Random::random_64u() % max_hi;
        if (RPS_UNLIKELY(hi < min_hi)) continue;
        auto lo = Rps_Random::random_64u() % max_lo;
        if (RPS_UNLIKELY(lo < min_lo)) continue;
        if (RPS_UNLIKELY(hash(hi,lo) == 0)) continue;
        return Rps_Id(hi,lo);
      };
  };
  Rps_Id (std::nullptr_t) : Rps_Id(random()) {};
#endif /*RPS_ONLY_ID_CODE*/
  Rps_Id () : Rps_Id(0,0) {};
  Rps_Id (const char*buf, const char**pend, bool *pok);
  Rps_Id (const Rps_Id&oid) : Rps_Id(oid.hi(), oid.lo()) {};
  void to_cbuf24(char cbuf[/*24*/]) const;
/// hashing, comparing, and equality testing operations on Rps_Id-s
  struct Hasher
  {
    std::size_t operator() (const Rps_Id& id) const
    {
      return (std::size_t)(id.hash());
    };
  };				// end Rps_Id::Hasher
  struct LessComparer
  {
    bool operator()  (const Rps_Id&id1, const Rps_Id&id2) const
    {
      return id1 < id2;
    }
  };				// end Rps_Id::LessComparer
  struct EqualTester
  {
    bool operator()  (const Rps_Id&id1, const Rps_Id&id2) const
    {
      return id1 == id2;
    }
  };				// end Rps_Id::EqualTester
};				// end class Rps_Id


#ifndef RPS_ONLY_ID_CODE
////////////////
class Rps_TupleObrefZone;
class Rps_SetObrefZone;
class Rps_SequenceObrefZone;

class Rps_Value
{
  friend class Rps_ZoneValue;
  friend class Rps_GarbageCollector;
  union
  {
    mutable const Rps_ZoneValue* _datav;
    intptr_t _intv;
  };
  static_assert(sizeof(_datav) == sizeof(_intv));
  static_assert(alignof(_datav) == alignof(_intv));
protected:

  void put_data(const Rps_ZoneValue*pz)
  {
    _datav = pz;
  };
  void put_int(intptr_t taggedint)
  {
    _intv = taggedint;
  };
public:
  operator bool () const
  {
    return _datav != nullptr;
  };
  bool operator ! () const
  {
    return _datav == nullptr;
  };
  // C++ rule of five:
  Rps_Value(const Rps_ZoneValue*d) : _datav(d) {};
  Rps_Value() : _datav(nullptr) {};
  Rps_Value(const Rps_Value&oth) : _datav(oth._datav) {};
  Rps_Value& operator= (const Rps_Value& oth)
  {
    _datav = oth._datav;
    return *this;
  };
  Rps_Value& operator= (Rps_Value&& other) noexcept
  {
    std::swap(_datav, other._datav);
    return *this;
  }
  bool is_int() const
  {
    return (_intv&1) != 0;
  };
  intptr_t as_int(intptr_t def=0) const
  {
    return is_int() ? (_intv>>1): def;
  };
  bool has_data() const
  {
    // in practice, data zones are 64 bits aligned. So its 3 least
    // significant bits are 0, when looking at it as some pointer.
    return  (_intv&7) == 0  && _datav != nullptr;
  };
  const Rps_ZoneValue* as_data(Rps_ZoneValue*def=nullptr) const
  {
    return has_data() ? _datav : def;
  };
  const Rps_ZoneValue* unsafe_data() const
  {
    return _datav;
  };
  bool is_empty() const
  {
    return _datav == nullptr || _datav == (Rps_ZoneValue*)RPS_EMPTYSLOT;
  };
  struct int_tag {};
  Rps_Value(int_tag, intptr_t i) : _intv((i<<1)|1) {};
  static Rps_Value of_int(intptr_t i=0)
  {
    return Rps_Value(int_tag{},i);
  };
  struct object_tag {};
  inline Rps_Value(object_tag, const Rps_ObjectZone*pob=nullptr);
  struct set_tag {};
  inline Rps_Value(set_tag, const Rps_SetObrefZone*pset=nullptr);
  struct tuple_tag {};
  inline Rps_Value(tuple_tag, const Rps_TupleObrefZone*ptup= nullptr);
  inline Rps_ObjectZone* as_object() const;
  inline const Rps_TupleObrefZone* as_tuple() const;
  inline const Rps_SetObrefZone* as_set() const;
  inline const Rps_SequenceObrefZone*as_sequence() const;
  Rps_ZoneValue*scanned_quasivalue(Rps_CallFrameZone*callingfra) const;
protected:
  void scan_value(Rps_CallFrameZone* callingfra) const
  {
    if (is_empty() || is_int()) return;
    _datav = scanned_quasivalue(callingfra);
  };
};				// end of Rps_Value



////////////////////////////////////////////////////////////////
/// A garbage collected memory zone inherits from Rps_ZoneValue.



// these macros are usable inside any function using RPS_LOCALFRAME
#define RPS_ALLOCATE_WITH_GAP(Gap, ...) rps_allocate_with_gap(RPS_CURFRAME, (Gap), ##__VA_ARGS__)
#define RPS_TYPED_ALLOCATE_WITH_GAP(Typ,Gap,...) rps_allocate_with_gap<Typ>(RPS_CURFRAME, (Gap), ##__VA_ARGS__)
#define RPS_ALLOCATE(...) rps_allocate(RPS_CURFRAME, ##__VA_ARGS__)
#define RPS_TYPED_ALLOCATE(Typ,...) rps_allocate<Typ>(RPS_CURFRAME, ##__VA_ARGS__)

/// Actually, not all zones represent a genuine values, some of them
/// have special types and are quasi-values.
class alignas(alignof(Rps_Value)) Rps_ZoneValue
{
  friend class Rps_Value;
  friend class Rps_GarbageCollector;
  friend class Rps_QuasiToken;
  enum Rps_Type _vtyp;
protected:
  struct zone_tag {};
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
public:
  template <typename ValClass, class ...Args>
  static ValClass* rps_allocate_with_gap(Rps_CallFrameZone*calfram,
                                         unsigned gap, Args... args)
  {
    assert (gap % alignof(ValClass) == 0);
    void* ptr = ValClass::allocate_rps_zone(sizeof(ValClass)+gap,calfram);
    ValClass* result = new(ptr) ValClass(args...);
    return result;
  }
public:
  template <typename ValClass, class ...Args>
  static ValClass* rps_allocate(Rps_CallFrameZone*calfram, Args... args)
  {
    size_t siz = sizeof(ValClass);
    assert (siz % alignof(ValClass) == 0);
    ValClass* result = nullptr;
    void* ptr = ValClass::allocate_rps_zone(siz,calfram);
    result = new(ptr) ValClass(args...);
    return result;
  }

  Rps_ZoneValue(Rps_Type ty) : _vtyp(ty)
  {
    assert ((int)ty>0
            || ty==Rps_TyCallFrame
            || ((int)ty<Rps_TyInt && ty>=rps_ty_min_quasi)
            || ty==Rps_TyDumper
            || ty==Rps_TyLoader);
  };
public:
  inline void rps_write_barrier(Rps_CallFrameZone*fr);
  Rps_Type type() const
  {
    return _vtyp;
  };
  template <class ValType>
  const ValType* as() const
  {
    if (type() == ValType::zone_type)
      return reinterpret_cast<const ValType*>(this);
    return nullptr;
  }
  static constexpr uint32_t maxsize = 1U << 30;
  // in some cases, for performance reasons, we specialize for very
  // common tiny sized values, below ...
  static constexpr uint32_t tinysize = 16;
  template<typename ZoneTy> static constexpr uint32_t alignment = alignof(ZoneTy);
  // a failure method when making something too big; currently it is
  // abort-ing, in the future it might throw some exception....
  [[noreturn]] static void fail_too_big_zone (const Rps_ZoneValue*zv, unsigned long wantedsize, const char*reason=nullptr)
  {
    if (reason) fprintf(stderr, "fail too big %ld @%p (%s)\n", wantedsize, (const void*) zv, reason);
    else fprintf(stderr, "fail too big %ld @%p\n", wantedsize, (const void*)zv);
    abort();
  }
  [[noreturn]] static void fail_too_big_zone(unsigned long wantedsize, const char*reason=nullptr)
  {
    if (reason) fprintf(stderr, "fail too big %ld (%s)\n", wantedsize, reason);
    else fprintf(stderr, "fail too big %ld\n", wantedsize);
    abort();
  }
  [[noreturn]] void fail_too_big(unsigned long wantedsize, const char*reason=nullptr)
  {
    fail_too_big_zone(this, wantedsize, reason);
  }
  inline Rps_ZoneValue* scanned_quasivalue(Rps_CallFrameZone* callingfra);
  static void scan_quasivalue(Rps_ZoneValue* &zv, Rps_CallFrameZone* callingfra)
  {
    if  (zv == nullptr || zv == RPS_EMPTYSLOT)
      return;
    zv = zv->scanned_quasivalue(callingfra);
  }
protected:
  void mutate_type(Rps_Type ty)
  {
    assert ((int)ty >= 0);
    _vtyp = ty;
  };
public:
  static void initialize(void);
};				// end of Rps_ZoneValue



class Rps_CallFrameZone : public Rps_ZoneValue
{
  friend class Rps_Loader;
  friend class Rps_GarbageCollector;
  const unsigned _cf_size;
  int _cf_state;
  Rps_CallFrameZone* _cf_prev;
  Rps_ObjectRef _cf_descr;
protected:
  Rps_ZoneValue* _cf_valarr[RPS_FLEXIBLE_DIM];
  Rps_CallFrameZone (unsigned siz,
                     Rps_CallFrameZone*callingframe,
                     Rps_ObjectRef descr)
    : Rps_ZoneValue(Rps_TyCallFrame), _cf_size(siz), _cf_state(0),
      _cf_prev(callingframe), _cf_descr(descr)
  {
    memset(_cf_valarr, 0, siz*sizeof(Rps_ZoneValue*));
  };
  ~Rps_CallFrameZone()
  {
    _cf_state = -1;
    _cf_prev = nullptr;
    _cf_descr = nullptr;
    memset(_cf_valarr, 0, _cf_size*sizeof(Rps_ZoneValue*));
  };
  inline Rps_CallFrameZone* scanned_quasivalue(Rps_CallFrameZone* callingfra);
public:
  static constexpr Rps_CallFrameZone* _bottom_frame_ = nullptr;
  int state() const
  {
    return _cf_state;
  };
  Rps_CallFrameZone* prev() const
  {
    return _cf_prev;
  };
  void set_state(int st)
  {
    _cf_state = st;
  };
  Rps_ObjectRef descr() const
  {
    return _cf_descr;
  };
  Rps_ZoneValue* operator[] (int ix) const
  {
    if (ix>=0 && ix<(int)_cf_size) return _cf_valarr[ix];
    return nullptr;
  };
  void* data() const
  {
    return const_cast<void*>((const void*)_cf_valarr);
  };
};				// end Rps_CallFrameZone

template<unsigned siz> class Rps_SizedCallFrameZone
  : public Rps_CallFrameZone
{
public:
  Rps_SizedCallFrameZone(Rps_CallFrameZone*callingframe,
                         Rps_ObjectRef descr) :
    Rps_CallFrameZone(siz, callingframe, descr) {};
  ~Rps_SizedCallFrameZone() {};
  static constexpr unsigned _const_size_ = siz;
};				// end Rps_SizedCallFrameZone

#define RPS_CURFRAME ((Rps_CallFrameZone*)&_)
#define RPS_WRITE_BARRIER() rps_write_barrier(RPS_CURFRAME)

#define RPS_LOCALFRAME_AT(Lin,PrevFrame,Descr,...)		\
  struct rps_localvars_##Lin { __VA_ARGS__ };			\
  constexpr unsigned _rps_localframesize_##Lin			\
  = sizeof(rps_localvars_##Lin)/sizeof(Rps_ZoneValue*);		\
  class  Rps_Framecl_##Lin :					\
    public Rps_SizedCallFrameZone<_rps_localframesize_##Lin> {	\
public:								\
Rps_Framecl_##Lin(Rps_CallFrameZone*callingframe##Lin,		\
                         Rps_ObjectRef descr##Lin)		\
: Rps_SizedCallFrameZone<_rps_localframesize_##Lin>		\
 (callingframe##Lin,descr##Lin) {};				\
__VA_ARGS__;							\
} /*end class  Rps_Framecl_##Lin*/;				\
Rps_Framecl_##Lin _(PrevFrame,Descr)

#define RPS_LOCALFRAME_AT_BIS(Lin,PrevFrame,Descr,...) \
  RPS_LOCALFRAME_AT(Lin,PrevFrame,Descr,__VA_ARGS__)

#define RPS_LOCALFRAME(PrevFrame,Descr,...) \
  RPS_LOCALFRAME_AT_BIS(__LINE__,(PrevFrame),(Descr), __VA_ARGS__)


////////////////////////////////////////////////////////////////



/***
 *  An instance of Rps_PointerCopyingZoneValue is some
 *  garbage-collected memory zone which is copied or moved by the GC
 *  and which contains internal pointers that should be scanned and/or
 *  forwarded.  It might perhaps also contain mutable non-GC-pointer
 *  data.
 ***/
class Rps_PointerCopyingZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_GarbageCollector;
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
  Rps_PointerCopyingZoneValue(Rps_Type ty) : Rps_ZoneValue(ty) {};
  inline Rps_PointerCopyingZoneValue* scanned_quasivalue(Rps_CallFrameZone* callingfra);
public:
};				// end class Rps_PointerCopyingZoneValue
////////////////////////////////////////////////////////////////



/***
 *  An instance of Rps_MutableCopyingZoneValue is some
 *  garbage-collected memory zone which is mutable (its content can
 *  change, and the changeable content contains GC-managed pointers),
 *  contains internal pointers that should be scanned and/or
 *  forwarded.  When mutation happens, the write_barrier method should
 *  be called to inform the GC that the zone contains changed GC-ed
 *  pointers.
 ***/
class Rps_MutableCopyingZoneValue : public Rps_PointerCopyingZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_GarbageCollector;
  Rps_MutableCopyingZoneValue(Rps_Type ty)
    : Rps_PointerCopyingZoneValue(ty) {};
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
  inline Rps_MutableCopyingZoneValue* scanned_quasivalue(Rps_CallFrameZone* callingfra);
public:
};				// end class Rps_MutableCopyingZoneValue


////////////////////////////////////////////////////////////////
/***
 * An instance of Rps_ScalarCopyingZoneValue is some garbage-collected
 * memory zone which is copied or moved by the GC and does not contain
 * any internal pointers to GC-ed zones.
 ***/
class Rps_ScalarCopyingZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_GarbageCollector;
protected:
  Rps_ScalarCopyingZoneValue(Rps_Type ty) : Rps_ZoneValue(ty) {};
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
  inline Rps_ScalarCopyingZoneValue* scanned_quasivalue(Rps_CallFrameZone* callingfra);
public:
};				// end class Rps_ScalarCopyingZoneValue


////////////////////////////////////////////////////////////////
/***
 * An instance of Rps_MarkSweepZoneValue is some garbage-collected
 * memory zone which is handled with a mark and sweep approach and is
 * not moved or copied by the garbage collector. Hence RefPerSys
 * objects, or GC-ed zones containing C++ container fields, are
 * instances of subclasses of Rps_MarkSweepZoneValue.
 ***/
class Rps_MarkSweepZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_ObjectZone;
  friend class Rps_GarbageCollector;
protected:
  virtual ~Rps_MarkSweepZoneValue() {};
  Rps_MarkSweepZoneValue(Rps_Type ty) : Rps_ZoneValue(ty) {};
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
  inline Rps_MarkSweepZoneValue* scanned_quasivalue(Rps_CallFrameZone* callingfra);
public:
};				// end class Rps_MarkSweepZoneValue



class Rps_CopyingHashedZoneValue : public Rps_PointerCopyingZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_PointerCopyingZoneValue;
protected:
  Rps_HashInt _hash;
  Rps_CopyingHashedZoneValue(Rps_Type ty, Rps_HashInt h=0) :
    Rps_PointerCopyingZoneValue(ty), _hash(h) {};
  void set_hash(Rps_HashInt h)
  {
    assert(h != 0);
    _hash = h;
  };
public:
  Rps_HashInt hash() const
  {
    return _hash;
  };
};				// end class Rps_CopyingHashedZoneValue



class Rps_CopyingSizedZoneValue : public Rps_CopyingHashedZoneValue
{
  friend class Rps_CopyingHashedZoneValue;
  friend class Rps_ZoneValue;
protected:
  uint32_t _size;
  Rps_CopyingSizedZoneValue(Rps_Type ty, uint32_t siz=0, Rps_HashInt h=0)
    : Rps_CopyingHashedZoneValue(ty, h), _size(siz) {};
  void set_size(uint32_t siz)
  {
    assert (_size == 0);
    _size = siz;
  };
public:
  uint32_t size() const
  {
    return _size;
  };
};				// end class Rps_CopyingSizedZoneValue


////////////////
/// quasi attribute arrays are not first-class values, just
/// quasivalues, but are pointed from most objects, those having few
/// attributes (up to perhaps a hundred of them)
class Rps_QuasiAttributeArray : public Rps_PointerCopyingZoneValue
{
  friend class Rps_ObjectZone;
  friend class Rps_ZoneValue;
  friend class Rps_PointerCopyingZoneValue;
  friend class Rps_GarbageCollector;
  // on a 64 bits machine, we are 8 byte aligned, so both of them are:
  uint32_t _qsizattr;	// allocated size
  uint32_t _qnbattrs;	// used number of entries
  std::pair<Rps_ObjectRef,Rps_Value> _qatentries[RPS_FLEXIBLE_DIM];
  // this constructor is private, it is called inside Rps_ObjectZone
  Rps_QuasiAttributeArray(unsigned siz, unsigned nb, const std::pair<Rps_ObjectRef,Rps_Value>*arr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiAttributeArray), _qsizattr(siz), _qnbattrs(nb)
  {
    assert(nb < std::numeric_limits<uint16_t>::max());
    assert(siz <= nb);
    assert(nb==0 || arr != nullptr);
    memset((void*)_qatentries, 0, siz*sizeof(std::pair<Rps_ObjectRef,Rps_Value>));
    if (nb>0 && arr)
      memcpy((void*)_qatentries, arr, nb*sizeof(std::pair<Rps_ObjectRef,Rps_Value>));
  }
  static inline const std::pair<Rps_ObjectRef,Rps_Value> nullpair{nullptr,nullptr};
public:
  struct entry_compare_st
  {
    bool operator ()
    (const std::pair<Rps_ObjectRef,Rps_Value>&p1,
     const std::pair<Rps_ObjectRef,Rps_Value>&p2)
    const
    {
      return p1.first < p2.first;
    }
  };
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
  std::pair<Rps_ObjectRef,Rps_Value>* begin()
  {
    return _qatentries;
  }
  std::pair<Rps_ObjectRef,Rps_Value>* end()
  {
    return _qatentries+_qnbattrs;
  }
  uint32_t allocated_size() const
  {
    return _qsizattr;
  };
  uint32_t count() const
  {
    return _qnbattrs;
  };
  const std::pair<Rps_ObjectRef,Rps_Value>&operator [] (int ix) const
  {
    if (ix<0) ix += _qnbattrs;
    if (ix>=0 && ix < (int)_qnbattrs)
      return _qatentries[ix];
    return nullpair;
  };
  const std::pair<Rps_ObjectRef,Rps_Value>&unsafe_at(int ix) const
  {
    return _qatentries[ix];
  };
  Rps_QuasiAttributeArray&
  put_at(int ix, Rps_ObjectRef keyob, Rps_Value va)
  {
    if (ix<0) ix += _qnbattrs;
    if (ix>=0 && ix<(int)_qnbattrs && keyob && va)
      {
        _qatentries[ix].first = keyob;
        _qatentries[ix].second = va;
      }
    return *this;
  };
  const Rps_ObjectRef attr_at(int ix) const
  {
    if (ix<0) ix += _qnbattrs;
    if (ix>=0 && ix<(int)_qnbattrs)
      return _qatentries[ix].first;
    return nullptr;
  }
  const Rps_ObjectRef unsafe_attr_at(int ix) const
  {
    return _qatentries[ix].first;
  }
  const Rps_Value val_at(int ix) const
  {
    if (ix<0) ix += _qnbattrs;
    if (ix>=0 && ix<(int)_qnbattrs)
      return _qatentries[ix].second;
    return nullptr;
  }
  const Rps_Value unsafe_val_at(int ix) const
  {
    return _qatentries[ix].second;
  }
};				// end class Rps_QuasiAttributeArray




////////////////
/// quasi components vectors are not first-class values, just
/// quasivalues, but are pointed from most objects, those having
/// some components.
class Rps_QuasiComponentVector : public Rps_PointerCopyingZoneValue
{
  friend class Rps_ObjectZone;
  friend class Rps_ZoneValue;
  friend class Rps_PointerCopyingZoneValue;
  // on a 64 bits machine, we are 8 byte aligned, so both of them are:
  uint32_t _qsizarr;	// allocated size
  uint32_t _qnbcomp;	// used number of components
  Rps_Value _qarrval[RPS_FLEXIBLE_DIM];
  // this constructor is private, it is called inside Rps_ObjectZone
  Rps_QuasiComponentVector(unsigned siz, unsigned nb, const Rps_Value*arr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiComponentVector), _qsizarr(siz), _qnbcomp(nb)
  {
    assert(nb < std::numeric_limits<uint16_t>::max());
    assert(siz <= nb);
    assert(nb==0 || arr != nullptr);
    memset((void*)_qarrval, 0, siz*sizeof(std::pair<Rps_ObjectRef,Rps_Value>));
    if (nb>0 && arr)
      memcpy((void*)_qarrval, arr, nb*sizeof(std::pair<Rps_ObjectRef,Rps_Value>));
  }
public:
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
  static unsigned constexpr _min_alloc_size_ = 5;
  uint32_t allocated_size() const
  {
    return _qsizarr;
  };
  uint32_t count() const
  {
    return _qnbcomp;
  };
  Rps_Value operator [] (int ix) const
  {
    if (ix<0) ix += _qnbcomp;
    if (ix>=0 && ix<(int)_qnbcomp)
      return _qarrval[ix];
    return nullptr;
  };
  Rps_Value& operator [] (int ix)
  {
    if (ix<0) ix += _qnbcomp;
    if (ix>=0 && ix<(int)_qnbcomp)
      return _qarrval[ix];
    throw std::out_of_range("QuasiComponentVector out of range");
  };
  Rps_Value unsafe_at(int ix) const
  {
    return  _qarrval[ix];
  };
#warning Rps_QuasiComponentVector is incomplete
};    // end class Rps_QuasiComponentVector


//
////////////////////////////////////////////////////////////////
/// the loader is stack allocated and has a pseudo call frame.
class Rps_Loader : Rps_ZoneValue
{
#warning Rps_Loader is very incomplete
  friend int main(int, char**);
  // the structure containing all quasivalue pointers needed for loading
  struct loadpointers_st
  {
    /// to be completed, but only GC-ed pointers here!
    Rps_Value lp_v1; /// for example
    /// TODO: Abhishek, please name lp_XXXX other GC-ed pointersd here
  };
  static constexpr unsigned _nb_gc_pointers = sizeof(struct loadpointers_st) /sizeof(void*);
  Rps_SizedCallFrameZone<_nb_gc_pointers> _ld_pseudoframe;
  std::string _ld_dirname;
  /// add private member functions; each of them can access the
  /// quasipointer Foo of the loader using RPS_LDATA(Foo) notation.
  void example_func(void);
  void example_gc_func(Rps_CallFrameZone*callingfra);
  //// TODO: Abhishek, please add other loading functions here.
protected:
  struct loadpointers_st& rps_loadpointers()
  {
    return
      *reinterpret_cast<struct loadpointers_st*>(_ld_pseudoframe.data());
  };
#define RPS_LDATA(Field) rps_loadpointers().Field
  Rps_Loader(const char*dirname) : Rps_ZoneValue(Rps_TyLoader),
    _ld_pseudoframe(nullptr,nullptr), _ld_dirname(dirname)
  {
    assert (dirname != nullptr);
  };
  ~Rps_Loader() {};
public:
  Rps_CallFrameZone* call_frame(void)
  {
    return &_ld_pseudoframe;
  };
  const std::string& directory_name(void) const
  {
    return _ld_dirname;
  };
  void do_load(void);
}; // end class Rps_Loader





////////////////////////////////////////////////////////////////
/// the dumper is stack allocated and has a pseudo call frame.
class Rps_Dumper : Rps_ZoneValue
{
#warning Rps_Dumper is very incomplete
  friend int main(int, char**);
  // the structure containing all quasivalue pointers needed for loading
  struct dumpointers_st
  {
    /// to be completed, but only GC-ed pointers here!
    Rps_Value dp_v1; /// for example
    /// TODO: Abhishek, please name dp_XXX other pointers here
  };
  static constexpr unsigned _nb_gc_pointers = sizeof(struct dumpointers_st) /sizeof(void*);
  Rps_SizedCallFrameZone<_nb_gc_pointers> _du_pseudoframe;
  std::string _du_dirname;
  /// add private member functions; each of them can access the
  /// quasipointer Foo of the dumper using RPS_DUDAT(Foo) notation.
  void example_func(void);
  void example_gc_func(Rps_CallFrameZone*callingfra);
  //// TODO: Abhishek, please add other functions for dumping
  ////
  struct dumpointers_st& rps_dumpointers()
  {
    return
      *reinterpret_cast<struct dumpointers_st*>(_du_pseudoframe.data());
  };
#define RPS_DUDAT(Field) rps_dumpointers().Field
protected:
  Rps_Dumper(const char*dirname) : Rps_ZoneValue(Rps_TyDumper),
    _du_pseudoframe(nullptr,nullptr), _du_dirname(dirname)
  {
    assert (dirname != nullptr);
  };
  ~Rps_Dumper() {};
public:
  Rps_CallFrameZone* call_frame(void)
  {
    return &_du_pseudoframe;
  };
  void do_dump(void);
}; // end class Rps_Dumper


////////////////
class Rps_SequenceObrefZone : public Rps_CopyingSizedZoneValue
{
  friend class Rps_ZoneValue;
public:
  static size_t byte_gap_for_size(uint32_t siz)
  {
    return sizeof(Rps_ObjectRef)*siz;
  };
  // useful in for-range loops:
  const Rps_ObjectRef* begin() const
  {
    return &_obarr[0];
  };
  const Rps_ObjectRef* end() const
  {
    return &_obarr[size()];
  };
protected:
  Rps_SequenceObrefZone(Rps_Type ty, uint32_t siz=0,  Rps_HashInt h=0, const Rps_ObjectRef*arr = nullptr)
    : Rps_CopyingSizedZoneValue(ty,siz,h)
  {
    if (arr != nullptr && siz>0)
      memcpy(_obarr, arr, sizeof(Rps_ObjectRef)*siz);
  };
protected:
  ~Rps_SequenceObrefZone() {};
  union   // at least one word needs to be "allocated", hence the _unused field
  {
    intptr_t _unused;
    Rps_ObjectRef _obarr[RPS_FLEXIBLE_DIM];
  };
  static Rps_HashInt hash_of_array(Rps_Type ty, uint32_t siz, const Rps_ObjectRef *arr);
  void compute_hash(void)
  {
    set_hash(hash_of_array(type(), size(), _obarr));
  };
  void mutate(Rps_Type ty)
  {
    assert (ty == Rps_TyTuple || ty == Rps_TySet);
    mutate_type(ty);
    compute_hash();
  };
  // mutate a value which is not reachable, so its hash does not need
  // to be updated.
  void mutate_nohash(Rps_Type ty)
  {
    assert (ty == Rps_TyTuple || ty == Rps_TySet);
    mutate_type(ty);
  }
};				// end Rps_SequenceObrefZone





////////////////
class Rps_TupleObrefZone : public Rps_SequenceObrefZone
{
  friend class Rps_SequenceObrefZone;
  friend class Rps_PointerCopyingZoneValue;
  friend class Rps_ZoneValue;
  friend class Rps_SetObrefZone;
private:
  Rps_TupleObrefZone(uint32_t siz, const Rps_ObjectRef*arr) :
    Rps_SequenceObrefZone(Rps_TyTuple, siz,
                          hash_of_array(Rps_TyTuple, siz, arr),
                          arr) { };
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
public:
  static constexpr Rps_Type zone_type = Rps_TyTuple;
  static Rps_TupleObrefZone* make(Rps_CallFrameZone*,uint32_t siz, const Rps_ObjectRef*arr);
  static Rps_TupleObrefZone* make(Rps_CallFrameZone*callingfra,const std::initializer_list<const Rps_ObjectRef> il)
  {
    return make(callingfra,il.size(), il.begin());
  };
};				// end of Rps_TupleObrefZone
Rps_Value::Rps_Value(tuple_tag, const Rps_TupleObrefZone*ptup) :
  Rps_Value(ptup) {};


////////////////
class Rps_TupleValue : public Rps_Value
{
  Rps_TupleValue(nullptr_t) : Rps_Value(nullptr) {};
public:
  void put_tuple(const Rps_TupleObrefZone*ptup)
  {
    assert (ptup == nullptr || ptup->type() == Rps_TyTuple);
    put_data(ptup);
  };
  Rps_TupleValue(Rps_Value val) : Rps_Value(val.as_tuple()) {};
  Rps_TupleValue(Rps_CallFrameZone*callingfra,uint32_t siz, Rps_ObjectRef const* arr)
    : Rps_Value(Rps_TupleObrefZone::make(callingfra,siz, arr)) {};
  Rps_TupleValue(Rps_CallFrameZone*callingfra,const std::initializer_list<const Rps_ObjectRef> il)
    : Rps_Value(Rps_TupleObrefZone::make(callingfra,il)) {};
  struct collect_tag {};
  // make a tuple from a collection of values, using only objects and
  // sequences from them and ignoring other values
  Rps_TupleValue(Rps_CallFrameZone*callingfra,collect_tag, const std::initializer_list<const Rps_Value> il);
  Rps_TupleValue(Rps_CallFrameZone*callingfra,collect_tag, uint32_t siz, const Rps_Value*arr);
  static Rps_TupleValue collect(Rps_CallFrameZone*callingfra,const std::initializer_list<const Rps_Value> il)
  {
    return Rps_TupleValue(callingfra,collect_tag{}, il);
  };
  static Rps_TupleValue collect(Rps_CallFrameZone*callingfra,uint32_t siz, const Rps_Value*arr)
  {
    return Rps_TupleValue(callingfra,collect_tag{}, siz, arr);
  };
};				// end Rps_TupleValue

const Rps_TupleObrefZone*
Rps_Value::as_tuple() const
{
  if (has_data())
    {
      return const_cast<const Rps_TupleObrefZone*>(unsafe_data()->as<Rps_TupleObrefZone>());
    }
  return nullptr;
} // end Rps_Value::as_tuple

////////////////

class Rps_SetObrefZone : public Rps_SequenceObrefZone
{
  friend class Rps_SequenceObrefZone;
  friend class Rps_PointerCopyingZoneValue;
  friend class Rps_ZoneValue;
  friend class Rps_TupleObrefZone;
private:
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
  Rps_SetObrefZone(uint32_t siz, const Rps_ObjectRef*arr) :
    Rps_SequenceObrefZone(Rps_TySet, siz,
                          hash_of_array(Rps_TySet, siz, arr),
                          arr) { };
  Rps_SetObrefZone(std::nullptr_t, uint32_t siz, const Rps_ObjectRef*arr) :
    Rps_SequenceObrefZone(Rps_TySet, siz,
                          0,
                          arr) { };
public:
  static constexpr Rps_Type zone_type = Rps_TySet;
  static Rps_SetObrefZone* make(Rps_CallFrameZone*callingfra,uint32_t siz, const Rps_ObjectRef*arr);
  static Rps_SetObrefZone* make(Rps_CallFrameZone*callingfra,const std::initializer_list<const Rps_ObjectRef> il)
  {
    return make(callingfra,il.size(), il.begin());
  };
}; // end of Rps_SetObrefZone

Rps_Value::Rps_Value(set_tag, const Rps_SetObrefZone*pset) :
  Rps_Value(pset) {};

class Rps_SetValue : public Rps_Value
{
public:
  Rps_SetValue() : Rps_SetValue(nullptr) {};
  Rps_SetValue(std::nullptr_t) : Rps_Value(nullptr) {};
  Rps_SetValue(Rps_Value val) : Rps_Value(val.as_set()) {};
  Rps_SetValue(Rps_CallFrameZone*callingfra,uint32_t siz, Rps_ObjectRef const* arr)
    : Rps_Value(Rps_SetObrefZone::make(callingfra,siz, arr)) {};
  Rps_SetValue(Rps_CallFrameZone*callingfra,const std::initializer_list<const Rps_ObjectRef> il)
    : Rps_Value(Rps_SetObrefZone::make(callingfra,il)) {};
  static Rps_SetValue tiny_benchmark_1(Rps_CallFrameZone*callingfra, unsigned num);
};				// end Rps_SetValue

const Rps_SetObrefZone*
Rps_Value::as_set() const
{
  if (has_data())
    {
      return const_cast<const Rps_SetObrefZone*>(unsafe_data()->as<Rps_SetObrefZone>());
    }
  return nullptr;
} // end Rps_Value::as_set


////////////////
const Rps_SequenceObrefZone*
Rps_Value::as_sequence() const
{
  if (has_data())
    {
      auto da = unsafe_data();
      if (da->type() == Rps_TySet || da->type() == Rps_TyTuple)
        return (const Rps_SequenceObrefZone*)da;
    };
  return nullptr;
} // end Rps_Value::as_sequence

////////////////

// Notice that NaN cannot be boxed as a double, since it is incomparable
class Rps_DoubleZone : public  Rps_ScalarCopyingZoneValue
{
private:
  intptr_t _unused;
  double _dbl;
  Rps_DoubleZone(double d=0.0) :
    Rps_ScalarCopyingZoneValue(Rps_TyDouble), _dbl(d)
  {
    assert (!std::isnan(d));
  };
public:
  static constexpr Rps_Type zone_type = Rps_TyDouble;
  static Rps_DoubleZone* make(double d=0.0);
};				// end Rps_DoubleZone

class Rps_DoubleValue : public Rps_Value
{
  Rps_DoubleValue(nullptr_t) : Rps_Value(nullptr) {};
public:
  void put_double(double d)
  {
    if (std::isnan(d)) put_data(nullptr);
    else put_data(Rps_DoubleZone::make(d));
  };
  Rps_DoubleValue(double d=0.0) : Rps_DoubleValue(nullptr)
  {
    if (std::isnan(d)) put_data(nullptr);
    else put_data(Rps_DoubleZone::make(d));
  };
};				// end Rps_DoubleValue

////////////////

class Rps_StringZone : public Rps_ScalarCopyingZoneValue
{
  friend class Rps_ScalarCopyingZoneValue;
  friend class Rps_ZoneValue;
  friend class Rps_StringValue;
public:
  static Rps_HashInt hash_cstr(const char*cstr, int32_t slen= -1);
  static size_t byte_gap_for_size(uint32_t siz)
  {
    return (siz+sizeof(void*)-sizeof(_unused)+1)&(~(size_t)(alignof(Rps_Value)-1));
  }
private:
  uint32_t _strlen;
  Rps_HashInt _strhash;
  union
  {
    intptr_t _unused;
    char _strbytes[RPS_FLEXIBLE_DIM];
  };
protected:
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
  Rps_StringZone(const char*sbytes, int32_t slen= -1, bool skipcheck=false) :
    Rps_ScalarCopyingZoneValue(Rps_TyString),
    _strlen((uint32_t)(slen<0)?(sbytes?(slen=strlen(sbytes)):(slen=0)):slen), _strhash(0)
  {
    if (!skipcheck)
      assert(u8_check((const uint8_t*)sbytes, slen) == nullptr);
    memcpy(_strbytes, sbytes, slen);
    _strhash = hash_cstr(_strbytes, slen);
    _strbytes[slen] = 0;
  };
public:
  friend Rps_StringZone*
  Rps_ZoneValue::rps_allocate_with_gap<Rps_StringZone,const char*,int32_t,bool>
  (Rps_CallFrameZone*,unsigned,const char*,int32_t,bool);
  static Rps_StringZone* make(Rps_CallFrameZone*callingfra, const char*sbytes, int32_t slen= -1);
  static constexpr Rps_Type zone_type = Rps_TyString;
  uint32_t size() const
  {
    return _strlen;
  };
  /// use very carefully, since Rps_StringZone-s are moved by the
  /// garbage collector.
  const char*unsafe_strbytes() const
  {
    return _strbytes;
  };
  /// return a copy of the string, since Rps_StringZone-s are moved
  std::string string() const
  {
    return std::string(_strbytes, _strlen);
  };
};				// end of Rps_StringZone



class Rps_StringValue : public Rps_Value
{
  Rps_StringValue(nullptr_t) : Rps_Value(nullptr) {};
public:
  void put_string(Rps_CallFrameZone*callingfra,const char*cstr, int slen= -1)
  {
    if (!cstr || cstr==RPS_EMPTYSLOT) slen=0;
    else put_data(Rps_StringZone::make(callingfra,cstr,slen));
  };
  void put_string(Rps_CallFrameZone*callingfra,const std::string&str)
  {
    put_string(callingfra,str.c_str(), str.size());
  };
  Rps_StringValue() : Rps_StringValue(nullptr) {};
  Rps_StringValue(Rps_CallFrameZone*callingfra, const char*cstr, int slen= -1)
    : Rps_StringValue(nullptr)
  {
    put_string(callingfra, cstr, slen);
  };
  Rps_StringValue(Rps_CallFrameZone*callingfra, const std::string&str)
    : Rps_StringValue(nullptr)
  {
    put_string(callingfra,str);
  };
  // return a *copy* of the string, since the contained Rps_StringZone
  // is potentially *moved* by our garbage collector at arbitrary times.
  std::string string() const
  {
    auto data = unsafe_data();
    if (data) return reinterpret_cast<const Rps_StringZone*>(data)->string();
    else return std::string();
  }
  uint32_t size() const
  {
    auto data = unsafe_data();
    if (data) return reinterpret_cast<const Rps_StringZone*>(data)->size();
    else return 0;
  }
  /// use very carefully, since Rps_StringZone-s are moved by our
  /// garbage collector.
  const char*unsafe_strbytes() const
  {
    auto data = unsafe_data();
    if (data) return reinterpret_cast<const Rps_StringZone*>(data)->unsafe_strbytes();
    else return nullptr;
  }
};				// end Rps_StringValue

////////////////////////////////////////////////////////////////

template <typename DataType> class Rps_PayloadZone;
class Rps_PayloadInternalData
{
  friend class Rps_PayloadZone<Rps_PayloadInternalData>;
protected:
  Rps_ObjectZone* _py_owner;
  virtual ~Rps_PayloadInternalData();
};				// end Rps_PayloadInternalData


template  <class PayloadDataType> class Rps_PayloadZone : Rps_MarkSweepZoneValue
{
  friend Rps_PayloadInternalData;
  friend PayloadDataType;
  PayloadDataType _py_data;
protected:
  Rps_PayloadZone() : Rps_MarkSweepZoneValue(Rps_TyPayload), _py_data() {};
  ~Rps_PayloadZone() {};
};				// end Rps_PayloadZone



class Rps_SetObjrefInternalData : Rps_PayloadInternalData
{
  friend class Rps_PaylSetObjrefZone;
  std::set<Rps_ObjectRef> _ida_set;
#warning Rps_SetObjrefInternalData should be implemented
public:
  virtual ~Rps_SetObjrefInternalData();
};				// end Rps_SetObjrefInternalData


class Rps_PaylSetObjrefZone
  : public Rps_PayloadZone<Rps_SetObjrefInternalData>
{
  friend Rps_SetObjrefInternalData;
  friend class Rps_ZoneValue;
  friend class Rps_MarkSweepZoneValue;
};				// end Rps_PaylSetObjrefZone;



////////////////
class Rps_ObjectZone : public  Rps_MarkSweepZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_ObjectRef;
  friend class Rps_MarkSweepZoneValue;
  friend class Rps_GarbageCollector;
  friend void print_types_info(void);
  const Rps_Id _ob_id;
  // we need some lock; we could later improve it, see perhaps
  // https://www.arangodb.com/2018/05/an-implementation-of-phase-fair-reader-writer-locks/
  mutable std::mutex _ob_mtxlock;
  /// the class of an object is so frequently used that it is an
  /// atomic pointer that does not need to be fetched under the mutex.
  std::atomic<Rps_ObjectZone*> _ob_atomclass;
  // TODO: we need not only the lock but also some kind of object buckets
  Rps_ObjectZone(const Rps_Id& oid, Rps_ObjectRef obclass= nullptr)
    : Rps_MarkSweepZoneValue(Rps_TyObject), _ob_id(oid), _ob_mtxlock(), _ob_atomclass(obclass),
      _obat_kind(atk_none), _obat_null(), _ob_compvec(nullptr)
  {
  };
  virtual ~Rps_ObjectZone(); /// called by the GC
  // object buckets are useful to manage the association between ids and objects
  struct BucketOb_st
  {
    std::mutex _bu_mtx;
    std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher,Rps_Id::EqualTester> _bu_map;
  };
  static BucketOb_st _ob_bucketarr[Rps_Id::maxbuckets];
  static BucketOb_st& bucket(const Rps_Id&id)
  {
    return _ob_bucketarr[id.bucket_num()];
  };
  BucketOb_st& bucket() const
  {
    return bucket(_ob_id);
  };
  Rps_ObjectZone(const Rps_Id& oid, const BucketOb_st& buck)
    : Rps_ObjectZone(oid)
  {
    assert (&buck == &bucket(oid));
  };
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
  ///
protected:
  void scan_object_content(Rps_CallFrameZone*callingfra) const;
protected:
  /*** Objects are quite common, and their attributes and components
       also.  Most objects have few attributes, since attributes are
       generalizing fields of objects (e.g. those of Java, Python,
       Common Lisp, or JavaScript); so we should special-case for that
       (as a plain array of entries) with efficiency in mind.  For few
       attributes, the best is probably a small array of
       (attribute-key-object, attribute-value) pairs which is seeked
       linearly, but is cache friendly.  In the more common case of at
       most a few dozens of attributes, putting then in some sorted
       array accessed dichotomically is a good solution.  For the very
       unusual case of hundred or much more of attributes, we could
       have a std::map (since we might want to iterate on attributes
       in reproducible way, e.g. by their ascending order).  So
       attributes are represented as a tagged union, and the usual
       case is a pointer to Rps_QuasiAttributeArray. Likewise, most
       objects have not many components, and these components are in
       some small dynamic array.

       The pathological case of an object with many thousands of
       components or thousands of attributes should be handled in
       principe, but could be coded later, or at least could be
       optimized much later.  We expect most objects to have no more
       than few dozen attributes, and no more than a few hundred
       components.  Any bigger case is pathological, and smaller
       objects are much more common than huge ones.
   ***/
private:
  //////////////// Attributes of objects.
  //
  ///// Bear in mind that we expect in most objects less than about a
  ///// few dozens of them, since attributes are generalizing fields
  ///// of objects (e.g. those of Java, Python, Common Lisp, or
  ///// JavaScript).  Objects with hundred of attributes are
  ///// practically rare, and big objects with thousands of them are
  ///// highly improbable (but semantically still possible, in
  ///// theory!) and that either is a practical symptom of a huge
  ///// engineering mess or would happen when RefPerSys would *invent*
  ///// attributes.
  enum attrkind_en {atk_none, atk_small, atk_medium, atk_big};
  attrkind_en _obat_kind;
public:
  ///// Thresholds below are fuzzy.  We add a bit of randomness; and
  ///// these are thresholds to make a growing decision, not for the
  ///// actually grown size which is a slightly bigger prime.
  static constexpr unsigned at_small_thresh= 10;
  ///// Above the sorted threshold, give or take a few units, we use a
  ///// std::map. But that case is practically rare enough that we
  ///// don't care, since very few objects would have more than a
  ///// hundred attributes.
  static constexpr unsigned at_sorted_thresh= 101; // a prime
  ///// the "fuzziness"
  static constexpr unsigned at_fuss = 8; // a power of two, at most 16!
  ///// initial size of _obat_small_atar:
  static constexpr unsigned at_small_initsize= 3;
  ///
private:
  union
  {
    std::nullptr_t _obat_null;		    // when atk_none
    /// small unordered attributes, up to at_small_thresh entries:
    Rps_QuasiAttributeArray* _obat_small_atar; // when atk_small
    // medium-sized ascending-ordered attributes, up to at_sorted_thresh entries:
    Rps_QuasiAttributeArray* _obat_sorted_atar; // when atk_medium
    // big-sized attributes in a std::map
    std::map<Rps_ObjectRef, Rps_Value> _obat_map_atar; // when atk_big
  };
  /* Nota Bene: the transition from small to sorted attributes happens
     smoothly and with some random noise, since we want to avoid too
     frequent oscillations between them (e.g. in the case when we add
     a few attributes, remove a few others of them, and so forth,
     around the thresholds.).  And likewise for the even less likely
     transition from sorted attributes to a big map of them. */
  //////////////// components of objects
  Rps_QuasiComponentVector* _ob_compvec;
  //////////////// methods
private:
  // approximate binary search dichotomy, internal method only
  std::pair<int,int> dichotomy_medium_sorted(Rps_ObjectRef keyob, Rps_QuasiAttributeArray*arr=nullptr) const
  {
    if (arr==nullptr)
      arr = _obat_sorted_atar;
    assert (_obat_kind == atk_medium);
    assert (arr != nullptr);
    assert (keyob);
    unsigned ln = arr->count();
    int lo = 0, hi = (int)ln;
    while (lo+4 < hi)
      {
        unsigned md = (lo+hi)/2;
        Rps_ObjectRef midob = arr->unsafe_attr_at(md);
        assert (midob);
        if (keyob == midob)
          {
            lo = md;
            hi = md;
            break;
          }
        else if (midob > keyob)
          hi = md;
        else
          lo = md;
      }
    return std::pair{lo,hi};
  };
public:
  /// find an object of given id, or else null:
  static Rps_ObjectZone*find_by_id(const Rps_Id&id);
  /// make a new object of given id, or find the existing one:
  static Rps_ObjectZone*make_of_id(Rps_CallFrameZone*,const Rps_Id&id);
  /// make a new object of random unused id:
  static Rps_ObjectZone*make(Rps_CallFrameZone*);
  ///
  ///
  Rps_HashInt hash() const
  {
    return _ob_id.hash();
  };
  const Rps_Id objid() const
  {
    return _ob_id;
  };
  static constexpr Rps_Type zone_type = Rps_TyObject;
  bool same (const Rps_ObjectZone*oth) const
  {
    if (!oth) return false;
    if (this != oth)
      {
        assert (_ob_id != oth->_ob_id);
        return false;
      }
    return true;
  }
  bool same (const Rps_ObjectZone&oth) const
  {
    return same(&oth);
  };
  bool different (const Rps_ObjectZone*oth) const
  {
    return ! (this == oth);
  };
  bool different (const Rps_ObjectZone&oth) const
  {
    return different(&oth);
  };
  bool less (const Rps_ObjectZone*oth)  const
  {
    if (!oth) return false;
    return _ob_id < oth->_ob_id;
  }
  bool less (const Rps_ObjectZone&oth) const
  {
    return less(&oth);
  };
  bool greater (const Rps_ObjectZone*oth)  const
  {
    if (!oth) return false;
    return _ob_id > oth->_ob_id;
  }
  bool greater(const Rps_ObjectZone&oth) const
  {
    return greater(&oth);
  };
  bool lessequal (const Rps_ObjectZone*oth)  const
  {
    if (!oth) return false;
    if (this == oth) return true;
    return _ob_id < oth->_ob_id;
  }
  bool lessequal(const Rps_ObjectZone&oth) const
  {
    return lessequal(&oth);
  };
  bool greaterequal (const Rps_ObjectZone*oth)  const
  {
    if (!oth) return false;
    if (this == oth) return true;
    return _ob_id < oth->_ob_id;
  }
  bool greaterequal (const Rps_ObjectZone&oth) const
  {
    return greaterequal(&oth);
  };
  bool operator == (const Rps_ObjectZone&oth) const
  {
    return same(oth);
  }
  bool operator != (const Rps_ObjectZone&oth) const
  {
    return different(oth);
  };
  bool operator < (const Rps_ObjectZone&oth) const
  {
    return less(oth);
  };
  bool operator > (const Rps_ObjectZone&oth) const
  {
    return greater(oth);
  };
  const Rps_ObjectRef object_class() const
  {
    auto obcla = _ob_atomclass.load();
    return obcla;
  }
  //// methods for attributes
  int nb_attrs() const
  {
    switch (_obat_kind)
      {
      case atk_none:
        return 0;
      case atk_small:
        return _obat_small_atar->count();
      case atk_medium:
        return _obat_sorted_atar->count();
      case atk_big:
        return _obat_map_atar.size();
      }
  };
  Rps_Value do_get_attr(Rps_ObjectRef keyob) const
  {
    switch (_obat_kind)
      {
      case atk_none:
        return nullptr;
      case atk_small:
      {
        auto smalattrs = _obat_small_atar;
        for (auto it: *smalattrs)
          {
            if (it.first == keyob)
              return it.second;
          }
        break;
      }
      case atk_medium:
      {
        unsigned ln = _obat_sorted_atar->count();
        assert (ln <= at_sorted_thresh);
        unsigned lo=0, hi=ln;
        {
          auto p = dichotomy_medium_sorted(keyob);
          lo= p.first;
          hi= p.second;
        }
        for (unsigned md = lo; md <hi; md++)
          {
            Rps_ObjectRef midob = _obat_sorted_atar->unsafe_attr_at(md);
            if (keyob == midob) return _obat_sorted_atar->unsafe_val_at(md);
          }
        break;
      }
      case atk_big:
      {
        auto it = _obat_map_atar.find(keyob);
        if (it != _obat_map_atar.end())
          return it->second;
        return nullptr;
      }
      return nullptr;
      }
  };				// end do_get_attr
  Rps_Value get_attr(Rps_ObjectRef keyob) const
  {
    // we'll probably special case some keys here later
    return do_get_attr(keyob);
  }
  void do_put_attr(Rps_CallFrameZone*callfra, Rps_ObjectRef keyob, Rps_Value valat);
  Rps_ObjectRef put_attr(Rps_CallFrameZone*callfra, Rps_ObjectRef keyob, Rps_Value valat)
  {
    do_put_attr(callfra, keyob, valat);
    return this;
  };
#warning mutating primitives may need an additional callingfra argument, see README.md
  void do_remove_attr(Rps_CallFrameZone*callfra, Rps_ObjectRef keyob);
  Rps_ObjectRef remove_attr(Rps_CallFrameZone*callfra, Rps_ObjectRef keyob)
  {
    do_remove_attr(callfra, keyob);
    return this;
  };
  //// methods for components
  unsigned nb_comps() const
  {
    if (_ob_compvec) return _ob_compvec->count();
    return 0;
  };
  Rps_Value get_comp(int ix) const
  {
    if (_ob_compvec) return (*_ob_compvec)[ix];
    return nullptr;
  };
  /// Resize the component vector to the given number of components,
  /// so perhaps grow or shrink it, if so needed.
  void do_resize_components (Rps_CallFrameZone*callfra, unsigned newnbcomp);
  Rps_ObjectRef resize_components(Rps_CallFrameZone*callfra, unsigned size)
  {
    do_resize_components(callfra, size);
    return this;
  };
  /// reserve the component vector to the given allocated size, with an empty count
  void do_reserve_components (Rps_CallFrameZone*callfra, unsigned size);
  Rps_ObjectRef reserve_components(Rps_CallFrameZone*callfra, unsigned size)
  {
    do_reserve_components(callfra, size);
    return this;
  }
  /// append a component
  void do_append_component(Rps_CallFrameZone*callfra, Rps_Value val);
  Rps_ObjectRef append_component(Rps_CallFrameZone*callfra, Rps_Value val)
  {
    do_append_component(callfra, val);
    return this;
  }
};				// end class Rps_ObjectZone

Rps_ObjectRef
Rps_ObjectRef::make(Rps_CallFrameZone*callingfra)
{
  return Rps_ObjectZone::make(callingfra);
}

////////////////


bool Rps_ObjectRef::operator == (const Rps_ObjectRef& oth) const
{
  if (_optr == oth._optr) return true;
  if (!_optr) return false;
  return _optr->same(oth);
}

bool Rps_ObjectRef::operator != (const Rps_ObjectRef& oth) const
{
  return !(*this == oth);
}

bool Rps_ObjectRef::operator < (const Rps_ObjectRef& oth) const
{
  if (_optr == oth._optr) return false;
  if (!oth._optr) return false;
  if (!_optr) return true;
  return _optr->less(oth);
}


bool Rps_ObjectRef::operator <= (const Rps_ObjectRef& oth) const
{
  if (_optr == oth._optr) return true;
  if (!oth._optr) return false;
  if (!_optr) return true;
  return _optr->less(oth);
}

bool  Rps_ObjectRef::operator >= (const Rps_ObjectRef& oth) const
{
  return oth <= *this;
}

bool  Rps_ObjectRef::operator > (const Rps_ObjectRef& oth) const
{
  return oth < *this;
}
void
Rps_ObjectRef::scan_objectref(Rps_CallFrameZone* callingfra) const
{
  if (!is_empty())
    _optr->scan_object_content(callingfra);
} // end Rps_ObjectRef::scan_objectref


Rps_Value::Rps_Value(object_tag, const Rps_ObjectZone*pob) :
  Rps_Value(pob) {};

////////////////////////////////////////////////////////////////

// to deal with integer values
class Rps_IntValue : public Rps_Value
{
public:
  Rps_IntValue(intptr_t i=0) : Rps_Value(Rps_Value::int_tag{},  i) {};
  operator intptr_t () const
  {
    return as_int();
  };
};				// end class Rps_IntValue

// to deal with object values
class Rps_ObjectValue : public Rps_Value
{
public:
  Rps_ObjectValue(Rps_Value val) :  Rps_Value(Rps_Value::object_tag{}, val.as_object()) {};
  Rps_ObjectValue(Rps_ObjectRef obr) : Rps_Value(Rps_Value::object_tag{}, obr.optr()) {};
  Rps_ObjectValue(Rps_ObjectZone* pob=nullptr) : Rps_Value(Rps_Value::object_tag{}, pob) {};
  operator Rps_ObjectRef () const
  {
    return as_object();
  };
  operator Rps_ObjectZone* () const
  {
    return as_object();
  };
}; // end Rps_ObjectValue

Rps_ObjectZone*
Rps_Value::as_object() const
{
  if (has_data())
    {
      return const_cast<Rps_ObjectZone*>(unsafe_data()->as<Rps_ObjectZone>());
    }
  return nullptr;
} // end Rps_Value::as_object


////////////////////////////////////////////////////////////////
class Rps_MutatorThread;
/**
 * The garbage collector
 *
 * There is no actual instance of Rps_GarbageCollector, but that class
 * is grouping static data and static member functions related to
 * garbage collection and allocation support.  Perhaps it should be a
 * C++ namespace, not a C++ class.
 */
class Rps_GarbageCollector
{
  friend class Rps_MutatorThread;
  // forbid instantiation:
  Rps_GarbageCollector() = delete;
  static unsigned constexpr _gc_thrmagic_ = 951957269 /*0x38bdb715*/;
  // a global flag which becomes set when GC is needed
  static std::atomic<bool> _gc_wanted;
  ////////////////
  // each worker thread should have its own
  struct thread_allocation_data
  {
    const unsigned _tha_magic; // always _gc_thrmagic_
    Rps_BirthMemoryBlock* _tha_birthblock;
  };
  static thread_local thread_allocation_data* _gc_thralloc_;
  ////////////////
  // for non-worker thread and large allocation, we have a global
  // static
  struct global_allocation_data
  {
    std::mutex _gla_mutex;
    Rps_BirthMemoryBlock* _gla_birthblock;
    Rps_LargeNewMemoryBlock* _gla_largenewblock;
  };
  static global_allocation_data _gc_globalloc_;
  ////////////////
public:
  // Scan a call stack and all the local frames in it:
  static void scan_call_stack(Rps_CallFrameZone*callingfra);
  // Forcibly run the garbage collector:
  static void run_garbcoll(Rps_CallFrameZone*callingfra);
  // Maybe run the garbage collector. Each allocating thread is
  // promising to call that at least once every few milliseconds,
  // unless special precautions are taken (disabling allocation and GC
  // in some non-worker thread doing a blocking operation like poll(2)
  // or read(2)...). Calling it more often makes almost no harm
  // (except a tiny performance loss). Calling it not often enough is
  // a severe condition and should be avoided, since it will impact
  // usability, latency and could even crash the system.
  static void maybe_garbcoll(Rps_CallFrameZone*callingfra)
  {
    if (_gc_wanted.load())
      run_garbcoll(callingfra);
  };
  // Call this quick function to give the intention of having the GC
  // being soon called, hopefully in the next few milliseconds.
  static void want_to_garbcoll(void)
  {
    _gc_wanted.store(true);
  }
  //////////////// write barrier
  /**
   * The write barrier should be very efficient, since it is crucial
   * for performance, because it will be called very often.
   * Conceptually, and by definition of the write barrier, the garbage
   * collector needs to process the unordered set of updated
   * quasivalues since the previous call to the GC.  Practically, when
   * there is a birth region, we use a Chesney like approach.
   **/
protected:
  // called when the birth region is missing or full:
  static void run_write_barrier(Rps_CallFrameZone*callingfra, Rps_ZoneValue*zva=nullptr);
public:
  static void write_barrier(Rps_CallFrameZone*callingfra, Rps_ZoneValue*zva)
  {
    maybe_garbcoll(callingfra);
    if (!zva) return;
    if (_gc_thralloc_)
      {
        /// add the pointer at end of birthblock
        Rps_BirthMemoryBlock*birthblock = _gc_thralloc_->_tha_birthblock;
        if (birthblock && birthblock->remaining_bytes() > sizeof(zva))
          {
          }
        else run_write_barrier(callingfra, zva);
      }
#warning Rps_GarbageCollector::write_barrier unimplemented
    RPS_FATAL(" Rps_GarbageCollector::write_barrier unimplemented callingfra@%p zva@%p",
              (void*)callingfra, (void*)zva);
  }
  ////////////////////////////////////////////////////////////////
  /// various allocation primitives.
  static void*allocate_marked_maybe_gc(size_t size, Rps_CallFrameZone*callingfra);
  static void*allocate_birth_maybe_gc(size_t size, Rps_CallFrameZone*callingfra)
  {
    void* ad = nullptr;
    assert (size < RPS_LARGE_BLOCK_SIZE - Rps_LargeNewMemoryBlock::_remain_threshold_ - 4*sizeof(void*));
    assert (size % (2*alignof(Rps_Value)) == 0);
    maybe_garbcoll(callingfra);
    if (size < RPS_SMALL_BLOCK_SIZE - Rps_BirthMemoryBlock::_remain_threshold_ - 4*sizeof(void*))
      {
        if (_gc_thralloc_)
          {
            Rps_BirthMemoryBlock*birthblock = _gc_thralloc_->_tha_birthblock;
            while (!ad)
              {
                if (birthblock && birthblock->remaining_bytes() > size)
                  {
                    ad = birthblock->allocate_zone(size);
                  }
                else
                  {
                    _gc_wanted.store(true);
                    Rps_GarbageCollector::run_garbcoll(callingfra);
                    // on the next loop, allocation should succeed
                    continue;
                  };
              };
          }
        else   // no _gc_thralloc_
          {
            void* ad = nullptr;
            while (!ad)
              {
                {
                  std::lock_guard<std::mutex> _gu(_gc_globalloc_._gla_mutex);
                  Rps_BirthMemoryBlock*globirthblock = _gc_globalloc_._gla_birthblock;
                  if (globirthblock && globirthblock->remaining_bytes() > size)
                    {
                      ad = globirthblock->allocate_zone(size);
                      break;
                    }
                  else
                    {
                      _gc_wanted.store(true);
                    }
                }
                if (!ad)
                  Rps_GarbageCollector::maybe_garbcoll (callingfra);
              }
          }
      } // end small size
    else if (size < RPS_LARGE_BLOCK_SIZE  - Rps_LargeNewMemoryBlock::_remain_threshold_ - 4*sizeof(void*))
      {
        void* ad = nullptr;
        while (!ad)
          {
            {
              std::lock_guard<std::mutex> _gu(_gc_globalloc_._gla_mutex);
              Rps_LargeNewMemoryBlock*glolargenewblock = _gc_globalloc_._gla_largenewblock;
              if (glolargenewblock && glolargenewblock->remaining_bytes() > size)
                {
                  ad = glolargenewblock->allocate_zone(size);
                  break;
                }
              else
                {
                  _gc_wanted.store(true);
                }
            }
            if (!ad)
              Rps_GarbageCollector::maybe_garbcoll (callingfra);
          };
      } // end large size
    else RPS_FATAL("too big size %zd for allocate_birth_maybe_gc", size);
  };
};				// end class Rps_GarbageCollector


////////////////////////////////////////////////////////////////

void
Rps_ZoneValue::rps_write_barrier(Rps_CallFrameZone*callingfra)
{
  Rps_GarbageCollector::write_barrier(callingfra, this);
} // end of Rps_ZoneValue::rps_write_barrier

void*
Rps_PointerCopyingZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callingfra)
{
  return Rps_GarbageCollector::allocate_birth_maybe_gc(totalsize, callingfra);
}      // end of Rps_PointerCopyingZoneValue::allocate_rps_zone

void*
Rps_MutableCopyingZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callingfra)
{
  return Rps_GarbageCollector::allocate_birth_maybe_gc(totalsize, callingfra);
}      // end of Rps_MutableCopyingZoneValue::allocate_rps_zone

void*
Rps_ScalarCopyingZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callingfra)
{
  return Rps_GarbageCollector::allocate_birth_maybe_gc(totalsize, callingfra);
}      // end of Rps_ScalarCopyingZoneValue::allocate_rps_zone

void*
Rps_MarkSweepZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callingfra)
{
  return Rps_GarbageCollector::allocate_marked_maybe_gc(totalsize, callingfra);
}      // end of Rps_MarkSweepZoneValue::allocate_rps_zone

////////////////////////////////////////////////////////////////
class Rps_MutatorThread: public std::thread
{
  friend class Rps_GarbageCollector;
public:
  Rps_MutatorThread();
  ~Rps_MutatorThread();
  void disable_garbage_collector(void);
  void enable_garbage_collector(void);
  void do_without_garbage_collector(const std::function<void(void)>&fun)
  {
    disable_garbage_collector();
    fun();
    enable_garbage_collector();
  };
#warning Rps_MutatorThread incomplete
#if 0 /// wrong code below. Don't compile yet!
  /// see https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
  template <typename ResType, typename ...Args>
  ResType do_without_garbage_collector(const std::function<ResType(Args args...)>& fun, Args... args...)
  {
    disable_garbage_collector();
    ResType res = std::invoke(fun, args...);
    enable_garbage_collector();
    return res;
  };
  template <class... Args>
  void do_without_garbage_collector(const std::function<void(Args... args)>&fun)
  {
    disable_garbage_collector();
    fun(args...);
    enable_garbage_collector();
  };
#endif //wrong code
};				// end class Rps_MutatorThread

////////////////////////////////////////////////////////////////
/// lexical tokens are quasivalues, garbage collected, but not "first
/// class citizen" values. They only exist during load time.
class Rps_QuasiToken : public  Rps_PointerCopyingZoneValue
{
  friend class Rps_PointerCopyingZoneValue;
  friend class Rps_ZoneValue;
public:
  enum TYPE_en
  {
    RPS_TOKEN_TYPE_NONE,
    RPS_TOKEN_TYPE_OBJECTID,
    RPS_TOKEN_TYPE_NAME,
    RPS_TOKEN_TYPE_STRING,
    RPS_TOKEN_TYPE_DOUBLE,
    RPS_TOKEN_TYPE_INTEGER,
    RPS_TOKEN_TYPE_LEFT_PARENTHESIS,
    RPS_TOKEN_TYPE_RIGHT_PARENTHESIS
  };



  struct LOCATION_st
  {
    const std::string* loc_filepathptr;	// pointer to some file path string
    ssize_t loc_lineno;
  };
  static constexpr LOCATION_st empty_location {nullptr,0};

private:

  Rps_QuasiToken::TYPE_en _tk_type;
  union
  {
    void* _tk_unusedptr;
    intptr_t _tk_integer;
    double _tk_double;
    Rps_StringValue _tk_string;
    ///TODO: maybe define some Rps_NameValue and use it below
    Rps_StringValue _tk_name;
    Rps_Id _tk_objectid;
  };
  Rps_QuasiToken::LOCATION_st _tk_location;
  struct tag_string {};
  struct tag_name {};
  struct tag_left_parenthesis {};
  struct tag_right_parenthesis {};
  static void* operator new (std::size_t, void*ptr)
  {
    return ptr;
  };
  Rps_QuasiToken()
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_NONE),
      _tk_unusedptr(nullptr)
  {
  };
  friend Rps_QuasiToken*Rps_ZoneValue::rps_allocate<Rps_QuasiToken>
  (Rps_CallFrameZone*,intptr_t,const LOCATION_st*);
  Rps_QuasiToken(intptr_t i,
                 const LOCATION_st* ploc=nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_INTEGER),
      _tk_integer(i),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  friend Rps_QuasiToken*Rps_ZoneValue::rps_allocate<Rps_QuasiToken>
  (Rps_CallFrameZone*,double,const LOCATION_st*);
  Rps_QuasiToken(double d, const LOCATION_st* ploc=nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_DOUBLE),
      _tk_double(d),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  friend Rps_QuasiToken*Rps_ZoneValue::rps_allocate<Rps_QuasiToken>
  (Rps_CallFrameZone*,tag_string, Rps_StringValue,const LOCATION_st*);
  Rps_QuasiToken(tag_string, Rps_StringValue strv, const LOCATION_st* ploc=nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_STRING),
      _tk_string(strv),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  friend Rps_QuasiToken*Rps_ZoneValue::rps_allocate<Rps_QuasiToken>
  (Rps_CallFrameZone*,tag_name, Rps_StringValue,const LOCATION_st*);
  Rps_QuasiToken(tag_name, Rps_StringValue strv, const LOCATION_st* ploc=nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_NAME),
      _tk_name(strv),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  Rps_QuasiToken(Rps_StringValue strv, const LOCATION_st* ploc=nullptr)
    : Rps_QuasiToken(tag_string{}, strv, ploc) {};

  Rps_QuasiToken(tag_left_parenthesis, const LOCATION_st* ploc = nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_LEFT_PARENTHESIS),
      _tk_unusedptr(nullptr),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  Rps_QuasiToken(tag_right_parenthesis, const LOCATION_st* ploc = nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_RIGHT_PARENTHESIS),
      _tk_unusedptr(nullptr),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  friend Rps_QuasiToken*Rps_ZoneValue::rps_allocate<Rps_QuasiToken>
  (Rps_CallFrameZone*,const Rps_Id&,const LOCATION_st*);
  Rps_QuasiToken(const Rps_Id&oid, const LOCATION_st* ploc=nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_OBJECTID),
      _tk_objectid(oid),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  ~Rps_QuasiToken() {};		// no-op, since garbage collected

public:

  /// Generates an integer quasi-token
  static Rps_QuasiToken*
  make_from_int(Rps_CallFrameZone* callingfra,
                intptr_t i,
                const LOCATION_st* ploc = nullptr)
  {
    return
      Rps_QuasiToken::rps_allocate<Rps_QuasiToken>(callingfra,
          i, ploc);
  };

  /// Overloaded method to generate integer quasi-token
  static Rps_QuasiToken*
  make_from_int(Rps_CallFrameZone* callingfra,
                intptr_t i,
                const LOCATION_st& loc)
  {
    return make_from_int(callingfra, i, &loc);
  };

  /// Generates a double quasi-token
  static Rps_QuasiToken*
  make_from_double(Rps_CallFrameZone* callingfra,
                   double d,
                   const LOCATION_st* ploc = nullptr)
  {
    return
      Rps_QuasiToken::rps_allocate<Rps_QuasiToken>(callingfra,
          d, ploc);
  };

  /// Overloaded method to generate a double quasi-token
  static Rps_QuasiToken*
  make_from_double(Rps_CallFrameZone* callingfra,
                   double d,
                   const LOCATION_st& loc)
  {
    return make_from_double(callingfra, d, &loc);
  };

  /// Generates a string quasi-token
  static Rps_QuasiToken*
  make_from_string(Rps_CallFrameZone* callingfra,
                   const std::string& s,
                   const LOCATION_st* ploc = nullptr);

  /// Overloaded method to generate a string quasi-token
  static Rps_QuasiToken*
  make_from_string(Rps_CallFrameZone* callingfra,
                   const std::string& s,
                   const LOCATION_st& loc)
  {
    return make_from_string(callingfra, s, &loc);
  };

  /// Generates an object ID quasi-token
  static Rps_QuasiToken*
  make_from_objid(Rps_CallFrameZone* callingfra,
                  const Rps_Id& id,
                  const LOCATION_st* ploc = nullptr)
  {
    return
      Rps_QuasiToken::rps_allocate<Rps_QuasiToken>(callingfra,
          id, ploc);
  };

  /// Overloaded method to generate an object ID quasi-token
  static Rps_QuasiToken*
  make_from_objid(Rps_CallFrameZone* callingfra,
                  const Rps_Id& id,
                  const LOCATION_st& loc)
  {
    return make_from_objid(callingfra, id, &loc);
  }
};				// end class Rps_QuasiToken



////////////////////////////////////////////////////////////////
class Rps_LexedFile
{
  std::string _lfil_path;
  // for getline(3)
  char* _lfil_linbuf; // malloc-ed by getline
  size_t _lfil_linsiz; // the allocated size of _lfil_linbuf, for getline
  ssize_t _lfil_linlen; // the actual line length
  int _lfil_lineno;	// current line number, starting from one
  //
  char* _lfil_iter;
  FILE* _lfil_hnd;
  size_t _lfil_sz;
public:
  Rps_LexedFile(const std::string& file_path);
  ~Rps_LexedFile();

  void reset_line (void)
  {
    assert (_lfil_linbuf != nullptr);
    _lfil_iter = _lfil_linbuf;
  };

  // gives false when the end of current line is reached.
  bool next(void)
  {
    assert (_lfil_linbuf != nullptr);
    assert (_lfil_iter != nullptr);
    if (*(_lfil_iter + 1))
      {
        _lfil_iter++;
        return true;
      }

    return false;
  };

  // get a new line, and gives false when end of file is reached
  bool read_line(void)
  {
    assert (_lfil_hnd != nullptr);

    _lfil_linlen = getline (&_lfil_linbuf, &_lfil_linsiz, _lfil_hnd);
    if (_lfil_linlen < 0)
      return false;

    _lfil_lineno++;
    _lfil_iter = _lfil_linbuf;

    return true;
  };

  /// Generates the token for the current line
  Rps_QuasiToken* tokenize(Rps_CallFrameZone* callframe);
};				// end class Rps_LexedFile


#endif /*RPS_ONLY_ID_CODE*/
#endif /*REFPERSYS_INCLUDED*/
// end of file refpersys.hh */

/****************************************************************
 * file refpersys.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its only C++ header file.
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
#include <atomic>
#include <stdexcept>

#include <cassert>
#include <cstring>
#include <cmath>

#include <argp.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "unistr.h"

#ifdef RPS_HAVE_MPS
#error Ravenbrook MPS is probably obsolete for RefPerSys
extern "C" {
#include "mps.h"
#include "mpsavm.h"
};				// end extern "C" for mps.h
static_assert(sizeof(mps_word_t) == sizeof(void*));
static_assert(alignof(mps_word_t) == alignof(void*));
#endif /*RPS_HAVE_MPS*/

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

extern void print_types_info(void);

class Rps_CallFrameZone;
class Rps_GarbageCollector;

#define RPS_FLEXIBLE_DIM 0	/* for flexible array members */

// size of blocks, in bytes
#define RPS_SMALL_BLOCK_SIZE (8<<20)
#define RPS_LARGE_BLOCK_SIZE (8*RPS_SMALL_BLOCK_SIZE)

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
protected:
  virtual ~Rps_MemoryBlock() = 0;
  void* operator new(size_t size);
  void operator delete(void*);
  intptr_t _bl_meta[_bl_metasize_];
  intptr_t _bl_data[RPS_FLEXIBLE_DIM];
  Rps_MemoryBlock(unsigned kindnum, Rps_BlockIndex ix, size_t size);
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
public:
  unsigned remaining_bytes() const
  {
    assert ((char*)_bl_curptr <= (char*)_bl_endptr);
    return (char*)_bl_endptr - (char*)_bl_curptr;
  };
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
  Rps_BirthMemoryBlock(Rps_BlockIndex ix) :
    Rps_MemoryBlock(blk_birth, ix, RPS_SMALL_BLOCK_SIZE - sizeof(Rps_BirthMemoryBlock)) {};
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
  Rps_LargeNewMemoryBlock(Rps_BlockIndex ix) :
    Rps_MemoryBlock(blk_largenew, ix, RPS_LARGE_BLOCK_SIZE - sizeof(Rps_LargeNewMemoryBlock)) {};
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
  struct markedmetadata_st
  {
  };
  struct markedmetadata_st& metadata()
  {
    return raw_metadata<markedmetadata_st>();
  };
  Rps_MarkedMemoryBlock(Rps_BlockIndex ix) :
    Rps_MemoryBlock(blk_marked, ix, RPS_SMALL_BLOCK_SIZE - sizeof(Rps_MarkedMemoryBlock)) {};
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
  Rps_ObjectZone*_optr;
public:
  Rps_ObjectZone* optr() const
  {
    return _optr;
  };
  const Rps_ObjectZone* const_optr() const
  {
    return const_cast<const Rps_ObjectZone*>(_optr);
  };
  // rule of five
  Rps_ObjectRef(Rps_ObjectZone*oz = nullptr) : _optr(oz) {};
  ~Rps_ObjectRef()
  {
    _optr = nullptr;
  };
  Rps_ObjectRef(const Rps_ObjectRef&oth)
  {
    _optr = oth._optr;
  };
  Rps_ObjectRef(Rps_ObjectRef&&oth) : _optr(std::exchange(oth._optr, nullptr)) {};
  Rps_ObjectRef& operator = (const Rps_ObjectRef& oth)
  {
    _optr = oth._optr;
    return *this;
  }
  Rps_ObjectRef& operator = (Rps_ObjectRef&& oth)
  {
    std::swap(_optr, oth._optr);
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
  operator Rps_ObjectZone* () const
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
    _optr = zob;
  };
  inline bool operator == (const Rps_ObjectRef& oth) const;
  inline bool operator != (const Rps_ObjectRef& oth) const;
  inline bool operator <= (const Rps_ObjectRef& oth) const;
  inline bool operator < (const Rps_ObjectRef& oth) const;
  inline bool operator > (const Rps_ObjectRef& oth) const;
  inline bool operator >= (const Rps_ObjectRef& oth) const;
};				// end class Rps_ObjectRef


////////////////////////////////////////////////////////////////
typedef uint32_t Rps_HashInt;
enum Rps_Type
{
  /// non-value types (or quasi-values)
#ifdef RPS_HAVE_MPS
  Rps_TyForwardedMPS = -50,
  Rps_TyPaddingMPS = -51,
#endif /*RPS_HAVE_MPS*/
  ///
  /// these are GC-scanned, but not GC-allocated:
  Rps_TyDumper = -32,
  Rps_TyLoader = -31,
  Rps_TyCallFrame = -30,
  Rps_TyPayload = -5,
  ///
  /// Quasi-values are indeed garbage collected, so GC-scanned and
  /// GC-allocated, but not "first class citizens" as genuine values
  /// are... So they don't directly go inside RpsValue-s.
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



////////////////////////////////////////////////////////////////
class Rps_Id
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
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
  Rps_Id () : Rps_Id(0,0) {};
  Rps_Id (std::nullptr_t) : Rps_Id(random()) {};
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
    const Rps_ZoneValue* _datav;
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
};				// end of Rps_Value



////////////////////////////////////////////////////////////////
/// A garbage collected memory zone inherits from Rps_ZoneValue.

#ifdef RPS_HAVE_MPS
/// Depending on its MPS allocation pool class, it actually will
/// inherit from various subclasses of Rps_ZoneValue.
#endif /*RPS_HAVE_MPS*/


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
#ifdef RPS_HAVE_MPS
  template <typename ValClass, class ...Args>
  static ValClass* mps_allocate_with_gap(unsigned gap, Args... args)
  {
    ValClass* result = nullptr;
    size_t siz = sizeof(ValClass);
    assert (siz % alignof(ValClass) == 0);
    assert (gap % alignof(ValClass) == 0);
    assert (ValClass::mps_allocpoint_ != nullptr);
    mps_addr_t ptr = nullptr;
    siz += gap;
    do
      {
        mps_res_t res = mps_reserve(&ptr, ValClass::mps_allocpoint_, siz);
        if (res != MPS_RES_OK)
          RPS_FATAL("allocation failure (#%d) for %zd - %m\n",
                    res, siz);
        result = new (ptr) ValClass(args...);
      }
    while (!mps_commit(ValClass::mps_allocpoint_, ptr, siz));
    return result;
  }
#endif /*RPS_HAVE_MPS*/
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
#ifdef RPS_HAVE_MPS
  template <typename ValClass, class ...Args>
  static ValClass* allocate_mps(Args... args)
  {
    mps_addr_t ptr = nullptr;
    assert (ValClass::mps_allocpoint_ != nullptr);
    do
      {
        mps_res_t res = mps_reserve(&ptr, ValClass::mps_allocpoint_, siz);
        if (res != MPS_RES_OK)
          RPS_FATAL("allocation failure (#%d) for %zd - %m\n",
                    res, siz);
        result = ::operator new (ptr) ValClass(args...);
      }
    while (!mps_commit(ValClass::allocpoint_, ptr, siz));
  }
#endif /*RPS_HAVE_MPS*/

  Rps_ZoneValue(Rps_Type ty) : _vtyp(ty)
  {
    assert ((int)ty>=0);
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
protected:
  void mutate_type(Rps_Type ty)
  {
    assert ((int)ty >= 0);
    _vtyp = ty;
  };
#ifdef RPS_HAVE_MPS
  static mps_arena_t _mps_valzone_arena;
  static mps_fmt_t _mps_valzone_fmt;
  static mps_res_t mps_valzone_scan(mps_ss_t ss, mps_addr_t base, mps_addr_t limit);
  static mps_addr_t mps_valzone_skip(mps_addr_t base);
  static void mps_valzone_fwd(mps_addr_t oldad, mps_addr_t newad);
  static mps_addr_t mps_valzone_isfwd(mps_addr_t addr);
  static void mps_valzone_pad(mps_addr_t addr, size_t size);
#endif /*RPS_HAVE_MPS*/
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

#ifdef RPS_HAVE_MPS
////////////////////////////////////////////////////////////////
class Rps_ForwardedMPSZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
  void* _fwdptr;
  size_t _fwdsiz;
public:
  Rps_ForwardedMPSZoneValue(void*fptr = nullptr, size_t siz = 0) : Rps_ZoneValue(Rps_TyForwardedMPS), _fwdptr(fptr), _fwdsiz(siz) {};
  void* forwardptr(void) const
  {
    return _fwdptr;
  };
  size_t forwardsize(void) const
  {
    return _fwdsiz;
  };
  void set_forwardptr(void*p = nullptr, size_t siz = 0)
  {
    _fwdptr = p;
    _fwdsiz = siz;
  };
};




class Rps_PaddingMPSZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
  size_t _padsize;
public:
  Rps_PaddingMPSZoneValue(size_t siz)
    : Rps_ZoneValue(Rps_TyPaddingMPS), _padsize(siz) {};
  size_t padsize(void) const
  {
    return _padsize;
  };
  void* next_byte(void) const
  {
    return (void*)((reinterpret_cast<const char*>(this))+_padsize);
  }
};				// end class Rps_PaddingMPSZoneValue
#endif /*RPS_HAVE_MPS*/
////////////////////////////////////////////////////////////////



/***
 *  An instance of Rps_PointerCopyingZoneValue is some
 *  garbage-collected memory zone which is copied or moved by the GC
 *  and which contains internal pointers that should be scanned and/or
 *  forwarded.  It might perhaps also contain mutable non-GC-pointer
 *  data.
 ***/
#ifdef RPS_HAVE_MPS
//// for AMC allocation pool class of MPS
#endif /*RPS_HAVE_MPS*/
class Rps_PointerCopyingZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
protected:
#ifdef RPS_HAVE_MPS
  static thread_local mps_ap_t mps_allocpoint_;
#endif /*RPS_HAVE_MPS*/
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
  Rps_PointerCopyingZoneValue(Rps_Type ty) : Rps_ZoneValue(ty) {};
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
  Rps_MutableCopyingZoneValue(Rps_Type ty)
    : Rps_PointerCopyingZoneValue(ty) {};
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
public:
};				// end class Rps_MutableCopyingZoneValue


////////////////////////////////////////////////////////////////
/***
 * An instance of Rps_ScalarCopyingZoneValue is some garbage-collected
 * memory zone which is copied or moved by the GC and does not contain
 * any internal pointers to GC-ed zones.
 ***/
#ifdef RPS_HAVE_MPS
//// for AMCZ allocation pool class of MPS
#endif /*RPS_HAVE_MPS*/
class Rps_ScalarCopyingZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
protected:
#ifdef RPS_HAVE_MPS
  static thread_local mps_ap_t mps_allocpoint_;
#endif /*RPS_HAVE_MPS*/
  Rps_ScalarCopyingZoneValue(Rps_Type ty) : Rps_ZoneValue(ty) {};
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
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
#ifdef RPS_HAVE_MPS
//// for AMS allocation pool class of MPS
#endif /*RPS_HAVE_MPS*/
class Rps_MarkSweepZoneValue : public Rps_ZoneValue
{
  friend class Rps_ZoneValue;
  friend class Rps_ObjectZone;
protected:
  virtual ~Rps_MarkSweepZoneValue() {};
  Rps_MarkSweepZoneValue(Rps_Type ty) : Rps_ZoneValue(ty) {};
#ifdef RPS_HAVE_MPS
  virtual const void* mps_scan(mps_ss_t ss) =0;
  virtual const void* mps_skip(void) const =0;
  virtual size_t mps_size(void) const =0;
  static thread_local mps_ap_t mps_allocpoint_;
#endif /*RPS_HAVE_MPS*/
protected:
  static inline void* allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram);
  static void* operator new  (std::size_t siz, zone_tag, Rps_CallFrameZone*callfram)
  {
    return allocate_rps_zone(siz,callfram);
  }
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
  void example_gc_func(Rps_CallFrameZone*callfram);
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
  void example_gc_func(Rps_CallFrameZone*callfram);
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




#ifdef RPS_HAVE_MPS
static_assert(sizeof(Rps_SequenceObrefZone) >= sizeof(Rps_ForwardedMPSZoneValue));
static_assert(alignof(Rps_SequenceObrefZone) >= alignof(Rps_ForwardedMPSZoneValue));

static_assert(sizeof(Rps_SequenceObrefZone) >= sizeof(Rps_PaddingMPSZoneValue));
static_assert(alignof(Rps_SequenceObrefZone) >= alignof(Rps_PaddingMPSZoneValue));
#endif /*RPS_HAVE_MPS*/



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
  static Rps_TupleObrefZone* make(Rps_CallFrameZone*callfram,const std::initializer_list<const Rps_ObjectRef> il)
  {
    return make(callfram,il.size(), il.begin());
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
  Rps_TupleValue(Rps_CallFrameZone*callfram,uint32_t siz, Rps_ObjectRef const* arr)
    : Rps_Value(Rps_TupleObrefZone::make(callfram,siz, arr)) {};
  Rps_TupleValue(Rps_CallFrameZone*callfram,const std::initializer_list<const Rps_ObjectRef> il)
    : Rps_Value(Rps_TupleObrefZone::make(callfram,il)) {};
  struct collect_tag {};
  // make a tuple from a collection of values, using only objects and
  // sequences from them and ignoring other values
  Rps_TupleValue(Rps_CallFrameZone*callfram,collect_tag, const std::initializer_list<const Rps_Value> il);
  Rps_TupleValue(Rps_CallFrameZone*callfram,collect_tag, uint32_t siz, const Rps_Value*arr);
  static Rps_TupleValue collect(Rps_CallFrameZone*callfram,const std::initializer_list<const Rps_Value> il)
  {
    return Rps_TupleValue(callfram,collect_tag{}, il);
  };
  static Rps_TupleValue collect(Rps_CallFrameZone*callfram,uint32_t siz, const Rps_Value*arr)
  {
    return Rps_TupleValue(callfram,collect_tag{}, siz, arr);
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
  static Rps_SetObrefZone* make(Rps_CallFrameZone*callfram,uint32_t siz, const Rps_ObjectRef*arr);
  static Rps_SetObrefZone* make(Rps_CallFrameZone*callfram,const std::initializer_list<const Rps_ObjectRef> il)
  {
    return make(callfram,il.size(), il.begin());
  };
}; // end of Rps_SetObrefZone

Rps_Value::Rps_Value(set_tag, const Rps_SetObrefZone*pset) :
  Rps_Value(pset) {};
class Rps_SetValue : public Rps_Value
{
public:
  Rps_SetValue(Rps_Value val) : Rps_Value(val.as_set()) {};
  Rps_SetValue(Rps_CallFrameZone*callfram,uint32_t siz, Rps_ObjectRef const* arr)
    : Rps_Value(Rps_SetObrefZone::make(callfram,siz, arr)) {};
  Rps_SetValue(Rps_CallFrameZone*callfram,const std::initializer_list<const Rps_ObjectRef> il)
    : Rps_Value(Rps_SetObrefZone::make(callfram,il)) {};
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


#ifdef RPS_HAVE_MPS
static_assert(sizeof(Rps_DoubleZone) >= sizeof(Rps_ForwardedMPSZoneValue));
static_assert(alignof(Rps_DoubleZone) >= alignof(Rps_ForwardedMPSZoneValue));
static_assert(sizeof(Rps_DoubleZone) >= sizeof(Rps_PaddingMPSZoneValue));
static_assert(alignof(Rps_DoubleZone) >= alignof(Rps_PaddingMPSZoneValue));
#endif /*RPS_HAVE_MPS*/


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
  static Rps_StringZone* make(Rps_CallFrameZone*callfram,const char*sbytes, int32_t slen= -1);
  static constexpr Rps_Type zone_type = Rps_TyString;
  uint32_t size() const
  {
    return _strlen;
  };
  /// use very carefully, since Rps_StringZone-s are moved by MPS at any
  /// time.
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


#ifdef RPS_HAVE_MPS
static_assert(sizeof(Rps_StringZone) >= sizeof(Rps_ForwardedMPSZoneValue));
static_assert(alignof(Rps_StringZone) >= alignof(Rps_ForwardedMPSZoneValue));
static_assert(sizeof(Rps_StringZone) >= sizeof(Rps_PaddingMPSZoneValue));
static_assert(alignof(Rps_StringZone) >= alignof(Rps_PaddingMPSZoneValue));
#endif /*RPS_HAVE_MPS*/

class Rps_StringValue : public Rps_Value
{
  Rps_StringValue(nullptr_t) : Rps_Value(nullptr) {};
public:
  void put_string(Rps_CallFrameZone*callfram,const char*cstr, int slen= -1)
  {
    if (!cstr) slen=0;
    else put_data(Rps_StringZone::make(callfram,cstr,slen));
  };
  void put_string(Rps_CallFrameZone*callfram,const std::string&str)
  {
    put_string(callfram,str.c_str(), str.size());
  };
  Rps_StringValue() : Rps_StringValue(nullptr) {};
  Rps_StringValue(Rps_CallFrameZone*callfram,const char*cstr, int slen= -1)
    : Rps_StringValue(nullptr)
  {
    put_string(callfram,cstr, slen);
  };
  Rps_StringValue(Rps_CallFrameZone*callfram,const std::string&str)
    : Rps_StringValue(nullptr)
  {
    put_string(callfram,str);
  };
  // return a *copy* of the string, since the contained Rps_StringZone
  // is potentially *moved* by MPS at arbitrary times.
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
  /// use very carefully, since Rps_StringZone-s are moved by MPS at any
  /// time.
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
#ifdef RPS_HAVE_MPS
  virtual void* mps_scan(mps_ss_t ss) =0;
#endif /*RPS_HAVE_MPS*/
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
#ifdef RPS_HAVE_MPS
  virtual void* mps_scan(mps_ss_t ss);
#endif /*RPS_HAVE_MPS*/
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
  friend class Rps_MarkSweepZoneValue;
  friend void print_types_info(void);
  const Rps_Id _ob_id;
  // we need some lock; we could later improve it, see perhaps
  // https://www.arangodb.com/2018/05/an-implementation-of-phase-fair-reader-writer-locks/
  std::mutex _ob_mtxlock;
  /// the class of an object is so frequently used that it is an
  /// atomic pointer that does not need to be fetched under the mutex.
  std::atomic<Rps_ObjectZone*> _ob_atomclass;
  // TODO: we need not only the lock but also some kind of object buckets
  Rps_ObjectZone(const Rps_Id& oid, Rps_ObjectRef obclass= nullptr)
    : Rps_MarkSweepZoneValue(Rps_TyObject), _ob_id(oid), _ob_mtxlock(), _ob_atomclass(obclass),
      _obat_kind(atk_none), _obat_null(), _ob_compvec(nullptr)
  {
  };
  virtual ~Rps_ObjectZone(); /// related to mps_finalize
#ifdef RPS_HAVE_MPS
  virtual size_t mps_size(void) const
  {
    return sizeof(Rps_ObjectZone);
  };
#endif /*RPS_HAVE_MPS*/
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
#ifdef RPS_HAVE_MPS
  /// scan the object, return next address
  const void* mps_really_scan(mps_ss_t ss);
  const void* mps_really_skip(void) const
  {
    return this+1;
  };
  virtual const void* mps_scan(mps_ss_t ss)
  {
    return mps_really_scan(ss);
  };
  virtual const void* mps_skip(void) const
  {
    return mps_really_skip();
  };
#endif /*RPS_HAVE_MPS*/
  /*** Objects are quite common, and their attributes and
       components also.  Most objects have few attributes, we should
       special-case for that with efficiency in mind. For few
       attributes, the best is probably a small array of
       (attribute-key-object, attribute-value) pairs which is seeked
       linearly, but is cache friendly. For the unusual case of a
       hundred or much more of attributes, we could have a std::map or
       a std::unordered_map. In the more common case of at most a few
       dozens of attributes, putting then in some sorted array
       accessed dichotomically is a good solution. So attributes are
       represented as a tagged union, and the usual case is a pointer
       to Rps_QuasiAttributeArray. Likewise, most objects have not
       many components, and these components are in some small dynamic
       array.

       The pathological case of an object with many thousands of
       components or thousands of attributes should be handled, but
       could be coded later, or at least could be optimized later.
   ***/
private:
  ///// attributes of objects
  enum attrkind_en {atk_none, atk_small, atk_medium, atk_big};
  attrkind_en _obat_kind;
  static constexpr unsigned at_small_thresh= 10;
  static constexpr unsigned at_sorted_thresh= 101; // a prime
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
  ///// components of objects
  Rps_QuasiComponentVector* _ob_compvec;
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
  Rps_Value get_attr(Rps_ObjectRef keyob) const
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
        while (lo+4 < hi)
          {
            unsigned md = (lo+hi)/2;
            Rps_ObjectRef midob = _obat_sorted_atar->unsafe_attr_at(md);
            if (keyob == midob) return _obat_sorted_atar->unsafe_val_at(md);
            else if (midob > keyob)
              hi = md;
            else
              lo = md;
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
  };				// end get_attr
  void do_put_attr(Rps_ObjectRef keyob, Rps_Value valat);
  Rps_ObjectRef put_attr(Rps_ObjectRef keyob, Rps_Value valat)
  {
    do_put_attr(keyob, valat);
    return this;
  };
  void do_remove_attr(Rps_ObjectRef keyob);
  Rps_ObjectRef remove_attr(Rps_ObjectRef keyob)
  {
    do_remove_attr(keyob);
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
  /// resize the component vector to the given size, so grow or shrink them
  void do_resize_components (unsigned size);
  Rps_ObjectRef resize_components(unsigned size)
  {
    do_resize_components(size);
    return this;
  };
  /// reserve the component vector to the given size, with an empty count
  void do_reset_components (unsigned size);
  Rps_ObjectRef reset_components(unsigned size)
  {
    do_reset_components(size);
    return this;
  }
  /// append a component
  void do_append_component(Rps_Value val);
  Rps_ObjectRef append_component(Rps_Value val)
  {
    do_append_component(val);
    return this;
  }
};				// end class Rps_ObjectZone

#ifdef RPS_HAVE_MPS
static_assert(sizeof(Rps_ObjectZone) >= sizeof(Rps_ForwardedMPSZoneValue));
static_assert(alignof(Rps_ObjectZone) >= alignof(Rps_ForwardedMPSZoneValue));
static_assert(sizeof(Rps_ObjectZone) >= sizeof(Rps_PaddingMPSZoneValue));
static_assert(alignof(Rps_ObjectZone) >= alignof(Rps_PaddingMPSZoneValue));
#endif /*RPS_HAVE_MPS*/


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
/**
 * The garbage collector
 *
 * There is no actual instance of Rps_GarbageCollector, but that class is
 * grouping static data and member functions related to garbage
 * collection and allocation support.  Perhaps it should be a C++
 * namespace, not a C++ class.
 */
class Rps_GarbageCollector
{
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
  static void scan_call_stack(Rps_CallFrameZone*callfram);
  // Forcibly run the garbage collector:
  static void run_garbcoll(Rps_CallFrameZone*callfram);
  // Maybe run the garbage collector. Each allocating thread is
  // promising to call that at least once every few milliseconds,
  // unless special precautions are taken (disabling allocation and GC
  // in some non-worker thread doing a blocking operation like poll(2)
  // or read(2)...). Calling it more often makes almost no harm
  // (except a tiny performance loss). Calling it not often enough is
  // a severe condition and should be avoided, since it will impact
  // usability, latency and could even crash the system.
  static void maybe_garbcoll(Rps_CallFrameZone*callfram)
  {
    if (_gc_wanted.load())
      run_garbcoll(callfram);
  };
  // Call this quick function to give the intention of having the GC
  // being soon called, hopefully in the next few milliseconds.
  static void want_to_garbcoll(void)
  {
    _gc_wanted.store(true);
  }
  ////////////////////////////////////////////////////////////////
  /// various allocation primitives.
  static void*allocate_marked_maybe_gc(size_t size, Rps_CallFrameZone*callfram)
  {
    void* ad = nullptr;
    assert (size < RPS_SMALL_BLOCK_SIZE - Rps_MarkedMemoryBlock::_remain_threshold_ - 4*sizeof(void*));
    maybe_garbcoll(callfram);
#warning Rps_GarbageCollector::allocated_marked_maybe_gc unimplemented
    RPS_FATAL("Rps_GarbageCollector::allocated_marked_maybe_gc unimplemented size=%zd", size);
  };
  static void*allocate_birth_maybe_gc(size_t size, Rps_CallFrameZone*callfram)
  {
    void* ad = nullptr;
    assert (size < RPS_LARGE_BLOCK_SIZE - Rps_LargeNewMemoryBlock::_remain_threshold_ - 4*sizeof(void*));
    assert (size % (2*alignof(Rps_Value)) == 0);
    maybe_garbcoll(callfram);
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
                    Rps_GarbageCollector::run_garbcoll(callfram);
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
                  Rps_GarbageCollector::maybe_garbcoll (callfram);
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
              Rps_GarbageCollector::maybe_garbcoll (callfram);
          };
      } // end large size
    else RPS_FATAL("too big size %zd for allocate_birth_maybe_gc", size);
  };
};				// end class Rps_GarbageCollector


////////////////////////////////////////////////////////////////
void*
Rps_PointerCopyingZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram)
{
  return Rps_GarbageCollector::allocate_birth_maybe_gc(totalsize, callfram);
}      // end of Rps_PointerCopyingZoneValue::allocate_rps_zone

void*
Rps_MutableCopyingZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram)
{
  return Rps_GarbageCollector::allocate_birth_maybe_gc(totalsize, callfram);
}      // end of Rps_MutableCopyingZoneValue::allocate_rps_zone

void*
Rps_ScalarCopyingZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram)
{
  return Rps_GarbageCollector::allocate_birth_maybe_gc(totalsize, callfram);
}      // end of Rps_ScalarCopyingZoneValue::allocate_rps_zone

void*
Rps_MarkSweepZoneValue::allocate_rps_zone(std::size_t totalsize, Rps_CallFrameZone*callfram)
{
  return Rps_GarbageCollector::allocate_marked_maybe_gc(totalsize, callfram);
}      // end of Rps_MarkSweepZoneValue::allocate_rps_zone



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
    RPS_TOKEN_TYPE_PARENTHESIS,
  };

#if 0
#warning dead code to be removed after review
  enum DELIMITER_en
  {
    RPS_DELIM_NONE,
    RPS_DELIM_LEFTPAREN,	// for '('
    RPS_DELIM_RIGHTPAREN, 	// for ')'
    //TODO: add much more delimiters
  };
#endif


  struct LOCATION_st
  {
    const std::string* loc_filepathptr;	// pointer to some file path string
    int loc_lineno;
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
    //DELIMITER_en _tk_delim;
    char _tk_paren;
  };
  Rps_QuasiToken::LOCATION_st _tk_location;
  struct tag_string {};
  struct tag_name {};
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
      _tk_type(RPS_TOKEN_TYPE_STRING),
      _tk_name(strv),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  Rps_QuasiToken(Rps_StringValue strv, const LOCATION_st* ploc=nullptr)
    : Rps_QuasiToken(tag_string{}, strv, ploc) {};

  //Rps_QuasiToken(DELIMITER_en delim, const LOCATION_st* ploc=nullptr)
  Rps_QuasiToken(char paren, const LOCATION_st* ploc = nullptr)
    : Rps_PointerCopyingZoneValue(Rps_TyQuasiToken),
      _tk_type(RPS_TOKEN_TYPE_PARENTHESIS),
      //_tk_delim(delim),
      _tk_paren(paren),
      _tk_location(ploc?(*ploc):empty_location)
  {
  };
  ~Rps_QuasiToken() {};		// no-op, since garbage collected

public:
  /// the lexer should use these makers
  static Rps_QuasiToken*make_from_int (intptr_t i, Rps_CallFrameZone* callframe, const LOCATION_st* ploc=nullptr)
  {
    return Rps_QuasiToken::rps_allocate<Rps_QuasiToken>(callframe, i, ploc);
  };
  static Rps_QuasiToken*make_from_int(intptr_t i, Rps_CallFrameZone* callframe, const LOCATION_st& loc)
  {
    return make_from_int(i, callframe, &loc);
  };
  ///
  static Rps_QuasiToken*make_from_double (double d, Rps_CallFrameZone* callframe, const LOCATION_st* ploc=nullptr)
  {
    return Rps_QuasiToken::rps_allocate<Rps_QuasiToken>(callframe, d, ploc);
  };
  static Rps_QuasiToken*make_from_double (double d, Rps_CallFrameZone* callframe, const LOCATION_st& loc)
  {
    return make_from_double(d, callframe, &loc);
  };
  static Rps_QuasiToken*make_from_string (const std::string&str, Rps_CallFrameZone* callframe, const LOCATION_st* ploc=nullptr);
  static Rps_QuasiToken*make_from_string (const std::string&str, Rps_CallFrameZone* callframe, const LOCATION_st& loc)
  {
    return make_from_string(str, callframe, &loc);
  };
};				// end class Rps_QuasiToken

class Rps_Lexer {
        public:
                Rps_Lexer(const char* file);
                void start(void);
                bool next(void);
                Rps_QuasiToken* tokenize(void);

        private:
                // adapted from
                // https://gist.github.com/arrieta/1a309138689e09375b90b3b1aa768e20
                bool is_whitespace(char c)
                {
                        switch (c) {
                                case ' ':
                                case '\t':
                                case '\r':
                                case '\n':
                                return true;
                        default:
                                return false;
                        }
                }

                bool is_integer_digit(char c)
                {
                        return (c >= '0' && c <= '9') || c == '+' || c == '-';
                }

                bool is_double_digit(char c)
                {
                        return is_integer_digit(c) || c == '.';
                }

                bool is_objid_digit(char c)
                {
                        return (c >= '0' && c <= '9')
                                || (c >= 'A' && c <= 'Z')
                                || (c >= 'a' && c <= 'z')
                                || c == '_';
                }

                char* _file;
                char* _dump;
                char* _iter;
};



#endif /*REFPERSYS_INCLUDED*/
// end of file refpersys.hh */

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
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
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
#include <deque>
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
#include <condition_variable>
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
#include <dlfcn.h>


// for programmatic C++ name demangling, see also
// https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/cxxabi.h
#include <cxxabi.h>

/// forward declaration of QString-s from Qt5
class QString;

// GNU libunistring https://www.gnu.org/software/libunistring/
// we use UTF-8 strings
#include "unistr.h"

#include "backtrace.h"


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

#define RPS_FRIEND_CLASS(Suffix) friend class Rps_##Suffix

// generated in _timestamp_rps.c
extern "C" const char rps_timestamp[];
extern "C" unsigned long rps_timelong;
extern "C" const char rps_directory[];
extern "C" const char rps_gitid[];
extern "C" const char rps_lastgittag[];
extern "C" const char rps_lastgitcommit[];
extern "C" const char rps_md5sum[];
extern "C" const char*const rps_files[];

/// backtrace support
extern "C" struct backtrace_state* rps_backtrace_state;

/// the program name
extern "C" const char* rps_progname;


//////////////// fatal error - aborting
extern "C" void rps_fatal_stop_at (const char *, int) __attribute__((noreturn));

#define RPS_FATAL_AT_BIS(Fil,Lin,Fmt,...) do {			\
    fprintf(stderr, "\n\n"		       			\
	    "*** RefPerSys FATAL:%s:%d: <%s>\n " Fmt "\n\n",	\
            Fil, Lin, __PRETTY_FUNCTION__, ##__VA_ARGS__);     	\
    rps_fatal_stop_at (Fil,Lin); } while(0)

#define RPS_FATAL_AT(Fil,Lin,Fmt,...) RPS_FATAL_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)

#define RPS_FATAL(Fmt,...) RPS_FATAL_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

#define RPS_FATALOUT_AT_BIS(Fil,Lin,...) do {	\
    std::clog << "** RefPerSys FATAL! "		\
	      << (Fil) << ":" << Lin << ":: "	\
	      << __VA_ARGS__ << std::endl;	\
    rps_fatal_stop_at (Fil,Lin); } while(0)

#define RPS_FATALOUT_AT(Fil,Lin,...) RPS_FATALOUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be RPS_FATALOUT("x=" << x)
#define RPS_FATALOUT(...) RPS_FATALOUT_AT(__FILE__,__LINE__,##__VA_ARGS__)


//////////////// warning

#define RPS_WARN_AT_BIS(Fil,Lin,Fmt,...) do {			\
    fprintf(stderr, "\n\n"		       			\
	    "*** RefPerSys WARN:%s:%d: <%s>\n " Fmt "\n\n",	\
            Fil, Lin, __PRETTY_FUNCTION__, ##__VA_ARGS__);     	\
    fflush(stderr); } while(0)

#define RPS_WARN_AT(Fil,Lin,Fmt,...) RPS_WARN_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)

// typical usage could be RPS_WARN("something bad x=%d", x)
#define RPS_WARN(Fmt,...) RPS_WARN_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

#define RPS_WARNOUT_AT_BIS(Fil,Lin,...) do {	\
    std::clog << "** RefPerSys WARN! "		\
	      << (Fil) << ":" << Lin << ":: "	\
	      << __VA_ARGS__ << std::endl;	\
    std::clog << std::flush; } while(0)

#define RPS_WARNOUT_AT(Fil,Lin,...) RPS_WARNOUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be RPS_WARNOUT("annoying x=" << x)
#define RPS_WARNOUT(...) RPS_WARNOUT_AT(__FILE__,__LINE__,##__VA_ARGS__)



//////////////// inform

#define RPS_INFORM_AT_BIS(Fil,Lin,Fmt,...) do {			\
    fprintf(stderr, "\n\n"		       			\
	    "*** RefPerSys INFORM:%s:%d: <%s>\n " Fmt "\n\n",	\
            Fil, Lin, __PRETTY_FUNCTION__, ##__VA_ARGS__);     	\
    fflush(stderr); } while(0)

#define RPS_INFORM_AT(Fil,Lin,Fmt,...) RPS_INFORM_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)

// typical usage could be RPS_INFORM("something bad x=%d", x)
#define RPS_INFORM(Fmt,...) RPS_INFORM_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

#define RPS_INFORMOUT_AT_BIS(Fil,Lin,...) do {	\
    std::clog << "** RefPerSys INFORM! "		\
	      << (Fil) << ":" << Lin << ":: "	\
	      << __VA_ARGS__ << std::endl;	\
    std::clog << std::flush; fflush(nullptr); } while(0)

#define RPS_INFORMOUT_AT(Fil,Lin,...) RPS_INFORMOUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be RPS_INFORMOUT("annoying x=" << x)
#define RPS_INFORMOUT(...) RPS_INFORMOUT_AT(__FILE__,__LINE__,##__VA_ARGS__)



//////////////// assert
#ifndef NDEBUG
#define RPS_ASSERT_AT_BIS(Fil,Lin,Func,Cond) do {      	\
    if (RPS_UNLIKELY(!(Cond))) {			\
  fprintf(stderr, "\n\n"				\
	  "*** RefPerSys ASSERT failed:%s\n"		\
	  "%s:%d: <%s>\n\n", #Cond,			\
	  Fil,Lin,Func);				\
  rps_fatal_stop_at(Fil,Lin); }} while(0)

#define RPS_ASSERT_AT(Fil,Lin,Func,Cond) RPS_ASSERT_AT_BIS(Fil,Lin,Func,Cond)
#define RPS_ASSERT(Cond) RPS_ASSERT_AT(__FILE__,__LINE__,__PRETTY_FUNCTION__,(Cond))
#define RPS_ASSERTPRINTF_AT_BIS(Fil,Lin,Func,Cond,Fmt,...) do {	\
    if (RPS_UNLIKELY(!(Cond))) {				\
      fprintf(stderr, "\n\n"					\
	      "*** RefPerSys ASSERTPRINTF failed:%s\n"		\
	      "%s:%d: <%s>\n", #Cond,				\
	      Fil, Lin, Func);					\
      fprintf(stderr, "!*!*! " Fmt "\n\n", ##__VA_ARGS__);	\
      rps_fatal_stop_at(Fil, Lin); }} while(0)

#define RPS_ASSERTPRINTF_AT(Fil,Lin,Func,Cond,Fmt,...) RPS_ASSERTPRINTF_AT_BIS(Fil,Lin,Func,Cond,Fmt,##__VA_ARGS__)
#define RPS_ASSERTPRINTF(Cond,Fmt,...) RPS_ASSERTPRINTF_AT(__FILE__,__LINE__,__PRETTY_FUNCTION__,(Cond),Fmt,##__VA_ARGS__)
#else
#define RPS_ASSERT(Cond) do { if (false && (Cond)) rps_fatal_stop_at(__FILE_,__LINE__); } while(0)
#define RPS_ASSERTPRINTF(Cond,Fmt,...)  do { if (false && (Cond)) \
      fprintf(stderr, Fmt "\n", ##__VA_ARGS__); } while(0)
#endif /*NDEBUG*/


static inline double rps_monotonic_real_time(void);
double rps_elapsed_real_time(void);
static inline double rps_process_cpu_time(void);
static inline double rps_thread_cpu_time(void);
extern "C" const char* rps_hostname(void);


extern void print_types_info(void);

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
// safely give a prime of given rank from the table
extern "C" int64_t rps_prime_ranked (int rk);
// give some prime greater or equal to a given integer, and set the
// rank if non-null pointer
extern "C" int64_t rps_prime_greaterequal_ranked (int64_t n, int*prank);
// give some prime less or equal to a given integer, and set the
// rank if non-null pointer
extern "C" int64_t rps_prime_lessequal_ranked (int64_t n, int*prank);


static constexpr unsigned rps_allocation_unit = 2*sizeof(void*);
static_assert ((rps_allocation_unit & (rps_allocation_unit-1)) == 0,
               "rps_allocation_unit is not a power of two");

class Rps_QuasiZone; // GC-managed piece of memory
class Rps_ZoneValue; // memory for values
class Rps_ObjectZone; // memory for objects

typedef uint32_t Rps_HashInt;



class Rps_ObjectRef // reference to objects, per C++ rule of five.
{
  Rps_ObjectZone*_optr;
protected:
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
  Rps_ObjectZone* to_object() const
  {
    if (is_empty()) return nullptr;
    return _optr;
  }
  const Rps_ObjectZone* to_constant_object() const
  {
    if (is_empty()) return nullptr;
    return _optr;
  }
  // rule of five
  Rps_ObjectRef(const Rps_ObjectZone*oz = nullptr)
    : _optr(const_cast<Rps_ObjectZone*>(oz))
  {
    if (RPS_UNLIKELY((oz == (const Rps_ObjectZone*)RPS_EMPTYSLOT)))
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
    RPS_ASSERT(_optr != nullptr);
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
    RPS_ASSERT(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone* operator * (void)
  {
    RPS_ASSERT(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone* operator -> (void)
  {
    RPS_ASSERT(_optr != nullptr);
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
  inline Rps_HashInt obhash (void) const;
};				// end class Rps_ObjectRef


////////////////////////////////////////////////////////////////
enum class Rps_Type : int16_t
{
  /// non-value types (or quasi-values)
  ///
  ///
  /// Values that could go into Rps_Value.
  Int = -1, // for tagged integers
  None = 0, // for nil
  ///
  /// Boxed genuine values, are "first class citizens" that could be
  /// in Rps_Value's data. Of course they are both GC-allocated and
  /// GC-scanned.
  String,
  Double,
  Set,
  Tuple,
  Object,
};

//////////////////////////////////////////////////////////////// values

//// forward declarations
class Rps_ObjectRef;
class Rps_ObjectZone;
class Rps_String;
class Rps_Double;
class Rps_SetOb;
class Rps_TupleOb;

//////////////// our value, a single word
class Rps_Value
{
public:
  struct Rps_IntTag {};
  struct Rps_DoubleTag {};
  struct Rps_ValPtrTag {};
  struct Rps_EmptyTag {};
  inline Rps_Value ();
  inline Rps_Value (nullptr_t);
  inline Rps_Value (Rps_EmptyTag);
  inline Rps_Value (intptr_t i, Rps_IntTag);
  inline Rps_Value (double d, Rps_DoubleTag);
  inline Rps_Value (const Rps_ZoneValue*ptr, Rps_ValPtrTag);
  /// C++ rule of five
  inline Rps_Value(const Rps_Value& other);
  inline Rps_Value(Rps_Value&& other);
  inline Rps_Value& operator = (const Rps_Value& other);
  inline Rps_Value& operator = (Rps_Value&& other);
  inline ~Rps_Value();
  Rps_Value(intptr_t i) : Rps_Value(i, Rps_IntTag{}) {};
  inline Rps_Value(const std::string&str);
  inline Rps_Value(double d);
  inline Rps_Value(const char*str, int len= -1);
  Rps_Value(const Rps_ZoneValue*ptr) : Rps_Value(ptr, Rps_ValPtrTag{}) {};
  Rps_Value(const Rps_ZoneValue& zv) : Rps_Value(&zv, Rps_ValPtrTag{}) {};
  inline bool operator == (const Rps_Value v) const;
  inline bool operator <= (const Rps_Value v) const;
  inline bool operator < (const Rps_Value v) const;
  inline bool operator != (const Rps_Value v) const;
  inline bool operator >= (const Rps_Value v) const;
  inline bool operator > (const Rps_Value v) const;
  inline bool is_int() const;
  inline bool is_ptr() const;
  inline bool is_object() const;
  inline bool is_set() const;
  inline bool is_string() const;
  inline bool is_double() const;
  inline bool is_tuple() const;
  inline bool is_null() const;
  inline bool is_empty() const;
  // convert, or else throw exception on failure
  inline intptr_t as_int() const;
  inline const Rps_ZoneValue* as_ptr() const;
  inline const Rps_SetOb* as_set() const;
  inline const Rps_TupleOb* as_tuple() const;
  inline  Rps_ObjectZone* as_object() const;
  inline const Rps_String* as_string() const;
  inline const Rps_Double* as_boxed_double() const;
  inline double as_double() const;
  inline const std::string as_cppstring() const;
  inline const char* as_cstring() const;
  // convert or give default
  inline intptr_t to_int(intptr_t def=0) const;
  inline const Rps_ZoneValue* to_ptr(const Rps_ZoneValue*zp = nullptr) const;
  inline const Rps_SetOb* to_set(const Rps_SetOb*defset= nullptr) const;
  inline const Rps_Double* to_boxed_double(const Rps_Double*defdbl= nullptr) const;
  inline double to_double(double def=std::nan("")) const;
  inline const Rps_TupleOb* to_tuple(const Rps_TupleOb* deftup= nullptr) const;
  inline const Rps_ObjectZone* to_object(const Rps_ObjectZone*defob
                                         =nullptr) const;
  inline const Rps_String* to_string( const Rps_String*defstr
                                      = nullptr) const;
  inline const std::string to_cppstring(std::string defstr= "") const;
private:
  union
  {
    intptr_t _ival;
    const Rps_ZoneValue* _pval;
    const void* _wptr;
  };
} __attribute__((aligned(rps_allocation_unit)));    // end of Rps_Value


//////////////// specialized subclasses of Rps_Value

class Rps_ObjectValue : public Rps_Value
{
  inline Rps_ObjectValue(const Rps_ObjectRef obr);
  inline Rps_ObjectValue(const Rps_Value val, const Rps_ObjectZone*defob=nullptr);
  inline Rps_ObjectValue(const Rps_ObjectZone* obz=nullptr);
  inline Rps_ObjectValue(nullptr_t);
}; // end class Rps_ObjectValue

class Rps_StringValue : public Rps_Value
{
  inline Rps_StringValue(const char*cstr, int len= -1);
  inline Rps_StringValue(const std::string str);
  inline Rps_StringValue(const Rps_Value val);
  inline Rps_StringValue(const Rps_String* strv);
  inline Rps_StringValue(nullptr_t);
}; // end class Rps_StringValue

class Rps_DoubleValue : public Rps_Value
{
  inline Rps_DoubleValue (double d=0.0);
  inline Rps_DoubleValue(const Rps_Value val);
}; // end class Rps_DoubleValue
////////////////////////////////////////////////////////////////



class Rps_Random
{
  static thread_local Rps_Random _rand_thr_;
  static bool _rand_is_deterministic_;
  static std::ranlux48 _rand_gen_deterministic_;
  static std::mutex _rand_mtx_deterministic_;
  /// the thread local random state
  unsigned long _rand_count;
  std::mt19937 _rand_generator;
  /// we could need very quick and poor small random numbers on just 4
  /// bits. For these, we care less about the random quality, but even
  /// more about speed. So we keep one 32 bits of random number in
  /// advance, and a count of the remaining random bits in it.
  uint32_t _rand_advance;
  uint8_t _rand_remainbits;
  unsigned _rand_threadrank;
  static std::atomic<unsigned> _rand_threadcount;
  static constexpr const unsigned _rand_reseed_period_ = 65536;
  /// private initializer
  void init_deterministic (void);
  /// private deterministic reseeder
  void deterministic_reseed (void);
  /// private constructor
  Rps_Random () :
    _rand_count(0), _rand_generator(), _rand_advance(0), _rand_remainbits(0),
    _rand_threadrank(std::atomic_fetch_add(&_rand_threadcount,1U))
  {
    if (_rand_is_deterministic_)
      init_deterministic();
  };
  ///
  uint32_t generate_32u(void)
  {
    if (RPS_UNLIKELY(_rand_count++ % _rand_reseed_period_ == 0))
      {
        if (RPS_UNLIKELY(_rand_is_deterministic_))
          deterministic_reseed();
        else
          {
            std::random_device randev;
            auto s1=randev(), s2=randev(), s3=randev(), s4=randev(),
                 s5=randev(), s6=randev(), s7=randev();
            std::seed_seq seq {s1,s2,s3,s4,s5,s6,s7};
            _rand_generator.seed(seq);
          }
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
    uint8_t res = _rand_advance & 0xf;
    _rand_remainbits -= 4;
    _rand_advance = _rand_advance>>4;
    return res;
  };
  uint8_t generate_quickly_8bits()
  {
    if (RPS_UNLIKELY(_rand_remainbits < 8))
      {
        _rand_advance = generate_32u();
        _rand_remainbits = 32;
      }
    uint8_t res = _rand_advance & 0xff;
    _rand_advance = _rand_advance>>8;
    _rand_remainbits -= 8;
    return res;
  };
public:
  static void start_deterministic(long seed); // to be called from main
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
  static uint8_t random_quickly_8bits()
  {
    return _rand_thr_.generate_quickly_8bits();
  };
};				// end class Rps_Random



////////////////////////////////////////////////////////////////
//// The objid support is in a separate file.
#include "oid_rps.hh"
////////////////////////////////////////////////////////////////

////////////////

////////////////
class Rps_BackTrace;
class Rps_BackTrace_Helper;
extern "C" void rps_print_simple_backtrace_level
(Rps_BackTrace* btp, FILE*outf, const char*beforemsg, uintptr_t pc);
extern "C" void rps_print_full_backtrace_level
(Rps_BackTrace* btp,
 FILE*outf, const char*beforemsg,
 uintptr_t pc, const char *filename, int lineno,
 const char *function);
class Rps_BackTrace
{
  RPS_FRIEND_CLASS(BackTrace_Helper);
  friend int main(int, char**);
  friend void rps_print_simple_backtrace_level
  (Rps_BackTrace* btp, FILE*outf, const char*beforemsg, uintptr_t pc);
  friend void rps_print_full_backtrace_level
  (Rps_BackTrace* btp,
   FILE*outf, const char*beforemsg,
   uintptr_t pc, const char *filename, int lineno,
   const char *function);
public:
  static constexpr unsigned _bt_magicnum_ = 0x32079c15;
  static constexpr unsigned _bt_maxdepth_ = 80;
  Rps_BackTrace(const char*name, const void*data = nullptr);
  virtual ~Rps_BackTrace();
  virtual void bt_error_method(const char*msg, int errnum);
  virtual int bt_simple_method(uintptr_t);
  virtual int bt_full_method(uintptr_t pc,
                             const char *filename, int lineno,
                             const char *function);
private:
  const unsigned _bt_magic;
  std::string _bt_name;
  std::function<int(Rps_BackTrace*,uintptr_t)> _bt_simplecb;
  std::function<int(Rps_BackTrace*,uintptr_t, const char* /*filnam*/,
                    int /*lineno*/, const char* /*funam*/)> _bt_fullcb;
  const void* _bt_data;
  static void bt_error_cb(void*data, const char*msg, int errnum);
  static int bt_simple_cb(void *data, uintptr_t pc);
  static int bt_full_cb(void *data, uintptr_t pc,
                        const char *filename, int lineno,
                        const char *function);
public:
  const void* data(void) const
  {
    return _bt_data;
  };
  unsigned magicnum(void) const
  {
    return _bt_magic;
  };
  const std::string&name (void) const
  {
    return _bt_name;
  };
  Rps_BackTrace& set_simple_cb(const std::function<int(Rps_BackTrace*,uintptr_t)>& cb)
  {
    _bt_simplecb = cb;
    return *this;
  };
  Rps_BackTrace& set_full_cb(const std::function<int(Rps_BackTrace*,uintptr_t, const char* /*filnam*/,
                             int /*lineno*/, const char* /*funam*/)> &cb)
  {
    _bt_fullcb = cb;
    return *this;
  };
  int do_simple_backtrace(int skip)
  {
    return backtrace_simple(rps_backtrace_state, skip,
                            bt_simple_cb,
                            bt_error_cb,
                            this
                           );
  };
  /// simple backtrace on stderr::
  static void run_simple_backtrace(int skip, const char*name=nullptr);
  Rps_BackTrace& simple_backtrace(int skip, int*res=nullptr)
  {
    int r = do_simple_backtrace(skip);
    if (res) *res = r;
    return *this;
  };
  ///
  int do_full_backtrace(int skip)
  {
    return backtrace_full(rps_backtrace_state, skip,
                          bt_full_cb,
                          bt_error_cb,
                          this
                         );
  }
  Rps_BackTrace& full_backtrace(int skip, int *res=nullptr)
  {
    int r = do_full_backtrace(skip);
    if (res) *res = r;
    return *this;
  }
  //// full backtrace on stderr::
  static void run_full_backtrace(int skip, const char*name=nullptr);
  static void print_backtrace(int skip, FILE* fil)
  {
    RPS_ASSERT (fil != nullptr);
    backtrace_print(rps_backtrace_state, skip, fil);
  };
};				// end class Rps_BackTrace

class Rps_BackTrace_Helper
{
  static constexpr unsigned _bth_magicnum_ = 689179293 /*0x29140a9d*/;
  unsigned _bth_magic; // always _bth_magicnum_
  mutable unsigned _bth_count;
  int _bth_lineno;
  int _bth_skip;
  mutable size_t _bth_bufsiz;
  mutable char* _bth_bufptr;
  std::string _bth_filename;
  mutable std::unique_ptr<std::ostream> _bth_out;
  Rps_BackTrace _bth_backtrace;
public:
  bool has_good_magic() const
  {
    return _bth_magic == _bth_magicnum_;
  };
  Rps_BackTrace_Helper(const char*fil, int line, int skip, const char*name);
  void do_out(void) const;
  std::ostream* swap_output(std::ostream*out) const
  {
    auto o = _bth_out.get();
    auto outp = std::unique_ptr<std::ostream> (out);
    std::swap(_bth_out, outp);
    return o;
  }
  ~Rps_BackTrace_Helper()
  {
    free(_bth_bufptr), _bth_bufptr=nullptr;
  };
};				// end of Rps_Backtrace_Helper

static inline
std::ostream& operator << (std::ostream& out, const Rps_BackTrace_Helper& rph);

// can appear in RPS_WARNOUT etc...
#define RPS_BACKTRACE_HERE(Skip,Name) \
  Rps_BackTrace_Helper(__FILE__,__LINE__,(Skip),(Name))

////////////////////////////////////////////////////// garbage collector

class Rps_GarbageCollector
{
  const std::function<void(void)> &_gc_rootmarkers;
  std::deque<Rps_ObjectRef> _gc_obscanque;
private:
  inline Rps_GarbageCollector(const std::function<void(void)> &rootmarkers);
  inline ~Rps_GarbageCollector();
  void run_gc(void);
public:
  void mark_obj(Rps_ObjectRef ob);
  void mark_value(Rps_Value val);
};				// end class Rps_GarbageCollector

////////////////////////////////////////////////////// quasi zones

class Rps_QuasiZone
{
  friend class Rps_GarbageCollector;
  const Rps_Type _type;
  volatile std::atomic_uint16_t _gcinfo;
  // we keep each quasi-zone in the _zonvec
  static std::mutex _mtx;
  static std::vector<Rps_QuasiZone*> _zonvec;
  uint32_t _rank;		// the rank in _zonvec;
  inline void* operator new (std::size_t siz, std::nullptr_t);
  inline void* operator new (std::size_t siz, unsigned wordgap);
public:
  Rps_Type stored_type(void) const
  {
    return _type;
  };
  template <typename ZoneClass, class ...Args> static ZoneClass*
  rps_allocate(Args... args)
  {
    return new(nullptr) ZoneClass(args...);
  };
  template <typename ZoneClass, class ...Args> static ZoneClass*
  rps_allocate_with_wordgap(unsigned wordgap, Args... args)
  {
    return new(wordgap) ZoneClass(args...);
  };
protected:
  inline Rps_QuasiZone(Rps_Type typ);
  virtual ~Rps_QuasiZone() =0;
  virtual uint32_t wordsize() const =0;
  virtual Rps_Type type() const
  {
    return _type;
  };
};				// end class Rps_QuasiZone;



//////////////////////////////////////////////////////////// zone values
class Rps_ZoneValue : public Rps_QuasiZone
{
  friend class Rps_Value;
  friend class Rps_GarbageCollector;
protected:
  inline Rps_ZoneValue(Rps_Type typ);
public:
  virtual void gc_mark(Rps_GarbageCollector&) =0;
  virtual Rps_HashInt val_hash () const =0;
  virtual bool equal(const Rps_ZoneValue&zv) const =0;
  virtual bool less(const Rps_ZoneValue&zv) const =0;
  inline bool operator == (const Rps_ZoneValue&zv) const;
  bool operator != (const Rps_ZoneValue&zv) const
  {
    return !(*this == zv);
  };
  virtual bool lessequal(const Rps_ZoneValue&zv) const
  {
    return *this == zv || less(zv);
  }
  inline bool operator < (const Rps_ZoneValue&zv) const;
  inline bool operator <= (const Rps_ZoneValue&zv) const;
  inline bool operator > (const Rps_ZoneValue&zv) const;
  inline bool operator >= (const Rps_ZoneValue&zv) const;
};    // end class Rps_ZoneValue

/////////////////////////////////////////////////// lazy hashed values
class Rps_LazyHashedZoneValue : public Rps_ZoneValue
{
private:
  mutable volatile std::atomic<Rps_HashInt> _lazyhash;
protected:
  virtual Rps_HashInt compute_hash(void) const =0;
  inline Rps_LazyHashedZoneValue(Rps_Type typ);
  virtual ~Rps_LazyHashedZoneValue() {};
public:
  Rps_HashInt lazy_hash() const
  {
    return _lazyhash.load();
  };
  virtual Rps_HashInt val_hash () const
  {
    volatile Rps_HashInt h = _lazyhash.load();
    if (RPS_UNLIKELY(h == 0))
      {
        h = compute_hash();
        RPS_ASSERT (h != 0);
        _lazyhash.store(h);
      }
    return h;
  };
};				// end of Rps_LazyHashedZoneValue
//////////////////////////////////////////////////////////// immutable strings

// compute a long hash in ht[0] and ht[1]. Return the number of UTF-8
// character or else 0 if cstr with len bytes is not proper UTF-8
extern "C"
int rps_compute_cstr_two_64bits_hash(int64_t ht[2], const char*cstr, int len= -1);

static inline Rps_HashInt rps_hash_cstr(const char*cstr, int len= -1);

class Rps_String : public Rps_LazyHashedZoneValue
{
  friend Rps_String*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_String,const char*,int>(unsigned,const char*,int);
  const uint32_t _bytsiz;
  const uint32_t _utf8len;
  union
  {
    const char _sbuf[RPS_FLEXIBLE_DIM];
    char _alignbuf[rps_allocation_unit] __attribute__((aligned(rps_allocation_unit)));
  };
protected:
  inline Rps_String (const char*cstr, int len= -1);
  static inline const char*normalize_cstr(const char*cstr);
  static inline int normalize_len(const char*cstr, int len);
  static inline uint32_t safe_utf8len(const char*cstr, int len);
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    return rps_hash_cstr(_sbuf);
  };
  virtual void gc_mark(Rps_GarbageCollector&) { };
public:
  virtual uint32_t wordsize() const
  {
    return (sizeof(Rps_String)+_bytsiz+1)/sizeof(void*);
  };
  static const Rps_String* make(const char*cstr, int len= -1);
  static const Rps_String* make(const QString&qs);
  static inline const Rps_String* make(const std::string&s);
  const char*cstr() const
  {
    return _sbuf;
  };
  const std::string cppstring() const
  {
    return std::string(_sbuf);
  };
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == Rps_Type::String)
      {
        auto othstr = reinterpret_cast<const Rps_String*>(&zv);
        auto lh = lazy_hash();
        auto othlh = othstr->lazy_hash();
        if (lh != 0 && othlh != 0 && lh != othlh) return false;
        return !strcmp(cstr(), othstr->cstr());
      }
    else return false;
  }
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() > Rps_Type::String) return false;
    if (zv.stored_type() < Rps_Type::String) return true;
    auto othstr = reinterpret_cast<const Rps_String*>(&zv);
    return strcmp(cstr(), othstr->cstr()) < 0;
  };
};    // end class Rps_String


//////////////// boxed doubles
class Rps_Double  : public Rps_LazyHashedZoneValue
{
  friend Rps_Double*
  Rps_QuasiZone::rps_allocate<Rps_Double,double>(double);
  const double _dval;
protected:
  inline Rps_Double (double d=0.0);
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    auto rh = std::hash<double> {}((double)_dval);
    Rps_HashInt h = static_cast<Rps_HashInt> (rh);
    if (RPS_UNLIKELY(h == 0))
      h = 987383;
    return h;
  };
  virtual void gc_mark(Rps_GarbageCollector&) { };
public:
  double dval() const
  {
    return _dval;
  };
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  static inline const Rps_Double*make(double d=0.0);
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == Rps_Type::Double)
      {
        auto othdbl = reinterpret_cast<const Rps_Double*>(&zv);
        return _dval == othdbl->dval();
      };
    return false;
  };
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() > Rps_Type::Double) return false;
    if (zv.stored_type() < Rps_Type::Double) return true;
    auto othdbl = reinterpret_cast<const Rps_Double*>(&zv);
    return _dval < othdbl->dval();
  };
}; // end class Rps_Double




//////////////////////////////////////////////////////////// object zones
class Rps_ObjectZone : public Rps_ZoneValue
{
  const Rps_Id _oid;
public:
  const Rps_Id oid() const
  {
    return _oid;
  };
  Rps_HashInt obhash() const
  {
    return _oid.hash();
  };
};				// end class Rps_ObjectZone

/////////////////////////// sequences (tuples or sets) of Rps_ObjectRef
class Rps_SetOb;
class Rps_TupleOb;
template<typename RpsSeq, Rps_Type seqty, unsigned k1, unsigned k2, unsigned k3>
class Rps_SeqObjRef : public Rps_LazyHashedZoneValue
{
  friend class Rps_SetOb;
  friend class Rps_TupleOb;
  friend RpsSeq*
  Rps_QuasiZone::rps_allocate_with_wordgap<RpsSeq,unsigned>(unsigned,unsigned);
  const unsigned _seqlen;
  Rps_ObjectRef _seqob[RPS_FLEXIBLE_DIM+1];
  Rps_SeqObjRef(unsigned len) : Rps_LazyHashedZoneValue(seqty), _seqlen(len)
  {
    memset (_seqob, 0, sizeof(Rps_ObjectRef)*len);
  };
  Rps_ObjectRef*raw_data()
  {
    return _seqob;
  };
public:
  static unsigned constexpr maxsize
    = std::numeric_limits<unsigned>::max() / 2;
  unsigned cnt() const
  {
    return _seqlen;
  };
  typedef const Rps_ObjectRef*iterator_t;
  iterator_t begin() const
  {
    return const_cast<const Rps_ObjectRef*>(_seqob);
  };
  iterator_t end() const
  {
    return const_cast<const Rps_ObjectRef*>(_seqob) + _seqlen;
  };
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this) + _seqlen * sizeof(_seqob[0])) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc)
  {
    for (auto ob: *this)
      if (ob)
        gc.mark_obj(ob);
  };
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    Rps_HashInt h0= 3317+(k3&0xff), h1= 31*_seqlen;
    for (unsigned ix=0; ix<_seqlen; ix += 2)
      {
        if (RPS_UNLIKELY(_seqob[ix].is_empty()))
          throw std::runtime_error("corrupted sequence of objects");
        h0 = (h0 * k1) ^ (_seqob[ix]->obhash() * k2 + ix);
        auto nextob = _seqob[ix+1];
        if (RPS_UNLIKELY(nextob.is_empty())) break;
        h1 = (h1 * k2) ^ (nextob->obhash() * k3 - (h0&0xfff));
      };
    Rps_HashInt h = 5*h0 + 11*h1;
    if (RPS_UNLIKELY(h == 0))
      h = ((h0 & 0xfffff) ^ (h1 & 0xfffff)) + (k1/128 + (_seqlen & 0xff) + 3);
    RPS_ASSERT(h != 0);
    return h;
  };
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == seqty)
      {
        auto oth = reinterpret_cast<const RpsSeq*>(&zv);
        if (RPS_LIKELY(reinterpret_cast<const Rps_LazyHashedZoneValue*>(this)->val_hash()
                       != reinterpret_cast<const Rps_LazyHashedZoneValue*>(oth)->val_hash()))
          return false;
        auto curcnt = cnt();
        if (RPS_LIKELY(oth->cnt() != curcnt))
          return false;
        for (unsigned ix = 0; ix < curcnt; ix++)
          if (_seqob[ix] != oth->_seqob[ix])
            return false;
        return true;
      }
    return false;
  }
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == seqty)
      {
        auto oth = reinterpret_cast<const RpsSeq*>(&zv);
        if (RPS_LIKELY(reinterpret_cast<const Rps_LazyHashedZoneValue*>(this)->val_hash()
                       != reinterpret_cast<const Rps_LazyHashedZoneValue*>(oth)->val_hash()))
          return false;
        return std::lexicographical_compare(begin(), end(),
                                            oth->begin(), oth->end());
      }
    return false;
  };
};    // end of Rps_SeqObjRef


/////////////////////////// sets of Rps_ObjectRef
unsigned constexpr rps_set_k1 = 7933;
unsigned constexpr rps_set_k2 = 8963;
unsigned constexpr rps_set_k3 = 19073;
class Rps_SetOb: public Rps_SeqObjRef<Rps_SetOb, Rps_Type::Set, rps_set_k1, rps_set_k2, rps_set_k3>
{
public:
  struct Rps_SetTag
  {
  };
  friend class Rps_SeqObjRef<Rps_SetOb, Rps_Type::Set, rps_set_k1, rps_set_k2, rps_set_k3>;
  typedef Rps_SeqObjRef<Rps_SetOb, Rps_Type::Set, rps_set_k1, rps_set_k2, rps_set_k3> parentseq_t;
  Rps_SetOb(unsigned len, Rps_SetTag) :parentseq_t (len) {};
  Rps_SetOb(const std::set<Rps_ObjectRef>& setob, Rps_SetTag);
protected:
  friend Rps_SetOb*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_SetOb,unsigned,Rps_SetTag>(unsigned,unsigned,Rps_SetTag);
public:
  // make a set from given object references
  static const Rps_SetOb*make(const std::set<Rps_ObjectRef>& setob);
  static const Rps_SetOb*make(const std::vector<Rps_ObjectRef>& vecob);
  static const Rps_SetOb*make(const std::initializer_list<Rps_ObjectRef>&elemil);
  // collect a set from several objects, tuples, or sets
  static const Rps_SetOb*collect(const std::vector<Rps_Value>& vecval);
  static const Rps_SetOb*collect(const std::initializer_list<Rps_Value>&valil);
#warning Rps_SetOb very incomplete
};// end of Rps_SetOb

/////////////////////////// tuples of Rps_ObjectRef
unsigned constexpr rps_tuple_k1 = 5939;
unsigned constexpr rps_tuple_k2 = 18917;
unsigned constexpr rps_tuple_k3 = 6571;
class Rps_TupleOb: public Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3>
{
public:
  struct Rps_TupleTag {};
  friend class  Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3>;
  typedef Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3> parentseq_t;
protected:
  Rps_TupleOb(unsigned len, Rps_TupleTag)
    : parentseq_t (len) {};
  friend Rps_TupleOb*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_TupleOb,unsigned,Rps_TupleTag>(unsigned,unsigned,Rps_TupleTag);
public:
  // make a tuple from given object references
  static const Rps_TupleOb*make(const std::vector<Rps_ObjectRef>& vecob);
  static const Rps_TupleOb*make(const std::initializer_list<Rps_ObjectRef>&compil);
  // collect a tuple from several objects, tuples, or sets
  static const Rps_TupleOb*collect(const std::vector<Rps_Value>& vecval);
  static const Rps_TupleOb*collect(const std::initializer_list<Rps_Value>&valil);
#warning Rps_TupleOb very incomplete
};// end of Rps_TupleOb

////////////////////////////////////////////////////////////////
extern "C" void rps_run_application (int& argc, char**argv); // in appli_qrps.cc

extern "C" void rps_dump_into (const std::string dirpath = "."); // in store_rps.cc

extern "C" void rps_garbage_collect (void);

#include "inline_rps.hh"

#endif /*REFPERSYS_INCLUDED*/
// end of file refpersys.hh */

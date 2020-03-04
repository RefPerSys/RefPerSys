/****************************************************************
 * file refpersys.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is almost its only public C++ header file.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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
#include <unordered_set>
#include <new>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <functional>
#include <typeinfo>
#include <locale>

#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <clocale>

#include <argp.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pthread.h>



// for programmatic C++ name demangling, see also
// https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/cxxabi.h
#include <cxxabi.h>

#include <QObject>
#include <QString>
#include <QPointer>


// JsonCPP https://github.com/open-source-parsers/jsoncpp
#include "json/json.h"


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
extern "C" const char rps_topdirectory[];
extern "C" const char rps_gitid[];
extern "C" const char rps_lastgittag[];
extern "C" const char rps_lastgitcommit[];
extern "C" const char rps_md5sum[];
extern "C" const char*const rps_files[];
extern "C" const char rps_makefile[];
extern "C" const char*const rps_subdirectories[];


///////////////////////////////////////////////////////////////////////////////
/// Provides miscellaneous runtime information for RefPerSys.
///
/// The RpsColophon class is a convenience C++ wrapper around the extern
/// constants generated at runtime in the _timestamp_rps.c file. This class
/// allows for a cleaner and more idiomatic way to reference the runtime
/// generated constants.
class RpsColophon
{
public:

  /// Gets the current timestamp.
  ///
  /// @see rps_timestamp[]
  static inline std::string timestamp()
  {
    return std::string (rps_timestamp);
  }

  /// Gets the current Git ID.
  ///
  /// @see rps_gitid[]
  static inline std::string git_id()
  {
    return std::string (rps_gitid);
  }

  /// Gets the MD5 sum of the source.
  ///
  /// @see rps_md5sum
  static inline std::string source_md5()
  {
    return std::string (rps_md5sum);
  }

  /// Gets the last Git commit details.
  ///
  /// @see rps_lastgitcommit
  static inline std::string last_git_commit()
  {
    return std::string (rps_lastgitcommit);
  }

  /// Gets the top level directory.
  ///
  /// rps_topdirectory
  static inline std::string top_directory()
  {
    return std::string (rps_topdirectory);
  }

  /// Gets the URL of the RefPerSys website.
  static inline std::string website()
  {
    return std::string ("http://refpersys.org/");
  }
};				// end class RpsColophon


/// backtrace support
extern "C" struct backtrace_state* rps_backtrace_state;

/// the program name
extern "C" const char* rps_progname;


/// the initial copyright year of RefPerSys
#define RPS_INITIAL_COPYRIGHT_YEAR 2019
// the number of jobs, that of threads, to run in parallel
extern "C" int rps_nbjobs;
#define RPS_NBJOBS_MIN 2
#define RPS_NBJOBS_MAX 20

/// is the current thread the main GUI Qt thread?
extern "C" bool rps_is_main_gui_thread(void);

/// the refpersys homedir, e.g. $REFPERSYS_HOME or $HOME or given with
/// --refpersys-home <dir>
extern "C" const char* rps_homedir(void);

extern "C" void rps_emit_gplv3_copyright_notice(std::ostream&outs, std::string path, std::string linprefix, std::string linsuffix);

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


/// this macro RPS_NOPRINT and the following one RPS_NOPRINTOUT are
/// optimized to no-op but still typechecked. They could be used as a
/// drop-in replacement to RPS_INFORM and RPS_INFORMOUT, so that only
/// a few letters of change are needed, while keeping a previous debug
/// or information output....
#define RPS_NOPRINT(Fmt,...) do { if (false) \
      RPS_INFORM(Fmt,##__VA_ARGS__); }while(0)

#define RPS_NOPRINTOUT(...) do { if(false) \
      RPS_INFORMOUT(__VA_ARGS__); }while(0)


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


#define RPS_RUNTIME_ERROR_OUT_AT_BIS(Fil,Lin,...) ({	\
      std::ostringstream outs##Lin;			\
      outs##Lin << Fil << ":"<< Lin << "::"		\
		<< __VA_ARGS__;				\
      auto res##Lin =					\
	std::runtime_error(outs##Lin.str());		\
      res##Lin; })

#define RPS_RUNTIME_ERROR_OUT_AT(Fil,Lin,...) RPS_RUNTIME_ERROR_OUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be throw RPS_RUNTIME_ERROR_OUT("annoying x=" << x)
#define RPS_RUNTIME_ERROR_OUT(...) RPS_RUNTIME_ERROR_OUT_AT(__FILE__,__LINE__,##__VA_ARGS__)



static inline double rps_monotonic_real_time(void);
static inline double rps_wallclock_real_time(void);
double rps_elapsed_real_time(void);
static inline double rps_process_cpu_time(void);
static inline double rps_thread_cpu_time(void);
extern "C" const char* rps_hostname(void);
extern "C" void*rps_proghdl; // dlopen handle of whole program

extern "C" Json::Value rps_string_to_json(const std::string&str);
extern "C" std::string rps_json_to_string(const Json::Value&jv);

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
class Rps_JsonZone; // memory for Json values
class Rps_QtPtrZone;
class Rps_QtPtrValue;
class Rps_GarbageCollector;
class Rps_Payload;
class Rps_PayloadSymbol;
class Rps_PayloadClassInfo;
class Rps_Loader;
class Rps_Dumper;
class Rps_CallFrame;
class Rps_Value;
class Rps_Id;


typedef uint32_t Rps_HashInt;


////////////////////////////////////////////////////////////////
class Rps_ObjectRef // reference to objects, per C++ rule of five.
{
  Rps_ObjectZone*_optr;
protected:
public:
  struct Rps_ObjIdStrTag {};
  Rps_ObjectZone* optr() const
  {
    return _optr;
  };
  inline std::recursive_mutex* objmtx(void) const;
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
  // get object from Json at load time
  Rps_ObjectRef(const Json::Value &, Rps_Loader*); //in store_rps.cc
  
  // build an object from its existing string oid, or else fail with C++ exception
  Rps_ObjectRef(Rps_CallFrame*callerframe, const char*oidstr, Rps_ObjIdStrTag);
  
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
  inline void gc_mark(Rps_GarbageCollector&) const;
  inline void dump_scan(Rps_Dumper* du, unsigned depth) const;
  inline Json::Value dump_json(Rps_Dumper* du) const;
  void output(std::ostream&os) const;
  /////////// the root space
  static inline Rps_ObjectRef root_space(void);
  ///////////
  // these functions throw an exception on failure (unless dontfail is true, then gives nil)
  // find an object with a given oid or name string
  static Rps_ObjectRef find_object_by_string(Rps_CallFrame*callerframe,  const std::string& str, bool dontfail=false);
  static Rps_ObjectRef find_object_by_oid(Rps_CallFrame*callerframe, Rps_Id oid, bool dontfail=false);
  // create a class of given super class and name
  static Rps_ObjectRef make_named_class(Rps_CallFrame*callerframe, Rps_ObjectRef superclassob, std::string name);
  // create a symbol of given name
  static Rps_ObjectRef make_new_symbol(Rps_CallFrame*callerframe, std::string name, bool isweak);
  static Rps_ObjectRef make_new_strong_symbol(Rps_CallFrame*callerframe, std::string name)
  {
    return make_new_symbol(callerframe, name, false);
  };
  static Rps_ObjectRef make_new_weak_symbol(Rps_CallFrame*callerframe, std::string name)
  {
    return make_new_symbol(callerframe, name, true);
  };
  // create an object of given class
  static Rps_ObjectRef make_object(Rps_CallFrame*callerframe, Rps_ObjectRef classob, Rps_ObjectRef spaceob=nullptr);
  // create a mutable set oject
  static Rps_ObjectRef make_mutable_set_object(Rps_CallFrame*callerframe, Rps_ObjectRef spaceob=nullptr);
  // the superclass of all objects, that is the `object` object
  static inline Rps_ObjectRef the_object_class(void);
  // the class of all classes, that is the `class` object
  static inline Rps_ObjectRef the_class_class(void);
  // the class of all symbols, that is the `symbol` object
  static inline Rps_ObjectRef the_symbol_class(void);
  // the class of mutable sets, that is the `mutable_set` object
  static inline Rps_ObjectRef the_mutable_set_class(void);
  // if this is a class-object, install a method of selector obsel and
  // closure closv; otherwise raise an exception; and likewise for two
  // or three selectors. The callerframe is not really useful today,
  // but might be useful later...
  // NB: we might call these from the temporary plugin window.
  void install_own_method(Rps_CallFrame*callerframe, Rps_ObjectRef obsel, Rps_Value closv);
  // likewise, but lock this class only once!
  void install_own_2_methods(Rps_CallFrame*callerframe, Rps_ObjectRef obsel0, Rps_Value closv0, Rps_ObjectRef obsel1, Rps_Value closv1);
  void install_own_3_methods(Rps_CallFrame*callerframe, Rps_ObjectRef obsel0, Rps_Value closv0, Rps_ObjectRef obsel1, Rps_Value closv1, Rps_ObjectRef obsel2, Rps_Value closv2);
};				// end class Rps_ObjectRef




static_assert(sizeof(Rps_ObjectRef) == sizeof(void*),
              "Rps_ObjectRef should have the size of a word");
static_assert(alignof(Rps_ObjectRef) == alignof(void*),
              "Rps_ObjectRef should have the alignment of a word");

// we could code Rps_ObjectFromOidRef(&_,"_41OFI3r0S1t03qdB2E") instead of rpskob_41OFI3r0S1t03qdB2E
class Rps_ObjectFromOidRef : public Rps_ObjectRef {
public:
  Rps_ObjectFromOidRef(Rps_CallFrame*callerframe, const char*oidstr) :
    Rps_ObjectRef(callerframe, oidstr, Rps_ObjectRef::Rps_ObjIdStrTag{}) {};
};				// end Rps_ObjectFromOidRef

/// mostly for debugging
inline std::ostream&
operator << (std::ostream&out, Rps_ObjectRef obr)
{
  obr.output(out);
  return out;
}

//////////////////////////////////////////////////////////////// constant objects

// constant objects starts with
#define RPS_CONSTANTOBJ_PREFIX "rpskob"

#define RPS_INSTALL_CONSTANT_OB(Oid) extern "C" Rps_ObjectRef rpskob##Oid;
#include "generated/rps-constants.hh"
unsigned constexpr rps_nb_constants = RPS_NB_CONSTANT_OB;

////////////////////////////////////////////////////////////////
enum class Rps_Type : std::int16_t
{
  CallFrame = std::numeric_limits<std::int16_t>::min(),
  ////////////////
  /// payloads are negative, below -1
  PaylQt = -11, // for Rps_PayloadQt<>
  PaylSymbol = -10, // symbol payload
  PaylSpace = -9, // space payload
  PaylStrBuf = -8, // mutable string buffer
  PaylRelation = -7, // mutable binary relation between objects
  PaylAssoc = -6, // mutable association from object to values
  PaylVectVal = -5, // mutable vector of values payload
  PaylVectOb = -4, // mutable vector of objects payload
  PaylSetOb = -3, // mutable set of objects payload
  PaylClassInfo = -2, // class information payload
  Payl__LeastRank = -2,
  ////////////////
  /// non-value types (or quasi-values)
  ///
  ///
  Int = -1, // for tagged integers
  None = 0, // for nil
  ////////////////
  ///
  /// Values that could go into Rps_Value:
  /// Boxed genuine values, are "first class citizens" that could be
  /// in Rps_Value's data. Of course they are both GC-allocated and
  /// GC-scanned.
  String,
  Double,
  Set,
  Tuple,
  Object,
  Closure,
  Instance,
  QtPtr,
  Json,
};

//////////////////////////////////////////////////////////////// values

//// forward declarations
class Rps_ObjectRef;
class Rps_ObjectZone;
class Rps_String;
class Rps_Double;
class Rps_SetOb;
class Rps_TupleOb;
class Rps_ClosureZone;
class Rps_InstanceZone;
class Rps_GarbageCollector;
class Rps_Loader; // in store_rps.cc
class Rps_Dumper; // in store_rps.cc
class Rps_ClosureValue;
class Rps_SetValue;
class Rps_InstanceValue;
class Rps_TupleValue;
struct Rps_TwoValues;

//////////////// our value, a single word
class Rps_Value
{
  friend class Rps_PayloadSymbol;
public:
  // the maximal depth of the inheritance graph. An arbitrary, but small limit.
  static constexpr unsigned maximal_inheritance_depth = 32;
  /// various C++ tags
  struct Rps_IntTag {};
  struct Rps_DoubleTag {};
  struct Rps_ValPtrTag {};
  struct Rps_EmptyTag {};
  /// various constructors
  inline Rps_Value ();
  inline Rps_Value (std::nullptr_t);
  inline Rps_Value (Rps_EmptyTag);
  inline Rps_Value (intptr_t i, Rps_IntTag);
  inline Rps_Value (double d, Rps_DoubleTag);
  inline Rps_Value (const Rps_ZoneValue*ptr, Rps_ValPtrTag);
  Rps_Value(Rps_ObjectRef obr)
    : Rps_Value((const Rps_ZoneValue*)(obr.optr()), Rps_ValPtrTag{})
  {
  };
  inline Rps_Value (const void*ptr, const Rps_PayloadSymbol*symb);
  inline Rps_Value (const void*ptr, Rps_CallFrame*cframe);
  Rps_Value(const Json::Value &hjv, Rps_Loader*ld); // in store_rps.cc
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
  ///
  Rps_ClosureValue closure_for_method_selector(Rps_CallFrame*cframe, Rps_ObjectRef obselector) const;
  inline const void* data_for_symbol(Rps_PayloadSymbol*) const;
  static constexpr unsigned max_gc_mark_depth = 100;
  inline void gc_mark(Rps_GarbageCollector&gc, unsigned depth= 0) const;
  void dump_scan(Rps_Dumper* du, unsigned depth) const;
  Json::Value dump_json(Rps_Dumper* du) const;
  inline bool operator == (const Rps_Value v) const;
  inline bool operator <= (const Rps_Value v) const;
  inline bool operator < (const Rps_Value v) const;
  inline bool operator != (const Rps_Value v) const;
  inline bool operator >= (const Rps_Value v) const;
  inline bool operator > (const Rps_Value v) const;
  inline Rps_Type type() const;
  inline bool is_int() const;
  inline bool is_ptr() const;
  inline bool is_object() const;
  inline bool is_instance() const;
  inline bool is_set() const;
  inline bool is_closure() const;
  inline bool is_string() const;
  inline bool is_double() const;
  inline bool is_tuple() const;
  inline bool is_null() const;
  inline bool is_empty() const;
  inline bool is_json() const;
  inline bool is_qtptr() const;
  operator bool () const
  {
    return !is_empty();
  };
  bool operator ! () const
  {
    return is_empty();
  };
  // convert, or else throw exception on failure
  inline intptr_t as_int() const;
  inline const Rps_ZoneValue* as_ptr() const;
  inline const Rps_SetOb* as_set() const;
  inline const Rps_TupleOb* as_tuple() const;
  inline  Rps_ObjectZone* as_object() const;
  inline const Rps_InstanceZone* as_instance() const;
  inline const Rps_String* as_string() const;
  inline const Rps_ClosureZone* as_closure() const;
  inline const Rps_Double* as_boxed_double() const;
  inline const Rps_JsonZone* as_json() const;
  inline const Rps_QtPtrZone* as_boxed_qtptr() const;
  inline const QPointer<QObject> as_qtptr() const;
  inline double as_double() const;
  inline const std::string as_cppstring() const;
  inline const char* as_cstring() const;
  Rps_ObjectRef compute_class(Rps_CallFrame*) const;
  // convert or give default
  inline intptr_t to_int(intptr_t def=0) const;
  inline const Rps_ZoneValue* to_ptr(const Rps_ZoneValue*zp = nullptr) const;
  inline const Rps_SetOb* to_set(const Rps_SetOb*defset= nullptr) const;
  inline const Rps_Double* to_boxed_double(const Rps_Double*defdbl= nullptr) const;
  inline double to_double(double def=std::nan("")) const;
  inline const Rps_JsonZone* to_boxed_json(const Rps_JsonZone*defjson= nullptr) const;
  inline const Json::Value to_json(const Json::Value defjv=Json::Value::nullSingleton()) const;
  inline const Rps_TupleOb* to_tuple(const Rps_TupleOb* deftup= nullptr) const;
  inline const Rps_ClosureZone* to_closure(const Rps_ClosureZone* defclos= nullptr) const;
  inline const Rps_ObjectZone* to_object(const Rps_ObjectZone*defob
                                         =nullptr) const;
  inline const Rps_InstanceZone* to_instance(const Rps_InstanceZone*definst =nullptr) const;
  inline const Rps_String* to_string( const Rps_String*defstr
                                      = nullptr) const;
  inline const QObject*to_qtptr(const QObject*defqt= nullptr) const;
  inline const std::string to_cppstring(std::string defstr= "") const;
  inline Rps_HashInt valhash() const noexcept;
  inline void output(std::ostream&out, unsigned depth=0) const;
  static constexpr unsigned max_output_depth = 5;
  Rps_Value get_attr(Rps_CallFrame*stkf, const Rps_ObjectRef obattr) const;
  inline void clear(void);
  inline Rps_Value& operator = (std::nullptr_t)
  {
    clear();
    return *this;
  };
  // test if this value is instance of obclass:
  inline bool is_instance_of(Rps_CallFrame*callerframe, Rps_ObjectRef obclass) const;
  // test if this value is a subclass of given obsuperclass:
  inline bool is_subclass_of(Rps_CallFrame*callerframe, Rps_ObjectRef obsuperclass) const;
  Rps_TwoValues send0(Rps_CallFrame*cframe, const Rps_ObjectRef obsel) const;
  Rps_TwoValues send1(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      Rps_Value arg0) const;
  Rps_TwoValues send2(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      Rps_Value arg0, const Rps_Value arg1) const;
  Rps_TwoValues send3(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1, const Rps_Value arg2) const;
  Rps_TwoValues send4(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3) const;
  Rps_TwoValues send5(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4) const;
  Rps_TwoValues send6(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5) const;
  Rps_TwoValues send7(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5,
                      const Rps_Value arg6) const;
  Rps_TwoValues send8(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5,
                      const Rps_Value arg6, const Rps_Value arg7) const;
  Rps_TwoValues send9(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5,
                      const Rps_Value arg6, const Rps_Value arg7,
                      const Rps_Value arg8) const;
  Rps_TwoValues send_vect(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                          const std::vector<Rps_Value>& argvec) const;
  Rps_TwoValues send_ilist(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                           const std::initializer_list<Rps_Value>& argil) const;
private:
  union
  {
    intptr_t _ival;
    const Rps_ZoneValue* _pval;
    const void* _wptr;
  };
};    // end of Rps_Value
static_assert(sizeof(Rps_Value) == sizeof(void*),
              "Rps_Value should have the size of a word");
static_assert(alignof(Rps_Value) == alignof(void*),
              "Rps_Value should have the alignment of a word");



////////////////////////////////////////////////////////////////

struct Rps_TwoValues
{
  Rps_Value main_val;
  Rps_Value xtra_val;
  Rps_TwoValues(std::nullptr_t) :
    main_val(nullptr), xtra_val(nullptr) {};
  Rps_TwoValues(Rps_Value m=nullptr, Rps_Value x=nullptr)
    : main_val(m), xtra_val(x) {};
  Rps_Value main() const
  {
    return main_val;
  };
  Rps_Value xtra() const
  {
    return xtra_val;
  };
  operator Rps_Value (void) const
  {
    return main();
  };
  void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0) const
  {
    if (main_val) main_val.gc_mark(gc,depth);
    if (xtra_val) xtra_val.gc_mark(gc,depth);
  };
  bool operator ! (void) const {
    return !main_val && !xtra_val;
  };
  operator bool (void) const {
    return main_val || xtra_val;
  };
};				// end Rps_TwoValues

/// mostly for debugging
inline std::ostream&
operator << (std::ostream&out, Rps_Value val)
{
  val.output(out);
  return out;
}

namespace std
{
template <> struct hash<Rps_ObjectRef>
{
  std::size_t operator()(Rps_ObjectRef const& obr) const noexcept
  {
    return obr.obhash();
  };
};
template <> struct hash<Rps_Value>
{
  std::size_t operator()(Rps_Value const& val) const noexcept
  {
    return val.valhash();
  };
};
};

//////////////// specialized subclasses of Rps_Value

class Rps_ObjectValue : public Rps_Value
{
public:
  inline Rps_ObjectValue(const Rps_ObjectRef obr);
  inline Rps_ObjectValue(const Rps_Value val, const Rps_ObjectZone*defob=nullptr);
  inline Rps_ObjectValue(const Rps_ObjectZone* obz=nullptr);
  inline Rps_ObjectValue(std::nullptr_t);
}; // end class Rps_ObjectValue

class Rps_StringValue : public Rps_Value
{
public:
  inline Rps_StringValue(const char*cstr, int len= -1);
  inline Rps_StringValue(const std::string str);
  inline Rps_StringValue(const Rps_Value val);
  inline Rps_StringValue(const Rps_String* strv);
  Rps_StringValue(const QString& qstr);
  inline Rps_StringValue(std::nullptr_t);
}; // end class Rps_StringValue

class Rps_DoubleValue : public Rps_Value
{
public:
  inline Rps_DoubleValue (double d=0.0);
  inline Rps_DoubleValue(const Rps_Value val);
}; // end class Rps_DoubleValue

class Rps_JsonValue : public Rps_Value
{
public:
  inline Rps_JsonValue(const Json::Value&jv);
  inline Rps_JsonValue(const Rps_Value val);
  Rps_JsonValue(double d)
    : Rps_JsonValue(Json::Value(d)) {};
  Rps_JsonValue(std::intptr_t i)
    : Rps_JsonValue(Json::Value((Json::Int64)i)) {};
  Rps_JsonValue(bool b)
    : Rps_JsonValue(Json::Value(b)) {};
  Rps_JsonValue(const std::string& s)
    : Rps_JsonValue(Json::Value(s)) {};
  Rps_JsonValue(std::nullptr_t)
    : Rps_JsonValue(Json::Value(Json::nullValue)) {};
}; // end class Rps_JsonValue

////////////////
class Rps_QtPtrValue : public Rps_Value
{
public:
    inline Rps_QtPtrValue(const QPointer<QObject> qptrval);
  inline Rps_QtPtrValue(Rps_Value val);
    Rps_QtPtrValue(const QObject* qo)
    {
        Rps_QtPtrValue(QPointer<QObject>(const_cast<QObject*>(qo)));
    };
    Rps_QtPtrValue(std::nullptr_t)
      : Rps_QtPtrValue(QPointer<QObject>(nullptr))
    { }
};				// end class Rps_QtPtrValue


struct Rps_SetTag
{
};				// end empty struct Rps_SetTag

struct Rps_TupleTag
{
}; // end empty struct Rps_TupleTag



////////////////
class Rps_SetValue : public Rps_Value
{
public:
  /// related to Rps_SetOb::make :
  inline Rps_SetValue (const std::set<Rps_ObjectRef>& obset);
  inline Rps_SetValue (const std::vector<Rps_ObjectRef>& obvec);
  inline Rps_SetValue (const std::initializer_list<Rps_ObjectRef>& obil, Rps_SetTag);
  Rps_SetValue (const std::initializer_list<Rps_ObjectRef>& obil) :
    Rps_SetValue(obil, Rps_SetTag{}) {};
  /// related to Rps_SetOb::collect :
  inline Rps_SetValue(const std::vector<Rps_Value>& vecval);
  inline Rps_SetValue(const std::initializer_list<Rps_Value>&valil);
  // "dynamic" casting :
  inline Rps_SetValue(const Rps_Value);
};    // end class Rps_SetValue




////////////////
class Rps_TupleValue : public Rps_Value
{
public:
  /// related to Rps_TupleOb::make :
  inline Rps_TupleValue (const std::vector<Rps_ObjectRef>& obvec);
  inline Rps_TupleValue (const std::initializer_list<Rps_ObjectRef>&compil, Rps_TupleTag);
  Rps_TupleValue(const std::initializer_list<Rps_ObjectRef>&compil)
    : Rps_TupleValue(compil, Rps_TupleTag{}) {};
  /// related to Rps_TupleOb::collect :
  inline Rps_TupleValue (const std::vector<Rps_Value>& vecval);
  inline Rps_TupleValue (const std::initializer_list<Rps_Value>&valil);
  // "dynamic" casting :
  inline Rps_TupleValue(Rps_Value val);
};    // end class Rps_TupleValue
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
extern "C" void rps_garbage_collect(std::function<void(Rps_GarbageCollector*)>* fun=nullptr);
class Rps_GarbageCollector
{
  friend void rps_garbage_collect(std::function<void(Rps_GarbageCollector*)>* fun);
  static std::atomic<Rps_GarbageCollector*> gc_this;
  static std::atomic<uint64_t> gc_count;
  friend class Rps_QuasiZone;
  std::mutex gc_mtx;
  std::atomic<bool> gc_running;
  const std::function<void(Rps_GarbageCollector*)> gc_rootmarkers;
  std::deque<Rps_ObjectRef> gc_obscanque;
  uint64_t gc_nbscan;
  uint64_t gc_nbmark;
  uint64_t gc_nbdelete;
  uint64_t gc_nbroots;
  double gc_startelapsedtime;
  double gc_startprocesstime;
private:
  Rps_GarbageCollector(const std::function<void(Rps_GarbageCollector*)> &rootmarkers=nullptr);
  ~Rps_GarbageCollector();
  void run_gc(void);
  void mark_gcroots(void);
public:
  double elapsed_time(void) const
  {
    return rps_elapsed_real_time() - gc_startelapsedtime;
  };
  double process_time(void) const
  {
    return rps_process_cpu_time() - gc_startprocesstime;
  };
  uint64_t nb_roots() const
  {
    return gc_nbroots;
  };
  uint64_t nb_scans() const
  {
    return gc_nbscan;
  };
  uint64_t nb_marks() const
  {
    return gc_nbmark;
  };
  uint64_t nb_deletions() const
  {
    return gc_nbdelete;
  };
  void mark_obj(Rps_ObjectZone* ob);
  void mark_obj(Rps_ObjectRef ob);
  void mark_value(Rps_Value val, unsigned depth=0);
  inline void mark_root_value(Rps_Value val);
  inline void mark_root_objectref(Rps_ObjectRef obr);
  inline void mark_call_stack(Rps_CallFrame*topframe);
};				// end class Rps_GarbageCollector

////////////////////////////////////////////////////// quasi zones

class Rps_TypedZone
{
  friend class Rps_GarbageCollector;
  friend class Rps_CallFrame;
protected:
  const Rps_Type qz_type;
  volatile mutable std::atomic_uint16_t qz_gcinfo;
public:
  Rps_TypedZone(const Rps_Type ty) : qz_type(ty), qz_gcinfo(0) {};
  ~Rps_TypedZone() {};
  Rps_Type stored_type(void) const
  {
    return qz_type;
  };
};


class Rps_QuasiZone : public Rps_TypedZone
{
  friend class Rps_GarbageCollector;
  // we keep each quasi-zone in the qz_zonvec
  static std::recursive_mutex qz_mtx;
  static std::vector<Rps_QuasiZone*> qz_zonvec;
  static uint32_t qz_cnt;
  uint32_t qz_rank;		// the rank in qz_zonvec;
protected:
  inline void* operator new (std::size_t siz, std::nullptr_t);
  inline void* operator new (std::size_t siz, unsigned wordgap);
  static constexpr uint16_t qz_gcmark_bit = 1;
public:
  static void initialize(void);
  static inline Rps_QuasiZone*nth_zone(uint32_t rk);
  static inline Rps_QuasiZone*raw_nth_zone(uint32_t rk, Rps_GarbageCollector&);
  inline bool is_gcmarked(Rps_GarbageCollector&) const;
  inline void set_gcmark(Rps_GarbageCollector&);
  inline void clear_gcmark(Rps_GarbageCollector&);
  static void clear_all_gcmarks(Rps_GarbageCollector&);
  inline static void run_locked_gc(Rps_GarbageCollector&, std::function<void(Rps_GarbageCollector&)>);
  inline static void every_zone(Rps_GarbageCollector&, std::function<void(Rps_GarbageCollector&, Rps_QuasiZone*)>);
  template <typename ZoneClass, class ...Args> static ZoneClass*
  rps_allocate(Args... args)
  {
    return new(nullptr) ZoneClass(args...);
  };
  template <typename ZoneClass> static ZoneClass*
  rps_allocate0(void)
  {
    return new(nullptr) ZoneClass();
  };
  template <typename ZoneClass, typename Arg1Class> static ZoneClass*
  rps_allocate1(Arg1Class arg1)
  {
    return new(nullptr) ZoneClass(arg1);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class> static ZoneClass*
  rps_allocate2(Arg1Class arg1, Arg2Class arg2)
  {
    return new(nullptr) ZoneClass(arg1, arg2);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class> static ZoneClass*
  rps_allocate3(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3)
  {
    return new(nullptr) ZoneClass(arg1, arg2, arg3);
  };
  template <typename ZoneClass, class ...Args> static ZoneClass*
  rps_allocate_with_wordgap(unsigned wordgap, Args... args)
  {
    return new(wordgap) ZoneClass(args...);
  };
  template <typename ZoneClass> static ZoneClass*
  rps_allocate0_with_wordgap(unsigned wordgap)
  {
    return new(wordgap) ZoneClass();
  };
  template <typename ZoneClass, typename Arg1Class> static ZoneClass*
  rps_allocate1_with_wordgap(unsigned wordgap, Arg1Class arg1)
  {
    return new(wordgap) ZoneClass(arg1);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class> static ZoneClass*
  rps_allocate2_with_wordgap(unsigned wordgap, Arg1Class arg1, Arg2Class arg2)
  {
    return new(wordgap) ZoneClass(arg1,arg2);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class> static ZoneClass*
  rps_allocate3_with_wordgap(unsigned wordgap, Arg1Class arg1, Arg2Class arg2, Arg3Class arg3)
  {
    return new(wordgap) ZoneClass(arg1,arg2,arg3);
  };
  void register_in_zonevec(void);
  void unregister_in_zonevec(void);
protected:
  inline Rps_QuasiZone(Rps_Type typ);
  virtual ~Rps_QuasiZone();
  //// the size, in 64 bits words, of the actual quasizone, needed by
  //// the garbage collector.  It should be rounded up, not down (for
  //// potential gaps required by ABI alignment).
  virtual uint32_t wordsize() const =0;
  virtual Rps_Type type() const
  {
    return qz_type;
  };
} __attribute__((aligned(rps_allocation_unit)));
// end class Rps_QuasiZone;



//////////////////////////////////////////////////////////// zone values
class Rps_ZoneValue : public Rps_QuasiZone
{
  friend class Rps_Value;
  friend class Rps_GarbageCollector;
protected:
  inline Rps_ZoneValue(Rps_Type typ);
public:
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const =0;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth) const =0;
  virtual void dump_scan(Rps_Dumper* du, unsigned depth) const =0;
  virtual Json::Value dump_json(Rps_Dumper* du) const =0;
  virtual Rps_HashInt val_hash () const =0;
  virtual void val_output(std::ostream& outs, unsigned depth) const =0;
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
  // we need to serialize some rare modifications (e.g. of metadata in
  // trees). They are rare, so we use a mutex indexed by the last ten
  // bits of hash code...  We are supposing that metadata is
  // read-mostly and only occasionally written...
  static constexpr unsigned lazy_nbmutexes = 1024;
  static std::mutex lazy_mtxarr[lazy_nbmutexes];
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
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const { };
public:
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual uint32_t wordsize() const
  {
    return (sizeof(Rps_String)+_bytsiz+1)/sizeof(void*);
  };
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  virtual void dump_scan(Rps_Dumper*, unsigned) const {};
  virtual Json::Value dump_json(Rps_Dumper*) const;
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
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const { };
  virtual void dump_scan(Rps_Dumper*, unsigned) const {};
  virtual Json::Value dump_json(Rps_Dumper*) const
  {
    return Json::Value(_dval);
  };
public:
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  double dval() const
  {
    return _dval;
  };
  virtual uint32_t wordsize() const
  {// we need to round the 64 bits word size up, hence...
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


/// magic getter C++ functions
typedef Rps_Value rps_magicgetterfun_t(Rps_CallFrame*callerframe, const Rps_Value val, const Rps_ObjectRef obattr);
#define RPS_GETTERFUN_PREFIX "rpsget"
// by convention, the extern "C" getter function inside fictuous attribute
// _3kVHiDzT42h045vHaB would be named rpsget_3kVHiDzT42h045vHaB

// application C++ functions
// the applied closure is in field cfram_clos of the caller frame.
// applying function
typedef Rps_TwoValues rps_applyingfun_t (Rps_CallFrame*callerframe,
    const Rps_Value arg0, const Rps_Value arg1, const Rps_Value arg2,
    const Rps_Value arg3, const std::vector<Rps_Value>* restargs);
#define RPS_APPLYINGFUN_PREFIX "rpsapply"
// by convention, the extern "C" applying function inside the fictuous connective _45vHaB3kVHiDzT42h0
// would be named rpsapply_45vHaB3kVHiDzT42h0
class Rps_Payload;
class Rps_ObjectZone : public Rps_ZoneValue
{
  friend class Rps_Loader;
  friend class Rps_Dumper;
  friend class Rps_Payload;
  friend class Rps_ObjectRef;
  friend class Rps_Value;
  friend Rps_ObjectZone*
  Rps_QuasiZone::rps_allocate<Rps_ObjectZone,Rps_Id,bool>(Rps_Id,bool);
  /// fields
  const Rps_Id ob_oid;
  mutable std::recursive_mutex ob_mtx;
  std::atomic<Rps_ObjectZone*> ob_class;
  std::atomic<Rps_ObjectZone*> ob_space;
  std::atomic<double> ob_mtime;
  std::map<Rps_ObjectRef, Rps_Value> ob_attrs;
  std::vector<Rps_Value> ob_comps;
  std::atomic<Rps_Payload*> ob_payload;
  std::atomic<rps_magicgetterfun_t*> ob_magicgetterfun;
  std::atomic<rps_applyingfun_t*> ob_applyingfun;
  /// constructors
  Rps_ObjectZone(Rps_Id oid, bool dontregister=false);
  Rps_ObjectZone(void);
  ~Rps_ObjectZone();
  static std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher> ob_idmap_;
  static std::map<Rps_Id,Rps_ObjectZone*> ob_idbucketmap_[Rps_Id::maxbuckets];
  static std::recursive_mutex ob_idmtx_;
  static void register_objzone(Rps_ObjectZone*);
  static Rps_Id fresh_random_oid(Rps_ObjectZone*ob =nullptr);
protected:
  void loader_set_class (Rps_Loader*ld, Rps_ObjectZone*obzclass)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(obzclass != nullptr);
    ob_class.store(obzclass);
  };
  void loader_set_mtime (Rps_Loader*ld, double mtim)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(mtim>0.0);
    ob_mtime.store(mtim);
  };
  void loader_set_space (Rps_Loader*ld, Rps_ObjectZone*obzspace)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(obzspace != nullptr);
    ob_space.store(obzspace);
  };
  void loader_put_attr (Rps_Loader*ld, const Rps_ObjectRef keyatob, const Rps_Value atval)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(keyatob);
    RPS_ASSERT(atval);
    ob_attrs.insert({keyatob, atval});
  };
  void loader_put_magicattrgetter(Rps_Loader*ld, rps_magicgetterfun_t*mfun)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(mfun != nullptr);
    // Rps_ObjectRef thisob(this);
    // RPS_INFORMOUT("loader_put_magicattrgetter thisob=" << thisob << ", mfun=" << (void*)mfun);
    ob_magicgetterfun.store(mfun);
  };
  void loader_put_applyingfunction(Rps_Loader*ld, rps_applyingfun_t*afun)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(afun != nullptr);
    // Rps_ObjectRef thisob(this);
    // RPS_INFORMOUT("loader_put_magicattrgetter thisob=" << thisob << ", mfun=" << (void*)mfun);
    ob_applyingfun.store(afun);
  };
  void loader_reserve_comps (Rps_Loader*ld, unsigned nbcomps)
  {
    RPS_ASSERT(ld != nullptr);
    ob_comps.reserve(nbcomps);
  };
  void loader_add_comp (Rps_Loader*ld, const Rps_Value compval)
  {
    RPS_ASSERT(ld != nullptr);
    ob_comps.push_back(compval);
  };
public:
  std::recursive_mutex* objmtxptr(void) const
  {
    return &ob_mtx;
  };
  void touch_now(void) {
    ob_mtime.store(rps_wallclock_real_time());
  };
  std::string string_oid(void) const;
  inline Rps_Payload*get_payload(void) const;
  inline Rps_PayloadClassInfo*get_classinfo_payload(void) const;
  template <class PaylClass> PaylClass* get_dynamic_payload(void) const
  {
    auto payl = get_payload();
    if (!payl)
      return nullptr;
    return dynamic_cast<PaylClass*>(payl);
  }
  inline bool has_erasable_payload(void) const;
  inline Rps_ObjectRef get_class(void) const;
  virtual Rps_ObjectRef compute_class([[maybe_unused]] Rps_CallFrame*stkf) const
  {
    return  get_class();
  };
  inline Rps_ObjectRef get_space(void) const;
  void put_space(Rps_ObjectRef obspace);
  //////////////// attributes
  Rps_Value set_of_attributes(Rps_CallFrame*stkf) const;
  unsigned nb_attributes(Rps_CallFrame*stkf) const;
  Rps_Value get_attr1(Rps_CallFrame*stkf,const Rps_ObjectRef obattr0) const;
  Rps_TwoValues get_attr2(Rps_CallFrame*stkf,const Rps_ObjectRef obattr0, const Rps_ObjectRef obattr1) const;
  // if obaattr is a magic attribute, throw an exception
  void remove_attr(const Rps_ObjectRef obattr);
  // put one, two, three, four attributes in the same object locking
  void put_attr(const Rps_ObjectRef obattr, const Rps_Value valattr);
  void put_attr2( const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                  const Rps_ObjectRef obattr1, const Rps_Value valattr1);
  void put_attr3(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                 const Rps_ObjectRef obattr1, const Rps_Value valattr1,
                 const Rps_ObjectRef obattr2, const Rps_Value valattr2);
  void put_attr4(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                 const Rps_ObjectRef obattr1, const Rps_Value valattr1,
                 const Rps_ObjectRef obattr2, const Rps_Value valattr2,
                 const Rps_ObjectRef obattr3, const Rps_Value valattr3);
  // exchange one, two, three, four attributes in the same object locking with their old attribute value
  void exchange_attr(const Rps_ObjectRef obattr, const Rps_Value valattr, Rps_Value*poldval);
  void exchange_attr2(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                      const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1);
  void exchange_attr3(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                      const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1,
                      const Rps_ObjectRef obattr2, const Rps_Value valattr2, Rps_Value*poldval2);
  void exchange_attr4(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                      const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1,
                      const Rps_ObjectRef obattr2, const Rps_Value valattr2, Rps_Value*poldval2,
                      const Rps_ObjectRef obattr3, const Rps_Value valattr3, Rps_Value*poldval3);
  // put attributes
  void put_attributes(const std::map<Rps_ObjectRef, Rps_Value>& newattrmap);
  void put_attributes(const std::initializer_list<std::pair<Rps_ObjectRef, Rps_Value>>& attril);
  void put_attributes(const std::vector<std::pair<Rps_ObjectRef, Rps_Value>>&attrvec);
  /////////////////////////////////// components
  // append one, two, three, four, more components in the same object locking
  void append_comp1(Rps_Value comp0);
  void append_comp2(Rps_Value comp0, Rps_Value comp1);
  void append_comp3(Rps_Value comp0, Rps_Value comp1, Rps_Value comp2);
  void append_comp4(Rps_Value comp0, Rps_Value comp1, Rps_Value comp2, Rps_Value comp4);
  void append_components(const std::initializer_list<Rps_Value>&compil);
  void append_components(const std::vector<Rps_Value>&compvec);
  unsigned nb_components(Rps_CallFrame*stkf) const;
  Rps_Value component_at (Rps_CallFrame*stkf, int rk, bool dontfail=false) const;
  Rps_Value instance_from_components(Rps_CallFrame*stkf, Rps_ObjectRef obinstclass) const;
  // get atomic fields
  inline double get_mtime(void) const;
  inline rps_applyingfun_t*get_applyingfun(const Rps_ClosureValue&closv) const
  {
    return ob_applyingfun.load();
  };
  inline void clear_payload(void);
  template<class PaylClass>
  PaylClass* put_new_plain_payload(void)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate1<PaylClass>(this);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };
  template<class PaylClass, typename Arg1Class>
  PaylClass* put_new_arg1_payload(Arg1Class arg1)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate2<PaylClass,Arg1Class>(this,arg1);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };
  template<class PaylClass, typename Arg1Class, typename Arg2Class>
  PaylClass* put_new_arg2_payload(Arg1Class arg1, Arg2Class arg2)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate3<PaylClass,Arg1Class,Arg2Class>(this,arg1,arg2);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };
  template<class PaylClass>
  PaylClass* put_new_plain_payload_with_wordgap(unsigned wordgap)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate_with_wordgap<PaylClass>(wordgap,this);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };
  template<class PaylClass, typename Arg1Class>
  PaylClass* put_new_arg1_payload_with_wordgap(unsigned wordgap, Arg1Class arg1)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate_with_wordgap<PaylClass,Arg1Class>(wordgap,this,arg1);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };
  template<class PaylClass, typename Arg1Class, typename Arg2Class>
  PaylClass* put_new_arg2_payload_with_wordgap(unsigned wordgap, Arg1Class arg1, Arg2Class arg2)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate_with_wordgap<PaylClass,Arg1Class,Arg2Class>(wordgap,this,arg1,arg2);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };
  virtual uint32_t wordsize() const
  {
    return sizeof(Rps_ObjectZone)/sizeof(void*);
  };
  static Rps_ObjectZone*make(void);
  static Rps_ObjectZone*make_or_find(Rps_Id);
  static Rps_ObjectZone*make_new(Rps_Id);
  static Rps_ObjectZone*find(Rps_Id);
  static Rps_ObjectZone*make_loaded(Rps_Id, Rps_Loader*);
  const Rps_Id oid() const
  {
    return ob_oid;
  };
  Rps_HashInt obhash() const
  {
    return ob_oid.hash();
  };
  // test if object is a RefPerSys class:
  inline bool is_class(void) const;
  // test if this object is instance of obclass:
  inline bool is_instance_of(Rps_ObjectRef obclass) const;
  // test if this object is a suclass of given obsuperclass:
  inline bool is_subclass_of(Rps_ObjectRef obsuperclass) const;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0) const;
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  void dump_scan_contents(Rps_Dumper*du) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  void dump_json_content(Rps_Dumper*du, Json::Value&)  const;
  virtual Rps_HashInt val_hash (void) const
  {
    return obhash();
  };
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  virtual bool equal(const Rps_ZoneValue&zv) const;
  virtual bool less(const Rps_ZoneValue&zv) const;
  virtual void mark_gc_inside(Rps_GarbageCollector&gc);
  // given a C string which looks like an oid prefix, so starts with
  // an underscore, a digit, and two alphanums, autocomplete that and
  // call a given C++ closure on every possible object ref, till that
  // closure returns true. Return the number of matches, or else 0
  static int autocomplete_oid(const char*prefix, const std::function<bool(const Rps_ObjectZone*)>&stopfun);
};				// end class Rps_ObjectZone

//////////////////////////////////////////////////////////// object payloads

//// signature of extern "C" functions for payload loading; their name starts with rpsldpy_
typedef void rpsldpysig_t(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& hjv, Rps_Id spacid, unsigned lineno);
#define RPS_PAYLOADING_PREFIX "rpsldpy_"



///////////////////////////////////////////// payload superclass
class Rps_Payload : public Rps_QuasiZone
{
  friend class Rps_ObjectZone;
  Rps_ObjectZone* payl_owner;
protected:
  inline Rps_Payload(Rps_Type, Rps_ObjectZone*);
  inline Rps_Payload(Rps_Type, Rps_ObjectRef);
  virtual ~Rps_Payload()
  {
    payl_owner = nullptr;
  };
  void clear_owner(void)
  {
    payl_owner = nullptr;
  };
public:
  Rps_Payload(Rps_Type ty, Rps_ObjectZone*obz, Rps_Loader*ld)
    : Rps_Payload(ty,obz)
  {
    RPS_ASSERT(ld != nullptr);
  };
  virtual const std::string payload_type_name(void) const =0;
  virtual bool is_erasable(void) const
  {
    return true;
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const =0;
  virtual void dump_scan(Rps_Dumper*du) const =0;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const =0;
  Rps_ObjectZone* owner() const
  {
    return payl_owner;
  };
};				// end Rps_Payload







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
  const Rps_ObjectRef*raw_const_data() const
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
  const Rps_ObjectRef at(int ix) const {
    if (ix<0) ix+= cnt();
    if (ix>=0 && ix <(int)cnt())
      return _seqob[ix];
    else
      throw std::range_error("index out of range in objref sequence");
  }
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
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned) const
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
        if (ix+1 >= _seqlen) break;
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
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /// gives the element index, or a negative number if not found
  inline int element_index(const Rps_ObjectRef obelem) const;
  inline bool contains(const Rps_ObjectRef obelem) const { return element_index(obelem) >= 0; };
  unsigned cardinal() const { return cnt(); };
#warning Rps_SetOb very incomplete
};// end of Rps_SetOb

/////////////////////////// tuples of Rps_ObjectRef
unsigned constexpr rps_tuple_k1 = 5939;
unsigned constexpr rps_tuple_k2 = 18917;
unsigned constexpr rps_tuple_k3 = 6571;
class Rps_TupleOb: public Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3>
{
public:
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
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
#warning Rps_TupleOb very incomplete
};// end of Rps_TupleOb


////////////////////////////////////////////////////////////////

/////////////////////////// tree zones, with connective and sons
class Rps_ClosureZone; // closures to be applied
class Rps_InstanceZone; // immutable instances

template<typename RpsTree, Rps_Type treety, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
class Rps_TreeZone : public Rps_LazyHashedZoneValue
{
  friend class Rps_ClosureZone;
  friend class Rps_InstanceZone;
  friend RpsTree*
  Rps_QuasiZone::rps_allocate_with_wordgap<RpsTree,unsigned>(unsigned,unsigned);
  const unsigned _treelen;
  mutable std::atomic<bool> _treetransient;
  mutable std::atomic<bool> _treemetatransient;
  mutable std::atomic<int32_t> _treemetarank;
  mutable std::atomic<Rps_ObjectZone*> _treemetaob;
  Rps_ObjectRef _treeconnob;
  Rps_Value _treesons[RPS_FLEXIBLE_DIM+1];
  Rps_TreeZone(unsigned len, Rps_ObjectRef obr=nullptr)
    : Rps_LazyHashedZoneValue(treety), _treelen(len), 
      _treetransient(false), _treemetatransient(false),
      _treemetarank(0), _treemetaob(nullptr),
      _treeconnob(obr)
  {
    memset (_treesons, 0, sizeof(Rps_Value)*len);
  };
  Rps_Value*raw_data_sons()
  {
    return _treesons;
  };
  const Rps_Value*raw_const_data_sons() const
  {
    return _treesons;
  };
public:
  static unsigned constexpr maxsize
    = std::numeric_limits<unsigned>::max() / 2;
  unsigned cnt() const
  {
    return _treelen;
  };
  Rps_ObjectRef conn() const
  {
    return _treeconnob;
  }
  bool is_transient(void) const { return _treetransient.load(); };
  bool is_metatransient(void) const { return _treemetatransient.load(); };
  int32_t metarank(void) const { return _treemetarank.load(); };
  Rps_ObjectZone* metaobject(void) const { return _treemetaob.load(); };
  std::pair<Rps_ObjectZone*,int32_t> const get_metadata(void) const;
  std::pair<Rps_ObjectZone*,int32_t> swap_metadata(Rps_ObjectRef obr, int32_t num=0, bool transient=false);
  void put_metadata(Rps_ObjectRef obr, int32_t num=0, bool transient=false);
  void put_transient_metadata(Rps_ObjectRef obr, int32_t num=0) { put_metadata(obr, num, true); };
  void put_persistent_metadata(Rps_ObjectRef obr, int32_t num=0) { put_metadata(obr, num, false); };
  typedef const Rps_Value*iterator_t;
  iterator_t begin() const
  {
    return const_cast<const Rps_Value*>(_treesons);
  };
  iterator_t end() const
  {
    return const_cast<const Rps_Value*>(_treesons) + _treelen;
  };
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this) + _treelen * sizeof(_treesons[0])) / sizeof(void*);
  };
  inline const Rps_Value at(int rk, bool dontfail=false) const;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth) const
  {
    gc.mark_obj(_treeconnob);
    for (auto vson: *this)
      if (vson)
        gc.mark_value(vson, depth+1);
  };
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    Rps_HashInt h0= 3317+(k1&0xff)+_treeconnob->obhash()*k3, h1= 211*_treelen;
    for (unsigned ix=0; ix<_treelen; ix += 2)
      {
        auto curson = _treesons[ix];
        if (RPS_LIKELY(!curson.is_empty()))
          h0 = (h0 * k1) ^ (curson.valhash() * k2 + ix);
        if (ix+1 >= _treelen)
          break;
        auto nextson = _treesons[ix+1];
        if (RPS_LIKELY(!nextson.is_empty()))
          h1 = (h1 * k3) ^ (nextson.valhash() * k4 - (h0&0xfff));
      };
    Rps_HashInt h = 53*h0 + 17*h1;
    if (RPS_UNLIKELY(h == 0))
      h = ((h0 & 0xfffff)
           ^ (h1 & 0xfffff)) + (k3/128
                                + (_treeconnob->obhash()%65353) + (_treelen & 0xff) + 13);
    RPS_ASSERT(h != 0);
    return h;
  };
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == treety)
      {
        auto oth = reinterpret_cast<const RpsTree*>(&zv);
        if (RPS_LIKELY(reinterpret_cast<const Rps_LazyHashedZoneValue*>(this)->val_hash()
                       != reinterpret_cast<const Rps_LazyHashedZoneValue*>(oth)->val_hash()))
          return false;
        auto curcnt = cnt();
        if (RPS_LIKELY(oth->cnt() != curcnt))
          return false;
        if (RPS_LIKELY(_treeconnob != oth->_treeconnob))
          return false;
        for (unsigned ix = 0; ix < curcnt; ix++)
          if (_treesons[ix] != oth->_treesons[ix])
            return false;
        return true;
      }
    return false;
  }
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == treety)
      {
        auto oth = reinterpret_cast<const RpsTree*>(&zv);
        if (_treeconnob < oth->_treeconnob)
          return true;
        if (_treeconnob > oth->_treeconnob)
          return false;
        RPS_ASSERT(_treeconnob == oth->_treeconnob);
        return std::lexicographical_compare(begin(), end(),
                                            oth->begin(), oth->end());
      }
    return false;
  };
};    // end of template Rps_TreeZone


//////////////////////////////////////////////// closures

unsigned constexpr rps_closure_k1 = 8161;
unsigned constexpr rps_closure_k2 = 9151;
unsigned constexpr rps_closure_k3 = 10151;
unsigned constexpr rps_closure_k4 = 13171;
struct Rps_ClosureTag {};
class Rps_ClosureZone :
  public Rps_TreeZone<Rps_ClosureZone, Rps_Type::Closure,
  rps_closure_k1, rps_closure_k2, rps_closure_k3, rps_closure_k4>
{
public:
  typedef  Rps_TreeZone<Rps_ClosureZone, Rps_Type::Closure,
           rps_closure_k1, rps_closure_k2, rps_closure_k3, rps_closure_k4> parenttree_t;
  friend parenttree_t;
protected:
  Rps_ClosureZone(unsigned len, Rps_ObjectRef connob, Rps_ClosureTag) :
    parenttree_t(len, connob)
  {
  };
  friend Rps_ClosureZone*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_ClosureZone,unsigned,Rps_ObjectRef,Rps_ClosureTag>(unsigned,unsigned,Rps_ObjectRef,Rps_ClosureTag);
public:
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /// make a closure with given connective and values
  static Rps_ClosureZone* make(Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil);
  static Rps_ClosureZone* make(Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec);
};    // end Rps_ClosureZone

class Rps_ClosureValue : public Rps_Value
{
public:
  inline Rps_ClosureValue() : Rps_Value() {};
  inline Rps_ClosureValue(std::nullptr_t) : Rps_Value(nullptr) {};
  // related to Rps_ClosureZone::make
  inline Rps_ClosureValue(const Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil);
  inline Rps_ClosureValue(const Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec);
  // "dynamic" casting
  inline Rps_ClosureValue(Rps_Value val);
// get the connective
  inline Rps_ObjectRef connob(void) const;
// clear the closure
  inline Rps_ClosureValue& operator = (std::nullptr_t)
  {
    clear();
    return *this;
  };
  inline  Rps_ClosureZone*operator -> (void) const;
  // application
  inline Rps_TwoValues apply0(Rps_CallFrame*callerframe) const;
  inline Rps_TwoValues apply1(Rps_CallFrame*callerframe, const Rps_Value arg0) const;
  inline Rps_TwoValues apply2(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1) const;
  inline Rps_TwoValues apply3(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2) const;
  inline Rps_TwoValues apply4(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3) const;
  inline Rps_TwoValues apply5(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4) const;
  inline Rps_TwoValues apply6(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5) const;
  inline Rps_TwoValues apply7(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5, const Rps_Value arg6) const;
  inline Rps_TwoValues apply8(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5, const Rps_Value arg6,
                              const Rps_Value arg7) const;
  inline Rps_TwoValues apply9(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5, const Rps_Value arg6,
                              const Rps_Value arg7, const Rps_Value arg8) const;
  Rps_TwoValues apply_vect(Rps_CallFrame*callerframe, const std::vector<Rps_Value>& argvec) const;
  Rps_TwoValues apply_ilist(Rps_CallFrame*callerframe, const std::initializer_list<Rps_Value>& argil) const;
};    // end Rps_ClosureValue



//////////////////////////////////////////////// instances

unsigned constexpr rps_instance_k1 = 8161;
unsigned constexpr rps_instance_k2 = 9151;
unsigned constexpr rps_instance_k3 = 10151;
unsigned constexpr rps_instance_k4 = 13171;
struct Rps_InstanceTag {};
class Rps_InstanceZone :
  public Rps_TreeZone<Rps_InstanceZone, Rps_Type::Instance,
  rps_instance_k1, rps_instance_k2, rps_instance_k3, rps_instance_k4>
{
public:
  typedef  Rps_TreeZone<Rps_InstanceZone, Rps_Type::Instance,
           rps_instance_k1, rps_instance_k2, rps_instance_k3, rps_instance_k4> parenttree_t;
  friend parenttree_t;
protected:
  Rps_InstanceZone(unsigned len, Rps_ObjectRef connob, Rps_InstanceTag) :
    parenttree_t(len, connob)
  {
  };
  friend Rps_InstanceZone*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_InstanceZone,unsigned,Rps_ObjectRef,Rps_InstanceTag>(unsigned,unsigned,Rps_ObjectRef,Rps_InstanceTag);
public:
  const Rps_Value* const_sons() const { return raw_const_data_sons(); };
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  Rps_ObjectRef get_class(void) const { return conn(); };
  const Rps_SetOb* set_attributes(void) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /// get the set of attributes in a class
  static const Rps_SetOb* class_attrset(Rps_ObjectRef obclass);
  /// make a instance with given class and components and no attributes
  static Rps_InstanceZone* load_from_json(Rps_Loader*ld, const Json::Value& jv);
  /// later fill such an empty instance
  void fill_loaded_instance_from_json(Rps_Loader*ld, Rps_ObjectRef obclass, const Json::Value& jv);
  // when loading, we could be able to allocate, but not to fill the
  // instance now; this happens when the class is not filled
  static Rps_InstanceZone* make_incomplete_loaded(Rps_Loader*ld, Rps_ObjectRef classob, unsigned siz);
  static Rps_InstanceZone* make_from_components(Rps_ObjectRef classob, const std::initializer_list<Rps_Value>& valil);
  static Rps_InstanceZone* make_from_components(Rps_ObjectRef classob, const std::vector<Rps_Value>& valvec);
  /// make an instance from both attributes and components
  static Rps_InstanceZone* make_from_attributes_components(Rps_ObjectRef classob,
							   const std::initializer_list<Rps_Value>& valil,const std::initializer_list<std::pair<Rps_ObjectRef,Rps_Value>>&attril);
  static Rps_InstanceZone* make_from_attributes_components(Rps_ObjectRef classob,
							   const std::vector<Rps_Value>& valvec,
							   const std::map<Rps_ObjectRef,Rps_Value>& attrmap);
  /// make an instance from attributes
  static Rps_InstanceZone* make_from_attributes(Rps_ObjectRef classob,
						const std::initializer_list<std::pair<Rps_ObjectRef,Rps_Value>>&attril);
  static Rps_InstanceZone* make_from_attributes(Rps_ObjectRef classob,
						const std::map<Rps_ObjectRef,Rps_Value>& attrmap);
};    // end class Rps_InstanceZone


class Rps_InstanceValue : public Rps_Value
{
public:
  inline Rps_InstanceValue() : Rps_Value() {};
  inline Rps_InstanceValue(std::nullptr_t) : Rps_Value(nullptr) {};
  // related to Rps_InstanceZone::make
  inline Rps_InstanceValue(const Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil);
  inline Rps_InstanceValue(const Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec);
  // "dynamic" casting
  inline Rps_InstanceValue(const Rps_Value val);
// get the class (stored in connective)
  inline Rps_ObjectRef get_class(void) const;
// clear the instance
  inline Rps_InstanceValue& operator = (std::nullptr_t)
  {
    clear();
    return *this;
  };
  inline  Rps_InstanceZone*operator -> (void) const;
}; // end class Rps_InstanceValue


//////////////////////////////////////////////// Json values

class Rps_JsonZone : public Rps_LazyHashedZoneValue
{
  friend Rps_JsonZone*
  Rps_QuasiZone::rps_allocate<Rps_JsonZone,const Json::Value&>(const Json::Value&);
  const Json::Value _jsonval;
protected:
  inline Rps_JsonZone(const Json::Value&jv);
  virtual Rps_HashInt compute_hash(void) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const { };
  virtual void dump_scan(Rps_Dumper*, unsigned) const {};
  virtual Json::Value dump_json(Rps_Dumper*) const;
public:
  const Json::Value& json() const { return _jsonval; };
  const Json::Value const_json() const { return _jsonval; };
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void val_output(std::ostream& outs, unsigned depth) const;
  virtual bool equal(const Rps_ZoneValue&zv) const;
  virtual bool less(const Rps_ZoneValue&zv) const;
  static Rps_JsonZone* load_from_json(Rps_Loader*ld, const Json::Value& jv);
  static Rps_JsonZone*make(const Json::Value& jv);
};				// end class Rps_JsonZone


////////////////////////////////////////////////////////////////


class Rps_QtPtrZone : public Rps_LazyHashedZoneValue
{
private:
  /// we count the number of Rps_QtPtrZone to give a unique rank to
  /// each of them; that rank is used for hashing and compare.
  static std::atomic<unsigned> qtptr_count;
  friend Rps_QtPtrZone*
  Rps_QuasiZone::rps_allocate1<Rps_QtPtrZone, const QPointer<QObject>>
                 (const QPointer<QObject>);

  const QPointer<QObject> _qptr_val;
  const unsigned _qptr_rank; // the unique rank

protected:
  inline Rps_QtPtrZone(const QPointer<QObject> qptrval);

  virtual Rps_HashInt compute_hash(void) const;

  virtual Rps_ObjectRef compute_class(Rps_CallFrame* stkf) const;

  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const
  { }

  virtual void dump_scan(Rps_Dumper*, unsigned) const 
  { };

  virtual Json::Value dump_json(Rps_Dumper*) const
  {
      return Json::Value::null;
  }

public:
  const QPointer<QObject> qptr() const
  {
      return _qptr_val;
  }
  unsigned rank() const { return _qptr_rank; };
  ///
  virtual std::uint32_t wordsize() const
  {
    // as usual, we round up the 64 bits word size ....
      return (sizeof (*this) + sizeof (void*) - 1) / sizeof (void*);
  }

  virtual void val_output(std::ostream& ostr, unsigned depth) const;

  virtual bool equal(const Rps_ZoneValue& zv) const;

  virtual bool less(const Rps_ZoneValue& zv) const;

  static Rps_QtPtrZone* make(const QPointer<QObject> qptrval);
};


////////////////////////////////////////////////////////////////

class Rps_CallFrame : public Rps_TypedZone
{
  const unsigned cfram_size;
  Rps_ObjectRef cfram_descr;
  Rps_CallFrame* cfram_prev;
  Rps_Value cfram_state;
  Rps_ClosureValue cfram_clos; // the invoking closure, if any
  std::function<void(Rps_GarbageCollector*)> cfram_marker;
  void* cfram_data[RPS_FLEXIBLE_DIM]; // a flexible array member in disguise
public:
  Rps_CallFrame(unsigned size, Rps_ObjectRef obdescr=nullptr, Rps_CallFrame*prev=nullptr)
    : Rps_TypedZone(Rps_Type::CallFrame),
      cfram_size(size),
      cfram_descr(obdescr),
      cfram_prev(prev),
      cfram_state(nullptr),
      cfram_clos(nullptr),
      cfram_marker(),
      cfram_data()
  {
    // we want flexible array members, see
    // https://en.wikipedia.org/wiki/Flexible_array_member but C++17
    // don't have them ....
    // see https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    if (size>0)
      memset((void*)&cfram_data, 0, size*sizeof(void*));
#pragma GCC diagnostic pop
  };
  ~Rps_CallFrame()
  {
    cfram_descr = nullptr;
    cfram_state = nullptr;
    cfram_prev = nullptr;
    cfram_clos = nullptr;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    // technically this is an undefined behavior. Pragmatically it should work as we want.
    if (cfram_size > 0)
      __builtin_memset((void*)&cfram_data, 0, cfram_size*sizeof(void*));
#pragma GCC diagnostic pop
  };
  void gc_mark_frame(Rps_GarbageCollector* gc);
  void set_closure(Rps_ClosureValue clos)
  {
    RPS_ASSERT(!clos.is_empty() && clos.is_closure());
    cfram_clos = clos;
  };
  void clear_closure(void)
  {
    cfram_clos = nullptr;
  };
  Rps_CallFrame*previous_call_frame(void) const
  {
    return cfram_prev;
  };
  void set_additional_gc_marker(const std::function<void(Rps_GarbageCollector*)>& gcmarkfun)
  {
    cfram_marker = gcmarkfun;
  };
  void clear_additional_gc_marker(void)
  {
    cfram_marker = nullptr;
  };
};				// end class Rps_CallFrame



#define RPS_LOCALFRAME_ATBIS(Lin,Descr,Prev,...)	\
  class Rps_FrameAt##Lin : public Rps_CallFrame {	\
  struct FrameData##Lin {__VA_ARGS__; };		\
public:							\
 Rps_FrameAt##Lin(Rps_ObjectRef obd##Lin,		\
		  Rps_CallFrame* prev##Lin) :		\
       Rps_CallFrame( (sizeof(FrameData##Lin)		\
			     + sizeof(void*) - 1)      	\
			    / sizeof(void*),		\
			    obd##Lin, prev##Lin)	\
    { };						\
  __VA_ARGS__;						\
  };							\
  Rps_FrameAt##Lin _((Descr),(Prev))

#define RPS_LOCALFRAME_AT(Lin,Descr,Prev,...) \
  RPS_LOCALFRAME_ATBIS(Lin,Descr,Prev,__VA_ARGS__)
#define RPS_LOCALFRAME(Descr,Prev,...) \
  RPS_LOCALFRAME_AT(__LINE__,Descr,Prev,__VA_ARGS__)

#define RPS_LOCALRETURN(Val) do {			\
  RPS_ASSERT(_.previous_call_frame() != nullptr);	\
  _.previous_call_frame()->clear_closure();		\
  return (Val); } while(0)

#define RPS_LOCALRETURNTWO(MainVal,XtraVal) do {       	\
  RPS_ASSERT(_.previous_call_frame() != nullptr);	\
  _.previous_call_frame()->clear_closure();		\
  return Rps_TwoValues((MainVal),(XtraVal)); } while(0)





////////////////////////////////////////////////////////////////
////// class information payload - for PaylClassInfo, objects of class
////// `class` _41OFI3r0S1t03qdB2E

extern "C" rpsldpysig_t rpsldpy_class;
class Rps_PayloadClassInfo : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend class Rps_Value;
  friend Rps_PayloadClassInfo*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadClassInfo,Rps_ObjectZone*>(Rps_ObjectZone*);
  // the superclass:
  Rps_ObjectRef pclass_super;
  // the dictionnary from selector to own methods
  std::map<Rps_ObjectRef,Rps_ClosureValue> pclass_methdict;
  // the optional name (a symbol)
  Rps_ObjectRef pclass_symbname;
  // for immutable instances, the set of attributes; it should not be
  // nil for them.  See
  // https://gitlab.com/bstarynk/refpersys/-/wikis/Immutable-instances-in-RefPerSys
  mutable std::atomic<const Rps_SetOb*> pclass_attrset;
  virtual ~Rps_PayloadClassInfo()
  {
    pclass_super = nullptr;
    pclass_methdict.clear();
    pclass_symbname = nullptr;
    pclass_attrset.store(nullptr);
  };
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadClassInfo(Rps_ObjectZone*owner);
  Rps_PayloadClassInfo(Rps_ObjectRef obr) :
    Rps_PayloadClassInfo(obr?obr.optr():nullptr) {};
public:
  const Rps_SetOb* attributes_set() const { return pclass_attrset.load(); };
  virtual const std::string payload_type_name(void) const
  {
    return "class";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
  inline Rps_PayloadClassInfo(Rps_ObjectZone*owner, Rps_Loader*ld);
  Rps_ObjectRef superclass() const
  {
    return pclass_super;
  };
  Rps_ObjectRef symbname() const
  {
    return pclass_symbname;
  };
  void put_superclass(Rps_ObjectRef obr)
  {
    pclass_super = obr;
  };
  inline void clear_symbname(void)
  {
    pclass_symbname = nullptr;
  };
  std::string class_name_str(void) const;
  void put_symbname(Rps_ObjectRef obr);
  const Rps_SetOb*class_attrset(void) const { return  pclass_attrset.load(); }
  void loader_put_symbname(Rps_ObjectRef obr, Rps_Loader*ld);
  void loader_put_attrset(const Rps_SetOb*setob, Rps_Loader*ld);			  
  Rps_ClosureValue get_own_method(Rps_ObjectRef obsel) const
  {
    if (!obsel)
      return Rps_ClosureValue(nullptr);
    auto it = pclass_methdict.find(obsel);
    if (it != pclass_methdict.end())
      return it->second;
    return Rps_ClosureValue(nullptr);
  };
  void put_own_method(Rps_ObjectRef obsel, Rps_ClosureValue clov)
  {
    if (obsel && clov && clov.is_closure())
      pclass_methdict.insert({obsel,clov});
  };
  void remove_own_method(Rps_ObjectRef obsel)
  {
    if (obsel)
      pclass_methdict.erase(obsel);
  };
};				// end Rps_PayloadClassInfo



////////////////////////////////////////////////////////////////
////// mutable set of objects payload - for PaylSetOb, objects of
////// class `mutable_set` _0J1C39JoZiv03qA2HA
extern "C" rpsldpysig_t rpsldpy_setob;
class Rps_PayloadSetOb : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadSetOb*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadSetOb,Rps_ObjectZone*>(Rps_ObjectZone*);
  std::set<Rps_ObjectRef> psetob;
  inline Rps_PayloadSetOb(Rps_ObjectZone*owner);
  Rps_PayloadSetOb(Rps_ObjectRef obr) :
    Rps_PayloadSetOb(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadSetOb()
  {
    psetob.clear();
  };
protected:
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "setob";
  };
  inline Rps_PayloadSetOb(Rps_ObjectZone*obz, Rps_Loader*ld);
  bool contains(const Rps_ObjectZone* obelem) const
  {
    return obelem && psetob.find(Rps_ObjectRef(obelem)) != psetob.end();
  };
  bool contains(const Rps_ObjectRef obr) const
  {
    return obr && psetob.find(obr) != psetob.end();
  };
  unsigned cardinal(void) const
  {
    return (unsigned) psetob.size();
  };
  void add(const Rps_ObjectZone* obelem)
  {
    if (obelem)
      psetob.insert(Rps_ObjectRef(obelem));
  };
  void add (const Rps_ObjectRef obrelem)
  {
    if (!obrelem.is_empty())
      psetob.insert(obrelem);
  };
  void remove(const Rps_ObjectZone* obelem)
  {
    if (obelem) psetob.erase(Rps_ObjectRef(obelem));
  };
  void remove (const Rps_ObjectRef obrelem)
  {
    if (obrelem) psetob.erase(obrelem);
  };
  Rps_SetValue to_set() const
  {
    return Rps_SetValue(psetob);
  };
  Rps_TupleValue to_tuple() const
  {
    std::vector<Rps_ObjectRef> vecob;
    vecob.reserve(cardinal());
    for (auto obrelem: psetob)
      {
        RPS_ASSERT(obrelem);
        vecob.push_back(obrelem);
      };
    return Rps_TupleValue(vecob);
  };
};				// end Rps_PayloadSetOb


////////////////////////////////////////////////////////////////
////// mutable vector of objects payload - for PaylVectOb and objects
////// of class `mutable_vector _8YknAApDQiF04BDe3W
extern "C" rpsldpysig_t rpsldpy_vectob;
class Rps_PayloadVectOb : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadVectOb*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadVectOb,Rps_ObjectZone*>(Rps_ObjectZone*);
  std::vector<Rps_ObjectRef> pvectob;
  inline Rps_PayloadVectOb(Rps_ObjectZone*owner);
  Rps_PayloadVectOb(Rps_ObjectRef obr) :
    Rps_PayloadVectOb(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadVectOb()
  {
    pvectob.clear();
  };
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "vectob";
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  inline Rps_PayloadVectOb(Rps_ObjectZone*obz, Rps_Loader*ld);
  unsigned size(void) const
  {
    return (unsigned) pvectob.size();
  };
  Rps_ObjectRef at(int ix) const
  {
    if (ix<0)
      ix += size();
    if (ix>=0 && ix<(int)size())
      return pvectob[ix];
    throw std::out_of_range("Rps_PayloadVectOb bad index");
  };
  void reserve(unsigned rsiz)
  {
    pvectob.reserve(rsiz);
  };
  void reserve_more(unsigned gap)
  {
    pvectob.reserve(size()+gap);
  };
  void push_back(const Rps_ObjectZone* obcomp)
  {
    if (obcomp)
      pvectob.push_back(Rps_ObjectRef(obcomp));
  };
  void push_back (const Rps_ObjectRef obrcomp)
  {
    if (obrcomp)
      pvectob.push_back(obrcomp);
  };
  Rps_TupleValue to_tuple() const
  {
    return Rps_TupleValue(pvectob);
  };
};				// end Rps_PayloadVectOb



////////////////////////////////////////////////////////////////
////// mutable space payload, objects of class `space`
////// _2i66FFjmS7n03HNNBx
extern "C" rpsldpysig_t rpsldpy_space;
class Rps_PayloadSpace : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_space;
  friend Rps_PayloadSpace*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadSpace,Rps_ObjectZone*>(Rps_ObjectZone*);
protected:
  inline Rps_PayloadSpace(Rps_ObjectZone*owner);
  Rps_PayloadSpace(Rps_ObjectRef obr) :
    Rps_PayloadSpace(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadSpace()
  {
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const
  {
    return false;
  };
public:
  virtual const std::string payload_type_name(void) const
  {
    return "space";
  };
  inline Rps_PayloadSpace(Rps_ObjectZone*obz, Rps_Loader*ld);
};				// end Rps_PayloadSpace


////////////////////////////////////////////////////////////////
////// symbol payload
extern "C" void rps_initialize_symbols_after_loading (Rps_Loader*ld);
extern "C" rpsldpysig_t rpsldpy_symbol;
class Rps_PayloadSymbol : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_symbol;
  friend void rps_initialize_symbols_after_loading(Rps_Loader*ld);
  friend Rps_PayloadSymbol*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadSymbol, Rps_ObjectZone*>(Rps_ObjectZone*);
  std::string symb_name;
  std::atomic<const void*> symb_data;
  std::atomic<bool> symb_is_weak;
  static std::recursive_mutex symb_tablemtx;
  static std::map<std::string,Rps_PayloadSymbol*> symb_table;
  static std::unordered_map<std::string,Rps_ObjectRef*> symb_hardcoded_hashtable;
protected:
  Rps_PayloadSymbol(Rps_ObjectZone*owner);
  Rps_PayloadSymbol(Rps_ObjectRef obr) :
    Rps_PayloadSymbol(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadSymbol()
  {
    std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
    if (!symb_name.empty())
      symb_table.erase(symb_name);
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const
  {
    return symb_is_weak.load();
  };
public:
  virtual const std::string payload_type_name(void) const
  {
    return "symbol";
  };
  static void gc_mark_strong_symbols(Rps_GarbageCollector*gc);
  void load_register_name(const char*name, Rps_Loader*ld,bool weak=false);
  void load_register_name(const std::string& str, Rps_Loader*ld, bool weak=false)
  {
    load_register_name (str.c_str(), ld, weak);
  };
  bool is_weak(void) const
  {
    return symb_is_weak.load();
  };
  bool symbol_is_weak(void) const
  {
    return symb_is_weak.load();
  };
  void set_weak(bool f)
  {
    symb_is_weak.store(f);
  };
  Rps_Value symbol_value(void) const
  {
    return Rps_Value(symb_data.load(), this);
  };
  void symbol_put_value(Rps_Value v)
  {
    symb_data.store(v.data_for_symbol(this));
  };
  const std::string& symbol_name(void) const
  {
    return symb_name;
  };
  static bool valid_name(const char*str);
  static bool valid_name(const std::string str)
  {
    return valid_name(str.c_str());
  };
  static Rps_ObjectRef find_named_object(const std::string&str)
  {
    std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
    auto it = symb_table.find(str);
    if (it != symb_table.end())
      {
        auto symb = it->second;
        if (symb)
          return symb->owner();
      };
    return nullptr;
  };
  static bool register_name(std::string name, Rps_ObjectRef obj, bool weak);
  static bool register_strong_name(std::string name, Rps_ObjectRef obj)
  {
    return register_name(name, obj, false);
  }
  static bool register_weak_name(std::string name, Rps_ObjectRef obj)
  {
    return register_name(name,obj, true);
  }
  static Rps_PayloadSymbol* find_named_payload(const std::string&str)
  {
    std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
    auto it = symb_table.find(str);
    if (it != symb_table.end())
      {
        auto symb = it->second;
        if (symb)
          return symb;
      };
    return nullptr;
  };
  static const Rps_SetValue set_of_all_symbols(void);
  static bool forget_name(std::string name);
  static bool forget_object(Rps_ObjectRef obj);
  // given a C string which looks like a C identifier starting with a letter,
  // autocomplete that and call a given C++ closure on every possible object ref and name, till that
  // closure returns true. Return the number of matches, or else 0
  static int autocomplete_name(const char*prefix, const std::function<bool(const Rps_ObjectZone*,const std::string&)>&stopfun);
};				// end Rps_PayloadSymbol



//////////////// template for payload holding Qt objects
template <class QtClass> class Rps_PayloadQt : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadQt*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadQt,Rps_ObjectZone*>(Rps_ObjectZone*);
protected:
  QPointer<QtClass> _qtptr;
  Rps_PayloadQt(Rps_ObjectZone*owner)
    :  Rps_Payload(Rps_Type::PaylQt, owner), _qtptr(nullptr) {};
  virtual ~Rps_PayloadQt();
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const {
    if (_qtptr)
      _qtptr->gc_mark(gc);
  };
  virtual void dump_scan(Rps_Dumper*) const {};
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const {};
  virtual bool is_erasable(void) const
  {
    return true;
  };
public:
  inline Rps_PayloadQt& set_qtptr(QtClass* qptr);
  inline Rps_PayloadQt& clear_qtptr(QtClass* qptr);
  inline QtClass* qtptr(void) const;
  QtClass& operator -> (void) const
  {
    if (owner()) {
      if (_qtptr)
      return *_qtptr;
      else
	throw  RPS_RUNTIME_ERROR_OUT("missing qtptr:" << payload_type_name());
    }
    else throw RPS_RUNTIME_ERROR_OUT("unowned Qt payload:" << payload_type_name());
  };
  QtClass& operator * (void) const
  {
    if (owner()) {
      if (_qtptr)
	return *_qtptr;
      else
	throw  RPS_RUNTIME_ERROR_OUT("missing qtptr:" << payload_type_name());
    } throw RPS_RUNTIME_ERROR_OUT("unowned Qt payload:" << payload_type_name());
  }
  virtual const std::string payload_type_name(void) const;
};				// end of template Rps_PayloadQt

////////////////////////////////////////////////////////////////

#define RPS_MANIFEST_JSON "rps_manifest.json"
#define RPS_USERPREFERENCE_JSON ".refpersys.json"
#define RPS_QTSETTINGS_BASEPATH ".qt-refpersys.ini"

//// global roots for garbage collection and persistence
/// the called function cannot add, remove or query the global root set
extern "C" void rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun);
extern "C" void rps_add_root_object (const Rps_ObjectRef);
extern "C" bool rps_remove_root_object (const Rps_ObjectRef);
extern "C" bool rps_is_root_object (const Rps_ObjectRef);
extern "C" std::set<Rps_ObjectRef> rps_set_root_objects(void);
extern "C" unsigned rps_nb_root_objects(void);
extern "C" void rps_initialize_roots_after_loading (Rps_Loader*ld);
extern "C" void rps_initialize_symbols_after_loading (Rps_Loader*ld);

extern "C" unsigned rps_hardcoded_number_of_roots(void);
extern "C" unsigned rps_hardcoded_number_of_symbols(void);
extern "C" unsigned rps_hardcoded_number_of_constants(void);

////////////////

extern "C" void rps_run_application (int& argc, char**argv); // in appli_qrps.cc

extern "C" void rps_garbcoll_application(Rps_GarbageCollector&gc);

extern "C" void rps_dump_into (const std::string dirpath = "."); // in store_rps.cc

// scan a code address, e.g. a C function pointer whose address is inside some dlopen-ed plugin
extern "C" void rps_dump_scan_code_addr(Rps_Dumper*, const void*);
// scan an object
extern "C" void rps_dump_scan_object(Rps_Dumper*, const Rps_ObjectRef obr);
// scan in a space a component object
extern "C" void rps_dump_scan_space_component(Rps_Dumper*, Rps_ObjectRef obrspace, Rps_ObjectRef obrcomp);
// scan a value
extern "C" void rps_dump_scan_value(Rps_Dumper*, const Rps_Value val, unsigned depth);
extern "C" Json::Value rps_dump_json_value(Rps_Dumper*, const Rps_Value val);
extern "C" Json::Value rps_dump_json_objectref(Rps_Dumper*, const Rps_ObjectRef obr);

// is an object dumpable?
extern "C" bool rps_is_dumpable_objref(Rps_Dumper*, const Rps_ObjectRef obr);

// is an object dumpable as attribute in another object?
extern "C" bool rps_is_dumpable_objattr(Rps_Dumper*, const Rps_ObjectRef obr);

// is a value dumpable?
extern "C" bool rps_is_dumpable_value(Rps_Dumper*, const Rps_Value val);


extern "C" void rps_load_from (const std::string& dirpath); // in store_rps.cc
 
extern "C" void rps_load_add_todo(Rps_Loader*,const std::function<void(Rps_Loader*)>& todofun);

extern "C" void rps_print_types_info (void);
/// C++ code can refer to root objects
#define RPS_ROOT_OB(Oid) rps_rootob##Oid

// C++ code can refer to named symbols
#define RPS_SYMB_OB(Nam) rps_symbob_##Nam


/// each root object is also a public variable
#define RPS_INSTALL_ROOT_OB(Oid) extern "C" Rps_ObjectRef RPS_ROOT_OB(Oid);
#include "generated/rps-roots.hh"

// each named global symbol is also a public variable
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Nam) extern "C" Rps_ObjectRef RPS_SYMB_OB(Nam);
#include "generated/rps-names.hh"


#include "inline_rps.hh"



#endif /*REFPERSYS_INCLUDED*/
// end of file refpersys.hh */

/****************************************************************
 * file ide-refpersys.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its only C++ header file for the temporary IDE
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      team@refpersys.org
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



#ifndef IDEREFPERSYS_INCLUDED
#define IDEREFPERSYS_INCLUDED

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
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <functional>

// FLTKL 1.3 - see www.fltk.org
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Buffer.H>
////////////////

/// HJSON-CPP, installed from https://github.com/hjson/hjson-cpp
#include "hjson/hjson.h"

#include "refpersys.hh"

typedef std::uint32_t rps_gapnum_t;
class Rps_Gap {
  rps_gapnum_t _gap;
public:
  Rps_Gap(std::uint64_t g=0) : _gap((rps_gapnum_t)g) {};
  Rps_Gap(rps_gapnum_t g=0) : _gap(g) {};
  rps_gapnum_t get() const { return _gap; };
  operator rps_gapnum_t (void) const { return get(); };
  Rps_Gap& put(rps_gapnum_t n) { _gap = n; return *this; };
  ~Rps_Gap() {};
  // rule of five:
  Rps_Gap(const Rps_Gap& src) : Rps_Gap(src._gap) {};
  Rps_Gap(Rps_Gap&& oth) noexcept : _gap(oth._gap) {};
  Rps_Gap& operator= (const Rps_Gap& oth) { _gap = oth._gap; return *this; };
  Rps_Gap&operator= (Rps_Gap&&oth) noexcept {
    std::swap (_gap, oth._gap);
    return *this;
  };
};				// end Rps_Gap
     

struct rps_plain_tag {}; // for new below


// quasi-value zone
class Rps_QuasiValueZ {
  Rps_Type _rps_type;
  volatile int16_t _rps_gcflags;
  uint32_t _rps_qvrank; // rank inside _rps_ptrvector
  static std::vector<Rps_QuasiValueZ*> _rps_ptrvector;
  static std::mutex _rps_mtxvector;
  Rps_QuasiValueZ(Rps_Type ty, uint32_t rank)
    : _rps_type(ty), _rps_gcflags(0), _rps_qvrank(rank) {};
protected:
  Rps_QuasiValueZ(Rps_Type ty);
  virtual ~Rps_QuasiValueZ();
  virtual Rps_Type type() const { return _rps_type; };
  int16_t gc_flags() const { return _rps_gcflags; };
  void* operator new (size_t siz) {
    return ::operator new (siz); };
  void* operator new (size_t siz, rps_plain_tag) {
    return ::operator new (siz); };
  void* operator new (size_t siz, Rps_Gap gap) {
    return ::operator new(siz+gap.get());    
  };
  void operator delete(void*p) noexcept { ::operator delete(p); };
};			       // end class Rps_QuasiValueZ

class Rps_Displayer {
  Fl_Text_Buffer* rps_disp_buffer;
  int rps_disp_maxdepth;
};			       // end Rps_Displayer

class Rps_GarbageCollector;

class Rps_ObjectZ;
class Rps_ValueZ : public Rps_QuasiValueZ {
protected:
  Rps_ValueZ(Rps_Type ty) : Rps_QuasiValueZ(ty) {};
public:
  virtual rps_hashint_t hash() const =0;
  virtual Rps_ObjectZ* valclass() const =0;
  virtual Rps_Type vtype() const =0;
  virtual void gcmark(Rps_GarbageCollector&gc, unsigned depth) const =0;
  virtual void display(Rps_Displayer&disp, unsigned depth=0) =0;
  virtual Hjson::Value serialize(void) =0;
};			       // end Rps_ValueZ



class Rps_HashedValueZ : public Rps_ValueZ {
protected:
  mutable rps_hashint_t _hash;
  Rps_HashedValueZ(Rps_Type ty, rps_hashint_t h)
    : Rps_ValueZ(ty), _hash(h) {};
  Rps_HashedValueZ& set_hash(rps_hashint_t h) {
    RPS_ASSERT(_hash == 0);
    RPS_ASSERT(h != 0);
    _hash = h;
    return *this;
  };
public:
  virtual rps_hashint_t hash() const { return _hash; };
  virtual void display(Rps_Displayer&disp, unsigned depth=0) =0;
  virtual Hjson::Value serialize(void) =0;
};			       // end Rps_HashedValueZ

extern bool rps_object_less (Rps_ObjectZ* l, Rps_ObjectZ* r);
extern bool rps_object_lessequal (Rps_ObjectZ* l, Rps_ObjectZ* r);
struct Rps_LessObjPtr
{
  bool operator() (Rps_ObjectZ* l, Rps_ObjectZ* r) const
  {
    return rps_object_less(l,r);
  };
};

////////////////////////////////////////////////////////////////
extern "C" Rps_ObjectZ* rps_string_class;

class Rps_StringValueZ : public Rps_HashedValueZ {
  uint32_t _str_bsize;		// byte size
  uint32_t _str_ulen;		// UTF-8 length
  const char _str_bytes[4];     /// actually a flexible array memeber
  Rps_StringValueZ(rps_hashint_t h, const char*bytes) :
    Rps_HashedValueZ(Rps_Type::String, h), _str_bytes("\0\0\0") {
    strcpy(const_cast<char*>(_str_bytes), bytes);
  };
  virtual ~Rps_StringValueZ();
public:
  virtual  Rps_ObjectZ* valclass() const {return  rps_string_class;};
  virtual Rps_Type vtype() const {return Rps_Type::String;};
  virtual void gcmark(Rps_GarbageCollector&, unsigned) const {};
  static Rps_StringValueZ* make(const char*str);
  static Rps_StringValueZ* make(const std::string s)
  { return make(s.c_str()); };
  virtual void display(Rps_Displayer&disp, unsigned depth=0);
  virtual Hjson::Value serialize(void);
  const char*cbytes() const { return _str_bytes; };
  uint32_t utf8len() const { return _str_ulen; };
  uint32_t bytsize() const { return _str_bsize; };
};				// end Rps_StringValueZ

////////////////////////////////////////////////////////////////
// a slow, simple, but boxed integer
extern "C" Rps_ObjectZ* rps_int_class;
class Rps_IntegerValueZ : public Rps_ValueZ {
  const int64_t _ival;
  Rps_IntegerValueZ(int64_t i) :
    Rps_ValueZ(Rps_Type::Int), _ival(i) {};
  virtual ~Rps_IntegerValueZ();
public:
  virtual  Rps_ObjectZ* valclass() const {return  rps_int_class;};
  virtual Rps_Type vtype() const {return Rps_Type::Int;};
  virtual void gcmark(Rps_GarbageCollector&, unsigned) const {};
  static Rps_IntegerValueZ* make(int64_t);
  virtual void display(Rps_Displayer&disp, unsigned depth=0);
  virtual Hjson::Value serialize(void);
  int64_t ival() const { return _ival; };
  virtual rps_hashint_t hash() const {
    rps_hashint_t h = (rps_hashint_t)_ival ^ (rps_hashint_t)(_ival % 1000000579);
    if (RPS_UNLIKELY(h==0))
      h=((_ival % 1000000933) & 0xffffff) + 5689;
    return h; };
}; // end Rps_IntegerValueZ
  
  
////////////////////////////////////////////////////////////////
// a slow, simple, but boxed double
extern "C" Rps_ObjectZ* rps_double_class;
class Rps_DoubleValueZ : public Rps_ValueZ {
  const double _dval;
  Rps_DoubleValueZ(double x) :
    Rps_ValueZ(Rps_Type::Double), _dval(x) {};
  virtual ~Rps_DoubleValueZ();
public:
  virtual  Rps_ObjectZ* valclass() const {return  rps_double_class;};
  virtual Rps_Type vtype() const {return Rps_Type::Double;};
  virtual void gcmark(Rps_GarbageCollector&, unsigned) const {};
  static Rps_DoubleValueZ* make(double d);
  virtual void display(Rps_Displayer&disp, unsigned depth=0);
  virtual Hjson::Value serialize(void);
  double dval() const { return _dval; };
  virtual rps_hashint_t hash() const;
}; // end Rps_DoubleValueZ
  

////////////////////////////////////////////////////////////////
// superclass for sequences (both tuples and sets) of objects
class Rps_SequenceValueZ : public Rps_HashedValueZ {
  uint32_t _seq_len;	       // the actual number of objects
  Rps_ObjectZ* _seq_obarr[1];  // actually a flexible array number
protected:
  Rps_SequenceValueZ(Rps_Type ty, unsigned alsiz, uint32_t len=0, Rps_ObjectZ*arr=nullptr, rps_hashint_t h=0):
    Rps_HashedValueZ(ty,  h), _seq_len(len), _seq_obarr{nullptr} {
    RPS_ASSERT(len<=alsiz);
     memset(_seq_obarr, 0, alsiz*sizeof(Rps_ObjectZ*));
     if (arr != nullptr)
       memcpy(_seq_obarr, arr, len*sizeof(Rps_ObjectZ*));
  };
  Rps_ObjectZ* unsafe_nth(unsigned rk) const {
    RPS_ASSERT(rk<_seq_len);
    return _seq_obarr[rk];
  };
  virtual void compute_hash(void) =0;
public:
  virtual rps_hashint_t hash() const {
    if (RPS_UNLIKELY(_hash) == 0)
      const_cast<Rps_SequenceValueZ*>(this)->compute_hash();
    return _hash;
  };
  /// for range for loops such as for (auto*it: *seq) {}, see
  /// https://en.cppreference.com/w/cpp/language/range-for
  typedef Rps_ObjectZ*const*iterator_t;
  struct reverse_iterator_t {
    iterator_t revit;
    reverse_iterator_t(Rps_ObjectZ*const*p) : revit(p) {};
    bool operator != (reverse_iterator_t right) const { return revit != right.revit; };
    reverse_iterator_t operator ++ (void)  { revit--; return *this; };
  };
  struct Reverse_Iteration {
    const Rps_SequenceValueZ* revseq;
    Reverse_Iteration(const Rps_SequenceValueZ*seq) : revseq(seq) {};
    reverse_iterator_t begin() const
    { return reverse_iterator_t(revseq->_seq_obarr+revseq->_seq_len-1); };
    reverse_iterator_t end() const
    {return reverse_iterator_t(revseq->_seq_obarr-1); };
  };
  const Reverse_Iteration reverse() const { return Reverse_Iteration(this); };
  int64_t allocated_seqsize() const { return rps_prime_above(_seq_len); };
  uint32_t seqlength() const { return _seq_len; };
  Rps_ObjectZ* nth(int ix) const {
    if (ix<0) ix += _seq_len;
    if (ix>=0 && ix<(int)_seq_len) return _seq_obarr[ix];
    return nullptr;
  };
  iterator_t begin() const {
    return _seq_obarr;
  };
  iterator_t end() const {
    return _seq_obarr + _seq_len;
  };
};			       // end of  Rps_SequenceValueZ


////////////////////////////////////////////////////////////////
// immutable sets of objects
extern "C" Rps_ObjectZ* rps_set_class;
class Rps_SetValueZ : public Rps_SequenceValueZ {
  Rps_SetValueZ(unsigned alsiz, uint32_t len=0, Rps_ObjectZ*arr=nullptr, rps_hashint_t h=0)
    : Rps_SequenceValueZ(Rps_Type::Set, alsiz, len, arr, h) {};
  virtual ~Rps_SetValueZ() {};
  virtual void compute_hash(void);
public:
  uint32_t cardinal() const { return seqlength(); };
  bool contains(Rps_ObjectZ*ob);
  static Rps_SetValueZ* make(const std::set<Rps_ObjectZ*,Rps_LessObjPtr>&setob);
}; 				// end Rps_SetValueZ


#endif /*IDEREFPERSYS_INCLUDED*/
////////// end of file ide-refpersys.hh

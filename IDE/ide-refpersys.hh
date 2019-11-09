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


class Rps_ValueZ : public Rps_QuasiValueZ {
protected:
  Rps_ValueZ(Rps_Type ty) : Rps_QuasiValueZ(ty) {};
public:
  virtual rps_hashint_t hash() const =0;
  virtual void display(Rps_Displayer&disp, unsigned depth=0) =0;
  virtual Hjson::Value serialize(void) =0;
};			       // end Rps_ValueZ



class Rps_HashedValueZ : public Rps_ValueZ {
  rps_hashint_t _hash;
protected:
  Rps_HashedValueZ(Rps_Type ty, rps_hashint_t h)
    : Rps_ValueZ(ty), _hash(h) {};
public:
  virtual rps_hashint_t hash() const { return _hash; };
  virtual void display(Rps_Displayer&disp, unsigned depth=0) =0;
  virtual Hjson::Value serialize(void) =0;
};			       // end Rps_HashedValueZ

class Rps_StringValueZ : public Rps_HashedValueZ {
  uint32_t _str_bsize;		// byte size
  uint32_t _str_ulen;		// UTF-8 length
  const char _str_bytes[4];
  Rps_StringValueZ(rps_hashint_t h, const char*bytes) :
    Rps_HashedValueZ(Rps_Type::String, h), _str_bytes("\0\0\0") {
    strcpy(const_cast<char*>(_str_bytes), bytes);
  };
  virtual ~Rps_StringValueZ();
public:
  static Rps_StringValueZ* make(const char*str);
  static Rps_StringValueZ* make(const std::string s)
  { return make(s.c_str()); };
  virtual void display(Rps_Displayer&disp, unsigned depth=0);
  virtual Hjson::Value serialize(void);
  const char*cbytes() const { return _str_bytes; };
  uint32_t utf8len() const { return _str_ulen; };
  uint32_t bytsize() const { return _str_bsize; };
};

#endif /*IDEREFPERSYS_INCLUDED*/
////////// end of file ide-refpersys.hh

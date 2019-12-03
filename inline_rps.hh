/****************************************************************
 * file inline_rps.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its internal C++ header file for inlined functions.
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



#ifndef INLINE_RPS_INCLUDED
#define INLINE_RPS_INCLUDED


//// this file should only be included at end of refpersys.hh
#ifndef REFPERSYS_INCLUDED
#error wrong direct inclusion of "inline_rps.hh"
#endif /*REFPERSYS_INCLUDED*/


//////////////////////////////////////////////////////////// time functions
// see http://man7.org/linux/man-pages/man2/clock_gettime.2.html
static inline double
rps_wallclock_real_time(void)
{
  struct timespec ts =  {0,0};
  if (clock_gettime(CLOCK_REALTIME, &ts))
    return NAN;
  return 1.0*ts.tv_sec + 1.0e-9*ts.tv_nsec;
} // end rps_wallclock_real_time

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


//////////////////////////////////////////////////////////// objids
std::string
Rps_Id::to_string() const
{
  char cbuf[24];
  memset (cbuf, 0, sizeof(cbuf));
  to_cbuf24(cbuf);
  return std::string(cbuf);
} // end of Rps_Id::to_string

static inline
std::ostream& operator << (std::ostream& out, const Rps_Id id)
{

  char cbuf[24];
  memset (cbuf, 0, sizeof(cbuf));
  id.to_cbuf24(cbuf);
  out << cbuf;
  return out;
} // end output of Rps_Id

//////////////////////////////////////////////////////////// backtracing
static inline
std::ostream& operator << (std::ostream& out, const Rps_BackTrace_Helper& rph)
{
  auto o = rph.swap_output(&out);
  out << std::endl;
  rph.do_out();
  rph.swap_output(o);
  out << std::endl;
  return out;
} // end of << for Rps_Backtrace_Helper


//////////////////////////////////////////////////////////// values
Rps_Value::Rps_Value() : _wptr(nullptr) {};

Rps_Value::~Rps_Value()
{
  _wptr = nullptr;
};

Rps_Value::Rps_Value(nullptr_t) : _wptr(nullptr) {};

Rps_Value::Rps_Value(Rps_EmptyTag) : _wptr (RPS_EMPTYSLOT) {};

Rps_Value::Rps_Value(intptr_t i, Rps_IntTag) :
  _ival(((i >> 1) << 1) | 1) {};

Rps_Value::Rps_Value(const Rps_ZoneValue*ptr, Rps_ValPtrTag) :
  _pval(ptr)
{
  RPS_ASSERT(ptr == nullptr || ptr == RPS_EMPTYSLOT
             || (((intptr_t)ptr) & (sizeof(void*)-1)) == 0);
}

bool Rps_Value::is_int() const
{
  return (_ival & 1) != 0;
};

bool Rps_Value::is_ptr() const
{
  return !is_int()
         && _wptr != nullptr
         && _wptr != RPS_EMPTYSLOT
         && (((intptr_t)_pval) & (sizeof(void*)-1)) == 0;
}

const Rps_ZoneValue*
Rps_Value::as_ptr() const
{
  if (is_ptr()) return _pval;
  else throw std::runtime_error("Rps_Value::as_ptr: value is not genuine pointer");
}


void
Rps_Value::gc_mark(Rps_GarbageCollector&gc, unsigned depth)
{
  if (!is_ptr()) return;
  if (_pval->is_gcmarked(gc)) return;
  Rps_ZoneValue* pzv = const_cast<Rps_ZoneValue*>(_pval);
  pzv->set_gcmark(gc);
  if (RPS_UNLIKELY(depth > max_gc_mark_depth))
    throw std::runtime_error("too deep gc_mark");
  pzv->gc_mark(gc, depth);
} // end Rps_Value::gc_mark

bool
Rps_Value::is_object() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Object;
} //end  Rps_Value::is_object()


Rps_HashInt
Rps_Value::valhash() const noexcept
{
  if (is_empty()) return 0;
  if (is_int())
    {
      intptr_t i = to_int();
      Rps_HashInt h = i ^ (i>>27);
      if (RPS_UNLIKELY(!h))
        h = (i & 0xfffff) + 13;
      RPS_ASSERT(h != 0);
      return h;
    }
  else if (is_ptr())
    {
      const Rps_ZoneValue*pval = as_ptr();
      RPS_ASSERT(pval != nullptr);
      return pval->val_hash();
    };
  return 0;
} // end of Rps_Value::valhash()

bool Rps_Value::is_set() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Set;
} //end  Rps_Value::is_set()

bool Rps_Value::is_tuple() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Tuple;
} //end  Rps_Value::is_tuple()

bool Rps_Value::is_string() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::String;
} //end  Rps_Value::is_string()

bool Rps_Value::is_double() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Double;
} //end  Rps_Value::is_double()

const Rps_SetOb*
Rps_Value::as_set() const
{
  if (is_set())
    return reinterpret_cast<const Rps_SetOb*>(_pval);
  else throw std::domain_error("Rps_Value::as_set: value is not genuine set");
} // end Rps_Value::as_set

const Rps_SetOb*
Rps_Value::to_set(const Rps_SetOb*defset) const
{
  if (is_set())
    return reinterpret_cast<const Rps_SetOb*>(const_cast<Rps_ZoneValue*>(_pval));
  else return defset;
} // end Rps_Value::to_set


Rps_SetValue::Rps_SetValue (const std::set<Rps_ObjectRef>& obset)
  : Rps_Value (Rps_SetOb::make(obset), Rps_ValPtrTag{})
{
} // end of Rps_SetValue::Rps_SetValue of set

Rps_SetValue::Rps_SetValue (const std::vector<Rps_ObjectRef>& obvec)
  : Rps_Value (Rps_SetOb::make(obvec), Rps_ValPtrTag{})
{
} // end of Rps_SetValue::Rps_SetValue of vector

Rps_SetValue::Rps_SetValue (const std::initializer_list<Rps_ObjectRef>& obil, Rps_SetTag)
  : Rps_Value (Rps_SetOb::make(obil), Rps_ValPtrTag{})
{
} // end of Rps_SetValue::Rps_SetValue of initializer_list

Rps_SetValue::Rps_SetValue(const std::vector<Rps_Value>& vecval)
  : Rps_Value (Rps_SetOb::collect(vecval), Rps_ValPtrTag{})
{
} // end of Rps_SetValue::Rps_SetValue of vector of values

Rps_SetValue::Rps_SetValue(const std::initializer_list<Rps_Value>&valil)
  : Rps_Value (Rps_SetOb::collect(valil), Rps_ValPtrTag{})
{
} // end of Rps_SetValue::Rps_SetValue of initializer_list of values

Rps_SetValue::Rps_SetValue(const Rps_Value val)
  : Rps_Value (val.is_set()?val.as_set():nullptr, Rps_ValPtrTag{})
{
} // end Rps_SetValue::Rps_SetValue dynamic

const Rps_TupleOb*
Rps_Value::as_tuple() const
{
  if (is_tuple())
    return reinterpret_cast<const Rps_TupleOb*>(_pval);
  else throw std::domain_error("Rps_Value::as_tuple: value is not genuine tuple");
} // end Rps_Value::as_tuple

const Rps_TupleOb*
Rps_Value::to_tuple(const Rps_TupleOb*deftup) const
{
  if (is_tuple())
    return reinterpret_cast<const Rps_TupleOb*>(const_cast<Rps_ZoneValue*>(_pval));
  else return deftup;
} // end Rps_Value::to_tuple



Rps_TupleValue::Rps_TupleValue (const std::vector<Rps_ObjectRef>& obvec)
  : Rps_Value (Rps_TupleOb::make(obvec), Rps_ValPtrTag{})
{
} // end of Rps_TupleValue::Rps_TupleValue of vector

Rps_TupleValue::Rps_TupleValue (const std::initializer_list<Rps_ObjectRef>& obil, Rps_TupleTag)
  : Rps_Value (Rps_TupleOb::make(obil), Rps_ValPtrTag{})
{
} // end of Rps_TupleValue::Rps_TupleValue of initializer_list

Rps_TupleValue::Rps_TupleValue(const std::vector<Rps_Value>& vecval)
  : Rps_Value (Rps_TupleOb::collect(vecval), Rps_ValPtrTag{})
{
} // end of Rps_TupleValue::Rps_TupleValue of vector of values

Rps_TupleValue::Rps_TupleValue(const std::initializer_list<Rps_Value>&valil)
  : Rps_Value (Rps_TupleOb::collect(valil), Rps_ValPtrTag{})
{
} // end of Rps_TupleValue::Rps_TupleValue of initializer_list of values

Rps_TupleValue::Rps_TupleValue(const Rps_Value val)
  : Rps_Value (val.is_set()?val.as_set():nullptr, Rps_ValPtrTag{})
{
} // end Rps_TupleValue::Rps_TupleValue dynamic

const Rps_String*
Rps_Value::as_string() const
{
  if (is_string())
    return reinterpret_cast<const Rps_String*>(_pval);
  else throw std::domain_error("Rps_Value::as_string: value is not genuine string");
} // end Rps_Value::as_string

const std::string
Rps_Value::as_cppstring() const
{
  if (is_string())
    return to_string()->cppstring();
  else
    throw std::domain_error("Rps_Value::as_cppstring: value is not genuine string");
} // end Rps_Value::as_cppstring

const char*
Rps_Value::as_cstring() const
{
  if (is_string())
    return to_string()->cstr();
  else
    throw std::domain_error("Rps_Value::as_string: value is not genuine string");
} // end Rps_Value::as_cstring

const Rps_String*
Rps_Value::to_string(const Rps_String*defstr) const
{
  if (is_string())
    return reinterpret_cast<const Rps_String*>(const_cast<Rps_ZoneValue*>(_pval));
  else return defstr;
} // end Rps_Value::to_string

const std::string
Rps_Value::to_cppstring(std::string defstr) const
{
  auto strp = to_string();
  if (strp) return strp->cppstring();
  else return defstr;
} // end of Rps_Value::to_cppstring

Rps_ObjectZone*
Rps_Value::as_object() const
{
  if (is_object())
    return reinterpret_cast<Rps_ObjectZone*>(const_cast<Rps_ZoneValue*>(_pval));
  else throw std::domain_error("Rps_Value::as_object: value is not genuine object");
} // end Rps_Value::as_object

const Rps_ObjectZone*
Rps_Value::to_object(const Rps_ObjectZone*defob) const
{
  if (is_object())
    return reinterpret_cast<Rps_ObjectZone*>(const_cast<Rps_ZoneValue*>(_pval));
  else return defob;
} // end Rps_Value::to_object

const Rps_Double*
Rps_Value::as_boxed_double() const
{
  if (is_double())
    return reinterpret_cast<const Rps_Double*>(_pval);
  else throw std::domain_error("Rps_Value::as_boxed_double: value is not genuine double");
} // end Rps_Value::as_boxed_double

double
Rps_Value::as_double() const
{
  if (is_double())
    return as_boxed_double()->dval();
  else throw std::domain_error("Rps_Value::as_double: value is not genuine double");
} // end Rps_Value::as_boxed_double

double
Rps_Value::to_double(double def) const
{
  if (is_double())
    return as_double();
  else return def;
} // end Rps_Value::to_double


bool
Rps_Value::operator == (const Rps_Value v) const
{
  if  (v._wptr == _wptr) return true;
  if (is_empty() || is_null()) return v.is_empty() || v.is_null();
  if (is_int()) return false;
  return (*as_ptr()) == (*v.as_ptr());
}   // end Rps_Value::operator ==

bool
Rps_Value::operator != (const Rps_Value v) const
{
  return !(*this == v);  // end Rps_Value::operator !=
}

bool
Rps_Value::operator <= (const Rps_Value v) const
{
  if  (v._wptr == _wptr)
    return true;
  if (is_empty() || is_null())
    return true;
  if (v.is_empty() || v.is_null())
    return false;
  if (is_int())
    return (v.is_int() && (as_int() <= v.as_int() || v.is_ptr()));
  if (is_ptr() && v.is_ptr())
    return (*as_ptr()) <= (*v.as_ptr());
  return false;
}   // end Rps_Value::operator <=

bool
Rps_Value::operator < (const Rps_Value v) const
{
  if  (v._wptr == _wptr)
    return false;
  if (is_empty() || is_null())
    return !(v.is_empty() || v.is_null());
  if (v.is_empty() || v.is_null())
    return false;
  if (is_int())
    return (v.is_int() && (as_int() < v.as_int() || v.is_ptr()));
  if (is_ptr() && v.is_ptr())
    return (*as_ptr()) <= (*v.as_ptr());
  return false;
}   // end Rps_Value::operator <

bool
Rps_Value::operator > (const Rps_Value v) const
{
  return *this < v;
} // end Rps_Value::operator <
bool
Rps_Value::operator >= (const Rps_Value v) const
{
  return *this <= v;
} // end Rps_Value::operator >=

////////////////////////////////////////////////////// quasi zones
Rps_QuasiZone::Rps_QuasiZone(Rps_Type ty)
  : qz_type(ty), qz_gcinfo(0)
{
  register_in_zonevec();
} // end of Rps_QuasiZone::Rps_QuasiZone

void
Rps_QuasiZone::every_zone(Rps_GarbageCollector&gc, std::function<void(Rps_GarbageCollector&, Rps_QuasiZone*)>fun)
{
  std::lock_guard<std::mutex> gu(qz_mtx);
  auto nbzones = (uint32_t)qz_zonvec.size();
  for (uint32_t zix=1; zix<nbzones; zix++)
    {
      auto curzon = qz_zonvec[zix];
      if (!curzon)
        continue;
      RPS_ASSERT(curzon->qz_rank == zix);
      fun(gc, curzon);
    }
} // end Rps_QuasiZone::every_zone

Rps_QuasiZone*
Rps_QuasiZone::nth_zone(uint32_t rk)
{
  std::lock_guard<std::mutex> gu(qz_mtx);
  if (rk<=0 || rk>=qz_zonvec.size()) return nullptr;
  return qz_zonvec[rk];
} // end  Rps_QuasiZone::nth_zone

Rps_QuasiZone*
Rps_QuasiZone::raw_nth_zone(uint32_t rk, Rps_GarbageCollector&)
{
  if (rk<=0 || rk>=qz_zonvec.size()) return nullptr;
  return qz_zonvec[rk];
} // end Rps_QuasiZone::raw_nth_zone

void
Rps_QuasiZone::run_locked_gc(Rps_GarbageCollector&gc, std::function<void(Rps_GarbageCollector&)>fun)
{
  std::lock_guard<std::mutex> gu(qz_mtx);
  fun(gc);
} // end Rps_QuasiZone::run_locked_gc


// the GC related routines below don't really use the
// Rps_GarbageCollector but needs one for typing safety.

// test the GC mark
bool
Rps_QuasiZone::is_gcmarked(Rps_GarbageCollector&) const
{
  auto gcinf = qz_gcinfo.load();
  return gcinf & qz_gcmark_bit;
} // end Rps_QuasiZone::is_gcmarked

// set the GC mark
void
Rps_QuasiZone::set_gcmark(Rps_GarbageCollector&)
{
  qz_gcinfo.fetch_or(qz_gcmark_bit);
} // end Rps_QuasiZone::set_gcmark

// clear the GC mark
void
Rps_QuasiZone::clear_gcmark(Rps_GarbageCollector&)
{
  qz_gcinfo.fetch_and(~qz_gcmark_bit);
} // end Rps_QuasiZone::clear _gcmark

inline void*
Rps_QuasiZone::operator new (std::size_t siz, std::nullptr_t)
{
  RPS_ASSERT(siz % sizeof(void*) == 0);
  return ::operator new (siz);
} // end plain Rps_QuasiZone::operator new


inline void*
Rps_QuasiZone::operator new (std::size_t siz, unsigned wordgap)
{
  RPS_ASSERT(siz % sizeof(void*) == 0);
  return ::operator new (siz + wordgap * sizeof(void*));
} // end wordgapped Rps_QuasiZone::operator new


//////////////////////////////////////////////////////////// zone values

bool Rps_Value::is_empty() const
{
  return _wptr == nullptr || _wptr == RPS_EMPTYSLOT;
}

bool Rps_Value::is_null() const
{
  return _wptr == nullptr;
}

std::intptr_t
Rps_Value::as_int() const
{
  if (!is_int()) throw std::invalid_argument("value is not an int");
  return _ival>>1;
}

intptr_t
Rps_Value::to_int(intptr_t def) const
{
  if (is_int())
    {
      return _ival>>1;
    }
  else return def;
} // end Rps_Value::to_int

Rps_Value::Rps_Value(const Rps_Value&oth) : _wptr(oth._wptr) {};

Rps_ZoneValue::Rps_ZoneValue(Rps_Type typ)
  : Rps_QuasiZone(typ)
{
  RPS_ASSERT (typ >= Rps_Type::None);
} // end of Rps_ZoneValue::Rps_ZoneValue

bool
Rps_ZoneValue::operator == (const Rps_ZoneValue&zv) const
{
  if (this == &zv) return true;
  if (this->stored_type() != zv.stored_type()) return false;
  return equal(zv);
}     // end Rps_ZoneValue::operator ==

bool
Rps_ZoneValue::operator <= (const Rps_ZoneValue&zv) const
{
  if (this == &zv) return true;
  if (this->stored_type() > zv.stored_type()) return false;
  if (this->stored_type() < zv.stored_type()) return true;
  return less(zv);
}     // end Rps_ZoneValue::operator <=

bool
Rps_ZoneValue::operator >= (const Rps_ZoneValue&zv) const
{
  return  zv <= (*this);
} // end Rps_ZoneValue::operator >=

bool
Rps_ZoneValue::operator < (const Rps_ZoneValue&zv) const
{
  if (this == &zv) return false;
  if (this->stored_type() > zv.stored_type()) return false;
  if (this->stored_type() < zv.stored_type()) return true;
  return less(zv);
}     // end Rps_ZoneValue::operator <

bool
Rps_ZoneValue::operator > (const Rps_ZoneValue&zv) const
{
  return  zv < (*this);
} // end Rps_ZoneValue::operator >


/////////////////////////////////////////////////// lazy hashed values
Rps_LazyHashedZoneValue::Rps_LazyHashedZoneValue(Rps_Type typ) :
  Rps_ZoneValue(typ), _lazyhash(0)
{
};				// end Rps_LazyHashedZoneValue

//////////////////////////////////////////////////////////// strings
Rps_HashInt
rps_hash_cstr(const char*cstr, int len)
{
  Rps_HashInt h = 0;
  int64_t ht[2] = {0,0};
  int utf8len = rps_compute_cstr_two_64bits_hash(ht, cstr, len);
  if (utf8len>=0)
    {
      h = Rps_HashInt (ht[0] ^  ht[1]);
      if (RPS_UNLIKELY(h == 0))
        {
          h = ((Rps_HashInt (ht[0] >> 24)) & 0xffffff)
              + ((Rps_HashInt (ht[1] >> 27)) & 0xffffff)
              + (utf8len & 0xfff) + 17;
          RPS_ASSERT(h != 0);
        }
      return h;
    }
  return 0;
}     // end of rps_hash_cstr

Rps_Value::Rps_Value(const std::string&str)
  : Rps_Value(Rps_String::make(str.c_str(), str.size()), Rps_ValPtrTag{}) {};

Rps_Value::Rps_Value(const char*str, int len)
  : Rps_Value(Rps_String::make(str, len), Rps_ValPtrTag{}) {};


const char*
Rps_String::normalize_cstr(const char*cstr)
{
  if (cstr == nullptr || cstr == (const char*)RPS_EMPTYSLOT
      || !*cstr) return "";
  return cstr;
} // end Rps_String::normalize_cstr


int
Rps_String::normalize_len(const char*cstr, int len)
{
  if (cstr == nullptr || cstr == (const char*)RPS_EMPTYSLOT
      || !*cstr) return 0;
  if (len>0) return len;
  return strlen(cstr);
} // end Rps_String::normalize_len

uint32_t
Rps_String::safe_utf8len(const char*cstr, int len)
{
  if (cstr == nullptr || cstr == (const char*)RPS_EMPTYSLOT
      || !*cstr) return 0;
  if (len<0)
    len = strlen(cstr);
  if (RPS_UNLIKELY(u8_check((const uint8_t*)cstr, (size_t)len) != nullptr))
    RPS_FATAL("corrupted UTF8 string %.*s", len, cstr);
  return u8_mblen((const uint8_t*)cstr, (size_t)len);
}; // end of Rps_String::safe_utf8len

Rps_String::Rps_String (const char*cstr, int len)
  : Rps_LazyHashedZoneValue (Rps_Type::String),
    _bytsiz(normalize_len(cstr,len)),
    _utf8len(safe_utf8len(cstr,len))
{
  cstr = normalize_cstr(cstr);
  if (_utf8len>0)
    memcpy(_alignbuf, cstr, _bytsiz);
}; // end of Rps_String::Rps_String

const Rps_String*
Rps_String::make(const std::string&s)
{
  return Rps_String::make(s.c_str(), s.size());
} // end Rps_String::make(const std::string&s)


Rps_StringValue::Rps_StringValue (const char*cstr, int len)
  : Rps_Value(Rps_String::make(cstr, len), Rps_ValPtrTag{})
{
} // end Rps_StringValue::Rps_StringValue (const char*cstr, int len)

Rps_StringValue::Rps_StringValue(const std::string str)
  : Rps_Value(Rps_String::make(str), Rps_ValPtrTag{})
{
} // end Rps_StringValue::Rps_StringValue

Rps_StringValue::Rps_StringValue(const Rps_Value val)
  : Rps_Value(val.is_string()?(val.to_string()):nullptr, Rps_ValPtrTag{})
{
} // end Rps_StringValue::Rps_StringValue(const Rps_Value val)

Rps_StringValue::Rps_StringValue(const Rps_String* strv)
  : Rps_Value(strv, Rps_ValPtrTag{})
{
} // end Rps_StringValue::Rps_StringValue(const Rps_String* strv)

Rps_StringValue::Rps_StringValue(nullptr_t)
  : Rps_Value(nullptr)
{
} // end of Rps_StringValue::Rps_StringValue(nullptr_t)
//////////////////////////////////////////////////////////// boxed doubles
Rps_Value::Rps_Value (double d, Rps_DoubleTag)
  : Rps_Value(Rps_Double::make(d), Rps_ValPtrTag())
{
  if (RPS_UNLIKELY(std::isnan(d)))
    throw std::invalid_argument("NaN cannot be an Rps_Value");
};      // end Rps_Value::Rps_Value (double d, Rps_DoubleTag)

Rps_Value::Rps_Value (double d) : Rps_Value::Rps_Value (d, Rps_DoubleTag{}) {};

const Rps_Double* Rps_Value::to_boxed_double(const Rps_Double*defdbl) const
{
  if (is_double()) return reinterpret_cast<const Rps_Double*>(_pval);
  else return defdbl;
}

Rps_Double::Rps_Double(double d)
  : Rps_LazyHashedZoneValue(Rps_Type::Double), _dval(d)
{
  if (RPS_UNLIKELY(std::isnan(d)))
    throw std::invalid_argument("NaN cannot be given to Rps_Double");
} // end of Rps_Double::Rps_Double

const Rps_Double*
Rps_Double::make(double d)
{
  if (RPS_UNLIKELY(std::isnan(d)))
    throw std::invalid_argument("NaN cannot be given to Rps_Double::make");
  return Rps_QuasiZone::rps_allocate<Rps_Double,double>(d);
} // end  Rps_Double::make

Rps_DoubleValue::Rps_DoubleValue (double d)
  : Rps_Value(Rps_Double::make(d), Rps_ValPtrTag{})
{
} // end Rps_DoubleValue::Rps_DoubleValue (double d=0.0)

Rps_DoubleValue::Rps_DoubleValue(const Rps_Value val)
  : Rps_Value(val.is_double()?val.to_boxed_double():nullptr,  Rps_ValPtrTag{})
{
} // end Rps_DoubleValue::Rps_DoubleValue

//////////////////////////////////////////////////////////// objects
Rps_HashInt
Rps_ObjectRef::obhash(void) const
{
  if (is_empty()) return 0;
  return _optr->obhash();
}      // end Rps_ObjectRef::obhash

bool
Rps_ObjectRef::operator == (const Rps_ObjectRef& oth) const
{
  if (is_empty()) return oth.is_empty();
  if (oth.is_empty()) return false;
  return _optr == oth._optr;
}

bool
Rps_ObjectRef::operator != (const Rps_ObjectRef& oth) const
{
  return ! (*this == oth);
}

bool
Rps_ObjectRef::operator <= (const Rps_ObjectRef& oth) const
{
  if (is_empty()) return true;
  if (oth.is_empty()) return false;
  return _optr->oid() <= oth._optr->oid();
}

bool
Rps_ObjectRef::operator < (const Rps_ObjectRef& oth) const
{
  if (is_empty()) return !oth.is_empty();
  if (oth.is_empty()) return false;
  return _optr->oid() < oth._optr->oid();
}

bool
Rps_ObjectRef::operator >= (const Rps_ObjectRef& oth) const
{
  return oth <= *this;
}

bool
Rps_ObjectRef::operator > (const Rps_ObjectRef& oth) const
{
  return oth < *this;
}

Rps_ObjectValue::Rps_ObjectValue(const Rps_ObjectRef obr)
  : Rps_Value (obr.to_object(), Rps_ValPtrTag{}) {};

Rps_ObjectValue::Rps_ObjectValue(const Rps_Value val, const Rps_ObjectZone*defob)
  : Rps_Value (val.is_object()?val.to_object():defob, Rps_ValPtrTag{}) {};

Rps_ObjectValue::Rps_ObjectValue(const Rps_ObjectZone* obz)
  : Rps_Value (obz, Rps_ValPtrTag{}) {};

Rps_ObjectValue::Rps_ObjectValue(nullptr_t)
  : Rps_Value (nullptr, Rps_ValPtrTag{}) {};

#endif /*INLINE_RPS_INCLUDED*/
////////////////////////////////////////////////// end of internal header file inline_rps.hh

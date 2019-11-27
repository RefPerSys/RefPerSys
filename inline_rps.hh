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
  else throw std::runtime_error("as_ptr: value is not genuine pointer");
}

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
  : _type(ty), _gcinfo(0) {};


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
//////////////////////////////////////////////////////////// boxed doubles
Rps_Value::Rps_Value (double d, Rps_DoubleTag)
  : Rps_Value(Rps_Double::make(d), Rps_ValPtrTag())
{
  if (RPS_UNLIKELY(std::isnan(d)))
    throw std::invalid_argument("NaN cannot be an Rps_Value");
};      // end Rps_Value::Rps_Value (double d, Rps_DoubleTag)

Rps_Value::Rps_Value (double d) : Rps_Value::Rps_Value (d, Rps_DoubleTag{}) {};

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

#endif /*INLINE_RPS_INCLUDED*/
////////////////////////////////////////////////// end of internal header file inline_rps.hh

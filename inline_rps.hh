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


#endif /*INLINE_RPS_INCLUDED*/
// end of internal header file inline_rps.hh

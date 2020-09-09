/****************************************************************
 * file inline_rps.hh
 * SPDX-License-Identifier: GPL-3.0-or-later
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


//////////////////////////////////////////////////////////// values
Rps_Value::Rps_Value() : _wptr(nullptr) {};

void Rps_Value::clear()
{
  _wptr = nullptr;
};

Rps_Value::Rps_Value(const void*ptr, const Rps_PayloadSymbol*symb) : _wptr(ptr)
{
  RPS_ASSERT(symb != nullptr && symb->stored_type() == Rps_Type::PaylSymbol);
};

Rps_Value::Rps_Value(const void*ptr, Rps_CallFrame*cframe) : _wptr(ptr)
{
  RPS_ASSERT(cframe != nullptr && cframe->stored_type() == Rps_Type::CallFrame);
};

Rps_Value::~Rps_Value()
{
  _wptr = nullptr;
};

Rps_Value::Rps_Value(std::nullptr_t) : _wptr(nullptr) {};

Rps_Value::Rps_Value(Rps_EmptyTag) : _wptr (RPS_EMPTYSLOT) {};

Rps_Value::Rps_Value(intptr_t i, Rps_IntTag) :
  _ival(((i >> 1) << 1) | 1) {};

Rps_Value::Rps_Value(const Rps_ZoneValue*ptr, Rps_ValPtrTag) :
  _pval(ptr)
{
  RPS_ASSERT(ptr == nullptr || ptr == RPS_EMPTYSLOT
             || (((intptr_t)ptr) & (sizeof(void*)-1)) == 0);
}

Rps_Value::Rps_Value(Rps_Value&&oth) :
  _wptr(oth._wptr) {};

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

Rps_Type
Rps_Value::type() const
{
  if (is_int())
    return Rps_Type::Int;
  else if (is_empty())
    return Rps_Type::None;
  else
    return _pval->type();
} // end Rps_Value::type()

const Rps_ZoneValue*
Rps_Value::as_ptr() const
{
  if (is_ptr()) return _pval;
  else throw std::runtime_error("Rps_Value::as_ptr: value is not genuine pointer");
}

const Rps_ZoneValue*
Rps_Value::to_ptr(const Rps_ZoneValue*defzp) const
{
  if (is_ptr()) return _pval;
  else return defzp;
}

void
Rps_Value::gc_mark(Rps_GarbageCollector&gc, unsigned depth) const
{
  if (!is_ptr()) return;
  if (_pval->is_gcmarked(gc)) return;
  Rps_ZoneValue* pzv = const_cast<Rps_ZoneValue*>(_pval);
  pzv->set_gcmark(gc);
  if (RPS_UNLIKELY(depth > max_gc_mark_depth))
    throw std::runtime_error("too deep gc_mark");
  pzv->gc_mark(gc, depth);
} // end Rps_Value::gc_mark


void
Rps_Value::output(std::ostream&out, unsigned depth) const
{
  if (is_int())
    out << as_int();
  else if (is_empty())
    out << "___";
  else if (is_ptr())
    as_ptr()->val_output(out,depth);
} // end Rps_Value::output

const void*
Rps_Value::data_for_symbol(Rps_PayloadSymbol*symb) const
{
  RPS_ASSERT(dynamic_cast<Rps_PayloadSymbol*>(symb) != nullptr);
  return _wptr;
} // end Rps_Value::data_for_symbol

bool
Rps_Value::is_object() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Object;
} //end  Rps_Value::is_object()


bool
Rps_Value::is_instance() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Instance;
} //end  Rps_Value::is_instance()


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

Rps_Value&
Rps_Value::operator=(Rps_Value&&r)
{
  _wptr = r._wptr;
  return *this;
}
Rps_Value&
Rps_Value::operator=(const Rps_Value&oth)
{
  _wptr = oth._wptr;
  return *this;
}

bool Rps_Value::is_set() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Set;
} //end  Rps_Value::is_set()

bool Rps_Value::is_closure() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Closure;
} //end  Rps_Value::is_closure()

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

bool Rps_Value::is_json() const
{
  return is_ptr()
         && as_ptr()->stored_type() == Rps_Type::Json;
} //end  Rps_Value::is_json()


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


const Rps_ClosureZone*
Rps_Value::as_closure() const
{
  if (is_closure())
    return reinterpret_cast<const Rps_ClosureZone*>(_pval);
  else throw std::domain_error("Rps_Value::as_closure: value is not genuine closure");
} // end Rps_Value::as_closure


const Rps_ClosureZone*
Rps_Value::to_closure(const Rps_ClosureZone*defcloz) const
{
  if (is_closure())
    return reinterpret_cast<const Rps_ClosureZone*>(const_cast<Rps_ZoneValue*>(_pval));
  else return defcloz;
} // end Rps_Value::to_closure


const Rps_JsonZone*
Rps_Value::as_json() const
{
  if (is_json())
    return reinterpret_cast<const Rps_JsonZone*>(_pval);
  else throw std::domain_error("Rps_Value::as_json: value is not genuine JSON");
} // end Rps_Value::as_json


int
Rps_SetOb::element_index(const Rps_ObjectRef obelem) const
{
  if (stored_type() != Rps_Type::Set) return -1;
  if (!obelem || obelem.is_empty()) return -1;
  RPS_ASSERT(obelem->stored_type() == Rps_Type::Object);
  unsigned card = cnt();
  RPS_ASSERT(card <= maxsize);
  auto setdata = (raw_const_data());
  int lo = 0, hi = (int)card - 1;
  while (lo + 4 < hi)
    {
      int md = (lo + hi) / 2;
      auto curobr = setdata[md];
      if (RPS_UNLIKELY(curobr == obelem))
        return md;
      RPS_ASSERT(!curobr.is_empty()
                 && curobr->stored_type() == Rps_Type::Object);
      if (curobr < obelem)
        lo = md;
      else
        hi = md;
    };
  for (int md = lo; md < hi; md++)
    {
      auto curobr = setdata[md];
      if (RPS_UNLIKELY(curobr == obelem))
        return md;
    }
  return -1;
} // end Rps_SetValue::element_index

Rps_SetValue::Rps_SetValue (void)
  : Rps_Value (&Rps_SetOb::the_empty_set(), Rps_ValPtrTag{})
{
} // end of Rps_SetValue::Rps_SetValue empty


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

////////////////
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
  else
    {
      RPS_DEBUG_LOG(LOWREP, "Rps_Value::as_object bad this=" << *this
                    << RPS_FULL_BACKTRACE_HERE(1, "bad as_object"));
      throw std::domain_error("Rps_Value::as_object: value is not genuine object");
    }
} // end Rps_Value::as_object

const Rps_InstanceZone*
Rps_Value::as_instance() const
{
  if (is_instance())
    return reinterpret_cast<const Rps_InstanceZone*>(const_cast<Rps_ZoneValue*>(_pval));
  else throw std::domain_error("Rps_Value::as_instance: value is not genuine instance");
} // end Rps_Value::as_instance

const Rps_ObjectZone*
Rps_Value::to_object(const Rps_ObjectZone*defob) const
{
  if (is_object())
    return reinterpret_cast<Rps_ObjectZone*>(const_cast<Rps_ZoneValue*>(_pval));
  else return defob;
} // end Rps_Value::to_object

const Rps_InstanceZone*
Rps_Value::to_instance(const Rps_InstanceZone*definst) const
{
  if (is_instance())
    return reinterpret_cast<const Rps_InstanceZone*>(const_cast<Rps_ZoneValue*>(_pval));
  else return definst;
} // end Rps_Value::to_instance

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


const Json::Value
Rps_Value::to_json(const Json::Value defjv) const
{
  if (is_json())
    return as_json()->json();
  else return defjv;
} // end Rps_Value::to_json


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
  : Rps_TypedZone(ty)
{
  register_in_zonevec();
} // end of Rps_QuasiZone::Rps_QuasiZone

void
Rps_QuasiZone::every_zone(Rps_GarbageCollector&gc, std::function<void(Rps_GarbageCollector&, Rps_QuasiZone*)>fun)
{
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
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
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
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
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
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

// test if this value is an instance of given obclass
bool
Rps_Value::is_instance_of(Rps_CallFrame*callerframe, Rps_ObjectRef obclass) const
{
  RPS_ASSERT(!callerframe || callerframe->stored_type() == Rps_Type::CallFrame);
  if (!obclass || !obclass->is_class())
    return false;
  Rps_ObjectRef thisclass = compute_class(callerframe);
  RPS_ASSERT(thisclass);
  if (thisclass == obclass)
    return true;
  return thisclass->is_instance_of(obclass);
} // end Rps_Value::is_instance_of


// test if this value is a subclass of given obsuperclass:
bool
Rps_Value::is_subclass_of(Rps_CallFrame*callerframe, Rps_ObjectRef obsuperclass) const
{
  RPS_ASSERT(!callerframe || callerframe->stored_type() == Rps_Type::CallFrame);
  if (!obsuperclass || !obsuperclass->is_class())
    return false;
  Rps_ObjectRef thisclass = compute_class(callerframe);
  RPS_ASSERT(thisclass);
  if (thisclass == obsuperclass)
    return true;
  return thisclass->is_subclass_of(obsuperclass);
} // end Rps_Value::is_subclass_of



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
  _alignbuf[_bytsiz] = (char)0;
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

Rps_StringValue::Rps_StringValue(std::nullptr_t)
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



Rps_JsonValue::Rps_JsonValue(const Json::Value&jv)
  : Rps_Value(Rps_JsonZone::make(jv))
{
} // end Rps_JsonValue::Rps_JsonValue

Rps_JsonValue::Rps_JsonValue(const Rps_Value val)
  : Rps_Value(val.is_json()?val.to_boxed_json():nullptr,  Rps_ValPtrTag{})
{
} // end Rps_JsonValue::Rps_JsonValue


const Rps_JsonZone*
Rps_Value::to_boxed_json(const Rps_JsonZone*defjs) const
{
  if (is_json()) return reinterpret_cast<const Rps_JsonZone*>(_pval);
  else return defjs;
} // end Rps_Value::to_boxed_json

//////////////////////////////////////////////////////////////// json

Rps_JsonZone::Rps_JsonZone(const Json::Value& jv)
  : Rps_LazyHashedZoneValue(Rps_Type::Json), _jsonval(jv)
{
} // end of Rps_JsonZone::Rps_JsonZone





//////////////////////////////////////////////////////////// objects zones



Rps_Payload*
Rps_ObjectZone::get_payload(void) const
{
  return ob_payload.load();
} // end Rps_ObjectZone::get_payload(void)

Rps_PayloadClassInfo*
Rps_ObjectZone::get_classinfo_payload(void) const
{
  auto payl = ob_payload.load();
  if (payl && RPS_UNLIKELY(payl->stored_type() == Rps_Type::PaylClassInfo))
    return reinterpret_cast<Rps_PayloadClassInfo*>(payl);
  return nullptr;
} // end Rps_ObjectZone::get_classinfo_payload

bool
Rps_ObjectZone::has_erasable_payload(void) const
{
  auto py = ob_payload.load();
  if (py != nullptr)
    return py->is_erasable();
  return false;
} // end Rps_ObjectZone::has_erasable_payload


void
Rps_ObjectZone::clear_payload(void)
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  Rps_Payload*oldpayl = ob_payload.exchange(nullptr);
  if (oldpayl)
    {
      if (oldpayl->owner() == this)
        {
          if (!oldpayl->is_erasable())
            {
              ob_payload.store(oldpayl);
              RPS_WARNOUT("cannot remove unerasable payload " <<
                          oldpayl->payload_type_name()
                          << " from " << Rps_ObjectRef(this)
                          << " of class:" << this->get_class());
              throw std::runtime_error(std::string("unerasable payload ")
                                       + oldpayl->payload_type_name()
                                       + std::string (" cannot be removed from ")
                                       + oid().to_string());
            }
          oldpayl->clear_owner();
        }
      delete oldpayl;
    }
} // end Rps_ObjectZone::clear_payload

Rps_ObjectRef
Rps_ObjectZone::get_class(void) const
{
  return Rps_ObjectRef(ob_class.load());
} // end Rps_ObjectZone::get_class


/// See section "the RefPerSys object model" of
/// refpersys-design.pdf document.
bool
Rps_ObjectZone::is_class(void) const
{
  auto curclass = get_class();
  // for performance, we special-case a few common cases....
  /// most classes are instances of the `class` class
  /// _41OFI3r0S1t03qdB2E, so this is the usual and quick case....
  if (curclass == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E)) // `class` class
    return true;
  if (curclass == RPS_ROOT_OB(_36I1BY2NetN03WjrOv)) // `symbol` class
    return false;
  if (curclass == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)) // `object` class
    return false;
  /// some classes might be instances of yet another metaclass, this is
  /// rare, and we use C++ dynamic cast of payload
  auto curpayl = get_dynamic_payload<Rps_PayloadClassInfo>();
  if (RPS_UNLIKELY(curpayl))
    // the astute reader would notice that C++ dynamic_cast is likely
    // to implement itself the ObjVlisp model, but this is a C++
    // implementation detail.
    return true;
  return false;
} // end Rps_ObjectZone::is_class

bool
Rps_ObjectZone::is_instance_of(Rps_ObjectRef obclass) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  int cnt = 0;
  Rps_ObjectRef obinitclass = obclass;
  Rps_ObjectRef obcurclass = obclass;
  if (!obinitclass || !obinitclass->is_class())
    return false;
  Rps_ObjectRef obthisclass = get_class();
  /// if the heap is severely corrupted, we might loop
  /// indefinitely... This should never happen, but we test against
  /// it...
  for (;;)
    {
      cnt++;
      /// this should not happen, except if our inheritance graph is corrupted
      if (RPS_UNLIKELY(cnt > (int)Rps_Value::maximal_inheritance_depth))
        {
          RPS_WARNOUT("too deep (" << cnt << ") inheritance for " << Rps_ObjectRef(this)
                      << " of class " << obinitclass);
          throw RPS_RUNTIME_ERROR_OUT("too deep (" << cnt << ") inheritance for " << Rps_ObjectRef(this)
                                      << " of class " << obinitclass);
        }
      if (obcurclass == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)) // `object` class
        return true;
      if (obthisclass == obcurclass)
        return true;
      if (obcurclass == RPS_ROOT_OB(_6XLY6QfcDre02922jz)) // `value` class
        return false;
      if (!obcurclass || !obcurclass->is_class())
        return false;
      auto curclasspayl = obcurclass->get_dynamic_payload<Rps_PayloadClassInfo>();
      RPS_ASSERT(curclasspayl);
      obcurclass = curclasspayl->superclass();
    }
} // end Rps_ObjectZone::is_instance_of

bool
Rps_ObjectZone::is_subclass_of(Rps_ObjectRef obsuperclass) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  int cnt = 0;
  Rps_ObjectRef obinitclass = obsuperclass;
  Rps_ObjectRef obcurclass = obsuperclass;
  if (!obinitclass || !obinitclass->is_class())
    return false;
  Rps_ObjectRef obthisclass = get_class();
  /// if the heap is severely corrupted, we might loop
  /// indefinitely... This should never happen, but we test against
  /// it...
  for (;;)
    {
      cnt++;
      /// this should not happen, except if our inheritance graph is corrupted
      if (RPS_UNLIKELY(cnt > (int)Rps_Value::maximal_inheritance_depth))
        {
          RPS_WARNOUT("too deep (" << cnt << ") inheritance for " << Rps_ObjectRef(this)
                      << " of class " << obinitclass);
          throw RPS_RUNTIME_ERROR_OUT("too deep (" << cnt << ") inheritance for " << Rps_ObjectRef(this)
                                      << " of class " << obinitclass);
        }
      if (obcurclass == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)) // `object` class
        return true;
      if (obthisclass == obcurclass)
        return true;
      if (obcurclass == RPS_ROOT_OB(_6XLY6QfcDre02922jz)) // `value` class
        return false;
      if (!obcurclass || !obcurclass->is_class())
        return false;
      auto curclasspayl = obcurclass->get_dynamic_payload<Rps_PayloadClassInfo>();
      RPS_ASSERT(curclasspayl);
      obcurclass = curclasspayl->superclass();
    }
} // end Rps_ObjectZone::is_superclass_of


Rps_ObjectRef
Rps_ObjectZone::get_space(void) const
{
  return Rps_ObjectRef(ob_space.load());
} // end Rps_ObjectZone::get_space

double
Rps_ObjectZone::get_mtime(void) const
{
  return ob_mtime.load();
} // end Rps_ObjectZone::get_mtime

//////////////////////////////////////////////////////////// objects references
Rps_HashInt
Rps_ObjectRef::obhash(void) const
{
  if (is_empty()) return 0;
  return _optr->obhash();
}      // end Rps_ObjectRef::obhash

Rps_ObjectRef
Rps_ObjectRef::root_space()
{
  return RPS_ROOT_OB(_8J6vNYtP5E800eCr5q);
} // end Rps_ObjectRef::root_space

Rps_ObjectRef
Rps_ObjectRef::the_object_class()
{
  return RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ);
} // end Rps_ObjectRef::the_object_class

Rps_ObjectRef
Rps_ObjectRef::the_symbol_class()
{
  return RPS_ROOT_OB(_36I1BY2NetN03WjrOv);
} // end Rps_ObjectRef::the_symbol_class

Rps_ObjectRef
Rps_ObjectRef::the_class_class()
{
  return RPS_ROOT_OB(_41OFI3r0S1t03qdB2E);//class∈class
} // end Rps_ObjectRef::the_class_class

Rps_ObjectRef
Rps_ObjectRef::the_mutable_set_class()
{
  return RPS_ROOT_OB(_0J1C39JoZiv03qA2HA);//mutable_set∈class
} // end Rps_ObjectRef::the_mutable_set_class

void
Rps_ObjectRef::gc_mark(Rps_GarbageCollector&gc) const
{
  if (is_empty()) return;
  _optr->gc_mark(gc);
} // end Rps_ObjectRef::gc_mark

void
Rps_ObjectRef::dump_scan(Rps_Dumper*du, unsigned depth) const
{
  RPS_ASSERT(du != nullptr);
  if (is_empty())
    return;
  _optr->dump_scan(du,depth);
} // end Rps_ObjectRef::dump_scan

Json::Value
Rps_ObjectRef::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (is_empty())
    return Json::Value();
  return _optr->dump_json(du);
} // end Rps_ObjectRef::dump_json

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

Rps_ObjectValue::Rps_ObjectValue(std::nullptr_t)
  : Rps_Value (nullptr, Rps_ValPtrTag{}) {};


//////////////// trees
template<typename RpsTree, Rps_Type treety, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
std::pair<Rps_ObjectZone*,int32_t> const
Rps_TreeZone<RpsTree,treety,k1,k2,k3,k4>::get_metadata(void) const
{
  Rps_HashInt h = val_hash ();
  std::lock_guard<std::mutex> gu(lazy_mtxarr[h % lazy_nbmutexes]);
  Rps_ObjectZone*mobz = metaobject();
  int32_t mr = metarank();
  return std::pair<Rps_ObjectZone*,int32_t> {mobz,mr};
} // end Rps_TreeZone::get_metadata


template<typename RpsTree, Rps_Type treety, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
const Rps_Value
Rps_TreeZone<RpsTree,treety,k1,k2,k3,k4>::at(int rk, bool dontfail) const
{
  if (rk < 0)
    rk += cnt();
  if (rk >= 0 && rk < (int) cnt())
    return _treesons[rk];
  else if (dontfail) return nullptr;
  else throw std::range_error("Rps_TreeZone::at out of range");
} // end Rps_TreeZone::at


template<typename RpsTree, Rps_Type treety, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
void
Rps_TreeZone<RpsTree,treety,k1,k2,k3,k4>::put_metadata(Rps_ObjectRef obr, int32_t num, bool transient)
{
  Rps_HashInt h = val_hash ();
  std::lock_guard<std::mutex> gu(lazy_mtxarr[h % lazy_nbmutexes]);
  Rps_ObjectZone* obz = nullptr;
  if (!obr.is_empty()) obz = *obr;
  _treemetaob.store(obz);
  _treemetarank.store(num);
  _treemetatransient.store(transient);
} // end Rps_TreeZone::put_metadata


template<typename RpsTree, Rps_Type treety, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
std::pair<Rps_ObjectZone*,int32_t>
Rps_TreeZone<RpsTree,treety,k1,k2,k3,k4>::swap_metadata(Rps_ObjectRef obr, int32_t num, bool transient)
{
  Rps_HashInt h = val_hash ();
  std::lock_guard<std::mutex> gu(lazy_mtxarr[h % lazy_nbmutexes]);
  Rps_ObjectZone* obz = nullptr;
  if (!obr.is_empty()) obz = *obr;
  Rps_ObjectZone* oldobz=_treemetaob.exchange(obz);
  int32_t oldnum = _treemetarank.exchange(num);
  _treemetatransient.store(transient);
  return   std::pair<Rps_ObjectZone*,int32_t> {oldobz, oldnum};
} // end Rps_TreeZone::swap_metadata

//////////////// closures

Rps_ClosureValue::Rps_ClosureValue (const Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil)
  : Rps_Value (Rps_ClosureZone::make(connob,valil), Rps_ValPtrTag{})
{
} // end of Rps_ClosureValue::Rps_ClosureValue of initializer_list

Rps_ClosureValue::Rps_ClosureValue (const Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec)
  : Rps_Value (Rps_ClosureZone::make(connob,valvec), Rps_ValPtrTag{})
{
} // end of Rps_ClosureValue::Rps_ClosureValue of vector

Rps_ClosureValue::Rps_ClosureValue(const Rps_Value val)
  : Rps_Value (val.is_closure()?val.as_closure():nullptr, Rps_ValPtrTag{})
{
}; // end Rps_ClosureValue::Rps_ClosureValue dynamic

Rps_ClosureZone*
Rps_ClosureValue::operator -> (void) const
{
  if (is_empty() || !is_closure())
    throw std::runtime_error("empty Rps_ClosureValue");
  return const_cast<Rps_ClosureZone*>(to_closure());
} // end Rps_ClosureValue::operator ->

Rps_ObjectRef
Rps_ClosureValue::connob(void) const
{
  if (is_empty() || !is_closure())
    return nullptr;
  return to_closure()->conn();
};

/////// closure applications
Rps_TwoValues
Rps_ClosureValue::apply0(Rps_CallFrame*callerframe) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return nullptr;
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  Rps_TwoValues res= appfun(callerframe, Rps_Value(nullptr), Rps_Value(nullptr),
                            Rps_Value(nullptr), Rps_Value(nullptr),
                            nullptr);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply0


Rps_TwoValues
Rps_ClosureValue::apply1(Rps_CallFrame*callerframe, const Rps_Value arg0) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return nullptr;
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return  Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  Rps_TwoValues res= appfun(callerframe, arg0, Rps_Value(nullptr),
                            Rps_Value(nullptr), Rps_Value(nullptr),
                            nullptr);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply1



Rps_TwoValues
Rps_ClosureValue::apply2(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    {
      RPS_DEBUG_LOG(MSGSEND, "apply2 " << *this << " no appfun");
      return nullptr;
    }
  callerframe->set_closure(*this);
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            Rps_Value(nullptr), Rps_Value(nullptr),
                            nullptr);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply2


Rps_TwoValues
Rps_ClosureValue::apply3(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return  Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  Rps_Value res= appfun(callerframe, arg0, arg1,
                        arg2, Rps_Value(nullptr),
                        nullptr);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply3


Rps_TwoValues
Rps_ClosureValue::apply4(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2,
                         const Rps_Value arg3) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return nullptr;
  callerframe->set_closure(*this);
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            arg2, arg3,
                            nullptr);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply4



Rps_TwoValues
Rps_ClosureValue::apply5(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2,
                         const Rps_Value arg3, const Rps_Value arg4) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return  Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  std::vector<Rps_Value> restvec(1);
  restvec[0] = arg4;
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            arg2, arg3,
                            &restvec);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply5


Rps_TwoValues
Rps_ClosureValue::apply6(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2,
                         const Rps_Value arg3, const Rps_Value arg4,
                         const Rps_Value arg5) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return nullptr;
  callerframe->set_closure(*this);
  std::vector<Rps_Value> restvec(2);
  restvec[0] = arg4;
  restvec[1] = arg5;
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            arg2, arg3,
                            &restvec);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply6



Rps_TwoValues
Rps_ClosureValue::apply7(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2,
                         const Rps_Value arg3, const Rps_Value arg4,
                         const Rps_Value arg5, const Rps_Value arg6) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return  Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  std::vector<Rps_Value> restvec(3);
  restvec[0] = arg4;
  restvec[1] = arg5;
  restvec[2] = arg6;
  Rps_Value res= appfun(callerframe, arg0, arg1,
                        arg2, arg3,
                        &restvec);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply7



Rps_TwoValues
Rps_ClosureValue::apply8(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2,
                         const Rps_Value arg3, const Rps_Value arg4,
                         const Rps_Value arg5, const Rps_Value arg6,
                         const Rps_Value arg7) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return  Rps_TwoValues(nullptr);
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return nullptr;
  callerframe->set_closure(*this);
  std::vector<Rps_Value> restvec(4);
  restvec[0] = arg4;
  restvec[1] = arg5;
  restvec[2] = arg6;
  restvec[3] = arg7;
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            arg2, arg3,
                            &restvec);
  callerframe->clear_closure();
  return res;
} // end Rps_ClosureValue::apply8


Rps_TwoValues
Rps_ClosureValue::apply9(Rps_CallFrame*callerframe, const Rps_Value arg0,
                         const Rps_Value arg1, const Rps_Value arg2,
                         const Rps_Value arg3, const Rps_Value arg4,
                         const Rps_Value arg5, const Rps_Value arg6,
                         const Rps_Value arg7, const Rps_Value arg8) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return nullptr;
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return  Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  std::vector<Rps_Value> restvec(5);
  restvec[0] = arg4;
  restvec[1] = arg5;
  restvec[2] = arg6;
  restvec[3] = arg7;
  restvec[4] = arg8;
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            arg2, arg3,
                            &restvec);
  callerframe->clear_closure();
  return  res;
} // end Rps_ClosureValue::apply9

Rps_TwoValues
Rps_ClosureValue::apply10(Rps_CallFrame*callerframe, const Rps_Value arg0,
                          const Rps_Value arg1, const Rps_Value arg2,
                          const Rps_Value arg3, const Rps_Value arg4,
                          const Rps_Value arg5, const Rps_Value arg6,
                          const Rps_Value arg7, const Rps_Value arg8,
                          const Rps_Value arg9) const
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  if (is_empty() || !is_closure())
    return nullptr;
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return  Rps_TwoValues(nullptr);
  rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
  if (!appfun)
    return  Rps_TwoValues(nullptr);
  callerframe->set_closure(*this);
  std::vector<Rps_Value> restvec(6);
  restvec[0] = arg4;
  restvec[1] = arg5;
  restvec[2] = arg6;
  restvec[3] = arg7;
  restvec[4] = arg8;
  restvec[5] = arg9;
  Rps_TwoValues res= appfun(callerframe, arg0, arg1,
                            arg2, arg3,
                            &restvec);
  callerframe->clear_closure();
  return  res;
} // end Rps_ClosureValue::apply10



////////////////////////////////////////////////////// immutable instances

Rps_InstanceValue::Rps_InstanceValue (const Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil)
  : Rps_Value (Rps_InstanceZone::make_from_components(connob,valil), Rps_ValPtrTag{})
{
} // end of Rps_InstanceValue::Rps_InstanceValue of initializer_list

Rps_InstanceValue::Rps_InstanceValue (const Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec)
  : Rps_Value (Rps_InstanceZone::make_from_components(connob,valvec), Rps_ValPtrTag{})
{
} // end of Rps_InstanceValue::Rps_InstanceValue of vector

Rps_InstanceValue::Rps_InstanceValue(const Rps_Value val)
  : Rps_Value (val.is_instance()?val.as_instance():nullptr, Rps_ValPtrTag{})
{
}; // end Rps_InstanceValue::Rps_InstanceValue dynamic

Rps_InstanceZone*
Rps_InstanceValue::operator -> (void) const
{
  if (is_empty() || !is_instance())
    throw std::runtime_error("empty Rps_InstanceValue");
  return const_cast<Rps_InstanceZone*>(to_instance());
} // end Rps_InstanceValue::operator ->

/************************** PAYLOADS *************************************/
/*************************************************************************/

Rps_Payload::Rps_Payload(Rps_Type ptyp, Rps_ObjectZone*obz)
  : Rps_QuasiZone(ptyp), payl_owner(obz)
{
  RPS_ASSERT(ptyp <= Rps_Type::Payl__LeastRank);
  RPS_ASSERT(obz != nullptr && obz->stored_type() == Rps_Type::Object);
} // end Rps_Payload::Rps_Payload

Rps_Payload::Rps_Payload(Rps_Type ptyp, Rps_ObjectRef obr)
  : Rps_QuasiZone(ptyp), payl_owner(obr.optr())
{
  RPS_ASSERT(ptyp <= Rps_Type::Payl__LeastRank);
  RPS_ASSERT(obr && obr->stored_type() == Rps_Type::Object);
} // end Rps_Payload::Rps_Payload

////// class information payload - for PaylClassInfo
Rps_PayloadClassInfo::Rps_PayloadClassInfo(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylClassInfo, owner),
    pclass_super(nullptr), pclass_methdict(), pclass_symbname(nullptr), pclass_attrset(nullptr)
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadClassInfo::Rps_PayloadClassInfo

Rps_PayloadClassInfo::Rps_PayloadClassInfo(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylClassInfo, owner, ld),
    pclass_super(nullptr), pclass_methdict(), pclass_symbname(nullptr), pclass_attrset(nullptr)
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadClassInfo::Rps_PayloadClassInfo ..loading


////// space payload - for PaylSpace
Rps_PayloadSpace::Rps_PayloadSpace(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylSpace, owner)
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadSpace::Rps_PayloadSpace

Rps_PayloadSpace::Rps_PayloadSpace(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylSpace, owner, ld)
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadSpace::Rps_PayloadSpace ..loading



////// mutable set of objects payload - for PaylSetOb
Rps_PayloadSetOb::Rps_PayloadSetOb(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylSetOb, owner), psetob()
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadSetOb::Rps_PayloadSetOb

Rps_PayloadSetOb::Rps_PayloadSetOb(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylSetOb, owner, ld),
    psetob()
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadSetOb::Rps_PayloadSetOb ..loading


////// mutable vector of objects payload - for PaylVectOb
Rps_PayloadVectOb::Rps_PayloadVectOb(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylVectOb, owner), pvectob()
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadVectOb::Rps_PayloadVectOb

Rps_PayloadVectOb::Rps_PayloadVectOb(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylVectOb, owner, ld),
    pvectob()
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadVectOb::Rps_PayloadVectOb ..loading






////////////////////////////////////////////////////////////////
///// garbage collector
void
Rps_GarbageCollector::mark_call_stack(Rps_CallFrame*topframe)
{
  for ( Rps_CallFrame*curframe = topframe;
        curframe != nullptr;
        curframe = (curframe->previous_call_frame()))
    {
      RPS_ASSERT(curframe->stored_type() == Rps_Type::CallFrame);
      curframe->gc_mark_frame(this);
    };
} // end Rps_GarbageCollector::mark_call_stack

void
Rps_GarbageCollector::mark_root_value(Rps_Value val)
{
  if (val.is_ptr())
    {
      mark_value(val,0);
      gc_nbroots++;
    };
}      // end Rps_GarbageCollector::mark_root_value

void
Rps_GarbageCollector::mark_root_objectref(Rps_ObjectRef obr)
{
  if (!obr.is_empty())
    {
      mark_obj(obr);
      gc_nbroots++;
    }
}      // end Rps_GarbageCollector::mark_root_objectref


////// agenda payload - for PaylAgenda
Rps_PayloadAgenda::Rps_PayloadAgenda(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylAgenda, owner)
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadAgenda::Rps_PayloadAgenda

Rps_PayloadAgenda::Rps_PayloadAgenda(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylAgenda, owner, ld)
{
  RPS_ASSERT(owner && owner->stored_type() == Rps_Type::Object);
}      // end Rps_PayloadAgenda::Rps_PayloadAgenda ..loading

Rps_ObjectRef
Rps_Agenda::the_agenda()
{
  return RPS_ROOT_OB(_1aGtWm38Vw701jDhZn);
}      // end Rps_Agenda::the_agenda

#endif /*INLINE_RPS_INCLUDED*/
////////////////////////////////////////////////// end of internal header file inline_rps.hh

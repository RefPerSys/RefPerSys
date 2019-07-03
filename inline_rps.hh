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



#ifndef INLINE_RPS_INCLUDED
#define INLINE_RPS_INCLUDED


//// this file should only be included at end of refpersys.hh
#ifndef REFPERSYS_INCLUDED
#error wrong direct inclusion of "inline_rps.hh"
#endif /*REFPERSYS_INCLUDED*/

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

// any type of GC-allocated quasi-value
static inline bool
rps_is_type_of_quasi_value(const Rps_Type ty)
{
  return (ty >= rps_ty_min_quasi && ty < Rps_Type::Int)
         || (ty >= Rps_Type::String && ty <= Rps_Type::Object);
} // end rps_is_type_of_quasi_value

// any type of GC-movable quasi-value
static inline bool
rps_is_type_of_movable_quasi_value(const Rps_Type ty)
{
  return ty == Rps_Type::QuasiAttributeArray
         || ty == Rps_Type::QuasiComponentVector
         || ty == Rps_Type::QuasiObjectVector
         || ty == Rps_Type::Tuple || ty == Rps_Type::Set
         || ty == Rps_Type::Double || ty == Rps_Type::String
         || ty == Rps_Type::QuasiToken;
} // end rps_is_type_of_movable_quasi_value

// any type of GC-allocated quasi value without internal value components
static inline bool
rps_is_type_of_scalar_quasi_value(const Rps_Type ty)
{
  return ty == Rps_Type::Double || ty == Rps_Type::String;
} // end rps_is_type_of_scalar_quasi_value

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

Rps_Value::Rps_Value(tuple_tag, const Rps_TupleObrefZone*ptup) :
  Rps_Value(ptup) {};

const Rps_TupleObrefZone*
Rps_Value::as_tuple() const
{
  if (has_data())
    {
      return const_cast<const Rps_TupleObrefZone*>(unsafe_data()->as<Rps_TupleObrefZone>());
    }
  return nullptr;
} // end Rps_Value::as_tuple

Rps_Value::Rps_Value(set_tag, const Rps_SetObrefZone*pset) :
  Rps_Value(pset) {};

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
      if (da->type() == Rps_Type::Set || da->type() == Rps_Type::Tuple)
        return (const Rps_SequenceObrefZone*)da;
    };
  return nullptr;
} // end Rps_Value::as_sequence

Rps_ObjectRef
Rps_ObjectRef::make(Rps_CallFrameZone*callingfra)
{
  return Rps_ObjectZone::make(callingfra);
}

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



Rps_ZoneValue*
Rps_ZoneValue::scanned_quasivalue(Rps_CallFrameZone*callingfra, unsigned depth)
{
  return Rps_GarbageCollector::scan_quasi_value(callingfra, this, depth);
} // end of Rps_ZoneValue::scanned_quasivalue



////////////////
std::pair<int,int>
Rps_ObjectZone::dichotomy_medium_sorted(Rps_ObjectRef keyob, Rps_QuasiAttributeArray*arr) const
{
  if (arr==nullptr)
    arr = _obat_sorted_atar;
  RPS_ASSERT (_obat_kind == atk_medium);
  RPS_ASSERT (arr != nullptr);
  RPS_ASSERT (keyob);
  unsigned ln = arr->count();
  int lo = 0, hi = (int)ln;
  while (lo+4 < hi)
    {
      unsigned md = (lo+hi)/2;
      Rps_ObjectRef midob = arr->unsafe_attr_at(md);
      RPS_ASSERT (midob);
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
} // end Rps_ObjectZone::dichotomy_medium_sorted



int
Rps_ObjectZone::nb_attrs() const
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
} // end Rps_ObjectZone::nb_attrs

Rps_Value
Rps_ObjectZone::do_get_attr(Rps_ObjectRef keyob) const
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
      RPS_ASSERTPRINTF (ln <= at_sorted_thresh, "ln=%u", ln);
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
}				// end Rps_ObjectZone::do_get_attr

////////////////

void*
Rps_GarbageCollector::allocate_birth_maybe_gc(size_t size, Rps_CallFrameZone*callingfra)
{
  void* ad = nullptr;
  if (RPS_UNLIKELY(size % rps_allocation_unit != 0))
    size = (size | (rps_allocation_unit-1))+1;
  RPS_ASSERTPRINTF (size < RPS_LARGE_BLOCK_SIZE - Rps_LargeNewMemoryBlock::_remain_threshold_ - 4*sizeof(void*),
                    "size=%zd", size);
  RPS_ASSERTPRINTF (size % (2*alignof(Rps_Value)) == 0, "size=%zd", size);
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
  else
    RPS_FATAL("too big size %zd for allocate_birth_maybe_gc", size);
  return ad;
} // end of Rps_GarbageCollector::allocate_birth_maybe_gc

Rps_ZoneValue*
Rps_GarbageCollector::scan_quasi_value(Rps_CallFrameZone*callingfra, Rps_ZoneValue*zv, unsigned depth)
{
  if (!zv || (void*)zv == RPS_EMPTYSLOT) return nullptr;
  auto ty = Rps_ZoneValue::get_type(zv);
  switch (ty)
    {
    case Rps_Type::Dumper:
    case Rps_Type::Loader:
    case Rps_Type::CallFrame:
    case Rps_Type::QuasiPayload:
    case Rps_Type::QuasiToken:
    case Rps_Type::QuasiAttributeArray:
    case Rps_Type::QuasiObjectVector:
    case Rps_Type::QuasiComponentVector:
    case Rps_Type::Int:
    case Rps_Type::None:
    case Rps_Type::String:
    case Rps_Type::Double:
    case Rps_Type::Set:
    case Rps_Type::Tuple:
    case Rps_Type::Object:
      break;
    }
#warning unimplemented Rps_GarbageCollector::scan_quasi_value
  RPS_FATAL("unimplemented Rps_GarbageCollector::scan_quasi_value callingfra@%p zv@%p of ty#%d",
            (void*)callingfra, (void*)zv, (int)ty);
} // end of Rps_GarbageCollector::scan_quasi_value

#endif /*INLINE_RPS_INCLUDED*/
// end of internal header file inline_rps.hh

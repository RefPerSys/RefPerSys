/****************************************************************
 * file values_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to immutable values and quasivalues
 *      See also morevalues_rps.cc file
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2019 - 2025 The Reflective Persistent System Team
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

#include "refpersys.hh"

extern "C" const char rps_values_gitid[];
const char rps_values_gitid[]= RPS_GITID;

extern "C" const char rps_values_date[];
const char rps_values_date[]= __DATE__;

extern "C" const char rps_values_shortgitid[];
const char rps_values_shortgitid[]= RPS_SHORTGITID;

void
Rps_Id::to_cbuf24(char cbuf[]) const
{
  /// example cbuf = "_0abcdefghijABCDEFG"
  ///                  |0         |11    |19
  static_assert(sizeof("_0abcdefghijABCDEFG")-1
                == 1+nbdigits_hi+nbdigits_lo);
  RPS_ASSERT (cbuf != nullptr);
  memset(cbuf, 0, buflen);
  char*last = cbuf+nbdigits_hi;
  auto pc = last;
  cbuf[0] = '_';
  uint64_t n = _id_hi;
  do
    {
      unsigned d = n % base;
      n = n / base;
      *pc = b62digits[d];
      pc--;
    }
  while (pc>cbuf);
  auto start = cbuf+nbdigits_hi;
  last = start+nbdigits_lo;
  pc = last;
  n = _id_lo;
  do
    {
      unsigned d = n % base;
      n = n / base;
      *pc = b62digits[d];
      pc--;
    }
  while (pc>start);
};        // end Rps_Id::to_cbuf24


/// opposite conversion from cbuf to oid
Rps_Id::Rps_Id (const char*cbuf, const char**pend, bool *pok) : Rps_Id ()
{
  auto lasthi = cbuf+nbdigits_hi+1;
  auto lastlo = lasthi + nbdigits_lo;
  uint64_t hi=0, lo=0;
  if (cbuf[0] != '_' && !isdigit(cbuf[1])) goto fail;
  for (auto pcb = cbuf+1; *pcb && pcb<lasthi; pcb++)
    {
      auto pcs = strchr(b62digits, *pcb);
      if (!pcs) goto fail;
      hi = hi*62 + (pcs-b62digits);
    }
  if ((hi > 0 && hi < min_hi) || hi >= max_hi)
    goto fail;
  for (auto pcb = lasthi; *pcb && pcb < lastlo; pcb++)
    {
      auto pcs = strchr(b62digits, *pcb);
      if (!pcs) goto fail;
      lo = lo*62 + (pcs-b62digits);
    }
  if ((lo > 0 && lo < min_lo) || lo >= max_lo)
    goto fail;
  _id_hi = hi;
  _id_lo = lo;
  if (pend)
    *pend = lastlo;
  if (pok)
    *pok = true;
  return;
fail:
  if (pend)
    *pend = cbuf;
  if (pok)
    *pok = false;
  return;
} // end Rps_Id::Rps_Id (const char*, char**, bool*)



//////////////////////////////////////////////// quasi values


std::recursive_mutex Rps_QuasiZone::qz_mtx;
std::vector<Rps_QuasiZone*> Rps_QuasiZone::qz_zonvec(100);
uint32_t Rps_QuasiZone::qz_cnt;
std::atomic<uint64_t> Rps_QuasiZone::qz_alloc_cumulw;

void
Rps_QuasiZone::initialize(void)
{
  static bool inited;
  if (inited) return;
  inited = true;
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
  qz_zonvec.reserve(100);
  qz_zonvec.push_back(nullptr);
} // end Rps_QuasiZone::initialize




Rps_QuasiZone::~Rps_QuasiZone()
{
  unregister_in_zonevec();
} // end of Rps_QuasiZone::~Rps_QuasiZone

void
Rps_QuasiZone::register_in_zonevec(void)
{
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
  if (RPS_UNLIKELY(9 * qz_zonvec.capacity() <= 8 * qz_zonvec.size()))
    {
      auto newcap = rps_prime_above(19*qz_zonvec.size()/16
                                    + 200);
      qz_zonvec.reserve(newcap);
    }
  if (RPS_LIKELY(qz_zonvec.size() > 128))
    {
      uint32_t rk = 1 + Rps_Random::random_32u() % (qz_zonvec.size() - 16);
      uint32_t endrk = rk + 24;
      if (endrk > (uint32_t)qz_zonvec.size())
        endrk = (uint32_t)qz_zonvec.size();
      for (uint32_t ix= rk; ix < endrk; ix++)
        if (qz_zonvec[ix] == nullptr)
          {
            qz_zonvec[ix] = this;
            this->qz_rank = ix;
            qz_cnt++;
            return;
          }
    }
  this->qz_rank = (uint32_t)qz_zonvec.size();
  qz_cnt++;
  qz_zonvec.push_back(this);
} // end of Rps_QuasiZone::register_in_zonevec

void
Rps_QuasiZone::unregister_in_zonevec(void)
{
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
  RPS_ASSERT(this->qz_rank>0
             && this->qz_rank < (uint32_t)qz_zonvec.size());
  RPS_ASSERT(qz_cnt>0 && qz_cnt<(uint32_t)qz_zonvec.size());
  RPS_ASSERT(qz_zonvec[this->qz_rank] == this);
  qz_zonvec[this->qz_rank] = nullptr;
  qz_cnt--;
} // end of Rps_QuasiZone::unregister_in_zonevec

void
Rps_QuasiZone::clear_all_gcmarks(Rps_GarbageCollector&gc)
{
  std::lock_guard<std::recursive_mutex> gu(qz_mtx);
  for (Rps_QuasiZone *qz : qz_zonvec)
    {
      if (!qz) continue;
      qz->clear_gcmark(gc);
    }
} // end of Rps_QuasiZone::clear_all_gcmarks


std::mutex Rps_LazyHashedZoneValue::lazy_mtxarr[Rps_LazyHashedZoneValue::lazy_nbmutexes];


/* Printing routine likely to be called by GDB */
void
rps_print_value(const Rps_Value val)
{
  std::cout << Rps_OutputValue(val, 0, Rps_Value::debug_maxdepth) << std::endl;
} // end rps_print_value

void
rps_print_ptr_value(const void*v)
{
  static_assert(sizeof(Rps_Value) == sizeof(v));
  Rps_Value val;
  memcpy((void*)&val, (const void*)&v, sizeof(v));
  std::cout << Rps_OutputValue(val, 0, Rps_Value::debug_maxdepth) << std::endl;
} // end rps_print_ptr_value

void
rps_limited_print_value(const Rps_Value val, unsigned depth, unsigned maxdepth)
{
  std::cout << Rps_OutputValue(val, depth, maxdepth) << std::endl;
} // end rps_limited_print_value

void
rps_limited_print_ptr_value(const void*v, unsigned depth, unsigned maxdepth)
{
  static_assert(sizeof(Rps_Value) == sizeof(v));
  Rps_Value val;
  memcpy((void*)&val, (const void*)&v, sizeof(v));
  std::cout << Rps_OutputValue(val, depth, maxdepth) << std::endl;
} // end rps_limited_print_ptr_value

//////////////////////////////////////////////// sets

Rps_SetOb::Rps_SetOb(const std::set<Rps_ObjectRef>& setob, Rps_SetTag)
  : Rps_SetOb::Rps_SetOb((unsigned) setob.size(), Rps_SetTag{})
{
  int ix=0;
  for (auto ob : setob)
    {
      RPS_ASSERT (ob);
      _seqob[ix++] = ob;
    }
} // end Rps_SetOb::Rps_SetOb


Rps_SetOb Rps_SetOb::_setob_emptyset_(0,Rps_SetTag{});

const Rps_SetOb*
Rps_SetOb::make(const std::set<Rps_ObjectRef>& setob)
{
  auto setsiz = setob.size();
  if (RPS_UNLIKELY(setsiz >= Rps_SeqObjRef::maxsize))
    throw std::length_error("Rps_SetOb::make with too many elements");
  for (auto ob : setob)
    if (RPS_UNLIKELY(!ob))
      throw std::invalid_argument("empty element to Rps_SetOb::make");
  return
    rps_allocate_with_wordgap<Rps_SetOb,const std::set<Rps_ObjectRef>&,Rps_SetTag>
    (setsiz,setob,Rps_SetTag{});
} // end of Rps_SetOb::make with set




const Rps_SetOb*
Rps_SetOb::make(const std::initializer_list<Rps_ObjectRef>&elemil)
{
  std::set<Rps_ObjectRef>elemset;
  for (auto elem: elemil)
    if (elem)
      elemset.insert(elem);
  return make(elemset);
} // end of Rps_SetOb::make with initializer_list



const Rps_SetOb*
Rps_SetOb::make(const std::vector<Rps_ObjectRef>&vecob)
{
  std::set<Rps_ObjectRef>elemset;
  for (auto ob: vecob)
    if (ob)
      elemset.insert(ob);
  return make(elemset);
} // end of Rps_SetOb::make with vector


const Rps_SetOb*
Rps_SetOb::collect(const std::vector<Rps_Value>&vecval)
{
  std::set<Rps_ObjectRef>elemset;
  for (auto val: vecval)
    {
      if (val.is_object())
        elemset.insert(Rps_ObjectRef(val.as_object()));
      else if (val.is_tuple())
        {
          auto tup = val.as_tuple();
          for (auto ob: *tup)
            if (ob)
              elemset.insert(ob);
        }
      else if (val.is_set())
        {
          auto set = val.as_set();
          for (auto ob: *set)
            elemset.insert(ob);
        }
    }
  return make(elemset);
} // end of Rps_SetOb::collect with vector



const Rps_SetOb*
Rps_SetOb::collect(const std::initializer_list<Rps_Value>&ilval)
{
  std::set<Rps_ObjectRef>elemset;
  for (auto val: ilval)
    {
      if (val.is_object())
        elemset.insert(Rps_ObjectRef(val.as_object()));
      else if (val.is_tuple())
        {
          auto tup = val.as_tuple();
          for (auto ob: *tup)
            if (ob)
              elemset.insert(ob);
        }
      else if (val.is_set())
        {
          auto set = val.as_set();
          for (auto ob: *set)
            elemset.insert(ob);
        }
    }
  return make(elemset);
} // end of Rps_SetOb::collect with initializer_list





void
Rps_SetOb::val_output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth>maxdepth)
    {
      out << "??";
      return;
    };
  out << "{";
  int cnt=0;
  for (Rps_ObjectRef ob: *this)
    {
      if (cnt>0) out <<", ";
      ob.output(out, depth+1, maxdepth);
      cnt++;
    }
  out << "}";
} // end Rps_SetOb::val_output


Rps_ObjectRef
Rps_SetOb::compute_class(Rps_CallFrame*) const
{
  return RPS_ROOT_OB(_6JYterg6iAu00cV9Ye); // the `set` class
} // end Rps_SetOb::compute_class


void
Rps_SetOb::repeat_increasing_each_element_until(Rps_CallFrame*cf, void*data,
    const std::function<bool(Rps_CallFrame*,void*/*data*/,Rps_ObjectRef/*elem*/)>& func) const
{
  RPS_ASSERT (cf==nullptr || cf->is_good_call_frame());
  RPS_ASSERT (this->type() == Rps_Type::Set);
  unsigned card = cnt();
  if (card > 0)
    {
      const Rps_ObjectRef* arr = raw_const_data();
      for (unsigned ix=0; ix<card; ix++)
        if (func(cf,data,arr[ix]))
          return;
    };
} // end Rps_SetOb::repeat_increasing_each_element_until

void
Rps_SetOb::repeat_decreasing_each_element_until(Rps_CallFrame*cf, void*data,
    const std::function<bool(Rps_CallFrame*,void*/*data*/,Rps_ObjectRef/*elem*/)>& func) const
{
  RPS_ASSERT (cf==nullptr || cf->is_good_call_frame());
  RPS_ASSERT (this->type() == Rps_Type::Set);
  unsigned card = cnt();
  if (card > 0)
    {
      const Rps_ObjectRef* arr = raw_const_data();
      for (int ix=(int)card-1; ix>=0; ix--)
        if (func(cf,data,arr[ix]))
          return;
    };
} // end Rps_SetOb::repeat_decreasing_each_element_each_element_until

//////////////////////////////////////// tuples
const Rps_TupleOb*
Rps_TupleOb::make(const std::vector<Rps_ObjectRef>& vecob)
{
  unsigned nbob = 0;
  {
    for (auto ob: vecob)
      if (ob)
        nbob++;
  }
  if (RPS_UNLIKELY(nbob > maxsize))
    throw std::length_error("Rps_TupleOb::make too many objects");
  if (RPS_LIKELY(nbob == vecob.size()))
    {
      auto tup =
        rps_allocate_with_wordgap<Rps_TupleOb, unsigned, Rps_TupleTag>
        (nbob, nbob, Rps_TupleTag{});
      auto rd = tup->raw_data();
      for (int ix=0; ix<(int)nbob; ix++) rd[ix] = vecob[ix];
      return tup;
    }
  else
    {
      std::vector<Rps_ObjectRef> vec;
      vec.reserve(nbob);
      for (auto ob: vecob)
        if (ob)
          vec.push_back(ob);
      return make(vec);
    }
} // end Rps_TupleOb::make from vector

const Rps_TupleOb*
Rps_TupleOb::make(const std::initializer_list<Rps_ObjectRef>&compil)
{
  std::vector<Rps_ObjectRef> vec(compil);
  return make(vec);
} // end of Rps_TupleOb::make from initializer_list

const Rps_TupleOb*
Rps_TupleOb::collect(const std::vector<Rps_Value>& vecval)
{
  unsigned nbcomp = 0;
  for (auto v : vecval)
    {
      if (v.is_object() && v.to_object() != nullptr)
        nbcomp++;
      else if (v.is_tuple())
        nbcomp += v.to_tuple()->cnt();
      else if (v.is_set())
        nbcomp += v.to_set()->cnt();
      else continue;
      if (RPS_UNLIKELY(nbcomp > maxsize))
        throw std::length_error("Rps_TupleOb::collect too many objects");
    }
  std::vector<Rps_ObjectRef> vecob;
  vecob.reserve(nbcomp);
  for (auto v : vecval)
    {
      if (v.is_object())
        {
          auto ob = v.to_object();
          if (ob)
            vecob.push_back(ob);
        }
      else if (v.is_tuple())
        {
          auto tup = v.to_tuple();
          for (auto ob : *tup)
            if (ob)
              vecob.push_back(ob);
        }
      else if (v.is_set())
        {
          auto set = v.to_set();
          for (auto ob : *set)
            if (ob)
              vecob.push_back(ob);
        }
      else continue;
    }
  return make(vecob);
} // end of Rps_TupleOb::collect from vector


const Rps_TupleOb*
Rps_TupleOb::collect(const std::initializer_list<Rps_Value>&valil)
{
  std::vector<Rps_Value> vecval(valil);
  return collect(vecval);
} // end Rps_TupleOb::collect from initializer_list



Rps_ObjectRef
Rps_TupleOb::compute_class( Rps_CallFrame*) const
{
  return RPS_ROOT_OB(_6NVM7sMcITg01ug5TC); // the `tuple` class
} // end Rps_TupleOb::compute_class

void
Rps_TupleOb::val_output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth > maxdepth)
    {
      out << "??";
      return;
    };
  out << "[";
  int cnt=0;
  for (Rps_ObjectRef ob: *this)
    {
      if (cnt>0) out <<", ";
      ob.output(out,depth+1,maxdepth);
      cnt++;
    }
  out << "]";
} // end Rps_TupleOb::val_output

////////////////////////////////////////////////// closures

Rps_ClosureZone*
Rps_ClosureZone::make(Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil)
{
  if (!connob)
    return nullptr;
  auto nbsons = valil.size();
  Rps_ClosureZone* cloz
    = Rps_QuasiZone::rps_allocate_with_wordgap<Rps_ClosureZone,unsigned,Rps_ObjectRef,Rps_ClosureTag>((nbsons*sizeof(Rps_Value)/sizeof(void*)),
        (unsigned)nbsons, connob,  Rps_ClosureTag{});
  int ix=0;
  Rps_Value*sonarr = cloz->raw_data_sons();
  for (auto val: valil)
    sonarr[ix++] = val;
  return cloz;
} // end ClosureZone::make

Rps_ClosureZone*
Rps_ClosureZone::make(Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec)
{
  if (!connob)
    return nullptr;
  auto nbsons = valvec.size();
  Rps_ClosureZone* cloz
    = Rps_QuasiZone::rps_allocate_with_wordgap<Rps_ClosureZone,unsigned,Rps_ObjectRef,Rps_ClosureTag>((nbsons*sizeof(Rps_Value)/sizeof(void*)),
        (unsigned)nbsons, connob, Rps_ClosureTag{});
  int ix=0;
  Rps_Value*sonarr = cloz->raw_data_sons();
  for (auto val: valvec)
    sonarr[ix++] = val;
  return cloz;
} // end ClosureZone::make


Rps_TwoValues
Rps_ClosureValue::apply_vect(Rps_CallFrame*callerframe, const std::vector<Rps_Value>& argvec) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  if (is_empty() || !is_closure())
    return nullptr;
  Rps_ObjectRef obconn = connob();
  if (!obconn)
    return nullptr;
  auto arity = argvec.size();
  switch (arity)
    {
    case 0:
      return apply0(callerframe);
    case 1:
      return apply1(callerframe, argvec[0]);
    case 2:
      return apply2(callerframe, argvec[0], argvec[1]);
    case 3:
      return apply3(callerframe, argvec[0], argvec[1], argvec[2]);
    case 4:
      return apply4(callerframe, argvec[0], argvec[1], argvec[2], argvec[3]);
    default:
    {
      rps_applyingfun_t*appfun = obconn->get_applyingfun(*this);
      if (!appfun)
        return nullptr;
      std::vector<Rps_Value> restvec(arity-4);
      for (unsigned ix=4; ix<(unsigned)arity; ix++)
        restvec[ix-4] = argvec[ix];
      callerframe->set_closure(*this);
      Rps_Value res= appfun(callerframe,argvec[0], argvec[1], argvec[2], argvec[3], &restvec);
      callerframe->clear_closure();
      return res;
    }
    }
} // end Rps_ClosureValue::apply_vect

Rps_TwoValues
Rps_ClosureValue::apply_ilist(Rps_CallFrame*callerframe, const std::initializer_list<Rps_Value>& argil) const
{
  std::vector <Rps_Value> argvec(argil);
  return apply_vect(callerframe,argvec);
} // end Rps_ClosureValue::apply_ilist


void
Rps_ClosureZone::val_output(std::ostream&out, unsigned  depth, unsigned maxdepth) const
{
  if (depth>maxdepth)
    {
      out << "??";
      return;
    };
  out << "%";
  conn().output(out, depth+1, maxdepth);
  if (depth > Rps_Value::max_output_depth || depth>maxdepth)
    {
      out << "(...)";
    }
  else
    {
      out << "(";
      int cnt=0;
      for (auto val: *this)
        {
          if (cnt>0) out <<", ";
          val.output(out, depth+1, maxdepth);
          cnt++;
        }
      out << ")";
    }
  if (depth<2)
    out << std::flush;
} // end Rps_ClosureZone::val_output


Rps_ObjectRef
Rps_ClosureZone::compute_class(Rps_CallFrame*) const
{
  return RPS_ROOT_OB(_4jISxMJ4PYU0050nUl); // the `closure` class
} // end Rps_ClosureZone::compute_class

//////////////// attributes

Rps_Value
Rps_Value::get_attr(Rps_CallFrame*stkf, const Rps_ObjectRef obattr) const
{
  // in principle, obattr type is always Object, but we need to be
  // absolutely sure, even in case of bugs, so we do check it
  if (obattr.is_empty() || obattr->stored_type() != Rps_Type::Object)
    return nullptr;
  rps_magicgetterfun_t*getfun = obattr->ob_magicgetterfun.load();
  if (getfun)
    return (*getfun)(stkf, *this, obattr);
  if (is_object())
    {
      const Rps_ObjectZone*thisob = as_object();
      std::lock_guard gu(thisob->ob_mtx);
      auto it = thisob->ob_attrs.find(obattr);
      if (it != thisob->ob_attrs.end())
        return it->second;
    };
  return nullptr;
} // end Rps_Value::get_attr


Rps_ObjectRef
Rps_Value::compute_class(Rps_CallFrame*stkf) const
{
  if (is_empty())
    return nullptr;
  else if (is_int())
    return RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b); // the `int` class
  else if (is_string())
    return RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); // the `string` class
  else if (is_double())
    return RPS_ROOT_OB(_98sc8kSOXV003i86w5); // the `double` class
  else if (is_tuple())
    return RPS_ROOT_OB(_6NVM7sMcITg01ug5TC); // the `tuple` class
  else if (is_set())
    return RPS_ROOT_OB(_6JYterg6iAu00cV9Ye); // the `set` class
  else if (is_closure())
    return RPS_ROOT_OB(_4jISxMJ4PYU0050nUl); // the `closure` class
  else if (is_ptr())
    return as_ptr()->compute_class(stkf);
  return nullptr;
} // end Rps_Value::compute_class

// the below member function computes, for the current value, the
// closure for the RefPerSys method of selector obselector. It is so
// important that it deserves a describing symbol of its own.
Rps_ClosureValue
Rps_Value::closure_for_method_selector(Rps_CallFrame*callerframe, Rps_ObjectRef obselectorarg) const
{
  // our frame descriptor is the `closure_for_method_selector` symbol
  RPS_LOCALFRAME(RPS_ROOT_OB(_6JbWqOsjX5T03M1eGM),
                 callerframe,
                 Rps_Value val; // the current value
                 Rps_ObjectRef obselect; // the attribute
                 Rps_ObjectRef obcurclass; // the current class
                 Rps_ClosureValue closval; // the resulting closure
                );
  _f.val = Rps_Value(*this);
  _f.obselect = obselectorarg;
  _f.obcurclass = _f.val.compute_class(&_);
  int loopcount = 0;
  RPS_DEBUG_LOG(MSGSEND, "closure_for_method_selector start val=" << _f.val
                << " obcurclass=" << _f.obcurclass
                << " obselect=" << _f.obselect);
  RPS_ASSERT(RPS_ROOT_OB(_6XLY6QfcDre02922jz)); // the `value` class exists, it has been loaded
  while (loopcount++ <  (int)maximal_inheritance_depth)
    {
      RPS_DEBUG_LOG(MSGSEND, "closure_for_method_selector obcurclass=" << _f.obcurclass << " obselect=" << _f.obselect << " loopcount=" << loopcount);
      if (!_f.obcurclass)   // should never happen
        {
          if (_f.val.is_object())
            RPS_FATALOUT("object @" << (void*)_f.val.unsafe_wptr()
                         << " of oid " << _f.val.to_object()->oid()
                         << " has no class");
          else
            RPS_FATALOUT("value @" << (void*)_f.val.unsafe_wptr()
                         << " of type#" << (int)(_f.val.to_ptr()?_f.val.to_ptr()->stored_type():Rps_Type::None) << " has no class");
        }
      std::lock_guard<std::recursive_mutex> gucurclass(*(_f.obcurclass->objmtxptr()));
      if (_f.obcurclass == RPS_ROOT_OB(_6XLY6QfcDre02922jz) // the topmost `value` class ends the loop
         )
        {
          /// if the `value` class is not a genuine RefPerSys class, it
          /// means that RefPerSys persistent heap is broken beyond
          /// repair.  So the asserts below are always satisfied.
          auto valclasspayl = reinterpret_cast<Rps_PayloadClassInfo*>(_f.obcurclass->get_payload());
          RPS_ASSERT(valclasspayl != nullptr);
          // the below check is faster than a C++ dynamic_cast:
          RPS_ASSERT(valclasspayl->stored_type() == Rps_Type::PaylClassInfo);
          _f.closval = valclasspayl->get_own_method(_f.obselect);
          RPS_DEBUG_LOG(MSGSEND, "closure_for_method_selector!value closval=" << _f.closval);
          if (_f.closval && _f.closval.is_closure()) // should be always true! But we need to check
            return _f.closval;
          else
            return Rps_ClosureValue(nullptr);
        }
      /// usual common case:
      if (_f.obcurclass->get_class() == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) // the `class` class
         )
        {
          auto valclasspayl = reinterpret_cast<Rps_PayloadClassInfo*>(_f.obcurclass->get_payload());
          RPS_ASSERT(valclasspayl != nullptr);
          // the below check is faster than a C++ dynamic_cast:
          RPS_ASSERT(valclasspayl->stored_type() == Rps_Type::PaylClassInfo);
          _f.closval = valclasspayl->get_own_method(_f.obselect);
          RPS_DEBUG_LOG(MSGSEND, "closure_for_method_selector!class closval=" << _f.closval);
          if (_f.closval && _f.closval.is_closure()) // should be always true! But we need to check
            return _f.closval;
          else
            {
              _f.obcurclass = valclasspayl->superclass();
              continue;
            }
        }
      /// special case for dynamic_cast, which is unlikely to happen
      else if (auto valclasspayl = _f.obcurclass->get_dynamic_payload<Rps_PayloadClassInfo>())
        {
          // could happen latter when _f.obcurclass is a subclass of `class` but unlikely
          RPS_ASSERT(valclasspayl != nullptr);
          // the below check is faster than a C++ dynamic_cast:
          RPS_ASSERT(valclasspayl->stored_type() == Rps_Type::PaylClassInfo);
          _f.closval = valclasspayl->get_own_method(_f.obselect);
          RPS_DEBUG_LOG(MSGSEND, "closure_for_method_selector!sub-class closval=" << _f.closval);
          if (_f.closval && _f.closval.is_closure()) // should be always true! But we need to check
            return _f.closval;
          else
            {
              _f.obcurclass = valclasspayl->superclass();
              continue;
            }
        }
    };
  /// we should never reach this point.... if we do, the persistent heap was corrupted...
  _f.obcurclass = _f.val.compute_class(&_);
  RPS_FATALOUT("failed to compute closure_for_method_selector for value " <<
               _f.val << " of class " << _f.obcurclass
               << " for selector " << _f.obselect);
} // end of Rps_Value::closure_for_method_selector





////////////////////////////////////////////////////////////////
//////////////// message sending protocol //////////////////////
////////////////////////////////////////////////////////////////

Rps_TwoValues
Rps_Value::send0(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  RPS_DEBUG_LOG(MSGSEND, "send0 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send0 selfv=" << _f.selfv
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply1(&_, _f.selfv);
  else
    RPS_DEBUG_LOG(MSGSEND, "send0 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send0



Rps_TwoValues
Rps_Value::send1(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 Rps_Value arg0) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  RPS_DEBUG_LOG(MSGSEND, "send1 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send1 selfv=" << _f.selfv
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply2(&_, _f.selfv, _f.arg0v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send1 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send1


Rps_TwoValues
Rps_Value::send2(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 Rps_Value arg0, const Rps_Value arg1) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  RPS_DEBUG_LOG(MSGSEND, "send2 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send2 selfv=" << _f.selfv
                << ", obsel=" << _f.obsel
                << ", closv=" << _f.closv);

  if (_f.closv.is_closure())
    {
      RPS_DEBUG_LOG(MSGSEND, "send2 applying to selfv=" << _f.selfv
                    << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                    << "… obsel=" << _f.obsel
                    << ", closv=" << _f.closv
                    << ", arg0v=" << _f.arg0v
                    << ", arg1v=" << _f.arg1v);
      return _f.closv.apply3(&_, _f.selfv, _f.arg0v, _f.arg1v);
    }
  else
    RPS_DEBUG_LOG(MSGSEND, "send2 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send2


Rps_TwoValues
Rps_Value::send3(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1, const Rps_Value arg2) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  RPS_DEBUG_LOG(MSGSEND, "send3 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send3 selfv=" << _f.selfv
                << ", obsel=" << _f.obsel
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply4(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send3 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send3



Rps_TwoValues
Rps_Value::send4(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1,
                 const Rps_Value arg2, const Rps_Value arg3) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                 Rps_Value arg3v; // the argument#3
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  _f.arg3v = arg3;
  RPS_DEBUG_LOG(MSGSEND, "send4 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v
                << ", arg3v=" << _f.arg3v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send4 selfv=" << _f.selfv
                << ", obsel=" << _f.obsel
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply5(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v, _f.arg3v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send4 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send4



Rps_TwoValues
Rps_Value::send5(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1,
                 const Rps_Value arg2, const Rps_Value arg3,
                 const Rps_Value arg4) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                 Rps_Value arg3v; // the argument#3
                 Rps_Value arg4v; // the argument#4
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  _f.arg3v = arg3;
  _f.arg4v = arg4;
  RPS_DEBUG_LOG(MSGSEND, "send5 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v
                << ", arg3v=" << _f.arg3v
                << ", arg4v=" << _f.arg4v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send5 selfv=" << _f.selfv
                << ", obsel=" << _f.obsel
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply6(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v, _f.arg3v, _f.arg4v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send5 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send5


Rps_TwoValues
Rps_Value::send6(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1,
                 const Rps_Value arg2, const Rps_Value arg3,
                 const Rps_Value arg4, const Rps_Value arg5) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                 Rps_Value arg3v; // the argument#3
                 Rps_Value arg4v; // the argument#4
                 Rps_Value arg5v; // the argument#5
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  _f.arg3v = arg3;
  _f.arg4v = arg4;
  _f.arg5v = arg5;
  RPS_DEBUG_LOG(MSGSEND, "send6 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v
                << ", arg3v=" << _f.arg3v
                << ", arg4v=" << _f.arg4v
                << ", arg5v=" << _f.arg5v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send6 selfv=" << _f.selfv
                << ", obsel=" << _f.obsel
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply7(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v, _f.arg3v, _f.arg4v, _f.arg5v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send6 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send6


Rps_TwoValues
Rps_Value::send7(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1,
                 const Rps_Value arg2, const Rps_Value arg3,
                 const Rps_Value arg4, const Rps_Value arg5,
                 const Rps_Value arg6) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                 Rps_Value arg3v; // the argument#3
                 Rps_Value arg4v; // the argument#4
                 Rps_Value arg5v; // the argument#5
                 Rps_Value arg6v; // the argument#6
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  _f.arg3v = arg3;
  _f.arg4v = arg4;
  _f.arg5v = arg5;
  _f.arg6v = arg6;
  RPS_DEBUG_LOG(MSGSEND, "send7 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v
                << ", arg3v=" << _f.arg3v
                << ", arg4v=" << _f.arg4v
                << ", arg5v=" << _f.arg5v
                << ", arg6v=" << _f.arg6v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  RPS_DEBUG_LOG(MSGSEND, "send7 selfv=" << _f.selfv
                << ", obsel=" << _f.obsel
                << ", closv=" << _f.closv);
  if (_f.closv.is_closure())
    return _f.closv.apply8(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v, _f.arg3v, _f.arg4v, _f.arg5v, _f.arg6v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send7 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send7


Rps_TwoValues
Rps_Value::send8(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1,
                 const Rps_Value arg2, const Rps_Value arg3,
                 const Rps_Value arg4, const Rps_Value arg5,
                 const Rps_Value arg6, const Rps_Value arg7) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                 Rps_Value arg3v; // the argument#3
                 Rps_Value arg4v; // the argument#4
                 Rps_Value arg5v; // the argument#5
                 Rps_Value arg6v; // the argument#6
                 Rps_Value arg7v; // the argument#7
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  _f.arg3v = arg3;
  _f.arg4v = arg4;
  _f.arg5v = arg5;
  _f.arg6v = arg6;
  _f.arg7v = arg7;
  RPS_DEBUG_LOG(MSGSEND, "send8 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v
                << ", arg3v=" << _f.arg3v
                << ", arg4v=" << _f.arg4v
                << ", arg5v=" << _f.arg5v
                << ", arg6v=" << _f.arg6v
                << ", arg7v=" << _f.arg7v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  if (_f.closv.is_closure())
    return _f.closv.apply9(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v, _f.arg3v, _f.arg4v, _f.arg5v, _f.arg6v, _f.arg7v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send8 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send8


Rps_TwoValues
Rps_Value::send9(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                 const Rps_Value arg0, const Rps_Value arg1,
                 const Rps_Value arg2, const Rps_Value arg3,
                 const Rps_Value arg4, const Rps_Value arg5,
                 const Rps_Value arg6, const Rps_Value arg7,
                 const Rps_Value arg8) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value arg0v; // the argument#0
                 Rps_Value arg1v; // the argument#1
                 Rps_Value arg2v; // the argument#2
                 Rps_Value arg3v; // the argument#3
                 Rps_Value arg4v; // the argument#4
                 Rps_Value arg5v; // the argument#5
                 Rps_Value arg6v; // the argument#6
                 Rps_Value arg7v; // the argument#7
                 Rps_Value arg8v; // the argument#8
                );
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  _f.arg0v = arg0;
  _f.arg1v = arg1;
  _f.arg2v = arg2;
  _f.arg3v = arg3;
  _f.arg4v = arg4;
  _f.arg5v = arg5;
  _f.arg6v = arg6;
  _f.arg7v = arg7;
  _f.arg8v = arg8;
  RPS_DEBUG_LOG(MSGSEND, "send9 selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << ", arg0v=" << _f.arg0v
                << ", arg1v=" << _f.arg1v
                << ", arg2v=" << _f.arg2v
                << ", arg3v=" << _f.arg3v
                << ", arg4v=" << _f.arg4v
                << ", arg5v=" << _f.arg5v
                << ", arg6v=" << _f.arg6v
                << ", arg7v=" << _f.arg7v
                << ", arg8v=" << _f.arg8v);
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  if (_f.closv.is_closure())
    return _f.closv.apply10(&_, _f.selfv, _f.arg0v, _f.arg1v, _f.arg2v, _f.arg3v, _f.arg4v, _f.arg5v, _f.arg6v, _f.arg7v, _f.arg8v);
  else
    RPS_DEBUG_LOG(MSGSEND, "send9 applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send9


Rps_TwoValues
Rps_Value::send_vect(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                     const std::vector<Rps_Value>& argvecarg) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                );
  std::vector<Rps_Value> argvect(argvecarg);
  _.set_additional_gc_marker
  ([=](Rps_GarbageCollector*gc)
  {
    for (auto v: argvect) gc->mark_value(v);
  });
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  RPS_DEBUG_LOG(MSGSEND, "send_vect selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << " argvecarg.size=" << argvecarg.size());
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  if (_f.closv.is_closure())
    {
      argvect.insert(argvect.begin(), _f.selfv);
      return _f.closv.apply_vect(&_, argvect);
    }
  else
    RPS_DEBUG_LOG(MSGSEND, "send_vect applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << ".... non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send_vect


Rps_TwoValues
Rps_Value::send_ilist(Rps_CallFrame*callerframe, const Rps_ObjectRef obselarg,
                      const std::initializer_list<Rps_Value>& argilarg) const
{
  //RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yQcFbU0seU018B48Z), // `message_sending` symbol
                 callerframe,
                 Rps_Value selfv; // the receiver
                 Rps_ClosureValue closv; // the closure
                 Rps_ObjectRef obsel; // the selector
                );
  std::vector<Rps_Value> argvec(argilarg);
  _.set_additional_gc_marker
  ([=](Rps_GarbageCollector*gc)
  {
    for (auto v: argvec) gc->mark_value(v);
  });
  _f.selfv = Rps_Value(*this);
  _f.obsel = obselarg;
  RPS_DEBUG_LOG(MSGSEND, "send_ilist selfv=" << _f.selfv
                << " of class:" <<  _f.selfv.compute_class(&_)
                << ", obsel=" << _f.obsel
                << " argilarg.size=" << argilarg.size());
  _f.closv = _f.selfv.closure_for_method_selector(&_,_f.obsel);
  if (_f.closv.is_closure())
    {
      argvec.insert(argvec.begin(), _f.selfv);
      return _f.closv.apply_vect(&_, argvec);
    }
  else
    RPS_DEBUG_LOG(MSGSEND, "send_ilist applying selfv=" << _f.selfv
                  << " of class:" <<  _f.selfv.compute_class(&_) << std::endl
                  << "… with obsel=" << _f.obsel
                  << " of class:" <<  _f.obsel->compute_class(&_) << std::endl
                  << "… non closure closv=" << _f.closv);
  return Rps_TwoValues(nullptr,nullptr);
} // end Rps_Value::send_ilist

std::ostream&
operator << (std::ostream&out, const std::vector<Rps_Value>& vect)
{
  out << "(|";
  auto sz = vect.size();
  for (unsigned ix=0; ix<(unsigned)sz; ix++)
    {
      if (ix>0) out << ",";
      if (ix % 4 == 0 && ix > 0) out << std::endl << " ";
      vect[ix].output(out,1);
    }
  out << "|)";
  return out;
};        // end operator << for std::vector<Rps_Value>

/* end of file value_rps.cc */


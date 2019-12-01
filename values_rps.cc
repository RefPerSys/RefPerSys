/****************************************************************
 * file values_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to immutable values and quasivalues
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

#include "refpersys.hh"

extern "C" const char rps_values_gitid[];
const char rps_values_gitid[]= RPS_GITID;

extern "C" const char rps_values_date[];
const char rps_values_date[]= __DATE__;


void
Rps_Id::to_cbuf24(char cbuf[]) const
{
  /// example cbuf = "_0abcdefghijABCDEFG"
  ///                  |0         |11    |19
  static_assert(sizeof("_0abcdefghijABCDEFG")-1 == 1+nbdigits_hi+nbdigits_lo);
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
};				// end Rps_Id::to_cbuf24


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
  if (hi < min_hi || hi >= max_hi) goto fail;
  for (auto pcb = lasthi; *pcb && pcb < lastlo; pcb++)
    {
      auto pcs = strchr(b62digits, *pcb);
      if (!pcs) goto fail;
      lo = lo*62 + (pcs-b62digits);
    }
  if (lo < min_lo || lo >= max_lo) goto fail;
  _id_hi = hi;
  _id_lo = lo;
  if (pend) *pend = lastlo;
  if (pok) *pok = true;
  return;
fail:
  if (pend) *pend = cbuf;
  if (pok) *pok = false;
  return;
} // end Rps_Id::Rps_Id (const char*, char**, bool*)



//////////////////////////////////////////////// quasi values


std::mutex Rps_QuasiZone::qz_mtx;
std::vector<Rps_QuasiZone*> Rps_QuasiZone::qz_zonvec(100);
uint32_t Rps_QuasiZone::qz_cnt;

Rps_QuasiZone::~Rps_QuasiZone()
{
  unregister_in_zonevec();
} // end of Rps_QuasiZone::~Rps_QuasiZone

void
Rps_QuasiZone::register_in_zonevec(void)
{
  std::lock_guard<std::mutex> gu(qz_mtx);
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
  std::lock_guard<std::mutex> gu(qz_mtx);
  RPS_ASSERT(this->qz_rank>0
             && this->qz_rank < (uint32_t)qz_zonvec.size());
  RPS_ASSERT(qz_cnt>0 && qz_cnt<(uint32_t)qz_zonvec.size());
  RPS_ASSERT(qz_zonvec[this->qz_rank] == this);
  qz_zonvec[this->qz_rank] = nullptr;
  qz_cnt--;
} // end of Rps_QuasiZone::unregister_in_zonevec

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
      memcpy (tup->raw_data(), vecob.data(), nbob * sizeof(Rps_ObjectRef));
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


/* end of file value_rps.cc */


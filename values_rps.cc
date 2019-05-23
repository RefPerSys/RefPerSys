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

#include "refpersys.hh"

void
Rps_Id::to_cbuf24(char cbuf[]) const
{
  /// example cbuf = "_0abcdefghijABCDEFG"
  ///                  |0         |11    |19
  static_assert(sizeof("_0abcdefghijABCDEFG")-1 == 1+nbdigits_hi+nbdigits_lo);
  assert (cbuf != nullptr);
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


#ifdef RPS_ONLY_ID_CODE
void
Rps_ZoneValue::initialize(void)
{
  RPS_FATAL("unimplemented Rps_ZoneValue::initialize");
} // end Rps_ZoneValue::initialize



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Rps_HashInt
Rps_SequenceObrefZone::hash_of_array(Rps_Type ty, uint32_t siz, const Rps_ObjectRef *arr)
{
  Rps_HashInt h1 = (Rps_HashInt)(31*(int)ty + 29);
  Rps_HashInt h2 = 11+(431*(siz&0x3fffff))+7*(int)ty;
  assert (siz == 0 || arr != nullptr);
  assert (siz < maxsize);
  for (auto ix=0u; ix<siz; ix++)
    {
      auto curobref = arr[ix];
      if (!curobref) continue;
      assert (curobref->type() == Rps_TyObject);
      auto curhash = curobref->hash();
      if (ix %2 == 0)
        h1 = (449*h1) ^ (curhash+1000);
      else
        h2 = (3449*h2+10) ^ (17*curhash);
    }
  Rps_HashInt h = h1 ^ h2;
  if (RPS_UNLIKELY(h==0))
    {
      h = (h1&0xfffff) + 541*(h2&0xfffff) + 10;
      assert (h!=0);
    }
  return h;
} /*end Rps_SequenceObrefZone::hash_of_array*/



Rps_HashInt
Rps_StringZone::hash_cstr(const char*cstr, int32_t slen)
{
  if (slen<0) slen= cstr?strlen(cstr):0;
  Rps_HashInt h1=slen&0xfffff, h2=31;
  int cnt=0;
  ucs4_t uc=0;
  for (auto pc = (const uint8_t*)cstr; (uc= (ucs4_t)*pc) != 0; (pc=u8_next(&uc, pc)), (cnt++))
    {
      if (cnt % 2 == 0)
        h1 = (h1*5413 + cnt) ^ (6427*uc + 10);
      else
        h2 = (h2*9419) ^ (11*cnt + 11437 * uc);
    }
  Rps_HashInt h = h1 ^ h2;
  if (RPS_UNLIKELY(h == 0))
    {
      h = 3*(h1&0xffff) + 17*(h2&0xffff) + (slen&0xffff) + 2;
      assert (h != 0);
    }
  return h;
} // end Rps_StringZone::hash_cstr


Rps_StringZone*
Rps_StringZone::make(Rps_CallFrameZone*callingfra,const char*sbytes, int32_t slen)
{
  if (!sbytes) slen= 0;
  else if (slen<0) slen = strlen(sbytes);
  if (slen>(int32_t)maxsize)
    fail_too_big_zone((uint32_t)slen, "too big string");
  uint32_t gap = byte_gap_for_size(slen);
  auto zstr = Rps_StringZone::rps_allocate_with_gap<Rps_StringZone>(callingfra, gap, sbytes, slen, false);
  return zstr;
} // end Rps_StringZone::make

Rps_TupleObrefZone*
Rps_TupleObrefZone::make(Rps_CallFrameZone*callingfra,uint32_t siz, const Rps_ObjectRef*arr)
{
  if (!arr) siz=0;
  else if (siz>maxsize)
    fail_too_big_zone(siz, "too big tuple");
  auto gap = byte_gap_for_size(siz);
  auto ztup = Rps_TupleObrefZone::rps_allocate_with_gap<Rps_TupleObrefZone>(callingfra, gap, siz, arr);
  return ztup;
} // end Rps_TupleObrefZone::make



Rps_SetObrefZone*
Rps_SetObrefZone::make(Rps_CallFrameZone*callingfra,uint32_t siz, const Rps_ObjectRef*arr)
{
  if (!arr) siz=0;
  else if (siz>maxsize)
    fail_too_big_zone(siz, "too big set");
  auto gap = byte_gap_for_size(siz);
  auto zqset = Rps_SetObrefZone::rps_allocate_with_gap<Rps_SetObrefZone>(callingfra, gap, nullptr, siz, arr);
  std::sort(zqset->_obarr, zqset->_obarr+siz);
  uint32_t card = 0;
  for (auto ix=0U; ix<siz; ix++)
    {
      auto curel = zqset->_obarr[ix];
      if (!curel) continue;
      if (ix==0) card++;
      else if (curel != zqset->_obarr[ix-1]) card++;
    }
  if (RPS_UNLIKELY(card != siz))
    {
      assert (card < siz);
      zqset->mutate(Rps_TyTuple);
      auto zqtup = zqset;
      auto cardgap = byte_gap_for_size(card);
      zqset = Rps_SetObrefZone::rps_allocate_with_gap<Rps_SetObrefZone>(callingfra,cardgap, nullptr, card, zqtup->_obarr+siz-card);
      uint32_t nbel = 0U;
      for (auto ix=0U; ix<siz; ix++)
        {
          auto curel = zqtup->_obarr[ix];
          if (!curel)
            continue;
          if (nbel==0)
            zqset->_obarr[nbel++] = curel;
          else if (curel != zqtup->_obarr[ix-1])
            zqset->_obarr[nbel++] = curel;;
        }
      assert (nbel == card);
    };
  zqset->compute_hash();
  return zqset;
} // end Rps_SetObrefZone::make


////////////////
Rps_TupleValue::Rps_TupleValue(Rps_CallFrameZone*callingfra,collect_tag, uint32_t siz, const Rps_Value*arr)
  : Rps_TupleValue(nullptr)
{
  if (!arr) siz = 0;
  if (siz == 0) return;
  /// we could want to special-case the frequent occurrence of small
  /// tuples, to keep their object on the call stack without using C++
  /// heap internally in vectors.
  std::vector<Rps_ObjectRef> vecob;
  vecob.reserve(siz);
  for (unsigned ix = 0; ix<(unsigned)siz; ix++)
    {
      auto val = arr[ix];
      if (!val) continue;
      auto pob = val.as_object();
      if (pob != nullptr)
        {
          vecob.push_back(pob);
          continue;
        }
      auto pseq = val.as_sequence();
      if (pseq != nullptr)
        {
          for (const Rps_ObjectRef obr: *pseq)
            {
              vecob.push_back(obr);
            }
          continue;
        }
    }
  put_tuple(Rps_TupleObrefZone::make(callingfra,vecob.size(), vecob.data()));
}

Rps_TupleValue::Rps_TupleValue(Rps_CallFrameZone*callingfra,collect_tag, const std::initializer_list<const Rps_Value> il)
  : Rps_TupleValue(nullptr)
{
  /// we could want to special-case the frequent occurrence of small
  /// tuples, to keep their object on the call stack without using C++
  /// heap internally in vectors.
  std::vector<Rps_ObjectRef> vecob;
  vecob.reserve(3*il.size()/2+4);
  for (auto val: il)
    {
      auto pob = val.as_object();
      if (pob != nullptr)
        {
          vecob.push_back(pob);
          continue;
        }
      auto pseq = val.as_sequence();
      if (pseq != nullptr)
        {
          for (const Rps_ObjectRef obr: *pseq)
            {
              vecob.push_back(obr);
            }
          continue;
        }
    }
  put_tuple(Rps_TupleObrefZone::make(callingfra,vecob.size(), vecob.data()));
} // end Rps_TupleValue::Rps_TupleValue(collect_tag, … il)




#endif /*RPS_ONLY_ID_CODE*/
/* end of file value_rps.cc */


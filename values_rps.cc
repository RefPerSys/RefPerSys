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


#ifdef RPS_HAVE_MPS
mps_arena_t Rps_ZoneValue::_mps_valzone_arena;
mps_fmt_t Rps_ZoneValue::_mps_valzone_fmt;
#endif /*RPS_HAVE_MPS*/

#warning the MPS routines could be wrong

void
Rps_ZoneValue::initialize(void)
{
#ifdef RPS_HAVE_MPS
  mps_res_t res;
  // create our arena
  MPS_ARGS_BEGIN(args)
  {
    MPS_ARGS_ADD(args, MPS_KEY_ARENA_SIZE, 128 * 1024 * 1024);
    res = mps_arena_create_k(&_mps_valzone_arena, mps_arena_class_vm(), args);
  }
  MPS_ARGS_END(args);
  if (res != MPS_RES_OK)
    RPS_FATAL("Couldn't create _mps_valzone_arena (#%d) - %m", res);
  // create our object format
  MPS_ARGS_BEGIN(args)
  {
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ALIGN, 2*alignof(Rps_Value));
    MPS_ARGS_ADD(args, MPS_KEY_FMT_SCAN, mps_valzone_scan);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_SKIP, mps_valzone_skip);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_FWD, mps_valzone_fwd);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ISFWD, mps_valzone_isfwd);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_PAD, mps_valzone_pad);
    res = mps_fmt_create_k(&_mps_valzone_fmt, _mps_valzone_arena, args);
  }
  MPS_ARGS_END(args);
  if (res != MPS_RES_OK)
    RPS_FATAL( "Couldn't create _mps_valzone_fmt (#%d) - %m", res);
#endif /*RPS_HAVE_MPS*/
} // end Rps_ZoneValue::initialize



////////////////////////////////////////////////////////////////
#ifdef RPS_HAVE_MPS
/// MPS related code
mps_res_t
Rps_ZoneValue::mps_valzone_scan(mps_ss_t ss, mps_addr_t base, mps_addr_t limit)
{
  MPS_SCAN_BEGIN(ss)
  {
    while (base < limit)
      {
        auto pzon = reinterpret_cast<Rps_ZoneValue*>(base);
        switch (pzon->type())
          {
          case Rps_TyForwardedMPS:
          {
            auto zfwd = reinterpret_cast<const Rps_ForwardedMPSZoneValue*>(pzon);
            base = reinterpret_cast<mps_addr_t>
                   ((char*)zfwd + zfwd->forwardsize());
          }
          continue;
          case Rps_TyPaddingMPS:
          {
            auto zpad = reinterpret_cast<const Rps_PaddingMPSZoneValue*>(pzon);
            base = reinterpret_cast<mps_addr_t>(zpad->next_byte());
          }
          continue;
          case Rps_TyString:
          {
            auto zstr = reinterpret_cast<const Rps_StringZone*>(pzon);
            base = (mps_addr_t) reinterpret_cast<const void*>
                   (zstr->_strbytes + Rps_StringZone::byte_gap_for_size(zstr->size()));
          }
          continue;
          case Rps_TyDouble:
          {
            auto zdbl =  reinterpret_cast<const Rps_DoubleZone*>(pzon);
            base =  (mps_addr_t) reinterpret_cast<const void*> (zdbl+1);
          }
          continue;
          case Rps_TyTuple:
          case Rps_TySet:
          {
            auto zseq = reinterpret_cast<Rps_SequenceObrefZone*>(pzon);
            auto size = zseq->size();
            for (unsigned ix=0; ix < size; ix++)
              {
                auto curptr = zseq->_obarr[ix].obptr();
                if (curptr)
                  {
                    void*curcomp = curptr;
                    mps_res_t res = MPS_FIX12(ss, &curcomp);
                    if (res == MPS_RES_OK)
                      (zseq->_obarr[ix]).set_obptr(reinterpret_cast<Rps_ObjectZone*>(curcomp));
                  }
              }
            base = reinterpret_cast<mps_addr_t>	(zseq->_obarr+size);
          }
          continue;
          case Rps_TyObject:
          {
            auto zob = reinterpret_cast<Rps_ObjectZone*>(pzon);
            const void*p = zob->mps_really_scan(ss);
            base = reinterpret_cast<mps_addr_t>((void*)p);
          }
          continue;
          }
        RPS_FATAL("corrupted scanned zone @%p of type %d\n",
                  (void*)pzon, (int)pzon->type());
      }
  }
  MPS_SCAN_END(ss);
  return MPS_RES_OK;
} // end Rps_ZoneValue::mps_valzone_scan


mps_addr_t
Rps_ZoneValue::mps_valzone_skip(mps_addr_t base)
{
  auto pzon = reinterpret_cast<Rps_ZoneValue*>(base);
  switch (pzon->type())
    {
    case Rps_TyForwardedMPS:
    {
      auto zfwd = reinterpret_cast<const Rps_ForwardedMPSZoneValue*>(pzon);
      return reinterpret_cast<mps_addr_t>
             ((char*)zfwd + zfwd->forwardsize());
    }
    case Rps_TyPaddingMPS:
    {
      auto zpad = reinterpret_cast<const Rps_PaddingMPSZoneValue*>(pzon);
      return reinterpret_cast<mps_addr_t>(zpad->next_byte());
    }
    case Rps_TyString:
    {
      auto zstr = reinterpret_cast<const Rps_StringZone*>(pzon);
      return (mps_addr_t) reinterpret_cast<const void*>
             (zstr->_strbytes + Rps_StringZone::byte_gap_for_size(zstr->size()));
    }
    case Rps_TyDouble:
    {
      auto zdbl = reinterpret_cast<const Rps_DoubleZone*>(pzon);
      return (mps_addr_t) reinterpret_cast<const void*> (zdbl+1);
    }
    case Rps_TyTuple:
    case Rps_TySet:
    {
      auto zseq = reinterpret_cast<Rps_SequenceObrefZone*>(pzon);
      auto size = zseq->size();
      return reinterpret_cast<mps_addr_t>	(zseq->_obarr+size);
    }
    case Rps_TyObject:
    {
      auto zob = reinterpret_cast<Rps_ObjectZone*>(pzon);
      return reinterpret_cast<mps_addr_t>(zob+1);
    }
    }
  RPS_FATAL("corrupted skipped zone @%p of type %d\n",
            (void*)pzon, (int)pzon->type());
} // end Rps_ZoneValue::mps_valzone_skip


void
Rps_ZoneValue::mps_valzone_fwd(mps_addr_t oldad, mps_addr_t newad)
{
  auto pzon = reinterpret_cast<Rps_ZoneValue*>(oldad);
  auto bytsiz = (char*)mps_valzone_skip(oldad) - (char*)pzon;
  assert ((unsigned) bytsiz >= (unsigned) sizeof(Rps_ForwardedMPSZoneValue));
  (void) new((void*)oldad) Rps_ForwardedMPSZoneValue(newad, bytsiz);
} // end of Rps_ZoneValue::mps_valzone_fwd


mps_addr_t
Rps_ZoneValue::mps_valzone_isfwd(mps_addr_t addr)
{
  auto pfwd = reinterpret_cast<Rps_ForwardedMPSZoneValue*>(addr);
  if (pfwd->type() == Rps_TyForwardedMPS) return pfwd->forwardptr();
  return nullptr;
} // end Rps_ZoneValue::mps_valzone_isfwd

void
Rps_ZoneValue::mps_valzone_pad(mps_addr_t addr, size_t size)
{
  assert (size >= sizeof(Rps_PaddingMPSZoneValue));
  new (addr) Rps_PaddingMPSZoneValue(size);
} // end Rps_ZoneValue::mps_valzone_pad
#endif /*RPS_HAVE_MPS*/

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


#ifdef RPS_HAVE_MPS
thread_local mps_ap_t Rps_ScalarCopyingZoneValue::mps_allocpoint_;
thread_local mps_ap_t Rps_PointerCopyingZoneValue::mps_allocpoint_;
thread_local mps_ap_t Rps_MarkSweepZoneValue::mps_allocpoint_;
#endif /*RPS_HAVE_MPS*/

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
/* end of file value_rps.cc */


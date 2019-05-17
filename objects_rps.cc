/****************************************************************
 * file objects_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      Low-level implementation of objects.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Niklas Rosencrantz <niklasro@gmail.com>
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

Rps_ObjectZone::BucketOb_st Rps_ObjectZone::_ob_bucketarr[Rps_Id::maxbuckets];

Rps_ObjectZone*
Rps_ObjectZone::find_by_id(const Rps_Id& id)
{
  if (!id) return nullptr;
  if (!id.valid()) return nullptr;
  auto& curbuck = bucket(id);
  auto buckguard = std::lock_guard<std::mutex>(curbuck._bu_mtx);
  auto it = curbuck._bu_map.find(id);
  if (it != curbuck._bu_map.end())
    return it->second;
  return nullptr;
} // end Rps_ObjectZone::find_by_id


Rps_ObjectZone*
Rps_ObjectZone::make_of_id(Rps_CallFrameZone*callingfra,const Rps_Id&id)
{
  if (!id) return nullptr;
  if (!id.valid()) return nullptr;
  auto& curbuck = bucket(id);
  auto buckguard = std::lock_guard<std::mutex>(curbuck._bu_mtx);
  auto it = curbuck._bu_map.find(id);
  if (it != curbuck._bu_map.end())
    return it->second;
  auto newob = rps_allocate<Rps_ObjectZone>(callingfra,id);
  curbuck._bu_map.insert({id,newob});
#ifdef RPS_HAVE_MPS
  mps_addr_t obad = (mps_addr_t)newob;
  mps_finalize(_mps_valzone_arena, &obad);
#endif /*RPS_HAVE_MPS*/
  return newob;
} // end Rps_ObjectZone::make_of_id




Rps_ObjectZone*
Rps_ObjectZone::make(Rps_CallFrameZone*callingfra)
{
  for (;;)   // this loop usually runs once (except on very unlikely
    // objid-s collisions)
    {
      auto id = Rps_Id::random();
      assert (id && id.valid());
      auto& curbuck = bucket(id);
      auto buckguard = std::lock_guard<std::mutex>(curbuck._bu_mtx);
      auto it = curbuck._bu_map.find(id);
      if (it != curbuck._bu_map.end())
        continue;
      auto newob = rps_allocate<Rps_ObjectZone>(callingfra,id);
      curbuck._bu_map.insert({id,newob});
#ifdef RPS_HAVE_MPS
      mps_addr_t obad = (mps_addr_t)newob;
      mps_finalize(_mps_valzone_arena, &obad);
#endif /*RPS_HAVE_MPS*/
      return newob;
    }
}			      // end of Rps_ObjectZone::make



Rps_ObjectZone::~Rps_ObjectZone()  /// only called by finalizer
{
  // should clear the object
#warning Rps_ObjectZone destructor is incomplete
  // at last, remove the dead object from its bucket
  {
    auto& curbuck = bucket();
    auto buckguard = std::lock_guard<std::mutex>(curbuck._bu_mtx);
    curbuck._bu_map.erase(_ob_id);
  }
} // end of Rps_ObjectZone::~Rps_ObjectZone




/// Resize the quasi component vector to the given size, so grow or
/// shrink them, or create that quasivalue and register it in this
/// object.
void
Rps_ObjectZone::do_resize_components (Rps_CallFrameZone*callingfra, unsigned newnbcomp)
{
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_ObjectRef thisob;
                 Rps_QuasiComponentVector* oldcompvec;
                 Rps_QuasiComponentVector* newcompvec;
                );
  _.oldcompvec = _ob_compvec;
  _.thisob = this;
  if (!_ob_compvec)
    {
      // Less common case of an object not having yet any quasi
      // component vector.
      auto alsiz = (newnbcomp<Rps_QuasiComponentVector::_min_alloc_size_
                    ?Rps_QuasiComponentVector::_min_alloc_size_
                    :rps_prime_above(newnbcomp));
      _.newcompvec =
        Rps_QuasiComponentVector::rps_allocate_with_gap<Rps_QuasiComponentVector>
        (RPS_CURFRAME,
         sizeof(Rps_QuasiComponentVector),
         alsiz*sizeof(Rps_Value),
         (unsigned)alsiz,
         (const Rps_Value*)nullptr);
      _.newcompvec->_qnbcomp = newnbcomp;
      _ob_compvec = _.newcompvec;
    }
  else
    {
      assert (_ob_compvec->_qnbcomp <= _ob_compvec->_qsizarr);
      // Usual case, there is a quasi component vector, we may need to
      // either shrink or resize it.
      auto alsiz = (newnbcomp<Rps_QuasiComponentVector::_min_alloc_size_
                    ?Rps_QuasiComponentVector::_min_alloc_size_
                    :rps_prime_above(newnbcomp));
      assert (alsiz >= newnbcomp);
      if (alsiz != _ob_compvec->_qsizarr)
        {
          _.newcompvec =
            Rps_QuasiComponentVector::rps_allocate_with_gap<Rps_QuasiComponentVector>
            (RPS_CURFRAME,
             sizeof(Rps_QuasiComponentVector),
             alsiz*sizeof(Rps_Value),
             (unsigned)alsiz,
             (const Rps_Value*)nullptr);
          unsigned nbcomp = std::min((unsigned)_.oldcompvec->_qnbcomp, (unsigned)newnbcomp);
          memcpy((void*)_.newcompvec->_qarrval,
                 (void*)_.oldcompvec->_qarrval,
                 nbcomp*sizeof(Rps_Value));
          _.newcompvec->_qnbcomp = nbcomp;
          _ob_compvec = _.newcompvec;
        }
      else
        {
          _.oldcompvec->_qnbcomp = std::min((unsigned)_.oldcompvec->_qnbcomp,  (unsigned)newnbcomp);
        }
    }
  RPS_WRITE_BARRIER();
} // end Rps_ObjectZone::do_resize_components




void
Rps_ObjectZone::do_reserve_components (Rps_CallFrameZone*callingfra, unsigned newsize)
{
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_ObjectRef thisob;
                 Rps_QuasiComponentVector* oldcompvec;
                 Rps_QuasiComponentVector* newcompvec;
                );
  _.oldcompvec = _ob_compvec;
  _.thisob = this;
  auto newalsiz = (newsize<Rps_QuasiComponentVector::_min_alloc_size_
                   ?Rps_QuasiComponentVector::_min_alloc_size_
                   :rps_prime_above(newsize));
  auto oldalsiz = _.oldcompvec?_.oldcompvec->allocated_size():0;
  if (newsize == 0)
    {
      _.newcompvec = nullptr;
    }
  else if (newalsiz != oldalsiz)
    {
      _.newcompvec =
        Rps_QuasiComponentVector::rps_allocate_with_gap<Rps_QuasiComponentVector>
        (RPS_CURFRAME,
         sizeof(Rps_QuasiComponentVector),
         newalsiz*sizeof(Rps_Value),
         (unsigned)newalsiz,
         (const Rps_Value*)nullptr);
    }
  else
    {
      _.newcompvec = _.oldcompvec;
      memset (_.oldcompvec->_qarrval, 0,
              oldalsiz*sizeof(Rps_Value));
    }
  _ob_compvec = _.newcompvec;
  RPS_WRITE_BARRIER();
} // end Rps_ObjectZone::do_reserve_components



void
Rps_ObjectZone::do_append_component(Rps_CallFrameZone*callingfra,Rps_Value val)
{
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_ObjectRef thisob;
                 Rps_QuasiComponentVector* oldcompvec;
                 Rps_QuasiComponentVector* newcompvec;
                );
  _.oldcompvec = _ob_compvec;
  _.thisob = this;
  if (RPS_UNLIKELY(_ob_compvec == nullptr))
    {
      auto alsiz = Rps_QuasiComponentVector::_min_alloc_size_;
      assert (alsiz > 1);
      _.newcompvec =
        Rps_QuasiComponentVector::rps_allocate_with_gap<Rps_QuasiComponentVector>
        (RPS_CURFRAME,
         sizeof(Rps_QuasiComponentVector),
         alsiz*sizeof(Rps_Value),
         (unsigned)alsiz,
         (const Rps_Value*)nullptr);
      _.newcompvec->_qnbcomp = 1;
      _.newcompvec->_qarrval[0] = val;
      _ob_compvec = _.newcompvec;
    }
  else
    {
      unsigned oldsiz = _ob_compvec->allocated_size();
      unsigned oldnbcomp = _ob_compvec->_qnbcomp;
      if (RPS_LIKELY(oldnbcomp + 1 < oldsiz))
        {
          _ob_compvec->_qarrval[oldnbcomp] = val;
          _ob_compvec->_qnbcomp = oldnbcomp+1;
        }
      else
        {
          unsigned newsiz = rps_prime_above(9*oldnbcomp/8 + 3);
          assert (newsiz > oldnbcomp+1);
          _.newcompvec =
            Rps_QuasiComponentVector::rps_allocate_with_gap<Rps_QuasiComponentVector>
            (RPS_CURFRAME,
             sizeof(Rps_QuasiComponentVector),
             newsiz*sizeof(Rps_Value),
             (unsigned)newsiz,
             (const Rps_Value*)_.oldcompvec->_qarrval);
          _.newcompvec->_qarrval[oldnbcomp] = val;
          _.newcompvec->_qnbcomp = oldnbcomp+1;
          _ob_compvec = _.newcompvec;
        }
    };
  RPS_WRITE_BARRIER();
} // end Rps_ObjectZone::do_append_component



void Rps_ObjectZone::do_put_attr(Rps_CallFrameZone*callingfra, Rps_ObjectRef keyobarg, Rps_Value valatarg)
{
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_ObjectRef thisob;
                 Rps_ObjectRef keyob;
                 Rps_Value valat;
                 Rps_ObjectRef altkeyob; // some "current" or "next" key
                 Rps_Value curvalat;
                 Rps_QuasiAttributeArray*oldattrs;
                 Rps_QuasiAttributeArray*newattrs;
                );
  if (!keyobarg || !valatarg)
    return;
  _.thisob = this;
  _.keyob = keyobarg;
  _.valat = valatarg;
  switch (_obat_kind)
    {
    case atk_none:
    {
      _.newattrs =
        Rps_QuasiAttributeArray::rps_allocate_with_gap<Rps_QuasiAttributeArray>
        (RPS_CURFRAME,
         sizeof(Rps_QuasiAttributeArray),
         at_small_initsize,
         0,
         nullptr
        );
      _.newattrs->_qatentries[0].first = _.keyob;
      _.newattrs->_qatentries[0].second = _.valat;
      _.newattrs->_qnbattrs = 1;
      _obat_kind = atk_small;
      _obat_small_atar = _.newattrs;
    }
    break;
    case atk_small:
    {
      assert (_obat_small_atar != nullptr);
      _.oldattrs = _obat_small_atar;
      unsigned oldalsize = _.oldattrs->_qsizattr;
      unsigned oldnbattr = _.oldattrs->_qnbattrs;
      assert (oldnbattr <= oldalsize && oldalsize <= at_small_thresh);
      if (RPS_LIKELY(oldnbattr+1 < oldalsize))
        {
          // enough space, no need to reallocate
          _.oldattrs->_qatentries[oldnbattr].first = nullptr;
          int pos = -1;
          bool found = false;
          unsigned newnbattr = oldnbattr;
          for (int ix=0; ix<oldnbattr; ix++)
            {
              if (_.oldattrs->_qatentries[ix].first == _.keyob)
                {
                  pos = ix;
                  found = true;
                  break;
                }
              else if (_.oldattrs->_qatentries[ix].first.is_empty())
                {
                  if (pos<0)
                    pos = ix;
                  continue;
                }
            };
          if (pos < 0)
            pos = oldnbattr;
          _.oldattrs->_qatentries[pos].first = _.keyob;
          _.oldattrs->_qatentries[pos].second = _.valat;
          if (!found)
            _.oldattrs->_qnbattrs = oldnbattr+1;
        }
      else
        {
          unsigned newsize = rps_prime_above(9*oldnbattr/8+3);
          unsigned newcnt = 0;
          _.newattrs =
            Rps_QuasiAttributeArray::rps_allocate_with_gap<Rps_QuasiAttributeArray>
            (RPS_CURFRAME,
             sizeof(Rps_QuasiAttributeArray),
             newsize,
             0,
             nullptr
            );
          /// The needed transition from small to sorted
          /// QuasiAttributeArray-s may happen smoothly and with a
          /// little random noise between 0 and 7 included, since we
          /// want to avoid too frequent oscillations between them
          /// (e.g. in the case when we add a few attributes, remove a
          /// few others of them, and so forth, around the
          /// thresholds.).
          if (newsize < at_sorted_thresh
              || newsize < at_sorted_thresh - 1 +  (Rps_Random::random_quickly_4bits() & 7))
            {
              // grow the _obat_small_atar up to newsize
              for (unsigned ix=0; ix<oldnbattr; ix++)
                {
                  _.altkeyob = _.oldattrs->_qatentries[ix].first;
                  if (_.altkeyob.is_empty())
                    continue;
                  _.curvalat = _.oldattrs->_qatentries[ix].second;
                  if (_.curvalat.is_empty())
                    continue;
                  _.newattrs->_qatentries[newcnt].first = _.altkeyob;
                  _.newattrs->_qatentries[newcnt].second = _.curvalat;
                }
              assert (newcnt < newsize);
              _.newattrs->_qnbattrs = newcnt;
              _obat_small_atar = _.newattrs;
              _obat_kind = atk_small;
            }
          else
            {
              // Here, the _obat_small_atar grows to become a
              // _obat_sorted_atar, that is, we actually make the
              // small to sorted transition.
              for (unsigned ix=0; ix<oldnbattr; ix++)
                {
                  _.altkeyob = _.oldattrs->_qatentries[ix].first;
                  if (_.altkeyob.is_empty())
                    continue;
                  _.curvalat = _.oldattrs->_qatentries[ix].second;
                  if (_.curvalat.is_empty())
                    continue;
                  _.newattrs->_qatentries[newcnt].first = _.altkeyob;
                  _.newattrs->_qatentries[newcnt].second = _.curvalat;
                }
              assert (newcnt < newsize);
              _.newattrs->_qnbattrs = newcnt;
              std::sort (_.newattrs->begin(), _.newattrs->end(),
                         Rps_QuasiAttributeArray::entry_compare_st{});
              _obat_sorted_atar = _.newattrs;
              _obat_kind = atk_medium;
            }
        }
    }
    break;
    case atk_medium:
    {
      assert (_obat_sorted_atar != nullptr);
      _.oldattrs = _obat_sorted_atar;
      unsigned oldalsize = _.oldattrs->_qsizattr;
      unsigned oldnbattr = _.oldattrs->_qnbattrs;
      assert (oldnbattr <= oldalsize && oldalsize <= at_sorted_thresh);
      if (RPS_LIKELY(oldnbattr+1 < at_sorted_thresh
                     && oldnbattr+1 < oldalsize))
        {
          /// No need to regrow the sorted quasi-attribute-array, but
          /// we need to perhaps insert the entry at the right place,
          /// to keep that array sorted. Using a dichotomical binary
          /// search and/or insertion.
          int inspos = -1;
          // Equality test on objects is significantly faster than
          // 'less than' or 'greater than' tests. So, ...

          int lo = 0, hi = oldnbattr;
          {
            auto p = dichotomy_medium_sorted(_.keyob, _.oldattrs);
            lo = p.first;
            hi = p.second;
          }
          assert (lo >= 0 && hi <= oldnbattr);
          if ( _.keyob < (_.altkeyob =  _.oldattrs->_qatentries[lo].first))
            {
              // insert before lo
              for (int ix=oldnbattr; ix>lo; ix--)
                _.oldattrs->_qatentries[ix] = _.oldattrs->_qatentries[ix-1];
              _.oldattrs->_qatentries[lo].first = _.keyob;
              _.oldattrs->_qatentries[lo].second = _.valat;
              _.oldattrs->_qnbattrs = oldnbattr+1;
            }
          else if (hi>0
                   && _.keyob > (_.altkeyob =  _.oldattrs->_qatentries[hi-1].first))
            {
              // insert after hi
              for (int ix=oldnbattr; ix>hi; ix--)
                _.oldattrs->_qatentries[ix] = _.oldattrs->_qatentries[ix-1];
              _.oldattrs->_qatentries[hi].first = _.keyob;
              _.oldattrs->_qatentries[hi].second = _.valat;
              _.oldattrs->_qnbattrs = oldnbattr+1;
            }
          else
            {
              //  Since _.oldattrs is sorted but not-small, and previous
              //  tests failed, we can:
              assert (hi+1<oldnbattr);
              // loop between lo included & hi excluded
              for (int k=lo; k<hi; k++)
                {
                  _.altkeyob = _.oldattrs->_qatentries[k].first;
                  if (_.altkeyob == _.keyob)
                    {
                      // replace the key's value
                      _.oldattrs->_qatentries[k].second = _.valat;
                      break;
                    }
                  else if (_.keyob <
                           (_.altkeyob = _.oldattrs->_qatentries[k+1].first))
                    {
                      // insert the key between k and k+1
                      for (int ix=oldnbattr; ix>k; ix--)
                        _.oldattrs->_qatentries[ix] = _.oldattrs->_qatentries[ix-1];
                      _.oldattrs->_qatentries[hi].first = _.keyob;
                      _.oldattrs->_qatentries[hi].second = _.valat;
                      _.oldattrs->_qnbattrs = oldnbattr+1;
                      break;
                    }
                }
            }
        }
      else if (oldnbattr+1 < at_sorted_thresh
               || oldnbattr + 1 < at_sorted_thresh + 1 + (Rps_Random::random_quickly_4bits() & 7))
        {
          /// we need to grow the sorted quasi-attribute-array
          unsigned newsize = rps_prime_above(9*oldnbattr/8+3);
          _.newattrs =
            Rps_QuasiAttributeArray::rps_allocate_with_gap<Rps_QuasiAttributeArray>
            (RPS_CURFRAME,
             sizeof(Rps_QuasiAttributeArray),
             newsize,
             0,
             nullptr
            );
          unsigned newcnt = 0;
          bool reached = false;
          for (int ix=0; ix<oldnbattr; ix++)
            {
              if (reached)
                _.newattrs->_qatentries[newcnt++] = _.oldattrs->_qatentries[ix];
              else
                {
                  _.altkeyob = _.oldattrs->_qatentries[ix].first;
                  if (_.altkeyob < _.keyob)
                    _.newattrs->_qatentries[newcnt++] = _.oldattrs->_qatentries[ix];
                  else if (_.altkeyob > _.keyob)
                    {
                      reached = true;
                      _.newattrs->_qatentries[newcnt].first = _.keyob;
                      _.newattrs->_qatentries[newcnt].second = _.valat;
                      newcnt++;
                      _.newattrs->_qatentries[newcnt++] = _.oldattrs->_qatentries[ix];
                    }
                  else
                    {
                      assert (_.altkeyob == _.keyob);
                      reached = true;
                      _.newattrs->_qatentries[newcnt].first = _.keyob;
                      _.newattrs->_qatentries[newcnt].second = _.valat;
                      newcnt++;
                    }
                }
            }
          _.newattrs->_qnbattrs = newcnt;
          _obat_sorted_atar = _.newattrs;
        }
      else
        {
          // the _obat_sorted_atar should be transformed in
          // _obat_map_atar
          _obat_sorted_atar = nullptr;
          _obat_kind = atk_big;
          // we need to construct that _obat_map_atar:
          (void) new ((void*)&_obat_map_atar) typeof(_obat_map_atar);
          for (int ix=0; ix<oldnbattr; ix++)
            _obat_map_atar.emplace(_.oldattrs->_qatentries[ix]);
        }
    }
    break;
    case atk_big:
    {
      _obat_map_atar.insert({_.keyob, _.valat});
    }
    break;
    } // end switch _obat_kind
  RPS_WRITE_BARRIER();
} // end Rps_ObjectZone::do_put_attr




void Rps_ObjectZone::do_remove_attr(Rps_CallFrameZone*callfra, Rps_ObjectRef keyob)
{
#warning unimplemented Rps_ObjectZone::do_remove_attr
  RPS_FATAL("unimplemented Rps_ObjectZone::do_remove_attr @%p key@%p",
            (void*)this, (void*)keyob.optr());
} // end Rps_ObjectZone::do_remove_attr







#ifdef RPS_HAVE_MPS
const void*
Rps_ObjectZone::mps_really_scan(mps_ss_t ss)
{
#warning incomplete Rps_ObjectZone::mps_really_scan
  auto obcla = object_class();
  void*curcomp = obcla;
  MPS_SCAN_BEGIN(ss);
  mps_res_t res = MPS_FIX12(ss, &curcomp);
  if (res != MPS_RES_OK)   /// this should never happen
    {
      char cbuf[24];
      objid().to_cbuf24(cbuf);
      RPS_FATAL("failed to scan class of object %s @%p (res#%d)\n",
                cbuf, (void*)this, res);
    };
  MPS_SCAN_END(ss);
  // no need to overwrite the class, since it is an object which cannot move
  return  reinterpret_cast<void*>(this+1);
} // end Rps_ObjectZone::mps_really_scan
#endif /*RPS_HAVE_MPS*/

// end of file objects_rps.cc

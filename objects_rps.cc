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
Rps_ObjectZone::make_of_id(Rps_CallFrameZone*callfram,const Rps_Id&id)
{
  if (!id) return nullptr;
  if (!id.valid()) return nullptr;
  auto& curbuck = bucket(id);
  auto buckguard = std::lock_guard<std::mutex>(curbuck._bu_mtx);
  auto it = curbuck._bu_map.find(id);
  if (it != curbuck._bu_map.end())
    return it->second;
  auto newob = rps_allocate<Rps_ObjectZone>(callfram,id);
  curbuck._bu_map.insert({id,newob});
#ifdef RPS_HAVE_MPS
  mps_addr_t obad = (mps_addr_t)newob;
  mps_finalize(_mps_valzone_arena, &obad);
#endif /*RPS_HAVE_MPS*/
  return newob;
} // end Rps_ObjectZone::make_of_id


Rps_ObjectZone*
Rps_ObjectZone::make(Rps_CallFrameZone*callfram)
{
  for (;;)   // this loop usually runs once (except on unlikely collisions)
    {
      auto id = Rps_Id::random();
      assert (id && id.valid());
      auto& curbuck = bucket(id);
      auto buckguard = std::lock_guard<std::mutex>(curbuck._bu_mtx);
      auto it = curbuck._bu_map.find(id);
      if (it != curbuck._bu_map.end())
        continue;
      auto newob = rps_allocate<Rps_ObjectZone>(callfram,id);
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

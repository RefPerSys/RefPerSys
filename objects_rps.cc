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

extern "C" const char rps_objects_gitid[];
const char rps_objects_gitid[]= RPS_GITID;

extern "C" const char rps_objects_date[];
const char rps_objects_date[]= __DATE__;


void
Rps_ObjectZone::gc_mark(Rps_GarbageCollector&gc, unsigned)
{
  std::lock_guard<std::shared_mutex> gu(ob_mtx);
  gc.mark_obj(this);
} // end of Rps_ObjectZone::gc_mark

void
Rps_ObjectZone::mark_gc_inside(Rps_GarbageCollector&gc)
{
  std::lock_guard<std::shared_mutex> gu(ob_mtx);
#warning perhaps the _gcinfo should be used here
  Rps_ObjectZone* obcla = ob_class.load();
  RPS_ASSERT(obcla != nullptr);
  gc.mark_obj(obcla);
  for (auto atit: ob_attrs)
    {
      gc.mark_obj(atit.first);
      if (atit.second.is_ptr())
        gc.mark_value(atit.second);
    }
  for (auto compv: ob_comps)
    {
      if (compv.is_ptr())
        gc.mark_value(compv);
    };
  Rps_Payload*payl = ob_payload.load();
  if (payl)
    payl->gc_mark(gc, 0);
} // end Rps_ObjectZone::mark_gc_inside

bool
Rps_ObjectZone::equal(const Rps_ZoneValue&zv) const
{
  if (zv.stored_type() == Rps_Type::Object)
    {
      auto othob = reinterpret_cast<const Rps_ObjectZone*>(&zv);
      return this == othob;
    }
  return false;
} // end of Rps_ObjectZone::equal

bool
Rps_ObjectZone::less(const Rps_ZoneValue&zv) const
{
  if (zv.stored_type() > Rps_Type::Object) return false;
  if (zv.stored_type() < Rps_Type::Object) return true;
  {
    auto othob = reinterpret_cast<const Rps_ObjectZone*>(&zv);
    return this->oid() < othob->oid();
  }
} // end of Rps_ObjectZone::less

////////////////////////////////////////////////////////////////
///// global roots for garbage collection and persistence

static std::set<Rps_ObjectRef> rps_object_root_set;
static std::mutex rps_object_root_mtx;

void
rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (auto ob: rps_object_root_set)
    fun(ob);
} // end rps_each_root_object


void
rps_add_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_root_set.insert(ob);
} // end rps_add_root_object


bool
rps_remove_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  if (it == rps_object_root_set.end())
    return false;
  rps_object_root_set.erase(it);
  return true;
} // end rps_remove_root_object

bool rps_is_root_object (const Rps_ObjectRef ob)
{
  if (!ob)
    return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  return it != rps_object_root_set.end();
} // end rps_is_root_object

std::set<Rps_ObjectRef>
rps_set_root_objects(void)
{
  std::set<Rps_ObjectRef> set;

  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (Rps_ObjectRef ob: rps_object_root_set)
    set.insert(ob);
  return set;
} // end rps_set_root_objects

unsigned
rps_nb_root_objects(void)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  return (unsigned) rps_object_root_set.size();
} // end rps_nb_root_objects

// end of file objects_rps.cc

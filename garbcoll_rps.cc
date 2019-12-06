/****************************************************************
 * file garbcoll_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the garbage collector.
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


extern "C" const char rps_garbcoll_gitid[];
const char rps_garbcoll_gitid[]= RPS_GITID;

extern "C" const char rps_garbcoll_date[];
const char rps_garbcoll_date[]= __DATE__;

std::atomic<Rps_GarbageCollector*>
Rps_GarbageCollector::gc_this;
Rps_GarbageCollector::Rps_GarbageCollector(const std::function<void(Rps_GarbageCollector*)> &rootmarkers) :
  gc_mtx(), gc_running(false), gc_rootmarkers(rootmarkers),
  gc_obscanque()
{
  RPS_ASSERT(gc_this.load() == nullptr);
  gc_this.store(this);
} // end Rps_GarbageCollector::Rps_GarbageCollector


Rps_GarbageCollector::~Rps_GarbageCollector()
{
  RPS_ASSERT(gc_this.load() == this);
  RPS_ASSERT(gc_running.load() == false);
  RPS_ASSERT(gc_obscanque.empty());
  gc_this.store(nullptr);
} // end Rps_GarbageCollector::~Rps_GarbageCollector

void
rps_garbage_collect (void)
{
  RPS_ASSERT(Rps_GarbageCollector::gc_this.load() == nullptr);
  Rps_GarbageCollector the_gc([](Rps_GarbageCollector*gc)
  {
    rps_each_root_object([=](Rps_ObjectRef obr)
    {
      obr.gc_mark(*gc);
    });
  });
  the_gc.run_gc();
  RPS_FATAL("unimplemented rps_garbage_collect");
#warning rps_garbage_collect unimplemented
} // end of rps_garbage_collect

void
Rps_GarbageCollector::mark_obj(Rps_ObjectRef ob)
{
  if (!ob) return;
  RPS_ASSERT(gc_running.load());
  if (!ob->is_gcmarked(*this))
    {
      ob->set_gcmark(*this);
      gc_obscanque.push_back(ob);
    }
} // end of Rps_GarbageCollector::mark_obj


void
Rps_GarbageCollector::run_gc(void)
{
  RPS_ASSERT(gc_running.load());
  Rps_QuasiZone::run_locked_gc
  (*this,
   [] (Rps_GarbageCollector&gc)
  {
    Rps_QuasiZone::clear_all_gcmarks(gc);
    while (!gc.gc_obscanque.empty())
      {
        auto obfront = gc.gc_obscanque.front();
        gc.gc_obscanque.pop_front();
        RPS_ASSERT(obfront);
        obfront->mark_gc_inside(gc);
      };
  });
  Rps_QuasiZone::every_zone
  (*this,
   [] (Rps_GarbageCollector&gc, Rps_QuasiZone*qz)
  {
    if (qz->is_gcmarked(gc))
      return;
    RPS_ASSERT(Rps_QuasiZone::raw_nth_zone(qz->qz_rank,gc) == qz);
    delete qz;
  });
#warning Rps_GarbageCollector::run_gc could be inncomplete or wrong
} // end Rps_GarbageCollector::run_gc

void
Rps_GarbageCollector::mark_obj(Rps_ObjectZone* ob)
{
  if (!ob) return;
  Rps_ObjectRef rob(ob);
  mark_obj(rob);
} // end of Rps_GarbageCollector::mark_obj

void
Rps_GarbageCollector::mark_value(Rps_Value val)
{
  if (val.is_empty() || val.is_int()) return;
  val.gc_mark(*this);
#warning Rps_GarbageCollector::mark_value unimplemented
} // end of Rps_GarbageCollector::mark_value

//////////////////////////////////////////////////////////// end of file garbcoll_rps.cc


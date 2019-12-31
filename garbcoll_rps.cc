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

std::atomic<Rps_GarbageCollector*> Rps_GarbageCollector::gc_this;
std::atomic<uint64_t> Rps_GarbageCollector::gc_count;

Rps_GarbageCollector::Rps_GarbageCollector(const std::function<unsigned(Rps_GarbageCollector*)> &rootmarkers) :
  gc_mtx(), gc_running(false), gc_rootmarkers(rootmarkers),
  gc_obscanque(), gc_nbscan(0), gc_nbmark(0), gc_nbdelete(0)
{
  RPS_ASSERT(gc_this.load() == nullptr);
  gc_this.store(this);
  gc_count.fetch_add(1);
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
  double startrealt = rps_monotonic_real_time();
  double startcput = rps_process_cpu_time();
  Rps_GarbageCollector the_gc;
  auto gcnt = Rps_GarbageCollector::gc_count.load();
  RPS_INFORM("rps_garbage_collect before run; count#%ld",
             gcnt);
  the_gc.run_gc();
  double endrealt = rps_monotonic_real_time();
  double endcput = rps_process_cpu_time();
  auto nbroots = the_gc.nb_roots();
  RPS_INFORM("rps_garbage_collect completed; count#%ld, %ld roots, %ld scans, %ld marks, %ld deletions, real %.3f, cpu %.3f sec",
             gcnt, (long) nbroots, (long)(the_gc.nb_scans()),  (long)(the_gc.nb_marks()),  (long)(the_gc.nb_deletions()),
             endrealt-startrealt, endcput-startcput);
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
Rps_GarbageCollector::mark_gcroots(void)
{
  unsigned nbroots=0;
  unsigned*pnbroots= &nbroots;
  rps_each_root_object([=](Rps_ObjectRef obr)
  {
    obr.gc_mark(*this);
    (*pnbroots)++;
  });
  ///
  /// mark the hardcoded global roots
#define RPS_INSTALL_ROOT_OB(Oid)    {           \
   if (RPS_ROOT_OB(Oid))                        \
     { RPS_ROOT_OB(Oid).gc_mark(*this); };	\
  };
#include "generated/rps-roots.hh"
  ///
  /// mark the hardcoded global symbols
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Nam)  {	\
   if (RPS_SYMB_OB(Nam))			\
     { RPS_SYMB_OB(Nam).gc_mark(*this); };	\
};
#include "generated/rps-names.hh"
  ///
  if (gc_rootmarkers)
    nbroots += gc_rootmarkers(this);
  gc_nbroots = nbroots;
} // end Rps_GarbageCollector::mark_gcroots


void
Rps_GarbageCollector::run_gc(void)
{
  RPS_ASSERT(!gc_running.load());
  gc_running.store(true);
  Rps_QuasiZone::run_locked_gc
  (*this,
   [] (Rps_GarbageCollector&gc)
  {
    Rps_QuasiZone::clear_all_gcmarks(gc);
    gc.mark_gcroots();
    Rps_PayloadSymbol::gc_mark_strong_symbols(&gc);
    while (!gc.gc_obscanque.empty())
      {
        auto obfront = gc.gc_obscanque.front();
        gc.gc_obscanque.pop_front();
        RPS_ASSERT(obfront);
        obfront->mark_gc_inside(gc);
        gc.gc_nbscan++;
      };
  });
  Rps_QuasiZone::every_zone
  (*this,
   [] (Rps_GarbageCollector&gc, Rps_QuasiZone*qz)
  {
    gc.gc_nbmark++;
    if (qz->is_gcmarked(gc))
      return;
    RPS_ASSERT(Rps_QuasiZone::raw_nth_zone(qz->qz_rank,gc) == qz);
    delete qz;
    gc.gc_nbdelete++;
  });
  gc_running.store(false);
#warning Rps_GarbageCollector::run_gc could be incomplete or wrong
} // end Rps_GarbageCollector::run_gc

void
Rps_GarbageCollector::mark_obj(Rps_ObjectZone* ob)
{
  if (!ob) return;
  Rps_ObjectRef rob(ob);
  mark_obj(rob);
} // end of Rps_GarbageCollector::mark_obj

void
Rps_GarbageCollector::mark_value(Rps_Value val, unsigned depth)
{
  if (val.is_empty() || val.is_int()) return;
  val.gc_mark(*this,depth);
} // end of Rps_GarbageCollector::mark_value

//////////////////////////////////////////////////////////// end of file garbcoll_rps.cc


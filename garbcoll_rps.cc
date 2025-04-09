/****************************************************************
 * file garbcoll_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the garbage collector and some code related to call frames.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_garbcoll_shortgitid[];
const char rps_garbcoll_shortgitid[]= RPS_SHORTGITID;


std::atomic<Rps_GarbageCollector*> Rps_GarbageCollector::gc_this_;
std::atomic<uint64_t> Rps_GarbageCollector::gc_count_;

Rps_GarbageCollector::Rps_GarbageCollector(const std::function<void(Rps_GarbageCollector*)> &rootmarkers) :
  gc_mtx(), gc_running(false), gc_magic(_gc_magicnum_),
  gc_rootmarkers(rootmarkers),
  gc_obscanque(),
  gc_nbscan(0), gc_nbmark(0), gc_nbdelete(0), gc_nbroots(0),
  gc_startrealtime(rps_wallclock_real_time()),
  gc_startelapsedtime(rps_elapsed_real_time()),
  gc_startprocesstime(rps_process_cpu_time())
{
  RPS_ASSERT(gc_this_.load() == nullptr);
  gc_this_.store(this);
  gc_count_.fetch_add(1);
} // end Rps_GarbageCollector::Rps_GarbageCollector


Rps_GarbageCollector::~Rps_GarbageCollector()
{
  RPS_ASSERT(is_valid_garbcoll());
  RPS_ASSERT(gc_this_.load() == this);
  RPS_ASSERT(gc_running.load() == false);
  RPS_ASSERT(gc_obscanque.empty());
  gc_this_.store(nullptr);
  gc_magic = 0;
} // end Rps_GarbageCollector::~Rps_GarbageCollector

void
Rps_CallFrame::gc_mark_frame(Rps_GarbageCollector* gc)
{
  RPS_ASSERT(gc != nullptr);
  //
  if (!cfram_descr.is_empty() && cfram_descr)
    cfram_descr->gc_mark(*gc);
  if (!cfram_state.is_empty() && cfram_state.is_ptr())
    cfram_state.as_ptr()->gc_mark(*gc,0);
  if (cfram_clos)
    cfram_clos.as_ptr()->gc_mark(*gc,0);
  if (cfram_marker)
    cfram_marker(gc);
  unsigned siz=cfram_size;
  if (cfram_xtradata)
    {
      void**frdata = reinterpret_cast<void**>(cfram_xtradata);
      for (unsigned ix=0; ix<siz; ix++)
        {
          Rps_Value curval(frdata[ix], this);
          if (!curval.is_empty() && curval.is_ptr())
            curval.as_ptr()->gc_mark(*gc,0);
        };
    }
} // end Rps_CallFrame::gc_mark_frame i.e.  Rps_ProtoCallFrame::gc_mark_frame


std::atomic<int> Rps_ProtoCallFrame::_cfram_output_depth_(16);

void // this is Rps_ProtoCallFrame::output
Rps_CallFrame::output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  out << "𝚫" /*U+1D6AB MATHEMATICAL BOLD CAPITAL DELTA*/ << depth
      << "[{" << cfram_descr << "/" << cfram_state;
  if (cfram_rankstate)
    out << "#" << cfram_rankstate;
  if (cfram_clos)
    out << "" << cfram_clos;
  Rps_CallFrameOutputSig_t*outputter = cfram_outputter.load();
  if (outputter && depth < maxdepth)
    {
      out << ":";
      (*outputter)(out,this, depth, maxdepth);
    }
  out << "}]" << std::endl;
  if ((int)depth<_cfram_output_depth_.load() && cfram_prev && depth < maxdepth)
    cfram_prev->output(out, depth+1, maxdepth);
} // end of Rps_CallFrame::output i.e. Rps_ProtoCallFrame::output


static std::atomic<bool> rps_gc_forbidden;

void
rps_forbid_garbage_collection(void)
{
  rps_gc_forbidden.store(true);
} // end rps_forbid_garbage_collection

void
rps_allow_garbage_collection(void)
{
  rps_gc_forbidden.store(false);
} // end rps_allow_garbage_collection

/* The top level function to call the garbage collector; the optional
   argument C++ std::function is marking more local data, e.g. calling
   Rps_ObjectRef::gc_mark or Rps_Value::gc_mark or some
   Rps_GarbageCollector::mark??? routine */
void
rps_garbage_collect (std::function<void(Rps_GarbageCollector*)>* pfun)
{
  if (rps_gc_forbidden.load())
    {
      RPS_WARNOUT("garbage collection is forbidden from "
                  <<  rps_current_pthread_name() << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_garbage_collect"));
      return;
    };
#warning TODO: we might want to wait half a second in rps_garbage_collect
  // e.g. in generated or hand-written plugins) since in some C++ code
  // (e.g. called by graphical toolkits or numerical routines),
  // quickly running external C++ library routines incompatible with a
  // moving garbage collector might want to allocate some RefPerSys
  // data but would be forbidden to run its garbage collector for a
  // short time.
  RPS_ASSERT(Rps_GarbageCollector::gc_this_.load() == nullptr);
  Rps_GarbageCollector the_gc([=](Rps_GarbageCollector*gc)
  {
    if (pfun)
      (*pfun)(gc);
  });
  auto gcnt = Rps_GarbageCollector::gc_count_.load();
  RPS_INFORM("rps_garbage_collect before run; count#%ld",
             gcnt);
  the_gc.run_gc();
  auto nbroots = the_gc.nb_roots();
  RPS_INFORM("rps_garbage_collect completed; count#%ld, %ld roots, %ld scans,"
             " %ld marks, %ld deletions, real %.3f, cpu %.3f sec",
             gcnt, (long) nbroots, (long)(the_gc.nb_scans()),  (long)(the_gc.nb_marks()),  (long)(the_gc.nb_deletions()),
             the_gc.elapsed_time(), the_gc.process_time());
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
  rps_each_root_object([=](Rps_ObjectRef obr)
  {
    this->mark_root_objectref(obr);
  });
  rps_garbcoll_application(*this);
  ///
  /// mark the hardcoded global roots
#define RPS_INSTALL_ROOT_OB(Oid)    {     \
   if (RPS_ROOT_OB(Oid))        \
     { this->mark_root_objectref(RPS_ROOT_OB(Oid)); };  \
  };
#include "generated/rps-roots.hh"

  ///
  /// mark the hardcoded global symbols
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Nam)  {   \
   if (RPS_SYMB_OB(Nam))        \
     { this->mark_root_objectref(RPS_SYMB_OB(Nam)); };  \
};
#include "generated/rps-names.hh"

  ///
  /// mark the constants
#define RPS_INSTALL_CONSTANT_OB(Oid) {    \
  if (rpskob##Oid)        \
    this->mark_root_objectref(rpskob##Oid); \
};
  Rps_PayloadUnixProcess::gc_mark_active_processes(*this);
#include "generated/rps-constants.hh"
  ///
  if (gc_rootmarkers)
    gc_rootmarkers(this);
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

void
rps_garbcoll_application(Rps_GarbageCollector&gc)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(gc.is_valid_garbcoll());
#warning incomplete rps_garbcoll_application
  RPS_WARNOUT("incomplete rps_garbcoll_application " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_garbcoll_application"));
} // end rps_garbcoll_application

//////////////////////////////////////////////////////////// end of file garbcoll_rps.cc


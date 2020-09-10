/****************************************************************
 * file main_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It implements the agenda mechanism.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
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


std::recursive_mutex Rps_Agenda::agenda_mtx_;
std::condition_variable_any Rps_Agenda::agenda_changed_condvar_;

std::deque<Rps_ObjectRef> Rps_Agenda::agenda_fifo_[Rps_Agenda::AgPrio__Last];

std::atomic<unsigned long>  Rps_Agenda::agenda_add_counter_;
std::atomic<bool> Rps_Agenda::agenda_is_running_;

const char*
Rps_Agenda::agenda_priority_names[Rps_Agenda::AgPrio__Last];


void
Rps_Agenda::initialize(void)
{
  agenda_priority_names[AgPrio_Low]= "low_priority";
  agenda_priority_names[AgPrio_Normal]= "normal_priority";
  agenda_priority_names[AgPrio_High]= "high_priority";
} // end Rps_Agenda::initialize

void
Rps_Agenda::gc_mark(Rps_GarbageCollector&gc)
{
  std::lock_guard<std::recursive_mutex> gu(agenda_mtx_);
  for (int ix=AgPrio_Low; ix<AgPrio__Last; ix++)
    {
      auto& curfifo = agenda_fifo_[ix];
      for (auto it: curfifo)
        {
          Rps_ObjectRef ob = *it;
          if (ob)
            ob->gc_mark(gc);
        }
    }
} // end Rps_Agenda::gc_mark

void
Rps_Agenda::dump_scan_agenda(Rps_Dumper*du)
{
  RPS_ASSERT (du != nullptr);
  std::lock_guard<std::recursive_mutex> gu(agenda_mtx_);
  for (int ix=AgPrio_Low; ix<AgPrio__Last; ix++)
    {
      auto& curfifo = agenda_fifo_[ix];
      for (auto it: curfifo)
        {
          Rps_ObjectRef ob = *it;
          if (ob)
            rps_dump_scan_object(du, ob);
        };
    }
} // end Rps_Agenda::dump_scan_agenda

void
Rps_Agenda::dump_json_agenda(Rps_Dumper*du, Json::Value&jv)
{
  RPS_ASSERT (du != nullptr);
  std::lock_guard<std::recursive_mutex> gu(agenda_mtx_);
  jv["payload"] = "agenda";
  for (int ix=AgPrio_Low; ix<AgPrio__Last; ix++)
    {
      auto& curfifo = agenda_fifo_[ix];
      const char*prioname = agenda_priority_names[ix];
      RPS_ASSERT(prioname != nullptr);
      if (!curfifo.empty())
        {
          Json::Value jseq(Json::arrayValue);
          for (auto it: curfifo)
            {
              Rps_ObjectRef ob = *it;
              if (ob && rps_is_dumpable_objref(du, ob))
                {
                  Json::Value job = rps_dump_json_objectref(du, ob);
                  jseq.append(job);
                }
            };
          jv[prioname] = jseq;
        }
    }
} // end Rps_Agenda::dump_json_agenda

void
Rps_Agenda::add_tasklet(agenda_prio_en prio, Rps_ObjectRef obtasklet)
{
  if (obtasklet.is_empty())
    return;
  auto obztask = obtasklet.to_object();
  if (!obztask)
    return;
  if ((int)prio < (int)AgPrio_Low || (int)prio >= AgPrio__Last)
    return;
  std::lock_guard<std::recursive_mutex> gu(agenda_mtx_);
  agenda_fifo_[prio].push_back(obtasklet);
  agenda_add_counter_.fetch_add(1);
  Rps_Agenda::agenda_changed_condvar_.notify_all();
} // end Rps_Agenda::add_tasklet


///// fetch a runnable tasklet from the agenda and remove it from there...
Rps_ObjectRef
Rps_Agenda::fetch_tasklet_to_run(void)
{
  Rps_ObjectRef res;
  std::lock_guard<std::recursive_mutex> gu(agenda_mtx_);
  for (int prio = (int)AgPrio_High; prio >= (int)AgPrio_Low; prio--)
    {
      auto& curfifo = agenda_fifo_[agenda_prio_en(prio)];
      if (curfifo.empty())
        continue;
      res = curfifo.front();
      curfifo.pop_front();
      return res;
    }
  return nullptr;
} // end Rps_Agenda::fetch_tasklet_to_run


/// the below function is the body of worker threads running the agenda
void
Rps_Agenda::run_agenda_worker(int ix)
{
  using namespace std::chrono_literals;
  if (ix<=0 || ix>rps_nbjobs)
    RPS_FATALOUT("run_agenda_worker invalid index#" << ix
                 << " for " << rps_nbjobs << " jobs");
  char pthname[16];
  memset (pthname, 0, sizeof(pthname));
  snprintf(pthname, sizeof(pthname), "rps-agw#%d", ix);
  pthread_setname_np(pthread_self(), pthname);
  RPS_LOCALFRAME(RPS_ROOT_OB(_1aGtWm38Vw701jDhZn), //the_agenda,
                 nullptr, // no caller frame
                 Rps_ObjectRef obtasklet;
                 Rps_InstanceValue descrval;
                 Rps_ClosureValue clostodo;
                );
  /// the descriptive value of our call frame
  _f.descrval =
    Rps_InstanceValue(RPS_ROOT_OB(_3s7ztCCoJsj04puTdQ),//agenda
  {Rps_Value((intptr_t)ix)});
  _.set_state_value(_f.descrval);
  long count = 0;
  while (agenda_is_running_.load())
    {
      try
        {
          count++;
          _f.obtasklet = Rps_Agenda::fetch_tasklet_to_run();
          Rps_PayloadTasklet*taskpayl = nullptr;
          if (_f.obtasklet)
            {
              taskpayl = _f.obtasklet->get_dynamic_payload<Rps_PayloadTasklet>();
              if (taskpayl && taskpayl->owner() == _f.obtasklet)
                _f.clostodo = taskpayl->todo_closure();
              if (_f.clostodo)
                {
                  _f.clostodo.apply1(&_, _f.obtasklet);
                }
            }
          else   // no tasklet, we wait for changes in agenda
            {
              Rps_Agenda::agenda_changed_condvar_.wait_for(agenda_mtx_, 500ms+ix*10ms);
            }
        }
      catch (std::exception& exc)
        {
          RPS_WARNOUT("run_agenda_worker " << pthname
                      << " got exception " << exc.what()
                      << " count#" << count
                      << " for tasklet " << _f.obtasklet
                      << " doing " << _f.clostodo);
        }
    }
} // end Rps_Agenda::run_agenda_worker

//// loading of agenda related payload
void
rpsldpy_agenda(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  auto paylagenda = obz->put_new_plain_payload<Rps_PayloadAgenda>();
  for (int  ix= Rps_Agenda::AgPrio_Low; ix< Rps_Agenda::AgPrio__Last; ix++)
    {
      const char*prioname =  Rps_Agenda::agenda_priority_names[ix];
      RPS_ASSERT(prioname != nullptr);
      auto jseq = jv [prioname];
      if (jseq.type() == Json::arrayValue)
        {
          unsigned seqsiz = jseq.size();
          for (unsigned ix=0; ix<seqsiz; ix++)
            {
              Json::Value jvcurelem = jseq[ix];
              auto obelem = Rps_ObjectRef(jvcurelem, ld);
              if (obelem)
                {
                  Rps_Agenda::add_tasklet((Rps_Agenda::agenda_prio_en)ix, obelem);
                }
            }
        }
    }
  RPS_DEBUG_LOG(LOAD, "incomplete rpsldpy_agenda obz=" << obz
                << " spacid=" << spacid
                << " lineno=" << lineno
                << RPS_FULL_BACKTRACE_HERE(1, "rpsldpy_agenda"));
#warning incomplete rpsldpy_agenda
} // end of rpsldpy_agenda

Rps_PayloadAgenda::~Rps_PayloadAgenda()
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
} // end Rps_PayloadAgenda::~Rps_PayloadAgenda

void
Rps_PayloadAgenda::gc_mark(Rps_GarbageCollector&gc) const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  Rps_Agenda::gc_mark(gc);
} // end Rps_PayloadAgenda::gc_mark

void
Rps_PayloadAgenda::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  RPS_ASSERT (du != nullptr);
  Rps_Agenda::dump_scan_agenda(du);
} // end Rps_PayloadAgenda::dump_scan

void
Rps_PayloadAgenda::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  RPS_ASSERT (du != nullptr);
  Rps_Agenda::dump_json_agenda(du,jv);
} // end Rps_PayloadAgenda::dump_json_content


bool
Rps_PayloadAgenda::is_erasable() const
{
  RPS_ASSERT (owner() == Rps_Agenda::the_agenda());
  return false;
} // end Rps_PayloadAgenda::is_erasable




////////////////////////////////////////////////////////////////
////////////// TASKLETS

//// loading of tasklet related payload
void
rpsldpy_tasklet(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  auto payltasklet = obz->put_new_plain_payload<Rps_PayloadTasklet>();
  if (jv.isMember("tasklet_todo"))
    {
      auto jtodo = jv["tasklet_todo"];
      auto jdelay = jv["tasklet_obsolete_delay"];
      payltasklet->tasklet_todoclos = Rps_ClosureValue(Rps_Value(jtodo,ld).as_closure());
      payltasklet->tasklet_obsoltime = rps_wallclock_real_time() + jdelay.asDouble();
    }
  RPS_DEBUG_LOG(LOAD, "rpsldpy_tasklet obz=" << obz
                << " spacid=" << spacid
                << " lineno=" << lineno
                << RPS_FULL_BACKTRACE_HERE(1, "rpsldpy_tasklet"));
} // end of rpsldpy_tasklet

Rps_PayloadTasklet::~Rps_PayloadTasklet()
{
  RPS_WARNOUT("unimplemented Rps_PayloadTasklet::~Rps_PayloadTasklet this="
              << (void*)this
              << RPS_FULL_BACKTRACE_HERE(1, "~Rps_PayloadTasklet"));
} // end Rps_PayloadTasklet::~Rps_PayloadTasklet

void
Rps_PayloadTasklet::gc_mark(Rps_GarbageCollector&gc) const
{
  if (tasklet_todoclos)
    {
      if (tasklet_obsoltime < gc.start_real_time())
        gc.mark_value(tasklet_todoclos);
    }
} // end Rps_PayloadTasklet::gc_mark

void
Rps_PayloadTasklet::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT (du != nullptr);
  if (!tasklet_transient && tasklet_todoclos)
    {
      if (tasklet_obsoltime < rps_dump_start_wallclock_time(du))
        rps_dump_scan_value(du, tasklet_obsoltime,0);
    }
} // end Rps_PayloadTasklet::dump_scan


void
Rps_PayloadTasklet::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT (du != nullptr);
  jv["payload"] = "tasklet";
  if (!tasklet_transient && tasklet_todoclos)
    {
      if (tasklet_obsoltime < rps_dump_start_wallclock_time(du))
        {
          jv["tasklet_todo"] = rps_dump_json_value(du, tasklet_todoclos);
          jv["tasklet_obsolete_delay"] =  rps_dump_start_wallclock_time(du) - tasklet_obsoltime;
        }
    }
  RPS_DEBUG_LOG(DUMP,"Rps_PayloadTasklet::dump_json_content this="
                << (void*)this << std::endl
                << jv << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadTasklet::dump_json_content"));
} // end Rps_PayloadTasklet::dump_json_content

bool
Rps_PayloadTasklet::is_erasable() const
{
  // a tasklet might be mutated to something else, even if I cannot
  // imagine why that could be useful.
  return true;
} // end Rps_PayloadTasklet::is_erasable


//// end of file agenda_rps.cc

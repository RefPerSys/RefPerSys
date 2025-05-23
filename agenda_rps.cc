/****************************************************************
 * file agenda_rps.cc
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
 *      © Copyright (C) 2020 - 2025 The Reflective Persistent System Team
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


extern "C" const char rps_agenda_gitid[];
const char rps_agenda_gitid[]= RPS_GITID;

extern "C" const char rps_agenda_shortgitid[];
const char rps_agenda_shortgitid[]= RPS_SHORTGITID;

extern "C" const char rps_agenda_date[];
const char rps_agenda_date[]= __DATE__;

unsigned long rps_run_delay;

double Rps_Agenda::agenda_timeout;
thread_local int rps_curthread_ix;
thread_local Rps_CallFrame* rps_curthread_callframe;

std::recursive_mutex Rps_Agenda::agenda_mtx_;
std::condition_variable_any Rps_Agenda::agenda_changed_condvar_;

std::deque<Rps_ObjectRef> Rps_Agenda::agenda_fifo_[Rps_Agenda::AgPrio__Last];

std::atomic<unsigned long>  Rps_Agenda::agenda_add_counter_;
std::atomic<bool> Rps_Agenda::agenda_is_running_;
std::atomic<std::thread*> Rps_Agenda::agenda_thread_array_[RPS_NBJOBS_MAX+2];

const char*
Rps_Agenda::agenda_priority_names[Rps_Agenda::AgPrio__Last];
std::atomic<Rps_Agenda::workthread_state_en>
Rps_Agenda::agenda_work_thread_state_[RPS_NBJOBS_MAX+2];
std::atomic<bool> Rps_Agenda::agenda_needs_garbcoll_;
std::atomic<uint64_t> Rps_Agenda::agenda_cumulw_gc_;
std::atomic<Rps_CallFrame*> Rps_Agenda::agenda_work_gc_callframe_[RPS_NBJOBS_MAX+2];
std::atomic<Rps_CallFrame**> Rps_Agenda::agenda_work_gc_current_callframe_ptr[RPS_NBJOBS_MAX+2];
void
Rps_Agenda::initialize(void)
{
  agenda_priority_names[AgPrio_Low]= "low_priority";
  agenda_priority_names[AgPrio_Normal]= "normal_priority";
  agenda_priority_names[AgPrio_High]= "high_priority";
  if (rps_run_delay > 0)
    {
      agenda_timeout =  rps_elapsed_real_time() + (double)rps_run_delay;
      RPS_ASSERT(agenda_timeout >0.0);
    }
  else
    agenda_timeout = 0.0;
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "Rps_Agenda::initialize agenda_timeout=" << agenda_timeout
                << " curthr:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_Agenda::initialize"));
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
  snprintf(pthname, sizeof(pthname), "rps-agw#%hd", (short) ix);
  pthread_setname_np(pthread_self(), pthname);
  rps_curthread_ix = ix;
  rps_curthread_callframe = nullptr;
  agenda_work_gc_current_callframe_ptr[ix].store(&rps_curthread_callframe);
  RPS_LOCALFRAME(RPS_ROOT_OB(_1aGtWm38Vw701jDhZn), //the_agenda,
                 RPS_NULL_CALL_FRAME, // no caller frame
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
  agenda_work_thread_state_[ix].store(WthrAg_Idle);
  // wait for this thread to be in agenda_thread_array_
  {
    /// we sleep a different amount of time to help ensure other threads do
    /// start...
    std::this_thread::sleep_for(32ms + ix * 16ms);
    int cnt=0;
    constexpr int maxloop = 100;
    for (cnt=0; cnt<=maxloop; cnt++)
      {
        std::thread*curthr = agenda_thread_array_[ix].load();
        if (!curthr)
          {
            std::this_thread::sleep_for(1ms);
            continue;
          }
        if (curthr->get_id() == std::this_thread::get_id())
          break;
        RPS_POSSIBLE_BREAKPOINT();
        std::this_thread::sleep_for(2ms);
      }
    if (cnt>=maxloop) // won't happen in practice
      RPS_FATALOUT("run_agenda_worker: failed to be in agenda_thread_array_[" << ix << "]");
  }
  ////
  RPS_POSSIBLE_BREAKPOINT();
  ////
  while (agenda_is_running_.load())
    {
      if (Rps_Agenda::agenda_cumulw_gc_.load() + Rps_Agenda::agenda_gc_threshold
          > Rps_QuasiZone::cumulative_allocated_wordcount())
        {
          Rps_Agenda::agenda_needs_garbcoll_.store(true);
          std::this_thread::sleep_for(1ms/2);
        }
      if (Rps_Agenda::agenda_needs_garbcoll_.load())
        Rps_Agenda::do_garbage_collect(ix, &_);
      else
        try
          {
            count++;
            switch (agenda_work_thread_state_[ix].load())
              {
              case WthrAg_Idle:
              case WthrAg_Run:
              {
                _f.obtasklet = Rps_Agenda::fetch_tasklet_to_run();
                Rps_PayloadTasklet*taskpayl = nullptr;
                if (_f.obtasklet)
                  {
                    taskpayl = _f.obtasklet->get_dynamic_payload<Rps_PayloadTasklet>();
                    if (taskpayl && taskpayl->owner() == _f.obtasklet)
                      _f.clostodo = taskpayl->todo_closure();
                    if (_f.clostodo)
                      {
                        Rps_Agenda::agenda_work_thread_state_[ix].store(WthrAg_Run);
                        _f.clostodo.apply1(&_, _f.obtasklet);
                      }
                  }
                else   // no tasklet, we wait for changes in agenda
                  {
                    Rps_Agenda::agenda_work_thread_state_[ix].store(WthrAg_Idle);
                    Rps_Agenda::agenda_changed_condvar_.wait_for(agenda_mtx_, 500ms+ix*10ms);
                  }
              }
              break;
              case WthrAg_GC:
              {
                std::this_thread::sleep_for(1ms);
                Rps_Agenda::agenda_changed_condvar_.notify_all();
              }
              break;
              case WthrAg_EndGC:
              {
                agenda_work_thread_state_[ix].store(WthrAg_Idle);
                Rps_Agenda::agenda_changed_condvar_.notify_all();
                // so on the next loop, the worker thread will try to fetch and run a tasklet
              }
              break;
              default:
                break;
              };      // end switch agenda_work_thread_state_[ix].load()
          } /// ending try...
        catch (std::exception& exc)
          {
            RPS_WARNOUT("run_agenda_worker " << pthname
                        << " got exception " << exc.what()
                        << " count#" << count
                        << " for tasklet " << _f.obtasklet
                        << " doing " << _f.clostodo);
            Rps_Agenda::agenda_work_thread_state_[ix].store(WthrAg_Idle);
          }
    };        // end while (agenda_is_running_.load())
  Rps_Agenda::agenda_changed_condvar_.notify_all();
  Rps_Agenda::agenda_work_thread_state_[ix].store(WthrAg__None);
} // end Rps_Agenda::run_agenda_worker


//// Do garbage collection from agenda worker threads. The actual GC
//// is running when ix == 1, so in the first worker thread. Other
//// worker threads are idle and waiting....
void
Rps_Agenda::do_garbage_collect(int ix, Rps_CallFrame*callframe)
{
  RPS_ASSERT(ix>=0 && ix<=RPS_NBJOBS_MAX);
  RPS_ASSERT(ix == rps_curthread_ix);
  RPS_ASSERT(agenda_work_gc_callframe_[ix].load() == nullptr);
  agenda_work_thread_state_[ix].store(Rps_Agenda::WthrAg_GC);
  agenda_work_gc_callframe_[ix].store(callframe);
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1ms/8);
  Rps_Agenda::agenda_changed_condvar_.notify_all();
  std::this_thread::sleep_for(1ms/16);
  /// at this point, we should wait for every other worker thread to be in WthrAg_GC state....
  bool every_worker_is_gc = false;
  {
    constexpr int maxwaitloop = 32;
    int loopcnt=0;
    while (!every_worker_is_gc)
      {
        std::unique_lock<std::recursive_mutex> ulock(agenda_mtx_);
        agenda_changed_condvar_.wait_for(ulock, 50ms+ix*10ms, [=,&every_worker_is_gc]
        {
          for (int wix=1; wix<rps_nbjobs; wix++)
            {
              std::thread*curthr = agenda_thread_array_[wix].load();
              if (!curthr)
                continue;
              if (agenda_work_thread_state_[wix].load() != Rps_Agenda::WthrAg_GC)
                return false;
            };
          every_worker_is_gc = true;
          return true;
        });
        if (loopcnt++ > maxwaitloop) /// should never happen!
          RPS_FATALOUT("Rps_Agenda::do_garbage_collect ix=" << ix
                       << " callframe=" << Rps_ShowCallFrame(callframe)
                       << " timed out waiting for other worker threads to GC");
      }
  }
  RPS_ASSERT(every_worker_is_gc);
  /// At this point, we do know that every worker thread is in garbage
  /// collection state, so is NOT running, don't change the call
  /// stack, so is NOT ALLOCATING.... The GC is then permitted to scan
  /// the call stacks in agenda_work_gc_callframe_ ... The first
  /// worker thread is doing the actual GC work.
  if (ix==1)
    {
      std::unique_lock<std::recursive_mutex> ulock(agenda_mtx_);
      std::function<void(Rps_GarbageCollector*)> gcfun([&](Rps_GarbageCollector*gc)
      {
        for (int thrix=1; thrix<rps_nbjobs; thrix++)
          {
            auto pcallfr = agenda_work_gc_callframe_[thrix].load();
            if (!pcallfr)
              continue;
            gc->mark_call_stack(pcallfr);
            agenda_work_gc_callframe_[thrix].store(nullptr);
          }
      });
      rps_garbage_collect(&gcfun);
    };
  std::this_thread::sleep_for(1ms/8);
  // Every thread which is in GC state switches to EndGC state.
  for (int wix=1; wix<rps_nbjobs; wix++)
    {
      std::thread*curthr = agenda_thread_array_[wix].load();
      if (!curthr)
        continue;
      if (agenda_work_thread_state_[wix].load() == Rps_Agenda::WthrAg_GC)
        agenda_work_thread_state_[wix].store(Rps_Agenda::WthrAg_EndGC);
    };
  std::this_thread::sleep_for(1ms/8);
  Rps_Agenda::agenda_changed_condvar_.notify_all();
  // at the next iteration of Rps_Agenda::run_agenda_worker, the
  // thread will resume usual work if agenda is non-empty....
} // end of Rps_Agenda::do_garbage_collect

/// start and run the agenda mechanism. This does not return till the
/// agenda has stopped.
void
rps_run_agenda_mechanism(int nbjobs)
{
  using namespace std::chrono_literals;
  if (nbjobs < RPS_NBJOBS_MIN)
    RPS_FATALOUT("rps_run_agenda_mechanism: too little number of jobs "
                 << nbjobs << " should be at least " << RPS_NBJOBS_MIN);
  if (nbjobs>RPS_NBJOBS_MAX)
    RPS_FATALOUT("rps_run_agenda_mechanism: too much number of jobs "
                 << nbjobs << " should be at most " << RPS_NBJOBS_MAX);
  if (nbjobs>rps_nbjobs)
    {
      RPS_WARNOUT("rps_run_agenda_mechanism: number of jobs "
                  << nbjobs << " reduced to " << rps_nbjobs);
      nbjobs = rps_nbjobs;
    };
  /*** TODO (1):
   *
   * We are running the event loop in one posix thread, which is
   * mostly sleeping, and waiting for SIGCHLD of
   * Rps_PayloadUnixProcess and polling Rps_PayloadPopenedFile etc...
   * See C++ code in file eventloop_rps.cc.  See Todo §2 below
   **/
  Rps_Agenda::agenda_is_running_.store(true);
  /// start all worker threads
  for (int ix=1; ix<nbjobs; ix++)
    {
      std::lock_guard<std::recursive_mutex> gu(Rps_Agenda::agenda_mtx_);
      auto curthr = new std::thread(Rps_Agenda::run_agenda_worker, ix);
      Rps_Agenda::agenda_thread_array_[ix].store(curthr);
    }
  while (true)
    {
      /*** TODO (2):
       *
       * Cooperation with unix processes and popen-ed commands is
       * needed in the agenda. See our file eventloop_rps.cc.  We do
       * need to use poll(2) system call and waitpid(2) system calls
       * and/or to handle SIGCHLD signals in that eventloop_rps.cc
       * file.  See Todo §1 above.
       ***/
      Rps_Agenda::agenda_changed_condvar_.wait_for(Rps_Agenda::agenda_mtx_,
          30ms + 1ms * Rps_Random::random_quickly_4bits());
      if (Rps_Agenda::agenda_is_running_.load())
        continue;
      for (int ix=1; ix<nbjobs; ix++)
        {
          auto thrp = Rps_Agenda:: agenda_thread_array_[ix].load();
          if (!thrp)
            continue;
          thrp->join();
          delete thrp;
          Rps_Agenda::agenda_thread_array_[ix].store(nullptr);
        }
    }
} // end of rps_run_agenda_mechanism

void
rps_stop_agenda_mechanism(void)
{
  Rps_Agenda::agenda_is_running_.store(false);
  Rps_Agenda::agenda_changed_condvar_.notify_all();
} // end of rps_stop_agenda_mechanism



//// loading of agenda related payload
void
rpsldpy_agenda(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_ASSERT(spacid);
  RPS_ASSERT(lineno>0);
  if (obz != RPS_ROOT_OB(_1aGtWm38Vw701jDhZn))   // the agenda rps_rootob_1aGtWm38Vw701jDhZn
    {
      RPS_POSSIBLE_BREAKPOINT();
      RPS_FATALOUT("in space " << spacid << " line " << lineno
                   << " obz=" << Rps_ObjectRef(obz)
                   << " the_agenda is RPS_ROOT_OB(_1aGtWm38Vw701jDhZn)");
    }
  auto paylagenda = obz->put_new_plain_payload<Rps_PayloadAgenda>();
  RPS_ASSERT(paylagenda);
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
  if (tasklet_permanent && tasklet_todoclos)
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
  if (tasklet_permanent && tasklet_todoclos)
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
  RPS_WARNOUT("Rps_PayloadTasklet::is_erasable() still a stub"
              << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadTasklet::is_erasable"));
  // a tasklet might be mutated to something else, even if I cannot
  // imagine why that could be useful.
  return true;
} // end Rps_PayloadTasklet::is_erasable

/// adding a pragma which works for both GCC and Clang
#pragma message "compiled agenda_rps.cc"

//// end of file agenda_rps.cc

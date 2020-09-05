/****************************************************************
 * file foxevloop_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the high-level FOX event-loop related
 *      code. See https://fox-toolkit.org/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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

#include "headfox_rps.hh"



extern "C" const char rps_foxevloop_gitid[];
const char rps_foxevloop_gitid[]= RPS_GITID;

extern "C" const char rps_foxevloop_date[];
const char rps_foxevloop_date[]= __DATE__;

static std::atomic<bool> rps_running_fox;

extern "C" pthread_t rps_main_gui_pthread;
pthread_t rps_main_gui_pthread;
Rps_GuiPreferences rps_gui_pref;


bool
rps_is_main_gui_thread(void)
{
  return pthread_self() == rps_main_gui_pthread;
} // end rps_is_main_gui_thread

std::string
rps_fox_version(void)
{
  std::string r ("FOX");
  char buf[32];
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%ud.%ud.%ud",
           FX::fxversion[0], FX::fxversion[1], FX::fxversion[2]);
  return std::string{"FOX "} + buf;
} // end  rps_fox_version

std::atomic<unsigned>  Rps_Todo_Base::_todo_serial_counter_;

void
Rps_Todo::output(std::ostream&out) const
{
  switch (kind())
    {
    case TODOK_function:
    {
      out << "TodoFunc#" << serial() << "@" << basename(filename()) << ":" << lineno() << '!' << timeout();
      if (label())
        out << '[' << label() << ']';
      auto todofun = as_todo_function();
      if (todofun.arg1() || todofun.arg2())
        {
          out << '{';
          if (todofun.arg1())
            out << "arg1=" << todofun.arg1() << (todofun.arg2()?",":"");
          if (todofun.arg2())
            out << "arg2=" << todofun.arg2();
          out << '}';
        };
    }
    break;
    case TODOK_closure:
    {
      out << "TodoClos#" << serial() << "@" << basename(filename()) << ":" << lineno() << '!' << timeout();
      if (label())
        out << '[' << label() << ']';
      auto todoclos = as_todo_closure();
      out << "(clos=" << todoclos.get_todo_clos();
      if (todoclos.get_todo_arg1())
        out << ", arg1="<< todoclos.get_todo_arg1();
      if (todoclos.get_todo_arg2())
        out << ", arg2="<< todoclos.get_todo_arg2();
      if (todoclos.get_todo_arg3())
        out << ", arg3="<< todoclos.get_todo_arg3();
      out << ")";
    };
    break;
    case TODOK_noop:
    {
      out << "TodoClos#" << serial() << "@" << basename(filename())
          << ":" << lineno() << '!' << timeout() << "**NOOP**";
      if (label())
        out << '[' << label() << ']';
    };
    break;
    };
} // end Rps_Todo::output

void
Rps_Todo::apply_todo(Rps_FoxEvLoop_CallFrame*cf)
{
  switch(kind())
    {
    case TODOK_closure:
      todo_closure.apply_todo_clos(cf);
      break;
    case TODOK_function:
      todo_function.apply_todo_function(cf);
      break;
    case TODOK_noop:
      break;
    }
} // end Rps_Todo::apply_todo

Rps_Todo::~Rps_Todo()
{
  RPS_ASSERT(rps_is_main_gui_thread());
};				// end Rps_Todo::~Rps_Todo

////////////////////////////////////////////////////////////////
Rps_Todo*
Rps_Todo_Collection::adding_todo(int ix, const Rps_Todo& todo)
{
  RPS_ASSERT(ix>=0 && ix < (int)_todocoll_vect.size());
  RPS_ASSERT(todo.kind() != TODOK_noop);
  Rps_Todo* tp = new Rps_Todo(todo);
  _todocoll_vect[ix].reset(tp);
  _todocoll_timemap.insert({todo.timeout(),ix});
  _todocoll_fifoqueue.push_back(ix);
  return tp;
} // end /*Rps_Todo_Collection::adding_todo*/



int
Rps_Todo_Collection::add_todo(const Rps_Todo&todo)
{
  RPS_ASSERT(!todo.is_todo_noop());
  constexpr int threshold = 15;
  int sz = (int)_todocoll_vect.size();
  if (sz < threshold || 4L*_todocoll_timemap.size() >= 3L*(unsigned)sz)
forced_push:
    {
      _todocoll_vect.push_back(nullptr);
      (void)adding_todo(sz,todo);
      RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::add_todo this@" << ((void*)this)
                    << " todo=" << todo << " -> " << sz);
      return sz;
    }
  else
    {
      int rd = Rps_Random:: random_quickly_16bits() % sz;
      for (int ix=rd; ix<sz; ix++)
        {
          if (!_todocoll_vect[ix])
            {
              (void)adding_todo(ix,todo);
              RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::add_todo this@" << ((void*)this)
                            << " todo=" << todo << " -> " << ix);
              return ix;
            }
        }
      for (int ix=rd-1; ix>=0; ix--)
        {
          if (!_todocoll_vect[ix])
            {
              (void)adding_todo(ix,todo);
              RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::add_todo this@" << ((void*)this)
                            << " todo=" << todo << " -> " << ix);
              return ix;
            }
        }
      // this goto should probably not be reached, but just in case...
      goto forced_push;
    }
} /// end Rps_Todo_Collection::add_todo

void
Rps_Todo_Collection::cleanup_done_or_old_todos(void)
{
  double curtim = rps_monotonic_real_time();
  int sz = (int)_todocoll_vect.size();
  /// cleanup the _todocoll_vect, removing noop-s there
  for (int ix=0; ix<sz; ix++)
    {
      Rps_Todo* curtodoptr = _todocoll_vect[ix].get();
      if (!curtodoptr)
        continue;
      double curtimout = curtodoptr->timeout();
      if (curtimout>curtim)
        continue;
      if (!curtodoptr->is_todo_noop())
        continue;
    }
  // clean up first old entries in _todocoll_fifoqueue
  {
    auto nextit =  _todocoll_fifoqueue.end();
    for (auto it = _todocoll_fifoqueue.begin();
         it != _todocoll_fifoqueue.end();
         it = nextit)
      {
        int curix = *it;
        RPS_ASSERT(curix >= 0 && curix < sz);
        Rps_Todo* curtodoptr = _todocoll_vect[curix].get();
        if (!curtodoptr)
          nextit = _todocoll_fifoqueue.erase(it);
        else if (curtodoptr->timeout() > curtim) break;
        else
          nextit = ++it;
      }
  }
  // clean old entries of _todocoll_timemap
  // see https://stackoverflow.com/a/263958/841108
  {
    double_to_index_iterator_t it = _todocoll_timemap.begin();
    while (it != _todocoll_timemap.end())
      {
        int curix = it->second;
        RPS_ASSERT(curix>=0 && curix<sz);
        Rps_Todo* curtodoptr = _todocoll_vect[curix].get();
        if (!curtodoptr)
          {
            ++it;
            continue;
          }
        if (curtodoptr->timeout() > curtim)
          break;
        it = _todocoll_timemap.erase(it);
      }
  }
} // end Rps_Todo_Collection::cleanup_old_or_done_todos

// run all the pending todos, those with a timeout before the current time
void
Rps_Todo_Collection::run_pending_todos(Rps_FoxEvLoop_CallFrame*cf, double curtim)
{
  if (curtim<=0.0)
    curtim = rps_monotonic_real_time();
  RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::run_pending_todos curtim=" << curtim << " cf@" << ((void*)cf)
                << ":" << std::endl << Rps_ShowCallFrame(cf));
  RPS_ASSERT (cf != nullptr);
  for (auto it : _todocoll_timemap)
    {
      int curix = it.second;
      RPS_ASSERT(curix>=0 && curix< (int)_todocoll_vect.size());
      Rps_Todo* curtodoptr = _todocoll_vect[curix].get();
      if (!curtodoptr)
        continue;
      RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::run_pending_todos curix=" << curix
                    << " curtodoptr:" << *curtodoptr);
      if (curtodoptr->is_todo_noop())
        continue;
      if  (curtodoptr->timeout() > curtim)
        return;
      RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::run_pending_todos applying todo " << *curtodoptr << " in  cf@" << ((void*)cf) << " curtim=" << curtim);
      curtodoptr->apply_todo(cf);
      RPS_DEBUG_LOG(GUI, "Rps_Todo_Collection::run_pending_todos applied todo " << *curtodoptr << " in  cf@" << ((void*)cf));
    }
  RPS_DEBUG_LOG(GUI, "end° Rps_Todo_Collection::run_pending_todos curtim=" << curtim << " cf@" << ((void*)cf)
                << ":" << std::endl << Rps_ShowCallFrame(cf));
#warning TODO: Rps_Todo_Collection::run_pending_todos should be called from Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos
} // end Rps_Todo_Collection::run_pending_todos


void
Rps_Todo_Collection::gc_mark_collected_todos(Rps_GarbageCollector*gc)
{
  RPS_ASSERT(gc && gc->is_valid_garbcoll());
  int sz = (int)_todocoll_vect.size();
  /// interate inside the vector...
  for (int ix=0; ix<sz; ix++)
    {
      Rps_Todo* curtodoptr = _todocoll_vect[ix].get();
      if (!curtodoptr)
        continue;
      switch (curtodoptr->kind())
        {
        case TODOK_noop:
          continue;
        case TODOK_function:
          continue;
        case TODOK_closure:
        {
          auto& curtodoclos = curtodoptr->as_todo_closure();
          curtodoclos.get_todo_clos().gc_mark(*gc);
          curtodoclos.get_todo_arg1().gc_mark(*gc);
          curtodoclos.get_todo_arg2().gc_mark(*gc);
          curtodoclos.get_todo_arg3().gc_mark(*gc);
        };
        continue;
        default: // should never happen
          RPS_FATALOUT("Rps_Todo_Collection::gc_mark_collected_todos corrupted ix=" << ix
                       << " curtodoptr@" << (void*)curtodoptr);
        }
    }
} // end Rps_Todo_Collection::gc_mark_collected_todos


/// static data of Rps_FoxEvLoop_CallFrame
std::atomic<long> Rps_FoxEvLoop_CallFrame::evloopfr_counter_;
std::recursive_mutex
Rps_FoxEvLoop_CallFrame::evloopfr_setmtx_;
std::set<Rps_FoxEvLoop_CallFrame*>
Rps_FoxEvLoop_CallFrame::evloopfr_set_;
std::atomic<Rps_FoxEvLoop_CallFrame*>
Rps_FoxEvLoop_CallFrame::evloopfr_curframe_;
///



Rps_FoxEvLoop_CallFrame::Rps_FoxEvLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
    Rps_ObjectRef descr, int depth, Rps_EventLoop_tag)
  :  Rps_FieldedCallFrame<Rps_FoxEvloop_Fields> (descr, callframe),
     evloopfr_todo_coll(this),
     evloopfr_mtx(),
     evloopfr_oldframe(nullptr),
     evloopfr_lineno(lineno),
     evloopfr_depth(depth),
     evloopfr_file(nullptr),
     evloopfr_serial(1+evloopfr_counter_.fetch_add(1))
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(evloopfr_depth >= 0);
  evloopfr_oldframe= std::atomic_exchange(&evloopfr_curframe_,this);
  set_outputter(outputter);
  RPS_DEBUGNL_LOG(GUI, "+°Rps_FoxEvLoop_CallFrame-constr line#" << lineno
                  << " depth#" << evloopfr_depth
                  << " callframe@" << (void*)callframe
                  << " evlserial#" << evloopfr_serial
                  << " this@" << (void*)this<< std::endl
                  << RPS_FULL_BACKTRACE_HERE(0, "Rps_FoxEvLoop_CallFrame°-constru"));
} // end Rps_FoxEvLoop_CallFrame::Rps_FoxEvLoop_CallFrame ...  Rps_EventLoop_tag


Rps_FoxEvLoop_CallFrame::Rps_FoxEvLoop_CallFrame(Rps_CallFrame*callframe, const char*filename, int lineno,
    Rps_ObjectRef descr, int depth, Rps_LoggedEventLoop_tag)

  :  Rps_FieldedCallFrame<Rps_FoxEvloop_Fields> (descr, callframe),
     evloopfr_todo_coll(this),
     evloopfr_mtx(),
     evloopfr_oldframe(nullptr),
     evloopfr_lineno(lineno),
     evloopfr_depth(depth),
     evloopfr_file(filename),
     evloopfr_serial(1+evloopfr_counter_.fetch_add(1))
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(evloopfr_depth >= 0);
  evloopfr_oldframe= std::atomic_exchange(&evloopfr_curframe_,this);
  set_outputter(outputter);
  RPS_DEBUGNL_LOG(GUI, "+°Rps_FoxEvLoop_CallFrame-constr line#" << lineno
                  << " depth#" << evloopfr_depth
                  << " callframe@" << (void*)callframe
                  << " evlserial#" << evloopfr_serial
                  << " this@" << (void*)this<< std::endl
                  << RPS_FULL_BACKTRACE_HERE(0, "Rps_FoxEvLoop_CallFrame°-constru"));
} // end Rps_FoxEvLoop_CallFrame::Rps_FoxEvLoop_CallFrame ... Rps_LoggedEventLoop_tag

Rps_FoxEvLoop_CallFrame::~Rps_FoxEvLoop_CallFrame()
{
  auto serial = evloopfr_serial;
  RPS_DEBUG_LOG(GUI, "-°Rps_FoxEvLoop_CallFrame this@" << (void*)this << " evlserial#" << evloopfr_serial << std::endl
                << RPS_FULL_BACKTRACE_HERE(0, "Rps_FoxEvLoop_CallFrame°-destruc"));
  if (rps_is_main_gui_thread())
    {
      evloopfr_curframe_.store(evloopfr_oldframe);
    }
  else
    {
      std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
      evloopfr_set_.erase(this);
    }
  RPS_DEBUG_LOG(GUI, "-°~Rps_FoxEvLoop_CallFrame-destr this@" << (void*)this << " evlserial#" << serial << std::endl);
} // end Rps_FoxEvLoop_CallFrame::~Rps_FoxEvLoop_CallFrame


void
Rps_FoxEvLoop_CallFrame::outputter(std::ostream&out,const Rps_ProtoCallFrame*cf)
{
  if (!cf)
    {
      out << "?*nullevcallframe*?";
      return;
    };
  const Rps_FoxEvLoop_CallFrame*curevframe = reinterpret_cast<const Rps_FoxEvLoop_CallFrame*>(cf);
  if (curevframe->evloopfr_file)
    out << curevframe->evloopfr_file << ":" << curevframe->evloopfr_lineno
        << "#" << curevframe->evloopfr_serial << "/d:" << curevframe->evloopfr_depth;
  else
    out <<  "?:" << curevframe->evloopfr_lineno << "#" << curevframe->evloopfr_serial
        << "/d:" << curevframe->evloopfr_depth;
  int nbtodos=0;
  {
    std::lock_guard<std::recursive_mutex> g(curevframe->evloopfr_mtx);
    nbtodos = curevframe->evloopfr_todo_coll.size();
  };
  if (nbtodos>0)
    out << "!" << nbtodos << "todos";
} // end Rps_FoxEvLoop_CallFrame::outputter



/// the object whose name is "fox_event_loop" is a RefPerSys frame
/// descriptor for FTK event loops GC call frames.
#define RPS_FOX_EVENT_LOOP_DESCR RPS_ROOT_OB(_39OsVkAJDdV00ohD5r)

Rps_FoxEvLoop_CallFrame*
Rps_FoxEvLoop_CallFrame::find_calling_event_call_frame(const Rps_CallFrame*callframe)
{
  for (const Rps_CallFrame*curcallframe = callframe;
       curcallframe != nullptr && Rps_CallFrame::is_good_call_frame(curcallframe);
       curcallframe = curcallframe->previous_call_frame())
    {
      if (curcallframe->call_frame_descriptor() == RPS_FOX_EVENT_LOOP_DESCR)
        return (Rps_FoxEvLoop_CallFrame*)(curcallframe);
    }
  return nullptr;
} // end Rps_FoxEvLoop_CallFrame::find_calling_event_call_frame

void
Rps_FoxEvLoop_CallFrame::gc_mark_todos(Rps_GarbageCollector*gc)
{
  RPS_ASSERT(gc != nullptr && gc->is_valid_garbcoll());
  std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
  evloopfr_todo_coll.gc_mark_collected_todos(gc);
} // end Rps_FoxEvLoop_CallFrame::gc_mark_todos



void
Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos(void)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_DEBUGNL_LOG(GUI, "**start Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos depth#" << evloopfr_depth
                  << " this@" << this << " evlserial#" << evloopfr_serial <<std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,"Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos*start")
                  << "... run_scheduled_fox_todos this=" << std::endl << Rps_ShowCallFrame(this)
                  << std::endl);

  std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
  RPS_DEBUG_LOG(GUI, "Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos depth#" << evloopfr_depth
                << " evlserial#" << evloopfr_serial
                << " colltodosiz=" << evloopfr_todo_coll.size());
  evloopfr_todo_coll.run_pending_todos(this);
  RPS_DEBUG_LOG(GUI, "end Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos depth#" << evloopfr_depth
                <<   " evlserial#" << evloopfr_serial<< std::endl);
}; // end Rps_FoxEvLoop_CallFrame::run_scheduled_fox_todos


Rps_FoxEvLoop_CallFrame*
Rps_FoxEvLoop_CallFrame::get_lower_evloop_callframe(void) const
{
  for (Rps_ProtoCallFrame*cf = previous_call_frame();
       cf != nullptr;
       cf = cf->previous_call_frame())
    {
      if (cf->call_frame_descriptor() == RPS_FOX_EVENT_LOOP_DESCR)
        {
          RPS_DEBUG_LOG(GUI, "get_lower_evloop_callframe this@"
                        << ((void*)this)
                        << std::endl << Rps_ShowCallFrame(this) << std::endl
                        << "... returns lower cf@" << ((void*)cf)
                        << std::endl << Rps_ShowCallFrame(cf) << std::endl);
          return reinterpret_cast<Rps_FoxEvLoop_CallFrame*>(cf);
        }
    };
  RPS_DEBUG_LOG(GUI, "get_lower_evloop_callframe this@"
                << ((void*)this)
                << std::endl << Rps_ShowCallFrame(this)
                << std::endl << " has no lower");
  return nullptr;
} // end Rps_FoxEvLoop_CallFrame::get_lower_evloop_callframe






void
rps_garbcoll_application(Rps_GarbageCollector&gc)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(gc.is_valid_garbcoll());
#warning incomplete rps_garbcoll_application
  RPS_WARNOUT("incomplete rps_garbcoll_application " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_garbcoll_application"));
} // end rps_garbcoll_application


void
rps_run_fox_gui(int &argc, char**argv)
{
  rps_main_gui_pthread = pthread_self();
  RPS_WARNOUT("incomplete rps_run_fox_gui argc=" << argc
              << " argv=" << Rps_Do_Output([=](std::ostream&out)
  {
    for (int ix=0; ix<argc; ix++)
      {
        out << " [" << ix << "]";
        if (argv[ix])
          out << "'" << argv[ix] << "'";
        else
          out << "*nil*";
      }
  })
      << std::endl
      << RPS_FULL_BACKTRACE_HERE(1, "rps_run_fox_gui"));
  // Make sure  we're linked against the right library version
  if(fxversion[0]!=FOX_MAJOR
      || fxversion[1]!=FOX_MINOR || fxversion[2]!=FOX_LEVEL)
    RPS_FATAL("FOX Library mismatch; expected version: %d.%d.%d, but found version: %d.%d.%d.\n",
              FOX_MAJOR,FOX_MINOR,FOX_LEVEL,
              fxversion[0],fxversion[1],fxversion[2]);
  RpsGui_FoxApplication app;
} // end rps_run_fox_gui


/***************** FOX application code ********************/
RpsGui_FoxApplication* RpsGui_FoxApplication::fxapp_inst;

FXDEFMAP(RpsGui_FoxApplication) RpsGui_FoxMapApplication[]=
{
  //  FXMAPFUNC(SEL_COMMAND,RpsGui_FoxApplication::ID_ABOUT,
  //          RpsGui_FoxApplication::onCmdAbout),
  //  FXMAPFUNC(SEL_COMMAND,RpsGui_FoxApplication::ID_HELP,
  //          RpsGui_FoxApplication::onCmdHelp),
};
// Object implementation
FXIMPLEMENT(RpsGui_FoxApplication,FXApp,RpsGui_FoxMapApplication,ARRAYNUMBER(RpsGui_FoxMapApplication));


RpsGui_FoxApplication::RpsGui_FoxApplication()
  : FXApp("refpersys-fox")
{
  fxapp_inst = this;
  RPS_DEBUG_LOG(GUI, "creating RpsGui_FoxApplication this@" << (void*)this
                << std::endl
                << "... from:" << std::endl
                << RPS_FULL_BACKTRACE_HERE(0, "RpsGui_FoxApplication::RpsGui_FoxApplication"));
} // end RpsGui_FoxApplication::RpsGui_FoxApplication

RpsGui_FoxApplication::~RpsGui_FoxApplication()
{
  RPS_DEBUG_LOG(GUI, "destroying RpsGui_FoxApplication this@" << (void*)this
                << std::endl
                << "... from:" << std::endl
                << RPS_FULL_BACKTRACE_HERE(0, "RpsGui_FoxApplication::~RpsGui_FoxApplication"));
  fxapp_inst = nullptr;
} // end RpsGui_FoxApplication::~RpsGui_FoxApplication

//// end of file foxevloop_rps.cc

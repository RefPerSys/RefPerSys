/****************************************************************
 * file fltkhi_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the high-level FLTK graphical user interface related
 *      code. See http://fltk.org/
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

#include "headfltk_rps.hh"



extern "C" const char rps_fltkhi_gitid[];
const char rps_fltkhi_gitid[]= RPS_GITID;

extern "C" const char rps_fltkhi_date[];
const char rps_fltkhi_date[]= __DATE__;

static std::atomic<bool> rps_running_fltk;

extern "C" pthread_t rps_main_gui_pthread;
pthread_t rps_main_gui_pthread;
Rps_GuiPreferences rps_gui_pref;

extern "C" int rps_fltk_arg_handler(int argc, char**argv, int &i);

std::string
rps_fltk_version(void)
{
  std::string res("FLTK ");
  char fltkgitbuf[24];
  memset (fltkgitbuf, 0, sizeof(fltkgitbuf));
  strncpy(fltkgitbuf, rps_fltkhi_gitid, 3*sizeof(fltkgitbuf)/4);
  res += "git ";
  res += fltkgitbuf;
  res += ", ABI:";
  res += std::to_string(Fl::abi_version());
  res += ", API:";
  res += std::to_string(Fl::api_version());
  return res;
} // end rps_fltk_version




////////////////////////////////////////////////////////////////
class Rps_FltkEventLoop_CallFrame;
struct Rps_Todo_Base
{
  static constexpr bool TODO_is_Function = true;
  static constexpr bool TODO_is_Closure = false;
  const bool todo_is_function;
  const int todo_lineno;
  const double todo_timeout;
  const char* todo_filename;
  const char* todo_label;
  typedef std::function<void(Rps_CallFrame*,void*,void*)> todo_func_t;
  Rps_Todo_Base(bool isfun, int lineno, double delay, const char*label=nullptr)
    : todo_is_function(isfun), todo_lineno(lineno),
      todo_timeout(rps_monotonic_real_time()+delay),
      todo_filename(nullptr),
      todo_label (label) {};
  Rps_Todo_Base(bool isfun, const char*filename, int lineno, double delay, const char*label=nullptr)
    : todo_is_function(isfun), todo_lineno(lineno),
      todo_timeout(rps_monotonic_real_time()+delay),
      todo_filename(filename),
      todo_label (label) {};
  ~Rps_Todo_Base() {};
  bool is_todo_function(void) const
  {
    return todo_is_function;
  };
  bool lineno(void) const
  {
    return todo_lineno;
  };
  const char*label(void) const
  {
    return todo_label;
  };
  const char*filename(void) const
  {
    return todo_filename;
  };
  double timeout(void) const
  {
    return todo_timeout;
  };
};				// end struct Rps_Todo_Base

class Rps_Todo_Function  : public Rps_Todo_Base
{
  friend class FltkEventLoop_CallFrame;
  const todo_func_t todo_func;
  void* todo_arg1;
  void* todo_arg2;
public:
  Rps_Todo_Function(int lineno, double delay, const todo_func_t& todo, void*arg1, void*arg2)
    : Rps_Todo_Base(TODO_is_Function, lineno, delay), todo_func(todo), todo_arg1(arg1), todo_arg2(arg2) {};
  Rps_Todo_Function(int lineno, const char*label, double delay, const todo_func_t& todo, void*arg1, void*arg2)
    : Rps_Todo_Base(TODO_is_Function, lineno, delay, label), todo_func(todo), todo_arg1(arg1), todo_arg2(arg2) {};
  Rps_Todo_Function(const char*filename, int lineno, const char*label, double delay, const todo_func_t& todo, void*arg1, void*arg2)
    : Rps_Todo_Base(TODO_is_Function, filename,  lineno, delay, label), todo_func(todo), todo_arg1(arg1), todo_arg2(arg2) {};
  ~Rps_Todo_Function() {};
  void apply_todo_fun(Rps_CallFrame*callframe, void*p1=nullptr, void*p2=nullptr)
  {
    todo_func(callframe,p1,p2);
  };
  void apply_todo_function(Rps_CallFrame*callframe)
  {
    todo_func(callframe, todo_arg1, todo_arg2);
  }
};				// end class Rps_Todo_Function



class Rps_Todo_Closure   : public Rps_Todo_Base
{
  friend class FltkEventLoop_CallFrame;
  const Rps_ClosureValue todo_closv;
  const Rps_Value todo_arg1v;
  const Rps_Value todo_arg2v;
  const Rps_Value todo_arg3v;
public:
  Rps_Todo_Closure(int lineno, const char*filename, double delay, const char*label, const Rps_ClosureValue closv,
                   const Rps_Value arg1v=nullptr, const Rps_Value arg2v=nullptr, const Rps_Value arg3v=nullptr)
    : Rps_Todo_Base(TODO_is_Closure, filename, lineno, delay, label),
      todo_closv(closv), todo_arg1v(arg1v), todo_arg2v(arg2v), todo_arg3v(arg3v) {};
  const Rps_ClosureValue& get_todo_clos() const
  {
    return todo_closv;
  };
  const Rps_Value& get_todo_arg1() const
  {
    return todo_arg1v;
  };
  const Rps_Value& get_todo_arg2() const
  {
    return todo_arg2v;
  };
  const Rps_Value& get_todo_arg3() const
  {
    return todo_arg3v;
  };
  void apply_todo_clos(Rps_FltkEventLoop_CallFrame*callfram)
  {
  }
}; // end class Rps_Todo_Closure


class Rps_Todo
{
  union
  {
    Rps_Todo_Closure todo_closure;
    Rps_Todo_Function todo_function;
    Rps_Todo_Base todo_base;
  };
public:
  /// see https://cpppatterns.com/patterns/rule-of-five.html, which we do follow for Rps_Todo
  Rps_Todo(Rps_Todo_Closure tc) : todo_closure(tc) {};
  Rps_Todo(Rps_Todo_Function tf): todo_function(tf) {};
  ~Rps_Todo();
  bool is_todo_function() const
  {
    return todo_base.is_todo_function();
  };
  bool is_todo_closure() const
  {
    return !(todo_base.is_todo_function());
  };
  Rps_Todo(Rps_Todo&& todosrc)
  {
    if (todosrc.is_todo_function()) new(&todo_function) Rps_Todo(todosrc.todo_function);
    else new(&todo_closure) Rps_Todo(todosrc.todo_closure);
  };
  Rps_Todo(const Rps_Todo&todosrc)
  {
    if (todosrc.is_todo_function()) new(&todo_function) Rps_Todo(todosrc.todo_function);
    else new(&todo_closure) Rps_Todo(todosrc.todo_closure);
  };
  const Rps_Todo_Closure& as_todo_closure() const
  {
    if (!is_todo_closure())
      throw std::range_error("Rps_Todo::as_todo_closure without a closure");
    return todo_closure;
  };
  const Rps_Todo_Function& as_todo_function() const
  {
    if (!is_todo_function())
      throw std::range_error("Rps_Todo::as_todo_function without a function");
    return todo_function;
  };
  bool lineno(void) const
  {
    return todo_base.lineno();
  };
  const char*label(void) const
  {
    return todo_base.label();
  };
  const char*filename(void) const
  {
    return todo_base.filename();
  };
  double timeout(void) const
  {
    return todo_base.timeout();
  };
}; // end class Rps_Todo

class Rps_FltkEventLoop_CallFrame : public Rps_CallFrame
{
  friend class Rps_Todo;
  friend void
  rps_fltk_add_delayed_labeled_todo_at(Rps_CallFrame*curframe, const char*filename, int lineno, const char*label, double delay,
                                       const std::function<void(Rps_CallFrame*,void*,void*)>& todo,
                                       void*arg1, void*arg2);
  friend void
  rps_fltk_add_delayed_labeled_closure_at(Rps_CallFrame*curframe,const char*filename, int lineno, const char*label, double delay,
                                          Rps_ClosureValue closv,
                                          Rps_Value arg1v, Rps_Value arg2v);
  friend void rps_fltk_stop_event_loop(void);
  friend void rps_fltk_event_loop(Rps_CallFrame*callframe);
  friend class Rps_GarbageCollector;
  /// garbage collected slots
  /// after here, no GC-ed value anymore
  ////////////////
  // Our TODO machinery: the key of below ordered map is an absolute
  // time, as given by rps_monotonic_real_time the value in entries of
  // below map is a Todo. We need to ensure that entries are unique
  // (by adding a tiny random delay when needed). We need a mutex to
  // ensure other threads than the GUI one could add TODO things. We
  // need garbage collection support of these TODO.
  std::map<double,Rps_Todo> evloopfr_todos;
  mutable std::recursive_mutex evloopfr_mtx;
  Rps_FltkEventLoop_CallFrame*evloopfr_oldframe;
  int evloopfr_lineno;
  short evloopfr_depth;
  /// the event loop frames in the GUI main thread are kept
  static std::atomic<Rps_FltkEventLoop_CallFrame*> evloopfr_curframe;
  /// the event loop frames elsewhere are collected in a locked set
  static std::recursive_mutex evloopfr_setmtx;
  static std::set<Rps_FltkEventLoop_CallFrame*> evloopfr_set;
  void fltk_event_wait(unsigned long count, double delay);
  void run_scheduled_fltk_todos();
#warning the TODO machinery of event loop (see rps_fltk_event_loop) need more fields here.
public:
  struct Rps_EventLoop_tag {};
  Rps_Value evloopfr_dummyval[1];
  void gc_mark_todos(Rps_GarbageCollector*);
  Rps_FltkEventLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
                              Rps_ObjectRef descr, int depth, Rps_EventLoop_tag);
  ~Rps_FltkEventLoop_CallFrame();
  static Rps_FltkEventLoop_CallFrame* find_calling_event_call_frame(const Rps_CallFrame*cf);
};				// end Rps_FltkEventLoop_CallFrame

std::atomic<Rps_FltkEventLoop_CallFrame*> Rps_FltkEventLoop_CallFrame::evloopfr_curframe;
std::recursive_mutex Rps_FltkEventLoop_CallFrame::evloopfr_setmtx;
std::set<Rps_FltkEventLoop_CallFrame*> Rps_FltkEventLoop_CallFrame::evloopfr_set;
static constexpr unsigned rps_evloop_nbvals = offsetof(Rps_FltkEventLoop_CallFrame, evloopfr_dummyval);


Rps_Todo::~Rps_Todo()
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_DEBUG_LOG(GUI, "Rps_Todo::~Rps_Todo this@" << this
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_Todo::~Rps_Todo"));
};				// end Rps_Todo::~Rps_Todo


Rps_FltkEventLoop_CallFrame::Rps_FltkEventLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
    Rps_ObjectRef descr, int depth, Rps_EventLoop_tag)
  : Rps_CallFrame(rps_evloop_nbvals, descr, callframe),
    evloopfr_dummyval(),
    evloopfr_oldframe(nullptr),
    evloopfr_lineno(lineno),
    evloopfr_depth(depth)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(evloopfr_depth >= 0);
  evloopfr_oldframe= std::atomic_exchange(&evloopfr_curframe,this);
  RPS_DEBUG_LOG(GUI, "Rps_FltkEventLoop_CallFrame eventloop line#" << lineno
                << " depth#" << evloopfr_depth
                << " callframe@" << (void*)callframe
                << " this@" << (void*)this);
} // end of Rps_FltkEventLoop_CallFrame::Rps_FltkEventLoop_CallFrame

Rps_FltkEventLoop_CallFrame::~Rps_FltkEventLoop_CallFrame()
{
  RPS_DEBUG_LOG(GUI, "~Rps_FltkEventLoop_CallFrame this@" << (void*)this);
  if (rps_is_main_gui_thread())
    {
      evloopfr_curframe.store(evloopfr_oldframe);
    }
  else
    {
      std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
      evloopfr_set.erase(this);
    }
}; // end Rps_FltkEventLoop_CallFrame::~Rps_FltkEventLoop_CallFrame

void
Rps_FltkEventLoop_CallFrame::gc_mark_todos(Rps_GarbageCollector*gc)
{
  RPS_ASSERT(gc != nullptr && gc->is_valid_garbcoll());
  std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
  for (auto todoit: evloopfr_todos)
    {
      Rps_Todo& curtodo = todoit.second;
      if (curtodo.is_todo_function())
        continue;
      else   // curtodo is a todo closure
        {
          auto& curtodoclos = curtodo.as_todo_closure();
          curtodoclos.get_todo_clos().gc_mark(*gc);
          curtodoclos.get_todo_arg1().gc_mark(*gc);
          curtodoclos.get_todo_arg2().gc_mark(*gc);
          curtodoclos.get_todo_arg3().gc_mark(*gc);
        }
    }
}; // end Rps_FltkEventLoop_CallFrame::gc_mark_todos


void
Rps_FltkEventLoop_CallFrame::run_scheduled_fltk_todos(void)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_DEBUG_LOG(GUI, "start Rps_FltkEventLoop_CallFrame::run_scheduled_fltk_todos depth#" << evloopfr_depth);

  std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
  std::vector<Rps_Todo> todovect;
  for (auto todoit: evloopfr_todos)
    {
      double timeout = todoit.first;
      if (timeout >= rps_monotonic_real_time())
        break;
      Rps_Todo& curtodo = todoit.second;
      todovect.push_back(curtodo);
    };
  RPS_DEBUG_LOG(GUI,
                "Rps_FltkEventLoop_CallFrame::run_scheduled_fltk_todos depth#"
                << evloopfr_depth << " should do " << todovect.size() << " todos");
  for (auto curtodo : todovect)
    {
      auto curit = evloopfr_todos.find(curtodo.timeout());
      RPS_ASSERT(curit != evloopfr_todos.end());
      evloopfr_todos.erase(curit);
      if (curtodo.is_todo_function())
        {
          Rps_Todo_Function& curtodofun = const_cast<Rps_Todo_Function&>(curtodo.as_todo_function());
          int todolineno = curtodo.lineno();
          const char*todofilename = curtodo.filename();
          RPS_DEBUG_LOG(GUI,
                        "run_scheduled_fltk_todos depth#"
                        << evloopfr_depth
                        << " before applying function from "
                        << todofilename << ":" << todolineno);
          curtodofun.apply_todo_function(this);
        }
      else   // curtodo is closure
        {
          auto& curtodoclos = curtodo.as_todo_closure();
          int todolineno = curtodo.lineno();
          const char*todofilename = curtodo.filename();
        }
    }
  RPS_DEBUG_LOG(GUI, "end Rps_FltkEventLoop_CallFrame::run_scheduled_fltk_todos depth#" << evloopfr_depth << std::endl);
}; // end Rps_FltkEventLoop_CallFrame::run_scheduled_fltk_todos


void
Rps_FltkEventLoop_CallFrame::fltk_event_wait(unsigned long count, double delay)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  Fl::flush();
  RPS_DEBUGNL_LOG(GUI, "in Rps_FltkEventLoop_CallFrame::fltk_event_wait depth#" << evloopfr_depth << ", count#" << count
                  << ", delay=" << delay);
  if (count % 16 == 0)
    RPS_DEBUG_LOG(GUI, "Rps_FltkEventLoop_CallFrame::fltk_event_wait depth#" << evloopfr_depth << ", count#" << count
                  << ", delay=" << delay << std::endl
                  <<  RPS_FULL_BACKTRACE_HERE(1, "fltk_event_wait"));
  auto delw = Fl::wait(delay);
  if (delw < 0)
    {
      RPS_WARNOUT("Rps_FltkEventLoop_CallFrame::fltk_event_wait: depth#" << evloopfr_depth << " broke delw=" << delw << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "fltk_event_wait"));
      return;
    }
  else
    RPS_DEBUG_LOG(GUI, "Rps_FltkEventLoop_CallFrame::fltk_event_wait depth#" << evloopfr_depth << ", count#" << count
                  << " after wait delw=" << delw);
  run_scheduled_fltk_todos();
  RPS_DEBUG_LOG(GUI, "end Rps_FltkEventLoop_CallFrame::fltk_event_wait depth#" << evloopfr_depth << std::endl);
}; // end Rps_FltkEventLoop_CallFrame::fltk_event_wait



void
rps_fltk_add_delayed_labeled_todo_at(Rps_CallFrame*curframe, const char*filename, int lineno, const char*label, double delay,
                                     const std::function<void(Rps_CallFrame*,void*,void*)>& todo,
                                     void*arg1, void*arg2)
{
  /// missing macros in headfltk_rps.hh to invoke this rps_fltk_add_delayed_labeled_todo_at
  RPS_ASSERT(curframe != nullptr);
  RPS_ASSERT(Rps_CallFrame::is_good_call_frame(curframe));
  RPS_ASSERT(delay >= 0);
  /** TODO: we should find the call frame below the given callframe
    which is an Rps_FltkEventLoop_CallFrame then invoke
    fltk_add_delayed_todo on it. We also need to handle the more
    complex case when rps_fltk_add_delayed_todo is called from a non
    GUI thread. */
  auto newtodo = Rps_Todo(Rps_Todo_Function(filename, lineno, label, delay, todo, arg1, arg2));
  if (rps_is_main_gui_thread())
    {
      Rps_FltkEventLoop_CallFrame*eventcallframe = Rps_FltkEventLoop_CallFrame::find_calling_event_call_frame(curframe);
      if (!eventcallframe)
        RPS_FATALOUT("no event call frame in rps_fltk_add_delayed_todo for callframe@" << curframe);
      std::lock_guard<std::recursive_mutex> gu(eventcallframe->evloopfr_mtx);
      Fl::flush();
      double todotime = rps_monotonic_real_time() + delay;
      int loopcnt = 0;
      for (;;)
        {
          RPS_ASSERT(loopcnt < 32); // it is very unlikely that we loop more than 32 times
          if (eventcallframe->evloopfr_todos.find(todotime) == eventcallframe->evloopfr_todos.end())
            {
              eventcallframe->evloopfr_todos.insert({todotime, newtodo});
              break;
            }
          else
            todotime = rps_monotonic_real_time() + delay + Rps_Random::random_quickly_16bits()*1.0e-6;
        }
    }
  else
    {
#warning unimplemented rps_fltk_add_delayed_todo for non GUI thread
      RPS_FATALOUT("unimplemented rps_fltk_add_delayed_labeled_todo_at nonGUI curframe@" << curframe << " delay=" << delay
                   << " arg1@" << arg1 << " arg2@" << arg2);
    }
#warning unimplemented rps_fltk_add_delayed_labeled_todo_at
  RPS_ASSERT(todo);
  RPS_FATALOUT("unimplemented rps_fltk_add_delayed_labeled_todo_at curframe=" << curframe
               << " filename=" << filename << ", lineno=" << lineno
               << " label=" << label << " delay=" << delay << " arg1=" << arg1 << " arg2=" << arg2);
} // end rps_fltk_add_delayed_labeled_todo_at



void
rps_fltk_add_delayed_labeled_closure_at(Rps_CallFrame*curframe,const char*filename, int lineno, const char*label, double delay,
                                        Rps_ClosureValue closv,
                                        Rps_Value arg1v, Rps_Value arg2v)
{
  RPS_ASSERT(Rps_CallFrame::is_good_call_frame(curframe));
  /** TODO: we should find the call frame below the given callframe
      which is an Rps_FltkEventLoop_CallFrame then insert the todo in
      it. We also need to handle the more complex case when
      rps_fltk_add_delayed_todo is called from a non GUI thread. */
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_closure_at curframe@" << curframe
                << " filename=" << filename << " lineno=" << lineno
                << " label=" << label << " delay=" << delay
                << " closv=" << closv << " arg1v=" << arg1v << " arg2v=" << arg2v);
  auto newtodo = Rps_Todo(Rps_Todo_Closure(lineno, filename, delay, label, closv, arg1v, arg2v));
  if (rps_is_main_gui_thread())
    {
      Rps_FltkEventLoop_CallFrame*eventcallframe = Rps_FltkEventLoop_CallFrame::find_calling_event_call_frame(curframe);
      if (!eventcallframe)
        RPS_FATALOUT("no event call frame in rps_fltk_add_delayed_labeled_closure_at for callframe@" << curframe << " filename=" << filename
                     << " lineno=" << lineno
                     << " label= " << label
                     << " delay=" << delay
                     <<" closv=" << closv
                     << " arg1v=" << arg1v << " arg2v=" << arg2v);
      std::lock_guard<std::recursive_mutex> gu(eventcallframe->evloopfr_mtx);
      Fl::flush();
      double todotime = rps_monotonic_real_time() + delay;
      int loopcnt = 0;
      for (;;)
        {
          RPS_ASSERT(loopcnt < 32); // it is very unlikely that we loop more than 32 times
          if (eventcallframe->evloopfr_todos.find(todotime) == eventcallframe->evloopfr_todos.end())
            {
              eventcallframe->evloopfr_todos.insert({todotime, newtodo});
              break;
            }
          else
            todotime = rps_monotonic_real_time() + delay + Rps_Random::random_quickly_16bits()*1.0e-6;
        }
    }
  else
    {
#warning unimplemented rps_fltk_add_delayed_labeled_closure_at
      RPS_FATALOUT("unimplemented rps_fltk_add_delayed_labeled_closure_at curframe=" << curframe
                   << " filename=" << filename << ", lineno=" << lineno << " label=" << label
                   << " delay=" << delay << " closv=" << closv << " arg1v=" << arg1v << " arg2v=" << arg2v);
    }
} // end rps_fltk_add_delayed_labeled_closure_at

void
rps_fltk_stop_event_loop(void)
{
// see  http://man7.org/linux/man-pages/man3/pthread_setname_np.3.html
  char curthname[24];
  memset(curthname, 0, sizeof(curthname));
  pthread_getname_np(pthread_self(), curthname, sizeof(curthname));
  RPS_DEBUG_LOG(GUI, "rps_fltk_stop_event_loop in " << curthname
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_stop_event_loop"));
  rps_running_fltk.store(false);
} // end rps_fltk_stop_event_loop


/// the object whose name is "fltk_event_loop" is a RefPerSys frame
/// descriptor for FTK event loops GC call frames.
#define RPS_FLTK_EVENT_LOOP_DESCR RPS_ROOT_OB(_39OsVkAJDdV00ohD5r)

void
rps_fltk_event_loop(Rps_CallFrame*callframe)
{
  static volatile unsigned depth;
  RPS_ASSERT(Rps_CallFrame::is_good_call_frame(callframe));
  RPS_ASSERT(rps_is_main_gui_thread());
  depth++;
  Rps_FltkEventLoop_CallFrame _(callframe, __LINE__,
                                RPS_FLTK_EVENT_LOOP_DESCR /*fltk_event_loop*/,
                                depth, Rps_FltkEventLoop_CallFrame::Rps_EventLoop_tag{});
#warning rps_fltk_event_loop should use Rps_FltkEventLoop_CallFrame
  unsigned long count=0;
  RPS_DEBUG_LOG(GUI, "start of rps_fltk_event_loop depth#" << depth
                << " callframe@" << callframe << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_event_loop start"));
  while (rps_running_fltk.load())
    {
      count++;
      double delay = RPS_DEBUG_ENABLED(GUI)?30.0:3.0;
      _.fltk_event_wait(count, delay);
    };
  RPS_DEBUG_LOG(GUI, "end of rps_fltk_event_loop depth#" << depth
                << " callframe@" << callframe << std::endl);
  depth--;
} // end rps_fltk_event_loop


Rps_FltkEventLoop_CallFrame*
Rps_FltkEventLoop_CallFrame::find_calling_event_call_frame(const Rps_CallFrame*callframe)
{
  Rps_FltkEventLoop_CallFrame*evcallframe = nullptr;
  for (const Rps_CallFrame*curcallframe = callframe;
       curcallframe != nullptr && Rps_CallFrame::is_good_call_frame(curcallframe);
       curcallframe = curcallframe->previous_call_frame())
    {
      if (curcallframe->call_frame_descriptor() == RPS_FLTK_EVENT_LOOP_DESCR)
        return (Rps_FltkEventLoop_CallFrame*)(curcallframe);
    }
  return nullptr;
} // end Rps_FltkEventLoop_CallFrame::find_calling_event_call_frame


void
rps_garbcoll_application(Rps_GarbageCollector&gc)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(gc.is_valid_garbcoll());
#warning incomplete rps_garbcoll_application
  RPS_WARNOUT("incomplete rps_garbcoll_application " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_garbcoll_application"));
} // end rps_garbcoll_application

bool
rps_is_main_gui_thread(void)
{
  RPS_DEBUG_LOG(GUI, "pthread_self() = " << pthread_self());
  RPS_DEBUG_LOG(GUI, "rps_main_gui_pthread = " << rps_main_gui_pthread);
  return pthread_self() == rps_main_gui_pthread;
} // end rps_is_main_gui_thread


/// callback for Fl::args; see
/// https://www.fltk.org/doc-1.4/classFl.html#a115903daf3593748cdd36a5e78e74534
/**
 * It is called with the same argc and argv, and with i set to the
 * index of the switch to be processed. The cb handler should return
 * zero if the switch is unrecognized, and not change i. It should
 * return non-zero to indicate the number of words processed if the
 * switch is recognized, i.e. 1 for just the switch, and more than 1
 * for the switch plus associated parameters. i should be incremented
 * by the same amount.
 **/
int
rps_fltk_arg_handler(int argc, char**argv, int &i)
{
  const char* curarg = nullptr;
  if (i>0 && i<argc)
    curarg=argv[i];
  RPS_DEBUG_LOG(GUI, "rps_fltk_arg_handler i#" << i
                << " argc=" << argc << ", "
                << (curarg?"curarg:":"*no current argument*")
                << (curarg?:" !")
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_arg_handler("));
  if (curarg)
    for (struct argp_option *aopt = rps_progoptions; aopt->name != nullptr; aopt++)
      {
        const char*restarg = nullptr;
        bool goteq = false;
        int deltarg = 0;
        int aoptnlen = (aopt->name)?strlen(aopt->name):0;
        if (curarg[0]=='-' && curarg[1]=='-' && strncmp(curarg+2, aopt->name, aoptnlen))
          {
            if (curarg[2+aoptnlen]=='=')
              {
                restarg = curarg+aoptnlen+3;
                goteq = true;
                deltarg = 1;
              }
          }
        else if (curarg[0]=='-' && isalpha(curarg[1]) && aopt->key==curarg[1])
          {
            restarg = curarg+2;
            if (*restarg && aopt->arg) deltarg=2;
            else if (!*restarg && !aopt->arg) deltarg=1;
          }
        if (!restarg)
          continue;
        error_t erropt = ARGP_ERR_UNKNOWN;
        struct argp_state *emptystate = reinterpret_cast<struct argp_state*>((void*)RPS_EMPTYSLOT);
        if (aopt->arg)
          {
            erropt = rps_parse1opt(aopt->key, (char*)restarg, emptystate);
          }
        else
          {
            erropt = rps_parse1opt(aopt->key, nullptr, emptystate);
          };
        if (erropt)
          RPS_WARNOUT("rps_fltk_arg_handler fail to parse " << curarg << " at index#" << i);
        i += deltarg;
        return i;
      };
  return 0;
} // end rps_fltk_arg_handler



////////////////////////////////////////////////////////////////
void
rps_fltk_initialize(int &argc, char**argv, Rps_CallFrame*callerframe)
{
  RPS_ASSERT(callerframe && callerframe->stored_type() == Rps_Type::CallFrame);
  rps_main_gui_pthread = pthread_self();
  RPS_DEBUG_LOG(GUI, "start rps_fltk_initialize callerframe@" << (void*)callerframe << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize"));
  // see https://www.fltk.org/doc-1.4/classFl.html#a1576b8c9ca3e900daaa5c36ca0e7ae48
  // and https://www.fltk.org/doc-1.4/classFl.html#a115903daf3593748cdd36a5e78e74534
  int argix= -1;
  if (Fl::args(argc, argv, argix, rps_fltk_arg_handler) == 0)
    {
      if (argix>0 && argix<argc)
        RPS_FATALOUT("rps_fltk_initialize failed to parse program arguments, argix=" << argix
                     << " bad program argument:" << argv[argix]);
      else
        RPS_FATALOUT("rps_fltk_initialize confused when parsing program arguments, argix#" << argix << ", argc=" << argc);
    }
  std::string titlestr;
  if (rps_gui_pref.gui_title.empty())
    {
      char titbuf[96];
      memset (titbuf, 0, sizeof(titbuf));
      snprintf(titbuf, sizeof(titbuf), "RefPerSys/%.16s p%ld@%.40s",
               rps_gitid,(long)getpid(),rps_hostname());
      titlestr = titbuf;
    }
  else
    titlestr = rps_gui_pref.gui_title;
  constexpr int default_gui_width = 640;
  constexpr int default_gui_height = 480;
  constexpr int min_gui_width = 64;
  constexpr int min_gui_height = 48;
  constexpr int max_gui_width = 4096;
  constexpr int max_gui_height = 2048;
  int w=default_gui_width, h=default_gui_width;
  {
    if (!rps_gui_pref.gui_geometry.empty())
      sscanf(rps_gui_pref.gui_geometry.c_str(), "%dx%d", &w, &h);
  }
  if (w < min_gui_width)
    w= min_gui_width;
  if (w > max_gui_height)
    w=max_gui_height;
  if (h < min_gui_height)
    h=min_gui_height;
  if (h > max_gui_height)
    h=max_gui_height;
  double scale=1.0;
  constexpr double gui_min_scale = 0.2;
  constexpr double gui_max_scale = 4.0;
  if (rps_gui_pref.gui_scale>gui_min_scale && rps_gui_pref.gui_scale<gui_max_scale)
    scale = rps_gui_pref.gui_scale;
  RPS_DEBUG_LOG(GUI, "rps_fltk_initialize w=" << w << ", h=" << h
                << ", scale=" << scale);
  if (scale != 1.0)
    {
      int nbscreens = Fl::screen_count();
      RPS_DEBUG_LOG(GUI, "rps_fltk_initialize nbscreens=" << nbscreens
                    << ", scale=" << scale);
      for (int scrix=0; scrix<nbscreens; scrix++)
        Fl::screen_scale(scrix, scale);
    }
  auto cmdwin = new RpsGui_CommandWindow(w, h, titlestr);
  RPS_DEBUG_LOG(GUI, "rps_fltk_initialize,  create a window: argc="
                << argc << " argv@" << argv << " cmdwin=" << cmdwin
                << " w=" << w << " h=" << h
                << " title:'" << titlestr << "'");
  /// delay initialization of menubar
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_0(callerframe, "rps_fltk_initialize-initialize_menubar", 0.1, [=](Rps_CallFrame*cf,void*,void*)
  {
    RPS_DEBUG_LOG(GUI, "rps_fltk_initialize todo initialize_menubar cmdwin=" << cmdwin << std::endl
                  <<  RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize  todo initialize_menubar"));
    cmdwin->begin();
    cmdwin->initialize_menubar();
    cmdwin->end();
    cmdwin->show();
    RPS_DEBUG_LOG(GUI, "rps_fltk_initialize todo end initialize_menubar cmdwin=" << cmdwin);
  });
  /// delay initialization of pack
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_0(callerframe,  "rps_fltk_initialize-initialize_pack", 0.2, [=](Rps_CallFrame*cf,void*,void*)
  {
    RPS_DEBUG_LOG(GUI, "rps_fltk_initialize todo initialize_pack cmdwin=" << cmdwin << std::endl
                  <<  RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize  todo initialize_pack"));
    cmdwin->begin();
    cmdwin->initialize_pack();
    cmdwin->end();
    cmdwin->show();
    RPS_DEBUG_LOG(GUI, "rps_fltk_initialize todo end initialize_pack cmdwin=" << cmdwin);
  });
  cmdwin->show();
  RPS_DEBUG_LOG(GUI, "rps_fltk_initialize ending cmdwin=" << RpsGui_ShowFullWidget<RpsGui_CommandWindow>(cmdwin)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize~end")
                << std::endl << std::endl);
} // end rps_fltk_initialize


void
rps_run_fltk_gui(int &argc, char**argv)
{
  rps_main_gui_pthread = pthread_self();
  Rps_FltkEventLoop_CallFrame _(nullptr, __LINE__,
                                RPS_FLTK_EVENT_LOOP_DESCR, 1,
                                Rps_FltkEventLoop_CallFrame::Rps_EventLoop_tag{});
  RPS_DEBUG_LOG(GUI, "start rps_run_fltk_gui _@" << ((void*)&_)  << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_fltk_gui"));
  //
  for (int ix=0; ix<argc; ix++)
    RPS_DEBUG_LOG(GUI, "FLTK GUI arg [" << ix << "]: " << argv[ix]);
  rps_running_fltk.store(true);
  rps_fltk_initialize(argc, argv, &_);
  RPS_DEBUG_LOG(GUI, "rps_run_fltk_gui before event loop _@" << ((void*)&_)  << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_fltk_gui pre-event"));
  rps_fltk_event_loop(&_);
  RPS_DEBUG_LOG(GUI, "end rps_run_fltk_gui _@" << ((void*)&_)  << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "end rps_run_fltk_gui") << std::endl);
} // rps_run_fltk_gui


////////////////////////////////////////////////////////////////
// C++ closure for _0TwK4TkhEGZ03oTa5m
//!display Val0 in Ob1Win at depth Val2Depth
extern "C" rps_applyingfun_t rpsapply_0TwK4TkhEGZ03oTa5m;
Rps_TwoValues
rpsapply_0TwK4TkhEGZ03oTa5m(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0val,
                            const Rps_Value arg1obwin, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0TwK4TkhEGZ03oTa5m,
                 callerframe, //
                 Rps_Value val0v;
                 Rps_ObjectRef winob1;
                 Rps_Value depth2v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  _.val0v = arg0val;
  _.winob1 = arg1obwin.to_object();
  _.depth2v = arg2depth;
  int depth = _.depth2v.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_0TwK4TkhEGZ03oTa5m start val0v=" << _.val0v
                << ", winob=" << _.winob1
                << ", depth=" << depth);
  ////==== body of _0TwK4TkhEGZ03oTa5m ====
#warning unimplemented rpsapply_0TwK4TkhEGZ03oTa5m
  RPS_FATAL("unimplemented rpsapply_0TwK4TkhEGZ03oTa5m");
} // end of rpsapply_0TwK4TkhEGZ03oTa5m !display Val0 in Ob1Win at depth Val2Depth


////////////////////////////////////////////////////////////////
// C++ closure for _8KJHUldX8GJ03G5OWp
//!method int/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_8KJHUldX8GJ03G5OWp;
Rps_TwoValues
rpsapply_8KJHUldX8GJ03G5OWp(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0recv, ///
                            const Rps_Value arg1obwin, ///
                            const Rps_Value arg2depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_8KJHUldX8GJ03G5OWp,
                 callerframe, //
                 Rps_Value intv;
                 Rps_ObjectRef obwin;
                 Rps_Value depthv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _8KJHUldX8GJ03G5OWp ====
  _.intv = arg0recv;
  _.obwin = arg1obwin.as_object();
  _.depthv = arg2depth;
  RPS_DEBUG_LOG(GUI, "rpsapply_8KJHUldX8GJ03G5OWp start intv=" << _.intv
                << ", obwin=" << _.obwin
                << ", depth=" << _.depthv);
#warning unimplemented rpsapply_8KJHUldX8GJ03G5OWp
  RPS_FATAL("unimplemented rpsapply_8KJHUldX8GJ03G5OWp");
} // end of  rpsapply_8KJHUldX8GJ03G5OWp !method int/display_value_fltk


////////////////////////////////////////////////////////////////

// C++ closure for _2KnFhlj8xW800kpgPt
//!method string/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_2KnFhlj8xW800kpgPt;
Rps_TwoValues
rpsapply_2KnFhlj8xW800kpgPt(Rps_CallFrame*callerframe,
                            const Rps_Value arg0_receiver,
                            const Rps_Value arg1_object_window,
                            const Rps_Value arg2_recursive_depth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_2KnFhlj8xW800kpgPt,
                 callerframe, //
                 Rps_Value string_value;
                 Rps_ObjectRef object_window;
                 Rps_Value recursive_depth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  _.string_value = arg0_receiver;
  RPS_ASSERT(_.string_value.is_string());
  _.object_window = arg1_object_window.as_object();
  RPS_ASSERT(_.object_window);
  _.recursive_depth = arg2_recursive_depth;
  RPS_ASSERT(_.recursive_depth.is_int());
  RPS_DEBUG_LOG(GUI, "rpsapply_2KnFhlj8xW800kpgPt start string_value=" << _.string_value
                << ", object_window, =" << _.object_window
                << ", recursive_depth=" <<  _.recursive_depth);
  ////==== body of _2KnFhlj8xW800kpgPt ====
#warning unimplemented rpsapply_2KnFhlj8xW800kpgPt
  RPS_FATAL("unimplemented rpsapply_2KnFhlj8xW800kpgPt");
} // end of  rpsapply_2KnFhlj8xW800kpgPt !method string/display_value_fltk

////////////////////////////////////////////////////////////////
// C++ closure for _7oa7eIzzcxv03TmmZH
//!method double/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_7oa7eIzzcxv03TmmZH;
Rps_TwoValues
rpsapply_7oa7eIzzcxv03TmmZH(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_7oa7eIzzcxv03TmmZH,
                 callerframe, //
                 Rps_Value doubleval;
                 Rps_ObjectRef object_window;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  ////==== body of _7oa7eIzzcxv03TmmZH !method double/display_value_fltk ====
  _.doubleval = arg0_recv;
  RPS_ASSERT (_.doubleval.is_double());
  _.object_window = arg1_objwnd.as_object();
  RPS_ASSERT(_.object_window);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT(_.recdepth.is_int());
  RPS_DEBUG_LOG(GUI, "rpsapply_7oa7eIzzcxv03TmmZH start doubleval=" << _.doubleval
                << "object_window, =" << _.object_window
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_7oa7eIzzcxv03TmmZH
  RPS_FATAL("unimplemented rpsapply_7oa7eIzzcxv03TmmZH");
}
// end of rpsapply_7oa7eIzzcxv03TmmZH !method double/display_value_fltk




// C++ closure for _33DFyPOJxbF015ZYoi
//!method tuple/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_33DFyPOJxbF015ZYoi;
Rps_TwoValues
rpsapply_33DFyPOJxbF015ZYoi(Rps_CallFrame*callerframe, //
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_33DFyPOJxbF015ZYoi,
                 callerframe, //
                 Rps_Value tupleval;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value arg3v;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _33DFyPOJxbF015ZYoi !method tuple/display_value_fltk ====
  _.tupleval = arg0_recv;
  RPS_ASSERT (_.tupleval.is_tuple());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_33DFyPOJxbF015ZYoi start tupleval=" << _.tupleval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_33DFyPOJxbF015ZYoi
  RPS_FATAL("unimplemented rpsapply_33DFyPOJxbF015ZYoi");
} // end of rpsapply_33DFyPOJxbF015ZYoi !method tuple/display_value_fltk


// C++ closure for _1568ZHTl0Pa00461I2
//!method set/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_1568ZHTl0Pa00461I2;
Rps_TwoValues
rpsapply_1568ZHTl0Pa00461I2(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_recv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_1568ZHTl0Pa00461I2,
                 callerframe, //
                 Rps_Value setval;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );

  ////==== body of _1568ZHTl0Pa00461I2 !method set/display_value_fltk ====
  _.setval = arg0_recv;
  RPS_ASSERT (_.setval.is_set());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_fltk start setval=" << _.setval
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_1568ZHTl0Pa00461I2
  RPS_FATAL("unimplemented rpsapply_1568ZHTl0Pa00461I2");
} // end of rpsapply_1568ZHTl0Pa00461I2 !method set/display_value_fltk


// C++ closure for _18DO93843oX02UWzq6
//!method object/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_18DO93843oX02UWzq6;
Rps_TwoValues
rpsapply_18DO93843oX02UWzq6(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_objrecv,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_18DO93843oX02UWzq6,
                 callerframe, //
                 Rps_ObjectRef obrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  RPS_DEBUG_LOG(GUI, "rpsapply_18DO93843oX02UWzq6 start arg0_objrecv="
                << arg0_objrecv << ", arg1_objwnd=" << arg1_objwnd
                << ", arg2_recdepth=" << arg2_recdepth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_18DO93843oX02UWzq6") << std::endl);
#warning unimplemented rpsapply_18DO93843oX02UWzq6
  RPS_FATAL("unimplemented rpsapply_18DO93843oX02UWzq6");

} // end of rpsapply_18DO93843oX02UWzq6 !method object/display_value_fltk


// C++ closure for _0rgijx7CCnq041IZEd
//!method immutable_instance/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_0rgijx7CCnq041IZEd;
Rps_TwoValues
rpsapply_0rgijx7CCnq041IZEd (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_inst, ///
                             const Rps_Value arg1_objwnd, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_0rgijx7CCnq041IZEd,
                 callerframe, //
                 Rps_InstanceValue instrecv;
                 Rps_ObjectRef objwnd;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obattr;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_fltk====
  _.instrecv = Rps_InstanceValue(arg0_inst.as_instance());
  RPS_ASSERT (_.instrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_0rgijx7CCnq041IZEd start instrecv=" << _.instrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_0rgijx7CCnq041IZEd
  RPS_FATAL("unimplemented rpsapply_0rgijx7CCnq041IZEd");
} // end of rpsapply_0rgijx7CCnq041IZEd !method immutable_instance/display_value_fltk




// C++ closure for _6Wi00FwXYID00gl9Ma
//!method closure/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_6Wi00FwXYID00gl9Ma;
Rps_TwoValues
rpsapply_6Wi00FwXYID00gl9Ma (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0_clos, ///
                             const Rps_Value arg1_objwnd, ///
                             const Rps_Value arg2_recdepth, ///
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_6Wi00FwXYID00gl9Ma,
                 callerframe, //
                 Rps_ClosureValue closrecv;
                 Rps_ObjectRef objwnd;
                 Rps_ObjectRef obconn;
                 Rps_ObjectRef obmeta;
                 Rps_Value recdepth;
                 Rps_Value compv;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _6Wi00FwXYID00gl9Ma !method closure/display_value_fltk ====
  _.closrecv = Rps_ClosureValue(arg0_clos.as_closure());
  RPS_ASSERT (_.closrecv);
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_6Wi00FwXYID00gl9Ma start closrecv=" << _.closrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
  _.obconn = _.closrecv->conn();
  unsigned width = _.closrecv->cnt();
#warning unimplemented rpsapply_6Wi00FwXYID00gl9Ma
  RPS_FATAL("unimplemented rpsapply_6Wi00FwXYID00gl9Ma");
} // end of rpsapply_6Wi00FwXYID00gl9Ma !method closure/display_value_fltk



// C++ closure for _42cCN1FRQSS03bzbTz
//!method json/display_value_fltk
extern "C" rps_applyingfun_t rpsapply_42cCN1FRQSS03bzbTz;
Rps_TwoValues
rpsapply_42cCN1FRQSS03bzbTz(Rps_CallFrame*callerframe, ///
                            const Rps_Value arg0_json,
                            const Rps_Value arg1_objwnd, ///
                            const Rps_Value arg2_recdepth,
                            [[maybe_unused]] const Rps_Value arg3_, ///
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs_)
{
  RPS_LOCALFRAME(rpskob_42cCN1FRQSS03bzbTz,
                 callerframe, //
                 Rps_Value jsrecv;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                );
  ////==== body of _42cCN1FRQSS03bzbTz !method json/display_value_fltk ====
  ;
  _.jsrecv = arg0_json;
  RPS_ASSERT (_.jsrecv.is_json());
  _.objwnd = arg1_objwnd.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2_recdepth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_42cCN1FRQSS03bzbTz start jsrecv=" << _.jsrecv
                << "objwnd =" << _.objwnd
                << ", recdepth=" <<  _.recdepth);
#warning unimplemented rpsapply_42cCN1FRQSS03bzbTz
  RPS_FATAL("unimplemented rpsapply_42cCN1FRQSS03bzbTz");
} // end of rpsapply_42cCN1FRQSS03bzbTz !method json/display_value_fltk




////////////////////////////////////////////////////////////////
// C++ closure for _4x9jd2yAe8A02SqKAx
//!method object/display_object_occurrence_fltk
extern "C" rps_applyingfun_t rpsapply_4x9jd2yAe8A02SqKAx;
Rps_TwoValues
rpsapply_4x9jd2yAe8A02SqKAx (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, //
                             [[maybe_unused]] const Rps_Value arg3_, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  RPS_LOCALFRAME(rpskob_4x9jd2yAe8A02SqKAx,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _4x9jd2yAe8A02SqKAx  !method object/display_object_occurrence_fltk ====
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx start arg0obj=" << arg0obj
                << ", arg1obwin=" << arg1obwin
                << ", arg2depth=" << arg2depth << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(2, "!method object/display_object_occurrence_fltk"));
  _.recvob = arg0obj.as_object();
  RPS_ASSERT(_.recvob);
  _.objwnd = arg1obwin.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2depth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_4x9jd2yAe8A02SqKAx recvob=" << _.recvob
                << " of class:" <<  _.recvob->compute_class(&_) << std::endl
                << "... objwnd=" << _.objwnd
                << " of class:" <<  _.objwnd->compute_class(&_)
                << "... depthi=" << depthi <<std::endl
                << "!method object/display_object_occurrence_fltk" << std::endl
                << RPS_DEBUG_BACKTRACE_HERE(1, "rpsapply_4x9jd2yAe8A02SqKAx")
                <<std::endl);
#warning unimplemented rpsapply_4x9jd2yAe8A02SqKAx
  RPS_FATAL("unimplemented rpsapply_4x9jd2yAe8A02SqKAx");
} // end of rpsapply_4x9jd2yAe8A02SqKAx !method object/display_object_occurrence_fltk


////////////////////////////////////////////////////////////////
// C++ closure for _5nSiRIxoYQp00MSnYA
//!method object!display_object_content_fltk
extern "C" rps_applyingfun_t rpsapply_5nSiRIxoYQp00MSnYA;
Rps_TwoValues
rpsapply_5nSiRIxoYQp00MSnYA (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0obj, //
                             const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, //
                             const Rps_Value arg3optqtposition, ///
                             [[maybe_unused]] const std::vector<Rps_Value>* restargs_ )
{
  /* In the usual case, this RefPerSys method is called with 3
     arguments.  But in special cases, the 4th argument is a position
     in the document of the text cursor .... */
  RPS_LOCALFRAME(rpskob_5nSiRIxoYQp00MSnYA,
                 callerframe, //
                 Rps_ObjectRef recvob;
                 Rps_ObjectRef classob;
                 Rps_ObjectRef objwnd;
                 Rps_Value recdepth;
                 Rps_Value optqtposition;
                 Rps_ObjectRef spacob;
                 Rps_Value setattrs;
                 Rps_ObjectRef attrob;
                 Rps_Value attrval;
                 Rps_Value curcomp;
                 Rps_Value resmainv;
                 Rps_Value resxtrav;
                 //....etc....
                );
  ////==== body of _5nSiRIxoYQp00MSnYA !method object!display_object_content_fltk ====
  _.recvob = arg0obj.as_object();
  RPS_ASSERT(_.recvob);
  _.objwnd = arg1obwin.as_object();
  RPS_ASSERT (_.objwnd);
  _.recdepth = arg2depth;
  RPS_ASSERT (_.recdepth.is_int());
  auto depthi = _.recdepth.to_int();
  _.optqtposition = arg3optqtposition;
  RPS_ASSERT (!_.optqtposition || _.optqtposition.is_int());
  std::lock_guard<std::recursive_mutex> objwndmtx(*(_.objwnd->objmtxptr()));
  RPS_DEBUG_LOG(GUI, "rpsapply_5nSiRIxoYQp00MSnYA start object!display_object_content_fltk recvob=" << _.recvob
                << ", objwnd =" << _.objwnd
                << " of class:" <<  _.objwnd->compute_class(&_) << std::endl
                << "... depthi=" <<  depthi
                << std::endl << "+++ object!display_object_content_fltk +++");
#warning unimplemented rpsapply_5nSiRIxoYQp00MSnYA
  RPS_FATAL("unimplemented rpsapply_5nSiRIxoYQp00MSnYA");
} // end of rpsapply_5nSiRIxoYQp00MSnYA !method object!display_object_content_fltk



////////////////////////////////////////////////////////////////
// C++ closure for _8lKdW7lgcHV00WUOiT
//!method class/display_object_payload_fltk
extern "C" rps_applyingfun_t rpsapply_8lKdW7lgcHV00WUOiT;
Rps_TwoValues
rpsapply_8lKdW7lgcHV00WUOiT (Rps_CallFrame*callerframe, ///
                             const Rps_Value arg0class, const Rps_Value arg1obwin, ///
                             const Rps_Value arg2depth, const Rps_Value _arg3 __attribute__((unused)), ///
                             const std::vector<Rps_Value>* restargs_ __attribute__((unused)))
{
  RPS_LOCALFRAME(rpskob_8lKdW7lgcHV00WUOiT,
                 callerframe, //
                 Rps_ObjectRef obclass; //
                 Rps_ObjectRef obwin; //
                 Rps_ObjectRef obsuper; //
                 Rps_Value depthv; //
                 Rps_Value resmainv; //
                 Rps_Value resxtrav; //
                 Rps_SetValue setselv; //
                 Rps_ObjectRef obcursel; //
                 Rps_ClosureValue curmethclos;
                );
  ////==== body of _8lKdW7lgcHV00WUOiT ====
  _.obclass = arg0class.as_object();
  RPS_ASSERT(_.obclass);
  _.obwin = arg1obwin.as_object();
  RPS_ASSERT(_.obwin);
  _.depthv = arg2depth;
  RPS_ASSERT(_.depthv.is_int());
  auto depthi = _.depthv.to_int();
  RPS_DEBUG_LOG(GUI, "rpsapply_8lKdW7lgcHV00WUOiT start !method class/display_object_payload_fltk @!@° obclass="
                << _.obclass << ", obwin=" << _.obwin
                << " of class:" << Rps_Value(_.obwin).compute_class(&_)
                << ", depthi=" << depthi << std::endl
                << RPS_FULL_BACKTRACE_HERE(2,
                    "?£!? rpsapply_8lKdW7lgcHV00WUOiT !method class/display_object_payload_fltk")
                << std::endl
               );
#warning unimplemented rpsapply_8lKdW7lgcHV00WUOiT
  RPS_FATAL("unimplemented rpsapply_8lKdW7lgcHV00WUOiT");
} // end of rpsapply_8lKdW7lgcHV00WUOiT




//////////////////////////////////////// end of file fltkhi_rps.cc

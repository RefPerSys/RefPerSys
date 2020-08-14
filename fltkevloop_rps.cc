/****************************************************************
 * file fltkevloop_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
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



extern "C" const char rps_fltkevloop_gitid[];
const char rps_fltkevloop_gitid[]= RPS_GITID;

extern "C" const char rps_fltkevloop_date[];
const char rps_fltkevloop_date[]= __DATE__;

static std::atomic<bool> rps_running_fltk;

static int rps_evloop_delay_factor = 1;


extern "C" pthread_t rps_main_gui_pthread;
pthread_t rps_main_gui_pthread;
Rps_GuiPreferences rps_gui_pref;

extern "C" int rps_fltk_arg_handler(int argc, char**argv, int &i);

std::string
rps_fltk_version(void)
{
  std::string res("FLTK ");
  char fltkgitbuf[48];
  memset (fltkgitbuf, 0, sizeof(fltkgitbuf));
  strncpy(fltkgitbuf, rps_fltkevloop_gitid, 3*sizeof(fltkgitbuf)/4);
  res += "git ";
  res += fltkgitbuf;
  res += ", ABI:";
  res += std::to_string(Fl::abi_version());
  res += ", API:";
  res += std::to_string(Fl::api_version());
  return res;
} // end rps_fltk_version




////////////////////////////////////////////////////////////////
class Rps_FltkEvLoop_CallFrame;
struct Rps_Todo_Base
{
  static std::atomic<unsigned> _todo_serial_counter_;
  static constexpr bool TODO_is_Function = true;
  static constexpr bool TODO_is_Closure = false;
  const bool todo_is_function;
  const int todo_lineno;
  const unsigned todo_serial;
  const double todo_timeout;
  const char* todo_filename;
  const char* todo_label;
  typedef std::function<void(Rps_CallFrame*,void*,void*)> todo_func_t;
  Rps_Todo_Base(bool isfun, int lineno, double delay, const char*label=nullptr)
    : todo_is_function(isfun), todo_lineno(lineno),
      todo_serial(1+_todo_serial_counter_.fetch_add(1)),
      todo_timeout(rps_monotonic_real_time()+delay),
      todo_filename(nullptr),
      todo_label (label) {};
  Rps_Todo_Base(bool isfun, const char*filename, int lineno, double delay, const char*label=nullptr)
    : todo_is_function(isfun), todo_lineno(lineno),
      todo_serial(1+_todo_serial_counter_.fetch_add(1)),
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
  unsigned serial(void) const
  {
    return todo_serial;
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
    todo_func(callframe, p1 ? p1 : todo_arg1, p2 ? p2 : todo_arg2);
  };
  void apply_todo_function(Rps_CallFrame*callframe)
  {
    todo_func(callframe, todo_arg1, todo_arg2);
  }
  const void*arg1() const
  {
    return todo_arg1;
  };
  const void*arg2() const
  {
    return todo_arg2;
  };
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
  void apply_todo_clos(Rps_FltkEvLoop_CallFrame*callfram) const
  {
    todo_closv.apply3(reinterpret_cast<Rps_CallFrame*>(callfram),
                      todo_arg1v, todo_arg2v, todo_arg3v);
  }
}; // end class Rps_Todo_Closure


class Rps_Todo
{
  union
  {
    Rps_Todo_Closure todo_closure;
    Rps_Todo_Function todo_function;
    Rps_Todo_Base todo_base;
    intptr_t todo_zone[1+std::max(sizeof(todo_closure),sizeof(todo_function))/sizeof(intptr_t)];
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
    if (todosrc.is_todo_function())
      new(&todo_zone) Rps_Todo(todosrc.todo_function);
    else
      new(&todo_zone) Rps_Todo(todosrc.todo_closure);
  };
  Rps_Todo(const Rps_Todo&todosrc)
  {
    if (todosrc.is_todo_function())
      new(&todo_zone) Rps_Todo(todosrc.todo_function);
    else
      new(&todo_zone) Rps_Todo(todosrc.todo_closure);
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
  unsigned serial(void) const
  {
    return todo_base.serial();
  };
  double timeout(void) const
  {
    return todo_base.timeout();
  };
  void output(std::ostream&out) const;
}; // end class Rps_Todo

inline std::ostream& operator << (std::ostream&out, const Rps_Todo& todo)
{
  todo.output(out);
  return out;
}

std::atomic<unsigned>  Rps_Todo_Base::_todo_serial_counter_;

void
Rps_Todo::output(std::ostream&out) const
{
  if (is_todo_function())
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
  else  // closure
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
    }
} // end Rps_Todo::output

Rps_Todo::~Rps_Todo()
{
  RPS_ASSERT(rps_is_main_gui_thread());
};				// end Rps_Todo::~Rps_Todo

////////////////////////////////////////////////////////////////

#warning Rps_FltkEventLoop_CallFrame should be redesigned

// the value fields in an Rps_FltkEvLoop_CallFrame are:
struct Rps_FltkEvloop_Fields
{
  Rps_Value evloopf_dummyv;
};				// end Rps_FltkEvloop_Fields

class Rps_FltkEvLoop_CallFrame :
  public Rps_FieldedCallFrame<Rps_FltkEvloop_Fields>
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
protected:
  /// after here, no instance GC-ed value anymore
  ////////////////
  // Our TODO machinery: the key of below ordered map is an absolute
  // time, as given by rps_monotonic_real_time the value in entries of
  // below map is a Todo. We need to ensure that entries are unique
  // (by adding a tiny random delay when needed). We need a mutex to
  // ensure other threads than the GUI one could add TODO things. We
  // need garbage collection support of these TODO.
  std::map<double,Rps_Todo> evloopfr_todos;
  mutable std::recursive_mutex evloopfr_mtx;
  Rps_FltkEvLoop_CallFrame*evloopfr_oldframe;
  int evloopfr_lineno;
  short evloopfr_depth;
  /// for debugging
  static std::atomic<long> evloopfr_counter_;
  long evloopfr_serial;
  /// the event loop frames in the GUI main thread are kept
  static std::atomic<Rps_FltkEvLoop_CallFrame*> evloopfr_curframe_;
  /// the event loop frames elsewhere are collected in a locked set
  static std::recursive_mutex evloopfr_setmtx_;
  static std::set<Rps_FltkEvLoop_CallFrame*> evloopfr_set_;
public:
  struct Rps_EventLoop_tag {};
  Rps_FltkEvLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
                           Rps_ObjectRef descr, int depth, Rps_EventLoop_tag);
  ~Rps_FltkEvLoop_CallFrame();
  static Rps_FltkEvLoop_CallFrame*
  find_calling_event_call_frame(const Rps_CallFrame*callframe);
  void gc_mark_todos(Rps_GarbageCollector*);
  void run_scheduled_fltk_todos(void);
};				// end class Rps_FltkEvLoop_CallFrame

/// static data of Rps_FltkEvLoop_CallFrame
std::atomic<long> Rps_FltkEvLoop_CallFrame::evloopfr_counter_;
std::recursive_mutex
Rps_FltkEvLoop_CallFrame::evloopfr_setmtx_;
std::set<Rps_FltkEvLoop_CallFrame*>
Rps_FltkEvLoop_CallFrame::evloopfr_set_;
std::atomic<Rps_FltkEvLoop_CallFrame*>
Rps_FltkEvLoop_CallFrame::evloopfr_curframe_;
///



Rps_FltkEvLoop_CallFrame::Rps_FltkEvLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
    Rps_ObjectRef descr, int depth, Rps_EventLoop_tag)
  :  Rps_FieldedCallFrame<Rps_FltkEvloop_Fields> (descr, callframe),
     evloopfr_todos(),
     evloopfr_mtx(),
     evloopfr_oldframe(nullptr),
     evloopfr_lineno(lineno),
     evloopfr_depth(depth),
     evloopfr_serial(1+evloopfr_counter_.fetch_add(1))
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(evloopfr_depth >= 0);
  evloopfr_oldframe= std::atomic_exchange(&evloopfr_curframe_,this);
  RPS_DEBUGNL_LOG(GUI, "+°Rps_FltkEvLoop_CallFrame-constr line#" << lineno
                  << " depth#" << evloopfr_depth
                  << " callframe@" << (void*)callframe
                  << " evlserial#" << evloopfr_serial
                  << " this@" << (void*)this<< std::endl
                  << RPS_FULL_BACKTRACE_HERE(0, "Rps_FltkEvLoop_CallFrame°-constru"));
} // end Rps_FltkEvLoop_CallFrame::Rps_FltkEvLoop_CallFrame

Rps_FltkEvLoop_CallFrame::~Rps_FltkEvLoop_CallFrame()
{
  auto serial = evloopfr_serial;
  RPS_DEBUG_LOG(GUI, "-°Rps_FltkEvLoop_CallFrame this@" << (void*)this << " evlserial#" << evloopfr_serial << std::endl
                << RPS_FULL_BACKTRACE_HERE(0, "Rps_FltkEvLoop_CallFrame°-destruc"));
  if (rps_is_main_gui_thread())
    {
      evloopfr_curframe_.store(evloopfr_oldframe);
    }
  else
    {
      std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
      evloopfr_set_.erase(this);
    }
  RPS_DEBUG_LOG(GUI, "-°~Rps_FltkEvLoop_CallFrame-destr this@" << (void*)this << " evlserial#" << serial << std::endl);
} // end Rps_FltkEvLoop_CallFrame::~Rps_FltkEvLoop_CallFrame




/// the object whose name is "fltk_event_loop" is a RefPerSys frame
/// descriptor for FTK event loops GC call frames.
#define RPS_FLTK_EVENT_LOOP_DESCR RPS_ROOT_OB(_39OsVkAJDdV00ohD5r)

Rps_FltkEvLoop_CallFrame*
Rps_FltkEvLoop_CallFrame::find_calling_event_call_frame(const Rps_CallFrame*callframe)
{
  Rps_FltkEvLoop_CallFrame*evcallframe = nullptr;
  for (const Rps_CallFrame*curcallframe = callframe;
       curcallframe != nullptr && Rps_CallFrame::is_good_call_frame(curcallframe);
       curcallframe = curcallframe->previous_call_frame())
    {
      if (curcallframe->call_frame_descriptor() == RPS_FLTK_EVENT_LOOP_DESCR)
        return (Rps_FltkEvLoop_CallFrame*)(curcallframe);
    }
  return nullptr;
} // end Rps_FltkEvLoop_CallFrame::find_calling_event_call_frame

void
Rps_FltkEvLoop_CallFrame::gc_mark_todos(Rps_GarbageCollector*gc)
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
} // end Rps_FltkEvLoop_CallFrame::gc_mark_todos



void
Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos(void)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_DEBUG_LOG(GUI, "start Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos depth#" << evloopfr_depth
                << " this@" << this << " evlserial#" << evloopfr_serial <<std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos*start"));

  std::lock_guard<std::recursive_mutex> gu(evloopfr_mtx);
  std::vector<Rps_Todo> todovect;
  RPS_DEBUG_LOG(GUI, "Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos depth#" << evloopfr_depth
                << " evlserial#" << evloopfr_serial
                << " evloopfr_todos@" << (void*)&evloopfr_todos
                << " evloopfr_todos size=" << evloopfr_todos.size());
  int todocnt=0;
  for (auto todoit: evloopfr_todos)
    {
      double timeout = todoit.first;
      if (timeout >= rps_monotonic_real_time())
        break;
      Rps_Todo& curtodo = todoit.second;
      todocnt++;
      RPS_DEBUG_LOG(GUI,
                    "Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos curtodo=" << curtodo
                    << " evlserial#" << evloopfr_serial
                    << " todocnt=" << todocnt);
      todovect.push_back(curtodo);
    };
  RPS_DEBUG_LOG(GUI,
                "Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos this@" << this
                << " depth#" << evloopfr_depth <<  " evlserial#" << evloopfr_serial << " should do " << todovect.size() << " todos");
  int loopcnt=0;
  for (auto curtodo : todovect)
    {
      loopcnt++;
      auto curit = evloopfr_todos.find(curtodo.timeout());
      RPS_ASSERT(curit != evloopfr_todos.end());
      evloopfr_todos.erase(curit);
      int todolineno = curtodo.lineno();
      const char*todofilename = curtodo.filename();
      const char*todolabel = curtodo.label();
      RPS_DEBUGNL_LOG(GUI,
                      "Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos todolineno=" << todolineno
                      << " todofilename=" << todofilename << " todolabel=" << todolabel
                      << " curtodo=" << curtodo << " loopcnt#" << loopcnt <<  " evlserial#" << evloopfr_serial);
      if (curtodo.is_todo_function())
        {
          Rps_Todo_Function& curtodofun = const_cast<Rps_Todo_Function&>(curtodo.as_todo_function());
          RPS_DEBUG_LOG(GUI,
                        "run_scheduled_fltk_todos function depth#"
                        << evloopfr_depth << " evlserial#" << evloopfr_serial
                        << " before applying function from "
                        << todofilename << ":" << todolineno << (todolabel?"labeled:":"") << (todolabel?todolabel:"")) ;
          curtodofun.apply_todo_function(this);
        }
      else   // curtodo is closure
        {
          auto& curtodoclos = curtodo.as_todo_closure();
          RPS_DEBUG_LOG(GUI,
                        "run_scheduled_fltk_todos closure depth#"
                        << evloopfr_depth << " evlserial#" << evloopfr_serial
                        << " before applying closure from "
                        << todofilename << ":" << todolineno << (todolabel?"labeled:":"") << (todolabel?todolabel:"")
                        << " closure=" << curtodoclos.get_todo_clos()
                        << " arg1=" << curtodoclos.get_todo_arg1()
                        << " arg2=" << curtodoclos.get_todo_arg2()
                        << " arg3=" << curtodoclos.get_todo_arg3());
          curtodoclos.apply_todo_clos(this);
        }
    }
  RPS_DEBUG_LOG(GUI, "end Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos depth#" << evloopfr_depth
                <<   " evlserial#" << evloopfr_serial<< std::endl);
}; // end Rps_FltkEvLoop_CallFrame::run_scheduled_fltk_todos

////////////////////////////////////////////////////////////////////////////////////////////////////



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
      which is an RpsOld_FltkEventLoop_CallFrame then invoke
      fltk_add_delayed_todo on it. We also need to handle the more
      complex case when rps_fltk_add_delayed_todo is called from a non
      GUI thread. */
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_todo_at filename=" << filename
                << " lineno=" << lineno << " label=" << label
                << " arg1=" << arg1
                << " arg2=" << arg2
                << " curframe@" << curframe);
  auto newtodo = Rps_Todo(Rps_Todo_Function(filename, lineno, label, delay, todo, arg1, arg2));
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_todo_at newtodo=" << newtodo);
  RPS_FATALOUT("incomplete rps_fltk_add_delayed_labeled_todo_at filename=" << filename
               << " lineno=" << lineno
               << " label=" << label);
  if (rps_is_main_gui_thread())
    {
      Rps_FltkEvLoop_CallFrame*eventcallframe =
        Rps_FltkEvLoop_CallFrame::find_calling_event_call_frame(curframe);
      if (!eventcallframe)
        RPS_FATALOUT("no event call frame in rps_fltk_add_delayed_todo for callframe@" << curframe);
      std::lock_guard<std::recursive_mutex> gu(eventcallframe->evloopfr_mtx);
      Fl::flush();
      double todotime = rps_monotonic_real_time() + delay;
      int loopcnt = 0;
      for (;;)
        {
          RPS_ASSERT(loopcnt < 32); // it is very unlikely that we
          // loop more than 32 times, we
          // would probably loop once or
          // twice...
          if (eventcallframe->evloopfr_todos.find(todotime) == eventcallframe->evloopfr_todos.end())
            {
              eventcallframe->evloopfr_todos.insert({todotime, newtodo});
              RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_todo_at inserted in eventcallframe@" << eventcallframe
                            << " evlserial#" << eventcallframe->evloopfr_serial
                            << " todotime=" << todotime
                            << " evloopfr_todos@" << (void*)(&eventcallframe->evloopfr_todos)
                            << " filename=" << filename
                            << " lineno=" << lineno << " label=" << label
                            << " newtodo=" << newtodo);
              break;
            }
          else
            todotime = rps_monotonic_real_time() + delay + Rps_Random::random_quickly_16bits()*1.0e-6;
          loopcnt++;
        }
    }
  else
    {
#warning unimplemented rps_fltk_add_delayed_todo for non GUI thread
      RPS_FATALOUT("unimplemented rps_fltk_add_delayed_labeled_todo_at nonGUI curframe=" << curframe
                   << " filename=" << filename << ", lineno=" << lineno
                   << " label=" << label << " delay=" << delay << " arg1=" << arg1 << " arg2=" << arg2);
    }
  RPS_ASSERT(todo);
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_todo_at ending filename=" << filename << ", lineno=" << lineno
                << " newtodo=" << newtodo << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "*end-rps_fltk_add_delayed_labeled_todo_at*") <<std::endl);
} // end rps_fltk_add_delayed_labeled_todo_at



void
rps_fltk_add_delayed_labeled_closure_at(Rps_CallFrame*curframe,const char*filename, int lineno, const char*label, double delay,
                                        Rps_ClosureValue closv,
                                        Rps_Value arg1v, Rps_Value arg2v)
{
  RPS_ASSERT(Rps_CallFrame::is_good_call_frame(curframe));
  /** TODO: we should find the call frame below the given callframe
      which is an Rps_FltkEvLoop_CallFrame then insert the todo in
      it. We also need to handle the more complex case when
      rps_fltk_add_delayed_todo is called from a non GUI thread. */
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_closure_at curframe@" << curframe
                << " filename=" << filename << " lineno=" << lineno
                << " label=" << label << " delay=" << delay
                << " closv=" << closv << " arg1v=" << arg1v << " arg2v=" << arg2v);
  auto newtodo = Rps_Todo(Rps_Todo_Closure(lineno, filename, delay, label, closv, arg1v, arg2v));
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_closure_at newtodo=" << newtodo);
  if (rps_is_main_gui_thread())
    {
      Rps_FltkEvLoop_CallFrame*eventcallframe
        = Rps_FltkEvLoop_CallFrame::find_calling_event_call_frame(curframe);
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
          RPS_ASSERT(loopcnt < 32); // it is very unlikely that we
          // loop more than 32 times, but
          // probably once or twice
          if (eventcallframe->evloopfr_todos.find(todotime) == eventcallframe->evloopfr_todos.end())
            {
              RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_closure_at_at inserted in eventcallframe=" << eventcallframe
                            << " evlserial#" << eventcallframe->evloopfr_serial
                            << " todotime=" << todotime
                            << " filename=" << filename
                            << " evloopfr_todos@" << (void*)(&eventcallframe->evloopfr_todos)
                            << " lineno=" << lineno << " label=" << label
                            << " delay=" << delay
                            << " closv=" << closv
                            << " arg1v=" << arg1v << " arg2v=" << arg2v);
              eventcallframe->evloopfr_todos.insert({todotime, newtodo});
              break;
            }
          else
            todotime = rps_monotonic_real_time() + delay + Rps_Random::random_quickly_16bits()*1.0e-6;
          loopcnt++;
        }
    }
  else
    {
#warning unimplemented rps_fltk_add_delayed_labeled_closure_at
      RPS_FATALOUT("unimplemented rps_fltk_add_delayed_labeled_closure_at curframe=" << curframe
                   << " filename=" << filename << ", lineno=" << lineno << " label=" << label
                   << " delay=" << delay << " closv=" << closv << " arg1v=" << arg1v << " arg2v=" << arg2v);
    }
  RPS_DEBUG_LOG(GUI, "rps_fltk_add_delayed_labeled_closure_at ending filename=" << filename << ", lineno=" << lineno
                << " todo=" << newtodo << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "*end-rps_fltk_add_delayed_labeled_closure_at*") <<std::endl);
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
  RPS_FATALOUT("incomplete rps_fltk_event_loop callframe=" << callframe << " depth=" << depth);
  unsigned long count=0;
#warning incomplete rps_fltk_event_loop
#if 0 && oldcode
  RpsOld_FltkEventLoop_CallFrame _(callframe, __LINE__,
                                   RPS_FLTK_EVENT_LOOP_DESCR /*fltk_event_loop*/,
                                   depth, RpsOld_FltkEventLoop_CallFrame::Rps_EventLoop_tag{});
#warning rps_fltk_event_loop should use RpsOld_FltkEventLoop_CallFrame
  RPS_DEBUGNL_LOG(GUI, "start of rps_fltk_event_loop depth#" << depth << " evlserial#" << _.evloopfr_serial
                  << " callframe@" << callframe << std::endl
                  <<  RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_event_loop start"));
  while (rps_running_fltk.load())
    {
      count++;
      double delay = rps_evloop_delay_factor*(RPS_DEBUG_ENABLED(GUI)?30.0:3.0);
      RPS_DEBUG_LOG(GUI, "rps_fltk_event_loop loop count#" << count << " depth=" << depth << " delay=" << delay);
      _.fltk_event_wait(count, delay);
    };
#endif 0 && oldcode
  RPS_DEBUG_LOG(GUI, "end of rps_fltk_event_loop depth#" << depth
                << " count=" << count
                << " callframe@" << callframe << std::endl << std::endl);
  depth--;
} // end rps_fltk_event_loop

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
  /// the RPS_DEBUG_LOG(GUI, ...) below log was useless, so we disable it
  RPS_NOPRINTOUT("rps_is_main_gui_thread pthread_self()=" << pthread_self()
                 << ", rps_main_gui_pthread=" << rps_main_gui_pthread);
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
      snprintf(titbuf, sizeof(titbuf), "RefPerSys/%.12s p%ld@%.40s",
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
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_0(callerframe,
                                      "TODO@ rps_fltk_initialize-initialize_menubar",
                                      0.1, [=](Rps_CallFrame*cf,void*,void*)
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
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_0(callerframe,
                                      "TODO@ rps_fltk_initialize-initialize_pack", 0.2, [=](Rps_CallFrame*cf,void*,void*)
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
  RPS_FATALOUT("incomplete rps_run_fltk_gui argc=" << argc << " argv=" << argv);
#if 0 && oldcode
  RpsOld_FltkEventLoop_CallFrame _(nullptr, __LINE__,
                                   RPS_FLTK_EVENT_LOOP_DESCR, 1,
                                   RpsOld_FltkEventLoop_CallFrame::Rps_EventLoop_tag{});
  RPS_DEBUG_LOG(GUI, "start rps_run_fltk_gui _@" << ((void*)&_)  << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_fltk_gui"));
  // for debugging purposes of the event loop we handle the
  // REFPERSYS_FLTKEVLOOP_DELAY_FACTOR environment variable...
  if (const char*delfact = getenv("REFPERSYS_FLTKEVLOOP_DELAY_FACTOR"))
    {
      rps_evloop_delay_factor = atoi(delfact);
      if (rps_evloop_delay_factor <= 0)
        rps_evloop_delay_factor = 1;
      RPS_INFORMOUT("rps_run_fltk_gui got REFPERSYS_FLTKEVLOOP_DELAY_FACTOR="
                    << delfact
                    << " so rps_evloop_delay_factor=" << rps_evloop_delay_factor);
    }
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
#endif /*oldcode*/
} // rps_run_fltk_gui




//////////////////////////////////////// end of file fltkevloop_rps.cc

/****************************************************************
 * file foxhead_rps.hh
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is the public C++ header file of the FOX based GUI
 *      See https://fox-toolkit.org/ for more about FOX
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



#ifndef HEADFOX_RPS_INCLUDED
#define HEADFOX_RPS_INCLUDED

#include "refpersys.hh"
#include "fx.h"
#include "fxdefs.h"



class Rps_FoxEvLoop_CallFrame; // forward declaration, see below

// we need to keep a kind, because one a todo has been done, its
// kind becomes noop....
enum Rps_TodoKind_t
{
  TODOK_noop,
  TODOK_function,
  TODOK_closure,
};
struct Rps_Todo_Base
{
  static std::atomic<unsigned> _todo_serial_counter_;
  mutable Rps_TodoKind_t todo_kind;
  const int todo_lineno; // C++ line number
  const unsigned todo_serial; // unique serial
  const double todo_timeout; // timeout threshold for
  // rps_monotonic_real_time()
  const char* todo_filename; // C++ file name
  const char* todo_label; // human readable label
  typedef std::function<void(Rps_CallFrame*,void*,void*)> todo_func_t;
  Rps_Todo_Base(Rps_TodoKind_t kind, int lineno, double delay, const char*label=nullptr)
    : todo_kind(kind), todo_lineno(lineno),
      todo_serial(1+_todo_serial_counter_.fetch_add(1)),
      todo_timeout(rps_monotonic_real_time()+delay),
      todo_filename(nullptr),
      todo_label (label) {};
  Rps_Todo_Base(Rps_TodoKind_t kind, const char*filename, int lineno, double delay, const char*label=nullptr)
    : todo_kind(kind), todo_lineno(lineno),
      todo_serial(1+_todo_serial_counter_.fetch_add(1)),
      todo_timeout(rps_monotonic_real_time()+delay),
      todo_filename(filename),
      todo_label (label) {};
  ~Rps_Todo_Base() {};
  bool is_todo_function(void) const
  {
    return todo_kind == TODOK_function;
  };
  bool is_todo_closure(void) const
  {
    return todo_kind == TODOK_closure;
  };
  bool is_todo_noop(void) const
  {
    return todo_kind == TODOK_noop;
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
  friend class FoxEventLoop_CallFrame;
  const todo_func_t todo_func;
  void* todo_arg1;
  void* todo_arg2;
public:
  Rps_Todo_Function(int lineno, double delay, const todo_func_t& todo, void*arg1, void*arg2)
    : Rps_Todo_Base(TODOK_function, lineno, delay), todo_func(todo), todo_arg1(arg1), todo_arg2(arg2) {};
  Rps_Todo_Function(int lineno, const char*label, double delay, const todo_func_t& todo, void*arg1, void*arg2)
    : Rps_Todo_Base(TODOK_function, lineno, delay, label), todo_func(todo), todo_arg1(arg1), todo_arg2(arg2) {};
  Rps_Todo_Function(const char*filename, int lineno, const char*label, double delay, const todo_func_t& todo, void*arg1, void*arg2)
    : Rps_Todo_Base(TODOK_function, filename,  lineno, delay, label), todo_func(todo), todo_arg1(arg1), todo_arg2(arg2) {};
  ~Rps_Todo_Function() {};
  void apply_todo_fun(Rps_CallFrame*callframe, void*p1=nullptr, void*p2=nullptr)
  {
    todo_func(callframe, p1 ? p1 : todo_arg1, p2 ? p2 : todo_arg2);
  };
  void apply_todo_function(Rps_CallFrame*callframe)
  {
    todo_func(callframe, todo_arg1, todo_arg2);
    todo_kind = TODOK_noop;
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
  friend class FoxEventLoop_CallFrame;
  const Rps_ClosureValue todo_closv;
  const Rps_Value todo_arg1v;
  const Rps_Value todo_arg2v;
  const Rps_Value todo_arg3v;
public:
  Rps_Todo_Closure(int lineno, const char*filename, double delay, const char*label, const Rps_ClosureValue closv,
                   const Rps_Value arg1v=nullptr, const Rps_Value arg2v=nullptr, const Rps_Value arg3v=nullptr)
    : Rps_Todo_Base(TODOK_closure, filename, lineno, delay, label),
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
  void apply_todo_clos(Rps_FoxEvLoop_CallFrame*callfram) const
  {
    todo_closv.apply3(reinterpret_cast<Rps_CallFrame*>(callfram),
                      todo_arg1v, todo_arg2v, todo_arg3v);
    todo_kind = TODOK_noop;
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
  Rps_TodoKind_t kind() const
  {
    return todo_base.todo_kind;
  };
  bool is_todo_function() const
  {
    return todo_base.is_todo_function();
  };
  bool is_todo_closure() const
  {
    return todo_base.is_todo_closure();
  };
  bool is_todo_noop() const
  {
    return todo_base.is_todo_noop();
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
  void apply_todo(Rps_FoxEvLoop_CallFrame*cf);
}; // end class Rps_Todo

inline std::ostream& operator << (std::ostream&out, const Rps_Todo& todo)
{
  todo.output(out);
  return out;
}


class Rps_FoxEvLoop_CallFrame;

/// for readability purposes, we need these functions below - knowing
/// the todo source location and label - and call them thru macros
/// transmitting __FILE__ and __FILE__ with a constexpr label, see
/// RPS_WARNOUT macro for inspiration
extern "C" void
rps_fox_add_delayed_labeled_todo_at(Rps_CallFrame*curframe, const char*filename, int lineno, const char*label, double delay,
                                    const std::function<void(Rps_CallFrame*,void*,void*)>& todo,
                                    void*arg1, void*arg2);
#define RPS_FOX_ADD_DELAYED_LABELED_TODO_AT(CurFrame,Label,Delay,Todo,A1,A2) do { \
    rps_fox_add_delayed_labeled_todo_at((CurFrame), __FILE__,__LINE__,	\
					 (Label),(Delay),(Todo),(A1),(A2)); \
} while(0)
#define RPS_FOX_ADD_DELAYED_LABELED_TODO_0(CurFrame,Label,Delay,Todo) \
  RPS_FOX_ADD_DELAYED_LABELED_TODO_AT((CurFrame),(Label),(Delay),(Todo),nullptr,nullptr)
#define RPS_FOX_ADD_DELAYED_LABELED_TODO_1(CurFrame,Label,Delay,Todo,A1) \
  RPS_FOX_ADD_DELAYED_LABELED_TODO_AT((CurFrame),(Label),(Delay),(Todo),(A1),nullptr)
#define RPS_FOX_ADD_DELAYED_LABELED_TODO_2(CurFrame,Label,Delay,Todo,A1,A2) \
  RPS_FOX_ADD_DELAYED_LABELED_TODO_AT((CurFrame),(Label),(Delay),(Todo),(A1),(A2))

extern "C" void
rps_fox_add_delayed_labeled_closure_at(Rps_CallFrame*curframe,const char*filename, int lineno, const char*label, double delay,
                                       Rps_ClosureValue closv,
                                       Rps_Value arg1v, Rps_Value arg2v);
#define RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_AT(CurFrame,Label,Delay,Clos,A1,A2) do { \
    rps_fox_add_delayed_labeled_todo_at((CurFrame), __FILE__,__LINE__,	\
					 (Label),(Delay),(Clos),(A1),(A2)); \
} while(0)
#define RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_0(CurFrame,Label,Delay,Clos) \
  RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_AT((CurFrame),(Label),(Delay),(Clos),nullptr,nullptr)
#define RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_1(CurFrame,Label,Delay,Clos,A1) \
  RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_AT((CurFrame),(Label),(Delay),(Clos),(A1),nullptr)
#define RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_2(CurFrame,Label,Delay,Clos,A1,A2) \
  RPS_FOX_ADD_DELAYED_LABELED_CLOSURE_AT((CurFrame),(Label),(Delay),(Clos),(A1),(A2))


// the ordered collection of todos inside a Rps_FoxEvLoop_CallFrame
// as its evloopfr_todo_coll field... See below.
class Rps_Todo_Collection
{
public:
  typedef  std::multimap<double,int> double_to_index_map_t;
  typedef double_to_index_map_t::iterator double_to_index_iterator_t;
private:
  Rps_FoxEvLoop_CallFrame* _todocoll_owning_callframe;
  // An internal vector of unique pointers to Todo-s. We refer to each
  // todo there by its index. That vector does not shrink, and may
  // contain null slots.
  std::vector<std::unique_ptr<Rps_Todo>> _todocoll_vect;
  // An ordered multimap from timeout to a todos index in above vector
  double_to_index_map_t _todocoll_timemap;
  // a first-in first-out chronological queue of todos indexes in above vector
  std::deque<int> _todocoll_fifoqueue;
  // internal routine to add a todo
  Rps_Todo* adding_todo(int ix /*index in _todocoll_vect*/, const Rps_Todo& todo);
public:
  Rps_FoxEvLoop_CallFrame* owning_call_frame(void) const
  {
    return _todocoll_owning_callframe;
  };
  Rps_Todo_Collection(Rps_FoxEvLoop_CallFrame*owncf=nullptr)
    : _todocoll_owning_callframe(owncf), _todocoll_vect(), _todocoll_timemap(), _todocoll_fifoqueue() {};
  ~Rps_Todo_Collection() {};
  Rps_Todo_Collection(const Rps_Todo_Collection&) = delete;
  Rps_Todo_Collection(Rps_Todo_Collection&&) = delete;
  // add a todo and return its index
  int add_todo(const Rps_Todo&);
  // run pending todos in given call frame and remove them; if provided,
  // curtim is the current monotonic time.
  void run_pending_todos(Rps_FoxEvLoop_CallFrame*cf, double curtim=0.0);
  // remove the already done or too old todos, that is NoOps
  void cleanup_done_or_old_todos(void);
  // garbage collect the collection
  void gc_mark_collected_todos(Rps_GarbageCollector*);
  int size() const
  {
    return (int) (_todocoll_timemap.size());
  };
};	       // end class Rps_Todo_Collection



// the value fields in an Rps_FoxEvLoop_CallFrame are:
struct Rps_FoxEvloop_Fields
{
  Rps_Value evloopf_dummyv;
};				// end Rps_FoxEvloop_Fields

class Rps_FoxEvLoop_CallFrame :
  public Rps_FieldedCallFrame<Rps_FoxEvloop_Fields>
{
  friend class Rps_Todo;
  friend void
  rps_fox_add_delayed_labeled_todo_at(Rps_CallFrame*curframe, const char*filename, int lineno, const char*label, double delay,
                                      const std::function<void(Rps_CallFrame*,void*,void*)>& todo,
                                      void*arg1, void*arg2);
  friend void
  rps_fox_add_delayed_labeled_closure_at(Rps_CallFrame*curframe,const char*filename, int lineno, const char*label, double delay,
                                         Rps_ClosureValue closv,
                                         Rps_Value arg1v, Rps_Value arg2v);
  friend class Rps_GarbageCollector;
  static void outputter(std::ostream&, const Rps_ProtoCallFrame*);
  /// for debugging
  static std::atomic<long> evloopfr_counter_;
protected:
  /// after here, no instance GC-ed value anymore
  ////////////////
  // Our TODO machinery: We need a mutex to ensure other threads than
  // the GUI one could add TODO things to our collections of TODOs. We
  // need garbage collection support of these TODO-s.
#warning should have a class Rps_Todo_Collection for our todos, with more than a map
  /* FIXME: this should be redesigned. The collection of todos should
     be a proper class Rps_TodoCollection, with a map from timeout to
     todos but also a FIFO queue of them... Probably as
     shared_ptr... */
  Rps_Todo_Collection evloopfr_todo_coll;
  mutable std::recursive_mutex evloopfr_mtx;
  Rps_FoxEvLoop_CallFrame*evloopfr_oldframe;
  int evloopfr_lineno;
  short evloopfr_depth;
  const char* evloopfr_file;
  long evloopfr_serial;
  /// the event loop frames in the GUI main thread are kept
  static std::atomic<Rps_FoxEvLoop_CallFrame*> evloopfr_curframe_;
  /// the event loop frames elsewhere are collected in a locked set
  static std::recursive_mutex evloopfr_setmtx_;
  static std::set<Rps_FoxEvLoop_CallFrame*> evloopfr_set_;
public:
  struct Rps_EventLoop_tag {};
  struct Rps_LoggedEventLoop_tag {};
  Rps_FoxEvLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
                          Rps_ObjectRef descr, int depth, Rps_EventLoop_tag);
  Rps_FoxEvLoop_CallFrame(Rps_CallFrame*callframe, const char*filename, int lineno,
                          Rps_ObjectRef descr, int depth, Rps_LoggedEventLoop_tag);
  ~Rps_FoxEvLoop_CallFrame();
  static Rps_FoxEvLoop_CallFrame*
  find_calling_event_call_frame(const Rps_CallFrame*callframe);
  void gc_mark_todos(Rps_GarbageCollector*);
  void run_scheduled_fox_todos(void);
  // fetch the event loop call frame below the current one, if any or
  // else null.
  Rps_FoxEvLoop_CallFrame*get_lower_evloop_callframe(void) const;
  /// return the current call frame, if inside the main thread and with an active Rps_FoxEvLoop_CallFrame
  static Rps_FoxEvLoop_CallFrame*current_call_frame(void)
  {
    return evloopfr_curframe_.load();
  };
};				// end class Rps_FoxEvLoop_CallFrame


class RpsGui_FoxApplication;
class RpsGui_FoxSimpleWindow : public FXMainWindow
{
  FXDECLARE(RpsGui_FoxSimpleWindow);
protected:
  FXMenuBar* guiwin_menubar;
  FXMenuPane* guiwin_appmenu;
  FXMenuPane* guiwin_editmenu;
  FXMenuPane* guiwin_helpmenu;
  void create_menubar(void);
private:
  RpsGui_FoxSimpleWindow() {};
  RpsGui_FoxSimpleWindow(const RpsGui_FoxSimpleWindow&);
  RpsGui_FoxSimpleWindow& operator= (const RpsGui_FoxSimpleWindow&);
public:
  static constexpr unsigned guiwin_default_width_ = 550;
  static constexpr unsigned guiwin_default_height_ = 444;
  static const char*make_title(void) {
    static char buf[80];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "RefPerSys/%.14s p%d@%s",
	     rps_gitid, (int)getpid(), rps_hostname());
    return buf;
  }
  static int parse_width_from_geometry(const char*str);
  static int parse_height_from_geometry(const char*str);
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdHelp(FXObject*,FXSelector,void*);
  long onCmdClose(FXObject*,FXSelector,void*);
  RpsGui_FoxSimpleWindow(RpsGui_FoxApplication*app);
  enum
  {
    ID_ABOUT=FXMainWindow::ID_LAST,
    ID_HELP,
    ID__LAST
  };
};				// end class RpsGui_FoxSimpleWindow


class RpsGui_FoxApplication : public FXApp
{
  FXDECLARE(RpsGui_FoxApplication);
  friend void rps_run_fox_gui(int&argc, char**argv);
  static RpsGui_FoxApplication* fxapp_inst_;
  RpsGui_FoxSimpleWindow* fxapp_simpwin;
  FXTime fxapp_waitdelay; // delay to wait the event loop, in nanoseconds
public:
  RpsGui_FoxApplication();
  virtual void create(); // create windows
  virtual ~RpsGui_FoxApplication();
  static RpsGui_FoxApplication*the_app()
  {
    return fxapp_inst_;
  };
  void run_app(Rps_FoxEvLoop_CallFrame*cf);
  RpsGui_FoxSimpleWindow* simple_window() const
  {
    return fxapp_simpwin;
  };
};				// end RpsGui_FoxApplication

#endif /*HEADFOX_RPS_INCLUDED*/
/// end of file headfox_rps.hh

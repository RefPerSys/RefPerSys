/****************************************************************
 * file fltkhead_rps.hh
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is the public C++ header file of the FLTK based GUI
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



#ifndef FLTKHEAD_RPS_INCLUDED
#define FLTKHEAD_RPS_INCLUDED

#include "refpersys.hh"

/// for FLTK - see https://www.fltk.org/

#include <FL/Fl.H>

// see https://www.fltk.org/doc-1.4/osissues.html#osissues_unix and
// https://stackoverflow.com/questions/61931734/fltk-1-4-widget-position-w-r-t-x11-root-window
#include <FL/platform.H>

#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/fl_ask.H>


//////////////// forward declaration of classes
class RpsGui_ShowWidget;
class RpsGui_SimpleWindow;
class RpsGui_CommandTextEditor;
class RpsGui_OutputTextEditor;
class RpsGui_OutputTextBuffer;
class RpsGui_InputCommand;
class RpsGui_MenuBar;
////////////////

class Rps_FltkEvLoop_CallFrame; // forward declaration, see below

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
  friend class FltkEventLoop_CallFrame;
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
  friend class FltkEventLoop_CallFrame;
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
  void apply_todo_clos(Rps_FltkEvLoop_CallFrame*callfram) const
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
  void apply_todo(Rps_FltkEvLoop_CallFrame*cf);
}; // end class Rps_Todo

inline std::ostream& operator << (std::ostream&out, const Rps_Todo& todo)
{
  todo.output(out);
  return out;
}


extern "C" void rps_fltk_event_loop(Rps_CallFrame*cf);

extern "C" void rps_fltk_stop_event_loop(void);

extern "C" void rps_fltk_initialize(int &argc, char**argv, Rps_CallFrame*curframe);


class Rps_FltkEvLoop_CallFrame;

/// for readability purposes, we need these functions below - knowing
/// the todo source location and label - and call them thru macros
/// transmitting __FILE__ and __FILE__ with a constexpr label, see
/// RPS_WARNOUT macro for inspiration
extern "C" void
rps_fltk_add_delayed_labeled_todo_at(Rps_CallFrame*curframe, const char*filename, int lineno, const char*label, double delay,
                                     const std::function<void(Rps_CallFrame*,void*,void*)>& todo,
                                     void*arg1, void*arg2);
#define RPS_FLTK_ADD_DELAYED_LABELED_TODO_AT(CurFrame,Label,Delay,Todo,A1,A2) do { \
    rps_fltk_add_delayed_labeled_todo_at((CurFrame), __FILE__,__LINE__,	\
					 (Label),(Delay),(Todo),(A1),(A2)); \
} while(0)
#define RPS_FLTK_ADD_DELAYED_LABELED_TODO_0(CurFrame,Label,Delay,Todo) \
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_AT((CurFrame),(Label),(Delay),(Todo),nullptr,nullptr)
#define RPS_FLTK_ADD_DELAYED_LABELED_TODO_1(CurFrame,Label,Delay,Todo,A1) \
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_AT((CurFrame),(Label),(Delay),(Todo),(A1),nullptr)
#define RPS_FLTK_ADD_DELAYED_LABELED_TODO_2(CurFrame,Label,Delay,Todo,A1,A2) \
  RPS_FLTK_ADD_DELAYED_LABELED_TODO_AT((CurFrame),(Label),(Delay),(Todo),(A1),(A2))

extern "C" void
rps_fltk_add_delayed_labeled_closure_at(Rps_CallFrame*curframe,const char*filename, int lineno, const char*label, double delay,
                                        Rps_ClosureValue closv,
                                        Rps_Value arg1v, Rps_Value arg2v);
#define RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_AT(CurFrame,Label,Delay,Clos,A1,A2) do { \
    rps_fltk_add_delayed_labeled_todo_at((CurFrame), __FILE__,__LINE__,	\
					 (Label),(Delay),(Clos),(A1),(A2)); \
} while(0)
#define RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_0(CurFrame,Label,Delay,Clos) \
  RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_AT((CurFrame),(Label),(Delay),(Clos),nullptr,nullptr)
#define RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_1(CurFrame,Label,Delay,Clos,A1) \
  RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_AT((CurFrame),(Label),(Delay),(Clos),(A1),nullptr)
#define RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_2(CurFrame,Label,Delay,Clos,A1,A2) \
  RPS_FLTK_ADD_DELAYED_LABELED_CLOSURE_AT((CurFrame),(Label),(Delay),(Clos),(A1),(A2))


// the ordered collection of todos inside a Rps_FltkEvLoop_CallFrame
// as its evloopfr_todo_coll field... See below.
class Rps_Todo_Collection
{
public:
  typedef  std::multimap<double,int> double_to_index_map_t;
  typedef double_to_index_map_t::iterator double_to_index_iterator_t;
private:
  Rps_FltkEvLoop_CallFrame* _todocoll_owning_callframe;
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
  Rps_FltkEvLoop_CallFrame* owning_call_frame(void) const
  {
    return _todocoll_owning_callframe;
  };
  Rps_Todo_Collection(Rps_FltkEvLoop_CallFrame*owncf=nullptr)
    : _todocoll_owning_callframe(owncf), _todocoll_vect(), _todocoll_timemap(), _todocoll_fifoqueue() {};
  ~Rps_Todo_Collection() {};
  Rps_Todo_Collection(const Rps_Todo_Collection&) = delete;
  Rps_Todo_Collection(Rps_Todo_Collection&&) = delete;
  // add a todo and return its index
  int add_todo(const Rps_Todo&);
  // run pending todos in given call frame and remove them; if provided,
  // curtim is the current monotonic time.
  void run_pending_todos(Rps_FltkEvLoop_CallFrame*cf, double curtim=0.0);
  // remove the already done or too old todos, that is NoOps
  void cleanup_done_or_old_todos(void);
  // garbage collect the collection
  void gc_mark_collected_todos(Rps_GarbageCollector*);
  int size() const
  {
    return (int) (_todocoll_timemap.size());
  };
};	       // end class Rps_Todo_Collection



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
  Rps_FltkEvLoop_CallFrame*evloopfr_oldframe;
  int evloopfr_lineno;
  short evloopfr_depth;
  const char* evloopfr_file;
  long evloopfr_serial;
  /// the event loop frames in the GUI main thread are kept
  static std::atomic<Rps_FltkEvLoop_CallFrame*> evloopfr_curframe_;
  /// the event loop frames elsewhere are collected in a locked set
  static std::recursive_mutex evloopfr_setmtx_;
  static std::set<Rps_FltkEvLoop_CallFrame*> evloopfr_set_;
public:
  struct Rps_EventLoop_tag {};
  struct Rps_LoggedEventLoop_tag {};
  Rps_FltkEvLoop_CallFrame(Rps_CallFrame*callframe, int lineno,
                           Rps_ObjectRef descr, int depth, Rps_EventLoop_tag);
  Rps_FltkEvLoop_CallFrame(Rps_CallFrame*callframe, const char*filename, int lineno,
                           Rps_ObjectRef descr, int depth, Rps_LoggedEventLoop_tag);
  ~Rps_FltkEvLoop_CallFrame();
  static Rps_FltkEvLoop_CallFrame*
  find_calling_event_call_frame(const Rps_CallFrame*callframe);
  void gc_mark_todos(Rps_GarbageCollector*);
  void fltk_event_wait(unsigned long count, double delay);
  void run_scheduled_fltk_todos(void);
  // fetch the event loop call frame below the current one, if any or
  // else null.
  Rps_FltkEvLoop_CallFrame*get_lower_evloop_callframe(void) const;
  /// return the current call frame, if inside the main thread and with an active Rps_FltkEvLoop_CallFrame
  static Rps_FltkEvLoop_CallFrame*current_call_frame(void)
  {
    return evloopfr_curframe_.load();
  };
};				// end class Rps_FltkEvLoop_CallFrame


////////////////////////////////////////////////////////////////
template<class FltkWidgetClass>
static inline bool
rps_fltk_get_window_geometry(const FltkWidgetClass*widg, int &x, int &y, int &w, int &h, float*scaleptr=nullptr)
{
  if (!widg)
    return false;
  int xoff= -1, yoff= -1;
  Fl_Window* window = widg->top_window_offset(xoff, yoff);
  if (!window || xoff<0 || yoff<0)
    return false;
  int widg_w= widg->w();
  int widg_h= widg->h();
  int wind_screen= window->screen_num();
  if (wind_screen < 0)
    return false;
  float scr_scale = Fl::screen_scale(wind_screen);
  if (scr_scale <= 0.0 || std::isnan(scr_scale))
    return false;
  if (scr_scale != 1.0)
    {
      widg_w = (int)(widg_w*scr_scale);
      widg_h = (int)(widg_h*scr_scale);
    }
  if (scaleptr)
    *scaleptr = scr_scale;
  x= xoff;
  y= yoff;
  w= widg_w;
  h= widg_h;
  return true;
} // end rps_fltk_get_window_geometry<FltkWidgetClass>


template<class FltkWidgetClass>
static inline std::string
rps_fltk_geometry_string(const FltkWidgetClass*widg)
{
  if (widg == nullptr)
    return "";
  Fl_Widget* parwid = widg->parent();
  int x= -1, y= -1, w= 0, h=0;
  float scale=1.0;
  const char* visiblestr =
    (widg->visible_r())
    ?"•"://U+2022 BULLET
    (parwid && parwid->visible_r())?"▫"// U+25AB WHITE SMALL SQUARE
    : "◌"; //U+25CC DOTTED CIRCLE
  if (rps_fltk_get_window_geometry<FltkWidgetClass>(widg, x, y, w, h, &scale))
    {
      char geombuf[80];
      memset (geombuf, 0, sizeof(geombuf));
      if (scale != 1.0)
        snprintf (geombuf, sizeof(geombuf), "[x=%d,y=%d,w=%d,h=%d,scale=%.3f%s]", x, y, w, h, scale, visiblestr);
      else
        snprintf (geombuf, sizeof(geombuf), "[x=%d,y=%d,w=%d,h=%d%s]", x, y, w, h, visiblestr);
      return std::string(geombuf);
    };
  return "[??]";
} // end rps_fltk_geometry_string


//////////
// NOTE ON ABOVE:
// Why do we need templates for rps_fltk_geometry_string() and
// rps_fltk_get_window_geometry()? Could we not do something like below, using
// only the Fl_Widget base class?
// Aslo, aren't floating point comparisons unsafe?

#pragma message "the RpsGui_Geometry class should follow the rule of five"
// see https://cpppatterns.com/patterns/rule-of-five.html
class RpsGui_Geometry
{
private:
  int gm_x;
  int gm_y;
  int gm_height;
  int gm_width;
  float gm_scale;
public:
  RpsGui_Geometry()
    : gm_x(0)
    , gm_y(0)
    , gm_height(0)
    , gm_width(0)
    , gm_scale(0.0f)
  { }

  RpsGui_Geometry(int x, int y, int width, int height, float scale)
    : gm_x(x)
    , gm_y(y)
    , gm_height(height)
    , gm_width(width)
    , gm_scale(scale)
  { }

  ~RpsGui_Geometry() { }

  inline int x() const
  {
    return gm_x;
  }
  inline void x(int val)
  {
    gm_x = val;
  }
  inline int y() const
  {
    return gm_y;
  }
  inline void y(int val)
  {
    gm_y = val;
  }
  inline int width() const
  {
    return gm_width;
  }
  inline void width(int val)
  {
    gm_width = val;
  }
  inline int height() const
  {
    return gm_height;
  }
  inline void height(int val)
  {
    gm_height = val;
  }
  inline float scale() const
  {
    return gm_scale;
  }
  inline void scale(float val)
  {
    gm_scale = val;
  }

  std::string str() const
  {
    std::stringstream ss;
    ss << "[x = " << x() << ", y = " << y() << ", w = " << width()
       << ", h = " << height();

    if (scale() != 1.0)
      ss << ", scale = " << scale();

    return ss.str();
  }

};


inline void
rpsgui_get_window_geometry(const Fl_Widget* wx, RpsGui_Geometry& gm)
{
  RPS_ASSERT (wx);

  int xoff = -1, yoff = -1;
  auto wnd = wx->top_window_offset(xoff, yoff);
  RPS_ASSERT (wnd && xoff >= 0 && yoff >= 0);

  gm.x(xoff);
  gm.y(yoff);

  auto scrnum = wnd->screen_num();
  RPS_ASSERT (scrnum >= 0);

  gm.scale(Fl::screen_scale(scrnum));
  RPS_ASSERT (!(gm.scale() <= 0.0f || std::isnan(gm.scale())));

  gm.width(wx->w() * gm.scale());
  gm.height(wx->h() * gm.scale());
}

////// END NOTE

/*** Below class is mostly to ease debugging, e.g. as
     RPS_DEBUG_LOG(GUI, "some widget=" << RpsGui_ShowWidget(widget));
 ***/
class RpsGui_ShowWidget
{
  const Fl_Widget* shown_widget;
public:
  RpsGui_ShowWidget(const Fl_Widget*widg = nullptr) : shown_widget(widg) {};
  RpsGui_ShowWidget(const Fl_Widget &widg) : RpsGui_ShowWidget(&widg) {};
  virtual ~RpsGui_ShowWidget()
  {
    shown_widget=nullptr;
  };
  RpsGui_ShowWidget(const RpsGui_ShowWidget&) = delete;
  virtual void output (std::ostream* pout) const;
  void output (std::ostream& out) const
  {
    output(&out);
  };
  const Fl_Widget* widget() const
  {
    return shown_widget;
  };
};				// end of RpsGui_ShowWidget

inline std::ostream&
operator << (std::ostream& out, const RpsGui_ShowWidget&shw)
{
  shw.output(out);
  return out;
} // end output operator <<  RpsGui_ShowWidget


template <class FltkWidgetClass>
class RpsGui_ShowFullWidget : public RpsGui_ShowWidget
{
public:
  RpsGui_ShowFullWidget(const FltkWidgetClass*widg = nullptr)
    : RpsGui_ShowWidget(widg) {};
  RpsGui_ShowFullWidget(const FltkWidgetClass* &widg)
    : RpsGui_ShowWidget(widg) {};
  virtual ~RpsGui_ShowFullWidget() {};
  RpsGui_ShowFullWidget(const RpsGui_ShowFullWidget&) = delete;
  virtual void output (std::ostream* pout) const
  {
    if (!pout) return;
    RpsGui_ShowWidget::output(pout);
    *pout << rps_fltk_geometry_string<FltkWidgetClass>(dynamic_cast<const FltkWidgetClass*>(widget()));
  };
};				// end  RpsGui_ShowFullWidget

template <class FltkWidgetClass>
static inline
std::ostream&operator << (std::ostream& out, const RpsGui_ShowFullWidget<FltkWidgetClass>&shw)
{
  shw.output(&out);
  return out;
} // end output operator << on RpsGui_ShowFullWidget




////////////////////////////////////////////////////////////////

enum RpsGui_WinTypes
{
  /// every FLTK widget has a type. Please grep the FLTK header files for "RESERVED_TYPE".
  RpsGuiTy__StartUnusedIndex = FL_DOUBLE_WINDOW+2,
  RpsGuiTy_SimpleWindow, //is probably 244
  RpsGuiTy_MenuBar,
  RpsGuiTy_Pack,
};

class RpsGui_SimpleWindow://  of fltktype RpsGuiTy_SimpleWindow
  public Fl_Double_Window
{
protected:
  RpsGui_MenuBar *guiwin_menubar;
  std::string guiwin_label;
  static constexpr int right_menu_gap=12;
  static constexpr int menu_height=24;
public:
  virtual int handle(int);
  static constexpr int guiwin_border = 7;
  void initialize_menubar(void);
  RpsGui_SimpleWindow(int w, int h, const std::string& lab);
  RpsGui_SimpleWindow(int x, int y, int w, int h, const std::string& lab);
public:
  virtual ~RpsGui_SimpleWindow();
  const std::string label_str(void) const
  {
    return guiwin_label;
  };
private:
  static void menu_dump_cb(Fl_Widget*widg, void*ptr);
  static void menu_exit_cb(Fl_Widget*widg, void*ptr);
  static void menu_quit_cb(Fl_Widget*widg, void*ptr);
};				// end class RpsGui_SimpleWindow

extern "C" RpsGui_SimpleWindow* rps_the_simple_window;

////////////////
class RpsGui_MenuBar : public Fl_Menu_Bar
{
public:
  virtual uchar type(void) const
  {
    return RpsGuiTy_MenuBar;
  };
  RpsGui_MenuBar(int X, int Y, int W, int H, const char*lab=nullptr);
  virtual ~RpsGui_MenuBar();
};				// end class RpsGui_MenuBar

class RpsGui_Pack : public Fl_Pack
{
public:
  virtual uchar type(void) const
  {
    return RpsGuiTy_Pack;
  };
  RpsGui_Pack(int X, int Y, int W, int H, const char*lab=nullptr);
  virtual ~RpsGui_Pack();
};				// end class RpsGui_Pack




class RpsGui_OutputTextBuffer : public Fl_Text_Buffer
{
};				// end class RpsGui_OutputTextBuffer

class RpsGui_InputCommand
{
};				// end class RpsGui_InputCommand

#endif /*FLTKHEAD_RPS_INCLUDED*/
// end of file fltkhead_rps.hh */

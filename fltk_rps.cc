/****************************************************************
 * file fltk_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the FLTK 1.3 graphical interface.  See
 *      also https://fltk.org - download FLTK source code and compile
 *      it with debugging enabled, see our README.md for more.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2024 - 2024 The Reflective Persistent System Team
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
 *
 ************************************************************************/

#include "refpersys.hh"

#ifndef RPS_WITH_FLTK
#error RefPerSys without FLTK
#endif

#if !RPS_WITH_FLTK
#error RefPerSys without FLTK toolkit
#endif

#warning fltk_rps.cc should be backported to FLTK 1.3

#include <stdarg.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Multi_Label.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
/// Fl_Flex.h is only in FLTK 1.4 not FLTK 1.3
#if FL_API_VERSION >= 10400
#include <FL/Fl_Flex.H>
#endif

/// conventional strings
extern "C" const char rps_fltk_gitid[];
const char rps_fltk_gitid[]= RPS_GITID;

extern "C" const char rps_fltk_date[];
const char rps_fltk_date[]= __DATE__;

extern "C" const char rps_fltk_shortgitid[];
const char rps_fltk_shortgitid[]= RPS_SHORTGITID;

extern "C" const int rps_fltk_api_version;
const int rps_fltk_api_version = FL_API_VERSION;

////////////////////////////////////////////////////////////////////////
////// ******** DECLARATIONS ********

class Rps_PayloadFltkThing;
class Rps_PayloadFltkWidget;
class Rps_PayloadFltkRefWidget;
class Rps_PayloadFltkWindow;
class Rps_FltkMainWindow;
class Rps_FltkDebugWindow;

extern "C" Rps_FltkMainWindow* rps_fltk_mainwin;
Rps_FltkMainWindow* rps_fltk_mainwin;

extern "C" Rps_FltkDebugWindow* rps_fltk_debugwin;
Rps_FltkDebugWindow* rps_fltk_debugwin;

extern "C" bool rps_fltk_is_initialized;

bool rps_fltk_is_initialized;


extern "C" void rps_fltk_input_fd_handler(FL_SOCKET fd, void *data);
extern "C" void rps_fltk_output_fd_handler(FL_SOCKET fd, void *data);

/// temporary payload for any FLTK object
class Rps_PayloadFltkThing : public Rps_Payload
{
  friend Rps_PayloadFltkThing*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadFltkThing,Rps_ObjectZone*>(Rps_ObjectZone*);
protected:
  union
  {
    void*fltk_ptr;
    // see https://github.com/fltk/fltk/issues/975
    Fl_Widget*fltk_widget;
    Fl_Window*fltk_window;
    Fl_Text_Buffer*fltk_text_buffer;
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadFltkThing(Rps_ObjectZone*owner);
  Rps_PayloadFltkThing(Rps_ObjectRef obr) :
    Rps_Payload(Rps_Type::PaylFltkThing,obr?obr.optr():nullptr), fltk_ptr(nullptr) {};
  Rps_PayloadFltkThing(Rps_Type rty, Rps_ObjectRef obr) :
    Rps_Payload(rty,obr?obr.optr():nullptr), fltk_ptr(nullptr)
  {
    RPS_ASSERT(rty== Rps_Type::PaylFltkWidget
               || rty==Rps_Type::PaylFltkWindow
               || rty==Rps_Type::PaylFltkThing);
  };
  Rps_PayloadFltkThing(Rps_Type rty, Rps_ObjectRef obr, Fl_Widget*wid) :
    Rps_Payload(rty,obr?obr.optr():nullptr), fltk_widget(wid)
  {
    RPS_ASSERT(rty== Rps_Type::PaylFltkWidget);
  };
  virtual const std::string payload_type_name(void) const
  {
    return "FltkThing";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
public:
  virtual ~Rps_PayloadFltkThing() =0;
};        // end class Rps_PayloadFltkThing

////////////////////////////////////////////////////////////////


/// temporary payload for any FLTK widget
class Rps_PayloadFltkWidget : public Rps_PayloadFltkThing
{
  friend Rps_PayloadFltkWidget*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadFltkWidget,Rps_ObjectZone*>(Rps_ObjectZone*);
  friend Rps_PayloadFltkWidget*
  Rps_QuasiZone::rps_allocate2<Rps_PayloadFltkWidget,Rps_ObjectZone*,Fl_Widget*>(Rps_ObjectZone*,Fl_Widget*);
  inline Rps_PayloadFltkWidget(Rps_ObjectZone*owner, Fl_Widget*wid);
  Rps_PayloadFltkWidget(Rps_ObjectRef obr)
    : Rps_PayloadFltkThing(Rps_Type::PaylFltkWidget, obr, nullptr) {};
  Rps_PayloadFltkWidget(Rps_ObjectRef obr, Fl_Widget*wid)
    : Rps_PayloadFltkThing(Rps_Type::PaylFltkWidget, obr, wid) {};
  virtual const std::string payload_type_name(void) const
  {
    if (fltk_widget)
      {
        std::string typwidname = typeid(*fltk_widget).name();
        return "FltkWidget/" + typwidname;
      }
    else
      return "FltkWidget-nil";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
  virtual ~Rps_PayloadFltkWidget()
  {
    delete fltk_widget;
  };
  Fl_Widget* get_widget(void) const
  {
    return fltk_widget;
  };
};        // end class Rps_PayloadFltkWidget


/// temporary payload for a reference to any FLTK widget
class Rps_PayloadFltkRefWidget :
  public Rps_PayloadFltkThing
{
  friend Rps_PayloadFltkRefWidget*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadFltkRefWidget,Rps_ObjectZone*>(Rps_ObjectZone*);
  friend Rps_PayloadFltkRefWidget*
  Rps_QuasiZone::rps_allocate2<Rps_PayloadFltkRefWidget,Rps_ObjectZone*,Fl_Widget*>(Rps_ObjectZone*,Fl_Widget*);
  inline Rps_PayloadFltkRefWidget(Rps_ObjectZone*owner, Fl_Widget*wid);
  Rps_PayloadFltkRefWidget(Rps_ObjectRef obr)
    : Rps_PayloadFltkThing(Rps_Type::PaylFltkRefWidget, obr, nullptr) {};
  Rps_PayloadFltkRefWidget(Rps_ObjectRef obr, Fl_Widget*wid)
    : Rps_PayloadFltkThing(Rps_Type::PaylFltkRefWidget, obr, wid)
  {
  };
  virtual const std::string payload_type_name(void) const
  {
    if (fltk_widget)
      {
        std::string typwidname = typeid(*fltk_widget).name();
        return "FltkRefWidget/" + typwidname;
      }
    else
      return "FltkRefWidget-nil";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
  virtual ~Rps_PayloadFltkRefWidget()
  {
    if (!owner()) return;
    std::unique_lock<std::recursive_mutex> guown (*(owner()->objmtxptr()));
    owner()->clear_payload();
    //FIXME: should we call explicitly ~Callback_User_Data()?
  };
  Fl_Widget* get_widget(void) const
  {
    return fltk_widget;
  };
};        // end class Rps_PayloadFltkRefWidget

Rps_PayloadFltkRefWidget::Rps_PayloadFltkRefWidget(Rps_ObjectZone*owner, Fl_Widget*wid)
  : Rps_PayloadFltkRefWidget(Rps_ObjectRef(owner),wid)
{
  if (wid)
    {
      RPS_ASSERT(owner);
      wid->user_data(this);
    }
} // end Rps_PayloadFltkRefWidget::Rps_PayloadFltkRefWidget


////////////////

class Rps_FltkInputTextEditor : public Fl_Text_Editor
{
  /* inspired by FLTK 1.4 examples/texteditor-with-dynamic-colors.cxx;
     it contains two data pointers with a mutex */
public:
  typedef void datadestroyer_sigt(Rps_FltkInputTextEditor*,void*,void*);
private:
  mutable std::recursive_mutex inputx_mtx;
  void* inputx_data1;
  void* inputx_data2;
  datadestroyer_sigt* destroy_datafunptr;
  std::function<datadestroyer_sigt> destroy_clos;
  Fl_Text_Buffer *inputx_textbuf;      // text buffer
  Fl_Text_Buffer *inputx_stylbuf;      // style buffer
public:
  template<typename Ty> Ty* data1() const
  {
    std::lock_guard<std::recursive_mutex> gu(inputx_mtx);
    return (Ty*)inputx_data1;
  };
  template<typename Ty> Ty* data2() const
  {
    std::lock_guard<std::recursive_mutex> gu(inputx_mtx);
    return (Ty*)inputx_data2;
  };
  void set_data(datadestroyer_sigt*fun,void*p1,void*p2)
  {
    std::lock_guard<std::recursive_mutex> gu(inputx_mtx);
    destroy_datafunptr = fun;
    destroy_clos = nullptr;
    inputx_data1 = p1;
    inputx_data2 = p2;
  };
  void set_data_with_closure(std::function<datadestroyer_sigt> clos, void*p1, void*p2)
  {
    std::lock_guard<std::recursive_mutex> gu(inputx_mtx);
    destroy_datafunptr = nullptr;
    destroy_clos = clos;
    inputx_data1 = p1;
    inputx_data2 = p2;
  };
  void clear_data(void)
  {
    std::lock_guard<std::recursive_mutex> gu(inputx_mtx);
    if (destroy_clos)
      destroy_clos(this,inputx_data1,inputx_data2);
    else if (destroy_datafunptr)
      (*destroy_datafunptr)(this,inputx_data1,inputx_data2);
    inputx_data1 = nullptr;
    inputx_data2 = nullptr;
    destroy_datafunptr = nullptr;
    destroy_clos = nullptr;
  };
  Rps_FltkInputTextEditor(int x, int y, int w, int h);
  virtual ~Rps_FltkInputTextEditor();
};              // end  Rps_FltkInputTextEditor

class Rps_FltkOutputTextDisplay : public  Fl_Text_Display
{
  /* inspired by FLTK 1.4 examples/texteditor-with-dynamic-colors.cxx */
  Fl_Text_Buffer *outputx_textbuf;      // text buffer
  Fl_Text_Buffer *outputx_stylbuf;      // style buffer
#warning Rps_FltkOutputTextDisplay need a lot more code
public:
  Rps_FltkOutputTextDisplay(int x, int y, int w, int h);
  virtual ~Rps_FltkOutputTextDisplay();
};              // end  Rps_FltkOutputTextDisplay

class Rps_FltkMainWindow: public Fl_Window
{
  friend class Rps_FltkDebugWindow;
  Fl_Menu_Bar* _mainwin_menubar;
#if FL_API_VERSION>=10400
  Fl_Flex* _mainwin_vflex;
#endif
  std::array<std::shared_ptr<Fl_Menu_Item>,
      (std::size_t)(2+(int)RPS_DEBUG__LAST)>
      _mainwin_dbgmenuarr;
  std::vector<std::string> _mainwin_stringvect;
  std::vector<char*> _mainwin_cstrvect;
  bool _mainwin_closing;
  void add_menu_item_for_debug_option(Rps_Debug dbg);
protected:
  void fill_main_window(void);
  void register_mainwin_string(const std::string&str);
  static void main_menu_cb(Fl_Widget*w, void*data);
  static void close_cb(Fl_Widget*w, void*data);
public:
  bool is_closing() const
  {
    return _mainwin_closing;
  };
  Rps_FltkMainWindow(int x, int y, int w, int h, const char*title);
  Rps_FltkMainWindow(int w, int h, const char*title);
  const char* asprintf_mainwin(const char*fmt, ...)
  __attribute__((format(printf, 2, 3))); /// since first arg is this
  virtual ~Rps_FltkMainWindow();
};        // end Rps_FltkMainWindow;


class Rps_FltkDebugWindow: public Fl_Window
{
  friend class Rps_FltkMainWindow;
  Fl_Menu_Bar* _dbgwin_menubar;
  Fl_Tile* _dbgwin_tile;
  Fl_Text_Buffer* _dbgwin_text_buffer;
  Fl_Text_Display* _dbgwin_top_text_display;
  Fl_Text_Display* _dbgwin_bottom_text_display;
  char _dbgwin_labuf[80];
  void fill_debug_window(void);
  static void debug_menu_cb(Fl_Widget*w, void*data);
public:
  static constexpr int min_width=200;
  static constexpr int min_height=100;
  Rps_FltkDebugWindow(int x, int y, int w, int h);
  Rps_FltkDebugWindow(int w, int h);
  virtual ~Rps_FltkDebugWindow();
};        // end Rps_FltkDebugWindow





////////////////////////////////////////////////////////////////
//////// ******* IMPLEMENTATION *******

Rps_PayloadFltkWidget::Rps_PayloadFltkWidget(Rps_ObjectZone*owner, Fl_Widget*wid)
  : Rps_PayloadFltkWidget(Rps_ObjectRef(owner), wid) {};

void
rps_fltk_add_input_fd(int fd,
                      Rps_EventHandler_sigt* f,
                      const char* explanation,
                      int ix)
{
  RPS_DEBUG_LOG(REPL, "rps_fltk_add_input_fd fd#" << fd
                << (explanation?" ":"") << (explanation?explanation:"")
                << " ix#" << ix << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_add_input_fd"));
  Fl::add_fd(fd, POLLIN, rps_fltk_input_fd_handler, (void*)(intptr_t)ix);
} // end rps_fltk_add_input_fd


void
rps_fltk_add_output_fd(int fd,
                       Rps_EventHandler_sigt* f,
                       const char* explanation,
                       int ix)
{
  RPS_DEBUG_LOG(REPL, "rps_fltk_add_input_fd fd#" << fd
                << (explanation?" ":"") << (explanation?explanation:"")
                << " ix#" << ix << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_add_input_fd"));
  Fl::add_fd(fd, POLLOUT, rps_fltk_output_fd_handler, (void*)(intptr_t)ix);
} // end rps_fltk_add_input_fd


/* this gets called by FLTK event loop when some input is readable on
   fd */
void
rps_fltk_input_fd_handler(FL_SOCKET fd, void *hdata)
{
  int ix=(int)(intptr_t)hdata;
  Rps_EventHandler_sigt*funptr=nullptr;
  struct pollfd pe= {};
  const char*expl=nullptr;
  void*data=nullptr;
  bool ok=rps_event_loop_get_entry(ix, &funptr, &pe, &expl, &data);
  RPS_DEBUG_LOG(REPL, "rps_fltk_input_fd_handler fd#" << fd
                << " ix#" << ix
                << (ok?"OK":"BAD") << " funptr@" << (void*)funptr
                << (expl?" expl:": "NOEXPL")
                << (expl?expl:"")
                << " data@" << data
                << " thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_input_fd_handler")
               );
  if (!ok)
    return;
  if (pe.fd != fd)
    return;
  if (!funptr)
    return;
  /* NOTICE: think more about garbage collection interaction with FLTK
     event loop. */
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/rps_curthread_callframe, //
                 /** locals **/
                 Rps_Value v;
                );
  _f.v = nullptr;
  (*funptr) (&_, fd, data);
  RPS_DEBUG_LOG(REPL, "rps_fltk_input_fd_handler done fd#" << fd
                << " ix#" << ix <<std::endl);
} // end rps_fltk_input_fd_handler


/* this gets called by FLTK event loop when some input is readable on
   fd */
void
rps_fltk_output_fd_handler(FL_SOCKET fd, void *hdata)
{
  int ix=(int)(intptr_t)hdata;
  Rps_EventHandler_sigt*funptr=nullptr;
  struct pollfd pe= {};
  const char*expl=nullptr;
  void*data=nullptr;
  bool ok=rps_event_loop_get_entry(ix, &funptr, &pe, &expl, &data);
  RPS_DEBUG_LOG(REPL, "rps_fltk_output_fd_handler fd#" << fd
                << " ix#" << ix
                << (ok?"OK":"BAD") << " funptr@" << (void*)funptr
                << (expl?" expl:": "NOEXPL")
                << (expl?expl:"")
                << " data@" << data
                << " thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_output_fd_handler")
               );
  if (!ok)
    return;
  if (pe.fd != fd)
    return;
  if (!funptr)
    return;
  /* NOTICE: think more about garbage collection interaction with FLTK
     event loop. */
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/rps_curthread_callframe, //
                 /** locals **/
                 Rps_Value v;
                );
  _f.v = nullptr;
  (*funptr) (&_, fd, data);
  RPS_DEBUG_LOG(REPL, "rps_fltk_output_fd_handler done fd#" << fd
                << " ix#" << ix <<std::endl);
} // end rps_fltk_output_fd_handler


void
rps_fltk_remove_input_fd(int fd)
{
  Fl::remove_fd(fd, POLLIN);
  RPS_DEBUG_LOG(REPL, "rps_fltk_remove_input_fd fd#" << fd
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_remove_input_fd"));
} // end rps_fltk_remove_input_fd

void
rps_fltk_remove_output_fd(int fd)
{
  Fl::remove_fd(fd, POLLOUT);
  RPS_DEBUG_LOG(REPL, "rps_fltk_remove_output_fd fd#" << fd
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_remove_output_fd"));
} // end rps_fltk_remove_output_fd

Rps_PayloadFltkThing::Rps_PayloadFltkThing(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylFltkThing,owner), fltk_ptr(nullptr)
{
#warning incomplete Rps_PayloadFltkThing::Rps_PayloadFltkThing
} // end of Rps_PayloadFltkThing::Rps_PayloadFltkThing

Rps_PayloadFltkThing::~Rps_PayloadFltkThing()
{
#warning incomplete Rps_PayloadFltkThing::~Rps_PayloadFltkThing
} // end destructor Rps_PayloadFltkThing::~Rps_PayloadFltkThing

void
Rps_PayloadFltkThing::gc_mark(Rps_GarbageCollector&gc) const
{
#warning incomplete Rps_PayloadFltkThing::gc_mark
} // end of Rps_PayloadFltkThing::gc_mark

void
Rps_PayloadFltkThing::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du);
  RPS_POSSIBLE_BREAKPOINT();
  // do nothing, since temporary payload
} // end Rps_PayloadFltkThing::dump_scan

void
Rps_PayloadFltkThing::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT(du);
  RPS_POSSIBLE_BREAKPOINT();
  // do nothing, since temporary payload
} // end Rps_PayloadFltkThing::dump_json_content




////////////////

Rps_FltkMainWindow::Rps_FltkMainWindow(int x, int y, int w, int h, const char*title)
  : Fl_Window(x,y,w,h,title),
    _mainwin_menubar(nullptr),
#if FL_API_VERSION >= 10400
    _mainwin_vflex(nullptr),
#endif
    _mainwin_dbgmenuarr(), _mainwin_stringvect(), _mainwin_cstrvect(),
    _mainwin_closing(false)
{
  constexpr int estimatenbstring = 20;
  _mainwin_stringvect.reserve(estimatenbstring);
  constexpr int estimatenbcstr = 32;
  _mainwin_cstrvect.reserve(estimatenbcstr);
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow x=" << x << ",y=" << y
                << ",w=" << w << ",h=" << h
                << ",title=" << Rps_Cjson_String(title)
                << " @" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow/xywh"));
  fill_main_window();
};

Rps_FltkMainWindow::Rps_FltkMainWindow(int w, int h, const char*title)
  : Rps_FltkMainWindow(10 + (Rps_Random::random_32u() % 32)*10,
                       10 + (Rps_Random::random_32u() % 32)*10,
                       w, h, title)
{
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow w=" << w << ",h=" << h
                << ",title=" << Rps_Cjson_String(title)
                << " @" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow/wh"));
}; // end Rps_FltkMainWindow::Rps_FltkMainWindow


// This internal function is needed for memory management purposes, to
// be sure the data inside the strings is kept allocated
void
Rps_FltkMainWindow::register_mainwin_string(const std::string&s)
{
  _mainwin_stringvect.push_back(s);
} // end Rps_FltkMainWindow::register_mainwin_string

const char*
Rps_FltkMainWindow::asprintf_mainwin(const char*fmt, ...)
{
  char*res = nullptr;
  RPS_POSSIBLE_BREAKPOINT();
  RPS_ASSERT(fmt);
  va_list args;
  va_start (args, fmt);
  int l = vasprintf(&res, fmt, args);
  va_end (args);
  if (l >= 0 && res)
    {
      _mainwin_cstrvect.push_back(res);
      RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::asprintf_mainwin #"
                    << _mainwin_cstrvect.size()-1 << "@" << (void*)res
                    << '"' << Rps_Cjson_String(res) << '"' << std::endl);
      return res;
    }
  RPS_FATALOUT("Rps_FltkMainWindow::asprintf_mainwin fmt="
               << "\'" << Rps_Cjson_String(fmt) << "\' failed %m");
} // end Rps_FltkMainWindow::asprintf_mainwin

void
Rps_FltkMainWindow::add_menu_item_for_debug_option(Rps_Debug dbglev)
{
  RPS_POSSIBLE_BREAKPOINT();
  const char* dbgitcstr = asprintf_mainwin("Debug/%s", rps_cstr_of_debug(dbglev));
  const char* datacstr = asprintf_mainwin("d:%s", rps_cstr_of_debug(dbglev));
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::add_menu_item_for_debug_option dbglev#"
                << (int)dbglev << ":" << rps_cstr_of_debug(dbglev)
                << " dbgitcstr:" << dbgitcstr << "@" << ((void*)(dbgitcstr))
                << " datacstr:" << datacstr << "@" << ((void*)(datacstr))
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,
                    "Rps_FltkMainWindow::add_menu_item_for_debug_option"));
  int rk = _mainwin_menubar->add(dbgitcstr,
                                 nullptr, main_menu_cb, (void*) datacstr,
                                 FL_MENU_TOGGLE);
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::add_menu_item_for_debug_option rk="
                << rk);
  Fl_Menu_Item*mitem = const_cast<Fl_Menu_Item*>(_mainwin_menubar->menu()+rk);
  RPS_ASSERT(mitem != nullptr);
  if (rps_debug_flags & (1 << unsigned(dbglev)))
    mitem->set();
  else
    mitem->clear();
  RPS_POSSIBLE_BREAKPOINT();
} // end Rps_FltkMainWindow::add_menu_item_for_debug_option

void
Rps_FltkMainWindow::fill_main_window(void)
{
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::fill_main_window"
                << " w=" << w() << ",h=" << h() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow/fill_main_window"));
  this->begin();
  //////////// the menubar
  {
    _mainwin_menubar = new Fl_Menu_Bar(0, 0, w(), 25);
    RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::fill_main_window _mainwin_menubar@" << (void*)_mainwin_menubar);
    _mainwin_menubar->add("&App/e&Xit", "^x", main_menu_cb, (void*)"X");
    _mainwin_menubar->add("&App/&Quit", "^q", main_menu_cb, (void*)"Q");
    _mainwin_menubar->add("&App/&Dump", "^d", main_menu_cb, (void*)"D");
    _mainwin_menubar->add("&Debug/Stop", "^s", main_menu_cb, (void*)"d-");
    _mainwin_menubar->add("&Debug/Clear", "^c", main_menu_cb, (void*)"d_");
    _mainwin_menubar->add("&Debug/Sho&w", "^w", main_menu_cb, (void*)"d+");
    ///
#define Rps_FLTK_debug_option(Dbgopt) add_menu_item_for_debug_option(RPS_DEBUG_##Dbgopt);
    RPS_DEBUG_OPTIONS(Rps_FLTK_debug_option);
#undef Rps_FLTK_debug_option
  };
  /////////////
#if FL_API_VERSION >= 10400
  _mainwin_vflex = new Fl_Flex(0, 25, w(), h()-26);
#endif
  {
    Fl_Widget*firstlabel = nullptr;
    char*labelstr=nullptr;
    char labelbuf[80];
    memset(labelbuf, 0, sizeof(labelbuf));
    /// in FLTK the @ is a magic escaping char, see www.fltk.org/doc-1.4/common.html#common_labels
    snprintf(labelbuf, sizeof(labelbuf), "refpersys-%s/p%d@@%s",
             rps_shortgitid, (int)getpid(), rps_hostname());
    RPS_POSSIBLE_BREAKPOINT();
    labelstr = strdup(labelbuf);
    RPS_DEBUG_LOG(REPL, "fill_main_window labelstr:" << labelstr);
#if FL_API_VERSION >= 10400
    _mainwin_vflex->spacing(2);
    _mainwin_vflex->begin();
    firstlabel = new Fl_Box(0,0,0,0,labelstr);
    _mainwin_vflex->end();
    _mainwin_vflex->layout();
#endif
  }
  this->end();
  callback(close_cb, nullptr);
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::fill_main_window done w="
                << w() << ",h=" << h()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow::fill_main_window-"));
} // end Rps_FltkMainWindow::fill_main_window

Rps_FltkMainWindow::~Rps_FltkMainWindow()
{
  RPS_POSSIBLE_BREAKPOINT();
  size_t nbstring = _mainwin_stringvect.size();
  _mainwin_stringvect.clear();
  size_t nbcstr = _mainwin_cstrvect.size();
  for (int i=0; i<(int)nbcstr; i++)
    {
      RPS_ASSERT(_mainwin_cstrvect[i]);
      _mainwin_cstrvect[i][0] = (char)0;
      free (_mainwin_cstrvect[i]);
      _mainwin_cstrvect[i] = nullptr;
    };
  _mainwin_cstrvect.clear();
  RPS_DEBUG_LOG(REPL, "~Rps_FltkMainWindow @" << (void*)this
                << " nbstring=" << nbstring << " nbcstr=" << nbcstr
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "~Rps_FltkMainWindow"));
  RPS_POSSIBLE_BREAKPOINT();
}; // end Rps_FltkMainWindow::~Rps_FltkMainWindow

void
Rps_FltkMainWindow::main_menu_cb(Fl_Widget*w, void*data)
{
  /// the widget w is the menu bar
  RPS_ASSERT((void*)rps_fltk_mainwin);
  RPS_ASSERT(rps_fltk_mainwin->_mainwin_menubar != nullptr);
  RPS_ASSERT((void*)w == (void*)rps_fltk_mainwin->_mainwin_menubar);
  int logicalsize = rps_fltk_mainwin->_mainwin_menubar->size();
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::main_menu_cb w@" << (void*)w
                << " _mainwin_menubar@" << (void*)rps_fltk_mainwin->_mainwin_menubar
                << " of type:" << (unsigned)(w->type())
                << " data@" << data << "=" << (const char*)data
                << " logicalsize=" << logicalsize
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow::main_menu_cb"));
  RPS_ASSERT(rps_is_main_thread());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/rps_curthread_callframe, //
                 /** locals **/
                 Rps_Value v;
                );
  if (!strcmp((const char*)data, "X")) // eXit
    {
      RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::main_menu_cb eXit");
      rps_dump_into(".", &_);
      rps_fltk_stop();
    }
  else if (!strcmp((const char*)data, "Q")) // Quit
    {
      RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::main_menu_cb Quit");
      rps_fltk_stop();
    }
  else if (!strcmp((const char*)data, "D")) // Dump
    {
      RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::main_menu_cb Dump");
      rps_dump_into(".", &_);
    }
  else if (!strcmp((const char*)data, "d-")) // debug stop
    {
#warning unimplemented debug stop
      RPS_WARNOUT("unimplemented debug stop rps_fltk_debugwin@"
                  << (void*)rps_fltk_debugwin
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "main_menu_cb debug stop"));
    }
  else if (!strcmp((const char*)data, "d+")) // debug show
    {
#warning unimplemented debug show
      RPS_DEBUG_LOG(REPL, "main_menu_cb debug show rps_fltk_debugwin@"
                    << (void*)rps_fltk_debugwin);
      if (!rps_fltk_debugwin)
        rps_fltk_debugwin = new Rps_FltkDebugWindow(670,480);
      rps_fltk_debugwin->show();
      RPS_WARNOUT("unimplemented debug show rps_fltk_debugwin@" << (void*)rps_fltk_debugwin
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "main_menu_cb debug show"));
    }
  else if (!strcmp((const char*)data, "d_")) // debug clear
    {
#warning unimplemented debug clear
      RPS_WARNOUT("unimplemented debug clear rps_fltk_debugwin@"
                  << (void*)rps_fltk_debugwin << " data=" << (const char*)data);
    }
  else if (!strncmp((const char*)data, "d:", 2)
           && isalpha(((const char*)data)[2])) // debug set
    {
      Rps_Debug dbgopt = rps_debug_of_string(std::string{(const char*)data+2});
      if (dbgopt > RPS_DEBUG__NONE)
        {
#warning unimplemented main_menu_cb to set or clear debug option
        }
    }
  else
    {
#warning unimplemented main_menu_cb
      RPS_WARNOUT("unimplemented main_menu_cb " << (const char*)data
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow::main_menu_cb strange"));
    }
} // end Rps_FltkMainWindow::main_menu_cb

void
Rps_FltkMainWindow::close_cb(Fl_Widget*wid, void*data)
{
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow::close_cb wid@" << (void*)wid
                << " data@" << data
                << " elapsed:" << rps_elapsed_real_time()
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow::close_cb"));
  if (RPS_DEBUG_ENABLED(REPL))
    {
      char pmapcmd[128];
      memset(pmapcmd, 0, sizeof(pmapcmd));
      snprintf(pmapcmd, sizeof(pmapcmd), "/usr/bin/pmap %d; /usr/bin/ps -lw %d",
               (int)getpid(), (int)getpid());
      fflush (nullptr);
      std::clog << std::flush;
      std::cerr << std::flush;
      std::cout << std::flush;
      if (system(pmapcmd))
        RPS_FATALOUT("failed to run " << pmapcmd);
    };
  rps_fltk_mainwin->_mainwin_closing = true;
  rps_do_stop_event_loop();
} // end Rps_FltkMainWindow::close_cb
#warning incomplete implementation of class Rps_FltkMainWindow


////////////////////////////////////////////////////////////////
//////// Debug window implementation
Rps_FltkDebugWindow::Rps_FltkDebugWindow(int xx, int yy, int ww, int hh) :
  Fl_Window(xx,yy,
            (ww>min_width)?ww:min_width, (hh>min_height)?hh:min_height,
            (snprintf((char*)memset(_dbgwin_labuf, 0, sizeof(_dbgwin_labuf)), sizeof(_dbgwin_labuf),
                      "refpersys-debug %s p%d@%s",
                      rps_shortgitid, (int)getpid(), rps_hostname()),
             _dbgwin_labuf)),
  _dbgwin_menubar(nullptr),
  _dbgwin_tile(nullptr),
  _dbgwin_text_buffer(nullptr),
  _dbgwin_top_text_display(nullptr),
  _dbgwin_bottom_text_display(nullptr)
{
  RPS_ASSERT(rps_fltk_debugwin == nullptr);
  rps_fltk_debugwin = this;
  RPS_DEBUG_LOG(REPL, "Rps_FltkDebugWindow@" << (void*)this
                << " x=" << x() << ",y=" << y() << ",w=" << w() << "h=" << h() << " lab:" << _dbgwin_labuf);
  fill_debug_window();
} // end Rps_FltkDebugWindow::Rps_FltkDebugWindow

//////// Debug window implementation
Rps_FltkDebugWindow::Rps_FltkDebugWindow(int ww, int hh)
  : Rps_FltkDebugWindow(17 + (Rps_Random::random_32u() % 32)*12,
                        18 + (Rps_Random::random_32u() % 32)*12, ww, hh)
{
  RPS_DEBUG_LOG(REPL,  "Rps_FltkDebugWindow@" << (void*)this
                << " x=" << x() << ",y=" << y() << ",w=" << w()
                << "h=" << h() << " lab:" << _dbgwin_labuf
                << " rps_fltk_debugwin=" << (void*)rps_fltk_debugwin);
} // end Rps_FltkDebugWindow::Rps_FltkDebugWindow

void
Rps_FltkDebugWindow::fill_debug_window(void)
{
  RPS_DEBUG_LOG(REPL, "Rps_FltkDebugWindow::fill_debug_window @"
                << (void*)this);
  begin();
  _dbgwin_menubar = new Fl_Menu_Bar(1, 1, w(), 25);
  RPS_DEBUG_LOG(REPL, "Rps_FltkkDebugWindow::fill_debug_window _dbgwin_menubar@" << _dbgwin_menubar);
  _dbgwin_menubar->add("Clear", "^c", Rps_FltkDebugWindow::debug_menu_cb, (void*)"d_");
  _dbgwin_tile = new Fl_Tile(1, _dbgwin_menubar->y()+2,  w(), h()-(_dbgwin_menubar->h()+3));
  constexpr int debug_text_initial_size=4096;
  constexpr int debug_text_gap_size=2000;
  _dbgwin_text_buffer = new  Fl_Text_Buffer(debug_text_initial_size, debug_text_gap_size);
  {
    char dbgbuf[128];
    memset(dbgbuf, 0, sizeof(dbgbuf));
    snprintf(dbgbuf, sizeof(dbgbuf), "RefPerSys p%d@%s debug\n", (int)getpid(), rps_hostname());
    _dbgwin_text_buffer->append(dbgbuf);
  }
  _dbgwin_tile->begin();
  // create _dbgwin_top_text_display
  _dbgwin_top_text_display = new Fl_Text_Display(1, 1,  _dbgwin_tile->w()-2, _dbgwin_tile->h()/2, "top");
  _dbgwin_top_text_display->buffer(_dbgwin_text_buffer);
  _dbgwin_top_text_display->show();
  // create _dbgwin_bottom_text_display
  _dbgwin_bottom_text_display = new Fl_Text_Display(1, _dbgwin_tile->h()/2,  _dbgwin_tile->w()-2, _dbgwin_tile->h()/2, "bottom");
  _dbgwin_bottom_text_display->buffer(_dbgwin_text_buffer);
  _dbgwin_bottom_text_display->show();
  _dbgwin_tile->end();
  _dbgwin_tile->show();
  /// TODO fixme: when the separator between top & bottom text display is moved, the entire tile should be redrawn...
  end();
#warning unimplemented Rps_FltkDebugWindow::fill_debug_window
  RPS_WARNOUT("unimplemented FltkDebugWindow::fill_debug_window" << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "FltkDebugWindow::fill_debug_window"));
} // end Rps_FltkDebugWindow::fill_debug_window

void
Rps_FltkDebugWindow::debug_menu_cb(Fl_Widget*w, void*data)
{
  /// the widget w is the menu bar
  RPS_ASSERT((void*)rps_fltk_debugwin);
  RPS_ASSERT(rps_fltk_debugwin->_dbgwin_menubar != nullptr);
  RPS_ASSERT((void*)w == (void*)rps_fltk_debugwin->_dbgwin_menubar);
  RPS_DEBUG_LOG(REPL, "Rps_FltkDebugWindow::debug_menu_cb w@" << (void*)w
                << " data@" << data << "=" << (const char*)data);
  RPS_WARNOUT("unimplemented FltkDebugWindow::debug_menu_cb" << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "FltkDebugWindow::debug_menu_cb"));
#warning unimplemented Rps_FltkDebugWindow::debug_menu_cb
} // end Rps_FltkDebugWindow::debug_menu_cb

Rps_FltkDebugWindow::~Rps_FltkDebugWindow()
{
  RPS_ASSERT(rps_fltk_debugwin == this);
  RPS_DEBUG_LOG(REPL, "Rps_FltkDebugWindow destroyed@" << (void*)this);
  rps_fltk_debugwin = nullptr;
} // end Rps_FltkDebugWindow destructor
////////////////
#undef rps_fltk_get_abi_version
int
rps_fltk_get_abi_version (void)
{
  return Fl::abi_version();
} // end rps_fltk_get_abi_version

#undef rps_fltk_get_api_version
int
rps_fltk_get_api_version (void)
{
  return Fl::api_version ();
} // end rps_fltk_get_api_version

void
rps_fltk_progoption(char*arg, struct argp_state*state, bool side_effect)
{
  RPS_DEBUG_LOG(PROGARG, "rps_fltk_progoption arg:" << arg
                << " next:"
                << (side_effect?state->next:-1)
                << " arg_num:"
                << (side_effect?state->arg_num:-1)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_progoption"));
  /// testfltk2 make target uses REPL debugging
  RPS_DEBUG_LOG(REPL, "rps_fltk_progoption arg:" << arg
                << " next:"
                << (side_effect?state->next:-1)
                << " arg_num:"
                << (side_effect?state->arg_num:-1)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_progoption"));
  if (side_effect)
    {
      /* see https://www.fltk.org/doc-1.4/classFl.html#a1576b8c9ca3e900daaa5c36ca0e7ae48 */
      int nw = Fl::arg(state->argc, state->argv, state->next);
      RPS_DEBUG_LOG(PROGARG, "rps_fltk_progoption nw:" << nw
                    <<  " next#" << state->next
                    <<  " argnum#" << state->arg_num);
      RPS_DEBUG_LOG(REPL, "rps_fltk_progoption nw:" << nw
                    <<  " next#" << state->next
                    <<  " argnum#" << state->arg_num
                    << " state.progargs::"
                    << RPS_OUT_PROGARGS(state->argc, state->argv)
                    << " state.argnum:" << state->arg_num << " state.next:"
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_progoption/a"));
    };
#warning missing code in rps_fltk_progoption
  if (arg)
    {
      RPS_WARNOUT("unimplemented rps_fltk_progoption arg=" <<  Rps_Cjson_String(arg)
                  << "' side_effect=" << (side_effect?"True":"False")
                  << " thread:" << rps_current_pthread_name() << std::endl
                  << " state.progargs::"
                  << RPS_OUT_PROGARGS(state->argc, state->argv)
                  << " state.argnum:" << state->arg_num
                  << " state.next:" << state->next
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_progoption/a"));
    }
  else
    RPS_WARNOUT("unimplemented rps_fltk_progoption noarg side_effect="
                << (side_effect?"True":"False")
                << " thread:" << rps_current_pthread_name() << std::endl
                << " state.progargs:"
                << RPS_OUT_PROGARGS(state->argc, state->argv)
                << " argnum:" << state->arg_num
                << " state.next:" << state->next
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_progoption/b"));
  rps_fltk_is_initialized = true;
} // end rps_fltk_progoption

bool
rps_fltk_enabled (void)
{
  return rps_fltk_is_initialized;
} // end rps_fltk_enabled


// flush the pending X11 or wayland requests
void
rps_fltk_flush (void)
{
  Fl::flush();
} // end rps_fltk_flush

void
rps_fltk_initialize (int argc, char**argv)
{
  char titlebuf[128];
  memset (titlebuf, 0, sizeof(titlebuf));
  if (!rps_run_name.empty())
    snprintf(titlebuf, sizeof(titlebuf),
             "RefPerSys/%.20s %.9s v%d.%d pid %d on %s",
             rps_run_name.c_str(),
             rps_shortgitid,
             rps_get_major_version(), rps_get_minor_version(),
             (int)getpid(),
             rps_hostname());
  else
    snprintf(titlebuf, sizeof(titlebuf),
             "RefPerSys %.9s v%d.%d pid %d on %s",
             rps_shortgitid,
             rps_get_major_version(), rps_get_minor_version(),
             (int)getpid(),
             rps_hostname());

  fl_open_display();
  rps_fltk_mainwin =
    new Rps_FltkMainWindow(/*width=*/770, /*height=*/550,
                                     titlebuf);
  rps_fltk_mainwin->show(argc, argv);
  rps_fltk_flush ();
  RPS_DEBUG_LOG(REPL, "rps_fltk_initialize showing mainwin@"
                << (void*)rps_fltk_mainwin
                << " DISPLAY=" << Rps_Cjson_String(getenv("DISPLAY"))
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize"));
  RPS_WARNOUT("incomplete rps_fltk_initialize " << titlebuf << std::endl
              << " thread:" << rps_current_pthread_name()
              << " progargs " << RPS_OUT_PROGARGS(argc, argv)
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize"));
} // end rps_fltk_initialize

extern "C" bool rps_fltk_program_is_quitting(void);
bool
rps_fltk_program_is_quitting(void)
{
#ifdef FLTK_API_VERSION >= 10400 ///FLTK 1.4
  RPS_DEBUG_LOG(REPL, "rps_fltk_program_is_quitting program_should_quit="
                << (Fl::program_should_quit())?"True":"false"
                << std::endl
                << " thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_program_is_quitting"));
  return Fl::program_should_quit();
#else
  if (!rps_fltk_mainwin)
    {
      RPS_DEBUG_LOG(REPL, "rps_fltk_program_is_quitting no mainwin"
                    << std::endl
                    << " thread:" << rps_current_pthread_name()
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_program_is_quitting"));
      return true;
    }
  RPS_DEBUG_LOG(REPL, "rps_fltk_program_is_quitting mainwin "
                << ((rps_fltk_mainwin->is_closing())?"closing":"not-closing")
                << std::endl
                << " thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_program_is_quitting"));
  return rps_fltk_mainwin->is_closing();
#endif  // FLTK 1.3
} // end rps_fltk_program_is_quitting


void
rps_fltk_stop(void)
{
  RPS_DEBUG_LOG(REPL, "rps_fltk_stop thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_stop"));
  Fl::lock();
#if FLTK_API_VERSION >= 10400 // FLTK 1.4
  /// in FLTK1.4 only Fl::program_should_quit(1);
  Fl::program_should_quit(1);
#else // FLTKI 1.3
  RPS_WARNOUT("rps_fltk_stop called from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_stop")
              << std::endl << " should quit FLTK event loop");
#endif
  Fl::unlock();
  RPS_DEBUG_LOG(REPL, "rps_fltk_stop done from thread:" << rps_current_pthread_name());
} // end rps_fltk_stop


void
rps_fltk_run (void)
{
  long loopcnt = 0;
  RPS_DEBUG_LOG(REPL, "rps_fltk_run thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_run"));
  RPS_ASSERT(rps_is_main_thread());
  constexpr double minimal_wait_delay = 1.25;;
  constexpr double plain_wait_delay = 16.0;
  if (rps_run_delay > 0.0)
    {
      double finalrealtime = rps_elapsed_real_time()+rps_run_delay;
      double waitdelay = rps_run_delay/16.0+0.02;
      if (waitdelay < minimal_wait_delay)
        waitdelay = minimal_wait_delay;
      RPS_DEBUG_LOG(REPL, "rps_fltk_run thread:" << rps_current_pthread_name() << " waitdelay=" << waitdelay);
      while (!rps_fltk_program_is_quitting())
        {
          loopcnt++;
          RPS_DEBUG_LOG(REPL, "rps_fltk_run thread:" << rps_current_pthread_name()
                        << " loopcnt#" << loopcnt //
                        << " elapsedrealtime:" << rps_elapsed_real_time() //
                        << " processcputime:" << rps_process_cpu_time());
          Fl::wait(waitdelay);
          if (rps_elapsed_real_time() > finalrealtime)
            {
              RPS_DEBUG_LOG(REPL, "rps_fltk_run thread:" << rps_current_pthread_name()
                            << " loopcnt#" << loopcnt
                            << " quit after "
                            << rps_run_delay << " elapsed sec."
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_run/stop"));
              // TODO: code review for FLTK1.3
              rps_fltk_stop();
              break;
            }
        };
    }
  else   // no rps_run_delay
    {
      while (!rps_fltk_program_is_quitting())
        {
          loopcnt++;
          RPS_DEBUG_LOG(REPL, "rps_fltk_run thread:" << rps_current_pthread_name()
                        << " loopcnt#" << loopcnt //
                        << " elapsedrealtime:" << rps_elapsed_real_time() //
                        << " processcputime:" << rps_process_cpu_time());
          Fl::wait(plain_wait_delay);
        };
      RPS_DEBUG_LOG(REPL, "rps_fltk_run thread:" << rps_current_pthread_name()
                    << " loopcnt#" << loopcnt
                    << " quit per request");
    }
  RPS_DEBUG_LOG(REPL, "rps_fltk_run ended thread:"
                << rps_current_pthread_name()
                << std::endl
                << " final loopcnt:" << loopcnt//
                << " elapsedrealtime:" << rps_elapsed_real_time() //
                << " processcputime:" << rps_process_cpu_time() //
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_run/end"));
} // end rps_fltk_run


void
rps_fltk_emit_sizes(std::ostream&out)
{
  out << std::endl
      << "/// FLTK related sizes and alignments from " << __FUNCTION__
      << std::endl;
  out << "/// for FLTK " << Fl::api_version() << std::endl;
  out << "#define RPS_FLTK_SIZEOF_FL_WINDOW " << sizeof(Fl_Window) << std::endl;
  out << "#define RPS_FLTK_SIZEOF_MAIN_WINDOW " << sizeof(Rps_FltkMainWindow) << std::endl;
  out << "#define RPS_FLTK_SIZEOF_FL_WIDGET " << sizeof(Fl_Widget) << std::endl;
  out << "#define RPS_FLTK_SIZEOF_FL_BUTTON " << sizeof(FL_Button) << std::endl;
  out << "#define RPS_FLTK_SIZEOF_FL_MENU_BAR " << sizeof(Fl_Menu_Bar) << std::endl;
  out << "#define RPS_FLTK_SIZEOF_FL_TEXT_BUFFER " << sizeof(Fl_Text_Buffer) << std::endl;
  out << "#define RPS_FLTK_SIZEOF_FL_TEXT_EDITOR " << sizeof(Fl_Text_Editor) << std::endl;
  out << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_FL_WINDOW " << alignof(Fl_Window) << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_MAIN_WINDOW " << alignof(Rps_FltkMainWindow) << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_FL_WIDGET " << alignof(Fl_Widget) << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_FL_BUTTON " << alignof(FL_Button) << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_FL_MENU_BAR " << alignof(Fl_Menu_Bar) << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_FL_TEXT_BUFFER " << alignof(Fl_Text_Buffer) << std::endl;
  out << "#define RPS_FLTK_ALIGNOF_FL_TEXT_EDITOR " << alignof(Fl_Text_Editor) << std::endl;
  out << "//// end of FLTK sizes and alignments for api "
      << Fl::api_version() << " abi " << Fl::abi_version() << std::endl;
} // end rps_fltk_emit_sizes

void
rps_fltk_printf_inform_message(const char*file, int line, const char*funcname, long count, const char*fmt, ...)
{
  va_list args;
  char*msg = nullptr;
  char buf[512];
  memset(buf, 0, sizeof(buf));
  va_start (args, fmt);
  int l = vsnprintf(buf, sizeof(buf), fmt, args);
  if (l>=(int)sizeof(buf)-1)
    {
      l=((l+7)|0xf)+1;
      msg = (char*)malloc(l);
      if (msg == nullptr)
        RPS_FATAL("rps_fltk_printf_inform_message [%s:%d:%s](#%ld) <%s> failed to malloc %d bytes",
                  file, line, funcname, count, fmt, l);
      memset (msg, 0, l);
      (void)  vsnprintf(msg, l, fmt, args);
      va_end (args);
    }
  else
    msg = buf;
  rps_fltk_show_debug_message(file,line,funcname,
                              (Rps_Debug)RPS_INFORM_MSG_LEVEL,
                              count,msg);
  if (msg != buf)
    free(msg);
} // end rps_fltk_printf_inform_message


void
rps_fltk_show_debug_message(const char*file, int line, const char*funcname,
                            Rps_Debug dbgopt, long dbgcount, const char*msg)
{
  RPS_ASSERT(file != nullptr);
  RPS_ASSERT(line != 0);
  RPS_ASSERT((int)dbgopt != 0);
  RPS_ASSERT(dbgcount>0);
  RPS_ASSERT(msg != nullptr);
#warning incomplete rps_fltk_show_debug_message
} // end rps_fltk_show_debug_message

//// end of file fltk_rps.cc

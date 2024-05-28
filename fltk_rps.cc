/****************************************************************
 * file fltk_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the FLTK 1.4 graphical interface.  See
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

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Box.H>

/// conventional strings
extern "C" const char rps_fltk_gitid[];
const char rps_fltk_gitid[]= RPS_GITID;

extern "C" const char rps_fltk_date[];
const char rps_fltk_date[]= __DATE__;

////////////////////////////////////////////////////////////////////////
////// ******** DECLARATIONS ********

class Rps_PayloadFltkThing;
class Rps_PayloadFltkWidget;
class Rps_PayloadFltkRefWidget;
class Rps_PayloadFltkWindow;
class Rps_FltkMainWindow;

extern "C" Rps_FltkMainWindow* rps_fltk_mainwin;
Rps_FltkMainWindow* rps_fltk_mainwin;

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
class Rps_PayloadFltkRefWidget : public Rps_PayloadFltkThing, Fl_Callback_User_Data
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
      wid->user_data(this, /*auto_free=*/true);
    }
} // end Rps_PayloadFltkRefWidget::Rps_PayloadFltkRefWidget


////////////////
class Rps_FltkMainWindow: public Fl_Window
{
public:
  Rps_FltkMainWindow(int x, int y, int w, int h, const char*title);
  Rps_FltkMainWindow(int w, int h, const char*title);
  virtual ~Rps_FltkMainWindow();
};        // end Rps_FltkMainWindow;








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
  : Fl_Window(x,y,w,h,title)
{
  RPS_DEBUG_LOG(REPL, "Rps_FltkMainWindow x=" << x << ",y=" << y
                << ",w=" << w << ",h=" << h
                << ",title=" << Rps_Cjson_String(title)
                << " @" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_FltkMainWindow/xywh"));
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

Rps_FltkMainWindow::~Rps_FltkMainWindow()
{
  RPS_DEBUG_LOG(REPL, "~Rps_FltkMainWindow @" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "~Rps_FltkMainWindow"));
}; // end Rps_FltkMainWindow::~Rps_FltkMainWindow



#warning incomplete implementation of class Rps_FltkMainWindow



////////////////
int
rps_fltk_abi_version (void)
{
  return Fl::abi_version();
} // end rps_fltk_abi_version

int
rps_fltk_api_version (void)
{
  return Fl::api_version ();
} // end rps_fltk_api_version

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
  if (side_effect)
    {
      /* see https://www.fltk.org/doc-1.4/classFl.html#a1576b8c9ca3e900daaa5c36ca0e7ae48 */
      int nw = Fl::arg(state->argc, state->argv, state->next);
      RPS_DEBUG_LOG(PROGARG, "rps_fltk_progoption nw:" << nw
                    <<  " next#" << state->next
                    <<  " argnum#" << state->arg_num);
    };
#warning missing code in rps_fltk_progoption
  if (arg)
    {
      RPS_WARNOUT("unimplemented rps_fltk_progoption arg=" <<  Rps_Cjson_String(arg)
                  << "' side_effect=" << (side_effect?"True":"False")
                  << " thread:" << rps_current_pthread_name() << std::endl
                  << " state.progargs::" << Rps_Do_Output([&](std::ostream&out)
      {
        rps_output_program_arguments(out, state->argc, state->argv);
      }) << " state.argnum:" << state->arg_num << " state.next:" << state->next
         << std::endl
         << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_progoption/a"));
    }
  else
    RPS_WARNOUT("unimplemented rps_fltk_progoption noarg side_effect="
                << (side_effect?"True":"False")
                << " thread:" << rps_current_pthread_name() << std::endl
                << " state.progargs:" << Rps_Do_Output([&](std::ostream&out)
    {
      rps_output_program_arguments(out, state->argc, state->argv);
    }) << " argnum:" << state->arg_num << " state.next:" << state->next
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
#warning missing code in rps_fltk_initialize to create FLTK windows
  char titlebuf[128];
  memset (titlebuf, 0, sizeof(titlebuf));
  snprintf(titlebuf, sizeof(titlebuf), "RefPerSys v%d.%d pid %d on %s",
           rps_get_major_version(), rps_get_minor_version(), (int)getpid(),
           rps_hostname());
  fl_open_display();
  rps_fltk_mainwin = new Rps_FltkMainWindow(/*width=*/500, /*height=*/300,
      titlebuf);
  rps_fltk_mainwin->show();
  rps_fltk_flush ();
  RPS_DEBUG_LOG(REPL, "rps_fltk_initialize showing mainwin@"
                << (void*)rps_fltk_mainwin
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize"));
  RPS_WARNOUT("unimplemented rps_fltk_initialize " << titlebuf << std::endl
              << " thread:" << rps_current_pthread_name()
              << " progargs "
              <<  Rps_Do_Output([&](std::ostream&out)
  {
    rps_output_program_arguments(out, argc, argv);
  })
      << std::endl
      << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_initialize"));
} // end rps_fltk_initialize

void
rps_fltk_stop(void)
{
  RPS_DEBUG_LOG(REPL, "rps_fltk_stop thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_fltk_stop"));
  Fl::lock();
  Fl::program_should_quit(1);
  Fl::unlock();
} // end rps_fltk_stop

//// end of file fltk_rps.cc

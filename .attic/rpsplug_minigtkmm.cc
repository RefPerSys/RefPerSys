// file RefPerSys/plugins/rpsplug_minigtkmm.cc
// SPDX-License-Identifier: GPL-3.0-or-later
// see http://refpersys.org/
// GPLv3+ licensed
// © Copyright 2024 Basile Starynkevitch <basile@starynkevitch.net>

/***
 *    This plugin contains a small GTKmm4 graphical interface

    Once compiled use it as

  ./refpersys --plugin-after-load=rpsplug_minigtkmm.so:$DISPLAY \
              --plugin-arg=rpsplug_minigtkmm: \
 ***/

#include "refpersys.hh"

//@@PKGCONFIG gtkmm-4.0
#include <gtkmm.h>

extern "C" Gtk::Window* rpsminigtk_mainwin;

extern "C" int rpsminigtk_prepoller(struct pollfd*, int npoll, Rps_CallFrame*);
void
rps_do_plugin(const Rps_Plugin*plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef ob;
                           Rps_Value v1;
                );
  errno = 0;
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg || !plugarg[0])
    plugarg=":0.0";
  RPS_DEBUG_LOG(REPL, "minigtkmm plugin argument" << plugarg);
  rpsminigtk_mainwin = new Gtk::Window;
  
} // end rps_do_plugin

int
rpsminigtk_prepoller(struct pollfd*pollarr, int npoll, Rps_CallFrame* cf)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/cf,
		 );
  RPS_DEBUG_LOG(REPL, "rpsminigtk_prepoller npoll=" << npoll
		<< " pollarr@" << (void*)pollarr);
  return 0;
} // end rpsminigtk_prepoller

Gtk::Window* rpsminigtk_mainwin;

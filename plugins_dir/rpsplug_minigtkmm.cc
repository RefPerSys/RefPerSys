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
  static int plugargc=0;
  static char* plugargv[4];
  new Gtk::Main(&plugargc, plugargv);
  RPS_DEBUG_LOG(REPL, "minigtkmm plugin argument" << plugarg);
} // end rps_do_plugin

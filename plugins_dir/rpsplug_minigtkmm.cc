// file RefPerSys/plugins/rpsplug_minifltk.cc
// SPDX-License-Identifier: GPL-3.0-or-later
// see http://refpersys.org/
// GPLv3+ licensed
// © Copyright 2024 Basile Starynkevitch <basile@starynkevitch.net>

/***
 *    This plugin contains a small FLTK 1.4 graphical interface

    Once compiled use it as

  ./refpersys --plugin-after-load=rpsplug_minigtkmm.so:$DISPLAY \
              --plugin-arg=rpsplug_minigtkmm: \
 ***/
//@PKGCONFIG gtkmm-3.0

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin*plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef ob;
                           Rps_Value v1;
                );
  errno = 0;
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
} // end rps_do_plugin

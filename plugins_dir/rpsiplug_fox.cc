// file RefPerSys/plugins_dir/rpsiplug_fox.cc
// SPDX-License-Identifier: GPL-3.0-or-later

/***
    © Copyright (C) 2026 by Basile STARYNKEVITCH, France
   program released under GNU General Public License v3+

   This is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
***/

#pragma message "compiling " __FILE__ " at " __DATE__ "@" __TIME__

#include "refpersys.hh"

#include "fx.h"

#include "fxver.h"

class RpsFxMainWindow : public FXMainWindow
{
  FXDECLARE(RpsFxMainWindow);
};        // end RpsFxMainWindow

class RpsFxApp : public FXApp
{
public:
  RpsFxApp(const FXString& name, const FXString&vendor);
  ~RpsFxApp();
};        // end class RpsFxApp

static RpsFxApp* foxapp;

RpsFxApp::RpsFxApp(const FXString& name, const FXString&vendor)
  : FXApp(name,vendor)
{
  RPS_DEBUG_LOG(EVLOOP, "RpsFxApp constr name=" << name.text()
                << " vendor=" << vendor.text()
                << " this@" << rps_unsigned_hex_string((uintptr_t)(this))
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsFxApp constr"));
  if (!RPS_DEBUG_ENABLED(EVLOOP))
    RPS_DEBUG_LOG(REPL, "RpsFxApp constr name=" << name.text()
                  << " vendor=" << vendor.text()
                  << " this@" << rps_unsigned_hex_string((uintptr_t)(this)));
} // end RpsFxApp constructor

RpsFxApp::~RpsFxApp()
{
  RPS_DEBUG_LOG(EVLOOP, "RpsFxApp destr"
                << " this@" << rps_unsigned_hex_string((uintptr_t)(this))
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsFxApp destr"));
  if (!RPS_DEBUG_ENABLED(EVLOOP))
    RPS_DEBUG_LOG(REPL, "RpsFxApp destr"
                  << " this@"
                  << rps_unsigned_hex_string((uintptr_t)(this)));
} // end RpsFxApp destructor

static void
rpsfox_delete_app(void)
{
  RPS_DEBUG_LOG(EVLOOP, "foxapp@"
                << rps_unsigned_hex_string((uintptr_t)foxapp)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"rpsfox_delete_app"));
  if (!RPS_DEBUG_ENABLED(EVLOOP))
    RPS_DEBUG_LOG(REPL, "foxapp@"
                  << rps_unsigned_hex_string((uintptr_t)foxapp)
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,"rpsfox_delete_app"));
  if (foxapp)
    delete foxapp;
} // end rpsfox_delete_app


FXDEFMAP(RpsFxMainWindow) RpsFxMainWindowMap[] =
{
};

FXIMPLEMENT(RpsFxMainWindow,FXMainWindow,RpsFxMainWindowMap,
            ARRAYNUMBER(RpsFxMainWindowMap));

extern "C" void
rps_do_interactive_plugin(const char*arg)
{
  ///
  if (FOX_MAJOR != fxversion[0] && FOX_MINOR != fxversion[1])
    RPS_FATALOUT("incompatible FOX-toolkit versions" <<std::endl
                 << " fox compiled version:" << FOX_MAJOR << "."
                 << FOX_MINOR << "." << FOX_LEVEL
                 << " linked " << fxversion[0] << "." << fxversion[1]
                 << "." << fxversion[2]);
  foxapp = new RpsFxApp("fox-refpersys-plugin-app", "refpersys.org");
  RPS_ASSERT(foxapp != nullptr);
  foxapp->init(rps_main_argc, (char**)rps_main_argv);
  if (!RPS_DEBUG_ENABLED(EVLOOP))
    RPS_DEBUG_LOG(REPL, "foxapp @"
                  << rps_unsigned_hex_string((uintptr_t)foxapp)
                  << " initialized");
  else
    RPS_DEBUG_LOG(EVLOOP, "foxapp @"
                  << rps_unsigned_hex_string((uintptr_t)foxapp)
                  << " initialized");
  rps_atexit(rpsfox_delete_app);
  RPS_WARNOUT("incomplete fox rps_do_interactive_plugin arg="
              << Rps_QuotedC_String(arg)
              << " fox compiled version:" << FOX_MAJOR << "."
              << FOX_MINOR << "." << FOX_LEVEL
              << " linked " << (int)(fxversion[0])
              << "." << (int)(fxversion[1])
              << "." << (int)(fxversion[2])
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1,"fox rps_do_interactive_plugin"));
#warning incomplete fox rps_do_interactive_plugin
} // end rps_do_interactive_plugin

#pragma message "done compiling " __FILE__ " at " __DATE__ "@" __TIME__
/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && make plugins_dir/rpsiplug_fox.so" ;;
 ** End: ;;
 ****************/


///////////////// end of file RefPerSys/plugins_dir/rpsiplug_fox.cc

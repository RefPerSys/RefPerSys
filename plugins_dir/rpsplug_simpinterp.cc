// file RefPerSys/plugins_dir/rpsplug_simpinterp.cc
// SPDX-License-Identifier: GPL-3.0-or-later
// see http://refpersys.org/
// GPLv3+ licensed
// © Copyright 2024 - 2025 Basile Starynkevitch <basile@starynkevitch.net>

/***
    This plugin contains a simple interpreter. The interpreter's
    syntax -probably inspired by C- has to be defined/approved by
    others. Its role is to provide some way for RefPerSys developers
    to modify the RefPerSys heap and test various modules including
    code generation ones.

    Once compiled use it as

    ./refpersys --plugin-after-load=plugins_dir/rpsplug_simpinterp.so \
    --plugin-arg=rpsplug_simpinterp:$SCRIPT_FILE

    ... and probably other program arguments (e.g. -AREPL)
***/

#include "refpersys.hh"


/// on whatsapp March 14 2024 Abhishek approved a C like syntax; we
/// hope for him to be allowed to informally specify it in Feb 2025
// See https://github.com/RefPerSys/RefPerSys/issues/21

void
rps_do_plugin(const Rps_Plugin*plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef ob;
                           Rps_Value v1;
                );
  _f.ob = nullptr;
  _f.v1 = nullptr;
  errno = 0;
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  RPS_DEBUG_LOG(REPL, "rps_do_plugin " << plugin->plugin_name
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, __FILE__ " plugin"));
  if (!plugarg || !plugarg[0])
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument (script file expected)");
  if (access(plugarg, R_OK))
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " with bad script argument " << plugarg
                 << ":" << strerror(errno));
  {
    struct stat plugstat= {};
    if (stat(plugarg,&plugstat))
      RPS_FATALOUT("failure: plugin "  << plugin->plugin_name
                   << " cannot stat script argument " << plugarg
                   << ":" << strerror(errno));
    if ((plugstat.st_mode & S_IFMT) != S_IFREG)
      {
        RPS_WARNOUT("failure: plugin " <<  plugin->plugin_name
                    << " script argument " << plugarg
                    << " not a regular file" << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "simple interpreter"));
        return;
      };
  }
  Rps_MemoryFileTokenSource toksrc(plugarg);
  RPS_WARNOUT("missing code:  plugin " <<  plugin->plugin_name
              << " script " << plugarg);
#warning a lot of missing code in rpsplug_simpinterp.cc
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_simpinterp.so && /bin/ln -svf $(/bin/pwd)/plugins_dir/rpsplug_simpinterp.so /tmp/" ;;
 ** End: ;;
 ****************/


////////////////////////////// end of file RefPerSys/plugins_dir/rpsplug_simpinterp.cc

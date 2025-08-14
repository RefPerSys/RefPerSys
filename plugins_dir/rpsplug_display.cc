// file RefPerSys/plugins_dir/rpsplug_display.cc
// SPDX-License-Identifier: GPL-3.0-or-later
// see http://refpersys.org/
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>

/***
    This plugin just displays an object. The plugin argument is an
    object name or oid

    Once compiled use it as

    ./refpersys --plugin-after-load=plugins_dir/rpsplug_display.so \
    --plugin-arg=rpsplug_display:<object-name-or-oid>

***/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin*plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef ob;
                );
  _f.ob = nullptr;
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  RPS_DEBUG_LOG(REPL, "rps_do_plugin " << plugin->plugin_name
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, __FILE__ " plugin"));
  if (!plugarg || !plugarg[0])
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument - expecting an object name or oid");
  _f.ob = Rps_ObjectRef::find_object_or_null_by_string(&_, std::string(plugarg));
  if (!_f.ob)
    RPS_WARNOUT("in git " << rps_shortgitid << " " << plugarg << " dont name any object");
  else
    RPS_INFORMOUT("in git " << rps_shortgitid << " object " << plugarg << " is:"
                  << RPS_OBJECT_DISPLAY(_f.ob));
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && ./do_build_refpersys-plugin plugins_dir/rpsplug_display.cc && /bin/ln -svf $(/bin/pwd)/plugins_dir/rpsplug_display.so /tmp/" ;;
 ** End: ;;
 ****************/


///////////////// end of file RefPerSys/plugins_dir/rpsplug_display.cc

// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// with improvement after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 - 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin remove an obsolete root object by clearing its space
/*****
  Once compiled, use it for example as:
  ./refpersys --plugin-after-load=/tmp/rpsplug_removeroot.so \
              --plugin-arg=rpsplug_removeroot:<oid|name> \
              --batch --dump=.

 ****/




#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obroot;
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty name or oid");
  RPS_FATALOUT("unimplemented rpsplug_removeroot arg=" << plugarg);
} // end rps_do_plugin


/****************
 **                           for Emacs...
  Local Variables: ;;
  compile-command: "cd $REFPERSYS_TOPDIR && \
    ./do-build-refpersys-plugin -v        \
      -i plugins_dir/rpsplug_removeroot.cc \
      -o plugins_dir/rpsplug_removeroot.so \
      -L /tmp/rpsplug_removeroot.so" ;;
  End: ;;
 ****************/

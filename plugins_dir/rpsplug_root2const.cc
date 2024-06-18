// see http://refpersys.org/
// passed to commits after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 - 2024 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin replace a root object by a constant rpskob_*
/*****

 Once compiled, use it for example as:

  ./refpersys --plugin-after-load=/tmp/rpsplug_root2const.so \
              --plugin-arg=rpsplug_root2const:oidorname \
              --extra=comment='some comment' \
              --batch --dump=.

 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obsymbol;
		 Rps_ObjectRef obnamedattr;
		 Rps_ObjectRef oboldroot;
		 Rps_Value namestr; // a string
		 Rps_Value commentstr;
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*comment = rps_get_extra_arg("comment");
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some name or root objectid");
  if (isalpha(plugarg[0])) {
  }
  else if (plugarg[0] == '_' && isalnum(plugarg[1])) {
  }
  else
    RPS_FATAL("failure  plugin " << plugin->plugin_name
	      << " with bad argument " << Rps_Cjson_String(plugarg));
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins_dir/rpsplug_root2const.cc /tmp/rpsplug_root2const.so" ;;
 ** End: ;;
 ****************/

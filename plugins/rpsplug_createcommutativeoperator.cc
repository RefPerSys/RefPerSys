// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2022 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin install a commutative REPL operator
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createcommutativeoperator.so --plugin-arg=rpsplug_createcommutativeoperator:<string> --batch --dump=.


#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewroot;
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some string");
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
                << Rps_QuotedC_String(plugarg));
  RPS_FATALOUT("rpsplug_createcommutativeoperator not implemented for "
               <<  Rps_QuotedC_String(plugarg));
#warning unimplemented rpsplug_createcommutativeoperator
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_createcommutativeoperator.cc /tmp/rpsplug_createcommutativeoperator.so" ;;
 ** End: ;;
 ****************/

// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2023 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin installs a commutative REPL operator
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createcommutativeoperator.so \
 *             --plugin-arg=rpsplug_createcommutativeoperator:++ \
 *             --extra=name=plusplus \
 *             --extra=precedence=8 \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obsuperclass,
		 Rps_ObjectRef obnewclass,
		 Rps_Value classname; // a string
		 );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*supername = rps_get_extra_arg("super");
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty name");
  /* TODO: check that supername is given and is naming an existing class. */
  /// FIXME: missing code checking supername and computing obsuperclass. 
  ///
  /* TODO: check that plugarg is a proper class name. */
  /// FIXME: check that plugarg is a proper class name.
  ///
  /* TODO: create the new obnewclass. */
  /* TODO: create a symbol for the new name. */
  /* TODO: put the obnewclass as value of the symbol. */
  RPS_FATALOUT("rpsplug_createclass not implemented for "
               <<  Rps_QuotedC_String(plugarg));
#warning unimplemented rpsplug_createclass
} // end rps_do_plugin

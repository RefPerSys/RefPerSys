// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2023 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin installs a commutative REPL operator
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createclass.so \
 *             --plugin-arg=rpsplug_createclass:new_class_name \
 *             --extra=super=superclass \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obsuperclass;
		 Rps_ObjectRef obnewclass;
		 Rps_Value classname; // a string
		 );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*supername = rps_get_extra_arg("super");
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty name");
  ///
  /* Check that plugarg is a good class name */
  {
    bool goodplugarg = isalpha(plugarg[0]);
    for (const char*pa = &plugarg[0]; goodplugarg && *pa; pa++)
      goodplugarg = isalnum(*pa) || *pa=='_';
  }
  /* Check that plugarg is some new name */
  if (auto nob = Rps_ObjectRef::find_object_or_null_by_string(&_, std::string(plugarg))) {
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name << " argument " << plugarg
		 << " is naming an existing object " << nob);
  };
  /* Check that supername is given and is naming an existing class. */
  if (!supername)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without super extra name. See comments in " << __FILE__);
  {
    bool goodsupername = isalpha(supername[0]);
    for (const char*pn = supername; goodsupername && *pn; pn++)
      goodsupername = isalnum(*pn) || *pn=='_';
    if (!goodsupername)
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name
		   << " with bad superclass name " << Rps_QuotedC_String(supername));
  }
  _f.obsuperclass =  Rps_ObjectRef::find_object_or_null_by_string(&_, std::string(supername));
  if (!_f.obsuperclass) 
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name
		   << " with unknown superclass name " << Rps_QuotedC_String(supername));
  if (!_f.obsuperclass->is_class())
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name
		   << " with super name " << Rps_QuotedC_String(supername)
		   << " not naming a class but " << _f.obsuperclass);
  /* TODO: create the new obnewclass. */
  /* TODO: create a symbol for the new name. */
  /* TODO: put the obnewclass as value of the symbol. */
  RPS_FATALOUT("rpsplug_createclass not implemented for "
               <<  Rps_QuotedC_String(plugarg));
#warning incomplete plugin code rpsplug_createclass
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_createclass.cc /tmp/rpsplug_createclass.so" ;;
 ** End: ;;
 ****************/

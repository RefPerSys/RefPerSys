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
		 Rps_ObjectRef obsymbol;
		 Rps_Value namestr; // a string
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
		   << " not naming a class but the object " << _f.obsuperclass << " of RefPerSys class "
		   << _f.obsuperclass->get_class());
  /* Create the new obnewclass. */
  _f.obnewclass = Rps_ObjectRef::make_named_class(&_, _f.obsuperclass, std::string{plugarg});
  /* Create a symbol for the new class name. */
  _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, std::string{plugarg});
  Rps_PayloadSymbol* paylsymb = _f.obsymbol->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT (paylsymb != nullptr);
  paylsymb->symbol_put_value(_f.obnewclass);
  _f.obnewclass->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), //symbol∈symbol
			  _f.obsymbol);
  _f.namestr = Rps_Value{std::string(plugarg)}
  _f.obnewclass->put_attr(RPS_ROOT_OB(_4FBkYDlynyC02QtkfG), //"name"∈named_attribute
			  _f.namestr);
  RPS_FATALOUT("rpsplug_createclass not implemented for "
               <<  Rps_QuotedC_String(plugarg)
		   << " with super name " << Rps_QuotedC_String(supername));
#warning incomplete plugin code rpsplug_createclass
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_createclass.cc /tmp/rpsplug_createclass.so" ;;
 ** End: ;;
 ****************/

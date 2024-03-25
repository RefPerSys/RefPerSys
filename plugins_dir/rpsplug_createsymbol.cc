// see http://refpersys.org/
// passed to commits after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates a new RefPerSys symbol
/*****
 * Once compiled, use it for example as:

  ./refpersys --plugin-after-load=/tmp/rpsplug_createsymbol.so \
              --plugin-arg=rpsplug_createsymbol:new_symbol_name \
              --extra=comment='some comment' \
              --extra=rooted=0 \
              --batch --dump=.

 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obsymbol;
                           Rps_Value namestr; // a string
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*comment = rps_get_extra_arg("comment");
  const char*rooted = rps_get_extra_arg("rooted");
  bool isrooted = false;
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty name");
  ///
  /* Check that plugarg is a good symbol name */
  {
    bool goodplugarg = isalpha(plugarg[0]);
    for (const char*pa = &plugarg[0]; goodplugarg && *pa; pa++)
      goodplugarg = isalnum(*pa) || *pa=='_';
    if (!goodplugarg)
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                   << " with bad name " << Rps_QuotedC_String(plugarg));
  }
  if (rooted)
    {
      if (!strcmp(rooted, "true")) isrooted = true;
      if (atoi(rooted) > 0) isrooted = true;
    };
  /* Check that plugarg is some new name */
  if (auto nob = Rps_ObjectRef::find_object_or_null_by_string(&_, std::string(plugarg)))
    {
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name << " argument " << plugarg
                   << " is naming an existing object " << nob);
    };
  /* Create a symbol for the new class name. */
  _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, std::string{plugarg});
  std::lock_guard<std::recursive_mutex> gusymbol(*(_f.obsymbol->objmtxptr()));
  Rps_PayloadSymbol* paylsymb = _f.obsymbol->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT (paylsymb != nullptr);
  _f.namestr = Rps_Value{std::string(plugarg)};
  /// avoid using bad _4FBkYDlynyC02QtkfG //"name"∈named_attribute
  _f.obsymbol->put_attr(RPS_ROOT_OB(_EBVGSfW2m200z18rx) //name∈named_attribute
                        _f.namestr);
  if (isrooted)
    {
      rps_add_root_object(_f.obsymbol);
      RPS_INFORMOUT("rpsplug_createsymbol added new root symbol " << _f.obsymbol
                    << " named " << plugarg );
    }
  else
    {
      RPS_INFORMOUT("rpsplug_createsymbol added new symbol " << _f.obsymbol
                    << " named " << plugarg);
    }
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_createsymbol.cc /tmp/rpsplug_createsymbol.so" ;;
 ** End: ;;
 ****************/

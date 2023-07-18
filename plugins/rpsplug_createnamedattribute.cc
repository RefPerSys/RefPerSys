// see http://refpersys.org/
// passed to commits after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates a new RefPerSys named attribute
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createnamedattribute.so \
 *             --plugin-arg=rpsplug_createnamedattribute:new_attr_name \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obsymbol;
                           Rps_ObjectRef obnamedattr;
                           Rps_Value namestr; // a string
                           Rps_Value commentstr;
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
  _f.obsymbol->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
                        _f.namestr);
  _f.obnamedattr =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_4pSwobFHGf301Qgwzh), //named_attribute∈class,
                               Rps_ObjectRef::root_space());
  _f.obnamedattr->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
                           _f.namestr);
  _f.obnamedattr->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), //symbol∈symbol
                           _f.obsymbol);
  if (comment)
    {
      _f.commentstr = Rps_StringValue(comment);
      _f.obnamedattr->put_attr(RPS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol,
                               _f.commentstr);
    }
  paylsymb->symbol_put_value(_f.obnamedattr);
  if (isrooted)
    {
      rps_add_root_object(_f.obnamedattr);
      RPS_INFORMOUT("rpsplug_createnamedattribute added new root named attribute " << _f.obnamedattr
                    << " named " << plugarg
                    << " with symbol " << _f.obsymbol);
    }
  else
    {
      RPS_INFORMOUT("rpsplug_createnamedattribute added new named attribute " << _f.obnamedattr
                    << " named " << plugarg
                    << " with symbol " << _f.obsymbol);
    }
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_createnamedattribute.cc /tmp/rpsplug_createnamedattribute.so" ;;
 ** End: ;;
 ****************/

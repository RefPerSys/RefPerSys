// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// with improvement after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 - 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates a new RefPerSys class
/*****
  Once compiled, use it for example as:
  ./refpersys --plugin-after-load=/tmp/rpsplug_createclass.so \
              --plugin-arg=rpsplug_createclass:new_class_name \
              --extra=super=superclass \
              --extra=comment='some comment' \
              --extra=rooted=0 \
        --extra=constant=1 \
              --batch --dump=.

 ****/




#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obmutsetclass;
                           Rps_ObjectRef obsuperclass;
                           Rps_ObjectRef obnewclass;
                           Rps_ObjectRef obsymbol;
                           Rps_Value namestr; // a string
                           Rps_Value commentstr; // a string
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*supername = rps_get_extra_arg("super");
  const char*comment = rps_get_extra_arg("comment");
  const char*rooted = rps_get_extra_arg("rooted");
  const char*constant = rps_get_extra_arg("constant");
  bool isrooted = false;
  bool isconstant = false;
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
  /* Check that supername is given and is naming an existing class. */
  if (!supername)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without super extra name. See comments in " << __FILE__);
  {
    bool goodsupername = isalpha(supername[0]) || supername[0]=='_';
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
  std::lock_guard<std::recursive_mutex> gunewclass(*(_f.obnewclass->objmtxptr()));
  if (comment)
    {
      _f.commentstr = Rps_StringValue(comment);
      _f.obnewclass->put_attr(RPS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol
                              _f.commentstr);
    }
  _f.namestr = Rps_Value{std::string(plugarg)};
  /// Avoid using below RPS_ROOT_OB(4FBkYDlynyC02QtkfG):"name"∈named_attribute
  /// it was was a mistake.
  _f.obnewclass
  ->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
             _f.namestr);
  /* Create a symbol for the new class name. */
  _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, std::string{plugarg});
  std::lock_guard<std::recursive_mutex> gusymbol(*(_f.obsymbol->objmtxptr()));
  Rps_PayloadSymbol* paylsymb = _f.obsymbol->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT (paylsymb != nullptr);
  paylsymb->symbol_put_value(_f.obnewclass);
  _f.obnewclass->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), //symbol∈symbol
                          _f.obsymbol);
  {
    _f.obmutsetclass = RPS_ROOT_OB(_4DsQEs8zZf901wT1LH); //"the_mutable_set_of_classes"∈mutable_set
    std::lock_guard<std::recursive_mutex> gumutsetclass(*(_f.obmutsetclass->objmtxptr()));
    Rps_PayloadSetOb* paylsetob = _f.obmutsetclass->get_dynamic_payload<Rps_PayloadSetOb>();
    RPS_ASSERT(paylsetob != nullptr);
    paylsetob->add(_f.obnewclass);
  };
  if (isrooted)
    {
      rps_add_root_object(_f.obnewclass);
      RPS_INFORMOUT("rpsplug_createclass added new root class " << _f.obnewclass
                    << " of hash " << _f.obnewclass->obhash()
                    << " named " << plugarg << " of super class " << _f.obsuperclass << " and symbol " << _f.obsymbol);
    }
  else
    {
      RPS_INFORMOUT("rpsplug_createclass added new class " << _f.obnewclass
                    << " of hash " << _f.obnewclass->obhash()
                    << " named " << plugarg << " of super class " << _f.obsuperclass << " and symbol " << _f.obsymbol);
    }
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_createclass.so && /bin/ln -sv $(/bin/pwd)/plugins_dir/rpsplug_createclass.so /tmp/" ;;
 ** End: ;;
 ****************/

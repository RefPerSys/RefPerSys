// see http://refpersys.org/
// passed to commits after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 - 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates a new RefPerSys symbol
/*****
 * Once compiled, use it for example as:

 ./refpersys --plugin-after-load=/tmp/rpsplug_createsymbol.so \
 --plugin-arg=rpsplug_createsymbol:new_symbol_name \
 --extra=comment='some comment' \
 --extra=rooted=0 \
 --extra=constant=1 \
 --batch --dump=.

****/

#include "refpersys.hh"

#warning "is buggy at commit  80608c7722cf"
/*** buggy invocation at commit 80608c7722cf Sun Jun 30 06:22 PM CEST 2024
./refpersys --plugin-after-load=/tmp/rpsplug_createsymbol.so \
  --plugin-arg=rpsplug_createsymbol:include_priority \
  --extra=comment='attribute for C++ include priority (an integer)' \
  --extra=rooted=0  --extra=constant=1  --batch --dump=.
***/
void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obsymbol;
		 Rps_Value namestr; // a string
		 Rps_Value commentstr; // a string
		 );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*comment = rps_get_extra_arg("comment");
  const char*rooted = rps_get_extra_arg("rooted");
  const char*constant = rps_get_extra_arg("constant");
  bool isrooted = false;
  bool isconstant = false;
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
  if (rooted && rooted[0])
    {
      if (!strcmp(rooted, "true"))
	isrooted = true;
      else if (atoi(rooted) > 0)
	isrooted = true;
    };
  if (constant && constant[0])
    {
      if (!strcmp(constant, "true"))
	isconstant = true;
      else if (atoi(constant) > 0)
	isconstant = true;
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
  _f.obsymbol->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
                        _f.namestr);
  _f.obsymbol->put_space(Rps_ObjectRef::root_space());
  if (comment && comment[0]) {
    _f.commentstr = Rps_Value{std::string(comment)};
    _f.obsymbol->put_attr(RPS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol
			  _f.commentstr);
    RPS_INFORMOUT("rpsplug_createsymbol put comment " << _f.commentstr
		  << " in created symbol " << _f.obsymbol);
  }
  if (isrooted)
    {
      rps_add_root_object(_f.obsymbol);
      RPS_INFORMOUT("rpsplug_createsymbol added new root symbol "
		    << _f.obsymbol
                    << " named " << plugarg);
    }
  else if (isconstant) {
    rps_add_constant_object(&_, _f.obsymbol);
    RPS_INFORMOUT("rpsplug_createsymbol added new constant symbol "
		  << _f.obsymbol
		  << " named " << plugarg);
  }
  else
    {
      RPS_INFORMOUT("rpsplug_createsymbol added new plain symbol "
		    << _f.obsymbol
                    << " named " << plugarg);
    }
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_createsymbol.so && /bin/ln -svf $(/bin/pwd)/plugins_dir/rpsplug_createsymbol.so /tmp/" ;;
 ** End: ;;
 ****************/

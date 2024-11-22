// see http://refpersys.org/
// passed to commits after  9d1db4092 (of July 13, 2023)
// GPLv3+ licensed
// © Copyright 2023 - 2024 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates a new RefPerSys named attribute
/*****

 Once compiled, use it for example as:

  ./refpersys --plugin-after-load=/tmp/rpsplug_createnamedattribute.so \
              --plugin-arg=rpsplug_createnamedattribute:new_attr_name \
              --extra=comment='some comment' \
              --extra=rooted=0 \
              --extra=constant=0 \
              --batch --dump=.

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
  if (rooted)
    {
      if (!strcmp(rooted, "true"))
	isrooted = true;
      else if (!strcmp(rooted, "false"))
	isrooted = false;
      else if (rooted[0]=='Y' || rooted[0]=='y')
	isrooted = true;
      else if (rooted[0]=='N' || rooted[0]=='n')
	isrooted = false;
      else if (isdigit(rooted[0])) 
	isrooted = atoi(rooted)>0;
      else RPS_WARNOUT(" plugin " << plugin->plugin_name
		       << " is ignoring rooted=" << Rps_QuotedC_String(rooted)
		       << " extra argument."
		       << std::endl << "(expecting true/false or yes/no or a digit)");
    };
  if (constant)
    {
      if (!strcmp(constant, "true"))
	isconstant = true;
      else if (!strcmp(constant, "false"))
	isconstant = false;
      else if (constant[0]=='Y' || constant[0]=='y')
	isconstant = true;
      else if (constant[0]=='N' || constant[0]=='n')
	isconstant = false;
      else if (isdigit(constant[0]))
	isconstant = atoi(constant)>0;
      else RPS_WARNOUT(" plugin " << plugin->plugin_name
		       << " is ignoring constant=" << Rps_QuotedC_String(constant)
		       << " extra argument."
		       << std::endl
		       << "(expecting true/false or yes/no or a digit)");
    };
  /* Check that plugarg is some new name */
  if (auto nob = Rps_ObjectRef::find_object_or_null_by_string(&_, std::string(plugarg)))
    {
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name << " argument " << plugarg
                   << " is naming an existing object " << nob);
    };
  RPS_INFORMOUT("plugin " << plugin->plugin_name
		<< " should create a symbol named " << Rps_Cjson_String(plugarg)
		<< " " << (isrooted?"root":"non-root")
		<< " " << (isconstant?"constant":"non-constant")
		);
  /* Create a symbol for the new class name. */
  _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, std::string{plugarg});
  std::lock_guard<std::recursive_mutex> gusymbol(*(_f.obsymbol->objmtxptr()));
  _f.obsymbol->put_space(Rps_ObjectRef::root_space());
  Rps_PayloadSymbol* paylsymb = _f.obsymbol->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT (paylsymb != nullptr);
  paylsymb->symbol_put_value(_f.obnamedattr);
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
      RPS_DEBUG_LOG(REPL, "rpsplug_createnamedattribute adding new root namedattr " << _f.obnamedattr
		    << " of class " << _f.obnamedattr->compute_class(&_) << " space " << _f.obnamedattr->get_space());
      rps_add_root_object(_f.obnamedattr);
      RPS_INFORMOUT("rpsplug_createnamedattribute added new root named attribute "
		    << _f.obnamedattr
                    << " named " << plugarg
                    << " with symbol " << _f.obsymbol);
    }
  else if (isconstant) {
      RPS_DEBUG_LOG(REPL, "rpsplug_createnamedattribute adding new constant namedattr " << _f.obnamedattr
		    << " of class " << _f.obnamedattr->compute_class(&_) << " space " << _f.obnamedattr->get_space());
    rps_add_constant_object(&_, _f.obnamedattr);
    RPS_INFORMOUT("rpsplug_createnamedattribute added new constant named attribute "
		  << _f.obnamedattr << std::endl
		  << " of class " << _f.obnamedattr->compute_class(&_) << " space " <<  _f.obnamedattr->get_space()
		  << " named " << plugarg
		  << " with symbol " << _f.obsymbol);
  }
  else
    {
      RPS_INFORMOUT("rpsplug_createnamedattribute added new named attribute " << _f.obnamedattr
		    << " of class " << _f.obnamedattr->compute_class(&_) << " space " << _f.obnamedattr->get_space()
                    << " named " << plugarg
                    << " with symbol " << _f.obsymbol);
    }
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./do-build-refpersys-plugin plugins_dir/rpsplug_createnamedattribute.cc -o /tmp/rpsplug_createnamedattribute.so" ;;
 ** End: ;;
 ****************/

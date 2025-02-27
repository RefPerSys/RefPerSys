// see http://refpersys.org/
// SPDX-License-Identifier: GPL-3.0-or-later
// GPLv3+ licensed file plugins_dir/rpsplug_create_cplusplus_primitive_type.cc
// © Copyright 2024 - 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin creates a new RefPerSys C++ primitive type for C++ generation
// and could be used by machine (GNU lightning) or libgccjit code generation
/*****
      Once compiled, use it for example as:
      ./refpersys --plugin-after-load=/tmp/rpsplug_create_cplusplus_primitive_type.so \
      --plugin-arg=rpsplug_create_cplusplus_create_primitive_type:native_int_type \
      --extra=comment='the native int type' \
      --batch --dump=.

      then update appropriately the rps_set_native_data_in_loader to force
      native alignement and size
****/




#include "refpersys.hh"

#warning incomplete plugins_dir/rpsplug_create_cplusplus_primitive_type.cc
void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obmutsetclass;
                           Rps_ObjectRef obsuperclass;
                           Rps_ObjectRef obcppprimtypclass;
                           Rps_ObjectRef obcpptype;
                           Rps_ObjectRef obsymbol;
                           Rps_Value namestr; // a string
                           Rps_Value commentstr; // a string
                           Rps_Value cplusplusstr; // a string
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*comment = rps_get_extra_arg("comment");
  const char*cplusplusname = rps_get_extra_arg("cplusplus");
  RPS_INFORMOUT("loaded plugin " <<  plugin->plugin_name
                << " file " << __FILE__
                << " comment:" << Rps_QuotedC_String(comment)
                << " plugarg:" << Rps_QuotedC_String(plugarg)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_do_plugin/createC++primtyp"));
  _f.obcppprimtypclass = RPS_ROOT_OB(_1XswYkom3Jm02YR3Vi); //cplusplus_primitive_type∈class
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin/createC++primtyp " << plugin->plugin_name
                 << " without argument; should be some non-empty name");
  RPS_POSSIBLE_BREAKPOINT();
  ///
  /* Check that plugarg is a good name */
  {
    bool goodplugarg = isalpha(plugarg[0]);
    for (const char*pa = &plugarg[0]; goodplugarg && *pa; pa++)
      goodplugarg = isalnum(*pa) || *pa=='_';
  }
  /* Check that plugarg is some new name */
  if (auto nob = Rps_ObjectRef::find_object_or_null_by_string(&_, std::string(plugarg)))
    {
      RPS_FATALOUT("failure: plugin " << plugin->plugin_name << " argument " << plugarg
                   << " is naming an existing object " << nob);
    };
  RPS_POSSIBLE_BREAKPOINT();
  _f.obcpptype = Rps_ObjectRef::make_object(&_,
                 /*class:*/_f.obcppprimtypclass,
                 /*space:*/Rps_ObjectRef::root_space()
                                           );
  if (comment)
    {
      _f.commentstr = Rps_StringValue(comment);
      _f.obcpptype->put_attr(RPS_ROOT_OB(_0jdbikGJFq100dgX1n), //comment∈symbol
                             _f.commentstr);
    }
  if (cplusplusname && isalpha(cplusplusname[0]))
    {
      _f.cplusplusstr =  Rps_Value{std::string(cplusplusname)};
      _f.obcpptype
      ->put_attr(rpskob_0fx0GtCX90Z03VI9mo, //!cplusplus_name∈named_attribute
                 _f.cplusplusstr);
    };
  _f.namestr = Rps_Value{std::string(plugarg)};
  /// Avoid using below RPS_ROOT_OB(4FBkYDlynyC02QtkfG):"name"∈named_attribute
  /// it was was a mistake.
  _f.obcpptype
  ->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
             _f.namestr);
  /* Create a symbol for the new class name. */
  _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, std::string{plugarg});
  std::lock_guard<std::recursive_mutex> gusymbol(*(_f.obsymbol->objmtxptr()));
  Rps_PayloadSymbol* paylsymb = _f.obsymbol->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT (paylsymb != nullptr);
  paylsymb->symbol_put_value(_f.obcpptype);
  _f.obcpptype->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), //symbol∈symbol
                         _f.obsymbol);
  rps_add_constant_object(&_, _f.obcpptype);
  RPS_INFORMOUT("rpsplug_create_cplusplus_code_class added new object " << std::endl
                << RPS_OBJECT_DISPLAY(_f.obcpptype) << std::endl
                << " named " << plugarg << " of class "
                << _f.obcppprimtypclass << " and symbol " << _f.obsymbol
                << " in space " << _f.obcpptype->get_space());
} // end rps_do_plugin


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_create_cplusplus_primitive_type.so &&  /bin/ln -sfv $(/bin/pwd)/plugins_dir/rpsplug_create_cplusplus_primitive_type.so /tmp/"" ;;
 ** End: ;;
 ****************/

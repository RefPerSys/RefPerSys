// see http://refpersys.org/
// passed to commits after  f7d110da9b (of Aug 10, 2025) of RefPerSys
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>

// This rpsplug_thesetreploper.cc plugin installs the mutable set of
// REPL operators.

// Conceptually this plugin should be successfully run only once and
// have a dump be done...

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewsetoper;
                           Rps_ObjectRef obsymbol;
                           Rps_Value strname;
                );
#define MYARGMAXLEN 64
  char argcopy[MYARGMAXLEN];
  bool argispunct = false;
  bool argisident = false;
  int precedence = -1;
  constexpr const char* nm = "set_of_repl_operators";
  memset (argcopy, 0, MYARGMAXLEN);
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
#warning incomplete rpsplug_thesetreploper.cc
  _f.obnewsetoper =
    Rps_PayloadSetOb::make_mutable_set_object(&_,
					      RPS_ROOTOB(_0J1C39JoZiv03qA2HA), // mutable_set
					      Rps_ObjectRef::root_space());
  std::lock_guard<std::recursive_mutex> gunewsetoper(*(_f.obnewsetoper->objmtxptr()));
  _f.strname = Rps_StringValue(nm);
  _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, nm);
  std::lock_guard<std::recursive_mutex> guobsymbol(*(_f.obsymbol->objmtxptr()));
  Rps_PayloadSymbol* paysymb = _f.obsymbol->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT(paysymb != nullptr);
  _f.obnewsetoper->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), //name∈named_attribute
			    _f.strname);
  _f.obnewsetoper->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), //symbol∈symbol
			    _f.obsymbol);
  paysymb->symbol_put_value(_f.obnewsetoper);
  rps_add_constant_object(&_, _f.obnewsetoper);
  rps_add_constant_object(&_, _f.obsymbol);
  /** TODO: We need to create a single constant object, named
      set_of_repl_operators, whose payload is a mutable set of
      objects.  Once dumped successfully, we need to improve the
      rpsplug_createcommutativeoperator.cc &
      rpsplug_createnoncommutativeoperator.cc */
  RPS_FATALOUT("rpsplug_thesetreploper incomplete implemented for "
               <<  Rps_QuotedC_String(plugarg) << std::endl
	       << " obnewsetoper=" << RPS_OBJECT_DISPLAY(_f.obnewsetoper)
	       << " obsymbol=" << << RPS_OBJECT_DISPLAY(_f.obsymbol));

} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && ./do-build-refpersys-plugin -v plugins_dir/rpsplug_thesetreploper.cc -o /tmp/rpsplug_thesetreploper.so" ;;
 ** End: ;;
 ****************/

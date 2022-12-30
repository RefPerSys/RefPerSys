// see http://refpersys.org/
// passed to commits after dd0c90db2992da (of Dec 28, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2022 Basile Starynkevitch <basile@starynkevitch.net>
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
		 Rps_ObjectRef obnewroot;
		 Rps_ObjectRef obcommutclass;
		 Rps_ObjectRef obclassoper;
                 Rps_Value strname;
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*xtraname = rps_get_extra_arg("name");
  const char*xtraprecedence = rps_get_extra_arg("precedence");
  /// get repl_precedence attribute, conventionally values are small non-negative tagged integers
  _f.obreplprecedence = RPS_ROOT_OB(_7iVRsTR8u3D00Cy0hp); //repl_precedence∈symbol
  /// get the repl_operator superclass
  _f.obclassoper =  RPS_ROOT_OB(_9j12Nhm4itk00YYUW7); //repl_operator∈class
  RPS_ASSERT(_f.obclassoper != nullptr);
  if (!plugarg)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some string");
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
                << Rps_QuotedC_String(plugarg)
		<< " and extra name " << Rps_QuotedC_String(xtraname)
		<< " and extra precedence " << Rps_QuotedC_String(xtraprecedence));
  _f.obcommutclass = 
  RPS_FATALOUT("rpsplug_createcommutativeoperator not implemented for "
               <<  Rps_QuotedC_String(plugarg));
#warning unimplemented rpsplug_createcommutativeoperator
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_createcommutativeoperator.cc /tmp/rpsplug_createcommutativeoperator.so" ;;
 ** End: ;;
 ****************/


// see http://refpersys.org/
// passed to commit fab255b66fcf2655 of RefPerSys (master branch)
// GPLv3+ licensed
// © Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createreplcmddict.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obclassstrdict;
		 Rps_ObjectRef obreplcmddict;
		 Rps_ObjectRef obreplcmdsy;
                 Rps_Value strname;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obclassstrdict = RPS_ROOT_OB(_3FztYBKABxZ02DUPRm); //string_dictionary∈class;
  _f.obreplcmddict
    = Rps_PayloadStringDict::make_string_dictionary_object
    (&_, _f.obclassstrdict,
     Rps_ObjectRef::root_space());
  _f.strname = Rps_String::make("repl_command_dict");
  _f.obreplcmddict->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx), // name
			     _f.strname);
  rps_add_root_object(_f.obreplcmddict);
  _f.obreplcmdsy = Rps_ObjectRef::make_new_strong_symbol(&_, "repl_command_dict");
  Rps_PayloadSymbol::register_strong_name("repl_command_dict",
					  _f.obreplcmdsy);
  _f.obreplcmdsy->put_space(Rps_ObjectRef::root_space());
  Rps_PayloadSymbol* paysy =
    _f.obreplcmdsy->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT(paysy);
  paysy->symbol_put_value(_f.obreplcmddict);
  _f.obreplcmddict->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), // symbol
			     _f.obreplcmdsy);
  RPS_INFORMOUT("did add repl_command_dict  := " << _f.obreplcmddict
		<< " symbol " << _f.obreplcmdsy);
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createreplcmddict.cc -o /tmp/rpsplug_createreplcmddict.so" ;;
 ** End: ;;
 ****************/

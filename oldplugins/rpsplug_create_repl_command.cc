 // see http://refpersys.org/
 // passed to commit 4c13d8a0b6ef98ffb of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_create_repl_command.so --batch --dump=.


#include "refpersys.hh"



void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obobjectclass;
		 Rps_ObjectRef obreplcommandclass;
		 Rps_Value strname;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
 
  /// the existing class for objects
  _f.obobjectclass = Rps_ObjectRef::the_object_class();
  _f.obreplcommandclass = Rps_ObjectRef::make_named_class(&_,
						     _f.obobjectclass,
						     "repl_command");
  _f.strname = Rps_String::make("repl_command");
  _f.obreplcommandclass->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				  _f.strname);
  rps_add_root_object(_f.obreplcommandclass);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_create_repl_command.cc -o /tmp/rpsplug_create_repl_command.so" ;;
  ** End: ;;
  ****************/

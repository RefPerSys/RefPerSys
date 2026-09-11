 // see http://refpersys.org/
 // passed to commit 4436465b6d25e6470cc of RefPerSys
 // GPLv3+ licensed
 // © Copyright 2021 Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_create_replcommands.so --batch --dump=.

 /// we then paste the generated C++ code from stdout into cmdrepl_rps.cc

#include "refpersys.hh"



void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_Value strname;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  rps_repl_create_command(&_, "show");
  rps_repl_create_command(&_, "help");
  rps_repl_create_command(&_, "put");
  rps_repl_create_command(&_, "remove");
  rps_repl_create_command(&_, "append");
  rps_repl_create_command(&_, "add_root");
  rps_repl_create_command(&_, "remove_root");
  rps_repl_create_command(&_, "make_symbol");
  rps_repl_create_command(&_, "generate_code");
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_create_replcommands.cc -o /tmp/rpsplug_create_replcommands.so" ;;
  ** End: ;;
  ****************/

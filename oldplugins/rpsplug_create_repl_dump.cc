 // see http://refpersys.org/
 // passed to commit af80c5b68b98b705b2c64 of RefPerSys
 // GPLv3+ licensed
 // © Copyright 2021 Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_create_repl_dump.so --batch --dump=.

 /// we then paste the generated C++ code from stdout into cmdrepl_rps.cc

#include "refpersys.hh"



void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  rps_repl_create_command(&_, "dump");
  RPS_INFORMOUT("end plugin " << plugin->plugin_name);
}

 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_create_repl_dump.cc -o /tmp/rpsplug_create_repl_dump.so" ;;
  ** End: ;;
  ****************/

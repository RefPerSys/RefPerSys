// see http://refpersys.org/
// passed to commits after 79cf0a6e0f1eb0a (of Nov 2, 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright Basile Starynkevitch <basile@starynkevitch.net>
// This plugin install one new root object given by its existing oid
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_installrootoid.so --plugin-arg=rpsplug_installrootoid:someoid --batch --dump=.


#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obnewroot;
		 );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
		<< Rps_QuotedC_String(plugarg));
  RPS_WARNOUT("incomplete rpsplug_installrootoid " << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "rps_do_plugin/installrootoid"));
#warning incomplete rpsplug_installrootoid.cc
} // end rps_do_plugin

 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC plugins/rpsplug_installrootoid.cc -o /tmp/rpsplug_installrootoid.so" ;;
  ** End: ;;
  ****************/

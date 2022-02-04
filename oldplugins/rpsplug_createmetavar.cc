 // see http://refpersys.org/
 // passed to commit 180424305b76aaeb of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createmetavar.so --batch --dump=.


#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obsymbmetavar;
		 Rps_Value strname;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
 
  /// the existing class for mutable vector objects
  _f.obsymbmetavar =
    Rps_ObjectRef::make_new_strong_symbol(&_,
					  std::string{"meta_variable"});
  _f.strname = Rps_String::make("meta_variable");
  _f.obsymbmetavar->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				_f.strname);
  rps_add_root_object(_f.obsymbmetavar);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/refpersys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createmetavar.cc -o /tmp/rpsplug_createmetavar.so" ;;
  ** End: ;;
  ****************/

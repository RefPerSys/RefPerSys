 // see http://refpersys.org/
 // passed to commit a97541b36c1682e3t of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createinplin.so --batch --dump=.


#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obinput;
		 Rps_ObjectRef obline;
		 Rps_Value strname;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
 
  _f.obinput = Rps_ObjectRef::make_new_strong_symbol (&_, "input");
  _f.strname = Rps_String::make("input");
  _f.obinput->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
			 _f.strname);
  rps_add_root_object(_f.obinput);
  
  _f.obline = Rps_ObjectRef::make_new_strong_symbol (&_, "line");
  _f.strname = Rps_String::make("line");
  _f.obline->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
			 _f.strname);
  rps_add_root_object(_f.obline);
 
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/refpersys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createinplin.cc -o /tmp/rpsplug_createinplin.so" ;;
  ** End: ;;
  ****************/

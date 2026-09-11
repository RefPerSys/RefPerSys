// see http://refpersys.org/
 // passed to commit 5dfbee91aa961 of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createdisplaysymbol.so --batch --dump=.


#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obdisplaysymb;
		 Rps_ObjectRef obreplverbsymb;
		 Rps_ObjectRef obclasscorefun;
		 Rps_ObjectRef obcorefun;
		 Rps_Value strname;
		 Rps_Value closurev;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obdisplaysymb = Rps_ObjectRef::make_new_strong_symbol (&_, "display");
  _f.strname = Rps_String::make("display");
  _f.obdisplaysymb->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
			 _f.strname);
  rps_add_root_object(_f.obdisplaysymb);
  RPS_INFORMOUT("new display symbol is " << _f.obdisplaysymb);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createdisplaysymbol.cc -o /tmp/rpsplug_createdisplaysymbol.so" ;;
  ** End: ;;
  ****************/


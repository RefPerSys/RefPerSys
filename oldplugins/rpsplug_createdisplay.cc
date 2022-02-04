// see http://refpersys.org/
 // passed to commit f63caa4da4df883c5c1 of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createdisplay.so --batch --dump=.


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
  _f.obreplverbsymb = Rps_ObjectRef::make_new_strong_symbol (&_, "repl_verb");
  _f.strname = Rps_String::make("repl_verb");
  _f.obreplverbsymb->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
			 _f.strname);
  rps_add_root_object(_f.obreplverbsymb);
  RPS_INFORMOUT("new repl_verb symbol is " << _f.obdisplaysymb);
  _f.obclasscorefun = RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS);
  RPS_INFORMOUT("obclasscorefun is " << _f.obclasscorefun);
  _f.obcorefun = Rps_ObjectRef::make_object
     (&_,
      _f.obclasscorefun,
      Rps_ObjectRef::root_space());
   _f.strname = Rps_String::make("display°repl_verb");
   _f.obcorefun->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
			  _f.strname);
   rps_add_root_object(_f.obcorefun);
  RPS_INFORMOUT("obcorefun display°repl_verb is " << _f.obdisplaysymb);
   _f.closurev = Rps_ClosureValue( _f.obcorefun,{});
   Rps_ObjectValue( _f.obdisplaysymb).as_object()->put_attr(_f.obreplverbsymb, _f.closurev); 
   RPS_INFORMOUT("obdisplaysymb=" << _f.obdisplaysymb
		 << ", obreplverbsymb=" << _f.obreplverbsymb
		 << ", closurev=" << _f.closurev);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createdisplay.cc -o /tmp/rpsplug_createdisplay.so" ;;
  ** End: ;;
  ****************/


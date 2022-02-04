 // see http://refpersys.org/
 // passed to commit d87108dcf6228025f of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createsystem.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_Value strname;
		 Rps_ObjectRef obclassthesystemclass;
		 Rps_ObjectRef obgeneratecode;
		 Rps_ObjectRef obrefpersys_system;
		 Rps_ObjectRef obselgeneratecode;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obclassthesystemclass = //
    Rps_ObjectRef::make_named_class(&_,
				    Rps_ObjectRef::the_object_class(),
				    "the_system_class");
  _f.strname = Rps_String::make("the_system_class");
  _f.obclassthesystemclass->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				     _f.strname);
  rps_add_root_object(_f.obclassthesystemclass);
  _f.obrefpersys_system =
    Rps_ObjectRef::make_object(&_,
			       _f.obclassthesystemclass,
			       Rps_ObjectRef::root_space());
  Rps_PayloadSymbol::register_strong_name("RefPerSys_system", _f.obrefpersys_system);
  _f.strname = Rps_String::make("RefPerSys_system");
  _f.obrefpersys_system->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				  _f.strname);
  rps_add_root_object(_f.obrefpersys_system);
  _f.obselgeneratecode =
    Rps_ObjectRef::make_object(&_,
			       Rps_ObjectRef::the_named_selector_class(),
			       Rps_ObjectRef::root_space());
  Rps_PayloadSymbol::register_strong_name("generate_code",  _f.obselgeneratecode);
  _f.strname = Rps_String::make("generate_code");
  _f.obselgeneratecode->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				  _f.strname);
  rps_add_root_object(_f.obselgeneratecode);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createsystem.cc -o /tmp/rpsplug_createsystem.so" ;;
  ** End: ;;
  ****************/

 // see http://refpersys.org/
 // passed to commit 08d64f2258b2f884a8 of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createtemporarycplusplus.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_Value strname;
		 Rps_ObjectRef obclasstempcplusplus;
		 Rps_ObjectRef obgeneratecode;
		 Rps_ObjectRef obrefpersys_system;
		 Rps_ObjectRef obselgeneratecode;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obclasstempcplusplus = //
    Rps_ObjectRef::make_named_class(&_,
				    Rps_ObjectRef::the_object_class(),
				    "temporary_cplusplus_code");
  _f.strname = Rps_String::make("temporary_cplusplus_code");
  _f.obclasstempcplusplus->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				     _f.strname);
  rps_add_root_object(_f.obclasstempcplusplus);
  RPS_INFORMOUT("made temporary_cplusplus_code class "
		<< _f.obclasstempcplusplus);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createtemporarycplusplus.cc -o /tmp/rpsplug_createtemporarycplusplus.so" ;;
  ** End: ;;
  ****************/

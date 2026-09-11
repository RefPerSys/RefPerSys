 // see http://refpersys.org/
 // passed to commit f2b2dad14c682e0 of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_createcodechunk.so --batch --dump=.


#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obclasscodechunk;
		 Rps_ObjectRef obclassmutablevector;
		 Rps_Value strname;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
 
  /// the existing class for mutable vector objects
  _f.obclassmutablevector = RPS_ROOT_OB(_8YknAApDQiF04BDe3W);
  _f.obclasscodechunk = Rps_ObjectRef::make_named_class(&_,
							_f.obclassmutablevector,
							"code_chunk");
  _f.strname = Rps_String::make("code_chunk");
  _f.obclasscodechunk->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				_f.strname);
  rps_add_root_object(_f.obclasscodechunk);
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/refpersys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createcodechunk.cc -o /tmp/rpsplug_createcodechunk.so" ;;
  ** End: ;;
  ****************/

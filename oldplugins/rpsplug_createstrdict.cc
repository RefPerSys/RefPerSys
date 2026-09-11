   
// see http://refpersys.org/
// passed to commit c9a46de96b97975c65882 of RefPerSys (master branch)
// GPLv3+ licensed
// © Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createstrdict.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obclassstrdict;
                 Rps_Value strname;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  // for Rps_PayloadStrdict
  _f.obclassstrdict = Rps_ObjectRef::make_named_class(&_,
                                  Rps_ObjectRef::the_class_class(),
                                  "string_dictionary");
  _f.strname = Rps_String::make("string_dictionary");
  _f.obclassstrdict->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
                            _f.strname);
  rps_add_root_object(_f.obclassstrdict);

} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/refpersys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createstrdict.cc -o /tmp/rpsplug_createstrdict.so" ;;
 ** End: ;;
 ****************/


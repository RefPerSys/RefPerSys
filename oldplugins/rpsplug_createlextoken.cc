   
// see http://refpersys.org/
// passed to commit 65a25814fe1ab6f5b4  of RefPerSys (master branch)
// GPLv3+ licensed
// © Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createlextoken.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obclassstrbuf;
                 Rps_Value strname;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  // for Rps_PayloadStrBuf
  _f.obclassstrbuf = Rps_ObjectRef::make_named_class(&_,
                                  Rps_ObjectRef::the_class_class(),
                                  "lexical_token");
  _f.strname = Rps_String::make("lexical_token");
  _f.obclassstrbuf->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
                            _f.strname);
  rps_add_root_object(_f.obclassstrbuf);

} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createlextoken.cc -o /tmp/rpsplug_createlextoken.so" ;;
 ** End: ;;
 ****************/


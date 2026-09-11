
// see http://refpersys.org/
// passed to commit 327a9575bc845f20b of RefPerSys (master branch)
// GPLv3+ licensed
// © 2021 Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_makewebpai.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obclasswebpi;
                 Rps_Value strname;
		 );
  _f.obclasswebpi =
    Rps_ObjectRef::make_named_class(&_,
				     RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ), //object∈class
				     std::string{"web_processing_instruction_class"});
  _f.strname = Rps_String::make("web_processing_instruction_class");
  _f.obclasswebpi->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
				     _f.strname);
  rps_add_root_object(_f.obclasswebpi);
}

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/local/include  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_makewebpai.cc -o /tmp/rpsplug_makewebpai.so" ;;
 ** End: ;;
 ****************/


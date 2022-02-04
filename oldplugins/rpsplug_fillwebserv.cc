   
// see http://refpersys.org/
// passed to commit 8435e1ecf011546c of RefPerSys (master branch)
// GPLv3+ licensed
// © Copyright 2021 Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_fillwebserv.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obwebservice;
                 Rps_Value strname;
		 Rps_ObjectRef obsymbwebservice;
		 Rps_ObjectRef obsetwebobj;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obwebservice = RPS_ROOT_OB(_0MInvb6lXCQ006IiJZ);
  _f.obsetwebobj =
    Rps_PayloadSetOb::make_mutable_set_object
    (&_,
     /*class:*/RPS_ROOT_OB(_0J1C39JoZiv03qA2HA), //mutable_set∈class
     /*space:*/Rps_ObjectRef::root_space());
  _f.obwebservice->put_attr(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ), //mutable_set∈class
			    _f.obsetwebobj);
  RPS_INFORMOUT("plugin "<< plugin->plugin_name
		<< " created web_service:" << _f.obwebservice
		<< " and its symbol:" << _f.obsymbwebservice);
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_fillwebserv.cc -o /tmp/rpsplug_fillwebserv.so" ;;
 ** End: ;;
 ****************/


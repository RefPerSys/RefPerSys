   
// see http://refpersys.org/
// passed to commit 8c03732b3d49ecfdof RefPerSys (master branch)
// GPLv3+ licensed
// © Copyright 2021 Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_createwebserv.so --batch --dump=.

#include "refpersys.hh"

void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obwebservice;
                 Rps_Value strname;
		 Rps_ObjectRef obsymbwebservice;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  /// we first create a web_service object of class _3FztYBKABxZ02DUPRm=string_dictionary∈class
  _f.obwebservice =
    Rps_ObjectRef::make_object(&_,
			       RPS_ROOT_OB(_3FztYBKABxZ02DUPRm), //string_dictionary∈class
			       Rps_ObjectRef::root_space());
  _f.strname = Rps_String::make("web_service");
  _f.obwebservice->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
                            _f.strname);
  rps_add_root_object (_f.obwebservice);
  _f.obsymbwebservice =
    Rps_ObjectRef::make_new_strong_symbol(&_, "web_service");
  {
    RPS_ASSERT(_f.obsymbwebservice);
    Rps_PayloadSymbol* paysy =
      _f.obsymbwebservice->get_dynamic_payload<Rps_PayloadSymbol>();
    RPS_ASSERT (paysy != nullptr);
    paysy->symbol_put_value(_f.obwebservice);
    _f.obwebservice->put_attr(RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5), //symbol∈symbol
			      _f.obsymbwebservice);
  }
  RPS_INFORMOUT("plugin "<< plugin->plugin_name
		<< " created web_service:" << _f.obwebservice
		<< " and its symbol:" << _f.obsymbwebservice);
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_createwebserv.cc -o /tmp/rpsplug_createwebserv.so" ;;
 ** End: ;;
 ****************/


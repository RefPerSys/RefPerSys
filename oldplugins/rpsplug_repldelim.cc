   
// see http://refpersys.org/
// passed to commit 7c4fc9f051d4b827ff5e  of RefPerSys (master branch)
// GPLv3+ licensed
// © Copyright Basile Starynkevitch <basile@starynkevitch.net>
// once compiled, use it as:
/// ./refpersys --plugin-after-load=/tmp/rpsplug_repldelim.so --batch --dump=.

#include "refpersys.hh"

Rps_ObjectRef
rpsplugdelim_add_delimiter (Rps_CallFrame*callframe,
			    Rps_PayloadStringDict*payl,
			    Rps_ObjectRef obdelimclassarg,
			    const char*delim)
{
  
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/callframe,
                 Rps_ObjectRef obdelimclass;
                 Rps_ObjectRef obdelimdict;
                 Rps_ObjectRef obcurdelim;
		 Rps_Value strname;
                 );
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(payl);
  RPS_ASSERT(obdelimclassarg);
  RPS_ASSERT(delim && ispunct(delim[0]));
  _f.obdelimclass = obdelimclassarg;
  _f.obdelimdict = payl->owner();
  _f.obcurdelim = Rps_ObjectRef::make_object
     (&_,
      _f.obdelimclass,
      Rps_ObjectRef::root_space());
  _f.strname =  Rps_String::make(delim);
  _f.obcurdelim->put_attr(_f.obdelimclass, _f.strname);
  payl->add(std::string(delim), _f.obcurdelim);
  RPS_INFORMOUT("add delimiter " << _f.obcurdelim << " for " << delim);
  return _f.obcurdelim;
} // end rpsplugdelim_add_delimiter


void rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obrepldelim;
		 Rps_ObjectRef obdelimclass;
		 Rps_ObjectRef obdelimleftpar;
		 Rps_ObjectRef obdelimrightpar;
		 Rps_ObjectRef obdelimleftbrack;
		 Rps_ObjectRef obdelimrightbrack;
		 Rps_ObjectRef obdelimleftbrace;
		 Rps_ObjectRef obdelimrightbrace;
		 Rps_Value strname;
                 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obrepldelim =
    Rps_PayloadStringDict::make_string_dictionary_object
    (&_,
     Rps_PayloadStringDict::the_string_dictionary_class(),
     Rps_ObjectRef::root_space());
  _f.obdelimclass = Rps_ObjectRef::make_named_class(&_,
						    Rps_ObjectRef::the_class_class(),
						    "repl_delimiter");
  _f.strname = Rps_String::make("repl_delimiter");
  _f.obdelimclass->put_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx),
                            _f.strname);
  rps_add_root_object(_f.obdelimclass);
  auto paylsdic =
    _f.obrepldelim->get_dynamic_payload<Rps_PayloadStringDict>();
  paylsdic->set_transient(false);
  rps_add_root_object(_f.obrepldelim);
  RPS_INFORMOUT("obdelimclass=" << _f.obdelimclass
		<< " obrepldelim=" << _f.obrepldelim);  
  _f.obdelimleftpar
    = rpsplugdelim_add_delimiter(&_, paylsdic, _f.obdelimclass, "("); 
  _f.obdelimrightpar =
    rpsplugdelim_add_delimiter(&_, paylsdic, _f.obdelimclass, ")");
  _f.obdelimleftbrack =
    rpsplugdelim_add_delimiter(&_, paylsdic, _f.obdelimclass, "[");
  _f.obdelimrightbrack =
    rpsplugdelim_add_delimiter(&_, paylsdic, _f.obdelimclass, "]");
  _f.obdelimleftbrace =
    rpsplugdelim_add_delimiter(&_, paylsdic, _f.obdelimclass, "{");
  _f.obdelimrightbrace = 
    rpsplugdelim_add_delimiter(&_, paylsdic, _f.obdelimclass, "}");
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "g++ -I$HOME/refpersys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_repldelim.cc -o /tmp/rpsplug_repldelim.so" ;;
 ** End: ;;
 ****************/


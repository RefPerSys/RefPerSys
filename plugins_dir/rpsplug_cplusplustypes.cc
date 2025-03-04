// passed to RefPerSys commits after 29811e1cd643 (of Feb, 24, 2025)
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin names some C++ primitive types reifications
/*****

      Once compiled, use it for example as:

      ./refpersys --plugin-after-load=/tmp/rpsplug_cplusplustypes.so \
      --batch --dump=.

****/

#include "refpersys.hh"

void
rpscplusplustype(Rps_CallFrame*callframe, const char*obid, const char*cppname)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/callframe,
		 Rps_ObjectRef obty;
		 Rps_Value str;
		 );
  _f.obty = Rps_ObjectRef::find_object_or_fail_by_string(&_, obid);
  _f.str = Rps_Value(std::string(cppname));
  RPS_ASSERT(_f.obty->is_instance_of(RPS_ROOT_OB(_1XswYkom3Jm02YR3Vi))); /*cplusplus_primitive_type∈class*/;
  if (_f.obty->get_physical_attr(rpskob_0fx0GtCX90Z03VI9mo /*!cplusplus_name∈named_attribute*/	))
    RPS_WARNOUT("already filled obty: " << RPS_OBJECT_DISPLAY(_f.obty));
  else 
    _f.obty							
      ->put_attr(rpskob_0fx0GtCX90Z03VI9mo, /*!cplusplus_name∈named_attribute*/	
		 _f.str);						
  RPS_INFORMOUT("updated" << std::endl << RPS_OBJECT_DISPLAY(_f.obty) 
		<< std::endl);
} // end rpscplusplustype

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obty;
		 Rps_Value str;
		 );

  RPS_ASSERT(plugin != nullptr);
  RPS_DEBUG_LOG(CMD, "start plugin "
                      << plugin->plugin_name << " from " << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "C++ types plugin"));
#define RPS_TYPE_CPLUSPLUS(Id,Cstr) rpscplusplustype(&_, #Id, #Cstr)
  RPS_TYPE_CPLUSPLUS(_4V1oeUOvmxo041XLTm,intptr_t);
  RPS_TYPE_CPLUSPLUS(_4nZ0jIKUbGr01OixPV,int);
  RPS_TYPE_CPLUSPLUS(_3NYlqvmSuTm024LDuD,long);
  RPS_INFORMOUT("did run C++ types plugin " <<  plugin->plugin_name
                << " from pid " << (int)getpid()
                << " on " << rps_hostname()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "C++ types plugins"));
}

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; gmake one-plugin REFPERSYS_PLUGIN_SOURCE=plugins_dir/rpsplug_cplusplustypes.cc REFPERSYS_PLUGIN_SHARED_OBJECT=plugins_dir/rpsplug_cplusplustypes.so; /bin/ln -svf plugins_dir/rpsplug_cplusplustypes.so /tmp" ;;
 ** End: ;;
 *****************/

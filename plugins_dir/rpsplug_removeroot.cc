// see http://refpersys.org/
// passed to commits after 4226408d42ea (march 2026) of RefPerSys
// GPLv3+ licensed
// © Copyright (C) 2026 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin remove an obsolete root object by clearing its space
/*****
  Once compiled, use it for example as:
  ./refpersys --plugin-after-load=/tmp/rpsplug_removeroot.so \
              --plugin-arg=rpsplug_removeroot:<oid|name> \
              --batch --dump=.

 ****/




#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                 Rps_ObjectRef obroot;
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty name or oid");
  _f.obroot = Rps_ObjectRef::find_object_or_null_by_string(&_, plugarg);
  if (!_f.obroot)
    RPS_FATALOUT("plugin rpsplug_removeroot arg=" << plugarg
                 " dont refer to any existing object");
  std::lock_guard<std::recursive_mutex> gu(*(_f.obroot->objmtxptr()));
  if (_f.obroot->space() != Rps_ObjectRef::root_spacve())
    RPS_FATALOUT("plugin rpsplug_removeroot arg=" << plugarg
		 " refer to non-root object "
		 << Rps_Object_Display(_f.obroot));
  RPS_INFORMOUT("removing tentative root "
		<< Rps_Object_Display(_f.obroot));
  _f.obroot->put_space(nullptr);
  RPS_WARNOUT("untested rpsplug_removeroot arg=" << plugarg
               << " obroot=" << _f.obroot);
} // end rps_do_plugin


/****************
 **                           for Emacs...
  Local Variables: ;;
  compile-command: "cd $REFPERSYS_TOPDIR && \
    ./do-build-refpersys-plugin -v        \
      -i plugins_dir/rpsplug_removeroot.cc \
      -o plugins_dir/rpsplug_removeroot.so \
      -L /tmp/rpsplug_removeroot.so" ;;
  End: ;;
 ****************/

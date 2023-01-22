// see http://refpersys.org/
// passed to commits after 679ffef6209bfa103  (of Jan , 2022) of RefPerSys
// GPLv3+ licensed
// © Copyright 2023 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin fills the_mutable_set_of_classes
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_fillmutsetclass.so \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obcur;
		 Rps_ObjectRef obclass;
		 Rps_ObjectRef obsetcla;
                );
  _f.obsetcla =  RPS_ROOT_OB(_4DsQEs8zZf901wT1LH); //"the_mutable_set_of_classes"∈mutable_set
  RPS_INFORMOUT("running plugin " << plugin->plugin_name
		<< "setofclasses:" << _f.obsetcla);
#warning unimplemented rpsplug_fillmutsetclass
  RPS_WARNOUT("unimplemented rpsplug_fillmutsetclass");
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./build-plugin.sh plugins/rpsplug_fillmutsetclass.cc /tmp/rpsplug_fillmutsetclass.so" ;;
 ** End: ;;
 ****************/

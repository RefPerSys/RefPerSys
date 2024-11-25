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
  int nbclasses = 0;
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obcur;
		 Rps_ObjectRef obclass;
		 Rps_ObjectRef obsetcla;
		 );
  _f.obsetcla =  RPS_ROOT_OB(_4DsQEs8zZf901wT1LH); //"the_mutable_set_of_classes"∈mutable_set
  auto paylsetcla = _f.obsetcla->get_dynamic_payload< Rps_PayloadSetOb>();
  RPS_ASSERT(paylsetcla != nullptr);
  std::string perspath = rps_get_loaddir() + "/persistore/sp"
    +  Rps_ObjectRef::root_space()->oid().to_string().c_str() + "-rps.json";
  
  RPS_INFORMOUT("running plugin " << plugin->plugin_name
		<< "setofclasses:" << _f.obsetcla
		<< " loaddir:" << rps_get_loaddir()
		<< " perspath:" << perspath);
  FILE* fsp = fopen(perspath.c_str(), "r");
  RPS_ASSERT(fsp);
  size_t linsiz = 128;
  char*linbuf = (char*) calloc(linsiz, 1);
  RPS_ASSERT(linbuf != nullptr);
  do {
    char idbuf[32];
    memset (idbuf, 0, sizeof(idbuf));
    memset (linbuf, 0, linsiz);
    ssize_t llen = getline(&linbuf,&linsiz, fsp);
    if (llen<0)
      break;
    int endid= -1;
    if (sscanf(linbuf, "//+ob_%30[A-Za-z0-9]%n", idbuf+1, &endid)>0 && endid>0) {
      idbuf[0] = '_';
      Rps_Id oid(idbuf);
      if (oid) {
	_f.obcur = Rps_ObjectRef::find_object_by_oid(&_, oid,
						     Rps_ObjectRef::Rps_Null_When_Missing);
	if (!_f.obcur)
	  continue;
	if (_f.obcur->is_class()) {
	  nbclasses++;
	  paylsetcla->add(_f.obcur);
	  RPS_INFORMOUT("found class object#" << nbclasses
			<< " " << _f.obcur);
	}
      };
    }
  } while (!feof(fsp));
  fclose(fsp);
#warning unimplemented rpsplug_fillmutsetclass
  RPS_WARNOUT("unimplemented rpsplug_fillmutsetclass found " << nbclasses << " RefPerSys classes");
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./do-build-refpersys-plugin plugins/rpsplug_fillmutsetclass.cc -o /tmp/rpsplug_fillmutsetclass.so" ;;
 ** End: ;;
 ****************/

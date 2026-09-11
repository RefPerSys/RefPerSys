 // see http://refpersys.org/
 // passed to commit 3fcae4ba430d235d763d of RefPerSys
 // GPLv3+ licensed
 // © Copyright Basile Starynkevitch <basile@starynkevitch.net>
 // once compiled, use it as:
 /// ./refpersys --plugin-after-load=/tmp/rpsplug_loopreplcmd.so --batch --dump=.


#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obreplcmdcla;
		 Rps_ObjectRef obreplcmddict;
		 Rps_ObjectRef obclassvectval;
		 Rps_ObjectRef obcurcmd;
		 Rps_ObjectRef obname;
		 Rps_Value compv;
		 Rps_Value strname;
		 );
  RPS_INFORMOUT("running plugin " << plugin->plugin_name);
  _f.obreplcmdcla = RPS_ROOT_OB(_8CncrUdoSL303T5lOK); //repl_command∈class
  _f.obreplcmddict = RPS_ROOT_OB(_5dkRQtwGUHs02MVQT0); //"repl_command_dict"∈string_dictionary
  _f.obname = RPS_ROOT_OB(_1EBVGSfW2m200z18rx); //name∈named_attribute
  RPS_INFORMOUT("obreplcmdcla=" << _f.obreplcmdcla
		<< " obreplcmddict=" << _f.obreplcmddict);
  RPS_ASSERT(_f.obreplcmddict);
  Rps_PayloadStringDict*payldict =
    _f.obreplcmddict->get_dynamic_payload<Rps_PayloadStringDict>();
  RPS_ASSERT(payldict);
  //// step 3 of https://framalistes.org/sympa/arc/refpersys-forum/2021-09/msg00013.html
  //// we want to loop on the components of  _f.obreplcmdcla
  RPS_ASSERT(_f.obreplcmdcla);
  int nbreplcmd =  (int) _f.obreplcmdcla->nb_components(&_);
  for (int cix=0; cix<nbreplcmd; cix++) {
    _f.compv =  _f.obreplcmdcla->component_at(&_, cix);
    RPS_INFORMOUT("cmd#" << cix << " = " << _f.compv);
    RPS_ASSERT(_f.compv.is_object());
    _f.obcurcmd = _f.compv.as_object();
    RPS_ASSERT(_f.obcurcmd);
    _f.strname = _f.obcurcmd->get_physical_attr(_f.obname).as_string();
    RPS_INFORMOUT("obcurcmd=" << _f.obcurcmd << " of class "
		  << _f.obcurcmd->get_class()
		  << " named " << _f.strname);
    RPS_ASSERT(_f.strname.is_string());
  }
} // end rps_do_plugin
 
 /****************
  **                           for Emacs...
  ** Local Variables: ;;
  ** compile-command: "g++ -I$HOME/RefPerSys -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -g -shared -fPIC $HOME/tmp/rpsplug_loopreplcmd.cc -o /tmp/rpsplug_loopreplcmd.so" ;;
  ** End: ;;
  ****************/

// see http://refpersys.org/ -*- C++ -*-
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
                           Rps_ObjectRef obsymb;
                           Rps_Value symbv;
                );
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty name or oid");
  _f.obroot = Rps_ObjectRef::find_object_or_null_by_string(&_, plugarg);
  if (!_f.obroot)
    RPS_FATALOUT("plugin rpsplug_removeroot arg=" << plugarg
                 << " dont refer to any existing object");
  std::lock_guard<std::recursive_mutex>
  guroot(*(_f.obroot->objmtxptr()));
  if (_f.obroot->get_space() != Rps_ObjectRef::root_space())
    RPS_FATALOUT("plugin rpsplug_removeroot arg=" << plugarg
                 << " refer to non-root object "
                 << RPS_OBJECT_DISPLAY(_f.obroot));
  if (_f.obroot->is_class())
    {
      Rps_PayloadClassInfo*paylclinf
        = _f.obroot->get_dynamic_payload<Rps_PayloadClassInfo>();
      RPS_ASSERT(paylclinf != nullptr);
      _f.obsymb = paylclinf->symbname();
    }
  else if (_f.obroot->is_instance_of(RPS_ROOT_OB(_36I1BY2NetN03WjrOv)) /*symbol∈class*/)
    {
      _f.obsymb = _f.obroot;
    };
  RPS_INFORMOUT("removing tentative root "
                << RPS_OBJECT_DISPLAY(_f.obroot)
                << std::endl << "❇ symbol:" /*U+2747 SPARKLE */
                << RPS_OBJECT_DISPLAY(_f.obsymb));
  Rps_PayloadSymbol* paylsymb = nullptr;
  if (_f.obsymb)
    {
      std::lock_guard<std::recursive_mutex>
      gusymb(*(_f.obsymb->objmtxptr()));
      paylsymb = _f.obsymb->get_dynamic_payload<Rps_PayloadSymbol>();
      if (paylsymb)
        {
          _f.symbv = paylsymb->symbol_value();
          if (_f.symbv.as_object()== _f.obroot)
            {
              std::string syname = paylsymb->symbol_name();
              paylsymb->symbol_put_value(nullptr);
              paylsymb->set_weak(true);
              if (Rps_PayloadSymbol::forget_name(syname))
                RPS_INFORMOUT("forgot symbol " << _f.obsymb
                              << " named "  << Rps_QuotedC_String(syname));
              else
                RPS_WARNOUT("failed to forget symbol "
                            << _f.obsymb
                            << " named " << Rps_QuotedC_String(syname));
            }
        };
    }
  _f.obroot->put_space(nullptr);
  _f.obsymb->put_space(nullptr);
  RPS_WARNOUT("untested rpsplug_removeroot arg=" << plugarg
              << " obroot=" << _f.obroot
              << " obsymb=" << _f.obsymb);
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

// file RefPerSys/plugins/rpsplug_simpinterp.cc
// SPDX-License-Identifier: GPL-3.0-or-later
// see http://refpersys.org/
// GPLv3+ licensed
// © Copyright 2024 Basile Starynkevitch <basile@starynkevitch.net>

/***
    This plugin contains a simple interpreter.

    Once compiled use it as

  ./refpersys --plugin-after-load=rpsplug_simpinterp.so:$SCRIPTFILE \
              --plugin-arg=rpsplug_simpinterp: \
 ***/

#include "refpersys.hh"

extern "C" char *rpsint_start;
extern "C" char *rpsint_end;

extern "C" void rpsint_parse_script(Rps_CallFrame*cf, Rps_ObjectRef ob);
void
rps_do_plugin(const Rps_Plugin*plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef ob;
		 Rps_Value v1;
                );
  int file_fd = -1;
  size_t file_len = 0;
  _f.ob = nullptr;
  _f.v1 = nullptr;
  errno = 0;
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  if (!plugarg || !plugarg[0])
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument (script file expected)");
  if (access(plugarg, R_OK))
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " with bad script argument " << plugarg
                 << ":" << strerror(errno));
  {
    struct stat plugstat= {};
    if (stat(plugarg,&plugstat))
      RPS_FATALOUT("failure: plugin "  << plugin->plugin_name
                   << " cannot stat script argument " << plugarg
                   << ":" << strerror(errno));
    if ((plugstat.st_mode & S_IFMT) != S_IFREG)
      RPS_FATALOUT("failure: plugin " <<  plugin->plugin_name
                   << " script argument " << plugarg << " not a regular file");
    file_len = plugstat.st_size;
  }
  /// map the file
  file_fd = open(plugarg, O_RDONLY | O_CLOEXEC);
  if (file_fd < 0)
    {
      RPS_FATALOUT("failure: plugin " <<  plugin->plugin_name
                   << " failed to open " << plugarg
                   << ":" << strerror(errno));
    };
  {
    void*ad = mmap(nullptr, file_len, PROT_READ, MAP_SHARED, file_fd, (off_t)0);
    if (ad == MAP_FAILED)
      RPS_FATALOUT("failure: plugin " <<  plugin->plugin_name
                   << " failed to mmap " << plugarg
                   << ":" << strerror(errno));
    rpsint_start = (char*)ad;
    rpsint_end = (char*)ad + file_len;
  };
  _f.ob = Rps_PayloadObjMap::make(&_);
  rpsint_parse_script(&_, _f.ob);
  RPS_WARNOUT("missing code:  plugin " <<  plugin->plugin_name
              << " script " << plugarg << " of " << file_len << " bytes");
#warning a lot of missing code in rpsplug_simpinterp.cc
} // end rps_do_plugin


void
rpsint_parse_script(Rps_CallFrame*cf, Rps_ObjectRef obint)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
		 Rps_ObjectRef obint;
		 );
  _f.obint = obint;
#warning empty rpsint_parse_script
  RPS_WARNOUT("empty rpsint_parse_script obint=" << _f.obint);
} // end rpsint_parse_script

char *rpsint_start;
char *rpsint_end;



////////////////////////////// end of file RefPerSys/plugins/rpsplug_simpinterp.cc

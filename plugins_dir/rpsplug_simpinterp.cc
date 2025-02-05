// file RefPerSys/plugins_dir/rpsplug_simpinterp.cc
// SPDX-License-Identifier: GPL-3.0-or-later
// see http://refpersys.org/
// GPLv3+ licensed
// © Copyright 2024 - 2025 Basile Starynkevitch <basile@starynkevitch.net>

/***
    This plugin contains a simple interpreter. The interpreter's
    syntax -probably inspired by C- has to be defined/approved by
    others. Its role is to provide some way for RefPerSys developers
    to modify the RefPerSys heap and test various modules including
    code generation ones.

    Once compiled use it as

    ./refpersys --plugin-after-load=plugins_dir/rpsplug_simpinterp.so \
    --plugin-arg=rpsplug_simpinterp:$SCRIPT_FILE

    ... and probably other program arguments (e.g. -AREPL)
***/

#include "refpersys.hh"

extern "C" char *rpsint_start;
extern "C" char *rpsint_cur;
extern "C" char *rpsint_end;
extern "C" int rpsint_lineno;


/// on whatsapp March 14 2024 Abhishek approved a C like syntax; we
/// hope for him to be allowed to informally specify it in Feb 2025
// See https://github.com/RefPerSys/RefPerSys/issues/21

#warning TODO: rpsint_parse_script has to return values which would be interpreted by future rpsint_eval_script
extern "C" void rpsint_parse_script(Rps_CallFrame*cf, Rps_ObjectRef ob);
extern "C" void rpsint_skip_spaces(void);
extern "C" bool rpsint_has_keyword(const char*kw);
extern "C" bool rpsint_has_intp(intptr_t* ii);
extern "C" bool rpsint_has_doublep(double* pd);
extern "C" bool rpsint_has_refint(intptr_t& r);
extern "C" bool rpsint_has_refdouble(double& r);

bool
rpsint_has_refint(intptr_t& r)
{
  return rpsint_has_intp(&r);
} // end rpsint_has_refint

bool
rpsint_has_refdouble(double& r)
{
  return rpsint_has_doublep(&r);
} // end rpsint_has_refdouble

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
  RPS_DEBUG_LOG(REPL, "rps_do_plugin " << plugin->plugin_name
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, __FILE__ " plugin"));
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
    rpsint_cur = rpsint_start;
    rpsint_end = (char*)ad + file_len;
  };
  _f.ob = Rps_PayloadObjMap::make(&_);
  rpsint_lineno = 1;
  rpsint_parse_script(&_, _f.ob);
  RPS_WARNOUT("missing code:  plugin " <<  plugin->plugin_name
              << " script " << plugarg << " of " << file_len << " bytes");
#warning a lot of missing code in rpsplug_simpinterp.cc
} // end rps_do_plugin

void
rpsint_skip_spaces(void)
{
  while (rpsint_cur < rpsint_end)
    {
      if (!isspace(*rpsint_cur)) return;
      if (*rpsint_cur=='\n') rpsint_lineno++;
      rpsint_cur++;
    }
} // end rpsint_skip_spaces

void
rpsint_parse_script(Rps_CallFrame*cf, Rps_ObjectRef obint)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/cf,
                           Rps_ObjectRef obint;
                );
  _f.obint = obint;
  rpsint_skip_spaces();
#warning empty rpsint_parse_script
  RPS_WARNOUT("empty rpsint_parse_script obint=" << _f.obint);
} // end rpsint_parse_script

bool
rpsint_has_keyword(const char*kw)
{
  RPS_ASSERT(kw && isalpha(kw[0]));
  rpsint_skip_spaces();
  int kwlen = strlen(kw);
  if (rpsint_cur+kwlen>rpsint_end)
    return false;
  for (int i=0; i<kwlen; i++)
    if (kw[i] != rpsint_cur[i])
      return false;
  if (!isalnum(rpsint_cur[kwlen]) || rpsint_cur[kwlen]=='_')
    return false;
  rpsint_cur += kwlen;
  return true;
} // end rpsint_has_keyword

bool
rpsint_has_intp(intptr_t* pi)
{
  if (!pi) return false;
  rpsint_skip_spaces();
  if (rpsint_cur>rpsint_end)
    return false;
  long l= 0;
  char*end=nullptr;
  l = strtol(rpsint_cur,&end,0);
  if (end>rpsint_cur)
    {
      rpsint_cur += (end-rpsint_cur);
      *pi = (intptr_t)l;
      return true;
    };
  return false;
} // end rpsint_has_intp

bool
rpsint_has_doublep(double* pd)
{
  if (!pd) return false;
  rpsint_skip_spaces();
  if (rpsint_cur>rpsint_end)
    return false;
  double d= 0;
  char*end=nullptr;
  d = strtod(rpsint_cur,&end);
  if (end>rpsint_cur)
    {
      rpsint_cur += (end-rpsint_cur);
      *pd = d;
      return true;
    };
  return false;
} // end rpsint_has_doublep

char *rpsint_start;
char *rpsint_cur;
char *rpsint_end;
int rpsint_lineno;

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_simpinterp.so && /bin/ln -svf $(/bin/pwd)/plugins_dir/rpsplug_simpinterp.so /tmp/" ;;
 ** End: ;;
 ****************/


////////////////////////////// end of file RefPerSys/plugins_dir/rpsplug_simpinterp.cc

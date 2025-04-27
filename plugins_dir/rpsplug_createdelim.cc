// see http://refpersys.org/
// passed to commits after 7d645156f6bb3d  (of April, 26, 2025) of RefPerSys
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>
// This plugin installs a REPL delimiter
/*****
 * Once compiled, use it for example as:
 * ./refpersys --plugin-after-load=/tmp/rpsplug_createdelim.so \
 *             --plugin-arg=rpsplug_createdelim:++ \
 *             --extra=name=plusplus \
 *             --extra=precedence=8 \
 *             --batch --dump=.
 *
 ****/

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewroot;
                           Rps_ObjectRef obclasscommut;
                           Rps_ObjectRef obclassoper;
                           Rps_ObjectRef obclassrepldelim;
                           Rps_ObjectRef obreplprecedence;
                           Rps_Value strname;
                );
#define MYARGMAXLEN 64
  char argcopy[MYARGMAXLEN];
  bool argispunct = false;
  bool argisident = false;
  memset (argcopy, 0, MYARGMAXLEN);
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*xtraname = rps_get_extra_arg("name");
  _f.obclassrepldelim = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
  RPS_ASSERT(_f.obclassrepldelim);
  RPS_ASSERT(_f.obclassrepldelim->is_class());
  if (!plugarg || plugarg[0]==(char)0)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " without argument; should be some non-empty string");
  if (strlen(plugarg) >= MYARGMAXLEN-1)
    RPS_FATALOUT("failure: plugin " << plugin->plugin_name
                 << " with too long argument " << Rps_QuotedC_String(plugarg));
  strncpy(argcopy, plugarg, MYARGMAXLEN);
  if (ispunct(plugarg[0]))
    {
      bool allpunct = true;
      for (const char*pc = plugarg; allpunct && *pc; pc++)
        allpunct = ispunct(*pc);
      argispunct = allpunct;
    };
  if (isalpha(plugarg[0]))
    {
      bool allident = true;
      for (const char*pc = plugarg; allident && *pc; pc++)
        allident = isalnum(*pc) || *pc=='_';
      argisident = allident;
    }
  if (!argispunct && !argisident)
    RPS_FATALOUT("rpsplug_createdelim with bad argument "
                 <<  Rps_QuotedC_String(plugarg)
                 << " not identifier or all-delim");
  RPS_INFORMOUT("running plugin " << plugin->plugin_name << " with argument "
                << Rps_QuotedC_String(plugarg)
                << " and extra name " << Rps_QuotedC_String(xtraname)
                << " and extra precedence " << Rps_QuotedC_String(xtraprecedence));
  /** TODO:
   *
   **/
  RPS_FATALOUT("rpsplug_createdelim not implemented for "
               <<  Rps_QuotedC_String(plugarg));
#warning unimplemented rpsplug_createdelim
} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; ./do-build-refpersys-plugin plugins/rpsplug_createdelim.cc -o /tmp/rpsplug_createdelim.so" ;;
 ** End: ;;
 ****************/

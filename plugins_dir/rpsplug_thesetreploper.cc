// see http://refpersys.org/
// passed to commits after  f7d110da9b (of Aug 10, 2025) of RefPerSys
// GPLv3+ licensed
// © Copyright 2025 Basile Starynkevitch <basile@starynkevitch.net>

// This rpsplug_thesetreploper.cc plugin installs the mutable set of
// REPL operators.

// Conceptually this plugin should be successfully run only once and
// have a dump be done...

#include "refpersys.hh"

void
rps_do_plugin(const Rps_Plugin* plugin)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/nullptr,
                           Rps_ObjectRef obnewsetoper;
                           Rps_Value strname;
                );
#define MYARGMAXLEN 64
  char argcopy[MYARGMAXLEN];
  bool argispunct = false;
  bool argisident = false;
  int precedence = -1;
  memset (argcopy, 0, MYARGMAXLEN);
  const char*plugarg = rps_get_plugin_cstr_argument(plugin);
  const char*xtraname = rps_get_extra_arg("name");
#warning incomplete rpsplug_thesetreploper.cc

  /** TODO: We need to create a single constant object, named
      set_of_repl_operators, whose payload is a mutable set of
      objects.  Once dumped successfully, we need to improve the
      rpsplug_createcommutativeoperator.cc &
      rpsplug_createnoncommutativeoperator.cc */
  RPS_FATALOUT("rpsplug_thesetreploper not implemented for "
               <<  Rps_QuotedC_String(plugarg));

} // end rps_do_plugin

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && ./do-build-refpersys-plugin plugins/rpsplug_thesetreploper.cc -o /tmp/rpsplug_thesetreploper.so" ;;
 ** End: ;;
 ****************/

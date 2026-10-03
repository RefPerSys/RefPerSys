/****************************************************************
 * file progargs_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It inmplements program argument handling
 *
 * Author(s):
 *      Basile Starynkevitch (France) <basile@starynkevitch.net>
 *      Niklas Rozencrantz (Sweden)   <niklasr@protonmail.com>
 *
 * past indian authors (no more interested after summer 2026)
 *      (Abhishek Chakravarti & Nimesh Neema)
 *
 *      © Copyright (C) 2019 - 2026 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include "refpersys.hh"
#include <elf.h>
#include <gelf.h>

extern "C" const char rps_progargs_gitid[];
const char rps_progargs_gitid[]= RPS_GITID;


extern "C" const char rps_progargs_shortgitid[];
const char rps_progargs_shortgitid[]= RPS_SHORTGITID;


extern "C" const char rps_progargs_basename[];
const char rps_progargs_basename[]= RPS_BASENAME;

extern "C" const char rps_progargs_baseid[];
const char rps_progargs_baseid[]= RPS_BASEID;


extern "C" rps_progarg_sig_t rpspa_set_debugging_after_load;
extern "C" rps_progarg_sig_t rpspa_set_debugging_at_start;
extern "C" rps_progarg_sig_t rpspa_debugging_help;
extern "C" rps_progarg_sig_t rpspa_help;
extern "C" rps_progarg_sig_t rpspa_syslog;
extern "C" rps_progarg_sig_t rpspa_version;
////////////////////////////////////////////////////////////////

struct rps_progarg_st
  rps_progarg_array[] =
{
  {
    .prar_str=(const char*)"debug-after-load",
    .prar_letter=(char)'A',
    .prar_argname="DGBFLAGS",
    .prar_expl=(const char*)"comma separated options of debugging names; list them with --debug-help",
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)rpspa_set_debugging_after_load
  },

  {
    .prar_str=(const char*)"debug-at-start",
    .prar_letter=(char)'D',
    .prar_argname="DBGFLAGS",
    .prar_expl=(const char*)"comma separated options of debugging names; list them with --debug-help",
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)rpspa_set_debugging_at_start
  },

  {
    .prar_str=(const char*)"help",
    .prar_letter=(char)'H',
    .prar_argname=nullptr,
    .prar_expl=(const char*)"give help and usage message",
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)rpspa_help
  },

  {
    .prar_str=(const char*)"debug-help",
    .prar_letter=(char)0,
    .prar_argname=nullptr,
    .prar_expl=(const char*)"list possible debugging flags (DBGFLAGS)",
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)rpspa_debugging_help
  },

  {
    .prar_str=(const char*)"syslog",
    .prar_letter=(char)0,
    .prar_argname=nullptr,
    .prar_expl=(const char*)"use syslog(3)",
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)rpspa_syslog
  },

  {
    .prar_str=(const char*)"version",
    .prar_letter=(char)'V',
    .prar_argname=nullptr,
    .prar_expl=(const char*)"give version information",
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)rpspa_version
  },

  {
    .prar_str=(const char*)nullptr,
    .prar_letter=(char)0,
    .prar_argname=nullptr,
    .prar_expl=(const char*)nullptr,
    .prar_data=nullptr,
    .prar_rout=(rps_progarg_sig_t*)nullptr
  }
};        // end rps_progarg_array
////////////////////////////////////////////////////////////////
void
rpspa_set_debugging_after_load (const char*curarg, int& pix, struct rps_progarg_st*prag)
{
#warning unimplemented rpspa_set_debugging_after_load
  RPS_UNIQUE_BREAKPOINT();
  // TODO: call rps_add_debug_cstr appropriately
  RPS_FATALOUT("unimplemented rpspa_set_debugging_after_load curarg=" << curarg
               << " pix=" << pix);
} // end rpspa_set_debugging_after_load

void
rpspa_set_debugging_at_start (const char*curarg, int& pix, struct rps_progarg_st*prag)
{
#warning unimplemented rpspa_set_debugging_at_start
  RPS_UNIQUE_BREAKPOINT();
  // TODO: call rps_add_debug_cstr appropriately
  RPS_FATALOUT("unimplemented rpspa_set_debugging_at_start curarg=" << curarg
               << " pix=" << pix);
} // end rpspa_set_debugging_at_start

void
rpspa_debugging_help (const char*curarg, int& pix, struct rps_progarg_st*prag)
{
#warning unimplemented rpspa_debugging_help
  RPS_UNIQUE_BREAKPOINT();
  RPS_FATALOUT("unimplemented rpspa_debugging_help curarg=" << curarg
               << " pix=" << pix);
} // end rpspa_debugging_help

void
rpspa_help (const char*curarg, int& pix, struct rps_progarg_st*prag)
{
#warning unimplemented rpspa_help
  RPS_UNIQUE_BREAKPOINT();
  RPS_ASSERT(prag && prag->prar_letter == 'H');
  RPS_FATALOUT("unimplemented rpspa_help curarg=" << curarg
               << " pix=" << pix);
} // end rpspa_help

void
rpspa_syslog (const char*curarg, int& pix, struct rps_progarg_st*prag)
{
#warning unimplemented rpspa_syslog
  RPS_UNIQUE_BREAKPOINT();
  RPS_FATALOUT("unimplemented rpspa_syslog curarg=" << curarg
               << " pix=" << pix);
} // end rpspa_syslog

void
rpspa_version (const char*curarg, int& pix, struct rps_progarg_st*prag)
{
  RPS_UNIQUE_BREAKPOINT();
  RPS_ASSERT(prag && prag->prar_letter == 'V');
  rps_show_version();
} // end rpspa_version

////////////////////////////////////////////////////////////////
bool rps_helpwanted;





/// rps_early_initialization is called by rps_parse_program_arguments
/// which is called early from main (before loading of the persistent heap)
static void
rps_early_initialization(int argc, char** argv)
{
  char*inside_emacs =
    getenv("INSIDE_EMACS"); /// GNU emacs is supposed to set this (.emacs)
  rps_argc = argc;
  rps_argv = argv;
  rps_progname = argv[0];
  char cwdbuf[rps_path_byte_size];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  if (argc == 2 && !strcmp(argv[1], "--full-git"))   /// see also rps_parse1opt
    {
      printf("%s\n", rps_gitid);
      fflush(nullptr);
      exit(EXIT_SUCCESS);
    }
  else if (argc == 2 && !strcmp(argv[1], "--short-git"))  /// see also rps_parse1opt
    {
      printf("%s\n", rps_shortgitid);
      fflush(nullptr);
      exit(EXIT_SUCCESS);
    }
  if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
    {
      fprintf(stderr, "%s: failed to getcwd: %s (%d bytes cwdbuf) [%s:%d git %s]\n",
              rps_progname, strerror(errno), (int)sizeof(cwdbuf),
              __FILE__, __LINE__-2, rps_progargs_shortgitid);
      fflush(nullptr);
      exit(EXIT_FAILURE);
    }

  /// dlopen to self
  rps_proghdl = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
  if (!rps_proghdl)
    {
      char *err = dlerror();
      fprintf(stderr, "%s failed to dlopen whole program (%s) in %s [%s:%d git %s]\n",
              rps_progname,
              err, cwdbuf,
              __FILE__, __LINE__-2, rps_progargs_shortgitid);
      syslog(LOG_ERR, "%s failed to dlopen whole program (%s) in %s [%s:%d git %s]\n", rps_progname,
             err, cwdbuf,
             __FILE__, __LINE__-2, rps_progargs_shortgitid);
      exit(EXIT_FAILURE);
    };
  ///
  rps_start_monotonic_time = rps_monotonic_real_time();
  rps_start_wallclock_real_time = rps_wallclock_real_time();
  /// https://man.archlinux.org/man/elf_version.3.en
  {
    unsigned ev = elf_version(EV_CURRENT);
    int l= __LINE__ -1;
    if (ev == EV_NONE)
      {
        int er= errno;
        std::cerr << "RefPerSys git " << RPS_SHORTGITID
                  << " failed to call elf_version in "
                  << __FILE__ << ":" << l
                  << " " << strerror(er) << std::endl;
      }
  }
  errno = 0;
  if (!inside_emacs)
    {
      rps_stdin_istty = isatty(STDIN_FILENO);
      rps_stderr_istty = isatty(STDERR_FILENO);
      rps_stdout_istty = isatty(STDOUT_FILENO);
      std::cout << "RefPerSys outside of EMACS git " << RPS_SHORTGITID
                << ", "<< (rps_stdin_istty?"tty stdin":"plain stdin")
                << ", "<< (rps_stderr_istty?"tty stderr":"plain stderr")
                << ", "<< (rps_stdout_istty?"tty stdout":"plain stdout")
                << ", " << __FILE__ << ":" << __LINE__ << std::endl;
      if (rps_stdin_istty && rps_stdout_istty)
        rps_readline_initialize();
    }
  else   ////// called inside emacs
    {
      rps_stdin_istty = false;  // INSIDE_EMACS
      rps_stderr_istty = false; // INSIDE_EMACS
      rps_stdout_istty = false; // INSIDE_EMACS
      std::cout << "since INSIDE_EMACS is "
                << Rps_QuotedC_String(inside_emacs)
                << " at " __FILE__ ":" << __LINE__ << std::endl
                << " disabling ANSI escapes from " << __FUNCTION__
                << " git " << RPS_SHORTGITID << std::endl;
    };
  if (uname (&rps_utsname))
    {
      fprintf(stderr, "%s: pid %d on %s failed to uname (%s:%d git %s):"
                      " %s\n", rps_progname,
              (int) getpid(), rps_hostname(), __FILE__, __LINE__,
              RPS_SHORTGITID,
              strerror(errno));
      syslog(LOG_ERR,  "%s: pid %d on %s failed to uname (%s:%d git %s):"
                       " %s\n", rps_progname,
             (int) getpid(), rps_hostname(), __FILE__, __LINE__,
             RPS_SHORTGITID,
             strerror(errno));
      exit(EXIT_FAILURE);
    };
  // compute the program invocation string
  rps_compute_program_invocation(argc, (const char**)argv);
  rps_main_thread_handle = pthread_self();
  {
    RPS_ASSERTPRINTF(strlen(cwdbuf)>0, "empty cwdbuf");
    char tmbfr[64];   // the time buffer string
    memset(tmbfr, 0, sizeof (tmbfr));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)) || cwdbuf[0] == (char)0)
      strcpy(cwdbuf, "./");
    rps_now_strftime_centiseconds_nolen(tmbfr, "%Y, %b, %D %H:%M:%S.__ %Z");
    std::cout << std::endl << "** STARTING RefPerSys git "
              << rps_shortgitid << " on " << rps_hostname()
              << " pid#" << getpid() << std::endl
              << " in " << cwdbuf << " at " << tmbfr << std::endl;
  }
  /// handle early a debug flag request
  if (argc > 1
      && !strncmp(argv[1], "--debug=", strlen("--debug=")))
    {
      rps_add_debug_cstr(argv[1]+strlen("--debug="));
    }
  else if (argc > 1 && argv[1][0]=='-' && argv[1][1]=='D')
    {
      rps_add_debug_cstr(argv[1]+2);
    };
  // also use REFPERSYS_DEBUG
  {
    const char*debugenv = getenv("REFPERSYS_DEBUG");
    if (debugenv)
      rps_add_debug_cstr(debugenv);
  }
  // For weird reasons, the program arguments are parsed more than
  // once... We don't care that much in practice...
  RPS_ASSERT(argc>0);
  // we forcibly set the REFPERSYS_PID environment variable
  {
    static char envpid[32];
    if (snprintf(envpid, sizeof(envpid), "REFPERSYS_PID=%d", (int)getpid()) < 1)
      RPS_FATAL("failed to snprintf buffer for REFPERSYS_PID: %m");
    if (putenv(envpid))
      RPS_FATAL("failed to putenv %s %m", envpid);
  }
  /// disable ASLR programmatically if --no-aslr is passed ; this
  /// should ease low-level debugging with GDB
  /// https://en.wikipedia.org/wiki/Address_space_layout_randomization
  /// see https://askubuntu.com/a/507954/64680
  rps_disable_aslr = false;
  {
    for (int ix=1; ix<argc; ix++)
      {
        const char*curarg=argv[ix];
        assert(curarg != nullptr);
        if (curarg[0] != '-') break;
        for (struct rps_progarg_st* pa = rps_progarg_array;
             pa->prar_str || pa->prar_letter;
             pa++)
          {
#warning incomplete code related to rps_progarg_array and argv
            int slen= pa->prar_str?strlen(pa->prar_str):0;
            if ((curarg[0] == '-' && curarg[0] != pa->prar_letter)
                || (curarg[0] == '-' && curarg[1] == '-'
                    && slen>=1 && !strncmp(curarg+2, pa->prar_str, slen)))
              {
                if (pa->prar_argname)
                  {
                  }
                else
                  {
                  };
              };
          };
      }
    if (rps_disable_aslr)
      {
        if (personality(ADDR_NO_RANDOMIZE) == -1)
          RPS_FATAL("%s failed to disable ASLR: %m", rps_progname);
        else
          RPS_INFORM("%s disabled ASLR (git %s).", rps_progname, rps_gitid);
      }
  }
  Rps_Agenda::initialize();
  unsetenv("LANG");
  unsetenv("LC_ADDRESS");
  unsetenv("LC_ALL");
  unsetenv("LC_IDENTIFICATION");
  unsetenv("LC_MEASUREMENT");
  unsetenv("LC_MONETARY");
  unsetenv("LC_NAME");
  unsetenv("LC_NUMERIC");
  unsetenv("LC_NUMERIC");
  unsetenv("LC_PAPER");
  unsetenv("LC_TELEPHONE");
  unsetenv("LC_TIME");
  setenv("LANG", "C", (int)true);
  setenv("LC_ALL", "C.UTF-8", (int)true);
  std::setlocale(LC_ALL, "C.UTF-8");
  rps_backtrace_common_state =
    backtrace_create_state(rps_progname, (int)true,
                           Rps_Backtracer::bt_error_cb,
                           nullptr);
  if (!rps_backtrace_common_state)
    {
      fprintf(stderr, "%s failed to make backtrace state.\n", rps_progname);
      exit(EXIT_FAILURE);
    }
  pthread_setname_np(pthread_self(), "rps-main");
  // hack to handle debug flag as first program argument
  if (argc>1 && !strncmp(argv[1], "--debug=", strlen("--debug=")))
    rps_add_debug_cstr((argv[1]+strlen("--debug=")));
  if (argc>1 && !strncmp(argv[1], "-d", strlen("-d")))
    rps_add_debug_cstr((argv[1]+strlen("-d")));
  ///
  if (rps_syslog_enabled)
    {
      openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
      if (rps_debug_flags != 0)
        syslog(LOG_USER|LOG_INFO,
               "start of refpersys inference engine git %s (on %s) debug %s",
               rps_shortgitid, rps_hostname(),
               rps_debug_level_string(rps_debug_flags.load()).c_str());
      else
        syslog(LOG_USER|LOG_INFO,
               "start of refpersys inference engine git %s (on %s) without debug",
               rps_shortgitid, rps_hostname());
    };
  RPS_FATALOUT("broken early initialization of RefPerSys process "
               << rps_decimal_string((int)getpid())
               << " on host " << rps_hostname()
               << " git " << rps_shortgitid);
} // end rps_early_initialization

extern "C" struct argp_option rps_progoptions[];
// rps_parse_program_arguments is called very early from main...
void
rps_parse_program_arguments(int &argc, char**argv)
{
#warning rps_parse_program_arguments is broken
  RPS_UNIQUE_BREAKPOINT();
  errno = 0;
  rps_early_initialization  (argc, argv);
  errno = 0;
  struct argp_state argstate;
  memset (&argstate, 0, sizeof(argstate));
#if 0 && oldcode
  argparser_rps.options = rps_progoptions; // defined in main_rps.cc
  argparser_rps.parser = rps_parse1opt;
  argparser_rps.args_doc = " ; # ";
  argparser_rps.doc =
    "RefPerSys - an opensource Artificial Intelligence inference engine project,\n"
    " open science, for Linux/x86-64; see refpersys.org for more.\n"
    " (REFlexive PERsystem SYStem is GPLv3+ licensed free software)\n"
    " You should have received a copy of the GNU General Public License\n"
    " along with this program.  If not, see www.gnu.org/licenses\n"
    " *** NO WARRANTY, not even for FITNESS FOR A PARTICULAR PURPOSE ***\n"
    " +++!!! use at your own risk !!!+++\n"
    " (shortgitid " RPS_SHORTGITID ")\n"
    "\n Accepted program options are:\n";
  argparser_rps.children = nullptr;
  argparser_rps.help_filter = nullptr;
  argparser_rps.argp_domain = nullptr;
  int aix= -1;
  if (argp_parse(&argparser_rps, argc, argv, 0, &aix, nullptr))
    RPS_FATALOUT("failed to parse program arguments to " << argv[0]
                 << " at program argument index aix=" << aix);
  if (rps_helpwanted)
    {
      RPS_UNIQUE_BREAKPOINT();
      std::cout << "*** debug level flag in C++ code ***" << std::endl;
      std::cout << "# levelname | explanation" << std::endl;
      char levbuf[80];
#define Rps_Explain_Level_Help(Level,Str) do {          \
    memset(levbuf, 0, sizeof(levbuf));                  \
    snprintf(levbuf, sizeof(levbuf), "\t %14s # %s",    \
             #Level, Str);                              \
    std::cout << levbuf << std::endl;                   \
  } while(0);
      RPS_DEBUG_OPTIONS(Rps_Explain_Level_Help);
#undef Rps_Explain_Level_Help
    } // end if rps_helpwanted
  RPS_POSSIBLE_BREAKPOINT();
#endif /* 0 && oldcode */
} // end rps_parse_program_arguments



/// most of the time this function is used thru RPS_OUT_PROGARGS macro
void
rps_output_program_arguments(std::ostream& out, int argc,
                             const char*const*argv)
{
  if (argc<0)
    {
      argc = rps_main_argc;
      argv = rps_main_argv;
    };
  for (int i=0; i<argc; i++)
    {
      if (i>0) out << ' ';
      const char*curparg = argv[i];
      if (!curparg)
        break;
      bool goodchar = true;
      for (const char* pc = curparg; goodchar && *pc; pc++)
        {
          if (isalnum(*pc) || *pc=='_' || *pc=='-' || *pc=='+'
              || *pc=='/' || *pc=='.' || *pc==',' || *pc==':'
              || *pc=='=' || *pc=='%' || *pc=='@')
            continue;
          else
            {
              goodchar = false;
              RPS_POSSIBLE_BREAKPOINT();
              break;
            }
        };
      if (goodchar)
        out << curparg;
      else
        out << Rps_QuotedC_String(curparg);
    };
  out << std::endl;
} // end rps_output_program_arguments

void
rps_compute_program_invocation(int argc, const char**argv)
{
  std::ostringstream outs;
  rps_output_program_arguments(outs, argc, argv);
  outs.flush();
  std::string pstr = outs.str();
  size_t plen = pstr.size();
  rps_program_invocation = (char*)calloc(1, ((plen+20)|0x1f)+1);
  if (rps_program_invocation)
    strncpy(rps_program_invocation, pstr.c_str(), plen);
} // end rps_compute_program_invocation

#if 0 && oldcode
/// Keep the options in alphabetical order of the name
struct argp_option rps_progoptions[] =
{

  /* ======= batch ======= */
  {/*name:*/ "batch", ///
    /*key:*/ RPSPROGOPT_BATCH, ///   -B
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run in batch mode, that is without any user interface "
    "(either graphical or command-line REPL).\n", //
    /*group:*/0 ///
  },
  /* ======= run a REPL command after load ======= */
  {/*name:*/ "command", ///
    /*key:*/ RPSPROGOPT_COMMAND, ///   -c
    /*arg:*/ "REPL_COMMAND", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run the given REPL_COMMAND;\n"
    "Try the help command for details.\n", //
    /*group:*/0 ///
  },
  /* ======= edit the C++ code of  a temporary plugin after load ======= */
  {/*name:*/ "cplusplus-editor-after-load", ///
    /*key:*/ RPSPROGOPT_CPLUSPLUSEDITOR_AFTER_LOAD, ///
    /*arg:*/ "EDITOR", ///
    /*flags:*/ 0, ///
    /*doc:*/ "prefill some C++ temporary file for plugin code,\n"
    " edit it with given EDITOR, then compile it"
    " and run its " RPS_PLUGIN_INIT_NAME "(const Rps_Plugin*) function.\n"
    " (if none is given, use $EDITOR from environment)\n"
    , //
    /*group:*/0 ///
  },
  /* ======= extra compilation flags for C++ code above after load ======= */
  {/*name:*/ "cplusplus-flags-after-load", ///
    /*key:*/ RPSPROGOPT_CPLUSPLUSFLAGS_AFTER_LOAD, ///
    /*arg:*/ "FLAGS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "set to FLAGS the extra compilation flags for the C++ code of the temporary plugin.\n", //
    /*group:*/0 ///
  },
  /* ======= debug flags ======= */
  {/*name:*/ "debug", ///
    /*key:*/ RPSPROGOPT_DEBUG, ///  -d
    /*arg:*/ "DEBUGFLAGS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "To set RefPerSys comma separated debug flags, pass --debug=help to get their list.\n"
    " Also from $REFPERSYS_DEBUG environment variable, if provided\n", ///
    /*group:*/0 ///
  },
  /* ======= debug  flags ======= */
  {/*name:*/ "debug-exit", ///
    /*key:*/ RPSPROGOPT_DEBUG_EXIT, /// --debug-exit=
    /*arg:*/ "DEBUGFLAGS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "To set RefPerSys debug flags just before exiting.\n", ///
    /*group:*/0 ///
  },
  /* ======= debug after load flags ======= */
  {/*name:*/ "debug-after-load", ///
    /*key:*/ RPSPROGOPT_DEBUG_AFTER_LOAD, /// -A
    /*arg:*/ "DEBUGFLAGS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "To set RefPerSys comma separated debug flags after the sucessful load.\n", ///
    /*group:*/0 ///
  },
  /* ======= debug file path ======= */
  {/*name:*/ "debug-path", ///
    /*key:*/ RPSPROGOPT_DEBUG_PATH, ///
    /*arg:*/ "DEBUGFILEPATH", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Output debug messages into given DEBUGFILEPATH instead of stderr.\n", ///
    /*group:*/0 ///
  },
  /* ======= dump into given directory ======= */
  {/*name:*/ "dump", ///
    /*key:*/ RPSPROGOPT_DUMP, ///   -D
    /*arg:*/ "DUMPDIR", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Dump the persistent state to given DUMPDIR directory.\n", ///
    /*group:*/0 ///
  },
  /* ======= extra argument ======= */
  {/*name:*/ "extra", ///
    /*key:*/ RPSPROGOPT_EXTRA_ARG, ///
    /*arg:*/ "EXTRA=ARG", /// -X
    /*flags:*/ 0, ///
    /*doc:*/ "To set for RefPerSys a named EXTRA argument to ARG.\n", ///
    /*group:*/0 ///
  },
  /* ======= script file ======= */
  {/*name:*/ "script", ///
    /*key:*/ RPSPROGOPT_SCRIPT, ///
    /*arg:*/ "SCRIPTFILE", ///
    /*flags:*/ 0, ///
    /*doc:*/ "To run the given SCRIPTFILE after loading.\n"
    " use --script=help to get more help about them\n", ///
    /*group:*/0 ///
  },
  /* ======= interface thru some FIFO, relevant for JSONRPC  ======= */
  {/*name:*/ "interface-fifo", ///
    /*key:*/ RPSPROGOPT_INTERFACEFIFO, ///
    /*arg:*/ "FIFO", ///
    /*flags:*/ 0, ///
    /*doc:*/ "use a pair of fifo(7) named FIFO.cmd (written) "
    "and FIFO.out (read) for JSONRPC communication between RefPerSys"
    " and some graphical user interface.  So when RefPerSys is given"
    " the program argument --interface-fifo=/tmp/chan, two"
    " FIFO channels may be created, and are used: /tmp/chan.out"
    " and /tmp/chan.cmd ... The /tmp/chan.out is written"
    " by the GUI interface, and read by RefPerSys; the "
    "/tmp/chan.cmd is read by the GUI interface, and written by "
    "the RefPerSys process.\n"
    , //
    /*group:*/0 ///
  },
  /* ======= number of jobs or threads ======= */
  {/*name:*/ "jobs", ///
    /*key:*/ RPSPROGOPT_JOBS, ///
    /*arg:*/ "NBJOBS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run <NBJOBS> threads - default is 5, minimum 3, maximum 24.\n",
    // see RPS_NBJOBS_MIN and RPS_NBJOBS_MAX in refpersys.hh and initial value below.
    /*group:*/0 ///
  },
  /* ======= the load directory ======= */
  {/*name:*/ "load", ///
    /*key:*/ RPSPROGOPT_LOADDIR, ///
    /*arg:*/ "LOADDIR", ///
    /*flags:*/ 0, ///
    /*doc:*/ "loads persistent state from LOADDIR, defaults to the source directory", ///
    /*group:*/0 ///
  },
  /* ======= without ASLR ; perhaps might not work in feb. 2023 ======= */
  {/*name:*/ "no-aslr", ///
    /*key:*/ RPSPROGOPT_NO_ASLR, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Forcibly disable Adress Space Layout Randomization."
    " Might not work.\n", //
    /*group:*/0 ///
  },
  /* ======= display the full git id ======= */
  {/*name:*/ "full-git", ///
    /*key:*/ RPSPROGOPT_FULL_GIT, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Output just the full gitid of the binary\n"
    " (suffixed by + if locally changed)\n", //
    /*group:*/0 ///
  },
  /* ======= display the short suffixed git id ======= */
  {/*name:*/ "short-git", ///
    /*key:*/ RPSPROGOPT_SHORT_GIT, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Output just the short gitid of the binary\n"
    " (suffixed by + if locally changed)\n", //
    /*group:*/0 ///
  },
  /* ======= without quick tests ======= */
  {/*name:*/ "no-quick-tests", ///
    /*key:*/ RPSPROGOPT_NO_QUICK_TESTS, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Disable quick tests after load by rps_small_quick_tests_after_load.\n", //
    /*group:*/0 ///
  },
  /* ======= without terminal ======= */
  {/*name:*/ "no-terminal", ///
    /*key:*/ RPSPROGOPT_NO_TERMINAL, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Forcibly disable terminal ANSI escape codes, even if stdout is a tty.\n", //
    /*group:*/0 ///
  },
  /* ======= set the locale for messages ======= */
  {/*name:*/ "locale", ///
    /*key:*/ RPSPROGOPT_LOCALE, ///
    /*arg:*/ "LOCALE", ///
    /*flags:*/ 0, ///
    /*doc:*/ "set the locale for internationlization of messages",
    /*group:*/0 ///
  },
  /* ======= dlopen a given plugin file after load ======= */
  {/*name:*/ "plugin-after-load", ///
    /*key:*/ RPSPROGOPT_PLUGIN_AFTER_LOAD, ///
    /*arg:*/ "PLUGIN", ///
    /*flags:*/ 0, ///
    /*doc:*/ "dlopen(3) after load the given PLUGIN "
    "(some *.so ELF shared object)"
    " and run its " RPS_PLUGIN_INIT_NAME "(const Rps_Plugin*) function.\n", //
    /*group:*/0 ///
  },
  /* ======= string argument to a previously given plugin file after load ======= */
  {/*name:*/ "plugin-arg", ///
    /*key:*/ RPSPROGOPT_PLUGIN_ARG, ///
    /*arg:*/ "PLUGIN_NAME:PLUGIN_ARG", ///
    /*flags:*/ 0, ///
    /*doc:*/ "pass to the loaded plugin <PLUGIN_NAME> the string <PLUGIN_ARG> "
    "(notice the colon separating them).\n", //
    /*group:*/0 ///
  },
  /* ======= string argument to a previously given plugin file after load ======= */
  {/*name:*/ "interactive-plugin-arg", ///
    /*key:*/ RPSPROGOPT_INTERACTIVE_PLUGIN_ARG, ///
    /*arg:*/ "INTERACT_PLUGIN_ARG", ///
    /*flags:*/ 0, ///
    /*doc:*/ "pass to the unique interactive plugin the <INTERACT_PLUGIN_ARG>\n",
    /*group:*/0 ///
  },
  /* ======= dlopen a unique interactive plugin file after load ======= */
  {/*name:*/ "interactive-plugin", ///
    /*key:*/ RPSPROGOPT_INTERACTIVE_PLUGIN_AFTER_LOAD, ///
    /*arg:*/ "INTERACT_PLUGIN", ///
    /*flags:*/ 0, ///
    /*doc:*/ "dlopen(3) after load the given interactive INTERACT_PLUGIN\n"
    "(some *.so ELF shared object)"
    " and run its " RPS_INTERACTIVE_PLUGIN_INIT_NAME "(const Rps_Plugin*) function.\n", //
    /*group:*/0 ///
  },
  /* ====== after loading heap & plugins, show help about preferences
     ===== */
  {/*name:*/ "preferences-help", ///
    /*key:*/ RPSPROGOPT_PREFERENCES_HELP, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "After loading heap and plugins, show help \n"
    "about user preferences (given in the preferences file)\n"
    , //
    /*group:*/0 ///
  },
  /* ====== publish some data to a remote URL and Web service which
     might make some statistics about RefPerSys ===== */
  {/*name:*/ "publish-me", ///
    /*key:*/ RPSPROGOPT_PUBLISH_ME, ///
    /*arg:*/ "URL", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Send to the given URL the build timestamp and builder.\n"
    " See rps_publish_me function in curl_rps.cc source file.\n"
    , //
    /*group:*/0 ///
  },
  /* ======= random oids ======= */
  {/*name:*/ "random-oid", ///
    /*key:*/ RPSPROGOPT_RANDOMOID, ///
    /*arg:*/ "NBOIDS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Print NBOIDS random object identifiers.\n",
    /*group:*/0 ///
  },
  /* ======= the RefPerSys home directory ======= */
  {/*name:*/ "refpersys-home", ///
    /*key:*/ RPSPROGOPT_HOMEDIR, ///
    /*arg:*/ "HOMEDIR", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Set the RefPerSys homedir, default to "
    "$REFPERSYS_HOME or $HOME\n", ///
    /*group:*/0 ///
  },
  /* ======= change current directory before loading ======= */
  {/*name:*/ "chdir-before-load", ///
    /*key:*/ RPSPROGOPT_CHDIR_BEFORE_LOAD, ///
    /*arg:*/ "DIRECTORY", ///
    /*flags:*/ 0, ///
    /*doc:*/ "change directory before loading to $DIRECTORY\n", ///
    /*group:*/0 ///
  },
  /* ======= change current directory after loading ======= */
  {/*name:*/ "chdir-after-load", ///
    /*key:*/ RPSPROGOPT_CHDIR_AFTER_LOAD, ///
    /*arg:*/ "DIRECTORY", ///
    /*flags:*/ 0, ///
    /*doc:*/ "change directory after loading to $DIRECTORY\n", ///
    /*group:*/0 ///
  },
  /* ======= Run RefPerSys for a limited time ======= */
  {/*name:*/ "run-delay", ///
    /*key:*/ RPSPROGOPT_RUN_DELAY, ///
    /*arg:*/ "RUNDELAY", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run RefPerSys agenda and event loop for a limited real time,\n"
    " e.g. --run-delay=50s or --run-delay=2m or --run-delay=5h\n\n", ///
    /*group:*/0 ///
  },
  /* ======= run a shell command with system(3) after load ======= */
  {/*name:*/ "run-after-load", ///
    /*key:*/ RPSPROGOPT_RUN_AFTER_LOAD, ///
    /*arg:*/ "SHELL_COMMAND", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run using system(3) the given shell SHELL_COMMAND after load and plugins;\n" //
    " The following environment variables have been set:\n" //
    "\t  * $REFPERSYS_GITID to the git id (with a + suffix if locally changed);\n" //
    "\t  * $REFPERSYS_TOPDIR to the top directory with source code and persistore/ ...;\n" //
    "\t  * $REFPERSYS_PID to the process id running the refpersys executable;\n" //
    "\t  * $REFPERSYS_USER_OID to the objectid corresponding to current user;\n" //
    "\n\n",
    /*group:*/0 ///
  },
  /* ======= syslog-ing ======= */
  {/*name:*/ "syslog", ///
    /*key:*/ RPSPROGOPT_SYSLOG, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Use system log with syslog(3) ...\n", //
    /*group:*/0 ///
  },
  /* ======= naming the run ======= */
  {/*name:*/ "run-name", ///
    /*key:*/ RPSPROGOPT_RUN_NAME, ///
    /*arg:*/ "RUN_NAME", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Set the name of this run to given RUN_NAME ...\n", //
    /*group:*/0 ///
  },
  /* ======= showing some message ======= */
  {/*name:*/ "echo", ///
    /*key:*/ RPSPROGOPT_ECHO, ///
    /*arg:*/ "MESSAGE", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Show the given MESSAGE when parsing program argument ...\n", //
    /*group:*/0 ///
  },
  /* ======= daemoning ======= */
  {/*name:*/ "daemon", ///
    /*key:*/ RPSPROGOPT_DAEMON, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Use daemon(3) ...\n", //
    /*group:*/0 ///
  },
  /* ======= test the read-eval-print lexer on string ======== */
  {/*name:*/ "test-repl-lexer", ///
    /*key:*/ RPSPROGOPT_TEST_REPL_LEXER, ///
    /*arg:*/ "TESTSTRING", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Test the read-eval-print-loop lexer on given TESTSTRING.\n"
    " [may become obsolete, see also --file-repl-lexer=… ]\n", //
    /*group:*/0 ///
  },
  /* ======= test the read-eval-print lexer on file or pipe ======== */
  {/*name:*/ "file-repl-lexer", ///
    /*key:*/ RPSPROGOPT_FILE_REPL_LEXER, ///
    /*arg:*/ "TESTFILE", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Test the REPL lexer on given <TESTFILE> file\n"
    " (or pipe if starting with | or !)\n"
    " [may become obsolete, see also --test-repl-lexer=… ]\n", //
    /*group:*/0 ///
  },
  /* ======= type information ======= */
  {/*name:*/ "type-info", ///
    /*key:*/ RPSPROGOPT_TYPEINFO, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Show type information (and test tagged integers).\n" //
    " (using rps_print_types_info from utilities_rps.cc)\n",
    /*group:*/0 ///
  },
  /* ======= pid-file ======= */
  {/*name:*/ "pid-file", ///
    /*key:*/ RPSPROGOPT_PID_FILE, ///
    /*arg:*/ "PID_FILE", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Write the pid of the running process into given PID_FILE.\n", //
    /*group:*/0 ///
  },
  /* ======= user preferences ======= */
  {/*name:*/ "user-pref", ///
    /*key:*/ RPSPROGOPT_USER_PREFERENCES, ///
    /*arg:*/ "USER_PREF", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Set the\n"
    "user preferences to given\n"
    "USER_PREF file; Lines there starting with # are comments.\n"
    "Lines before the first *REFPERSYS_USER_PREFERENCES are ignored.\n"
    "\t So they could be some shell script....\n"
    "See also --preferences-help option.\n"
    "The format is en.wikipedia.org/wiki/INI_file with named values...\n"
    "The preferences file has sections starting\n"
    "with [secname]. Others are <name>=<value>, e.g.\n"
    " color='black' or height=345 ...\n"
    "\nDefault preference file is"
    " $HOME/" REFPERSYS_DEFAULT_PREFERENCE_PATH "\n"
    "Using . or / as a preference file is not having any.\n"
    , //
    /*group:*/0 ///
  },
  /* ======= version info ======= */
  {/*name:*/ "version", ///
    /*key:*/ RPSPROGOPT_VERSION, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Show version information, then exit.\n", //
    /*group:*/0 ///
  },
  /* ======= terminating empty option ======= */
  {/*name:*/(const char*)0, ///
    /*key:*/0, ///
    /*arg:*/(const char*)0, ///
    /*flags:*/0, ///
    /*doc:*/(const char*)0, ///
    /*group:*/0 ///
  }
};
#endif /*0 && oldcode*/

//// end of file RefPerSys/progargs_rps.cc

/****************************************************************
 * file main_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the main function and related, program option parsing,
 *      code.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2025 The Reflective Persistent System Team
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


extern "C" const char rps_main_gitid[];
const char rps_main_gitid[]= RPS_GITID;

extern "C" const char rps_main_date[];
const char rps_main_date[]= __DATE__;

extern "C" const char rps_main_shortgitid[];
const char rps_main_shortgitid[]= RPS_SHORTGITID;

extern "C" char rps_buffer_proc_version[];
char rps_buffer_proc_version[rps_path_byte_size];

#pragma message "Compiling " __FILE__ " on operating system " RPS_OPERSYS " for architecture " RPS_ARCH

#if RPS_HAS_OPERSYS_GNU_Linux
#pragma message "Compiling " __FILE__ " on GNU/Linux"
#endif

#if RPS_HAS_ARCH_86_64
#pragma message "Compiling " __FILE__ " for 64 bits x86"
#endif

struct utsname rps_utsname;

char rps_progexe[rps_path_byte_size];

static std::atomic<std::uint8_t> rps_exit_atomic_code;

extern "C" char*rps_chdir_path_after_load;

char*rps_chdir_path_after_load;
std::vector<Rps_Plugin> rps_plugins_vector;
std::map<std::string,std::string> rps_pluginargs_map;
extern "C" char*rps_pidfile_path;


std::string rps_cpluspluseditor_str;
std::string rps_cplusplusflags_str;
std::string rps_dumpdir_str;
std::vector<std::string> rps_command_vec;
std::string rps_test_repl_string;
char*rps_pidfile_path;
/// the … is unicode U+2026 HORIZONTAL ELLIPSIS in UTF8 \xe2\x80\xA6


extern "C" std::atomic<long> rps_debug_atomic_counter;
std::atomic<long> rps_debug_atomic_counter;

const char* rps_get_proc_version(void)
{
  return rps_buffer_proc_version;
} // end rps_get_proc_version

long
rps_incremented_debug_counter(void)
{
  return 1+rps_debug_atomic_counter.fetch_add(1);
} // end rps_incremented_debug_counter

long
rps_debug_counter(void)
{
  return rps_debug_atomic_counter.load();
} // end rps_debug_counter

static void rps_kill_wait_gui_process(void);

error_t rps_parse1opt (int key, char *arg, struct argp_state *state);


/// Keep the options in alphabetical order of the name
struct argp_option rps_progoptions[] =
{

  /* ======= batch ======= */
  {/*name:*/ "batch", ///
    /*key:*/ RPSPROGOPT_BATCH, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run in batch mode, that is without any user interface "
    "(either graphical or command-line REPL).\n", //
    /*group:*/0 ///
  },
  /* ======= run a REPL command after load ======= */
  {/*name:*/ "command", ///
    /*key:*/ RPSPROGOPT_COMMAND, ///
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
    /*key:*/ RPSPROGOPT_DEBUG, ///
    /*arg:*/ "DEBUGFLAGS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "To set RefPerSys comma separated debug flags, pass --debug=help to get their list.\n"
    " Also from $REFPERSYS_DEBUG environment variable, if provided\n", ///
    /*group:*/0 ///
  },
  /* ======= debug after load flags ======= */
  {/*name:*/ "debug-after-load", ///
    /*key:*/ RPSPROGOPT_DEBUG_AFTER_LOAD, ///
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
    /*key:*/ RPSPROGOPT_DUMP, ///
    /*arg:*/ "DUMPDIR", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Dump the persistent state to given DUMPDIR directory.\n", ///
    /*group:*/0 ///
  },
  /* ======= FLTK GUI library ======= */
  {/*name:*/ "fltk", ///
    /*key:*/ RPSPROGOPT_FLTK, ///
    /*arg:*/ "GUIPREFERENCES", ///
    /*flags:*/ OPTION_ARG_OPTIONAL, ///
    /*doc:*/ "pass, if GUIPREFERENCES is given,"
    " to FLTK graphical library; enable FLTK graphics.\n"
    "\t see fltk.org for details\n", ///
    /*group:*/0 ///
  },
  /* ======= extra argument ======= */
  {/*name:*/ "extra", ///
    /*key:*/ RPSPROGOPT_EXTRA_ARG, ///
    /*arg:*/ "EXTRA=ARG", ///
    /*flags:*/ 0, ///
    /*doc:*/ "To set for RefPerSys a named EXTRA argument to ARG.\n", ///
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
    /*doc:*/ "Forcibly disable Adress Space Layout Randomization. Might not work.\n", //
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
  /* ======= test the read-eval-print lexer ======== */
  {/*name:*/ "test-repl-lexer", ///
    /*key:*/ RPSPROGOPT_TEST_REPL_LEXER, ///
    /*arg:*/ "TESTLEXSTRING", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Test the read-eval-print-loop lexer on given TESTLEXSTRING."
    " (this option might become obsolete).\n", //
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
    /*doc:*/ "Set the user preferences to given\n"
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



struct backtrace_state* rps_backtrace_common_state;
const char* rps_progname;

int rps_argc;
char** rps_argv;
char* rps_program_invocation;



char* rps_run_command_after_load = nullptr;
char* rps_debugflags_after_load = nullptr;

std::vector<std::function<void(Rps_CallFrame*)>> rps_do_after_load_vect;

void* rps_proghdl = nullptr;

bool rps_batch = false;
bool rps_disable_aslr = false;
bool rps_without_terminal_escape = false;
bool rps_daemonized = false;
bool rps_without_quick_tests = false;
bool rps_test_repl_lexer = false;
bool rps_syslog_enabled = false;
bool rps_stdout_istty = false;
bool rps_stderr_istty = false;

std::atomic<unsigned> rps_debug_flags;

FILE* rps_debug_file;
pid_t rps_gui_pid;

thread_local Rps_Random Rps_Random::_rand_thr_;

typedef std::function<void(void)> rps_todo_func_t;
static std::vector<rps_todo_func_t> rps_main_todo_vect;

std::string rps_my_load_dir;


unsigned rps_call_frame_depth(const Rps_CallFrame*callframe)
{
  if (callframe==nullptr) return 0;
  else
    return callframe->call_frame_depth();
} // end rps_call_frame_depth




void
rps_set_exit_code(std::uint8_t ex)
{
  rps_exit_atomic_code.store(ex);
} // end rps_set_exit_code


int rps_nbjobs = RPS_NBJOBS_MIN + 2;


/// the rps_run_loaded_application is called after loading...
void
rps_run_loaded_application(int &argc, char **argv)
{

  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED, //
                 /*callerframe:*/RPS_NULL_CALL_FRAME,
                 Rps_ObjectRef tempob;
                );
  {
    char cwdbuf[128];
    memset (cwdbuf, 0, sizeof(cwdbuf));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
      RPS_FATALOUT("rps_run_loaded_application failed to getcwd " << strerror(errno)
                   << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application"));
    RPS_INFORM("rps_run_loaded_application: start of %s (with %d args)\n"
               ".. gitid %s version %d.%d\n"
               ".. build timestamp %s\n"
               ".. last git commit %s\n"
               ".. md5sum %s\n"
               ".. in %s\n"
               ".. on host %s pid %d\n",
               argv[0], argc, rps_gitid, rps_get_major_version(), rps_get_minor_version(),
               rps_timestamp,
               rps_lastgitcommit,
               rps_md5sum,
               cwdbuf,
               rps_hostname(), (int)getpid());
  }
  /// if told, enable extra debugging after load
  if (rps_debugflags_after_load)
    {
      rps_add_debug_cstr(rps_debugflags_after_load);
      RPS_INFORMOUT("did set after load "
                    << " of RefPerSys process " << (int)getpid() << std::endl
                    << "… on " << rps_hostname()
                    << " shortgit " << rps_shortgitid << " version " << rps_get_major_version() << "." << rps_get_minor_version()
                    << " debug to "
                    << Rps_Do_Output([&](std::ostream& out)
      {
        rps_output_debug_flags(out);
      }));
    }
  ////
  if (rps_without_quick_tests)
    {
      RPS_INFORM("rps_run_loaded_application dont run quick tests after load");
    }
  else
    {
      RPS_DEBUG_LOG(LOWREP, "rps_run_loaded_application before running rps_small_quick_tests_after_load from "
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application/quick-tests")
                    << std::endl
                    << " with call frame:" << std::endl
                    << Rps_ShowCallFrame(&_));
      rps_small_quick_tests_after_load();
      RPS_DEBUG_LOG(LOWREP, "rps_run_loaded_application after running rps_small_quick_tests_after_load");
    };
  /// create the fifos if a prefix is given with
  if (!rps_get_fifo_prefix().empty())
    {
      RPS_DEBUG_LOG(REPL, "rps_run_loaded_application create fifo with prefix "
                    << rps_get_fifo_prefix());
      rps_do_create_fifos_from_prefix();
    }
  RPS_DEBUG_LOG(REPL, "rps_run_loaded_application after load & fifos"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application"));
  //// running the given Unix command after load
  if (rps_run_command_after_load)
    {
#warning TODO: this needs a code review
      if (rps_get_fifo_prefix().empty())
        RPS_INFORM("before running command '%s' after load with environment variables...\n"
                   " REFPERSYS_PID=%ld, REFPERSYS_GITID=%s, REFPERSYS_TOPDIR=%s",
                   rps_run_command_after_load, (long)getpid(), rps_gitid, rps_topdirectory);
      else
        RPS_INFORM("before running command '%s' after load with environment variables...\n"
                   "… REFPERSYS_PID=%ld, REFPERSYS_GITID=%s,\n"
                   "… REFPERSYS_TOPDIR=%s, REFPERSYS_FIFO_PREFIX=%s",
                   rps_run_command_after_load, (long)getpid(), rps_gitid,
                   rps_topdirectory, rps_get_fifo_prefix().c_str());
      fflush(nullptr); /// needed before system
      int nok = system(rps_run_command_after_load);
      if (nok)
        RPS_FATAL("failed to run command '%s' after load (status #%d)",
                  rps_run_command_after_load, nok);
      else
        RPS_INFORM("after successfully running command '%s' after load", rps_run_command_after_load);
    }
  else if (!rps_get_fifo_prefix().empty())
    {
      RPS_INFORM("before running default GUI command '%s'  after load"
                 " with environment variables...\n"
                 "… REFPERSYS_PID=%ld, REFPERSYS_GITID=%s,\n"
                 " ... REFPERSYS_TOPDIR=%s, REFPERSYS_FIFO_PREFIX=%s",
                 rps_gui_script_executable, (long)getpid(), rps_gitid,
                 rps_topdirectory, rps_get_fifo_prefix().c_str());
      std::string fifo_cmd_path, fifo_out_path;
      fifo_cmd_path = rps_get_fifo_prefix() + ".cmd";
      fifo_out_path = rps_get_fifo_prefix() + ".out";
      /* create the fifo if they dont exist */
      if (!rps_is_fifo(fifo_cmd_path) || !rps_is_fifo(fifo_out_path))
        rps_do_create_fifos_from_prefix();
      fflush(nullptr);
      pid_t guipid = fork();
      if (guipid < 0)
        RPS_FATALOUT("failed to fork for running the GUI script"
                     << rps_gui_script_executable);
      if (guipid == 0)
        {
          // child process
          // close many file desriptors
          for (int fd=3; fd<256; fd++)
            close(fd);
          close(STDIN_FILENO);
          int nullfd = open("/dev/null", O_RDONLY);
          if (nullfd>0) //unlikely
            dup2(nullfd, STDIN_FILENO);
          execl(rps_gui_script_executable, rps_gui_script_executable,
                rps_get_fifo_prefix().c_str(), nullptr);
          perror(rps_gui_script_executable);
          _exit(126);
          return;
        };
      rps_gui_pid = guipid;
      atexit(rps_kill_wait_gui_process);
    }
  //// if told, run an editor for C++ code
  if (!rps_cpluspluseditor_str.empty() || !rps_cplusplusflags_str.empty())
    {
      RPS_INFORMOUT("rps_run_loaded_application should edit user C++ code with editor='"
                    << rps_cpluspluseditor_str << "' and compile flags '" << rps_cplusplusflags_str << "'"
                    << std::endl
                    << " with call frame " << Rps_ShowCallFrame(&_));
      rps_edit_run_cplusplus_code (&_);
    }
  //// initialize the FLTK windows in --fltk mode
  if (rps_fltk_enabled ())
    {
      RPS_DEBUG_LOG(REPL, "rps_run_loaded_application initializing FLTK");
      rps_fltk_initialize (argc, argv);
    };
  //// running the given plugins after load - should happen after
  //// edition of C++ code
  if (!rps_plugins_vector.empty())
    {
      int pluginix = 0;
      std::string curplugname;
      try
        {
          for (auto& curplugin : rps_plugins_vector)
            {
              curplugname = curplugin.plugin_name;
              void* dopluginad = dlsym(curplugin.plugin_dlh, RPS_PLUGIN_INIT_NAME);
              if (!dopluginad)
                RPS_FATALOUT("cannot find symbol " RPS_PLUGIN_INIT_NAME " in plugin " << curplugname << ":" << dlerror());
              rps_plugin_init_sig_t* pluginit = reinterpret_cast<rps_plugin_init_sig_t*>(dopluginad);
              (*pluginit)(&curplugin);
              RPS_INFORMOUT("rps_run_loaded_application initialized plugin#" << pluginix << " " << curplugname);
              curplugname.erase();
              pluginix ++;
            };
        }
      catch (std::exception& exc)
        {
          RPS_WARNOUT("rps_run_loaded_application failed to run plugin #" << pluginix
                      << rps_plugins_vector[pluginix].plugin_name
                      << " got exception " << exc.what()
                     );
        }
    };
  /////
  ///// testing the REPL lexer
  if (!rps_test_repl_string.empty())
    {
      RPS_DEBUG_LOG(REPL, "running test repl string "
                    << rps_test_repl_string);
      try
        {
          rps_run_test_repl_lexer(rps_test_repl_string);
          RPS_INFORMOUT("successfully done rps_run_test_repl_lexer on "
                        << Rps_Cjson_String(rps_test_repl_string));
        }
      catch (std::exception& exc)
        {
          RPS_WARNOUT("rps_run_test_repl_lexer in rps_run_loaded_application failed on "
                      << Rps_Cjson_String(rps_test_repl_string)
                      << " with exception " << exc.what()
                      << std::endl
                      << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application/testrepl"));
          return;
        };
    }
  /////
#if RPS_USE_CURL
  /// publish using web techniques information about this process
  /// see bugs.gentoo.org/939581,
  if (!rps_publisher_url_str.empty())
    rps_curl_publish_me(rps_publisher_url_str.c_str());
#endif /*RPS_USE_CURL*/
  ////  command vectors
  if (!rps_command_vec.empty())
    {
      RPS_INFORMOUT("before running " << rps_command_vec.size() << " command[s]");
      int nbcmd = (int)rps_command_vec.size();
      try
        {
          rps_do_repl_commands_vec(rps_command_vec);
          RPS_INFORMOUT("after running successfully "
                        << nbcmd << " commands");
        }
      catch (std::exception& exc)
        {
          RPS_WARNOUT("rps_run_loaded_application got exception "
                      << exc.what()
                      << " when running " << nbcmd << " commands:"
                      << Rps_Do_Output([&](std::ostream& out)
          {
            for (int cix=0; cix<nbcmd; cix++)
              {
                out << std::endl << rps_command_vec[cix];
              }
          })
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application/exc"));
        };
    }
  if (access(rps_gui_script_executable, X_OK))
    RPS_WARNOUT("default GUI script " << rps_gui_script_executable << " is not executable");
  ////
  ////
  ////
  if (!rps_get_fifo_prefix().empty())
    {
#pragma message "main_rps.cc with RPSJSONRPC:" __DATE__ "@" __TIME__
      RPS_INFORMOUT("initialize JSONRPC with rps_fifo_prefix:" << rps_get_fifo_prefix() << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application JSONRPC"));
      rps_jsonrpc_initialize();
    };
  RPS_DEBUG_LOG(REPL, "rps_run_loaded_application ended in thread " << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_loaded_application ending"));
} // end rps_run_loaded_application



static long
rps_fill_cplusplus_temporary_code(Rps_CallFrame*callerframe, Rps_ObjectRef tempobarg, int tcnt, const char*tempcppfilename)
{
  long tfilsiz = -1;
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED, //
                 /*callerframe:*/callerframe,
                 Rps_ObjectRef tempob;
                );
  _f.tempob = tempobarg;
  RPS_DEBUG_LOG(CMD, "rps_fill_cplusplus_temporary_code tempob=" << _f.tempob
                << " tempcppfilename=" << tempcppfilename
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  FILE* tfil = fopen(tempcppfilename, "w");
  fprintf (tfil, "//// temporary [plugin] file %s for RefPerSys\n", tempcppfilename);
  fprintf (tfil, "//// see refpersys.org website\n");
  fprintf (tfil, "//// passed to commit %s\n", rps_lastgitcommit);
  fprintf (tfil, "//// rps_shortgitid %s\n", rps_shortgitid);
  fprintf (tfil, "//// rps_md5sum %s\n", rps_md5sum);
  fprintf (tfil, "//// rps_timestamp %s\n", rps_timestamp);
  fprintf (tfil, "//// GPLv3+ licensed - see /www.gnu.org/licenses/quick-guide-gplv3.en.html\n");
  fprintf (tfil, "\n\n#" "include \"refpersys.hh\"\n\n");
  fprintf (tfil, "\n" "void rps_do_plugin(const Rps_Plugin*plugin)\n{\n");
  fprintf (tfil,
           "  RPS_LOCALFRAME(/*descr:*/\n"
           "                 Rps_ObjectRef::find_object_by_string(rps_edit_cplusplus_callframe,\n"
           "                                                                std::string{\"%s\"},\n"
           "                                                               Rps_ObjectRef::Rps_Fail_If_Not_Found),\n"
           "                 /*callerframe:*/rps_edit_cplusplus_callframe,\n"
           "                 /***** your locals here ******/\n"
           "                 );\n",
           _f.tempob->oid().to_string().c_str());
  fprintf (tfil, "  RPS_ASSERT(plugin != nullptr);\n");
  fprintf (tfil, "  RPS_DEBUG_LOG(CMD, \"start plugin \"\n"
           "                      << plugin->plugin_name << \" from \" << std::endl\n");
  fprintf (tfil, "                << RPS_FULL_BACKTRACE_HERE(1, \"temporary C++ plugin\"));\n");
  fprintf (tfil, "#warning temporary incomplete %s\n", tempcppfilename);
  fprintf (tfil, //
           "  RPS_INFORMOUT(\"did run temporary plugin \" << plugin->plugin_name\n"
           "                << \" from pid \" << (int)getpid()\n"
           "                << \" on \" << rps_hostname() << \" orig.git %s\"\n"
           "                << std::endl\n"
           "                << RPS_FULL_BACKTRACE_HERE(1, \"temporary %s#%d\"));\n",
           rps_shortgitid, _f.tempob->oid().to_string().c_str(), tcnt);
  fprintf (tfil, "} // end rps_do_plugin in %s\n", tempcppfilename);
  fprintf (tfil, "\n\n\n");
  fprintf (tfil,
           "/*********\n" //
           " **                 for Emacs...\n" //
           " ** Local-Variables: ;;\n" //
           " ** compile-command: \"cd %s; %s %s /tmp/rpsplug_%s.so\" ;;",
           rps_topdirectory, rps_plugin_builder, tempcppfilename, _f.tempob->oid().to_string().c_str());
  fprintf (tfil, //
           " ** End: ;;\n" //
           " ********/\n");
  fprintf (tfil, "\n\n\n // ********* eof %s *********\n", tempcppfilename);
  fflush (tfil);
  tfilsiz = ftell(tfil);
  RPS_INFORMOUT("filled temporary plugin " << tempcppfilename << " with " << tfilsiz << " bytes from pid " << (int)getpid() << " git " << rps_shortgitid
                << std::endl << " using object " << _f.tempob << " count " << tcnt);
  return tfilsiz;
} // end rps_fill_cplusplus_temporary_code

Rps_CallFrame*rps_edit_cplusplus_callframe;
void
rps_edit_run_cplusplus_code (Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callerframe,
                 Rps_ObjectRef tempob;
                );
  static int tcnt;
  tcnt++;
  double cpustartim = rps_process_cpu_time();
  double realstartim = rps_wallclock_real_time();
  //RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT_CALLFRAME (callerframe);
  RPS_ASSERT(rps_is_main_thread());
  rps_edit_cplusplus_callframe = &_;
  char tempfilprefix[80];
  memset (tempfilprefix, 0, sizeof(tempfilprefix));
  _f.tempob =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_3HIxVgAGg5303g7AZs), //temporary_cplusplus_code∈class
                               nullptr);
  snprintf (tempfilprefix, sizeof(tempfilprefix), "/var/tmp/rpscpp_%s-r%u-p%u",
            _f.tempob->oid().to_string().c_str(), (unsigned) Rps_Random::random_32u(),
            (unsigned) getpid());
  RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code tempfilprefix=" << tempfilprefix
                << " tempob=" << _f.tempob);
  RPS_ASSERT(strlen(tempfilprefix) < sizeof(tempfilprefix)-6);
  char tempcppfilename [96];
  memset (tempcppfilename, 0, sizeof(tempcppfilename));
  strcpy(tempcppfilename, tempfilprefix);
  strcat (tempcppfilename, ".cc");
  RPS_ASSERT(strlen(tempcppfilename) < sizeof(tempcppfilename));
  char tempsofilename [96];
  memset (tempsofilename, 0, sizeof(tempcppfilename));
  strcpy(tempsofilename, tempfilprefix);
  strcat (tempsofilename, ".so");
  RPS_ASSERT(strlen(tempsofilename) < sizeof(tempsofilename)-6);
  (void) remove (tempsofilename);
  RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code tempob=" << _f.tempob
                << " tempcppfilename=" << tempcppfilename
                << " tempsofilename=" << tempsofilename
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  long tfilsiz = -1;
  //// fill once the temporary file
  tfilsiz = rps_fill_cplusplus_temporary_code(&_, _f.tempob, tcnt, tempcppfilename);
  if (rps_cpluspluseditor_str.empty())
    {
      const char*editorenv = getenv("EDITOR");
      RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code "
                    << (editorenv?"EDITOR=":"no $EDITOR")
                    << (editorenv?editorenv:" in environment"));
      if (!editorenv  && !access("/usr/bin/editor", X_OK))
        editorenv = "/usr/bin/editor";
      RPS_DEBUG_LOG(CMD, "using " << editorenv << " to edit C++ code from "
                    << Rps_ShowCallFrame(&_));
      errno = 0;
      if (access(editorenv, X_OK))
        RPS_FATALOUT("rps_edit_run_cplusplus_code without any editor " << editorenv << ":"
                     << strerror(errno)
                     << " - from "
                     << Rps_ShowCallFrame(&_)
                     << std::endl
                     << RPS_FULL_BACKTRACE_HERE(1, "rps_edit_run_cplusplus_code *no-editor*"));
      rps_cpluspluseditor_str.assign(editorenv);
    }
  bool cppcompilegood = false;
  while (!cppcompilegood)
    {
      std::ostringstream cmdout;
      cmdout << rps_cpluspluseditor_str << " " << tempcppfilename;
      RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code before running " << cmdout.str());
      int cmdbad = system(cmdout.str().c_str());
      if (cmdbad != 0)
        {
          RPS_FATALOUT("rps_edit_run_cplusplus_code failed to edit with " << cmdout.str()
                       << " which exited " << cmdbad);
        };
      struct stat tempstat;
      memset (&tempstat, 0, sizeof(tempstat));
      if (stat(tempcppfilename, &tempstat))
        RPS_FATALOUT("rps_edit_run_cplusplus_code failed to stat file " << tempcppfilename << ":" << strerror(errno)
                     << std::endl
                     << " - from "
                     << Rps_ShowCallFrame(&_));
      RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code tempcppfilename=" << tempcppfilename
                    << " with " << tempstat.st_size << " bytes");
      if ((long)tempstat.st_size == (long)tfilsiz)
        RPS_WARNOUT("rps_edit_run_cplusplus_code unchanged size " << tfilsiz << " of temporary C++ file " << tempcppfilename
                    << std::endl
                    << " - from "
                    << Rps_ShowCallFrame(&_)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1,
                                               "rps_edit_run_cplusplus_code *unchangedsize*"));
      RPS_INFORMOUT("rps_edit_run_cplusplus_code should compile C++ code in " << tempcppfilename
                    << std::endl
                    << " - from "
                    << RPS_FULL_BACKTRACE_HERE(1,
                        "rps_edit_run_cplusplus_code")
                    << std::endl);
      std::string cwdpath;
      bool needchdir = false;
      {
        // not very good, but in practice good enough before bootstrapping
        // see https://softwareengineering.stackexchange.com/q/289427/40065
        char cwdbuf[rps_path_byte_size];
        memset(cwdbuf, 0, sizeof(cwdbuf));
        if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
          RPS_FATAL("rps_edit_run_cplusplus_code getcwd failed: %m");
        cwdpath = std::string(cwdbuf);
      }
      needchdir = cwdpath != std::string{};
      //// our compilation command is...
      std::string buildplugincmd;
      buildplugincmd.reserve(140);
      if (needchdir)
        {
          buildplugincmd += "cd '";
          buildplugincmd += Rps_QuotedC_String(rps_topdirectory);
          buildplugincmd += "' && ";
        };
      buildplugincmd += rps_gnu_make;
      buildplugincmd += " one-plugin ";
      buildplugincmd += "REFPERSYS_PLUGIN_SOURCE='";
      buildplugincmd += Rps_QuotedC_String(tempcppfilename);
      buildplugincmd += "' ";
      buildplugincmd += "REFPERSYS_PLUGIN_SHARED_OBJECT='";
      buildplugincmd += Rps_QuotedC_String(tempsofilename);
      buildplugincmd += "'\n";
      errno= 0;
      if (needchdir)
        {
          RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code before chdir to " << rps_topdirectory
                        << " from " << cwdpath << " for C++ file " << tempcppfilename);
          if (chdir(rps_topdirectory))
            RPS_FATALOUT("rps_edit_run_cplusplus_code failed to chdir to " << rps_topdirectory
                         << ":" << strerror(errno));
        }
      RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code buildplugincmd: " << buildplugincmd);
      errno = 0;
      RPS_INFORMOUT("building temporary plugin with " << buildplugincmd << " from pid:" << (int)getpid());
      fflush(nullptr);
      int buildres = system(buildplugincmd.c_str());
      if (buildres != 0)
        {
          RPS_WARNOUT("rps_edit_run_cplusplus_code build command " << buildplugincmd
                      << " failed -> " << buildres
                      << " - from "
                      << Rps_ShowCallFrame(&_)
                      << std::endl
                      << RPS_FULL_BACKTRACE_HERE(1, "rps_edit_run_cplusplus_code build failure"));
          cppcompilegood = false;
        }
      else
        cppcompilegood = true;
      if (needchdir)
        {
          RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code before chdir to " << cwdpath << " for C++ file " << tempcppfilename);
          if (chdir(cwdpath.c_str()))
            RPS_FATALOUT("rps_edit_run_cplusplus_code failed to chdir to " << cwdpath
                         << ":" << strerror(errno));
        }
      errno = 0;
      if (cppcompilegood && access(tempsofilename, R_OK))
        RPS_FATALOUT("rps_edit_run_cplusplus_code failed to build " << tempsofilename
                     << ":" << strerror(errno)
                     << " - from "
                     << Rps_ShowCallFrame(&_)
                     << std::endl
                     << RPS_FULL_BACKTRACE_HERE(1, "rps_edit_run_cplusplus_code *buildfail*"));
    };        // end while !cppcompilegood

  RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code after compilation to " << tempsofilename);
  errno = 0;
  {
    if (access(tempsofilename, R_OK))
      RPS_FATALOUT("rps_edit_run_cplusplus_code cannot access " << tempsofilename << " : " << strerror(errno));
    // we dlopen now to check that the plugin machinerey will be able to do so.
    void* tempdlh = dlopen (tempsofilename, RTLD_NOW|RTLD_GLOBAL);
    // so it is dlopen-ed twice (once here, later by plugin loading) but we don't care!
    if (!tempdlh)
      RPS_FATALOUT("rps_edit_run_cplusplus_code failed to dlopen temporary C++ plugin "
                   << tempsofilename << " : " << dlerror());
    RPS_DEBUG_LOG(CMD, "rps_edit_run_cplusplus_code did dlopen " << tempsofilename);
    Rps_Plugin templugin(tempsofilename, tempdlh);
    rps_plugins_vector.push_back(templugin);
  }
  ////////////////
  double cpuendtim = rps_process_cpu_time();
  double realendtim = rps_wallclock_real_time();
  RPS_WARNOUT("rps_edit_run_cplusplus_code incomplete build of " << tempsofilename
              << " in " << (cpuendtim-cpustartim) << " CPU, "
              << (realendtim-realstartim) << " real seconds - from "
              << Rps_ShowCallFrame(&_)
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_edit_run_cplusplus_code ending"));
#warning rps_edit_cplusplus_code still incomplete
  rps_edit_cplusplus_callframe = nullptr;
#warning rps_edit_cplusplus_code is perhaps incomplete
} // end rps_edit_run_cplusplus_code





////////////////////////////////////////////////////////////////
///// status routines
const Rps_Status
Rps_Status::get(void)
{
  static std::mutex mtx;
  static long pgsiz;
  std::lock_guard<std::mutex> gu(mtx);
  if (!pgsiz)
    pgsiz=getpagesize();
  Rps_Status res;
  // see https://man7.org/linux/man-pages/man5/proc.5.html
  FILE *f = fopen("/proc/self/statm", "r");
  if (!f)
    RPS_FATAL("Rps_Status::get to open /proc/self/statm -%m");
  long prog_sz=0, rss_sz=0, shared_sz=0, text_sz=0, lib_sz=0, data_sz=0, dt_sz=0;
  int nbs = fscanf(f, "%ld %ld %ld %ld %ld %ld %ld",
                   &prog_sz, &rss_sz, &shared_sz, &text_sz, &lib_sz, &data_sz, &dt_sz);
  if (nbs<7)
    RPS_FATAL("Rps_Status::get fscanf failure nbs=%d expected seven", nbs);
  res.prog_sizemb_stat = (prog_sz*pgsiz) >>20;
  res.rss_sizemb_stat = (rss_sz*pgsiz) >>20;
  res.shared_sizemb_stat = (shared_sz*pgsiz) >>20;
  res.cputime_stat = rps_process_cpu_time();
  res.elapsedtime_stat = rps_elapsed_real_time();
  return res;
} // end Rps_Status::get

void
Rps_Status::output(std::ostream&out) const
{
  out << " status{prog:" << prog_sizemb_stat << "Mb, rss:" <<
      rss_sizemb_stat << "Mb, shared:" << shared_sizemb_stat << "Mb, ";
  char buf[24];
  // the snprintf below won't fail in practice
  memset(buf, 0, sizeof(buf));
  if (snprintf(buf, sizeof(buf), "%.3f", cputime_stat)<0)
    RPS_FATAL("Rps_Status::output snprintf cputime failure %m");
  out << "cpu:" << buf << "s, ";
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%.3f", elapsedtime_stat);
  if (snprintf(buf, sizeof(buf), "%.3f", cputime_stat)<0)
    RPS_FATAL("Rps_Status::output snprintf elapsedtime failure %m");
  out << "elapsed:" << buf << "s}" << std::flush;
};        // end Rps_Status::output

////////////////////////////////////////////////////////////////
std::atomic<unsigned> Rps_Random::_rand_threadcount;
bool Rps_Random::_rand_is_deterministic_;
std::ranlux48 Rps_Random::_rand_gen_deterministic_;
std::mutex Rps_Random::_rand_mtx_deterministic_;


// static method called once by main
void
Rps_Random::start_deterministic(long seed)
{
  std::lock_guard<std::mutex> guard(_rand_mtx_deterministic_);
  _rand_gen_deterministic_.seed (seed);
  _rand_is_deterministic_ = true;
} // end of Rps_Random::start_deterministic


// private initializer, thread specific
void
Rps_Random::init_deterministic(void)
{
  std::lock_guard<std::mutex> guard(_rand_mtx_deterministic_);
  RPS_ASSERT(_rand_is_deterministic_);
  _rand_generator.seed(_rand_gen_deterministic_());
} // end of  Rps_Random::init_deterministic

void
Rps_Random::deterministic_reseed(void)
{
  std::lock_guard<std::mutex> guard(_rand_mtx_deterministic_);
  RPS_ASSERT(_rand_is_deterministic_);
  _rand_generator.seed(_rand_gen_deterministic_());
} // end of Rps_Random::deterministic_reseed




////////////////////////////////////////////////////////////////

/// each root object is also a public variable, define them
#define RPS_INSTALL_ROOT_OB(Oid) Rps_ObjectRef RPS_ROOT_OB(Oid);
#include "generated/rps-roots.hh"

/// each global symbol is also a public variable, define them
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name) Rps_ObjectRef RPS_SYMB_OB(Name);
#include "generated/rps-names.hh"

/// each constant object is also a public variable, define them
#define RPS_INSTALL_CONSTANT_OB(Oid) Rps_ObjectRef rpskob##Oid;
#include "generated/rps-constants.hh"


unsigned
rps_hardcoded_number_of_roots(void)
{
#define RPS_INSTALL_ROOT_OB(Oid)
#include "generated/rps-roots.hh"
  return RPS_NB_ROOT_OB;
} // end rps_hardcoded_number_of_roots


unsigned
rps_hardcoded_number_of_symbols(void)
{
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Nam)
#include "generated/rps-names.hh"
  return RPS_NB_NAMED_ROOT_OB;
} // end rps_hardcoded_number_of_symbols


unsigned
rps_hardcoded_number_of_constants(void)
{
#define RPS_INSTALL_CONSTANT_OB(Oid)
#include "generated/rps-constants.hh"
  return RPS_NB_CONSTANT_OB;
} // end of rps_hardcoded_number_of_constants



void
rps_small_quick_tests_after_load(void)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 RPS_NULL_CALL_FRAME,
                 Rps_ObjectRef obtempcpp;
                 Rps_ObjectRef obdispweb;
                 Rps_ObjectRef obnew;
                 Rps_ObjectRef obfoundnew;
                );
  RPS_DEBUG_LOG(CMD, "start rps_small_quick_tests_after_load in "
                << Rps_ShowCallFrame(&_)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_small_quick_tests_after_load"));
  _f.obtempcpp = Rps_ObjectRef::find_object_or_fail_by_string(&_, "temporary_cplusplus_code");
  RPS_DEBUG_LOG(CMD, "rps_small_quick_tests_after_load obtempcpp=" << _f.obtempcpp);
  RPS_ASSERT(_f.obtempcpp);
  _f.obnew = Rps_ObjectRef::make_object(&_, Rps_ObjectRef::the_object_class());
  RPS_DEBUG_LOG(CMD, "rps_small_quick_tests_after_load obnew=" << _f.obnew);
  RPS_ASSERT(_f.obnew);
  _f.obfoundnew = Rps_ObjectRef::find_object_or_fail_by_oid(&_, _f.obnew->oid());
  RPS_DEBUG_LOG(CMD, "rps_small_quick_tests_after_load obfoundnew=" << _f.obfoundnew << " obnew=" << _f.obnew);
  RPS_ASSERT(_f.obnew == _f.obfoundnew);
#warning should add some clever tests on  Rps_Value::is_instance_of and Rps_Value::is_subclass_of
  RPS_DEBUG_LOG(CMD, "end rps_small_quick_tests_after_load");
} // end rps_small_quick_tests_after_load


///////////////////////////////////////////////////////////////////////////////
// Implementation of debugging routines
///////////////////////////////////////////////////////////////////////////////

static pthread_mutex_t rps_debug_mutex = PTHREAD_MUTEX_INITIALIZER;

static std::string
rps_debug_level(Rps_Debug dbgopt)
{
#define DEBUG_LEVEL(dbgopt) case RPS_DEBUG_##dbgopt: return #dbgopt;

  switch (dbgopt)
    {
      RPS_DEBUG_OPTIONS(DEBUG_LEVEL);
    case RPS_DEBUG__EVERYTHING:
      return std::string{"*EveryDbg*"};
    //
    default:
    {
      char dbglevel[16];
      memset(dbglevel, 0, sizeof (dbglevel));
      snprintf(dbglevel, sizeof(dbglevel), "?DBG?%d",
               static_cast<int>(dbgopt));

      return std::string(dbglevel);
    }
    }
  //
#undef DEBUG_LEVEL
} // end rps_debug_level

static void rps_close_debug_file(void)
{
  if (rps_debug_file)
    {
      fflush(rps_debug_file);
      if (rps_debug_path[0])
        fprintf(rps_debug_file, "\n\n*** end of RefPerSys debug file %s ****\n", rps_debug_path);
      else
        fprintf(rps_debug_file, "\n\n*** end of RefPerSys debug ***\n");
      fprintf(rps_debug_file, "gitid %s version %d.%d, built %s,\n"
              " on host %s, md5sum %s, elapsed %.3f, process %.3f sec\n",
              rps_gitid, rps_get_major_version(), rps_get_minor_version(),
              rps_timestamp, rps_hostname(),  rps_md5sum,
              rps_elapsed_real_time(), rps_process_cpu_time());
      fflush(rps_debug_file);
      fsync(fileno(rps_debug_file));
      fclose(rps_debug_file);
      rps_debug_file=nullptr;
    }
} // end rps_close_debug_file

void
rps_set_debug_output_path(const char*filepath)
{
  if (rps_debug_file)
    RPS_FATAL("cannot set debug file twice, second time to %s", filepath);
  if (!access(filepath, R_OK))
    {
      char backupath[128];
      memset(backupath, 0, sizeof(backupath));
      snprintf(backupath, sizeof(backupath), "%s~", filepath);
      rename(filepath, backupath);
    };
  FILE* fdbg=fopen(filepath, "w");
  if (!fdbg)
    RPS_FATAL("cannot open debug file %s - %m", filepath);
  fprintf(fdbg, "*@#*@#*@#* RefPerSys debug file %s *@#*@#*@#*\n"
          "See refpersys.org - built %s\n"
          "On host %s pid %d gitid %s version %d.%d topdir %s\n"
          "####################################\n\n",
          filepath,
          rps_timestamp,
          rps_hostname(), (int)getpid(), rps_gitid,
          rps_get_major_version(), rps_get_minor_version(),
          rps_topdirectory);
  fflush(fdbg);
  rps_debug_file = fdbg;
  strncpy(rps_debug_path, filepath, sizeof(rps_debug_path)-1);
  atexit(rps_close_debug_file);
} // end rps_set_debug_output_path

////////////////////////////////////////////////////////////////
// if fline is negative, print a newline before....
void
rps_debug_printf_at(const char *fname, int fline, const char*funcname, Rps_Debug dbgopt,
                    const char *fmt, ...)
{
  char threadbfr[24];
  memset(threadbfr, 0, sizeof (threadbfr));
  bool ismainth = rps_is_main_thread();
  if (ismainth)
    strcpy(threadbfr, "▬!$"); //U+25AC BLACK RECTANGLE
  else
    {
      char thrbuf[16];
      memset (thrbuf, 0, sizeof(thrbuf));
      pthread_getname_np(pthread_self(), thrbuf, sizeof(thrbuf)-1);
      snprintf(threadbfr, sizeof(threadbfr),
               // U+2045 LEFT SQUARE BRACKET WITH QUILL
               "⁅%s:%d⁆"
               // U+2046 RIGHT SQUARE BRACKET WITH QUILL
               , thrbuf,  static_cast<int>(rps_thread_id()));
    }
  RPS_ASSERT(threadbfr[0] != (char)0);
  //
  char tmbfr[64];
  memset(tmbfr, 0, sizeof (tmbfr));
  rps_now_strftime_centiseconds_nolen(tmbfr, "%H:%M:%S.__ ");
  //
  char *msg = nullptr, *bigbfr = nullptr;
  char bfr[256];
  memset(bfr, 0, sizeof (bfr));
  //
  va_list arglst;
  va_start(arglst, fmt);
  int len = vsnprintf(bfr, sizeof (bfr), fmt, arglst);
  va_end(arglst);
  //
  if (RPS_UNLIKELY (len >= static_cast<int>(sizeof (bfr)) - 1))
    {
      bigbfr = static_cast<char*>(malloc(len + 10));
      if (bigbfr)
        {
          memset(bigbfr, 0, len + 10);
          va_start(arglst, fmt);
          (void) vsnprintf(bigbfr, len + 1, fmt, arglst);
          va_end(arglst);
          msg = bigbfr;
        }
    }
  else
    msg = bfr;
  //

  {
    pthread_mutex_lock(&rps_debug_mutex);
    long ndbg = rps_debug_atomic_counter.fetch_add(1);
    //
    char debugcntstr[32];
    memset (debugcntstr, 0, sizeof(debugcntstr));
    if (ndbg<1000)
      snprintf(debugcntstr, sizeof(debugcntstr), "%03ld", ndbg);
    else if (ndbg<100000)
      snprintf(debugcntstr, sizeof(debugcntstr), "%05ld", ndbg);
    else
      snprintf(debugcntstr, sizeof(debugcntstr), "%ld", ndbg);
    char datebfr[48];
    memset(datebfr, 0, sizeof (datebfr));
    char debugcstr[24];
    memset (debugcstr, 0, sizeof(debugcstr));
    if (!rps_debug_level(dbgopt).empty())
      strncpy(debugcstr, rps_debug_level(dbgopt).c_str(), sizeof(debugcstr)-1);
    //
#define RPS_DEBUG_DATE_PERIOD 64
    if (ndbg % RPS_DEBUG_DATE_PERIOD == 0)
      {
        rps_now_strftime_centiseconds_nolen(datebfr, "%Y-%b-%d@%H:%M:%s.__ %Z");
      }
    //
    if (rps_syslog_enabled)
      {
        syslog(RPS_DEBUG_LOG_LEVEL, "RPS-DEBUG#%s %7s %s @%s:%d %s %s",
               debugcntstr,
               debugcstr, threadbfr, fname, fline, tmbfr, msg);
      }
    else if (rps_debug_file)
      {
        fprintf(rps_debug_file, "RPS DEBUG#%s %7s %s",
                debugcntstr,
                debugcstr, threadbfr);
        fprintf(rps_debug_file, " %s:%d %s %s\n",
                fname, (fline>0)?fline:(-fline),
                tmbfr, msg);
        if (ndbg % RPS_DEBUG_DATE_PERIOD == 0)
          {
            if (!rps_debug_file && rps_debug_file != stdout && rps_debug_file != stderr)
              fprintf(stderr, "\n¤RPS-DEBUG#%s *^*^* %s\n",
                      debugcntstr, datebfr);
            else
              fprintf(rps_debug_file, "\n*RPS DEBUG#%s ~  *^*^* %s\n",
                      debugcntstr, datebfr);
          }
        fflush(rps_debug_file);
      }
    else // no syslog, no debug file
      {
        bool ontty = isatty(STDERR_FILENO);
        if (fline<0 || strchr(msg, '\n'))
          fputc('\n', stderr);
        fprintf(stderr, "%sRPS DEBUG#%s %7s%s %s",
                ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
                debugcntstr,
                debugcstr,
                ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
                threadbfr);
        fprintf(stderr, "%s@%s:%d%s %s\n%s\n",
                ontty?RPS_TERMINAL_ITALICS_ESCAPE:"",
                fname, (fline>0)?fline:(-fline),
                ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
                tmbfr, msg);
        fflush(stderr);
        //
        if (ndbg % RPS_DEBUG_DATE_PERIOD == 0)
          {
            fprintf(stderr, "%sRPS.DEBUG %04ld ~ %s *^*^*%s\n\n",
                    ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
                    ndbg, datebfr,
                    ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
          }
        //
        fflush(nullptr);
      }
    if (rps_fltk_enabled())
      rps_fltk_show_debug_message(fname, fline,funcname, dbgopt, ndbg, msg);
    //
    pthread_mutex_unlock(&rps_debug_mutex);
  }
  //
  if (bigbfr)
    free(bigbfr);
} // end rps_debug_printf_at


/// function called by atexit to kill then wait the GUI process
void
rps_kill_wait_gui_process(void)
{
  int guistatus = 0;
  if (kill(rps_gui_pid, SIGTERM))
    RPS_WARNOUT("failed to SIGTERM the GUI process " << rps_gui_pid);
  usleep(32*1024);
  (void) kill(rps_gui_pid, SIGKILL);
  usleep(32*1024);
  if (waitpid(rps_gui_pid, &guistatus, 0) == rps_gui_pid)
    RPS_INFORMOUT("GUI process ended with status " << guistatus);
  if (guistatus >0)
    RPS_FATALOUT("GUI process failed with status " << guistatus);
} // end rps_kill_wait_gui_process

/// registered to atexit....
extern "C" void rps_exiting(void);


void
rps_exiting(void)
{
  static char cwdbuf[rps_path_byte_size];
  char *mycwd = getcwd(cwdbuf, sizeof(cwdbuf)-2);
  syslog(LOG_INFO, "RefPerSys process %d on host %s in %s git %s version %d.%d exiting (%d);\n"
         "… elapsed %.3f sec, CPU %.3f sec;\n"
         "%s%s%s%s",
         (int)getpid(), rps_hostname(), mycwd, rps_shortgitid,
         rps_get_major_version(), rps_get_minor_version(),
         rps_exit_atomic_code.load(),
         rps_elapsed_real_time(), rps_process_cpu_time(),
         (rps_program_invocation?"invocation: ":""),
         (rps_program_invocation?:""),
         (rps_run_name.empty()?"":" run "),
         rps_run_name.c_str());
  if (!rps_syslog_enabled)
    {
      printf("RefPerSys process %d on host %s in %s git %s version %d.%d exiting (%d);\n"
             " ... elapsed %.3f sec, CPU %.3f sec\n",
             (int)getpid(), rps_hostname(), mycwd, rps_shortgitid,
             rps_get_major_version(), rps_get_minor_version(),
             rps_exit_atomic_code.load(),
             rps_elapsed_real_time(), rps_process_cpu_time());
      if (rps_program_invocation)
        printf("… invocation: %s\n", rps_program_invocation);
      if (!rps_run_name.empty())
        printf("… run: %s\n", rps_run_name.c_str());
      fflush(stdout);
    }
} // end rps_exiting


static char*rps_stored_locale;

const char*
rps_locale(void)
{
  return rps_stored_locale;
} // end rps_locale

int
main (int argc, char** argv)
{
  rps_progname = argv[0];
  char*mylocale = nullptr;
  bool helpwanted = false;
  bool versionwanted = false;
  _Pragma("message \"start of main\"");
  if (argc>1 && !strcmp(argv[1], "--help"))
    helpwanted = true;
  if (argc>1 && !strcmp(argv[1], "--version"))
    versionwanted = true;
  //// if --locale is given then process it quicky
  for (int lix=1; lix<argc; lix++)
    {
      if (!strcmp(argv[lix], "--locale") && lix+1<argc)
        mylocale = argv[lix+1];
      else if (!strncmp(argv[lix], "--locale=", strlen("--locale=")))
        mylocale = argv[lix]+strlen("--locale=");
    }
  if (mylocale)
    {
      char*l = setlocale(LC_ALL, mylocale);
      if (!l)
        RPS_FATALOUT("failed to set locale to " << mylocale);
      rps_stored_locale = l;
    }
  else
    rps_stored_locale = setlocale(LC_ALL, nullptr);
  RPS_ASSERT(rps_stored_locale != nullptr);
  rps_stdout_istty = isatty(STDOUT_FILENO);
  static_assert(sizeof(rps_progexe) > 80);
  ///// read /proc/version which hopefully is GNU/Linux specific
  {
    FILE* procversf = fopen("/proc/version", "r");
    if (!procversf)
      RPS_FATALOUT("failed to fopen /proc/version " << strerror(errno));
    if (!fgets(rps_buffer_proc_version, rps_path_byte_size-2, procversf))
      RPS_FATALOUT("failed to fgets /proc/version (fd#"
                   << fileno(procversf) << ") " << strerror(errno));
    fclose(procversf);
  }
  {
    memset(rps_progexe, 0, sizeof(rps_progexe));
    ssize_t pxl = readlink("/proc/self/exe",
                           rps_progexe, sizeof(rps_progexe));
    if (pxl <= 0 || pxl >= (ssize_t) sizeof(rps_progexe)-2)
#warning perhaps use a popen here
      // maybe we want a popen of which of the realpath of argv[0]?
      strcpy(rps_progexe, "$(/usr/bin/which refpersys)");
  }
  static_assert (sizeof(int64_t) == 8 && alignof(int64_t) == 8);
  static_assert (sizeof(int32_t) == 4 && alignof(int32_t) == 4);
  static_assert (sizeof(long) == 8 && alignof(long) == 8);
  static_assert (sizeof(void*) == 8 && alignof(void*) == 8);
  static_assert (sizeof(int) == 4 && alignof(int) == 4);
  static_assert (sizeof(time_t) == 8 && alignof(time_t) == 8);
  if (versionwanted)
    rps_show_version();
  rps_parse_program_arguments(argc, argv);
  if (helpwanted || versionwanted)
    printf("%s minimal jobs or threads number %d, maximal %d, default %d\n",
           rps_progname, RPS_NBJOBS_MIN, RPS_NBJOBS_MAX, rps_nbjobs);
  if (rps_pidfile_path)
    {
      FILE* fpid = fopen(rps_pidfile_path, "w");
      if (!fpid)
        RPS_FATALOUT("failed to open pid file " << rps_pidfile_path << " - " << strerror(errno));
      fprintf(fpid, "%ld\n", (long)getpid());
      fflush(nullptr);
      fclose(fpid);
    };
  ///
  static char cwdbuf[rps_path_byte_size];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  char *mycwd = getcwd(cwdbuf, sizeof(cwdbuf)-2);
  if (cwdbuf[sizeof(cwdbuf)-4] != 0)
    {
      /// In practice this won't happen. When it does, increase
      /// rps_path_byte_size in refpersys.hh, currently it is more than
      /// three hundred. The POSIX limit of 4Kbytes is unreasonable and
      /// since we often allocate abolute file paths on the call stack
      /// we avoid using _POSIX_PATH_MAX
      RPS_FATALOUT("too long current working directory " << cwdbuf
                   << " expecting less that " << (sizeof(cwdbuf)-4) << " bytes.");
    }
  if (rps_stdout_istty)
    {
      std::cout << std::endl
                << RPS_TERMINAL_BOLD_ESCAPE << RPS_TERMINAL_BLINK_ESCAPE
                << "!-!-! starting RefPerSys !-!-!"
                << RPS_TERMINAL_NORMAL_ESCAPE << std::endl;
    }
  else
    {
      std::cout << std::endl
                << "!-!-! starting RefPerSys !-!-!" << std::endl;
    };
  RPS_INFORMOUT(argv[0] << " process " <<  (int)getpid() << " on " << rps_hostname()
                << " in " << Rps_Cjson_String(mycwd?mycwd:"./") <<std::endl
                << "executable " << rps_progexe << " git " << rps_shortgitid
                << " version " << rps_get_major_version()
                << "." << rps_get_minor_version()
                << std::endl
                << "built " << rps_timestamp << std::endl
                << (rps_batch?"batch":"interactive")
                << " (" << rps_nbjobs << " jobs)" << std::endl
                <<             "… This is an open source inference engine software,\n"
                "…  GPLv3+ licensed, no warranty !\n"
                "…  See refpersys.org and https://www.gnu.org/licenses/ ....\n"
                << "fullgit " << rps_gitid << " branch " << rps_gitbranch
                << std::endl
                << RPS_OUT_PROGARGS(argc, argv) << std::endl);
  if (!mycwd)
    RPS_FATALOUT("getcwd failed for " << (sizeof(cwdbuf)-2) << " bytes.");
  if (rps_run_delay > 0 && rps_batch)
    RPS_FATALOUT("a run delay of " << rps_run_delay << " seconds is incompatible with the batch mode");
  RPS_DEBUG_PRINTF(REPL, "main is at @%p, rps_end_of_main is at @%p (pid %d on %s)",
                   (void*)main,
                   (void*)rps_end_of_main,
                   (int)getpid(), rps_hostname());
  ////
  //// extend the Unix environment if needed
  rps_extend_env();
  //// test the macro (generating nop instructions in assembler) for possible breakpoints;
  RPS_POSSIBLE_BREAKPOINT();
  ////
  Rps_QuasiZone::initialize();
  rps_check_mtime_files();
#if RPS_USE_CURL
  rps_initialize_curl();
#endif /*RPS_USE_CURL*/
//// initialize the gccjit
  rps_gccjit_initialize ();
  atexit(rps_gccjit_finalize);
  if (rps_my_load_dir.empty())
    {
      const char* rpld = realpath(rps_topdirectory, nullptr);
      if (!rpld)
        rpld = rps_topdirectory;
      RPS_ASSERT(rpld != nullptr);
      rps_my_load_dir = std::string(rpld);
      free ((void*)rpld);
    };
  rps_load_from(rps_my_load_dir);
  RPS_POSSIBLE_BREAKPOINT();
  //// at this point the persistent heap has been completely loaded!
  if (rps_chdir_path_after_load)
    {
      if (chdir(rps_chdir_path_after_load))
        RPS_FATALOUT("failed to chdir to " << rps_chdir_path_after_load << " after loading :"
                     << strerror(errno));
      char cwdbuf[rps_path_byte_size+4];
      memset (cwdbuf, 0, sizeof(cwdbuf));
      if (!getcwd(cwdbuf, rps_path_byte_size))
        RPS_FATALOUT("failed to getcwd after chdir after load to " << rps_chdir_path_after_load
                     << ":" << strerror(errno));
      RPS_INFORMOUT("after loading the current directory has changed to " << cwdbuf);
    };
  atexit (rps_exiting);
  if (!rps_batch)
    rps_initialize_event_loop();
  rps_run_loaded_application(argc, argv);
  RPS_POSSIBLE_BREAKPOINT();
  if (!rps_batch)
    {
      RPS_POSSIBLE_BREAKPOINT();
      if (rps_fltk_enabled())
        {
          RPS_DEBUG_LOG(REPL, "main before calling rps_fltk_run"
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "main/fltk-run+"));
          rps_fltk_run();
          RPS_DEBUG_LOG(REPL, "main rps_fltk_run ended"
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "main/fltk-run-"));
        }
      else
        {
          RPS_DEBUG_LOG(REPL, "main before calling rps_event_loop"
                        << RPS_FULL_BACKTRACE_HERE(1, "main"));
          rps_event_loop();
          RPS_DEBUG_LOG(REPL, "main after calling rps_event_loop");
        };
      RPS_DEBUG_LOG(REPL, "main before calling rps_run_after_event_loop");
      RPS_POSSIBLE_BREAKPOINT();
      rps_run_after_event_loop();
    }
  ////
  RPS_POSSIBLE_BREAKPOINT();
  if (!rps_dumpdir_str.empty())
    {
      char cwdbuf[rps_path_byte_size+4];
      memset (cwdbuf, 0, sizeof(cwdbuf));
      if (!getcwd(cwdbuf, rps_path_byte_size))
        strcpy(cwdbuf, "./");
      RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) will dump into %s\n"
                 "… from current directory %s\n",
                 (int)getpid(), rps_hostname(), rps_shortgitid,
                 rps_dumpdir_str.c_str(), cwdbuf);
      RPS_POSSIBLE_BREAKPOINT();
      rps_dump_into(rps_dumpdir_str);
    };
  RPS_POSSIBLE_BREAKPOINT();
  /// The following assembler code sets the C symbol rps_end_of_main
  /// and the several nop assembler instructions could facilitate self
  /// modification of machine code, or GDB breakpoints.
  ///
  /// Of course, we don't care about inefficiency.... Code size does
  /// not matter, and CPU time neither.  See also similar
  /// RPS_POSSIBLE_BREAKPOINT macro in refpersys.hh
  asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop");
  asm volatile (".globl rps_end_of_main; .type rps_end_of_main, @function");
  asm volatile ("rps_end_of_main: nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;");
  asm volatile (".size rps_end_of_main, . - rps_end_of_main");
  asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop");
  if (rps_debug_file)
    fflush(rps_debug_file);
  fflush(nullptr);
  RPS_POSSIBLE_BREAKPOINT();
  RPS_INFORMOUT("end of RefPerSys process "
                << (int)getpid() << " on " << rps_hostname()
                << std::endl
                << "… executable " << rps_progexe
                << " git " << rps_gitid << " branch " << rps_gitbranch
                << std::endl
                << "… built " << rps_timestamp
                << " loaded state " << rps_my_load_dir << std::endl
                << " elapsed " << rps_elapsed_real_time()
                << ", cpu " << rps_process_cpu_time() << " seconds;"
                << std::endl
                << "… /proc/version " << rps_get_proc_version()
                << std::endl
                << "Final debug flags:"
                << Rps_Do_Output([&](std::ostream& out)
  {
    rps_output_debug_flags(out);
  }) << std::endl
     << " invocation was:" << std::endl
     << RPS_OUT_PROGARGS(argc, argv) << std::endl
     << Rps_Do_Output([&](std::ostream& out)
  {
    int ex = rps_exit_atomic_code.load();
    if (ex==0)
      out << rps_progexe << " exiting normally";
    else
      out << rps_progexe << " failing #" << ex;
  })
      << std::flush
               );
  if (rps_program_invocation)
    {
      free(rps_program_invocation);
      rps_program_invocation = nullptr;
    };
  return rps_exit_atomic_code.load();
} // end of main


/////////////////// end of file main_rps.cc

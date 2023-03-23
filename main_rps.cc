
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
 *      © Copyright 2019 - 2023 The Reflective Persistent System Team
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


extern "C" pid_t rps_gui_pid;

static std::atomic<std::uint8_t> rps_exit_atomic_code;



/// actually, in function main we have something like  asm volatile ("rps_end_of_main: nop");
extern "C" void rps_end_of_main(void);

extern "C" void rps_edit_run_cplusplus_code (Rps_CallFrame*callerframe);

extern "C" void rps_small_quick_tests_after_load (void);


extern "C" std::vector<Rps_Plugin> rps_plugins_vector;
std::vector<Rps_Plugin> rps_plugins_vector;
extern "C" std::map<std::string,std::string> rps_pluginargs_map;
std::map<std::string,std::string> rps_pluginargs_map;


extern "C" std::string rps_cpluspluseditor_str;
std::string rps_cpluspluseditor_str;
extern "C" std::string rps_cplusplusflags_str;
std::string rps_cplusplusflags_str;
extern "C" std::string rps_dumpdir_str;
std::string rps_dumpdir_str;
extern "C" std::vector<std::string> rps_command_vec;
std::vector<std::string> rps_command_vec;
extern "C" std::string rps_test_repl_string;
std::string rps_test_repl_string;



static std::string rps_publisher_url_str;
static std::map<std::string,std::string> rps_dict_extra_arg;

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
    "and FIFO.out (read) for communication.\n"
    , //
    /*group:*/0 ///
  },
  /* ======= number of jobs or threads ======= */
  {/*name:*/ "jobs", ///
    /*key:*/ RPSPROGOPT_JOBS, ///
    /*arg:*/ "NBJOBS", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run <NBJOBS> threads - default is 3, minimum 2, maximum 20.\n", //
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
  /* ====== publish some data to a remote URL and Web service which
     might make some statistics about RefPerSys ===== */
  {/*name:*/ "publish-me", ///
    /*key:*/ RPSPROGOPT_PUBLISH_ME, ///
    /*arg:*/ "URL", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Send to the given URL the build timestamp and builder.\n"
    " See rps_publish_me function in this " __FILE__ " source file.\n"
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
    /*doc:*/ "set the RefPerSys homedir, default to $REFPERSYS_HOME or $HOME\n\n", ///
    /*group:*/0 ///
  },
  /* ======= run a shell command with system(3) after load ======= */
  {/*name:*/ "run-after-load", ///
    /*key:*/ RPSPROGOPT_RUN_AFTER_LOAD, ///
    /*arg:*/ "SHELL_COMMAND", ///
    /*flags:*/ 0, ///
    /*doc:*/ "Run using system(3) the given shell SHELL_COMMAND after load and plugins;\n" //
    " The following environment variables have been set:\n" //
    "  * $REFPERSYS_GITID to the git id (with a + suffix if locally changed);\n" //
    "  * $REFPERSYS_TOPDIR to the top directory with source code and persistore/ ...;\n" //
    "  * $REFPERSYS_PID to the process id running the refpersys executable;\n" //
    "  * $REFPERSYS_USER_OID to the objectid corresponding to current user;\n" //
    "\n\n",
    /*group:*/0 ///
  },
  /* ======= syslog-ing ======= */
  {/*name:*/ "syslog", ///
    /*key:*/ RPSPROGOPT_SYSLOG, ///
    /*arg:*/ nullptr, ///
    /*flags:*/ 0, ///
    /*doc:*/ "use system log with syslog(3) ...\n", //
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
    /*doc:*/ "Show type information (and test tagged integers).\n", //
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

char* rps_run_command_after_load = nullptr;
char* rps_debugflags_after_load = nullptr;

std::vector<std::function<void(Rps_CallFrame*)>> rps_do_after_load_vect;

void* rps_proghdl = nullptr;

bool rps_batch = false;
bool rps_disable_aslr = false;
bool rps_without_terminal_escape = false;
bool rps_without_quick_tests = false;
bool rps_test_repl_lexer = false;
bool rps_syslog_enabled = false;
bool rps_stdout_istty = false;
bool rps_stderr_istty = false;

unsigned rps_debug_flags;

FILE* rps_debug_file;
static char rps_debug_path[128];
static  struct rps_fifo_fdpair_st rps_fifo_pair;
pid_t rps_gui_pid;

thread_local Rps_Random Rps_Random::_rand_thr_;

typedef std::function<void(void)> rps_todo_func_t;
static std::vector<rps_todo_func_t> rps_main_todo_vect;
static std::string rps_my_load_dir;

// we may have a pair of FIFO to communicate with some external
// process (for graphical user interface), perhaps mini-edit-fltk on
// https://github.com/bstarynk/misc-basile/ ... The FIFO prefix is
// $FIFOPREFIX. The messages from the GUI user interface to RefPerSys
// are on $FIFOPREFIX.out; the messages from RefPerSys to that GUI
// user interface are on $FIFOPREFIX.cmd
static std::string rps_fifo_prefix;

std::string
rps_get_fifo_prefix(void)
{
  return rps_fifo_prefix;
} // end rps_get_fifo_prefix

struct rps_fifo_fdpair_st
rps_get_gui_fifo_fds(void)
{
  if (rps_gui_pid)
    return rps_fifo_pair;
  else return {-1, -1};
} // end  rps_get_gui_fifo_fd

pid_t
rps_get_gui_pid(void)
{
  return rps_gui_pid;
} // end rps_get_gui_pid


static void
rps_remove_fifos(void)
{
  std::string cmdfifo = rps_get_fifo_prefix()+".cmd";
  if (rps_is_fifo(cmdfifo))
  remove(cmdfifo.c_str());
  std::string outfifo = rps_get_fifo_prefix()+".out";
  if (rps_is_fifo(outfifo))
  remove(outfifo.c_str());
} // end rps_remove_fifos

void
rps_do_create_fifos(std::string prefix)
{
  int cmdfd= -1;
  int outfd= -1;
  bool rmatex = false;
  if (rps_fifo_prefix.empty())
    rps_fifo_prefix = prefix;
  std::string cmdfifo = rps_get_fifo_prefix()+".cmd";
  std::string outfifo = rps_get_fifo_prefix()+".out";
  if (!rps_is_fifo(cmdfifo)) {
    if (mkfifo(cmdfifo.c_str(), 0660)<0)
      RPS_FATALOUT("failed to create command FIFO " << cmdfifo << ":" << strerror(errno));
    rmatex = true;
  }
  cmdfd = open(cmdfifo.c_str(), 0660 | O_CLOEXEC);
  if (cmdfd<0) 
    RPS_FATALOUT("failed to open command FIFO " << cmdfifo << ":" << strerror(errno));
  if (!rps_is_fifo(outfifo)) {
    if (mkfifo(outfifo.c_str(), 0660)<0)
      RPS_FATALOUT("failed to create output FIFO " << outfifo << ":" << strerror(errno));
    rmatex = true;
  }
  outfd = open(outfifo.c_str(), 0440 | O_CLOEXEC);
  if (outfd<0) 
    RPS_FATALOUT("failed to open output FIFO " << outfifo << ":" << strerror(errno));
  if (rmatex)
    atexit(rps_remove_fifos);
  rps_fifo_pair.fifo_ui_wcmd = cmdfd;
  rps_fifo_pair.fifo_ui_rout = outfd;
} // end rps_do_create_fifos



unsigned rps_call_frame_depth(const Rps_CallFrame*callframe)
{
  if (callframe==nullptr) return 0;
  else
    return callframe->call_frame_depth();
} // end rps_call_frame_depth

static void rps_parse_program_arguments(int &argc, char**argv);

static char rps_bufpath_homedir[384];

static pthread_t rps_main_thread_handle;


bool rps_is_main_thread(void)
{
  return pthread_self() == rps_main_thread_handle;
} // end rps_is_main_thread

const char*
rps_homedir(void)
{
  static std::mutex homedirmtx;
  std::lock_guard<std::mutex> gu(homedirmtx);
  if (RPS_UNLIKELY(rps_bufpath_homedir[0] == (char)0))
    {
      const char*rpshome = getenv("REFPERSYS_HOME");
      const char*home = getenv("HOME");
      const char*path = rpshome?rpshome:home;
      if (!path)
        RPS_FATAL("no RefPerSys home ($REFPERSYS_HOME or $HOME)");
      char* rp = realpath(path, nullptr);
      if (!rp)
        RPS_FATAL("realpath failed on RefPerSys home %s - %m",
                  path);
      if (strlen(rp) >= sizeof(rps_bufpath_homedir) -1)
        RPS_FATAL("too long realpath %s on RefPerSys home %s", rp, path);
      strncpy(rps_bufpath_homedir, rp, sizeof(rps_bufpath_homedir) -1);
    }
  return rps_bufpath_homedir;
} // end rps_homedir

const std::string&
rps_get_loaddir(void)
{
  return rps_my_load_dir;
} // end rps_get_loaddir

const char*
rps_hostname(void)
{
  static char hnambuf[64];
  if (RPS_UNLIKELY(!hnambuf[0]))
    gethostname(hnambuf, sizeof(hnambuf)-1);
  return hnambuf;
} // end rps_hostname


void
rps_set_exit_code(std::uint8_t ex)
{
  rps_exit_atomic_code.store(ex);
} // end rps_set_exit_code

void
rps_emit_gplv3_copyright_notice(std::ostream&outs, std::string path, std::string linprefix, std::string linsuffix)
{
  outs << linprefix
       << "GENERATED file " << path  << " / DO NOT EDIT!"
       << linsuffix << std::endl;
  outs << linprefix
       << "This file is part of the Reflective Persistent System."
       << linsuffix << std::endl;
  {
    time_t nowtime = time(nullptr);
    struct tm nowtm = {};
    localtime_r(&nowtime, &nowtm);
    outs << linprefix << " © Copyright " << RPS_INITIAL_COPYRIGHT_YEAR
         << " - " << (nowtm.tm_year+1900)
         << " The Reflective Persistent System Team."
         << linsuffix << std::endl;
    outs << linprefix
         << " see refpersys.org and contact team@refpersys.org for more."
         << linsuffix << std::endl;
  }
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This program is free software: you can redistribute it and/or modify"
       << linsuffix << std::endl;
  outs << linprefix << "it under the terms of the GNU General Public License as published by"
       << linsuffix << std::endl;
  outs << linprefix << "the Free Software Foundation, either version 3 of the License, or"
       << linsuffix << std::endl;
  outs << linprefix << "(at your option) any later version."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This program is distributed in the hope that it will be useful,"
       << linsuffix << std::endl;
  outs << linprefix << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
       << linsuffix << std::endl;
  outs << linprefix << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
       << linsuffix << std::endl;
  outs << linprefix << "GNU General Public License for more details."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "You should have received a copy of the GNU "
       "General Public License"
       << linsuffix << std::endl;
  outs << linprefix << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
       << linsuffix << std::endl;
} // end rps_emit_gplv3_copyright_notice


////////////////
void
rps_print_types_info(void)
{
#define TYPEFMT_rps "%-58s:"
  printf(TYPEFMT_rps "   size  align   (bytes)\n", "**TYPE**");
#define EXPLAIN_TYPE(Ty) printf(TYPEFMT_rps " %5d %5d\n", #Ty,		\
				(int)sizeof(Ty), (int)alignof(Ty))

#define EXPLAIN_TYPE2(Ty1,Ty2) printf(TYPEFMT_rps " %5d %5d\n",	\
				      #Ty1 "," #Ty2,		\
				      (int)sizeof(Ty1,Ty2),	\
				      (int)alignof(Ty1,Ty2))

#define EXPLAIN_TYPE3(Ty1,Ty2,Ty3) printf(TYPEFMT_rps " %5d %5d\n",	\
					  #Ty1 "," #Ty2 "," #Ty3,	\
					  (int)sizeof(Ty1,Ty2,Ty3),	\
					  (int)alignof(Ty1,Ty2,Ty3))
#define EXPLAIN_TYPE4(Ty1,Ty2,Ty3,Ty4) printf(TYPEFMT_rps " %5d %5d\n",	\
					      #Ty1 "," #Ty2 "," #Ty3 "," #Ty4, \
					      (int)sizeof(Ty1,Ty2,Ty3,Ty4), \
					      (int)alignof(Ty1,Ty2,Ty3,Ty4))
  EXPLAIN_TYPE(int);
  EXPLAIN_TYPE(double);
  EXPLAIN_TYPE(char);
  EXPLAIN_TYPE(bool);
  EXPLAIN_TYPE(void*);
  EXPLAIN_TYPE(std::mutex);
  EXPLAIN_TYPE(std::shared_mutex);
  EXPLAIN_TYPE(std::recursive_mutex);
  EXPLAIN_TYPE(std::atomic<void*>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::recursive_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::string);
  EXPLAIN_TYPE(std::vector<std::string>);
  EXPLAIN_TYPE(std::set<std::string>);
  EXPLAIN_TYPE2(std::map<Rps_ObjectRef, Rps_Value>);
  EXPLAIN_TYPE2(std::unordered_map<std::string, Rps_ObjectRef*>);
  EXPLAIN_TYPE3(std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher>);
  EXPLAIN_TYPE3(std::variant<unsigned, std::function<Rps_Value(void*)>,
                std::function<int(void*,Rps_ObjectRef)>>);
  EXPLAIN_TYPE(Rps_Backtracer);
  EXPLAIN_TYPE(Rps_ClosureValue);
  EXPLAIN_TYPE(Rps_ClosureZone);
  EXPLAIN_TYPE(Rps_Double);
  EXPLAIN_TYPE(Rps_DoubleValue);
  EXPLAIN_TYPE(Rps_GarbageCollector);
  EXPLAIN_TYPE(Rps_HashInt);
  EXPLAIN_TYPE(Rps_Id);
  EXPLAIN_TYPE(Rps_ObjectRef);
  EXPLAIN_TYPE(Rps_ObjectValue);
  EXPLAIN_TYPE(Rps_ObjectZone);
  EXPLAIN_TYPE(Rps_Payload);
  EXPLAIN_TYPE(Rps_PayloadClassInfo);
  EXPLAIN_TYPE(Rps_PayloadSetOb);
  EXPLAIN_TYPE(Rps_PayloadVectOb);
  EXPLAIN_TYPE(Rps_QuasiZone);
  EXPLAIN_TYPE(Rps_SetOb);
  EXPLAIN_TYPE(Rps_SetValue);
  EXPLAIN_TYPE(Rps_String);
  EXPLAIN_TYPE(Rps_StringValue);
  EXPLAIN_TYPE(Rps_TupleOb);
  EXPLAIN_TYPE(Rps_TupleValue);
  EXPLAIN_TYPE(Rps_Type);
  EXPLAIN_TYPE(Rps_Value);
  EXPLAIN_TYPE(Rps_ZoneValue);
#undef EXPLAIN_TYPE4
#undef EXPLAIN_TYPE3
#undef EXPLAIN_TYPE
#undef TYPEFMT_rps
  putchar('\n');
  fflush(nullptr);
  std::cout << "@@°°@@ The tagged integer one hundred is "
            << Rps_Value::make_tagged_int(100)
            << std::endl
            << "... and the tagged integer minus one billion is "
            <<  Rps_Value::make_tagged_int(-1000000000)
            << " !!! " << std::endl;
} // end rps_print_types_info




////////////////////////////////////////////////////////////////
// TIME ROUTINES
////////////////////////////////////////////////////////////////

int rps_nbjobs = RPS_NBJOBS_MIN + 1;

static double rps_start_monotonic_time;
static double rps_start_wallclock_real_time;
double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
}

double rps_get_start_wallclock_real_time()
{
  return rps_start_wallclock_real_time;
}

static void
rps_check_mtime_files(void)
{
  struct stat selfstat = {};
  if (stat("/proc/self/exe", &selfstat))
    RPS_FATAL("stat /proc/self/exe: %m");
  char exebuf[128];
  memset (exebuf, 0, sizeof(exebuf));
  if (readlink("/proc/self/exe", exebuf, sizeof(exebuf)-1)<0)
    RPS_FATAL("readlink /proc/self/exe: %m");
  for (const char*const*curpath = rps_files; *curpath; curpath++)
    {
      int lencurpath = strlen(*curpath);
      if (lencurpath < 6 || strstr(*curpath, "attic/"))
        continue;
      std::string curpathstr(*curpath);
      /// Files under webroot could be sent to browser, so we don't
      /// care about them being newer than executable....
      auto wrp = curpathstr.find("webroot/");
      if (wrp < curpathstr.size())
        continue;
      std::string curfullpathstr= std::string{rps_topdirectory} + "/" + curpathstr;
      struct stat curstat = {};
      if (stat(curfullpathstr.c_str(), &curstat))
        {
          RPS_WARNOUT("rps_check_mtime_files: stat " << curfullpathstr << " failed: " << strerror(errno));
          continue;
        };
      if (curstat.st_mtime > (time_t) rps_timelong)
        RPS_WARNOUT("rps_check_mtime_files: " << curfullpathstr.c_str()
                    << " is younger by "
                    << (curstat.st_mtime - (time_t) rps_timelong)
                    << " seconds than current executable " << exebuf
                    << ", so consider rebuilding with make");
    }
  char makecmd [128];
  memset (makecmd, 0, sizeof(makecmd));
  if (snprintf(makecmd, sizeof(makecmd), "make -t -C %s -q objects", rps_topdirectory) < (int)sizeof(makecmd)-1)
    {
      int bad = system(makecmd);
      if (bad)
        RPS_WARNOUT("rps_check_mtime_files: " << makecmd
                    << " failed with status# " << bad);
      else
        RPS_INFORMOUT("rps_check_mtime_files: did " << std::string(makecmd) << " successfully");
    }
  else
    RPS_FATAL("rps_check_mtime_files failed to construct makecmd in %s: %m",
              rps_topdirectory);
} // end rps_check_mtime_files



/// In a format string passed to strftime, replace .__ with the
/// centisecond fractional part of the time. See of course
/// http://man7.org/linux/man-pages/man3/strftime.3.html etc... Notice
/// that debugging facilities use that function, e.g. it gets called
/// from rps_debug_printf_at used by RPS_DEBUG_LOG and RPS_DEBUG_PRINTF
/// macros.
char *
rps_strftime_centiseconds(char *bfr, size_t len, const char *fmt, double tm)
{
  if (!bfr || !fmt || len<4)
    return nullptr;
  //
  memset (bfr, 0, len);
  //
  struct tm tmstruct;
  memset(&tmstruct, 0, sizeof (tmstruct));
  //
  time_t time = static_cast<time_t>(tm);
  strftime(bfr, len, fmt, localtime_r(&time, &tmstruct));
  //
  char *dotdunder = strstr(bfr, ".__");
  if (dotdunder)
    {
      double intpart = 0.0;
      double fraction = modf(tm, &intpart);

      char minibuf[16];
      memset(minibuf, 0, sizeof (minibuf));
      assert(fraction >= 0.0 && fraction < 1.0);

      snprintf(minibuf, sizeof (minibuf), "%.02f", fraction);
      minibuf[4] = (char)0;
      const char* dotminib = strchr(minibuf, '.');
      if (dotminib && dotminib<minibuf+sizeof(minibuf)-4)
        {
          strncpy(dotdunder, dotminib, 3);
        }
    }

  return bfr;
} // end rps_strftime_centiseconds



////////////////////////////////////////////////////////////////
void
rps_extend_env(void)
{
  static char pidenv[64];
  snprintf(pidenv, sizeof(pidenv), "REFPERSYS_PID=%d", (int)getpid());
  putenv(pidenv);
  static char gitenv[64];
  snprintf(gitenv, sizeof(gitenv), "REFPERSYS_GITID=%s", rps_gitid);
  putenv(gitenv);
  static char topdirenv[384];
  snprintf(topdirenv, sizeof(topdirenv), "REFPERSYS_TOPDIR=%s", rps_topdirectory);
  putenv(topdirenv);
  static char fifoenv[256];
  if (!rps_fifo_prefix.empty()) {
    snprintf(fifoenv, sizeof(fifoenv), "REFPERSYS_FIFO_PREFIX=%s", rps_fifo_prefix.c_str());
    putenv(fifoenv);
  }
} // end rps_extend_env


static void
rps_early_initialization(int argc, char** argv)
{
  rps_argc = argc;
  rps_argv = argv;
  rps_start_monotonic_time = rps_monotonic_real_time();
  rps_start_wallclock_real_time = rps_wallclock_real_time();
  rps_stderr_istty = isatty(STDERR_FILENO);
  rps_stdout_istty = isatty(STDOUT_FILENO);
  rps_progname = argv[0];
  rps_proghdl = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
  if (!rps_proghdl)
    {
      fprintf(stderr, "%s failed to dlopen whole program (%s)\n", rps_progname,
              dlerror());
      exit(EXIT_FAILURE);
    };
  rps_main_thread_handle = pthread_self();
  {
    char cwdbuf[128];
    memset (cwdbuf, 0, sizeof(cwdbuf));
    char tmbfr[64];
    memset(tmbfr, 0, sizeof (tmbfr));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)) || cwdbuf[0] == (char)0)
      strcpy(cwdbuf, "./");
    rps_now_strftime_centiseconds_nolen(tmbfr, "%Y, %b, %D %H:%M:%S.__ %Z");
    std::cout << std::endl << "** STARTING RefPerSys git " << rps_shortgitid << " on " << rps_hostname() << " pid#" << getpid()
	      << " in " << cwdbuf << " at " << tmbfr << std::endl;
  }
  /// handle early a debug flag request
  if (argc > 1
      && !strncmp(argv[1], "--debug=", strlen("--debug=")))
    {
      rps_set_debug(argv[1]+strlen("--debug="));
    }
  else if (argc > 1 && argv[1][0]=='-' && argv[1][1]==RPSPROGOPT_DEBUG)
    {
      rps_set_debug(argv[1]+2);
    };
  // also use REFPERSYS_DEBUG
  {
    const char*debugenv = getenv("REFPERSYS_DEBUG");
    if (debugenv)
      rps_set_debug(debugenv);
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
        if (!strcmp(argv[ix], "--no-aslr"))
          rps_disable_aslr = true;
        else if (!strcmp(argv[ix], "-B") || !strcmp(argv[ix], "--batch"))
          rps_batch = true;
        else if (!strcmp(argv[ix], "--without-terminal"))
          rps_without_terminal_escape = true;
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
    rps_set_debug(std::string(argv[1]+strlen("--debug=")));
  if (argc>1 && !strncmp(argv[1], "-d", strlen("-d")))
    rps_set_debug(std::string(argv[1]+strlen("-d")));
  ///
  if (rps_syslog_enabled && rps_debug_flags != 0)
    openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
} // end rps_early_initialization

////////////////////////////////////////////////////////////////
int
main (int argc, char** argv)
{
  rps_early_initialization(argc, argv);
  rps_parse_program_arguments(argc, argv);
  ///
  RPS_INFORM("%s%s" "!-!-! starting RefPerSys !-!-!" "%s" //
	     " %s process %d on host %s\n" //
	     "... (stdout %s, stderr %s) with %d arguments\n" //
             "... gitid %.16s built %s, %s mode (%d jobs)\n"
	     "This is open source software, GPL licensed, no warranty!\n"
	     ".... See http://refpersys.org/ .....\n",
             RPS_TERMINAL_BOLD_ESCAPE, RPS_TERMINAL_BLINK_ESCAPE,
             RPS_TERMINAL_NORMAL_ESCAPE,
             argv[0], (int)getpid(), rps_hostname(),
             rps_stdout_istty?"tty":"plain",
             rps_stderr_istty?"tty":"plain",
	     argc,
             rps_gitid, rps_timestamp,
             (rps_batch?"batch":"interactive"),
             rps_nbjobs);
  RPS_DEBUG_PRINTF(REPL, "main is at @%p, rps_end_of_main is at @%p (pid %d on %s)",
		   (void*)main,
		   (void*)rps_end_of_main,
		   (int)getpid(), rps_hostname());
  ////
  //// extend the environment if needed
  rps_extend_env();
  ////
  Rps_QuasiZone::initialize();
  rps_check_mtime_files();
  rps_initialize_curl();
  if (rps_my_load_dir.empty()) {
    const char* rpld = realpath(rps_topdirectory, nullptr);
    if (!rpld)
      rpld = rps_topdirectory;
    RPS_ASSERT(rpld != nullptr);
    rps_my_load_dir = std::string(rpld);
    free ((void*)rpld);
  };
  rps_load_from(rps_my_load_dir);
  if (!rps_batch)
    rps_initialize_event_loop();
  rps_run_application(argc, argv);
  if (!rps_batch)
    rps_event_loop();
  ////
  if (!rps_dumpdir_str.empty())
    {
      char cwdbuf[128];
      memset (cwdbuf, 0, sizeof(cwdbuf));
      if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
        strcpy(cwdbuf, "./");
      RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) will dump into %s\n"
                 "... from current directory %s\n",
                 (int)getpid(), rps_hostname(), rps_shortgitid,
                 rps_dumpdir_str.c_str(), cwdbuf);
      rps_dump_into(rps_dumpdir_str);
    }
  /// The following assembler code sets the C symbol rps_end_of_main
  /// and the several nop assembler instructions could facilitate self
  /// modification of machine code, or GDB breakpoints.
  ///
  /// Of course, we don't care about inefficiency.... Code size does
  /// not matter, and CPU time neither
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
  RPS_INFORM("end of RefPerSys process %d on host %s loaded state %s\n"
             "... gitid %.16s built %s elapsed %.3f sec, process %.3f sec",
             (int)getpid(), rps_hostname(),
	     rps_my_load_dir.c_str(),
	     rps_gitid, rps_timestamp,
             rps_elapsed_real_time(), rps_process_cpu_time());
  return rps_exit_atomic_code.load();
} // end of main

const char*
rps_get_plugin_cstr_argument(const Rps_Plugin*plugin)
{
  if (!plugin)
    return nullptr;
  auto it = rps_pluginargs_map.find(plugin->plugin_name);
  if (it == rps_pluginargs_map.end())
    return nullptr;
  return it->second.c_str();
} // end rps_get_plugin_cstr_argument


// Parse a single program option, skipping side effects when state is
// empty.
error_t
rps_parse1opt (int key, char *arg, struct argp_state *state)
{
  bool side_effect = state && (void*)state != RPS_EMPTYSLOT;
  switch (key)
    {
    case RPSPROGOPT_DEBUG:
    {
      if (side_effect)
        rps_set_debug(std::string(arg));
    }
    return 0;
    case RPSPROGOPT_DEBUG_PATH:
    {
      if (side_effect)
        rps_set_debug_output_path(arg);
    }
    return 0;
    case RPSPROGOPT_LOADDIR:
    {
      rps_my_load_dir = std::string(arg);
    }
    return 0;
    case RPSPROGOPT_COMMAND:
    {
      rps_command_vec.push_back(std::string(arg));
    }
    return 0;
    case RPSPROGOPT_INTERFACEFIFO:
      {
	rps_fifo_prefix = std::string(arg);
      }
      return 0;
    case RPSPROGOPT_BATCH:
    {
      rps_batch = true;
    }
    return 0;
    case RPSPROGOPT_JOBS:
    {
      int nbjobs = atoi(arg);
      if (nbjobs <= RPS_NBJOBS_MIN)
        nbjobs = RPS_NBJOBS_MIN;
      else if (nbjobs > RPS_NBJOBS_MAX)
        nbjobs = RPS_NBJOBS_MAX;
      rps_nbjobs = nbjobs;
    }
    return 0;
    case RPSPROGOPT_PUBLISH_ME:
      {
	if (!rps_publisher_url_str.empty())
	  RPS_FATAL("cannot give twice the --publish-me <URL> option");
	rps_publisher_url_str = arg;
      }
    return 0;
    case RPSPROGOPT_DUMP:
    {
      if (side_effect)
        rps_dumpdir_str = std::string(arg);
    }
    return 0;
    case RPSPROGOPT_HOMEDIR:
    {
      struct stat rhomstat;
      memset (&rhomstat, 0, sizeof(rhomstat));
      if (stat(arg, &rhomstat))
        RPS_FATAL("failed to stat --refpersys-home %s: %m",
                  arg);
      if (!S_ISDIR(rhomstat.st_mode))
        RPS_FATAL("given --refpersys-home %s is not a directory",
                  arg);
      if ((rhomstat.st_mode & (S_IRUSR|S_IXUSR)) !=  (S_IRUSR|S_IXUSR))
        RPS_FATAL("given --refpersys-home %s is not user readable and executable",
                  arg);
      if (side_effect)
        {
          char*rhomrp = realpath(arg, nullptr);
          if (!rhomrp)
            RPS_FATAL("realpath failed on given --refpersys-home %s - %m",
                      arg);
          if (strlen(rhomrp) >= sizeof(rps_bufpath_homedir) -1)
            RPS_FATAL("too long realpath %s on given --refpersys-home %s - %m",
                      rhomrp, arg);
          strncpy(rps_bufpath_homedir, rhomrp, sizeof(rps_bufpath_homedir) -1);
          free (rhomrp), rhomrp = nullptr;
          RPS_INFORMOUT("set RefPerSys home directory to " << rps_bufpath_homedir);
        };
    }
    return 0;
    case RPSPROGOPT_RANDOMOID:
    {
      int nbrand = atoi(arg);
      if (nbrand <= 0) nbrand = 2;
      else if (nbrand > 100) nbrand = 100;
      if (side_effect)
        {
          RPS_INFORM("output of %d random objids generated on %.2f\n", nbrand,
                     rps_wallclock_real_time());
          printf("*    %-20s" "\t  %-19s" "   %-12s" "\t %-10s\n",
                 " objid", "hi", "lo", "hash");
          printf("========================================================"
                 "===========================\n");
          for (int ix = 0; ix<nbrand; ix++)
            {
              auto rid = Rps_Id::random();
              printf("! %22s" "\t  %19lld" " %12lld" "\t %10u\n",
                     rid.to_string().c_str(),
                     (long long) rid.hi(),
                     (long long) rid.lo(),
                     (unsigned) rid.hash());
            }
          printf("--------------------------------------------------------"
                 "---------------------------\n");
          fflush(nullptr);
        }
    }
    return 0;
    case RPSPROGOPT_TYPEINFO:
    {
      if (side_effect)
        rps_print_types_info ();
      rps_batch = true;
    }
    return 0;
    case RPSPROGOPT_SYSLOG:
    {
      if (side_effect)
        {
          rps_syslog_enabled = true;
          openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
          RPS_INFORM("using syslog");
        }
    }
    return 0;
    case RPSPROGOPT_NO_TERMINAL:
    {
      rps_without_terminal_escape = true;
    }
    return 0;
    case RPSPROGOPT_NO_ASLR:
    {
      // was already handled
      RPS_ASSERT(rps_disable_aslr);
    }
    return 0;
    case RPSPROGOPT_NO_QUICK_TESTS:
    {
      rps_without_quick_tests = true;
    }
    return 0;
    case RPSPROGOPT_TEST_REPL_LEXER:
    {
      if (side_effect) {
	if (!rps_test_repl_string.empty())
	  RPS_FATALOUT("only one --test-repl-lexer=TESTLEXSTRING can"
		       " be given, but already got "
		       << rps_test_repl_string);
	rps_test_repl_string = arg;
	RPS_INFORMOUT("will test the REPL lexer on:" << rps_test_repl_string
		      << std::endl << "... that is the " << rps_test_repl_string.size()
		      << " bytes string " << Rps_QuotedC_String(rps_test_repl_string));
      }
    }
    return 0;
    case RPSPROGOPT_DEBUG_AFTER_LOAD:
    {
      if (!rps_debugflags_after_load || side_effect)
        rps_debugflags_after_load = arg;
    }
    return 0;
    case RPSPROGOPT_EXTRA_ARG:
      {
	int eqnextpos= -1;
	char extraname[64];
	memset (extraname, 0, sizeof(extraname));
	if (sscanf(arg, "%60[A-Za-z0-9]=%n", extraname, &eqnextpos) >= 1
	    && isalpha(extraname[0])
	    && eqnextpos > 1 && arg[eqnextpos-1] == '='
	    && isalpha(extraname[0])) {
	  for (const char*n = extraname; *n; n++)
	    if (!isalnum(*n) && *n != '_')
	      RPS_FATALOUT("invalid extra named argument " << extraname);
	  if (rps_dict_extra_arg.find(extraname) != rps_dict_extra_arg.end())
	    RPS_FATALOUT("extra named argument " << extraname
			 << " cannot be set more than once");
	  std::string extraval{arg+eqnextpos};
	  rps_dict_extra_arg.insert({extraname, extraval});
	  RPS_INFORMOUT("set extra argument " << extraname
			<< " to " << Rps_QuotedC_String(extraval));
	}
	else
	  RPS_FATALOUT("bad extra named argument " << arg
		       << " that is " << Rps_QuotedC_String(arg)
		       << " extra name is " << Rps_QuotedC_String(extraname)
		      );
      }
      return 0;
    case RPSPROGOPT_RUN_AFTER_LOAD:
    {
      if (rps_run_command_after_load)
        RPS_FATALOUT("only one --run-after-load command can be given, not both " << rps_run_command_after_load
                     << " and " << arg);
      rps_run_command_after_load = arg;
    }
    return 0;
    case RPSPROGOPT_PLUGIN_AFTER_LOAD:
    {
      void* dlh = dlopen(arg, RTLD_NOW|RTLD_GLOBAL);
      if (!dlh)
        RPS_FATALOUT("failed to dlopen plugin " << arg << " : " << dlerror());
      const char* bnplug = basename(arg);
      Rps_Plugin curplugin(bnplug, dlh);
      RPS_INFORMOUT("loaded plugin#" << rps_plugins_vector.size() << " from " << arg << " from process pid#" << (int)getpid()
		    << " basenamed " << Rps_QuotedC_String(bnplug));
      rps_plugins_vector.push_back(curplugin);
    }
    return 0;
    case RPSPROGOPT_PLUGIN_ARG:
      {
	char plugname[80];
	char plugarg[128];
	memset (plugname, 0, sizeof(plugname));
	memset (plugarg, 0, sizeof(plugarg));
	if (!arg)
	  RPS_FATALOUT("missing --plugin-arg");
	if (strlen(arg) >= sizeof(plugname) + sizeof(plugarg) - 1)
	  RPS_FATALOUT("too long --plugin-arg" << arg
		       << " should be shorter than " << ( sizeof(plugname) + sizeof(plugarg)) << " bytes");
	if (sscanf(arg, "%78[a-zA-Z0-9_]:%126s", plugname, plugarg) < 2)
	  RPS_FATALOUT("expecting --plugin-arg=<plugin-name>:<plugin-arg-string but got " << arg);
	int pluginix= -1;
	int plugcnt = 0;
	for (Rps_Plugin curplugin: rps_plugins_vector) {
	  std::string curplugname = curplugin.plugin_name;
	  int pluglenam= curplugname.length();
	  if (pluglenam > 4 && curplugname.substr(pluglenam-3) == ".so")
	    curplugname.erase(pluglenam-3);
	  RPS_DEBUG_LOG (REPL, "plugin#" << plugcnt << " is named " << Rps_QuotedC_String(curplugname));
	  if (curplugname == plugname) {
	    pluginix = plugcnt;
	    break;
	  };
	  plugcnt++;
	}
	if (pluginix<0)
	  RPS_FATALOUT("--plugin-arg=" << plugname << ":" << plugarg
		       << " without such loaded plugin (loaded " << plugcnt << " plugins)");
	Rps_Plugin thisplugin = rps_plugins_vector[pluginix];
	rps_pluginargs_map[plugname] = std::string{plugarg};
	RPS_INFORMOUT("registering plugin argument of --plugin-arg " << arg
		      << " plugname=" << Rps_QuotedC_String(plugname)
		      << " plugarg=" << Rps_QuotedC_String(plugarg)
		    << std::endl
		    << RPS_FULL_BACKTRACE_HERE(1, "--plugin-arg processing"));
      }
      return 0;
    case RPSPROGOPT_CPLUSPLUSEDITOR_AFTER_LOAD:
    {
      RPS_DEBUG_LOG(CMD, "option --cplusplus-editor "
                    << (arg?" with '":" without ")
                    << (arg?arg:" argument")
                    << (arg?"'":" !!!")
                    << (side_effect?" side-effecting"
                        :" without side effect"));
      if (side_effect)
        {
          if (!arg || !arg[0])
            {
              char* editor = getenv("EDITOR");
              if (editor && editor[0])
                {
                  arg = editor;
                  RPS_INFORMOUT("using $EDITOR variable " << editor << " as C++ editor");
                }
            };
          if (!arg || !arg[0])
            RPS_FATALOUT("program option --cplusplus-editor-after-load"
			 " without explicit editor,\n"
			 "... and no $EDITOR environment variable");
          if (!rps_cpluspluseditor_str.empty())
            RPS_FATALOUT("program option --cplusplus-editor-after-load"
			 " given twice with "
                         << rps_cpluspluseditor_str << " and " << arg);
          rps_cpluspluseditor_str.assign(arg);
        };
    }
    return 0;
    case RPSPROGOPT_CPLUSPLUSFLAGS_AFTER_LOAD:
    {
      if (side_effect)
        {
          if (!rps_cplusplusflags_str.empty())
            RPS_FATALOUT("program option --cplusplus-flags-after-load given twice with "
                         << rps_cplusplusflags_str << " and " << arg);
          rps_cplusplusflags_str.assign(arg);
        }
    }
    return 0;
    case RPSPROGOPT_VERSION:
    {
      if (side_effect)
        {
          int nbfiles=0;
          int nbsubdirs=0;
          for (auto pfiles=rps_files; *pfiles; pfiles++)
            nbfiles++;
          for (auto psubdirs=rps_subdirectories; *psubdirs; psubdirs++)
            nbsubdirs++;
          std::cout << "RefPerSys, an Artificial Intelligence system - work in progress..." << std::endl;
          std::cout << "version information:\n"
                    << " program name: " << rps_progname << std::endl
                    << " build time: " << rps_timestamp << std::endl
                    << " top directory: " << rps_topdirectory << std::endl
                    << " git id: " << rps_gitid << std::endl
                    << " short git id: " << rps_shortgitid << std::endl
                    << " last git tag: " << rps_lastgittag << std::endl
                    << " last git commit: " << rps_lastgitcommit << std::endl
                    << " md5sum of " << nbfiles << " source files: " << rps_md5sum << std::endl
                    << " with " << nbsubdirs << " subdirectories." << std::endl
                    << " GNU glibc: " << gnu_get_libc_version() << std::endl
	    /* TODO: near commit 191d55e1b31c, march 2023; decide
	       which parser generator to really use... and drop the
	       other one.  Non technical considerations,
	       e.g. licensing, is important to some partners... */
                    << " Gnu Bison parser generator: " << rps_gnubison_command
		    << " version: " << rps_gnubison_version
		    << " at: " << rps_gnubison_realpath
		    << std::endl
		    << " default GUI script: " << rps_gui_script_executable << std::endl
                    << " Read Eval Print Loop: " << rps_repl_version() << std::endl
                    << " libCURL for web client: " << rps_curl_version() << std::endl
		    << " JSONCPP: " << JSONCPP_VERSION_STRING << std::endl
                    << " made with: " << rps_makefile << std::endl
                    << " running on " << rps_hostname() << std::endl
		    << "This executable was built by "
		    << rps_building_user_name
		    << " of email " << rps_building_user_email
		    << std::endl;
          {
            char cwdbuf[256];
            memset (cwdbuf, 0, sizeof(cwdbuf));
            if (getcwd(cwdbuf, sizeof(cwdbuf)))
              std::cout << " in " << cwdbuf;
          };
          std::cout << std::endl << " C++ compiler: " << rps_cxx_compiler_version << std::endl
		    << " extra compilation flags: " << rps_build_xtra_cflags << std::endl
                    << " free software license: GPLv3+, see https://gnu.org/licenses/gpl.html" << std::endl
                    << "+++++ there is no WARRANTY, to the extent permitted by law ++++" << std::endl
                    << "***** see also refpersys.org *****"
                    << std::endl << std::endl;
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    };				// end switch key
  return ARGP_ERR_UNKNOWN;
} // end rps_parse1opt

struct argp argparser_rps;

void
rps_parse_program_arguments(int &argc, char**argv)
{
  errno = 0;
  struct argp_state argstate;
  memset (&argstate, 0, sizeof(argstate));
  argparser_rps.options = rps_progoptions;
  argparser_rps.parser = rps_parse1opt;
  argparser_rps.args_doc = " ; # ";
  argparser_rps.doc =
    "RefPerSys - an opensource Artificial Intelligence project,\n"
    " open science, for Linux/x86-64; see refpersys.org for more.\n"
    " (REFlexive PERsystem SYStem is GPLv3+ licensed free software)\n"
    " You should have received a copy of the GNU General Public License\n"
    " along with this program.  If not, see www.gnu.org/licenses\n"
    " *** NO WARRANTY, not even for FITNESS FOR A PARTICULAR PURPOSE ***\n"
    " +++!!! use at your own risk !!!+++\n"
    " (shortgitid " RPS_SHORTGITID " built at " __DATE__ ")\n"
    "\n Accepted program options are:\n";
  argparser_rps.children = nullptr;
  argparser_rps.help_filter = nullptr;
  argparser_rps.argp_domain = nullptr;
  if (argp_parse(&argparser_rps, argc, argv, 0, nullptr, nullptr))
    RPS_FATALOUT("failed to parse program arguments to " << argv[0]);
} // end rps_parse_program_arguments



extern "C" void rps_run_test_repl_lexer(const std::string&); // defined in file lexer_rps.cc
void
rps_run_application(int &argc, char **argv)
{

  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED, //
		 /*callerframe:*/RPS_NULL_CALL_FRAME,
		 Rps_ObjectRef tempob;
                );
  {
    char cwdbuf[128];
    memset (cwdbuf, 0, sizeof(cwdbuf));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
      RPS_FATALOUT("rps_run_application failed to getcwd " << strerror(errno)
                   << RPS_FULL_BACKTRACE_HERE(1, "rps_run_application"));
    RPS_INFORM("rps_run_application: start of %s\n"
               ".. gitid %s\n"
               ".. build timestamp %s\n"
               ".. last git commit %s\n"
               ".. md5sum %s\n"
               ".. in %s\n"
               ".. on host %s pid %d\n",
               argv[0], rps_gitid,
               rps_timestamp,
               rps_lastgitcommit,
               rps_md5sum,
               cwdbuf,
               rps_hostname(), (int)getpid());
  }
  /// if told, enable extra debugging after load
  if (rps_debugflags_after_load)
    {
      rps_set_debug(rps_debugflags_after_load);
      RPS_INFORM("rps_run_application did set debug after load to %s", rps_debugflags_after_load);
    }
  ////
  if (rps_without_quick_tests)
    {
      RPS_INFORM("rps_run_application dont run quick tests after load");
    }
  else
    {
      RPS_DEBUG_LOG(LOWREP, "rps_run_application before running rps_small_quick_tests_after_load from "
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_run_application/quick-tests")
                    << std::endl
                    << " with call frame:" << std::endl
                    << Rps_ShowCallFrame(&_));
      rps_small_quick_tests_after_load();
      RPS_DEBUG_LOG(LOWREP, "rps_run_application after running rps_small_quick_tests_after_load");
    };
  /// create the fifos if a prefix is given with 
   if (!rps_fifo_prefix.empty())
     rps_do_create_fifos(rps_fifo_prefix);
  //// running the given Unix command after load
  if (rps_run_command_after_load)
    {
      if (rps_fifo_prefix.empty())
      RPS_INFORM("before running command '%s' after load with environment variables...\n"
		 " REFPERSYS_PID=%ld, REFPERSYS_GITID=%s, REFPERSYS_TOPDIR=%s",
                 rps_run_command_after_load, (long)getpid(), rps_gitid, rps_topdirectory);
      else
	RPS_INFORM("before running command '%s' after load with environment variables...\n"
		   "... REFPERSYS_PID=%ld, REFPERSYS_GITID=%s,\n"
		   "... REFPERSYS_TOPDIR=%s, REFPERSYS_FIFO_PREFIX=%s",
		   rps_run_command_after_load, (long)getpid(), rps_gitid,
		   rps_topdirectory, rps_fifo_prefix.c_str());
      fflush(nullptr); /// needed before system
      int nok = system(rps_run_command_after_load);
      if (nok)
        RPS_FATAL("failed to run command '%s' after load (status #%d)",
                  rps_run_command_after_load, nok);
      else
        RPS_INFORM("after successfully running command '%s' after load", rps_run_command_after_load);
    }
  else if (!rps_fifo_prefix.empty()) {
      RPS_INFORM("before running default GUI command '%s'  after load"
		 " with environment variables...\n"
		 "... REFPERSYS_PID=%ld, REFPERSYS_GITID=%s,\n"
		 " ... REFPERSYS_TOPDIR=%s, REFPERSYS_FIFO_PREFIX=%s",
		 rps_gui_script_executable, (long)getpid(), rps_gitid,
		 rps_topdirectory, rps_fifo_prefix.c_str());
      std::string fifo_cmd_path, fifo_out_path;
      fifo_cmd_path = rps_get_fifo_prefix() + ".cmd";
      fifo_out_path = rps_get_fifo_prefix() + ".out";
      /* TODO: use rps_is_fifo to avoid useless mkfifo */
#warning should mkfifo here and fill 
      fflush(nullptr);
      pid_t guipid = fork();
      if (guipid < 0)
	RPS_FATALOUT("failed to fork for running the GUI script"
		     << rps_gui_script_executable);
      if (guipid == 0) {
	// child process
	// close many file desriptors
	for (int fd=3; fd<256; fd++)
	  close(fd);
	close(STDIN_FILENO);
	int nullfd = open("/dev/null", O_RDONLY);
	if (nullfd>0) //unlikely
	  dup2(nullfd, STDIN_FILENO);
	execl(rps_gui_script_executable, rps_gui_script_executable,
	      rps_fifo_prefix.c_str(), nullptr);
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
      RPS_INFORMOUT("rps_run_application should edit user C++ code with editor='"
                    << rps_cpluspluseditor_str << "' and compile flags '" << rps_cplusplusflags_str << "'"
                    << std::endl
                    << " with call frame " << Rps_ShowCallFrame(&_));
      rps_edit_run_cplusplus_code (&_);
    }
  //// running the given plugins after load - should happen after
  //// edition of C++ code
  if (!rps_plugins_vector.empty())
    {
      int pluginix = 0;
      try
        {
          for (auto& curplugin : rps_plugins_vector)
            {

              void* dopluginad = dlsym(curplugin.plugin_dlh, RPS_PLUGIN_INIT_NAME);
              if (!dopluginad)
                RPS_FATALOUT("cannot find symbol " RPS_PLUGIN_INIT_NAME " in plugin " << curplugin.plugin_name << ":" << dlerror());
              rps_plugin_init_sig_t* pluginit = reinterpret_cast<rps_plugin_init_sig_t*>(dopluginad);
              (*pluginit)(&curplugin);
              pluginix ++;
            };
        }
      catch (std::exception& exc)
        {
          RPS_WARNOUT("rps_run_application failed to run plugin #" << pluginix
                      << rps_plugins_vector[pluginix].plugin_name
                      << " got exception " << exc.what()
                     );
        }
    };
  /////
  ///// testing the REPL lexer
  if (!rps_test_repl_string.empty()) {
    rps_run_test_repl_lexer(rps_test_repl_string);
  }
  /////
  /// publish using web techniques information about this process
  if (!rps_publisher_url_str.empty())
    rps_publish_me(rps_publisher_url_str.c_str());

  ////  command vectors
  if (!rps_command_vec.empty())
    {
      RPS_INFORMOUT("before running " << rps_command_vec.size() << " command[s]");
      rps_do_repl_commands_vec(rps_command_vec);
      RPS_INFORMOUT("after running " << rps_command_vec.size() << " commands");
    }
  if (access(rps_gui_script_executable, X_OK))
    RPS_WARNOUT("default GUI script " << rps_gui_script_executable << " is not executable");
  ////
  ////
  if (!rps_fifo_prefix.empty()) {
#pragma message "main_rps.cc with RPSJSONRPC:" __DATE__ "@" __TIME__
    RPS_INFORMOUT("initialize JSONRPC with rps_fifo_prefix:" << rps_fifo_prefix<< std::endl
		  << RPS_FULL_BACKTRACE_HERE(1, "rps_run_application JSONRPC"));
      jsonrpc_initialize_rps();
  };
} // end rps_run_application


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
	   " ** compile-command: \"%s -I%s -I/usr/local/include/  -I/usr/include/jsoncpp -std=gnu++17 -Wall -Wextra -Og -g -shared -fPIC ",
	   rps_cxx_compiler_command, rps_topdirectory);
  fprintf (tfil, "%s ", tempcppfilename);
  fprintf (tfil, " -o /tmp/rpsplug_%s.so\" ;;\n",  _f.tempob->oid().to_string().c_str());
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
      if (cmdbad != 0) {
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
                    << Rps_ShowCallFrame(&_));
      std::string cwdpath;
      bool needchdir = false;
      {
        // not very good, but in practice good enough before bootstrapping
        // see https://softwareengineering.stackexchange.com/q/289427/40065
        char cwdbuf[128];
        memset(cwdbuf, 0, sizeof(cwdbuf));
        if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
          RPS_FATAL("rps_edit_run_cplusplus_code getcwd failed: %m");
        cwdpath = std::string(cwdbuf);
      }
      needchdir = cwdpath != std::string{};
      //// our compilation command is...
      std::string buildplugincmd{rps_topdirectory};
      buildplugincmd += "/build-plugin.sh ";
      buildplugincmd += tempcppfilename;
      buildplugincmd += " ";
      buildplugincmd += tempsofilename;
      RPS_WARNOUT("rps_edit_run_cplusplus_code incomplete for C++ code in "<< tempcppfilename
                  << " should compile into " << tempsofilename
                  << " using either make plugin or build-plugin.sh"
                  << std::endl
                  << " - from "
                  << Rps_ShowCallFrame(&_)
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_edit_run_cplusplus_code *incomplete*"));
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
      if (buildres != 0) {
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
    };				// end while !cppcompilegood

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
};				// end Rps_Status::output

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



////////////////
void
rps_fatal_stop_at (const char *filnam, int lin)
{
  static constexpr int skipfatal=2;
  assert(filnam != nullptr);
  assert (lin>=0);
  char errbuf[80];
  memset (errbuf, 0, sizeof(errbuf));
  snprintf (errbuf, sizeof(errbuf), "FATAL STOP (%s:%d)", filnam, lin);
  bool ontty = isatty(STDERR_FILENO);
  if (rps_debug_file)
    fprintf(rps_debug_file, "\n*§*§* RPS FATAL %s:%d *§*§*\n", filnam, lin);
  fprintf(stderr, "\n" "%s%sRPS FATAL:%s\n"
          " RefPerSys gitid %s,\n"
          "\t built timestamp %s,\n"
          "\t on host %s, md5sum %s,\n"
          "\t elapsed %.3f, process %.3f sec\n",
          ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
          ontty?RPS_TERMINAL_BLINK_ESCAPE:"",
          ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
          rps_gitid, rps_timestamp, rps_hostname(), rps_md5sum,
          rps_elapsed_real_time(), rps_process_cpu_time());
  if (rps_debug_file && rps_debug_file != stderr && rps_debug_path[0])
    {
      fprintf(stderr, "*°* see debug output in %s\n", rps_debug_path);
      fprintf(rps_debug_file, "RefPerSys gitid %s built %s was started on %s pid %d as:\n",
	      rps_shortgitid, rps_timestamp, rps_hostname(), (int)getpid());
      for (int aix=0; aix<rps_argc; aix++) {
	fputc(' ', rps_debug_file);
	const char*curarg = rps_argv[aix];
	bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
	  || curarg[0]=='_' || curarg[0]=='.' || curarg[0]=='-';
	for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
	  isplainarg = *pc>' ' && *pc<(char)127
	    && *pc != '\'' && *pc != '\\' && *pc != '\"'
	    && isprint(*pc);
	if (isplainarg)
	  fputs(curarg, rps_debug_file);
	else
	  fprintf(rps_debug_file, "'%s'", Rps_QuotedC_String(curarg).c_str());
      }
      fputc('\n', rps_debug_file);
    }
  fflush (stderr);
  fflush (rps_debug_file);
  {
    auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                               filnam, lin,
                               skipfatal, "RefPerSys FATAL ERROR",
                               &std::clog);
    backt.output(std::clog);
    std::clog << "===== end fatal error at " << filnam << ":" << lin
              << " ======" << std::endl << std::flush;
    std::clog << "RefPerSys gitid " << rps_shortgitid << " built " << rps_timestamp
	      << " was started on " << rps_hostname() << " pid " << (int)getpid() << " as:" << std::endl;
      for (int aix=0; aix<rps_argc; aix++) {
	const char*curarg = rps_argv[aix];
	bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
	  || curarg[0]=='_' || curarg[0]=='.'  || curarg[0]=='-';
	for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
	  isplainarg = *pc>' ' && *pc<(char)127
	    && *pc != '\'' && *pc != '\\' && *pc != '\"'
	    && isprint(*pc);
	if (isplainarg)
	  std::clog << ' ' << curarg;
	else
	  std::clog << ' ' << Rps_QuotedC_String(curarg);
      }
      std::clog << std::endl << std::flush;
  }
  fflush(nullptr);
  abort();
} // end rps_fatal_stop_at


void rps_debug_warn_at(const char*file, int line)
{
  if (rps_debug_file)
    {
      fprintf(rps_debug_file, "\n*** REFPERSYS WARNING at %s:%d ***\n", file, line);
      fflush(rps_debug_file);
    }
  else
    {
      std::cerr << std::flush;
      std::cerr << std::endl << "**!** REFPERSYS WARNING at " << file << ":" << line << std::endl;
    }
} // end rps_debug_warn_at

///////////////////////////////////////////////////////// debugging support
/// X macro tricks used twice below... see en.wikipedia.org/wiki/X_Macro
bool
rps_set_debug_flag(const std::string &curlev)
{
  bool goodflag = false;
  if (curlev == "NEVER")
    {
      RPS_WARNOUT("forbidden debug level " << curlev);
    }
  else if (curlev == "help")
    {
      goodflag = true;
    }
  ///
  /* second X macro trick for processing several comma-separated debug flags, in all cases as else if branch  */
  ///
#define Rps_SET_DEBUG(Opt)						\
  else if (curlev == #Opt) {						\
    bool alreadygiven = rps_debug_flags & (1 << RPS_DEBUG_##Opt);	\
    rps_debug_flags |= (1 << RPS_DEBUG_##Opt);				\
    goodflag = true;							\
    if (!alreadygiven)							\
      RPS_INFORMOUT("setting debugging flag " << #Opt);	 }
  ///
  RPS_DEBUG_OPTIONS(Rps_SET_DEBUG);
#undef Rps_SET_DEBUG
  ////
  if (!goodflag)
    RPS_WARNOUT("unknown debug level " << curlev);
  return goodflag;
} // end rps_set_debug_flag

void
rps_set_debug(const std::string &deblev)
{
  static bool didhelp;
  if (deblev == "help" && !didhelp)
    {
      /* first X macro for help debug flag.... */
      didhelp = true;
      fprintf(stderr, "%s debugging options for git %s built at %s ...\n",
              rps_progname, rps_shortgitid, rps_timestamp);
      fprintf(stderr, "Comma separated debugging levels with -D<debug-level> or --debug=<debug-level> or --debug-after-load=<debug-level>:\n");

#define Rps_SHOW_DEBUG(Opt) fprintf(stderr, "\t%s\n", #Opt);
      RPS_DEBUG_OPTIONS(Rps_SHOW_DEBUG);
#undef Rps_SHOW_DEBUG

      fflush(nullptr);
    }
  else
    {
      const char*comma=nullptr;
      for (const char*pc = deblev.c_str(); pc && *pc; pc = comma?(comma+1):nullptr)
        {
          comma = strchr(pc, ',');
          std::string curlev;
          if (comma && comma>pc)
            curlev = std::string(pc, comma-pc);
          else
            curlev = std::string(pc);
          if (!rps_set_debug_flag(curlev))
            RPS_FATALOUT("unexpected debug level " << curlev
                         << "; use --debug=help to get all known debug levels");
        };			// end for const char*pc ...

    } // else case, for deblev which is not help

  RPS_DEBUG_LOG(MISC, "rps_debug_flags=" << rps_debug_flags);
} // end rps_set_debug



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
      if (rps_debug_path[0])
        fprintf(rps_debug_file, "\n\n*** end of RefPerSys debug file %s ****\n", rps_debug_path);
      else
        fprintf(rps_debug_file, "\n\n*** end of RefPerSys debug ***\n");
      fprintf(rps_debug_file, "gitid %s, built %s, on host %s, md5sum %s, elapsed %.3f, process %.3f sec\n",
              rps_gitid, rps_timestamp, rps_hostname(),  rps_md5sum,
              rps_elapsed_real_time(), rps_process_cpu_time());
      fflush(rps_debug_file);
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
          "On host %s pid %d gitid %s topdir %s\n"
          "####################################\n\n",
          filepath,
          rps_timestamp,
          rps_hostname(), (int)getpid(), rps_gitid, rps_topdirectory);
  fflush(fdbg);
  rps_debug_file = fdbg;
  strncpy(rps_debug_path, filepath, sizeof(rps_debug_path)-1);
  atexit(rps_close_debug_file);
} // end rps_set_debug_output_path

////////////////////////////////////////////////////////////////
// if fline is negative, print a newline before....
void
rps_debug_printf_at(const char *fname, int fline, Rps_Debug dbgopt,
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
  char bfr[160];
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
  static long debug_count = 0;

  {
    pthread_mutex_lock(&rps_debug_mutex);
    long ndbg = debug_count++;
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


const char*
rps_get_extra_arg(const char*name)
{
  if (!name) return nullptr;
  bool is_good_name=isalpha(name[0]);
  for (const char*pc = name; is_good_name && *pc; pc++)
    is_good_name = isalnum(*pc) || *pc == '_';
  if (!is_good_name)
    return nullptr;
  std::string goodstr{name};
  auto it = rps_dict_extra_arg.find(goodstr);
  if (it == rps_dict_extra_arg.end())
    return nullptr;
  return it->second.c_str();
} // end rps_get_extra_arg

/////////////////// end of file main_rps.cc

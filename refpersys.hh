/****************************************************************
 * file refpersys.hh
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is almost its only public C++ header file.
 *
 * Author(s):
 *      Basile Starynkevitch, France   <basile@starynkevitch.net>
 *      Abhishek Chakravarti, India    <abhishek@taranjali.org>
 *      Nimesh Neema, India            <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2025 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * You can consider RefPerSys as either GPLv3+ or LGPLv3+ licensed (at
 * your choice)
 *
 * License: GPLv3+ (file COPYING-GPLv3)
 *    This software is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 * Alternative license: LGPLv3+ (file COPYING-LGPLv3)
 *    This software is is free software: you can
 *    redistribute it and/or modify it under the terms of the GNU
 *    Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details or the Lesser
 *    General Public License.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/



#ifndef REFPERSYS_INCLUDED
#define REFPERSYS_INCLUDED

// A name containing `unsafe` refers to something which should be used
// with great caution.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#if __cplusplus < 201412L
#error expecting C++17 standard
#endif

#include <set>
#include <map>
#include <deque>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <new>
#include <random>
#include <istream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <functional>
#include <typeinfo>
#include <locale>

#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <clocale>
#include <filesystem>


///
#include <argp.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/syslog.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <setjmp.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pthread.h>
#include <limits.h>
#include <locale.h>
#include <libintl.h> //// gettext(3) and friends
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/personality.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <wordexp.h>
#include <glob.h>



/// libssh2-1-dev package on Debian
#include <libssh2.h>

/// libgmp-dev package on Debian (GNU multiprecision library, and its
/// C++ wrapper)
#include <gmpxx.h>

#define RPS_WITH_FLTK 1 /* could be 1 if using fltk.org graphical toolkit */

//// the generated/rpsdata.h contain only preprocessor #define-s and #undef
//// it may undef RPS_WITH_FLTK. It has a pragma message
//// it is simpler to not use it... (but needed in some files)
#ifdef RPS_WITH_DATA
#include "generated/rpsdata.h"
#endif //RPS_WITH_DATA

extern "C" const char* rps_locale(void);


/// keep the debug options in alphabetical order
#define RPS_DEBUG_OPTIONS(dbgmacro) \
  dbgmacro(CMD)                     \
  dbgmacro(CODEGEN)                 \
  dbgmacro(COMPL_REPL)              \
  dbgmacro(DUMP)                    \
  dbgmacro(EVENT_LOOP)              \
  dbgmacro(GARBAGE_COLLECTOR)       \
  dbgmacro(GUI)                     \
  dbgmacro(LOAD)                    \
  dbgmacro(LOWREP)                  \
  dbgmacro(LOW_REPL)                \
  dbgmacro(MISC)                    \
  dbgmacro(MSGSEND)                 \
  dbgmacro(PARSE)                   \
  dbgmacro(PARSE_STRING)            \
  dbgmacro(PROGARG)                 \
  dbgmacro(REPL)                    \
  /*end RPS_DEBUG_OPTIONS*/

#define RPS_DEBUG_OPTION_DEFINE(dbgopt) RPS_DEBUG_##dbgopt,

/// passed to rps_fltk_show_debug_message as the debug level for
/// warning, inform, fatal
constexpr int RPS_INFORM_MSG_LEVEL= -1;
constexpr int RPS_WARNING_MSG_LEVEL= -2;
constexpr int RPS_FATAL_MSG_LEVEL= -3;



enum Rps_Debug
{
  RPS_DEBUG__FATAL_MSG_LEVEL=  RPS_FATAL_MSG_LEVEL /*ie -3*/,
  RPS_DEBUG__WARNING_MSG_LEVEL=  RPS_WARNING_MSG_LEVEL /*ie -2*/,
  RPS_DEBUG__INFORM_MSG_LEVEL= RPS_INFORM_MSG_LEVEL /*ie -1*/,
  RPS_DEBUG__NONE= 0,
  // expands to RPS_DEBUG_CMD, RPS_DEBUG_DUMP, RPS_DEBUG_GARBAGE_COLLECTOR...
  RPS_DEBUG_OPTIONS(RPS_DEBUG_OPTION_DEFINE)
  RPS_DEBUG__LAST,
  RPS_DEBUG__EVERYTHING=0xffff,
};

// forward declaration
class Rps_ProtoCallFrame;
typedef Rps_ProtoCallFrame Rps_CallFrame;

typedef void Rps_EventHandler_sigt(Rps_CallFrame*, int /*fd*/, void* /*data*/);

#if RPS_WITH_FLTK
extern "C" int rps_fltk_get_abi_version (void);
extern "C" int rps_fltk_get_api_version (void);
extern "C" void rps_fltk_initialize (int argc, char**argv);
extern "C" void rps_fltk_progoption(char*arg, struct argp_state*, bool side_effect);
extern "C" bool rps_fltk_enabled (void);
extern "C" void rps_fltk_run (void);
extern "C" void rps_fltk_stop (void);
extern "C" void rps_fltk_flush (void);
extern "C" void rps_fltk_show_debug_message(const char*file, int line, const char*funcname,
    Rps_Debug dbgopt, long dbgcount,
    const char*msg);
extern "C" void rps_fltk_printf_inform_message(const char*file, int line, const char*funcname, long dbgcount,
    const char*fmt, ...)
__attribute__ ((format (printf, 5, 6)));
/* add an input file descriptor event handler to FLTK event loop */
extern "C" void rps_fltk_add_input_fd(int fd,
                                      Rps_EventHandler_sigt* f,
                                      const char* explanation,
                                      int ix);
/* add an output file descriptor event handler to FLTK event loop */
extern "C" void rps_fltk_add_output_fd(int fd,
                                       Rps_EventHandler_sigt* f,
                                       const char* explanation,
                                       int ix);
/* remove an input file descriptor event handler from FLTK event loop */
extern "C" void rps_fltk_remove_input_fd(int fd);
/* remove an output file descriptor event handler from FLTK event loop */
extern "C" void rps_fltk_remove_output_fd(int fd);
/* emit the size and align */
extern "C" void rps_fltk_emit_sizes(std::ostream&out);
#else /*not RPS_WITH_FLTK*/
#define rps_fltk_get_abi_version() 0
#define rps_fltk_get_api_version() 0
#define rps_fltk_initialize() do {}while(0)
#define rps_fltk_progoption(Arg,State,SidEff) do {}while(0)
#define rps_fltk_enabled() false
#define rps_fltk_add_input_fd(Fd,Fun,Expl,Ix) do {}while(0)
#define rps_fltk_add_output_fd(Fd,Fun,Expl,Ix) do {}while(0)
#define rps_fltk_remove_input_fd(Fd) do{}while(0)
#define rps_fltk_remove_output_fd(Fd) do{}while(0)
#define rps_fltk_stop() do{}while(0)
#define rps_fltk_run() do{}while(0)
#define rps_fltk_flush() do{}while(0)
#define rps_fltk_emit_sizes(Out) do{}while(0)
#define rps_fltk_printf_inform_message(File,Lin,Funcname,Count,Fmt,...) do{}while(0)
#endif

class Rps_QuasiZone; // GC-managed piece of memory
class Rps_ZoneValue; // memory for values
class Rps_ObjectZone; // memory for objects
class Rps_JsonZone; // memory for Json values
class Rps_LexTokenZone; /// memory for reified lexical tokens, mostly in repl_rps.cc
class Rps_DequVal;
class Rps_GarbageCollector;
class Rps_Payload;
class Rps_PayloadSymbol;
class Rps_PayloadClassInfo;
class Rps_PayloadStrBuf;
class Rps_PayloadWebPi;
class Rps_PayloadAgenda;
class Rps_PayloadTasklet;
class Rps_PayloadUnixProcess;   // transient payload for forked processes
class Rps_PayloadPopenedFile;   // transient payload for popened command
class Rps_PayloadCppStream;     // transient payload for C++ streams
class Rps_PayloadGccJit;  //  payload for libgccjit
// code generation
#if RPS_WITH_FLTK
class Rps_PayloadFltkThing;
class Rps_PayloadFltkWidget;
class Rps_PayloadFltkWindow;
//TODO: add perhaps Rps_PayloadFltkWindow?
#endif
class Rps_Loader;
class Rps_Dumper;
class Rps_ProtoCallFrame;
class Rps_TokenSource;
class Rps_Value;
class Rps_Id;


typedef uint32_t Rps_HashInt;
typedef Rps_ProtoCallFrame Rps_CallFrame;

constexpr unsigned rps_path_byte_size = 384;
extern "C" char rps_bufpath_homedir[rps_path_byte_size];
extern "C" char rps_loaded_directory[rps_path_byte_size];
extern "C" char rps_debug_path[rps_path_byte_size];

extern "C" int rps_get_major_version(void);
extern "C" int rps_get_minor_version(void);
#define RPS_MAJOR_VERSION_NUM 0
#define RPS_MINOR_VERSION_NUM 6


extern "C" std::map<std::string,std::string> rps_pluginargs_map;
extern "C" std::string rps_cpluspluseditor_str;
extern "C" std::string rps_cplusplusflags_str;
extern "C" std::string rps_dumpdir_str;
extern "C" std::vector<std::string> rps_command_vec;
extern "C" std::string rps_test_repl_string;
extern "C" std::string rps_publisher_url_str;
extern "C" bool rps_without_quick_tests;

/// Given some SHORTPATH like "foo123.xyz" return a temporary unique
/// full path in the dump directory which would be renamed at end of
/// dump to the SHORTPATH...
extern "C" std::string rps_dumper_temporary_path(Rps_Dumper*du, std::string shortpath);

extern "C" char* rps_run_command_after_load;
extern "C" char* rps_debugflags_after_load;

extern "C" const char* rps_get_proc_version(void);

extern "C" std::string rps_run_name;

extern "C" std::string rps_stringprintf(const char*fmt, ...)
__attribute__((format (printf, 1, 2))); // in utilities_rps.cc


extern "C" {

#define RPS_USE_CURL 0 // temporary, see https://bugs.gentoo.org/939581 Invalid conversion from int to CURLoption 

#if RPS_USE_CURL
  // https://curl.se/libcurl/ is a web client library
#include "curl/curl.h"
#endif /*RPS_USE_CURL*/

// before November 2024 we used GNU lightning from https://www.gnu.org/software/lightning


};

// http://man7.org/linux/man-pages/man3/gnu_get_libc_version.3.html
#include <gnu/libc-version.h>


// for programmatic C++ name demangling, see also
// https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/cxxabi.h
#include <cxxabi.h>



// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG jsoncpp
// JsonCPP https://github.com/open-source-parsers/jsoncpp
#include "json/json.h"


// GNU libunistring https://www.gnu.org/software/libunistring/
// we use UTF-8 strings
#include "unistr.h"

#include "backtrace.h"

// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define RPS_UNLIKELY(P) __builtin_expect(!!(P),0)
#define RPS_LIKELY(P) !__builtin_expect(!(P),0)
#define RPS_UNUSED __attribute__((unused))
#else
#define RPS_UNLIKELY(P) (P)
#define RPS_LIKELY(P) (P)
#define RPS_UNUSED
#endif

#define RPS_FRIEND_CLASS(Suffix) friend class Rps_##Suffix

// generated in __timestamp.c by rps-generate-timestamp.sh utility script
extern "C" const char rps_timestamp[];
extern "C" unsigned long rps_timelong;
extern "C" const char rps_topdirectory[];
extern "C" const char rps_gitid[];
extern "C" const char rps_shortgitid[];
extern "C" const char rps_gitbranch[];
extern "C" const char rps_lastgittag[];
extern "C" const char rps_lastgitcommit[];
extern "C" const char rps_md5sum[];
extern "C" const char*const rps_files[];
/// the GNU make utility see www.gnu.org/software/make/
extern "C" const char rps_gnumakefile[];
extern "C" const char rps_gnu_make[];
extern "C" const char rps_gnu_make_version[];
extern "C" const char rps_gnu_make_features[];
/// the GNU bison parser generator see www.gnu.org/software/bison/
extern "C" const char rps_gnu_bison[];
extern "C" const char rps_gnu_bison_version[];

/// the Carburetta parser generator see carburetta.com & github.com/kingletbv/carburetta
extern "C" const char rps_carburetta[];
extern "C" const char rps_carburetta_version[];


/// subdirectoris
extern "C" const char*const rps_subdirectories[];

/// realpath and version of the GNU C++ compiler see gcc.gnu.org
extern "C" const char rps_cxx_compiler_realpath[];
extern "C" const char rps_cxx_compiler_version[];

/// the GPP generic preprocessor from logological.org/gpp
/// we may in 2025 not need to use it
extern "C" const char rps_gpp_preprocessor_command[];
extern "C" const char rps_gpp_preprocessor_realpath[];
extern "C" const char rps_gpp_preprocessor_version[];


extern "C" const char rps_plugin_builder[];

/// In commit 92c6e6b70d2 of Feb, 8, 2024 we used to mention GNU bison and GPP
/// GNU bison is a parser generator, see www.gnu.org/software/bison/
/// GPP is a general purpose preprocessor, see logological.org/gpp
/// we did had some constants in __timestamp.c related to them.


///// a process running the GUI communicating using JSONRPC
extern "C" const char rps_gui_script_executable[];

// the Linux user compiling RefPerSys
extern "C" const char rps_building_user_name[];
extern "C" const char rps_building_user_email[];

/// the computer on which this RefPerSys is built
extern "C" const char rps_building_host[];
extern "C" const char rps_building_operating_system[];
extern "C" const char rps_building_opersysname[]; /// with only letters, digits, underscores
extern "C" const char rps_building_machine[];
extern "C" const char rps_building_machname[]; /// with only letters, digits, underscores

/// current utsname
extern "C" struct utsname rps_utsname;


////////////////////////////////////////////////////////////////
//// from RefPerSys program arguments
extern "C" bool rps_daemonized;  /// --daemon option
extern "C" bool rps_syslog_enabled; /// --syslog option

///////////////////////////////////////////////////////////////////////////////
/// Provides miscellaneous runtime information for RefPerSys.
///

extern "C" pid_t rps_gui_pid;

/// a pair of Unix file descriptors, JSONRPC....
struct rps_fifo_fdpair_st
{
  int fifo_ui_wcmd; // the commands written to the GUI process
  int fifo_ui_rout; // the outputs read from the GUI process
};
extern "C" void rps_put_fifo_prefix(const char*pref);

//// postpone, by popening /bin/at, the remove of a file 5 minutes
//// after RefPerSys normally exited.
extern "C" void rps_postponed_remove_file(const std::string& path);

/// this is scheduling their removal, called by atexit
extern "C" void rps_schedule_files_postponed_removal(void);

extern "C" std::string rps_get_fifo_prefix(void);
extern "C" void rps_do_create_fifos(std::string prefix);
extern "C" struct rps_fifo_fdpair_st rps_get_gui_fifo_fds(void);
extern "C" pid_t rps_get_gui_pid(void);
extern "C" bool rps_is_fifo(std::string path); // in eventloop_rps.cc

extern "C" void rps_parse_program_arguments(int& argc, char**argv);

extern "C" void rps_run_test_repl_lexer(const std::string&); // defined in file lexer_rps.cc

/// actually, in function main we have something like  asm volatile ("rps_end_of_main: nop");
extern "C" void rps_end_of_main(void);


extern "C" void rps_edit_run_cplusplus_code (Rps_CallFrame*callerframe);

extern "C" void rps_small_quick_tests_after_load (void);

extern "C" void rps_check_mtime_files (void);

// when set, no GUI is running
extern "C" bool rps_batch;

/// https://en.wikipedia.org/wiki/Address_space_layout_randomization
extern "C" bool rps_disable_aslr;

extern "C" bool rps_run_repl;


extern "C" void rps_jsonrpc_initialize(void);
extern "C" void rps_gccjit_initialize(void);
extern "C" void rps_gccjit_finalize(void); // passed to atexit(3)

/// Our event loop can call C++ closures before the poll(2) system
/// call in the event loop. This C++ closure (or std::function) could
/// add additional sources to the event loop. This
/// rps_register_event_loop_prepoller function returns some index for
/// the unregistering function.
extern "C" int rps_register_event_loop_prepoller(std::function<void (struct pollfd*, int& npoll, Rps_CallFrame*)> fun);
extern "C" void rps_unregister_event_loop_prepoller(int rank);
extern "C" bool rps_is_active_event_loop(void);

/// register C function input handler
extern "C" void rps_event_loop_add_input_fd_handler
(int fd,
 Rps_EventHandler_sigt*cfun,
 const char* explanation = nullptr,
 void*data = nullptr);
extern "C" void rps_event_loop_add_output_fd_handler
(int fd,
 Rps_EventHandler_sigt*cfun,
 const char* explanation = nullptr,
 void*data = nullptr);
extern "C" void rps_event_loop_remove_input_fd_handler(int fd);
extern "C" void rps_event_loop_remove_output_fd_handler(int fd);
extern "C" void rps_initialize_event_loop(void);
extern "C" void rps_event_loop(void); // run the event loop
extern "C" void rps_run_after_event_loop(void); // run after some event loop
extern "C" void rps_register_after_event_loop(std::function<void(void)>f); // run after some event loop
extern "C" void rps_do_stop_event_loop(void);
// in eventloop_rps.cc, tell if the event loop is running.
extern "C" bool rps_event_loop_is_running(void);
/* return true, and fill the information, about entry#ix in event loop
   internal data, or else return false */
extern "C" bool rps_event_loop_get_entry(int ix,
    Rps_EventHandler_sigt** pfun, struct pollfd*po, const char**pexpl, void**pdata);
// in eventloop_rps.cc, give the counter for the loop, or -1 if it is
// not running.
extern "C" long rps_event_loop_counter(void);

/// backtrace support
extern "C" struct backtrace_state* rps_backtrace_common_state;

/// the program name
extern "C" const char* rps_progname;
/// the program executable (readlink /proc/self/exe)
extern "C" char rps_progexe[];
/// the program arguments
extern "C" int rps_argc;
extern "C" char** rps_argv;
/// the program invocation
extern "C" char* rps_program_invocation;

extern "C" void rps_compute_program_invocation(void);

/// the load directory
extern "C" std::string rps_my_load_dir;



/// the initial copyright year of RefPerSys
#define RPS_INITIAL_COPYRIGHT_YEAR 2019
// the number of jobs, that is of threads, to run in parallel
extern "C" int rps_nbjobs;
#define RPS_NBJOBS_MIN 3
#define RPS_NBJOBS_MAX 24


extern "C" bool rps_stdout_istty;
extern "C" bool rps_stderr_istty;


/// is the current thread the main thread?
extern "C" bool rps_is_main_thread(void);

/// the refpersys homedir, e.g. $REFPERSYS_HOME or $HOME or given with
/// --refpersys-home <dir>
extern "C" const char* rps_homedir(void);

/// the refpersys load directory
extern "C" const std::string& rps_get_loaddir(void);

extern "C" void rps_emit_gplv3_copyright_notice_AT(std::ostream&outs, const char*fil, int lin, const char*fromfunc, std::string path, std::string linprefix, std::string linsuffix, std::string owner="", std::string reason="");

#define rps_emit_gplv3_copyright_notice(Out,...) rps_emit_gplv3_copyright_notice_AT(Out,__FILE__,__LINE__,__PRETTY_FUNCTION__,##__VA_ARGS__)

extern "C" void rps_emit_lgplv3_copyright_notice_AT(std::ostream&outs, const char*fil, int lin, const char*fromfunc, std::string path, std::string linprefix, std::string linsuffix, std::string owner="", std::string reason="");

#define rps_emit_lgplv3_copyright_notice(Out,...) rps_emit_gplv3_copyright_notice_AT(Out,__FILE__,__LINE__,__PRETTY_FUNCTION__,##__VA_ARGS__)

extern "C" FILE*rps_debug_file;

//////////////// fatal error - aborting
extern "C" void rps_fatal_stop_at (const char *, int) __attribute__((noreturn));

#define RPS_FATAL_AT_BIS(Fil,Lin,Fmt,...) do {                          \
    if (rps_syslog_enabled) {                                           \
      syslog(LOG_ALERT, "*+* RefPerSys FATAL:%s:%d: {%s} " Fmt,         \
             Fil, Lin, __PRETTY_FUNCTION__,                             \
             ##__VA_ARGS__);                                            \
    } else {                                                            \
      bool ontty = rps_stderr_istty;                                    \
      fprintf(stderr, "\n\n"                                            \
              "%s*** RefPerSys FATAL:%s%s:%d: {%s}\n " Fmt "\n\n",      \
              (ontty?RPS_TERMINAL_BOLD_ESCAPE:""),                      \
              (ontty?RPS_TERMINAL_NORMAL_ESCAPE:""),                    \
              Fil, Lin, __PRETTY_FUNCTION__,                            \
              ##__VA_ARGS__);                                           \
  };                                                                    \
    if (rps_fltk_enabled())                                             \
      rps_fltk_printf_inform_message(Fil, Lin, __PRETTY_FUNCTION__,     \
             rps_incremented_debug_counter(),                           \
             "FATAL:" Fmt, ##__VA_ARGS__);                              \
  if (rps_debug_file && rps_debug_file != stderr)                       \
    fprintf(rps_debug_file,                                             \
            "\n\n*°* RefPerSys °FATAL° %s:%d:%s " Fmt "*°*\n",          \
            Fil, Lin, __PRETTY_FUNCTION__,                              \
            ##__VA_ARGS__);                                             \
  rps_fatal_stop_at (Fil,Lin); } while(0)

#define RPS_FATAL_AT(Fil,Lin,Fmt,...) RPS_FATAL_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)
#define RPS_FATAL(Fmt,...) RPS_FATAL_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)



#define RPS_FATALOUT_AT_BIS(Fil,Lin,...) do {                   \
      std::ostringstream outl##Lin;                             \
      outl##Lin <<   __VA_ARGS__ << std::flush;                 \
      outl##Lin.flush();                                        \
    if (rps_syslog_enabled) {                                   \
      syslog(LOG_ALERT,                                         \
             "*+* RefPerSys FATAL:%s:%d: {%s} %s",              \
             Fil, Lin, __PRETTY_FUNCTION__,                     \
             outl##Lin.str().c_str());                          \
    } else {                                                    \
      bool ontty = rps_stderr_istty;                            \
      fprintf(stderr, "\n\n"                                    \
              "%s*** RefPerSys FATAL:%s%s:%d: {%s}\n %s\n\n",   \
              (ontty?RPS_TERMINAL_BOLD_ESCAPE:""),              \
              (ontty?RPS_TERMINAL_NORMAL_ESCAPE:""),            \
              Fil, Lin, __PRETTY_FUNCTION__,                    \
              outl##Lin.str().c_str());                         \
  };                                                            \
  if (rps_debug_file && rps_debug_file != stderr)               \
    fprintf(rps_debug_file,                                     \
            "\n\n*°* RefPerSys °FATAL° %s:%d:%s %s*°*\n",       \
            Fil, Lin, __PRETTY_FUNCTION__,                      \
            outl##Lin.str().c_str());                           \
  rps_fatal_stop_at (Fil,Lin); } while(0)

#define RPS_FATALOUT_AT(Fil,Lin,...) RPS_FATALOUT_AT_BIS(Fil,Lin,##__VA_ARGS__)
// typical usage would be RPS_FATALOUT("x=" << x)
#define RPS_FATALOUT(...) RPS_FATALOUT_AT(__FILE__,__LINE__,##__VA_ARGS__)




//////////////// warnings


extern "C" long rps_incremented_debug_counter(void);

extern "C" void rps_debug_warn_at(const char*file, int line);
#define RPS_WARN_AT_BIS(Fil,Lin,Fmt,...) do {                   \
    if (rps_syslog_enabled) {                                   \
      syslog(LOG_WARNING, "RefPerSys WARN:%s:%d: {%s} " Fmt,    \
             Fil, Lin, __PRETTY_FUNCTION__, ##__VA_ARGS__);     \
 } else {                                                       \
    bool ontty##Lin = rps_stderr_istty;                         \
    fprintf(stderr, "\n\n"                                      \
            "%s*** RefPerSys WARN:%s%s:%d: {%s}\n " Fmt "\n\n", \
            ontty##Lin?RPS_TERMINAL_BOLD_ESCAPE:"",             \
            ontty##Lin?RPS_TERMINAL_NORMAL_ESCAPE:"",           \
            Fil, Lin, __PRETTY_FUNCTION__, ##__VA_ARGS__);      \
    rps_debug_warn_at(Fil,Lin);                                 \
    fflush(stderr); } } while(0)

#define RPS_WARN_AT(Fil,Lin,Fmt,...) RPS_WARN_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)

// typical usage could be RPS_WARN("something bad x=%d", x)
#define RPS_WARN(Fmt,...) RPS_WARN_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

#define RPS_WARNOUT_AT_BIS(Fil,Lin,...) do {                    \
    if (rps_syslog_enabled) {                                   \
      std::ostringstream outl##Lin;                             \
      outl##Lin <<   __VA_ARGS__ << std::flush;                 \
      outl##Lin.flush();                                        \
      syslog(LOG_WARNING, "RefPerSys WARN:%s:%d: {%s} %s",      \
             Fil, Lin, __PRETTY_FUNCTION__,                     \
             outl##Lin.str().c_str());                          \
    }                                                           \
    else {                                                      \
      bool ontty##Lin = rps_stderr_istty;                       \
      std::clog << (ontty##Lin?RPS_TERMINAL_BOLD_ESCAPE:"")     \
                << "** RefPerSys WARN!"                         \
                << (ontty##Lin?RPS_TERMINAL_NORMAL_ESCAPE:"")   \
                << " " << (Fil) << ":" << Lin << ":: "          \
                << __VA_ARGS__ << std::endl;                    \
    };                                                          \
    rps_debug_warn_at(Fil,Lin);  } while(0)

#define RPS_WARNOUT_AT(Fil,Lin,...) RPS_WARNOUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be RPS_WARNOUT("annoying x=" << x)
#define RPS_WARNOUT(...) RPS_WARNOUT_AT(__FILE__,__LINE__,##__VA_ARGS__)



static inline pid_t rps_thread_id(void)
{
  return syscall(SYS_gettid, 0L);
} // end rps_thread_id



// see https://en.wikipedia.org/wiki/ANSI_escape_code
extern "C" bool rps_without_terminal_escape;
// adapted from https://github.com/bstarynk
#define RPS_TERMINAL_NORMAL_ESCAPE \
  (rps_without_terminal_escape?"":"\033[0m")
#define RPS_TERMINAL_BOLD_ESCAPE \
  (rps_without_terminal_escape?"":"\033[1m")
#define RPS_TERMINAL_FAINT_ESCAPE \
  (rps_without_terminal_escape?"":"\033[2m")
#define RPS_TERMINAL_ITALICS_ESCAPE \
  (rps_without_terminal_escape?"":"\033[3m")
#define RPS_TERMINAL_UNDERLINE_ESCAPE \
  (rps_without_terminal_escape?"":"\033[4m")
#define RPS_TERMINAL_BLINK_ESCAPE \
  (rps_without_terminal_escape?"":"\033[5m")



extern "C" void rps_set_exit_code(std::uint8_t); // in main_rps.cc

/////////////////////////////////////////////////////// PLUGINS AFTER LOAD

struct Rps_Plugin
{
  std::string plugin_name;
  void* plugin_dlh;
  Rps_Plugin (const char*name, void*dlh)
    : plugin_name(name), plugin_dlh(dlh)
  {
    int plnamlen = plugin_name.length();
    if (plnamlen > 4
        && plugin_name.substr(plnamlen-3) == ".so")
      plugin_name.erase(plnamlen-3);
  };
  ~Rps_Plugin ()
  {
    plugin_name.erase();
    plugin_dlh = nullptr;
  }
};
#define RPS_PLUGIN_INIT_NAME "rps_do_plugin"
typedef void rps_plugin_init_sig_t(const Rps_Plugin*curplugin);
extern "C" rps_plugin_init_sig_t rps_do_plugin;
// if plugin not found or without argument, return the nullptr
extern "C" const char*rps_get_plugin_cstr_argument(const Rps_Plugin*plugin);

extern "C" std::vector<Rps_Plugin> rps_plugins_vector;

////////////////////////////////////////////////////////////////

struct Rps_Status
{
  // sizes in megabytes
  long prog_sizemb_stat;
  long rss_sizemb_stat;
  long shared_sizemb_stat;
  // time in seconds;
  float cputime_stat;
  float elapsedtime_stat;
  Rps_Status() : prog_sizemb_stat(0), rss_sizemb_stat(0),
    shared_sizemb_stat(0), cputime_stat(0.0), elapsedtime_stat(0.0) {};
  static const Rps_Status get(void);
  void output(std::ostream&out) const;
};                              // end struct Rps_Status

inline std::ostream& operator << (std::ostream&out, const Rps_Status& rst)
{
  rst.output(out);
  return out;
};                              // end operator << for Rps_Status


#if RPS_USE_CURL
//// a function to interact with some web service, usually on
//// http://refpersys.org/ to transmit information about this
//// RefPerSys process.. This is related to the --publish-me <URL>
//// program option in main_rps.cc
extern "C" void rps_curl_publish_me(const char*url);
#endif /*RPS_USE_CURL*/
///////////////////////////////////////////////////////////////////////////////
// DEBUGGING MACROS
// Adapted from MELT Monitor project
// https://github.com/bstarynk/melt-monitor/blob/master/meltmoni.hh#L278
///////////////////////////////////////////////////////////////////////////////

extern "C" bool rps_is_set_debug(const std::string &deblev);
extern "C" Rps_Debug rps_debug_of_string(const std::string &deblev);
extern "C" const char*rps_cstr_of_debug(Rps_Debug); /* gives null for
                   non-debug options */

// output a set of debug flags, or rps_debug_flags if flag is zero...
extern "C" void rps_output_debug_flags(std::ostream&out, unsigned flags=0);

/// add or remove a comma or space separated list of debug flags
extern "C" void rps_add_debug_cstr(const char*);
extern "C" void rps_remove_debug_cstr(const char*);

////////////////////////////////////////////////////////////////
///// parsing program options
enum rps_progoption_en
{
  RPSPROGOPT__NONE=0,
  /// keep these options in numerical order!
  RPSPROGOPT_DEBUG_AFTER_LOAD='A',
  RPSPROGOPT_BATCH='B',
  RPSPROGOPT_DUMP='D',
  RPSPROGOPT_FLTK='F',
  RPSPROGOPT_JSONRPC='J',      // no direct GUI, but use JSONRPC
  RPSPROGOPT_LOADDIR='L',
  RPSPROGOPT_COMMAND='c',
  RPSPROGOPT_DEBUG='d',
  RPSPROGOPT_INTERFACEFIFO='i',
  /// see also github.com/bstarynk/misc-basile/blob/master/mini-edit-JSONRPC.md
  RPSPROGOPT_JOBS='j',
  RPSPROGOPT_USER_PREFERENCES='U',
  RPSPROGOPT_HOMEDIR=1000,
  RPSPROGOPT_CHDIR_BEFORE_LOAD,
  RPSPROGOPT_CHDIR_AFTER_LOAD,
  RPSPROGOPT_RANDOMOID,
  RPSPROGOPT_TYPEINFO,
  RPSPROGOPT_SYSLOG,
  RPSPROGOPT_DAEMON,
  RPSPROGOPT_FULL_GIT,
  RPSPROGOPT_SHORT_GIT,
  RPSPROGOPT_PID_FILE,
  RPSPROGOPT_NO_TERMINAL,
  RPSPROGOPT_NO_ASLR,
  RPSPROGOPT_NO_QUICK_TESTS,
  RPSPROGOPT_TEST_REPL_LEXER,
  RPSPROGOPT_RUN_DELAY,
  RPSPROGOPT_RUN_AFTER_LOAD,
  RPSPROGOPT_PLUGIN_AFTER_LOAD,
  RPSPROGOPT_PLUGIN_ARG,
  RPSPROGOPT_CPLUSPLUSEDITOR_AFTER_LOAD,
  RPSPROGOPT_CPLUSPLUSFLAGS_AFTER_LOAD,
  RPSPROGOPT_DEBUG_PATH,
  RPSPROGOPT_EXTRA_ARG,
  RPSPROGOPT_RUN_NAME,
  RPSPROGOPT_ECHO,
  RPSPROGOPT_VERSION,
  RPSPROGOPT_PREFERENCES_HELP,
  RPSPROGOPT_PUBLISH_ME,
};

extern "C" std::string rps_user_preferences_path(void);
extern "C" bool rps_want_user_preferences_help(void);
#define REFPERSYS_DEFAULT_PREFERENCE_PATH ".refpersysrc" /*in $HOME*/

/// if state is RPS_EMPTYSLOT no serious side-effect happens
extern "C" error_t rps_parse1opt (int key, char *arg, struct argp_state *state);
extern "C" struct argp_option rps_progoptions[];

/// The program arguments can contain --extra NAME=VALUE
/// if the NAME is unknown return nullptr...
extern "C" const char*rps_get_extra_arg(const char*name);

extern "C" void rps_do_create_fifos_from_prefix(void);

extern "C" void rps_extend_env(void);

extern "C" unsigned long rps_run_delay; // in seconds
////////////////////////////////////////////////////////////////

extern "C" std::atomic<unsigned> rps_debug_flags;

void rps_set_debug_output_path(const char*filepath);

#define RPS_DEBUG_LOG_LEVEL LOG_DEBUG // syslog debug log level

// so we could code  RPS_DEBUG_PRINTF(NEVER, ....)
#define RPS_DEBUG_NEVER RPS_DEBUG__NONE

#define RPS_DEBUG_ENABLED(dbgopt) (rps_debug_flags & (1 << RPS_DEBUG_##dbgopt))

/// debug print to stderr or syslog or to the file given to
/// rps_set_debug_output_path ....; if fline is negative, print a
/// newline before....
void
rps_debug_printf_at(const char *fname, int fline,const char*funcname, Rps_Debug dbgopt,
                    const char *fmt, ...)
__attribute__ ((format (printf, 5, 6)));


#define RPS_DEBUG_PRINTF_AT(fname, fline, dbgopt, fmt, ...)    \
do                                                             \
  {                                                            \
    if (RPS_DEBUG_ENABLED(dbgopt))                             \
      rps_debug_printf_at(fname, fline,__FUNCTION__,           \
        RPS_DEBUG_##dbgopt, fmt,                               \
                          ##__VA_ARGS__);                      \
  }                                                            \
while (0)


#define RPS_DEBUG_PRINTF_AT_BIS(fname, fline, dbgopt, fmt, ...)  \
   RPS_DEBUG_PRINTF_AT(fname, fline, dbgopt, fmt, ##__VA_ARGS__)

#define RPS_DEBUG_PRINTF(dbgopt, fmt, ...) \
  RPS_DEBUG_PRINTF_AT_BIS(__FILE__, __LINE__, dbgopt, fmt, ##__VA_ARGS__)

#define RPS_DEBUG_NLPRINTF(dbgopt, fmt, ...) \
  RPS_DEBUG_PRINTF_AT_BIS(__FILE__, -__LINE__, dbgopt, fmt, ##__VA_ARGS__)


#define RPS_DEBUG_LOG_AT(fname, fline, dbgopt, logmsg)   do     \
  {                                                             \
    if (RPS_DEBUG_ENABLED(dbgopt))                              \
      {                                                         \
        std::ostringstream _logstream_##fline;                  \
        _logstream_##fline << logmsg << std::flush;             \
        rps_debug_printf_at(fname, fline, __FUNCTION__,         \
          RPS_DEBUG_##dbgopt,                                   \
          "%s",                                                 \
          _logstream_##fline.str().c_str());                    \
      }                                                         \
  }                                                             \
while (0)

#define RPS_DEBUG_LOG_AT_BIS(fname, fline, dbgopt, logmsg)  \
   RPS_DEBUG_LOG_AT(fname, fline, dbgopt, logmsg)

/// example of usage: RPS_DEBUG_LOG(MISC, "x=" << x) related to RPS_DEBUG_MISC
#define RPS_DEBUG_LOG(dbgopt, logmsg) \
  RPS_DEBUG_LOG_AT_BIS(__FILE__, __LINE__, dbgopt, logmsg)

#define RPS_DEBUGNL_LOG_AT(fname, fline, dbgopt, logmsg)        \
  do { /*in RPS_DEBUGNL_LOG_AT*/                                \
    if (RPS_DEBUG_ENABLED(dbgopt))                              \
      {                                                         \
        std::ostringstream _logstream_##fline;                  \
        _logstream_##fline << logmsg << std::flush;             \
        rps_debug_printf_at(fname, -fline, __FUNCTION__,        \
          RPS_DEBUG_##dbgopt,                                   \
                            "%s",                               \
                            _logstream_##fline.str().c_str());  \
      }                                                         \
  }                                                             \
while (0)

#define RPS_DEBUGNL_LOG_AT_BIS(fname, fline, dbgopt, logmsg)  \
   RPS_DEBUGNL_LOG_AT(fname, fline, dbgopt, logmsg)

/// example of usage: RPS_DEBUG_LOG(MISC, "x=" << x) related to RPS_DEBUG_MISC
#define RPS_DEBUGNL_LOG(dbgopt, logmsg) \
  RPS_DEBUGNL_LOG_AT_BIS(__FILE__, __LINE__, dbgopt, logmsg)


//////////////// inform

#define RPS_INFORM_AT_BIS(Fil,Lin,Fmt,...) do {                 \
    bool ontty = rps_stdout_istty;                              \
    if (rps_syslog_enabled) {                                   \
      syslog(LOG_INFO, "RefPerSys INFORM %s:%d: %s " Fmt "\n",  \
       Fil, Lin, __PRETTY_FUNCTION__, ##__VA_ARGS__);           \
    } else {                                                    \
      fprintf(stdout, "\n\n"                                    \
            "%s*** RefPerSys INFORM:%s %s:%d: %s<%s>%s\n "      \
            Fmt "\n\n",                                         \
            ontty?RPS_TERMINAL_BOLD_ESCAPE:"",                  \
            ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",                \
            Fil, Lin,                                           \
            ontty?RPS_TERMINAL_ITALICS_ESCAPE:"",               \
            __PRETTY_FUNCTION__,                                \
            ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",                \
            ##__VA_ARGS__);                                     \
      fflush(stdout); };                                        \
    if (rps_fltk_enabled())                                     \
      rps_fltk_printf_inform_message                            \
        (Fil,Lin,                                               \
         __PRETTY_FUNCTION__,                                   \
         rps_incremented_debug_counter(),                       \
         Fmt,                                                   \
         ##__VA_ARGS__);                                        \
} while(0)

#define RPS_INFORM_AT(Fil,Lin,Fmt,...) RPS_INFORM_AT_BIS(Fil,Lin,Fmt,##__VA_ARGS__)

// typical usage could be RPS_INFORM("something bad x=%d", x)
#define RPS_INFORM(Fmt,...) RPS_INFORM_AT(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

#define RPS_INFORMOUT_AT_BIS(Fil,Lin,...) do {          \
    std::ostringstream outs_##Lin;                      \
    if (rps_syslog_enabled) {                           \
      outs_##Lin << __VA_ARGS__  << std::flush;         \
      syslog(LOG_INFO, "%s:%d:%s %s\n",                 \
       (Fil), (Lin), __PRETTY_FUNCTION__,               \
       outs_##Lin.str().c_str());                       \
    } else {                                            \
    bool ontty = rps_stdout_istty;                      \
    outs_##Lin                                          \
      << (ontty?RPS_TERMINAL_BOLD_ESCAPE:"")            \
      << "** RefPerSys INFORM!"                         \
      <<  (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"") << " "  \
      << (ontty?RPS_TERMINAL_ITALICS_ESCAPE:"")         \
      << (Fil) << ":" << Lin << ": "                    \
      <<  __PRETTY_FUNCTION__                           \
      << (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"")          \
      << ' ' << __VA_ARGS__  << std::flush;             \
    fputs(outs_##Lin.str().c_str(), stdout);            \
    fputc('\n', stdout);                                \
    fflush(stdout);                                     \
  }                                                     \
} while(0)

#define RPS_INFORMOUT_AT(Fil,Lin,...) RPS_INFORMOUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be RPS_INFORMOUT("annoying x=" << x)
#define RPS_INFORMOUT(...) RPS_INFORMOUT_AT(__FILE__,__LINE__,##__VA_ARGS__)


/// this macro RPS_NOPRINT and the following one RPS_NOPRINTOUT are
/// optimized to no-op but still typechecked. They could be used as a
/// drop-in replacement to RPS_INFORM and RPS_INFORMOUT, so that only
/// a few letters of change are needed, while keeping a previous debug
/// or information output....
#define RPS_NOPRINT(Fmt,...) do { if (false) \
      RPS_INFORM(Fmt,##__VA_ARGS__); }while(0)

#define RPS_NOPRINTOUT(...) do { if (false) \
      RPS_INFORMOUT(__VA_ARGS__); }while(0)


//////////////// assert
#ifndef NDEBUG
///
#define RPS_ASSERT_AT_BIS(Fil,Lin,Func,Cond) do {               \
  if (RPS_UNLIKELY(!(Cond))) {                                  \
    if (rps_syslog_enabled)                                     \
      syslog(LOG_CRIT,                                          \
       "*** RefPerSys ASSERT failed: %s *** [%s:%d:%s]",        \
       #Cond, Fil, Lin, Func);                                  \
    else                                                        \
      fprintf(stderr, "\n\n"                                    \
        "%s*** RefPerSys ASSERT failed: %s%s\n"                 \
        "%s:%d: {%s}\n\n",                                      \
        (rps_stderr_istty?RPS_TERMINAL_BOLD_ESCAPE:""),         \
        #Cond,                                                  \
        (rps_stderr_istty?RPS_TERMINAL_NORMAL_ESCAPE:""),       \
        Fil,Lin,Func);                                          \
  rps_fatal_stop_at(Fil,Lin); }} while(0)

#define RPS_ASSERT_AT(Fil,Lin,Func,Cond) RPS_ASSERT_AT_BIS(Fil,Lin,Func,Cond)
#define RPS_ASSERT(Cond) RPS_ASSERT_AT(__FILE__,__LINE__,__PRETTY_FUNCTION__,(Cond))

#define RPS_ASSERTPRINTF_AT_BIS(Fil,Lin,Func,Cond,Fmt,...) do { \
  if (RPS_UNLIKELY(!(Cond))) {                                  \
    if (rps_syslog_enabled)                                     \
      syslog(LOG_CRIT,                                          \
       "*** RefPerSys ASSERTPRINTF failed:"                     \
       " %s *** [%s:%d:%s]" Fmt,                                \
       #Cond, Fil, Lin, Func, ##__VA_ARGS__);                   \
    else {                                                      \
      fprintf(stderr, "\n\n"                                    \
              "%s*** RefPerSys ASSERTPRINTF failed:%s %s\n"     \
              "%s:%d: {%s}\n",                                  \
          (rps_stderr_istty?RPS_TERMINAL_BOLD_ESCAPE:""),       \
                #Cond,                                          \
          (rps_stderr_istty?RPS_TERMINAL_NORMAL_ESCAPE:""),     \
              Fil, Lin, Func);                                  \
      fprintf(stderr, "!*!*! " Fmt "\n\n", ##__VA_ARGS__);      \
    };                                                          \
    rps_fatal_stop_at(Fil, Lin); }                              \
 } while(0)

#define RPS_ASSERTPRINTF_AT(Fil,Lin,Func,Cond,Fmt,...) RPS_ASSERTPRINTF_AT_BIS(Fil,Lin,Func,Cond,Fmt,##__VA_ARGS__)
#define RPS_ASSERTPRINTF(Cond,Fmt,...) RPS_ASSERTPRINTF_AT(__FILE__,__LINE__,__PRETTY_FUNCTION__,(Cond),Fmt,##__VA_ARGS__)
#define RPS_ASSERT_CALLFRAME(Callframe) \
    RPS_ASSERT((Callframe) != nullptr && (Callframe)->is_good_call_frame())
#else
#define RPS_ASSERT(Cond) do { if (false && (Cond)) rps_fatal_stop_at(__FILE_,__LINE__); } while(0)
#define RPS_ASSERTPRINTF(Cond,Fmt,...)  do { if (false && (Cond)) \
      fprintf(stderr, Fmt "\n", ##__VA_ARGS__); } while(0)
#define RPS_ASSERT(Callframe)

#endif /*NDEBUG*/


#define RPS_RUNTIME_ERROR_OUT_AT_BIS(Fil,Lin,...) ({    \
      std::ostringstream outs##Lin;                     \
      outs##Lin << Fil << ":"<< Lin << "::"             \
                << __VA_ARGS__;                         \
      auto res##Lin =                                   \
        std::runtime_error(outs##Lin.str());            \
      res##Lin; })

#define RPS_RUNTIME_ERROR_OUT_AT(Fil,Lin,...) RPS_RUNTIME_ERROR_OUT_AT_BIS(Fil,Lin,##__VA_ARGS__)

// typical usage would be throw RPS_RUNTIME_ERROR_OUT("annoying x=" << x)
#define RPS_RUNTIME_ERROR_OUT(...) RPS_RUNTIME_ERROR_OUT_AT(__FILE__,__LINE__,##__VA_ARGS__)


extern "C" void rps_show_version(void);

///////////////////////////////////////////////////////////////////////////////
// TIME ROUTINES
///////////////////////////////////////////////////////////////////////////////


static inline double rps_monotonic_real_time(void);
static inline double rps_wallclock_real_time(void);
double rps_elapsed_real_time(void);
double rps_get_start_wallclock_real_time(void);
static inline double rps_process_cpu_time(void);
static inline double rps_thread_cpu_time(void);

char *rps_strftime_centiseconds(char *bfr, size_t len, const char *fmt,
                                double m);

#define rps_now_strftime_centiseconds(Bfr, Len, Fmt) \
  rps_strftime_centiseconds((Bfr), (Len), (Fmt), rps_wallclock_real_time())

#define rps_now_strftime_centiseconds_nolen(Bfr, Fmt) do {      \
    static_assert(sizeof((Bfr)) > 16);                          \
    rps_now_strftime_centiseconds((Bfr), sizeof ((Bfr)),        \
                                  (Fmt));                       \
} while (0)


struct rps_timer
{
  double monotonic_start;
  double monotonic_stop;
  double wallclock_start;
  double wallclock_stop;
  double cpu_start;
  double cpu_stop;
};


inline void
rps_timer_start(struct rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  hnd->monotonic_start = rps_monotonic_real_time();
  hnd->wallclock_start = rps_wallclock_real_time();
  hnd->cpu_start = rps_thread_cpu_time();

  hnd->monotonic_stop = 0.0;
  hnd->wallclock_stop = 0.0;
  hnd->cpu_stop = 0.0;
}


inline void
rps_timer_stop(struct rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  hnd->monotonic_stop = rps_monotonic_real_time();
  hnd->wallclock_stop = rps_wallclock_real_time();
  hnd->cpu_stop = rps_thread_cpu_time();
}


inline double
rps_timer_monotonic_start(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->monotonic_start;
}


inline double
rps_timer_monotonic_stop(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->monotonic_stop;
}


inline double
rps_timer_monotonic_elapsed(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->monotonic_stop - hnd->monotonic_start;
}


inline double
rps_timer_wallclock_start(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->wallclock_start;
}


inline double
rps_timer_wallclock_stop(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->wallclock_stop;
}


inline double
rps_timer_wallclock_elapsed(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->wallclock_stop - hnd->wallclock_start;
}


inline double
rps_timer_cpu_start(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->cpu_start;
}


inline double
rps_timer_cpu_stop(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->cpu_stop;
}


inline double
rps_timer_cpu_elapsed(const rps_timer *hnd)
{
  RPS_ASSERT (hnd);

  return hnd->cpu_stop - hnd->cpu_start;
}


#define RPS_TIMER_START()       \
    struct rps_timer __timer;   \
    rps_timer_start(&__timer)


#define RPS_TIMER_STOP(context)                                     \
    rps_timer_stop(&__timer);                                       \
    RPS_DEBUG_LOG(REPL, "real time = "                              \
        << rps_timer_wallclock_elapsed(&__timer) << "; cpu time = " \
        << rps_timer_cpu_elapsed(&__timer));


/* the debugger could put a breakpoint, or we could overwrite the
   binary executable code with a call; don't forget to flush the
   caches! */
#define RPS_POSSIBLE_BREAKPOINT() do { \
  asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n" \
                "nop; nop; nop; nop; nop; nop; nop; nop;\n" \
                "nop; nop; nop; nop; nop; nop; nop; nop;\n" \
); } while(0)

///////////////////////////////////////////////////////////////////////////////

extern "C" const char* rps_hostname(void);
extern "C" void*rps_proghdl; // dlopen handle of whole program

extern "C" Json::Value rps_string_to_json(const std::string&str);
extern "C" std::string rps_json_to_string(const Json::Value&jv);
extern "C" void rps_output_program_arguments(std::ostream& out,
    int argc, const char*const*argv);

#define RPS_OUT_PROGARGS(Argc,Argv)                     \
  Rps_Do_Output([&](std::ostream&out)                   \
{                                                       \
  rps_output_program_arguments(out, (Argc), (Argv));    \
})

#define RPS_FLEXIBLE_DIM 0      /* for flexible array members */

/// In rare occasions (some kind of array hash table, perhaps) we may
/// need a pointer value which is non null but still morally "empty":
/// this is rarely useful, and any code using that should be carefully
/// written.
#define RPS_EMPTYSLOT   ((const void*)(((intptr_t*)nullptr)+1))

// size of blocks, in bytes
#define RPS_SMALL_BLOCK_SIZE (8<<20)
#define RPS_LARGE_BLOCK_SIZE (8*RPS_SMALL_BLOCK_SIZE)

static_assert(RPS_SMALL_BLOCK_SIZE & (~ (RPS_SMALL_BLOCK_SIZE-1)),
              "RPS_SMALL_BLOCK_SIZE should be some power of two");


//////// Prime numbers; useful e.g. for hashing, etc....
// Give, using some a table of primes, some prime number above or below a
// given integer, and reasonably close to it (e.g. less than 20% from
// it).
extern "C" int64_t rps_prime_above (int64_t n);
extern "C" int64_t rps_prime_below (int64_t n);
// safely give a prime of given rank from the table
extern "C" int64_t rps_prime_ranked (int rk);
// give some prime greater or equal to a given integer, and set the
// rank if non-null pointer
extern "C" int64_t rps_prime_greaterequal_ranked (int64_t n, int*prank);
// give some prime less or equal to a given integer, and set the
// rank if non-null pointer
extern "C" int64_t rps_prime_lessequal_ranked (int64_t n, int*prank);
/////////////////


// give the name of the current pthread
static inline std::string rps_current_pthread_name(void);

// give its index
static inline int rps_current_pthread_index(void);

extern "C" thread_local int rps_curthread_ix;
extern "C" thread_local Rps_CallFrame* rps_curthread_callframe;

static constexpr unsigned rps_allocation_unit = 2*sizeof(void*);
static_assert ((rps_allocation_unit & (rps_allocation_unit-1)) == 0,
               "rps_allocation_unit is not a power of two");

#define RPS_NULL_CALL_FRAME ((Rps_CallFrame*)nullptr)
#define RPS_CALL_FRAME_UNDESCRIBED ((Rps_ObjectRef)nullptr)

extern "C" unsigned rps_call_frame_depth(const Rps_CallFrame*);
////////////////////////////////////////////////////////////////
class Rps_ObjectRef // reference to objects, per C++ rule of five.
{
  Rps_ObjectZone*_optr;
protected:
public:
  struct Rps_ObjIdStrTag {};
  const std::string as_string(void) const;
  operator const std::string  () const
  {
    return as_string();
  };
  Rps_ObjectZone* optr() const
  {
    return _optr;
  };
  inline std::recursive_mutex* objmtx(void) const;
  const Rps_ObjectZone* const_optr() const
  {
    return const_cast<const Rps_ObjectZone*>(_optr);
  };
  /// Compare two object references for display to humans
  /// so if both have names, use them....
  static int compare_for_display(const Rps_ObjectRef leftob,
                                 const Rps_ObjectRef rightob);
  bool is_empty() const
  {
    return _optr == nullptr || _optr == (Rps_ObjectZone*)RPS_EMPTYSLOT;
  }
  Rps_ObjectZone* to_object() const
  {
    if (is_empty()) return nullptr;
    return _optr;
  }
  const Rps_ObjectZone* to_constant_object() const
  {
    if (is_empty()) return nullptr;
    return _optr;
  }
  // get object from Json at load time
  Rps_ObjectRef(const Json::Value &, Rps_Loader*); //in store_rps.cc

  // build an object from its existing string oid, or else fail with C++ exception
  Rps_ObjectRef(Rps_CallFrame*callerframe, const char*oidstr, Rps_ObjIdStrTag);

  // rule of five
  Rps_ObjectRef(const Rps_ObjectZone*oz = nullptr)
    : _optr(const_cast<Rps_ObjectZone*>(oz))
  {
    if (RPS_UNLIKELY((oz == (const Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
  };
  ~Rps_ObjectRef()
  {
    _optr = nullptr;
  };
  Rps_ObjectRef(const Rps_ObjectRef&oth)
  {
    if (RPS_UNLIKELY((oth._optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
    else
      _optr = oth._optr;
  };
  Rps_ObjectRef(Rps_ObjectRef&&oth) : _optr(std::move(oth._optr))
  {
    if (RPS_UNLIKELY((_optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
  };
  Rps_ObjectRef& operator = (const Rps_ObjectRef& oth)
  {
    _optr = oth._optr;
    if (RPS_UNLIKELY((_optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
    return *this;
  }
  Rps_ObjectRef& operator = (Rps_ObjectRef&& oth)
  {
    std::swap(_optr, oth._optr);
    if (RPS_UNLIKELY((_optr == (Rps_ObjectZone*)RPS_EMPTYSLOT)))
      _optr = nullptr;
    return *this;
  }
  const Rps_ObjectZone& operator * (void) const
  {
    RPS_ASSERT(_optr != nullptr);
    return *_optr;
  };
  bool operator ! () const
  {
    return _optr == nullptr;
  };
  operator const Rps_ObjectZone* () const
  {
    return _optr;
  };
  operator Rps_ObjectZone* ()
  {
    return _optr;
  };
  operator bool () const
  {
    return _optr != nullptr;
  };
  const Rps_ObjectZone* operator -> (void) const
  {
    RPS_ASSERT(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone* operator * (void)
  {
    RPS_ASSERT(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone* operator -> (void)
  {
    RPS_ASSERT(_optr != nullptr);
    return _optr;
  };
  Rps_ObjectZone*obptr() const
  {
    return _optr;
  };
  void set_obptr(Rps_ObjectZone*zob)
  {
    if (RPS_UNLIKELY(zob == (Rps_ObjectZone*)RPS_EMPTYSLOT))
      zob = nullptr;
    _optr = zob;
  };
  //////// in Rps_ObjectRef....
  inline bool operator == (const Rps_ObjectRef& oth) const;
  inline bool operator != (const Rps_ObjectRef& oth) const;
  inline bool operator <= (const Rps_ObjectRef& oth) const;
  inline bool operator < (const Rps_ObjectRef& oth) const;
  inline bool operator > (const Rps_ObjectRef& oth) const;
  inline bool operator >= (const Rps_ObjectRef& oth) const;
  inline Rps_HashInt obhash (void) const;
  inline void gc_mark(Rps_GarbageCollector&) const;
  inline void dump_scan(Rps_Dumper* du, unsigned depth) const;
  inline Json::Value dump_json(Rps_Dumper* du) const;
  void output(std::ostream&out, unsigned depth=0,unsigned maxdepth=0) const;
  /////////// the root space
  static inline Rps_ObjectRef root_space(void);
  ///////////
  // these functions throw an exception on failure (unless dontfail is true, then gives nil)
  // find an object with a given oid or name string
  enum Find_Behavior_en
  {
    Rps_Null_When_Missing,
    Rps_Fail_If_Not_Found
  };
  static Rps_ObjectRef find_object_by_string(Rps_CallFrame*callerframe,  const std::string& str, Find_Behavior_en fb);
  static inline Rps_ObjectRef find_object_or_null_by_string(Rps_CallFrame*callerframe,  const std::string& str)
  {
    return find_object_by_string(callerframe, str, Rps_Null_When_Missing);
  };
  static inline Rps_ObjectRef find_object_or_fail_by_string(Rps_CallFrame*callerframe,  const std::string& str)
  {
    return find_object_by_string(callerframe, str, Rps_Fail_If_Not_Found);
  };
  static Rps_ObjectRef find_object_by_oid(Rps_CallFrame*callerframe, Rps_Id oid, Find_Behavior_en fb);
  static inline Rps_ObjectRef find_object_or_null_by_oid(Rps_CallFrame*callerframe, Rps_Id oid);
  static inline Rps_ObjectRef find_object_or_fail_by_oid(Rps_CallFrame*callerframe, Rps_Id oid);
  static Rps_ObjectRef really_find_object_by_oid(const Rps_Id& oid);
  // create a class of given super class and name
  static Rps_ObjectRef make_named_class(Rps_CallFrame*callerframe, Rps_ObjectRef superclassob, std::string name);
  // create a symbol of given name (static methods of Rps_ObjectRef)
  static Rps_ObjectRef make_new_symbol(Rps_CallFrame*callerframe, std::string name, bool isweak);
  static Rps_ObjectRef make_new_strong_symbol(Rps_CallFrame*callerframe, std::string name)
  {
    return make_new_symbol(callerframe, name, false);
  };
  static Rps_ObjectRef make_new_weak_symbol(Rps_CallFrame*callerframe, std::string name)
  {
    return make_new_symbol(callerframe, name, true);
  };
  // create an object of given class
  static Rps_ObjectRef make_object(Rps_CallFrame*callerframe, Rps_ObjectRef classob, Rps_ObjectRef spaceob=nullptr);
  // the superclass of all objects, that is the `object` object
  static inline Rps_ObjectRef the_object_class(void);
  // the class of all classes, that is the `class` object
  static inline Rps_ObjectRef the_class_class(void);
  // the class of all symbols, that is the `symbol` object
  static inline Rps_ObjectRef the_symbol_class(void);
  // the class of mutable sets, that is the `mutable_set` object
  static inline Rps_ObjectRef the_mutable_set_class(void);
  // the class of named selectors, that is the `named_selector` object
  static inline Rps_ObjectRef the_named_selector_class(void);
  // the `name` object
  static inline Rps_ObjectRef the_name_object(void);
  // if this is a class-object, install a method of selector obsel and
  // closure closv; otherwise raise an exception; and likewise for two
  // or three selectors. The callerframe is not really useful today,
  // but might be useful later...<
  // NB: we might call these from the temporary plugin window.
  void install_own_method(Rps_CallFrame*callerframe, Rps_ObjectRef obsel, Rps_Value closv);
  // likewise, but lock this class only once!
  void install_own_2_methods(Rps_CallFrame*callerframe, Rps_ObjectRef obsel0, Rps_Value closv0, Rps_ObjectRef obsel1, Rps_Value closv1);
  void install_own_3_methods(Rps_CallFrame*callerframe, Rps_ObjectRef obsel0, Rps_Value closv0, Rps_ObjectRef obsel1, Rps_Value closv1, Rps_ObjectRef obsel2, Rps_Value closv2);
};                              // end class Rps_ObjectRef



static_assert(sizeof(Rps_ObjectRef) == sizeof(void*),
              "Rps_ObjectRef should have the size of a word");
static_assert(alignof(Rps_ObjectRef) == alignof(void*),
              "Rps_ObjectRef should have the alignment of a word");


class Rps_Object_Display /// use it only with RPS_OBJECT_DISPLAY.... macros
{
  Rps_ObjectRef _dispobref;
  const char* _dispfile;
  int _displine;
  int _dispdepth;
public:
  static constexpr int disp_default_depth=1;
  static constexpr int disp_max_depth=8;
  Rps_Object_Display() : _dispobref(nullptr), _dispfile(nullptr), _displine(0), _dispdepth(0) {};
  Rps_Object_Display(const Rps_ObjectRef obr, int depth, const char*file, int line)
    : _dispobref(obr), _dispfile(file), _displine(line),
      _dispdepth(disp_default_depth) {};
  Rps_Object_Display(const Rps_ObjectRef obr, const char*file, int line)
    : _dispobref(obr), _dispfile(file), _displine(line),
      _dispdepth(disp_default_depth) {};
  ~Rps_Object_Display()
  {
    _dispobref=nullptr;
    _dispfile=nullptr;
    _dispdepth=0;
    _displine=0;
  };
  Rps_Object_Display(const Rps_Object_Display&) = default;
  Rps_Object_Display(Rps_Object_Display&&) = default;
  void output_display(std::ostream&out) const;
  void output_routine_addr(std::ostream&out, void*funaddr) const;
};        // end class Rps_Object_Display

#define RPS_OBJECT_DISPLAY(Ob) Rps_Object_Display((Ob),__FILE__,__LINE__)
#define RPS_OBJECT_DISPLAY_DEPTH(Ob,Depth) Rps_Object_Display((Ob),(Depth),__FILE__,__LINE__)

extern "C" void rps_sort_object_vector_for_display(std::vector<Rps_ObjectRef>&);

inline std::ostream&operator <<(std::ostream&out, const Rps_Object_Display& obdisp)
{
  obdisp.output_display(out);
  return out;
};

// we could code Rps_ObjectFromOidRef(&_,"_41OFI3r0S1t03qdB2E") instead of rpskob_41OFI3r0S1t03qdB2E
class Rps_ObjectFromOidRef : public Rps_ObjectRef
{
public:
  Rps_ObjectFromOidRef(Rps_CallFrame*callerframe, const char*oidstr) :
    Rps_ObjectRef(callerframe, oidstr, Rps_ObjectRef::Rps_ObjIdStrTag{}) {};
};                              // end Rps_ObjectFromOidRef

/// mostly for debugging
inline std::ostream&
operator << (std::ostream&out, Rps_ObjectRef obr)
{
  obr.output(out);
  return out;
}

//////////////////////////////////////////////////////////////// constant objects

// constant objects starts with
#define RPS_CONSTANTOBJ_PREFIX "rpskob"

#define RPS_INSTALL_CONSTANT_OB(Oid) extern "C" Rps_ObjectRef rpskob##Oid;
#include "generated/rps-constants.hh"
unsigned constexpr rps_nb_constants = RPS_NB_CONSTANT_OB;

////////////////////////////////////////////////////////////////
enum class Rps_Type : std::int16_t
{
  CallFrame = std::numeric_limits<std::int16_t>::min(),
  ////////////////
  /// payloads are negative, below -1
  PaylLightCodeGen = -28,
  PaylMachlearn = -27,
  PaylFltkRefWidget = -26,
  PaylFltkWidget = -25,
  PaylFltkWindow = -24,
  PaylFltkThing = -23,
  PaylCplusplusGen = -22,    // for C++ code generation
  PaylGccjit = -21,    // for GNU libgccjit code generation
  PaylEnviron = -20,         // for environments
  PaylObjMap = -19,          // for object maps
  PaylCppStream = -18,     // for transient C++ streams
  PaylPopenedFile = -17,   // for transient popened commands
  PaylUnixProcess = -16,        // for transient of forked unix processes
  PaylWebHandler = -15, // for reification of Web handlers,
  // i.e. Rps_PayloadWebHandler-s
  PaylWebex = -14, // for reification as temporary objects of HTTP
  // requests+replies, i.e. Web exchanges.
  PaylTasklet = -13, // for small tasklets inside agenda
  PaylStringDict = -12, // the dictionnaries associating strings to values
  PaylAgenda = -11, // *the* unique agenda
  PaylSymbol = -10, // symbol payload
  PaylSpace = -9, // space payload
  PaylStrBuf = -8, // mutable string buffer
  PaylRelation = -7, // mutable binary relation between objects
  PaylAssoc = -6, // mutable association from object to values
  PaylVectVal = -5, // mutable vector of values payload
  PaylVectOb = -4, // mutable vector of objects payload
  PaylSetOb = -3, // mutable set of objects payload
  PaylClassInfo = -2, // class information payload
  Payl__LeastRank = -2,
  ////////////////
  /// non-value types (or quasi-values)
  ///
  ///
  Int = -1, // for tagged integers
  None = 0, // for nil
  ////////////////
  ///
  /// Values that could go into Rps_Value:
  /// Boxed genuine values, are "first class citizens" that could be
  /// in Rps_Value's data. Of course they are both GC-allocated and
  /// GC-scanned.
  String,
  Double,
  Set,
  Tuple,
  Object,
  Closure,
  Instance,
  Json,
  LexToken,
};

//////////////////////////////////////////////////////////////// values

//// forward declarations
class Rps_ObjectRef;
class Rps_ObjectZone;
class Rps_String;
class Rps_Double;
class Rps_SetOb;
class Rps_TupleOb;
class Rps_LexTokenZone;
class Rps_ClosureZone;
class Rps_InstanceZone;
class Rps_GarbageCollector;
class Rps_Loader; // in load_rps.cc
class Rps_Dumper; // in dump_rps.cc
class Rps_ClosureValue;
class Rps_SetValue;
class Rps_InstanceValue;
class Rps_TupleValue;
class Rps_LexTokenValue; // mostly in repl_rps.cc
class Rps_OutputValue;
struct Rps_TwoValues;

//////////////// our value, a single word
class Rps_Value
{
  friend class Rps_PayloadSymbol;
  friend class Rps_OutputValue;
public:
  // the maximal depth of the inheritance graph. An arbitrary, but small limit.
  static constexpr unsigned maximal_inheritance_depth = 32;
  static constexpr unsigned debug_maxdepth= 3;
  /// various C++ tags
  struct Rps_IntTag {};
  struct Rps_DoubleTag {};
  struct Rps_ValPtrTag {};
  struct Rps_EmptyTag {};
  /// various constructors
  inline Rps_Value ();
  inline Rps_Value (std::nullptr_t);
  inline Rps_Value (Rps_EmptyTag);
  inline Rps_Value (intptr_t i, Rps_IntTag);
  inline Rps_Value (double d, Rps_DoubleTag);
  inline Rps_Value (const Rps_ZoneValue*ptr, Rps_ValPtrTag);
  Rps_Value(Rps_ObjectRef obr)
    : Rps_Value((const Rps_ZoneValue*)(obr.optr()), Rps_ValPtrTag{})
  {
  };
  inline Rps_Value (const void*ptr, const Rps_PayloadSymbol*symb);
  inline Rps_Value (const void*ptr, Rps_CallFrame*cframe);
  Rps_Value(const Json::Value &hjv, Rps_Loader*ld); // in store_rps.cc
  /// C++ rule of five
  inline Rps_Value(const Rps_Value& other);
  inline Rps_Value(Rps_Value&& other);
  inline Rps_Value& operator = (const Rps_Value& other);
  inline Rps_Value& operator = (Rps_Value&& other);
  inline ~Rps_Value();
  Rps_Value(intptr_t i) : Rps_Value(i, Rps_IntTag{}) {};
  inline Rps_Value(const std::string&str);
  inline Rps_Value(double d);
  inline Rps_Value(const char*str, int len= -1);
  Rps_Value(const Rps_ZoneValue*ptr) : Rps_Value(ptr, Rps_ValPtrTag{}) {};
  Rps_Value(const Rps_ZoneValue& zv) : Rps_Value(&zv, Rps_ValPtrTag{}) {};
  ///
  Rps_ClosureValue closure_for_method_selector(Rps_CallFrame*cframe, Rps_ObjectRef obselector) const;
  inline const void* data_for_symbol(Rps_PayloadSymbol*) const;
  static constexpr unsigned max_gc_mark_depth = 100;
  inline void gc_mark(Rps_GarbageCollector&gc, unsigned depth= 0) const;
  void dump_scan(Rps_Dumper* du, unsigned depth) const;
  Json::Value dump_json(Rps_Dumper* du) const;
  inline bool operator == (const Rps_Value v) const;
  inline bool operator <= (const Rps_Value v) const;
  inline bool operator < (const Rps_Value v) const;
  inline bool operator != (const Rps_Value v) const;
  inline bool operator >= (const Rps_Value v) const;
  inline bool operator > (const Rps_Value v) const;
  inline Rps_Type type() const;
  inline bool is_int() const;
  inline bool is_ptr() const;
  inline bool is_object() const;
  inline bool is_instance() const;
  inline bool is_set() const;
  inline bool is_closure() const;
  inline bool is_string() const;
  inline bool is_double() const;
  inline bool is_tuple() const;
  inline bool is_null() const;
  inline bool is_empty() const;
  inline bool is_json() const;
  inline bool is_lextoken() const;
  operator bool () const
  {
    return !is_empty();
  };
  bool operator ! () const
  {
    return is_empty();
  };
  // convert, or else throw exception on failure
  inline intptr_t as_int() const;
  inline const Rps_ZoneValue* as_ptr() const;
  inline const Rps_SetOb* as_set() const;
  inline const Rps_TupleOb* as_tuple() const;
  inline  Rps_ObjectZone* as_object() const;
  inline const Rps_InstanceZone* as_instance() const;
  inline const Rps_String* as_string() const;
  inline const Rps_ClosureZone* as_closure() const;
  inline const Rps_Double* as_boxed_double() const;
  inline const Rps_JsonZone* as_json() const;
  inline const Rps_LexTokenZone* as_lextoken() const;
  inline double as_double() const;
  inline const std::string as_cppstring() const;
  inline const char* as_cstring() const;
  Rps_ObjectRef compute_class(Rps_CallFrame*) const;
  // convert or give default
  static inline Rps_Value make_tagged_int(intptr_t);
  inline intptr_t to_int(intptr_t def=0) const;
  inline const Rps_ZoneValue* to_ptr(const Rps_ZoneValue*zp = nullptr) const;
  inline const Rps_SetOb* to_set(const Rps_SetOb*defset= nullptr) const;
  inline const Rps_Double* to_boxed_double(const Rps_Double*defdbl= nullptr) const;
  inline double to_double(double def=std::nan("")) const;
  inline const Rps_JsonZone* to_boxed_json(const Rps_JsonZone*defjson= nullptr) const;
  inline const Json::Value to_json(const Json::Value defjv=Json::Value::nullSingleton()) const;
  inline const Rps_TupleOb* to_tuple(const Rps_TupleOb* deftup= nullptr) const;
  inline const Rps_ClosureZone* to_closure(const Rps_ClosureZone* defclos= nullptr) const;
  inline const Rps_ObjectZone* to_object(const Rps_ObjectZone*defob
                                         =nullptr) const;
  inline const Rps_InstanceZone* to_instance(const Rps_InstanceZone*definst =nullptr) const;
  inline const Rps_String* to_string( const Rps_String*defstr
                                      = nullptr) const;
  inline const Rps_LexTokenZone* to_lextoken(void) const;
  inline const std::string to_cppstring(std::string defstr= "") const;
  inline Rps_HashInt valhash() const noexcept;
  static constexpr unsigned max_output_depth = 5;
  inline void output(std::ostream&out, unsigned depth=0, unsigned maxdepth= max_output_depth) const;
  Rps_Value get_attr(Rps_CallFrame*stkf, const Rps_ObjectRef obattr) const;
  Rps_Value get_physical_attr(const Rps_ObjectRef obattr) const;
  inline void clear(void);
  inline Rps_Value& operator = (std::nullptr_t)
  {
    clear();
    return *this;
  };
  // test if this value is instance of obclass:
  inline bool is_instance_of(Rps_CallFrame*callerframe, Rps_ObjectRef obclass) const;
  // test if this value is a subclass of given obsuperclass:
  inline bool is_subclass_of(Rps_CallFrame*callerframe, Rps_ObjectRef obsuperclass) const;
  Rps_TwoValues send0(Rps_CallFrame*cframe, const Rps_ObjectRef obsel) const;
  Rps_TwoValues send1(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      Rps_Value arg0) const;
  Rps_TwoValues send2(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      Rps_Value arg0, const Rps_Value arg1) const;
  Rps_TwoValues send3(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1, const Rps_Value arg2) const;
  Rps_TwoValues send4(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3) const;
  Rps_TwoValues send5(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4) const;
  Rps_TwoValues send6(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5) const;
  Rps_TwoValues send7(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5,
                      const Rps_Value arg6) const;
  Rps_TwoValues send8(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5,
                      const Rps_Value arg6, const Rps_Value arg7) const;
  Rps_TwoValues send9(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                      const Rps_Value arg0, const Rps_Value arg1,
                      const Rps_Value arg2, const Rps_Value arg3,
                      const Rps_Value arg4, const Rps_Value arg5,
                      const Rps_Value arg6, const Rps_Value arg7,
                      const Rps_Value arg8) const;
  Rps_TwoValues send_vect(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                          const std::vector<Rps_Value>& argvec) const;
  Rps_TwoValues send_ilist(Rps_CallFrame*cframe, const Rps_ObjectRef obsel,
                           const std::initializer_list<Rps_Value>& argil) const;
  const void* unsafe_wptr() const
  {
    return _wptr;
  };
private:
  union
  {
    intptr_t _ival;
    const Rps_ZoneValue* _pval;
    const void* _wptr;
  };
  inline bool is_subclass_with_count_and_depth(Rps_CallFrame*callerframe,
      uint64_t count, Rps_ObjectRef obsuperclass,
      Rps_ObjectRef obthisclass, int depth) const;
};    // end of Rps_Value
static_assert(sizeof(Rps_Value) == sizeof(void*),
              "Rps_Value should have the size of a word or pointer");
static_assert(alignof(Rps_Value) == alignof(void*),
              "Rps_Value should have the alignment of a word or pointer");


////////////////////////////////////////////////////////////////
//// utility class to output a value with a given depth and maximal depth to some std::ostream
//// typical usage: RPS_DEBUG_LOG("v1=" << Rps_OutputValue(v1, /*depth:*/3, /*maxdepth:*/5));
class Rps_OutputValue
{
  friend class Rps_Value;
  const Rps_Value _out_val;
  const unsigned _out_depth;
  const unsigned _out_maxdepth;
  static constexpr unsigned out_default_depth=3;
public:
  Rps_OutputValue(const Rps_Value val, unsigned depth, unsigned maxdepth= out_default_depth)
    : _out_val(val), _out_depth(depth), _out_maxdepth(maxdepth) {};
  Rps_OutputValue(const Rps_ObjectRef ob, unsigned depth, unsigned maxdepth= out_default_depth)
    : _out_val(ob), _out_depth(depth), _out_maxdepth(maxdepth) {};
  ~Rps_OutputValue() {};
  unsigned depth_out() const
  {
    return _out_depth;
  };
  unsigned maxdepth_out() const
  {
    return _out_maxdepth;
  };
  const Rps_Value value_out() const
  {
    return _out_val;
  };
  void do_output(std::ostream& out) const; /// in morevalues_rps.cc
};                              // end class Rps_OutputValue


/// printing routines likely to be called from GDB debugger
extern "C" void rps_print_value(const Rps_Value val); // in values_rps.cc
extern "C" void rps_print_objectref(Rps_ObjectRef ob); // in utilities_rps.cc
extern "C" void rps_print_ptr_value(const void*v); // in values_rps.cc
extern "C" void rps_limited_print_value(const Rps_Value val, unsigned depth, unsigned maxdepth);
extern "C" void rps_limited_print_ptr_value(const void*v, unsigned depth, unsigned maxdepth);

inline std::ostream& operator<< (std::ostream&out, const Rps_OutputValue oval)
{
  oval.do_output(out);
  return out;
} // end of operator<< (std::ostream&out, const Rps_OutputValue oval)

////////////////////////////////////////////////////////////////

struct Rps_TwoValues
{
  Rps_Value main_val;
  Rps_Value xtra_val;
  Rps_TwoValues(std::nullptr_t) :
    main_val(nullptr), xtra_val(nullptr) {};
  Rps_TwoValues(Rps_Value m=nullptr, Rps_Value x=nullptr)
    : main_val(m), xtra_val(x) {};
  Rps_Value main() const
  {
    return main_val;
  };
  Rps_Value xtra() const
  {
    return xtra_val;
  };
  operator Rps_Value (void) const
  {
    return main();
  };
  void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0) const
  {
    if (main_val) main_val.gc_mark(gc,depth);
    if (xtra_val) xtra_val.gc_mark(gc,depth);
  };
  bool operator ! (void) const
  {
    return !main_val && !xtra_val;
  };
  operator bool (void) const
  {
    return main_val || xtra_val;
  };
};                              // end Rps_TwoValues

/// mostly for debugging
inline std::ostream&
operator << (std::ostream&out, Rps_Value val)
{
  val.output(out);
  return out;
}

std::ostream&operator << (std::ostream&out, const std::vector<Rps_Value>& vect);

namespace std
{
template <> struct hash<Rps_ObjectRef>
{
  std::size_t operator()(Rps_ObjectRef const& obr) const noexcept
  {
    return obr.obhash();
  };
};
template <> struct hash<Rps_Value>
{
  std::size_t operator()(Rps_Value const& val) const noexcept
  {
    return val.valhash();
  };
};
};

//////////////// specialized subclasses of Rps_Value

class Rps_ObjectValue : public Rps_Value
{
public:
  inline Rps_ObjectValue(const Rps_ObjectRef obr);
  inline Rps_ObjectValue(const Rps_Value val, const Rps_ObjectZone*defob=nullptr);
  inline Rps_ObjectValue(const Rps_ObjectZone* obz=nullptr);
  inline Rps_ObjectValue(std::nullptr_t);
}; // end class Rps_ObjectValue

class Rps_StringValue : public Rps_Value
{
public:
  inline Rps_StringValue(const char*cstr, int len= -1);
  inline Rps_StringValue(const std::string str);
  inline Rps_StringValue(const Rps_Value val);
  inline Rps_StringValue(const Rps_String* strv);
  inline Rps_StringValue(std::nullptr_t);
  inline Rps_StringValue() : Rps_StringValue(nullptr) {};
}; // end class Rps_StringValue

class Rps_DoubleValue : public Rps_Value
{
public:
  inline Rps_DoubleValue (double d=0.0);
  inline Rps_DoubleValue(const Rps_Value val);
}; // end class Rps_DoubleValue

class Rps_JsonValue : public Rps_Value
{
public:
  inline Rps_JsonValue(const Json::Value&jv);
  inline Rps_JsonValue(const Rps_Value val);
  Rps_JsonValue(double d)
    : Rps_JsonValue(Json::Value(d)) {};
  Rps_JsonValue(std::intptr_t i)
    : Rps_JsonValue(Json::Value((Json::Int64)i)) {};
  Rps_JsonValue(bool b)
    : Rps_JsonValue(Json::Value(b)) {};
  Rps_JsonValue(const std::string& s)
    : Rps_JsonValue(Json::Value(s)) {};
  Rps_JsonValue(std::nullptr_t)
    : Rps_JsonValue(Json::Value(Json::nullValue)) {};
}; // end class Rps_JsonValue

struct Rps_SetTag
{
};                              // end empty struct Rps_SetTag

struct Rps_TupleTag
{
}; // end empty struct Rps_TupleTag



////////////////
class Rps_SetValue : public Rps_Value
{
public:
  /// related to Rps_SetOb::make :
  inline Rps_SetValue (const std::set<Rps_ObjectRef>& obset);
  inline Rps_SetValue (const std::vector<Rps_ObjectRef>& obvec);
  inline Rps_SetValue (const std::initializer_list<Rps_ObjectRef>& obil, Rps_SetTag);
  Rps_SetValue (const std::initializer_list<Rps_ObjectRef>& obil) :
    Rps_SetValue(obil, Rps_SetTag{}) {};
  /// related to Rps_SetOb::collect :
  inline Rps_SetValue(const std::vector<Rps_Value>& vecval);
  inline Rps_SetValue(const std::initializer_list<Rps_Value>&valil);
  // "dynamic" casting :
  inline Rps_SetValue(const Rps_Value);
  inline Rps_SetValue();
};    // end class Rps_SetValue




////////////////
class Rps_TupleValue : public Rps_Value
{
public:
  /// related to Rps_TupleOb::make :
  inline Rps_TupleValue (const std::vector<Rps_ObjectRef>& obvec);
  inline Rps_TupleValue (const std::initializer_list<Rps_ObjectRef>&compil, Rps_TupleTag);
  Rps_TupleValue(const std::initializer_list<Rps_ObjectRef>&compil)
    : Rps_TupleValue(compil, Rps_TupleTag{}) {};
  /// related to Rps_TupleOb::collect :
  inline Rps_TupleValue (const std::vector<Rps_Value>& vecval);
  inline Rps_TupleValue (const std::initializer_list<Rps_Value>&valil);
  // "dynamic" casting :
  inline Rps_TupleValue(Rps_Value val);
};    // end class Rps_TupleValue

////////////////
class Rps_LexTokenValue : public Rps_Value
{
  friend class Rps_LexTokenZone;
  friend class Rps_TokenSource;
public:
  inline Rps_LexTokenValue (const Rps_LexTokenZone& lxtok);
  inline Rps_LexTokenValue (const Rps_LexTokenZone* plxtok);
  // "dynamic" casting :
  inline Rps_LexTokenValue(Rps_Value val);
  inline Rps_LexTokenValue(); // needed to create empty null token
  Rps_LexTokenValue(std::nullptr_t):Rps_LexTokenValue() {};
};    // end class Rps_LexTokenValue
////////////////////////////////////////////////////////////////



class Rps_Random
{
  static thread_local Rps_Random _rand_thr_;
  static bool _rand_is_deterministic_;
  static std::ranlux48 _rand_gen_deterministic_;
  static std::mutex _rand_mtx_deterministic_;
  /// the thread local random state
  unsigned long _rand_count;
  std::mt19937 _rand_generator;
  /// we could need very quick and poor small random numbers on just 4
  /// bits. For these, we care less about the random quality, but even
  /// more about speed. So we keep one 32 bits of random number in
  /// advance, and a count of the remaining random bits in it.
  uint32_t _rand_advance;
  uint8_t _rand_remainbits;
  unsigned _rand_threadrank;
  static std::atomic<unsigned> _rand_threadcount;
  static constexpr const unsigned _rand_reseed_period_ = 65536;
  /// private initializer
  void init_deterministic (void);
  /// private deterministic reseeder
  void deterministic_reseed (void);
  /// private constructor
  Rps_Random () :
    _rand_count(0), _rand_generator(), _rand_advance(0), _rand_remainbits(0),
    _rand_threadrank(std::atomic_fetch_add(&_rand_threadcount,1U))
  {
    if (_rand_is_deterministic_)
      init_deterministic();
  };
  ///
  uint32_t generate_32u(void)
  {
    if (RPS_UNLIKELY(_rand_count++ % _rand_reseed_period_ == 0))
      {
        if (RPS_UNLIKELY(_rand_is_deterministic_))
          deterministic_reseed();
        else
          {
            std::random_device randev;
            auto s1=randev(), s2=randev(), s3=randev(), s4=randev(),
                 s5=randev(), s6=randev(), s7=randev();
            std::seed_seq seq {s1,s2,s3,s4,s5,s6,s7};
            _rand_generator.seed(seq);
          }
      }
    return _rand_generator();
  };
  uint32_t generate_nonzero_32u(void)
  {
    uint32_t r = 0;
    do
      {
        r = generate_32u();
      }
    while (RPS_UNLIKELY(r==0));
    return r;
  };
  uint64_t generate_64u(void)
  {
    return (static_cast<uint64_t>(generate_32u())<<32) | static_cast<uint64_t>(generate_32u());
  };
  uint8_t generate_quickly_4bits()
  {
    if (RPS_UNLIKELY(_rand_remainbits < 4))
      {
        _rand_advance = generate_32u();
        _rand_remainbits = 32;
      }
    uint8_t res = _rand_advance & 0xf;
    _rand_remainbits -= 4;
    _rand_advance = _rand_advance>>4;
    return res;
  };
  uint8_t generate_quickly_8bits()
  {
    if (RPS_UNLIKELY(_rand_remainbits < 8))
      {
        _rand_advance = generate_32u();
        _rand_remainbits = 32;
      }
    uint8_t res = _rand_advance & 0xff;
    _rand_advance = _rand_advance>>8;
    _rand_remainbits -= 8;
    return res;
  };
  uint16_t generate_quickly_16bits()
  {
    if (RPS_UNLIKELY(_rand_remainbits < 8))
      {
        _rand_advance = generate_32u();
        _rand_remainbits = 32;
      }
    uint8_t res = _rand_advance & 0xffff;
    _rand_advance = _rand_advance>>16;
    _rand_remainbits -= 16;
    return res;
  };
public:
  static void start_deterministic(long seed); // to be called from main
  static uint32_t random_32u(void)
  {
    return _rand_thr_.generate_32u();
  };
  static uint64_t random_64u(void)
  {
    return _rand_thr_.generate_64u();
  };
  static uint32_t random_nonzero_32u(void)
  {
    return _rand_thr_.generate_nonzero_32u();
  };
  static uint8_t random_quickly_4bits()
  {
    return _rand_thr_.generate_quickly_4bits();
  };
  static uint8_t random_quickly_8bits()
  {
    return _rand_thr_.generate_quickly_8bits();
  };
  static uint16_t random_quickly_16bits()
  {
    return _rand_thr_.generate_quickly_16bits();
  };
};                              // end class Rps_Random


class Rps_Do_Output
{
  std::function<void(std::ostream&)> _outfun;
public:
  Rps_Do_Output(std::function<void(std::ostream&)> f) : _outfun(f) {};
  ~Rps_Do_Output() = default;
  Rps_Do_Output(const Rps_Do_Output&) = delete;
  Rps_Do_Output(Rps_Do_Output&&) = delete;
  void out(std::ostream&out) const
  {
    _outfun(out);
  };
};

inline
std::ostream& operator << (std::ostream&out, const Rps_Do_Output&d)
{
  d.out(out);
  return out;
};


////////////////////////////////////////////////////////////////
//// The objid support is in a separate file.
#include "oid_rps.hh"
////////////////////////////////////////////////////////////////

////////////////

////////////////
/// global data, returned by backtrace_create_state called once early
/// in main
extern "C" struct backtrace_state* rps_backtrace_common_state;
/// https://en.wikipedia.org/wiki/X_Macro
#define RPS_BACKTRACE_XMACRO(Mac,...)           \
    Mac(FullOut,__VA_ARGS__)                    \
    Mac(FullClos,__VA_ARGS__)



class Rps_Backtracer
{
public:
  ////// define tagging empty structs
  /*** Below Xmacro expands to :
   *    struct FullOut_Tag{};
   *    struct FullClos_Tag{};
   *    /// etc...
   ***/
#define Rps_BACKTRACER_TagXm(Mac,X) struct Mac##_Tag{};
  RPS_BACKTRACE_XMACRO(Rps_BACKTRACER_TagXm)
#undef Rps_BACKTRACER_TagXm
  ////
  static constexpr std::uint32_t _backtr_magicnum_ = 3364921659; // 0xc890a13b
  enum class Kind : std::uint16_t
  {
    None = 0,
    ////// define kind enumarations
    /*** Below Xmacro expands to :
     *    FullOut_Kind,
     *    FullClos_Kind
     *    /// etc...
     ***/
#define Rps_BACKTRACER_KindXm(Mac,X) Mac##_Kind,
    RPS_BACKTRACE_XMACRO(Rps_BACKTRACER_KindXm)
#undef Rps_BACKTRACER_KindXm
  };
  enum class Todo : std::uint16_t
  {
    Do_Nothing = 0,
    Do_Output,
    Do_Print,
  };
  typedef std::ostringstream FullOut_t;
  typedef std::function<bool(Rps_Backtracer&,  uintptr_t pc,
                             const char*pcfile, int pclineno,
                             const char*pcfun)> FullClos_t;
private:
  static std::recursive_mutex _backtr_mtx_;
  // see https://en.cppreference.com/w/cpp/utility/variant
  std::uint32_t backtr_magic;
  mutable enum Todo backtr_todo;
  mutable bool backtr_ontty;
  const bool backtr_mainthread;
  mutable bool backtr_gotlast;
  mutable std::variant<std::nullptr_t            // for Kind::None
#define Rps_BACKTRACER_VariantXm(Mac,X) , Mac##_t
  RPS_BACKTRACE_XMACRO(Rps_BACKTRACER_VariantXm)
#undef Rps_BACKTRACER_VariantXm
  >  backtr_variant;
  std::ostream*backtr_outs;
  const std::string backtr_fromfile;
  const int backtr_fromline;
  int backtr_skip;
  int backtr_depth;
  const std::string backtr_name;
  void bt_error_method(const char*msg, int errnum);
  static int backtrace_simple_cb(void*data, uintptr_t pc);
  static int backtrace_full_cb(void *data, uintptr_t pc,
                               const char *filename, int lineno,
                               const char *function);
  static void backtrace_error_cb(void* data, const char*msg, int errnum);
public:
  /// function passed to backtrace_create_state as error handler
  static void bt_error_cb(void *data, const char *msg,  int errnum);
  std::uint32_t magicnum() const
  {
    return backtr_magic;
  };
  Kind bkind() const
  {
    return (Kind)backtr_variant.index();
  };
  const std::string bkindname() const;
  std::ostream* boutput() const;
  bool bmainthread() const
  {
    return backtr_mainthread;
  };
  ///
  virtual void output(std::ostream&outs);
  virtual void print(FILE*outf);
  Rps_Backtracer(struct FullOut_Tag,
                 const char*fromfil, const int fromlin, int skip,
                 const char*name,  std::ostream* out=nullptr);
  Rps_Backtracer(struct FullClos_Tag,
                 const char*fromfil, const int fromlin,  int skip,
                 const char*name,
                 const std::function<bool(Rps_Backtracer&bt,  uintptr_t pc,
                                          const char*pcfile, int pclineno,
                                          const char*pcfun
                                         )>& fun);
  std::string pc_to_string(uintptr_t pc, bool*gotmain=nullptr);
  std::string detailed_pc_to_string(uintptr_t pc, const char*pcfile, int pclineno,
                                    const char*pcfun);
  virtual ~Rps_Backtracer();
};                              // end Rps_Backtracer




static inline
std::ostream& operator << (std::ostream& out, const Rps_Backtracer& rpb)
{
  const_cast<Rps_Backtracer*>(&rpb)->output(out);
  return out;
}

// can appear in RPS_WARNOUT etc...
#define RPS_FULL_BACKTRACE_HERE(Skip,Name) \
  Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},__FILE__,__LINE__,(Skip),(Name),&std::cerr)

// can appear in RPS_DEBUG_LOG etc...
#define RPS_DEBUG_BACKTRACE_HERE(Skip,Name) \
  Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},__FILE__,__LINE__,(Skip),(Name),(std::ostream*)nullptr)

////////////////////////////////////////////////////// garbage collector
/* Our top level function to call the garbage collector; the optional
   argument C++ std::function is marking more local data, e.g. calling
   Rps_ObjectRef::gc_mark or Rps_Value::gc_mark or some
   Rps_GarbageCollector::mark??? routine. See comments or warnings in
   garbcoll_rps.cc file... */
extern "C" void rps_garbage_collect(std::function<void(Rps_GarbageCollector*)>* fun=nullptr);
class Rps_GarbageCollector
{
  friend void rps_garbage_collect(std::function<void(Rps_GarbageCollector*)>* fun);
  static unsigned constexpr _gc_magicnum_ = 0xdae21691;  // 3672250001
  static std::atomic<Rps_GarbageCollector*> gc_this_;
  static std::atomic<uint64_t> gc_count_;
  friend class Rps_QuasiZone;
  std::mutex gc_mtx;
  std::atomic<bool> gc_running;
  unsigned gc_magic;
  const std::function<void(Rps_GarbageCollector*)> gc_rootmarkers;
  std::deque<Rps_ObjectRef> gc_obscanque;
  uint64_t gc_nbscan;
  uint64_t gc_nbmark;
  uint64_t gc_nbdelete;
  uint64_t gc_nbroots;
  double gc_startrealtime;
  double gc_startelapsedtime;
  double gc_startprocesstime;
private:
  Rps_GarbageCollector(const std::function<void(Rps_GarbageCollector*)> &rootmarkers=nullptr);
  ~Rps_GarbageCollector();
  void run_gc(void);
  void mark_gcroots(void);
public:
  double elapsed_time(void) const
  {
    return rps_elapsed_real_time() - gc_startelapsedtime;
  };
  double process_time(void) const
  {
    return rps_process_cpu_time() - gc_startprocesstime;
  };
  double start_real_time(void) const
  {
    return gc_startrealtime;
  }
  uint64_t nb_roots() const
  {
    return gc_nbroots;
  };
  uint64_t nb_scans() const
  {
    return gc_nbscan;
  };
  uint64_t nb_marks() const
  {
    return gc_nbmark;
  };
  uint64_t nb_deletions() const
  {
    return gc_nbdelete;
  };
  void mark_obj(Rps_ObjectZone* ob);
  void mark_obj(Rps_ObjectRef ob);
  void mark_value(Rps_Value val, unsigned depth=0);
  inline void mark_root_value(Rps_Value val);
  inline void mark_root_objectref(Rps_ObjectRef obr);
  inline void mark_call_stack(Rps_CallFrame*topframe);
  inline bool is_valid_garbcoll() const
  {
    return gc_magic == _gc_magicnum_;
  };
};                              // end class Rps_GarbageCollector

////////////////////////////////////////////////////// quasi zones

class Rps_TypedZone
{
  friend class Rps_GarbageCollector;
  friend class Rps_ProtoCallFrame;
protected:
  const Rps_Type qz_type;
  volatile mutable std::atomic_uint16_t qz_gcinfo;
public:
  Rps_TypedZone(const Rps_Type ty) : qz_type(ty), qz_gcinfo(0) {};
  ~Rps_TypedZone() {};
  Rps_Type stored_type(void) const
  {
    return qz_type;
  };
};


class Rps_QuasiZone : public Rps_TypedZone
{
  friend class Rps_GarbageCollector;
  friend class Rps_LexTokenZone;
  // we keep each quasi-zone in the qz_zonvec
  static std::recursive_mutex qz_mtx;
  static std::vector<Rps_QuasiZone*> qz_zonvec;
  static uint32_t qz_cnt;
  // the cumulated amount of allocated words
  static std::atomic<uint64_t> qz_alloc_cumulw;
  uint32_t qz_rank;             // the rank in qz_zonvec;
protected:
  inline void* operator new (std::size_t siz, std::nullptr_t);
  inline void* operator new (std::size_t siz, unsigned wordgap);
  static constexpr uint16_t qz_gcmark_bit = 1;
public:
  /// gives the number of machine words (8 bytes) allocated since
  /// start of process...
  static uint64_t cumulative_allocated_wordcount()
  {
    return qz_alloc_cumulw.load();
  };
  static void initialize(void);
  static inline Rps_QuasiZone*nth_zone(uint32_t rk);
  static inline Rps_QuasiZone*raw_nth_zone(uint32_t rk, Rps_GarbageCollector&);
  inline bool is_gcmarked(Rps_GarbageCollector&) const;
  inline void set_gcmark(Rps_GarbageCollector&);
  inline void clear_gcmark(Rps_GarbageCollector&);
  static void clear_all_gcmarks(Rps_GarbageCollector&);
  inline static void run_locked_gc(Rps_GarbageCollector&, std::function<void(Rps_GarbageCollector&)>);
  inline static void every_zone(Rps_GarbageCollector&, std::function<void(Rps_GarbageCollector&, Rps_QuasiZone*)>);
  template <typename ZoneClass, class ...Args> static ZoneClass*
  rps_allocate(Args... args)
  {
    return new(nullptr) ZoneClass(args...);
  };
  template <typename ZoneClass> static ZoneClass*
  rps_allocate0(void)
  {
    return new(nullptr) ZoneClass();
  };
  template <typename ZoneClass, typename Arg1Class> static ZoneClass*
  rps_allocate1(Arg1Class arg1)
  {
    return new(nullptr) ZoneClass(arg1);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class> static ZoneClass*
  rps_allocate2(Arg1Class arg1, Arg2Class arg2)
  {
    return new(nullptr) ZoneClass(arg1, arg2);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class> static ZoneClass*
  rps_allocate3(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3)
  {
    return new(nullptr) ZoneClass(arg1, arg2, arg3);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class, typename Arg4Class>
  static ZoneClass*
  rps_allocate4(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3, Arg4Class arg4)
  {
    return new(nullptr) ZoneClass(arg1, arg2, arg3, arg4);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class, typename Arg4Class, typename Arg5Class>
  static ZoneClass*
  rps_allocate5(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3, Arg4Class arg4, Arg5Class arg5)
  {
    return new(nullptr) ZoneClass(arg1, arg2, arg3, arg4, arg5);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class, typename Arg4Class, typename Arg5Class, typename Arg6Class>
  static ZoneClass*
  rps_allocate6(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3, Arg4Class arg4, Arg5Class arg5, Arg6Class arg6)
  {
    return new(nullptr) ZoneClass(arg1, arg2, arg3, arg4, arg5, arg6);
  };
  template <typename ZoneClass, class ...Args> static ZoneClass*
  rps_allocate_with_wordgap(unsigned wordgap, Args... args)
  {
    return new(wordgap) ZoneClass(args...);
  };
  template <typename ZoneClass> static ZoneClass*
  rps_allocate0_with_wordgap(unsigned wordgap)
  {
    return new(wordgap) ZoneClass();
  };
  template <typename ZoneClass, typename Arg1Class> static ZoneClass*
  rps_allocate1_with_wordgap(unsigned wordgap, Arg1Class arg1)
  {
    return new(wordgap) ZoneClass(arg1);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class> static ZoneClass*
  rps_allocate2_with_wordgap(unsigned wordgap, Arg1Class arg1, Arg2Class arg2)
  {
    return new(wordgap) ZoneClass(arg1,arg2);
  };
  template <typename ZoneClass, typename Arg1Class, typename Arg2Class, typename Arg3Class> static ZoneClass*
  rps_allocate3_with_wordgap(unsigned wordgap, Arg1Class arg1, Arg2Class arg2, Arg3Class arg3)
  {
    return new(wordgap) ZoneClass(arg1,arg2,arg3);
  };
  void register_in_zonevec(void);
  void unregister_in_zonevec(void);
protected:
  inline Rps_QuasiZone(Rps_Type typ);
  virtual ~Rps_QuasiZone();
  //// the size, in 64 bits words, of the actual quasizone, needed by
  //// the garbage collector.  It should be rounded up, not down (for
  //// potential gaps required by ABI alignment).
  virtual uint32_t wordsize() const =0;
public:
  virtual Rps_Type type() const
  {
    return qz_type;
  };
} __attribute__((aligned(rps_allocation_unit)));
// end class Rps_QuasiZone;



//////////////////////////////////////////////////////////// zone values
class Rps_ZoneValue : public Rps_QuasiZone
{
  friend class Rps_Value;
  friend class Rps_GarbageCollector;
protected:
  inline Rps_ZoneValue(Rps_Type typ);
public:
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const =0;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth) const =0;
  virtual void dump_scan(Rps_Dumper* du, unsigned depth) const =0;
  virtual Json::Value dump_json(Rps_Dumper* du) const =0;
  virtual Rps_HashInt val_hash () const =0;
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const =0;
  virtual bool equal(const Rps_ZoneValue&zv) const =0;
  virtual bool less(const Rps_ZoneValue&zv) const =0;
  inline bool operator == (const Rps_ZoneValue&zv) const;
  bool operator != (const Rps_ZoneValue&zv) const
  {
    return !(*this == zv);
  };
  virtual bool lessequal(const Rps_ZoneValue&zv) const
  {
    return *this == zv || less(zv);
  }
  inline bool operator < (const Rps_ZoneValue&zv) const;
  inline bool operator <= (const Rps_ZoneValue&zv) const;
  inline bool operator > (const Rps_ZoneValue&zv) const;
  inline bool operator >= (const Rps_ZoneValue&zv) const;
};    // end class Rps_ZoneValue

/////////////////////////////////////////////////// lazy hashed values
class Rps_LazyHashedZoneValue : public Rps_ZoneValue
{
private:
  mutable volatile std::atomic<Rps_HashInt> _lazyhash;
protected:
  virtual Rps_HashInt compute_hash(void) const =0;
  inline Rps_LazyHashedZoneValue(Rps_Type typ);
  virtual ~Rps_LazyHashedZoneValue() {};
  // we need to serialize some rare modifications (e.g. of metadata in
  // trees). They are rare, so we use a mutex indexed by the last ten
  // bits of hash code...  We are supposing that metadata is
  // read-mostly and only occasionally written...
  static constexpr unsigned lazy_nbmutexes = 1024;
  static std::mutex lazy_mtxarr[lazy_nbmutexes];
public:
  Rps_HashInt lazy_hash() const
  {
    return _lazyhash.load();
  };
  virtual Rps_HashInt val_hash () const
  {
    volatile Rps_HashInt h = _lazyhash.load();
    if (RPS_UNLIKELY(h == 0))
      {
        h = compute_hash();
        RPS_ASSERT (h != 0);
        _lazyhash.store(h);
      }
    return h;
  };
};                              // end of Rps_LazyHashedZoneValue




////////////////////////////////////////////////// file path utilities
/** Given a shell pattern like foo/x*.h and a directory path like
   /usr/include:/usr/local/include find a readable plain file path;
   tilde patterns ~joe are expanded and $XX are expanded but not command
   line substitution like $(ls -lt *foo|head -1); for example
   rps_glob_plain_path("sys/stat.h",
   "/usr/include/:/usr/include/x86-64-linux/gnu/") would return
   "/usr/include/sys/stat.h" on my Linux desktop. If no file is found,
   the empty string is returned. */
std::string rps_glob_plain_file_path(const char*shellpat, const char*dirpath);

//////////////////////////////////////////////////////////// immutable strings

// compute a long hash in ht[0] and ht[1]. Return the number of UTF-8
// character or else 0 if cstr with len bytes is not proper UTF-8. This is an important function, whose source code is shared in guifltk-refpersys
extern "C"
int rps_compute_cstr_two_64bits_hash(int64_t ht[2], const char*cstr, int len= -1);

static inline Rps_HashInt rps_hash_cstr(const char*cstr, int len= -1);

class Rps_String : public Rps_LazyHashedZoneValue
{
  friend class Rps_LexTokenZone;
  friend class Rps_LexTokenValue;
  friend class Rps_TokenSource;
  friend Rps_String*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_String,const char*,int>(unsigned,const char*,int);
  const uint32_t _bytsiz;
  const uint32_t _utf8len;
  union
  {
    const char _sbuf[RPS_FLEXIBLE_DIM];
    char _alignbuf[rps_allocation_unit] __attribute__((aligned(rps_allocation_unit)));
  };
protected:
  inline Rps_String (const char*cstr, int len= -1);
  static inline const char*normalize_cstr(const char*cstr);
  static inline int normalize_len(const char*cstr, int len);
  static inline uint32_t safe_utf8len(const char*cstr, int len);
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    return rps_hash_cstr(_sbuf);
  };
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const { };
public:
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual uint32_t wordsize() const
  {
    return (sizeof(Rps_String)+_bytsiz+1)/sizeof(void*);
  };
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual void dump_scan(Rps_Dumper*, unsigned) const {};
  virtual Json::Value dump_json(Rps_Dumper*) const;
  static const Rps_String* make(const char*cstr, int len= -1);
  static inline const Rps_String* make(const std::string&s);
  const char*cstr() const
  {
    return _sbuf;
  };
  const std::string cppstring() const
  {
    return std::string(_sbuf);
  };
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == Rps_Type::String)
      {
        auto othstr = reinterpret_cast<const Rps_String*>(&zv);
        auto lh = lazy_hash();
        auto othlh = othstr->lazy_hash();
        if (lh != 0 && othlh != 0 && lh != othlh) return false;
        return !strcmp(cstr(), othstr->cstr());
      }
    else return false;
  }
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() > Rps_Type::String) return false;
    if (zv.stored_type() < Rps_Type::String) return true;
    auto othstr = reinterpret_cast<const Rps_String*>(&zv);
    return strcmp(cstr(), othstr->cstr()) < 0;
  };
};    // end class Rps_String


//////////////// boxed doubles
class Rps_Double  : public Rps_LazyHashedZoneValue
{
  friend Rps_Double*
  Rps_QuasiZone::rps_allocate<Rps_Double,double>(double);
  const double _dval;
protected:
  inline Rps_Double (double d=0.0);
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    auto rh = std::hash<double> {}((double)_dval);
    Rps_HashInt h = static_cast<Rps_HashInt> (rh);
    if (RPS_UNLIKELY(h == 0))
      h = 987383;
    return h;
  };
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const { };
  virtual void dump_scan(Rps_Dumper*, unsigned) const {};
  virtual Json::Value dump_json(Rps_Dumper*) const
  {
    return Json::Value(_dval);
  };
public:
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  double dval() const
  {
    return _dval;
  };
  virtual uint32_t wordsize() const
  {
    // we need to round the 64 bits word size up, hence...
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  static inline const Rps_Double*make(double d=0.0);
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == Rps_Type::Double)
      {
        auto othdbl = reinterpret_cast<const Rps_Double*>(&zv);
        return _dval == othdbl->dval();
      };
    return false;
  };
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() > Rps_Type::Double) return false;
    if (zv.stored_type() < Rps_Type::Double) return true;
    auto othdbl = reinterpret_cast<const Rps_Double*>(&zv);
    return _dval < othdbl->dval();
  };
}; // end class Rps_Double

class Rps_DequVal : public std::deque<Rps_Value>
{
  const char*dqu_srcfil;
  int dqu_srclin;
public:
  typedef std::deque<Rps_Value> std_deque_superclass;
  Rps_DequVal() : std::deque<Rps_Value>(), dqu_srcfil("?"), dqu_srclin(-1) {} ;
  Rps_DequVal(const char*sfil, int lin) : std::deque<Rps_Value>(), dqu_srcfil(sfil), dqu_srclin(lin) {};
  virtual ~Rps_DequVal()
  {
    dqu_srcfil=nullptr;
    dqu_srclin=0;
  };
  virtual Rps_HashInt compute_hash(void) const;
  Rps_DequVal(std::initializer_list<Rps_Value> il,const char*sfil=nullptr, int lin=0)
    : std::deque<Rps_Value>(il), dqu_srcfil(sfil), dqu_srclin(lin) {};
  Rps_DequVal(const std::vector<Rps_Value>& vec,const char*sfil=nullptr, int lin=0);
  Rps_DequVal(const Json::Value&jv, Rps_Loader*ld,const char*sfil=nullptr, int lin=0);
  void output(std::ostream&out, unsigned depth, unsigned maxdepth) const;
  Json::Value dump_json(Rps_Dumper*du) const;
  virtual void dump_scan(Rps_Dumper* du, unsigned depth) const;
  void really_gc_mark(Rps_GarbageCollector&gc, unsigned depth) const;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0) const
  {
    really_gc_mark(gc, depth);
  };
  void push_back(const Rps_Value val);
  void pop_front(void);
};                              // end class Rps_DequVal

std::ostream&
operator << (std::ostream&out, const Rps_DequVal& dq);

////////////////////////////////////////////////////////////////
struct Rps_ChunkData_st;

extern "C" void rps_parsrepl_failing_at(const char*fil, int lin, int cnt, const std::string&failstr);

#define RPS_PARSREPL_FAILURE_AT(Fram,Out,Fil,Lin,Cnt) do {     \
    std::ostringstream _failstream_##Lin;                      \
    _failstream_##Lin << Out << " ~#" << Cnt << std::endl;     \
    Rps_Backtracer backtr##Lin(Rps_Backtracer::FullOut_Tag{},  \
             (Fil),(Lin),1,                                    \
             "ParsReplFailing",                                \
             &_failstream_##Lin);                              \
    rps_parsrepl_failing_at(Fil,Lin,Cnt,                       \
          _failstream_##Lin.str());                            \
} while(0)

#define RPS_PARSREPL_FAILURE(Fram,Out) \
  RPS_PARSREPL_FAILURE_AT(Fram,Out,__FILE__,__LINE__,__COUNTER__)

extern "C" void rps_do_one_repl_command(Rps_CallFrame*callframe, Rps_ObjectRef obenv,
                                        const std::string&cmd,
                                        const char*title=nullptr);




////////////////////////////////// token sources are for lexing
class Rps_TokenSource           // this is *not* a value .....
{
  friend class Rps_LexTokenValue;
  friend class Rps_LexTokenZone;
  friend void
  rps_do_one_repl_command(Rps_CallFrame*callframe,
                          Rps_ObjectRef obenvarg,
                          const std::string&cmd,
                          const char*title);
  std::string toksrc_name;
  int toksrc_line, toksrc_col;
  int toksrc_counter;
  static Rps_TokenSource* toksrc_current_;
protected:
  /// could be called by subclasses
  void really_gc_mark(Rps_GarbageCollector&gc, unsigned depth);
  std::string toksrc_linebuf;
  Rps_DequVal toksrc_token_deq;
  Rps_StringValue* toksrc_ptrnameval;
  Rps_TokenSource(std::string name);
  void set_name(std::string name)
  {
    toksrc_name = name;
  };
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0);
  std::string lex_quoted_literal_string(Rps_CallFrame*callframe);
  std::string lex_raw_literal_string(Rps_CallFrame*callframe);
  Rps_Value lex_code_chunk(Rps_CallFrame*callframe);
  Rps_Value lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg, Rps_ChunkData_st*chkdata);
  void starting_new_input_line(void)
  {
    toksrc_col=0;
    toksrc_line++;
  };
  void advance_cursor_bytes(unsigned nb)
  {
    toksrc_col += nb;
    if (toksrc_col > (int) toksrc_linebuf.size())
      toksrc_col = toksrc_linebuf.size();
  };
  Rps_Value get_delimiter(Rps_CallFrame*callframe);
public:
  static constexpr unsigned max_gc_depth = 128;
  static Rps_TokenSource* current_token_source(void)
  {
    return toksrc_current_;
  };
  const char*curcptr(void) const
  {
    if (toksrc_linebuf.empty())
      return nullptr;
    auto linesiz = toksrc_linebuf.size();
    if (toksrc_col>=0 && (int)toksrc_col<(int)linesiz)
      return toksrc_linebuf.c_str()+toksrc_col;
    return nullptr;
  };                            // end Rps_TokenSource::curcptr
  const std::string current_line(void) const
  {
    return toksrc_linebuf;
  };
  const Rps_LexTokenZone* make_token(Rps_CallFrame*callframe,
                                     Rps_ObjectRef lexkind, Rps_Value lexval, const Rps_String*sourcev);
  virtual ~Rps_TokenSource();
  void display_current_line_with_cursor(std::ostream&out) const;
  virtual void output (std::ostream&out, unsigned depth, unsigned maxdepth) const = 0;
  /// TODO: the display method for token source would also show the cursor in a fancy way,
  /// inspired by GCC-12 error messages.  Maybe like
  /// ***** line 345 "abcdef" col 2
  /// *****             ^
  virtual void display (std::ostream&out) const = 0;
  const Rps_DequVal& token_dequeue(void) const
  {
    return toksrc_token_deq;
  };
  void consume_front_token(Rps_CallFrame*callframe, bool *psuccess=nullptr);
  void append_back_new_token(Rps_CallFrame*callframe, Rps_Value tokenv);
  virtual bool get_line(void) =0; // gives true when another line has been read
  Rps_TokenSource(const Rps_TokenSource&) = delete;
  Rps_TokenSource() = delete;
  const std::string& name(void) const
  {
    return toksrc_name;
  };
  const std::string position_str(int col= -1) const;
  int token_count(void) const
  {
    return  toksrc_counter;
  };
  // return the source name as a string value, hopefully memoized
  Rps_Value source_name_val(Rps_CallFrame*callframe);
  // lookahead a lexical token, with a an empty toksrc_token_deq
  // rank#0 being the next one
  Rps_Value lookahead_token(Rps_CallFrame*callframe, unsigned rank=0);
  /* TODO: parsing routines can also be used for lookahead purpose,
     with the convention that the bool pointer -pokparse- is null, and
     returning nullptr on lookahead falure, and the true object (of
     oid _1GIJ6Koh9Rn009AWww) on lookahead success */
#define RPS_DO_LOOKAHEAD ((bool*)nullptr)
  static bool is_looking_ahead(bool*p)
  {
    return p == RPS_DO_LOOKAHEAD;
  };
  ///////////////////
  ////
  //// generated parsing declarations
#include "generated/rps-parser-decl.hh"
  ///////////////////
  //// Our parsing routines; the token dequeue pointer is for
  //// lookahead... The flag pointed by `pokparse` is set to true if
  //// provided, non-nil, and parsing successful.
  ///////////////////
  Rps_Value parse_using_closure(Rps_CallFrame*callframe, Rps_ClosureValue closval);
  ////
  //// Parse an expression. On success, the parsed expression is
  //// returned. On failure, the nil value is returned, and *pokparse
  //// is set to false when given.
  Rps_Value parse_expression(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  //// generic routine to parse symetrical binary operations like
  //// addition +
  Rps_Value parse_symmetrical_binaryop(Rps_CallFrame*callframe,
                                       Rps_ObjectRef binoper, Rps_ObjectRef bindelim,
                                       std::function<Rps_Value(Rps_CallFrame*,bool*)> parser_binop,
                                       bool*pokparse, const char*opername=nullptr);
  //// generic routine to parse asymmetrical non commutative
  //// operations like division /
  Rps_Value parse_asymmetrical_binaryop(Rps_CallFrame*callframe,
                                        Rps_ObjectRef binoper, Rps_ObjectRef bindelim,
                                        std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,
                                            bool*)> parser_leftop,
                                        std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,bool*)>
                                        parser_rightop,
                                        bool*pokparse, const char*opername=nullptr);
  Rps_Value parse_polyop(Rps_CallFrame*callframe,  Rps_ObjectRef polyoper, Rps_ObjectRef polydelim,
                         std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,bool*)> parser_suboperand,
                         bool*pokparse, const char*opername=nullptr);
  ///
  /// A disjunction is a sequence of one or more disjuncts separated
  /// by the logical or operator denoted || (see delimiter
  /// _1HsUfOkNw0W033EIW1)
  Rps_Value parse_disjunction(Rps_CallFrame*callframe,  bool*pokparse=nullptr);
  ///
  /// A conjunction is a sequence of one or more conjuncts seperated
  /// by the logical and operator denoted && (see delimiter
  /// _2YVmrhVcwW00120rTK)
  Rps_Value parse_conjunction(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  ///
  /// A comparison is either a simple comparand (or sum) or two of
  /// them deparated by compare operators like != (see delimiter
  /// _1vF5VHnSdhr01VBomx) or <= (see delimiter _1mfq8qfixB401umCL9")
  /// etc..
  Rps_Value parse_comparison(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  Rps_Value parse_comparand(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  Rps_Value parse_sum(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  Rps_Value parse_product(Rps_CallFrame*callframe, bool*pokparse=nullptr);

  Rps_Value parse_factor(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  Rps_Value parse_term(Rps_CallFrame*callframe, bool*pokparse=nullptr);
  /// a primary expression is a simple thing
  Rps_Value parse_primary(Rps_CallFrame*callframe,  bool*pokparse=nullptr);
  bool can_start_primary(Rps_CallFrame*callframe);
  /// Once we have parsed a primary, it could be followed by a primary
  /// complement. This routine is given the primary expression and
  /// return, when successful, a larger expression. It accepts fields
  /// and message sending.
  Rps_Value parse_primary_complement(Rps_CallFrame*callframe, Rps_Value primaryexp, bool*pokparse=nullptr);
#warning other recursive descent parsing routines are needed, with a syntax documented in doc/repl.md
  ///////
  int line(void) const
  {
    return toksrc_line;
  };
  int col(void) const
  {
    return toksrc_col;
  };
  /// on lexical error, get_token returns null and does not change the position
  Rps_LexTokenValue get_token(Rps_CallFrame*callframe);
};                              // end Rps_TokenSource

inline std::ostream& operator << (std::ostream&out, Rps_TokenSource& toksrc)
{
  toksrc.output(out, 0, Rps_Value::debug_maxdepth);
  return out;
}

class Rps_CinTokenSource : public Rps_TokenSource
{
public:
  virtual void output(std::ostream&out, unsigned depth, unsigned maxdepth) const
  {
    if (depth > maxdepth && &out != &std::cout && &out != &std::cerr && &out != &std::clog)
      RPS_WARNOUT("Rps_CinTokenSource " << name()
                  << " depth=" << depth << " greater than maxdepth=" << maxdepth);
    out << "CinTokenSource" << name() << '@' << position_str() << " tok.cnt:" << token_count();
  };
  Rps_CinTokenSource();
  virtual ~Rps_CinTokenSource();
  virtual bool get_line(void);
  virtual void display(std::ostream&out) const;
};             // end Rps_CinTokenSource


class Rps_StreamTokenSource : public Rps_TokenSource
{
  std::ifstream toksrc_input_stream;
public:
  Rps_StreamTokenSource(std::string path);
  virtual void output(std::ostream&out, unsigned depth, unsigned maxdepth) const
  {
    if (depth > maxdepth)
      RPS_WARNOUT("Rps_StreamTokenSource " << name()
                  << " depth=" << depth << " greater than maxdepth=" << maxdepth);
    out << "StreamTokenSource" << name() << '@' << position_str() << " tok.cnt:" << token_count();
  };
  virtual ~Rps_StreamTokenSource();
  virtual bool get_line(void);
  virtual void display(std::ostream&out) const;
};             // end Rps_StreamTokenSource



class Rps_StringTokenSource : public Rps_TokenSource
{
  std::istringstream toksrcstr_inp;
  const std::string toksrcstr_str;
public:
  Rps_StringTokenSource(std::string inpstr, std::string name);
  virtual void output(std::ostream&out, unsigned depth, unsigned maxdepth) const;
  virtual  ~Rps_StringTokenSource();
  virtual bool get_line();
  const std::string str() const
  {
    return toksrcstr_str;
  };
  virtual void display(std::ostream&out) const;
};                                                            // end Rps_StringTokenSource

class Rps_MemoryFileTokenSource;
extern "C" void rps_parse_user_preferences(Rps_MemoryFileTokenSource*);

/// this is testing if the user preferences has been parsed
extern "C" bool rps_has_parsed_user_preferences(void);

extern "C" std::string rps_userpref_get_string(const std::string& section, const std::string& name,
    const std::string& default_value);
/// C compatible: all arguments are non-null pointers, returns an
/// strdup-ed string. Sets *pfound iff found the preference
extern "C" const char*rps_userpref_find_dup_cstring(bool *pfound,
    const char*csection, const char* cname);
/// returns the raw preference cstring without duplication or null if
/// not found. It might not work....
extern "C" const char*rps_userpref_raw_cstring(const char*csection, const char*cname);

extern "C" long rps_userpref_get_long(const std::string& section, const std::string& name, long default_value);
/// C compatible: all arguments are non-null pointers, returns a long
/// preference or else 0. Sets *pfound iff found the preference
extern "C" long rps_userpref_find_clong(bool *pfound,
                                        const char*csection, const char* cname);

extern "C" double rps_userpref_get_double(const std::string& section, const std::string& name, double default_value);
/// C compatible: all arguments are non-null pointers, returns a
/// double preference or else 0.0. Sets *pfound iff found the preference
extern "C" double rps_userpref_find_cdouble(bool *pfound,
    const char*csection, const char* cname);

extern "C" bool rps_userpref_get_bool(const std::string& section, const std::string& name, bool default_value);
/// C compatible: all arguments are non-null pointers, returns a
/// boolean preference or else false. Sets *pfound iff found the
/// preference
extern "C"  bool rps_userpref_find_cbool(bool *pfound,
    const char*csection, const char* cname);

extern "C" bool rps_userpref_has_section(const std::string& section);
extern "C" bool rps_userpref_with_csection(const char*csection);

extern "C" bool rps_userpref_has_value(const std::string& section, const std::string& name);
extern "C" bool rps_userpref_with_cvalue(const char*csection, const char*cname);



////////////////
class Rps_MemoryFileTokenSource : public Rps_TokenSource
{
  friend void  rps_parse_user_preferences(Rps_MemoryFileTokenSource*);
  const std::string toksrcmfil_path;
  const char*toksrcmfil_start; // page-aligned, in memory
  const char*toksrcmfil_line;  // pointer to start of current line
  const char*toksrcmfil_end; // the end of the file as mmap-ed
  const char*toksrcmfil_nextpage; // the next virtual memory page (page-aligned)
public:
  Rps_MemoryFileTokenSource(const std::string path);
  virtual void output(std::ostream&out, unsigned depth, unsigned maxdepth) const;
  virtual  ~Rps_MemoryFileTokenSource();
  virtual bool get_line();
  const std::string path() const
  {
    return toksrcmfil_path;
  };
  virtual void display(std::ostream&out) const;
};        // end Rps_MemoryFileTokenSource


constexpr const unsigned rps_chunkdata_magicnum = 0x2fa19e6d; // 799121005
struct Rps_ChunkData_st /// not a value neither
{
  unsigned chunkdata_magic;     // always rps_chunkdata_magicnum
  int chunkdata_lineno;
  int chunkdata_colno;
  std::string chunkdata_name;
  char chunkdata_endstr[16];
};                              // end Rps_ChunkData_st

//////////////// boxed lexical token - always transient
class Rps_LexTokenZone  : public Rps_LazyHashedZoneValue
{
  friend Rps_LexTokenZone*
  Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,Rps_String*,int,int>(Rps_ObjectRef lxkind,Rps_Value lxval,Rps_String*lxpath,int lxline,int lxcol);
  friend class Rps_GarbageCollector;
  friend class Rps_LexTokenValue;
  friend class Rps_QuasiZone;
  friend class Rps_TokenSource;
  Rps_ObjectRef lex_kind;
  Rps_Value lex_val;
  const Rps_String* lex_file;
  const Rps_TokenSource* lex_src;
  int lex_lineno;
  int lex_colno;
  unsigned lex_serial;
public:
  inline void* operator new (std::size_t siz, std::nullptr_t)
  {
    return Rps_QuasiZone::operator new(siz,nullptr);
  }
  void set_serial (unsigned serial);
  unsigned serial(void) const
  {
    return lex_serial;
  };
protected:
  Rps_LexTokenZone(Rps_TokenSource*psrc, Rps_ObjectRef kind, Rps_Value val, const Rps_String*string, int line, int col);
protected:
  virtual ~Rps_LexTokenZone();
  virtual Rps_HashInt compute_hash(void) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const;
  virtual void dump_scan(Rps_Dumper*, unsigned) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
public:
  Rps_ObjectRef lxkind() const
  {
    return lex_kind;
  };
  Rps_Value lxval() const
  {
    return lex_val;
  };
  const Rps_String* lxfile() const
  {
    return lex_file;
  };
  Rps_TokenSource*lxsrc() const
  {
    return const_cast<Rps_TokenSource*>(lex_src);
  };
  int lxline() const
  {
    return lex_lineno;
  };
  int lxcol() const
  {
    return lex_colno;
  };
  static Rps_ObjectRef the_lexical_token_class(void);
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual uint32_t wordsize() const
  {
    // we need to round the 64 bits word size up, hence...
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual bool equal(const Rps_ZoneValue&zv) const;
  virtual bool less(const Rps_ZoneValue&zv) const;
#pragma message "probably obsolete Rps_LexTokenZone::{lexical_line_getter_fun,tokenize} functions"
  /// The signature of a function to retrieve the next line....  on
  /// purpose close to existing rps_repl_get_next_line in our C++ file
  /// repl_rps.cc...
  typedef
  std::function<bool(Rps_CallFrame*callframe,
                     std::istream*inp,
                     const char*input_name,
                     const char**plinebuf, int*plineno)
  > lexical_line_getter_fun;
  // Tokenize a lexical token; an optional double-ended queue of
  // already lexed token enable limited backtracking when needed....
  static const Rps_LexTokenZone* tokenize
  (Rps_CallFrame*callframe, std::istream*inp,
   const char*input_name,
   const char**plinebuf, int&lineno, int& colno,
   lexical_line_getter_fun linegetter = nullptr,
   std::deque<Rps_LexTokenZone*>* pque = nullptr);
}; // end class Rps_LexTokenZone


/// a C++ closure for getting the REPL lexical token.... with
/// lookahead=0, next token, with lookahead=1 the second-next token
extern "C" std::function<Rps_LexTokenValue(Rps_CallFrame*,unsigned)> rps_repl_cmd_lexer_fun;
/// these REPL lexical tokens are looked ahead, so we need a function
/// to consume them... Returning true when the leftmost token is
/// forgotten
extern "C" std::function<bool(Rps_CallFrame*)> rps_repl_consume_cmd_token_fun;
extern "C" bool rps_repl_stopped;

//////////////////////////////////////////////////////////// object zones


/// magic getter C++ functions
typedef Rps_Value rps_magicgetterfun_t(Rps_CallFrame*callerframe, const Rps_Value val, const Rps_ObjectRef obattr);
#define RPS_GETTERFUN_PREFIX "rpsget"
// by convention, the extern "C" getter function inside fictuous attribute
// _3kVHiDzT42h045vHaB would be named rpsget_3kVHiDzT42h045vHaB

// application C++ functions
// the applied closure is in field cfram_clos of the caller frame.
// applying function
typedef Rps_TwoValues rps_applyingfun_t (Rps_CallFrame*callerframe,
    const Rps_Value arg0, const Rps_Value arg1, const Rps_Value arg2,
    const Rps_Value arg3, const std::vector<Rps_Value>* restargs);
#define RPS_APPLYINGFUN_PREFIX "rpsapply"
// by convention, the extern "C" applying function inside the fictuous connective _45vHaB3kVHiDzT42h0
// would be named rpsapply_45vHaB3kVHiDzT42h0
class Rps_Payload;
class Rps_ObjectZone : public Rps_ZoneValue
{
  friend class Rps_Object_Display;
  ///
public:
  enum registermode_en
  {
    OBZ_DONT_REGISTER,
    OBZ_REGISTER
  };
  friend class Rps_Loader;
  friend class Rps_Dumper;
  friend class Rps_Payload;
  friend class Rps_ObjectRef;
  friend class Rps_Value;
  friend Rps_ObjectZone*
  Rps_QuasiZone::rps_allocate<Rps_ObjectZone,Rps_Id,registermode_en>(Rps_Id,registermode_en);
private:
  /// fields
  const Rps_Id ob_oid;
  mutable std::recursive_mutex ob_mtx;
  std::atomic<Rps_ObjectZone*> ob_class;
  std::atomic<Rps_ObjectZone*> ob_space;
  std::atomic<double> ob_mtime;
  std::map<Rps_ObjectRef, Rps_Value> ob_attrs;
  std::vector<Rps_Value> ob_comps;
  std::atomic<Rps_Payload*> ob_payload;
  std::atomic<rps_magicgetterfun_t*> ob_magicgetterfun;
  std::atomic<rps_applyingfun_t*> ob_applyingfun;
  /// constructors
  Rps_ObjectZone(Rps_Id oid, registermode_en regmod);
  Rps_ObjectZone(void);
  ~Rps_ObjectZone();
  static std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher> ob_idmap_;
  static std::map<Rps_Id,Rps_ObjectZone*> ob_idbucketmap_[Rps_Id::maxbuckets];
  static std::recursive_mutex ob_idmtx_;
  static void register_objzone(Rps_ObjectZone*);
  static Rps_Id fresh_random_oid(Rps_ObjectZone*ob =nullptr);
protected:
  void loader_set_class (Rps_Loader*ld, Rps_ObjectZone*obzclass)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(obzclass != nullptr);
    ob_class.store(obzclass);
  };
  void loader_set_mtime (Rps_Loader*ld, double mtim)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(mtim>0.0);
    ob_mtime.store(mtim);
  };
  void loader_set_space (Rps_Loader*ld, Rps_ObjectZone*obzspace)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(obzspace != nullptr);
    ob_space.store(obzspace);
  };
  void loader_put_attr (Rps_Loader*ld, const Rps_ObjectRef keyatob, const Rps_Value atval)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(keyatob);
    RPS_ASSERT(atval);
    ob_attrs.insert({keyatob, atval});
  };
  void loader_put_magicattrgetter(Rps_Loader*ld, rps_magicgetterfun_t*mfun)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(mfun != nullptr);
    RPS_DEBUG_LOG(LOAD,
                  "loader_put_magicattrgetter thisob=" << Rps_ObjectRef(this)
                  << ", mfun=" << (void*)mfun);
    ob_magicgetterfun.store(mfun);
  };
  void loader_put_applyingfunction(Rps_Loader*ld, rps_applyingfun_t*afun)
  {
    RPS_ASSERT(ld != nullptr);
    RPS_ASSERT(afun != nullptr);
    Rps_ObjectRef thisob(this);
    RPS_DEBUG_LOG(LOAD,
                  "loader_put_applyingfunction thisob=" <<  Rps_ObjectRef(this)
                  << ", afun=" << (void*)afun);
    ob_applyingfun.store(afun);
  };
  void loader_reserve_comps (Rps_Loader*ld, unsigned nbcomps)
  {
    RPS_ASSERT(ld != nullptr);
    ob_comps.reserve(nbcomps);
  };
  void loader_add_comp (Rps_Loader*ld, const Rps_Value compval)
  {
    RPS_ASSERT(ld != nullptr);
    ob_comps.push_back(compval);
  };
public:
  std::recursive_mutex* objmtxptr(void) const
  {
    return &ob_mtx;
  };
  rps_magicgetterfun_t*magic_getter_function(void) const
  {
    return ob_magicgetterfun.load();
  };
  rps_applyingfun_t*applying_function(void) const
  {
    return ob_applyingfun.load();
  };
  void put_applying_function(rps_applyingfun_t*afun);
  void touch_now(void)
  {
    ob_mtime.store(rps_wallclock_real_time());
  };
  std::string string_oid(void) const;
  inline Rps_Payload*get_payload(void) const;
  const std::string payload_type_name(void) const;
  inline Rps_PayloadClassInfo*get_classinfo_payload(void) const;
  template <class PaylClass> PaylClass* get_dynamic_payload(void) const
  {
    auto payl = get_payload();
    if (!payl)
      return nullptr;
    return dynamic_cast<PaylClass*>(payl);
  }
  inline bool has_erasable_payload(void) const;
  inline Rps_ObjectRef get_class(void) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  inline Rps_ObjectRef get_space(void) const;
  void put_space(Rps_ObjectRef obspace);
  //////////////// attributes
  Rps_Value set_of_attributes(Rps_CallFrame*stkf) const;
  Rps_Value set_of_physical_attributes() const;
  unsigned nb_physical_attributes() const;
  unsigned nb_attributes(Rps_CallFrame*stkf) const;
  Rps_Value get_physical_attr(const Rps_ObjectRef obattr0) const;
  Rps_Value get_attr1(Rps_CallFrame*stkf,const Rps_ObjectRef obattr0) const;
  Rps_TwoValues get_attr2(Rps_CallFrame*stkf,const Rps_ObjectRef obattr0, const Rps_ObjectRef obattr1) const;
  // if obaattr is a magic attribute, throw an exception
  void remove_attr(const Rps_ObjectRef obattr);
  // put one, two, three, four attributes in the same object locking
  void put_attr(const Rps_ObjectRef obattr, const Rps_Value valattr);
  void put_attr2( const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                  const Rps_ObjectRef obattr1, const Rps_Value valattr1);
  void put_attr3(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                 const Rps_ObjectRef obattr1, const Rps_Value valattr1,
                 const Rps_ObjectRef obattr2, const Rps_Value valattr2);
  void put_attr4(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                 const Rps_ObjectRef obattr1, const Rps_Value valattr1,
                 const Rps_ObjectRef obattr2, const Rps_Value valattr2,
                 const Rps_ObjectRef obattr3, const Rps_Value valattr3);
  // exchange one, two, three, four attributes in the same object locking with their old attribute value
  void exchange_attr(const Rps_ObjectRef obattr, const Rps_Value valattr, Rps_Value*poldval);
  void exchange_attr2(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                      const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1);
  void exchange_attr3(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                      const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1,
                      const Rps_ObjectRef obattr2, const Rps_Value valattr2, Rps_Value*poldval2);
  void exchange_attr4(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                      const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1,
                      const Rps_ObjectRef obattr2, const Rps_Value valattr2, Rps_Value*poldval2,
                      const Rps_ObjectRef obattr3, const Rps_Value valattr3, Rps_Value*poldval3);
  // put attributes
  void put_attributes(const std::map<Rps_ObjectRef, Rps_Value>& newattrmap);
  void put_attributes(const std::initializer_list<std::pair<Rps_ObjectRef, Rps_Value>>& attril);
  void put_attributes(const std::vector<std::pair<Rps_ObjectRef, Rps_Value>>&attrvec);
  /////////////////////////////////// components
  // append one, two, three, four, more components in the same object locking
  void append_comp1(Rps_Value comp0);
  void append_comp2(Rps_Value comp0, Rps_Value comp1);
  void append_comp3(Rps_Value comp0, Rps_Value comp1, Rps_Value comp2);
  void append_comp4(Rps_Value comp0, Rps_Value comp1, Rps_Value comp2, Rps_Value comp4);
  void append_components(const std::initializer_list<Rps_Value>&compil);
  void append_components(const std::vector<Rps_Value>&compvec);
  unsigned nb_components(Rps_CallFrame*stkf) const;
  unsigned nb_physical_components (void) const;
  const std::vector<Rps_Value> vector_physical_components(void) const;
  Rps_Value component_at (Rps_CallFrame*stkf, int rk, bool dontfail=false) const;
  Rps_Value replace_component_at ([[maybe_unused]] Rps_CallFrame*stkf, int rk,  Rps_Value comp0, bool dontfail=false);
  Rps_Value instance_from_components(Rps_CallFrame*stkf, Rps_ObjectRef obinstclass) const;
  // get atomic fields
  inline double get_mtime(void) const;
  inline rps_applyingfun_t*get_applyingfun(const Rps_ClosureValue&) const
  {
    return ob_applyingfun.load();
  };
  inline rps_applyingfun_t* get_applying_ptrfun() const
  {
    return ob_applyingfun.load();
  };
  inline void clear_payload(void);
  template<class PaylClass>
  PaylClass* put_new_plain_payload(void)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl = Rps_QuasiZone::rps_allocate1<PaylClass>(this);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_plain_payload
  template<class PaylClass, typename Arg1Class>
  PaylClass* put_new_arg1_payload(Arg1Class arg1)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate2<PaylClass,Arg1Class>(this,arg1);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_arg1_payload
  template<class PaylClass, typename Arg1Class, typename Arg2Class>
  PaylClass* put_new_arg2_payload(Arg1Class arg1, Arg2Class arg2)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate3<PaylClass,Arg1Class,Arg2Class>(this,arg1,arg2);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_arg2_payload
  template<class PaylClass, typename Arg1Class, typename Arg2Class, typename Arg3Class>
  PaylClass* put_new_arg3_payload(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate4<PaylClass,Arg1Class,Arg2Class,Arg3Class>
      (this,arg1,arg2,arg3);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_arg3_payload
  template<class PaylClass, typename Arg1Class, typename Arg2Class, typename Arg3Class, typename Arg4Class>
  PaylClass* put_new_arg4_payload(Arg1Class arg1, Arg2Class arg2, Arg3Class arg3, Arg4Class arg4)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate5<PaylClass,Arg1Class,Arg2Class,Arg3Class,Arg4Class>(this,arg1,arg2,arg3,arg4);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_arg4_payload
  template<class PaylClass>
  PaylClass* put_new_plain_payload_with_wordgap(unsigned wordgap)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate_with_wordgap<PaylClass>(wordgap,this);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_plain_payload_with_wordgap
  template<class PaylClass, typename Arg1Class>
  PaylClass* put_new_arg1_payload_with_wordgap(unsigned wordgap, Arg1Class arg1)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate_with_wordgap<PaylClass,Arg1Class>(wordgap,this,arg1);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_arg1_payload_with_wordgap
  template<class PaylClass, typename Arg1Class, typename Arg2Class>
  PaylClass* put_new_arg2_payload_with_wordgap(unsigned wordgap, Arg1Class arg1, Arg2Class arg2)
  {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    PaylClass*newpayl =
      Rps_QuasiZone::rps_allocate_with_wordgap<PaylClass,Arg1Class,Arg2Class>(wordgap,this,arg1,arg2);
    Rps_Payload*oldpayl = ob_payload.exchange(newpayl);
    if (oldpayl)
      delete oldpayl;
    return newpayl;
  };                            // end put_new_arg2_payload_with_wordgap
  virtual uint32_t wordsize() const
  {
    return sizeof(Rps_ObjectZone)/sizeof(void*);
  };
  static Rps_ObjectZone*make(void);
  static Rps_ObjectZone*make_or_find(Rps_Id);
  static Rps_ObjectZone*make_new(Rps_Id);
  static Rps_ObjectZone*find(Rps_Id);
  static Rps_ObjectZone*make_loaded(Rps_Id, Rps_Loader*);
  const Rps_Id oid() const
  {
    return ob_oid;
  };
  Rps_HashInt obhash() const
  {
    return ob_oid.hash();
  };
  // test if object is a RefPerSys class:
  inline bool is_class(void) const;
  // test if this object is instance of obclass:
  inline bool is_instance_of(Rps_ObjectRef obclass) const;
  // test if this object is a suclass of given obsuperclass:
  inline bool is_subclass_of(Rps_ObjectRef obsuperclass) const;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0) const;
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  void dump_scan_contents(Rps_Dumper*du) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  void dump_json_content(Rps_Dumper*du, Json::Value&)  const;
  virtual Rps_HashInt val_hash (void) const
  {
    return obhash();
  };
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual bool equal(const Rps_ZoneValue&zv) const;
  virtual bool less(const Rps_ZoneValue&zv) const;
  virtual void mark_gc_inside(Rps_GarbageCollector&gc);
  // given a C string which looks like an oid prefix, so starts with
  // an underscore, a digit, and two alphanums, autocomplete that and
  // call a given C++ closure on every possible object ref, till that
  // closure returns true. Return the number of matches, or else 0
  static int autocomplete_oid(const char*prefix, const std::function<bool(const Rps_ObjectZone*)>&stopfun);
};                              // end class Rps_ObjectZone

//////////////////////////////////////////////////////////// object payloads

//// signature of extern "C" functions for payload loading; their name starts with rpsldpy_
typedef void rpsldpysig_t(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& hjv, Rps_Id spacid, unsigned lineno);
//// this signature is also used for "loadrout" JSON members....
//// see Rps_Loader::parse_json_buffer_second_pass near load_rps.cc:750
//// after commit b384a473798 (mid-december 2024)
#define RPS_PAYLOADING_PREFIX "rpsldpy_"



///////////////////////////////////////////// payload superclass
class Rps_Payload : public Rps_QuasiZone
{
  friend class Rps_ObjectZone;
  Rps_ObjectZone* payl_owner;
protected:
  inline Rps_Payload(Rps_Type, Rps_ObjectZone*);
  inline Rps_Payload(Rps_Type, Rps_ObjectRef);
  virtual ~Rps_Payload()
  {
    payl_owner = nullptr;
  };
  void clear_owner(void)
  {
    payl_owner = nullptr;
  };
public:
  Rps_Payload(Rps_Type ty, Rps_ObjectZone*obz, Rps_Loader*ld)
    : Rps_Payload(ty,obz)
  {
    RPS_ASSERT(ld != nullptr);
  };
  virtual const std::string payload_type_name(void) const =0;
  /// a payload is erasable if it can be removed, e.g. replaced by another one.
  virtual bool is_erasable(void) const
  {
    return true;
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const =0;
  virtual void dump_scan(Rps_Dumper*du) const =0;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const =0;
  Rps_ObjectZone* owner() const
  {
    return payl_owner;
  };
  virtual void output_payload([[maybe_unused]] std::ostream&out, [[maybe_unused]] unsigned depth, [[maybe_unused]] unsigned maxdepth) const
  {
    RPS_ASSERT(depth <= maxdepth);
  };
};                              // end Rps_Payload







/////////////////////////// sequences (tuples or sets) of Rps_ObjectRef
class Rps_SetOb;
class Rps_TupleOb;
template<typename RpsSeq, Rps_Type seqty, unsigned k1, unsigned k2, unsigned k3>
class Rps_SeqObjRef : public Rps_LazyHashedZoneValue
{
  friend class Rps_SetOb;
  friend class Rps_TupleOb;
  friend RpsSeq*
  Rps_QuasiZone::rps_allocate_with_wordgap<RpsSeq,unsigned>(unsigned,unsigned);
  const unsigned _seqlen;
  Rps_ObjectRef _seqob[RPS_FLEXIBLE_DIM+1];
  Rps_SeqObjRef(unsigned len) : Rps_LazyHashedZoneValue(seqty), _seqlen(len)
  {
    memset ((void*)_seqob, 0, sizeof(Rps_ObjectRef)*len);
  };
  Rps_ObjectRef*raw_data()
  {
    return _seqob;
  };
  const Rps_ObjectRef*raw_const_data() const
  {
    return _seqob;
  };
public:
  static unsigned constexpr maxsize
    = std::numeric_limits<unsigned>::max() / 2;
  unsigned cnt() const
  {
    return _seqlen;
  };
  const Rps_ObjectRef at(int ix) const
  {
    if (ix<0)
      ix += cnt();
    if (ix>=0 && ix <(int)cnt())
      return _seqob[ix];
    else
      throw std::range_error("index out of range in objref sequence");
  }
  const Rps_ObjectRef random_object_in_sequence_or_fail (int startix= 0, int endix= -1) const
  {
    if (startix < 0)
      startix += cnt();
    if (endix < 0)
      endix += cnt();
    if (endix < 0)
      endix += cnt();
    if (startix < 0 || startix >= (int)cnt()-1)
      throw std::range_error("start index out of range");
    if (endix < 0 || endix >= (int)cnt()-1)
      throw std::range_error("end index out of range");
    if (startix > endix)
      throw std::invalid_argument("start index after end index");
    if (startix == endix)
      return _seqob[startix];
    RPS_ASSERT(startix < endix);
    unsigned rd = Rps_Random::random_32u() % (endix - startix);
    return _seqob[startix + rd];
  };
  const Rps_ObjectRef random_object_in_sequence_or_default (Rps_ObjectRef defob, int startix= 0, int endix= -1) const
  {
    if (startix < 0)
      startix += cnt();
    if (endix < 0)
      endix += cnt();
    if (endix < 0)
      endix += cnt();
    if (startix < 0 || startix >= (int)cnt()-1)
      return defob;
    if (endix < 0 || endix >= (int)cnt()-1)
      return defob;
    if (startix > endix)
      return defob;
    if (startix == endix)
      return _seqob[startix];
    RPS_ASSERT(startix < endix);
    unsigned rd = Rps_Random::random_32u() % (endix - startix);
    return _seqob[startix + rd];
  };
  typedef const Rps_ObjectRef*iterator_t;
  iterator_t begin() const
  {
    return const_cast<const Rps_ObjectRef*>(_seqob);
  };
  iterator_t end() const
  {
    return const_cast<const Rps_ObjectRef*>(_seqob) + _seqlen;
  };
  /// repeatedly iterate on the sequence object, till given function
  /// FUN returns true...
  template <typename Datatype_t>
  void
  iterate_data(const std::function<bool(const Rps_ObjectRef,
                                        Datatype_t data)>& fun,
               Datatype_t data)
  {
    for (auto it: *this)
      {
        Rps_ObjectRef curob = it;
        if (fun(curob, data))
          return;
      }
  } // end iterate_data
  /// repeatedly iterate on the sequence object in reverse order, till
  /// given function FUN returns true...
  template <typename Datatype_t>
  void
  reverse_iterate_data(const std::function<bool(const Rps_ObjectRef,
                       Datatype_t data)>& fun,
                       Datatype_t data)
  {
    const unsigned len = cnt();
    for (int ix = (int)len-1; ix>=0; ix--)
      {
        Rps_ObjectRef curob = _seqob[ix];
        if (fun(curob, data))
          return;
      }
  } // end reverse_iterate_data
  //////////
  //// iteratively apply a given closure CLOSV in ascending order to
  //// objects OB, until application returns {nil,nil}
  inline void iterate_apply0(Rps_CallFrame*stkf, const Rps_Value closv);
  //// iteratively apply a given closure in descending order to
  //// objects OB, until application returns {nil,nil}
  inline void reverse_iterate_apply0(Rps_CallFrame*stkf, const Rps_Value closv);
  //// iteratively apply a given closure CLOSV in ascending order to
  //// objects OB, and value ARG0, until application of
  //// CLOSV(OB,ARG0) returns {nil,nil}
  inline void iterate_apply1(Rps_CallFrame*stkf, const Rps_Value closv, const Rps_Value arg0v);
  //// iteratively apply a given closure CLOSV in descending order to
  //// objects OB, and value ARG0, until application of CLOSV(OB,ARG0)
  //// returns {nil,nil}
  inline void reverse_iterate_apply1(Rps_CallFrame*stkf, const Rps_Value closv, const Rps_Value arg0v);
  ///////
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this) + _seqlen * sizeof(_seqob[0])) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned) const
  {
    for (auto ob: *this)
      if (ob)
        gc.mark_obj(ob);
  };
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    Rps_HashInt h0= 3317+(k3&0xff), h1= 31*_seqlen;
    for (unsigned ix=0; ix<_seqlen; ix += 2)
      {
        if (RPS_UNLIKELY(_seqob[ix].is_empty()))
          throw std::runtime_error("corrupted sequence of objects");
        h0 = (h0 * k1) ^ (_seqob[ix]->obhash() * k2 + ix);
        if (ix+1 >= _seqlen) break;
        auto nextob = _seqob[ix+1];
        if (RPS_UNLIKELY(nextob.is_empty())) break;
        h1 = (h1 * k2) ^ (nextob->obhash() * k3 - (h0&0xfff));
      };
    Rps_HashInt h = 5*h0 + 11*h1;
    if (RPS_UNLIKELY(h == 0))
      h = ((h0 & 0xfffff) ^ (h1 & 0xfffff)) + (k1/128 + (_seqlen & 0xff) + 3);
    RPS_ASSERT(h != 0);
    return h;
  };
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == seqty)
      {
        auto oth = reinterpret_cast<const RpsSeq*>(&zv);
        if (RPS_LIKELY(reinterpret_cast<const Rps_LazyHashedZoneValue*>(this)->val_hash()
                       != reinterpret_cast<const Rps_LazyHashedZoneValue*>(oth)->val_hash()))
          return false;
        auto curcnt = cnt();
        if (RPS_LIKELY(oth->cnt() != curcnt))
          return false;
        for (unsigned ix = 0; ix < curcnt; ix++)
          if (_seqob[ix] != oth->_seqob[ix])
            return false;
        return true;
      }
    return false;
  }
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == seqty)
      {
        auto oth = reinterpret_cast<const RpsSeq*>(&zv);
        return std::lexicographical_compare(begin(), end(),
                                            oth->begin(), oth->end());
      }
    return false;
  };
};    // end of Rps_SeqObjRef


/////////////////////////// sets of Rps_ObjectRef
unsigned constexpr rps_set_k1 = 7933;
unsigned constexpr rps_set_k2 = 8963;
unsigned constexpr rps_set_k3 = 19073;
class Rps_SetOb: public Rps_SeqObjRef<Rps_SetOb, Rps_Type::Set, rps_set_k1, rps_set_k2, rps_set_k3>
{
  static Rps_SetOb _setob_emptyset_;
public:
  friend class Rps_SeqObjRef<Rps_SetOb, Rps_Type::Set, rps_set_k1, rps_set_k2, rps_set_k3>;
  typedef Rps_SeqObjRef<Rps_SetOb, Rps_Type::Set, rps_set_k1, rps_set_k2, rps_set_k3> parentseq_t;
  Rps_SetOb(unsigned len, Rps_SetTag) :parentseq_t (len) {};
  Rps_SetOb(const std::set<Rps_ObjectRef>& setob, Rps_SetTag);
protected:
  friend Rps_SetOb*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_SetOb,unsigned,Rps_SetTag>(unsigned,unsigned,Rps_SetTag);
public:
  // make a set from given object references
  static const Rps_SetOb*make(const std::set<Rps_ObjectRef>& setob);
  static const Rps_SetOb*make(const std::vector<Rps_ObjectRef>& vecob);
  static const Rps_SetOb*make(const std::initializer_list<Rps_ObjectRef>&elemil);
  // collect a set from several objects, tuples, or sets
  static const Rps_SetOb*collect(const std::vector<Rps_Value>& vecval);
  static const Rps_SetOb*collect(const std::initializer_list<Rps_Value>&valil);
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /// gives the element index, or a negative number if not found
  inline int element_index(const Rps_ObjectRef obelem) const;
  inline bool contains(const Rps_ObjectRef obelem) const
  {
    return element_index(obelem) >= 0;
  };
  unsigned cardinal() const
  {
    return cnt();
  };
  static const Rps_SetOb& the_empty_set(void)
  {
    return _setob_emptyset_;
  };
  inline Rps_ObjectRef element_after_or_equal(const Rps_ObjectRef obelem) const;
  inline Rps_ObjectRef element_after(const Rps_ObjectRef obelem) const;
  inline Rps_ObjectRef element_before_or_equal(const Rps_ObjectRef obelem) const;
  inline Rps_ObjectRef element_before(const Rps_ObjectRef obelem) const;
  inline Rps_ObjectRef minimal_element(void) const;
  inline Rps_ObjectRef maximal_element(void) const;
  inline Rps_ObjectRef random_element_or_fail(int startix=0, int endix= -1) const;
  inline Rps_ObjectRef random_element_or_default(Rps_ObjectRef defob, int startix=0, int endix= -1) const;
  // repeat the given func on each element, in increasing order, till the func returns false
  void repeat_increasing_each_element_until(Rps_CallFrame*cf, void*data,
      const std::function<bool(Rps_CallFrame*,void*/*data*/,Rps_ObjectRef/*elem*/)>& func) const;
  // repeat the given func on each element, in decreasing order, till the func returns false
  void repeat_decreasing_each_element_until(Rps_CallFrame*cf, void*data,
      const std::function<bool(Rps_CallFrame*,void*/*data*/,Rps_ObjectRef/*elem*/)>& func) const;
};// end of Rps_SetOb



/////////////////////////// tuples of Rps_ObjectRef
unsigned constexpr rps_tuple_k1 = 5939;
unsigned constexpr rps_tuple_k2 = 18917;
unsigned constexpr rps_tuple_k3 = 6571;
class Rps_TupleOb: public Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3>
{
public:
  friend class  Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3>;
  typedef Rps_SeqObjRef<Rps_TupleOb, Rps_Type::Tuple, rps_tuple_k1, rps_tuple_k2, rps_tuple_k3> parentseq_t;
protected:
  Rps_TupleOb(unsigned len, Rps_TupleTag)
    : parentseq_t (len) {};
  friend Rps_TupleOb*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_TupleOb,unsigned,Rps_TupleTag>(unsigned,unsigned,Rps_TupleTag);
public:
  // make a tuple from given object references
  static const Rps_TupleOb*make(const std::vector<Rps_ObjectRef>& vecob);
  static const Rps_TupleOb*make(const std::initializer_list<Rps_ObjectRef>&compil);
  // collect a tuple from several objects, tuples, or sets
  static const Rps_TupleOb*collect(const std::vector<Rps_Value>& vecval);
  static const Rps_TupleOb*collect(const std::initializer_list<Rps_Value>&valil);
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /* find then compute the index of the first component equal to a
     given object, not smaller than a starting index, or -1; if startix
     is negative count from last component... */
  inline int index_found_after(Rps_ObjectRef findob, int startix=0);
  /* symetrically find the index before an ending index.. */
  inline int index_found_before(Rps_ObjectRef findob, int endix= -1);
  inline Rps_ObjectRef random_component_or_fail(int startix=0, int endix= -1) const;
  inline Rps_ObjectRef random_component_or_default(Rps_ObjectRef defob, int startix=0, int endix= -1) const;
  inline unsigned size() const
  {
    return cnt();
  };
#pragma message "perhaps Rps_TupleOb is incomplete"
  /// maybe we might need specialized and faster
  /// make1(Rps_ObjectRef) make2(Rps_ObjectRef,Rps_ObjectRef)
  /// and make3(Rps_ObjectRef,Rps_ObjectRef,Rps_ObjectRef) etc etc
};// end of Rps_TupleOb


////////////////////////////////////////////////////////////////

/////////////////////////// tree zones, with connective and sons
class Rps_ClosureZone; // closures to be applied
class Rps_InstanceZone; // immutable instances

template<typename RpsTree, Rps_Type treety, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
class Rps_TreeZone : public Rps_LazyHashedZoneValue
{
  friend class Rps_ClosureZone;
  friend class Rps_InstanceZone;
  friend RpsTree*
  Rps_QuasiZone::rps_allocate_with_wordgap<RpsTree,unsigned>(unsigned,unsigned);
  const unsigned _treelen;
  mutable std::atomic<bool> _treetransient;
  mutable std::atomic<bool> _treemetatransient;
  mutable std::atomic<int32_t> _treemetarank;
  mutable std::atomic<Rps_ObjectZone*> _treemetaob;
  Rps_ObjectRef _treeconnob;
  Rps_Value _treesons[RPS_FLEXIBLE_DIM+1];
  Rps_TreeZone(unsigned len, Rps_ObjectRef obr=nullptr)
    : Rps_LazyHashedZoneValue(treety), _treelen(len),
      _treetransient(false), _treemetatransient(false),
      _treemetarank(0), _treemetaob(nullptr),
      _treeconnob(obr)
  {
    memset ((void*)_treesons, 0, sizeof(Rps_Value)*len);
  };
  Rps_Value*raw_data_sons()
  {
    return _treesons;
  };
  const Rps_Value*raw_const_data_sons() const
  {
    return _treesons;
  };
public:
  static unsigned constexpr maxsize
    = std::numeric_limits<unsigned>::max() / 2;
  unsigned cnt() const
  {
    return _treelen;
  };
  Rps_ObjectRef conn() const
  {
    return _treeconnob;
  }
  bool is_transient(void) const
  {
    return _treetransient.load();
  };
  bool is_metatransient(void) const
  {
    return _treemetatransient.load();
  };
  int32_t metarank(void) const
  {
    return _treemetarank.load();
  };
  Rps_ObjectZone* metaobject(void) const
  {
    return _treemetaob.load();
  };
  std::pair<Rps_ObjectZone*,int32_t> const get_metadata(void) const;
  std::pair<Rps_ObjectZone*,int32_t> swap_metadata(Rps_ObjectRef obr, int32_t num=0, bool transient=false);
  void put_metadata(Rps_ObjectRef obr, int32_t num=0, bool transient=false);
  void put_transient_metadata(Rps_ObjectRef obr, int32_t num=0)
  {
    put_metadata(obr, num, true);
  };
  void put_persistent_metadata(Rps_ObjectRef obr, int32_t num=0)
  {
    put_metadata(obr, num, false);
  };
  typedef const Rps_Value*iterator_t;
  iterator_t begin() const
  {
    return const_cast<const Rps_Value*>(_treesons);
  };
  iterator_t end() const
  {
    return const_cast<const Rps_Value*>(_treesons) + _treelen;
  };
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this) + _treelen * sizeof(_treesons[0])) / sizeof(void*);
  };
  inline const Rps_Value at(int rk, bool dontfail=false) const;
  virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth) const
  {
    gc.mark_obj(_treeconnob);
    for (auto vson: *this)
      if (vson)
        gc.mark_value(vson, depth+1);
  };
protected:
  virtual Rps_HashInt compute_hash(void) const
  {
    Rps_HashInt h0= 3317+(k1&0xff)+_treeconnob->obhash()*k3, h1= 211*_treelen;
    for (unsigned ix=0; ix<_treelen; ix += 2)
      {
        auto curson = _treesons[ix];
        if (RPS_LIKELY(!curson.is_empty()))
          h0 = (h0 * k1) ^ (curson.valhash() * k2 + ix);
        if (ix+1 >= _treelen)
          break;
        auto nextson = _treesons[ix+1];
        if (RPS_LIKELY(!nextson.is_empty()))
          h1 = (h1 * k3) ^ (nextson.valhash() * k4 - (h0&0xfff));
      };
    Rps_HashInt h = 53*h0 + 17*h1;
    if (RPS_UNLIKELY(h == 0))
      h = ((h0 & 0xfffff)
           ^ (h1 & 0xfffff)) + (k3/128
                                + (_treeconnob->obhash()%65353) + (_treelen & 0xff) + 13);
    RPS_ASSERT(h != 0);
    return h;
  };
  virtual bool equal(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == treety)
      {
        auto oth = reinterpret_cast<const RpsTree*>(&zv);
        if (RPS_LIKELY(reinterpret_cast<const Rps_LazyHashedZoneValue*>(this)->val_hash()
                       != reinterpret_cast<const Rps_LazyHashedZoneValue*>(oth)->val_hash()))
          return false;
        auto curcnt = cnt();
        if (RPS_LIKELY(oth->cnt() != curcnt))
          return false;
        if (RPS_LIKELY(_treeconnob != oth->_treeconnob))
          return false;
        for (unsigned ix = 0; ix < curcnt; ix++)
          if (_treesons[ix] != oth->_treesons[ix])
            return false;
        return true;
      }
    return false;
  }
  virtual bool less(const Rps_ZoneValue&zv) const
  {
    if (zv.stored_type() == treety)
      {
        auto oth = reinterpret_cast<const RpsTree*>(&zv);
        if (_treeconnob < oth->_treeconnob)
          return true;
        if (_treeconnob > oth->_treeconnob)
          return false;
        RPS_ASSERT(_treeconnob == oth->_treeconnob);
        return std::lexicographical_compare(begin(), end(),
                                            oth->begin(), oth->end());
      }
    return false;
  };
};    // end of template Rps_TreeZone


//////////////////////////////////////////////// closures

unsigned constexpr rps_closure_k1 = 8161;
unsigned constexpr rps_closure_k2 = 9151;
unsigned constexpr rps_closure_k3 = 10151;
unsigned constexpr rps_closure_k4 = 13171;
struct Rps_ClosureTag {};
class Rps_ClosureZone :
  public Rps_TreeZone<Rps_ClosureZone, Rps_Type::Closure,
  rps_closure_k1, rps_closure_k2, rps_closure_k3, rps_closure_k4>
{
public:
  typedef  Rps_TreeZone<Rps_ClosureZone, Rps_Type::Closure,
           rps_closure_k1, rps_closure_k2, rps_closure_k3, rps_closure_k4> parenttree_t;
  friend parenttree_t;
protected:
  Rps_ClosureZone(unsigned len, Rps_ObjectRef connob, Rps_ClosureTag) :
    parenttree_t(len, connob)
  {
  };
  friend Rps_ClosureZone*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_ClosureZone,unsigned,Rps_ObjectRef,Rps_ClosureTag>(unsigned,unsigned,Rps_ObjectRef,Rps_ClosureTag);
public:
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /// make a closure with given connective and values
  static Rps_ClosureZone* make(Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil);
  static Rps_ClosureZone* make(Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec);
};    // end Rps_ClosureZone

class Rps_ClosureValue : public Rps_Value
{
public:
  inline Rps_ClosureValue() : Rps_Value() {};
  inline Rps_ClosureValue(std::nullptr_t) : Rps_Value(nullptr) {};
  // related to Rps_ClosureZone::make
  inline Rps_ClosureValue(const Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil);
  inline Rps_ClosureValue(const Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec);
  // "dynamic" casting
  inline Rps_ClosureValue(Rps_Value val);
// get the connective
  inline Rps_ObjectRef connob(void) const;
// clear the closure
  inline Rps_ClosureValue& operator = (std::nullptr_t)
  {
    clear();
    return *this;
  };
  inline  Rps_ClosureZone*operator -> (void) const;
  // application
  inline Rps_TwoValues apply0(Rps_CallFrame*callerframe) const;
  inline Rps_TwoValues apply1(Rps_CallFrame*callerframe, const Rps_Value arg0) const;
  inline Rps_TwoValues apply2(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1) const;
  inline Rps_TwoValues apply3(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2) const;
  inline Rps_TwoValues apply4(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3) const;
  inline Rps_TwoValues apply5(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4) const;
  inline Rps_TwoValues apply6(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5) const;
  inline Rps_TwoValues apply7(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5, const Rps_Value arg6) const;
  inline Rps_TwoValues apply8(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5, const Rps_Value arg6,
                              const Rps_Value arg7) const;
  inline Rps_TwoValues apply9(Rps_CallFrame*callerframe, const Rps_Value arg0,
                              const Rps_Value arg1, const Rps_Value arg2,
                              const Rps_Value arg3, const Rps_Value arg4,
                              const Rps_Value arg5, const Rps_Value arg6,
                              const Rps_Value arg7, const Rps_Value arg8) const;
  inline Rps_TwoValues apply10(Rps_CallFrame*callerframe, const Rps_Value arg0,
                               const Rps_Value arg1, const Rps_Value arg2,
                               const Rps_Value arg3, const Rps_Value arg4,
                               const Rps_Value arg5, const Rps_Value arg6,
                               const Rps_Value arg7, const Rps_Value arg8, const Rps_Value arg9) const;
  Rps_TwoValues apply_vect(Rps_CallFrame*callerframe, const std::vector<Rps_Value>& argvec) const;
  Rps_TwoValues apply_ilist(Rps_CallFrame*callerframe, const std::initializer_list<Rps_Value>& argil) const;
};    // end Rps_ClosureValue



//////////////////////////////////////////////// instances

unsigned constexpr rps_instance_k1 = 8161;
unsigned constexpr rps_instance_k2 = 9151;
unsigned constexpr rps_instance_k3 = 10151;
unsigned constexpr rps_instance_k4 = 13171;
struct Rps_InstanceTag {};
class Rps_InstanceZone :
  public Rps_TreeZone<Rps_InstanceZone, Rps_Type::Instance,
  rps_instance_k1, rps_instance_k2, rps_instance_k3, rps_instance_k4>
{
public:
  typedef  Rps_TreeZone<Rps_InstanceZone, Rps_Type::Instance,
           rps_instance_k1, rps_instance_k2, rps_instance_k3, rps_instance_k4> parenttree_t;
  friend parenttree_t;
protected:
  Rps_InstanceZone(unsigned len, Rps_ObjectRef connob, Rps_InstanceTag) :
    parenttree_t(len, connob)
  {
  };
  friend Rps_InstanceZone*
  Rps_QuasiZone::rps_allocate_with_wordgap<Rps_InstanceZone,unsigned,Rps_ObjectRef,Rps_InstanceTag>(unsigned,unsigned,Rps_ObjectRef,Rps_InstanceTag);
public:
  const Rps_Value* const_sons() const
  {
    return raw_const_data_sons();
  };
  virtual void dump_scan(Rps_Dumper*du, unsigned depth=0) const;
  virtual Json::Value dump_json(Rps_Dumper*) const;
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  Rps_ObjectRef get_class(void) const
  {
    return conn();
  };
  const Rps_SetOb* set_attributes(void) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  /// get the set of attributes in a class
  static const Rps_SetOb* class_attrset(Rps_ObjectRef obclass);
  /// make a instance with given class and components and no attributes
  static Rps_InstanceZone* load_from_json(Rps_Loader*ld, const Json::Value& jv);
  /// later fill such an empty instance
  void fill_loaded_instance_from_json(Rps_Loader*ld, Rps_ObjectRef obclass, const Json::Value& jv);
  // when loading, we could be able to allocate, but not to fill the
  // instance now; this happens when the class is not filled
  static Rps_InstanceZone* make_incomplete_loaded(Rps_Loader*ld, Rps_ObjectRef classob, unsigned siz);
  static Rps_InstanceZone* make_from_components(Rps_ObjectRef classob, const std::initializer_list<Rps_Value>& valil);
  static Rps_InstanceZone* make_from_components(Rps_ObjectRef classob, const std::vector<Rps_Value>& valvec);
  /// make an instance from both attributes and components
  static Rps_InstanceZone* make_from_attributes_components(Rps_ObjectRef classob,
      const std::initializer_list<Rps_Value>& valil,const std::initializer_list<std::pair<Rps_ObjectRef,Rps_Value>>&attril);
  static Rps_InstanceZone* make_from_attributes_components(Rps_ObjectRef classob,
      const std::vector<Rps_Value>& valvec,
      const std::map<Rps_ObjectRef,Rps_Value>& attrmap);
  /// make an instance from attributes
  static Rps_InstanceZone* make_from_attributes(Rps_ObjectRef classob,
      const std::initializer_list<std::pair<Rps_ObjectRef,Rps_Value>>&attril);
  static Rps_InstanceZone* make_from_attributes(Rps_ObjectRef classob,
      const std::map<Rps_ObjectRef,Rps_Value>& attrmap);
};    // end class Rps_InstanceZone


class Rps_InstanceValue : public Rps_Value
{
public:
  inline Rps_InstanceValue() : Rps_Value() {};
  inline Rps_InstanceValue(std::nullptr_t) : Rps_Value(nullptr) {};
  // related to Rps_InstanceZone::make
  inline Rps_InstanceValue(const Rps_ObjectRef connob, const std::initializer_list<Rps_Value>& valil);
  inline Rps_InstanceValue(const Rps_ObjectRef connob, const std::vector<Rps_Value>& valvec);
  // "dynamic" casting
  inline Rps_InstanceValue(const Rps_Value val);
// get the class (stored in connective)
  inline Rps_ObjectRef get_class(void) const;
// clear the instance
  inline Rps_InstanceValue& operator = (std::nullptr_t)
  {
    clear();
    return *this;
  };
  inline  Rps_InstanceZone*operator -> (void) const;
}; // end class Rps_InstanceValue


//////////////////////////////////////////////// Json values

class Rps_JsonZone : public Rps_LazyHashedZoneValue
{
  friend Rps_JsonZone*
  Rps_QuasiZone::rps_allocate<Rps_JsonZone,const Json::Value&>(const Json::Value&);
  const Json::Value _jsonval;
protected:
  inline Rps_JsonZone(const Json::Value&jv);
  virtual Rps_HashInt compute_hash(void) const;
  virtual Rps_ObjectRef compute_class(Rps_CallFrame*stkf) const;
  virtual void gc_mark(Rps_GarbageCollector&, unsigned) const { };
  virtual void dump_scan(Rps_Dumper*, unsigned) const {};
  virtual Json::Value dump_json(Rps_Dumper*) const;
public:
  const Json::Value& json() const
  {
    return _jsonval;
  };
  const Json::Value const_json() const
  {
    return _jsonval;
  };
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void val_output(std::ostream& outs, unsigned depth, unsigned maxdepth) const;
  virtual bool equal(const Rps_ZoneValue&zv) const;
  virtual bool less(const Rps_ZoneValue&zv) const;
  static Rps_JsonZone* load_from_json(Rps_Loader*ld, const Json::Value& jv);
  static Rps_JsonZone*make(const Json::Value& jv);
};                              // end class Rps_JsonZone


////////////////////////////////////////////////////////////////


class Rps_ProtoCallFrame;
typedef Rps_ProtoCallFrame Rps_CallFrame;
typedef void Rps_CallFrameOutputSig_t(std::ostream&/*out*/, const Rps_ProtoCallFrame*/*frame*/,unsigned/*depth*/,unsigned /*maxdepth*/);
////////////////////////////////////////////////////////////////
//// the common superclass of our call frames
class Rps_ProtoCallFrame : public Rps_TypedZone
{
  friend unsigned rps_call_frame_depth(const Rps_CallFrame*);
protected:
  const unsigned cfram_size;
  Rps_ObjectRef cfram_descr;
  Rps_CallFrame* cfram_prev;
  Rps_Value cfram_state;
  intptr_t cfram_rankstate;
  Rps_ClosureValue cfram_clos; // the invoking closure, if any
  intptr_t* cfram_xtradata;
  std::atomic<Rps_CallFrameOutputSig_t*> cfram_outputter;
  std::function<void(Rps_GarbageCollector*)> cfram_marker;
public:
  static constexpr unsigned _cfram_max_size_ = 1024;
  static std::atomic<int> _cfram_output_depth_;
  Rps_ProtoCallFrame(unsigned size, void*xdata, Rps_ObjectRef obdescr=nullptr, Rps_CallFrame*prev=nullptr)
    : Rps_TypedZone(Rps_Type::CallFrame),
      cfram_size(size),
      cfram_descr(obdescr),
      cfram_prev(prev),
      cfram_state(nullptr),
      cfram_rankstate(0),
      cfram_clos(nullptr),
      cfram_xtradata((intptr_t*) xdata),
      cfram_outputter(nullptr),
      cfram_marker()
  {
    // ensure that if some size is given, the xdata is a suitably
    // aligned pointer...
    assert (size == 0
            || (xdata != nullptr
                && (((intptr_t)xdata & (alignof(intptr_t)-1)) == 0)));
    assert (size < _cfram_max_size_);
    rps_curthread_callframe = this;
  }; // end Rps_ProtoCallFrame constructor
  ~Rps_ProtoCallFrame()
  {
    if (cfram_size > 0)
      {
        assert (cfram_xtradata != nullptr);
        memset ((void*)cfram_xtradata, 0, cfram_size*sizeof(intptr_t));
      }
    rps_curthread_callframe = cfram_prev;
    cfram_xtradata = nullptr;
    cfram_descr = nullptr;
    cfram_prev = nullptr;
    cfram_state = nullptr;
    cfram_rankstate = 0;
    cfram_clos = nullptr;
  }; // end Rps_ProtoCallFrame destructor
  void set_outputter(std::nullptr_t)
  {
    cfram_outputter.store(nullptr);
  };
  void clear_outputter(void)
  {
    cfram_outputter.store(nullptr);
  };
  void set_outputter(Rps_CallFrameOutputSig_t*outputter=nullptr)
  {
    cfram_outputter.store(outputter);
  };
  void gc_mark_frame(Rps_GarbageCollector* gc);
  void set_closure(Rps_ClosureValue clos)
  {
    RPS_ASSERT(!clos.is_empty() && clos.is_closure());
    cfram_clos = clos;
  };
  void set_state_value(Rps_Value val)
  {
    cfram_state = val;
  };
  Rps_Value state_value() const
  {
    return cfram_state;
  };
  void set_state_rank(int rk)
  {
    cfram_rankstate = rk;
  };
  int state_rank()
  {
    return cfram_rankstate;
  };
  void clear_closure(void)
  {
    cfram_clos = nullptr;
  };
  Rps_CallFrame*previous_call_frame(void) const
  {
    return cfram_prev;
  };
  void set_additional_gc_marker(const std::function<void(Rps_GarbageCollector*)>& gcmarkfun)
  {
    cfram_marker = gcmarkfun;
  };
  void clear_additional_gc_marker(void)
  {
    cfram_marker = nullptr;
  };
  bool is_good_call_frame() const
  {
    assert (cfram_size <  _cfram_max_size_);
    return stored_type() == Rps_Type::CallFrame;
  };
  static bool is_good_call_frame(const Rps_CallFrame*cf)
  {
    return cf==nullptr||cf->is_good_call_frame();
  };
  Rps_ObjectRef call_frame_descriptor() const
  {
    return cfram_descr;
  };
  Rps_Value call_frame_state() const
  {
    return cfram_state;
  };
  Rps_ClosureValue call_frame_closure() const
  {
    return cfram_clos;
  };
  void output(std::ostream&out, unsigned depth=0, unsigned maxdepth=0) const;
  unsigned call_frame_depth(void) const
  {
    unsigned d=0;
    for (Rps_CallFrame const*curf = this; curf && is_good_call_frame(curf); curf=curf->cfram_prev) d++;
    return d;
  };
  /// TODO: implement fully in cmdrepl_rps.cc file some
  /// evaluate_repl_expr(Rps_Value expr,Rps_ObjectRef envob) member
  /// function here.  can throw some runtime exception on failure...
  Rps_TwoValues evaluate_repl_expr(Rps_Value expr,Rps_ObjectRef envob);
  Rps_Value evaluate_repl_expr1(Rps_Value expr,Rps_ObjectRef envob);
  //// TODO: interpret a REPL statement; may throw an exception
  void interpret_repl_statement(Rps_ObjectRef stmtob,Rps_ObjectRef envob);
  ///
  /// Where `expr` would be some expression to evaluate, and `envob`
  /// would be an object reifying the environment (variables and their
  /// associated values) See function rpsapply_7WsQyJK6lty02uz5KT for
  /// REPL command show
};                              // end class Rps_ProtoCallFrame

extern "C" Rps_TwoValues rps_full_evaluate_repl_expr(Rps_CallFrame*callframe,Rps_Value expr,Rps_ObjectRef envob);
extern "C" Rps_Value rps_simple_evaluate_repl_expr(Rps_CallFrame*callframe,Rps_Value expr,Rps_ObjectRef envob);
extern "C" void rps_interpret_repl_statement(Rps_CallFrame*callframe, Rps_ObjectRef stmtob,Rps_ObjectRef envob);


/// get the first REPL environment object (the environment attribute
/// of RefPerSys_system)
extern "C" Rps_ObjectRef rps_get_first_repl_environment(void);

class Rps_ShowCallFrame
{
  const Rps_ProtoCallFrame* _callframe;
public:
  Rps_ShowCallFrame(const Rps_ProtoCallFrame*cf = nullptr) : _callframe(cf) {};
  ~Rps_ShowCallFrame()
  {
    _callframe=nullptr;
  };
  void output(std::ostream&out) const
  {
    if (_callframe)
      _callframe->output(out);
    else
      out << "[*nullrpsframe*]" << std::flush;
  }
};                              // end class Rps_ShowCallFrame

inline std::ostream&
operator << (std::ostream&out, Rps_ProtoCallFrame*fr)
{
  if (fr)
    fr->output(out);
  else
    out << "[*nullrpsframe*]" << std::flush;
  return out;
} // end operator << (std::ostream, Rps_ProtoCallFrame*)

inline std::ostream&
operator << (std::ostream&out, Rps_ShowCallFrame scf)
{
  scf.output(out);
  return out;
};                              // end  operator << (std::ostream&, Rps_ShowCallFrame)

template <unsigned WordSize> class Rps_SizedCallFrame;
template <typename FrameFields> class Rps_FieldedCallFrame;

template <unsigned WordSize> class Rps_SizedCallFrame
  : public Rps_ProtoCallFrame
{
  void* cfram_word_data[WordSize];
public:
  typedef Rps_SizedCallFrame<WordSize> This_frame;
  Rps_SizedCallFrame (Rps_ObjectRef obdescr=nullptr, Rps_CallFrame*prev=nullptr)
    :  Rps_ProtoCallFrame(WordSize, cfram_word_data, obdescr, prev)
  {
  };
  ~Rps_SizedCallFrame()
  {
  };
};                              // end of Rps_SizedCallFrame template


template <typename FrameFields> class  Rps_FieldedCallFrame :
  public Rps_ProtoCallFrame
{
  FrameFields cfram_fields;
public:
  typedef  Rps_FieldedCallFrame<FrameFields> This_frame;
  FrameFields& fields()
  {
    return cfram_fields;
  };
  FrameFields* fieldsptr()
  {
    return &cfram_fields;
  };
  const FrameFields& constfields() const
  {
    return cfram_fields;
  };
  Rps_FieldedCallFrame (Rps_ObjectRef obdescr=nullptr, Rps_CallFrame*prev=nullptr)
    :  Rps_ProtoCallFrame(sizeof(FrameFields)/sizeof(void*), &cfram_fields, obdescr, prev)
  {
  };
  ~Rps_FieldedCallFrame ()
  {
  };
};                              // end of Rps_FieldedCallFrame template



#define RPS_LOCALFRAME_ATBIS(Lin,Descr,Prev,...)        \
  struct RpsFrameData##Lin {__VA_ARGS__; };             \
  typedef                                               \
  Rps_FieldedCallFrame<RpsFrameData##Lin>               \
   Rps_FldCallFrame##Lin;                               \
  class Rps_FrameAt##Lin :                              \
    public Rps_FldCallFrame##Lin {                      \
public:                                                 \
 Rps_FrameAt##Lin(Rps_ObjectRef obd##Lin,               \
                  Rps_CallFrame* prev##Lin) :           \
 Rps_FldCallFrame##Lin(obd##Lin, prev##Lin)             \
    { };                                                \
  };                                                    \
  Rps_FrameAt##Lin _((Descr),(Prev));                   \
  auto& _f = *_.fieldsptr();                            \
  /*end RPS_LOCALFRAME_ATBIS*/


/// coding convention in RPS_LOCALFRAME: if the Prev-ious frame is
/// null, use the RPS_NULL_CALL_FRAME macro... if the Descr-iptor is
/// null, use RPS_CALL_FRAME_UNDESCRIBED

#define RPS_LOCALFRAME_AT(Lin,Descr,Prev,...)       \
  RPS_LOCALFRAME_ATBIS(Lin,Descr,Prev,__VA_ARGS__)
#define RPS_LOCALFRAME(Descr,Prev,...)              \
  RPS_LOCALFRAME_AT(__LINE__,Descr,Prev,__VA_ARGS__)

#define RPS_LOCALRETURN(Val) do {                   \
  RPS_ASSERT(_.previous_call_frame() != nullptr);   \
  _.previous_call_frame()->clear_closure();         \
  return (Val); } while(0)

#define RPS_LOCALRETURNTWO(MainVal,XtraVal) do {    \
  RPS_ASSERT(_.previous_call_frame() != nullptr);   \
  _.previous_call_frame()->clear_closure();         \
  return Rps_TwoValues((MainVal),(XtraVal)); } while(0)





////////////////////////////////////////////////////////////////
////// class information payload - for PaylClassInfo, objects of class
////// `class` _41OFI3r0S1t03qdB2E

extern "C" rpsldpysig_t rpsldpy_classinfo;
class Rps_PayloadClassInfo : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend class Rps_Value;
  friend Rps_PayloadClassInfo*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadClassInfo,Rps_ObjectZone*>(Rps_ObjectZone*);
  // the superclass:
  Rps_ObjectRef pclass_super;
  // the dictionary from selector to own methods
  std::map<Rps_ObjectRef,Rps_ClosureValue> pclass_methdict;
  // the optional name (a symbol)
  Rps_ObjectRef pclass_symbname;
  // for immutable instances, the set of attributes; it should not be
  // nil for them.  See
  // https://gitlab.com/bstarynk/refpersys/-/wikis/Immutable-instances-in-RefPerSys
  mutable std::atomic<const Rps_SetOb*> pclass_attrset;
  virtual ~Rps_PayloadClassInfo()
  {
    pclass_super = nullptr;
    pclass_methdict.clear();
    pclass_symbname = nullptr;
    pclass_attrset.store(nullptr);
  };
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadClassInfo(Rps_ObjectZone*owner);
  Rps_PayloadClassInfo(Rps_ObjectRef obr) :
    Rps_PayloadClassInfo(obr?obr.optr():nullptr) {};
public:
  const Rps_SetOb* attributes_set() const
  {
    return pclass_attrset.load();
  };
  virtual const std::string payload_type_name(void) const
  {
    return "classinfo";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
  inline Rps_PayloadClassInfo(Rps_ObjectZone*owner, Rps_Loader*ld);
  Rps_ObjectRef superclass() const
  {
    return pclass_super;
  };
  Rps_ObjectRef symbname() const
  {
    return pclass_symbname;
  };
  void put_superclass(Rps_ObjectRef obr)
  {
    pclass_super = obr;
  };
  inline void clear_symbname(void)
  {
    pclass_symbname = nullptr;
  };
  std::string class_name_str(void) const;
  void put_symbname(Rps_ObjectRef obr);
  const Rps_SetOb*class_attrset(void) const
  {
    return  pclass_attrset.load();
  };
  void loader_put_symbname(Rps_ObjectRef obr, Rps_Loader*ld);
  void loader_put_attrset(const Rps_SetOb*setob, Rps_Loader*ld);
  ///
  Rps_SetValue compute_set_of_own_method_selectors(Rps_CallFrame*) const;
  Rps_ClosureValue get_own_method(Rps_ObjectRef obsel) const
  {
    if (!obsel)
      return Rps_ClosureValue(nullptr);
    auto it = pclass_methdict.find(obsel);
    if (it != pclass_methdict.end())
      return it->second;
    return Rps_ClosureValue(nullptr);
  };
  void put_own_method(Rps_ObjectRef obsel, Rps_ClosureValue clov)
  {
    if (obsel && clov && clov.is_closure())
      pclass_methdict.insert({obsel,clov});
  };
  void remove_own_method(Rps_ObjectRef obsel)
  {
    if (obsel)
      pclass_methdict.erase(obsel);
  };
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadClassInfo



////////////////////////////////////////////////////////////////
////// mutable set of objects payload - for PaylSetOb, objects of
////// class `mutable_set` _0J1C39JoZiv03qA2HA
extern "C" rpsldpysig_t rpsldpy_setob;
class Rps_PayloadSetOb : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadSetOb*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadSetOb,Rps_ObjectZone*>(Rps_ObjectZone*);
  std::set<Rps_ObjectRef> psetob;
  inline Rps_PayloadSetOb(Rps_ObjectZone*owner);
  Rps_PayloadSetOb(Rps_ObjectRef obr) :
    Rps_PayloadSetOb(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadSetOb()
  {
    psetob.clear();
  };
protected:
  virtual uint32_t wordsize() const
  {
    return (sizeof(*this)+sizeof(void*)-1) / sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "setob";
  };
  /// make a mutable set of given class and space.
  /// if the class is wrong, throw an exception
  static Rps_ObjectRef make_mutable_set_object(Rps_CallFrame*cfr,
      Rps_ObjectRef classob=nullptr,
      Rps_ObjectRef spaceob=nullptr);
  inline Rps_PayloadSetOb(Rps_ObjectZone*obz, Rps_Loader*ld);
  bool contains(const Rps_ObjectZone* obelem) const
  {
    return obelem && psetob.find(Rps_ObjectRef(obelem)) != psetob.end();
  };
  bool contains(const Rps_ObjectRef obr) const
  {
    return obr && psetob.find(obr) != psetob.end();
  };
  unsigned cardinal(void) const
  {
    return (unsigned) psetob.size();
  };
  void add(const Rps_ObjectZone* obelem)
  {
    if (obelem)
      psetob.insert(Rps_ObjectRef(obelem));
  };
  void add (const Rps_ObjectRef obrelem)
  {
    if (!obrelem.is_empty())
      psetob.insert(obrelem);
  };
  void remove(const Rps_ObjectZone* obelem)
  {
    if (obelem) psetob.erase(Rps_ObjectRef(obelem));
  };
  void remove (const Rps_ObjectRef obrelem)
  {
    if (obrelem) psetob.erase(obrelem);
  };
  Rps_SetValue to_set() const
  {
    return Rps_SetValue(psetob);
  };
  Rps_TupleValue to_tuple() const
  {
    std::vector<Rps_ObjectRef> vecob;
    vecob.reserve(cardinal());
    for (auto obrelem: psetob)
      {
        RPS_ASSERT(obrelem);
        vecob.push_back(obrelem);
      };
    return Rps_TupleValue(vecob);
  };
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadSetOb


////////////////////////////////////////////////////////////////
////// mutable vector of objects payload - for PaylVectOb and objects
////// of class `mutable_object_vector _8YknAApDQiF04BDe3W
extern "C" rpsldpysig_t rpsldpy_vectob;
class Rps_PayloadVectOb : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend Rps_PayloadVectOb*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadVectOb,Rps_ObjectZone*>(Rps_ObjectZone*);
  std::vector<Rps_ObjectRef> pvectob;
  inline Rps_PayloadVectOb(Rps_ObjectZone*owner);
  Rps_PayloadVectOb(Rps_ObjectRef obr) :
    Rps_PayloadVectOb(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadVectOb()
  {
    pvectob.clear();
  };
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "vectob";
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  inline Rps_PayloadVectOb(Rps_ObjectZone*obz, Rps_Loader*ld);
  unsigned size(void) const
  {
    return (unsigned) pvectob.size();
  };
  Rps_ObjectRef at(int ix) const
  {
    if (ix<0)
      ix += size();
    if (ix>=0 && ix<(int)size())
      return pvectob[ix];
    throw std::out_of_range("Rps_PayloadVectOb bad index");
  };
  void reserve(unsigned rsiz)
  {
    pvectob.reserve(rsiz);
  };
  void reserve_more(unsigned gap)
  {
    pvectob.reserve(size()+gap);
  };
  void push_back(const Rps_ObjectZone* obcomp)
  {
    if (obcomp)
      pvectob.push_back(Rps_ObjectRef(obcomp));
  };
  void push_back (const Rps_ObjectRef obrcomp)
  {
    if (obrcomp)
      pvectob.push_back(obrcomp);
  };
  Rps_TupleValue to_tuple() const
  {
    return Rps_TupleValue(pvectob);
  };
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadVectOb




////////////////////////////////////////////////////////////////
////// mutable vector of values payload - for PaylVectVal and objects
////// of class `mutable_value_vector` €_8Iz9vBfs4sl041FiuR
extern "C" rpsldpysig_t rpsldpy_vectval;
class Rps_PayloadVectVal : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend  rpsldpysig_t rpsldpy_vectval;
  friend Rps_PayloadVectVal*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadVectVal,Rps_ObjectZone*>(Rps_ObjectZone*);
  std::vector<Rps_Value> pvectval;
  inline Rps_PayloadVectVal(Rps_ObjectZone*owner);
  Rps_PayloadVectVal(Rps_ObjectRef obr) :
    Rps_PayloadVectVal(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadVectVal()
  {
    pvectval.clear();
  };
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "vectval";
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  inline Rps_PayloadVectVal(Rps_ObjectZone*obz, Rps_Loader*ld);
  unsigned size(void) const
  {
    return (unsigned) pvectval.size();
  };
  Rps_Value at(int ix) const
  {
    if (ix<0)
      ix += size();
    if (ix>=0 && ix<(int)size())
      return pvectval[ix];
    throw std::out_of_range("Rps_PayloadVectVal bad index");
  };
  void reserve(unsigned rsiz)
  {
    pvectval.reserve(rsiz);
  };
  void reserve_more(unsigned gap)
  {
    pvectval.reserve(size()+gap);
  };
  void push_back(const Rps_Value val)
  {
    if (val)
      pvectval.push_back(val);
  };
  void push_back (const Rps_ObjectRef obrcomp)
  {
    if (obrcomp)
      pvectval.push_back(Rps_ObjectValue(obrcomp));
  };
  /* make a new closure from a given connective and the values inside
     the vector payload: */
  const Rps_ClosureZone* make_closure_zone_from_vector(Rps_ObjectRef connob);
  //
  /* make a new instance of a given class and the values inside the
     vector payload: */
  const Rps_InstanceZone* make_instance_zone_from_vector(Rps_ObjectRef classob);
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadVectVal


////////////////////////////////////////////////////////////////
////// mutable string buffer payload
extern "C" rpsldpysig_t rpsldpy_string_buffer;
class Rps_PayloadStrBuf : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_string_buffer;
  friend Rps_PayloadStrBuf*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadStrBuf,Rps_ObjectZone*>(Rps_ObjectZone*);
  Rps_PayloadStrBuf(Rps_ObjectZone*owner);
  Rps_PayloadStrBuf(Rps_ObjectRef obr) :
    Rps_PayloadStrBuf(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadStrBuf();
  mutable std::stringbuf strbuf_buffer;
  int strbuf_indent;
  bool strbuf_transient;
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  // Create a string buffer object, and throws an exception if obclass
  // is wrong:
  static Rps_ObjectRef make_string_buffer_object(Rps_CallFrame*callframe, Rps_ObjectRef obclass=nullptr, Rps_ObjectRef obspace=nullptr);
  virtual const std::string payload_type_name(void) const
  {
    return "string_buffer";
  };
  std::stringbuf& string_buffer(void)
  {
    return strbuf_buffer;
  };
  const std::stringbuf& const_string_buffer(void) const
  {
    return strbuf_buffer;
  };
  //std::ostringstream* output_string_stream_ptr(void) { return &strbuf_stream; };
  //const std::ostream& output_stream(void) const { return strbuf_out; };
  //std::ostringstream& output_string_stream(void) { return strbuf_out; };
  int indentation(void) const
  {
    return strbuf_indent;
  };
  void set_indentation(int ind=0)
  {
    strbuf_indent = ind;
  };
  void more_indentation(int delta)
  {
    strbuf_indent += delta;
  };
  void less_indentation(int delta)
  {
    strbuf_indent -= delta;
  };
  bool is_transient(void) const
  {
    return strbuf_transient;
  };
  void set_transient(bool fl=true)
  {
    strbuf_transient=fl;
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  inline Rps_PayloadStrBuf(Rps_ObjectZone*obz, Rps_Loader*ld);
  static inline Rps_ObjectRef the_string_buffer_class(void);
  //- unsigned buffer_length(void) const
  //- {
  //-  return strbuf_buffer.size();
  //- };
  std::string buffer_cppstring(void) const
  {
    return strbuf_buffer.str();
  };
  Rps_StringValue buffer_stringval(void);
  void clear_buffer(void);
  void append_string(const std::string&str);
  void prepend_string(const std::string&str);
  //  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end of class Rps_PayloadStrBuf


////////////////////////////////////////////////////////////////
////// mutable dictionary payload  - associate strings to values
extern "C" rpsldpysig_t rpsldpy_string_dictionary;
class Rps_PayloadStringDict : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_string_dictionary;
  friend Rps_PayloadStringDict*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadStringDict,Rps_ObjectZone*>(Rps_ObjectZone*);
  Rps_PayloadStringDict(Rps_ObjectZone*owner);
  Rps_PayloadStringDict(Rps_ObjectRef obr) :
    Rps_PayloadStringDict(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadStringDict();
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
public:
  // Create a string dictionary object, and throws an exception if obclass
  // is wrong:
  static Rps_ObjectRef make_string_dictionary_object(Rps_CallFrame*callframe, Rps_ObjectRef obclass=nullptr, Rps_ObjectRef obspace=nullptr);
  virtual const std::string payload_type_name(void) const
  {
    return "string_dictionary";
  };
  static Rps_ObjectRef the_string_dictionary_class(void);
  Rps_Value find(const std::string&str) const;
  void add(const std::string&str, Rps_Value val);
  void remove(const std::string&str);
  void set_transient(bool transient=false);
  /// in following iterations, when stopfun returns true, the iteration stops
  void iterate_with_callframe(Rps_CallFrame*callframe, const std::function <bool(Rps_CallFrame*,const std::string&,const Rps_Value)>& stopfun);
  void iterate_with_data(void*data, const std::function <bool(void*,const std::string&,const Rps_Value)>& stopfun);
  /// iterate by applying a closure to the owner, a fresh string value and associated value till the closure returns nil
  void iterate_apply(Rps_CallFrame*callframe, Rps_Value closv);
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
private:
  std::map<std::string, Rps_Value> dict_map;
  bool dict_is_transient;
}; // end class Rps_PayloadStringDict


////////////////////////////////////////////////////////////////
////// mutable space payload, objects of class `space`
////// _2i66FFjmS7n03HNNBx
extern "C" rpsldpysig_t rpsldpy_space;
class Rps_PayloadSpace : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_space;
  friend Rps_PayloadSpace*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadSpace,Rps_ObjectZone*>(Rps_ObjectZone*);
protected:
  inline Rps_PayloadSpace(Rps_ObjectZone*owner);
  Rps_PayloadSpace(Rps_ObjectRef obr) :
    Rps_PayloadSpace(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadSpace()
  {
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const
  {
    return false;
  };
public:
  virtual const std::string payload_type_name(void) const
  {
    return "space";
  };
  inline Rps_PayloadSpace(Rps_ObjectZone*obz, Rps_Loader*ld);
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadSpace


////////////////////////////////////////////////////////////////
////// symbol payload
extern "C" void rps_initialize_symbols_after_loading (Rps_Loader*ld);
extern "C" rpsldpysig_t rpsldpy_symbol;
class Rps_PayloadSymbol : public Rps_Payload
{
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_symbol;
  friend void rps_initialize_symbols_after_loading(Rps_Loader*ld);
  friend Rps_PayloadSymbol*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadSymbol, Rps_ObjectZone*>(Rps_ObjectZone*);
  std::string symb_name;
  std::atomic<const void*> symb_data;
  std::atomic<bool> symb_is_weak;
  static std::recursive_mutex symb_tablemtx;
  static std::map<std::string,Rps_PayloadSymbol*> symb_table;
  static std::unordered_map<std::string,Rps_ObjectRef*> symb_hardcoded_hashtable;
protected:
  Rps_PayloadSymbol(Rps_ObjectZone*owner);
  Rps_PayloadSymbol(Rps_ObjectRef obr) :
    Rps_PayloadSymbol(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadSymbol();
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const
  {
    return symb_is_weak.load();
  };
public:
  virtual const std::string payload_type_name(void) const
  {
    return "symbol";
  };
  static void gc_mark_strong_symbols(Rps_GarbageCollector*gc);
  void load_register_name(const char*name, Rps_Loader*ld,bool weak=false);
  void load_register_name(const std::string& str, Rps_Loader*ld, bool weak=false)
  {
    load_register_name (str.c_str(), ld, weak);
  };
  bool is_weak(void) const
  {
    return symb_is_weak.load();
  };
  bool symbol_is_weak(void) const
  {
    return symb_is_weak.load();
  };
  void set_weak(bool f)
  {
    symb_is_weak.store(f);
  };
  Rps_Value symbol_value(void) const
  {
    return Rps_Value(symb_data.load(), this);
  };
  void symbol_put_value(Rps_Value v)
  {
    symb_data.store(v.data_for_symbol(this));
  };
  const std::string& symbol_name(void) const
  {
    return symb_name;
  };
  static bool valid_name(const char*str);
  static bool valid_name(const std::string str)
  {
    return valid_name(str.c_str());
  };
  static inline Rps_ObjectRef find_named_object(const std::string&str);
  static bool register_name(std::string name, Rps_ObjectRef obj, bool weak);
  static bool register_strong_name(std::string name, Rps_ObjectRef obj)
  {
    return register_name(name, obj, false);
  }
  static bool register_weak_name(std::string name, Rps_ObjectRef obj)
  {
    return register_name(name,obj, true);
  }
  static Rps_PayloadSymbol* find_named_payload(const std::string&str)
  {
    std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
    auto it = symb_table.find(str);
    if (it != symb_table.end())
      {
        auto symb = it->second;
        if (symb)
          return symb;
      };
    return nullptr;
  };
  static const Rps_SetValue set_of_all_symbols(void);
  static bool forget_name(std::string name);
  static bool forget_object(Rps_ObjectRef obj);
  // given a C string which looks like a C identifier starting with a
  // letter, autocomplete that string and call a given C++ closure on
  // every possible object ref and completed name, till that closure
  // returns true. Return the number of matches, or else 0
  static int autocomplete_name(const char*prefix,
                               const std::function<bool(const Rps_ObjectZone*,const std::string&)>&stopfun);
  virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadSymbol





extern "C" rpsldpysig_t rpsldpy_objmap;
// Rps_PayloadObjMap contains a std::map of objects to values.
class Rps_PayloadObjMap : public Rps_Payload
{
  std::map<Rps_ObjectRef,Rps_Value> obm_map;
  Rps_Value obm_descr;
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_objmap;
  friend Rps_PayloadObjMap*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadObjMap,Rps_ObjectZone*>(Rps_ObjectZone*);
protected:
  Rps_PayloadObjMap(Rps_ObjectZone*owner);
  Rps_PayloadObjMap(Rps_ObjectRef obr) :
    Rps_PayloadObjMap(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadObjMap()
  {
    obm_map.clear();
    obm_descr = nullptr;
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  void gc_mark_objmap(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  void dump_scan_objmap_internal(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  void dump_json_objmap_internal_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const
  {
    return false;
  };
public:
  virtual const std::string payload_type_name(void) const
  {
    return "objmap";
  };
  static Rps_ObjectZone* make(Rps_CallFrame*cf, Rps_ObjectRef classob=nullptr,
                              Rps_ObjectRef spaceob=nullptr);
  static Rps_Value get(Rps_ObjectRef obmap, Rps_ObjectRef obkey, Rps_Value defaultval=nullptr, bool*missing=nullptr);
  Rps_Value get_obmap(Rps_ObjectRef obkey, Rps_Value defaultval=nullptr, bool*missing=nullptr) const;
  static void put(Rps_ObjectRef obmap, Rps_ObjectRef  obkey, Rps_Value val);
  void put_obmap(Rps_ObjectRef obkey, Rps_Value val);
  static bool remove(Rps_ObjectRef obmap, Rps_ObjectRef obkey);
  bool remove_obmap(Rps_ObjectRef obkey);
  bool has_key_obmap(Rps_ObjectRef obkey) const;
  static bool has_key(Rps_ObjectRef obmap, Rps_ObjectRef obkey);
  inline Rps_PayloadObjMap(Rps_ObjectZone*obz, Rps_Loader*ld);
  Rps_Value get_descr() const
  {
    return obm_descr;
  };
  void put_descr(Rps_Value d)
  {
    obm_descr = d;
  };
  void do_each_entry(Rps_CallFrame*cf, std::function<bool(Rps_CallFrame*,Rps_ObjectRef,Rps_Value,void*)> f,
                     void* clientdata=nullptr) const
  {
    for (auto it: obm_map)
      {
        if (f(cf, it.first, it.second, clientdata))
          break;
      }
  };
  //virtual void output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const;
};                              // end Rps_PayloadObjMap



/// environments also have a parent environment

extern "C" rpsldpysig_t rpsldpy_environment;

/// Find it the given environment envob the binding for varob. If
/// pmissing is given, set it to true when a binding was missing, to
/// false when a binding was found.
extern "C" Rps_Value rps_environment_get_shallow_bound_value(Rps_ObjectRef envob, Rps_ObjectRef varob,
    bool *missing=nullptr);
/// Find the depth of the environment with a binding for varob, or else return -1
extern "C" int rps_environment_find_binding_depth(Rps_ObjectRef envob, Rps_ObjectRef varob);
/// Find in the given envob or its parent or ancestor environment the
/// bound value of a given variable; if found, the *pdepth (when
/// given) is set to the depth of the environment and the *penv (when
/// given) to the environment object...; if missing *pdepth becomes
/// negative, and *penvob is cleared.
extern "C" Rps_Value rps_environment_find_bound_value(Rps_ObjectRef envob, Rps_ObjectRef varob,
    int*pdepth=nullptr, Rps_ObjectRef*penvob=nullptr);
/// Add or put a binding in the current environment.  Do nothing when envob is not an environment.
void rps_environment_add_shallow_binding(Rps_CallFrame*callframe,
    Rps_ObjectRef envob, Rps_ObjectRef varob, Rps_Value val);
/// overwrite a binding in the deep environment containing it, or when not found in the current one. Return affected depth
extern "C" int rps_environment_overwrite_binding(Rps_CallFrame*callframe,
    Rps_ObjectRef envob, Rps_ObjectRef varob, Rps_Value val,
    Rps_ObjectRef*penvob=nullptr);
/// remove a binding in the current environment, returning old value
Rps_Value rps_environment_remove_shallow_binding(Rps_CallFrame*callframe,
    Rps_ObjectRef envob, Rps_ObjectRef varob, bool*pfound=nullptr);
/// remove a binding in the environment containing it (perhaps deeply) - return the depth or -1 if not found
extern "C" int rps_environment_remove_deep_binding(Rps_CallFrame*callframe,
    Rps_ObjectRef startenvob, Rps_ObjectRef varob,
    Rps_ObjectRef*penvob=nullptr,
    Rps_Value*poldval=nullptr);

class Rps_PayloadEnvironment : public Rps_PayloadObjMap
{
  Rps_ObjectRef env_parent;
  friend class Rps_ObjectRef;
  friend class Rps_ObjectZone;
  friend rpsldpysig_t rpsldpy_environment;
  friend Rps_PayloadEnvironment*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadEnvironment,Rps_ObjectZone*>(Rps_ObjectZone*);
  friend Rps_Value rps_environment_get_bound_value(Rps_ObjectRef envob, Rps_ObjectRef varob,
      int*pdepth, Rps_ObjectRef*penvob);
protected:
  Rps_PayloadEnvironment(Rps_ObjectZone*owner);
  Rps_PayloadEnvironment(Rps_ObjectRef obr) :
    Rps_PayloadObjMap(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadEnvironment()
  {
  };
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const
  {
    return false;
  };
public:
  virtual const std::string payload_type_name(void) const
  {
    return "environment";
  };
  inline Rps_PayloadEnvironment(Rps_ObjectZone*obz, Rps_Loader*ld);
  static Rps_ObjectZone* make(Rps_CallFrame*cf, Rps_ObjectRef classob=nullptr, Rps_ObjectRef spaceob=nullptr);
  static Rps_ObjectZone* make_with_parent_environment(Rps_CallFrame*cf,
      Rps_ObjectRef parentob,
      Rps_ObjectRef classob=nullptr,
      Rps_ObjectRef spaceob=nullptr);

  Rps_ObjectRef get_parent_environment() const
  {
    return  env_parent;
  };
  void put_parent_environment(Rps_ObjectRef envob);
#warning Rps_PayloadEnvironment not fully implemented
};                              // end Rps_PayloadEnvironment



////////////////////////////////////////////////////////////////

#define RPS_MANIFEST_JSON "rps_manifest.json"
// same as used in rps_manifest.json file
#define RPS_PREVIOUS_MANIFEST_FORMAT "RefPerSysFormat2023A"


/// the next format should enable computing some data at load time...
/// using the loadrout JSON member (for loading objects whose content
/// is operating system or architecture dependent).
#define RPS_MANIFEST_FORMAT "RefPerSysFormat2024A"

// the user manifest is optional, in the rps_homedir()
// so using $REFPERSYS_HOME or $HOME
#define RPS_USER_MANIFEST_JSON ".refpersys.json"

/***************************************************************
 * ROOT OBJECTS
 *
 * The root objects are GC-scanned and GC-marked at every garbage
 * collection.  They are listed in generated/rps-roots.hh which is
 * written at dump time and contains one RPS_INSTALL_ROOT_OB macro
 * invocation line per root object. For example the agenda object of
 * oid _1aGtWm38Vw701jDhZn is installed by a C++ line like
 * rps_install_root_ob(_1aGtWm38Vw701jDhZn) with the C++ macro name in
 * capital.
 *
 * Each root object is pointed by an extern "C" global C++ variable
 * whose prefix is rps_rootob. For example
 * rps_rootob_1aGtWm38Vw701jDhZn is the_agenda root object, and should
 * be refered by RPS_ROOTOB(_1aGtWm38Vw701jDhZn) in C++ code (it is an
 * Rps_ObjectRef with a _optr inside).  The actual root object is
 * allocated early at load time in Rps_Loader::initialize_root_objects
 * Adding a root object means registering it in some global internal
 * structure, and removing it means unregistering it (but the object
 * itself remains in memory).  We want to avoid having too many root
 * objects.
 ***************************************************************/
//// global roots for garbage collection and persistence
/// the called function cannot add, remove or query the global root set
extern "C" void rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun);
extern "C" void rps_add_root_object (const Rps_ObjectRef);
/// Both rps_remove_root_object and rps_is_root_object return false if
/// argument is not a root object.  Of course rps_remove_root_object
/// also removes it.
extern "C" bool rps_remove_root_object (const Rps_ObjectRef);
extern "C" bool rps_is_root_object (const Rps_ObjectRef);
extern "C" std::set<Rps_ObjectRef> rps_set_root_objects(void);
extern "C" unsigned rps_nb_root_objects(void);
extern "C" void rps_initialize_roots_after_loading (Rps_Loader*ld);
extern "C" unsigned rps_hardcoded_number_of_roots(void);

extern "C" void rps_add_constant_object(Rps_CallFrame*callframe, const Rps_ObjectRef obconst);
extern "C" void rps_remove_constant_object(Rps_CallFrame*callframe, const Rps_ObjectRef obconst);
extern "C" void rps_initialize_symbols_after_loading (Rps_Loader*ld);


extern "C" unsigned rps_hardcoded_number_of_symbols(void);
extern "C" unsigned rps_hardcoded_number_of_constants(void);

//////////////// initial Read-Eval-Print-Loop using GNU readline

extern "C" std::string rps_repl_version(void); // in repl_rps.cc

/// Interpret from either a given input stream,
/// or using readline if inp is null. In repl_rps.cc
extern "C" void rps_repl_interpret(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, int& lineno);

/// Create a new REPL command, and output to stdout some draft C++
/// code to parse it.... To be called from the main thread.
/// Implemented in our C++ file repl_rps.cc
extern "C" void rps_repl_create_command(Rps_CallFrame*callframe, const char*commandname);

extern "C" std::istream*rps_repl_input;
extern "C" bool rps_repl_stopped;




////////////////////////////////////////////////////////////////

extern "C" void rps_initialize_curl(void);
extern "C" std::string rps_curl_version(void); // in curl_rps.cc
extern "C" void rps_garbcoll_application(Rps_GarbageCollector&gc);





////................................................................
//// Code generation routines (either C++ files later compiled as a
//// dlopen-able plugin using the do-build-refpersys-plugin, or in-memory
//// code generation of using GNU lightning).  Both routines return
//// true on successful code generation.
////................................................................

/// GNU lightning approved on Whatsapp by Abishek Chakravarti on July,
/// 24, 2023 but given that it is too difficult to compile we phase
/// out GNU lightning and will use libgccjit (see gcc.gnu.org/onlinedocs/jit ...) instead.
/// so no more Rps_generate_lightning_code
extern "C" bool rps_generate_cplusplus_code(Rps_CallFrame*callerframe,
    Rps_ObjectRef obmodule,
    Rps_Value genparamv=nullptr);


////................................................................
//// load and dump routines.  See files load_rps.cc and dump_rps.cc
////................................................................

extern "C" Json::Value rps_load_string_to_json(const std::string&str, const char*filnam=nullptr, int lineno=0);
extern "C" std::string rps_load_json_to_string(const Json::Value&jv);

extern "C" void rps_dump_into (std::string dirpath = ".", Rps_CallFrame* callframe = nullptr); // in store_rps.cc
extern "C" double rps_dump_start_elapsed_time(Rps_Dumper*);
extern "C" double rps_dump_start_process_time(Rps_Dumper*);
extern "C" double rps_dump_start_wallclock_time(Rps_Dumper*);
extern "C" double rps_dump_start_monotonic_time(Rps_Dumper*);

// scan a code address, e.g. a C function pointer whose address is inside some dlopen-ed plugin
extern "C" void rps_dump_scan_code_addr(Rps_Dumper*, const void*);
// scan an object
extern "C" void rps_dump_scan_object(Rps_Dumper*, const Rps_ObjectRef obr);
// scan in a space a component object
extern "C" void rps_dump_scan_space_component(Rps_Dumper*, Rps_ObjectRef obrspace, Rps_ObjectRef obrcomp);
// scan a value
extern "C" void rps_dump_scan_value(Rps_Dumper*, const Rps_Value val, unsigned depth);
extern "C" Json::Value rps_dump_json_value(Rps_Dumper*, const Rps_Value val);
extern "C" Json::Value rps_dump_json_objectref(Rps_Dumper*, const Rps_ObjectRef obr);

// is an object dumpable?
extern "C" bool rps_is_dumpable_objref(Rps_Dumper*, const Rps_ObjectRef obr);

// is an object dumpable as attribute in another object?
extern "C" bool rps_is_dumpable_objattr(Rps_Dumper*, const Rps_ObjectRef obr);

// is a value dumpable?
extern "C" bool rps_is_dumpable_value(Rps_Dumper*, const Rps_Value val);


extern "C" void rps_load_from (const std::string& dirpath); // in store_rps.cc

extern "C" void rps_load_add_todo(Rps_Loader*,const std::function<void(Rps_Loader*)>& todofun);

extern "C" void rps_print_types_info (void);

extern "C" void rps_repl_lexer_test(void);

extern "C" void rps_do_repl_commands_vec(const std::vector<std::string>&cmdvec);


/// this routine set some native data in loaded heap, like the size of
/// predefined types...
extern "C" void rps_set_native_data_in_loader(Rps_Loader*);

extern "C" void rps_run_loaded_application(int &argc, char **argv);

///// UTF8 encoded string output (in file scalar_rps.cc)
/// output a C string in HTML encoding; if nl2br is true, every newline is output as <br/>
extern "C" void rps_output_utf8_html(std::ostream&out, const char*str, int bytlen= -1, bool nl2br= false);
/// output a C string in C or JSON encoding
extern "C" void rps_output_utf8_cjson(std::ostream&out, const char*str, int bytlen= -1);


/// output in HTML encoding
class Rps_Html_String : public std::string
{
public:
  Rps_Html_String(const char*str, int len= -1) :
    std::string(str?str:"", (len>=0 && str)?len:strlen(str?str:"")) {};
  Rps_Html_String(const std::string&str) : std::string(str) {};
  ~Rps_Html_String() = default;
  virtual void output(std::ostream&out) const
  {
    rps_output_utf8_html(out, c_str(), (int)size());
  };
}; // end class  Rps_Html_String
inline std::ostream&operator << (std::ostream&out, const Rps_Html_String&hstr)
{
  hstr.output(out);
  return out;
};  // end << Rps_Html_String

/// output in HTML encoding, with newlines changed to <br/>
class Rps_Html_Nl2br_String : public Rps_Html_String
{
public:
  Rps_Html_Nl2br_String(const char*str, int len= -1) :
    Rps_Html_String(str,len) {};
  Rps_Html_Nl2br_String(const std::string&str) :
    Rps_Html_String(str) {};
  ~Rps_Html_Nl2br_String() = default;
  virtual void output(std::ostream&out) const
  {
    rps_output_utf8_html(out, c_str(), (int)size(), true);
  };
}; // end class Rps_Html_Nl2br_String

class Rps_Cjson_String : public std::string
{
public:
  Rps_Cjson_String(const char*str, int len= -1) :
    std::string(str?str:"", (len>=0 && str)?len:strlen(str?str:"")) {};
  Rps_Cjson_String(const std::string&str) : std::string(str) {};
  ~Rps_Cjson_String() = default;
  void output(std::ostream&out) const
  {
    rps_output_utf8_cjson(out, c_str(), (int)size());
  };
};
inline std::ostream&operator << (std::ostream&out, const Rps_Cjson_String&hstr)
{
  hstr.output(out);
  return out;
};  // end << Rps_Cjson_String


/// for output a string double-quoted like for C
class Rps_QuotedC_String : public std::string
{
protected:
  bool qtc_empty;
public:
  Rps_QuotedC_String(const char*str, int len= -1) :
    std::string(str?str:"", (len>=0 && str)?len:strlen(str?str:"")),
    qtc_empty(str==nullptr) {};
  Rps_QuotedC_String(const std::string&str)
    : std::string(str), qtc_empty(false) {};
  Rps_QuotedC_String(const Rps_QuotedC_String&) = default;
  Rps_QuotedC_String(Rps_QuotedC_String&&) = default;
  ~Rps_QuotedC_String() = default;
  void output(std::ostream&out) const
  {
    if (qtc_empty)
      out << "*null*";
    else
      {
        out << "\"";
        rps_output_utf8_cjson(out, c_str(), (int)size());
        out << "\"";
      }
  };
  bool is_empty() const
  {
    return qtc_empty;
  };
};        // end class Rps_QuotedC_String

inline std::ostream&operator << (std::ostream&out, const Rps_QuotedC_String&hstr)
{
  hstr.output(out);
  return out;
};  // end << Rps_QuotedC_String

/// for output a string single-quoted like for the GNU bash shell
class Rps_SingleQuotedC_String : public Rps_QuotedC_String
{
public:
  Rps_SingleQuotedC_String(const char*str, int len= -1) : Rps_QuotedC_String(str,len) {};
  Rps_SingleQuotedC_String(const std::string&str)
    : Rps_QuotedC_String(str) {};
  Rps_SingleQuotedC_String(const Rps_SingleQuotedC_String&) = default;
  Rps_SingleQuotedC_String(Rps_SingleQuotedC_String&&) = default;
  ~Rps_SingleQuotedC_String() = default;
  void output(std::ostream&out) const
  {
    if (qtc_empty)
      out << "*null*";
    else
      {
        out << "'";
        rps_output_utf8_cjson(out, c_str(), (int)size());
        out << "'";
      }
  };
};        // end class Rps_SingleQuotedC_String

inline std::ostream&operator << (std::ostream&out, const Rps_SingleQuotedC_String&hstr)
{
  hstr.output(out);
  return out;
};  // end << Rps_SingleQuotedC_String

//////////////////////////////////////////////////////////////////
/// initial agenda machinery;

/// start and run the agenda mechanism, that is start NBJOBS worker threads
/// each running Rps_Agenda::run_agenda_worker; this does not return
/// till the agenda is stopped
extern "C" void rps_run_agenda_mechanism(int nbjobs);
/// stop the agenda mechanism
extern "C" void rps_stop_agenda_mechanism(void);

/****
 * The agenda is a unique and central data-structure (and RefPerSys
 * object `the_agenda` with a `Rps_PayloadAgenda` payload) managing
 * todo things. Each item to be done in the agenda is a tasklet (some
 * object of class `tasklet`, each with its `Rps_PayloadTasklet`
 * payload).
 ****/
extern "C" rpsldpysig_t rpsldpy_agenda;
class Rps_Agenda   /// all member functions are static...
{
  friend class Rps_GarbageCollector;
  friend class Rps_PayloadAgenda;
  friend class Rps_PayloadUnixProcess;
  friend class Rps_PayloadPopenedFile;
  friend void rps_event_loop (void);
  friend void rps_initialize_event_loop(void);
  friend rpsldpysig_t rpsldpy_agenda;
  friend void rps_run_agenda_mechanism(int nbjobs);
  friend void rps_stop_agenda_mechanism(void);
  // elapsed real time when agenda
  // should stop running, related to
  // --run-delay program option...
  static double agenda_timeout;
public:
  enum agenda_prio_en
  {
    AgPrio_Idle= -1,
    AgPrio__None= 0,
    AgPrio_Low,
    AgPrio_Normal,
    AgPrio_High,
    AgPrio__Last
  };
  enum workthread_state_en
  {
    WthrAg__None,
    WthrAg_Idle, //  the worker thread is idle
    WthrAg_GC,   // the worker thread is garbage collecting
    WthrAg_EndGC, // the worker thread has ended garbage collection,
    // and will be running again on the next loop
    WthrAg_Run,  // the worker thread is running and allocating
    WthrAg__Last
  };
  static const char* agenda_priority_names[AgPrio__Last];
  static inline Rps_ObjectRef the_agenda();
  static inline Rps_ObjectRef tasklet_class();
  static void gc_mark(Rps_GarbageCollector&);
  static void initialize(void);
  static void add_tasklet(agenda_prio_en prio, Rps_ObjectRef obtasklet);
  static Rps_ObjectRef fetch_tasklet_to_run(void);
  static void run_agenda_worker(int ix);
  static void do_garbage_collect(int ix, Rps_CallFrame*callframe);
protected:
  static void dump_scan_agenda(Rps_Dumper*du);
  static void dump_json_agenda(Rps_Dumper*du, Json::Value&jv);
private:
  static std::recursive_mutex agenda_mtx_;
  static std::condition_variable_any agenda_changed_condvar_;
  static std::atomic<unsigned long> agenda_add_counter_;
  static std::deque<Rps_ObjectRef> agenda_fifo_[AgPrio__Last];
  static std::atomic<bool> agenda_is_running_; // true when agenda is running
  static std::atomic<bool> agenda_needs_garbcoll_; // true when GC is needed
  /// the cumulated amount of allocated words at previous GC is:
  static std::atomic<uint64_t> agenda_cumulw_gc_;
  // once a megaword has been allocated, we want to garbage collect, hence:
  static constexpr uint64_t agenda_gc_threshold = 1<<20;
  static std::atomic<std::thread*> agenda_thread_array_[RPS_NBJOBS_MAX+2];
  static std::atomic<workthread_state_en> agenda_work_thread_state_[RPS_NBJOBS_MAX+2];
  /// the call frames below makes sense only during garbage collection....
  static std::atomic<Rps_CallFrame*> agenda_work_gc_callframe_[RPS_NBJOBS_MAX+2];
  static std::atomic<Rps_CallFrame**> agenda_work_gc_current_callframe_ptr[RPS_NBJOBS_MAX+2];
};                              // end class Rps_Agenda



/// the payload with agenda, only for the_agenda predefined object
class Rps_PayloadAgenda : public Rps_Payload
{
  friend class Rps_Agenda;
  friend rpsldpysig_t rpsldpy_agenda;
  /// What is really used is the attributes in the_agenda singleton
  /// object.  Conceptually the values inside Rps_Agenda above are the
  /// fields of the unique Rps_PayloadAgenda tied to the_agenda root object
  /// of objid _1aGtWm38Vw701jDhZn "the_agenda"∈agenda
public:
  inline Rps_PayloadAgenda(Rps_ObjectZone*owner);
  inline Rps_PayloadAgenda(Rps_ObjectZone*owner, Rps_Loader*ld);
  Rps_PayloadAgenda(Rps_ObjectRef obr) :
    Rps_PayloadAgenda(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadAgenda();
protected:
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "agenda";
  };
};  // end of Rps_PayloadAgenda


/// the payload with tasklets, inside the_agenda
class Rps_PayloadTasklet : public Rps_Payload
{
  friend class Rps_Agenda;
  friend rpsldpysig_t rpsldpy_agenda;
  friend rpsldpysig_t rpsldpy_tasklet;
  /// a tasklet has a closure to apply to run it
  /// and a obsolescence time
  Rps_ClosureValue tasklet_todoclos; // the closure to apply to
  double tasklet_obsoltime; // obsolescence time
  bool tasklet_permanent;
public:
  inline Rps_PayloadTasklet(Rps_ObjectZone*owner);
  inline Rps_PayloadTasklet(Rps_ObjectZone*owner, Rps_Loader*ld);
  Rps_PayloadTasklet(Rps_ObjectRef obr) :
    Rps_PayloadTasklet(obr?obr.optr():nullptr) {};
  virtual ~Rps_PayloadTasklet();
protected:
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "tasklet";
  };
  Rps_ClosureValue todo_closure(void) const
  {
    return tasklet_todoclos;
  };
};  // end of Rps_PayloadTasklet


typedef std::vector<std::string> rps_cppvect_of_string_t;

/// the transient payload for unix processes (see PaylUnixProcess)
class Rps_PayloadUnixProcess : public Rps_Payload
{
  friend class Rps_Agenda;
  friend class Rps_PayloadAgenda;
  friend void rps_event_loop(void);
  std::atomic<pid_t> _unixproc_pid;
  std::string _unixproc_exe;
  rps_cppvect_of_string_t _unixproc_argv;
  Rps_ClosureValue _unixproc_closure; // handle termination of the Unix process
  Rps_ClosureValue _unixproc_inputclos; // handle input condition
  Rps_ClosureValue _unixproc_outputclos; // handle output condition
  int _unixproc_pipeinputfd;             // input pipe(2)
  int _unixproc_pipeoutputfd;            // output pipe(2)
  std::atomic<unsigned> _unixproc_cpu_time_limit; // for setrlimit(RLIMIT_CPU, ...) in child
  std::atomic<unsigned> _unixproc_elapsed_time_limit;
  std::atomic<time_t> _unixproc_start_time;
  /// limits which are 0 are not set!
  std::atomic<unsigned> _unixproc_as_mb_limit; // megabytes for  setrlimit(RRLIMIT_AS, ...) in child
  std::atomic<unsigned> _unixproc_fsize_mb_limit; // megabytes for  setrlimit(RRLIMIT_FSIZE, ...) in child
  std::atomic<unsigned> _unixproc_core_mb_limit; // megabytes for  setrlimit(RRLIMIT_CORE, ...) in child
  std::atomic<bool> _unixproc_forbid_core;
  std::atomic<unsigned> _unixproc_nofile_limit; // fds for  setrlimit(RRLIMIT_NOFILE, ...) in child
  static std::set<Rps_PayloadUnixProcess*> set_of_runnable_processes;
  static std::deque<Rps_PayloadUnixProcess*> queue_of_runnable_processes;
  static std::mutex mtx_of_runnable_processes;
  friend Rps_PayloadUnixProcess*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadUnixProcess,Rps_ObjectZone*>(Rps_ObjectZone*);
#warning Rps_PayloadUnixProcess may need cooperation with agenda.
  /*** TODO:
   *
   *   Perhaps we need a field containing a Rps_ClosureValue to handle
   * termination of that process in the agenda?
   *
   * The agenda machinery needs to handle unix process termination and
   * SIGCHLD signal.
   **/
public:
  Rps_PayloadUnixProcess(Rps_ObjectZone*owner, Rps_Loader*ld); // impossible
  Rps_PayloadUnixProcess(Rps_ObjectZone*owner);
  virtual ~Rps_PayloadUnixProcess();
  static Rps_ObjectRef make_dormant_unix_process_object(Rps_CallFrame*curf,const std::string& exec);
protected:
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "unixprocess";
  };
  void add_process_argument (const std::string& arg);
  /// the methods related to limits return the old one, and if given a >0 number set it
  // if a process is running, gives it current .rlim_cur as obtained with prlimit....
  unsigned address_space_megabytes_limit(unsigned newlimit=0);
  unsigned file_size_megabytes_limit(unsigned newlimit=0);
  unsigned core_megabytes_limit(unsigned newlimit=0);
  void forbid_core_dump();      // force the CORE limit to 0
  unsigned nofile_limit(unsigned newlimit=0);
  /// the process closure is called when the process has ended...
  const Rps_ClosureValue get_process_closure(void) const;
  void put_process_closure(Rps_ClosureValue);
  /// the input closure is called when the process needs some input on
  /// its stdin, which is then some pipe(2). The closure is given the
  /// owner of the Rps_PayloadUnixProcess as argument and should
  /// return a string, a number, a Json which gets written on the
  /// pipe.
  const Rps_ClosureValue get_input_closure(void) const;
  // set the input closure, should be called before forking ie before
  // start_process
  void put_input_closure(Rps_ClosureValue);
  /// the output closure is called when the process give some output on
  /// its stdout, which is then some pipe(2). The closure is given the
  /// owner of the Rps_PayloadUnixProcess as argument
  const Rps_ClosureValue get_output_closure(void) const;
  // set the output closure, should be called before forking ie before
  // start_process.
  void put_output_closure(Rps_ClosureValue);
#warning missing member functions related to output pipe...
  /// fork the process
  void start_process(Rps_CallFrame*callframe);
  static void gc_mark_active_processes(Rps_GarbageCollector&);
  static void do_on_active_process_queue(std::function<void(Rps_ObjectRef,Rps_CallFrame*,void*)> fun, Rps_CallFrame*callframe, void*client_data=nullptr);
  /*** TODO:
   *
   * We probably need a static member function to fork a unix process,
   * given the executable path and program arguments.  We may also
   * need to redirect its stdin/stdout/stderr ...  The QProcess class
   * from Qt might be inspirational.
   **/
  /*** TODO:
   *
   * We need to manage and keep the set of forked unix processes and improve the
   * agenda machinery to handle their termination.
   */
};  // end of Rps_PayloadUnixProcess


/// the transient payload for popened files (see PaylOpenedFile)
class Rps_PayloadPopenedFile : public Rps_Payload
{
  friend class Rps_Agenda;
  friend class Rps_PayloadAgenda;
  const std::string _popened_cmd;
  const bool _popened_to_read;
  std::atomic<FILE*> _popened_file;
public:
  Rps_PayloadPopenedFile(Rps_ObjectZone*owner, const std::string command, bool reading);
  Rps_PayloadPopenedFile(Rps_ObjectZone*owner, Rps_Loader*ld); // impossible
  Rps_PayloadPopenedFile(Rps_ObjectRef obr, const std::string command, bool reading) :
    Rps_PayloadPopenedFile(obr?obr.optr():nullptr, command, reading) {};
  virtual ~Rps_PayloadPopenedFile();
protected:
  virtual uint32_t wordsize(void) const
  {
    return (sizeof(*this)+sizeof(void*)-1)/sizeof(void*);
  };
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  virtual bool is_erasable(void) const;
public:
  virtual const std::string payload_type_name(void) const
  {
    return "popenedfile";
  };
  const std::string command_string_popen() const
  {
    return _popened_cmd;
  };
  bool is_reading_popen() const
  {
    return _popened_to_read;
  };
  bool is_writing_popen() const
  {
    return !_popened_to_read;
  };
  FILE* get_popened_file() const
  {
    return _popened_file.load();
  };
};  // end of Rps_PayloadPopenedFile


/// related to eventloop and agenda
extern "C" void rps_postpone_dump(void);
extern "C" void rps_postpone_garbage_collection(void);
extern "C" void rps_postpone_quit(void);
extern "C" void rps_postpone_exit_with_dump(void);
extern "C" void rps_postpone_child_process(void);

//////////////////////////////////////////////////////////////////
/// C++ code can refer to root objects
#define RPS_ROOT_OB(Oid) rps_rootob##Oid

// C++ code can refer to named symbols
#define RPS_SYMB_OB(Nam) rps_symbob_##Nam


/// each root object is also a public variable
#define RPS_INSTALL_ROOT_OB(Oid) extern "C" Rps_ObjectRef RPS_ROOT_OB(Oid);
#include "generated/rps-roots.hh"

// each named global symbol is also a public variable
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Nam) extern "C" Rps_ObjectRef RPS_SYMB_OB(Nam);
#include "generated/rps-names.hh"




extern "C" Rps_CallFrame*rps_edit_cplusplus_callframe;
#include "inline_rps.hh"



#endif /*REFPERSYS_INCLUDED*/
// end of file refpersys.hh */

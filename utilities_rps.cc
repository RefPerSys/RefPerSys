/****************************************************************
 * file utilities_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has some utilities functions.
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
#include <lightning.h>

#ifdef RPS_HAVE_ARCH_x86_64
#warning utilities_rps.cc includes lightning/jit_x86.h
#include "lightning/jit_x86.h"
#endif

extern "C" const char rps_utilities_gitid[];
const char rps_utilities_gitid[]= RPS_GITID;

extern "C" const char rps_utilities_date[];
const char rps_utilities_date[]= __DATE__;



/// https://lists.gnu.org/archive/html/lightning/2023-08/msg00004.html
/// see also file lightgen_rps.cc

extern "C" const int rps_gnulightning_jitstate_size;
extern "C" const int rps_gnulightning_jitstate_align;

// we may have a pair of FIFO to communicate with some external
// process (for graphical user interface), perhaps mini-edit-fltk on
// https://github.com/bstarynk/misc-basile/ ... The FIFO prefix is
// $FIFOPREFIX. The messages from the GUI user interface to RefPerSys
// are on $FIFOPREFIX.out; the messages from RefPerSys to that GUI
// user interface are on $FIFOPREFIX.cmd
static std::string rps_fifo_prefix;
char rps_bufpath_homedir[rps_path_byte_size];
char rps_debug_path[rps_path_byte_size];
static  struct rps_fifo_fdpair_st rps_fifo_pair;

static std::vector<std::string> rps_postponed_removed_files_vector;
static std::mutex rps_postponed_lock;

std::string rps_publisher_url_str;

static pthread_t rps_main_thread_handle;


bool rps_is_main_thread(void)
{
  return pthread_self() == rps_main_thread_handle;
} // end rps_is_main_thread


static std::map<std::string,std::string> rps_dict_extra_arg;

void
rps_put_fifo_prefix(const char*pref)
{
  RPS_ASSERT(rps_is_main_thread());
  if (!rps_fifo_prefix.empty())
    return;
  if (pref && pref[0])
    rps_fifo_prefix = std::string(pref);
} // end rps_put_fifo_prefix

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
  std::string cmdfifo = rps_fifo_prefix+".cmd";
  if (rps_is_fifo(cmdfifo))
    remove(cmdfifo.c_str());
  std::string outfifo = rps_fifo_prefix+".out";
  if (rps_is_fifo(outfifo))
    remove(outfifo.c_str());
} // end rps_remove_fifos

void
rps_do_create_fifos_from_prefix(void)
{
  int cmdfd= -1;
  int outfd= -1;
  bool rmatex = false;
  char cwdbuf[rps_path_byte_size];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  RPS_ASSERT(rps_is_main_thread());
  if (getcwd(cwdbuf, sizeof(cwdbuf)-1))
    strcpy(cwdbuf, "./");
  std::string cmdfifo = rps_fifo_prefix+".cmd";
  std::string outfifo = rps_fifo_prefix+".out";
  if (!rps_is_fifo(cmdfifo))
    {
      if (mkfifo(cmdfifo.c_str(), 0660)<0)
        RPS_FATALOUT("failed to create command FIFO " << cmdfifo << ":" << strerror(errno));
      RPS_INFORMOUT("created command FIFO " << cmdfifo
                    << " cwd:" << cwdbuf << " pid "
                    << (int)getpid() << " on " << rps_hostname()
                    << " git " << rps_shortgitid);
      rmatex = true;
    }
  cmdfd = open(cmdfifo.c_str(), 0660 | O_CLOEXEC);
  if (cmdfd<0)
    RPS_FATALOUT("failed to open command FIFO " << cmdfifo << ":" << strerror(errno));
  if (!rps_is_fifo(outfifo))
    {
      if (mkfifo(outfifo.c_str(), 0660)<0)
        RPS_FATALOUT("failed to create output FIFO " << outfifo << ":" << strerror(errno));
      RPS_INFORMOUT("created output FIFO " << outfifo
                    << " cwd:" << cwdbuf << " pid "
                    << (int)getpid() << " on " << rps_hostname()
                    << " git " << rps_shortgitid);
      rmatex = true;
    }
  outfd = open(outfifo.c_str(), 0440 | O_CLOEXEC);
  if (outfd<0)
    RPS_FATALOUT("failed to open output FIFO " << outfifo << ":" << strerror(errno));
  if (rmatex)
    atexit(rps_remove_fifos);
  rps_fifo_pair.fifo_ui_wcmd = cmdfd;
  rps_fifo_pair.fifo_ui_rout = outfd;
  RPS_INFORMOUT("RefPerSys did create (in pid " << (int)getpid()
                << ") command and output FIFOs to communicate with GUI" << std::endl
                << "... using for written commands to GUI " << cmdfifo << " fd#" << cmdfd
                << std::endl
                << "... and for reading JSON output from GUI " << outfifo << " fd#" << outfd);
  usleep(1000);
} // end rps_do_create_fifos


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
    outs << linprefix
         << "Copyright (C) "
         << RPS_INITIAL_COPYRIGHT_YEAR
         << " - "
         << (nowtm.tm_year + 1900)
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


void
rps_emit_lgplv3_copyright_notice(std::ostream&outs, std::string path, std::string linprefix, std::string linsuffix, std::string owner)
{
  outs << linprefix
       << "GENERATED file " << path  << " / DO NOT EDIT!"
       << linsuffix << std::endl;
  outs << linprefix
       << "This file is generated by the RefPerSys software see refpersys.org and contact team@refpersys.org "
       << linsuffix << std::endl;
  {
    time_t nowtime = time(nullptr);
    struct tm nowtm = {};
    localtime_r(&nowtime, &nowtm);
    outs << linprefix
         << "Copyright (C) "
         << RPS_INITIAL_COPYRIGHT_YEAR
         << " - "
         << (nowtm.tm_year + 1900)
         << ((owner.empty()) ? "The Reflective Persistent Team" : owner.c_str());
    outs << linsuffix << std::endl;
  }
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This file is free software: you can redistribute it and/or modify"
       << linsuffix << std::endl;
  outs << linprefix << "it under the terms of the GNU Lesser General Public License as"
       << linsuffix << std::endl;
  outs << linprefix << "published by the Free Software Foundation, either version 3 of the"
       << linsuffix << std::endl;
  outs << linprefix << "License, or (at your option) any later version."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This file is distributed in the hope that it will be useful,"
       << linsuffix << std::endl;
  outs << linprefix << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
       << linsuffix << std::endl;
  outs << linprefix << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
       << linsuffix << std::endl;
  outs << linprefix << "GNU Lesser General Public License for more details."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "You should have received a copy of the GNU "
       "Lesser General Public License"
       << linsuffix << std::endl;
  outs << linprefix << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
       << linsuffix << std::endl;
} // end rps_emit_lgplv3_copyright_notice


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
  EXPLAIN_TYPE(std::ostream);
  EXPLAIN_TYPE(std::ostringstream);
  EXPLAIN_TYPE(FILE);
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
  {
    /// https://lists.gnu.org/archive/html/lightning/2023-08/msg00004.html
    /// see also lightgen_rps.cc file
    printf(TYPEFMT_rps " %5d %5d\n","GNU lightning jit_state",
           rps_gnulightning_jitstate_size, rps_gnulightning_jitstate_align);
  }
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



/// this rps_extend_env is called early from main.  It is extending
/// the Unix environment.
void
rps_extend_env(void)
{
  static std::atomic<bool> extended;
  if (extended) return;
  RPS_ASSERT(rps_is_main_thread());
  extended = true;
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
  if (!rps_fifo_prefix.empty())
    {
      snprintf(fifoenv, sizeof(fifoenv), "REFPERSYS_FIFO_PREFIX=%s", rps_fifo_prefix.c_str());
      putenv(fifoenv);
    }
} // end rps_extend_env




void
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




////////////////////////////////////////////////////////////////

static double rps_start_monotonic_time;
static double rps_start_wallclock_real_time;



/// rps_early_initialization is called by rps_parse_program_arguments which is called early from main.
static void
rps_early_initialization(int argc, char** argv)
{
  rps_argc = argc;
  rps_argv = argv;
  rps_progname = argv[0];
  rps_stderr_istty = isatty(STDERR_FILENO);
  rps_stdout_istty = isatty(STDOUT_FILENO);
  rps_start_monotonic_time = rps_monotonic_real_time();
  rps_start_wallclock_real_time = rps_wallclock_real_time();
  if (uname (&rps_utsname))
    {
      fprintf(stderr, "%s: pid %d on %s failed to uname (%s:%d git %s): %s\n", rps_progname,
              (int) getpid(), rps_hostname(), __FILE__, __LINE__, RPS_SHORTGITID,
              strerror(errno));
      syslog(LOG_ERR,  "%s: pid %d on %s failed to uname (%s:%d git %s): %s\n", rps_progname,
             (int) getpid(), rps_hostname(), __FILE__, __LINE__, RPS_SHORTGITID,
             strerror(errno));
      exit(EXIT_FAILURE);
    };
  /// dlopen to self
  rps_proghdl = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
  if (!rps_proghdl)
    {
      char *err = dlerror();
      fprintf(stderr, "%s failed to dlopen whole program (%s)\n", rps_progname,
              err);
      syslog(LOG_ERR, "%s failed to dlopen whole program (%s)\n", rps_progname,
             err);
      exit(EXIT_FAILURE);
    };
  // initialize GNU lightning
  init_jit (rps_progname);
  rps_main_thread_handle = pthread_self();
  {
    char cwdbuf[rps_path_byte_size];
    memset (cwdbuf, 0, sizeof(cwdbuf));
    char tmbfr[64];		// the time buffer string
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
        else if (!strcmp(argv[ix], "--daemon"))
          {
            rps_daemonized = true;
            rps_syslog_enabled = true;
          }
        else if (!strcmp(argv[ix], "--syslog"))
          rps_syslog_enabled = true;
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
  RPS_INFORMOUT("done early initialization of RefPerSys process "
                << (int)getpid() << " on host " << rps_hostname()
                << " git " << rps_shortgitid);
} // end rps_early_initialization

////////////////////////////////////////////////////////////////
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
      rps_put_fifo_prefix(arg);
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
          if (strlen(rhomrp) >= rps_path_byte_size -1)
            RPS_FATAL("too long realpath %s on given --refpersys-home %s - %m",
                      rhomrp, arg);
          strncpy(rps_bufpath_homedir, rhomrp, rps_path_byte_size -1);
          free (rhomrp), rhomrp = nullptr;
          RPS_INFORMOUT("set RefPerSys home directory to " << rps_bufpath_homedir);
        };
    }
    return 0;
    case RPSPROGOPT_RUN_DELAY:
    {
      int pos= -1;
      long dl= -1;
      /// example argument: --run-delay=45s for elapsed seconds
      if ((pos= -1), sscanf(arg, "%li s%n", &rps_run_delay, &pos) > 0
          && rps_run_delay>0 && pos>0)
        RPS_INFORMOUT("RefPerSys will run its agenda for "
                      <<  rps_run_delay
                      << " elapsed seconds.");
      ///
      /// example argument: --run-delay=10m for elapsed minutes
      else if ( ((pos= -1), (dl=0)),
                sscanf(arg, "%li m%n", &dl, &pos) > 0
                && dl>0 && pos>0)
        {
          rps_run_delay = dl*60;
          RPS_INFORMOUT("RefPerSys will run its agenda for "
                        << dl << " minutes so "
                        << rps_run_delay << " elapsed seconds");
        }

      ///
      /// example argument: --run-delay=3h for elapsed hours
      else if ( ((pos= -1), (dl=0)),
                sscanf(arg, "%li h%n", &dl, &pos) > 0
                && dl>0 && pos>0)
        {
          rps_run_delay = dl*3600;
          RPS_INFORMOUT("RefPerSys will run its agenda for "
                        << dl << " hours so " << rps_run_delay
                        << " elapsed seconds");
        }
      else
        RPS_FATAL("invalid --run-delay=%s argument.\n"
                  "\t (should be like 90s or 20m or 2h)",
                  arg);
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
      if (side_effect && !rps_syslog_enabled)
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
    case RPSPROGOPT_DAEMON:
    {
      rps_without_terminal_escape = true;
      char cwdbuf[rps_path_byte_size];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      if (side_effect)
        {
          if (!rps_syslog_enabled)
            {
              rps_syslog_enabled = true;
              openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
            };
          RPS_INFORM("using syslog with daemon");
          if (daemon(/*nochdir:*/1,
                                 /*noclose:*/0))
            RPS_FATAL("failed to daemon");
          rps_daemonized = true;
          const char*cw = getcwd(cwdbuf, sizeof(cwdbuf)-1);
          RPS_INFORM("daemonized pid %d in dir %s git %s",
                     (int)getpid(), cw, rps_shortgitid);
        };
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
      if (side_effect)
        {
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
          && isalpha(extraname[0]))
        {
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
      for (Rps_Plugin curplugin: rps_plugins_vector)
        {
          std::string curplugname = curplugin.plugin_name;
          int pluglenam= curplugname.length();
          if (pluglenam > 4 && curplugname.substr(pluglenam-3) == ".so")
            curplugname.erase(pluglenam-3);
          RPS_DEBUG_LOG (REPL, "plugin#" << plugcnt << " is named " << Rps_QuotedC_String(curplugname));
          if (curplugname == plugname)
            {
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
          std::cout << "RefPerSys, an open source Artificial Intelligence system" << std::endl;
          std::cout << " symbolic inference engine - work in progress..." << std::endl;
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
                    << " Gnu multi-precision library version: " << gmp_version << std::endl
                    << " at: " << rps_gnubison_realpath
                    << std::endl
                    << " GPP generic preprocessor: "
                    << rps_gpp_command
                    << " version: " << rps_gpp_version
                    << " at: " << rps_gpp_realpath
                    << std::endl
                    << " default GUI script: " << rps_gui_script_executable << std::endl
                    << " Read Eval Print Loop: " << rps_repl_version() << std::endl
                    << " libCURL for web client: " << rps_curl_version() << std::endl
                    << " JSONCPP: " << JSONCPP_VERSION_STRING << std::endl
                    << " made with: " << rps_makefile << std::endl
                    << " running on " << rps_hostname() << std::endl
                    << "This " << ((rps_is_link_time_optimized>0)?"link-time-optimized":"normal") << " executable was built by "
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



// rps_parse_program_arguments is called very early from main...
void
rps_parse_program_arguments(int &argc, char**argv)
{
  errno = 0;
  rps_early_initialization(argc, argv);
  errno = 0;
  struct argp_state argstate;
  memset (&argstate, 0, sizeof(argstate));
  argparser_rps.options = rps_progoptions; // defined in main_rps.cc
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
  int aix= -1;
  if (argp_parse(&argparser_rps, argc, argv, 0, &aix, nullptr))
    RPS_FATALOUT("failed to parse program arguments to " << argv[0]
                 << " at program argument index aix=" << aix);
  RPS_POSSIBLE_BREAKPOINT();
} // end rps_parse_program_arguments


void
rps_output_program_arguments(std::ostream& out, int argc, const char*const*argv)
{
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
              || *pc=='/' || *pc=='.' || *pc==',' || *pc==':')
            continue;
          else
            {
              goodchar = false;
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

void
rps_postponed_remove_file(const std::string& path)
{
  std::lock_guard<std::mutex> gu(rps_postponed_lock);
  if (rps_postponed_removed_files_vector.empty())
    atexit(rps_schedule_files_postponed_removal);
  rps_postponed_removed_files_vector.push_back(std::string(path));
} // end rps_postponed_remove_file

void
rps_schedule_files_postponed_removal(void)
{
  std::lock_guard<std::mutex> gu(rps_postponed_lock);
  if (rps_postponed_removed_files_vector.empty())
    return;
  FILE* pat = popen("/bin/at now + 5 minutes", "w");
  if (!pat)
    {
      RPS_WARNOUT("failed to open /bin/at now + 5 minutes :" << strerror(errno));
      return;
    };
  if (rps_syslog_enabled)
    syslog(LOG_NOTICE, "RefPerSys will later remove %d files (in five minutes, with /bin/at)", (int) rps_postponed_removed_files_vector.size());
  else
    RPS_INFORM("RefPerSys will later remove %d files (in five minutes, with /bin/at)", (int) rps_postponed_removed_files_vector.size());
  for  (auto rf: rps_postponed_removed_files_vector)
    {
      if (rps_syslog_enabled)
        syslog(LOG_NOTICE, "*rm  %s",  Rps_QuotedC_String(rf).c_str());
      else
        printf(" *rm %s\n", Rps_QuotedC_String(rf).c_str());
      fprintf(pat, "/bin/rm -f %s\n", Rps_QuotedC_String(rf).c_str());
    }
  rps_postponed_removed_files_vector.clear();
  pclose(pat);
} // end rps_schedule_files_postponed_removal

////////////////
void
rps_fatal_stop_at (const char *filnam, int lin)
{
  static constexpr int skipfatal=2;
  assert(filnam != nullptr);
  assert (lin>=0);
  char errbuf[80];
  memset (errbuf, 0, sizeof(errbuf));
  char cwdbuf[rps_path_byte_size];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  if (!getcwd(cwdbuf, sizeof(cwdbuf)) || cwdbuf[0] == (char)0)
    strcpy(cwdbuf, "./");
  snprintf (errbuf, sizeof(errbuf), "FATAL STOP (%s:%d)", filnam, lin);
  /* we always syslog.... */
  syslog(LOG_EMERG, "RefPerSys fatal stop (%s:%d) git %s build %s pid %d on %s, elapsed %.3f, process %.3f secin %s",
         filnam, lin, rps_shortgitid, rps_timestamp, (int)getpid(), rps_hostname(),
         rps_elapsed_real_time(), rps_process_cpu_time(), cwdbuf);
  bool ontty = isatty(STDERR_FILENO);
  if (rps_debug_file)
    fprintf(rps_debug_file, "\n*§*§* RPS FATAL %s:%d *§*§*\n", filnam, lin);
  if (!rps_syslog_enabled)
    fprintf(stderr, "\n" "%s%sRPS FATAL:%s\n"
            " RefPerSys gitid %s,\n"
            "\t built timestamp %s,\n"
            "\t on host %s, md5sum %s,\n"
            "\t elapsed %.3f, process %.3f sec in %s\n",
            ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
            ontty?RPS_TERMINAL_BLINK_ESCAPE:"",
            ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
            rps_gitid, rps_timestamp, rps_hostname(), rps_md5sum,
            rps_elapsed_real_time(), rps_process_cpu_time(), cwdbuf);
  if (rps_debug_file && rps_debug_file != stderr && rps_debug_path[0])
    {
      fprintf(stderr, "*°* see debug output in %s\n", rps_debug_path);
      fprintf(rps_debug_file, "RefPerSys gitid %s built %s was started on %s pid %d as:\n",
              rps_shortgitid, rps_timestamp, rps_hostname(), (int)getpid());
      for (int aix=0; aix<rps_argc; aix++)
        {
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
  if (rps_syslog_enabled)
    {
      std::ostringstream outl;
      auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                                 filnam, lin,
                                 skipfatal, "RefPerSys FATAL ERROR",
                                 &outl);
      backt.output(outl);
      outl << "===== end fatal error at " << filnam << ":" << lin
           << " ======" << std::endl << std::flush;
      outl << "RefPerSys gitid " << rps_shortgitid << " built " << rps_timestamp
           << " was started on " << rps_hostname() << " pid " << (int)getpid() << " as:" << std::endl;
      for (int aix=0; aix<rps_argc; aix++)
        {
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'  || curarg[0]=='-';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            outl << ' ' << curarg;
          else
            outl << ' ' << Rps_QuotedC_String(curarg);
        }
      outl << std::endl << std::flush;
      syslog(LOG_EMERG, "RefPerSys fatal from %s", outl.str().c_str());
    }
  else
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
      for (int aix=0; aix<rps_argc; aix++)
        {
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
  rps_schedule_files_postponed_removal();
  abort();
} // end rps_fatal_stop_at


void rps_debug_warn_at(const char*file, int line)
{
  if (rps_syslog_enabled)
    {
      syslog(LOG_WARNING, "** REFPERSYS WARNING AT %s:%d (git %s pid %d) **", file, line,
             rps_shortgitid, (int)getpid());
    }
  else
    {
      std::cerr << std::flush;
      std::cerr << std::endl << "**!** REFPERSYS WARNING at " << file << ":" << line << std::endl;
    };
  if (rps_debug_file)
    {
      fprintf(rps_debug_file, "\n*** REFPERSYS WARNING at %s:%d ***\n", file, line);
      fflush(rps_debug_file);
    }
} // end rps_debug_warn_at


////////////////////////////////////////////////////////////////
// TIME ROUTINES
////////////////////////////////////////////////////////////////

double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
}

double rps_get_start_wallclock_real_time()
{
  return rps_start_wallclock_real_time;
}


void
rps_print_objectref(Rps_ObjectRef ob)
{
  std::cout << ob << std::endl;
} // end rps_print_objectref

////////////////////////////////////////////////////////////////
///// global roots for garbage collection and persistence

static std::set<Rps_ObjectRef> rps_object_root_set;
static std::mutex rps_object_root_mtx;
static std::unordered_map<Rps_Id,Rps_ObjectRef*,Rps_Id::Hasher> rps_object_global_root_hashtable;

void
rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (auto ob: rps_object_root_set)
    fun(ob);
} // end rps_each_root_object


void
rps_add_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_root_set.insert(ob);
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      *(rootit->second) = ob;
  }
} // end rps_add_root_object


bool
rps_remove_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  if (it == rps_object_root_set.end())
    return false;
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      (*(rootit->second)) = Rps_ObjectRef(nullptr);
  }
  rps_object_root_set.erase(it);
  return true;
} // end rps_remove_root_object

void
rps_initialize_roots_after_loading (Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_global_root_hashtable.max_load_factor(3.5);
  rps_object_global_root_hashtable.reserve(5*rps_hardcoded_number_of_roots()/4+3);
#define RPS_INSTALL_ROOT_OB(Oid) {		\
    const char*end##Oid = nullptr;		\
    bool ok##Oid = false;			\
    Rps_Id id##Oid(#Oid, &end##Oid, &ok##Oid);	\
    RPS_ASSERT (end##Oid && !*end##Oid);	\
    RPS_ASSERT (ok##Oid);			\
    RPS_ASSERT (id##Oid.valid());		\
    rps_object_global_root_hashtable[id##Oid]	\
      = &RPS_ROOT_OB(Oid);			\
  };
#include "generated/rps-roots.hh"
} // end of rps_initialize_roots_after_loading

bool rps_is_root_object (const Rps_ObjectRef ob)
{
  if (!ob)
    return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  return it != rps_object_root_set.end();
} // end rps_is_root_object

std::set<Rps_ObjectRef>
rps_set_root_objects(void)
{
  std::set<Rps_ObjectRef> set;

  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (Rps_ObjectRef ob: rps_object_root_set)
    set.insert(ob);
  return set;
} // end rps_set_root_objects

unsigned
rps_nb_root_objects(void)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  return (unsigned) rps_object_root_set.size();
} // end rps_nb_root_objects



void
rps_initialize_symbols_after_loading(Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::recursive_mutex> gu(Rps_PayloadSymbol::symb_tablemtx);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.max_load_factor(2.5);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.reserve(5*rps_hardcoded_number_of_symbols()/4+3);
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name) {		\
    Rps_PayloadSymbol::symb_hardcoded_hashtable[#Name]	\
      = &RPS_SYMB_OB(Name);				\
  };
#include "generated/rps-names.hh"
} // end of rps_initialize_symbols_after_loading

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
      fprintf(stderr, "Comma separated debugging levels with -D<debug-level>\n"
              "\tor --debug=<debug-level> or --debug-after-load=<debug-level>:\n");

#define Rps_SHOW_DEBUG(Opt) fprintf(stderr, "\t%s\n", #Opt);
      RPS_DEBUG_OPTIONS(Rps_SHOW_DEBUG);
#undef Rps_SHOW_DEBUG
      fflush(nullptr);
    }
  else if (deblev.empty())
    {
      RPS_WARNOUT("empty debugging from " << RPS_FULL_BACKTRACE_HERE(1, "rps_set_debug/empty"));
    }
  else if (isdigit(deblev[0]))
    {
      char*pend = nullptr;
      long lev = strtol(&deblev[0], &pend, 0);
      if (pend && *pend != (char)0)
        RPS_WARNOUT("bad numerical debug level " << lev << " in " << deblev);
      rps_debug_flags = lev;
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

void
rps_output_debug_flags(std::ostream&out,  unsigned flags)
{
  if (!flags)
    flags = rps_debug_flags.load();
  out << flags << "=" ;
  int nbf = 0;
  //
#define SHOW_DBGFLAG(Lev)			\
  do {						\
    if (flags & (1<< RPS_DEBUG_##Lev)) {	\
      if (nbf > 0)				\
	out << ',';				\
      out << #Lev;				\
      nbf++;					\
    }						\
  } while(0);
  RPS_DEBUG_OPTIONS(SHOW_DBGFLAG);
#undef SHOW_DBGFLAG
  out << std::flush;
} // end rps_output_debug_flags



//// end of file utilities_rps.cc

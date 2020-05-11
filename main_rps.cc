/****************************************************************
 * file main_rps.cc
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
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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
 ******************************************************************************/

#include "refpersys.hh"
#include "qthead_qrps.hh"


extern "C" const char rps_main_gitid[];
const char rps_main_gitid[]= RPS_GITID;

extern "C" const char rps_main_date[];
const char rps_main_date[]= __DATE__;

struct backtrace_state* rps_backtrace_common_state;
const char* rps_progname;

void* rps_proghdl;

bool rps_batch = false;
bool rps_disable_aslr = false;
bool rps_without_terminal_escape = false;

bool rps_syslog_enabled = false;
bool rps_stdout_istty = false;
bool rps_stderr_istty = false;

unsigned rps_debug_flags;

thread_local Rps_Random Rps_Random::_rand_thr_;

typedef std::function<void(void)> rps_todo_func_t;
static std::vector<rps_todo_func_t> rps_main_todo_vect;


const char*
rps_hostname(void)
{
  static char hnambuf[64];
  if (RPS_UNLIKELY(!hnambuf[0]))
    gethostname(hnambuf, sizeof(hnambuf)-1);
  return hnambuf;
} // end rps_hostname




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
         << " see http://refpersys.org/ and contact team@refpersys.org for more."
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
  outs << linprefix << "You should have received a copy of the GNU General Public License"
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
  EXPLAIN_TYPE(QColor);
  EXPLAIN_TYPE(QProcess);
  EXPLAIN_TYPE(QString);
  EXPLAIN_TYPE(QTextCharFormat);
  EXPLAIN_TYPE(QTextCursor);
  EXPLAIN_TYPE(QTextDocument);
  EXPLAIN_TYPE(QTextEdit);
  EXPLAIN_TYPE(QTextFragment);
  ///
  EXPLAIN_TYPE(RpsQApplication);
  EXPLAIN_TYPE(RpsQOutputTextDocument);
  EXPLAIN_TYPE(RpsQOutputTextEdit);
  EXPLAIN_TYPE(RpsQWindow);
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
  EXPLAIN_TYPE(Rps_PayloadQt<QTextEdit>);
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
} // end rps_print_types_info




////////////////////////////////////////////////////////////////
// TIME ROUTINES
////////////////////////////////////////////////////////////////

int rps_nbjobs = RPS_NBJOBS_MIN + 1;

static double rps_start_monotonic_time;
double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
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
      std::string curpathstr(*curpath);
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
                    << ", so consider rebuilding with omake");
    }
  char makecmd [128];
  memset (makecmd, 0, sizeof(makecmd));
  if (snprintf(makecmd, sizeof(makecmd), "make -t -C %s -q objects", rps_topdirectory) < (int)sizeof(makecmd)-1)
    {
      int bad = system(makecmd);
      if (bad)
        RPS_WARNOUT("rps_check_mtime_files: " << makecmd
                    << " failed with status# " << bad);
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
  if (!bfr || !fmt || !len)
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

      char minibfr[16];
      memset(minibfr, 0, sizeof (minibfr));
      assert(fraction >= 0.0 && fraction < 1.0);

      snprintf(minibfr, sizeof (minibfr), "%.02f", fraction);
      const char* dotminib = strchr(minibfr, '.');
      if (dotminib && dotminib<minibfr+sizeof(minibfr)-4)
        {
          strncpy(dotdunder, dotminib, 3);
        }
    }

  return bfr;
} // end rps_strftime_centiseconds






////////////////////////////////////////////////////////////////
int
main (int argc, char** argv)
{
  rps_start_monotonic_time = rps_monotonic_real_time();
  rps_stderr_istty = isatty(STDERR_FILENO);
  rps_stdout_istty = isatty(STDOUT_FILENO);
  RPS_ASSERT(argc>0);
  rps_progname = argv[0];
  /// disable ASLR programmatically if --no-aslr is passed ; this
  /// should ease low-level debugging with GDB
  /// https://en.wikipedia.org/wiki/Address_space_layout_randomization
  /// see https://askubuntu.com/a/507954/64680
  rps_disable_aslr = false;
  {
    for (int ix=1; ix<argc; ix++)
      if (!strcmp(argv[ix], "--no-aslr"))
        rps_disable_aslr = true;
    if (rps_disable_aslr)
      {
        if (personality(ADDR_NO_RANDOMIZE) == -1)
          RPS_FATAL("%s failed to disable ASLR: %m", rps_progname);
      }
  }
  rps_proghdl = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
  if (!rps_proghdl)
    {
      fprintf(stderr, "%s failed to dlopen whole program (%s)\n", rps_progname,
              dlerror());
      exit(EXIT_FAILURE);
    };
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
  QLocale::setDefault(QLocale::c());
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
  for (int aix=1; aix<argc; aix++)
    if (!strcmp(argv[aix], "--without-terminal"))
      rps_without_terminal_escape = true;
  ///
  if (rps_syslog_enabled && rps_debug_flags != 0)
    openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
  ///
  RPS_INFORM("%s%s" "!-!-! starting RefPerSys !-!-!" "%s" " %s process %d on host %s\n"
             "... gitid %.16s built %s (main@%p)",
             RPS_TERMINAL_BOLD_ESCAPE, RPS_TERMINAL_BLINK_ESCAPE,
             RPS_TERMINAL_NORMAL_ESCAPE,
             argv[0], (int)getpid(), rps_hostname(), rps_gitid, rps_timestamp,
             (void*)main);
  ////
  Rps_QuasiZone::initialize();
  rps_check_mtime_files();
  //// FIXME: should have some real code here
  rps_run_application(argc, argv);
  RPS_INFORM("end of RefPerSys process %d on host %s\n"
             "... gitid %.16s built %s elapsed %.3f sec, process %.3f sec",
             (int)getpid(), rps_hostname(), rps_gitid, rps_timestamp,
             rps_elapsed_real_time(), rps_process_cpu_time());
  return 0;
} // end of main




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
  fprintf(stderr, "\n%s%sRPS FATAL:%s\n"
          " RefPerSys gitid %s, built timestamp %s,\n"
          "\t on host %s, md5sum %s, elapsed %.3f, process %.3f sec\n",
          ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
          ontty?RPS_TERMINAL_BLINK_ESCAPE:"",
          ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
          rps_gitid, rps_timestamp, rps_hostname(), rps_md5sum,
          rps_elapsed_real_time(), rps_process_cpu_time());
  fflush(stderr);
  {
    auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                               filnam, lin,
                               skipfatal, "RefPerSys FATAL ERROR",
                               &std::clog);
    backt.output(std::clog);
    std::clog << "===== end fatal error at " << filnam << ":" << lin << " ======" << std::endl << std::flush;
  }
  fflush(nullptr);
  abort();
} // end rps_fatal_stop_at


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


////////////////////////////////////////////////////////////////

void
rps_debug_printf_at(const char *fname, int fline, Rps_Debug dbgopt,
                    const char *fmt, ...)
{
  char threadbfr[24];
  memset(threadbfr, 0, sizeof (threadbfr));
  pthread_getname_np(pthread_self(), threadbfr, sizeof (threadbfr) - 1);
  fflush(nullptr);
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
    char datebfr[48];
    memset(datebfr, 0, sizeof (datebfr));
    //
#define RPS_DEBUG_DATE_PERIOD 64
    if (ndbg % RPS_DEBUG_DATE_PERIOD == 0)
      {
        rps_now_strftime_centiseconds_nolen(datebfr, "%Y-%b-%d@%H:%M:%s.__ %Z");
      }
    //
    if (rps_syslog_enabled)
      {
        syslog(RPS_DEBUG_LOG_LEVEL, "RPS-DEBUG %7s <%s:%d> @%s:%d %s %s",
               rps_debug_level(dbgopt).c_str(), threadbfr,
               static_cast<int>(rps_thread_id()), fname, fline, tmbfr, msg);
      }
    else
      {
        // no syslog
        bool ontty = isatty(STDERR_FILENO);
        if (strchr(msg, '\n'))
          fputc('\n', stderr);
        fprintf(stderr, "%sRPS DEBUG %7s%s <%s:%d> %s@%s:%d%s %s\n%s\n",
                ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
                ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
                rps_debug_level(dbgopt).c_str(), threadbfr,
                static_cast<int>(rps_thread_id()),
                ontty?RPS_TERMINAL_ITALICS_ESCAPE:"",
                fname, fline,
                ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
                tmbfr, msg);
        fflush(stderr);
        //
        if (ndbg % RPS_DEBUG_DATE_PERIOD == 0)
          {
            fprintf(stderr, "%sRPS DEBUG %04ld ~ %s *^*^*%s\n",
                    ontty?RPS_TERMINAL_BOLD_ESCAPE:"", ndbg, datebfr,
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


/////////////////// end of file main_rps.cc

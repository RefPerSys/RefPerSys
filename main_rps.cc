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
 *      © Copyright 2019 The Reflective Persistent System Team
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

extern "C" const char rps_objects_date[];
const char rps_main_date[]= __DATE__;

struct backtrace_state* rps_backtrace_state;
const char* rps_progname;

void* rps_proghdl;

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

void rps_emit_gplv3_copyright_notice(std::ostream&outs, std::string path, std::string linprefix, std::string linsuffix)
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

/**** old obsolete code
enum rps_option_key_en
{
  Rps_Key_PrintRandomId = 1024,
  Rps_Key_Version,
  Rps_Key_ParseId,
  Rps_Key_ExplainTypes,
  Rps_Key_Deterministic,
  Rps_Key_PrimeAbove,
  Rps_Key_PrimeBelow,
  Rps_Key_ObjectTinyBenchmark1,
  Rps_Key_QtGui,
};

const struct argp_option rps_argopt_vec[] =
{
  {
    .name = "print-random-id",
    .key = Rps_Key_PrintRandomId,
    .arg = "count",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "print one or several random objectid[s]",
    .group = 0,
  },
  {
    .name = "deterministic",
    .key = Rps_Key_Deterministic,
    .arg = "seed",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "seed PRNGs in a deterministic way with given SEED",
    .group = 0,
  },
  {
    .name = "prime-above",
    .key = Rps_Key_PrimeAbove,
    .arg = "number",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "print a prime above a given number",
    .group = 0,
  },
  {
    .name = "prime-below",
    .key = Rps_Key_PrimeBelow,
    .arg = "number",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "print a prime below a given number",
    .group = 0,
  },
  {
    .name = "object-tinybenchmark1",
    .key = Rps_Key_ObjectTinyBenchmark1,
    .arg = "count",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "run the object tinybenchmark1 COUNT times",
    .group = 0,
  },
  {
    .name = "explain-types",
    .key = Rps_Key_ExplainTypes,
    .arg = NULL,
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "print information about types",
    .group = 0,
  },
  {
    .name = "version",
    .key = Rps_Key_Version,
    .arg = NULL,
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "print version information",
    .group = 0,
  },
  {
    .name = "parse-id",
    .key = Rps_Key_ParseId,
    .arg = "oid",
    .flags = 0,
    .doc = "parse a objectid",
    .group = 0,
  },
  {
    .name = "qt-gui",
    .key = Rps_Key_QtGui,
    .arg = NULL,
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "run Qt GUI",
    .group = 0,
  },
  { },
};
***/


void rps_print_types_info(void)
{
  printf("%-38s:   size  align   (in bytes)\n", "**TYPE**");
#define EXPLAIN_TYPE(Ty) printf("%-38s: %5d %5d\n", #Ty,	\
				(int)sizeof(Ty), (int)alignof(Ty))

#define EXPLAIN_TYPE2(Ty1,Ty2) printf("%-38s: %5d %5d\n",		\
				      #Ty1 "," #Ty2, 			\
				      (int)sizeof(Ty1,Ty2),		\
				      (int)alignof(Ty1,Ty2))

#define EXPLAIN_TYPE3(Ty1,Ty2,Ty3) printf("%-38s: %5d %5d\n",		\
					  #Ty1 "," #Ty2 "," #Ty3,	\
					  (int)sizeof(Ty1,Ty2,Ty3),	\
					  (int)alignof(Ty1,Ty2,Ty3))
  EXPLAIN_TYPE(int);
  EXPLAIN_TYPE(double);
  EXPLAIN_TYPE(char);
  EXPLAIN_TYPE(bool);
  EXPLAIN_TYPE(void*);
  EXPLAIN_TYPE(std::mutex);
  EXPLAIN_TYPE(std::shared_mutex);
  EXPLAIN_TYPE(std::recursive_mutex);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::recursive_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::string);
  EXPLAIN_TYPE2(std::map<Rps_ObjectRef, Rps_Value>);
  EXPLAIN_TYPE(QString);
  ///
  EXPLAIN_TYPE(RpsQApplication);
  EXPLAIN_TYPE(RpsQWindow);
  EXPLAIN_TYPE(Rps_BackTrace);
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
#undef EXPLAIN_TYPE
  putchar('\n');
  fflush(nullptr);
} // end rps_print_types_info




////////////////////////////////////////////////////////////////
int rps_nbjobs = RPS_NBJOBS_MIN + 1;

static double rps_start_monotonic_time;
double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
}

int
main(int argc, char** argv)
{
  rps_start_monotonic_time = rps_monotonic_real_time();
  RPS_ASSERT(argc>0);
  rps_progname = argv[0];
  rps_proghdl = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
  if (!rps_proghdl)
    {
      fprintf(stderr, "%s failed to dlopen whole program (%s)\n", rps_progname,
              dlerror());
      exit(EXIT_FAILURE);
    };
  rps_backtrace_state =
    backtrace_create_state(rps_progname, (int)true,
                           Rps_BackTrace::bt_error_cb,
                           nullptr);
  if (!rps_backtrace_state)
    {
      fprintf(stderr, "%s failed to make backtrace state.\n", rps_progname);
      exit(EXIT_FAILURE);
    }

  pthread_setname_np(pthread_self(), "rps-main");
  RPS_INFORM("starting RefPerSys %s process %d on host %s\n"
             "... gitid %.16s built %s",
             argv[0], (int)getpid(), rps_hostname(), rps_gitid, rps_timestamp);
  Rps_QuasiZone::initialize();
  //// FIXME: should have some real code here
  rps_run_application(argc, argv);
  RPS_INFORM("end of RefPerSys process %d on host %s\n"
             "... gitid %.16s built %s elapsed %.3f sec, process %.3f sec",
             (int)getpid(), rps_hostname(), rps_gitid, rps_timestamp,
             rps_elapsed_real_time(), rps_process_cpu_time());
  return 0;
} // end of main



/// notice that Rps_BackTrace should use assert, not RPS_ASSERT!
void
Rps_BackTrace::bt_error_method(const char*msg, int errnum)
{
  assert (msg != nullptr);
  fprintf(stderr, "BackTrace Error <%s:%d> %s (#%d)",
          __FILE__, __LINE__,
          msg?msg:"???",
          errnum);
  fflush(nullptr);
} // end Rps_BackTrace::bt_error_method



void
Rps_BackTrace::bt_error_cb (void *data, const char *msg,
                            int errnum)
{
  assert (data != nullptr);
  Rps_BackTrace* btp = static_cast<Rps_BackTrace*>(data);
  assert (btp->magicnum() == _bt_magicnum_);
  btp->bt_error_method(msg, errnum);
} // end of Rps_BackTrace::bt_error_cb



void
rps_print_simple_backtrace_level(Rps_BackTrace* btp, FILE*outf, const char*beforebuf, uintptr_t pc)
{
  if (btp == nullptr || btp->magicnum() != Rps_BackTrace::_bt_magicnum_)
    RPS_FATAL("bad btp@%p", (void*)btp);
  if (!outf)
    RPS_FATAL("missing outf");
  if (!beforebuf)
    beforebuf = "*";
  if (pc==0 || pc==(uintptr_t)-1)
    {
      fprintf(outf, "%s *********\n", beforebuf);
      return;
    }
  const char*demangled = nullptr;
  Dl_info dif = {};
  memset ((void*)&dif, 0, sizeof(dif));
  std::string filnamestr;
  std::string funamestr;
  if (dladdr((void*)pc, &dif))
    {
      int delta = pc - (uintptr_t) dif.dli_saddr;
      if (dif.dli_fname && strstr(dif.dli_fname, ".so"))
        filnamestr = std::string(basename(dif.dli_fname));
      else if (dif.dli_fname && strstr(dif.dli_fname, rps_progname))
        filnamestr = std::string(basename(rps_progname));
      if (dif.dli_sname != nullptr)
        {
          if (dif.dli_sname[0] == '_')
            {
              int status = -1;
              demangled  = abi::__cxa_demangle(dif.dli_sname, NULL, 0, &status);
              if (demangled && demangled[0])
                funamestr = std::string (demangled);
            };
          if (funamestr.empty())
            funamestr = std::string(dif.dli_sname);
        }
      else funamestr = "??";
      if (delta != 0)
        {
          if (funamestr.empty())
            fprintf (outf, "%s %p: %s+%#x\n",
                     beforebuf, (void*)pc, filnamestr.c_str(), delta);
          else
            fprintf (outf, "%s %p: %40s+%#x %s\n",
                     beforebuf, (void*)pc, filnamestr.c_str(), delta,
                     funamestr.c_str());
        }
      else
        {
          if (funamestr.empty())
            fprintf (outf, "%s %p: %s+%#x\n",
                     beforebuf, (void*)pc, filnamestr.c_str(), delta);
          else
            fprintf (outf, "%s %p: %40s+%#x %s\n",
                     beforebuf, (void*)pc, filnamestr.c_str(),
                     delta, funamestr.c_str());
        }
    }
  else
    fprintf(outf, "%s %p.\n", beforebuf, (void*)pc);
  if (demangled)
    free((void*)demangled), demangled = nullptr;
  fflush(outf);
} // end of rps_print_simple_backtrace_level




void rps_print_full_backtrace_level
(Rps_BackTrace* btp,
 FILE*outf, const char*beforebuf,
 uintptr_t pc, const char *filename, int lineno,
 const char *function)
{
  if (btp == nullptr || btp->magicnum() != Rps_BackTrace::_bt_magicnum_)
    RPS_FATAL("bad btp@%p", (void*)btp);
  if (!outf)
    RPS_FATAL("missing outf");
  if (!beforebuf)
    beforebuf = "*";
  if (pc==0 || pc==(uintptr_t)-1)
    {
      fprintf(outf, "%s *********\n", beforebuf);
      return;
    }
  std::string locstr;
  std::string funamestr;
  char locbuf[80];
  Dl_info dif = {};
  const char* demangled = nullptr;
  bool wantdladdr = false;
  memset ((void*)&dif, 0, sizeof(dif));
  if (function)
    {
      if (function[0] == '_')
        {
          int status = -1;
          demangled  = abi::__cxa_demangle(function, NULL, 0, &status);
          if (demangled && demangled[0])
            funamestr = std::string (demangled);
        }
      else funamestr = std::string (function);
    }
  else
    wantdladdr = true;
  memset(locbuf, 0, sizeof(locbuf));
  if (filename)
    {
      snprintf(locbuf, sizeof(locbuf), "%s:%d",
               basename(filename), lineno);
      locstr = std::string(locbuf);
    }
  else
    wantdladdr = true;
  if (wantdladdr && dladdr((void*)pc, &dif))
    {
      int delta = pc - (uintptr_t) dif.dli_saddr;
      if (locstr.empty() && dif.dli_fname && strstr(dif.dli_fname, ".so"))
        {
          if (delta != 0)
            snprintf(locbuf, sizeof(locbuf),
                     "!%s+%#x", basename(dif.dli_fname), delta);
          else snprintf(locbuf, sizeof(locbuf),
                          "!%s", basename(dif.dli_fname));
          locstr = std::string (locbuf);
        }
      else if (locstr.empty() && dif.dli_fname && strstr(dif.dli_fname, rps_progname))
        {
          if (delta != 0)
            snprintf(locbuf, sizeof(locbuf),
                     "!!%s+%#x", basename(dif.dli_fname), delta);
          else snprintf(locbuf, sizeof(locbuf),
                          "!!%s", basename(dif.dli_fname));
          locstr = std::string (locbuf);
        }
      if (funamestr.empty())
        {
          if (dif.dli_sname && dif.dli_sname[0] == '_')
            {
              int status = -1;
              demangled  = abi::__cxa_demangle(dif.dli_sname, NULL, 0, &status);
              if (demangled && demangled[0])
                funamestr = std::string (demangled);
            }
          else if (dif.dli_sname)
            funamestr = std::string(dif.dli_sname);
          if (funamestr.empty())
            {
              funamestr = std::string("?_?");
            }
        }
    };
  fprintf(outf, "%s %p %s %s\n",
          beforebuf, (void*)pc, funamestr.c_str(), locstr.c_str());
  fflush(outf);
  if (demangled)
    free((void*)demangled), demangled=nullptr;
} // end of rps_print_full_backtrace_level



int
Rps_BackTrace::bt_simple_method(uintptr_t ad)
{
  if (ad == 0 || ad == (uintptr_t)-1) return 0;
  if (_bt_simplecb)
    return _bt_simplecb(this,ad);
  rps_print_simple_backtrace_level(this,stderr,"*",ad);
  return 1;
} // end of  Rps_BackTrace::bt_simple_method

int
Rps_BackTrace::bt_simple_cb(void*data, uintptr_t pc)
{
  assert (data != nullptr);
  if (pc == 0 || pc == (uintptr_t)-1) return 1;
  Rps_BackTrace* btp = static_cast<Rps_BackTrace*>(data);
  assert (btp->magicnum() == _bt_magicnum_);
  return btp->bt_simple_method(pc);
} // end  Rps_BackTrace::bt_simple_cb


int
Rps_BackTrace::bt_full_method(uintptr_t pc,
                              const char *filename, int lineno,
                              const char *function)
{
  if (pc == 0 || pc == (uintptr_t)-1)
    return 1;
  if (_bt_fullcb)
    return _bt_fullcb(this,pc,filename,lineno,function);
  rps_print_full_backtrace_level(this,stderr,"*",
                                 pc,filename,lineno,function);
  return 0;
} // end Rps_BackTrace::bt_full_method

int
Rps_BackTrace::bt_full_cb(void *data, uintptr_t pc,
                          const char *filename, int lineno,
                          const char *function)
{
  assert (data != nullptr);
  if (pc == 0 || pc == (uintptr_t)-1) return 1;
  Rps_BackTrace* btp = static_cast<Rps_BackTrace*>(data);
  assert (btp->magicnum() == _bt_magicnum_);
  return btp->bt_full_method(pc, filename, lineno, function);
} // end Rps_BackTrace::bt_full_cb

Rps_BackTrace::Rps_BackTrace(const char*name, const void*data)
  : _bt_magic(_bt_magicnum_),
    _bt_name(name?name:"??"),
    _bt_simplecb(),
    _bt_fullcb(),
    _bt_data(data)
{
} // end of Rps_BackTrace::Rps_BackTrace

Rps_BackTrace::~Rps_BackTrace()
{
  assert (magicnum() == _bt_magicnum_);
  _bt_data = nullptr;
} // end Rps_BackTrace::~Rps_BackTrace


void
Rps_BackTrace::run_simple_backtrace(int skip, const char*name)
{
  Rps_BackTrace bt(name?name:"plain");
  int depthcount = 0;
  bt.set_simple_cb([&,stderr](Rps_BackTrace*btp, uintptr_t pc)
  {
    assert (btp != nullptr && btp-> magicnum() ==  _bt_magicnum_);
    depthcount++;
    if (depthcount > (int)_bt_maxdepth_)
      {
        fprintf(stderr, "\n.... etc .....\n");
        return depthcount;
      }
    if (depthcount %5 == 0)
      fputc('\n', stderr);
    char countbuf[16];
    memset (countbuf, 0, sizeof(countbuf));
    snprintf(countbuf, sizeof(countbuf), "[%d] ", depthcount);
    rps_print_simple_backtrace_level(btp, stderr, countbuf, pc);
    fflush(stderr);
    return 0;
  });
  fprintf(stderr, "SIMPLE BACKTRACE (%s)\n", bt.name().c_str());
  bt.simple_backtrace(skip);
  fprintf(stderr, "***** end of %d simple backtrace levels (%s) *****\n",
          depthcount, bt.name().c_str());
  fflush(stderr);
} // end Rps_BackTrace::run_simple_backtrace


void
Rps_BackTrace::run_full_backtrace(int skip, const char*name)
{
  Rps_BackTrace bt(name?name:"full");
  int depthcount = 0;
  bt.set_full_cb([&,stderr](Rps_BackTrace*btp, uintptr_t pc,
                            const char*filnam, int lineno, const char*funam)
  {
    assert (btp != nullptr && btp-> magicnum() ==  _bt_magicnum_);
    if (pc == 0 || pc == (uintptr_t)-1)
      return -1;
    depthcount++;
    if (depthcount > (int)_bt_maxdepth_)
      {
        fprintf(stderr, "\n.... etc .....\n");
        return depthcount;
      }
    if (depthcount %5 == 0)
      fputc('\n', stderr);
    char countbuf[16];
    memset (countbuf, 0, sizeof(countbuf));
    snprintf(countbuf, sizeof(countbuf), "[%d] ", depthcount);
    rps_print_full_backtrace_level(btp, stderr, countbuf,
                                   pc, filnam, lineno, funam);
    return 0;
  });
  char thnam[40];
  memset(thnam, 0, sizeof(thnam));
  pthread_getname_np(pthread_self(), thnam, sizeof(thnam)-1);
  fprintf(stderr, "FULL BACKTRACE (%s) <%s>\n", bt.name().c_str(), thnam);
  bt.full_backtrace(skip);
  fprintf(stderr, "***** end of %d full backtrace levels (%s) *****\n",
          depthcount, bt.name().c_str());
  fflush(stderr);
} // end Rps_BackTrace::run_full_backtrace



Rps_BackTrace_Helper::Rps_BackTrace_Helper(const char*fil, int line, int skip, const char*name)
  : _bth_magic(_bth_magicnum_), _bth_count(0),
    _bth_lineno(line), _bth_skip(skip),
    _bth_bufsiz(0),
    _bth_bufptr(nullptr),
    _bth_filename(fil),
    _bth_out(nullptr),
    _bth_backtrace(name,(void*)this)
{
  _bth_backtrace.set_full_cb
  ([=](Rps_BackTrace*btp, uintptr_t pc, const char*filnam, int lineno, const char*funam)
  {
    if (pc == 0 || pc == (uintptr_t)-1)
      return -1;
    if (!_bth_out) return -1;
    Rps_BackTrace_Helper*bth = reinterpret_cast<Rps_BackTrace_Helper*>(const_cast<void*>(btp->data()));
    FILE* foutlin = open_memstream(&_bth_bufptr, &_bth_bufsiz);
    if (!foutlin) RPS_FATAL("failed to open_memstream");
    assert (bth != nullptr && bth->has_good_magic());
    assert (btp == &bth->_bth_backtrace);
    bth->_bth_count++;
    char countbuf[16];
    memset (countbuf, 0, sizeof(countbuf));
    snprintf(countbuf, sizeof(countbuf), "[%d] ", bth->_bth_count);
    rps_print_full_backtrace_level(&bth->_bth_backtrace, foutlin, countbuf, pc, filnam, lineno, funam);
    fputc((char)0, foutlin);
    fflush(foutlin);
    *_bth_out << _bth_bufptr << std::endl;
    fclose(foutlin);
    return 0;
  });
} // end Rps_BackTrace_Helper::Rps_BackTrace_Helper

void
Rps_BackTrace_Helper::do_out(void) const
{
  _bth_backtrace.run_full_backtrace(_bth_skip);
} // end Rps_BackTrace_Helper::do_out

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
  assert(filnam != nullptr);
  assert (lin>=0);
  char errbuf[80];
  memset (errbuf, 0, sizeof(errbuf));
  snprintf (errbuf, sizeof(errbuf), "FATAL STOP (%s:%d)", filnam, lin);
  fprintf(stderr, "FATAL: RefPerSys gitid %s, built timestamp %s,\n"
          "\t on host %s, md5sum %s, elapsed %.3f, process %.3f sec\n",
          rps_gitid, rps_timestamp, rps_hostname(), rps_md5sum,
          rps_elapsed_real_time(), rps_process_cpu_time());
  fflush(stderr);
  Rps_BackTrace::run_full_backtrace(3, errbuf);
  fflush(nullptr);
  abort();
} // end rps_fatal_stop_at

/// each root object is also a public variable, define them
#define RPS_INSTALL_ROOT_OB(Oid) Rps_ObjectRef rps_rootob##Oid;
#include "generated/rps-roots.hh"


/// end of file main_rps.cc

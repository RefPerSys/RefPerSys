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
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io>
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

struct backtrace_state* rps_backtrace_state;
const char* rps_progname;

void* rps_proghdl;

thread_local Rps_Random Rps_Random::_rand_thr_;

enum rps_option_key_en
{
  Rps_Key_PrintRandomId = 1024,
  Rps_Key_Version,
  Rps_Key_ParseId,
  Rps_Key_ExplainTypes,
  Rps_Key_PrimeAbove,
  Rps_Key_PrimeBelow,
  Rps_Key_ObjectTinyBenchmark1,
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
    .name = "prime-above",
    .key = Rps_Key_PrimeAbove,
    .arg = "number",
    .flags = OPTION_ARG_OPTIONAL,
    .doc = "print a prime above a given number"
    " (see also the 'primes' program from BSD games)",
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
    .doc = "print version informatrion",
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
  { },
};


void print_types_info(void)
{
  printf("%-36s:   size  align   (in bytes)\n", "**TYPE**");
#define EXPLAIN_TYPE(Ty) printf("%-36s: %5d %5d\n", #Ty,	\
				(int)sizeof(Ty), (int)alignof(Ty))

#define EXPLAIN_TYPE2(Ty1,Ty2) printf("%-36s: %5d %5d\n",		\
				      #Ty1 "," #Ty2, 			\
				      (int)sizeof(Ty1,Ty2),		\
				      (int)alignof(Ty1,Ty2))

#define EXPLAIN_TYPE3(Ty1,Ty2,Ty3) printf("%-36s: %5d %5d\n",		\
					  #Ty1 "," #Ty2 "," #Ty3,	\
					  (int)sizeof(Ty1,Ty2,Ty3),	\
					  (int)alignof(Ty1,Ty2,Ty3))
  EXPLAIN_TYPE(int);
  EXPLAIN_TYPE(double);
  EXPLAIN_TYPE(char);
  EXPLAIN_TYPE(bool);
  EXPLAIN_TYPE(Rps_HashInt);
  EXPLAIN_TYPE(intptr_t);
  EXPLAIN_TYPE(Rps_Id);
  EXPLAIN_TYPE(Rps_Type);
  EXPLAIN_TYPE(Rps_ObjectRef);
  EXPLAIN_TYPE(Rps_ZoneValue);
  EXPLAIN_TYPE(Rps_MarkSweepZoneValue);
  EXPLAIN_TYPE(Rps_CopyingSizedZoneValue);
  EXPLAIN_TYPE(Rps_Value);
  EXPLAIN_TYPE(Rps_ObjectZone);
  EXPLAIN_TYPE(Rps_QuasiAttributeArray);
  EXPLAIN_TYPE(Rps_TupleObrefZone);
  EXPLAIN_TYPE(Rps_SetObrefZone);
  EXPLAIN_TYPE(Rps_DoubleZone);
  EXPLAIN_TYPE(Rps_StringZone);
  EXPLAIN_TYPE(Rps_PaylSetObjrefZone);
  ///
  EXPLAIN_TYPE(Rps_QuasiAttributeArray);
  EXPLAIN_TYPE(Rps_QuasiComponentVector);
  EXPLAIN_TYPE(Rps_QuasiToken);
  ///
  EXPLAIN_TYPE(Rps_MemoryBlock);
  EXPLAIN_TYPE(Rps_BirthMemoryBlock);
  EXPLAIN_TYPE(Rps_LargeNewMemoryBlock);
  EXPLAIN_TYPE(Rps_MarkedMemoryBlock);
  ///
  EXPLAIN_TYPE(std::set<intptr_t>);
  EXPLAIN_TYPE(std::set<std::string>);
  EXPLAIN_TYPE2(std::map<intptr_t,void*>);
  EXPLAIN_TYPE2(std::map<std::string,void*>);
  EXPLAIN_TYPE2(std::unordered_map<intptr_t,void*>);
  EXPLAIN_TYPE2(std::variant<std::string,void*>);
  EXPLAIN_TYPE(std::vector<intptr_t>);
  EXPLAIN_TYPE(std::atomic<intptr_t>);
  EXPLAIN_TYPE(std::string);
  EXPLAIN_TYPE(std::mutex);
#undef EXPLAIN_TYPE
  putchar('\n');
  fflush(nullptr);
} // end print_types_info




error_t rps_argopt_parse(int key, char*arg, struct argp_state*state)
{
  (void) arg;
  (void) state;
  switch (key)
    {
    case Rps_Key_Version:
    {
      printf("RefPerSys version information::\n");
      printf(" timestamp:     %s (%lu)\n", timestamp_rps, timenum_rps);
      printf(" gitid:         %s\n", gitid_rps);
      printf(" md5sum of C++: %s\n", md5sum_rps);
      printf(" build cwd:     %s\n", cwd_rps);
      printf(" build host:    %s\n", buildhost_rps);
      printf(" C++ compiler:  %s\n", cxxcompiler_rps);
      printf(" C++ sources:   %s\n", sourcefiles_rps);
      printf(" C++ headers:   %s\n", headerfiles_rps);
      putchar('\n');
    }
    break;
    case Rps_Key_PrintRandomId:
    {
      int count = 1;
      const char*countstr = arg;
      if (countstr && countstr[0])
        count = std::stoi(countstr);
      if (count > 1)
        printf("printing %d random id\n", count);
      for (int ix=0; ix<count; ix++)
        {
          auto rdid = Rps_Id::random();
          char cbuf[24];
          memset (cbuf,0,sizeof(cbuf));
          rdid.to_cbuf24(cbuf);
          printf("random object id#%d: %s (hi=%#llx,lo=%#llx) h=%u\n",
                 ix,
                 cbuf,
                 (unsigned long long)rdid.hi(), (unsigned long long)rdid.lo(),
                 (unsigned)rdid.hash());
        }
    }
    break;
    case Rps_Key_PrimeAbove:
    {
      long num = 0;
      const char*numstr = arg;
      if (numstr && numstr[0])
        num = std::stoi(numstr);
      long pr = rps_prime_above(num);
      printf("A prime above %ld is %ld (but see also primes program from BSD games)\n", num, pr);
    }
    break;
    case Rps_Key_PrimeBelow:
    {
      long num = 0;
      const char*numstr = arg;
      if (numstr && numstr[0])
        num = std::stoi(numstr);
      long pr = rps_prime_below(num);
      printf("A prime below %ld is %ld\n", num, pr);
    }
    break;
    case Rps_Key_ObjectTinyBenchmark1:
    {
      unsigned num = 0;
      constexpr unsigned minbench = 1000;
      const char*numstr = arg;
      if (numstr && numstr[0])
        num = std::stoi(numstr);
      if (num<minbench)
        num = minbench;
      printf("before running Rps_ObjectRef::tinybenchmark1 with count %u.\n", num);
      fflush(nullptr);
      Rps_ObjectRef::tiny_benchmark_1(nullptr, num);
    }
    break;
    case Rps_Key_ExplainTypes:
      print_types_info();
      break;
    case Rps_Key_ParseId:
    {
      const char*idstr = arg;
      const char*end = nullptr;
      bool ok = false;
      assert (idstr != nullptr);
      Rps_Id pid (idstr, &end, &ok);
      if (ok)
        {
          char cbuf[24];
          memset (cbuf,0,sizeof(cbuf));
          pid.to_cbuf24(cbuf);
          printf("parsed '%s' as object id %s (hi=%#llx,lo=%#llx) h=%u\n",
                 idstr, cbuf,
                 (unsigned long long)pid.hi(), (unsigned long long)pid.lo(),
                 (unsigned)pid.hash());
          if (end && *end)
            printf("trailing '%s' of '%s' is not parsed\n", end, idstr);
        }
      else fprintf(stderr, "failed to parse id %s\n", idstr);
    }
    break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
} // end rps_argopt_parse


int
main(int argc, char** argv)
{
  struct argp argopt =
  {
    .options = rps_argopt_vec,
    .parser = rps_argopt_parse,
    .args_doc = NULL,
    .doc = NULL,
    .children = NULL,
    .help_filter = NULL,
    .argp_domain = NULL
  };
  assert(argc>0);
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
  if (argc < 2)
    {
      printf("missing argument to %s, try %s --help\n", argv[0], argv[0]);
      exit(EXIT_FAILURE);
    }

  argp_parse (&argopt, argc, argv, 0, NULL, NULL);
  return 0;
} // end of main



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


int
Rps_BackTrace::bt_simple_method(uintptr_t ad, bool nofun)
{
  if (ad == 0) return 0;
  if (!nofun && _bt_simplecb)
    return _bt_simplecb(this,ad);
  Dl_info dif = {};
  memset ((void*)&dif, 0, sizeof(dif));
  if (dladdr((void*)ad, &dif))
    {
      int delta = (uintptr_t) dif.dli_saddr - ad;
      std::string fnamestr;
      if (strstr(dif.dli_fname, ".so"))
        fnamestr = std::string(basename(dif.dli_fname));
      if (delta != 0)
        {
          if (fnamestr.empty())
            fprintf (stderr, "%p: %s+%d\n",
                     (void*)ad, dif.dli_sname, delta);
          else
            fprintf (stderr, "%p: %40s+%d %s\n",
                     (void*)ad, dif.dli_sname, delta,
                     fnamestr.c_str());
        }
      else
        {
          if (fnamestr.empty())
            fprintf (stderr, "%p: %s+%d\n",
                     (void*)ad, dif.dli_sname, delta);
          else
            fprintf (stderr, "%p: %40s+%d %s\n",
                     (void*)ad, dif.dli_sname,
                     delta, fnamestr.c_str());
        }
    }
  else
    fprintf(stderr, "%p.\n", (void*)ad);
  fflush(stderr);
  return 0;
} // end of  Rps_BackTrace::bt_simple_method

int
Rps_BackTrace::bt_simple_cb(void*data, uintptr_t pc)
{
  assert (data != nullptr);
  if (pc == 0 || pc == (uintptr_t)-1) return 1;
  Rps_BackTrace* btp = static_cast<Rps_BackTrace*>(data);
  assert (btp->magicnum() == _bt_magicnum_);
  return btp->bt_simple_method(pc,true);
} // end  Rps_BackTrace::bt_simple_cb


int
Rps_BackTrace::bt_full_method(uintptr_t pc,
                              const char *filename, int lineno,
                              const char *function, bool nofun)
{
  if (pc == 0 || pc == (uintptr_t)-1)
    return 1;
  if (!nofun && _bt_fullcb)
    return _bt_fullcb(this,pc,filename,lineno,function);
  if (!filename) filename="?.?";
  if (!function) function="???";
  Dl_info dif = {};
  memset ((void*)&dif, 0, sizeof(dif));
  if (dladdr((void*)pc, &dif))
    {
      std::string fnamestr;
      if (strstr(dif.dli_fname, ".so"))
        fnamestr = std::string(basename(dif.dli_fname));
      if (!filename) filename="??";
      int delta = (uintptr_t) dif.dli_saddr - pc;
      if (delta != 0)
        {
          if (fnamestr.empty())
            fprintf (stderr, "%p: %s (%s:%d) %s+%d\n",
                     (void*)pc, function, filename, lineno, dif.dli_sname, delta);
          else
            fprintf (stderr, "%p:  %s (%s:%d) %40s+%d %s\n",
                     (void*)pc, function, filename, lineno, dif.dli_sname, delta,
                     fnamestr.c_str());
        }
      else
        {
          if (fnamestr.empty())
            fprintf (stderr, "%p: %s (%s:%d) %s+%d\n",
                     (void*)pc,function, filename, lineno,  dif.dli_sname, delta);
          else
            fprintf (stderr, "%p: %s (%s:%d) %40s+%d %s\n",
                     (void*)pc, function, filename, lineno,dif.dli_sname,
                     delta, fnamestr.c_str());
        }
    }
  else
    fprintf(stderr, "%p. (%s:%d) %s\n",
            (void*)pc, filename, lineno, function);
  fflush(stderr);
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
  return btp->bt_full_method(pc, filename, lineno, function, true);
} // end Rps_BackTrace::bt_full_cb

Rps_BackTrace::Rps_BackTrace(const char*name, const void*data)
  : _bt_magic(_bt_magicnum_),
    _bt_name(name?name:"??"),
    _bt_data(data), _bt_simplecb(), _bt_fullcb()
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
    fprintf(stderr, "[%d] ", depthcount);
    return btp->bt_simple_method(pc);
  });
  fprintf(stderr, "SIMPLE BACKTRACE (%s)\n", bt.name().c_str());
  bt.simple_backtrace(skip);
  fprintf(stderr, "***** end of %d simple backstrace (%s) *****\n",
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
    depthcount++;
    if (depthcount > (int)_bt_maxdepth_)
      {
        fprintf(stderr, "\n.... etc .....\n");
        return depthcount;
      }
    if (depthcount %5 == 0) fputc('\n', stderr);
    fprintf(stderr, "[%d] ", depthcount);
    return btp->bt_full_method(pc, filnam, lineno, funam);
  });
  fprintf(stderr, "FULL BACKTRACE (%s)\n", bt.name().c_str());
  bt.full_backtrace(skip);
  fprintf(stderr, "***** end of %d full backstrace (%s) *****\n",
          depthcount, bt.name().c_str());
  fflush(stderr);
} // end Rps_BackTrace::run_full_backtrace

void
rps_fatal_stop_at (const char *filnam, int lin)
{
  assert(filnam != nullptr);
  assert (lin>=0);
  char errbuf[80];
  memset (errbuf, 0, sizeof(errbuf));
  snprintf (errbuf, sizeof(errbuf), "FATAL STOP (%s:%d)", filnam, lin);
  fprintf(stderr, "FATAL: gitid %s, built timestamp %s,\n"
	  "\t host %s, md5sum %s\n",
          gitid_rps, timestamp_rps, buildhost_rps, md5sum_rps);
  fflush(stderr);
  Rps_BackTrace::run_full_backtrace(1, errbuf);
  fflush(nullptr);
  abort();
} // end rps_fatal_stop_at

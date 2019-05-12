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
 *      Niklas Rosencrantz <niklasro@gmail.com>
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

thread_local Rps_Random Rps_Random::_rand_thr_;

enum rps_option_key_en
{
  Rps_Key_PrintRandomId = 1024,
  Rps_Key_Version,
  Rps_Key_ParseId,
  Rps_Key_ExplainTypes,
  Rps_Key_PrimeAbove,
  Rps_Key_PrimeBelow,
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
#ifdef RPS_HAVE_MPS
  EXPLAIN_TYPE(mps_ss_t);
  EXPLAIN_TYPE(mps_arena_t);
  EXPLAIN_TYPE(mps_ap_t);
#endif /*RPS_HAVE_MPS*/
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
      printf(" timestamp:     %s (%ul)\n", timestamp_rps, timenum_rps);
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
  if (argc < 2)
    {
      printf("missing argument to %s, try %s --help\n", argv[0], argv[0]);
      exit(EXIT_FAILURE);
    }
  argp_parse (&argopt, argc, argv, 0, NULL, NULL);
  return 0;
} // end of main

void
rps_fatal_stop_at (const char *fil, int lin)
{
#warning rps_fatal_stop_at should show a backtrace
  // FIXME: should show a backtrace
  assert(fil != nullptr);
  assert (lin>=0);
  abort();
} // end rps_fatal_stop_at

/****************************************************************
 * file scripting_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It provides support for scripting (after loading the heap)
 *
 * Author(s):
 *      Basile STARYNKEVITCH (France) <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2025 - 2025 The Reflective Persistent System Team
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
 *********************************************************************/


#include "refpersys.hh"

// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG gmp
//@@PKGCONFIG gmpxx

extern "C" const char rps_scripting_gitid[];
const char rps_scripting_gitid[]= RPS_GITID;

extern "C" const char rps_scripting_date[];
const char rps_scripting_date[]= __DATE__;

extern "C" const char rps_scripting_shortgitid[];
const char rps_scripting_shortgitid[]= RPS_SHORTGITID;

extern "C" const char rps_scripting_timestamp[];
const char rps_scripting_timestamp[]= __TIMESTAMP__;


extern "C" void rps_scripting_help(void);
extern "C" void rps_scripting_add_script(const char*);

extern "C" const char rps_scripting_help_english_text[];


extern "C" void rps_run_one_script_file(Rps_CallFrame*, int ix);

extern "C" const int rps_script_maxnum = 1024;

/// vector of real path to script files
static std::vector<const char*> rps_scripts_vector;

const char rps_scripting_help_english_text[] =
  R"help(
A script file is a textual file.
All its initial lines before a line containing REFPERSYS_SCRIPT are ignored.
Hence these initial lines could contain some shell script, etc.
)help"
  ;
#warning more text needed inside rps_scripting_help_english_text

extern "C" const char  rps_scripting_magic_string[];

const char rps_scripting_magic_string[] = "REFPERSYS_SCRIPT";
#define RPS_SCRIPT_MAGIC_STR "REFPERSYS_SCRIPT"


void
rps_scripting_help(void)
{
  RPS_FATALOUT("unimplemented rps_scripting_help" << std::endl
               << rps_scripting_help_english_text);
#warning rps_scripting_help unimplemented
} // end rps_scripting_help


void
rps_scripting_add_script(const char*path)
{
  if (access(path, R_OK))
    RPS_FATALOUT("script file " << Rps_QuotedC_String(path)
                 << " is not accessible: "
                 << strerror(errno));
  char*rp = realpath(path, nullptr);
  if (rp == path) /* Same pointer, we want it to be malloc-ed in all
		    cases! */
    rp = strdup(path);
  if (!rp)
    RPS_FATALOUT("realpath(3) of "
                 <<  Rps_QuotedC_String(path) << " failed: "
                 << strerror(errno));
#warning should rps_do_at_exit_cpp "free(rp)" in rps_scripting_add_script
  if (!rps_is_main_thread())
    RPS_FATALOUT("adding script file " << rp << " from non main thread");
  if ((int) rps_scripts_vector.size() >  rps_script_maxnum)
    RPS_FATALOUT ("too many " << rps_scripts_vector.size()
                  << " script files (for " << rp << ")");
  rps_scripts_vector.push_back(rp);
  RPS_INFORMOUT("added script file #" << rps_scripts_vector.size()
                << ": " << rp);
} // end rps_scripting_add_script


void
rps_run_scripts_after_load(Rps_CallFrame* caller)
{
  if (rps_scripts_vector.empty())
    return;
  RPS_LOCALFRAME(rpskob_0XidDOU8sDm015tq4s /*=!running_script∈symbol*/,
                 caller,
                 Rps_Value strv;
                );
  RPS_DEBUG_LOG(REPL, "starting rps_run_scripts_after_load for "
                << rps_scripts_vector.size() << " scripts"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_scripts_after_load"));
  for (int ix=0; ix<rps_scripts_vector.size(); ix++) {
      try {
          rps_run_one_script_file(&_, ix);
        } catch (std::exception& ex) {
          RPS_FATALOUT("failed to run script#" << ix
                       << " " << rps_scripts_vector[ix]
                       << " got exception "
                       << ex.what());
        };
    }
  RPS_DEBUG_LOG(REPL, "end rps_run_scripts_after_load for "
                << rps_scripts_vector.size() << " scripts");
} // end rps_run_scripts_after_load



////////////////
void
rps_run_one_script_file(Rps_CallFrame*callframe, int ix)
{
  char modline[64];
  memset (modline, 0, sizeof(modline));
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(ix >= 0 && ix < (int)rps_scripts_vector.size());
  RPS_ASSERT(!strcmp(rps_scripting_magic_string,  RPS_SCRIPT_MAGIC_STR));
  const char*curpath = rps_scripts_vector[ix];
  const std::string curpstr(curpath);
  Rps_MemoryFileTokenSource tsrc(curpstr);
  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file ix#" << ix
                << " curpath=" << curpath
                << std::endl << " … tsrc=" << tsrc);
  RPS_POSSIBLE_BREAKPOINT();
  bool gotmagic=false;
  while (!gotmagic && tsrc.reached_end()) {
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file tsrc=" << tsrc);
      RPS_POSSIBLE_BREAKPOINT();
      if (!tsrc.get_line()) {
          RPS_POSSIBLE_BREAKPOINT();
          continue;
        };
      const char*clp = tsrc.curcptr();
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file @"
                    <<  tsrc.position_str()
                    << " clp=" << Rps_QuotedC_String(clp));
      RPS_POSSIBLE_BREAKPOINT();
      if (!clp) {
          RPS_DEBUG_LOG(REPL, "rps_run_one_script_file tsrc=" << tsrc
                        <<  " ¤eof @" << tsrc.position_str());
          RPS_POSSIBLE_BREAKPOINT();
          break;
        };
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file clp="
                    << Rps_QuotedC_String(clp)
                    << " @" << tsrc.position_str());
      RPS_POSSIBLE_BREAKPOINT();
      const char* magp = strstr(clp, rps_scripting_magic_string);
      if (magp) {
          static_assert(sizeof(modline)>60);
          RPS_POSSIBLE_BREAKPOINT();
          gotmagic= true;
          memset(modline, 0, sizeof(modline));
          int p = -1;
          int n = sscanf(magp,  RPS_SCRIPT_MAGIC_STR " %60[A-Za-z0-9_]%p", modline, &p);
          if (n > 0 && isascii(modline[0]) && p>0) {
              RPS_DEBUG_LOG(REPL, "rps_run_one_script_file clp="
                            << Rps_QuotedC_String(clp)
                            << " @" << tsrc.position_str()
                            << " modline=" << modline);
              RPS_POSSIBLE_BREAKPOINT();
#warning should use modline cleverly
              if (!strcmp(modline, "carbon")) { // see test_dir/005script.bash
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_WARNOUT("unimplemented rps_run_one_script_file ix=" << ix
                              << " curpath=" << curpath << " *CARBON* "
                              << " tsrc=" << tsrc << " @"  << tsrc.position_str()
                              << std::endl
                              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_one_script_file/CARBON"));
#warning rps_run_one_script_file in carbon mode should use routines from carbrepl_rps.cbrt, probably  rps_do_carburetta_command
                }
              else if (!strcmp(modline, "echo")) { // see test_dir/006echo.bash
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_WARNOUT("unimplemented rps_run_one_script_file ix=" << ix
                              << " curpath=" << curpath << " *ECHO* "
                              << " tsrc=" << tsrc << " @"  << tsrc.position_str()
                              << std::endl
                              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_one_script_file/ECHO"));
                  while (tsrc.get_line()) {
                      const char*clp = tsrc.curcptr();
                      if (!clp)
                        break;
                      RPS_DEBUG_LOG(REPL, "¤echo: clp=" << Rps_QuotedC_String(clp)
                                    << " @" << tsrc.position_str());
                      std::cout << clp << std::flush;
                    } // end while get_line in echo mode
                  std::cout << std::endl;
                } // end echo mode
            };
        };
#warning rps_run_one_script_file has missing code here
      RPS_POSSIBLE_BREAKPOINT();
    };				// end while !gotmagic...
  RPS_POSSIBLE_BREAKPOINT();
  RPS_WARNOUT("unimplemented rps_run_one_script_file ix=" << ix
              << " curpath=" << curpath << " "
              << (gotmagic?"GOTmagic":"NO!MAGIC")
              << " tsrc=" << tsrc << " @"  << tsrc.position_str()
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_one_script_file"));
  RPS_POSSIBLE_BREAKPOINT();
#warning rps_run_one_script_file incompletely unimplemented
} // end rps_run_one_script_file

//// end of file scripting_rps.cc

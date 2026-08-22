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
 *      © Copyright (C) 2025 - 2026 The Reflective Persistent System Team
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
#include <sys/resource.h>

// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG gmp
//@@PKGCONFIG gmpxx
//@@PKGCONFIG readline

#include "readline/readline.h"

extern "C" const char rps_scripting_gitid[];
const char rps_scripting_gitid[]= RPS_GITID;

extern "C" const char rps_scripting_shortgitid[];
const char rps_scripting_shortgitid[]= RPS_SHORTGITID;


extern "C" const char rps_scripting_basename[];
const char rps_scripting_basename[]= RPS_BASENAME;

extern "C" const char rps_scripting_baseid[];
const char rps_scripting_baseid[]= RPS_BASEID;


extern "C" void rps_scripting_help(void);
extern "C" void rps_scripting_add_script(const char*);

extern "C" const char rps_scripting_help_english_text[];


extern "C" void rps_run_one_script_file(Rps_CallFrame*, int ix);

extern "C" void rps_run_script_carbon_mode(Rps_CallFrame*,
    Rps_TokenSource&,
    int ix, int loopcnt);
extern "C" void rps_run_script_parse_mode(Rps_CallFrame*,
    Rps_TokenSource&,
    int ix, int loopcnt);
extern "C" void rps_run_script_echo_mode(Rps_CallFrame*,
    Rps_TokenSource&,
    int ix, int loopcnt);
extern "C" void rps_run_script_minicarb_mode(Rps_CallFrame*,
    Rps_TokenSource&,
    int ix, int loopcnt);




extern "C" const int rps_script_maxnum = 1024;


struct rps_script_st
{
  char* script_arg; // strdup-ed
  Rps_TokenSource* (*script_maker)(const char*);
};
/// vector of strings eg real path to script files
static std::vector<struct rps_script_st> rps_scripts_vector;


const char rps_scripting_help_english_text[] =
  R"help(A script file is a textual file, or - for stdin, _ for readline,
and if starting by | or ! a popen-ed command.  All its initial lines
before a line containing REFPERSYS_SCRIPT are ignored.  Hence these
initial lines could contain some shell script, etc.  That
REFPERSYS_SCRIPT word should be followed by a short C-like identifier
identifying the mode.  That mode defines how is the script parsed and
usable.
)help"
  ;
#warning more text needed inside rps_scripting_help_english_text

extern "C" const char  rps_scripting_magic_string[];

#define RPS_SCRIPT_MAGIC_STR "REFPERSYS_SCRIPT"
const char rps_scripting_magic_string[] = RPS_SCRIPT_MAGIC_STR;

extern "C" Rps_TokenSource*rps_make_cin_token_source(const char*);
extern "C" Rps_TokenSource*rps_make_readline_token_source(const char*);
extern "C" Rps_TokenSource*rps_make_file_token_source(const char*);

extern "C" Rps_TokenSource*rps_make_memory_file_token_source(const char*);
extern "C" Rps_TokenSource*rps_make_pipe_token_source(const char*);

extern "C" Rps_TokenSource*rps_make_string_token_source(const char*);

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
  Rps_TokenSource* (*maker)(const char*) = nullptr;
  char* dupath = nullptr;
  RPS_POSSIBLE_BREAKPOINT();
  RPS_ASSERT(path);
  if (!rps_is_main_thread())
    RPS_FATALOUT("adding script " << path << " from non main thread");
  if ((int) rps_scripts_vector.size() >  rps_script_maxnum)
    RPS_FATALOUT ("too many " << rps_scripts_vector.size()
                  << " script (for " << path << ")");
  RPS_DEBUG_LOG(REPL, "scripting@add@script " << Rps_QuotedC_String(path));
  RPS_UNIQUE_BREAKPOINT();
  if (!strcmp(path, "-")) {
      RPS_UNIQUE_BREAKPOINT();
      maker = rps_make_cin_token_source;		// use cin
      dupath = strdup("-");
      RPS_ASSERT(dupath);
    }
  else if (!strcmp(path, "_")) {
      RPS_UNIQUE_BREAKPOINT();
      maker = rps_make_readline_token_source;		// use readline
      dupath = strdup("_");
      RPS_ASSERT(dupath);
      RPS_UNIQUE_BREAKPOINT();
    }
  else if (path[0]=='|' || path[0]=='!') {
      RPS_UNIQUE_BREAKPOINT();
      maker = rps_make_pipe_token_source;		// use pipe
      dupath = strdup(path);
      RPS_ASSERT(dupath);
    }
  else if (path[1] && access(path, R_OK))
    RPS_FATALOUT("script file " << Rps_QuotedC_String(path)
                 << " is not accessible: "
                 << strerror(errno));
  RPS_UNIQUE_BREAKPOINT();
  if (!dupath)
    dupath = realpath(path, nullptr);
  if (dupath == path) /* Same pointer, we want it to be malloc-ed in all
		     cases! */
    dupath = strdup(path);
  if (!dupath)
    RPS_FATALOUT("realpath(3) or strdup(3) of "
                 <<  Rps_QuotedC_String(path) << " failed: "
                 << strerror(errno));
  long fsiz = -1;
  if (!maker) {
      FILE* f = fopen(dupath, "r");
      if (f) {
          if (fseek(f, 0, SEEK_END)) {
              fsiz = ftell(f);
              rewind(f);
              fclose(f);
            }
        }
      else
        RPS_FATALOUT("failed to fopen script file " << dupath
                     << " : " << strerror(errno));
    };
  struct stat scriptstat = {};
  if (stat(dupath, &scriptstat))
    RPS_FATALOUT("failed to stat script file " << dupath
                 << " : " << strerror(errno));
  if (fsiz==0)
    RPS_FATALOUT("script file " << dupath << " is empty");
  if (fsiz<0 && !maker) { /// non-seekable file, maybe FIFO or Unix socket?
      RPS_UNIQUE_BREAKPOINT();
      maker = rps_make_file_token_source;
      RPS_DEBUG_LOG(REPL, "maker default to file token source for dupath=" << dupath);
    };
  RPS_POSSIBLE_BREAKPOINT();
  RPS_ASSERT(maker);
  if (rps_scripts_vector.empty()) {
      /////
      /****
                   ** Only the main thread can call rps_scripting_add_script, so
                   ** no more synchronization or mutex is needed to :
                   *****/
      /////
      rps_do_on_exit([=](void){
        rps_scripts_vector.clear();
      });
      RPS_POSSIBLE_BREAKPOINT();
      RPS_DEBUG_LOG(REPL, "rps_scripting_add_script first call dupath="
                    << Rps_QuotedC_String(dupath)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_scripting_add_script/first"));
    }
  else {
      // rps_scripting_add_script was already called
      RPS_POSSIBLE_BREAKPOINT();
      RPS_DEBUG_LOG(REPL, "rps_scripting_add_script other call#"
                    << rps_scripts_vector.size()
                    << " dupath=" << Rps_QuotedC_String(dupath)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_scripting_add_script/other"));

    };
  RPS_POSSIBLE_BREAKPOINT();
  if (!maker)
    RPS_FATALOUT("no maker for script " << path);
  struct rps_script_st s;
  s.script_arg = (dupath);
  s.script_maker = maker;
  rps_scripts_vector.push_back(s);
  RPS_INFORMOUT("added script file #" << rps_scripts_vector.size()
                << ": " << dupath);
  RPS_POSSIBLE_BREAKPOINT();
} // end rps_scripting_add_script


void
rps_run_scripts_after_load(Rps_CallFrame* caller)
{
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "rps_run_scripts_after_load running "
                << rps_scripts_vector.size() << " scripts from:"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_scripts_after_load"));
  if (rps_scripts_vector.empty())
    return;
  RPS_POSSIBLE_BREAKPOINT();
  RPS_ASSERT_CALLFRAME(caller);
  RPS_LOCALFRAME(rpskob_0XidDOU8sDm015tq4s /*=!running_script∈symbol*/,
                 caller,
                 Rps_Value strv;
                );
  RPS_DEBUG_LOG(REPL, "starting rps_run_scripts_after_load for "
                << rps_scripts_vector.size() << " scripts"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_scripts_after_load"));
  RPS_UNIQUE_BREAKPOINT();
  for (int ix=0;
       ix<(int)rps_scripts_vector.size();
       ix++) {
      RPS_ASSERT(rps_scripts_vector.size() <= rps_script_maxnum);
      RPS_POSSIBLE_BREAKPOINT();
      try {
          RPS_UNIQUE_BREAKPOINT();
          RPS_DEBUG_LOG(REPL, "rps_run_scripts_after_load will run script#"
                        << ix
                        << " " << rps_scripts_vector[ix].script_arg);
          RPS_POSSIBLE_BREAKPOINT();
          rps_run_one_script_file(&_, ix);
          RPS_POSSIBLE_BREAKPOINT();
          RPS_DEBUG_LOG(REPL, "rps_run_scripts_after_load did run script#"
                        << ix
                        << " " << rps_scripts_vector[ix].script_arg);
        } catch (std::exception& ex) {
          RPS_FATALOUT("failed to run script#" << ix
                       << " " << rps_scripts_vector[ix].script_arg
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
  RPS_ASSERT(ix >= 0 && ix < (int)rps_scripts_vector.size()
             && ix <= rps_script_maxnum);
  RPS_ASSERT(!strcmp(rps_scripting_magic_string,  RPS_SCRIPT_MAGIC_STR));
  const char* sarg
    = rps_scripts_vector[ix].script_arg;
  Rps_TokenSource* (*smaker)(const char*) =
    rps_scripts_vector[ix].script_maker;
  RPS_ASSERT(sarg != nullptr && sarg[0] != (char)0);
  RPS_ASSERT(smaker);
  RPS_UNIQUE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file ix#" << ix
                << " sarg=" << Rps_QuotedC_String(sarg)
                << " thread:" << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "+rps_run_one_script_file"));
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_ObjectRef obenv;);
  RPS_UNIQUE_BREAKPOINT();
  Rps_TokenSource* ptsrc = (*smaker)(sarg);
  RPS_UNIQUE_BREAKPOINT();
  (*ptsrc).fill_current_line_buffer();
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file ix#" << ix
                << " sarg=" << Rps_QuotedC_String(sarg)
                << std::endl << " … (*ptsrc)=" << (*ptsrc)
                << " curcptr=" << Rps_QuotedC_String((*ptsrc).curcptr()));
  RPS_POSSIBLE_BREAKPOINT();
  bool gotmagic=false;
  int loopcnt=0;
  bool gotlin = (*ptsrc).get_line();
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file ix#" << ix
                << " (*ptsrc)=" << (*ptsrc)
                << (gotlin?" got line": " noline")
                << " curcptr=" << Rps_QuotedC_String((*ptsrc).curcptr())
                << (((*ptsrc).reached_end())?" reachedEND": " notEND"));
  RPS_POSSIBLE_BREAKPOINT();
  while (!gotmagic
         && gotlin
         && !(*ptsrc).reached_end()) {
      loopcnt++;
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file (*ptsrc)=" << (*ptsrc)
                    << " start loop#" << loopcnt
                    << " obenv=" << _f.obenv
                    << " curcptr=" << Rps_QuotedC_String((*ptsrc).curcptr()));
      RPS_POSSIBLE_BREAKPOINT();
      if (!(gotlin=(*ptsrc).get_line())) {
          RPS_POSSIBLE_BREAKPOINT();
          continue;
        };
      const char*clp = (*ptsrc).curcptr();
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file @"
                    <<  (*ptsrc).position_str()
                    << " loop#" << loopcnt
                    << " obenv=" << _f.obenv
                    << " clp=" << Rps_QuotedC_String(clp));
      RPS_POSSIBLE_BREAKPOINT();
      if (!clp) {
          RPS_DEBUG_LOG(REPL, "rps_run_one_script_file (*ptsrc)=" << (*ptsrc)
                        << " loop#" << loopcnt
                        <<  " ¤maybe-eof @" << (*ptsrc).position_str());
          RPS_POSSIBLE_BREAKPOINT();
          if ((*ptsrc).get_line()) {
              clp = (*ptsrc).curcptr();
              RPS_DEBUG_LOG(REPL, "rps_run_one_script_file (*ptsrc)=" << (*ptsrc)
                            << " loop#" << loopcnt << " got-line "
                            << " clp=" << Rps_QuotedC_String(clp));
            }
          else {
              RPS_DEBUG_LOG(REPL, "rps_run_one_script_file (*ptsrc)=" << (*ptsrc)
                            << " loop#" << loopcnt << " eof "
                            << ((*ptsrc).reached_end()
                                ?" reached-end"
                                :" °notReachedEnd"));
              break;
            }
        };
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file clp="
                    << Rps_QuotedC_String(clp)
                    << " loop#" << loopcnt
                    << " @" << (*ptsrc).position_str());
      RPS_POSSIBLE_BREAKPOINT();
      if (!clp) {
          RPS_DEBUG_LOG(REPL, "rps_run_one_script_file °NULL-clp"
                        << " loop#" << loopcnt
                        << " @" << (*ptsrc).position_str()
                        << " " << ((*ptsrc).reached_end()?"°atend":"°notend")
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_run_one_script_file °NULL-clp"));
          usleep(12345);        // temporary code to slow down
          // debugging output
          RPS_POSSIBLE_BREAKPOINT();
#warning rps_run_one_script_file incomplete when clp is null
        };
      RPS_UNIQUE_BREAKPOINT();
      const char* magp = strstr(clp, rps_scripting_magic_string);
      if (magp) {
          static_assert(sizeof(modline)>60);
          RPS_POSSIBLE_BREAKPOINT();
          gotmagic= true;
          memset(modline, 0, sizeof(modline));
          int p = -1;
          int n = sscanf(magp,  RPS_SCRIPT_MAGIC_STR " %60[A-Za-z0-9_]%n",
                         modline, &p);
          if (n > 0 && isascii(modline[0]) && p>0) {
              RPS_DEBUG_LOG(REPL, "rps_run_one_script_file clp="
                            << Rps_QuotedC_String(clp)
                            << " @" << (*ptsrc).position_str()
                            << " modline=" << modline
                            << " loop#" << loopcnt);
              RPS_POSSIBLE_BREAKPOINT();
#warning should use modline cleverly
              if (!strcmp(modline, "carbon")) { // see test_dir/005script.bash
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file/CARBON ix=" << ix
                                << " sarg=" << sarg
                                << " *CARBON* "
                                << " (*ptsrc)=" << (*ptsrc) << " @"  << (*ptsrc).position_str()
                                << " loop#" << loopcnt);
                  rps_run_script_carbon_mode(&_, (*ptsrc), ix, loopcnt);
                  RPS_POSSIBLE_BREAKPOINT();
                }
              if (!strcmp(modline, "parse")) {
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file/PARSE ix=" << ix
                                << " sarg=" << Rps_QuotedC_String(sarg)
                                << " *PARSE* "
                                << " (*ptsrc)=" << (*ptsrc) << " @"  << (*ptsrc).position_str()
                                << " loop#" << loopcnt);
                  rps_run_script_parse_mode(&_, (*ptsrc), ix, loopcnt);
                  RPS_POSSIBLE_BREAKPOINT();
                }
              else if (!strcmp(modline, "echo")) { // see test_dir/006echo.bash
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file/ECHO ix=" << ix
                                << " sarg=" << Rps_QuotedC_String(sarg)
                                << " *ECHO* "
                                << " (*ptsrc)=" << (*ptsrc)
                                << " @"  << (*ptsrc).position_str()
                                << " loop#" << loopcnt);
                  rps_run_script_echo_mode(&_, (*ptsrc), ix, loopcnt);
                  RPS_POSSIBLE_BREAKPOINT();
                } // end echo mode
              else if (!strcmp(modline, "minicarb")) { // see minicarb_rps.cbrt
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_DEBUG_LOG(REPL, "rps_run_one_script_file/MINICARB ix=" << ix
                                << " sarg=" << Rps_QuotedC_String(sarg)
                                << " *MINICARB* "
                                << " (*ptsrc)=" << (*ptsrc)
                                << " @"  << (*ptsrc).position_str()
                                << " loop#" << loopcnt);
                  RPS_DEBUG_LOG(LOWREP, "rps_run_one_script_file/MINICARB ix=" << ix
                                << " sarg=" << Rps_QuotedC_String(sarg)
                                << " *MINICARB* "
                                << " (*ptsrc)=" << (*ptsrc)
                                << " @"  << (*ptsrc).position_str()
                                << " loop#" << loopcnt);
                  rps_run_script_minicarb_mode(&_, (*ptsrc), ix, loopcnt);
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_DEBUG_LOG(REPL, "after rps_run_script_minicarb_mode (*ptsrc)="
                                << (*ptsrc));
                  RPS_UNIQUE_BREAKPOINT();
                  return;
                } // end minicarb mode
              else {
                  RPS_POSSIBLE_BREAKPOINT();
                  RPS_WARNOUT("rps_run_one_script_file ix#" << ix
                              << " sarg=" << Rps_QuotedC_String(sarg)
                              << " unexpected modline="
                              << Rps_QuotedC_String(modline)
                              << " (*ptsrc)=" << (*ptsrc)
                              << " @"  << (*ptsrc).position_str()
                              << " loop#" << loopcnt);
                  RPS_POSSIBLE_BREAKPOINT();
                }
            };
        };
#warning rps_run_one_script_file has missing code here
      RPS_DEBUG_LOG(REPL, "rps_run_one_script_file endloop @"
                    <<  (*ptsrc).position_str()
                    << " loop#" << loopcnt
                    << (gotmagic?" GOTMAGIC":" noMAGIC"));
      RPS_POSSIBLE_BREAKPOINT();
    };                          // end while !gotmagic...
  RPS_POSSIBLE_BREAKPOINT();
  RPS_WARNOUT("unimplemented rps_run_one_script_file ix=" << ix
              << std::endl
              << "… sarg=" << Rps_QuotedC_String(sarg) << " "
              << (gotmagic?"GOTmagic":"NO!MAGIC")
              << " loop#" << loopcnt
              << " (*ptsrc)=" << (*ptsrc) << " @"  << (*ptsrc).position_str()
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_one_script_file")
              << std::endl);
  RPS_POSSIBLE_BREAKPOINT();
#warning rps_run_one_script_file incompletely unimplemented
} // end rps_run_one_script_file



void
rps_run_script_carbon_mode(Rps_CallFrame*callfr,
                           Rps_TokenSource&tsrc,
                           int ix, int loopcnt)
{
  const char*clp = tsrc.curcptr();
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,callfr,
                 Rps_ObjectRef obenv;);
  _f.obenv = rps_get_first_repl_environment();
  RPS_DEBUG_LOG(REPL, "rps_run_script_carbon_mode clp="
                << Rps_QuotedC_String(clp) << " obenv=" << _f.obenv);
  RPS_WARNOUT("unimplemented rps_run_script_carbon_mode ix=" << ix
              << " tsrc=" << tsrc
              << " @" << tsrc.position_str()
              << " loop#" << loopcnt
              << std::endl << " *obenv=" << std::endl
              << RPS_OBJECT_DISPLAY(_f.obenv)
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_script_carbon_mode"));
  RPS_POSSIBLE_BREAKPOINT();
  rps_do_carburetta_command(&_, _f.obenv, &tsrc);
  RPS_DEBUG_LOG(REPL, "rps_run_script_carbon_mode clp="
                << Rps_QuotedC_String(clp)
                << " @" << tsrc.position_str() << std::endl
                << RPS_OBJECT_DISPLAY(_f.obenv)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "-rps_run_script_carbon_mode")
                << " ix=" << ix << " loop#" << loopcnt);
} // end rps_run_script_carbon_mode


void
rps_run_script_parse_mode(Rps_CallFrame*callfr,
                          Rps_TokenSource&tsrc,
                          int ix, int loopcnt)
{
  const char*clp = tsrc.curcptr();
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,callfr,
                 Rps_ObjectRef obenv;);
  _f.obenv = nullptr;
  RPS_DEBUG_LOG(REPL, "rps_run_script_carbon_mode clp="
                << Rps_QuotedC_String(clp) << " obenv=" << _f.obenv);
  RPS_WARNOUT("unimplemented rps_run_script_parse_mode ix=" << ix
              << " tsrc=" << tsrc
              << " @" << tsrc.position_str()
              << " loop#" << loopcnt
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_script_parse_mode"));
  RPS_POSSIBLE_BREAKPOINT();
  rps_do_carburetta_command(&_, (Rps_ObjectRef)nullptr, &tsrc);
  RPS_DEBUG_LOG(REPL, "rps_run_script_parse_mode clp="
                << Rps_QuotedC_String(clp)
                << " @" << tsrc.position_str() << std::endl
                << RPS_OBJECT_DISPLAY(_f.obenv)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "-rps_run_script_parse_mode")
                << " ix=" << ix << " loop#" << loopcnt);
} // end rps_run_script_parse_mode



void
rps_run_script_echo_mode(Rps_CallFrame*callfr,
                         Rps_TokenSource&tsrc,
                         int ix, int loopcnt)
{
  const char*clp = tsrc.curcptr();
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,callfr,
                 Rps_ObjectRef obenv;);
  RPS_INFORMOUT("rps_run_one_echo_mode ix=" << ix
                << " loopcnt=" << loopcnt);
  RPS_DEBUG_LOG(REPL, "rps_run_script_echo_mode ix=" << ix
                << " tsrc=" << tsrc << " @"  << tsrc.position_str()
                << " loop#" << loopcnt
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_script_echo_mode"));
  while (tsrc.get_line() && !tsrc.reached_end()) {
      clp = tsrc.curcptr();
      if (!clp)
        break;
      RPS_DEBUG_LOG(REPL, "¤echo: clp=" << Rps_QuotedC_String(clp)
                    << " outloop#" << loopcnt
                    << " @" << tsrc.position_str());
      std::cout << clp << std::flush;
    } // end while get_line in echo mode
  std::cout << std::endl;
  RPS_DEBUG_LOG(REPL, "rps_run_script_echo_mode clp="
                << Rps_QuotedC_String(clp)
                << " @" << tsrc.position_str()
                << " echo mode"
                << RPS_FULL_BACKTRACE_HERE(1, "-rps_run_script_echo_mode")
                << " ix=" << ix << " loop#" << loopcnt);
} // end rps_run_script_echo_mode




void
rps_run_script_minicarb_mode(Rps_CallFrame*callfr,
                             Rps_TokenSource&tsrc,
                             int ix, int loopcnt)
{
  const char*clp = tsrc.curcptr();
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,callfr,
                 Rps_ObjectRef obenv;);
  _f.obenv = rps_get_first_repl_environment();
  {
    struct rlimit rlcore;
    rlcore.rlim_cur= (16<<30); //16Gbytes
    rlcore.rlim_max= (32<<30); //32Gbytes
    RPS_DEBUG_LOG(REPL, "setrlimit rlimit_core pid " << getpid());
    if (setrlimit(RLIMIT_CORE, &rlcore))
      RPS_FATALOUT("failed to set core limit (16Gb soft, 32Gb hard):"
                   << strerror(errno));
    else
      RPS_INFORMOUT("did set core limit to 16Gb soft, 32Gb hard for pid "
                    << getpid());
  };
  if (RPS_DEBUG_ENABLED(REPL) || RPS_DEBUG_ENABLED(LOWREP))
    {
      char lbuf[256];
      FILE*flim = fopen("/proc/self/limits", "r");
      if (!flim)
        RPS_FATALOUT("failed to open /proc/self/limits " << strerror(errno));
      RPS_INFORMOUT("our /proc/self/limits is (pid " << getpid() << "):");
      do {
          memset(lbuf, 0, sizeof(lbuf));
          if (!fgets(lbuf, (int)sizeof(lbuf), flim))
            break;
          fputs(lbuf, stdout);
        } while (!feof(flim));
      fclose(flim);
      fprintf(stdout, "###eof-limits (pid:%d) [%s:%d]\n", (int)getpid(),
              __FILE__, __LINE__-1);
      fflush(stdout);
    }
  RPS_DEBUG_LOG(REPL, "rps_run_script_minicarb_mode clp="
                << Rps_QuotedC_String(clp) << " obenv=" << _f.obenv);
  RPS_DEBUG_LOG(LOWREP, "rps_run_script_minicarb_mode clp="
                << Rps_QuotedC_String(clp) << " obenv=" << _f.obenv
                << " ix=" << ix << " loopcnt=" << loopcnt
                << " tsrc=" << tsrc);
  RPS_DEBUG_LOG(REPL, "rps_run_script_minicarb_mode tsrc="
                << tsrc << " ix=" << ix
                << " before call to rps_do_minicarb_command obenv="
                << _f.obenv);
  RPS_UNIQUE_BREAKPOINT();
  rps_do_minicarb_command(&_, _f.obenv, &tsrc);
  RPS_DEBUG_LOG(REPL, "rps_run_script_minicarb_mode tsrc="
                << tsrc << " ix=" << ix
                << " after call to rps_do_minicarb_command obenv="
                << _f.obenv);
  RPS_WARNOUT("incomplete rps_run_script_minicarb_mode ix=" << ix
              << " tsrc=" << tsrc
              << " @" << tsrc.position_str()
              << " loop#" << loopcnt
              << std::endl << " *obenv=" << std::endl
              << RPS_OBJECT_DISPLAY(_f.obenv)
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_run_script_minicarb_mode"));
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "end°°rps_run_script_minicarb_mode clp="
                << Rps_QuotedC_String(clp) << " obenv=" << _f.obenv);
} // end rps_run_script_minicarb_mode


Rps_TokenSource*
rps_make_cin_token_source(const char*a)
{
  RPS_ASSERT(a && !strcmp(a, "-"));
  return new Rps_CinTokenSource();
} // end rps_make_cin_token_source

Rps_TokenSource*
rps_make_readline_token_source(const char*a)
{
  RPS_ASSERT(a && !strcmp(a, "_"));
  return new Rps_ReadlineTokenSource();
} // end rps_make_readline_token_source


Rps_TokenSource*
rps_make_file_token_source(const char*a)
{
  RPS_ASSERT(a);
  return new Rps_FileTokenSource(a);
} // end rps_make_file_token_source

Rps_TokenSource*
rps_make_memory_file_token_source(const char*a)
{
  RPS_ASSERT(a);
  return new Rps_MemoryFileTokenSource(a);
} // end rps_make_memory_token_source

Rps_TokenSource*
rps_make_pipe_token_source(const char*a)
{
  RPS_ASSERT(a);
  return new Rps_PipeTokenSource(a);
} // end rps_make_pipe_token_source


Rps_TokenSource*
rps_make_string_token_source(const char*a)
{
  RPS_ASSERT(a);
  return new Rps_StringTokenSource(std::string(a),
                                   std::string(Rps_QuotedC_String(a)));
} // end rps_make_memory_token_source

//// end of file scripting_rps.cc

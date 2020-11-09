/****************************************************************
 * file repl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the Read-Eval-Print-Loop code using GNU readline
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

#include "readline/readline.h"

extern "C" const char rps_repl_gitid[];
const char rps_repl_gitid[]= RPS_GITID;

extern "C" const char rps_repl_date[];
const char rps_repl_date[]= __DATE__;

extern "C" std::istream*rps_repl_input = nullptr;

/// Interpret from either a given input stream,
/// or using readline if inp is null.
extern "C" void rps_repl_interpret(Rps_CallFrame*callframe, std::istream*inp, const char*input_name);

/*** The lexer. We return a pair of values.
 ***/
extern "C" Rps_TwoValues rps_repl_lexer(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char*linebuf, int &lineno, int& colno);

/// the lexer for the

std::string
rps_repl_version(void)
{

  std::string res = "REPL";
  {
    char gitstart[128];
    memset (gitstart, 0, sizeof(gitstart));
    strncpy(gitstart, rps_repl_gitid, (2*sizeof(gitstart))/3+2);
    res += " git ";
    res += gitstart;
  }
  res += " ReadLine ";
  res += rl_library_version;
  return res;
} // end rps_repl_version

void
rps_repl_interpret(Rps_CallFrame*callframe, std::istream*inp, const char*input_name)
{
  std::istream*previous_input=nullptr;
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(input_name != nullptr);
  // descriptor is: _6x4XcZ1fxp403uBUoz) //"rps_repl_interpret"∈core_function
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_6x4XcZ1fxp403uBUoz),
                           /*callerframe:*/callframe,
                );
  previous_input = rps_repl_input;
  rps_repl_input = inp;
  rps_repl_input = previous_input;
} // end rps_repl_interpret

Rps_TwoValues
rps_repl_lexer(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char*linebuf, int &lineno, int& colno)
{

  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(input_name != nullptr);
  RPS_ASSERT(linebuf != nullptr);
  int linelen = strlen(linebuf);
  RPS_ASSERT(callframe != nullptr);
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_ObjectRef oblex;
                );
  RPS_ASSERT(colno >= 0 && colno <= linelen);
  while (colno < linelen && isspace(linebuf[colno]))
    colno++;
  ////////////////
  /// lex numbers?
  if (((linebuf[colno] == '+' || linebuf[colno] == '-') && isdigit(linebuf[colno+1]))
      || isdigit(linebuf[colno]))
    {
      char*endint=nullptr;
      char*endfloat=nullptr;
      const char*startnum = linebuf+colno;
      long long l = strtoll(startnum, &endint, 0);
      double d = strtod(startnum, &endfloat);
      RPS_ASSERT(endint != nullptr && endfloat != nullptr);
      if (endfloat > endint)
        {
          colno += endfloat - startnum;
          return Rps_TwoValues{RPS_ROOT_OB(_98sc8kSOXV003i86w5), //double∈class
                               Rps_DoubleValue(d)};
        }
      else
        {
          colno += endint - startnum;
          return Rps_TwoValues{RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b), //int∈class
                               Rps_Value(l, Rps_Value::Rps_IntTag{})};
        }
    }
  /// lex infinities (double) - but not NAN
  else if (!strncmp(linebuf+colno, "+INF", 4)
           || !strncmp(linebuf+colno, "-INF", 4))
    {
      bool pos = linebuf[colno] == '+';
      colno += 4;
      return Rps_TwoValues{RPS_ROOT_OB(_98sc8kSOXV003i86w5), //double∈class
                           Rps_DoubleValue(pos
                                           ?std::numeric_limits<double>::infinity()
                                           : -std::numeric_limits<double>::infinity())};
    }
  //////////////// lex named objects or objids
  else if (isalpha(linebuf[colno]) || linebuf[colno]=='_')
    {
      int startname = colno;
      while (isalnum(linebuf[colno]) || linebuf[colno]=='_')
        colno++;
      std::string namestr(linebuf+startname, colno-startname);
      _f.oblex = Rps_ObjectRef::find_object_by_string(&_, namestr, true);
      if (_f.oblex)
        return Rps_TwoValues(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ), //object∈class
                             _f.oblex);
      /// some new symbol
      if (isalpha(namestr[0]))
        return Rps_TwoValues(RPS_ROOT_OB(_36I1BY2NetN03WjrOv), //symbol∈class
                             Rps_StringValue(namestr));
      /// otherwise, fail to lex, so
      colno = startname;
      RPS_WARNOUT("rps_repl_lexer " << input_name << " line " << lineno << ", column " << colno
                  << " : bad name " << linebuf+colno);
      return Rps_TwoValues(nullptr, nullptr);
    }
  RPS_FATALOUT("unimplemented rps_repl_lexer inp@" << (void*)inp
               << " input_name=" << input_name
               << " line_buf='" << Rps_Cjson_String(linebuf) << "'"
               << " lineno=" << lineno
               << " colno=" << colno
               << " curpos=" << linebuf+colno
               << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer"));
#warning unimplemented rps_repl_lexer
} // end of rps_repl_lexer



void
rps_read_eval_print_loop(int &argc, char **argv)
{
  for (int ix=0; ix<argc; ix++)
    RPS_DEBUG_LOG(REPL, "REPL arg [" << ix << "]: " << argv[ix]);
#warning incomplete rps_read_eval_print_loop
  RPS_WARNOUT("incomplete rps_read_eval_print_loop " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_read_eval_print_loop"));
} // end of rps_read_eval_print_loop

// end of file repl_rps.cc

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

/*** The lexer. We return a pair of values. The first describing the
     second.  For example, a lexed integer is given as
     (int,<tagged-integer-value>).
 ***/
extern "C" Rps_TwoValues rps_repl_lexer(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char*linebuf, int &lineno, int& colno);

extern "C" std::string rps_lex_literal_string(const char*input_name, const char*linebuf, int lineno, int& colno);

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
  /// literal string prefix, à la C++
  char litprefix[16];
  memset (litprefix, 0, sizeof(litprefix));
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
  //// literal strings are like in C++
  else if (linebuf[colno] == '"')   /// plain literal string, on a single line
    {
      std::string litstr =
        rps_lex_literal_string(input_name, linebuf, lineno, colno);
      return Rps_TwoValues(RPS_ROOT_OB(_62LTwxwKpQ802SsmjE), //string∈class
                           litstr);
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

std::string
rps_lex_literal_string(const char*input_name, const char*linebuf, int lineno, int& colno)
{
  std::string rstr;
  int curcol = colno;
  RPS_ASSERT(linebuf[colno] == '"');
  curcol++;
  char curch = 0;
  while ((curch=linebuf[curcol]) != (char)0)
    {
      if (curch == '"')
        {
          colno = curcol+1;
          return rstr;
        }
      else if (curch == '\\')
        {
          char nextch = linebuf[curcol+1];
          char shortbuf[16];
          memset(shortbuf, 0, sizeof(shortbuf));
          switch (nextch)
            {
            case '\'':
            case '\"':
            case '\\':
              rstr.push_back (nextch);
              curcol += 2;
              continue;
            case 'a':
              rstr.push_back ('\a');
              curcol += 2;
              continue;
            case 'b':
              rstr.push_back ('\b');
              curcol += 2;
              continue;
            case 'f':
              rstr.push_back ('\f');
              curcol += 2;
              continue;
            case 'e':			// ESCAPE
              rstr.push_back ('\033');
              curcol += 2;
              continue;
            case 'n':
              rstr.push_back ('\n');
              curcol += 2;
              continue;
            case 'r':
              rstr.push_back ('\r');
              curcol += 2;
              continue;
            case 't':
              rstr.push_back ('\t');
              curcol += 2;
              continue;
            case 'v':
              rstr.push_back ('\v');
              curcol += 2;
              continue;
            case 'x':
            {
              int p = -1;
              int c = 0;
              if (sscanf (linebuf + curcol + 2, "%02x%n", &c, &p) > 0 && p > 0)
                {
                  rstr.push_back ((char)c);
                  curcol += p + 2;
                  continue;
                }
              else
                goto lexical_error_backslash;
            }
            case 'u': // four hexdigit escape Unicode
            {
              int p = -1;
              int c = 0;
              if (sscanf (linebuf + curcol + 2, "%04x%n", &c, &p) > 0 && p > 0)
                {
                  int l =
                    u8_uctomb ((uint8_t *) shortbuf, (ucs4_t) c, sizeof(shortbuf));
                  if (l > 0)
                    {
                      rstr.append(shortbuf);
                      curcol += p + l;
                      continue;
                    }
                  else
                    goto lexical_error_backslash;
                }
            }
            case 'U': // eight hexdigit escape Unicode
            {
              int p = -1;
              int c = 0;
              if (sscanf (linebuf + curcol + 2, "%08x%n", &c, &p) > 0 && p > 0)
                {
                  int l =
                    u8_uctomb ((uint8_t *) shortbuf, (ucs4_t) c, sizeof(shortbuf));
                  if (l > 0)
                    {
                      rstr.append(shortbuf);
                      curcol += p + l;
                      continue;
                    }
                  else
                    goto lexical_error_backslash;
                }
            }
            default:
              goto lexical_error_backslash;
            } // end switch nextch
        } // end if curch is '\\'
      else if (curch >= ' ' && curch < 0x7f)
        {
          rstr.push_back (curch);
          curcol++;
          continue;
        }
      /// accepts any correctly encoded UTF-8
      else if (int l = u8_mblen ((uint8_t *)(linebuf + curcol), strlen(linebuf)-curcol); l>0)
        {
          rstr.append(linebuf + curcol, l);
          curcol += l;
          continue;
        }
      /// improbable lexical error....
      else
        {
          RPS_WARNOUT("rps_lex_literal_string " << input_name << " line " << lineno << ", column " << colno
                      << ": lexical error at " << linebuf+colno);
          throw std::runtime_error("lexical error");
        }
    } // end while
lexical_error_backslash:
  RPS_WARNOUT("rps_lex_literal_string " << input_name << " line " << lineno << ", column " << colno
              << " : bad backslash escape " << linebuf+colno);
  throw std::runtime_error("lexical bad backslash escape");
} // end rps_lex_literal_string

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

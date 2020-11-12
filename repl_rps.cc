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
#include "readline/history.h"

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

extern "C" std::string rps_lex_raw_literal_string(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int lineno, int& colno);

extern "C" Rps_Value rps_lex_code_chunk(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int& lineno, int& colno);

// return true iff th next line has been gotten
extern "C" bool
rps_repl_get_next_line(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int*plineno, std::string prompt="");

static Rps_CallFrame*rps_readline_callframe;

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

bool
rps_repl_get_next_line(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int*plineno, std::string prompt)
{
  RPS_ASSERT(callframe);
  RPS_ASSERT(input_name);
  RPS_ASSERT(plinebuf);
  RPS_ASSERT(plineno);
  if (inp)   // we use it
    {
      if (inp->eof())
        {
          free (plinebuf), *plinebuf = nullptr;
          return false;
        }
      std::string linestr;
      std::getline(*inp, linestr);
      if (linestr.empty() || linestr[linestr.size()-1] != '\n')
        linestr.push_back('\n');
      free (plinebuf), *plinebuf = strdup(linestr.c_str());
      (*plineno)++;
      return true;
    }
  else
    {
      RPS_ASSERT(rps_is_main_thread());
      if (prompt.empty())
        prompt = std::string(input_name) + ":";
      if (!rps_without_terminal_escape)
        {
          prompt.insert(0,  RPS_TERMINAL_BOLD_ESCAPE);
          prompt.append(RPS_TERMINAL_NORMAL_ESCAPE);
        }
      rps_readline_callframe = callframe;
      free (plinebuf), *plinebuf = readline(prompt.c_str());
      rps_readline_callframe = nullptr;
      if (*plinebuf)
        {
          (*plineno)++;
          if (*plinebuf[0])
            add_history(*plinebuf);
          return true;
        };
    }
  free (plinebuf), *plinebuf = nullptr;
  return false;
} // end of rps_repl_get_next_line

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
  //// raw literal strings may span across several lines, like in C++
  //// see https://en.cppreference.com/w/cpp/language/string_literal
  else if (linebuf[colno] == 'R' && linebuf[colno+1] == '"'
           && isalpha(linebuf[colno+2]))
    {
      std::string litstr =
        rps_lex_raw_literal_string(&_, inp, input_name, &linebuf, lineno, colno);
      return Rps_TwoValues(RPS_ROOT_OB(_62LTwxwKpQ802SsmjE), //string∈class
                           litstr);
    }
  //// a code chunk or macro string is mixing strings and
  //// objects.... Inspired by GCC MELT see
  //// starynkevitch.net/Basile/gcc-melt/MELT-Starynkevitch-DSL2011.pdf
  /* Improvement over GCC MELT: a macro string can start with "#{"
     ending with "}#" or "#a{" ending with "}a#" or "#ab{" ending with
     "}ab#" or "#abc{" ending with "}abc#" or "#abcd{" ending with
     "}abcd#" or "#abcde{" ending with "}abcde#" or "#abcdef{" ending
     with "}abcdef#" with the letters being arbitrary latin letters,
     upper or lowercase. but no more than 7 letters. */
  else if (linebuf[colno] == '#'
           && ((colno+2 < linelen  && linebuf[colno+1] == '{')
               || (colno+3 < linelen  && linebuf[colno+2] == '{'
                   && isalpha(linebuf[colno+1]))
               || (colno+4 < linelen  && linebuf[colno+3] == '{'
                   && isalpha(linebuf[colno+1]) && isalpha(linebuf[colno+2]))
               || (colno+5 < linelen  && linebuf[colno+4] == '{'
                   && isalpha(linebuf[colno+1]) && isalpha(linebuf[colno+2])
                   && isalpha(linebuf[colno+3]))
               ||  (colno+6 < linelen  && linebuf[colno+5] == '{'
                    && isalpha(linebuf[colno+1]) && isalpha(linebuf[colno+2])
                    && isalpha(linebuf[colno+3]) &&  isalpha(linebuf[colno+4]))
               ||  (colno+7 < linelen  && linebuf[colno+6] == '{'
                    && isalpha(linebuf[colno+1]) && isalpha(linebuf[colno+2])
                    && isalpha(linebuf[colno+3]) &&  isalpha(linebuf[colno+4])
                    &&  isalpha(linebuf[colno+5]))
               ||  (colno+8 < linelen  && linebuf[colno+7] == '{'
                    && isalpha(linebuf[colno+1]) && isalpha(linebuf[colno+2])
                    && isalpha(linebuf[colno+3]) &&  isalpha(linebuf[colno+4])
                    &&  isalpha(linebuf[colno+5]) &&  isalpha(linebuf[colno+6]))
               ||  (colno+9 < linelen  && linebuf[colno+8] == '{'
                    && isalpha(linebuf[colno+1]) && isalpha(linebuf[colno+2])
                    && isalpha(linebuf[colno+3]) &&  isalpha(linebuf[colno+4])
                    &&  isalpha(linebuf[colno+5]) &&  isalpha(linebuf[colno+6])
                    &&  isalpha(linebuf[colno+7]))
              ))
    {
      RPS_FATALOUT("rps_repl_lexer should call rps_lex_code_chunk inp@" << (void*)inp
                   << " input_name=" << input_name
                   << " line_buf='" << Rps_Cjson_String(linebuf) << "'"
                   << " lineno=" << lineno
                   << " colno=" << colno
                   << " curpos=" << linebuf+colno
                   << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer"));
#warning rps_repl_lexer should call rps_lex_code_chunk
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
              else
                goto lexical_error_backslash;
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




std::string
rps_lex_raw_literal_string(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int lineno, int& colno)
{
  /// For C++, raw literal strings are multi-line, and explained in
  /// en.cppreference.com/w/cpp/language/string_literal ... For
  /// example R"delim(raw characters \)delim" In RefPerSys, we
  /// restrict the <delim> to contain only letters, up to 15 of
  /// them...
  char delim[16];
  memset (delim, 0, sizeof(delim));
  RPS_ASSERT(callframe);
  RPS_ASSERT(plinebuf && *plinebuf);
  RPS_ASSERT(colno >= 0 && colno<(int)strlen(*plinebuf));
  RPS_ASSERT(input_name);
  int pos= -1;
  if (sscanf((*plinebuf)+colno, "R\"%15[A-Za-z](%n", delim, &pos) < 1
      || !isalpha(delim[0])
      || pos<=1)
    /// should never happen
    RPS_FATALOUT("corrupted rps_lex_raw_literal_string inp@" << (void*)inp
                 << " input_name=" << input_name
                 << " line_buf="  << (plinebuf?"'":"*") << (plinebuf?Rps_Cjson_String(*plinebuf):"*missing*") << (plinebuf?"'":"*")
                 << " lineno=" << lineno
                 << " colno=" << colno
                 << " curpos="  << (plinebuf?"'":"*") << (plinebuf?((*plinebuf)+colno):"*none*") << (plinebuf?"'":"*")
                 << " callframe=" << Rps_ShowCallFrame(callframe)
                 << std::endl
                 << RPS_FULL_BACKTRACE_HERE(1, "rps_lex_raw_literal_string"));
  char endstr[32];
  memset(endstr, 0, sizeof(endstr));
  snprintf(endstr, sizeof(endstr), ")%s\"", delim);
  std::string str;
  int curcolno = colno + pos;
  const char* pc = (*plinebuf) + curcolno;
  while (pc != nullptr)
    {
      const char*endp = strstr(pc, endstr);
      if (endp)
        {
          str.append(pc, endp-pc);
          colno = endp + strlen(endstr) - *plinebuf;
          return str;
        };
      str.append(pc);
      bool gotline = rps_repl_get_next_line(callframe, inp, input_name, plinebuf, &lineno, std::string{endstr});
      if (!gotline)
        return str;
      lineno++;
      colno = 0;
      curcolno = 0;
      pc = (*plinebuf);
    };
} // end rps_lex_raw_literal_string


Rps_Value
rps_lex_code_chunk(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int &lineno, int& colno)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
		 /*callerframe:*/callframe,
		 Rps_ObjectRef obchk;
		 Rps_Value inputnamestrv;
                );
  char endstr[16];
  char start[8];
  memset(endstr, 0, sizeof(endstr));
  memset(start, 0, sizeof(start));
  int pos= -1;
  RPS_ASSERT(plinebuf);
  RPS_ASSERT(input_name);
  const char*linbuf= *plinebuf;
  if (linbuf[colno] == '#' && linbuf[colno+1] == '{')
    strcpy(endstr, "}#");
  else if (sscanf(linbuf+colno, "#%6[a-zA-Z]{%n", start, &pos)>0 && pos>0)
    {
      RPS_ASSERT(strlen(endstr) < sizeof(endstr)-2);
      snprintf(endstr, sizeof(endstr), "}%s#", start);
    }
  colno += strlen(endstr);
  _f.obchk =
    Rps_ObjectRef::make_object(&_,
			       RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
			       nullptr);
  _f.inputnamestrv = Rps_StringValue(input_name);
  _f.obchk->put_attr2(RPS_ROOT_OB(_1B7ITSHTZWp00ektj1), //input∈symbol
		      _f.inputnamestrv,
		      RPS_ROOT_OB(_5FMX3lrhiw601iqPy5), //line∈symbol
		      Rps_Value((intptr_t)lineno, Rps_Value::Rps_IntTag{})
		      );
  // So we first need to create these attributes...
  auto paylvec = _f.obchk->put_new_plain_payload<Rps_PayloadVectOb>();
  RPS_ASSERT(paylvec);
  // TODO: we should add vector components to _f.obchk, reading several lines...
#warning very incomplete rps_lex_code_chunk, should be documented elsewhere...
  RPS_FATALOUT("unimplemented rps_lex_code_chunk inp@" << (void*)inp
               << " input_name=" << input_name
               << " line_buf="  << (plinebuf?"'":"*") << (plinebuf?Rps_Cjson_String(*plinebuf):"*missing*") << (plinebuf?"'":"*")
               << " lineno=" << lineno
               << " colno=" << colno
               << " curpos="  << (plinebuf?"'":"*") << (plinebuf?((*plinebuf)+colno):"*none*") << (plinebuf?"'":"*")
               << " callframe=" << Rps_ShowCallFrame(callframe)
               << std::endl
               << RPS_FULL_BACKTRACE_HERE(1, "rps_lex_code_chunk"));
} // end rps_lex_code_chunk

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

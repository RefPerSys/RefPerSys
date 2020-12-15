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
extern "C" bool rps_repl_stopped;

/// Interpret from either a given input stream,
/// or using readline if inp is null.
extern "C" void rps_repl_interpret(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, int& lineno);

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

/// for GNU readline autocompletion.  See example in
/// https://thoughtbot.com/blog/tab-completion-in-gnu-readline and
/// documentation in
/// https://tiswww.case.edu/php/chet/readline/readline.html#SEC45

extern "C"  std::vector<std::string> rps_completion_vect;
extern "C" char **rpsrepl_name_or_oid_completion(const char *, int, int);
extern "C" char *rpsrepl_name_or_oid_generator(const char *, int);

static Rps_CallFrame*rps_readline_callframe;
std::vector<std::string> rps_completion_vect;

bool rps_repl_stopped;

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
rps_repl_interpret(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, int& lineno)
{
  std::istream*previous_input=nullptr;
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(input_name != nullptr);
  // descriptor is: _6x4XcZ1fxp403uBUoz) //"rps_repl_interpret"∈core_function
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_6x4XcZ1fxp403uBUoz),
                           /*callerframe:*/callframe,
                           Rps_ObjectRef lexkindob;
                           Rps_Value lexdatav;
                );
  // a double ended queue to keep the lexical tokens
  std::deque<Rps_Value> token_deq;
  const char* linebuf=nullptr;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    for (auto tokenv : token_deq)
      gc->mark_value(tokenv);
  });
  RPS_DEBUG_LOG(REPL, "rps_repl_interpret start input_name=" << input_name
                << ", lineno=" << lineno
                << " callframe: " << Rps_ShowCallFrame(&_));
  previous_input = rps_repl_input;
  rps_repl_input = inp;
  bool endcommand = false;
  std::string prompt = std::string(input_name) + " RPS>";
  bool gotline = rps_repl_get_next_line(&_, inp, input_name, &linebuf, &lineno, prompt);
  if (gotline)
    {
      int colno = 0;
      while (!endcommand)
        {
          int startline = lineno;
          int startcol = colno;
          Rps_TwoValues lexpair = rps_repl_lexer(&_, inp, input_name, linebuf, lineno, colno);
          if (!lexpair.main())
            break;
          _f.lexkindob = lexpair.main().to_object();
          _f.lexdatav = lexpair.xtra();
          RPS_DEBUG_LOG(REPL, "rps_repl_interpret " << input_name << "L" << startline << "C" << startcol
                        << " lexkind=" << _f.lexkindob
                        << " lexdatav=" << _f.lexdatav);
#warning we need some condition on the lexing to stop it; perhaps stopping commands by double-semi-colon à la Ocaml
        };
    }
  else
    RPS_WARNOUT("rps_repl_interpret no line in " << input_name << "L" << lineno);
  RPS_FATALOUT("unimplemented rps_repl_interpret frame=" << Rps_ShowCallFrame(&_)
               << " inp=" << inp << " input_name=" << input_name
               << " lineno=" << lineno << std::endl
               << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_interpret"));
#warning rps_repl_interpret unimplemented, should call rps_repl_get_next_line then parse using rps_repl_lexer
  rps_repl_input = previous_input;
} // end rps_repl_interpret

bool
rps_repl_get_next_line(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int*plineno, std::string prompt)
{
  RPS_ASSERT(callframe);
  RPS_ASSERT(input_name);
  RPS_ASSERT(plinebuf);
  RPS_ASSERT(plineno);
  RPS_DEBUG_LOG(REPL, "rps_repl_get_next_line start inp=" << inp
                << " input_name=" << input_name
                << " *plineno=" << (*plineno)
                << " *plinebuf=" << (*plinebuf)
                << " prompt=" << prompt);
  if (inp && inp != &std::cin)   // we use it
    {
      if (inp->eof())
        {
          if (*plinebuf)
            free ((void*)*plinebuf), *plinebuf = nullptr;
          return false;
        }
      std::string linestr;
      std::getline(*inp, linestr);
      if (linestr.empty() || linestr[linestr.size()-1] != '\n')
        linestr.push_back('\n');
      if (*plinebuf)
        {
          free ((void*)*plinebuf), *plinebuf=nullptr;
          *plinebuf = strdup(linestr.c_str());
          RPS_ASSERT(*plinebuf); // strdup is unlikely to fail
        }
      (*plineno)++;
      RPS_DEBUG_LOG(REPL, "rps_repl_get_next_line  new *plinebuf='"
                    << (*plinebuf) << "' of length "
                    << strlen(*plinebuf)
                    << ", *plineno=" << *plineno
                    << ", linestr='" << linestr << "'");
      return true;
    }
  else // inp is null or std::cin
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
      RPS_DEBUG_LOG(REPL, "rps_repl_get_next_line before readline prompt=" << prompt
                    << " backtrace:" << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_get_next_line/before readline")
                    << std::endl
                    << "... callframe=" << Rps_ShowCallFrame(callframe)
                    << std::endl);
      free ((void*)*plinebuf), *plinebuf = nullptr;
      {
        char *rl = readline(prompt.c_str());
        RPS_DEBUG_LOG(REPL, "rps_repl_get_next_line " << (rl?"did readline '":"failed readline!")
                      << (rl?rl:"!")
                      << (rl?"'":"!-!")
                      << std::endl
                      << RPS_FULL_BACKTRACE_HERE(1,
                          (rl
                           ?"rps_repl_get_next_line/after readline"
                           :"rps_repl_get_next_line/failed readline"))
                      << std::endl
                      << "... callframe=" << Rps_ShowCallFrame(callframe)
                      << std::endl);
        *plinebuf = rl;
      }
      rps_readline_callframe = nullptr;
      if (*plinebuf)
        {
          // no need to increment the lineno, it was incremented by caller
          if (*plinebuf[0])
            add_history(*plinebuf);
          return true;
        };
    }
  free ((void*)*plinebuf), *plinebuf = nullptr;
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
                           Rps_Value chunkv;
                           Rps_Value semval;
                );
  RPS_ASSERT(colno >= 0 && colno <= linelen);
  RPS_DEBUG_LOG(REPL, "rps_repl_lexer start inp="<< inp
                << "input_name=" << (input_name?:"?nil?")
                << (linebuf?", linebuf=":", linebuf ")
                << (linebuf?linebuf:"*nil*")
                << ", lineno=" << lineno << ", colno=" << colno
                << ", linelen=" << linelen);
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
          _f.semval = Rps_DoubleValue(d);
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer float " << d << " colno=" << colno << " semval=" << _f.semval);
          return Rps_TwoValues{RPS_ROOT_OB(_98sc8kSOXV003i86w5), //double∈class
                               _f.semval};
        }
      else
        {
          colno += endint - startnum;
          _f.semval = Rps_Value::make_tagged_int(l);
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer int " << l << " colno=" << colno << " semval=" << _f.semval);
          return Rps_TwoValues{RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b), //int∈class
                               _f.semval};
        }
    }
  /// lex infinities (double) - but not NAN
  else if (!strncmp(linebuf+colno, "+INF", 4)
           || !strncmp(linebuf+colno, "-INF", 4))
    {
      bool pos = linebuf[colno] == '+';
      colno += 4;
      double infd = (pos
                     ?std::numeric_limits<double>::infinity()
                     : -std::numeric_limits<double>::infinity());
      _f.semval = Rps_DoubleValue(infd);
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer infinity " << infd << " colno=" << colno << " semval=" << _f.semval);
      return Rps_TwoValues{RPS_ROOT_OB(_98sc8kSOXV003i86w5), //double∈class
                           _f.semval};
    }
  //////////////// lex named objects or objids
  else if (isalpha(linebuf[colno]) || linebuf[colno]=='_')
    {
      int startnamecol = colno;
      while (isalnum(linebuf[colno]) || linebuf[colno]=='_')
        colno++;
      std::string namestr(linebuf+startnamecol, colno-startnamecol);
      _f.oblex = Rps_ObjectRef::find_object_by_string(&_, namestr, true);
      if (_f.oblex)
        {
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer object " << _f.oblex << " colno=" << colno
                        << " named " << namestr);
          return Rps_TwoValues(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ), //object∈class
                               _f.oblex);
        }
      /// some new symbol
      if (isalpha(namestr[0]))
        {
          _f.semval = Rps_StringValue(namestr);
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer new name " << namestr << " colno=" << colno << " semval=" << _f.semval);
          return Rps_TwoValues(RPS_ROOT_OB(_36I1BY2NetN03WjrOv), //symbol∈class
                               _f.semval);
        }
      /// otherwise, fail to lex, so
      {
        int oldcol = colno;
        colno = startnamecol;
        RPS_DEBUG_LOG(REPL, "rps_repl_lexer bad namestr " << namestr << " line " << lineno << ", column " << colno
                      << " oldcol " << oldcol);
        RPS_WARNOUT("rps_repl_lexer " << input_name << " line " << lineno << ", column " << colno
                    << " : bad name " << linebuf+colno);
        return Rps_TwoValues(nullptr, nullptr);
      }
    }
  //// literal strings are like in C++
  else if (linebuf[colno] == '"')   /// plain literal string, on a single line
    {
      std::string litstr =
        rps_lex_literal_string(input_name, linebuf, lineno, colno);
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer colno=" << colno << " literal string:" << Json::Value(litstr));
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer colno=" << colno << " multiline literal string:" << Json::Value(litstr));
      return Rps_TwoValues(RPS_ROOT_OB(_62LTwxwKpQ802SsmjE), //string∈class
                           litstr);
    }
  else if (linebuf[colno] == (char)0)   /// end of line
    {
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer eol lineno=" << lineno << " colno=" << colno
                    << " inputname=" << input_name << std::endl);
      return Rps_TwoValues{nullptr,nullptr};
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer start chunk  lineno#" << lineno
                    << ", colno#" << colno
                    << ", input_name=" << input_name
                    << " linebuf:" << linebuf);
      _f.chunkv = rps_lex_code_chunk(&_, inp, input_name, &linebuf, lineno, colno);
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer code chunk " << _f.chunkv
                    << " ending lineno#" << lineno
                    << ", colno#" << colno
                    << ", input_name=" << input_name);
      return Rps_TwoValues{RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           _f.chunkv};
    }

  else if (ispunct(linebuf[colno]))
    {
      RPS_FATALOUT("unimplemented delimiter rps_repl_lexer inp@" << (void*)inp
                   << " input_name=" << input_name
                   << " line_buf='" << Rps_Cjson_String(linebuf) << "'"
                   << " lineno=" << lineno
                   << " colno=" << colno
                   << " curpos=" << linebuf+colno
                   << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer"));
#warning unimplemented rps_repl_lexer for delimiter
    }

  RPS_FATALOUT("unimplemented rps_repl_lexer inp@" << (void*)inp
               << " input_name=" << input_name
               << " line_buf='" << Rps_Cjson_String(linebuf) << "'"
               << " lineno=" << lineno
               << " colno=" << colno
               << " curpos:" << linebuf+colno
               << std::endl
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

constexpr const unsigned rps_chunkdata_magicnum = 0x2fa19e6d; // 799121005
struct Rps_ChunkData_st
{
  unsigned chunkdata_magic;
  int chunkdata_lineno;
  int chunkdata_colno;
  char chunkdata_endstr[24];
  std::istream* chunkdata_inp;
  std::string chunkdata_input_name;
  const char**chunkdata_plinebuf;
};				// end Rps_ChunkData_st

Rps_Value
rps_lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg,  Rps_ChunkData_st*chkdata);

Rps_Value
rps_lex_code_chunk(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, const char**plinebuf, int &lineno, int& colno)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obchk;
                           Rps_Value inputnamestrv;
                           Rps_Value chunkelemv;
                );
  char endstr[16];
  char start[8];
  memset(endstr, 0, sizeof(endstr));
  memset(start, 0, sizeof(start));
  int pos= -1;
  RPS_ASSERT(plinebuf);
  RPS_ASSERT(input_name);
  const char*linbuf= *plinebuf;
  int startlineno = lineno;
  int startcolno = colno;
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
  RPS_DEBUG_LOG(REPL, "starting rps_lex_chunk " << input_name << "L" << startlineno << "C" << startcolno
                << " new obchk=" << _f.obchk);
  _f.inputnamestrv = Rps_StringValue(input_name);
  _f.obchk->put_attr2(RPS_ROOT_OB(_1B7ITSHTZWp00ektj1), //input∈symbol
                      _f.inputnamestrv,
                      RPS_ROOT_OB(_5FMX3lrhiw601iqPy5), //line∈symbol
                      Rps_Value((intptr_t)lineno, Rps_Value::Rps_IntTag{})
                     );
  // So we first need to create these attributes...
  auto paylvec = _f.obchk->put_new_plain_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvec);
  Rps_ChunkData_st chkdata;
  memset (&chkdata, 0, sizeof(chkdata));
  chkdata.chunkdata_magic = rps_chunkdata_magicnum;
  chkdata.chunkdata_lineno = lineno;
  chkdata.chunkdata_colno = colno;
  strcpy(chkdata.chunkdata_endstr, endstr);
  chkdata.chunkdata_inp = inp;
  chkdata.chunkdata_input_name.assign(input_name);
  chkdata.chunkdata_plinebuf = plinebuf;
  // TODO: we should add vector components to _f.obchk, reading several lines...
  do
    {
      RPS_DEBUG_LOG(REPL, "in rps_lex_chunk chunking "
                    << chkdata.chunkdata_input_name
                    << " L"<< chkdata.chunkdata_lineno
                    << ",C" << chkdata.chunkdata_colno
                    << ((*chkdata.chunkdata_plinebuf)?" linbuf:":" no linbuf")
                    << ((*chkdata.chunkdata_plinebuf)?:" **"));
      RPS_ASSERT(chkdata.chunkdata_magic == rps_chunkdata_magicnum);
      _f.chunkelemv = rps_lex_chunk_element(&_, _f.obchk, &chkdata);
      if (_f.chunkelemv)
        {
          RPS_DEBUG_LOG(REPL, "rps_lex_code_chunk pushing " << _f.chunkelemv
                        << " into " << _f.obchk << " @"
                        << chkdata.chunkdata_input_name
                        << " L"<< chkdata.chunkdata_lineno
                        << ",C" << chkdata.chunkdata_colno);
          paylvec->push_back(_f.chunkelemv);
          // see https://framalistes.org/sympa/arc/refpersys-forum/2020-12/msg00036.html
        }
      else
        RPS_DEBUG_LOG(REPL, "rps_lex_code_chunk no chunk into "
                      << _f.obchk << " @"
                      << chkdata.chunkdata_input_name
                      << " L"<< chkdata.chunkdata_lineno
                      << ",C" << chkdata.chunkdata_colno);
    }
  while (_f.chunkelemv);
  RPS_DEBUG_LOG(REPL, "ending rps_lex_chunk " << input_name << "L" << startlineno << "C" << startcolno
                << "-L" << chkdata.chunkdata_lineno << "C" << chkdata.chunkdata_colno
                << " obchk=" << _f.obchk);
  lineno = chkdata.chunkdata_lineno;
  colno = chkdata.chunkdata_colno;
  *plinebuf = *chkdata.chunkdata_plinebuf;
  chkdata.chunkdata_magic =0;
  return _f.obchk;
} // end rps_lex_code_chunk



/// Inside a code chunk represented by object obchkarg, parse some
/// chunk element...
Rps_Value
rps_lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg,  Rps_ChunkData_st*chkdata)
{
  RPS_ASSERT(chkdata != nullptr && chkdata->chunkdata_magic ==  rps_chunkdata_magicnum);
  RPS_ASSERT(callframe != nullptr && callframe->is_good_call_frame());
  RPS_ASSERT(chkdata->chunkdata_plinebuf != nullptr);
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obchunk;
                           Rps_Value chkelemv;
                           Rps_ObjectRef namedobv;
                );
  _f.obchunk = obchkarg;
  RPS_ASSERT(_f.obchunk);
  auto paylvect = _f.obchunk->get_dynamic_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvect != nullptr);
  const char*linestart = *chkdata->chunkdata_plinebuf;
  int linelen = strlen(linestart);
  RPS_ASSERT(chkdata->chunkdata_colno >= 0 && chkdata->chunkdata_colno<linelen);
  RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start obchunk=" << _f.obchunk
                << ", linestart='" << linestart << "', linelen=" << linelen
                << " @L" << chkdata->chunkdata_lineno << ",C"
                <<  chkdata->chunkdata_colno
                <<  " current:"
                << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                << linestart+chkdata->chunkdata_colno
                << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-start"));
  /// For C name-like things, we return the object naming them or else a string
  if (isalpha(linestart[chkdata->chunkdata_colno]))
    {
      int startnamcol = chkdata->chunkdata_colno;
      int endnamcol = startnamcol;
      while (endnamcol<linelen && (isalnum(linestart[endnamcol]) || linestart[endnamcol]=='_'))
        endnamcol++;
      int namlen = endnamcol-startnamcol;
      std::string namstr = std::string(linestart+startnamcol, namlen);
      _f.namedobv = Rps_ObjectRef::find_object_by_string(&_, namstr);
      chkdata->chunkdata_colno = endnamcol;
      if (_f.namedobv)
        {
          RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element name obchunk=" << _f.obchunk
                        << " -> namedobv=" << _f.namedobv
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.namedobv;
        }
      else
        {
          _f.chkelemv = Rps_StringValue(namstr);
          RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element string obchunk=" << _f.obchunk
                        << " -> chkelemv=" << _f.chkelemv
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.chkelemv;
        }
    }
  /// For sequence of spaces, we return an instance of class space and value the number of space characters
  else if (isspace(linestart[chkdata->chunkdata_colno]))
    {
      int startspacecol = chkdata->chunkdata_colno;
      int endspacecol = startspacecol;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start space obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      while (endspacecol<linelen && isspace(linestart[endspacecol]))
        endspacecol++;
      _f.chkelemv = Rps_InstanceValue(RPS_ROOT_OB(_2i66FFjmS7n03HNNBx), //space∈class
                                      std::initializer_list<Rps_Value>
      {
        Rps_Value((intptr_t)(endspacecol-startspacecol),
        Rps_Value::Rps_IntTag{})
      });
      chkdata->chunkdata_colno += endspacecol-startspacecol+1;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element space obchunk=" << _f.obchunk
                    << " -> chkelemv=" << _f.chkelemv
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      return _f.chkelemv;
    }
  /// code chunk meta-variable or meta-notation....
  else if (linestart[chkdata->chunkdata_colno] == '$'
           && chkdata->chunkdata_colno < linelen)
    {
      // a dollar followed by a name is a meta-variable; that name should be known
      const char*metastr = linestart+chkdata->chunkdata_colno;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start meta obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " metastr:" << metastr);
      if (isalpha(metastr[1]))
        {
          int startnameix=1, endnameix=1;
          while (isalnum(metastr[endnameix])||metastr[endnameix]=='_')
            endnameix++;
          std::string metaname(metastr+1, endnameix-startnameix);
          _f.namedobv = Rps_ObjectRef::find_object_by_string(&_, metaname);
          if (!_f.namedobv)
            {
              RPS_WARNOUT("rps_lex_chunk_element: bad metavariable name " << metaname
                          << " input " <<  chkdata->chunkdata_input_name
                          << " line " <<  chkdata->chunkdata_lineno << ", column " << chkdata->chunkdata_colno);
              throw std::runtime_error("lexical error - metaname in code chunk");
            }
          chkdata->chunkdata_colno += endnameix-startnameix+1;
          _f.chkelemv = Rps_InstanceValue(RPS_ROOT_OB(_1oPsaaqITVi03OYZb9), //meta_variable∈symbol
                                          std::initializer_list<Rps_Value> {_f.namedobv});
          return _f.chkelemv;
        }
      /// two dollars are parsed as one
      else if (metastr[1] == '$')
        {
          // two dollars should be parsed as a single one, and we make that a string with following letters...
          int startnameix=1, endnameix=2;
          while (isalnum(metastr[endnameix])||metastr[endnameix]=='_')
            endnameix++;
          std::string dollname(metastr+1, endnameix-startnameix);
          chkdata->chunkdata_colno +=  endnameix-startnameix+1;
          _f.chkelemv = Rps_StringValue(dollname);
          return _f.chkelemv;
        }
      /// a dollar followed by a dot is ignored....
      else if (metastr[1] == '.')
        {
          chkdata->chunkdata_colno += 2;
          return nullptr;
        }
      /// probably other dollar things should be parsed as delimiters....
    }
  else
    {
      const char*curstr = linestart+chkdata->chunkdata_colno;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element INCOMPLETE  callframe=" << Rps_ShowCallFrame(callframe)
                    << std::endl << "... obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " curstr:"
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                    << curstr
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                    << std::endl
                    <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-incomplete")
                   );
    }
#warning we need to document and implement other chunk element conventions in rps_lex_chunk_element, in particular delimiters...
  RPS_FATALOUT("unimplemented rps_lex_chunk_element callframe=" << Rps_ShowCallFrame(callframe)
               << " obchkarg=" << obchkarg
               << " chkdata=" << chkdata
               << " @L" << chkdata->chunkdata_lineno << ",C"
               <<  chkdata->chunkdata_colno);
#warning unimplemented rps_lex_chunk_element
  return nullptr;
} // end rps_lex_chunk_element



////////////////////////////////////////////////////////////////
//// the Rps_LexTokenZone values are transient, but do need some GC support

Rps_LexTokenZone::Rps_LexTokenZone(Rps_ObjectRef kindob, Rps_Value val, const Rps_String*filestringp, int line, int col)
  : Rps_LazyHashedZoneValue(Rps_Type::LexToken),
    lex_kind(kindob),
    lex_val(val),
    lex_file(filestringp),
    lex_lineno(line),
    lex_colno(col)
{
  RPS_ASSERT (!kindob || kindob->stored_type() == Rps_Type::Object);
  RPS_ASSERT (!filestringp || filestringp->stored_type() == Rps_Type::String);
} // end Rps_LexTokenZone::Rps_LexTokenZone

Rps_LexTokenZone::~Rps_LexTokenZone()
{
  lex_kind = nullptr;
  lex_val = nullptr;
} // end Rps_LexTokenZone::~Rps_LexTokenZone

Rps_HashInt
Rps_LexTokenZone::compute_hash(void) const
{
  auto hkind = lex_kind?(lex_kind->val_hash()):0;
  auto hval = lex_val?(lex_val.valhash()):0;
  auto hfil = lex_file?(lex_file->val_hash()):0;
  // all the constants below are primes
  uint64_t h1 = (hkind * 45887) ^ (hval * 75937);
  uint64_t h2 = (hfil * 85817) + (lex_lineno * 85931 - lex_colno * 8573);
  Rps_HashInt h = (Rps_HashInt)(h1 + h2);
  if (!h)
    h = (h1&0xffff) + (h2&0xfffff) + 17;
  RPS_ASSERT(h != 0);
  return h;
} // end Rps_LexTokenZone

Rps_ObjectRef
Rps_LexTokenZone::the_lexical_token_class(void)
{
  return RPS_ROOT_OB(_0S6DQvp3Gop015zXhL);
} // end Rps_LexTokenZone::the_lexical_token_class

Rps_ObjectRef
Rps_LexTokenZone::compute_class(Rps_CallFrame*callframe) const
{
  // we need to create some lexical_token class object...
  RPS_ASSERT(callframe == nullptr || callframe->is_good_call_frame());
  return the_lexical_token_class();
} // end Rps_LexTokenZone::compute_class

void
Rps_LexTokenZone::gc_mark(Rps_GarbageCollector&gc, unsigned depth) const
{
  if (is_gcmarked(gc))
    return;
  if (RPS_UNLIKELY(depth > Rps_Value::max_gc_mark_depth))
    throw std::runtime_error("too deep Rps_LexTokenZone::gc_mark");
  if (lex_kind)
    lex_kind->gc_mark(gc,depth+1);
  if (lex_val)
    lex_val.gc_mark(gc,depth+1);
  if (lex_file)
    lex_file->gc_mark(gc,depth+1);
} // end Rps_LexTokenZone::gc_mark

void
Rps_LexTokenZone::dump_scan(Rps_Dumper*du, unsigned int) const
{
  RPS_ASSERT(du != nullptr);
} // end Rps_LexTokenZone::dump_scan

Json::Value
Rps_LexTokenZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  return Json::Value (Json::nullValue);
} // end Rps_LexTokenZone::dump_json


void
Rps_LexTokenZone::val_output(std::ostream&out, unsigned int depth) const
{
  out << "LexToken{kind=" << lex_kind;
  if (depth > Rps_Value::max_output_depth)
    {
      out << "...}";
      return;
    };
  out << ", val=";
  lex_val.output(out, depth+1);
  if (lex_file)
    {
      out << ", @" << lex_file->cppstring() << ":" << lex_lineno << ":" << lex_colno;
    };
  out << "}";
} // end Rps_LexTokenZone::val_output


bool
Rps_LexTokenZone::equal(Rps_ZoneValue const&zv) const
{
  if (zv.stored_type() == Rps_Type::LexToken)
    {
      auto othlt = reinterpret_cast<const Rps_LexTokenZone*>(&zv);
      auto lh = lazy_hash();
      auto othlh = othlt->lazy_hash();
      if (lh != 0 && othlh != 0 && lh != othlh)
        return false;
      if (lex_file && othlt->lex_file)
        {
          if (lex_lineno != othlt->lex_lineno)
            return false;
          if (lex_colno != othlt->lex_colno)
            return false;
        };
      if (lex_kind != othlt->lex_kind)
        return false;
      if (!lex_file && !othlt->lex_file)
        return lex_val == (othlt->lex_val);
      return lex_val == othlt->lex_val && lex_file == othlt->lex_file;
    }
  return false;
} // end Rps_LexTokenZone::equal

bool
Rps_LexTokenZone::less(Rps_ZoneValue const&zv) const
{
  if (zv.stored_type() == Rps_Type::LexToken)
    {
      if (equal(zv))
        return false;
      auto othlt = reinterpret_cast<const Rps_LexTokenZone*>(&zv);
      if (!lex_file && othlt->lex_file)
        return true;
      if (!othlt->lex_file)
        return false;
      if (*lex_file < *othlt->lex_file)
        return true;
      else if (*lex_file > *othlt->lex_file)
        return false;
      if (lex_lineno < othlt->lex_lineno)
        return true;
      else if (lex_lineno > othlt->lex_lineno)
        return false;
      if  (lex_colno < othlt->lex_colno)
        return true;
      else if (lex_colno > othlt->lex_colno)
        return false;
      if (lex_kind < othlt->lex_kind)
        return true;
      else if (lex_kind > othlt->lex_kind)
        return false;
      if (lex_val < othlt->lex_val)
        return true;
      else if (lex_val > othlt->lex_val)
        return false;
      RPS_FATALOUT("Rps_LexTokenZone::less corruption: this=" << *this
                   << ", other=" << *othlt);
    }
  else
    return  Rps_Type::LexToken < zv.stored_type();
} // end Rps_LexTokenZone::less

// Tokenize a lexical token; an optional double-ended queue of
// already lexed token enable limited backtracking when needed....
const Rps_LexTokenZone*
Rps_LexTokenZone::tokenize(Rps_CallFrame*callframe, std::istream*inp,
                           const char*input_name,
                           const char**plinebuf, int&lineno, int& colno,
                           lexical_line_getter_fun linegetter,
                           std::deque<Rps_LexTokenZone*>* pque)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0TvVIbOU16z028VWvv),
                           /*callerframe:*/callframe,
                           Rps_ObjectRef lexkindob;
                           Rps_Value lexdatav;
                           Rps_Value lextokv;
                           Rps_Value kindnamv;
                           Rps_Value resv;
                );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    if (pque)
      {
        for (Rps_LexTokenZone* lxtokv: *pque)
          gc->mark_value(lxtokv);
      };
  });
  std::string inputstr(input_name?:"");
  const char*curinp = (plinebuf && colno>=0 && colno<(int)strlen(*plinebuf))
                      ?((*plinebuf)+colno)
                      :nullptr;
  int pquelen = pque?pque->size():-1;
  RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize start inputstr="
                << inputstr << (curinp?"curinp=":"curinp ")
                << (curinp?curinp:" *NUL*")
                << ", lineno=" << lineno
                << ", colno=" << colno
                << (pque?", pque len: ":",no pque ")
                << pquelen);
  if (pquelen>0)
    {
      _f.lextokv = pque->front();
      pque->pop_front();
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize dequeued " << _f.lextokv);
      return _f.lextokv.to_lextoken();
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize inputstr="
                    << inputstr << ", lineno=" << lineno
                    << ", colno=" << colno
                    << (curinp?"curinp='":"curinp ")
                    << (curinp?curinp:" *NUL*")
                    << (curinp?"' ":"")
                    << " before calling rps_repl_lexer" << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_LexTokenZone::tokenize"));
      int startline = lineno;
      int startcol = colno;
      int linelen= (*plinebuf)?((int)(strlen(*plinebuf))):0;
      while (colno<linelen && isspace((*plinebuf)[colno]))
        colno++;
      curinp = (plinebuf && colno>=0 && colno<(int)strlen(*plinebuf))
               ?((*plinebuf)+colno)
               :nullptr;
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize just  before rps_repl_lexer inputstr="
                    << inputstr << ", lineno=" << lineno
                    << ", colno=" << colno
                    << (curinp?", curinp='":", curinp ")
                    << (curinp?curinp:" *NUL*")
                    << (curinp?"' ":""));
      Rps_TwoValues twolex =
        rps_repl_lexer(&_, inp, input_name, *plinebuf, lineno, colno);
      _f.lexkindob = twolex.main_val.as_object();
      _f.lextokv = twolex.xtra_val;
      _f.kindnamv = nullptr;
      _f.kindnamv = _f.lexkindob
                    ->get_attr1(&_,RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); // /name∈named_attribute
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize from rps_repl_lexer got lexkindob=" << _f.lexkindob
                    << "/" << _f.kindnamv
                    << ", lextok=" << _f.lextokv
                    << ", lineno=" << lineno
                    << ", colno=" << colno);
      {
        Rps_LexTokenZone* lextok =
          Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
          (_f.lexkindob,
           _f.lextokv,
           Rps_StringValue(input_name).as_string(),
           startline,
           startcol);
        _f.resv = Rps_LexTokenValue(lextok);
        RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize gives " << _f.resv);
        return lextok;
      }
    }
  RPS_FATALOUT("unimplemented Rps_LexTokenZone::tokenize inputstr="
               << inputstr << " line:" << lineno
               << ", column:" << colno
               << (curinp?", current input:":", no input")
               << (curinp?curinp:" ..."));
#warning incomplete Rps_LexTokenZone::tokenize, should wrap rps_repl_lexer
} // end Rps_LexTokenZone::tokenize
////////////////////////////////////////////////////////////////



char **
rpsrepl_name_or_oid_completion(const char *text, int start, int end)
{
  /* Notice that the start and end are byte indexes, and that matters
   *  with UTF-8. */
  RPS_DEBUG_LOG(COMPL_REPL, "text='" << text << "' start=" << start
                << ", end=" << end
		<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rpsrepl_name_or_oid_completion"));
  rps_completion_vect.clear();
  int nbmatch = 0;
  // for objid, we require four characters including the leading
  // underscore to autocomplete...
  if (end>start+4 && text[start] == '_' && isdigit(text[start+1])
      && isalnum(text[start+2]) && isalnum(text[start+4]))
    {
      std::string prefix(text+start, end-start);
      RPS_DEBUG_LOG(COMPL_REPL, "oid autocomplete prefix='" << prefix << "'");
      // use oid autocompletion, with
      // Rps_ObjectZone::autocomplete_oid...
      nbmatch = Rps_ObjectZone::autocomplete_oid
                (prefix.c_str(),
                 [&] (const Rps_ObjectZone* obz)
      {
        RPS_ASSERT(obz != nullptr);
        Rps_Id oid = obz->oid();
        RPS_DEBUG_LOG(COMPL_REPL, "oid autocomplete oid=" << oid);
        rps_completion_vect.push_back(oid.to_string());
        return false;
      });
      RPS_DEBUG_LOG(COMPL_REPL, "oid autocomplete prefix='" << prefix << "' -> nbmatch=" << nbmatch);
    }
  // for names, we require two characters to autocomplete
  else if (end>start+2 && isalpha(text[start]) && (isalnum(text[start+1]) || text[start+1]=='_'))
    {
      // use symbol name autocompletion, with
      // Rps_PayloadSymbol::autocomplete_name...
      std::string prefix(text+start, end-start);
      RPS_DEBUG_LOG(COMPL_REPL, "name autocomplete prefix='" << prefix << "'");
      nbmatch =  Rps_PayloadSymbol::autocomplete_name
                 (prefix.c_str(),
                  [&] (const Rps_ObjectZone*obz, const std::string&name)
      {
        RPS_ASSERT(obz != nullptr);
        RPS_DEBUG_LOG(COMPL_REPL, "symbol autocomplete name=" << name);
        rps_completion_vect.push_back(name);
        return false;
      });
      RPS_DEBUG_LOG(COMPL_REPL, "name autocomplete prefix='" << prefix << "' -> nbmatch=" << nbmatch);
    }
  if (RPS_DEBUG_ENABLED(COMPL_REPL))
    {
      int ix=0;
      for (auto str: rps_completion_vect)
        {
          RPS_DEBUG_LOG(COMPL_REPL, "[" << ix << "]='" << str << "'");
          ix++;
        }
    }
  if (nbmatch==0)
    return nullptr;
  else
    {
      rl_attempted_completion_over = 1;
      return rl_completion_matches(text, rpsrepl_name_or_oid_generator);
    }
} // end rpsrepl_name_or_oid_completion

char *
rpsrepl_name_or_oid_generator(const char *text, int state)
{
  /// the initial state is 0....
  RPS_DEBUG_LOG(COMPL_REPL, "text='" << text << "' state#" << state << " rps_completion_vect.size="
                << rps_completion_vect.size() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rpsrepl_name_or_oid_generator"));
  if (rps_completion_vect.size() == 1)
    {
      if (state==0)
        return strdup(rps_completion_vect[0].c_str());
      else
        return nullptr;
    }
  RPS_WARNOUT("rpsrepl_name_or_oid_generator incomplete text=='" << text << "' state#" << state
              << " with " <<rps_completion_vect.size() << " completions");
#warning rpsrepl_name_or_oid_generator incomplete...
  return nullptr;
} // end rpsrepl_name_or_oid_generator

void
rps_read_eval_print_loop(int &argc, char **argv)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/nullptr,
                );
  for (int ix=0; ix<argc; ix++)
    RPS_DEBUG_LOG(REPL, "REPL arg [" << ix << "]: " << argv[ix]);
  RPS_ASSERT(rps_is_main_thread());
  RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop start frame=" << Rps_ShowCallFrame(&_));
  char *linebuf = nullptr;
  int lineno=0;
  int count=0;
  rl_attempted_completion_function = rpsrepl_name_or_oid_completion;
  while (!rps_repl_stopped)
    {
      count++;
      char prompt[16];
      memset(prompt, 0, sizeof(prompt));
      snprintf(prompt, sizeof(prompt), "Rps_REPL#%d", count);
      RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop lineno=" << lineno << " prompt=" << prompt);
      if (count % 4 == 0)
        usleep(128*1024);
      rps_repl_interpret(&_, nullptr, prompt, lineno);
      RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop done prompt=" << prompt << std::endl);
    };
} // end of rps_read_eval_print_loop


void
rps_repl_lexer_test(void)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0S6DQvp3Gop015zXhL),  //lexical_token∈class
                           /*callerframe:*/nullptr,
                           Rps_Value curlextokenv;
                );
  RPS_ASSERT(rps_is_main_thread());
  double startrealtime = rps_wallclock_real_time();
  double startcputime = rps_thread_cpu_time();
  RPS_DEBUG_LOG(REPL, "start rps_repl_lexer_test gitid " << rps_gitid
                << " callframe:" << Rps_ShowCallFrame(&_));
  const char *linebuf = nullptr;
  int lineno=0;
  int colno=0;
  int count=0;
  int nbtok=0;
  int oldcolno= 0;
  int oldlineno= 0;
  rl_attempted_completion_function = rpsrepl_name_or_oid_completion;
  while (!rps_repl_stopped)
    {
      char prompt[32];
      memset(prompt, 0, sizeof(prompt));
      if (count % 4 == 0)
        {
          usleep(32768); // to slow down on infinite loop
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test startloop count=" << count
                        << " lineno=" << lineno
                        << " colno=" << colno
                        << std::endl
                        <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer_test startloop"));
        };
      count++;
      oldcolno = colno;
      oldlineno = lineno;
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test looping count#" << count
                    << " oldcolno=" << oldcolno
                    << " oldlineno=" << oldlineno
                    << (linebuf?" linebuf='":"linbuf!")
                    << (linebuf?linebuf:"*nul*")
                    << (linebuf?"'":"!")
                    << std::endl
                    << ((linebuf&&colno<strlen(linebuf))?"... curptr'":"... ?no ")
                    << ((linebuf&&colno<strlen(linebuf))?(linebuf+colno):"*curptr*")
                    << ((linebuf&&colno<strlen(linebuf))?"'":"**"));
      if (linebuf==nullptr || colno>=(int)strlen(linebuf))
        {
          snprintf(prompt, sizeof(prompt), "Rps_LEXTEST#%d:", count);
          lineno++;
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test lineno=" << lineno << " prompt=" << prompt
                        << ", count=" << count);
          bool gotline = rps_repl_get_next_line(&_, &std::cin, prompt, &linebuf, &lineno, prompt);
          if (!gotline)
            break;
        }
      if (linebuf && colno < (int)strlen(linebuf))
        {
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test lineno=" << lineno
                        << ", colno=" << colno
                        << ", linebuf:" << linebuf);
          _f.curlextokenv =
            Rps_LexTokenZone::tokenize
            (&_, &std::cin,
             prompt,
             &linebuf,
             lineno,
             colno,
             [=,&nbtok](Rps_CallFrame*lex_callframe,
                        std::istream*lex_inp,
                        const char*lex_input_name,
                        const char**lex_plinebuf, int*lex_plineno)
          {
            bool ok =
              rps_repl_get_next_line(lex_callframe,
                                     lex_inp,
                                     lex_input_name,
                                     lex_plinebuf,
                                     lex_plineno,
                                     prompt);
            RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test⊕ ok=" << ok
                          << " lineno="<< lineno
                          << " colno="<< colno
                          << " oldlineno=" << oldlineno
                          << " oldcolno=" << oldcolno
                          << " lex_callframe=" << Rps_ShowCallFrame(lex_callframe)
                          << std::endl);
            usleep(1000);
            return ok;
          }
            );
          RPS_INFORMOUT("rps_repl_lexer_test curlextokenv=" << _f.curlextokenv << std::endl
                        << " old.line#" << oldlineno
                        << ", col#" << oldcolno
                        << " now line#" << lineno
                        << ", col#" << colno
                        << " count#" << count
                        <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer_test") << std::endl);
          if (_f.curlextokenv)
            {
              usleep (1000);
              nbtok++;
              RPS_ASSERT (colno != oldcolno || lineno != oldlineno);
            }
          else
            rps_repl_stopped = true;
        }
      int linbuflen = (int)strlen(linebuf);
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test endloop nbtok=" << nbtok << ", count=" << count
                    << ", lineno=" << lineno << ", colno=" << colno
                    << ", oldlineno=" << oldlineno
                    << ", oldcolno=" << oldcolno
                    << "," << std::endl
                    << ((linebuf&&colno<linbuflen)?"... curptr'":"... ?no ")
                    << ((linebuf&&colno<linbuflen)?(linebuf+colno):"*curptr*")
                    << ((linebuf&&colno<linbuflen)?"'":"**")
                    << "... last curlextokenv=" << _f.curlextokenv << std::endl);
      if (count % 4 == 0)
        {
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test endloop #!# count=" << count
                        << " nbtok=" << nbtok
                        << " curlextokenv=" << _f.curlextokenv
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer_test endloop")
                        << std::endl);
          usleep (200000);
        }
      else
        usleep (1000);
    } // end while (!rps_repl_stopped
  //
  RPS_DEBUG_LOG(REPL, "ending rps_repl_lexer_test lineno=" << lineno << ", colno=" << colno
                << ", count=" << count
                << ", nbtok=" << nbtok
                << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer_test ending")
                << std::endl);
  double endrealtime = rps_wallclock_real_time();
  double endcputime = rps_thread_cpu_time();
  RPS_INFORMOUT("rps_repl_lexer_test got " << nbtok << " lexical tokens in "
                << (endrealtime-startrealtime) << " real, "
                << (endcputime-startcputime) << " cpu seconds.");
} // end rps_repl_lexer_test

// end of file repl_rps.cc

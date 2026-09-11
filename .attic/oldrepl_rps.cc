/****************************************************************
 * file attic/oldrepl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has old unused code from repl_rps.cc
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2023 The Reflective Persistent System Team
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

extern "C" const char rps_oldrepl_gitid[];
const char rps_oldrepl_gitid[]= RPS_GITID;

extern "C" const char rps_oldrepl_date[];
const char rps_oldrepl_date[]= __DATE__;


#if 0 && oldcode
void
rps_repl_interpret_token_source(Rps_CallFrame*callframe, Rps_TokenSource& toksource)
{
  [[maybe_unused]] std::istream *previous_input = nullptr;

  RPS_ASSERT(rps_is_main_thread());
  // descriptor is: _6x4XcZ1fxp403uBUoz) //"rps_repl_interpret_token_source"∈core_function
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_6x4XcZ1fxp403uBUoz),
                           /*callerframe:*/callframe,
                           Rps_Value lextokenv;
                           Rps_Value lexval;
                );
  // a double ended queue to keep the lexical tokens
  Rps_DequVal token_deq(__FILE__,__LINE__);
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    token_deq.gc_mark(gc);
  });
  std::string startpos = toksource.position_str();
  RPS_DEBUG_LOG(REPL, "rps_repl_interpret_token_source start "
                << startpos
                << " callframe: " << Rps_ShowCallFrame(&_));
  bool endcommand = false;

  [[maybe_unused]] int cmdcount = 0;

  {
    while (!endcommand)
      {
        std::string commandpos = toksource.position_str();
        try
          {
            _f.lextokenv = toksource.get_token(&_);
            RPS_DEBUG_LOG(REPL, "rps_repl_interpret_token_source commandpos:" << commandpos << " lextokenv=" << _f.lextokenv
                          << " currentpos:" << toksource.position_str());
            if (!_f.lextokenv)
              {
                endcommand = true;
                break;
              }
            RPS_ASSERT(_f.lextokenv.is_lextoken());
            const Rps_LexTokenZone* lextokz = _f.lextokenv.as_lextoken();
            RPS_ASSERT(lextokz);
            if (lextokz->lxkind()
                != RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ))  //object∈class
              {
                RPS_WARNOUT("rps_repl_interpret_token_source command at "
                            << commandpos
                            << " should start with an object but got "
                            << _f.lextokenv);
                continue;
              }
            _f.lexval = lextokz->lxval();
            RPS_DEBUG_LOG(REPL, "rps_repl_interpret_token_source commandpos:" << commandpos << " lexval=" << _f.lexval);
            ///
#warning unimplemented rps_repl_interpret_token_source
            RPS_FATALOUT("rps_repl_interpret_token_source unimplemented commandpos " << commandpos << " lextokenv=" << _f.lextokenv);
          } // ending try...
        catch (std::exception& exc)
          {
            RPS_WARNOUT("rps_repl_interpret_token_source failed to interpret " << commandpos
                        << " got exception " << exc.what());
          }
#warning we need some condition on the lexing to stop it; perhaps stopping commands by double-semi-colon à la Ocaml
      };
  }
} // end rps_repl_interpret_token_source



Rps_TwoValues
rps_repl_cmd_lexing(Rps_CallFrame*callframe, unsigned lookahead)
{
  RPS_DEBUG_LOG(REPL, "rps_repl_cmd_lexing callframe:"<< std::endl
                << Rps_ShowCallFrame(callframe)
                << std::endl << "lookahead=" << lookahead
                << " rps_repl_cmd_lexer_fun " << (rps_repl_cmd_lexer_fun?"set":"clear")
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_cmd_lexing"));
  if (rps_repl_cmd_lexer_fun)
    return rps_repl_cmd_lexer_fun(callframe,lookahead);
  else
    return Rps_TwoValues{nullptr,nullptr};
} // end of rps_repl_cmd_lexing



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

  static long cnt;
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(input_name != nullptr);
  RPS_ASSERT(linebuf != nullptr);
  int linelen = strlen(linebuf);
  RPS_ASSERT(callframe != nullptr);
  cnt++;
  long callcnt = cnt;
  /// literal string prefix, à la C++
  char litprefix[16];
  memset (litprefix, 0, sizeof(litprefix));
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ObjectRef oblex;
                 Rps_Value chunkv;
                 Rps_Value semval;
                 Rps_ObjectRef obdictdelim;
                 Rps_Value delimv;
                );
  RPS_ASSERT(colno >= 0 && colno <= linelen);
  RPS_DEBUG_LOG(REPL, "rps_repl_lexer start callcnt#" << callcnt <<" inp@"<< inp
                << ", input_name=" << (input_name?:"?nil?")
                << (linebuf?", linebuf='":", linebuf ")
                << (linebuf?linebuf:"*nil*")
                << (linebuf?"'":"")
                << ", lineno=" << lineno << ", colno=" << colno
                << ", linelen=" << linelen
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer/start"));
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
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer callcnt#" << callcnt <<" => float " << d << " colno=" << colno << " semval=" << _f.semval);
          return Rps_TwoValues{RPS_ROOT_OB(_98sc8kSOXV003i86w5), //double∈class
                               _f.semval};
        }
      else
        {
          colno += endint - startnum;
          _f.semval = Rps_Value::make_tagged_int(l);
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer callcnt#" << callcnt <<" => int " << l << " colno=" << colno << " semval=" << _f.semval);
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer callcnt#" << callcnt <<" => infinity " << infd << " colno=" << colno << " semval=" << _f.semval);
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer: namestr=" << namestr << " @L" << lineno << "C" << startnamecol
                    << std::endl << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer/name"));
      _f.oblex = Rps_ObjectRef::find_object_or_fail_by_string(&_, namestr);
      if (_f.oblex)
        {
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer  callcnt#" << callcnt <<" => object " << _f.oblex << " colno=" << colno
                        << " named " << namestr);
          return Rps_TwoValues(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ), //object∈class
                               _f.oblex);
        }
      else
        RPS_DEBUG_LOG(REPL, "rps_repl_lexer: new namestr=" << namestr);
      /// some new symbol
      if (isalpha(namestr[0]))
        {
          _f.semval = Rps_StringValue(namestr);
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer  callcnt#" << callcnt <<" => new name " << namestr << " colno=" << colno << " semval=" << _f.semval);
          return Rps_TwoValues(RPS_ROOT_OB(_36I1BY2NetN03WjrOv), //symbol∈class
                               _f.semval);
        }
      /// otherwise, fail to lex, so
      {
        int oldcol = colno;
        colno = startnamecol;
        RPS_DEBUG_LOG(REPL, "rps_repl_lexer:  callcnt#" << callcnt <<" bad namestr " << namestr << " line " << lineno << ", column " << colno
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer  callcnt#" << callcnt <<" => colno=" << colno << " literal string:" << Json::Value(litstr));
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer   callcnt#" << callcnt <<" => colno=" << colno << " multiline literal string:" << Json::Value(litstr));
      return Rps_TwoValues(RPS_ROOT_OB(_62LTwxwKpQ802SsmjE), //string∈class
                           litstr);
    }
  else if (linebuf[colno] == (char)0)   /// end of line
    {
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer   callcnt#" << callcnt <<" => eol lineno=" << lineno << " colno=" << colno
                    << " inputname=" << input_name << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer/EOL"));
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
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer  callcnt#" << callcnt <<" => code chunk " << _f.chunkv
                    << " ending lineno#" << lineno
                    << ", colno#" << colno
                    << ", input_name=" << input_name);
      return Rps_TwoValues{RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           _f.chunkv};
    }

  else if (ispunct(linebuf[colno]))
    {
      constexpr int max_delim_len = 5;
      _f.obdictdelim = RPS_ROOT_OB(_627ngdqrVfF020ugC5); //"repl_delim"∈string_dictionary
      auto paylstrdict = _f.obdictdelim->get_dynamic_payload<Rps_PayloadStringDict>();
      RPS_ASSERT (paylstrdict != nullptr);
      std::string delimstr;
      int startcolno = colno;
      int endcolno = colno;
      while (ispunct(linebuf[endcolno]) && endcolno < linelen && endcolno < startcolno + max_delim_len)
        endcolno++;
      delimstr = std::string(linebuf+startcolno, endcolno-startcolno);
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer maybe delimiter " << delimstr
                    << " input_name=" << input_name
                    << " line_buf='" << Rps_Cjson_String(linebuf) << "'"
                    << " lineno=" << lineno
                    << " colno=" << colno
                    << " endcolno=" << endcolno);
      while (!delimstr.empty() && endcolno>startcolno)
        {
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer candidate delim " << delimstr << " L" << lineno << "C" << startcolno);
          _f.delimv = paylstrdict->find(delimstr);
          if (_f.delimv)
            {
              RPS_DEBUG_LOG(REPL, "rps_repl_lexer  callcnt#" << callcnt <<" => delim " << delimstr << " L" << lineno << "C" << startcolno
                            << " as " << _f.delimv);
              colno = endcolno;
              return Rps_TwoValues(RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK), //repl_delimiter∈class
                                   _f.delimv);
              break;
            }
          else
            endcolno--;
        }
      if (endcolno <= startcolno)
        {
          RPS_WARNOUT("unknown delimiter rps_repl_lexer inp@" << (void*)inp
                      << " input_name=" << input_name
                      << " line_buf='" << Rps_Cjson_String(linebuf) << "'"
                      << " lineno=" << lineno
                      << " colno=" << colno
                      << " curpos=" << linebuf+colno << std::endl
                      << " delimiter: " << Rps_Cjson_String(delimstr) <<" callcnt#" << callcnt
                      << std::endl
                      << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer/unknown delim"));
          return Rps_TwoValues(nullptr, nullptr);
        }
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
#warning incompletely unimplemented rps_repl_lexer
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
  int startlineno= lineno;
  int startcolno= colno;
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
  RPS_DEBUG_LOG(REPL, "rps_lex_raw_literal_string end  input_name=" << input_name
                << " start L"<< startlineno
                << ",C" << startcolno
                << " end L" << lineno
                << ",C" << colno
                << " str='" << str << "'"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_lex_raw_literal_string end"));
  return str;
} // end rps_lex_raw_literal_string
#endif /*0 && oldcode*/





//- Rps_Value
//- rps_repl_cmd_tokenizer(Rps_CallFrame*lexcallframe,
//-                        Rps_ObjectRef cmdreplobarg,
//-                        Rps_Value cmdparserarg,
//-                        unsigned lookahead,
//-                        Rps_DequVal& token_deq,
//-                        const char*input_name,
//-                        const char*&linebuf, int &lineno, int &colno,
//-                        std::string prompt
//-                       )
//- {
//-   static long tokencount;
//-   RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
//-                            /*callerframe:*/lexcallframe,
//-                            Rps_ObjectRef lexkindob;
//-                            Rps_ObjectRef cmdreplob;
//-                            Rps_Value cmdparserv;
//-                            Rps_Value lexdatav;
//-                            Rps_Value lextokenv;
//-                            Rps_Value parsmainv;
//-                            Rps_Value parsxtrav;
//-                 );
//-   _f.cmdreplob = cmdreplobarg;
//-   _f.cmdparserv = cmdparserarg;
//-   int startline = lineno;
//-   int startcol = colno;
//-   int eol = linebuf?strlen(linebuf):(-1);
//-   const char*curp = (linebuf && lineno < eol)?(linebuf+lineno):nullptr;
//-   RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer°start input_name=" << input_name
//-                 << " linebuf=" << linebuf << std::endl
//-                 << "... L" << lineno << "C" << colno
//-                 << " prompt=" << prompt
//-                 << " cmdreplob=" << _f.cmdreplob
//-                 << " cmdparserv=" << _f.cmdparserv
//-                 << " lookahead=" << lookahead << std::endl
//-                 << "...curframe:" << Rps_ShowCallFrame(&_)
//-                 << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_cmd_tokenizer")
//-                 << "...@curp='" << Rps_Cjson_String(curp) << "'"  << " calldepth=" << _.call_frame_depth() << std::endl);
//-   while (lookahead > token_deq.size())
//-     {
//-       RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer§ input_name=" << input_name
//-                     << " linebuf=" << linebuf << std::endl
//-                     << "... L" << lineno << "C" << colno
//-                     << "...lookahead=" << lookahead  << " token_deq.size=" << token_deq.size());
//-       {
//-         RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer lookahead=" << lookahead
//-                       << " need lexing since token_deq.size=" << token_deq.size()
//-                       <<  " @"
//-                       << input_name << "L" << startline << "C" << startcol
//-                       << std::endl
//-                       << " curframe:" << Rps_ShowCallFrame(&_));
//-         Rps_TwoValues lexpair = rps_repl_lexer(&_, rps_repl_input,   input_name, linebuf, lineno, colno);
//-         _f.lexkindob = lexpair.main().to_object();
//-         _f.lexdatav = lexpair.xtra();
//-         RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer lexkindob=" << _f.lexkindob
//-                       << " lexdatav=" << _f.lexdatav
//-                       << " @" << input_name
//-                       << "L" << lineno
//-                       << "C" << colno);
//-         if (!_f.lexkindob)
//-           return Rps_Value{nullptr};
//-         _f.lextokenv
//-           = Rps_LexTokenZone::tokenize(&_,
//-                                        rps_repl_input,   input_name, &linebuf, lineno, colno,
//-                                        [&](Rps_CallFrame*tokencallframe,
//-                                            std::istream*tokeninp,
//-                                            const char*tokeninputname,
//-                                            const char**tokenplinebuf,
//-                                            int*tokenplineno)
//-         {
//-           RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer getnextline tokeninputname="
//-                         << tokeninputname << " tokenplineno=" << *tokenplineno);
//-           return
//-             rps_repl_get_next_line(tokencallframe,
//-                                    tokeninp,
//-                                    tokeninputname,
//-                                    tokenplinebuf,
//-                                    tokenplineno,
//-                                    prompt);
//-         }
//-                                       );
//-         RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer lextokenv=" << _f.lextokenv
//-                       << " lookahead=" << lookahead
//-                       << " token_deq.size:" << token_deq.size());
//-         if (_f.lextokenv)
//-           {
//-             token_deq.push_back(_f.lextokenv);
//-             RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer new token_deq=" << token_deq);
//-             continue; /// the while  (lookahead > token_deq.size()) loop
//-           }
//-         else
//-           return Rps_Value(nullptr);
//-       };
//-       RPS_ASSERT(lookahead >= token_deq.size());
//-       RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer lookahead=" << lookahead
//-                     << " cmdreplob=" << _f.cmdreplob
//-                     << " :->◑ " << token_deq[lookahead]
//-                     << " #" << ++tokencount);
//-       return token_deq[lookahead];
//-     };                         // end while (lookahead > token_deq.size())
//-   RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer before applying " << _f.cmdparserv
//-                 << " to cmdreplob=" << _f.cmdreplob
//-                 << std::endl
//-                 <<  " @"
//-                 << input_name << "L" << startline << "C" << startcol
//-                 << std::endl
//-                 << " curframe:" << Rps_ShowCallFrame(&_));
//-   Rps_TwoValues parspair = Rps_ClosureValue(_f.cmdparserv.to_closure()).apply1 (&_, _f.cmdreplob);
//-   rps_repl_cmd_lexer_fun = nullptr;
//-   rps_repl_consume_cmd_token_fun = nullptr;
//-   _f.parsmainv = parspair.main();
//-   _f.parsxtrav = parspair.xtra();
//-   RPS_DEBUG_LOG(REPL, "rps_repl_cmd_tokenizer for command " << _f.cmdreplob << " after applying " << _f.cmdparserv
//-                 << " -> parsmainv=" << _f.parsmainv
//-                 << ", parsxtrav=" << _f.parsxtrav
//-                 << " :->◑ " << _f.lextokenv
//-                 << " #" << ++tokencount
//-                 << std::endl
//-                 <<  " @"
//-                 << input_name << "L" << startline << "C" << startcol);
//-   return _f.lextokenv;
//- } // end rps_repl_cmd_tokenizer
//-


///// end of useless file RefPerSys/attic/oldrepl_rps.cc

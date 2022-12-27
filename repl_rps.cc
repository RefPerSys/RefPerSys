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
 *      © Copyright 2019 - 2022 The Reflective Persistent System Team
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


/// Interpret from either a given input stream,
/// or using readline if inp is null.
extern "C" void rps_repl_interpret(Rps_CallFrame*callframe, std::istream*inp, const char*input_name, int& lineno);

std::vector<std::string> rps_completion_vect;

/// a C++ closure for getting the REPL lexical token.... with
/// lookahead=0, next token, with lookahead=1 the second-next token
std::function<Rps_LexTokenValue(Rps_CallFrame*,unsigned)> rps_repl_cmd_lexer_fun;

/// these REPL lexical tokens are looked ahead, so we need a function
/// to consume them... Returning true when the leftmost token is
/// forgotten
std::function<bool(Rps_CallFrame*)> rps_repl_consume_cmd_token_fun;

bool rps_repl_stopped;

std::string
rps_repl_version(void)
{

  std::string res = "REPL";
  {
    char gitstart[48];
    memset (gitstart, 0, sizeof(gitstart));
    strncpy(gitstart, rps_repl_gitid, sizeof(gitstart)-2);
    res += " git ";
    res += gitstart;
  }
  return res;
} // end rps_repl_version



extern "C" rps_applyingfun_t rpsapply_repl_not_implemented;

Rps_TwoValues
rpsapply_repl_not_implemented(Rps_CallFrame*callerframe,
                              const Rps_Value arg0,
                              const Rps_Value arg1,
                              const Rps_Value arg2,
                              const Rps_Value arg3,
                              const std::vector<Rps_Value>* restargs)
{
  if (!restargs)
    RPS_WARNOUT("rpsapply_repl_not_implemented arg0:" << arg0
                << " arg1:" << arg1
                << " arg2:" << arg2
                << " arg3:" << arg3
                << std::endl
                << " from caller frame:"
                << Rps_ShowCallFrame(callerframe)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_repl_not_implemented"));
  else
    {
      auto nbargs = restargs->size();
      std::ostringstream outs;
      for (int aix = 0; aix < (int)nbargs; aix++)
        {
          outs << " rest#" << aix << ":" << restargs->at(aix);
        }
      outs.flush();
      RPS_WARNOUT("rpsapply_repl_not_implemented arg0:" << arg0
                  << " arg1:" << arg1
                  << " arg2:" << arg2
                  << " arg3:" << arg3
                  << std::endl
                  << outs.str()
                  << std::endl
                  << " from caller frame:"
                  << Rps_ShowCallFrame(callerframe)
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_repl_not_implemented/many"));
    }
  return {nullptr,nullptr};
} // end rpsapply_repl_not_implemented




/// Create a new REPL command, and output to stdout some draft C++
/// code to parse it.... To be called from the main thread.
void
rps_repl_create_command(Rps_CallFrame*callframe, const char*commandname)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_4CZZ2JlnkQT02YJ6sM), //repl_command∈symbol
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obsymb;
                           Rps_ObjectRef obcommand;
                           Rps_ObjectRef obfun;
                           Rps_ObjectRef obreplcmdclass;
                           Rps_Value closv;
                           Rps_Value strnamev;
                );
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame(callframe));
  RPS_ASSERT(commandname != nullptr);
  _f.obreplcmdclass = RPS_ROOT_OB(_8CncrUdoSL303T5lOK); //repl_command∈class
  bool goodname = isalpha(commandname[0]);
  for (const char*pc = commandname; *pc && goodname; pc++)
    goodname = isalnum(*pc)
               || (pc>commandname && *pc == '_' && pc[-1] != '_');
  if (!goodname)
    {
      RPS_WARNOUT("rps_repl_create_command invalid command name " << commandname << std::endl
                  << ".. called from " <<  Rps_ShowCallFrame(&_));
      std::string msg = "invalid REPL command name ";
      msg += commandname;
      throw std::runtime_error(msg);
    };
  _f.strnamev = Rps_StringValue(commandname);
  _f.obsymb = Rps_ObjectRef::make_new_strong_symbol(&_, std::string(commandname));
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> obsymb=" << _f.obsymb);
  RPS_ASSERT(_f.obsymb);
  Rps_PayloadSymbol* paylsymb =
    _f.obsymb->get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT(paylsymb);
  /// the command name should be a fresh symbol...
  if (paylsymb->symbol_value())
    {
      RPS_WARNOUT("rps_repl_create_command command name "
                  << commandname << " already known as symbol " << _f.obsymb
                  << std::endl
                  << ".. called from " <<  Rps_ShowCallFrame(&_));
      return;
    }
  _f.obcommand
    = Rps_ObjectRef::make_object(&_,
                                 RPS_ROOT_OB(_8CncrUdoSL303T5lOK), //repl_command∈class
                                 Rps_ObjectRef::root_space());
  paylsymb->symbol_put_value(_f.obcommand);
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> obcommand=" << _f.obcommand);
  /* We need to create some object ObFun, of class 9Gz1oNPCnkB00I6VRS
     == core_function∈class and make a closure from it; that closure
     would be the repl_command_parser == _4I8GwXXfO3P01cdzyd of
     ObFun. We also need to output on stdout some C++ skeleton code
     for it. */
  _f.obfun
    = Rps_ObjectRef::make_object(&_,
                                 RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS), //core_function∈class
                                 Rps_ObjectRef::root_space());
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> obfun=" << _f.obfun);
  _f.strnamev = Rps_StringValue(std::string{commandname} + "°replcfun");
  _f.closv = Rps_ClosureValue(_f.obfun, {_f.obcommand,_f.obsymb});
  _f.obfun->put_attr(RPS_ROOT_OB(_8CncrUdoSL303T5lOK), //repl_command∈class
                     _f.obcommand);
  _f.obfun->put_attr(Rps_ObjectRef::the_name_object(),
                     _f.strnamev);
  _f.obfun->put_applying_function(rpsapply_repl_not_implemented);
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " -> closv=" << _f.closv);
  _f.obcommand->put_attr(RPS_ROOT_OB(_4I8GwXXfO3P01cdzyd), ///  repl_command_parser∈symbol
                         _f.closv);
  _f.strnamev = Rps_StringValue(commandname);
  _f.obcommand->put_attr(Rps_ObjectRef::the_name_object(),
                         _f.strnamev);
  // the symbol should be reachable at dump time and known to the command
  _f.obcommand->put_attr(Rps_ObjectRef::the_symbol_class(),
                         _f.obsymb);
  std::cout << std::endl << std::endl << std::endl
            << "/*# C++ function " << _f.obfun << " for REPL command " << commandname << "*/" << std::endl;
  std::cout << "extern \"C\" rps_applyingfun_t rpsapply" << _f.obfun->oid() << ";" << std::endl;
  std::cout << "Rps_TwoValues" << std::endl << "rpsapply"
            << _f.obfun->oid() << "(Rps_CallFrame*callerframe," << std::endl
            << "                           const Rps_Value arg0," << std::endl
            << "                           const Rps_Value arg1," << std::endl
            << "                           [[maybe_unused]] const Rps_Value arg2," << std::endl
            << "                           [[maybe_unused]] const Rps_Value arg3," << std::endl
            << "                           [[maybe_unused]] const std::vector<Rps_Value>* restargs)" << std::endl
            << "{" << std::endl
            << "   static Rps_Id descoid;\n"
            << "   if (!descoid)\n" "     descoid=Rps_Id(\"" <<  _f.obfun->oid() << "\");" << std::endl
            << "   RPS_" "LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid)," << std::endl
            << "                   callerframe," << std::endl
            << "   );" << std::endl
            << "   RPS_" "DEBUG_LOG(CMD, \"REPL command " << commandname << " start arg0=\" << arg0 << \"∈\" << arg0.compute_class(&_)"
            << std::endl
            << "                << \" arg1=\" << arg1 <<  << \"∈\" << arg1.compute_class(&_) << std::endl" << std::endl
            << "                << \" from \" << std::endl" << std::endl
            << "                << Rps_ShowCallFrame(&_));" << std::endl
            << "#warning incomplete rpsapply" << _f.obfun->oid() << " for REPL command " << commandname << std::endl
            << "  RPS_" "WARNOUT(\"incomplete rpsapply" << _f.obfun->oid() << " for REPL command " << commandname << " from \" << std::endl" << std::endl
            << "                << RPS_FULL_BACKTRACE_HERE(1, \"rpsapply" << _f.obfun->oid() << " for REPL command " << commandname << "\"));" << std::endl
            << "  return {nullptr,nullptr};" << std::endl
            << "} //end of rpsapply" << _f.obfun->oid() << " for REPL command " << commandname
            << std::endl << std::endl;
  _f.obreplcmdclass->append_comp1(Rps_ObjectValue(_f.obcommand));
  RPS_DEBUG_LOG(CMD, "rps_repl_create_command commandname " << commandname
                << " added " << _f.obcommand << " to repl_command class " << _f.obreplcmdclass);
  /* see also rps_repl_interpret which would apply that closure */
#warning rps_repl_create_command incomplete
  RPS_WARNOUT("rps_repl_create_command incomplete for command "
              << commandname << " obfun=" << _f.obfun
              << " obcommand=" << _f.obcommand);
} // end rps_repl_create_command





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
  RPS_LOCALFRAME(/*descr:*/nullptr,
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


Rps_Value
rps_lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg,  Rps_ChunkData_st*chkdata);

Rps_Value
rps_lex_code_chunk(Rps_CallFrame *callframe, [[maybe_unused]] std::istream *inp,
                   [[maybe_unused]] const char *input_name,
                   [[maybe_unused]] const char **plinebuf, [[maybe_unused]] int &lineno,
                   [[maybe_unused]] int& colno)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obchk;
                           Rps_Value inputnamestrv;
                           Rps_Value chunkelemv;
                );
#warning rps_lex_code_chunk is obsolete -see Rps_TokenSource::lex_code_chunk
#if 0 && oldcode
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
  struct Rps_ChunkData_st chkdata = {};
  //memset (&chkdata, 0, sizeof(chkdata));
  chkdata.chunkdata_magic = rps_chunkdata_magicnum;
  chkdata.chunkdata_lineno = lineno;
  chkdata.chunkdata_colno = colno;
  strcpy(chkdata.chunkdata_endstr, endstr);
  //chkdata.chunkdata_inp = inp;
  chkdata.chunkdata_name.assign(input_name);
  //chkdata.chunkdata_plinebuf = plinebuf;
  // TODO: we should add vector components to _f.obchk, reading several lines...
  do
    {
      RPS_DEBUG_LOG(REPL, "in rps_lex_chunk chunking "
                    << chkdata.chunkdata_name
                    << " L"<< chkdata.chunkdata_lineno
                    << ",C" << chkdata.chunkdata_colno
                    << " endstr='" << chkdata.chunkdata_endstr << "'"
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
                << " obchk=" << _f.obchk
                << std::endl
                << Rps_Do_Output([=](std::ostream&outs)
  {
    unsigned nbchk = paylvec->size();
    for (unsigned ix=0; ix<nbchk; ix++)
      {
        outs << " [" << ix << "]="
             << paylvec->at(ix) << std::endl;
      }
  }));
  lineno = chkdata.chunkdata_lineno;
  colno = chkdata.chunkdata_colno;
  *plinebuf = *chkdata.chunkdata_plinebuf;
  chkdata.chunkdata_magic =0;
#endif /*0 && oldcode*/
  return _f.obchk;
} // end rps_lex_code_chunk



/// Inside a code chunk represented by object obchkarg, parse some
/// chunk element...
Rps_Value
rps_lex_chunk_element(Rps_CallFrame *callframe,
                      [[maybe_unused]] Rps_ObjectRef obchkarg, Rps_ChunkData_st *chkdata)
{
  RPS_ASSERT(chkdata != nullptr && chkdata->chunkdata_magic ==  rps_chunkdata_magicnum);
  RPS_ASSERT(callframe != nullptr && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obchunk;
                           Rps_Value chkelemv;
                           Rps_ObjectRef namedobv;
                );
#warning rps_lex_chunk_element is obsolete code
#if 0 && oldcode
  RPS_ASSERT(chkdata->chunkdata_plinebuf != nullptr);
  _f.obchunk = obchkarg;
  RPS_ASSERT(_f.obchunk);
  auto paylvect = _f.obchunk->get_dynamic_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvect != nullptr);
  const char*linestart = nullptr; //*chkdata->chunkdata_plinebuf;
#warning obsolete code in rps_lex_chunk_element which has no more sense
  int linelen = strlen(linestart);
  RPS_ASSERT(chkdata->chunkdata_colno >= 0 && chkdata->chunkdata_colno<linelen);
  RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start obchunk=" << _f.obchunk
                << ", linestart='" << linestart << "', linelen=" << linelen
                << " @L" << chkdata->chunkdata_lineno << ",C"
                <<  chkdata->chunkdata_colno
                << " endstr='" << chkdata->chunkdata_endstr
                <<  "' current:"
                << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                << linestart+chkdata->chunkdata_colno
                << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                << std::endl
                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-start"));
  /// For C name-like things, we return the object naming them or else a string
  const char*curstr = linestart+chkdata->chunkdata_colno;
  if (isalpha(*curstr))
    {
      int startnamcol = chkdata->chunkdata_colno;
      int endnamcol = startnamcol;
      while (endnamcol<linelen && (isalnum(linestart[endnamcol]) || linestart[endnamcol]=='_'))
        endnamcol++;
      int namlen = endnamcol-startnamcol;
      std::string namstr = std::string(linestart+startnamcol, namlen);
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element obchunk=" << _f.obchunk
                    << " namstr='" << namstr << "' starting L"
                    << chkdata->chunkdata_lineno << ",C" << startnamcol
                    << " endnamcol=" << endnamcol);
      _f.namedobv = Rps_ObjectRef::find_object_by_string(&_, namstr,
                    Rps_ObjectRef::Null_When_Missing);
      chkdata->chunkdata_colno = endnamcol;
      curstr = nullptr;
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
  else if (isspace(*curstr))
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
      curstr = nullptr;
      return _f.chkelemv;
    }
  /// code chunk meta-variable or meta-notation....
  else if (*curstr == '$'
           && chkdata->chunkdata_colno < linelen)
    {
      // a dollar followed by a name is a meta-variable; that name should be known
      const char*metastr = curstr;
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element start meta obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " endstr='" << chkdata->chunkdata_endstr
                    << "' metastr:" << metastr);
      if (isalpha(metastr[1]))
        {
          int startnameix=1, endnameix=1;
          while (isalnum(metastr[endnameix])||metastr[endnameix]=='_')
            endnameix++;
          std::string metaname(metastr+1, endnameix-startnameix);
          _f.namedobv = Rps_ObjectRef::find_object_by_string(&_, metaname,
                        Rps_ObjectRef::Null_When_Missing);
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
  else if (*curstr=='}' && !strncmp(curstr, chkdata->chunkdata_endstr, strlen(chkdata->chunkdata_endstr)))
    {
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element ending  callframe=" << Rps_ShowCallFrame(callframe)
                    << std::endl << "... obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " curstr:"
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                    << curstr
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                    << " endstr='" << chkdata->chunkdata_endstr << "'"
                    << std::endl
                    <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-ending")
                   );
      chkdata->chunkdata_colno += strlen(chkdata->chunkdata_endstr);
      return nullptr;
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "rps_lex_chunk_element INCOMPLETE  callframe=" << Rps_ShowCallFrame(callframe)
                    << std::endl << "... obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno
                    << " curstr:"
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_UNDERLINE_ESCAPE:"`")
                    << curstr
                    << ((rps_stdout_istty && !rps_batch)?RPS_TERMINAL_NORMAL_ESCAPE:"'")
                    << " endstr='" << chkdata->chunkdata_endstr << "'"
                    << std::endl
                    <<  RPS_FULL_BACKTRACE_HERE(1, "rps_lex_chunk_element-incomplete")
                   );
    }
  RPS_FATALOUT("unimplemented rps_lex_chunk_element callframe=" << Rps_ShowCallFrame(callframe)
               << " obchkarg=" << obchkarg
               << " chkdata=" << chkdata
               << " @L" << chkdata->chunkdata_lineno << ",C"
               <<  chkdata->chunkdata_colno
               << " linestart='" << linestart << "'");
#endif /*0 && obsolete*/
#warning we need to document and implement other chunk element conventions in rps_lex_chunk_element, in particular delimiters...
#warning unimplemented rps_lex_chunk_element
  return nullptr;
} // end rps_lex_chunk_element



////////////////////////////////////////////////////////////////
//// the Rps_LexTokenZone values are transient, but do need some GC support

Rps_LexTokenZone::Rps_LexTokenZone(Rps_TokenSource* tsrc, Rps_ObjectRef kindob, Rps_Value val, const Rps_String*filestringp, int line, int col)
  : Rps_LazyHashedZoneValue(Rps_Type::LexToken),
    lex_kind(kindob),
    lex_val(val),
    lex_file(filestringp),
    lex_src(tsrc),
    lex_lineno(line),
    lex_colno(col),
    lex_serial(0)
{
  RPS_ASSERT (!kindob || kindob->stored_type() == Rps_Type::Object);
  RPS_ASSERT (!filestringp || filestringp->stored_type() == Rps_Type::String);
} // end Rps_LexTokenZone::Rps_LexTokenZone

void
Rps_LexTokenZone::set_serial(unsigned serial)
{
  if (serial==0) return;
  RPS_ASSERT (lex_serial==0);
  lex_serial = serial;
} // end Rps_LexTokenZone::set_serial

Rps_LexTokenZone::~Rps_LexTokenZone()
{
  lex_kind = nullptr;
  lex_val = nullptr;
  lex_src = nullptr;
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
  bool showchunk = false;
  if (lex_serial>0)
    out << "LexToken#" << lex_serial << "{";
  else
    out << "LexToken{";
  if (lex_kind == RPS_ROOT_OB(_36I1BY2NetN03WjrOv)) // symbol∈class
    out<<"°symbol";
  else if (lex_kind == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b)) // int∈class
    out<<"°int";
  else if (lex_kind == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE)) //string∈class
    out<<"°string";
  else if (lex_kind == RPS_ROOT_OB(_98sc8kSOXV003i86w5)) // double∈class
    out<<"°double";
  else if (lex_kind == RPS_ROOT_OB(_3rXxMck40kz03RxRLM))   // code_chunk∈class
    {
      out<<"°code_chunk";
      if (depth==0)
        showchunk = true;
    }
  else if (lex_kind == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)) // object∈class
    out<<"°object";
  else if (lex_kind == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK)) //repl_delimiter∈class
    out<<"°delim";
  else if (!lex_kind)
    out<<"°°nul°°";
  else
    out<<"kind:" << lex_kind;
  if (depth > Rps_Value::max_output_depth)
    {
      out << "...}";
      return;
    };
  out << ", val=";
  lex_val.output(out, depth+1);
  if (lex_val.is_object() && depth<=1)
    {
      Rps_ObjectRef obr = lex_val.as_object();
      if (obr)
        {
          std::unique_lock<std::recursive_mutex> guobr (*obr->objmtxptr());
          Rps_Value vname = obr->get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name∈named_attribute
          if (auto paylvect = obr->get_dynamic_payload<Rps_PayloadVectVal>())
            {
              unsigned vsiz = paylvect->size();
              out << "⟪";
              for (unsigned ix=0; ix<vsiz; ix++)
                {
                  if (ix>0 && ix % 4==0)
                    {
                      out << ",";
                      out << std::endl;
                      for (unsigned k=depth; k>0; k--) out << " ";
                    }
                  else if (ix>0) out << ", ";
                  out << "["<< ix << "]:" << paylvect->at(ix);
                }
              out << "⟫";
            }
          else if (auto paylsymb = obr->get_dynamic_payload<Rps_PayloadSymbol>())
            {
              out << "symb:" << paylsymb->symbol_name();
            }
          else if (auto paylclass = obr->get_dynamic_payload<Rps_PayloadClassInfo>())
            {
              out << "class:" << paylclass->class_name_str();
            }
          else if (vname.is_string())
            {
              out << "°named" << Rps_QuotedC_String(vname.to_string()->cppstring());
            }
        }
    }
  if (showchunk)
    {
#warning missing code in Rps_LexTokenZone::val_output for code_chunk
      RPS_WARNOUT("Rps_LexTokenZone::val_output missing code to show a code chunk "
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_LexTokenZone::val_output/codechunk?"));
    }
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

#if 0 && oldcode
// Tokenize a lexical token; an optional double-ended queue of
// already lexed token enable limited backtracking when needed....
const Rps_LexTokenZone*
Rps_LexTokenZone::tokenize(Rps_CallFrame*callframe, std::istream*inp,
                           const char*input_name,
                           const char**plinebuf, int&lineno, int& colno,
                           [[maybe_unused]] lexical_line_getter_fun linegetter,
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
      pque->gc_mark(gc);
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
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize dequeued " << _f.lextokv
                    << " from " << (*pque)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_LexTokenZone::tokenize dequeued"));
      return _f.lextokv.to_lextoken();
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize inputstr="
                    << inputstr << ", lineno=" << lineno
                    << ", colno=" << colno << ", "
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
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize just  before rps_repl_lexer inputstr='"
                    << Rps_Cjson_String(inputstr) << "', lineno=" << lineno
                    << ", colno=" << colno
                    << (curinp?", curinp='":", curinp ")
                    << (curinp?curinp:" *NUL*")
                    << (curinp?"' ":""));
      Rps_TwoValues twolex =
        rps_repl_lexer(&_, inp, input_name, *plinebuf, lineno, colno);
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize "
                    << " lineno=" << lineno
                    << ", colno=" << colno
                    << " twolex! main:" << twolex.main_val
                    << ", xtra:" << twolex.xtra_val);
      _f.lexkindob = twolex.main_val.to_object();
      _f.lextokv = twolex.xtra_val;
      _f.kindnamv = nullptr;
      if (_f.lexkindob)
        _f.kindnamv = _f.lexkindob
                      ->get_attr1(&_,RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); // /name∈named_attribute
      RPS_DEBUG_LOG(REPL, "Rps_LexTokenZone::tokenize from rps_repl_lexer got lexkindob=" << _f.lexkindob
                    << "/" << _f.kindnamv
                    << ", lextok=" << _f.lextokv
                    << ", lineno=" << lineno
                    << ", colno=" << colno);
      {
        Rps_LexTokenZone* lextok =
          Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
          (this,
           _f.lexkindob,
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
#endif /*0 && oldcode*/
////////////////////////////////////////////////////////////////



//§ char **
//§ rpsrepl_name_or_oid_completion(const char *text, int start, int end)
//§ {
//§   /* Notice that the start and end are byte indexes, and that matters
//§    *  with UTF-8.  But they are indexes in the current line, while text
//§    *  is what should be completed... */
//§   RPS_DEBUG_LOG(COMPL_REPL, "text='" << text << "' start=" << start
//§                 << ", end=" << end
//§                 << std::endl
//§                 << RPS_FULL_BACKTRACE_HERE(1, "rpsrepl_name_or_oid_completion"));
//§   rps_completion_vect.clear();
//§   std::string prefix;
//§   int nbmatch = 0;
//§   // for objid, we require four characters including the leading
//§   // underscore to autocomplete...
//§   if (end>start+4 && text[0] == '_' && isdigit(text[1])
//§       && isalnum(text[2]) && isalnum(text[3]))
//§     {
//§       prefix.assign(text, end-start);
//§       RPS_DEBUG_LOG(COMPL_REPL, "oid autocomplete prefix='" << prefix << "'");
//§       // use oid autocompletion, with
//§       // Rps_ObjectZone::autocomplete_oid...
//§       nbmatch = Rps_ObjectZone::autocomplete_oid
//§                 (prefix.c_str(),
//§                  [&] (const Rps_ObjectZone* obz)
//§       {
//§         RPS_ASSERT(obz != nullptr);
//§         Rps_Id oid = obz->oid();
//§         RPS_DEBUG_LOG(COMPL_REPL, "oid autocomplete oid=" << oid
//§                       << " for prefix='" << prefix << "'"
//§                       << std::endl
//§                       << RPS_FULL_BACKTRACE_HERE(1, "autocomploid/rpsrepl_name_or_oid_completion"));
//§         rps_completion_vect.push_back(oid.to_string());
//§         return false;
//§       });
//§       RPS_DEBUG_LOG(COMPL_REPL, "oid autocomplete prefix='" << prefix << "' -> nbmatch=" << nbmatch);
//§     }
//§   // for names, we require two characters to autocomplete
//§   else if (end>start+2 && isalpha(text[0]) && (isalnum(text[1]) || text[1]=='_'))
//§     {
//§       // use symbol name autocompletion, with
//§       // Rps_PayloadSymbol::autocomplete_name...
//§       prefix.assign(text, end-start);
//§       RPS_DEBUG_LOG(COMPL_REPL, "name autocomplete prefix='" << prefix << "'");
//§       nbmatch =  Rps_PayloadSymbol::autocomplete_name
//§                  (prefix.c_str(),
//§                   [&] (const Rps_ObjectZone*obz, const std::string&name)
//§       {
//§         RPS_ASSERT(obz != nullptr);
//§         RPS_DEBUG_LOG(COMPL_REPL, "symbol autocomplete name='" << name
//§                       << "', obz:" << Rps_ObjectRef(obz)
//§                       << " for prefix='" << prefix << "'"
//§                       << std::endl
//§                       << RPS_FULL_BACKTRACE_HERE(1, "autocomplname/rpsrepl_name_or_oid_completion"));
//§         rps_completion_vect.push_back(name);
//§         return false;
//§       });
//§       RPS_DEBUG_LOG(COMPL_REPL, "name autocomplete prefix='" << prefix << "' -> nbmatch=" << nbmatch);
//§     }
//§   else
//§     RPS_DEBUG_LOG(COMPL_REPL, "no autocomplete prefix='" << prefix << "'"
//§                   << " text='" << text << "' start=" << start << " end=" << end);
//§   //
//§   RPS_DEBUG_LOG(COMPL_REPL, "autocomplete rps_completion_vect nbmatch=" << nbmatch << "rps_completion_vect: siz#" << rps_completion_vect.size()
//§                 << Rps_Do_Output([&](std::ostream& out)
//§   {
//§     int ix=0;
//§     for (auto str: rps_completion_vect)
//§       {
//§         if (ix % 4 == 0)
//§           out << std::endl << "...";
//§         out << " [" << ix << "]::'" << str<< "'";
//§         ix++;
//§       }
//§   })
//§       << std::endl
//§       << RPS_FULL_BACKTRACE_HERE(1, "rpsrepl_name_or_oid_completion-autocomplete"));
//§   if (nbmatch==0)
//§     return nullptr;
//§   else
//§     {
//§       rl_attempted_completion_over = 1;
//§       RPS_DEBUG_LOG(COMPL_REPL, "rpsrepl_name_or_oid_completion nbmatch=" << nbmatch
//§                     << " text='" << text << "' start=" << start
//§                     << " end='" << end);
//§       return rl_completion_matches(text, rpsrepl_name_or_oid_generator);
//§     }
//§ } // end rpsrepl_name_or_oid_completion
//§
//§
//§
//§ char *
//§ rpsrepl_name_or_oid_generator(const char *text, int state)
//§ {
//§   /// the initial state is 0....
//§   RPS_DEBUG_LOG(COMPL_REPL, "text='" << text << "' state#" << state << " rps_completion_vect.size="
//§                 << rps_completion_vect.size()
//§                 << Rps_Do_Output([&](std::ostream& outs)
//§   {
//§     int cix=0;
//§     for (auto curcomp: rps_completion_vect)
//§       {
//§         if (cix % 4 == 0) outs << std::endl;
//§         outs << "completion[" << cix << "]==" << curcomp;
//§         cix++;
//§       }
//§   })
//§       << std::endl
//§       << RPS_FULL_BACKTRACE_HERE(1, "rpsrepl_name_or_oid_generator"));
//§   if (rps_completion_vect.size() == 1)
//§     {
//§       if (state==0)
//§         return strdup(rps_completion_vect[0].c_str());
//§       else
//§         return nullptr;
//§     }
//§   RPS_WARNOUT("rpsrepl_name_or_oid_generator incomplete text=='" << text << "' state#" << state
//§               << " with " <<rps_completion_vect.size() << " completions");
//§ #warning rpsrepl_name_or_oid_generator incomplete...
//§   return nullptr;
//§ } // end rpsrepl_name_or_oid_generator




//§ void
//§ rps_read_eval_print_loop(int &argc, char **argv)
//§ {
//§   RPS_LOCALFRAME(/*descr:*/nullptr,
//§                            /*callerframe:*/nullptr,
//§                            Rps_Value lextokv;
//§                            Rps_Value lexval;
//§                            Rps_ObjectRef cmdob;
//§                            Rps_Value cmdparserv;
//§                            Rps_Value parsmainv;
//§                            Rps_Value parsextrav;
//§                 );
//§   for (int ix=0; ix<argc; ix++)
//§     RPS_DEBUG_LOG(REPL, "REPL arg [" << ix << "]: " << argv[ix]);
//§   RPS_ASSERT(rps_is_main_thread());
//§   RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop start frame=" << Rps_ShowCallFrame(&_));
//§   int count=0;
//§   rl_attempted_completion_function = rpsrepl_name_or_oid_completion;
//§   Rps_ReadlineTokenSource rltoksrc("_-_");
//§   while (!rps_repl_stopped)
//§     {
//§       char prompt[32];
//§       memset(prompt, 0, sizeof(prompt));
//§       count++;
//§       snprintf(prompt, sizeof(prompt), "Rps_REPL#%d: ", count);
//§       rltoksrc.set_prompt(prompt);
//§       RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop command count#" << count);
//§       if (count % 4 == 0)
//§         usleep(128*1024);
//§       if (!rltoksrc.get_line())
//§         break;
//§       std::string commandpos = rltoksrc.position_str();
//§       RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop count#" << count
//§                     << " commandpos=" << commandpos
//§                     << " current_line='"
//§                     << Rps_Cjson_String(rltoksrc.current_line()) << "'");
//§       _f.lextokv = rltoksrc.get_token(&_);
//§       RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop got lextokv=" << _f.lextokv << " pos=" << rltoksrc.position_str());
//§       if (!_f.lextokv)
//§         break;
//§       const Rps_LexTokenZone* lextokz = _f.lextokv.as_lextoken();
//§       RPS_ASSERT(lextokz);
//§       if (lextokz->lxkind()
//§           != RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ))  //object∈class
//§         {
//§           RPS_WARNOUT("rps_read_eval_print_loop command at "
//§                       << commandpos << std::endl
//§                       << "Should start with an object but got "
//§                       << _f.lextokv);
//§           continue;
//§         }
//§       _f.lexval = lextokz->lxval();
//§       RPS_ASSERT(_f.lexval.is_object());
//§       _f.cmdob = _f.lexval.as_object();
//§       RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop cmdob=" << _f.cmdob
//§                     << " at " << commandpos);
//§       if (_f.lexval.is_instance_of(&_,RPS_ROOT_OB(_8CncrUdoSL303T5lOK)))   //repl_command∈class
//§         {
//§           _f.cmdparserv = _f.cmdob
//§                           ->get_attr1(&_,RPS_ROOT_OB(_4I8GwXXfO3P01cdzyd)); //repl_command_parser∈symbol
//§           RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop cmdob=" << _f.cmdob
//§                         << " is repl_command of repl_command_parser: " << _f.cmdparserv
//§                         << " lextokv=" << _f.lextokv);
//§           if (_f.cmdparserv.is_closure())
//§             {
//§               Rps_TwoValues parspair = Rps_ClosureValue(_f.cmdparserv.to_closure()).apply2 (&_, _f.cmdob, _f.lextokv);
//§               _f.parsmainv = parspair.main();
//§               _f.parsextrav = parspair.xtra();
//§               RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop applied " << _f.cmdparserv << " to "
//§                             << _f.cmdob
//§                             << " and got parsmainv:" << _f.parsmainv << ", parsextrav=" << _f.parsextrav
//§                             << " now position is " <<  rltoksrc.position_str());
//§               if (!_f.parsmainv && !_f.parsextrav)
//§                 RPS_WARNOUT("rps_read_eval_print_loop: REPL command " << _f.cmdob << " at " << commandpos << " failed using "
//§                             << _f.cmdparserv << std::endl);
//§               continue;
//§             }
//§           else
//§             {
//§               RPS_WARNOUT("rps_read_eval_print_loop: REPL command " << _f.cmdob << " has a bad command parser " << _f.cmdparserv
//§                           << " after " << _f.lexval);
//§               continue;
//§             }
//§           RPS_DEBUG_LOG(REPL, "rps_read_eval_print_loop at  " << commandpos << " count#" << count <<  " pos=" << rltoksrc.position_str());
//§         }
//§       else
//§         {
//§           RPS_WARNOUT("rps_read_eval_print_loop: REPL command unexpected token " <<  _f.lextokv << " at " << commandpos << " now at " << rltoksrc.position_str());
//§           continue;
//§         }
//§     };
//§   RPS_INFORMOUT("rps_read_eval_print_loop ending count#" << count << " at " << rltoksrc.position_str());
//§ } // end of rps_read_eval_print_loop


void
rps_do_repl_commands_vec(const std::vector<std::string>&cmdvec)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/nullptr,
                           Rps_Value lextokv;
                           Rps_Value lexval;
                           Rps_ObjectRef cmdob;
                           Rps_Value cmdparserv;
                           Rps_Value parsmainv;
                           Rps_Value parsextrav;
                );
  RPS_ASSERT(rps_is_main_thread());
  int nbcmd = (int) (cmdvec.size());
  RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec start nbcmd:" << nbcmd);
  std::string cmdstr;
  for (int cix=0; cix<nbcmd; cix++)
    {
      RPS_DEBUG_LOG(REPL, "REPL command [" << cix << "]: " << cmdvec[cix]);
      int count=0;
      {
        char bufpath[64];
        memset (bufpath, 0, sizeof(bufpath));
        snprintf(bufpath, sizeof(bufpath), "ReplCmd[%d]", cix);
        Rps_StringTokenSource intoksrc(cmdvec[cix], std::string(bufpath));
        if (cix % 4 == 0)
          usleep(128*1024);
        if (!intoksrc.get_line())
          break;
        std::string commandpos = intoksrc.position_str();
        RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec command#" << cix
                      << Rps_Cjson_String(cmdvec[cix])
                      << " @" << bufpath << std::endl
                      << "... intoksrc:" << intoksrc
                      << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
        _f.lextokv = intoksrc.get_token(&_);
        _f.lexval = nullptr;
        _f.cmdob = nullptr;
        RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec got lextokv=" << _f.lextokv
                      << " pos=" << intoksrc.position_str() << std::endl
                      << "... intoksrc:" << intoksrc
                      << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
        if (!_f.lextokv)
          break;
        const Rps_LexTokenZone* lextokz = _f.lextokv.as_lextoken();
        RPS_ASSERT(lextokz);
        if (lextokz->lxkind()
            == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ))  //object∈class
          {
            _f.lexval = lextokz->lxval();
            RPS_ASSERT(_f.lexval.is_object());
            _f.cmdob = _f.lexval.as_object();
            RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec cmdob=" << _f.cmdob
                          << " at " << commandpos<< std::endl
                          << "... intoksrc:" << intoksrc
                          << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
          }
        else if (lextokz->lxkind()
                 ==RPS_ROOT_OB(_36I1BY2NetN03WjrOv) //symbol∈class
                )
          {
            _f.lexval = lextokz->lxval();
            RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec symbol token " << _f.lextokv << " of value " << _f.lexval
                          << " at " << commandpos<< std::endl
                          << "... intoksrc:" << intoksrc
                          << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr()));
            if (_f.lexval.is_string())
              _f.cmdob = Rps_PayloadSymbol::find_named_object(_f.lexval.as_string()->cppstring());
#warning unimplemented symbol token rps_do_repl_commands_vec
            RPS_WARNOUT("unimplemented symbol token rps_do_repl_commands_vec lextok="
                        << _f.lextokv
                        << " cmdob=" << _f.cmdob<< std::endl
                        << "... intoksrc:" << intoksrc
                        << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_do_repl_commands_vec/symbol"));
          }
        else
          {
            RPS_WARNOUT("rps_do_repl_commands_vec command at "
                        << commandpos << std::endl
                        << "Should start with an object or symbol but got "
                        << _f.lextokv << " of kind " << lextokz->lxkind()
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_do_repl_commands_vec/non-obj-cmd"));
            continue;
          }
        RPS_ASSERT(_f.lexval.is_object());
        _f.cmdob = _f.lexval.as_object();
        RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec cmdob=" << _f.cmdob
                      << " at " << commandpos);
        if (_f.lexval.is_instance_of(&_,RPS_ROOT_OB(_8CncrUdoSL303T5lOK)))   //repl_command∈class
          {
            _f.cmdparserv = _f.cmdob
                            ->get_attr1(&_,RPS_ROOT_OB(_4I8GwXXfO3P01cdzyd)); //repl_command_parser∈symbol
            RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec cmdob=" << _f.cmdob
                          << " is repl_command of repl_command_parser: " << _f.cmdparserv
                          << " lextokv=" << _f.lextokv);
            if (_f.cmdparserv.is_closure())
              {
                RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec cmdob=" << _f.cmdob << " before applying cmdparserv=" << _f.cmdparserv << " with lextokv=" << _f.lextokv
                              << " command at " << commandpos
                              << " ... at position " <<  intoksrc.position_str()
                              << " curptr:" << Rps_QuotedC_String(intoksrc.curcptr()) << std::endl
                              <<  RPS_FULL_BACKTRACE_HERE(1, "rps_do_repl_commands_vec/before-apply"));
                Rps_TwoValues parspair = Rps_ClosureValue(_f.cmdparserv.to_closure()).apply2 (&_, _f.cmdob, _f.lextokv);
                _f.parsmainv = parspair.main();
                _f.parsextrav = parspair.xtra();
                RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec applied " << _f.cmdparserv << " to cmd#" << cix << " "<< std::endl
                              << "... intoksrc:" << intoksrc
                              << " curcptr:" << Rps_QuotedC_String(intoksrc.curcptr())
                              << _f.cmdob
                              << " and got parsmainv:" << _f.parsmainv << ", parsextrav=" << _f.parsextrav
                              << std::endl
                              << "... now position is " <<  intoksrc.position_str() << " curptr:" <<
                              Rps_QuotedC_String(intoksrc.curcptr())  << std::endl
                              <<  RPS_FULL_BACKTRACE_HERE(1, "rps_do_repl_commands_vec/applied"));
                if (!_f.parsmainv && !_f.parsextrav)
                  {
                    RPS_WARNOUT("rps_do_repl_commands_vec: REPL command#" << cix
                                << " " << _f.cmdob << " at " << commandpos << " failed using "
                                << _f.cmdparserv << std::endl
                                << " ... at position " <<  intoksrc.position_str() << " curptr:" << Rps_QuotedC_String(intoksrc.curcptr()) << std::endl
                                <<  RPS_FULL_BACKTRACE_HERE(1, "rps_do_repl_commands_vec/apply failed"));
                    continue;
                  }
              }
            else
              {
                RPS_WARNOUT("rps_do_repl_commands_vec: REPL command " << _f.cmdob << " has a bad command parser " << _f.cmdparserv
                            << " after " << _f.lexval);
                continue;
              }
            RPS_DEBUG_LOG(REPL, "rps_do_repl_commands_vec at  " << commandpos << " count#" << count <<  " pos=" << intoksrc.position_str());
          }
        else
          {
            RPS_WARNOUT("rps_do_repl_commands_vec: REPL command unexpected token " <<  _f.lextokv << " at " << commandpos << " now at " << intoksrc.position_str());
            continue;
          }
      }
    }
} // end of rps_do_repl_commands_vec





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
//-   RPS_LOCALFRAME(/*descr:*/nullptr,
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


// end of file repl_rps.cc

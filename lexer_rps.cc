/****************************************************************
 * file lexer_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the lexer support for the Read Eval Print Loop
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2021 The Reflective Persistent System Team
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

#include <wordexp.h>

extern "C" const char rps_lexer_gitid[];
const char rps_lexer_gitid[]= RPS_GITID;

extern "C" const char rps_lexer_date[];
const char rps_lexer_date[]= __DATE__;

extern "C" Rps_StringValue rps_lexer_token_name_str_val;
Rps_StringValue rps_lexer_token_name_str_val(nullptr);

Rps_TokenSource::Rps_TokenSource(std::string name)
  : toksrc_name(name), toksrc_line(0), toksrc_col(0), toksrc_linebuf{},
    toksrc_ptrnameval(nullptr)
{
} // end Rps_TokenSource::Rps_TokenSource

void
Rps_TokenSource::gc_mark(Rps_GarbageCollector&gc, unsigned depth)
{
  RPS_ASSERT(gc.is_valid_garbcoll());
  really_gc_mark(gc,depth);
} // end Rps_TokenSource::gc_mark

void
Rps_TokenSource::really_gc_mark(Rps_GarbageCollector&gc, unsigned depth)
{
  RPS_ASSERT(gc.is_valid_garbcoll());
  RPS_ASSERT(depth < max_gc_depth);
  if (rps_lexer_token_name_str_val)
    rps_lexer_token_name_str_val.gc_mark(gc,depth);
} // end Rps_TokenSource::really_gc_mark


/// the current token source name is needed as a string value for
/// constructing Rps_LexTokenValue-s. We want to avoid creating that
/// source name, as a RefPerSys string, at every lexical token. So we
/// try to memoize it in rps_lexer_token_name_str_val;
Rps_Value
Rps_TokenSource::name_val(Rps_CallFrame*callframe)
{
  RPS_ASSERT(callframe==nullptr || callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  if (rps_lexer_token_name_str_val && rps_lexer_token_name_str_val.is_string()
      && rps_lexer_token_name_str_val.to_cppstring() == toksrc_name)
    return rps_lexer_token_name_str_val;
  rps_lexer_token_name_str_val = Rps_String::make(toksrc_name);
  return rps_lexer_token_name_str_val;
} // end Rps_TokenSource::name_val


const Rps_LexTokenZone*
Rps_TokenSource::make_token(Rps_CallFrame*callframe,
                            Rps_ObjectRef lexkindarg, Rps_Value lexvalarg, Rps_String*sourcev)
{
  RPS_LOCALFRAME(RPS_ROOT_OB(_0S6DQvp3Gop015zXhL), //lexical_token∈class
                 /*callerframe:*/callframe,
                 Rps_ObjectRef lexkindob;
                 Rps_Value lexval;
                 Rps_LexTokenZone*tokenp;
                 Rps_Value namestrv;
                 const Rps_String* nstrv;
                );
  RPS_ASSERT(rps_is_main_thread());
  _f.lexkindob = lexkindarg;
  _f.lexval = lexvalarg;
  _f.namestrv = name_val(&_);
  _f.nstrv = _f.namestrv.as_string();
  _f.tokenp =
    Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
    ( _f.lexkindob, _f.lexval, _f.nstrv,
      toksrc_line, toksrc_col);
  return _f.tokenp;
} // end Rps_TokenSource::make_token

Rps_TokenSource::~Rps_TokenSource()
{
  toksrc_name.clear();
  toksrc_line= -1;
  toksrc_col= -1;
  toksrc_linebuf.clear();
} // end Rps_TokenSource::~Rps_TokenSource

Rps_StreamTokenSource::Rps_StreamTokenSource(std::string path)
  : Rps_TokenSource(""), toksrc_input_stream()
{
  wordexp_t wx= {};
  int err = wordexp(path.c_str(), &wx, WRDE_SHOWERR);
  if (err)
    {
      RPS_WARNOUT("stream token source for '" << Rps_Cjson_String(path) << "' failed: error#" << err);
      throw std::runtime_error(std::string{"bad stream token source:"} + path);
    }
  if (wx.we_wordc == 0)
    {
      RPS_WARNOUT("no stream token source for '" << (Rps_Cjson_String(path)) << "'");
      throw std::runtime_error(std::string{"no stream token source:"} + path);
    }
  else if (wx.we_wordc > 1)
    {
      RPS_WARNOUT("ambiguous stream token source for '" << path << "' expanded to "
                  << wx.we_wordv[0] << " and " << wx.we_wordv[1]
                  << ((wx.we_wordc>2)?" etc...":" files"));
      throw std::runtime_error(std::string{"ambiguous stream token source:"} + path);
    };
  char* curword = wx.we_wordv[0];
  toksrc_input_stream.open(curword);
  set_name(std::string(curword));
} // end Rps_StreamTokenSource::Rps_StreamTokenSource


Rps_StreamTokenSource::~Rps_StreamTokenSource()
{
  toksrc_input_stream.close();
} // end ps_StreamTokenSource::~Rps_StreamTokenSource

bool
Rps_StreamTokenSource::get_line(void)
{
  std::getline( toksrc_input_stream, toksrc_linebuf);
  if (!toksrc_input_stream && toksrc_linebuf.empty()) return false;
  return true;
} // end Rps_StreamTokenSource::get_line



Rps_CinTokenSource::Rps_CinTokenSource()
  : Rps_TokenSource("-")
{
};				// end Rps_CinTokenSource::Rps_CinTokenSource

Rps_CinTokenSource::~Rps_CinTokenSource()
{
};	// end Rps_CinTokenSource::~Rps_CinTokenSource

bool
Rps_CinTokenSource::get_line(void)
{
  std::getline(std::cin, toksrc_linebuf);
  if (!std::cin && toksrc_linebuf.empty()) return false;
  return true;
} // end Rps_CinTokenSource::get_line



Rps_LexTokenValue
Rps_TokenSource::get_token(Rps_CallFrame*callframe)
{
  RPS_ASSERT(callframe==nullptr || callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0S6DQvp3Gop015zXhL), //lexical_token∈class
                           /*callerframe:*/callframe,
                           Rps_Value res;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lextokv;
                           Rps_ObjectRef oblex;
                           Rps_Value namev;
                );
  const char* curp = curcptr();
  size_t linelen = toksrc_linebuf.size();
  while (curp && isspace(*curp) && toksrc_col<(int)linelen)
    curp++, toksrc_col++;
  if (toksrc_col>=(int)linelen)
    return nullptr;
  /// lex numbers?
  if (isdigit(*curp) ||
      ((curp[0] == '+' || curp[0] == '-') && isdigit(curp[1])))
    {
      int curlin = toksrc_line;
      int curcol = toksrc_col;
      char*endint=nullptr;
      char*endfloat=nullptr;
      const char*startnum = curp;
      long long l = strtoll(startnum, &endint, 0);
      double d = strtod(startnum, &endfloat);
      RPS_ASSERT(endint != nullptr && endfloat != nullptr);
      if (endfloat > endint)
        {
          toksrc_col += endfloat - startnum;
          _f.lextokv = Rps_DoubleValue(d);
          _f.lexkindob = RPS_ROOT_OB(_98sc8kSOXV003i86w5); //double∈class
        }
      else
        {
          toksrc_col += endint - startnum;
          _f.lextokv = Rps_Value::make_tagged_int(l);
          _f.lexkindob = RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b); //int∈class
        }
      _f.namev = name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (_f.lexkindob, _f.lextokv,
         str,
         curlin, curcol);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token number :-◑> " << _f.res);
      return _f.res;
    } //- end lexing numbers
  ///
  /// lex infinities (double) - but not NAN
  else if ((!strncmp(curp, "+INF", 4)
            || !strncmp(curp, "-INF", 4))
           && !isalnum(curp[4]))
    {
      int curlin = toksrc_line;
      int curcol = toksrc_col;
      bool pos = *curp == '+';
      double infd = (pos
                     ?std::numeric_limits<double>::infinity()
                     : -std::numeric_limits<double>::infinity());
      toksrc_col += 4;
      _f.lextokv = Rps_DoubleValue(infd);
      _f.lexkindob = RPS_ROOT_OB(_98sc8kSOXV003i86w5); //double∈class
      _f.namev= name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (_f.lexkindob, _f.lextokv,
         str,
         curlin, curcol);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token infinity :-◑> " << _f.res);
      return _f.res;
    } //- end lexing infinities

  /// lex names or objectids
  else if (isalpha(*curp) || *curp == '_')
    {
      const char*startname = curp;
      int curlin = toksrc_line;
      int curcol = toksrc_col;
      int startcol = curcol;
      while ((isalpha(*curp) || *curp == '_') && toksrc_col<(int)linelen)
        curp++, toksrc_col++;
      std::string namestr(startname, toksrc_col-startcol);
      RPS_DEBUG_LOG(REPL, "get_token oid|name " << namestr);
      _f.oblex = Rps_ObjectRef::find_object_by_string(&_, namestr,
                 Rps_ObjectRef::Null_When_Missing);
      _f.namev = name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      if (_f.oblex)
        {
          _f.lexkindob = RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ); //object∈class
          Rps_LexTokenZone* lextok =
            Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (_f.lexkindob, _f.lextokv,
             str,
             curlin, curcol);
          _f.res = Rps_LexTokenValue(lextok);
          RPS_DEBUG_LOG(REPL, "get_token object :-◑> " << _f.res);
          return _f.res;
        }
      else if (isalpha(namestr[0]))  // new symbol
        {
          _f.lexkindob = RPS_ROOT_OB(_36I1BY2NetN03WjrOv); //symbol∈class
          Rps_LexTokenZone* lextok =
            Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (_f.lexkindob, _f.lextokv,
             str,
             curlin, curcol);
          _f.res = Rps_LexTokenValue(lextok);
          RPS_DEBUG_LOG(REPL, "get_token symbol :-◑> " << _f.res);
          return _f.res;
        }
      else   // bad name
        {
          toksrc_col = startcol;
          RPS_DEBUG_LOG(REPL, "get_token bad name " << _f.namev << " at L" << toksrc_line << ",C" << toksrc_col
                        << " of " << toksrc_name);
          return nullptr;
        }
    }
  //// literal single line strings are like in C++
  else if (*curp == '"')   /// plain literal string, on a single line
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      std::string litstr =
        rps_lex_literal_string(toksrc_name.c_str(), toksrc_linebuf.c_str(), toksrc_line, toksrc_col);
      _f.namev= name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lexkindob = RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); //string∈class
      _f.lextokv = Rps_String::make(litstr);
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token single-line string :-◑> " << _f.res);
      return _f.res;
    } // end single-line literal string token

  //// raw literal strings may span across several lines, like in C++
  //// see https://en.cppreference.com/w/cpp/language/string_literal
  else if (*curp == 'R'
           && curp[0] == 'R' && curp[1] == '"' && isalpha(curp[2]))
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      std::string litstr = lex_raw_literal_string(&_);
      _f.namev= name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lexkindob = RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); //string∈class
      _f.lextokv = Rps_String::make(litstr);
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token multi-line literal string :-◑> " << _f.res);
      return _f.res;
    } // end possibly multi-line raw literal strings

  //// a code chunk or macro string is mixing strings and
  //// objects.... Inspired by GCC MELT see
  //// starynkevitch.net/Basile/gcc-melt/MELT-Starynkevitch-DSL2011.pdf
  /* Improvement over GCC MELT: a macro string can start with "#{"
     ending with "}#" or "#a{" ending with "}a#" or "#ab{" ending with
     "}ab#" or "#abc{" ending with "}abc#" or "#abcd{" ending with
     "}abcd#" or "#abcde{" ending with "}abcde#" or "#abcdef{" ending
     with "}abcdef#" with the letters being arbitrary latin letters,
     upper or lowercase. but no more than 7 letters. */
  else if (curp[0] == '#'
           && (
             curp[1] == '{'
             || (isalpha(curp[1])
                 && (curp[2] == '{')
                 || (isalpha(curp[2])
                     && (curp[3] == '{'
                         || (isalpha(curp[3])
                             && (curp[4] == '{'
                                 || (isalpha(curp[4])
                                     && (curp[5] == '{')
                                     || (isalpha(curp[6])
                                         && (curp[6] == '{'
                                             || (isalpha(curp[7])
                                                 && (curp[7] == '{'
                                                     || (isalpha(curp[7])
                                                         && (curp[8] == '{'
                                                             || (isalpha(curp[8])
                                                                 && curp[9] == '{')
                                                            )
                                                        )
                                                    )
                                                )
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
           )
          )
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      RPS_DEBUG_LOG(REPL, "get_token code_chunk starting at " << toksrc_name
                    << ":L" << linestart << ",C" << colstart << " " << curp);
      _f.namev= name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lextokv = lex_code_chunk(&_);
      _f.lexkindob = RPS_ROOT_OB(_3rXxMck40kz03RxRLM); //code_chunk∈class
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate5<Rps_LexTokenZone,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token code_chunk :-◑> " << _f.res);
      return _f.res;
#warning  Rps_TokenSource::get_token should lex code chunks
      RPS_FATALOUT("unimplemented Rps_TokenSource::get_token code chunk @ " << name()
                   << ":L" << toksrc_line << ",C" << toksrc_col);

    }

#warning Rps_TokenSource::get_token unimplemented
  RPS_FATALOUT("unimplemented Rps_TokenSource::get_token @ " << name()
               << ":L" << toksrc_line << ",C" << toksrc_col);
  // we should refactor properly the rps_repl_lexer & Rps_LexTokenZone constructor here
} // end Rps_TokenSource::get_token


std::string
Rps_TokenSource::lex_raw_literal_string(Rps_CallFrame*callframe)
{
  std::string result;
#warning some code from rps_lex_raw_literal_string file repl_rps.cc lines 1050-1115 should go here
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  const char* curp = curcptr();
  size_t linelen = toksrc_linebuf.size();
  /// For C++, raw literal strings are multi-line, and explained in
  /// en.cppreference.com/w/cpp/language/string_literal ... For
  /// example R"delim(raw characters \)delim" In RefPerSys, we
  /// restrict the <delim> to contain only letters, up to 15 of
  /// them...
  char delim[16];
  memset (delim, 0, sizeof(delim));
  int pos= -1;
  int startlineno= toksrc_line;
  int startcolno= toksrc_col;
  std::string locname = toksrc_name;
  if (sscanf(curp,  "R\"%15[A-Za-z](%n", delim, &pos) < 1
      || !isalpha(delim[0])
      || pos<=1)
    /// should never happen
    RPS_FATALOUT("corrupted Rps_TokenSource::lex_raw_literal_string '"
                 << Rps_Cjson_String(curp) << "'"
                 << std::endl
                 << Rps_ShowCallFrame(callframe));
  toksrc_col += pos;
  RPS_ASSERT(strlen(delim)>0 && strlen(delim)<15);
  char endstr[24];
  memset(endstr, 0, sizeof(endstr));
  snprintf(endstr, sizeof(endstr), ")%s\"", delim);
  RPS_DEBUG_LOG(REPL, "lex_raw_literal_string start L" << startlineno
                << ",C" << startcolno
                << "@" << toksrc_name
                << " endstr " << endstr);
  const char*endp = nullptr;
  while ((curp = curcptr()) != nullptr
         && (endp=strstr(curp, endstr)) == nullptr)
    {
      std::string reststr{curp};
      if (!get_line())
        {
          RPS_WARNOUT("Rps_TokenSource::lex_raw_literal_string without end of string "
                      << endstr
                      << " starting L" << startlineno
                      << ",C" << startcolno
                      << "@" << toksrc_name
                      << std::endl
                      << Rps_ShowCallFrame(callframe));
          throw std::runtime_error(std::string{"lex_raw_literal_string failed to find "}
                                   + endstr);
        }
      result += reststr;
    };				// end while curp....
  if (endp)
    toksrc_col += endp - curp;
  RPS_DEBUG_LOG(REPL, "lex_raw_literal_string gives '"
                << Rps_Cjson_String(result)
                << "' at " << locname << "@L" << startlineno
                << ",C" << startcolno);
  return result;
} // end Rps_TokenSource::lex_raw_literal_string


Rps_Value
Rps_TokenSource::lex_code_chunk(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_Value res;
                );
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  struct Rps_ChunkData_st chkdata = {};
#warning unimplemented Rps_TokenSource::lex_code_chunk, see rps_lex_code_chunk in repl_rps.cc:1133-1232
  RPS_FATALOUT("unimplemented Rps_TokenSource::get_token @ " << name()
               << ":L" << toksrc_line << ",C" << toksrc_col
               << std::endl
               << Rps_ShowCallFrame(&_));
} // end of Rps_TokenSource::lex_code_chunk

Rps_Value
Rps_TokenSource::lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg, Rps_ChunkData_st*chkdata)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_Value res;
                );
  RPS_FATALOUT("unimplemented Rps_TokenSource::lex_chunk_element");
#warning unimplemented Rps_TokenSource::lex_chunk_element, see rps_lex_chunk_element in repl_rps.cc:1229-1415
} // end Rps_TokenSource::lex_chunk_element
//// end of file lexer_rps.cc

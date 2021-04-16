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

/// GNU readline www.gnu.org/software/readline/
#include "readline/readline.h"
#include "readline/history.h"

/// libunistring www.gnu.org/software/libunistring/
#include "unictype.h"
#include "uniconv.h"

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
    Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,
    Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
    (this, _f.lexkindob, _f.lexval, _f.nstrv,
     toksrc_line, toksrc_col);
  return _f.tokenp;
} // end Rps_TokenSource::make_token

const std::string
Rps_TokenSource::position_str(int col) const
{
  if (col<0) col = toksrc_col;
  std::ostringstream outs;
  outs << toksrc_name << ":L" << toksrc_line << ",C:"  << col << std::flush;
  return outs.str();
} // end Rps_TokenSource::position_str

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
  new_input_line();
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
  new_input_line();
  return true;
} // end Rps_CinTokenSource::get_line


Rps_ReadlineTokenSource::Rps_ReadlineTokenSource(std::string path)
  : Rps_TokenSource(path)
{
} // end Rps_ReadlineTokenSource::Rps_ReadlineTokenSource

Rps_ReadlineTokenSource::~Rps_ReadlineTokenSource()
{
};	// end Rps_ReadlineTokenSource::~Rps_ReadlineTokenSource

bool
Rps_ReadlineTokenSource::get_line(void)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_DEBUG_LOG(REPL, "Rps_ReadlineTokenSource::get_line from" << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_ReadlineTokenSource::get_line"));
  char *rl = readline(readline_prompt.c_str());
  if (!rl) return false;
  toksrc_linebuf.assign(rl);
  free (rl), rl = nullptr;
  new_input_line();
  return true;
} // end Rps_ReadlineTokenSource::get_line


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
                           Rps_Value delimv;
                           Rps_ObjectRef obdelim;
                );
  const char* curp = curcptr();
  if (curp)
    RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token start curp='" << Rps_Cjson_String(curp) << "' at " << position_str());
  else
    RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token start no curp at " << position_str());

  ucs4_t curuc=0;
  int ulen= -1;
  size_t linelen = toksrc_linebuf.size();
  // check that we have a proper UTF-8 character
  ulen=curp?u8_strmbtouc(&curuc, (const uint8_t*)curp):0; // length in bytes
  if (ulen<0)
    {
      std::ostringstream errout;
      errout << "bad UTF-8 encoding in " << toksrc_name << ":L" << toksrc_line << ",C" << toksrc_col << std::flush;
      RPS_WARNOUT("Rps_TokenSource::get_token: " << errout.str());
      throw std::runtime_error(errout.str());
    }
  while (curp && isspace(*curp) && toksrc_col<(int)linelen)
    curp++, toksrc_col++;
  if (curp)
    RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token curp='" << Rps_Cjson_String(curp) << "' at " << position_str());
  else
    RPS_DEBUG_LOG(REPL, "Rps_TokenSource::get_token no curp at " << position_str());
  if (toksrc_col>=(int)linelen)
    return nullptr;
  ulen=curp?u8_strmbtouc(&curuc, (const uint8_t*)curp):0; // length in bytes
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
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         curlin, curcol);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token number :-◑> " << _f.res << " @! " << position_str());
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
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         curlin, curcol);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token infinity :-◑> " << _f.res << " @! " << position_str());
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
      _f.namev = name_val(&_);
      RPS_DEBUG_LOG(REPL, "get_token oid|name '" << namestr << "' namev=" << _f.namev << " at "
                    << position_str(startcol) << " ... " << position_str());
      _f.oblex = Rps_ObjectRef::find_object_or_null_by_string(&_, namestr);
      RPS_DEBUG_LOG(REPL, "get_token oid|name '" << namestr << "' oblex=" << _f.oblex);
      const Rps_String* str = _f.namev.to_string();
      RPS_DEBUG_LOG(REPL, "get_token namestr='" << Rps_Cjson_String(namestr) << "' oblex=" << _f.oblex
                    << " namev=" << _f.namev << ", str=" << Rps_Value(str));
      if (_f.oblex)
        {
          _f.lexkindob = RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ); //object∈class
          _f.lextokv = _f.oblex;
          Rps_LexTokenZone* lextokz =
            Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (this, _f.lexkindob, _f.lextokv,
             str,
             curlin, curcol);
          _f.res = Rps_LexTokenValue(lextokz);
          RPS_DEBUG_LOG(REPL, "get_token object :-◑> " << _f.res << " @! " << position_str());
          return _f.res;
        }
      else if (isalpha(namestr[0]))  // new symbol
        {
          _f.lexkindob = RPS_ROOT_OB(_36I1BY2NetN03WjrOv); //symbol∈class
          _f.lextokv = Rps_StringValue(namestr);
          Rps_LexTokenZone* lextok =
            Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (this, _f.lexkindob, _f.lextokv,
             str,
             curlin, curcol);
          _f.res = Rps_LexTokenValue(lextok);
          RPS_DEBUG_LOG(REPL, "get_token symbol :-◑> " << _f.res << " @! " << position_str());
          return _f.res;
        }
      else   // bad name
        {
          toksrc_col = startcol;
          RPS_DEBUG_LOG(REPL, "get_token bad name " << _f.namev << " @! " << position_str());
          return nullptr;
        }
    }
  //// literal single line strings are like in C++
  else if (*curp == '"')   /// plain literal string, on a single line
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      std::string litstr = lex_quoted_literal_string(&_);
      _f.namev= name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lexkindob = RPS_ROOT_OB(_62LTwxwKpQ802SsmjE); //string∈class
      _f.lextokv = Rps_String::make(litstr);
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token single-line string :-◑> " << _f.res << " @! " << position_str());
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
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token multi-line literal string :-◑> " << _f.res << " @! " << position_str());
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
                 && (curp[2] == '{'
                     || (isalpha(curp[2])
                         && (curp[3] == '{'
                             || (isalpha(curp[3])
                                 && (curp[4] == '{'
                                     || (isalpha(curp[4])
                                         && (curp[5] == '{'
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
           )
          )
    {
      int linestart = toksrc_line;
      int colstart = toksrc_col;
      RPS_DEBUG_LOG(REPL, "get_token code_chunk starting at " << position_str() << curp);
      _f.namev= name_val(&_);
      const Rps_String* str = _f.namev.to_string();
      _f.lextokv = lex_code_chunk(&_);
      _f.lexkindob = RPS_ROOT_OB(_3rXxMck40kz03RxRLM); //code_chunk∈class
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         str,
         linestart, colstart);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token code_chunk :-◑> " << _f.res << " @! " << position_str());
      return _f.res;
    } // end lexing code chunk

  //// sequence of at most four ASCII or UTF-8 punctuation
  else if (ispunct(*curp) || uc_is_punct(curuc))
    {
      RPS_DEBUG_LOG(REPL, "get_token start punctuation curp='" << Rps_Cjson_String(curp) << "' at " << position_str());
      std::string delimpos = position_str();
      int startcol = toksrc_col;
      _f.delimv = get_delimiter(&_);
      std::string delimstartstr {curp};
      RPS_DEBUG_LOG(REPL, "get_token after get_delimiter_object delimv=" << _f.delimv << " at " << position_str());
      if (!_f.delimv)
        {
          RPS_WARNOUT("invalid delimiter " << Rps_Cjson_String(delimstartstr) << " at " << delimpos);
          std::string warndelimstr{"invalid delimiter "};
          warndelimstr +=  Rps_Cjson_String(delimstartstr);
          warndelimstr += " at ";
          warndelimstr += delimpos;
          throw std::runtime_error(warndelimstr);
        }
      _f.lexkindob = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
      _f.lextokv = _f.delimv;
      const Rps_String* strv = _f.namev.to_string();
      Rps_LexTokenZone* lextok =
        Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
        (this,_f.lexkindob, _f.lextokv,
         strv,
         toksrc_line, startcol);
      _f.res = Rps_LexTokenValue(lextok);
      RPS_DEBUG_LOG(REPL, "get_token delimiter :-◑> " << _f.res << " at " << position_str());
      return _f.res;
    }
#warning Rps_TokenSource::get_token unimplemented
  RPS_FATALOUT("unimplemented Rps_TokenSource::get_token @ " << name() << " @! " << position_str());
  // we should refactor properly the rps_repl_lexer & Rps_LexTokenZone constructor here
} // end Rps_TokenSource::get_token


Rps_Value
Rps_TokenSource::get_delimiter(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value res;
                           Rps_ObjectRef obdictdelim;
                           Rps_Value delimv;
                           Rps_Value namev;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lextokv;
                );
  const char*curp=nullptr;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  std::string startpos = position_str();
  _f.obdictdelim = RPS_ROOT_OB(_627ngdqrVfF020ugC5); //"repl_delim"∈string_dictionary
  auto paylstrdict = _f.obdictdelim->get_dynamic_payload<Rps_PayloadStringDict>();
  RPS_ASSERT (paylstrdict != nullptr);
  std::string delimstr;
  int nbdelimch=0; // number of delimiter UTF8 characters
  bool again=false;
  const char* startp = curcptr();
  unsigned startcol = toksrc_col;
  ucs4_t curuc=0;
  RPS_ASSERT(startp);
  curp = startp;
  do
    {
      curuc = 0;
      int ulen= -1;
      again=false;
      if (!curp)
        break;
      if (isspace(*curp))
        break;
      // FIXME: maybe we should remember the byte position of every
      // delimiter character (it can be UTF-8 like °)
      if (*curp < 127 && ispunct(*curp))
        {
          delimstr.push_back(*curp);
          curp ++;
          nbdelimch ++;
          again = true;
        }
      else if (*(const uint8_t*)curp > 128
               && (ulen=u8_strmbtouc(&curuc, (const uint8_t*)curp) // length in bytes
                        > 0)
               &&  uc_is_punct(curuc))
        {
          delimstr.append(curp, ulen);
          curp += ulen;
          nbdelimch ++;
          again = true;
        }
      else
        again = false;
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "get_delimiter delimstr='" << Rps_Cjson_String(delimstr) << "' nbdelimch="
                << nbdelimch << " startpos:" << startpos);
  /***
   * TODO: we need to find the longest substring in delimstr which is a known delimiter
   ***/
  int loopcnt = 0;
  static constexpr int maxloopdelim = 16;
  while (!delimstr.empty() && loopcnt < maxloopdelim)
    {
      loopcnt ++;
      _f.delimv = paylstrdict->find(delimstr);
      RPS_DEBUG_LOG(REPL, "get_delimiter punctuation delimv=" << _f.delimv << " for delimstr='"
                    << Rps_Cjson_String(delimstr) << "' loopcnt#" << loopcnt);
      if (_f.delimv)
        {
          _f.lexkindob = RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK); //repl_delimiter∈class
          _f.lextokv = _f.delimv;
          toksrc_col += delimstr.size();
          const Rps_String* strv = _f.namev.to_string();
          Rps_LexTokenZone* lextok =
            Rps_QuasiZone::rps_allocate6<Rps_LexTokenZone,Rps_TokenSource*,Rps_ObjectRef,Rps_Value,const Rps_String*,int,int>
            (this,_f.lexkindob, _f.lextokv,
             strv,
             toksrc_line, startcol);
          _f.res = Rps_LexTokenValue(lextok);
          RPS_DEBUG_LOG(REPL, "get_delimiter delimiter :-◑> " << _f.res << " at " << position_str());
          return _f.res;
        };
      /// truncate the delimstr by one ending UTF-8
      const uint8_t* prevu8 = u8_prev(&curuc,
                                      reinterpret_cast<const uint8_t*>
                                      (delimstr.c_str()+delimstr.size()),
                                      reinterpret_cast<const uint8_t*>(delimstr.c_str()));
      RPS_DEBUG_LOG(REPL, "get_delimiter prevu8='" << (const char*)prevu8
                    << "' for delimstr='" << delimstr << "' with curuc#" << (int)curuc
                    << " at " << startpos << " loopcnt#" << loopcnt);
      if (!prevu8)
        break;
      unsigned curlen = delimstr.size();
      unsigned prevlen = (delimstr.c_str()+delimstr.size() - (const char*)prevu8) +1;
      RPS_DEBUG_LOG(REPL, "get_delimiter for delimstr='" << delimstr <<"' curlen=" << curlen
                    << " prevlen=" << prevlen);
      if (prevlen==0 || prevlen > delimstr.size())
        break;
      delimstr[prevlen] = (char)0; /// facilitates debugging, in principle useless
      delimstr.resize(prevlen);
      RPS_DEBUG_LOG(REPL, "get_delimiter now delimstr='" << delimstr << "' at " << startpos
                    << " prevlen=" << prevlen << " curlen=" << curlen
                    << " loopcnt#" << loopcnt << std::endl);
      usleep (250000);
    }
  RPS_WARNOUT("Rps_TokenSource::get_delimiter failing at " << startpos
              << " for " << startp);
  std::string failmsg {"Rps_TokenSource::get_delimiter failing at "};
  failmsg += startpos;
  throw std::runtime_error{failmsg};
} // end Rps_TokenSource::get_delimiter

std::string
Rps_TokenSource::lex_quoted_literal_string(Rps_CallFrame*callframe)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  std::string rstr;
  const char* curp = curcptr();
  ucs4_t curuc=0;
  size_t linelen = toksrc_linebuf.size();
  const char*eol = curp + (linelen-toksrc_col);
  rstr.reserve(4+2*(eol-curp)/3);
  RPS_ASSERT(*curp == '"');
  toksrc_col++, curp++;
  char curch = 0;
  while ((curch=(*curp)) != (char)0)
    {
      if (curch == '"')
        {
          toksrc_col++;
          return rstr;
        }
      else if (curch == '\\')
        {
          char nextch = curp[1];
          char shortbuf[16];
          memset(shortbuf, 0, sizeof(shortbuf));
          switch (nextch)
            {
            case '\'':
            case '\"':
            case '\\':
              rstr.push_back (nextch);
              toksrc_col += 2;
              continue;
            case 'a':
              rstr.push_back ('\a');
              toksrc_col += 2;
              continue;
            case 'b':
              rstr.push_back ('\b');
              toksrc_col += 2;
              continue;
            case 'f':
              rstr.push_back ('\f');
              toksrc_col += 2;
              continue;
            case 'e':			// ESCAPE
              rstr.push_back ('\033');
              toksrc_col += 2;
              continue;
            case 'n':
              rstr.push_back ('\n');
              toksrc_col += 2;
              continue;
            case 'r':
              rstr.push_back ('\r');
              toksrc_col += 2;
              continue;
            case 't':
              rstr.push_back ('\t');
              toksrc_col += 2;
              continue;
            case 'v':
              rstr.push_back ('\v');
              toksrc_col += 2;
              continue;
            case 'x':
            {
              int p = -1;
              int c = 0;
              if (sscanf (curp + 2, "%02x%n", &c, &p) > 0 && p > 0)
                {
                  rstr.push_back ((char)c);
                  toksrc_col += p + 2;
                  continue;
                }
              else
                goto lexical_error_backslash;
            }
            case 'u': // four hexdigit escape Unicode
            {
              int p = -1;
              int c = 0;
              if (sscanf (curp + 2, "%04x%n", &c, &p) > 0 && p > 0)
                {
                  int l =
                    u8_uctomb ((uint8_t *) shortbuf, (ucs4_t) c, sizeof(shortbuf));
                  if (l > 0)
                    {
                      rstr.append(shortbuf);
                      toksrc_col += p + l;
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
              if (sscanf (curp + 2, "%08x%n", &c, &p) > 0 && p > 0)
                {
                  int l =
                    u8_uctomb ((uint8_t *) shortbuf, (ucs4_t) c, sizeof(shortbuf));
                  if (l > 0)
                    {
                      rstr.append(shortbuf);
                      toksrc_col += p + l;
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
          toksrc_col++;
          continue;
        }
      /// accepts any correctly encoded UTF-8
      else if (int l = u8_mblen ((uint8_t *)(curp), eol-curp); l>0)
        {
          rstr.append(curp, l);
          toksrc_col += l;
          continue;
        }
      /// improbable lexical error....
      else
        {
          RPS_WARNOUT("Rps_TokenSource::lex_quoted_literal_string : lexical error at "
                      << position_str());
          throw std::runtime_error("lexical error");
        }
    } // end while
lexical_error_backslash:
  RPS_WARNOUT("Rps_TokenSource::lex_quoted_literal_string  : bad backslash escape at "
              << position_str());
  throw std::runtime_error("lexical bad backslash escape");
} // end Rps_TokenSource::lex_quoted_literal_string

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
                << "' at " << position_str());
  return result;
} // end Rps_TokenSource::lex_raw_literal_string


Rps_Value
Rps_TokenSource::lex_code_chunk(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obchunk;
                           Rps_Value namev;
                           Rps_Value res;
                           Rps_Value chunkelemv;
                );
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  struct Rps_ChunkData_st chkdata = {};
  chkdata.chunkdata_magic = rps_chunkdata_magicnum;
  chkdata.chunkdata_lineno = toksrc_line;
  chkdata.chunkdata_colno = toksrc_col;
  chkdata.chunkdata_name = toksrc_name;
  const char* curp = curcptr();
  _f.namev= name_val(&_);
  RPS_ASSERT(curp != nullptr && *curp != (char)0);
  char startchunk[12];
  memset(startchunk, 0, sizeof(startchunk));
  int pos= -1;
  if (!strncmp(curp, "#{", 2))
    {
      pos = 2;
      startchunk[0] = (char)0;
      strcpy(chkdata.chunkdata_endstr, "}#");
    }
  else if (sscanf(curp,  "#%6[a-zA-Z]{%n", startchunk, &pos)>0 && pos>0 && isalpha(startchunk[0]))
    {
      snprintf(chkdata.chunkdata_endstr, sizeof(chkdata.chunkdata_endstr), "}%s#", startchunk);
    }
  else // should never happen
    RPS_FATALOUT("corrupted Rps_TokenSource::lex_code_chunk @ " << position_str()
                 << " " << curp
                 << std::endl);
  _f.obchunk =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                               nullptr);
  _f.obchunk->put_attr2(RPS_ROOT_OB(_1B7ITSHTZWp00ektj1), //input∈symbol
                        _f.namev,
                        RPS_ROOT_OB(_5FMX3lrhiw601iqPy5), //line∈symbol
                        Rps_Value((intptr_t)chkdata.chunkdata_lineno, Rps_Value::Rps_IntTag{})
                       );
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk @ " << position_str()
                << " start obchunk:" << _f.obchunk
                << " endstr:'" << chkdata.chunkdata_endstr << "'");
  auto paylvec = _f.obchunk->put_new_plain_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvec);
  int oldline = -1;
  int oldcol = -1;
  toksrc_col += strlen(chkdata.chunkdata_endstr);
  chkdata.chunkdata_colno += strlen(chkdata.chunkdata_endstr);
  do
    {
      oldline = toksrc_line;
      oldcol = toksrc_col;
      _f.chunkelemv = lex_chunk_element(&_, _f.obchunk, &chkdata);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk @ " << name()
                    << ":L" << toksrc_line << ",C" << toksrc_col
                    << std::endl
                    << " obchunk=" << _f.obchunk
                    << ", chunkelemv=" << _f.chunkelemv);
      if (_f.chunkelemv)
        paylvec->push_back(_f.chunkelemv);
      /// for $. chunkelemv is null, but position changed...
      ///
      /// possibly related:
      /// https://framalistes.org/sympa/arc/refpersys-forum/2020-12/msg00036.html
    }
  while (_f.chunkelemv || toksrc_col>oldcol || toksrc_line>oldline);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_code_chunk "
                << " :-◑> obchunk=" << _f.obchunk << " @!" << position_str());
  return _f.obchunk;
} // end of Rps_TokenSource::lex_code_chunk



Rps_Value
Rps_TokenSource::lex_chunk_element(Rps_CallFrame*callframe, Rps_ObjectRef obchkarg, Rps_ChunkData_st*chkdata)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(chkdata && chkdata->chunkdata_magic == rps_chunkdata_magicnum);
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_3rXxMck40kz03RxRLM), //code_chunk∈class
                           /*callerframe:*/callframe,
                           Rps_Value res;
                           Rps_ObjectRef obchunk;
                           Rps_ObjectRef namedob;
                );
  _f.obchunk = obchkarg;
  RPS_DEBUG_LOG(LOW_REPL, "Rps_TokenSource::lex_chunk_element chunkdata_colno=" << chkdata->chunkdata_colno
                << " curpos:" << position_str()
                << " linebuf:'" << toksrc_linebuf << "' of size:" << toksrc_linebuf.size());
  RPS_ASSERT(chkdata->chunkdata_colno>=0
             && chkdata->chunkdata_colno <= (int)toksrc_linebuf.size());
  const char*pc = toksrc_linebuf.c_str() + chkdata->chunkdata_colno;
  const char*eol =  toksrc_linebuf.c_str() +  toksrc_linebuf.size();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element pc='" << Rps_Cjson_String(pc) << "'"
                << " @" << position_str(chkdata->chunkdata_colno));
  if (!pc || pc[0] == (char)0 || pc == eol)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element end-of-line");
      toksrc_col = toksrc_linebuf.size();
      return nullptr;
    }
  // name-like chunk element
  if (isalpha(*pc))
    {
      /// For C name-like things, we return the object naming them or else a string
      int startnamecol =  chkdata->chunkdata_colno;
      const char*startname = pc;
      const char*endname = pc;
      const char*eol =  toksrc_linebuf.c_str() +  toksrc_linebuf.size();
      while ((isalnum(*endname) || *endname=='_') && endname<eol)
        endname++;
      std::string curname(startname, endname - startname);
      _f.namedob = Rps_ObjectRef::find_object_or_null_by_string(&_, curname);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element curname='" << curname
                    << "' in " << position_str(startnamecol)
                    << " namedob=" << _f.namedob);
      chkdata->chunkdata_colno += endname - startname;
      if (_f.namedob)
        _f.res = Rps_ObjectValue(_f.namedob);
      else
        _f.res = Rps_StringValue(curname);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element curname='" << curname
                    << "' res=" << _f.res << " at " << position_str(startnamecol));

      return _f.res;
    }
  // integer (base 10) chunk element
  else if (isdigit(*pc) || (pc[0] == '-' && isdigit(pc[1])))
    {
      char* endnum = nullptr;
      long long ll = strtoll(pc, &endnum, 10);
      int startcol =  chkdata->chunkdata_colno ;
      chkdata->chunkdata_colno += endnum - pc;
      _f.res = Rps_Value((intptr_t)ll,
                         Rps_Value::Rps_IntTag{});
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element number=" << ll
                    << " res=" << _f.res << " at " << position_str(startcol));
      return _f.res;
    }
  /// For sequence of spaces, we return an instance of class space and
  /// value the number of space characters
  else if (isspace(*pc))
    {
      int startspacecol = chkdata->chunkdata_colno;
      int endspacecol = startspacecol;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element start space obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      while (pc<eol && isspace(*pc))
        endspacecol++, pc++;
      _f.res = Rps_InstanceValue(RPS_ROOT_OB(_2i66FFjmS7n03HNNBx), //space∈class
                                 std::initializer_list<Rps_Value>
      {
        Rps_Value((intptr_t)(endspacecol-startspacecol),
        Rps_Value::Rps_IntTag{})
      });
      chkdata->chunkdata_colno += endspacecol-startspacecol;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element obchunk=" << _f.obchunk
                    << " -> number res=" << _f.res
                    << " @" << position_str(startspacecol)
                    << " now chunking @ " << position_str(chkdata->chunkdata_colno));
      return _f.res;
    }
  /// code chunk meta-variable or meta-notation....
  else if (*pc == '$' && pc < eol)
    {
      int startcol = chkdata->chunkdata_colno;
      // a dollar followed by a name is a meta-variable; that name should be known
      if (isalpha(pc[1]))
        {
          const char*startname = pc+1;
          const char*endname = startname;
          while (endname < eol && (isalnum(*endname) || *endname == '_'))
            endname++;;
          std::string metaname(startname, endname-startname);
          _f.namedob = Rps_ObjectRef::find_object_or_null_by_string(&_, metaname);
          if (!_f.namedob)
            {
              RPS_WARNOUT("lex_chunk_element: unknown metavariable name " << metaname
                          << " in " << position_str(startcol));
              throw std::runtime_error(std::string{"lexical error - bad metaname "} + metaname + " in code chunk");
            }
          chkdata->chunkdata_colno += (endname-startname) + 1;
          _f.res = Rps_InstanceValue(RPS_ROOT_OB(_1oPsaaqITVi03OYZb9), //meta_variable∈symbol
                                     std::initializer_list<Rps_Value> {_f.namedob});
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element space obchunk=" << _f.obchunk
                        << " -> metavariable res=" << _f.res
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.res;
        }
      /// two dollars are parsed as one
      else if (pc[1]=='$')
        {
          chkdata->chunkdata_colno += 2;
          _f.res = Rps_StringValue("$");
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element dollar obchunk=" << _f.obchunk
                        << " -> dollar-string res=" << _f.res
                        << " @L" << chkdata->chunkdata_lineno << ",C"
                        <<  chkdata->chunkdata_colno);
          return _f.res;
        }
    }
  //// end of chunk is }letters# or }#
  else if (*pc == '}' && !strcmp(pc, chkdata->chunkdata_endstr))
    {
      chkdata->chunkdata_colno += strlen(chkdata->chunkdata_endstr);
      _f.res = nullptr;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element end-of-chunk obchunk=" << _f.obchunk
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      toksrc_col += strlen(chkdata->chunkdata_endstr);
      return nullptr;
    }
  //// any other sequence of UTF-8 excluding right brace } or dollar sign $; we stop lexing when letter, digit, space
  else
    {
      RPS_ASSERT(eol != nullptr && eol >= pc);
      size_t restsiz = eol - pc;
      const char *startpc = pc;
      const uint8_t* curu8p = (const uint8_t*)pc;
      const uint8_t* eolu8p = (const uint8_t*)eol;
      while (curu8p < eolu8p)
        {
          if (*(const char*)curu8p == '}' || *(const char*)curu8p == '$')
            break;
          if (isspace(*pc))
            break;
          if (isalnum(*pc))
            break;
          int u8len = u8_mblen(curu8p, eolu8p - curu8p);
          if (u8len <= 0)
            break;
          curu8p += u8len;
          pc += u8len;
        };
      std::string str{startpc, curu8p-(const uint8_t*)startpc};
      _f.res = Rps_StringValue(str);
      chkdata->chunkdata_colno += str.size();
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lex_chunk_element strseq obchunk=" << _f.obchunk
                    << " -> plain-string res=" << _f.res
                    << " @L" << chkdata->chunkdata_lineno << ",C"
                    <<  chkdata->chunkdata_colno);
      return _f.res;
    }
#warning Rps_TokenSource::lex_chunk_element should parse delimiters...
  RPS_FATALOUT("unimplemented Rps_TokenSource::lex_chunk_element obchunk=" << _f.obchunk << " @ " << name()
               << ":L" << toksrc_line << ",C" << toksrc_col);
#warning unimplemented Rps_TokenSource::lex_chunk_element, see rps_lex_chunk_element in repl_rps.cc:1229-1415
} // end Rps_TokenSource::lex_chunk_element


Rps_Value
Rps_TokenSource::lookahead_token(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, unsigned rank)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                );
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token start rank#" << rank << " token_deq:" << token_deq
                << " pos:" << position_str() << " from:" << std::endl << Rps_ShowCallFrame(&_));
  RPS_ASSERT(_.call_frame_depth() < 32);
  while (token_deq.size() < rank)
    {
      _f.lextokv = get_token(&_);
      if (_f.lextokv)
        token_deq.push_back(_f.lextokv);
      else
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token rank#" << rank << " missing from:"
                        << std::endl << Rps_ShowCallFrame(&_));
          return nullptr;
        }
    };
  if (rank<token_deq.size())
    {
      _f.lextokv = token_deq[rank];
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token rank#" << rank << " => " << _f.lextokv);
      return _f.lextokv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::lookahead_token rank#" << rank << " missing:"
                << std::endl << Rps_ShowCallFrame(&_));
  return nullptr;
} // end Rps_TokenSource::lookahead_token




void
rps_repl_lexer_test(void)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0S6DQvp3Gop015zXhL),  //lexical_token∈class
                           /*callerframe:*/nullptr,
                           Rps_Value curlextokenv;
                );
  RPS_ASSERT(rps_is_main_thread());

  RPS_TIMER_START();

  RPS_DEBUG_LOG(REPL, "start rps_repl_lexer_test gitid " << rps_gitid
                << " callframe:" << Rps_ShowCallFrame(&_));
  rl_attempted_completion_function = rpsrepl_name_or_oid_completion;
  Rps_ReadlineTokenSource rltoksrc("-*-");
  int tokcnt=0;
  int lincnt = 0;
  while (!rps_repl_stopped)
    {
      char prompt[32];
      memset(prompt, 0, sizeof(prompt));
      if (lincnt % 4 == 0)
        {
          usleep(32768); // to slow down on infinite loop
          RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test startloop lincnt=" << lincnt
                        << " "
                        << rltoksrc.position_str()
                        << std::endl
                        <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_lexer_test startloop"));
        };
      snprintf(prompt, sizeof(prompt), "Rps_LEXTEST#%d:", lincnt);
      rltoksrc.set_prompt(prompt);
      do
        {
          _f.curlextokenv = rltoksrc.get_token(&_);
          if (_f.curlextokenv)
            {
              tokcnt++;
              RPS_INFORMOUT("token#" << tokcnt << ":" << _f.curlextokenv
                            << " from " << rltoksrc.position_str());
            }
          else
            RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test no token "
                          << rltoksrc.position_str());
        }
      while (_f.curlextokenv);
      if (!rltoksrc.get_line())
        break;
      lincnt++;
      RPS_DEBUG_LOG(REPL, "rps_repl_lexer_test got fresh line#" << lincnt
                    << " '"
                    << Rps_Cjson_String(rltoksrc.current_line()) << "' "
                    << rltoksrc.position_str());
    }
  RPS_DEBUG_LOG(REPL, "end rps_repl_lexer_test lincnt=" << lincnt
                << " tokcnt=" << tokcnt
                << " at " << rltoksrc.position_str()
                << std::endl);

  RPS_TIMER_STOP(REPL);
} // end rps_repl_lexer_test


//// end of file lexer_rps.cc

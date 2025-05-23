
/****************************************************************
 * file suparsrepl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the support routines for parsing the Read Eval Print Loop
 *      (moved some code from parsrepl_rps.cc to here in july 2023)
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2019 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_suparsrepl_gitid[];
const char rps_suparsrepl_gitid[]= RPS_GITID;

extern "C" const char rps_suparsrepl_date[];
const char rps_suparsrepl_date[]= __DATE__;

extern "C" const char rps_suparsrepl_shortgitid[];
const char rps_suparsrepl_shortgitid[]= RPS_SHORTGITID;

void
rps_parsrepl_failing_at(const char*fil, int lin, int cnt, const std::string&failstr)
{
  /// added to facilitate gdb debugging and breakpoints....
  asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
  ///
  RPS_DEBUG_PRINTF_AT(fil,lin,REPL,"§ParsReplFailing#%d: %s", cnt, failstr.c_str());
  /// added to facilitate gdb debugging....
  asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
  RPS_DEBUG_LOG_AT(fil,lin,REPL,"rps_parsrepl_failing_at " << failstr);
  /// added to facilitate gdb debugging....
  asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"
                " nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
  ///
} // end rps_parsrepl_failing_at

/// hopefully these TokenSource::parse routines could be used a lot...


//// routine to parse a binary operation like Aleft BINOP Aright. Is
//// given BINOP as an operation (semantics) and a delimiter
//// (syntax). Is given the parsing C++ closure for both Aleft and
//// Aright.
Rps_Value
Rps_TokenSource::parse_symmetrical_binaryop(Rps_CallFrame*callframe,
    Rps_ObjectRef binoper, Rps_ObjectRef bindelim,
    std::function<Rps_Value(Rps_CallFrame*,bool*)> parser_binop,
    bool*pokparse, const char*opername)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED, //
                 callframe,
                 Rps_Value resexprsymv;
                 Rps_Value lextokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_ObjectRef binoperob;
                 Rps_ObjectRef bindelimob;
                );
  static long callcnt;
  long callnum= ++callcnt;
  if (!opername)
    opername="???";
  bool leftok = false;
  bool rightok = false;
  std::string startpos = position_str();
  /// ensure the GC knows them
  _f.binoperob = binoper;
  _f.bindelimob = bindelim;
  RPS_DEBUG_LOG(REPL, "+Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername
                << " in " << (*this)
                << " START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_) << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (is_looking_ahead(pokparse))
    {
      // TODO: review, ... only lookahead for left part....
      _f.leftv = parser_binop(&_, RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername
                    << " lookahead " << (_f.leftv?"success":"failure"));
      return _f.leftv;
    }
  else
    _f.leftv = parser_binop(&_, &leftok);
  if (!leftok)
    {
      RPS_PARSREPL_FAILURE(&_,"-Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " "
                           << opername << " LEFT FAILURE startpos:" <<  startpos
                           << " curpos" << position_str()
                           << std::endl
                           << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/fail-left"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername
                << " leftv=" << _f.leftv << " in " << (*this)
                << "startpos:" << startpos << " pos:" << position_str() << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  _f.lextokv =  lookahead_token(&_, 0);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv);
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername << " after leftv=" << _f.leftv
                << " lextokv:" << _f.lextokv
                << " in:" << (*this)
                << "startpos:" << startpos << " pos:" << position_str()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/after-leftv") << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object()
      &&  _f.lextokv.to_lextoken()->lxval().to_object() == bindelim)
    {
      (void) get_token(&_); // consume the operator
    }
  else
    {
      RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " "
                           << opername << " BAD DELIM FAILURE startpos:" <<  startpos
                           << " curpos" << position_str()  << " calldepth="
                           << rps_call_frame_depth(&_)
                           << std::endl
                           << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/fail-bad-delim"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername << " after bindelim:" << bindelim
                << " leftv=" << _f.leftv
                << " lextokv:" << _f.lextokv
                << " in:" << (*this)
                << "startpos:" << startpos << " pos:" << position_str());
  _f.rightv = parser_binop(&_,&rightok);
  if (!rightok)

    {
      RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " "
                           << opername << " FAIL at  " << position_str()
                           << " start: " << startpos
                           << "leftv:" << _f.leftv
                           << " lextokv:" << _f.lextokv
                           << " in:" << (*this) << std::endl
                           << "… token_deq:" << toksrc_token_deq << std::endl
                           << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      RPS_WARNOUT("Rps_TokenSource::parse_symmetrical_binop failed for "<< opername << " at " << position_str()
                  << " starting " << startpos
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/fail-right"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  else
    {
      /// _f.rightv has been parsed
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop¤" << callnum << " " << opername << " RIGHT  startpos:" <<  startpos
                    << " curpos" << position_str() << " leftv:" << _f.leftv
                    << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES rightv " << _f.rightv
                    << " in:" << (*this) << std::endl
                    << "… token_deq:" << toksrc_token_deq << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.leftv;
    }
  _f.resexprsymv = Rps_InstanceValue(_f.binoperob, {_f.leftv, _f.rightv});
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " calldepth="
                << rps_call_frame_depth(&_)
                << " GIVES resexprsymv=" << _f.resexprsymv  << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr() << std::endl
                << "… token_deq:" << toksrc_token_deq);
  return _f.resexprsymv;
} // end Rps_TokenSource::parse_symmetrical_binaryop



//// Routine to parse a binary operation like A BINOP B. Is given
//// BINOP as an operation (semantics to construct the result) and a
//// delimiter (lexing and syntax). Is given the parsing C++ closure
//// for parsing A and the other parsing C++ closure for parsing B.
////
Rps_Value
Rps_TokenSource::parse_asymmetrical_binaryop(Rps_CallFrame*callframe,
    Rps_ObjectRef binoper, Rps_ObjectRef bindelim,
    std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,bool*)>
    parser_leftop,
    std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,bool*)>
    parser_rightop,
    bool*pokparse, const char*opername)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr, //
                           callframe,
                           Rps_Value resexprsymv;
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_ObjectRef binoperob;
                           Rps_ObjectRef bindelimob;
                );
  static long callcnt;
  long callnum= ++ callcnt;
  if (!opername)
    opername="???";
  bool leftok = false;
  bool rightok = false;
  std::string startpos = position_str();
  /// ensure the GC knows them
  _f.binoperob = binoper;
  _f.bindelimob = bindelim;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername
                <<" START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " in " << (*this) << " calldepth="
                << rps_call_frame_depth(&_)
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (is_looking_ahead(pokparse))
    {
      /// for lookahead just run the left parsing...
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername <<" before LOOKAHEAD "
                    << "  startpos:" << startpos
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());

      _f.leftv = parser_leftop(&_,this,RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername
                    <<" after LOOKAHEAD leftv: " << _f.leftv
                    << "  startpos:" << startpos
                    << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr() << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
  else
    _f.leftv = parser_leftop(&_,this,&leftok);
  if (!leftok)
    {
      RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " LEFT FAILURE startpos:" <<  startpos
                           << " curpos" << position_str()  << " calldepth="
                           << rps_call_frame_depth(&_)
                           << " token_deq:" << toksrc_token_deq
                           << " curcptr:" << curcptr());
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " before lefttokzero "
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr());
  _f.lextokv =  lookahead_token(&_, 0);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername
                    << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      return _f.leftv;
    }
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object()
      &&  _f.lextokv.to_lextoken()->lxval().to_object() == bindelim)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " beforeA rightop "
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
#warning perhaps Rps_TokenSource::consume_front_token(&_) should be called here?
      (void) get_token(&_); // consume the operator
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " beforeB rightop "
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      _f.rightv = parser_rightop(&_,this,&rightok);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " after rightop "
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr() << " rightv=" << _f.rightv << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (!rightok)
        {
          RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " RIGHT FAILURE startpos:" <<  startpos
                               << " curpos" << position_str()  << " calldepth="
                               << rps_call_frame_depth(&_)
                               << " token_deq:" << toksrc_token_deq
                               << " curcptr:" << curcptr());
          RPS_WARNOUT("failed to parse asymmetrical right operand for "<< opername << " at " << position_str()
                      << " starting " << startpos);
          if (pokparse)
            *pokparse = false;
          return nullptr;
        }
    }
  else
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop¤" << callnum << " " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      return _f.leftv;
    }
  _f.resexprsymv = Rps_InstanceValue(_f.binoperob, {_f.leftv, _f.rightv});
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical binop¤" << callnum << " " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " calldepth="
                << rps_call_frame_depth(&_)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr());
  return _f.resexprsymv;
} // end Rps_TokenSource::parse_asymmetrical_binaryop




///// Routine to parse a sequence of n operands X1, X2, ... each
///// separated by the same operation OP so X1 OP X2 OP X3 ... OP Xn
Rps_Value
Rps_TokenSource::parse_polyop(Rps_CallFrame*callframe, Rps_ObjectRef polyoper, Rps_ObjectRef polydelim,
                              std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,bool*)> parser_suboperand,
                              bool*pokparse, const char*opername)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_Value resexprv;
                 Rps_Value lextokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value curargv;
                 Rps_ObjectRef operob;
                 Rps_ObjectRef delimob;
                );
  std::vector<Rps_Value> argvect;
  bool leftok = false;
  static long callcnt;
  long callnum= ++ callcnt;
  /// GC needs this:
  _f.operob = polyoper;
  _f.delimob = polydelim;
  _.set_additional_gc_marker([&](Rps_GarbageCollector* gc)
  {
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_polyop ?
    for (auto curargv : argvect)
      gc->mark_value(curargv);
  });
  std::string startpos = position_str();
  if (!opername)
    opername="???";
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername
                << " START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_)
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (is_looking_ahead(pokparse))
    {
      // TODO: review, ... only lookahead for first suboperand....
      _f.leftv = parser_suboperand(&_,this, RPS_DO_LOOKAHEAD);
      if (_f.leftv)
        RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " lookahead success"
                      << " token_deq:" << toksrc_token_deq
                      << " curcptr:" << curcptr());
      else
        RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " lookahead failure"
                             << " token_deq:" << toksrc_token_deq
                             << " curcptr:" << curcptr());
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " before leftsuboper"
                << " START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr() << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  _f.leftv = parser_suboperand(&_,this,&leftok);
  if (!leftok)
    {
      RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " LEFT FAILURE startpos:" <<  startpos
                           << " curpos" << position_str()  << " calldepth="
                           << rps_call_frame_depth(&_)
                           << " token_deq:" << toksrc_token_deq
                           << " curcptr:" << curcptr());
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  argvect.push_back(_f.leftv);
  while (_f.lextokv.is_lextoken()
         && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
         &&  _f.lextokv.to_lextoken()->lxval().is_object()
         &&  _f.lextokv.to_lextoken()->lxval().to_object() == polydelim)
    {
      bool okarg = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " loop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << " startpos:" << startpos);
      (void) get_token(&_); // consume the operator delimiter
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " loop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << " startpos:" << startpos << " before parsersuboper");
      _f.curargv = parser_suboperand(&_,this,&okarg);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername
                    << " loop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << " startpos:" << startpos
                    << " after parsersuboper curargv:" << _f.curargv << ", "
                    << (okarg?"ok":"fail") << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (!okarg)
        {
          RPS_PARSREPL_FAILURE(&_, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername
                               << " failed to parse operand#" << argvect.size() <<" for "<< opername << " at " << position_str()
                               << " starting " << startpos);
          RPS_WARNOUT("parse_polyop failed to parse operand#" << argvect.size() <<" for "<< opername << " at " << position_str()
                      << " starting " << startpos);
          break;
        }
      argvect.push_back(_f.curargv);
      okarg = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername << " endloop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << std::endl
                    << "… argvect:" << argvect);
    };        // end while loop
  _f.resexprv = Rps_InstanceValue(_f.operob, argvect);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop¤" << callnum << " " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " resexpr:" << _f.resexprv
                << " calldepth="
                << rps_call_frame_depth(&_)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr());
  if (pokparse)
    *pokparse = true;
  return _f.resexprv;
} // end Rps_TokenSource::parse_polyop

Rps_Value
Rps_TokenSource::parse_using_closure(Rps_CallFrame*callframe, Rps_ClosureValue closval)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_ClosureValue closv;
                 Rps_Value resv;
                );
  _f.closv = closval;
  RPS_ASSERT(_f.closv.is_closure());
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_using_closure unimplemented closv=" << _f.closv
                << " position:" << position_str()
                << " token_deq:" << toksrc_token_deq
                << " in:" << (*this)
                << " curcptr:" << curcptr() << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  RPS_PARSREPL_FAILURE(&_,"Rps_TokenSource::parse_using_closure unimplemented closv=" << _f.closv
                       << " position:" << position_str()
                       << " token_deq:" << toksrc_token_deq
                       << " in:" << (*this)
                       << " curcptr:" << curcptr());
  RPS_FATALOUT("Rps_TokenSource::parse_using_closure unimplemented closv=" << _f.closv
               << " position:" << position_str()
               << " token_deq:" << toksrc_token_deq
               << " in:" << (*this)
               << " curcptr:" << curcptr());
#warning unimplemented Rps_TokenSource::parse_using_closure
  /* TODO: we should apply the _f.closv .... */
  return _f.resv;
} // end Rps_TokenSource::parse_using_closure

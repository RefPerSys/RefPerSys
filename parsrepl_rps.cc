
/****************************************************************
 * file parsrepl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the parser support for the Read Eval Print Loop
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
  RPS_LOCALFRAME(/*descr:*/nullptr,callframe,
                           Rps_Value resexprsymv;
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_ObjectRef binoperob;
                           Rps_ObjectRef bindelimob;
                );
  if (!opername)
    opername="???";
  bool leftok = false;
  bool rightok = false;
  std::string startpos = position_str();
  /// ensure the GC knows them
  _f.binoperob = binoper;
  _f.bindelimob = bindelim;
  RPS_DEBUG_LOG(REPL, "+Rps_TokenSource::parse_symmetrical_binop " << opername
                << " in " << (*this)
                << " START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_));
  if (is_looking_ahead(pokparse))
    {
      // TODO: review, ... only lookahead for left part....
      _f.leftv = parser_binop(&_, RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop " << opername
                    << " lookahead " << (_f.leftv?"success":"failure"));
      return _f.leftv;
    }
  else
    _f.leftv = parser_binop(&_, &leftok);
  if (!leftok)
    {
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::parse_symmetrical_binop- "
                    << opername << " LEFT FAILURE startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/fail-left"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop " << opername
                << " leftv=" << _f.leftv << " in " << (*this)
                << "startpos:" << startpos << " pos:" << position_str());
  _f.lextokv =  lookahead_token(&_, 0);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::parse_symmetrical_binop " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv);
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop " << opername << " after leftv=" << _f.leftv
                << " lextokv:" << _f.lextokv
                << " in:" << (*this)
                << "startpos:" << startpos << " pos:" << position_str()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/after-leftv"));
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object()
      &&  _f.lextokv.to_lextoken()->lxval().to_object() == bindelim)
    {
      (void) get_token(&_); // consume the operator
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop "
                    << opername << " BAD DELIM FAILURE startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/fail-bad-delim"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop " << opername << " after bindelim:" << bindelim
                << " leftv=" << _f.leftv
                << " lextokv:" << _f.lextokv
                << " in:" << (*this)
                << "startpos:" << startpos << " pos:" << position_str());
  _f.rightv = parser_binop(&_,&rightok);
  if (!rightok)
    {
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::parse_symmetrical_binop " << opername << " FAIL at  " << position_str()
                    << " start: " << startpos
                    << "leftv:" << _f.leftv
                    << " lextokv:" << _f.lextokv
                    << " in:" << (*this) << std::endl
                    << "... token_deq:" << toksrc_token_deq);
      RPS_WARNOUT("Rps_TokenSource::parse_symmetrical_binop failed for "<< opername << " at " << position_str()
                  << " starting " << startpos
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_symmetrical_binop/fail-right"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  else
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv
                    << " in:" << (*this) << std::endl
                    << "... token_deq:" << toksrc_token_deq);
      return _f.leftv;
    }
  _f.resexprsymv = Rps_InstanceValue(_f.binoperob, {_f.leftv, _f.rightv});
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_symmetrical_binop " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " calldepth="
                << rps_call_frame_depth(&_)
                << " GIVES resexprsymv=" << _f.resexprsymv  << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr() << std::endl
                << "... token_deq:" << toksrc_token_deq);
  return _f.resexprsymv;
} // end Rps_TokenSource::parse_symmetrical_binaryop



//// Routine to parse a binary operation like A BINOP B. Is given
//// BINOP as an operation (semantics) and a delimiter (syntax). Is
//// given the parsing C++ closure for parsing A and the other parsing
//// C++ closure for parsing B.
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
  RPS_LOCALFRAME(/*descr:*/nullptr,callframe,
                           Rps_Value resexprsymv;
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_ObjectRef binoperob;
                           Rps_ObjectRef bindelimob;
                );
  if (!opername)
    opername="???";
  bool leftok = false;
  bool rightok = false;
  std::string startpos = position_str();
  /// ensure the GC knows them
  _f.binoperob = binoper;
  _f.bindelimob = bindelim;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername
                <<" START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " in " << (*this) << " calldepth="  << rps_call_frame_depth(&_));
  if (is_looking_ahead(pokparse))
    {
      /// for lookahead just run the left parsing...
      _f.leftv = parser_leftop(&_,this,RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername <<" LOOKAHEAD "
                    << (_f.leftv?"success":"failure")
                    << "  startpos:" << startpos
                    << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
    }
  else
    _f.leftv = parser_leftop(&_,this,&leftok);
  if (!leftok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " LEFT FAILURE startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " before lefttokzero "
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr());
  _f.lextokv =  lookahead_token(&_, 0);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " beforeA rightop "
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
#warning perhaps Rps_TokenSource::consume_front_token(&_) should be called here?
      (void) get_token(&_); // consume the operator
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " beforeB rightop "
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      _f.rightv = parser_rightop(&_,this,&rightok);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " after rightop "
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr() << " rightv=" << _f.rightv);
      if (!rightok)
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " RIGHT FAILURE startpos:" <<  startpos
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_asymmetrical_binop " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      return _f.leftv;
    }
  _f.resexprsymv = Rps_InstanceValue(_f.binoperob, {_f.leftv, _f.rightv});
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical binop " << opername <<" END position:" << position_str()
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
  RPS_LOCALFRAME(/*descr:*/nullptr,callframe,
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername
                << " START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_));
  if (is_looking_ahead(pokparse))
    {
      // TODO: review, ... only lookahead for first suboperand....
      _f.leftv = parser_suboperand(&_,this, RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername << " lookahead " << (_f.leftv?"success":"failure")
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr());
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername << " before leftsuboper"
                << " START position:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr());
  _f.leftv = parser_suboperand(&_,this,&leftok);
  if (!leftok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername << " LEFT FAILURE startpos:" <<  startpos
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername << " loop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << " startpos:" << startpos);
      (void) get_token(&_); // consume the operator delimiter
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername << " loop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << " startpos:" << startpos << " before parsersuboper");
      _f.curargv = parser_suboperand(&_,this,&okarg);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername
                    << " loop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << " startpos:" << startpos
                    << " after parsersuboper curargv:" << _f.curargv << ", "
                    << (okarg?"ok":"fail"));
      if (!okarg)
        {
          RPS_WARNOUT("parse_polyop failed to parse operand#" << argvect.size() <<" for "<< opername << " at " << position_str()
                      << " starting " << startpos);
          break;
        }
      argvect.push_back(_f.curargv);
      okarg = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername << " endloop polydelim:" << polydelim  << " curpos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr:" << curcptr()
                    << std::endl
                    << "... argvect:" << argvect);
    };				// end while loop
  _f.resexprv = Rps_InstanceValue(_f.operob, argvect);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_polyop " << opername <<" END position:" << position_str()
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



/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_expression(Rps_CallFrame*callframe, bool*pokparse)
{
  // a REPL expression is a sequence of disjuncts separated by ||
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value resexprv;
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_ObjectRef lexkindob;
                           Rps_ObjectRef ordelimob;
                           Rps_ObjectRef oroperob;
                           Rps_Value lexvalv;
                );
  std::vector<Rps_Value> disjvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_expression ?
    // but the disjvect needs to be GC-marked
    for (auto disjv : disjvect)
      gc->mark_value(disjv);
  });
  bool ok = false;
  static std::atomic_long exprcounter;
  long exprnum = 1+ exprcounter.fetch_add(1);
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#" << exprnum << " START position:"
                << startpos << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " in:" << (*this) << std::endl
                << "... token_deq:" << toksrc_token_deq
                << " calldepth=" << rps_call_frame_depth(&_)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression start"));
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " looked tokrk0 lextokv="
                << _f.lextokv << std::endl << "... in:" << (*this)
                << " position:" << position_str() << " startpos:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " failing_A at startpos:" << startpos
                    << " in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_A"));
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " before parse_disjunction"
                << " in:" << (*this)
                << " position:" << position_str()
                << " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << " token_deq:" << toksrc_token_deq);
  _f.leftv = parse_disjunction(&_, &ok);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " got disjunction "
                << (ok?"ok":"bad") << " leftv=" << _f.leftv
                <<  " position:" << position_str()<< " startpos:" << startpos
                << std::endl
                << " token_deq:" << toksrc_token_deq);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " failing_B (no-left-disjunction) at startpos:" << startpos
                    << " in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_B (noleftdisj)"));
      return nullptr;
    }
  disjvect.push_back(_f.leftv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " leftv=" << _f.leftv
                << " disjvect:" << disjvect
                <<  " position:" << position_str()<< " startpos:" << startpos);
  bool again = false;
  static Rps_Id idordelim;
  if (!idordelim)
    idordelim = Rps_Id("_1HsUfOkNw0W033EIW1"); // id of "or!binop"∈repl_delimiter
  static Rps_Id idoroper;
  if (!idoroper)
    idoroper = Rps_Id("_1ghZV0g1dtR02xPgqk"); // id of "or!binop"∈repl_binary_operator
  do
    {
      again = false;
      _f.lextokv =  lookahead_token(&_,  0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " testing or lextokv=" << _f.lextokv
                    << " position:" << position_str()<< " startpos:" << startpos << " disjvect:" << disjvect
                    << std::endl << " ... in:" << (*this));
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == idordelim)
        {
          (void) get_token(&_); // consume the or operator
          again = true;
          if (!_f.oroperob)
            _f.oroperob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,idoroper);
        }
      else
        again = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " oroperob="
                    << _f.oroperob << " disjvect:" << disjvect
                    << " position:" << position_str()
                    << " startpos:" << startpos
                    << " in:" << (*this)
                    << (again?"again":"notagain"));
      if (again)
        {
          bool okright=false;
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " before parsedisjright disjvect:" << disjvect
                        << " position:" << position_str()
                        << " startpos:" << startpos
                        << "  in:" << (*this));
          _f.rightv = parse_disjunction(&_, &okright);
          if (okright)
            {
              disjvect.push_back(_f.rightv);
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " after parsedisjright rightv=" << _f.rightv
                            << " disjvect:" << disjvect
                            << " position:" << position_str()
                            << " startpos:" << startpos
                            << "  in:" << (*this));
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_C"));
              RPS_WARNOUT("parse_expression failed to parse disjunct at " << position_str()
                          << " startpos:" << startpos);
              return nullptr;
            }
        }
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " oroperob=" << _f.oroperob
                << "nbdisj:" << disjvect.size()
                << std::endl
                << "...token_deq:" << toksrc_token_deq
                << " position:" << position_str()<< " startpos:" << startpos);
  if (disjvect.size() > 1)
    {
      /// we make an instance:
      _f.resexprv = Rps_InstanceValue(_f.oroperob, disjvect);
    }
  else
    {
      _f.resexprv = disjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression#"  << exprnum << " result: "
                << _f.resexprv << " position:" << position_str()<< " startpos:" << startpos
                << std::endl
                << " token_deq:" << toksrc_token_deq
                << " calldepth=" << rps_call_frame_depth(&_) << std::endl
                << " in:" << (*this));
  return _f.resexprv;
} // end Rps_TokenSource::parse_expression




////////////////
/// This member function returns some disjunct which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_disjunction(Rps_CallFrame*callframe, bool*pokparse)
{
  /// a disjunction is a sequence of one or more conjunct separated by
  /// && - the and operator
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value resdisjv;
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_ObjectRef lexkindob;
                           Rps_ObjectRef ordelimob;
                           Rps_ObjectRef oroperob;
                           Rps_Value lexvalv;
                );
  std::vector<Rps_Value> conjvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_disjunction ?
    // but the conjvect needs to be GC-marked
    for (auto conjv : conjvect)
      gc->mark_value(conjv);
  });
  bool ok = false;
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction START startpos:"
                << startpos << " calldepth#" << rps_call_frame_depth(&_)
                << "  in:" << (*this) << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction lextokv=" << _f.lextokv
                << " position: " << position_str() << " startpos=" << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction failing_A"));
      return nullptr;
    }
  _f.leftv = parse_conjunction(&_,  &ok);
  if (ok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction after leftconj leftv=" << _f.leftv
                    << " at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction/after-leftconj"));
    }
  else
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction failing_B (noleftconj) at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction failing_B/noleftconj"));
      return nullptr;
    }
  conjvect.push_back(_f.leftv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction leftv=" << _f.leftv
                << " position: " << position_str() << " startpos:" << startpos);
  bool again = false;
  static Rps_Id idordelim;
  if (!idordelim)
    idordelim = Rps_Id("_1HsUfOkNw0W033EIW1"); // id of "or!delim"∈repl_delimiter
  static Rps_Id idoroper;
  if (!idoroper)
    idoroper = Rps_Id("_1ghZV0g1dtR02xPgqk"); // id of "or!binop"∈repl_binary_operator
  do
    {
      again = false;
      _f.lextokv =  lookahead_token(&_,  0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction testing and lextokv=" << _f.lextokv << " position:" << position_str() << " startpos:" << startpos);
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind()
          == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == idordelim)
        {
          (void) get_token(&_); // consume the or operator
          again = true;
          if (!_f.oroperob)
            _f.oroperob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,idoroper);
        }
      else
        again = false;
      if (again)
        {
          bool okright=false;
          _f.rightv = parse_conjunction(&_,  &okright);
          if (okright)
            conjvect.push_back(_f.rightv);
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction failing_C"));
              RPS_WARNOUT("Rps_TokenSource::parse_disjunction failed to parse conjunct at " << position_str() << " starting " << startpos);
              return nullptr;
            }
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction position:" << position_str() << " startpos:" << startpos << "endloop "
                    << (again?"again":"STOP")
                    << " calldepth#" << rps_call_frame_depth(&_));
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction oroperob=" << _f.oroperob
                << "nbconj:" << conjvect.size()  << " position:" << position_str() << " startpos:" << startpos
                << " calldepth#" << rps_call_frame_depth(&_));
  if (conjvect.size() > 1)
    {
      /// we make an instance:
      _f.resdisjv = Rps_InstanceValue(_f.oroperob, conjvect);
    }
  else
    {
      _f.resdisjv = conjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction result "
                << _f.resdisjv << " position:" << position_str()
                << " startpos:" << startpos
                << " calldepth#" << rps_call_frame_depth(&_));
  return _f.resdisjv;
} // end Rps_TokenSource::parse_disjunction





////////////////
Rps_Value
Rps_TokenSource::parse_conjunction(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_LOCALFRAME(nullptr, callframe,
                 Rps_Value lextokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_Value disjv;
                 Rps_Value conjv;
                 Rps_ObjectRef lexkindob;
                 Rps_ObjectRef anddelimob;
                 Rps_ObjectRef andbinopob;
                 Rps_Value lexvalv;);

  std::vector<Rps_Value> conjvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector* gc)
  {
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_conjunction ?
    for (auto disjv : conjvect)
      gc->mark_value(disjv);
  });
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction DTART at " << startpos
                << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_));
  static Rps_Id id_and_delim;
  if (!id_and_delim)
    id_and_delim = Rps_Id("_2YVmrhVcwW00120rTK");
  static Rps_Id id_and_binop;
  if (!id_and_binop)
    id_and_binop = Rps_Id("_8xk4sKglyzG01dloqq");
  _f.anddelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_and_delim);
  _f.andbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_and_binop);
  //
  bool ok = false;
  _f.lextokv = lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction lextokv="  << _f.lextokv
                << " position: " << position_str() << " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " anddelim:" << _f.anddelimob
                << " andbinop:" << _f.andbinopob
                << " token_deq:" << toksrc_token_deq);

  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing_A"));

      return nullptr;
    }
  //
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction before left comparison in:" << (*this)
                << " position:" << position_str()
                << " lextokv:" << _f.lextokv
                << " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  _f.leftv = parse_comparison(&_, &ok);

  if (ok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction got left comparison " << _f.leftv
                    << " position:" << position_str()
                    << " startpos:" << startpos
                    << " curcptr:" << Rps_QuotedC_String(curcptr()));

    }
  else
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction failing_B (noleftcompar) at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing_B/noleftcompar"));
      return nullptr;
    }

  conjvect.push_back(_f.leftv);

  bool again = false;
  do
    {
      again = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction conjvect:" << conjvect
                    << " at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()));
      _f.lextokv =  lookahead_token(&_,  0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction testing or lextokv=" << _f.lextokv
                    << " position:" << position_str());
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction conjvect:" << conjvect
                    << " at startpos:" << startpos
                    << "  in:" << (*this)
                    << " curcptr:" << Rps_QuotedC_String(curcptr()));
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == id_and_delim)
        {
          (void) get_token(&_); // consume the and operator
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction startpos:" << startpos
                        << "  in:" << (*this)
                        << " position:" << position_str()
                        << " curcptr:" << Rps_QuotedC_String(curcptr()));
          again = true;
          if (!_f.andbinopob)
            _f.andbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_and_binop);
        }
      else
        again = false;
      if (again)
        {
          bool okright=false;
          _f.rightv = parse_comparison(&_,  &okright);
          if (okright)
            conjvect.push_back(_f.rightv);
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing_C"));

              RPS_WARNOUT("failed to parse conjunct at " << position_str());
              return nullptr;
            }
        }
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction andbinopob=" << _f.andbinopob
                << "nbconj:" << conjvect.size() << " at " << startpos);
  if (conjvect.size() > 1)
    {
      /// we make an instance:
      _f.conjv = Rps_InstanceValue(_f.andbinopob, conjvect);
    }
  else
    {
      _f.conjv = conjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction gives "
                << _f.conjv << " at " << startpos);
  return _f.conjv;
} // end Rps_TokenSource::parse_conjunction


Rps_Value
Rps_TokenSource::parse_comparison(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(nullptr, callframe,
                 Rps_Value lextokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_Value compv;
                 Rps_ObjectRef cmpdelimob;
                 Rps_ObjectRef cmpbinopob;
                );
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison begin at " << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  /// lessequal is <=
  static Rps_Id id_lessequal_delim;
  if (!id_lessequal_delim)
    id_lessequal_delim = Rps_Id("_1mfq8qfixB401umCL9");
  static Rps_Id id_lessequal_binop;
  if (!id_lessequal_binop)
    id_lessequal_binop = Rps_Id("_7kiAonuM6tZ018OcyU");
  // less is <
  static Rps_Id id_less_delim;
  if (!id_less_delim)
    id_less_delim = Rps_Id("_0yxQCphDiO102S0BnY");
  static Rps_Id id_less_binop;
  if (!id_less_binop)
    id_less_binop = Rps_Id("_7E9GRiz630X04AEDlB");
  /// greater is >
  static Rps_Id id_greater_delim;
  if (!id_greater_delim)
    id_greater_delim = Rps_Id("_04IcidRP5Jh00zW0qs");
  static Rps_Id id_greater_binop;
  if (!id_greater_binop)
    id_greater_binop = Rps_Id("_40mWEiSX65P02fzIdD");
  /// greaterequal is >=
  static Rps_Id id_greaterequal_delim;
  if (!id_greaterequal_delim)
    id_greaterequal_delim = Rps_Id("_312DlYVXeEs01aTJpg");
  static Rps_Id id_greaterequal_binop;
  if (!id_greaterequal_binop)
    id_greaterequal_binop = Rps_Id("_8p431uwpLJI00r5FQD");
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison startpos:" << startpos);
  bool okleft = false;
  _f.leftv = parse_comparand(&_,  &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison leftv=" << _f.leftv << " startpos:" << startpos
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()));
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison failing_A"));

      RPS_WARNOUT("parse_comparison failed to parse left comparand at " << startpos
                  << " current line:" << Rps_QuotedC_String(current_line())
                  << " curpos " << position_str()
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison fail"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison after left lextok=" << _f.lextokv << " pos:" << position_str());
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison testing lextokv=" << _f.lextokv << " position:" << position_str());
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object())
    {
      Rps_Id delimid =  _f.lextokv.to_lextoken()->lxval().to_object()->oid();
      RPS_WARNOUT("Rps_TokenSource::parse_comparison incomplete delimid=" << delimid  << " position:" << position_str()
                  << " startpos:" << startpos
                  << std::endl << " leftv=" << _f.leftv << " lextokv=" << _f.lextokv);
    }
  else
    {
      RPS_WARNOUT("Rps_TokenSource::parse_comparison unexpected lextokv="
                  << _f.lextokv << " position:" << position_str()
                  << " startpos:" << startpos
                  << std::endl << " leftv=" << _f.leftv);
    }
#warning unimplemented Rps_TokenSource::parse_comparison
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_comparison from " << Rps_ShowCallFrame(callframe)
               << " in:" << (*this) << " at " << position_str()
               << " startpos:" << startpos
               << std::endl
               << "... leftv=" << _f.leftv << " lextokv=" << _f.lextokv);
} // end Rps_TokenSource::parse_comparison


// a comparand - something on left or right side of compare operators is a sequence of terms with additive operators
Rps_Value
Rps_TokenSource::parse_comparand(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(nullptr, callframe,
                 Rps_Value lextokv;
                 Rps_Value lexopertokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_Value compv;
                 Rps_ObjectRef cmpdelimob;
                 Rps_ObjectRef cmpbinopob;
                );
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand START from " << Rps_ShowCallFrame(&_)
                << " in " << (*this) << " at " <<  startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  bool okleft = false;
  _f.leftv = parse_term(&_,  &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand leftv=" << _f.leftv << " startpos:" << startpos
                    << " currentpos:" << position_str());
    }
  else
    {
      RPS_WARNOUT("parse_comparand failed to parse left comparand at " << startpos
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparand fail"));
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison failing_A"));
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_,  0);
  _f.lexopertokv =  lookahead_token(&_,  1);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand  leftv=" << _f.leftv
                << " lextokv:" << _f.lextokv << " lexopertokv:" << _f.lexopertokv
                << " startpos:" << startpos
                << " currentpos:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
#warning unimplemented Rps_TokenSource::parse_comparand
  /***
   * we probably should loop and collect all terms if they are separated by the same additive operator
   ***/
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_comparand from " << Rps_ShowCallFrame(callframe)
               << " in:" << (*this) << " at startpos: " << startpos
               << " currentpos:" << position_str()
               << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
               << std::endl
               << "... lextokv=" << _f.lextokv << " lexopertokv=" << _f.lexopertokv << " leftv=" << _f.leftv
               << std::endl
               << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparand incomplete"));
} // end Rps_TokenSource::parse_comparand


Rps_Value
Rps_TokenSource::parse_factor(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(nullptr, callframe,
                 Rps_Value lextokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_ObjectRef binoperob;
                 Rps_ObjectRef plusdelimob;
                 Rps_ObjectRef plusbinopob;
                 Rps_ObjectRef minusdelimob;
                 Rps_ObjectRef minusbinopob;
                );
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor START startpos:" << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  /// + delimiter and binary operator
  static Rps_Id id_plus_delim;
  if (!id_plus_delim)
    id_plus_delim = Rps_Id("_4ShDsOWk7al02eDRTM");
  static Rps_Id id_plus_binop;
  if (!id_plus_binop)
    id_plus_binop = Rps_Id("_51jvc2mFhql03qwRg6");
  _f.plusdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_plus_delim); // "plus!delim"∈repl_delimiter
  _f.plusbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_plus_binop); // "plus!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor plusdelimob=" << _f.plusdelimob << " plusbinopob=" << _f.plusbinopob);
  /// - delimiter and binary operator
  static Rps_Id id_minus_delim;
  if (!id_minus_delim)
    id_minus_delim = Rps_Id("_3isfNUOsxXY02Bww6l");
  static Rps_Id id_minus_binop;
  if (!id_minus_binop)
    id_minus_binop = Rps_Id("_13ffyyJhGHI01ySLhK");
  _f.minusdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_minus_delim); // "minus!delim"∈repl_delimiter
  _f.minusbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_minus_binop); // "minus!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor minusdelimob=" << _f.minusdelimob << " minusbinopob=" << _f.minusbinopob);
  ///
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor started from " << Rps_ShowCallFrame(&_)
                << " in " << (*this)<< " at " <<  startpos
                << " token_deq:" << toksrc_token_deq);
  bool okleft = false;
  _f.leftv = parse_primary(&_,  &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor got primary leftv=" << _f.leftv
                    << " startpos:" << startpos << " at " << position_str() << " in " << (*this));
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor failing_A"));
      RPS_WARNOUT("Rps_TokenSource::parse_factor failed to parse left primary at " << position_str()
                  << " startpos:" << startpos << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor fail/left"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor lextokv=" << _f.lextokv
                << " in " << (*this) << " at " <<  startpos);

  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object())
    {
      if (_f.lextokv.to_lextoken()->lxval().to_object() ==  _f.plusdelimob)
        _f.binoperob = _f.plusbinopob;
      else if (_f.lextokv.to_lextoken()->lxval().to_object() == _f.minusdelimob)
        _f.binoperob = _f.minusbinopob;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor lextokv=" << _f.lextokv
                << " in:" << (*this) << " at " <<  startpos << " binoperob=" << _f.binoperob);
  if (_f.binoperob)
    {
      bool okright = false;
      /// consume the + or - delimiter...
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor lexgotokv=" << _f.lexgotokv
                    << " in:" << (*this) << " at "
                    <<  startpos << " binoperob=" << _f.binoperob);
      RPS_ASSERT(_f.lexgotokv  == _f.lextokv);
      _f.rightv = parse_term(&_, &okright);
      if (!okright)
        {
          RPS_WARNOUT("parse_factor failed to parse right term starting " << startpos << " at " << position_str());
          if (pokparse)
            *pokparse = false;
          return nullptr;
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor leftv=" << _f.leftv
                    << " rightv=" << _f.rightv
                    << " in:" << (*this) << " at " << position_str()
                    << " startpos:" <<  startpos << " binoperob=" << _f.binoperob);
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor leftv=" << _f.leftv
                    << " in:" << (*this) << " at "
                    <<  position_str() << " startpos:" << startpos);
    }
#warning incomplete Rps_TokenSource::parse_factor
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_factor from " << Rps_ShowCallFrame(callframe)
               << " in:" << (*this) << " at " << position_str()  << " startpos:" << startpos
               << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor incomplete"));
} // end Rps_TokenSource::parse_factor



/// a term is a sequence of factors with multiplicative operators
/// between them.... All the operators should be the same. Otherwise we build intermediate subexpressions
Rps_Value
Rps_TokenSource::parse_term(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(nullptr, callframe,
                 Rps_Value restermv;
                 Rps_Value lextokv;
                 Rps_Value lexopertokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_ObjectRef binoperob;
                 Rps_ObjectRef curoperob;
                 Rps_ObjectRef bindelimob;
                 Rps_ObjectRef multdelimob;
                 Rps_ObjectRef multbinopob;
                 Rps_ObjectRef divdelimob;
                 Rps_ObjectRef divbinopob;
                 Rps_ObjectRef moddelimob;
                 Rps_ObjectRef modbinopob;
                 Rps_ObjectRef lexoperdelimob;
                );
  std::vector<Rps_Value> operandvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    // maybe token_deq is already GC-marked by caller....
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_term ?
    // but the operandvect needs to be GC-marked
    for (auto operv : operandvect)
      gc->mark_value(operv);
  });
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term begin startpos:"
                << startpos << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  /// multiplication operator and * delim
  static Rps_Id id_mult_delim;
  if (!id_mult_delim)
    id_mult_delim = Rps_Id("_2uw3Se5dPOU00yhxpA"); // id of "mult!delim"∈repl_delimiter
  static Rps_Id id_mult_oper;
  if (!id_mult_oper)
    id_mult_oper = Rps_Id("_4QX7Cg3gDkd005b9bn"); // id of "mult!binop"∈repl_binary_operator
  _f.multdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_delim); // "mult!delim"∈repl_delimiter
  _f.multbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_oper); // "mult!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term startpos:" << startpos << " multdelimob:" << _f.multdelimob
                << " multbinopob: " << _f.multbinopob
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  /// division operator and / delim
  static Rps_Id id_div_delim;
  if (!id_div_delim)
    id_div_delim = Rps_Id("_3ak80l3pr9700M90pz"); // id of "div!delim"∈repl_delimiter
  static Rps_Id id_div_oper;
  if (!id_div_oper)
    id_div_oper = Rps_Id("_0GTVGelTnCP01I0od2"); // id of "div!binop"∈repl_binary_operator
  _f.divdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_delim); // "div!delim"∈repl_delimiter
  _f.divbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_oper); // "div!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term START startpos:" << startpos << " divdelimob:" << _f.divdelimob
                << " divbinopob: " << _f.divbinopob
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_));
  /// modulus operator and % delim
  static Rps_Id id_mod_delim;
  if (!id_mod_delim)
    id_mod_delim = Rps_Id("_5yI4nqeRQdR02PcUIi"); // id of mod!delim"∈repl_delimiter
  static Rps_Id id_mod_oper;
  if (!id_mod_oper)
    id_mod_oper = Rps_Id("_07mKKY7ByIq03w7k4J"); // id of "mod!binop"∈repl_binary_operator
  _f.moddelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mod_delim); // "mod!delim"∈repl_delimiter
  _f.modbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mod_oper); // "mod!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term  startpos:" << startpos << " moddelimob:" << _f.moddelimob
                << " modbinopob: " << _f.modbinopob
                << " pos:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
  /////
  ////////////////
  bool okleft = false;
  _f.leftv = parse_primary(&_, &okleft);
  if (okleft)
    {
      operandvect.push_back(_f.leftv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term leftv=" << _f.leftv << " startpos:" << startpos << "  in:" << (*this)
                    << " operandvect:" << operandvect
                    << " pos:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term failing_A (left nonok) at startpos:" << startpos
                    << "  in:" << (*this)
                    << " leftv:" << _f.leftv
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term failing_A"));
      RPS_WARNOUT("parse_term failed to parse left primary starting " << startpos
                  << " pos:" << position_str()
                  << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term before lookahead_token leftv=" << _f.leftv
                << " in:" << (*this) << " position_str:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)(curcptr()));
  bool again = true;
  int loopcnt = 0;
  while (again)
    {
      loopcnt++;
      RPS_DEBUGNL_LOG(REPL, "Rps_TokenSource::parse_term **startloop @ " << position_str()
                      << " loopcnt#" << loopcnt
                      << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
      again = false;
      _f.lextokv = lookahead_token(&_,  0);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term after leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                    << " in:" << (*this) << " at " <<  startpos << " loopcnt#" << loopcnt
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << ((void*)curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term after left"));
      _f.lexopertokv = lookahead_token(&_,  1);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term after leftv=" << _f.leftv << " lextokv="
                    << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv
                    << " pos:" << position_str() << " startpos:" << startpos
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
      usleep (250000);
      _f.lextokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term got token after leftv=" << _f.leftv << " got lextok=" << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv << "  in:" << (*this)
                    << " @! " << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                    << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term after-left"));
      if (!_f.lextokv)
        break;
      if (_f.lexopertokv && _f.lexopertokv.is_lextoken()
          &&  _f.lexopertokv.to_lextoken()->lxkind()
          == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lexopertokv.to_lextoken()->lxval().is_object())
        {
          _f.lexoperdelimob =  _f.lexopertokv.to_lextoken()->lxval().to_object();
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term got token after leftv=" << _f.leftv
                        << " got lexopertokv=" << _f.lexopertokv << " bindelimob=" << _f.bindelimob
                        << " binoperob=" << _f.binoperob
                        << " @! " << position_str()
                        << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
          if (_f.lexoperdelimob == _f.multdelimob)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term lexopertokv:" << _f.lexopertokv << " multiply at " << position_str());
              _f.curoperob = _f.multbinopob;
            }
          else if (_f.lexoperdelimob == _f.divdelimob)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term lexopertokv:" << _f.lexopertokv << " divide at " << position_str());
              _f.curoperob = _f.divbinopob;
            }
          else if (_f.lexoperdelimob == _f.moddelimob)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term lexopertokv:" << _f.lexopertokv << " modulus at " << position_str());
              _f.curoperob = _f.modbinopob;
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term lexopertokv:" << _f.lexopertokv
                            << " strange lexoperdelimob:" << _f.lexoperdelimob << " :!-> return leftv:" << _f.leftv);
              if (pokparse)
                *pokparse = true;
              return _f.leftv;
            }
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term got token after leftv=" << _f.leftv << " curoperob=" << _f.curoperob
                        << " lexopertokv=" << _f.lexopertokv);
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term operandvect:" << operandvect
                    << " curoperob=" << _f.curoperob << " binoperob=" << _f.binoperob
                    << " loopcnt#" << loopcnt
                    << " pos:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()) << "@" << ((void*)curcptr()));
      if (_f.curoperob)
        {
          if (!_f.binoperob)
            _f.binoperob = _f.curoperob;
          if (_f.binoperob == _f.curoperob)
            {
              bool okright = false;
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term operandvect:" << operandvect << " leftv=" << _f.leftv
                            << " before parse_primary of right" << position_str());
              _f.rightv = parse_primary(&_, &okright);
              if (!okright)
                {
                  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term failing_B at startpos:" << startpos
                                << "  in:" << (*this)
                                << " position:" << position_str()
                                << " curcptr:" << Rps_QuotedC_String(curcptr())
                                << std::endl
                                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term failing_B"));
                  RPS_WARNOUT("Rps_TokenSource::parse_term with invalid primary on right of "
                              << _f.curoperob << " at " << position_str()
                              << " with operandvect:" << operandvect);
                  if (pokparse)
                    *pokparse = false;
                  return nullptr;
                }
              operandvect.push_back(_f.rightv);
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term operandvect:" << operandvect << " leftv=" << _f.leftv
                            << " rightv=" << _f.rightv << " loopcnt#" << loopcnt
                            << " curoperob=" << _f.curoperob << " position_str:" << position_str()
                            << " " << Rps_QuotedC_String(curcptr()));
              again = true;
              continue;
            }
          else
            {
              /* binoperob != curoperob */
#warning Rps_TokenSource::parse_term make two things?
              RPS_FATALOUT("missing code in Rps_TokenSource::parse_term from " << Rps_ShowCallFrame(callframe)
                           << " in:" << (*this) << " at " << startpos
                           << " operandvect:" << operandvect
                           << " rightv:" << _f.rightv
                           << " binoperob:" << _f.binoperob
                           << " curoperob:" << _f.curoperob << _f.curoperob << " position_str:" << position_str()
                           << " " << Rps_QuotedC_String(curcptr()));
            }
        }
      else
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term breakingloop operandvect:" << operandvect << " leftv=" << _f.leftv
                        << " curoperob=" << _f.curoperob << " right=" << _f.rightv
                        << " binoperob=" << _f.binoperob);
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term **endingloop @ " << position_str()
                    << " operandvect:" << operandvect << " curoperob:" << _f.curoperob
                    << std::endl << "... in:" << (*this)
                    << (again?"again":"stop")
                    << " loopcnt#" << loopcnt << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "@" << (void*)(curcptr()));
    } // end while (again)
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term endloop#" << loopcnt
                << " leftv=" << _f.leftv << " in:" << (*this)
                << std::endl << " operandvect:" << operandvect
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << "@" << ((void*)curcptr())
                << " calldepth:" << rps_call_frame_depth(&_));
  if (operandvect.size() == 1)
    {
      _f.restermv = operandvect[0];
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term GIVES restermv=" << _f.restermv << " in:" << (*this)
                    << " startpos:" << startpos << " at:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "@" << ((void*)curcptr())
                    << " calldepth:" << rps_call_frame_depth(&_));
      return _f.restermv;
    }
  else
    {
#warning incomplete  Rps_TokenSource::parse_term
      RPS_FATALOUT("Rps_TokenSource::parse_term INCOMPLETE in:" << (*this)
                   << " curcptr " << Rps_QuotedC_String(curcptr())
                   << "  curpos:" << position_str()
                   << " operandvect:" << operandvect
                   << "  calldepth:" << rps_call_frame_depth(&_)
                   << " binoperob:" << _f.binoperob
                   << " curoperob:" << _f.curoperob
                   << " leftv=" << _f.leftv << std::endl
                   << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term/INCOMPLETE"));
    }
} // end Rps_TokenSource::parse_term




/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_primary(Rps_CallFrame*callframe,  bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lexvalv;
                           Rps_ObjectRef obres;
                           Rps_ObjectRef obdelim;
                           Rps_Value exprv;
                );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_primary ?
  });
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary START startpos:" << startpos
                << "  in:" << (*this)
                << " callframe:" << callframe<< " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << " token_deq:" << toksrc_token_deq<< std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary start"));
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary start lextokv=" << _f.lextokv
                << " startpos:" << startpos << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_A"));
      return nullptr;
    }
  const Rps_LexTokenZone* ltokz = _f.lextokv.to_lextoken();
  RPS_ASSERT (ltokz != nullptr);
  _f.lexkindob = ltokz->lxkind();
  _f.lexvalv = ltokz->lxval();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary lexkindob="
                << _f.lexkindob << " lexval=" << _f.lexvalv << " position:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << "  in:" << (*this));
  if (!can_start_primary(&_))
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary failing bad start lexkindob="
                    << _f.lexkindob << " lexval=" << _f.lexvalv << " position:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "  in:" << (*this));
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary failing_B at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_B"));
      return nullptr;
    }
  if (_f.lexkindob == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) // int
      && _f.lexvalv.is_int())
    {
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary lexgotokv " << _f.lexgotokv
                    << " lexval " << _f.lexvalv
                    << " position:" << position_str() << " curcptr " << Rps_QuotedC_String(curcptr()));
#warning Rps_TokenSource::parse_primary perhaps buggy for test04
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary => int " << _f.lexvalv
                    << " lextokv:" << _f.lextokv
                    << " lexgotokv:" << _f.lexgotokv
                    << " startpos: " << startpos << "  in:" << (*this)
                    << " at " << position_str() << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary int"));
      if (pokparse)
        *pokparse = true;
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
           && _f.lexvalv.is_string())
    {
      _f.lexgotokv = get_token(&_);
      if (pokparse)
        *pokparse = true;
      if (_f.lexgotokv)
        toksrc_token_deq.push_back(_f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary => string " << _f.lexvalv
                    << "  in:" << (*this)
                    << " lexgotokv:" << _f.lexgotokv
                    << " at " << position_str() << " startpos:" << startpos
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary string"));
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
           && _f.lexvalv.is_double())
    {
      _f.lexgotokv = get_token(&_);
      if (pokparse)
        *pokparse = true;
      if (_f.lexgotokv)
        toksrc_token_deq.push_back(_f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary => double " << _f.lexvalv << " lexgotokv:" << _f.lexgotokv
                    << "  in:" << (*this)
                    << " at " << position_str() << " startpos:" << startpos
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary double"));
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
           && _f.lexvalv.is_object())
    {
      _f.obres = _f.lexvalv.to_object();
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary :: object "
                    << _f.obres << " lexgotokv:" << _f.lexgotokv << std::endl
                    << "...  in:" << (*this)
                    << " at " << position_str());
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary :: object "
                    << _f.obres << " next lexgotokv:" << _f.lexgotokv << std::endl
                    << "...  in:" << (*this)
                    << " at " << position_str());
      if (!_f.lexgotokv)
        {
          if (pokparse)
            *pokparse = true;
          return _f.obres;
        }
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK)) //repl_delimiter∊class
    {
      _f.obdelim =  _f.lexvalv.to_object();
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary delimited"
                    << " lexkindob:" << _f.lexkindob
                    << " obdelim=" << _f.obdelim << std::endl
                    << "...  in:" << (*this)
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " position:" << position_str() << " startpos:" << startpos);
      // test for  leftparen _4YM7mv0GrSp03OkF8T
      if (_f.obdelim
          == RPS_ROOT_OB(_4YM7mv0GrSp03OkF8T) //leftparen!delim∊repl_delimiter
         )
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary got leftparen "
                        << "  in:" << (*this) << std::endl
                        << "... lextokv:" << _f.lextokv
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " position:" << position_str() << " startpos:" << startpos << std::endl
                        << " token_deq:" << toksrc_token_deq);
          _f.lexgotokv = get_token(&_); /// consume the leftparen
#warning perhaps Rps_TokenSource::consume_front_token(&_) should be called here?
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary got leftparen before subexpression"
                        << "  in:" << (*this) << std::endl
                        << "... lextokv:" << _f.lextokv
                        << "... lexgotokv:" << _f.lexgotokv
                        << " lexkindob:" << _f.lexkindob
                        << " obdelim=" << _f.obdelim
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " position:" << position_str() << " startpos:" << startpos << std::endl
                        << " token_deq:" << toksrc_token_deq << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary before subexpression"));
          bool oksubexpr = false;
          _f.exprv = parse_expression(&_, &oksubexpr);
          if (oksubexpr)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary after subexpression "
                            << _f.exprv << " startpos:" << startpos << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << " token_deq:" << toksrc_token_deq
                            << "  in:" << (*this)
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary after subexpression"));
#warning  Rps_TokenSource::parse_primary should check for rightparen and consume
              RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary with leftparen " << _f.obdelim
                           << "  in:" << (*this)
                           << " startpos:" << startpos
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
#warning TODO: Rps_TokenSource::parse_primary use rightparen _7CG9m1NXpMo01edTUl and build subexpression object
              /* TODO: we probably should make then return some object or some
              instance for that primary in parenthesis... */
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " token_deq:" << toksrc_token_deq
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_C"));
              RPS_WARNOUT("Rps_TokenSource::parse_primary failed to parse subexpression in parenthesis"
                          << "  in:" << (*this)
                          << " startpos:" << startpos
                          << " position:" << position_str()
                          << " curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                          << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary/failed-subexpression"));
              if (pokparse)
                *pokparse = false;
              return nullptr;
            }
        }
      else
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary failing_D at startpos:" << startpos
                        << "  in:" << (*this)
                        << " position:" << position_str()
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_C"));
          RPS_WARNOUT("Rps_TokenSource::parse_primary failing, unexpected delimiter " << _f.obdelim
                      << "  in:" << (*this)
                      << " startpos:" << startpos
                      << " position:" << position_str());
          if (pokparse)
            *pokparse = false;
          return nullptr;
        }
#warning incomplete Rps_TokenSource::parse_primary with delimiter
    } // end if lexkindob is _2wdmxJecnFZ02VGGFK //repl_delimiter∊class
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary @@incomplete"
                << "  in:" << (*this) << std::endl
                << "... lextokv:" << _f.lextokv
                << " lexkindob:" << _f.lexkindob << std::endl
                << "... startpos:" << startpos
                << " position:" << position_str()
                << std::endl
                << "... curcptr:" << Rps_QuotedC_String(curcptr())
                << " obdelim:" << _f.obdelim
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary@@incomplete"));
  ///////@@@@@@@@@@Rps_TokenSource::parse_primary_complement should be called?
#warning unimplemented Rps_TokenSource::parse_primary
  /** TODO:
   * we probably want to code some recursive descent parser for REPL,
   * but we need some specification (in written English, using EBNF
   * notation....) of REPL expressions
   *
   * That specification of REPL expressions should go into file
   * doc/repl.md or into doc/
   **/
  RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary "
               << Rps_ShowCallFrame(&_) << std::endl
               << "  in:" << (*this)
               << " lextokv:" << _f.lextokv << std::endl
               << " ... lexkindob:" << _f.lexkindob
               << " lexvalv:" << _f.lexvalv
               << " lexgotokv:" << _f.lexgotokv
               << " position_str:" << position_str()
               << " startpos:" << startpos
               << " curcptr:" << Rps_QuotedC_String(curcptr()));
} // end Rps_TokenSource::parse_primary


bool
Rps_TokenSource::can_start_primary(Rps_CallFrame*callframe)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                           Rps_ObjectRef lexkindob;
                           Rps_ObjectRef delimob;
                           Rps_Value lexvalv;
                );
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_parse_primary starting startpos:" << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " callframe:" << callframe
                << " token_deq:" << toksrc_token_deq);
  _f.lextokv =  lookahead_token(&_,  0);
  if (!_f.lextokv)
    return false;
  const Rps_LexTokenZone* ltokz = _f.lextokv.to_lextoken();
  RPS_ASSERT (ltokz != nullptr);
  _f.lexkindob = ltokz->lxkind();
  _f.lexvalv = ltokz->lxval();
  if (_f.lexkindob == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) // int
      && _f.lexvalv.is_int())
    return true;
  else if (_f.lexkindob == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
           && _f.lexvalv.is_double())
    return true;
  else if (_f.lexkindob == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
           && _f.lexvalv.is_string())
    return true;
  else if (_f.lexkindob == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
           && _f.lexvalv.is_object())
    return true;
  else if (_f.lexkindob == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK))   //repl_delimiter∊class
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_parse_primary delimiter "
                    << " lexvalv=" << _f.lexvalv << " at startpos: " << startpos
                    << " position_str:" << position_str() << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::can_parse_primary§delim"));
      if (_f.lexvalv.is_object())
        {
          _f.delimob = _f.lexvalv.to_object();
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_parse_primary delimob "
                        << _f.delimob<< " at startpos: " << startpos
                        << " position_str:" << position_str());
          if (_f.delimob == RPS_ROOT_OB(_4YM7mv0GrSp03OkF8T))   // leftparen!delim∊repl_delimiter
            {
              return true;
            }
        }
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_parse_primary fail lexkindob=" << _f.lexkindob
                << " lexvalv=" << _f.lexvalv);
  return false;
} // end Rps_TokenSource::can_start_primary


Rps_Value
Rps_TokenSource::parse_primary_complement(Rps_CallFrame*callframe, Rps_Value primaryexparg, bool*pokparse)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value primaryexpv;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lexvalv;
                );
  std::string startpos = position_str();
  _f.primaryexpv = primaryexparg;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary_complement begin startpos:" << startpos
                << " primaryexpv:" << _f.primaryexpv << " in " << *this);
  _f.lextokv =  lookahead_token(&_, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary_complement start lextokv=" << _f.lextokv
                << " in:" << (*this)
                << " primaryexpv:" << _f.primaryexpv
                << " position:" << startpos);
  if (pokparse)
    *pokparse = false;
#warning unimplemented Rps_TokenSource::parse_primary_complement
  RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary_complement "
               << Rps_ShowCallFrame(callframe)
               << " in:" << (*this)
               << " primaryexp:" << _f.primaryexpv
               << " startpos:" << startpos);
} // end Rps_TokenSource::parse_primary_complement

///// end of file parsrepl_rps.cc of RefPerSys

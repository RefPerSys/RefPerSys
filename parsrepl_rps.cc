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
Rps_TokenSource::parse_symmetrical_binaryop(Rps_CallFrame*callframe, Rps_DequVal& token_deq,
    Rps_ObjectRef binoper, Rps_ObjectRef bindelim,
    std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,Rps_DequVal&,bool*)> parser_binop,
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse symmetrical binop " << opername <<" START position:" << startpos << " calldepth="
                << rps_call_frame_depth(&_));
  if (is_looking_ahead(pokparse))
    {
      // TODO: review, ... only lookahead for left part....
      _f.leftv = parser_binop(&_,this,token_deq, RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse symmetrical binop " << opername << " lookahead " << (_f.leftv?"success":"failure"));
      return _f.leftv;
    }
  else
    _f.leftv = parser_binop(&_,this,token_deq,&leftok);
  if (!leftok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse symmetrical binop " << opername << " LEFT FAILURE startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv);
      return _f.leftv;
    }
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object()
      &&  _f.lextokv.to_lextoken()->lxval().to_object() == bindelim)
    {
      (void) get_token(&_); // consume the operator
      _f.rightv = parser_binop(&_,this,token_deq,&rightok);
      if (!rightok)
        {
          RPS_WARNOUT("failed to parse right operand for "<< opername << " at " << position_str()
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv);
      return _f.leftv;
    }
  _f.resexprsymv = Rps_InstanceValue(_f.binoperob, {_f.leftv, _f.rightv});
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse symmetrical binop " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " calldepth="
                << rps_call_frame_depth(&_));
  return _f.resexprsymv;
} // end Rps_TokenSource::parse_symmetrical_binaryop



//// Routine to parse a binary operation like A BINOP B. Is given
//// BINOP as an operation (semantics) and a delimiter (syntax). Is
//// given the parsing C++ closure for parsing A and the other parsing
//// C++ closure for parsing B.
////
Rps_Value
Rps_TokenSource::parse_asymmetrical_binaryop(Rps_CallFrame*callframe, Rps_DequVal& token_deq,
    Rps_ObjectRef binoper, Rps_ObjectRef bindelim,
    std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,Rps_DequVal&,bool*)>
    parser_leftop,
    std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,Rps_DequVal&,bool*)>
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical binop " << opername <<" START position:" << startpos << " calldepth="
                << rps_call_frame_depth(&_));
  if (is_looking_ahead(pokparse))
    {
      /// for lookahead just run the left parsing...
      _f.leftv = parser_leftop(&_,this,token_deq,RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse symmetrical binop " << opername <<" LOOKAHEAD "
                    << (_f.leftv?"success":"failure")
                    << "  startpos:" << startpos
                    << " calldepth="
                    << rps_call_frame_depth(&_));
    }
  else
    _f.leftv = parser_leftop(&_,this,token_deq,&leftok);
  if (!leftok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical binop " << opername << " LEFT FAILURE startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv);
      return _f.leftv;
    }
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object()
      &&  _f.lextokv.to_lextoken()->lxval().to_object() == bindelim)
    {
      (void) get_token(&_); // consume the operator
      _f.rightv = parser_rightop(&_,this,token_deq,&rightok);
      if (!rightok)
        {
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical " << opername << " LEFTONLY  startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_)
                    << " GIVES " << _f.leftv);
      return _f.leftv;
    }
  _f.resexprsymv = Rps_InstanceValue(_f.binoperob, {_f.leftv, _f.rightv});
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse asymmetrical binop " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " calldepth="
                << rps_call_frame_depth(&_));
  return _f.resexprsymv;
} // end Rps_TokenSource::parse_asymmetrical_binaryop



///// Routine to parse a sequence of n operands X1, X2, ... each
///// separated by the same operation OP so X1 OP X2 OP X3 ... OP Xn
Rps_Value
Rps_TokenSource::parse_polyop(Rps_CallFrame*callframe, Rps_DequVal& token_deq,  Rps_ObjectRef polyoper, Rps_ObjectRef polydelim,
                              std::function<Rps_Value(Rps_CallFrame*,Rps_TokenSource*,Rps_DequVal&,bool*)> parser_suboperand,
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
    token_deq.gc_mark(*gc);
    for (auto curargv : argvect)
      gc->mark_value(curargv);
  });
  std::string startpos = position_str();
  if (!opername)
    opername="???";
  if (is_looking_ahead(pokparse))
    {
      // TODO: review, ... only lookahead for first suboperand....
      _f.leftv = parser_suboperand(&_,this,token_deq, RPS_DO_LOOKAHEAD);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse symmetrical binop " << opername << " lookahead " << (_f.leftv?"success":"failure"));
      return _f.leftv;
    }
  _f.leftv = parser_suboperand(&_,this,token_deq,&leftok);
  if (!leftok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse polyop " << opername << " LEFT FAILURE startpos:" <<  startpos
                    << " curpos" << position_str()  << " calldepth="
                    << rps_call_frame_depth(&_));
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
      (void) get_token(&_); // consume the operator delimiter
      _f.curargv = parser_suboperand(&_,this,token_deq,&okarg);
      if (!okarg)
        {
          RPS_WARNOUT("parse_polyop failed to parse operand#" << argvect.size() <<" for "<< opername << " at " << position_str()
                      << " starting " << startpos);
          break;
        }
      argvect.push_back(_f.curargv);
    }
  _f.resexprv = Rps_InstanceValue(_f.operob, argvect);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse poly oper " << opername <<" END position:" << position_str()
                << " startpos:" << startpos
                << " resexpr:" << _f.resexprv
                << " calldepth="
                << rps_call_frame_depth(&_));
  if (pokparse)
    *pokparse = true;
  return _f.resexprv;
} // end Rps_TokenSource::parse_polyop



/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_expression(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
    // maybe token_deq is already GC-marked by caller....
    token_deq.gc_mark(*gc);
    // but the disjvect needs to be GC-marked
    for (auto disjv : disjvect)
      gc->mark_value(disjv);
  });
  bool ok = false;
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression start position:" << startpos << " calldepth="
                << rps_call_frame_depth(&_) << std::endl << "parse_expression callframe:"
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression start"));
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression lextokv="
                << _f.lextokv << " position:" << position_str()<< " startpos:" << startpos);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.leftv = parse_disjunction(&_, token_deq, &ok);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression got disjunction " << (ok?"ok":"bad") << " leftv=" << _f.leftv
                <<  " position:" << position_str()<< " startpos:" << startpos);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  disjvect.push_back(_f.leftv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression leftv=" << _f.leftv
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
      _f.lextokv =  lookahead_token(&_, token_deq, 0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression testing or lextokv=" << _f.lextokv
                    << " position:" << position_str()<< " startpos:" << startpos << " disjvect:" << disjvect);
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression oroperob="
                    << _f.oroperob
                    << " position:" << position_str()<< " startpos:" << startpos);
      if (again)
        {
          bool okright=false;
          _f.rightv = parse_disjunction(&_, token_deq, &okright);
          if (okright)
            disjvect.push_back(_f.rightv);
          else
            {
              RPS_WARNOUT("failed to parse disjunct at " << position_str());
              return nullptr;
            }
        }
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression oroperob=" << _f.oroperob
                << "nbdisj:" << disjvect.size()
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression result: "
                << _f.resexprv << " position:" << position_str()<< " startpos:" << startpos
                << " calldepth=" << rps_call_frame_depth(&_)
                << " token_deq=" << token_deq);
  return _f.resexprv;
} // end Rps_TokenSource::parse_expression




////////////////
/// This member function returns some disjunct which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_disjunction(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
    // maybe token_deq is already GC-marked by caller....
    token_deq.gc_mark(*gc);
    // but the conjvect needs to be GC-marked
    for (auto conjv : conjvect)
      gc->mark_value(conjv);
  });
  bool ok = false;
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction startpos:"
                << startpos << " calldepth#" << rps_call_frame_depth(&_));
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction lextokv="
                << _f.lextokv
                << " position: " << position_str() << " startpos=" << startpos);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.leftv = parse_conjunction(&_, token_deq, &ok);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
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
      _f.lextokv =  lookahead_token(&_, token_deq, 0);
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
          _f.rightv = parse_conjunction(&_, token_deq, &okright);
          if (okright)
            conjvect.push_back(_f.rightv);
          else
            {
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
Rps_TokenSource::parse_conjunction(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
    token_deq.gc_mark(*gc);
    for (auto disjv : conjvect)
      gc->mark_value(disjv);
  });
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
  _f.lextokv = lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction lextokv="
                << _f.lextokv
                << " position: "
                << position_str()
                << " anddelim:" << _f.anddelimob
                << " andbinop:" << _f.andbinopob);

  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;

      return nullptr;
    }

  std::string startpos = position_str();
  _f.leftv = parse_comparison(&_, token_deq, &ok);

  if (!ok)
    {
      if (pokparse)
        *pokparse = false;

      return nullptr;
    }

  conjvect.push_back(_f.leftv);

  bool again = false;
  do
    {
      again = false;
      _f.lextokv =  lookahead_token(&_, token_deq, 0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction testing or lextokv=" << _f.lextokv << " position:" << position_str());
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == id_and_delim)
        {
          (void) get_token(&_); // consume the and operator
          again = true;
          if (!_f.andbinopob)
            _f.andbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_and_binop);
        }
      else
        again = false;
      if (again)
        {
          bool okright=false;
          _f.rightv = parse_comparison(&_, token_deq, &okright);
          if (okright)
            conjvect.push_back(_f.rightv);
          else
            {
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
Rps_TokenSource::parse_comparison(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
  _f.leftv = parse_comparand(&_, token_deq, &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison leftv=" << _f.leftv << " startpos:" << startpos);
    }
  else
    {
      RPS_WARNOUT("parse_comparison failed to parse left comparand at " << startpos
                  << " current line:" << Rps_QuotedC_String(current_line())
                  << " curpos " << position_str()
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison fail"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
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
               << " with token_deq=" << token_deq << " at " << position_str()
               << " startpos:" << startpos
               << std::endl
               << "... leftv=" << _f.leftv << " lextokv=" << _f.lextokv);
} // end Rps_TokenSource::parse_comparison


// a comparand - something on left or right side of compare operators is a sequence of terms with additive operators
Rps_Value
Rps_TokenSource::parse_comparand(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand start from " << Rps_ShowCallFrame(&_)
                << " with token_deq=" << token_deq << " at " <<  startpos);
  bool okleft = false;
  _f.leftv = parse_term(&_, token_deq, &okleft);
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
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  _f.lexopertokv =  lookahead_token(&_, token_deq, 1);
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
               << " with token_deq=" << token_deq << " at startpos: " << startpos
               << " currentpos:" << position_str()
               << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
               << std::endl
               << "... lextokv=" << _f.lextokv << " lexopertokv=" << _f.lexopertokv << " leftv=" << _f.leftv
               << std::endl
               << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparand incomplete"));
} // end Rps_TokenSource::parse_comparand


Rps_Value
Rps_TokenSource::parse_factor(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor start from " << Rps_ShowCallFrame(&_)
                << " with token_deq=" << token_deq << " at " <<  startpos);
  bool okleft = false;
  _f.leftv = parse_primary(&_, token_deq, &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor got primary leftv=" << _f.leftv
                    << " startpos:" << startpos << " at " << position_str());
    }
  else
    {
      RPS_WARNOUT("Rps_TokenSource::parse_factor failed to parse left primary at " << position_str()
                  << " startpos:" << startpos << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor fail/left"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor lextokv=" << _f.lextokv
                << " with token_deq=" << token_deq << " at " <<  startpos);

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
                << " with token_deq=" << token_deq << " at " <<  startpos << " binoperob=" << _f.binoperob);
  if (_f.binoperob)
    {
      bool okright = false;
      /// consume the + or - delimiter...
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor lexgotokv=" << _f.lexgotokv
                    << " with token_deq=" << token_deq << " at "
                    <<  startpos << " binoperob=" << _f.binoperob);
      RPS_ASSERT(_f.lexgotokv  == _f.lextokv);
      _f.rightv = parse_term(&_, token_deq, &okright);
      if (!okright)
        {
          RPS_WARNOUT("parse_factor failed to parse right term starting " << startpos << " at " << position_str());
          if (pokparse)
            *pokparse = false;
          return nullptr;
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor leftv=" << _f.leftv
                    << " rightv=" << _f.rightv
                    << " with token_deq=" << token_deq << " at " << position_str()
                    << " startpos:" <<  startpos << " binoperob=" << _f.binoperob);
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor leftv=" << _f.leftv
                    << " with token_deq=" << token_deq << " at "
                    <<  position_str() << " startpos:" << startpos);
    }
#warning incomplete Rps_TokenSource::parse_factor
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_factor from " << Rps_ShowCallFrame(callframe)
               << " with token_deq=" << token_deq << " at " << position_str()  << " startpos:" << startpos
               << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor incomplete"));
} // end Rps_TokenSource::parse_factor



/// a term is a sequence of factors with multiplicative operators
/// between them.... All the operators should be the same. Otherwise we build intermediate subexpressions
Rps_Value
Rps_TokenSource::parse_term(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
    token_deq.gc_mark(*gc);
    // but the operandvect needs to be GC-marked
    for (auto operv : operandvect)
      gc->mark_value(operv);
  });
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term starting startpos:" << startpos);
  /// multiplication operator and * delim
  static Rps_Id id_mult_delim;
  if (!id_mult_delim)
    id_mult_delim = Rps_Id("_2uw3Se5dPOU00yhxpA"); // id of "mult!delim"∈repl_delimiter
  static Rps_Id id_mult_oper;
  if (!id_mult_oper)
    id_mult_oper = Rps_Id("_4QX7Cg3gDkd005b9bn"); // id of "mult!binop"∈repl_binary_operator
  _f.multdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_delim); // "mult!delim"∈repl_delimiter
  _f.multbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_oper); // "mult!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term  startpos:" << startpos << " multdelimob:" << _f.multdelimob
                << " multbinopob: " << _f.multbinopob);
  /// division operator and / delim
  static Rps_Id id_div_delim;
  if (!id_div_delim)
    id_div_delim = Rps_Id("_3ak80l3pr9700M90pz"); // id of "div!delim"∈repl_delimiter
  static Rps_Id id_div_oper;
  if (!id_div_oper)
    id_div_oper = Rps_Id("_0GTVGelTnCP01I0od2"); // id of "div!binop"∈repl_binary_operator
  _f.divdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_delim); // "div!delim"∈repl_delimiter
  _f.divbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_oper); // "div!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term  startpos:" << startpos << " divdelimob:" << _f.divdelimob
                << " divbinopob: " << _f.divbinopob);
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
  _f.leftv = parse_primary(&_, token_deq, &okleft);
  if (okleft)
    {
      operandvect.push_back(_f.leftv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term leftv=" << _f.leftv << " startpos:" << startpos << " token_deq:" << token_deq
                    << " operandvect:" << operandvect
                    << " pos:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
    }
  else
    {
      RPS_WARNOUT("parse_term failed to parse left primary starting " << startpos
                  << " pos:" << position_str()
                  << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term before lookahead_token leftv=" << _f.leftv
                << " token_deq=" << token_deq << " position_str:" << position_str()
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
      _f.lextokv = lookahead_token(&_, token_deq, 0);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term after leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                    << " with token_deq=" << token_deq << " at " <<  startpos << " loopcnt#" << loopcnt
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << ((void*)curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term after left"));
      _f.lexopertokv = lookahead_token(&_, token_deq, 1);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term after leftv=" << _f.leftv << " lextokv="
                    << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv
                    << " pos:" << position_str() << " startpos:" << startpos
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
      usleep (250000);
      _f.lextokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term got token after leftv=" << _f.leftv << " got lextok=" << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv << " token_deq:" << token_deq
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
              _f.rightv = parse_primary(&_, token_deq, &okright);
              if (!okright)
                {
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
                           << " with token_deq=" << token_deq << " at " << startpos
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
                    << std::endl << "... token_deq=" << token_deq
                    << (again?"again":"stop")
                    << " loopcnt#" << loopcnt << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "@" << (void*)(curcptr()));
    } // end while (again)
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term endloop#" << loopcnt
                << " leftv=" << _f.leftv << " token_deq=" << token_deq
                << std::endl << " operandvect:" << operandvect
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << "@" << ((void*)curcptr())
                << " calldepth:" << rps_call_frame_depth(&_));
  if (operandvect.size() == 1)
    {
      _f.restermv = operandvect[0];
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term GIVES restermv=" << _f.restermv << " token_deq=" << token_deq
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "@" << ((void*)curcptr())
                    << " calldepth:" << rps_call_frame_depth(&_));
      return _f.restermv;
    }
  else
    {
    }
#warning unimplemented Rps_TokenSource::parse_term
  /* we probably should make a term with operandvect here ... */
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_term from " << Rps_ShowCallFrame(callframe) << std::endl
               << "... operandvect:" << operandvect
               << " binoperob:" << _f.binoperob
               << " curoperob:" << _f.curoperob
               << " leftv=" << _f.leftv << std::endl
               << "... with token_deq=" << token_deq << " at " << startpos
               << "  curpos:" << position_str()
               << " curcptr " << Rps_QuotedC_String(curcptr())
               << "@" << ((void*)curcptr())
               << " calldepth:" << rps_call_frame_depth(&_));
} // end Rps_TokenSource::parse_term




/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_primary(Rps_CallFrame*callframe, Rps_DequVal& token_deq, bool*pokparse)
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
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary starting startpos:" << startpos
                << " token_deq:" << token_deq
                << " callframe:" << callframe);
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary start lextokv=" << _f.lextokv
                << " startpos:" << startpos << " token_deq:" << token_deq);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  const Rps_LexTokenZone* ltokz = _f.lextokv.to_lextoken();
  RPS_ASSERT (ltokz != nullptr);
  _f.lexkindob = ltokz->lxkind();
  _f.lexvalv = ltokz->lxval();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary lexkindob="
                << _f.lexkindob << " lexval=" << _f.lexvalv << " position:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << " token_deq:" << token_deq);
  if (!can_start_primary(&_, token_deq))
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary failing bad start lexkindob="
                    << _f.lexkindob << " lexval=" << _f.lexvalv << " position:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << token_deq);
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  if (_f.lexkindob == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) // int
      && _f.lexvalv.is_int())
    {
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary lexgotokv " << _f.lexgotokv
                    << " lexval " << _f.lexvalv
                    << " position:" << position_str() << " curcptr " << Rps_QuotedC_String(curcptr()));
#warning Rps_TokenSource::parse_primary  buggy near here for REPL command show 1 * 2 + 3 * 4
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary => int " << _f.lexvalv
                    << " lextokv:" << _f.lextokv
                    << " lexgotokv:" << _f.lexgotokv
                    << " startpos: " << startpos << " token_deq:" << token_deq
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
        token_deq.push_back(_f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary => string " << _f.lexvalv
                    << " token_deq:" << token_deq
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
        token_deq.push_back(_f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary => double " << _f.lexvalv << " lexgotokv:" << _f.lexgotokv
                    << " token_deq:" << token_deq
                    << " at " << position_str() << " startpos:" << startpos
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary double"));
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
           && _f.lexvalv.is_object())
    {
      _f.obres = _f.lexvalv.to_object();
      token_deq.pop_front();
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary :: object "
                    << _f.obres << " lexgotokv:" << _f.lexgotokv << std::endl
                    << "... token_deq:" << token_deq
                    << " at " << position_str());
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary :: object "
                    << _f.obres << " next lexgotokv:" << _f.lexgotokv << std::endl
                    << "... token_deq:" << token_deq
                    << " at " << position_str());
      if (!_f.lexgotokv)
        return _f.obres;
#warning  incomplete Rps_TokenSource::parse_primary with object
      /*** sometimes a primary make be followed by complement */
      RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary object§unimplemented: "
                   << " token_deq:" << token_deq
                   << " lextokv:" << _f.lextokv
                   << " lexkindob:" << _f.lexkindob
                   << " lexvalv:" << _f.lexvalv);
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK)) //repl_delimiter∊class
    {
      _f.obdelim =  _f.lexvalv.to_object();
      token_deq.pop_front();
      // test for  leftparen _4YM7mv0GrSp03OkF8T
      if (_f.obdelim
          == RPS_ROOT_OB(_4YM7mv0GrSp03OkF8T) //leftparen!delim∊repl_delimiter
         )
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary subexpression"
                        << " token_deq:" << token_deq
                        << " lextokv:" << _f.lextokv
                        << " lexkindob:" << _f.lexkindob
                        << " obdelim=" << _f.obdelim);
          bool oksubexpr = false;
          _f.exprv = parse_expression(&_,token_deq, &oksubexpr);
          if (oksubexpr)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary gotsubexpression "
                            << _f.exprv << " startpos:" << startpos << " position:" << position_str()
                            << " token_deq:" << token_deq);
#warning TODO:  Rps_TokenSource::parse_primary should build a subexpression object...
            }
          RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary with leftparen " << _f.obdelim
                       << " token_deq:" << token_deq
                       << " startpos:" << startpos
                       << " position:" << position_str());
#warning TODO: Rps_TokenSource::parse_primary use rightparen _7CG9m1NXpMo01edTUl
        }
      else
        RPS_FATALOUT("Rps_TokenSource::parse_primary unexpected delimiter " << _f.obdelim
                     << " token_deq:" << token_deq
                     << " startpos:" << startpos
                     << " position:" << position_str());
#warning incomplete Rps_TokenSource::parse_primary with delimiter
    } // end if lexkindob is _2wdmxJecnFZ02VGGFK //repl_delimiter∊class
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary @@incomplete"
                << " token_deq:" << token_deq
                << " lextokv:" << _f.lextokv
                << " lexkindob:" << _f.lexkindob);
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
               << " token_deq:" << token_deq
               << " lextokv:" << _f.lextokv << std::endl
               << " ... lexkindob:" << _f.lexkindob
               << " lexvalv:" << _f.lexvalv
               << " lexgotokv:" << _f.lexgotokv
               << " position_str:" << position_str()
               << " startpos:" << startpos);
} // end Rps_TokenSource::parse_primary

bool
Rps_TokenSource::can_start_primary(Rps_CallFrame*callframe, Rps_DequVal& token_deq)
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
                << " token_deq:" << token_deq
                << " callframe:" << callframe);
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
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
Rps_TokenSource::parse_primary_complement(Rps_CallFrame*callframe, Rps_DequVal& token_deq, Rps_Value primaryexparg, bool*pokparse)
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
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary_complement start lextokv=" << _f.lextokv
                << " token_deq:" << token_deq
                << " primaryexpv:" << _f.primaryexpv
                << " position:" << startpos);
  if (pokparse)
    *pokparse = false;
#warning unimplemented Rps_TokenSource::parse_primary_complement
  RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary_complement "
               << Rps_ShowCallFrame(callframe)
               << " token_deq:" << token_deq
               << " primaryexp:" << _f.primaryexpv
               << " startpos:" << startpos);
} // end Rps_TokenSource::parse_primary_complement


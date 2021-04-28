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

/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_expression(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
{
  // a REPL expression is a sequence of disjuncts separated by ||
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_Value disjv;
                           Rps_ObjectRef lexkindob;
                           Rps_ObjectRef ordelimob;
                           Rps_ObjectRef oroperob;
                           Rps_Value lexvalv;
                );
  std::vector<Rps_Value> disjvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    // maybe token_deq is already GC-marked by caller....
    for (auto tokenv : token_deq)
      gc->mark_value(tokenv);
    // but the disjvect needs to be GC-marked
    for (auto disjv : disjvect)
      gc->mark_value(disjv);
  });
  bool ok = false;
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression lextokv=" << _f.lextokv << " position:" << position_str());
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  std::string startpos = position_str();
  _f.leftv = parse_disjunction(&_, token_deq, &ok);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  disjvect.push_back(_f.leftv);
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression testing or lextokv=" << _f.lextokv << " position:" << position_str());
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
                << "nbdisj:" << disjvect.size() << " at " << startpos);
  if (disjvect.size() > 1)
    {
      /// we make an instance:
      _f.disjv = Rps_InstanceValue(_f.oroperob, disjvect);
    }
  else
    {
      _f.disjv = disjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression gives "
                << _f.disjv);
  return _f.disjv;
} // end Rps_TokenSource::parse_expression




////////////////
/// This member function returns some disjunct which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_disjunction(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
{
  /// a disjunct is a sequence of one or more conjunct separated by && - the and operator
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_Value leftv;
                           Rps_Value rightv;
                           Rps_Value conjv;
                           Rps_ObjectRef lexkindob;
                           Rps_ObjectRef anddelimob;
                           Rps_ObjectRef andoperob;
                           Rps_Value lexvalv;
                );
  std::vector<Rps_Value> conjvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    // maybe token_deq is already GC-marked by caller....
    for (auto tokenv : token_deq)
      gc->mark_value(tokenv);
    // but the conjvect needs to be GC-marked
    for (auto conjv : conjvect)
      gc->mark_value(conjv);
  });
  bool ok = false;
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction lextokv="
                << _f.lextokv
                << " position: "
                << position_str());
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  std::string startpos = position_str();
  _f.leftv = parse_conjunction(&_, token_deq, &ok);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  conjvect.push_back(_f.leftv);
  bool again = false;
  static Rps_Id idanddelim;
  if (!idanddelim)
    idanddelim = Rps_Id("_1HsUfOkNw0W033EIW1"); // id of "and!binop"∈repl_delimiter
  static Rps_Id idandoper;
  if (!idandoper)
    idandoper = Rps_Id("_2YVmrhVcwW00120rTK"); // id of "and!binop"∈repl_binary_operator
  do
    {
      again = false;
      _f.lextokv =  lookahead_token(&_, token_deq, 0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction testing and lextokv=" << _f.lextokv << " position:" << position_str());
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == idanddelim)
        {
          (void) get_token(&_); // consume the and operator
          again = true;
          if (!_f.andoperob)
            _f.andoperob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,idandoper);
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
              RPS_WARNOUT("failed to parse conjunct at " << position_str());
              return nullptr;
            }
        }
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction andoperob=" << _f.andoperob
                << "nbconj:" << conjvect.size() << " at " << startpos);
  if (conjvect.size() > 1)
    {
      /// we make an instance:
      _f.conjv = Rps_InstanceValue(_f.andoperob, conjvect);
    }
  else
    {
      _f.conjv = conjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction gives "
                << _f.conjv);
  return _f.conjv;
} // end Rps_TokenSource::parse_disjunction





////////////////
Rps_Value
Rps_TokenSource::parse_conjunction(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
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
    for (auto tokenv : token_deq)
      gc->mark_value(tokenv);

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
Rps_TokenSource::parse_comparison(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
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
#warning unimplemented Rps_TokenSource::parse_comparison
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_comparison from " << Rps_ShowCallFrame(callframe)
               << " with token_deq=" << token_deq << " at " << position_str());
} // end Rps_TokenSource::parse_comparison


// a comparand - something on left or right side of compare operators is a sequence of terms with additive operators
Rps_Value
Rps_TokenSource::parse_comparand(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand leftv=" << _f.leftv << " startpos:" << startpos);
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
                << " lextokv:" << _f.lextokv << " lexopertokv:" << _f.lexopertokv);
#warning unimplemented Rps_TokenSource::parse_comparand
  /***
   * we probably should loop and collect all terms if they are separated by the same additive operator
   ***/
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_comparand from " << Rps_ShowCallFrame(callframe)
               << " with token_deq=" << token_deq << " at " << startpos);
} // end Rps_TokenSource::parse_comparand


Rps_Value
Rps_TokenSource::parse_factor(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
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
#warning Rps_TokenSource::parse_factor should probably use parse_primary here
  _f.leftv = parse_term(&_, token_deq, &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor leftv=" << _f.leftv << " startpos:" << startpos);
    }
  else
    {
      RPS_WARNOUT("parse_factor failed to parse left term at " << startpos);
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
                    << " with token_deq=" << token_deq << " at "
                    <<  startpos << " binoperob=" << _f.binoperob);
    }
  else
    {
    }
#warning unimplemented Rps_TokenSource::parse_factor
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_factor from " << Rps_ShowCallFrame(callframe)
               << " with token_deq=" << token_deq << " at " << startpos);
} // end Rps_TokenSource::parse_factor



/// a term is a sequence of factors with multiplicative operators
/// between them....
Rps_Value
Rps_TokenSource::parse_term(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(nullptr, callframe,
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
    for (auto tokenv : token_deq)
      gc->mark_value(tokenv);
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
                << " modbinopob: " << _f.modbinopob);
  /////
  ////////////////
  bool okleft = false;
  _f.leftv = parse_primary(&_, token_deq, &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term leftv=" << _f.leftv << " startpos:" << startpos << " token_deq:" << token_deq
                    << " pos:" << position_str());
      operandvect.push_back(_f.leftv);
    }
  else
    {
      RPS_WARNOUT("parse_term failed to parse left primary at " << startpos);
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term before lookahead_token leftv=" << _f.leftv
                << " token_deq=" << token_deq << " position_str:" << position_str());
  bool again = true;
  int loopcnt = 0;
  while (again)
    {
      loopcnt++;
      RPS_DEBUGNL_LOG(REPL, "Rps_TokenSource::parse_term **startloop @ " << position_str() << " loopcnt#" << loopcnt << "curcptr:" << curcptr());
      again = false;
      _f.lextokv = lookahead_token(&_, token_deq, 0);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term after leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                    << " with token_deq=" << token_deq << " at " <<  startpos << " loopcnt#" << loopcnt
		    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term after left"));
      _f.lexopertokv = lookahead_token(&_, token_deq, 1);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term after leftv=" << _f.leftv << " lextokv="
                    << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv
                    << " pos:" << position_str() << " startpos:" << startpos);
      usleep (250000);
      _f.lextokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term got token after leftv=" << _f.leftv << " got lextok=" << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv
                    << " @! " << position_str()
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
                        << " binoperob=" << _f.binoperob);
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
                    << " curoperob=" << _f.curoperob << " binoperob=" << _f.binoperob << " loopcnt#" << loopcnt
		    << " pos:" << position_str() << Rps_QuotedC_String(curcptr()));
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
	      if (!okright) {
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
            { /* binoperob != curoperob */
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
    } // end while (again)
#warning unimplemented Rps_TokenSource::parse_term
  /* we probably should make a term with operandvect here ... */
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_term from " << Rps_ShowCallFrame(callframe)
	       << " operandvect:" << operandvect
	       << " binoperob:" << _f.binoperob
	       << " curoperob:" << _f.curoperob
               << " with token_deq=" << token_deq << " at " << startpos
	       << "  curpos:" << position_str());
} // end Rps_TokenSource::parse_term




/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_primary(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           /*callerframe:*/callframe,
                           Rps_Value lextokv;
                           Rps_Value lexgotokv;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lexvalv;
                );
  std::string startpos = position_str();
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary lextokv=" << _f.lextokv << " position:" << startpos);
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
                << _f.lexkindob << " lexval=" << _f.lexvalv << " position:" << position_str());
  if (_f.lexkindob == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) // int
      && _f.lexvalv.is_int())
    {
      _f.lexgotokv = get_token(&_);
      if (_f.lexgotokv)
        token_deq.push_back(_f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary int " << _f.lexvalv
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary string " << _f.lexvalv
                    << " token_deq:" << token_deq
                    << " lexgotokv:" << _f.lexgotokv
                    << " at " << position_str());
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary double " << _f.lexvalv << " lexgotokv:" << _f.lexgotokv
                    << " token_deq:" << token_deq
                    << " at " << position_str());
      return _f.lexvalv;
    }
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

Rps_Value
Rps_TokenSource::parse_primary_complement(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, Rps_Value primaryexparg, bool*pokparse)
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
#warning unimplemented Rps_TokenSource::parse_primary_complement
  RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary_complement "
               << Rps_ShowCallFrame(callframe)
               << " token_deq:" << token_deq
               << " primaryexp:" << _f.primaryexpv
               << " startpos:" << startpos);
} // end Rps_TokenSource::parse_primary_complement


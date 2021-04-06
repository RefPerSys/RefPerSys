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
                           Rps_ObjectRef andob;
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
  _f.leftv = parse_disjunct(&_, token_deq, &ok);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  disjvect.push_back(_f.leftv);
  bool again = false;
  static Rps_Id idand;
  if (!idand)
    idand = Rps_Id("_8xk4sKglyzG01dloqq"); // id of "and!binop"∈repl_binary_operato
  do
    {
      again = false;
      _f.lextokv =  lookahead_token(&_, token_deq, 0);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression testing and lextokv=" << _f.lextokv << " position:" << position_str());
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == idand)
        {
          (void) get_token(&_); // consume the and operator
          again = true;
          if (!_f.andob)
            _f.andob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,idand);
        }
      else
        again = false;
      if (again)
        {
          bool okright=false;
          _f.rightv = parse_disjunct(&_, token_deq, &okright);
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression  andob=" << _f.andob
                << "nbdisj:" << disjvect.size());
  if (disjvect.size() > 1)
    {
      /// we make an instance:
      _f.disjv = Rps_InstanceValue(_f.andob, disjvect);
    }
  else
    {
      _f.disjv = disjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression gives "
                << _f.disjv);
  return _f.disjv;
} // end Rps_TokenSource::parse_expression


/// This member function returns some disjunct which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_disjunct(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
{
} // end Rps_TokenSource::parse_disjunct


Rps_Value
Rps_TokenSource::parse_conjunct(Rps_CallFrame*callframe, std::deque<Rps_Value>& token_deq, bool*pokparse)
{
} // end Rps_TokenSource::parse_conjunct


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
  _f.lextokv =  lookahead_token(&_, token_deq, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary lextokv=" << _f.lextokv << " position:" << position_str());
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
      RPS_ASSERT(_f.lexgotokv == _f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary int " << _f.lexvalv);
      if (pokparse)
        *pokparse = true;
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
           && _f.lexvalv.is_string())
    {
      _f.lexgotokv = get_token(&_);
      RPS_ASSERT(_f.lexgotokv == _f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary string " << _f.lexvalv);
      if (pokparse)
        *pokparse = true;
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
           && _f.lexvalv.is_double())
    {
      _f.lexgotokv = get_token(&_);
      RPS_ASSERT(_f.lexgotokv == _f.lexgotokv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary double " << _f.lexvalv);
      if (pokparse)
        *pokparse = true;
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
               << Rps_ShowCallFrame(&_)
               << " token_deq:" << token_deq
               << " lextokv:" << _f.lextokv << std::endl
               << " ... lexkindob:" << _f.lexkindob
               << " lexvalv:" << _f.lexvalv
               << " position_str:" << position_str());
} // end Rps_TokenSource::parse_primary


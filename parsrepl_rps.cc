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
 *      © Copyright 2019 - 2024 The Reflective Persistent System Team
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


extern "C" const char rps_parsrepl_gitid[];
const char rps_parsrepl_gitid[]= RPS_GITID;

extern "C" const char rps_parsrepl_date[];
const char rps_parsrepl_date[]= __DATE__;

extern "C" const char rps_parsrepl_shortgitid[];
const char rps_parsrepl_shortgitid[]= RPS_SHORTGITID;

/// useful only for debugging
static bool
rps_parsrepl_termvect_stammering(std::vector<Rps_Value>& termvect, int line)
{
  size_t sz = termvect.size();
  if  (sz>1 && termvect[sz-1] == termvect[sz-2])
    {
      RPS_POSSIBLE_BREAKPOINT();
      RPS_DEBUG_LOG_AT(__FILE__,line,REPL, "@termvect_stammering " << termvect << std::endl
                       << RPS_FULL_BACKTRACE_HERE(1,"rps_parsrepl_termvect_stammering"));
      RPS_POSSIBLE_BREAKPOINT();
      return true;
    }
  else
    return false;
} // end rps_parsrepl_termvect_stammering

/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_expression(Rps_CallFrame*callframe, bool*pokparse)
{
  // a REPL expression is a sequence of disjuncts separated by ||
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
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
  static long callcnt;
  long callnum= ++ callcnt;
  static std::atomic_long exprcounter;
  long exprnum = 1+ exprcounter.fetch_add(1);
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#" << exprnum << " START position:"
                << startpos << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " in:" << (*this) << std::endl
                << "… token_deq:" << toksrc_token_deq
                << " calldepth=" << rps_call_frame_depth(&_)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression start")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " looked tokrk0 lextokv="
                << _f.lextokv << std::endl << "… in:" << (*this)
                << " position:" << position_str() << " startpos:" << startpos
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                           << " failing_A at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << std::endl
                           << " token_deq:" << toksrc_token_deq);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                    << " failing_A at startpos:" << startpos
                    << " in:" << (*this) << std::endl
                    << "… position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << "… token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_A"));
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " before parse_disjunction"
                << " in:" << (*this) << std::endl
                << "… position:" << position_str()
                << " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << "… token_deq:" << toksrc_token_deq);
  _f.leftv = parse_disjunction(&_, &ok);
  if (!ok)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"
                    << exprnum << " failing_B (no-left-disjunction) at startpos:" << startpos
                    << " in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_B (noleftdisj)")
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                           << " failing_B at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << std::endl
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " got disjunction "
                << " in " << (*this) << std::endl
                << "… leftv=" << _f.leftv
                <<  " position:" << position_str()<< " startpos:" << startpos
                << std::endl
                << "… token_deq:" << toksrc_token_deq);
  disjvect.push_back(_f.leftv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << std::endl
                << "… in " << (*this)
                << " leftv=" << _f.leftv
                << " disjvect:" << disjvect << std::endl
                <<  " position:" << position_str()<< " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << "… token_deq:" << toksrc_token_deq);
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
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                        << " position:" << position_str()<< " startpos:" << startpos << std::endl
                        << " failed lookahea.tokzero"
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << std::endl
                        << "… token_deq:" << toksrc_token_deq
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " testing or lextokv=" << _f.lextokv
                    << " position:" << position_str()<< " startpos:" << startpos << " disjvect:" << disjvect
                    << std::endl << " … in:" << (*this));
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == idordelim)
        {
          (void) get_token(&_); // consume the or operator
#warning please code review Rps_TokenSource::parse_expression on consuming or
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " consumed or lextokv=" << _f.lextokv
                        << " position:" << position_str()<< " startpos:" << startpos << " disjvect:" << disjvect
                        << std::endl << " … in:" << (*this));
          again = true;
          if (!_f.oroperob)
            _f.oroperob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,idoroper);
        }
      else
        again = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " oroperob="
                    << _f.oroperob << " disjvect:" << disjvect
                    << " position:" << position_str()
                    << " startpos:" << startpos
                    << " in:" << (*this)
                    << (again?"again":"notagain"));
      if (again)
        {
          bool okright=false;
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " before parsedisjright disjvect:" << disjvect
                        << " position:" << position_str()
                        << " startpos:" << startpos
                        << "  in:" << (*this));
          _f.rightv = parse_disjunction(&_, &okright);
          if (okright)
            {
              disjvect.push_back(_f.rightv);
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                            << " after parsedisjright rightv=" << _f.rightv
                            << " disjvect:" << disjvect
                            << " position:" << position_str()
                            << " startpos:" << startpos
                            << "  in:" << (*this)
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              }));
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                            << " failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_C"));
              RPS_PARSREPL_FAILURE(&_,
                                   "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                                   << " failing_C at startpos:" << startpos
                                   << " in:" << (*this)
                                   << " position:" << position_str()
                                   << " curcptr:" << Rps_QuotedC_String(curcptr())
                                   << std::endl
                                   << " token_deq:" << toksrc_token_deq);
              RPS_WARNOUT("parse_expression failed to parse disjunct at " << position_str()
                          << " startpos:" << startpos);
              return nullptr;
            }
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " startpos:" << startpos
                    << (again?"looping":"stop-loop")
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression/endloop"));
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                << " oroperob=" << _f.oroperob
                << "nbdisj:" << disjvect.size()
                << std::endl
                << "…token_deq:" << toksrc_token_deq
                << " position:" << position_str()
                << " startpos:" << startpos
                << " disjvect:" << disjvect
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (disjvect.empty())
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " failing_empty at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_expression failing_empty")
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum
                           << " failing_empty at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << std::endl
                           << " token_deq:" << toksrc_token_deq);
      RPS_WARNOUT("parse_expression failed to parse disjunct (none) at " << position_str()
                  << " startpos:" << startpos);
      return nullptr;
    }
  else if (disjvect.size() > 1)
    {
      /// we make an instance:
      _f.resexprv = Rps_InstanceValue(_f.oroperob, disjvect);
    }
  else
    {
      _f.resexprv = disjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_expression¤" << callnum << "#"  << exprnum << " result: "
                << _f.resexprv << " position:" << position_str()<< " startpos:" << startpos
                << std::endl
                << " token_deq:" << toksrc_token_deq
                << " calldepth=" << rps_call_frame_depth(&_) << std::endl
                << " in:" << (*this));
  if (pokparse)
    *pokparse = true;
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
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
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
  static long callcnt;
  long callnum= ++ callcnt;
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " START startpos:"
                << startpos << " calldepth#" << rps_call_frame_depth(&_)
                << "  in:" << (*this) << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " lextokv=" << _f.lextokv
                << " position: " << position_str() << std::endl << "… startpos=" << startpos
                << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction failing_A"));

      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_disjunction¤" << callnum << " failing_A at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << std::endl
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  // now _f.lextokv is non-null and is token0
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " before parse_conjun left at startpos:" << startpos
                << "  in:" << (*this) << std::endl
                << "… lextokv=" << _f.lextokv
                << " position:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  ok = false;
  _f.leftv = parse_conjunction(&_,  &ok);
  if (ok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " after left parse_conjunction leftv=" << _f.leftv
                    << " at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction/after-leftconj")
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
  else
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " failing_B (noleftconj) at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " leftv:" << _f.leftv << " should be null"
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction failing_B/noleftconj"));
      RPS_ASSERT (_f.leftv == nullptr);
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_disjunction¤" << callnum << " failing_B at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << std::endl
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  conjvect.push_back(_f.leftv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " leftv=" << _f.leftv
                << " position: " << position_str() << " startpos:" << startpos
                << " conjvect:" << conjvect);
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " looping position:" << position_str() << " startpos:" << startpos
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      _f.lextokv =  lookahead_token(&_,  0);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " in loop position:" << position_str() << " startpos:" << startpos
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq << " lextok:" << _f.lextokv);
      if (!_f.lextokv)
        {
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << "  testing and lextokv="
                    << _f.lextokv << " position:" << position_str() << " startpos:" << startpos
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq);
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << (again?"again":"done")
                    << " position:" << position_str() << " startpos:" << startpos
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " conjvect:" << conjvect
                    << " in " << (*this));
      if (again)
        {
          bool okright=false;
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " before parse_conjonction lextokv="
                        << _f.lextokv << " position:" << position_str() << " startpos:" << startpos
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " leftv:" << _f.leftv);
          _f.rightv = parse_conjunction(&_,  &okright);
          if (okright)
            {
              conjvect.push_back(_f.rightv);
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " parsed right conjunct rightv="
                            << _f.rightv << std::endl << "… startpos:" << startpos
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << " conjvect:" << conjvect << std::endl
                            << "… position:" << position_str() << " in " << (*this)
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              }));
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_disjunction failing_C"));
              RPS_PARSREPL_FAILURE(&_,
                                   "Rps_TokenSource::parse_disjunction¤" << callnum << " failing_C at startpos:" << startpos
                                   << " in:" << (*this)
                                   << " position:" << position_str()
                                   << " curcptr:" << Rps_QuotedC_String(curcptr())
                                   << std::endl
                                   << " token_deq:" << toksrc_token_deq);
              RPS_WARNOUT("Rps_TokenSource::parse_disjunction¤" << callnum << " failed to parse conjunct at " << position_str() << " starting " << startpos);
              return nullptr;
            }
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " position:" << position_str() << " startpos:" << startpos << "endloop "
                    << (again?"again":"STOP")
                    << " calldepth#" << rps_call_frame_depth(&_));
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum << " oroperob=" << _f.oroperob
                << "nbconj:" << conjvect.size()  << " position:" << position_str() << " startpos:" << startpos
                << " calldepth#" << rps_call_frame_depth(&_)
                << " conjvect:" << conjvect);
  if (conjvect.size() > 1)
    {
      /// we make an instance:
      _f.resdisjv = Rps_InstanceValue(_f.oroperob, conjvect);
    }
  else
    {
      _f.resdisjv = conjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_disjunction¤" << callnum <<" result "
                << _f.resdisjv << " position:" << position_str()
                << " startpos:" << startpos
                << " calldepth#" << rps_call_frame_depth(&_)
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (pokparse)
    *pokparse = true;
  return _f.resdisjv;
} // end Rps_TokenSource::parse_disjunction





////////////////
Rps_Value
Rps_TokenSource::parse_conjunction(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
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
  static long callcnt;
  long callnum= ++ callcnt;
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " START at " << startpos
                << "  in:" << (*this) << std::endl
                << "… token_deq:" << toksrc_token_deq
                << " curcptr:" << curcptr()
                << " calldepth=" << rps_call_frame_depth(&_) << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction/start") << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " lextokv="  << _f.lextokv
                << " position: " << position_str() << std::endl
                << "… startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " anddelim:" << _f.anddelimob
                << " andbinop:" << _f.andbinopob
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing_A"));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_conjunction¤" << callnum << " failing_A at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  //
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " before parsing left comparison in:" << (*this)
                << std::endl
                << "… position:" << position_str()
                << " lextokv:" << _f.lextokv
                << " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr()));
  _f.leftv = parse_comparison(&_, &ok);
  if (ok)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " got left comparison " << _f.leftv
                    << " position:" << position_str()
                    << " startpos:" << startpos
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));

    }
  else
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " failing_B (noleftcompar) at startpos:"
                    << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing_B/noleftcompar"));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_conjunction¤" << callnum << " failing_B at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  ///

  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum
                << " conjvect:" << conjvect << " leftv:" << _f.leftv
                << " in:" << (*this)
                << " position:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  conjvect.push_back(_f.leftv);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum
                << " grown conjvect:" << conjvect << " leftv:" << _f.leftv
                << " in:" << (*this)
                << " position:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);

  bool again = false;
  do
    {
      again = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " looping conjvect:" << conjvect
                    << " at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      _f.lextokv =  lookahead_token(&_,  0);
      if (!_f.lextokv)
        {
          //end of input!
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " EOI at startpos:" << startpos
                        << "  in:" << (*this)
                        << " position:" << position_str() << std::endl
                        << ".. curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq
                        << " conjvect:" << conjvect);
          again = false;
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " testing or lextokv=" << _f.lextokv
                    << " position:" << position_str());
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " conjvect:" << conjvect
                    << " at startpos:" << startpos
                    << "  in:" << (*this)
                    << " curcptr:" << Rps_QuotedC_String(curcptr()));
      if (_f.lextokv.is_lextoken()
          && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lextokv.to_lextoken()->lxval().is_object()
          &&  _f.lextokv.to_lextoken()->lxval().to_object()->oid() == id_and_delim)
        {
          (void) get_token(&_); // consume the and operator
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " startpos:" << startpos
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
            {
              conjvect.push_back(_f.rightv);
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤"
                            << callnum << " rightv:" << _f.rightv
                            << " at startpos:" << startpos << std::endl
                            << "…  in:" << (*this)
                            << " conjvect:" << conjvect
                            << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              }));

            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤"
                            << callnum << " failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              })
                  << std::endl << " position:" << position_str()
                  << " curcptr:" << Rps_QuotedC_String(curcptr())
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing_C"));
              //
              RPS_PARSREPL_FAILURE(&_,
                                   "Rps_TokenSource::parse_conjunction¤"
                                   << callnum << " failing_C at startpos:" << startpos
                                   << " in:" << (*this)
                                   << " position:" << position_str()
                                   << " curcptr:" << Rps_QuotedC_String(curcptr())
                                   << " token_deq:" << toksrc_token_deq);
              RPS_WARNOUT("failed to parse conjunct at " << position_str()
                          << " in:" << (*this)
                          << " position:" << position_str()
                          << " curcptr:" << Rps_QuotedC_String(curcptr())
                          << " token_deq:" << toksrc_token_deq);
              return nullptr;
            }
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum
                    << " startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << (again?"looping":"stoploop"));
    }
  while (again);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum
                << " ENDLOOP andbinopob=" << _f.andbinopob
                << "nbconj:" << conjvect.size() << " at " << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq
                << " conjvect:" << conjvect);
  if (conjvect.empty())
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " failing noconjunct at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_conjunction failing noconjunct"));

      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_conjunction¤" << callnum << " failing noconjunct at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      RPS_WARNOUT("failed to parse conjunct at " << position_str());
      return nullptr;
    }
  if (conjvect.size() > 1)
    {
      /// we make an instance:
      _f.conjv = Rps_InstanceValue(_f.andbinopob, conjvect);
    }
  else
    {
      _f.conjv = conjvect[0];
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_conjunction¤" << callnum << " GIVES "
                << _f.conjv << " at " << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << " token_deq:" << toksrc_token_deq << std::endl);
  if (pokparse)
    *pokparse = true;
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
  static long callcnt;
  long callnum= ++ callcnt;
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << " BEGIN at " << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << " startpos:" << startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  bool okleft = false;
  _f.leftv = parse_comparand(&_,  &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << " leftv=" << _f.leftv << " startpos:" << startpos
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << " failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison failing_A"));

      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_comparison¤" << callnum << "  failing_A at startpos:" << startpos
                           << "  in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);

      RPS_WARNOUT("parse_comparison¤" << callnum << "  failed to parse left comparand at " << startpos
                  << " current line:" << Rps_QuotedC_String(current_line())
                  << " curpos " << position_str()
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison fail"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << "  after left lextok=" << _f.lextokv << " pos:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << " GIVES leftv:" << _f.leftv
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq);
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparison¤" << callnum << "  testing lextokv="
                << _f.lextokv << " position:" << position_str());
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object())
    {
      Rps_Id delimid =  _f.lextokv.to_lextoken()->lxval().to_object()->oid();
      RPS_WARNOUT("Rps_TokenSource::parse_comparison¤" << callnum << "  incomplete delimid=" << delimid  << " position:" << position_str()
                  << " startpos:" << startpos
                  << std::endl << " leftv=" << _f.leftv << " lextokv=" << _f.lextokv);
    }
  else
    {
      RPS_WARNOUT("Rps_TokenSource::parse_comparison¤" << callnum << "  unexpected lextokv="
                  << _f.lextokv << " position:" << position_str()
                  << " startpos:" << startpos
                  << std::endl << " leftv=" << _f.leftv
                  << " token_deq:" << toksrc_token_deq);
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_comparison¤" << callnum << "  failing_XX at startpos:" << startpos
                           << "  in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);

    }
#warning unimplemented Rps_TokenSource::parse_comparison
  RPS_PARSREPL_FAILURE(&_,
                       "Rps_TokenSource::parse_comparison¤" << callnum << " failing_final at startpos:" << startpos
                       << "  in:" << (*this)
                       << " position:" << position_str()
                       << "… leftv=" << _f.leftv
                       << " curcptr:" << Rps_QuotedC_String(curcptr())
                       << " token_deq:" << toksrc_token_deq
                       << std::endl
                       << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_comparison¤" << callnum << " from " << Rps_ShowCallFrame(callframe)
               << " in:" << (*this) << " at " << position_str()
               << " startpos:" << startpos
               << std::endl
               << "… leftv=" << _f.leftv << " lextokv=" << _f.lextokv);
} // end Rps_TokenSource::parse_comparison




////////////////////////////////////////////////////////////////

Rps_Value
Rps_TokenSource::parse_sum(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(nullptr, callframe,
                 Rps_Value lextokv;
                 Rps_Value lexopertokv;
                 Rps_Value lexgotokv;
                 Rps_Value leftv;
                 Rps_Value rightv;
                 Rps_ObjectRef plusdelimob;
                 Rps_ObjectRef plusbinopob;
                 Rps_ObjectRef minusdelimob;
                 Rps_ObjectRef minusbinopob;
                 Rps_ObjectRef delimob;
                 Rps_ObjectRef pastdelimob;
                );
  std::vector<Rps_Value> termvect;
  _.set_additional_gc_marker([&](Rps_GarbageCollector* gc)
  {
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_sum ?
    for (auto termv : termvect)
      gc->mark_value(termv);
  });
  std::string startpos = position_str();
  static long callcnt;
  long callnum= ++ callcnt;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                << " START from " << Rps_ShowCallFrame(&_)
                << " in " << (*this) << " at " <<  startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << " termvect:" << termvect << std::endl
      << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*")
      << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_sum/START")
               );
  bool okleft = false;
  _f.leftv = parse_term(&_, &okleft);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                << " startpos:" << startpos << std::endl
                << " after parse_term leftv=" << _f.leftv
                << (okleft?" okleft":" NOTOKleft")
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }) << " termvect:" << termvect << " "
     << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*"));
  if (!okleft)
    {
      if (pokparse)
        *pokparse = false;
      return nullptr;
    };
  /// + delimiter and binary operator
  static Rps_Id id_plus_delim;
  if (!id_plus_delim)
    id_plus_delim = Rps_Id("_4ShDsOWk7al02eDRTM");
  static Rps_Id id_plus_binop;
  if (!id_plus_binop)
    id_plus_binop = Rps_Id("_51jvc2mFhql03qwRg6");
  _f.plusdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_plus_delim); // "plus!delim"∈repl_delimiter
  _f.plusbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_plus_binop); // "plus!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                << " plusdelimob=" << _f.plusdelimob << std::endl
                << "… plusbinopob=" << _f.plusbinopob
                << " in " << (*this) << std::endl
                << "… leftv=" << _f.leftv
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << "… token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  /// - delimiter and binary operator
  static Rps_Id id_minus_delim;
  if (!id_minus_delim)
    id_minus_delim = Rps_Id("_3isfNUOsxXY02Bww6l");
  static Rps_Id id_minus_binop;
  if (!id_minus_binop)
    id_minus_binop = Rps_Id("_13ffyyJhGHI01ySLhK");
  _f.minusdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_minus_delim); // "minus!delim"∈repl_delimiter
  _f.minusbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_minus_binop); // "minus!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                << "@missing minusdelimob=" << _f.minusdelimob << std::endl
                << " minusbinopob=" << _f.minusbinopob
                << " leftv=" << _f.leftv
                << std::endl
                << "… curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  /// simple case for test01 in commit  e23928170e (oct.7, 2023)
  if (!curcptr() && toksrc_token_deq.empty())
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum << " in:" << (*this)
                    << "simple-case-test01/e23928170e gives leftv="
                    << _f.leftv);

      if (pokparse)
        *pokparse = true;
      return _f.leftv;
    };
  _f.lextokv =  lookahead_token(&_, 0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum << " in:" << (*this)
                << " leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                << std::endl
                << "… curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  bool again = false;
  if (_f.lextokv.is_lextoken()
      && _f.lextokv.to_lextoken()->lxkind() == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
      &&  _f.lextokv.to_lextoken()->lxval().is_object())
    {
      _f.delimob =  _f.lextokv.to_lextoken()->lxval().as_object();
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                    << " in:" << (*this)
                    << " leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                    << std::endl
                    << "… delimob=" << _f.delimob
                    << std::endl
                    << "… curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }) << " termvect:" << termvect << " "
         << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*"));
      if (_f.delimob ==  _f.plusdelimob || _f.delimob == _f.minusdelimob)
        {
          if (!_f.pastdelimob)
            _f.pastdelimob = _f.delimob;
          again = (_f.delimob== _f.pastdelimob);
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum << " in:" << (*this)
                        << " leftv=" << _f.leftv
                        << " lextokv=" << _f.lextokv << std::endl
                        << "… delimob=" << _f.delimob
                        << " pastdelimob=" << _f.pastdelimob << (again?"again":"stop")
                        << std::endl
                        << "… curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq << std::endl);
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum <<
                    " termvect:" << termvect << " "
                    << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*") << " before loop "
                    <<  (again?"again":"stop")
                    << std::endl
                    << "… curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      int loopcnt = 0;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                    << " termvect:" << termvect<< " "
                    << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*")
                    << " leftv=" << _f.leftv
                    << " lextokv=" << _f.lextokv
                    << " beforeloop again=" << again
                    << std::endl
                    << "… curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      ///////////
      RPS_DEBUGNL_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                      << " BEFORELOOP loopcnt#" << loopcnt
                      << " termvect:" << termvect<< " "
                      << " leftv:" << _f.leftv
                      << std::endl
                      << "… lextokv=" << _f.lextokv
                      << " delimob=" << _f.delimob
                      << " pastdelimob=" << _f.pastdelimob << (again?", again":"; stop")
                      << std::endl
                      << "… curcptr:" << Rps_QuotedC_String(curcptr())
                      << (again?"again":"NOTAGAIN")
                      << " token_deq:" << toksrc_token_deq << std::endl);
      ///////
      while (again)
        {
          loopcnt++;
          RPS_DEBUGNL_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                          << " BEGINLOOP loopcnt#" << loopcnt
                          << " termvect:" << termvect<< " "
                          << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*")
                          << " leftv:" << _f.leftv
                          << std::endl
                          << "… lextokv=" << _f.lextokv
                          << " delimob=" << _f.delimob
                          << " pastdelimob=" << _f.pastdelimob << (again?", again":"; stop")
                          << std::endl
                          << "… curcptr:" << Rps_QuotedC_String(curcptr())
                          << " token_deq:" << toksrc_token_deq << std::endl
                          << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          if (!curcptr())
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                            << " STOPLOOP loopcnt#" << loopcnt);
              break;
            };
          //////////////// for test01c bug in mid october 2024, e.g. commit 8aaec2fc59cb
#if 1       // once test01c is good make that #if 0
          /// temporary code to catch make test01c bug in commit e78fd8f9b3 at mid Oct. 2024
          if (termvect.size()>0 && termvect[termvect.size()-1] == _f.leftv)
            {
              RPS_POSSIBLE_BREAKPOINT();
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                            << " loopcnt#" << loopcnt << " *Test01c-bug*"
                            << " in:" << (*this)
                            << " thread:" << rps_current_pthread_name()
                            << std::endl
                            << "… curcptr:" << Rps_QuotedC_String(curcptr())
                            << " token_deq:" << toksrc_token_deq
                            << " termvect=" << termvect << " is ending with leftv="
                            << _f.leftv << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1,"Rps_TokenSource::parse_sum/Test01c-bug"));
              RPS_POSSIBLE_BREAKPOINT();
            };
          /// end of temporary code to catch make test01c bug in commit e78fd8f9b3 at mid Oct. 2024
#endif /*for test01c*/
          ////////////////
          termvect.push_back(_f.leftv);
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                        << " loopcnt#" << loopcnt
                        << " in:" << (*this)
                        << " termvect:" << termvect << " "
                        << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*") << std::endl
                        << "… leftv=" << _f.leftv
                        << " token_deq:" << toksrc_token_deq << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          bool succeeded=false;
          consume_front_token(&_, &succeeded);
          /// following two lines added in Sept 2024 near 2759775144
          _f.leftv = nullptr;
          _f.rightv = nullptr;
          if (!succeeded)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                            << " loopcnt#" << loopcnt
                            << " in:" << (*this)
                            << " termvect:" << termvect << " "
                            << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*") << std::endl
                            << "… curcptr:" << Rps_QuotedC_String(curcptr())
                            << " failed-consume:"  << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              }));
              break;
            }
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                        << " loopcnt#" << loopcnt
                        << " in:" << (*this)
                        << " termvect:" << termvect  << " "
                        << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*")<< std::endl
                        << "… curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          _f.lextokv =  lookahead_token(&_, 0);
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤"
                        << callnum << " loopcnt#" << loopcnt
                        << " in:" << (*this)
                        << " termvect:" << termvect << " "
                        << (rps_parsrepl_termvect_stammering (termvect, __LINE__)?"!st!²":"!st!*") << std::endl
                        << "… leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                        << " delimob=" << _f.delimob
                        << " pastdelimob=" << _f.pastdelimob
                        << " lextokv=" << _f.lextokv
                        << "… curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          okleft = false;
          _f.leftv = parse_term(&_, &okleft);
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                        << " loopcnt#" << loopcnt
                        << " in:" << (*this) << " termvect=" << termvect  << std::endl
                        << "… leftv=" << _f.leftv << " lextokv=" << _f.lextokv << std::endl
                        << "… delimob=" << _f.delimob
                        << " pastdelimob=" << _f.pastdelimob
                        << " lextokv=" << _f.lextokv
                        << std::endl
                        << "… curcptr:" << Rps_QuotedC_String(curcptr()) << " leftv=" << _f.leftv
                        << (okleft?" OKleft":" NOTOKleft")
                        << " token_deq:" << toksrc_token_deq << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          if (!okleft)
            again = false;
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                            << " loopcnt#" << loopcnt
                            << " pushing leftv:" << _f.leftv
                            << " with rightv=" << _f.rightv
                            << " to termvect:"<< termvect
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1,
                                "Rps_TokenSource::parse_sum/pushleft"));
              termvect.push_back(_f.leftv);
              again = true;
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_sum¤" << callnum
                            << " loopcnt#" << loopcnt
                            << " in:" << (*this) << std::endl
                            << "… leftv=" << _f.leftv << " lextokv=" << _f.lextokv << std::endl
                            << "… delimob=" << _f.delimob << std::endl
                            << "… pushed to termvect=" << termvect
                            << " pastdelimob=" << _f.pastdelimob
                            << " lextokv=" << _f.lextokv
                            << std::endl
                            << "… curcptr:" << Rps_QuotedC_String(curcptr())
                            << " continuing-loop token_deq:" << toksrc_token_deq << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              })
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_sum/p"));
              continue;
            };
          //// reached when okleft is false (see previous parse_term)....
          RPS_FATALOUT("missing code in Rps_TokenSource::parse_sum¤" << callnum
                       << " loopcnt#" << loopcnt
                       << " from " << Rps_ShowCallFrame(callframe)
                       << " in:" << (*this) << " at " << position_str()<< std::endl
                       << "… startpos:" << startpos << " token_deq:" << toksrc_token_deq
                       << " curcptr:" << Rps_QuotedC_String(curcptr())
                       << std::endl
                       << "… leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                       << std::endl << "…  termvect=" << termvect
                       << (again?" again":" notagain") << " delimob=" << _f.delimob
                       << " pastdelimob=" << _f.pastdelimob
                       << std::endl
                       << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
#warning incomplete Rps_TokenSource::parse_sum
        };      // end while again
      ////////////////////////////
      /////////
      RPS_DEBUG_LOG (REPL, "Rps_TokenSource::parse_sum¤" << callnum << " loopcnt#" << loopcnt
                     << " endloop in:" << (*this)
                     << std::endl
                     << "… at " << position_str()<< std::endl
                     <<  "… termvect=" << termvect
                     << " pastdelimob=" << _f.pastdelimob
                     << " rightv=" << _f.rightv
                     << std::endl
                     << "… plusdelimob=" << _f.plusdelimob
                     << " plusbinopob=" << _f.plusbinopob
                     << std::endl
                     << "… minusdelimob=" << _f.minusdelimob
                     << " minusbinopob=" << _f.minusbinopob
                     << " delimob=" << _f.delimob
                     << std::endl
                     << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      })
          << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_sum/q"));
    };        // end if
  /***
   * We probably should loop and collect all terms if they are
   * separated by the same additive delimiter with its operator.
   *
   * Near commit 95d44aa788cf1 (dec. 2022) we should probably improve
   * our plugins/rpsplug_createcommutativeoperator.cc to create more
   * classes, new operators and delimiters, and get attribute from
   * them, etc…
   *
   * See https://framalistes.org/sympa/arc/refpersys-forum/2022-12/msg00069.html
   ***/
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_sum¤" << callnum << " from " << Rps_ShowCallFrame(callframe) << std::endl
               << "… in:" << (*this) << " at " << position_str()<< std::endl
               << "… startpos:" << startpos << " token_deq:" << toksrc_token_deq
               << " curcptr:" << Rps_QuotedC_String(curcptr())
               << std::endl
               << "… leftv=" << _f.leftv << " lextokv=" << _f.lextokv
               << " rightv=" << _f.rightv
               << std::endl
               << "… plusdelimob=" << _f.plusdelimob
               << " plusbinopob=" << _f.plusbinopob
               << std::endl
               << "… termvect=" << termvect << std::endl
               << "… minusdelimob=" << _f.minusdelimob
               << " minusbinopob=" << _f.minusbinopob
               << std::endl
               << "…  delimob=" << _f.delimob
               << std::endl
               << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
} // end Rps_TokenSource::parse_sum


// a comparand - something on left or right side of compare operators such as < or !=
// is a sequence of terms with additive operators
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
  static long callcnt;
  long callnum= ++ callcnt;
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " START from " << Rps_ShowCallFrame(&_)
                << " in " << (*this) << " at " <<  startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  bool okleft = false;
  _f.leftv = parse_sum(&_,  &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " leftv=" << _f.leftv << " startpos:" << startpos
                    << " currentpos:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
  else
    {
      RPS_WARNOUT("parse_comparand¤" << callnum << " failed to parse left comparand at " << startpos
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparand fail"));
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()) << " token_deq:" <<  toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparison failing_A"));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_comparand¤" << callnum << " failing_A at startpos:" << startpos
                           << "  in:" << (*this)
                           << " position:" << position_str()
                           << "… leftv=" << _f.leftv
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  //// Here _f.leftv has been parsed
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " lextokv°0: " << _f.lextokv
                << " leftv=" << _f.leftv
                << " position:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  _f.lexopertokv =  lookahead_token(&_,  1);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << "  leftv=" << _f.leftv
                << " lextokv°0:" << _f.lextokv
                << "… lexopertokv°~:" << _f.lexopertokv
                << std::endl
                << "… startpos:" << startpos
                << " currentpos:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  if (!_f.lexopertokv)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " no lexopertokv  leftv=" << _f.leftv
                    << " lextokv:" << _f.lextokv
                    << " startpos:" << startpos
                    << " currentpos:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (!toksrc_token_deq.empty())
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " no lexopertokv  leftv=" << _f.leftv
                        << " consume front token"
                        << " lextokv:" << _f.lextokv
                        << " startpos:" << startpos
                        << " currentpos:" << position_str()
                        << " curcptr " << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          consume_front_token(&_);
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " no lexopertokv  leftv=" << _f.leftv
                    << " lextokv:" << _f.lextokv
                    << " startpos:" << startpos
                    << " currentpos:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " ending gives " << _f.leftv
                    << " startpos:" << startpos
                    << " currentpos:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq);
      return _f.leftv;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_comparand¤" << callnum << " *INCOMPLETE* leftv=" << _f.leftv
                << " lextokv:" << _f.lextokv
                << std::endl
                << "… lexopertokv-at1:" << _f.lexopertokv
                << " startpos:" << startpos
                << " currentpos:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << std::endl
      << RPS_FULL_BACKTRACE_HERE(1, "incomplete Rps_TokenSource::parse_comparand"));
#warning TODO: missing code in Rps_TokenSource::parse_comparand
  RPS_PARSREPL_FAILURE(&_,
                       "Rps_TokenSource::parse_comparand¤" << callnum << " missing code at startpos:" << startpos<<
                       " leftv=" << _f.leftv
                       << " lextokv:" << _f.lextokv
                       << " in:" << (*this) << std::endl
                       << "…. position:" << position_str()
                       << " curcptr:" << Rps_QuotedC_String(curcptr()) << std::endl
                       << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << std::endl
      << " token_deq:" << toksrc_token_deq
      << " lextokv=" << _f.lextokv);
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_comparand¤" << callnum
               << std::endl
               //<< "… from " << std::endl << Rps_ShowCallFrame(callframe) << std::endl
               << "… parse_comparand in:" << (*this)
               << std::endl << " at startpos: " << startpos
               << " currentpos:" << position_str()
               << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
               << std::endl
               << "… lextokv=" << _f.lextokv << " lexopertokv=" << _f.lexopertokv << std::endl
               << "… parse_comparand¤" << callnum << " leftv=" << _f.leftv
               << std::endl
               << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << std::endl
      << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_comparand incomplete"));
} // end Rps_TokenSource::parse_comparand



Rps_Value
Rps_TokenSource::parse_factor(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  static long callcnt;
  long callnum= ++ callcnt;
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " START startpos:" << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  /// + delimiter and binary operator
  static Rps_Id id_plus_delim;
  if (!id_plus_delim)
    id_plus_delim = Rps_Id("_4ShDsOWk7al02eDRTM");
  static Rps_Id id_plus_binop;
  if (!id_plus_binop)
    id_plus_binop = Rps_Id("_51jvc2mFhql03qwRg6");
  _f.plusdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_plus_delim); // "plus!delim"∈repl_delimiter
  _f.plusbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_plus_binop); // "plus!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " plusdelimob=" << _f.plusdelimob << " plusbinopob=" << _f.plusbinopob);
  /// - delimiter and binary operator
  static Rps_Id id_minus_delim;
  if (!id_minus_delim)
    id_minus_delim = Rps_Id("_3isfNUOsxXY02Bww6l");
  static Rps_Id id_minus_binop;
  if (!id_minus_binop)
    id_minus_binop = Rps_Id("_13ffyyJhGHI01ySLhK");
  _f.minusdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_minus_delim); // "minus!delim"∈repl_delimiter
  _f.minusbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_minus_binop); // "minus!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " minusdelimob=" << _f.minusdelimob << " minusbinopob=" << _f.minusbinopob);
  ///
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " started from " << Rps_ShowCallFrame(&_)
                << " in " << (*this)<< " at " <<  startpos
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  bool okleft = false;
  _f.leftv = parse_primary(&_,  &okleft);
  if (okleft)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " got primary leftv=" << _f.leftv
                    << " startpos:" << startpos << " at " << position_str() << " in " << (*this)
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor failing_A"));
      RPS_WARNOUT("Rps_TokenSource::parse_factor¤" << callnum << " failed to parse left primary at " << position_str()
                  << " startpos:" << startpos << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor fail/left"));
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " lextokv-at0=" << _f.lextokv
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " lextokv=" << _f.lextokv
                << " in:" << (*this) << " at " <<  startpos << " binoperob=" << _f.binoperob
                << " position:" << position_str()
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " token_deq:" << toksrc_token_deq);
  if (_f.binoperob)
    {
      bool okright = false;
      /// consume the + or - delimiter…
      _f.lexgotokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " lexgotokv=" << _f.lexgotokv
                    << " in:" << (*this) << " at "
                    <<  startpos << " binoperob=" << _f.binoperob
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      RPS_ASSERT(_f.lexgotokv  == _f.lextokv);
      _f.rightv = parse_term(&_, &okright);
      if (!okright)
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " FAIL right-term lexgotokv=" << _f.lexgotokv
                        << " in:" << (*this) << " at "
                        <<  startpos << " binoperob=" << _f.binoperob
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq);
          RPS_PARSREPL_FAILURE(&_,
                               "Rps_TokenSource::parse_factor¤" << callnum << " FAIL right-term at startpos:" << startpos
                               << " in:" << (*this)
                               << " position:" << position_str()
                               << " curcptr:" << Rps_QuotedC_String(curcptr())
                               << " token_deq:" << toksrc_token_deq);
          RPS_WARNOUT("parse_factor failed to parse right term starting " << startpos << " at " << position_str());
          if (pokparse)
            *pokparse = false;
          return nullptr;
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " leftv=" << _f.leftv
                    << " rightv=" << _f.rightv
                    << " in:" << (*this) << " at " << position_str()
                    << " startpos:" <<  startpos << " binoperob=" << _f.binoperob
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq);
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_factor¤" << callnum << " leftv=" << _f.leftv
                    << " in:" << (*this) << " at "
                    <<  position_str() << " startpos:" << startpos
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
#warning incomplete Rps_TokenSource::parse_factor
  RPS_FATALOUT("missing code in Rps_TokenSource::parse_factor¤" << callnum << " from " << Rps_ShowCallFrame(callframe)
               << " in:" << (*this) << " at " << position_str()  << " startpos:" << startpos
               << " curcptr:" << Rps_QuotedC_String(curcptr())
               << " token_deq:" << toksrc_token_deq
               << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_factor incomplete"));
} // end Rps_TokenSource::parse_factor



/// a term is a sequence of factors with multiplicative operators
/// between them…. All the operators should be the same. Otherwise we build intermediate subexpressions
Rps_Value
Rps_TokenSource::parse_term(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  static long callcnt;
  long callnum= ++ callcnt;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
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
    // maybe token_deq is already GC-marked by caller….
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_term ?
    // but the operandvect needs to be GC-marked
    for (auto operv : operandvect)
      gc->mark_value(operv);
  });
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum
                << " BEGIN startpos:"
                << startpos << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << std::endl);
  /// multiplication operator and * delim
  static Rps_Id id_mult_delim;
  if (!id_mult_delim)
    id_mult_delim = Rps_Id("_2uw3Se5dPOU00yhxpA"); // id of "mult!delim"∈repl_delimiter
  static Rps_Id id_mult_oper;
  if (!id_mult_oper)
    id_mult_oper = Rps_Id("_4QX7Cg3gDkd005b9bn"); // id of "mult!binop"∈repl_binary_operator
  _f.multdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_delim); // "mult!delim"∈repl_delimiter
  _f.multbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_oper); // "mult!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " startpos:" << startpos << " multdelimob:" << _f.multdelimob
                << " multbinopob: " << _f.multbinopob
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  /// division operator and / delim
  static Rps_Id id_div_delim;
  if (!id_div_delim)
    id_div_delim = Rps_Id("_3ak80l3pr9700M90pz"); // id of "div!delim"∈repl_delimiter
  static Rps_Id id_div_oper;
  if (!id_div_oper)
    id_div_oper = Rps_Id("_0GTVGelTnCP01I0od2"); // id of "div!binop"∈repl_binary_operator
  _f.divdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_delim); // "div!delim"∈repl_delimiter
  _f.divbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_oper); // "div!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum
                << " START startpos:" << startpos << " divdelimob:" << _f.divdelimob
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << "  startpos:" << startpos << " moddelimob:" << _f.moddelimob
                << " modbinopob: " << _f.modbinopob
                << " pos:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  /////
  ////////////////
  bool okleft = false;
  _f.leftv = parse_primary(&_, &okleft);
  if (okleft)
    {
      operandvect.push_back(_f.leftv);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum
                    << " did parseprimary leftv=" << _f.leftv
                    << " startpos:" << startpos << "  in:" << (*this)
                    << " operandvect:" << operandvect << std::endl
                    << ".. pos:" << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    }
  else
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum
                    << " failing_A (left primary nonok) at startpos:" << startpos
                    << "  in:" << (*this)
                    << " leftv:" << _f.leftv
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term failing_A")
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_term¤" << callnum << " fail left prim at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      RPS_WARNOUT("parse_term failed to parse left primary starting " << startpos
                  << " pos:" << position_str()
                  << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
      if (pokparse)
        *pokparse = false;
      return nullptr;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum
                << " before loop leftv=" << _f.leftv
                << " in:" << (*this) << " position_str:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)(curcptr())
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term/bef.loop"));
  bool again = true;
  int loopcnt = 0;
  while (again)
    {
      loopcnt++;
      RPS_DEBUGNL_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " **startloop @ " << position_str()
                      << " loopcnt#" << loopcnt
                      << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                      << " token_deq:" << toksrc_token_deq
                      << std::endl
                      << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      again = false;
      _f.lextokv = lookahead_token(&_,  0);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " after leftv=" << _f.leftv << " lextokv=" << _f.lextokv
                    << " in:" << (*this) << " at " <<  startpos << " loopcnt#" << loopcnt
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term after left"));
      _f.lexopertokv = lookahead_token(&_,  1);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " after leftv=" << _f.leftv << " lextokv="
                    << _f.lextokv
                    << " lexopertokv=" << _f.lexopertokv
                    << " pos:" << position_str() << " startpos:" << startpos
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      usleep (250000);    // for debugging
      RPS_POSSIBLE_BREAKPOINT();
      _f.lextokv = get_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " got token after leftv=" << _f.leftv
                    << " operandvect=" << operandvect
                    << " got lextok=" << _f.lextokv << std::endl
                    << "… lexopertokv=" << _f.lexopertokv << "  in:" << (*this) << std::endl
                    << "… @! " << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      })
          << std::endl << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term after-left"));
      if (!_f.lextokv)
        break;
      if (_f.lexopertokv && _f.lexopertokv.is_lextoken()
          &&  _f.lexopertokv.to_lextoken()->lxkind()
          == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK) //repl_delimiter∈class
          &&  _f.lexopertokv.to_lextoken()->lxval().is_object())
        {
          _f.lexoperdelimob =  _f.lexopertokv.to_lextoken()->lxval().to_object();
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " got token after leftv=" << _f.leftv
                        << " got lexopertokv=" << _f.lexopertokv << " bindelimob=" << _f.bindelimob
                        << " lexoperdelimob=" << _f.lexoperdelimob
                        << " binoperob=" << _f.binoperob
                        << " @! " << position_str() << std::endl
                        << " .. curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr()
                        << " token_deq:" << toksrc_token_deq << std::endl
                        << " .. in: " << (*this)
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          if (_f.lexoperdelimob == _f.multdelimob)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " lexopertokv:" << _f.lexopertokv << " multiply at " << position_str()
                            << std::endl << "… token_deq:" << toksrc_token_deq << " startpos:" << startpos);
              _f.curoperob = _f.multbinopob;
            }
          else if (_f.lexoperdelimob == _f.divdelimob)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " lexopertokv:" << _f.lexopertokv << " divide at " << position_str()
                            << std::endl << "… token_deq:" << toksrc_token_deq << " startpos:" << startpos);
              _f.curoperob = _f.divbinopob;
            }
          else if (_f.lexoperdelimob == _f.moddelimob)
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " lexopertokv:" << _f.lexopertokv << " modulus at " << position_str()
                            << std::endl << "… token_deq:" << toksrc_token_deq << " startpos:" << startpos);
              _f.curoperob = _f.modbinopob;
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " lexopertokv:" << _f.lexopertokv
                            << " strange lexoperdelimob:" << _f.lexoperdelimob << " :!-> return leftv:" << _f.leftv
                            << std::endl << "… token_deq:" << toksrc_token_deq << " startpos:" << startpos
                            <<" curcptr " << Rps_QuotedC_String(curcptr()));
              if (pokparse)
                *pokparse = true;
              return _f.leftv;
            }
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " got token after leftv=" << _f.leftv << " curoperob=" << _f.curoperob
                        << " lexopertokv=" << _f.lexopertokv);
        }
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " operandvect:" << operandvect
                    << " curoperob=" << _f.curoperob << " binoperob=" << _f.binoperob << " lexoperdelimob=" << _f.lexoperdelimob
                    << " loopcnt#" << loopcnt
                    << " pos:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr()) << "@" << ((void*)curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (_f.curoperob)
        {
          if (!_f.binoperob)
            _f.binoperob = _f.curoperob;
          if (_f.binoperob == _f.curoperob)
            {
              bool okright = false;
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " operandvect:" << operandvect << " leftv=" << _f.leftv
                            << " before parse_primary of right" << position_str());
              _f.rightv = parse_primary(&_, &okright);
              if (!okright)
                {
                  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " failing_B at startpos:" << startpos
                                << "  in:" << (*this)
                                << " position:" << position_str()
                                << " curcptr:" << Rps_QuotedC_String(curcptr())
                                << std::endl
                                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term failing_B")
                                << std::endl
                                << Rps_Do_Output([&](std::ostream& out)
                  {
                    this->display_current_line_with_cursor(out);
                  }));
                  RPS_PARSREPL_FAILURE(&_,
                                       "Rps_TokenSource::parse_term¤" << callnum << " invalid primary at startpos:" << startpos
                                       << " in:" << (*this)
                                       << " position:" << position_str()
                                       << " curcptr:" << Rps_QuotedC_String(curcptr())
                                       << " token_deq:" << toksrc_token_deq);
                  RPS_WARNOUT("Rps_TokenSource::parse_term with invalid primary on right of "
                              << _f.curoperob << " at " << position_str()
                              << " with operandvect:" << operandvect);
                  if (pokparse)
                    *pokparse = false;
                  return nullptr;
                }
              operandvect.push_back(_f.rightv);
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " operandvect:" << operandvect << " leftv=" << _f.leftv
                            << " rightv=" << _f.rightv << " loopcnt#" << loopcnt
                            << " curoperob=" << _f.curoperob << " position_str:" << position_str()
                            << " " << Rps_QuotedC_String(curcptr())
                            << " operandvect:" << operandvect
                            << " binoperop=" << _f.binoperob
                            << " token_deq:" << toksrc_token_deq
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              }));
              again = true;
              continue;
            }
          else
            {
              /* binoperob != curoperob */
#warning Rps_TokenSource::parse_term make two things?
              RPS_FATALOUT("missing code in Rps_TokenSource::parse_term¤" << callnum << " from " << Rps_ShowCallFrame(callframe)
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
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " breakingloop operandvect:"
                        << operandvect << " leftv=" << _f.leftv
                        << " curoperob=" << _f.curoperob << " right=" << _f.rightv
                        << " binoperob=" << _f.binoperob);
          break;
        };
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " **endingloop @ " << position_str()
                    << " operandvect:" << operandvect << " curoperob:" << _f.curoperob
                    << std::endl << "… in:" << (*this)
                    << (again?"again":"stop")
                    << " loopcnt#" << loopcnt << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "@" << (void*)(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
    } // end while (again)
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " endloop#" << loopcnt
                << " leftv=" << _f.leftv << " in:" << (*this)
                << std::endl << " operandvect:" << operandvect
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << "@" << ((void*)curcptr())
                << " calldepth:" << rps_call_frame_depth(&_));
  if (operandvect.size() == 1)
    {
      _f.restermv = operandvect[0];
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_term¤" << callnum << " GIVES restermv=" << _f.restermv << " in:" << (*this)
                    << " startpos:" << startpos << " at:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "@" << ((void*)curcptr())
                    << " calldepth:" << rps_call_frame_depth(&_)
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (pokparse)
        *pokparse = true;
      return _f.restermv;
    }
  else
    {
#warning incomplete  Rps_TokenSource::parse_term
      RPS_FATALOUT("Rps_TokenSource::parse_term¤" << callnum << " INCOMPLETE in:" << (*this)
                   << " curcptr " << Rps_QuotedC_String(curcptr())
                   << "  curpos:" << position_str()
                   << " operandvect:" << operandvect
                   << "  calldepth:" << rps_call_frame_depth(&_)
                   << " binoperob:" << _f.binoperob
                   << " curoperob:" << _f.curoperob
                   << " operandvect:" << operandvect
                   << " leftv=" << _f.leftv << std::endl
                   << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_term/INCOMPLETE"));
    }
} // end Rps_TokenSource::parse_term


/// a term is a sequence of factors with multiplicative operators
/// between them…. All the operators should be the same. Otherwise we build intermediate subexpressions
Rps_Value
Rps_TokenSource::parse_product(Rps_CallFrame*callframe, bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  static long callcnt;
  long callnum= ++ callcnt;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
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
    // maybe token_deq is already GC-marked by caller….
    RPS_ASSERT(gc != nullptr);
    this->gc_mark(*gc);
#warning code review needed. Do we need  this->gc_mark(*gc) in Rps_TokenSource::parse_term ?
    // but the operandvect needs to be GC-marked
    for (auto operv : operandvect)
      gc->mark_value(operv);
  });
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_product¤" << callnum
                << " BEGIN startpos:"
                << startpos << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  })
      << std::endl);
  /// multiplication operator and * delim
  static Rps_Id id_mult_delim;
  if (!id_mult_delim)
    id_mult_delim = Rps_Id("_2uw3Se5dPOU00yhxpA"); // id of "mult!delim"∈repl_delimiter
  static Rps_Id id_mult_oper;
  if (!id_mult_oper)
    id_mult_oper = Rps_Id("_4QX7Cg3gDkd005b9bn"); // id of "mult!binop"∈repl_binary_operator
  _f.multdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_delim); // "mult!delim"∈repl_delimiter
  _f.multbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_mult_oper); // "mult!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_product¤" << callnum << " startpos:" << startpos << " multdelimob:" << _f.multdelimob
                << " multbinopob: " << _f.multbinopob
                << " token_deq:" << toksrc_token_deq
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  /// division operator and / delim
  static Rps_Id id_div_delim;
  if (!id_div_delim)
    id_div_delim = Rps_Id("_3ak80l3pr9700M90pz"); // id of "div!delim"∈repl_delimiter
  static Rps_Id id_div_oper;
  if (!id_div_oper)
    id_div_oper = Rps_Id("_0GTVGelTnCP01I0od2"); // id of "div!binop"∈repl_binary_operator
  _f.divdelimob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_delim); // "div!delim"∈repl_delimiter
  _f.divbinopob = Rps_ObjectRef::find_object_or_fail_by_oid(&_,id_div_oper); // "div!binop"∈repl_binary_operator
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_product¤" << callnum
                << " START startpos:" << startpos << " divdelimob:" << _f.divdelimob
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_product¤" << callnum << "  startpos:" << startpos << " moddelimob:" << _f.moddelimob
                << " modbinopob: " << _f.modbinopob
                << " pos:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr()) << "@" << (void*)curcptr());
  /////
  RPS_FATALOUT("unimplemented Rps_TokenSource::parse_product¤" << callnum << "  startpos:" << startpos
               << " curcptr " << Rps_QuotedC_String(curcptr())
               << " in " << (*this));
#warning unimplemented   Rps_TokenSource::parse_product
  ////////////////
}


/// This member function returns some expression which could later be
/// evaluated to a value; the *pokparse flag, when given, is set to
/// true if and only if parsing was successful.
Rps_Value
Rps_TokenSource::parse_primary(Rps_CallFrame*callframe,  bool*pokparse)
{
  RPS_ASSERT(rps_is_main_thread());
  static long callcnt;
  long callnum= ++ callcnt;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
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
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " START startpos:" << startpos
                << "  in:" << (*this) << std::endl
                << "… callframe:" << callframe<< " curcptr:" << Rps_QuotedC_String(curcptr())
                << std::endl
                << "… token_deq:" << toksrc_token_deq<< std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary start")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  _f.lextokv =  lookahead_token(&_,  0);
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " started lextokv=" << _f.lextokv
                << " startpos:" << startpos << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq<< " curcptr:" << Rps_QuotedC_String(curcptr()));
  if (!_f.lextokv)
    {
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " failing_A at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_A"));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_primary¤" << callnum << " fail left prim at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  const Rps_LexTokenZone* ltokz = _f.lextokv.to_lextoken();
  RPS_ASSERT (ltokz != nullptr);
  _f.lexkindob = ltokz->lxkind();
  _f.lexvalv = ltokz->lxval();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " lexkindob="
                << _f.lexkindob << " lexval=" << _f.lexvalv
                << std::endl
                << " position:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << " lextokv=" << _f.lextokv
                << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq);
  if (!can_start_primary(&_))
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum
                    << " failing canstartprim lexkindob="
                    << _f.lexkindob << " lexval=" << _f.lexvalv << " position:" << position_str()
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << "  in:" << (*this));
      if (pokparse)
        *pokparse = false;
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum
                    << " failing_B at startpos:" << startpos
                    << "  in:" << (*this)
                    << " position:" << position_str()
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_B"));
      RPS_PARSREPL_FAILURE(&_,
                           "Rps_TokenSource::parse_primary¤" << callnum << " fail bad start at startpos:" << startpos
                           << " in:" << (*this)
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
      return nullptr;
    }
  /// can_start_primary was successful…
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum
                << " ok can_start_primary lexkindob="
                << _f.lexkindob << " lexval=" << _f.lexvalv << std::endl
                << "… startpos:" << startpos << " position:" << position_str()
                << " curcptr " << Rps_QuotedC_String(curcptr())
                << " lextokv=" << _f.lextokv
                << "  in:" << (*this)
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  if (_f.lexkindob == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) // int
      && _f.lexvalv.is_int())
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " lexgotokv " << _f.lexgotokv
                    << " lexval " << _f.lexvalv
                    << " position:" << position_str() << " curcptr " << Rps_QuotedC_String(curcptr()));
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum <<" => int " << _f.lexvalv
                    << " lextokv:" << _f.lextokv << std::endl
                    << " … lexgotokv:" << _f.lexgotokv
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " startpos: " << startpos << std::endl
                    << "…  in:" << (*this)
                    << " token_deq:" << toksrc_token_deq
                    << " at " << position_str() << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary int"));
      if (pokparse)
        *pokparse = true;
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
           && _f.lexvalv.is_string())
    {
      if (pokparse)
        *pokparse = true;
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " => string " << _f.lexvalv
                    << "  in:" << (*this)
                    << " lexgotokv:" << _f.lexgotokv
                    << " at " << position_str() << " startpos:" << startpos
                    << " token_deq:" << toksrc_token_deq
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary string"));
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
           && _f.lexvalv.is_double())
    {
      if (pokparse)
        *pokparse = true;
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " => double "
                    << _f.lexvalv << " lexgotokv:" << _f.lexgotokv << std::endl
                    << "… in:" << (*this)
                    << " lexgotokv:" << _f.lexgotokv
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << " startpos: " << startpos << std::endl
                    << " at " << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary double"));
      return _f.lexvalv;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
           && _f.lexvalv.is_object())
    {
      _f.obres = _f.lexvalv.to_object();
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " before consumetok "
                    << _f.obres << " lexgotokv:" << _f.lexgotokv << std::endl
                    << "…  in:" << (*this)
                    << " at " << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr " << Rps_QuotedC_String(curcptr()));
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " :: object "
                    << _f.obres << " lexgotokv:" << _f.lexgotokv << std::endl
                    << "…  in:" << (*this)
                    << " at " << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr " << Rps_QuotedC_String(curcptr()));
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum
                    << " :: object after gettok"
                    << _f.obres << " next lexgotokv:" << _f.lexgotokv << std::endl
                    << "…  in:" << (*this)
                    << " at " << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      if (pokparse)
        *pokparse = true;
      RPS_DEBUG_LOG(REPL, "-Rps_TokenSource::parse_primary¤" << callnum
                    << " succeeds => obres:" << _f.obres
                    << std::endl << "… in " << (*this)
                    << " at " << position_str()
                    << " token_deq:" << toksrc_token_deq
                    << " curcptr " << Rps_QuotedC_String(curcptr())
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
      {
        this->display_current_line_with_cursor(out);
      }));
      return _f.obres;
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_2wdmxJecnFZ02VGGFK)) //repl_delimiter∊class
    {
      _f.obdelim =  _f.lexvalv.to_object();
      consume_front_token(&_);
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " delimited"
                    << " lexkindob:" << _f.lexkindob
                    << " obdelim=" << _f.obdelim << std::endl
                    << "…  in:" << (*this)
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " position:" << position_str() << " startpos:" << startpos);
      // test for  leftparen _4YM7mv0GrSp03OkF8T
      if (_f.obdelim
          == RPS_ROOT_OB(_4YM7mv0GrSp03OkF8T) //leftparen!delim∊repl_delimiter
         )
        {
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " got leftparen "
                        << "  in:" << (*this) << std::endl
                        << "… lextokv:" << _f.lextokv
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " position:" << position_str() << " startpos:" << startpos << std::endl
                        << " token_deq:" << toksrc_token_deq
                        << std::endl
                        << Rps_Do_Output([&](std::ostream& out)
          {
            this->display_current_line_with_cursor(out);
          }));
          /// we did consume the leftparen-
          _f.lexgotokv = get_token(&_);
          //WRONG _f.lexgotokv = lookahead_token(&_, 0);
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " got leftparen before parsing-subexpression"
                        << "  in:" << (*this) << std::endl
                        << "… lextokv:" << _f.lextokv << std::endl
                        << "… lexgotokv:" << _f.lexgotokv << std::endl
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
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " after subexpression "
                            << _f.exprv << " startpos:" << startpos << " position:" << position_str()
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << " token_deq:" << toksrc_token_deq
                            << "  in:" << (*this)
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary after subexpression")
                            << std::endl
                            << Rps_Do_Output([&](std::ostream& out)
              {
                this->display_current_line_with_cursor(out);
              }));
#warning  Rps_TokenSource::parse_primary should check for rightparen and consume
              RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary with leftparen " << _f.obdelim
                           << "  in:" << (*this)
                           << " startpos:" << startpos
                           << " position:" << position_str()
                           << " curcptr:" << Rps_QuotedC_String(curcptr())
                           << " token_deq:" << toksrc_token_deq);
#warning TODO: Rps_TokenSource::parse_primary use rightparen _7CG9m1NXpMo01edTUl and build subexpression object
              /* TODO: we probably should make then return some object or some
              instance for that primary in parenthesis… */
            }
          else
            {
              RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " failing_C at startpos:" << startpos
                            << "  in:" << (*this)
                            << " position:" << position_str()
                            << " token_deq:" << toksrc_token_deq
                            << " curcptr:" << Rps_QuotedC_String(curcptr())
                            << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_C"));
              RPS_PARSREPL_FAILURE(&_,
                                   "Rps_TokenSource::parse_primary¤" << callnum << " fail subexpr in paren at startpos:" << startpos
                                   << " in:" << (*this)
                                   << " position:" << position_str()
                                   << " curcptr:" << Rps_QuotedC_String(curcptr())
                                   << " token_deq:" << toksrc_token_deq);
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
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " failing_D at startpos:" << startpos
                        << "  in:" << (*this)
                        << " position:" << position_str()
                        << " curcptr:" << Rps_QuotedC_String(curcptr())
                        << " token_deq:" << toksrc_token_deq
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary failing_D"));
          RPS_PARSREPL_FAILURE(&_,
                               "Rps_TokenSource::parse_primary¤" << callnum << " failD unexpected delim" << _f.obdelim
                               <<" at startpos:" << startpos
                               << " in:" << (*this)
                               << " position:" << position_str()
                               << " curcptr:" << Rps_QuotedC_String(curcptr())
                               << " token_deq:" << toksrc_token_deq);
          RPS_WARNOUT("Rps_TokenSource::parse_primary¤" << callnum << " failing, unexpected delimiter " << _f.obdelim
                      << "  in:" << (*this)
                      << " startpos:" << startpos
                      << " position:" << position_str());
          if (pokparse)
            *pokparse = false;
          return nullptr;
        }
#warning incomplete Rps_TokenSource::parse_primary with delimiter
    } // end if lexkindob is _2wdmxJecnFZ02VGGFK //repl_delimiter∊class
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::parse_primary¤" << callnum << " @@incomplete"
                << "  in:" << (*this) << std::endl
                << "… lextokv:" << _f.lextokv
                << " lexkindob:" << _f.lexkindob << std::endl
                << "… startpos:" << startpos
                << " position:" << position_str()
                << std::endl
                << "… curcptr:" << Rps_QuotedC_String(curcptr())
                << " obdelim:" << _f.obdelim
                << " token_deq:" << toksrc_token_deq
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::parse_primary@@incomplete")
                << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
  ///////@@@@@@@@@@Rps_TokenSource::parse_primary_complement should be called?
#warning unimplemented Rps_TokenSource::parse_primary
  /** TODO:
   * we probably want to code some recursive descent parser for REPL,
   * but we need some specification (in written English, using EBNF
   * notation….) of REPL expressions
   *
   * That specification of REPL expressions should go into file
   * doc/repl.md or into doc/
   **/
  RPS_FATALOUT("unimplemented Rps_TokenSource::parse_primary¤" << callnum << std::endl
               << Rps_ShowCallFrame(&_) << std::endl
               << "  in:" << (*this)
               << " lextokv:" << _f.lextokv << std::endl
               << " … lexkindob:" << _f.lexkindob
               << " lexvalv:" << _f.lexvalv
               << " lexgotokv:" << _f.lexgotokv
               << " position_str:" << position_str()
               << " startpos:" << startpos
               << " curcptr:" << Rps_QuotedC_String(curcptr())
               << " token_deq:" << toksrc_token_deq);
} // end Rps_TokenSource::parse_primary


bool
Rps_TokenSource::can_start_primary(Rps_CallFrame*callframe)
{
  RPS_ASSERT(rps_is_main_thread());
  static long callcnt;
  long callnum= ++ callcnt;
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/callframe,
                 Rps_Value lextokv;
                 Rps_ObjectRef lexkindob;
                 Rps_ObjectRef delimob;
                 Rps_Value lexvalv;
                );
  std::string startpos = position_str();
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_start_primary¤" << callnum <<" starting startpos:" << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " callframe:" << callframe
                << " token_deq:" << toksrc_token_deq);
  _f.lextokv =  lookahead_token(&_,  0);
  if (!_f.lextokv)
    {
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_start_primary¤" << callnum << " CANNOT, startpos:" << startpos
                    << "  in:" << (*this)
                    << " curcptr:" << Rps_QuotedC_String(curcptr())
                    << " token_deq:" << toksrc_token_deq);
      return false;
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_parse_primary¤" << callnum << " lextokv:" << _f.lextokv
                <<" startpos:" << startpos
                << "  in:" << (*this)
                << " curcptr:" << Rps_QuotedC_String(curcptr())
                << " callframe:" << callframe
                << " token_deq:" << toksrc_token_deq);
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
      RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_start_primary¤" << callnum << " delimiter "
                    << " lexvalv=" << _f.lexvalv << " at startpos: " << startpos
                    << " position_str:" << position_str() << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_TokenSource::can_parse_primary§delim"));
      if (_f.lexvalv.is_object())
        {
          _f.delimob = _f.lexvalv.to_object();
          RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_start_primary¤" << callnum << " delimob "
                        << _f.delimob<< " at startpos: " << startpos
                        << " position_str:" << position_str());
          if (_f.delimob == RPS_ROOT_OB(_4YM7mv0GrSp03OkF8T))   // leftparen!delim∊repl_delimiter
            {
              return true;
            }
        }
    }
  RPS_DEBUG_LOG(REPL, "Rps_TokenSource::can_start_primary¤" << callnum <<" fail lexkindob=" << _f.lexkindob
                << " lexvalv=" << _f.lexvalv);
  return false;
} // end Rps_TokenSource::can_start_primary


Rps_Value
Rps_TokenSource::parse_primary_complement(Rps_CallFrame*callframe, Rps_Value primaryexparg, bool*pokparse)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
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
                << " primaryexpv:" << _f.primaryexpv << " in " << *this << std::endl
                << Rps_Do_Output([&](std::ostream& out)
  {
    this->display_current_line_with_cursor(out);
  }));
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


#include "generated/rps-parser-impl.cc"

///// end of file parsrepl_rps.cc of RefPerSys

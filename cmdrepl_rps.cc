/****************************************************************
 * file cmdrepl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It implements the Read-Eval-Print-Loop commands in relation to
 *      repl_rps.cc file.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2021 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_cmdrepl_gitid[];
const char rps_repl_gitid[]= RPS_GITID;

extern "C" const char rps_cmdrepl_date[];
const char rps_cmdrepl_date[]= __DATE__;

extern "C" const char rps_cmdrepl_shortgitid[];
const char rps_repl_shortgitid[]= RPS_SHORTGITID;

// internal code to evaluate composite expressions like arithmetic, conditionals, etc...
static Rps_TwoValues
rps_full_evaluate_repl_composite_object(Rps_CallFrame*callframe, unsigned long count, Rps_ObjectRef exprobarg, Rps_ObjectRef envobarg,  unsigned depth=0);


/// FIXME: declare  rps_full_evaluate_repl_instance
#warning should declare rps_full_evaluate_repl_instance


//// TODO: define some cmdrepl_rps.cc local conventions for local frames so a future
//// RPS_REPLEVAL_LOCALFRAME macro which defines and initialize exprv and envob
#warning we could need some RPS_REPLEVAL_LOCALFRAME macro local to this source file.

/// Evaluate for the REPL machinery in given callframe the expression
/// `expr` in the environment given by `envob`; should give two values
/// which should not be both null.  This routine might be called in a
/// non-REPL thread by agenda....
Rps_TwoValues
rps_full_evaluate_repl_expr(Rps_CallFrame*callframe, Rps_Value exprarg, Rps_ObjectRef envobarg)
{
#define TEMPORARY_CODE 1
  RPS_ASSERT_CALLFRAME (callframe);
  RPS_ASSERT(envobarg);
  constexpr int maxloop=256;
  int framdepth = callframe->call_frame_depth();
  unsigned startdbgflags = rps_debug_flags.load();
  static std::atomic<unsigned long> eval_repl_counter_;
  const unsigned long eval_number = 1+eval_repl_counter_.fetch_add(1);
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_Value closv;
                 Rps_Value exprv;
                 Rps_ObjectRef evalob;
                 Rps_ObjectRef envob;
                 Rps_ObjectRef nextenvob;
                 Rps_ObjectRef firstenvob;
                 Rps_ObjectRef classob;
                 Rps_Value mainresv;
                 Rps_Value extraresv;
                );
  _f.closv = _.call_frame_closure();
  _f.exprv = exprarg;
  _f.envob = envobarg;
  _f.nextenvob = nullptr;
  _f.firstenvob = envobarg;
  /// macros to ease debugging
#define RPS_REPLEVAL_GIVES_BOTH_AT(V1,V2,LIN) do {              \
    _f.mainresv = (V1);                                         \
    _f.extraresv = (V2);                                        \
    RPS_DEBUG_LOG_AT(__FILE__,LIN,REPL,                         \
                     __FUNCTION__ << "#"                        \
                     << eval_number << " of expr:" << _f.exprv  \
                     << " in envob:" << _f.envob                \
                     << " gives main:" << _f.mainresv           \
                     << ", extra:" << _f.extraresv);            \
    return Rps_TwoValues(_f.mainresv,_f.extraresv);             \
  } while(0)
#define RPS_REPLEVAL_GIVES_BOTH(V1,V2) RPS_REPLEVAL_GIVES_BOTH_AT((V1),(V2),__LINE__)
  ///
#define RPS_REPLEVAL_GIVES_PLAIN_AT(V1,LIN) do {        \
    _f.mainresv = (V1);                                 \
    _f.extraresv = nullptr;                             \
    RPS_DEBUG_LOG_AT(__FILE__,LIN,REPL,                 \
                     __FUNCTION__ << "#"                \
                     << eval_number << " of expr:"      \
                     << _f.exprv                        \
                     << " in envob:" << _f.envob        \
                     << " gives main:" << _f.mainresv   \
                     << ", extra:" << _f.extraresv);    \
    return Rps_TwoValues(_f.mainresv,_f.extraresv);     \
  } while(0)
  ///
#define RPS_REPLEVAL_GIVES_PLAIN(V) RPS_REPLEVAL_GIVES_PLAIN_AT((V),__LINE__)
  ///
#define RPS_REPLEVAL_FAIL_AT(MSG,LOG,LIN) do {                  \
    RPS_DEBUG_LOG_AT(__FILE__,LIN,REPL,                         \
                     __FUNCTION__ << "#"                        \
                     << eval_number << " FAILS for expr:"       \
                     << _f.exprv                                \
                     << " in envob:" << _f.envob                \
                     << "::" << (MSG)                           \
                     << "; " << LOG);                           \
    throw  std::runtime_error("rps_full_evaluate_repl_expr "    \
                              " fail " #MSG "@" #LIN); }        \
  while(0)
  ///
  ///
#define  RPS_REPLEVAL_FAIL(MSG,LOG) RPS_REPLEVAL_FAIL_AT(MSG,LOG,__LINE__)
  ///
  ///
  RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#"
                << eval_number << " *STARTEVAL*"
                << " expr:" << _f.exprv
                << " in env:" << _f.envob << " framdepth=" << framdepth);
  /// to check the above failure macro:
  if (!_f.envob || _f.envob->stored_type() != Rps_Type::Object)
    {
      // This don't happen in practice, but tests that
      // RPS_REPLEVAL_FAIL macro is good enough...
      RPS_REPLEVAL_FAIL("*check-fail*","never happens no envob"
                        << _f.envob);
    };
  std::lock_guard gu(*_f.envob->objmtxptr());
  if (!_f.envob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
    {
      RPS_REPLEVAL_FAIL("bad environment","The envob " << _f.envob << " of class "
                        << _f.envob->get_class() << " is not a valid environment");
    };
  auto envpayl = _f.envob->get_dynamic_payload<Rps_PayloadEnvironment>();
  if (!envpayl)
    {
      RPS_REPLEVAL_FAIL("bad environment payload","The envob " << _f.envob << " of class "
                        << _f.envob->get_class() << " without environment payload");
    };
  /* environments should have bindings, probably with Rps_PayloadEnvironment */
#warning rps_full_evaluate_repl_expr should check that envob is an environment, with bindings and optional parent env....
  RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#"
                << eval_number << " *STARTEVAL*"
                << " expr:" << _f.exprv
                << " in env:" << _f.envob);
  /* we try to put common cases first... */
  RPS_POSSIBLE_BREAKPOINT();
  if (_f.exprv.is_int())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_double())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_string())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_tuple())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_set())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_closure())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_empty())
    /// return a secondary value to avoid "failure"
    RPS_REPLEVAL_GIVES_BOTH(nullptr,
                            RPS_ROOT_OB(_2i66FFjmS7n03HNNBx)); //space∈class
  else if (_f.exprv.is_json())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_lextoken())
    RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
  else if (_f.exprv.is_instance())
    {
      _f.classob = _f.exprv.compute_class(&_);
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " instance expr:" << _f.exprv
                    << " of class:" << _f.classob
                    << " in env:" << _f.envob);
#warning TODO: should probably define and call a rps_full_evaluate_repl_instance
      RPS_FATALOUT("rps_full_evaluate_repl_expr#" << eval_number
                   << " UNIMPLEMENTED instance expr:" << _f.exprv
                   << " of class:" << _f.classob
                   << " in env:" << _f.envob);
    }
  else if (_f.exprv.is_object())
    {
      _f.evalob = _f.exprv.as_object();
      std::lock_guard<std::recursive_mutex> gu(*_f.evalob->objmtxptr());
      _f.classob = _f.exprv.compute_class(&_);
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " object expr:" << _f.exprv
                    << " of class:" << _f.classob << " physicalclass:" << _f.evalob->get_class()
                    << " in env:" << _f.envob);
    };
  ///
  RPS_ASSERT(_f.classob && _f.classob->is_class());
  RPS_POSSIBLE_BREAKPOINT();
  /****
   * Evaluation of variables - perhaps anonymous ones
   ****/
  if (_f.classob == RPS_ROOT_OB(_4HJvNCh35Lu00n5z3R) //variable∈class
      || _f.classob->is_subclass_of(RPS_ROOT_OB(_4HJvNCh35Lu00n5z3R) //variable∈class
                                   ))
    {
      int count=0;
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " object expr:" << _f.exprv
                    << " is variable envob:" <<_f.envob);
      while (count++ < maxloop && _f.envob)
        {
          _f.nextenvob = nullptr;
          std::lock_guard gu(*_f.envob->objmtxptr());
          RPS_POSSIBLE_BREAKPOINT();
          auto paylenv = _f.envob->get_dynamic_payload<Rps_PayloadEnvironment>();
          RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                        << " object expr:" << _f.exprv << std::endl
                        << " evalob=" << _f.evalob
                        << ", loopcount:" << count
                        << " envob=" << _f.envob << ' '
                        << ((paylenv != nullptr)?"*env*":"*NOTENV*")
                        << ", firstenvob=" << _f.firstenvob);
          if (paylenv)
            {
              bool missing = false;
              RPS_POSSIBLE_BREAKPOINT();
              _f.mainresv = paylenv->get_obmap(_f.evalob,/*defaultval:*/nullptr,&missing);
              if (!missing)
                {
                  RPS_REPLEVAL_GIVES_PLAIN(_f.mainresv);
                }
              RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                            << " object expr:" << _f.exprv << " missing in envob=" << _f.envob
                            << ", firstenvob=" << _f.firstenvob
                            << " loopcount:" << count);
              _f.nextenvob = paylenv->get_parent_environment();
            }
          else // envob without Rps_PayloadEnvironment
            RPS_REPLEVAL_FAIL("bad environment","The envob " << _f.envob << " of class "
                              << _f.envob->get_class() << " has no payload environment;"
                              << " first env was " <<_f.firstenvob
                              << " evaluating variable " << _f.exprv);
          RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                        << " object variable:" << _f.evalob << " ending loop count#" << count
                        << " is variable envob:" <<_f.envob << " firstenvob:" << _f.firstenvob
                        << " envob=" << _f.envob << " nextenvob=" << _f.nextenvob);
          _f.envob = _f.nextenvob;
          RPS_POSSIBLE_BREAKPOINT();
        } // end while count... loop for variable
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " object VARIABLE expr:" << _f.evalob << " exprv:" << _f.exprv
                    << " unbound in envob=" << _f.envob << " firstenvob=" << _f.firstenvob << " count#" << count << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1,"rps_full_evaluate_repl_expr unboundvar"));
      RPS_POSSIBLE_BREAKPOINT();
      RPS_REPLEVAL_FAIL("unbound variable","Variable " << _f.evalob << " unbound with envob " << _f.envob << " of class "
                        << _f.envob->get_class()
                        << " first env was " <<_f.firstenvob);
    }
  /****
   * Evaluation of symbolic variables - named ones
   ****/
  else if (_f.classob ==  RPS_ROOT_OB(_4Si5RBkg1Qm0285SD0) //symbolic_variable∈class
           || _f.classob->is_subclass_of(RPS_ROOT_OB(_4Si5RBkg1Qm0285SD0) //symbolic_variable∈class
                                        ))
    {
      int count=0;
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " object expr:" << _f.exprv
                    << " is symbolic_variable envob:" <<_f.envob);
      RPS_POSSIBLE_BREAKPOINT();
      while (count++ < maxloop && _f.envob)
        {
          _f.nextenvob = nullptr;
          std::lock_guard gu(*_f.envob->objmtxptr());
          auto paylenv = _f.envob->get_dynamic_payload<Rps_PayloadEnvironment>();
          if (paylenv)
            {
              bool missing = false;
              _f.mainresv = paylenv->get_obmap(_f.evalob,nullptr,&missing);
              if (!missing)
                {
                  RPS_REPLEVAL_GIVES_PLAIN(_f.mainresv);
                }
              _f.nextenvob = paylenv->get_parent_environment();
            }
          else // envob without Rps_PayloadEnvironment
            RPS_REPLEVAL_FAIL("bad environment","The envob " << _f.envob << " of class "
                              << _f.envob->get_class() << " has no payload environment;"
                              << " first env was " <<_f.firstenvob
                              << " evaluating symbolic variable " << _f.exprv);
          RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                        << " object variable:" << _f.evalob << " ending loop count#" << count
                        << " is symbolic_variable envob:" <<_f.envob << " firstenvob:" << _f.firstenvob
                        << " nextenvob:" << _f.nextenvob);
          RPS_POSSIBLE_BREAKPOINT();
          _f.envob = _f.nextenvob;
        };      // end while count<... symbvar
      RPS_REPLEVAL_FAIL("unbound symbolic variable","Symbolic variable " << _f.evalob
                        << " unbound with envob " << _f.envob << " of class "
                        << _f.envob->get_class()
                        << " first env was " <<_f.firstenvob);
    }
#warning TODO: fixme evaluation of various repl_expression-s e.g. conditional, arithmetic, application
  else if  (_f.classob ==  RPS_ROOT_OB(_1jJaY1usnpR02WUvSX) //repl_expression∈class
            || _f.classob->is_subclass_of(RPS_ROOT_OB(_1jJaY1usnpR02WUvSX) //repl_expression∈class
                                         ))
    {
      RPS_POSSIBLE_BREAKPOINT();
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " repl_expression:" << _f.evalob
                    << " of class " << _f.classob
                    << " in envob:" <<_f.envob << " firstenvob:" << _f.firstenvob);
      Rps_TwoValues two = rps_full_evaluate_repl_composite_object(&_, eval_number, _f.evalob, _f.envob, 0);
      RPS_REPLEVAL_GIVES_BOTH(two.main_val, two.xtra_val);
    }
  else
    {
      // any other object is self evaluating! or NOT?
      // TODO: think more.
      RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_expr#" << eval_number
                    << " object:" << _f.evalob << " of class " << _f.classob << " is selfevaluating in envob:" <<_f.envob << " firstenvob:" << _f.firstenvob);

      RPS_POSSIBLE_BREAKPOINT();
      RPS_REPLEVAL_GIVES_PLAIN(_f.exprv);
    }
} // end rps_full_evaluate_repl_expr


Rps_TwoValues
rps_full_evaluate_repl_composite_object(Rps_CallFrame*callframe, unsigned long count, Rps_ObjectRef exprobarg, Rps_ObjectRef envobarg,  unsigned depth)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_ObjectRef exprob;
                 Rps_ObjectRef envob;
                 Rps_Value mainresv;
                 Rps_Value otheresv;
                );
  _f.exprob = exprobarg;
  _f.envob = envobarg;
  RPS_DEBUG_LOG(REPL, "rps_full_evaluate_repl_composite_object#" << count <<" START exprob:" << _f.exprob << " envob:" << _f.envob
                << " depth:" << depth << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_full_evaluate_repl_composite_object"));
#warning unimplemented rps_full_evaluate_repl_composite_object
  RPS_FATALOUT("rps_full_evaluate_repl_composite_object#" << count <<" UNIMPLEMENTED exprob:" << _f.exprob << " envob:" << _f.envob << " depth:" << depth);
} // end rps_full_evaluate_repl_composite_object

/// forget our macros
#undef RPS_REPLEVAL_GIVES_BOTH_AT
#undef RPS_REPLEVAL_GIVES_BOTH
#undef RPS_REPLEVAL_GIVES_PLAIN_AT
#undef RPS_REPLEVAL_GIVES_PLAIN

Rps_Value
rps_simple_evaluate_repl_expr(Rps_CallFrame*callframe, Rps_Value expr, Rps_ObjectRef envob)
{
  RPS_ASSERT_CALLFRAME (callframe);
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_Value exprv;
                 Rps_ObjectRef envob;
                 Rps_Value mainresv;
                 Rps_Value otheresv;
                );
  _f.exprv = expr;
  _f.envob = envob;

  RPS_DEBUG_LOG(REPL, "rps_simple_evaluate_repl_expr START expr:" << _f.exprv << " envob:" << _f.envob << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_simple_evaluate_repl_expr"));
  {
    Rps_TwoValues two = rps_full_evaluate_repl_expr(&_,_f.exprv,_f.envob);
    _f.mainresv = two.main();
    _f.otheresv = two.xtra();
  }
  return _f.mainresv;
} // end rps_simple_evaluate_repl_expr



////////////////

Rps_PayloadEnvironment::Rps_PayloadEnvironment(Rps_ObjectZone*obown) :
  Rps_PayloadObjMap(obown),
  env_parent(nullptr)
{
} // end Rps_PayloadEnvironment::Rps_PayloadEnvironment

Rps_ObjectZone*
Rps_PayloadEnvironment::make(Rps_CallFrame*callframe, Rps_ObjectRef classob, Rps_ObjectRef spaceob)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED, callframe,
                 Rps_ObjectRef obclass;
                 Rps_ObjectRef obspace;
                 Rps_ObjectRef obenv;
                );
  _f.obclass = classob;
  _f.obspace = spaceob;
  RPS_ASSERT(classob);
  RPS_ASSERT(classob == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
             || classob->is_subclass_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)));
  _f.obenv = Rps_ObjectRef::make_object(&_, _f.obclass, _f.obspace);
  auto paylenv = _f.obenv->put_new_plain_payload<Rps_PayloadEnvironment>();
  RPS_ASSERT(paylenv);
  return _f.obenv;
} // end Rps_PayloadEnvironment::make

Rps_ObjectZone*
Rps_PayloadEnvironment::make_with_parent_environment(Rps_CallFrame*callframe, Rps_ObjectRef parentob, Rps_ObjectRef classob, Rps_ObjectRef spaceob)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED, callframe,
                 Rps_ObjectRef obparent;
                 Rps_ObjectRef obclass;
                 Rps_ObjectRef obspace;
                 Rps_ObjectRef obenv;
                );
  _f.obclass = classob;
  _f.obparent = parentob;
  _f.obspace = spaceob;
  RPS_ASSERT(classob);
  RPS_ASSERT(classob == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
             || classob->is_subclass_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)));
  RPS_ASSERT(!parentob || parentob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)));
  _f.obenv = Rps_ObjectRef::make_object(&_, _f.obclass, _f.obspace);
  auto paylenv = _f.obenv->put_new_plain_payload<Rps_PayloadEnvironment>();
  RPS_ASSERT(paylenv);
  paylenv->env_parent = parentob;
  return _f.obenv;
} // end Rps_PayloadEnvironment::make_with_parent_environment

Rps_Value
rps_environment_get_shallow_bound_value(Rps_ObjectRef envob, Rps_ObjectRef varob,
                                        bool *pmissing)
{
  if (envob.is_empty())
    {
      if (pmissing)
        *pmissing = true;
      return nullptr;
    }
  std::lock_guard gu(*envob->objmtxptr());
  bool isgoodenv = false;
  if (envob->get_class() == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)) //environment∈class
    isgoodenv = true;
  else if (envob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
    isgoodenv = true;
  if (!isgoodenv)
    {
      if (pmissing)
        *pmissing=true;
      return nullptr;
    };
  auto paylenv = envob->get_dynamic_payload<Rps_PayloadEnvironment>();
  if (!paylenv)
    {
      if (pmissing)
        *pmissing = true;
      return nullptr;
    };
  return paylenv->get_obmap(varob, nullptr, pmissing);
} // end rps_environment_get_shallow_bound_value

constexpr int rps_environment_maxloop = 4096;

int
rps_environment_find_binding_depth(Rps_ObjectRef envob, Rps_ObjectRef varob)
{
  int depth=0;
  int loopcnt = 0;
  Rps_ObjectRef firstenvob = envob;
  for(;;)
    {
      if (loopcnt++ > rps_environment_maxloop)
        {
          // this should never happen in practice....
          RPS_WARNOUT("rps_environment_find_binding_depth looping "
                      << loopcnt << " times for initial environment " << envob << " and variable " << varob
                      << std::endl <<  RPS_FULL_BACKTRACE_HERE(1, "rps_environment_find_binding_depth"));
          return -1;
        }
      if (envob.is_empty())
        {
          return -1;
        };
      std::lock_guard gu(*envob->objmtxptr());
      bool isgoodenv = false;
      if (envob->get_class() == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)) //environment∈class
        isgoodenv = true;
      else if (envob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
        isgoodenv = true;
      if (!isgoodenv)
        return -1;
      auto paylenv = envob->get_dynamic_payload<Rps_PayloadEnvironment>();
      if (!paylenv)
        return -1;
      if (paylenv->has_key_obmap(varob))
        return depth;
      depth++;
      envob = paylenv->get_parent_environment();
      if (!envob)
        return -1;
    };
} // end rps_environment_find_binding_depth



Rps_Value
rps_environment_find_bound_value(Rps_ObjectRef envob, Rps_ObjectRef varob,
                                 int*pdepth, Rps_ObjectRef*penvob)
{
  int depth=0;
  int loopcnt = 0;
  Rps_ObjectRef firstenvob = envob;
  for(;;)
    {
      if (loopcnt++ > rps_environment_maxloop)
        {
          // this should never happen in practice....
          RPS_WARNOUT("rps_environment_find_bound_value looping "
                      << loopcnt << " times for initial environment " << envob << " and variable " << varob
                      << std::endl <<  RPS_FULL_BACKTRACE_HERE(1, "rps_environment_find_binding_depth"));
          if (pdepth)
            *pdepth = -1;
          if (penvob)
            *penvob = envob;
          return nullptr;
        }
      if (envob.is_empty())
        {
          if (pdepth)
            *pdepth = -1;
          if (penvob)
            *penvob = nullptr;
          return nullptr;
        };
      std::lock_guard gu(*envob->objmtxptr());
      bool isgoodenv = false;
      if (envob->get_class() == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)) //environment∈class
        isgoodenv = true;
      else if (envob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
        isgoodenv = true;
      if (!isgoodenv)
        {
          if (pdepth)
            *pdepth = -1;
          if (penvob)
            *penvob = nullptr;
          return nullptr;
        }
      auto paylenv = envob->get_dynamic_payload<Rps_PayloadEnvironment>();
      if (!paylenv)
        {
          if (pdepth)
            *pdepth = -1;
          if (penvob)
            *penvob = nullptr;
          return nullptr;
        }
      if (Rps_Value v = paylenv->get_obmap(varob))
        {
          if (pdepth)
            *pdepth = depth;
          if (penvob)
            *penvob = envob;
          return v;
        }
      depth++;
      envob = paylenv->get_parent_environment();
      if (!envob)
        {
          if (pdepth)
            *pdepth = -1;
          if (penvob)
            *penvob = nullptr;
          return nullptr;
        }
    };
} // end rps_environment_find_bound_value


void
rps_environment_add_shallow_binding(Rps_CallFrame*callframe,
                                    Rps_ObjectRef envob, Rps_ObjectRef varob, Rps_Value val)
{
  RPS_ASSERT(callframe && callframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_ObjectRef envob;
                 Rps_ObjectRef varob;
                 Rps_Value valv;
                );
  _f.envob = envob;
  _f.varob = varob;
  _f.valv = val;
  if (!envob || envob.is_empty())
    return;
  if (!varob || varob.is_empty())
    return;
  std::lock_guard gu(*envob->objmtxptr());
  bool isgoodenv = false;
  if (envob->get_class() == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)) //environment∈class
    isgoodenv = true;
  else if (envob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
    isgoodenv = true;
  if (!isgoodenv)
    return;
  auto paylenv = envob->get_dynamic_payload<Rps_PayloadEnvironment>();
  if (!paylenv)
    return;
  paylenv->put_obmap(_f.varob, _f.valv);
} // end rps_environment_add_shallow_binding

/// overwrite a binding in the deep environment containing it, or when
/// not found in the current one.  Return affected depth.
int
rps_environment_overwrite_binding(Rps_CallFrame*callframe,
                                  Rps_ObjectRef envob,
                                  Rps_ObjectRef varob, Rps_Value val,
                                  Rps_ObjectRef*penvob)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callframe,
                 Rps_ObjectRef envob;
                 Rps_ObjectRef firstenvob;
                 Rps_ObjectRef varob;
                 Rps_Value valv;
                );
  _f.envob = envob;
  _f.firstenvob = envob;
  _f.varob = varob;
  _f.valv = val;
  int loopcnt = 0;
  int depth = 0;
  for(;;)
    {
      if (loopcnt++ > rps_environment_maxloop)
        {
          // this should never happen in practice....
          RPS_WARNOUT("rps_environment_overwrite_binding looping "
                      << loopcnt << " times for initial environment " << _f.firstenvob << " and variable " << _f.varob << " value " << _f.valv
                      << std::endl <<  RPS_FULL_BACKTRACE_HERE(1, "rps_environment_find_binding_depth"));
          if (penvob)
            *penvob = _f.envob;
          return -1;
        }
      if (_f.envob.is_empty())
        {
          if (penvob)
            *penvob = nullptr;
          return -1;
        };
      std::lock_guard gu(*_f.envob->objmtxptr());
      bool isgoodenv = false;
      if (_f.envob->get_class() == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)) //environment∈class
        isgoodenv = true;
      else if (_f.envob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
        isgoodenv = true;
      if (!isgoodenv)
        {
          if (penvob)
            *penvob = nullptr;
          return -1;
        }
      auto paylenv = envob->get_dynamic_payload<Rps_PayloadEnvironment>();
      if (!paylenv)
        {
          if (penvob)
            *penvob = nullptr;
          return -1;
        }
      if (Rps_Value v = paylenv->get_obmap(_f.varob))
        {
          if (penvob)
            *penvob = _f.envob;
          paylenv->put_obmap(_f.varob, _f.valv);
          return depth;
        }
      depth++;
      envob = paylenv->get_parent_environment();
      if (!envob)
        break;
    };
  std::lock_guard gu(*_f.firstenvob->objmtxptr());
  bool isgoodenv = false;
  if (_f.firstenvob->get_class() == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a)) //environment∈class
    isgoodenv = true;
  else if (_f.firstenvob->is_instance_of(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a))) //environment∈class
    isgoodenv = true;
  if (!isgoodenv)
    {
      if (penvob)
        *penvob = nullptr;
      return -1;
    };
  auto paylenv = _f.firstenvob->get_dynamic_payload<Rps_PayloadEnvironment>();
  if (!paylenv)
    {
      if (penvob)
        *penvob = nullptr;
      return -1;
    }
  paylenv->put_obmap(_f.varob, _f.valv);
  return 0;
} // end rps_environment_overwrite_binding


void
Rps_PayloadEnvironment::gc_mark(Rps_GarbageCollector&gc) const
{
  gc_mark_objmap(gc);
  if (env_parent)
    gc.mark_obj(env_parent);
} // end Rps_PayloadEnvironment::gc_mark

void
Rps_PayloadEnvironment::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du);
  dump_scan_objmap_internal(du);
  if (rps_is_dumpable_objref(du, env_parent))
    rps_dump_scan_object(du, env_parent);
} // end Rps_PayloadEnvironment::dump_scan

void
Rps_PayloadEnvironment::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT(du);
  jv["payload"] = "environment";
  dump_json_objmap_internal_content(du, jv);
  if (rps_is_dumpable_objref(du, env_parent))
    jv["parent_env"] = rps_dump_json_objectref(du, env_parent);
  else
    jv["parent_env"] = Json::nullValue;
} // end Rps_PayloadEnvironment::dump_json_content

void
Rps_PayloadEnvironment::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  /// most of the code below is "temporarily" duplicated from
  /// Rps_PayloadObjMap::output_payload in file morevalues_rps.cc
  /// we hope to later (in 2025?) have this C++ code generated at dump time
  RPS_ASSERT(depth <= maxdepth);
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> gudispob(*owner()->objmtxptr());
  int nbobjmap = (int) get_obmap_size();
  if (nbobjmap==0)
    out << BOLD_esc << "-empty environment-" << NORM_esc;
  else
    out << BOLD_esc << "-environment of " << nbobjmap
        << ((nbobjmap>1)?" entries":" entry");
  Rps_Value dv = get_descr();
  if (dv)
    out << " described by " << NORM_esc << Rps_OutputValue(dv, depth, maxdepth) << std::endl;
  else
    out << " plain" << NORM_esc << std::endl;
  std::vector<Rps_ObjectRef> attrvect(nbobjmap);
  do_each_obmap_entry<std::vector<Rps_ObjectRef>&>(attrvect,
      [&](std::vector<Rps_ObjectRef>&atvec,
          Rps_ObjectRef atob,
          [[unused]]Rps_Value, [[unused]]void*)
  {
    atvec.push_back(atob);
    return false;
  });
  rps_sort_object_vector_for_display(attrvect);
  for (int ix=0; ix<(int)nbobjmap; ix++)
    {
      const Rps_ObjectRef curattr = attrvect[ix];
      const Rps_Value curval = get_obmap(curattr);
      out << BOLD_esc << "*"
          << NORM_esc << curattr << ": "
          << Rps_OutputValue(curval, depth, maxdepth)
          << std::endl;
    };
  Rps_ObjectRef parenvob = get_parent_environment();
  if (!parenvob)
    out << BOLD_esc << "- no parent env -" << NORM_esc
        << std::endl;
  else
    out << BOLD_esc << "- parent env: " << NORM_esc;
  out << parenvob << BOLD_esc << "-" << NORM_esc << std::endl;
} // end Rps_PayloadEnvironment::output_payload



void
rpsldpy_environment (Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  auto paylenv = obz->put_new_plain_payload<Rps_PayloadEnvironment>();
  const Json::Value& jobmap = jv["objmap"];
  const Json::Value&  jdescr = jv["descr"];
  const Json::Value& jparent = jv["parent_env"];
  if (jobmap.type () == Json::objectValue)
    {
      auto membvec = jobmap.getMemberNames(); // vector of strings
      for (const std::string& keystr : membvec)
        {
          Rps_ObjectRef keyob(keystr, ld);
          Rps_Value val = Rps_Value(jobmap[keystr], ld);
          paylenv->put_obmap(keyob, val);
        }
    }
  paylenv->put_descr(Rps_Value(jdescr, ld));
  if (jparent)
    paylenv->env_parent = Rps_ObjectRef(jparent,ld);
} // end rpsldpy_environment

////////////////

void
Rps_CallFrame::interpret_repl_statement(Rps_ObjectRef stmtob,Rps_ObjectRef envob)
{
  RPS_FATALOUT("unimplemented Rps_CallFrame::interpret_repl_statement stmtob=" << stmtob << " envob=" << envob);
#warning unimplemented Rps_CallFrame::interpret_repl_statement
} // end Rps_CallFrame::interpret_repl_statement


void rps_interpret_repl_statement(Rps_CallFrame*callframe, Rps_ObjectRef stmtob,Rps_ObjectRef envob)
{
  RPS_ASSERT(callframe != nullptr && callframe->is_good_call_frame());
  RPS_ASSERT(stmtob);
  RPS_ASSERT(envob);
  callframe->interpret_repl_statement(stmtob, envob);
} // end rps_interpret_repl_statement


////////////////

Rps_TwoValues
Rps_CallFrame::evaluate_repl_expr(Rps_Value expr, Rps_ObjectRef envob)
{
  return rps_full_evaluate_repl_expr(this,expr,envob);
} // end Rps_CallFrame::evaluate_repl_expr



Rps_Value
Rps_CallFrame::evaluate_repl_expr1(Rps_Value expr, Rps_ObjectRef envob)
{
  return rps_simple_evaluate_repl_expr(this,expr,envob);
} // end Rps_CallFrame::evaluate_repl_expr1



/* C++ closure _61pgHb5KRq600RLnKD for REPL command dump parsing*/
extern "C" rps_applyingfun_t rpsapply_61pgHb5KRq600RLnKD;
Rps_TwoValues
rpsapply_61pgHb5KRq600RLnKD(Rps_CallFrame*callerframe, // REPL dump command
                            const Rps_Value arg0,
                            [[maybe_unused]] const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  //RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT_CALLFRAME (callerframe);
  static long callcnt;
  callcnt++;
  static Rps_Id descoid;
  if (!descoid) // this happens only once!
    descoid=Rps_Id("_61pgHb5KRq600RLnKD");
  RPS_DEBUG_LOG(REPL, "REPL command dump callcnt#" << callcnt
                << " descoid=" << descoid
                << " CALLED from:" << std::endl
                << Rps_ShowCallFrame(callerframe) << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "REPL command dump rpsapply_61pgHb5KRq600RLnKD"));
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_ObjectRef replcmdob;
                           Rps_ObjectRef lexkindob;
                           Rps_Value lexval;
                           Rps_Value closv;
                           Rps_Value lextokv;
                           Rps_Value nextokv;
                           Rps_ObjectRef lexob;
                           Rps_ObjectRef nextlexob;
                           Rps_Value nextlexval;
                );
  _f.closv = _.call_frame_closure();
  RPS_DEBUG_LOG(CMD, "REPL command dump start callcnt#" << callcnt << " arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) <<std::endl
                << " arg2=" << arg2 << " arg3=" << arg3 << std::endl
                << " callingclos=" << _f.closv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_)
                << "**calldepth=" << _.call_frame_depth()
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD/REPL cmd dump"));
  RPS_DEBUG_LOG(REPL, "REPL command dump start callcnt#" << callcnt << " arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) <<std::endl
                << " arg2=" << arg2 << " arg3=" << arg3 << std::endl
                << " callingclos=" << _f.closv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_)
                << "**calldepth=" << _.call_frame_depth()
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD/REPL cmd dump"));
  _f.replcmdob = arg0.to_object();
  _f.lextokv = arg1;
  RPS_ASSERT(_.call_frame_depth() < 7);
  RPS_DEBUG_LOG(CMD, "REPL command dump framedepth=" << _.call_frame_depth()
                << " lextokv=" << _f.lextokv
                <<" curframe:"
                << std::endl << Rps_ShowCallFrame(&_)
                << RPS_FULL_BACKTRACE_HERE(1, "REPL command dump rpsapply_61pgHb5KRq600RLnKD"));
  const Rps_LexTokenZone* ltokz = _f.lextokv.to_lextoken();
  RPS_ASSERT (ltokz != nullptr);
  {
    Rps_TokenSource*tksrc = ltokz->lxsrc();
    RPS_ASSERT (tksrc != nullptr);
    _f.nextokv = tksrc->get_token(&_);
    RPS_DEBUG_LOG(CMD, "REPL command dump callcnt#" << callcnt << " lexval=" << _f.lexval << " nextokv=" << _f.nextokv
                  << " framedepth=" << _.call_frame_depth());
    const Rps_LexTokenZone* nextokz = _f.nextokv.to_lextoken();
    RPS_ASSERT(nextokz);
    _f.nextlexob = nextokz->lxkind();
    _f.nextlexval = nextokz->lxval();
    RPS_DEBUG_LOG(CMD, "REPL command dump callcnt#" << callcnt << " lexval=" << _f.lexval << " nextokv=" << _f.nextokv
                  << " nextlexob=" << _f.nextlexob << " nextlexval=" << _f.nextlexval);
  }
  std::string dumpdir;
  bool dumped=false;
  RPS_DEBUG_LOG(CMD, "REPL command dump callcnt#" << callcnt << " lexob=" << _f.lexob << " lextokv=" << _f.lextokv
                << " nextlexob=" << _f.nextlexob << " nextlexval=" << _f.nextlexval
                << " framedepth=" << _.call_frame_depth() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "REPL command dump rpsapply_61pgHb5KRq600RLnKD /nextlex"));

  // Attempt to check if there are no more tokens following
  RPS_ASSERT (_f.nextlexval);
  const Rps_LexTokenZone* lastokzone = _f.nextlexval.to_lextoken();
  if (lastokzone != nullptr)
    RPS_FATALOUT("invalid REPL syntax for dump command");
  RPS_DEBUG_LOG(CMD, "REPL command dump dot callcnt#" << callcnt
                << " lexob=" << _f.lexob
                << " nextlexob=" << _f.nextlexob
                << " nextlexval=" << _f.nextlexval);
  ///
  if (_f.nextlexval.is_object() && _f.nextlexval.to_object()->oid() == Rps_Id("_78wsBiJhJj1025DIs1"))  // the dot "."∈repl_delimiter
    {
      RPS_DEBUG_LOG(CMD, "REPL command dump dot callcnt#" << callcnt
                    << " framedepth=" << _.call_frame_depth());
      // dump to current directory
      rps_dump_into(".", &_);
      dumpdir=".";
      dumped = true;
      RPS_DEBUG_LOG(CMD, "REPL command dumped  callcnt#" << callcnt << " into current directory callcnt#" << callcnt);
      return {_f.nextlexval, nullptr};
    }
  else if (_f.nextlexval.is_string()) //string∈class #
    {
      std::string dirstr = _f.nextlexval.as_cppstring();
      RPS_DEBUG_LOG(CMD, "REPL command dumping into '" << Rps_Cjson_String (dirstr) << "' callcnt#" << callcnt
                    << " framedepth=" << _.call_frame_depth());
      DIR* dirh = opendir(dirstr.c_str());
      if (dirh)
        {
          closedir(dirh);
          RPS_DEBUG_LOG(CMD, "REPL command dumping into existing dir '"
                        << Rps_Cjson_String (dirstr) << "' callcnt#" << callcnt);
          rps_dump_into(dirstr.c_str(), &_);
          dumpdir = dirstr;
          dumped = true;
        }
      else if (!mkdir(dirstr.c_str(), 0750))
        {
          RPS_DEBUG_LOG(CMD, "REPL command dumping into fresh dir '"
                        << Rps_Cjson_String (dirstr) << "' callcnt#" << callcnt);
          rps_dump_into(dirstr.c_str(), &_);
          dumpdir = dirstr;
          dumped = true;
        }
      else
#warning rpsapply_61pgHb5KRq600RLnKD should use wordexp(3) on the string
        // see https://man7.org/linux/man-pages/man3/wordexp.3.html
        RPS_WARNOUT("REPL command dump unimplemented into '" << dirstr << "' callcnt#" << callcnt);
    }
  RPS_DEBUG_LOG(CMD, "REPL command dump dumped= " << (dumped?"true":"false")
                << " dumpdir=" << dumpdir << " callcnt#" << callcnt<< " nextlexob:" << _f.nextlexval);
  if (dumped)
    return {Rps_StringValue(dumpdir),nullptr};
  else
    RPS_WARNOUT("non-dumped REPL token for command dump - dumpdir=" << dumpdir
                << " callcnt#" << callcnt<< " nextlexval" << _f.nextlexval);
#warning incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump
  RPS_WARNOUT("incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD for REPL command dump") << std::endl
              << " arg0=" << arg0 << " arg1=" << arg1 << " callcnt#" << callcnt
              << " nextlexob:" << _f.nextlexob << std::endl
              << " lextokv:" << _f.lextokv << ", nextokv:"  << _f.nextokv);
  RPS_FATALOUT("REPL command dump rpsapply_61pgHb5KRq600RLnKD incomplete, should test nextlexob=" << _f.nextlexob
               << " and nextlexval=" << _f.nextlexval);
  return {nullptr,nullptr};
} //end of rpsapply_61pgHb5KRq600RLnKD for REPL command dump



void
Rps_Object_Display::output_routine_addr(std::ostream&out, void*funaddr) const
{
  if (funaddr == nullptr)
    out << "⏚"; //U+23DA EARTH GROUND
  else if (funaddr == RPS_EMPTYSLOT) // should not happen....
    out << "⦱"; //U+29B1 EMPTY SET WITH OVERBAR
  else
    {
      Dl_info adinf;
      memset ((void*)&adinf, 0, sizeof(adinf));
      if (dladdr(funaddr, &adinf))
        {
          if (funaddr==adinf.dli_saddr)
            {
              out << "&" << adinf.dli_sname;
              const char*demangled = nullptr;
              if (adinf.dli_sname[0]=='_' && adinf.dli_sname[1])   // mangled C++ name
                {
                  int status = -1;
                  demangled  = abi::__cxa_demangle(adinf.dli_sname, nullptr, 0, &status);
                  if (demangled && status == 0 && demangled[0])
                    out << "≡" // U+2261 IDENTICAL TO
                        << demangled;
                  free((void*)demangled);
                }
              out << "=" << funaddr;
            }
          else
            {
              size_t delta=(const char*)funaddr - (const char*)adinf.dli_saddr;
              out << "&" << adinf.dli_sname << "+" << delta << "=" << funaddr;
            }
        }
      else
        out << "?" << funaddr;
      if (adinf.dli_fname) /// shared object name
        out << " in " << adinf.dli_fname;
    };
} // end Rps_Object_Display::output_routine_addr


void
rps_sort_object_vector_for_display(std::vector<Rps_ObjectRef>&vectobr)
{
  std::sort(vectobr.begin(), vectobr.end(),
            [](Rps_ObjectRef leftob, Rps_ObjectRef rightob)
  {
    return Rps_ObjectRef::compare_for_display
           (leftob,rightob)<0;
  });
} // end rps_sort_object_vector_for_display


//// called in practice by RPS_OBJECT_DISPLAY macro
void
Rps_Object_Display::output_display(std::ostream&out) const
{
  char obidbuf[32];
  memset (obidbuf, 0, sizeof(obidbuf));
#warning incomplete Rps_Object_Display::output_display should be moved to objects_rps.cc
  if (!_dispfile)
    return;
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  /*** FIXME:
   *
   * in practice we may need to define a special debugging output
   * stream, since in commit 31e7ab2efcce973 (may 2025) the ontty
   * above is always false because RPS_DEBUGNL_LOG_AT and RPS_DEBUG_AT
   * macros are declaring a local std::ostringstream...
   ***/
  RPS_POSSIBLE_BREAKPOINT();
  if (!_dispobref)
    {
      out << BOLD_esc << "__" << NORM_esc
          << " (*" << _dispfile << ":" << _displine << "*)" << std::endl;
      return;
    };
  /// We lock the displayed object to avoid other threads modifying it
  /// during the display.
  std::lock_guard<std::recursive_mutex> gudispob(*_dispobref->objmtxptr());
  _dispobref->oid().to_cbuf24(obidbuf);
  out  << std::endl
       << BOLD_esc
       << "{¤¤ object " << _dispobref
       << NORM_esc
       << std::endl << "  of class "
       << _dispobref->get_class()
       << std::endl;
  {
    Rps_ObjectRef obspace =  _dispobref->get_space();
    if (!obspace.is_empty())
      out <<  "¤ in space " << _dispobref->get_space() << std::endl;
    else
      out << BOLD_esc << "¤ temporary" << NORM_esc << " space"
          << std::endl;
  };
  double obmtim = _dispobref->get_mtime();
  {
    char mtimbuf[64];
    memset (mtimbuf, 0, sizeof(mtimbuf));
    rps_strftime_centiseconds(mtimbuf, sizeof(mtimbuf),
                              "%Y, %b, %d %H:%M:%S.__ %Z", obmtim);
    out   << BOLD_esc << "** mtime: " << mtimbuf
          << "   *hash:" << _dispobref->val_hash()
          << NORM_esc
          << std::endl;
  };
  //// °°°°°°°°°°° display function pointers .....
  rps_magicgetterfun_t* getfun = _dispobref->magic_getter_function();
  if (getfun)
    {
      out << BOLD_esc << "⊚ magic attribute getter function "
          << NORM_esc;
      output_routine_addr(out, reinterpret_cast<void*>(getfun));
    }
  rps_applyingfun_t*applfun = _dispobref->applying_function();
  if (applfun)
    {
      out << BOLD_esc << "⊚ applying function "
          << NORM_esc;
      output_routine_addr(out, reinterpret_cast<void*>(applfun));
    }
  //// °°°°°°°°°°° display physical attributes
  Rps_Value setphysattr = _dispobref->set_of_physical_attributes();
  if (setphysattr.is_empty())
    out << BOLD_esc
        << "** no physical attributes **"
        << NORM_esc << std::endl;
  else
    {
      RPS_ASSERT(setphysattr.is_set());
      const Rps_SetOb*physattrset = setphysattr.as_set();
      unsigned nbphysattr = physattrset->cardinal();
      if (nbphysattr == 1)
        {
          const Rps_ObjectRef thesingleattr = physattrset->at(0);
          RPS_ASSERT(thesingleattr);
          const Rps_Value thesingleval = _dispobref->get_physical_attr(thesingleattr);
          out<< BOLD_esc << "** one physical attribute **"
             << NORM_esc << std::endl;
          out << BOLD_esc << "*"
              << NORM_esc << thesingleattr << ": "
              << Rps_OutputValue(thesingleval, _dispdepth, disp_max_depth)
              << std::endl;
        }
      else
        {
          /// TODO: we need to sort physattrset in displayable order
          /// (alphabetically by name, else by objid), using
          /// Rps_ObjectRef::compare_for_display
          out<< BOLD_esc << "** "
             << nbphysattr << " physical attributes **"
             << NORM_esc << std::endl;
          std::vector<Rps_ObjectRef> attrvect(nbphysattr);
          for (int ix=0; ix<(int)nbphysattr; ix++)
            attrvect[ix] = physattrset->at(ix);
          rps_sort_object_vector_for_display(attrvect);
          for (int ix=0; ix<(int)nbphysattr; ix++)
            {
              const Rps_ObjectRef curattr = attrvect[ix];
              const Rps_Value curval =  _dispobref->get_physical_attr(curattr);
              out << BOLD_esc << "*"
                  << NORM_esc << curattr << ": "
                  << Rps_OutputValue(curval, _dispdepth, disp_max_depth)
                  << std::endl;
            }
        };
    };
  //// °°°°°°°°°°° display physical components
  unsigned nbphyscomp = _dispobref->nb_physical_components();
  if (nbphyscomp == 0)
    {
      out << BOLD_esc << "* no physical components *" << NORM_esc << std::endl;
    }
  else if (nbphyscomp == 1)
    {
      out << BOLD_esc << "* one physical component *" << NORM_esc << std::endl;
    }
  else
    {
      out << BOLD_esc << "* " << nbphyscomp << " physical components *"
          << NORM_esc << std::endl;
    }
  const std::vector<Rps_Value> vectcomp =
    _dispobref->vector_physical_components();
  for (unsigned ix=0; ix<nbphyscomp; ix++)
    {
      out << BOLD_esc << "[" << ix << "]" << NORM_esc << " "
          << Rps_OutputValue(vectcomp[ix], _dispdepth, disp_max_depth)
          << std::endl;
    };
  Rps_Payload*payl = _dispobref->get_payload();
  if (!payl)
    {
      out << BOLD_esc << "* no payload *" << NORM_esc << std::endl;
    }
  else
    {
      out << BOLD_esc << "* " << payl->payload_type_name() << " payload *"
          << NORM_esc << std::endl;
      payl->output_payload(out, _dispdepth, disp_max_depth);
    };
  char oidpref[16];
  memset (oidpref, 0, sizeof(oidpref));
  memcpy (oidpref, obidbuf, sizeof(oidpref)/2);
  out << " " << BOLD_esc << "|-" << oidpref << "¤¤}" << NORM_esc << std::endl;
} // end Rps_Object_Display::output_display




/***
 * The signature of this function was approved on whatsapp by Abhishek
 *  CHAKRAVARTI (India) on Fri July 7, 2023.
 *
 ** TODO: declare it in refpersys.hh
 **/
extern "C"
void rps_show_object_for_repl(Rps_CallFrame*callerframe,
                              const Rps_ObjectRef shownobarg,
                              std::ostream* pout,
                              unsigned depth)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(pout != nullptr);
  static Rps_Id showdescoid;
  if (!showdescoid)
    showdescoid=Rps_Id("_2wi3wsd8tVF01MBeeF"); // for the show∈symbol
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(showdescoid),
                           callerframe,
                           Rps_ObjectRef shownob;
                           Rps_ObjectRef curattrob;
                           Rps_Value curval;
                           Rps_Value subvalv;
                           Rps_Value attrsetv;
                );
  _f.shownob = shownobarg;
  if (!_f.shownob)
    {
      *pout << "__";
      return;
    }
  /// we lock the shown object to avoid other threads modifying it during the show.
  std::lock_guard<std::recursive_mutex> gushownob(*_f.shownob->objmtxptr());
  bool ontty =
    (pout == &std::cout)?isatty(STDOUT_FILENO):false;
  if (rps_without_terminal_escape)
    ontty=false;
  if (depth==0)
    {
#warning move this code then use Rps_Object_Display::output_display near line 968 above
      (*pout)
          << std::endl << std::endl << "================================" << std::endl
          << (ontty?RPS_TERMINAL_BOLD_ESCAPE:"")
          << "¤! object " << _f.shownob
          << (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"")
          << std::endl << "  of class "
          << _f.shownob->get_class()
          << std::endl
          << " in space " << _f.shownob->get_space() << std::endl;
      double obmtim = _f.shownob->get_mtime();
      {
        char mtimbuf[64];
        memset (mtimbuf, 0, sizeof(mtimbuf));
        rps_strftime_centiseconds(mtimbuf, sizeof(mtimbuf),
                                  "%Y, %b, %d %H:%M:%S.__ %Z", obmtim);
        (*pout) << "** mtime: " << mtimbuf
                << "   *hash:" << _f.shownob->val_hash()
                << std::endl;
      }
      unsigned nbat = _f.shownob->nb_attributes(&_);
      if (nbat == 0)
        (*pout) << "** without attributes **" << std::endl;
      else if (nbat == 1)
        (*pout) << "** with one attribute:" << std::endl;
      else
        {
          (*pout) << "** with " << nbat << " attributes:" << std::endl;
          _f.attrsetv = _f.shownob->set_of_attributes(&_);
          for (int aix = 0; aix < (int) nbat; aix++)
            {
              _f.curattrob = _f.attrsetv.as_set()->at(aix);
              if (!_f.curattrob)
                continue;
              _f.subvalv = _f.shownob->get_physical_attr(_f.curattrob);
              (*pout) << "* " << _f.curattrob << " : ";
              _f.subvalv.output((*pout), 0);
              (*pout) << std::endl;
            }
        }
      unsigned nbcomp = _f.shownob->nb_components(&_);
      if (nbcomp == 0)
        (*pout) << "** without components **" << std::endl;
      else
        {
          if (nbcomp == 1)
            (*pout) << "** with one component:" << std::endl;
          else
            (*pout) << "** with " << nbcomp << " components:" << std::endl;
          for (int cix=0; cix<(int)nbcomp; cix++)
            {
              _f.subvalv = _f.shownob->component_at(&_, cix);
              (*pout) << "[" << cix << "] ";
              _f.subvalv.output((*pout), 0);
              (*pout) << std::endl;
            }
        }
      Rps_Payload*payl = _f.shownob->get_payload();
      if (!payl)
        (*pout) << "** without payload **" << std::endl;
      else
        {
          Rps_Type typayl = payl->type();
          (*pout) << "** with payload of "
                  << _f.shownob->payload_type_name()
                  << " type#" << (int)typayl
                  << " **" << std::endl;
#warning we probably want to display some common payloads here
        }
      rps_applyingfun_t* apfun = _f.shownob->get_applying_ptrfun();
      if (!apfun)
        (*pout) << "** without applying function **" << std::endl;
      else
        {
          Dl_info appinfo;
          memset ((void*)&appinfo, 0, sizeof(appinfo));
          if (dladdr((void*)apfun,&appinfo))
            {
              (*pout)
                  << "** with applying function " << appinfo.dli_sname
                  << "@" << (void*)apfun
                  << " in " << appinfo.dli_fname << std::endl;
            }
          else
            (*pout) << "** with applying function unnamed @" << (void*)apfun << std::endl;
        };
      (*pout) << std::endl << std::endl;
    } // end if depth==0
  else
    {
      (*pout) <<  _f.shownob;
    };
} // end rps_show_object_for_repl




extern "C"
void rps_show_instance_for_repl(Rps_CallFrame*callerframe,
                                const Rps_InstanceValue arginst,
                                std::ostream* pout,
                                unsigned depth)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(pout != nullptr);
  static Rps_Id showdescoid;
  if (!showdescoid)
    showdescoid=Rps_Id("_2wi3wsd8tVF01MBeeF"); // for the show∈symbol
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(showdescoid),
                           callerframe,
                           Rps_InstanceValue inst;
                           Rps_ObjectRef obclass;
                           Rps_ObjectRef obmeta;
                           Rps_SetValue attrset;
                           Rps_ObjectRef curattrob;
                           Rps_Value curval;
                );
  int32_t metark=0;
  _f.inst = arginst;
  bool trans = _f.inst->is_transient();
  bool metatrans = _f.inst->is_metatransient();
  _f.obclass = _f.inst->get_class();
  _f.obmeta = _f.inst->metaobject();
  metark = _f.inst->metarank();
  _f.attrset = Rps_SetValue(_f.inst->set_attributes());
  bool ontty =
    (pout == &std::cout)?isatty(STDOUT_FILENO):false;
  if (rps_without_terminal_escape)
    ontty=false;
  if (depth==0)
    {
      (*pout)
          << std::endl << std::endl << "================================" << std::endl
          << (ontty?RPS_TERMINAL_BOLD_ESCAPE:"")
          << "¤¤ showing "
          << (trans?"transient":"permanent")
          << " instance "
          << (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"")
          << std::endl << "  of class "
          << _f.obclass
          << std::endl << " hash#" << _f.inst->val_hash()  << std::endl;
      if (metark || _f.obmeta)
        {
          (*pout) << (metatrans?"metatransient":"metadata")
                  << " µrk#" << metark << " µob:"
                  << _f.obmeta << std::endl;
        };
      int rk=0;
      for (auto catob : * _f.attrset.as_set())
        {
          _f.curattrob = catob;
          (*pout) << "°" << _f.curattrob;
          _f.curval = _f.inst->at(rk);
          (*pout) << ":" << _f.curval;
          (*pout) << std::endl;
          rk++;
        };

    }
  else   // depth >0
    {
      arginst->val_output(*pout, depth, Rps_Value::max_output_depth);
    };
} // end rps_show_instance_for_repl




////////////////
/* C++ function _7WsQyJK6lty02uz5KT for REPL command show*/
extern "C" rps_applyingfun_t rpsapply_7WsQyJK6lty02uz5KT;
Rps_TwoValues
rpsapply_7WsQyJK6lty02uz5KT(Rps_CallFrame*callerframe, // REPL command show expr
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_7WsQyJK6lty02uz5KT");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_Value a0;
                           Rps_Value a1;
                           Rps_ObjectRef replcmdob;
                           Rps_ObjectRef evalenvob;
                           Rps_ObjectRef shownob;
                           Rps_ObjectRef curattrob;
                           Rps_Value lextokv;
                           Rps_Value showv;
                           Rps_InstanceValue showninstv;
                           Rps_Value evalshowv;
                           Rps_SetValue attrsetv;
                           Rps_Value subvalv;
                );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
  });
  _f.a0 = arg0;
  _f.a1 = arg1;
  RPS_DEBUG_LOG(CMD, "CMD command show start a0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << ";" << std::endl << " a1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) <<std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command show start a0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << ";  a1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) <<std::endl
                << Rps_ShowCallFrame(&_)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT"));
  _f.replcmdob = _f.a0.as_object();
  _f.lextokv = _f.a1;
#warning REPL command show may need some local Rps_TokenSource
  RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6/started replcmdob:"
                << _f.replcmdob << " lextokv:" << _f.lextokv
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT"));
  RPS_ASSERT(_.call_frame_depth() < 7);
  RPS_DEBUG_LOG(CMD, "REPL command show framedepth=" << _.call_frame_depth()
                << " lextokv=" << _f.lextokv
                <<" curframe:"
                << std::endl << Rps_ShowCallFrame(&_)
                << RPS_FULL_BACKTRACE_HERE(1, "REPL command show rpsapply_7WsQyJK6lty02uz5KT"));
  const Rps_LexTokenZone* ltokz = _f.lextokv.to_lextoken();
  RPS_ASSERT (ltokz != nullptr);
  std::string showpos;
  {
    Rps_TokenSource*tksrc = ltokz->lxsrc();
    RPS_ASSERT (tksrc != nullptr);
    showpos = tksrc->position_str();
    RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6 tksrc:" << *tksrc << std::endl
                  << "… before parse_expression pos:" << showpos
                  << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                  << " token_deq:" << tksrc->token_dequeue());
    RPS_DEBUG_LOG(CMD, "REPL command show lextokv=" << _f.lextokv << " framedepth:"
                  << _.call_frame_depth()
                  << " before parse_expression");
    _f.lextokv = tksrc->get_token(&_);
    RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6 tksrc:" << *tksrc
                  << " before parse_expression pos:" << showpos
                  << " got-tok " << _f.lextokv
                  << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                  << " token_deq:" << tksrc->token_dequeue());
    RPS_DEBUG_LOG(CMD, "REPL command show got lextokv=" << _f.lextokv
                  << " from " << RPS_FULL_BACKTRACE_HERE(1, "REPL command show rpsapply_7WsQyJK6lty02uz5KT/gotnext"));
    if (_f.lextokv)
      {
        tksrc->append_back_new_token(&_, _f.lextokv);
        RPS_DEBUG_LOG(REPL, "rpsapply_7WsQyJK6lty02uz5KT for REPL command show tksrc becomes " << (*tksrc)
                      << std::endl
                      << "… curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                      << " token_deq:" << tksrc->token_dequeue()
                      << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_7WsQyJK6lty02uz5KT for REPL command show"));
      }
    RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6/before pars.expr. tksrc:" << (*tksrc) << " replcmdob:" << _f.replcmdob << std::endl
                  << " lextokv:" << _f.lextokv
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT/before parsexp")
                  << std::endl << "… before parse_expression token_deq:"
                  << tksrc->token_dequeue()
                  << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr()) << std::endl);
    bool okparsexp = false;
    _f.showv = tksrc->parse_expression(&_,&okparsexp);
    if (!okparsexp)
      {
        RPS_WARNOUT("command show°_7WsQyJK6 failed to parse expression in " << (*tksrc)
                    << std::endl
                    << " replcmdob:" << _f.replcmdob << std::endl
                    << "… token_deq:" << tksrc->token_dequeue()
                    << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                    << " lextokv:" << _f.lextokv << " showv:" << _f.showv
                    << std::endl
                    << Rps_Do_Output([&](std::ostream& out)
        {
          tksrc->display_current_line_with_cursor(out);
        })
            << std::endl
            << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT/fail parsexp")
            << std::endl);
        return  {nullptr,nullptr};
      };
    if (!_f.evalenvob)
      _f.evalenvob = rps_get_first_repl_environment();
    RPS_DEBUG_LOG(CMD, "REPL command show lextokv=" << _f.lextokv << " framedepth:"<< _.call_frame_depth()
                  << " after successful parse_expression showv=" << _f.showv);
    RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6/after pars.expr. tksrc:" << (*tksrc) << std::endl
                  << "… replcmdob:" << _f.replcmdob << std::endl
                  << "… token_deq:" << tksrc->token_dequeue()
                  << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                  << " lextokv:" << _f.lextokv << " should evaluate showv:" << _f.showv
                  << " in evalenvob:" << _f.evalenvob
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT/after parsexp")
                  << std::endl);
    _f.evalshowv = rps_simple_evaluate_repl_expr(&_,
                   _f.showv, _f.evalenvob);
    std::cout << "##" << RPS_TERMINAL_BOLD_ESCAPE << showpos
              << RPS_TERMINAL_NORMAL_ESCAPE << " : "
              << _f.showv << std::endl;
    std::cout << std::endl
              << "¤¤¤¤¤¤ SHOW expr. " << _f.showv << std::endl
              << " in environment " << _f.evalenvob << std::endl
              << " evaluated to " << _f.evalshowv << std::endl;
    if (_f.evalshowv.is_object())
      {
#warning this code should be moved into  rps_show_object_for_repl above
        _f.shownob = _f.evalshowv.as_object();
        rps_show_object_for_repl(&_,_f.shownob, &std::cout, 0);
      }
    else if (_f.evalshowv.is_instance())
      {
        _f.showninstv = Rps_InstanceValue(_f.evalshowv.as_instance());
        rps_show_instance_for_repl(&_, _f.showninstv, &std::cout, 0);
      }
    if (_f.showv || _f.evalshowv)
      return {_f.evalshowv, _f.showv};
    else
      return {nullptr, RPS_ROOT_OB(_9uwZtDshW4401x6MsY)}; //space∈symbol
  }
#warning incomplete rpsapply_7WsQyJK6lty02uz5KT for REPL command show
  RPS_WARNOUT("incomplete rpsapply_7WsQyJK6lty02uz5KT for REPL command show  of " << _f.showv << " from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_7WsQyJK6lty02uz5KT for REPL command show"));
  return {nullptr,nullptr};
} //end of rpsapply_7WsQyJK6lty02uz5KT for REPL command show


Rps_ObjectRef
rps_get_first_repl_environment(void)
{
  return RPS_ROOT_OB(_1Io89yIORqn02SXx4p) //RefPerSys_system∈the_system_class
         ->get_physical_attr(RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
                            ).as_object();
} // end rps_get_first_repl_environment

/* C++ function _2TZNwgyOdVd001uasl for REPL command help*/
extern "C" rps_applyingfun_t rpsapply_2TZNwgyOdVd001uasl;
Rps_TwoValues
rpsapply_2TZNwgyOdVd001uasl(Rps_CallFrame*callerframe, /// REPL command help
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_2TZNwgyOdVd001uasl");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_Value a0;
                           Rps_Value a1;
                           Rps_ObjectRef cmdob;
                           Rps_Value tokenv;
                           Rps_ObjectRef obdictcmd;
                           Rps_Value curcmdv;
                );
  _f.a0 = arg0;
  _f.a1 = arg1;
  _f.obdictcmd = RPS_ROOT_OB(_5dkRQtwGUHs02MVQT0);
  RPS_DEBUG_LOG(CMD, "REPL command help start a0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << " a1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) << std::endl
                << " obdictcmd=" << _f.obdictcmd
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  if (!RPS_DEBUG_ENABLED(CMD))
    {
      RPS_DEBUG_LOG(REPL, "REPL command help° start 0=" << _f.a0
                    << "∈" << _f.a0.compute_class(&_)
                    << " a1=" << _f.a1
                    << "∈" << _f.a1.compute_class(&_) << std::endl
                    << " arg2=" << arg2 << " arg3=" << arg3
                    << " obdictcmd=" << _f.obdictcmd
                    << (restargs?" restargs=":" no restargs:")
                    << (restargs?(*restargs):std::vector<Rps_Value>())
                    << " from " << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "REPL command help°"));
    };
  _f.cmdob = _f.a0.as_object();
  RPS_ASSERT(_f.obdictcmd);
  std::lock_guard<std::recursive_mutex> gu(*_f.obdictcmd->objmtxptr());
  auto payldict = _f.obdictcmd->get_dynamic_payload<Rps_PayloadStringDict>();
  RPS_ASSERT(payldict);
  payldict->iterate_with_callframe(&_, [&](Rps_CallFrame*cf,
                                   const std::string&name,
                                   const Rps_Value val)
  {
    /* TODO: we probably need some internal rps_LOCALFRAME right here! */
#warning incomplete code inside rpsapply_2TZNwgyOdVd001uasl for REPL command help
    _f.curcmdv = val;
    if (_f.curcmdv.is_object())
      {
      }
    return false;
  });
#warning incomplete rpsapply_2TZNwgyOdVd001uasl for REPL command help
  RPS_WARNOUT("incomplete rpsapply_2TZNwgyOdVd001uasl for REPL command help from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_2TZNwgyOdVd001uasl for REPL command help"));
  return {nullptr,nullptr};
} //end of rpsapply_2TZNwgyOdVd001uasl for REPL command help



/* C++ function _28DGtmXCyOX02AuPLd for REPL command put*/
extern "C" rps_applyingfun_t rpsapply_28DGtmXCyOX02AuPLd;
Rps_TwoValues
rpsapply_28DGtmXCyOX02AuPLd(Rps_CallFrame*callerframe, // REPL command put dest index newval
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_28DGtmXCyOX02AuPLd");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_Value a0;
                           Rps_Value a1;
                           Rps_Value a2;
                           Rps_ObjectRef ob0;
                           Rps_ObjectRef ob1;
                           Rps_ObjectRef obenv;
                           Rps_ObjectRef obdest;
                           Rps_ObjectRef obindex;
                           Rps_Value vdest;
                           Rps_Value vindex;
                           Rps_Value vnewval;
                           Rps_Value voldval;
                );
  _f.a0 = arg0;
  _f.a1 = arg1;
  _f.a2 = arg2;
  _f.obenv = rps_get_first_repl_environment();
  RPS_DEBUG_LOG(CMD, "REPL command put start arg0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << " arg1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) << std::endl
                << " arg2=" << _f.a2
                << "∈" << _f.a2.compute_class(&_) << std::endl
                << " obenv:" << _f.obenv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command put start arg0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << " arg1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) << std::endl
                << " arg2=" << _f.a2
                << "∈" << _f.a2.compute_class(&_) << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  _f.vdest = rps_simple_evaluate_repl_expr(&_, _f.a0, _f.obenv);
  RPS_DEBUG_LOG(REPL, "REPL command put destination vdest=" << _f.vdest);
  _f.obdest = _f.vdest.as_object();
  if (!_f.obdest)
    {
      RPS_WARNOUT("in REPL command put the destination vdest=" << _f.vdest << " is not an object" << std::endl
                  << "index expression being a1=" << _f.a1);
      return {nullptr,nullptr};
    }
  std::lock_guard<std::recursive_mutex> guobdest(*_f.obdest->objmtxptr());
  _f.vindex = rps_simple_evaluate_repl_expr(&_, _f.a1, _f.obenv);
  RPS_DEBUG_LOG(REPL, "REPL command put destination vdest=" << _f.vdest << " index vindex=" << _f.vindex);
  _f.vnewval = rps_simple_evaluate_repl_expr(&_, _f.a2, _f.obenv);
  RPS_DEBUG_LOG(REPL, "REPL command put destination vdest=" << _f.vdest  << " index vindex=" << _f.vindex
                << " newvalue vnewval=" << _f.vnewval);
  if (_f.vindex.is_object())
    {
      _f.obindex = _f.vindex.as_object();
      if (_f.vnewval)
        {
          _f.obdest->put_attr(_f.obindex, _f.vnewval);
          RPS_INFORMOUT("REPL command put obdest=" << _f.obdest << " attribute:" << _f.obindex
                        << " new value:" << _f.vnewval);
          return {_f.obdest, _f.vnewval};
        }
      else
        {
          _f.voldval = _f.obdest->get_attr1(&_, _f.obindex);
          _f.obdest->remove_attr(_f.obindex);
          RPS_INFORMOUT("REPL command put obdest=" << _f.obdest
                        << " removed attribute:" << _f.obindex
                        << " old value was:" << _f.voldval);
          return {_f.obdest, _f.voldval};
        }
    }
  else if (_f.vindex.is_int())
    {
      intptr_t ix = _f.vindex.as_int();
      _f.voldval = _f.obdest->component_at (&_, (int)ix);
      _f.obdest->replace_component_at(&_, (int)ix, _f.vnewval);
      RPS_INFORMOUT("REPL command replaced in obdest=" << _f.obdest << " at index " << ix
                    << " component by " << _f.vnewval << " old value was " << _f.voldval);
      return {_f.obdest, _f.voldval};
    }
  else
    {
      RPS_WARNOUT("in REPL command put obdest=" << _f.obdest  << std::endl
                  << "with invalid index (not object or integer) being vindex=" << _f.vindex);
      return {nullptr,nullptr};
    }
#warning incomplete rpsapply_28DGtmXCyOX02AuPLd for REPL command put
  RPS_WARNOUT("rpsapply_28DGtmXCyOX02AuPLd for REPL command put obdest=" << _f.obdest
              << " index=" << _f.vindex
              << " called from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_28DGtmXCyOX02AuPLd for REPL command put"));
  return {nullptr,nullptr};
} //end of rpsapply_28DGtmXCyOX02AuPLd for REPL command put



/* C++ function _09ehnxiXQKo006cZer for REPL command remove*/
extern "C" rps_applyingfun_t rpsapply_09ehnxiXQKo006cZer;
Rps_TwoValues
rpsapply_09ehnxiXQKo006cZer(Rps_CallFrame*callerframe, // REPL command remove dest index
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_09ehnxiXQKo006cZer");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_Value a0;
                           Rps_Value a1;
                           Rps_Value destv;
                           Rps_Value indexv;
                           Rps_ObjectRef obenv;
                           Rps_ObjectRef obdest;
                           Rps_ObjectRef obattr;
                );
  _f.a0 = arg0;
  _f.a1 = arg1;
  _f.obenv = rps_get_first_repl_environment();
  RPS_DEBUG_LOG(CMD, "REPL command remove start arg0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << " arg1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) << std::endl
                << " obenv="<< _f.obenv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command remove start arg0=" << _f.a0
                << "∈" << _f.a0.compute_class(&_)
                << " arg1=" << _f.a1
                << "∈" << _f.a1.compute_class(&_) << std::endl
                << " obenv="<< _f.obenv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  _f.destv = rps_simple_evaluate_repl_expr(&_, _f.a0, _f.obenv);
  RPS_DEBUG_LOG(REPL, "REPL command remove destination destv=" << _f.destv);
  _f.obdest = _f.destv.as_object();
  if (!_f.obdest)
    {
      RPS_WARNOUT("in REPL command remove the destination destv="
                  << _f.destv << " is not an object" << std::endl
                  << "index expression being a1=" << _f.a1);
      return {nullptr,nullptr};
    }
  std::lock_guard<std::recursive_mutex> guobdest(*_f.obdest->objmtxptr());
  _f.indexv = rps_simple_evaluate_repl_expr(&_, _f.a1, _f.obenv);
  RPS_DEBUG_LOG(REPL, "REPL command remove destination destv=" << _f.destv << " indexv=" << _f.indexv);
#warning incomplete rpsapply_09ehnxiXQKo006cZer for REPL command remove
  RPS_WARNOUT("incomplete rpsapply_09ehnxiXQKo006cZer for REPL command remove from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_09ehnxiXQKo006cZer for REPL command remove"));
  return {nullptr,nullptr};
} //end of rpsapply_09ehnxiXQKo006cZer for REPL command remove



/* C++ function _9LCCu7TQI0Z0166mw3 for REPL command append*/
extern "C" rps_applyingfun_t rpsapply_9LCCu7TQI0Z0166mw3;
Rps_TwoValues
rpsapply_9LCCu7TQI0Z0166mw3(Rps_CallFrame*callerframe, /// REPL command append dest comp
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_9LCCu7TQI0Z0166mw3");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_Value a0dest;
                           Rps_Value a1comp;
                );
  _f.a0dest = arg0;
  _f.a1comp = arg1;
  RPS_DEBUG_LOG(CMD, "REPL command append start a0dest=" << _f.a0dest
                << "∈" << _f.a0dest.compute_class(&_)
                << " a1comp=" << _f.a1comp
                << "∈" << _f.a1comp.compute_class(&_)
                << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command append start a0dest=" << _f.a0dest
                << "∈" << _f.a0dest.compute_class(&_)
                << " a1comp=" << _f.a1comp
                << "∈" << _f.a1comp.compute_class(&_)
                << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_9LCCu7TQI0Z0166mw3 for REPL command append
  RPS_WARNOUT("incomplete rpsapply_9LCCu7TQI0Z0166mw3 for REPL command append from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_9LCCu7TQI0Z0166mw3 for REPL command append"));
  return {nullptr,nullptr};
} //end of rpsapply_9LCCu7TQI0Z0166mw3 for REPL command append



/* C++ function _982LHCTfHdC02o4a6Q for REPL command add_root*/
extern "C" rps_applyingfun_t rpsapply_982LHCTfHdC02o4a6Q;
Rps_TwoValues
rpsapply_982LHCTfHdC02o4a6Q(Rps_CallFrame*callerframe, /// REPL command add_root
                            const Rps_Value arg0,
                            [[maybe_unused]] const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_982LHCTfHdC02o4a6Q");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                           Rps_Value a0rootexp;
                           Rps_Value rootv;
                           Rps_ObjectRef obenv;
                           Rps_ObjectRef obroot;
                );
  _f.a0rootexp = arg0;
  _f.obenv = rps_get_first_repl_environment();
  RPS_DEBUG_LOG(CMD, "REPL command add_root start a0rootexp=" << _f.a0rootexp
                << "∈" <<_f.a0rootexp.compute_class(&_)
                << " obenv=" << _f.obenv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command add_root start a0rootexp=" << _f.a0rootexp
                << "∈" << _f.a0rootexp.compute_class(&_)
                << " obenv=" << _f.obenv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  _f.rootv = rps_simple_evaluate_repl_expr(&_, _f.a0rootexp, _f.obenv);
  RPS_DEBUG_LOG(REPL, "REPL command add_root rootv=" << _f.rootv);
  _f.obroot = _f.rootv.as_object();
  if (!_f.obroot)
    {
      RPS_WARNOUT("in REPL command add_root the evaluated rootv="
                  << _f.rootv << " is not an object" << std::endl);
      return {nullptr,nullptr};
    }
  std::lock_guard<std::recursive_mutex> guobdest(*_f.obroot->objmtxptr());
  rps_add_root_object(_f.obroot);
  RPS_INFORMOUT("successfully added new root object " << _f.obroot);
  return {_f.rootv,nullptr};
} //end of rpsapply_982LHCTfHdC02o4a6Q for REPL command add_root


/* C++ function _2G5DNSyfWoP002Vv6X for REPL command remove_root*/
extern "C" rps_applyingfun_t rpsapply_2G5DNSyfWoP002Vv6X;
Rps_TwoValues
rpsapply_2G5DNSyfWoP002Vv6X(Rps_CallFrame*callerframe, // REPL command remove_root
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_2G5DNSyfWoP002Vv6X");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                );
  RPS_DEBUG_LOG(CMD, "REPL command remove_root start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_)
                << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command remove_root start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root
  RPS_WARNOUT("incomplete rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root"));
  return {nullptr,nullptr};
} //end of rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root

/* C++ function _55RPnvwSLXz028jyDk for REPL command make_symbol*/
extern "C" rps_applyingfun_t rpsapply_55RPnvwSLXz028jyDk;
Rps_TwoValues
rpsapply_55RPnvwSLXz028jyDk(Rps_CallFrame*callerframe, // REPL make_symbol
                            const Rps_Value arg0,
                            [[maybe_unused]] const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_55RPnvwSLXz028jyDk");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                );
  RPS_DEBUG_LOG(CMD, "REPL command make_symbol start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_)
                << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command make_symbol start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_)
                << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol
  RPS_WARNOUT("incomplete rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol"));
  return {nullptr,nullptr};
} //end of rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol




//// end of file cmdrepl_rps.cc

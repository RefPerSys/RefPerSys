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
 *      © Copyright 2021 The Reflective Persistent System Team
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

extern "C" const char rps_cmdrepl_gitid[];
const char rps_repl_gitid[]= RPS_GITID;

extern "C" const char rps_cmdrepl_date[];
const char rps_cmdrepl_date[]= __DATE__;

/* C++ closure _61pgHb5KRq600RLnKD for REPL command dump parsing*/
extern "C" rps_applyingfun_t rpsapply_61pgHb5KRq600RLnKD;
Rps_TwoValues
rpsapply_61pgHb5KRq600RLnKD(Rps_CallFrame*callerframe,
                            const Rps_Value arg0,
                            [[maybe_unused]] const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_DEBUG_LOG(REPL, "REPL command dump CALLED from " << Rps_ShowCallFrame(callerframe));
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_61pgHb5KRq600RLnKD");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
		 callerframe,
		 Rps_ObjectRef replcmdob;
		 Rps_ObjectRef lexkindob;
		 Rps_Value lexval;
		 Rps_Value closv;
		 Rps_ObjectRef lexob;
		 );
  _f.closv = _.call_frame_closure();
  RPS_DEBUG_LOG(CMD, "REPL command dump start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) <<std::endl
                << " arg2=" << arg2 << " arg3=" << arg3 << std::endl
                << " callingclos=" << _f.closv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_)
                << std::endl << "**calldepth=" << _.call_frame_depth()
		<< std::endl << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD/REPL cmd dump"));
  RPS_ASSERT(_.call_frame_depth() < 10);
  _f.lexval = rps_repl_cmd_lexer_fun(&_, 0);
  RPS_DEBUG_LOG(CMD, "REPL command dump lexval=" << _f.lexval);
  if (_f.lexval.is_object())
    _f.lexob = _f.lexval.to_object();
  std::string dumpdir;
  bool eaten=false;
  if (_f.lexob && _f.lexob->oid() == Rps_Id("_78wsBiJhJj1025DIs1"))  // the dot "."∈repl_delimiter
    {
      RPS_DEBUG_LOG(CMD, "REPL command dump dot consuming....");
      RPS_ASSERT(rps_repl_consume_cmd_token_fun);
      bool eaten = rps_repl_consume_cmd_token_fun(&_);
      RPS_DEBUG_LOG(CMD, "REPL command dump dot eaten= " << (eaten?"true":"false"));
      // dump to current directory
      rps_dump_into(".", &_);
      dumpdir=".";
      RPS_DEBUG_LOG(CMD, "REPL command dumped into current directory");
      return {_f.lexob, nullptr};
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE)) //string∈class #
    {
      std::string dirstr = _f.lexval.as_cppstring();
      RPS_DEBUG_LOG(CMD, "REPL command dumping into " << dirstr);
      RPS_ASSERT(rps_repl_consume_cmd_token_fun);
      eaten = rps_repl_consume_cmd_token_fun(&_);
      RPS_DEBUG_LOG(CMD, "REPL command dump into " << dirstr << " eaten= " << (eaten?"true":"false"));
      DIR* dirh = opendir(dirstr.c_str());
      if (dirh)
        {
          closedir(dirh);
          rps_dump_into(dirstr.c_str(), &_);
          dumpdir = dirstr;
        }
      else if (!mkdir(dirstr.c_str(), 0750))
        {
          rps_dump_into(dirstr.c_str(), &_);
          dumpdir = dirstr;
        }
      else
#warning rpsapply_61pgHb5KRq600RLnKD should use wordexp(3) on the string
        // see https://man7.org/linux/man-pages/man3/wordexp.3.html
        RPS_WARNOUT("REPL command dump unimplemented into " << dirstr);
    }
  RPS_DEBUG_LOG(CMD, "REPL command dump eaten= " << (eaten?"true":"false") << " dumpdir=" << dumpdir);
  if (eaten)
    return {Rps_StringValue(dumpdir),nullptr};
  else
    RPS_WARNOUT("unconsumed REPL token for command dump - dumpdir=" << dumpdir);
#warning incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump
  RPS_WARNOUT("incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD for REPL command dump") << std::endl
              << " arg0=" << arg0 << " arg1=" << arg1);
  return {nullptr,nullptr};
} //end of rpsapply_61pgHb5KRq600RLnKD for REPL command dump




/* C++ function _7WsQyJK6lty02uz5KT for REPL command show*/
extern "C" rps_applyingfun_t rpsapply_7WsQyJK6lty02uz5KT;
Rps_TwoValues
rpsapply_7WsQyJK6lty02uz5KT(Rps_CallFrame*callerframe,
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
                );
  RPS_DEBUG_LOG(CMD, "REPL command show start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << ";  arg1=" << arg1
                << "∈" << arg1.compute_class(&_) <<std::endl
                << ";  arg2=" << arg2
                << "∈" << arg2.compute_class(&_)
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_7WsQyJK6lty02uz5KT for REPL command show
  RPS_WARNOUT("incomplete rpsapply_7WsQyJK6lty02uz5KT for REPL command show from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_7WsQyJK6lty02uz5KT for REPL command show"));
  return {nullptr,nullptr};
} //end of rpsapply_7WsQyJK6lty02uz5KT for REPL command show


/* C++ function _2TZNwgyOdVd001uasl for REPL command help*/
extern "C" rps_applyingfun_t rpsapply_2TZNwgyOdVd001uasl;
Rps_TwoValues
rpsapply_2TZNwgyOdVd001uasl(Rps_CallFrame*callerframe,
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
                );
  RPS_DEBUG_LOG(CMD, "REPL command help start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_2TZNwgyOdVd001uasl for REPL command help
  RPS_WARNOUT("incomplete rpsapply_2TZNwgyOdVd001uasl for REPL command help from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_2TZNwgyOdVd001uasl for REPL command help"));
  return {nullptr,nullptr};
} //end of rpsapply_2TZNwgyOdVd001uasl for REPL command help



/* C++ function _28DGtmXCyOX02AuPLd for REPL command put*/
extern "C" rps_applyingfun_t rpsapply_28DGtmXCyOX02AuPLd;
Rps_TwoValues
rpsapply_28DGtmXCyOX02AuPLd(Rps_CallFrame*callerframe,
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_28DGtmXCyOX02AuPLd");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                );
  RPS_DEBUG_LOG(CMD, "REPL command put start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_28DGtmXCyOX02AuPLd for REPL command put
  RPS_WARNOUT("incomplete rpsapply_28DGtmXCyOX02AuPLd for REPL command put from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_28DGtmXCyOX02AuPLd for REPL command put"));
  return {nullptr,nullptr};
} //end of rpsapply_28DGtmXCyOX02AuPLd for REPL command put



/* C++ function _09ehnxiXQKo006cZer for REPL command remove*/
extern "C" rps_applyingfun_t rpsapply_09ehnxiXQKo006cZer;
Rps_TwoValues
rpsapply_09ehnxiXQKo006cZer(Rps_CallFrame*callerframe,
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
                );
  RPS_DEBUG_LOG(CMD, "REPL command remove start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_09ehnxiXQKo006cZer for REPL command remove
  RPS_WARNOUT("incomplete rpsapply_09ehnxiXQKo006cZer for REPL command remove from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_09ehnxiXQKo006cZer for REPL command remove"));
  return {nullptr,nullptr};
} //end of rpsapply_09ehnxiXQKo006cZer for REPL command remove



/* C++ function _9LCCu7TQI0Z0166mw3 for REPL command append*/
extern "C" rps_applyingfun_t rpsapply_9LCCu7TQI0Z0166mw3;
Rps_TwoValues
rpsapply_9LCCu7TQI0Z0166mw3(Rps_CallFrame*callerframe,
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
                );
  RPS_DEBUG_LOG(CMD, "REPL command append start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_)
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
rpsapply_982LHCTfHdC02o4a6Q(Rps_CallFrame*callerframe,
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            [[maybe_unused]] const Rps_Value arg2,
                            [[maybe_unused]] const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  static Rps_Id descoid;
  if (!descoid) descoid=Rps_Id("_982LHCTfHdC02o4a6Q");
  RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                           callerframe,
                );
  RPS_DEBUG_LOG(CMD, "REPL command add_root start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << " arg1=" << arg1
                << "∈" << arg1.compute_class(&_) << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_982LHCTfHdC02o4a6Q for REPL command add_root
  RPS_WARNOUT("incomplete rpsapply_982LHCTfHdC02o4a6Q for REPL command add_root from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_982LHCTfHdC02o4a6Q for REPL command add_root"));
  return {nullptr,nullptr};
} //end of rpsapply_982LHCTfHdC02o4a6Q for REPL command add_root


/* C++ function _2G5DNSyfWoP002Vv6X for REPL command remove_root*/
extern "C" rps_applyingfun_t rpsapply_2G5DNSyfWoP002Vv6X;
Rps_TwoValues
rpsapply_2G5DNSyfWoP002Vv6X(Rps_CallFrame*callerframe,
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
#warning incomplete rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root
  RPS_WARNOUT("incomplete rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root"));
  return {nullptr,nullptr};
} //end of rpsapply_2G5DNSyfWoP002Vv6X for REPL command remove_root

/* C++ function _55RPnvwSLXz028jyDk for REPL command make_symbol*/
extern "C" rps_applyingfun_t rpsapply_55RPnvwSLXz028jyDk;
Rps_TwoValues
rpsapply_55RPnvwSLXz028jyDk(Rps_CallFrame*callerframe,
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
#warning incomplete rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol
  RPS_WARNOUT("incomplete rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol"));
  return {nullptr,nullptr};
} //end of rpsapply_55RPnvwSLXz028jyDk for REPL command make_symbol



Rps_Value
rps_repl_parse_value_expression(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           callframe,
                           Rps_Value expval;
                           Rps_ObjectRef lexkind;
                           Rps_Value lexval;
                           Rps_LexTokenValue lextokv;
                );
  {
    Rps_TwoValues twov = rps_repl_cmd_lexing(&_,0);
    _f.lexkind = twov.main().as_object();
    _f.lexval = twov.xtra();
    RPS_DEBUG_LOG(REPL, "rps_repl_parse_value_expression lexkind=" << _f.lexkind
                  << " lexval=" << _f.lexval << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_parse_value_expression"));
  }
#warning rps_repl_parse_value_expression is incomplete
  RPS_WARNOUT("incomplete rps_repl_parse_value_expression from " << std::endl
              <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_parse_value_expression") << std::endl
              << Rps_ShowCallFrame(&_));
  return _f.expval;
} // end rps_repl_parse_value_expression


Rps_Value
rps_repl_parse_object_expression(Rps_CallFrame*callframe)
{
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           callframe,
                           Rps_Value obexpval;
                           Rps_ObjectRef lexkind;
                           Rps_Value lexval;
                           Rps_LexTokenValue lextokv;
                );
  {
    Rps_TwoValues twov = rps_repl_cmd_lexing(&_,0);
    _f.lexkind = twov.main().as_object();
    _f.lexval = twov.xtra();
    RPS_DEBUG_LOG(REPL, "rps_repl_parse_object_expression lexkind=" << _f.lexkind
                  << " lexval=" << _f.lexval << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_repl_parse_object_expression"));
  }
#warning rps_repl_parse_object_expression is incomplete
  RPS_WARNOUT("incomplete rps_repl_parse_object_expression from " << std::endl
              <<  RPS_FULL_BACKTRACE_HERE(1, "rps_repl_parse_object_expression") << std::endl
              << Rps_ShowCallFrame(&_));
  return _f.obexpval;
} // end rps_repl_parse_object_expression


//// end of file cmdrepl_rps.cc

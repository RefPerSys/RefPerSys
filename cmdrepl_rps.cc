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
 *      © Copyright 2021 - 2022 The Reflective Persistent System Team
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
          RPS_DEBUG_LOG(CMD, "REPL command dumping into existing dir '" << Rps_Cjson_String (dirstr) << "' callcnt#" << callcnt);
          rps_dump_into(dirstr.c_str(), &_);
          dumpdir = dirstr;
          dumped = true;
        }
      else if (!mkdir(dirstr.c_str(), 0750))
        {
          RPS_DEBUG_LOG(CMD, "REPL command dumping into fresh dir '" << Rps_Cjson_String (dirstr) << "' callcnt#" << callcnt);
          rps_dump_into(dirstr.c_str(), &_);
          dumpdir = dirstr;
          dumped = true;
        }
      else
#warning rpsapply_61pgHb5KRq600RLnKD should use wordexp(3) on the string
        // see https://man7.org/linux/man-pages/man3/wordexp.3.html
        RPS_WARNOUT("REPL command dump unimplemented into '" << dirstr << "' callcnt#" << callcnt);
    }
  RPS_DEBUG_LOG(CMD, "REPL command dump dumped= " << (dumped?"true":"false") << " dumpdir=" << dumpdir << " callcnt#" << callcnt<< " nextlexob:" << _f.nextlexval);
  if (dumped)
    return {Rps_StringValue(dumpdir),nullptr};
  else
    RPS_WARNOUT("non-dumped REPL token for command dump - dumpdir=" << dumpdir << " callcnt#" << callcnt<< " nextlexval" << _f.nextlexval);
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
                           Rps_ObjectRef replcmdob;
                           Rps_Value lextokv;
                           Rps_Value showv;
                );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
  });
  RPS_DEBUG_LOG(CMD, "REPL command show start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << ";  arg1=" << arg1
                << "∈" << arg1.compute_class(&_) <<std::endl
                << ";  arg2=" << arg2
                << "∈" << arg2.compute_class(&_)
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  RPS_DEBUG_LOG(REPL, "REPL command show start arg0=" << arg0
                << "∈" << arg0.compute_class(&_)
                << ";  arg1=" << arg1
                << "∈" << arg1.compute_class(&_) <<std::endl
                << ";  arg2=" << arg2
                << "∈" << arg2.compute_class(&_)
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning REPL command show may need some local Rps_TokenSource
  _f.replcmdob = arg0.to_object();
  _f.lextokv = arg1;
  RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6/started replcmdob:" << _f.replcmdob << " lextokv:" << _f.lextokv
                << std::endl << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT"));
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
                  << "... before parse_expression pos:" << showpos
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
		      << "... curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
		      << " token_deq:" << tksrc->token_dequeue()
		      << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_7WsQyJK6lty02uz5KT for REPL command show"));
      }
    RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6/before pars.expr. tksrc:" << (*tksrc) << " replcmdob:" << _f.replcmdob << " lextokv:" << _f.lextokv
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT/before parsexp")
                  << std::endl << ".... before parse_expression token_deq:"
                  << tksrc->token_dequeue()
                  << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr()) << std::endl);
    bool okparsexp = false;
    _f.showv = tksrc->parse_expression(&_,&okparsexp);
    if (okparsexp)
      {
        RPS_DEBUG_LOG(CMD, "REPL command show lextokv=" << _f.lextokv << " framedepth:"<< _.call_frame_depth()
                      << " after successful parse_expression showv=" << _f.showv);
        RPS_DEBUG_LOG(REPL, "REPL command show°_7WsQyJK6/after pars.expr. tksrc:" << (*tksrc)
                      << " replcmdob:" << _f.replcmdob << std::endl
                      << "... token_deq:" << tksrc->token_dequeue()
                      << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                      << " lextokv:" << _f.lextokv << " showv:" << _f.showv
                      << std::endl
                      << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT/after parsexp")
                      << std::endl);
        std::cout << "##" << RPS_TERMINAL_BOLD_ESCAPE << showpos
                  << RPS_TERMINAL_NORMAL_ESCAPE << " : "
                  << _f.showv << std::endl;
#warning rpsapply_7WsQyJK6lty02uz5KT for REPL command show should probably EVALUATE showv
        /** TODO: we probably need some evaluating function, perhaps some
        Rps_CallFrame::evaluate_repl_expr(Rps_Value expr,Rps_ObjectRef ctxob) member function **/
        /// temporary return. Should do something fancy
        return {_f.replcmdob, _f.showv};
      }
    else   // command show°_7WsQyJK6/failed to parse expression
      {
        RPS_WARNOUT("command show°_7WsQyJK6 failed to parse expression in " << (*tksrc)
                    << std::endl
                    << " replcmdob:" << _f.replcmdob << std::endl
                    << "... token_deq:" << tksrc->token_dequeue()
                    << " curcptr:" << Rps_QuotedC_String(tksrc->curcptr())
                    << " lextokv:" << _f.lextokv << " showv:" << _f.showv
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "%command show°_7WsQyJK6lty02uz5KT/fail parsexp")
                    << std::endl);

      };
  }
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




//// end of file cmdrepl_rps.cc

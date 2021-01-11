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
                            const Rps_Value arg1,
                            const Rps_Value arg2,
                            const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
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
                << " arg1=" << arg1 << " arg2=" << arg2 << " arg3=" << arg3 << std::endl
                << " callingclos=" << _f.closv
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
  _f.replcmdob = arg0.to_object();
  _f.lexkindob = arg1.to_object();
  _f.lexval = arg2;
  if (_f.lexval.is_object())
    _f.lexob = _f.lexval.to_object();
  if (_f.lexob && _f.lexob->oid() == Rps_Id("_78wsBiJhJj1025DIs1"))  // the dot "."∈repl_delimiter
    {
      // dump to current directory
      rps_dump_into(".", &_);
      RPS_DEBUG_LOG(CMD, "REPL command dumped into current directory");
      return {_f.lexob, nullptr};
    }
  else if (_f.lexkindob == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE)) //string∈class #
    {
      std::string dirstr = _f.lexval.as_cppstring();
#warning rpsapply_61pgHb5KRq600RLnKD should use wordexp(3) on the string
      // see https://man7.org/linux/man-pages/man3/wordexp.3.html
      RPS_FATALOUT("REPL command dump unimplemented into " << dirstr);
    }
  /*** For the "dump ." command:
   * arg0 is _2Fy0WfTEAbr03HTthe, the "dump"/∈repl_command
   * arg1 is _2wdmxJecnFZ02VGGFK, the repl_delimiter∈class
   * arg2 is _78wsBiJhJj1025DIs1, the dot "."∈repl_delimiter
   * arg4 is a column number
   ***/
#warning incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump
  RPS_WARNOUT("incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump from " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD for REPL command dump") << std::endl
              << " arg0=" << arg0 << " arg1=" << arg1);
  return {nullptr,nullptr};
} //end of rpsapply_61pgHb5KRq600RLnKD for REPL command dump


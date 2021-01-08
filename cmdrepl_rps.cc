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

/* C++ function _61pgHb5KRq600RLnKD for REPL command dump*/
extern "C" rps_applyingfun_t rpsapply_61pgHb5KRq600RLnKD;
Rps_TwoValues
rpsapply_61pgHb5KRq600RLnKD(Rps_CallFrame*callerframe,
                           const Rps_Value arg0,
                           const Rps_Value arg1,
                           [[maybe_unused]] const Rps_Value arg2,
                           [[maybe_unused]] const Rps_Value arg3,
                           [[maybe_unused]] const std::vector<Rps_Value*> restargs)
{
   static Rps_Id descoid;
   if (!descoid) descoid=Rps_Id("_61pgHb5KRq600RLnKD");
   RPS_LOCALFRAME(/*descr:*/Rps_ObjectRef::really_find_object_by_oid(descoid),
                   callerframe,
   );
   RPS_DEBUG_LOG(CMD, "REPL command dump start arg0=" << arg0
                << " arg1=" << arg1 << std::endl
                << " from " << std::endl
                << Rps_ShowCallFrame(&_));
#warning incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump
  RPS_WARNOUT("incomplete rpsapply_61pgHb5KRq600RLnKD for REPL command dump from " << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_61pgHb5KRq600RLnKD for REPL command dump"));
  return {nullptr,nullptr};
} //end of rpsapply_61pgHb5KRq600RLnKD for REPL command dump


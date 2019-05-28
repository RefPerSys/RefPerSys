/****************************************************************
 * file minitests_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *  Mini-tests, temporarily before the primordial persistence
 *  milestone.  See https://gitlab.com/bstarynk/refpersys/milestones/1
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io>
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

void
Rps_ObjectRef::tiny_benchmark_1(Rps_CallFrameZone*callingfra, unsigned num)
{
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_SetValue setv;
                );
  _.setv = Rps_SetValue::tiny_benchmark_1(RPS_CURFRAME, num);
  assert (_.setv);
  RPS_FATAL("unimplemented Rps_ObjectRef::tiny_benchmark_1 num=%u", num);
#warning Rps_ObjectRef::tiny_benchmark_1 unimplemented
} // end of Rps_ObjectRef::tiny_benchmark_1



Rps_SetValue
Rps_SetValue::tiny_benchmark_1(Rps_CallFrameZone*callingfra, unsigned num)
{
  constexpr unsigned benchsize = Rps_ObjectZone::at_sorted_thresh+5;
  RPS_LOCALFRAME(callingfra, /*descr:*/nullptr,
                 Rps_SetValue setresv;
                 Rps_Value arrval[benchsize];
                 Rps_ObjectRef arrob[benchsize];
                );
#warning unimplemented Rps_SetValue::tiny_benchmark_1
  RPS_FATAL("unimplemented Rps_SetValue::tiny_benchmark_1 num=%u", num);
} // end Rps_SetValue::tiny_benchmark_1


/// end of file minitests_rps.cc

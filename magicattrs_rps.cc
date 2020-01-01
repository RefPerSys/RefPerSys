/****************************************************************
 * file magicattrs_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to magical attributes
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
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

extern "C" const char rps_magicattrs_gitid[];
const char rps_magicattrs_gitid[]= RPS_GITID;

extern "C" const char rps_magicattrs_date[];
const char rps_magicattrs_date[]= __DATE__;

/// the `class` magic attribute _36I1BY2NetN03WjrOv
extern "C"  rps_magicgetterfun_t rpsget_36I1BY2NetN03WjrOv;
Rps_Value
rpsget_36I1BY2NetN03WjrOv(const Rps_Value valarg, const Rps_ObjectRef obattrarg,
			  Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(RPS_ROOT_OB(_36I1BY2NetN03WjrOv),
                 callerframe,
                 Rps_Value val; // the value
                 Rps_ObjectRef obattr; // the attribute
                );
  _.obattr = obattrarg;
  _.val = valarg;
  RPS_ASSERT (_.obattr == RPS_ROOT_OB(_36I1BY2NetN03WjrOv));
  return Rps_Value(_.val.compute_class(&_));
} // end of rpsget_36I1BY2NetN03WjrOv

// end of file magicattrs_rps.cc

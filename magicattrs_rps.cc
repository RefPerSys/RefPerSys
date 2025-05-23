/****************************************************************
 * file magicattrs_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to magical attributes
 *      whose operations trigger extra code using function pointers
 *      which are dlsym-able and dladdr-able on Linux.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2020 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_magicattrs_shortgitid[];
const char rps_magicattrs_shortgitid[]= RPS_SHORTGITID;


/// the `class` magic attribute and class _41OFI3r0S1t03qdB2E
extern "C" Rps_Value
rpsget_41OFI3r0S1t03qdB2E(const Rps_Value valarg, const Rps_ObjectRef obattrarg,
                          Rps_CallFrame*callerframe);

Rps_Value
rpsget_41OFI3r0S1t03qdB2E(const Rps_Value valarg, const Rps_ObjectRef obattrarg,
                          Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(rpskob_41OFI3r0S1t03qdB2E,
                 callerframe,
                 Rps_Value val; // the value
                 Rps_ObjectRef obattr; // the attribute
                );
  RPS_ASSERT(RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) == rpskob_41OFI3r0S1t03qdB2E);
  _f.obattr = obattrarg;
  _f.val = valarg;
  RPS_ASSERT (_f.obattr == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E));
  return Rps_Value(_f.val.compute_class(&_));
} // end of rpsget_41OFI3r0S1t03qdB2E  - magic getter `class`

/// the `space` magic attribute _9uwZtDshW4401x6MsY
extern "C" Rps_Value
rpsget_9uwZtDshW4401x6MsY(const Rps_Value valarg, const Rps_ObjectRef obattrarg,
                          Rps_CallFrame*callerframe);

Rps_Value
rpsget_9uwZtDshW4401x6MsY(const Rps_Value valarg, const Rps_ObjectRef obattrarg,
                          Rps_CallFrame*callerframe)
{
  RPS_LOCALFRAME(rpskob_9uwZtDshW4401x6MsY,
                 callerframe,
                 Rps_Value val; // the value
                 Rps_ObjectRef obattr; // the attribute
                );
  _f.obattr = obattrarg;
  _f.val = valarg;
  RPS_ASSERT(RPS_ROOT_OB(_9uwZtDshW4401x6MsY) == rpskob_9uwZtDshW4401x6MsY);
  RPS_ASSERT (_f.obattr == RPS_ROOT_OB(_9uwZtDshW4401x6MsY));
  if (!_f.val.is_empty() && _f.val.is_object())
    return Rps_Value(_f.val.as_object()->get_space());
  return nullptr;
} // end of rpsget_9uwZtDshW4401x6MsY - magic getter `space`

// end of file magicattrs_rps.cc

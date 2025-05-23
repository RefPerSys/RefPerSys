/****************************************************************
 * file output_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to pretty output of values and objects
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright (C) 2024 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_output_gitid[];
const char rps_output_gitid[]= RPS_GITID;

extern "C" const char rps_output_date[];
const char rps_output_date[]= __DATE__;

extern "C" const char rps_output_shortgitid[];
const char rps_output_shortgitid[]= RPS_SHORTGITID;

////////////////


void
Rps_OutputValue::do_output(std::ostream& out) const
{
  /// output value _out_val at _out_depth to out, using Rps_ZoneValue::val_output....
  if (_out_depth > _out_maxdepth)
    {
      out << "?";
      return;
    };
  if (_out_val.is_empty())
    {
      out << "*nil*";
      return;
    }
  else if (_out_val.is_int())
    {
      out << _out_val.as_int();
      return;
    };
  const Rps_ZoneValue* outzv = _out_val.as_ptr();
  RPS_ASSERT(outzv);
  outzv->val_output(out, _out_depth, _out_maxdepth);
} // end Rps_OutputValue::do_output



#warning TODO: add or move code related to pretty output here

//// end of file output_rps.cc

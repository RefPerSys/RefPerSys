/****************************************************************
 * file userpref_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for user preferences.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net> (France)
 *      Abhishek Chakravarti <abhishek@taranjali.org> (India)
 *      Nimesh Neema <nimeshneema@gmail.com> (India)
 *
 *      © Copyright 2024 - 2024 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it
 *    and/or modify it under the terms of the GNU General Public
 *    License as published by the Free Software Foundation, either
 *    version 3 of the License, or (at your option) any later
 *    version. Alternatively, at your choice you can also use the GNU
 *    Lesser General Public License v3 or any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details (or LGPLv3).
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "refpersys.hh"


extern "C" const char rps_userpref_gitid[];
const char rps_userpref_gitid[]= RPS_GITID;

extern "C" const char rps_userpref_date[];
const char rps_userpref_date[]= __DATE__;

extern "C" const char rps_userpref_shortgitid[];
const char rps_userpref_shortgitid[]= RPS_SHORTGITID;

extern "C" void rps_set_user_preferences(char*);

void rps_set_user_preferences(char*path)
{
#warning unimplemented rps_set_user_preferences
  RPS_FATALOUT("unimplemented user preferences file '"
               << Rps_Cjson_String(path) << "'");
} // end  rps_set_user_preferences


////// end of file userpref_rps.cc

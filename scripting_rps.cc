/****************************************************************
 * file scripting_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It provides support for scripting (after loading the heap)
 *
 * Author(s):
 *      Basile STARYNKEVITCH (France) <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2025 - 2025 The Reflective Persistent System Team
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
 *
 *********************************************************************/


#include "refpersys.hh"

// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG gmp
//@@PKGCONFIG gmpxx

extern "C" const char rps_scripting_gitid[];
const char rps_scripting_gitid[]= RPS_GITID;

extern "C" const char rps_scripting_date[];
const char rps_scripting_date[]= __DATE__;

extern "C" const char rps_scripting_shortgitid[];
const char rps_scripting_shortgitid[]= RPS_SHORTGITID;

extern "C" const char rps_scripting_timestamp[];
const char rps_scripting_timestamp[]= __TIMESTAMP__;

#warning missing code in scripting_rps.cc

extern "C" void rps_scripting_help(void);
extern "C" void rps_scripting_add_script(const char*);


void
rps_scripting_help(void)
{
  RPS_FATAL("unimplemented rps_scripting_help");
#warning rps_scripting_help unimplemented
} // end rps_scripting_help


void
rps_scripting_add_script(const char*path)
{
  RPS_FATALOUT("unimplemented rps_scripting_add_script path=" << path);
#warning rps_scripting_add_script unimplemented
} // end rps_scripting_add_script

//// end of file scripting_rps.cc

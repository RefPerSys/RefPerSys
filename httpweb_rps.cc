/****************************************************************
 * file httpweb_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System. See refpersys.org
 *
 *      It has the web interface code, using cpp-httplib
 *      from https://github.com/yhirose/cpp-httplib/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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

// compile time configuration of httplib.h
#define CPPHTTPLIB_THREAD_POOL_COUNT (1+rps_nbjobs/2)
#define CPPHTTPLIB_ZLIB_SUPPORT 1
#define CPPHTTPLIB_OPENSSL_SUPPORT 1

#include "httplib.h"

void
rps_web_initialize_service(const char*servarg)
{
} // end rps_web_initialize_service


void
rps_run_web_service()
{
  RPS_FATAL("unimplemented rps_run_web_service");
#warning rps_run_web_service unimplemented
} // end rps_run_web_service

///////// end of file web_rps.cc

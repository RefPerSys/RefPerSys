/****************************************************************
 * file garbcoll_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the garbage collector.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
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


extern "C" const char rps_garbcoll_gitid[];
const char rps_garbcoll_gitid[]= RPS_GITID;

extern "C" const char rps_garbcoll_date[];
const char rps_garbcoll_date[]= __DATE__;

void rps_garbage_collect (void)
{
  RPS_FATAL("unimplemented rps_garbage_collect");
#warning rps_dump_into unimplemented
} // end of rps_garbage_collect


//////////////////////////////////////////////////////////// end of file garbcoll_rps.cc


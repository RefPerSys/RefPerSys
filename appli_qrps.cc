/****************************************************************
 * file appli_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the Qt5 code related to the Qt5 application
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
#include "qthead_qrps.hh"

RpsQApplication::RpsQApplication(int &argc, char*argv[])
  : QApplication(argc, argv)
{
} // end of RpsQApplication::RpsQApplication

void
RpsQApplication::dump_state(QString dirpath)
{
} // end of RpsQApplication::dump_state


void rps_run_application(int &argc, char**argv) {
  RPS_INFORM("rps_run_application: start of %s gitid %s host %s pid %d\n",
	     argv[0], rps_gitid, rps_hostname(), (int)getpid());
} // end of rps_run_application

//////////////// moc generated file
#include "_qthead_qrps.inc.hh"

//////////////////////////////////////// end of file appli_qrps.cc

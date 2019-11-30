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

extern "C" const char rps_appli_gitid[];
const char rps_appli_gitid[]= RPS_GITID;

extern "C" const char rps_appli_date[];
const char rps_appli_date[]= __DATE__;

// these functions are here, because other files might not include <QString>
const Rps_String*
Rps_String::make(const QString&qs)
{
  return Rps_String::make(qs.toStdString());
} // end Rps_String::make(const QString&qs)

Rps_StringValue::Rps_StringValue(const QString&qs)
  : Rps_Value(Rps_String::make(qs), Rps_ValPtrTag{})
{
} // end of Rps_StringValue::Rps_StringValue(const QString&qs)

////////////////

RpsQApplication::RpsQApplication(int &argc, char*argv[])
  : QApplication(argc, argv)
{
  std::shared_ptr<RpsQWindow> wnd (new RpsQWindow ());
  wnd->resize (640, 480); // TODO: get dimensions from $HOME/.RefPerSys
  wnd->setWindowTitle ("RefPerSys");

  this->_wnd_vec.push_back (wnd);
} // end of RpsQApplication::RpsQApplication

void
RpsQApplication::dump_state(QString dirpath)
{
} // end of RpsQApplication::dump_state


std::shared_ptr<RpsQWindow> RpsQApplication::getWindow(size_t index)
{
  return this->_wnd_vec.at (index);
}


void rps_run_application(int &argc, char **argv)
{
  RPS_INFORM("rps_run_application: start of %s gitid %s host %s pid %d\n",
             argv[0], rps_gitid, rps_hostname(), (int)getpid());

  RpsQApplication app (argc, argv);
  app.getWindow (0)->show ();
  (void) app.exec ();
} // end of rps_run_application

//////////////// moc generated file
#include "_qthead_qrps.inc.hh"

//////////////////////////////////////// end of file appli_qrps.cc

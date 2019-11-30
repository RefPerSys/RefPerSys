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
void
RpsQApplication::add_new_window(void)
{
  std::lock_guard gu(app_mutex);
  auto window = new RpsQWindow();
  window->setWindowTitle(QString("RefPerSys #%").arg(app_windvec.size()));
  window->resize (640, 480); // TODO: get dimensions from $HOME/.RefPerSys
  app_windvec.emplace_back(window);
} // end of RpsQApplication::add_new_window

RpsQApplication::RpsQApplication(int &argc, char*argv[])
  : QApplication(argc, argv)
{
  app_windvec.reserve(16);
  app_windvec.push_back(nullptr); // we don't want a 0 index.
  add_new_window();
} // end of RpsQApplication::RpsQApplication

void
RpsQApplication::dump_state(QString dirpath)
{
} // end of RpsQApplication::dump_state


RpsQWindow* RpsQApplication::getWindowPtr(int ix)
{
  std::lock_guard gu(app_mutex);
  if (ix < 0)
    ix += app_windvec.size();
  if (ix <= 0 || ix > app_windvec.size())
    return nullptr;
  return app_windvec.at(ix).get();
}


void rps_run_application(int &argc, char **argv)
{
  RPS_INFORM("rps_run_application: start of %s gitid %s host %s pid %d\n",
             argv[0], rps_gitid, rps_hostname(), (int)getpid());

  RpsQApplication app (argc, argv);
  app.add_new_window();
  (void) app.exec ();
} // end of rps_run_application

//////////////// moc generated file
#include "_qthead_qrps.inc.hh"

//////////////////////////////////////// end of file appli_qrps.cc

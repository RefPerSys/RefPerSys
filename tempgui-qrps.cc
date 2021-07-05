/****************************************************************
 * file tempgui_qrps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *
 *      This file is part of the Reflective Persistent System.  It is
 *      the header file for some optional dlopen-ed plugin using Qt5
 *      It is tightly related to tempgui_qrps.cc
 *      See on https://framalistes.org/sympa/arc/refpersys-forum/
 *      the messages 2021-07/msg00002.html
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2021 The Reflective Persistent System Team
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

#include "tempgui-qrps.hh"
#include "tempgui-qrps.moc.hh"

std::recursive_mutex rpsqt_mtx;
QApplication* rpsqt_app;

std::set<RpsTemp_MainWindow*> RpsTemp_MainWindow::mainwin_set;
RpsTemp_MainWindow::RpsTemp_MainWindow() {
  {
    RPS_DEBUG_LOG(GUI, "start RpsTemp_MainWindow this@" << (void*)this);
    RPSQT_WITH_LOCK();
    RPS_DEBUG_LOG(GUI, "start RpsTemp_MainWindow window#" << mainwin_set.size());
    mainwin_set.insert(this);
    connect(this, &QObject::destroyed, this,
	    [=](){
	      RPSQT_WITH_LOCK();
	      mainwin_set.erase(this);
	      RPS_ASSERT(rpsqt_app != nullptr);
	      RPS_DEBUG_LOG(GUI, "destroying RpsTemp_MainWindow @" << (void*)this);
	      if (mainwin_set.empty()) {
		rpsqt_app->exit();
	      }
	    });
  }
#warning incomplete RpsTemp_MainWindow::RpsTemp_MainWindow constructor
  RPS_WARNOUT("incomplete RpsTemp_MainWindow::RpsTemp_MainWindow constructor this@" << (void*)this
	      << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_MainWindow::RpsTemp_MainWindow"));
} // end RpsTemp_MainWindow::RpsTemp_MainWindow


void
rps_tempgui_init_progarg(int &argc, char**argv)
{
  RPSQT_WITH_LOCK();
  RPS_ASSERT(rpsqt_app == nullptr);
  rpsqt_app = new QApplication(argc, argv);
  QCoreApplication::setOrganizationName("refpersys.org");
  QCoreApplication::setApplicationName("RefPerSys temporary Qt");
  QCoreApplication::setApplicationVersion(rps_shortgitid);
  RPS_INFORMOUT("with QApplication " << rpsqt_app);
} // end rps_tempgui_init



void
rps_tempgui_run(void)
{
  RPS_INFORMOUT("rps_tempgui_run:"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rps_tempgui_run"));
  RPS_ASSERT(rpsqt_app != nullptr);
#warning incomplete rps_tempgui_run
  RPS_WARNOUT("should use rpsqt_app->exec");
#warning rps_tempgui_run should use rpsqt_app->exec
} //  end rps_tempgui_run
//// end of file tempgui-qrps.cc for refpersys.org

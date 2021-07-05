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


//////////////// main window
std::set<RpsTemp_MainWindow*> RpsTemp_MainWindow::mainwin_set_;
RpsTemp_MainWindow::RpsTemp_MainWindow()
  : mainwin_rank(0),
    mainwin_dumpact(nullptr),
    mainwin_quitact(nullptr),
    mainwin_exitact(nullptr),
    mainwin_objbrowser(nullptr)
{
  {
    RPS_DEBUG_LOG(GUI, "start RpsTemp_MainWindow this@" << (void*)this);
    RPSQT_WITH_LOCK();
    mainwin_set_.insert(this);
    mainwin_rank = mainwin_set_.size();
    RPS_DEBUG_LOG(GUI, "start RpsTemp_MainWindow window#" << mainwin_rank);
    setMinimumSize(512, 480); // minimal size in pixels
    {
      char titlebuf[48];
      memset (titlebuf, 0, sizeof(titlebuf));
      snprintf(titlebuf, sizeof(titlebuf), "RefPerSys/p%d window#%d",
	       (int)getpid(), mainwin_rank);
      setWindowTitle(QString(titlebuf));
    }
    connect(this, &QObject::destroyed, this,
	    [=](){
	      RPSQT_WITH_LOCK();
	      mainwin_set_.erase(this);
	      RPS_ASSERT(rpsqt_app != nullptr);
	      RPS_DEBUG_LOG(GUI, "destroying RpsTemp_MainWindow @" << (void*)this);
	      if (mainwin_set_.empty()) {
		rpsqt_app->exit();
	      }
	    });
  }
  create_menus();
#warning incomplete RpsTemp_MainWindow::RpsTemp_MainWindow constructor
  RPS_WARNOUT("incomplete RpsTemp_MainWindow::RpsTemp_MainWindow constructor this@" << (void*)this
	      << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_MainWindow::RpsTemp_MainWindow"));
} // end RpsTemp_MainWindow::RpsTemp_MainWindow


void
RpsTemp_MainWindow::create_menus(void)
{
    RPSQT_WITH_LOCK();
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::create_menus start mainwin#"
		  << rank());
    auto mbar = menuBar();
    auto appmenu = mbar->addMenu("App");
   mainwin_dumpact =  appmenu->addAction("&Dump");
   mainwin_quitact =  appmenu->addAction("&Quit");
   mainwin_exitact =  appmenu->addAction("e&Xit");
   mbar->show();
} // end RpsTemp_MainWindow::create_menus


////////////////////////////////////////////////////////////////
///// object browser
RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser()
  : QTextBrowser()
{
#warning incomplete RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser constructor
  RPS_WARNOUT("incomplete RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser constructor this@" << (void*)this
	      << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser"));
} // end RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser




void
rps_tempgui_init_progarg(int &argc, char**argv)
{
  RPSQT_WITH_LOCK();
  RPS_ASSERT(rpsqt_app == nullptr);
  rpsqt_app = new QApplication(argc, argv);
  QCoreApplication::setOrganizationName("refpersys.org");
  QCoreApplication::setApplicationName("RefPerSys temporary Qt");
  QCoreApplication::setApplicationVersion(rps_shortgitid);
  {
    RpsTemp_MainWindow*firstwin = new RpsTemp_MainWindow();
    firstwin->setVisible(true);
    firstwin->show();
    RPS_DEBUG_LOG(GUI, "showing firstwin@" << (void*)firstwin << " rank#" << firstwin->rank());
  }
  RPS_INFORMOUT("with QApplication " << rpsqt_app);
} // end rps_tempgui_init_progarg



void
rps_tempgui_run(void)
{
  RPS_INFORMOUT("rps_tempgui_run:"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rps_tempgui_run"));
  RPS_ASSERT(rpsqt_app != nullptr);
  int ok = rpsqt_app->exec();
  RPS_DEBUG_LOG(GUI, "rps_tempgui_run after exec ok=" << ok<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "ending rps_tempgui_run"));
} //  end rps_tempgui_run

//// end of file tempgui-qrps.cc for refpersys.org

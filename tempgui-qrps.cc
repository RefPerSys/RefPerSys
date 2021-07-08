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
RpsTemp_Application* rpsqt_app;

RpsTemp_Application::RpsTemp_Application(int &argc, char **argv)
  : QApplication::QApplication(argc, argv) {
};				// end RpsTemp_Application::RpsTemp_Application

void
RpsTemp_Application::do_dump(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_dump start" <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::do_dump")
		);
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 /*callerframe:*/nullptr,
		 );
  rps_dump_into(".", &_);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_dump end");
} // end RpsTemp_Application::do_dump

void
RpsTemp_Application::do_exit(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_exit start" <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::do_exit")
		);
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 /*callerframe:*/nullptr,
		 );
  rps_dump_into(".", &_);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_exit did dump");
  this->exit(0);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_exit end");
} // end RpsTemp_Application::do_exit


void
RpsTemp_Application::do_quit(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_quit start" <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::do_quit")
		);
  this->exit(0);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_quit end");
} // end RpsTemp_Application::do_quit

void
RpsTemp_Application::do_new_window(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_new_window start" <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::do_new_window")
		);
  {
    RpsTemp_MainWindow*newwin = new RpsTemp_MainWindow();
    newwin->setVisible(true);
    newwin->show();
    RPS_DEBUG_LOG(GUI, "showing newwin@" << (void*)newwin << " rank#" << newwin->rank() 
	      << (newwin->isVisible()?" visible":" hidden"));
  }
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_new_window end");
} // end RpsTemp_Application::do_dump

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
  mainwin_centralframe = new QFrame(this);
  mainwin_centralframe->setFrameStyle(QFrame::Box|QFrame::Plain);
  mainwin_vbox = new QVBoxLayout(mainwin_centralframe);
  fill_vbox();
  mainwin_centralframe->update();
  mainwin_centralframe->show();
  setCentralWidget(mainwin_centralframe);
#warning incomplete RpsTemp_MainWindow::RpsTemp_MainWindow constructor
  RPS_WARNOUT("incomplete RpsTemp_MainWindow::RpsTemp_MainWindow constructor this@" << (void*)this << " window#" << mainwin_rank
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
  /// dump action
  mainwin_dumpact = appmenu->addAction("&Dump");
  mainwin_dumpact->setToolTip("dump the heap and continue");
  connect(mainwin_dumpact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_dump);
  /// quit action
  mainwin_quitact = appmenu->addAction("&Quit");
  mainwin_quitact->setToolTip("quit without dumping state");
  connect(mainwin_quitact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_quit);
  /// exit action
  mainwin_exitact = appmenu->addAction("e&Xit");
  mainwin_exitact->setToolTip("exit after dumping the heap");
  connect(mainwin_exitact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_exit);
  /// new window action
  mainwin_newact = appmenu->addAction("N&ew window");
  mainwin_newact->setToolTip("make a new window");
  connect(mainwin_newact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_new_window);
  ///
  mbar->show();
  setVisible(true);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::create_menus ended mainwin#"
		<< rank() << " @" << (void*)this
		<< (isVisible()?" shown":" hidden"));
} // end RpsTemp_MainWindow::create_menus


void
RpsTemp_MainWindow::fill_vbox(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox start mainwin#"
		<< rank());
  RPS_ASSERT(mainwin_vbox);
  mainwin_objbrowser = new RpsTemp_ObjectBrowser(this);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox mainwin_objbrowser@" << (void*)mainwin_objbrowser
		<< " mainwin#" << rank());
  mainwin_showframe = new QFrame(this);
  mainwin_showframe->setFrameStyle(QFrame::Box|QFrame::Plain);
  mainwin_showlabel = new QLabel(QString("show object:"),mainwin_showframe);
  mainwin_showlabel->show();
  mainwin_showframe->update();
  mainwin_showframe->show();
  mainwin_vbox->addWidget(mainwin_showframe);
  mainwin_vbox->addWidget(mainwin_objbrowser);
  mainwin_vbox->update();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox end mainwin#"
		<< rank());
} // end RpsTemp_MainWindow::fill_vbox

////////////////////////////////////////////////////////////////
///// object browser
RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser(QWidget*parent)
  : QTextBrowser(parent)
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
  rpsqt_app = new RpsTemp_Application(argc, argv);
  QCoreApplication::setOrganizationName("refpersys.org");
  QCoreApplication::setApplicationName("RefPerSys temporary Qt");
  QCoreApplication::setApplicationVersion(rps_shortgitid);
  {
    RpsTemp_MainWindow*firstwin = new RpsTemp_MainWindow();
    firstwin->setVisible(true);
    firstwin->show();
    RPS_DEBUG_LOG(GUI, "showing firstwin@" << (void*)firstwin << " rank#" << firstwin->rank() 
	      << (firstwin->isVisible()?" visible":" hidden"));
  }
  RPS_INFORMOUT("with QApplication " << rpsqt_app);
} // end rps_tempgui_init_progarg



void
rps_tempgui_run(void)
{
  RPS_INFORMOUT("rps_tempgui_run start:"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rps_tempgui_run"));
  RPS_ASSERT(rpsqt_app != nullptr);
  RPS_DEBUG_LOG(GUI, "rps_tempgui_run before one RpsTemp_Application::processEvents");
  /* In principle, these are useless. Since rpsqt_app->exec should do it. */
  RpsTemp_Application::processEvents();
  RpsTemp_Application::sendPostedEvents();
  usleep(1000);
  RPS_DEBUG_LOG(GUI, "rps_tempgui_run before exec"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rps_tempgui_run/before exec"));
  int ok = rpsqt_app->exec();
  RPS_DEBUG_LOG(GUI, "rps_tempgui_run after exec ok=" << ok<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "ending rps_tempgui_run"));
} //  end rps_tempgui_run

//// end of file tempgui-qrps.cc for refpersys.org

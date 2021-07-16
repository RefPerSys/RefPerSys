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

std::ostream&
operator << (std::ostream&out, const QRect&r)
{
  out<< "rect[x=" << r.x() << ",y=" << r.y()
     << ",w=" << r.width() << ",h=" << r.height() << "]";
  return out;
}

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

////////////
void
RpsTemp_Application::do_garbage_collect(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_garbage_collect start" <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::do_garbage_collect")
		);
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 nullptr, // no caller frame
		 );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    this->xtra_gc_mark(gc);
  });
  std::function<void(Rps_GarbageCollector*)> gcfun([&](Rps_GarbageCollector*gc)
  {
    gc->mark_call_stack(&_);
  });
  rps_garbage_collect(&gcfun);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_garbage_collect done"<<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "done RpsTemp_Application::do_garbage_collect")
	);
} // end RpsTemp_Application::do_garbage_collect

void
RpsTemp_Application::xtra_gc_mark(Rps_GarbageCollector*gc)
{
  RPSQT_WITH_LOCK();
  RPS_ASSERT(gc != nullptr);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::xtra_gc_mark"<<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::xtra_gc_mark")
	);
  RpsTemp_MainWindow::garbage_collect_all_main_windows(gc);
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::xtra_gc_mark done");
} // end RpsTemp_Application::xtra_gc_mark

//// main window
std::set<RpsTemp_MainWindow*> RpsTemp_MainWindow::mainwin_set_;
RpsTemp_MainWindow::RpsTemp_MainWindow()
  : mainwin_rank(0),
    mainwin_dumpact(nullptr),
    mainwin_quitact(nullptr),
    mainwin_exitact(nullptr),
    mainwin_newact(nullptr),
    mainwin_garbcollact(nullptr),
    mainwin_centralframe(nullptr),
    mainwin_vbox(nullptr),
    mainwin_showframe(nullptr),
    mainwin_showhbox(nullptr),
    mainwin_showlabel(nullptr),
    mainwin_shownobject(nullptr),
    mainwin_showncompleter(nullptr),
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
      char titlebuf[64];
      memset (titlebuf, 0, sizeof(titlebuf));
      snprintf(titlebuf, sizeof(titlebuf), "RefPerSys/p%d°%s window#%d",
	       (int)getpid(), rps_shortgitid, mainwin_rank);
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
RpsTemp_MainWindow::garbage_collect_all_main_windows(Rps_GarbageCollector*gc)
{
  RPS_ASSERT(gc);
  for (RpsTemp_MainWindow*mainwin: mainwin_set_)
    mainwin->garbage_collect_main_window(gc);
} // end RpsTemp_Application::garbage_collect_all_main_windows


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
  mainwin_newact = appmenu->addAction("&New window");
  mainwin_newact->setToolTip("make a new window");
  connect(mainwin_newact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_new_window);
  /// garbage collect action
  mainwin_garbcollact = appmenu->addAction("&Garb. Coll.");
  mainwin_garbcollact->setToolTip("force entire garbage collection");
  connect(mainwin_garbcollact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_garbage_collect);
  ///
  mbar->show();
  setVisible(true);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::create_menus ended mainwin#"
		<< rank() << " @" << (void*)this
		<< (isVisible()?" shown":" hidden"));
} // end RpsTemp_MainWindow::create_menus


void
RpsTemp_MainWindow::garbage_collect_main_window(Rps_GarbageCollector*gc)
{
  RpsTemp_ObjectBrowser* objbr = objbrowser();
  RPS_ASSERT(objbr);
  objbr->garbage_collect_object_browser(gc);
} // end RpsTemp_MainWindow::garbage_collect_main_window


void
RpsTemp_MainWindow::fill_vbox(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox start mainwin#"
		<< rank());
  this->setStyleSheet("RpsTemp_ObjectBrowser {background-color: yellow}");
  this->setStyleSheet("RpsTemp_LineEdit {background-color: azure}");
  RPS_ASSERT(mainwin_vbox);
  mainwin_objbrowser = new RpsTemp_ObjectBrowser(this);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox mainwin_objbrowser@" << (void*)mainwin_objbrowser
		<< " mainwin#" << rank());
  mainwin_showframe = new QFrame(this);
  mainwin_showframe->setMinimumSize(32,16);
  mainwin_showframe->setFrameStyle(QFrame::Box|QFrame::Plain);
  mainwin_showhbox = new QHBoxLayout(mainwin_showframe);
  mainwin_showlabel = new QLabel(QString("show object:"),mainwin_showframe);
  mainwin_showlabel->setMinimumSize(16,10);
  mainwin_showlabel->show();
  mainwin_showhbox->addWidget(mainwin_showlabel);
  mainwin_shownobject = new RpsTemp_ObjectLineEdit(mainwin_showframe);
  mainwin_showncompleter = new RpsTemp_ObjectCompleter(mainwin_shownobject);
  mainwin_showhbox->addWidget(mainwin_shownobject);
  mainwin_shownobject->show();
  mainwin_showframe->update();
  mainwin_showframe->show();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox mainwin#" << rank()
		<< " mainwin_vbox@" << (void*)mainwin_vbox
		<< " mainwin_showframe@" << (void*)mainwin_showframe
		<< " mainwin_objbrowser@" << (void*)mainwin_objbrowser
		<< " objbrowser¤" << mainwin_objbrowser->rect());
  mainwin_vbox->addWidget(mainwin_showframe);
  mainwin_vbox->addWidget(mainwin_objbrowser);
  mainwin_vbox->update();
  connect(mainwin_shownobject, &RpsTemp_ObjectLineEdit::editingFinished,
	  this, &RpsTemp_MainWindow::do_enter_shown_object);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox end mainwin#"
		<< rank() << " showlabel¤" << mainwin_showlabel->rect()
		<< " showframe¤" << mainwin_showframe->rect()
		<< " objbrowser¤" << mainwin_objbrowser->rect());
} // end RpsTemp_MainWindow::fill_vbox


// slot when mainwin_shownobject has been entered
void
RpsTemp_MainWindow::do_enter_shown_object(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object start mainwin#"
		<< rank());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 /*callerframe:*/nullptr,
		 Rps_ObjectRef showob;
		 );
  RPS_ASSERT(mainwin_shownobject != nullptr);
  std::string obshowstring = mainwin_shownobject->text().toStdString();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object obshowstring="
		<< Rps_QuotedC_String(obshowstring)
		<< std::endl
		<< Rps_ShowCallFrame(&_)
		<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "incomplete RpsTemp_MainWindow::do_enter_shown_object"));
  _f.showob = Rps_ObjectRef::find_object_or_null_by_string(&_, obshowstring);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object by name showob="
		<< _f.showob);
  RPS_WARNOUT("incomplete RpsTemp_MainWindow::do_enter_shown_object mainwin#" << rank()
	      << " obshowstring=" << Rps_QuotedC_String(obshowstring));
#warning incomplete RpsTemp_MainWindow::do_enter_shown_object
} // end RpsTemp_MainWindow::do_enter_shown_object

////////////////////////////////////////////////////////////////
///// object browser
RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser(QWidget*parent)
  : QTextBrowser(parent),
    objbr_mtx(),
    objbr_defaultdepth(3),
    objbr_shownobvect(),
    objbr_mapshownob()
{
#warning incomplete RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser constructor
  RPS_WARNOUT("incomplete RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser constructor this@" << (void*)this
	      << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser"));
  setHtml(QString("<h1>object browser</h1>"));
  setReadOnly(true);
} // end RpsTemp_ObjectBrowser::RpsTemp_ObjectBrowser

void
RpsTemp_ObjectBrowser::garbage_collect_object_browser(Rps_GarbageCollector*gc)
{
  RPS_ASSERT(gc != nullptr);
  std::lock_guard<std::mutex> curguard(objbr_mtx);
  int nbshownob = (int)objbr_shownobvect.size();
  for (int shix=0; shix<nbshownob; shix++) {
    struct shown_object_st& curshob= objbr_shownobvect[shix];
    RPS_ASSERT(curshob.shob_obref);
    gc->mark_obj(curshob.shob_obref);
  }
} // end RpsTemp_ObjectBrowser::garbage_collect_object_browser


RpsTemp_ObjectLineEdit::RpsTemp_ObjectLineEdit(QWidget*parent)
  : QLineEdit(parent),
    oblined_completer(nullptr) {
  RPSQT_WITH_LOCK();
  setToolTip("enter object id or name\n"
	     "the TAB is autocompleting");
  setMinimumSize(40,10);
  oblined_completer = new RpsTemp_ObjectCompleter(this);
  RPS_DEBUG_LOG(GUI, "RpsTemp_ObjectLineEdit::RpsTemp_ObjectLineEdit this@"
		<< (void*)this << " oblined_completer@" << (void*)oblined_completer);
} // end RpsTemp_ObjectLineEdit


RpsTemp_ObjectCompleter::RpsTemp_ObjectCompleter(QObject*parent)
  : QCompleter(parent)
{  
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_ObjectCompleter::RpsTemp_ObjectCompleter this@" << (void*)this
		<< " parent@" << (void*)parent);
} // end RpsTemp_ObjectCompleter

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

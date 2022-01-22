/****************************************************************
 * file qtgui_qrps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *
 *      This file is part of the Reflective Persistent System.
 *      It is the Qt graphical user interface, enabled by -Q program argument
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2021 - 2022 The Reflective Persistent System Team
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

#include "qtgui-qrps.hh"
#include "qtgui-qrps.moc.hh"

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
RpsTemp_Application::do_open_command(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_Application::do_open_command start" <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_Application::do_open_command")
		);
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 nullptr, // no caller frame
		 );
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    this->xtra_gc_mark(gc);
  });
  if (!_app_cmdwin) {
    _app_cmdwin = new QMainWindow();
      char titlebuf[64];
      memset (titlebuf, 0, sizeof(titlebuf));
      snprintf(titlebuf, sizeof(titlebuf), "RefPerSys/p%d°%s command",
	       (int)getpid(), rps_shortgitid);
     _app_cmdwin->setWindowTitle(QString(titlebuf));
     _app_cmdwin->setMinimumSize(QSize{550,250});
  };
  if (!_app_cmdedit) {
    _app_cmdedit = new RpsTemp_CommandEdit();
    _app_cmdwin->setCentralWidget(_app_cmdedit);
  }
  _app_cmdedit->show();
  _app_cmdwin->show();
  std::function<void(Rps_GarbageCollector*)> gcfun([&](Rps_GarbageCollector*gc)
  {
    gc->mark_call_stack(&_);
  });
} // end RpsTemp_Application::do_open_command

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
    mainwin_commandact(nullptr),
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
  /// command action
  mainwin_commandact = appmenu->addAction("&Command");
  mainwin_commandact->setToolTip("open command window");
  connect(mainwin_commandact, &QAction::triggered,
	  rpsqt_app, &RpsTemp_Application::do_open_command);
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
  connect(mainwin_objbrowser, &RpsTemp_ObjectBrowser::need_refresh_display,
	  mainwin_objbrowser, &RpsTemp_ObjectBrowser::refresh_object_browser,
	  Qt::QueuedConnection);
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::fill_vbox end mainwin#"
		<< rank() << " showlabel¤" << mainwin_showlabel->rect()
		<< " showframe¤" << mainwin_showframe->rect()
		<< " objbrowser¤" << mainwin_objbrowser->rect());
} // end RpsTemp_MainWindow::fill_vbox




// slot when some text in mainwin_shownobject Qt5 widget has been entered
void
RpsTemp_MainWindow::do_enter_shown_object(void)
{
  RPSQT_WITH_LOCK();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object start mainwin#"
		<< rank());
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 /*callerframe:*/nullptr,
		 Rps_ObjectRef showob;
		 Rps_ObjectRef classob;
		 Rps_ObjectRef strbufob;
		 Rps_Value mainv;
		 Rps_Value xtrav;
		 );
  int ix = -1;
  RPS_ASSERT(mainwin_shownobject != nullptr);
  std::string obshowstring = mainwin_shownobject->text().toStdString();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object obshowstring="
		<< Rps_QuotedC_String(obshowstring)
		<< std::endl
		<< Rps_ShowCallFrame(&_)
		<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "incomplete RpsTemp_MainWindow::do_enter_shown_object"));
  _f.showob = Rps_ObjectRef::find_object_or_null_by_string(&_, obshowstring);
  if (_f.showob)
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object by name showob="
		  << _f.showob << " == " << Rps_OutputValue(_f.showob)
		  << " of class " << _f.showob->compute_class(&_)
		  << " == " << Rps_OutputValue(_f.showob->compute_class(&_)));
  else
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object no object for " << obshowstring);
  _f.classob = _f.showob?(_f.showob->compute_class(&_)):nullptr;
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object showob=" << _f.showob << " of class " << _f.classob);
  /****
   * we need to handle several cases:
   * first case: _f.showob is null; then display some error dialog
   * second case: _f.showob is already shown, then redisplay every object in this window.
   * third case: _f.showob is another object, then append it and redisplay every object
   ****/
  if (!_f.showob) {		// first case: no shown object, should display a Qt dialog and clear
    char warntitle[64];
    memset (warntitle, 0, sizeof(warntitle));
    snprintf (warntitle, sizeof(warntitle), "no object for window #%d", mainwin_rank);
    QString qwarndetails;
    mainwin_shownobject->clear();
    qwarndetails.append("Input <tt>");
    qwarndetails.append(obshowstring.c_str());
    qwarndetails.append("</tt> is <b>not</b> a valid RefPerSys object.");
#warning for some reason this Qt warning appears twice.
    /* FIXME: Basile tried to postpone with do_a_millisecond_later the display of
       the below Qt warning, but it does not improve ... */
    QMessageBox::warning(this, QString(warntitle), qwarndetails);
    return;
  }
  else if (mainwin_objbrowser->refpersys_object_is_shown(_f.showob, &ix)) { // second case, _f.showob is already shown
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object already shown object "
		  << _f.showob << " at index " << ix << " in window#" << mainwin_rank);
  }
  else { // third case, _f.showob is not shown
    mainwin_objbrowser->add_shown_object(_f.showob, obshowstring);
  }
  do_a_millisecond_later([this](RpsTemp_MainWindow*mainw) {
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object refreshing millisec window#" << mainw->mainwin_rank);
    mainw->mainwin_objbrowser->refresh_object_browser();
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::do_enter_shown_object did refresh_object_browser window#"
		  << mainw->mainwin_rank << " millisec"); 
  });
} // end RpsTemp_MainWindow::do_enter_shown_object

void
RpsTemp_MainWindow::do_a_millisecond_later(std::function<void(RpsTemp_MainWindow*w)> fun)
{
  QTimer::singleShot(1, Qt::CoarseTimer, [=](void){fun(this);});
} // end RpsTemp_MainWindow::do_a_millisecond_later

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
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  int nbshownob = (int)objbr_shownobvect.size();
  for (int shix=0; shix<nbshownob; shix++) {
    struct shown_object_st& curshob= objbr_shownobvect[shix];
    RPS_ASSERT(curshob.shob_obref);
    gc->mark_obj(curshob.shob_obref);
  }
} // end RpsTemp_ObjectBrowser::garbage_collect_object_browser

/// Return the default object display depth
int
RpsTemp_ObjectBrowser::default_display_depth(void) const
{
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  return objbr_defaultdepth;
} // end of RpsTemp_ObjectBrowser::default_display_depth


/// Set the default object display depth. Ensure it is reasonable.
void
RpsTemp_ObjectBrowser::put_default_display_depth(int newdepth)
{
  if (newdepth<=0)
    newdepth=1;
  else if (newdepth>_objbr_maxdepth)
    newdepth= _objbr_maxdepth;
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  objbr_defaultdepth = newdepth;
} // end RpsTemp_ObjectBrowser::put_default_display_depth


void
RpsTemp_ObjectBrowser::show_one_object_in_frame(Rps_CallFrame*callerframe, struct shown_object_st& shob)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(shob.shob_obref);
  RPS_ASSERT(shob.shob_depth >= 0);
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  RPSQT_WITH_LOCK();
  RPS_LOCALFRAME(nullptr,
                 callerframe, //
		 Rps_ObjectRef showob;
		 Rps_ObjectRef strbufob;
		 Rps_Value mainv;
		 Rps_Value xtrav;
		 );
  int displaydepth = shob.shob_depth;
  _f.showob = shob.shob_obref;
  RPS_DEBUGNL_LOG(GUI, "RpsTemp_MainWindow::show_one_object_in_frame start displaydepth=" << displaydepth
		  << " showob=" << _f.showob << " cursorpos=" << cursor_position()
		  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "start RpsTemp_ObjectBrowser::show_one_object_in_frame"));
  move_cursor_at_end();
  /// should send selector display_object_content_web to showob with arguments strbufob depth=tagged<>
  _f.strbufob = Rps_PayloadStrBuf::make_string_buffer_object(&_);
  {
    RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::show_one_object_in_frame sending display_object_content_web to showob="
		  << _f.showob << " cursorpos=" << cursor_position());
    Rps_TwoValues two = //
      Rps_Value(_f.showob).send2(&_, //
				 RPS_ROOT_OB(_02iWbXmFx8f04ldLRt), //"display_object_content_web"∈named_selector
				 _f.strbufob,
				 Rps_Value::make_tagged_int(displaydepth));
    _f.mainv = two.main();
    _f.xtrav = two.xtra();
  }
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::show_one_object_in_frame after display_object_content_web showob="
		<< _f.showob << " mainv=" << _f.mainv);
  Rps_PayloadStrBuf*paylsbuf =
    _f.strbufob->get_dynamic_payload<Rps_PayloadStrBuf>();
  RPS_ASSERT(paylsbuf != nullptr);
  std::string outstr = paylsbuf->buffer_cppstring();
  RPS_DEBUG_LOG(GUI, "RpsTemp_ObjectBrowser::show_one_object_in_frame "
	      << " showob=" << _f.showob << " strbufob=" << _f.strbufob
	      << " mainv=" << Rps_OutputValue(_f.mainv)
	      << " xtrav=" << Rps_OutputValue(_f.xtrav)
		<< " cursorpos=" << cursor_position()
	      << "::::" << std::endl
	      << outstr
	      << std::endl << RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_ObjectBrowser::show_one_object_in_frame"));
  QString qoutstr = QString::fromStdString(outstr);
  insertHtml(qoutstr);
  RPS_DEBUG_LOG(GUI, "RpsTemp_ObjectBrowser::show_one_object_in_frame ending "
		<< " showob=" << _f.showob
		<< " cursorpos=" << cursor_position());
} // end RpsTemp_ObjectBrowser::show_one_object_in_frame


int
RpsTemp_ObjectBrowser::cursor_position(void) const {
  QTextCursor tcur = textCursor();
  return tcur.position();
} // end RpsTemp_ObjectBrowser::cursor_position


void
RpsTemp_ObjectBrowser::move_cursor_at_end(void) {
  moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
} // end RpsTemp_ObjectBrowser::move_cursor_at_end


bool
RpsTemp_ObjectBrowser::refpersys_object_is_shown(Rps_ObjectRef ob, int* pix) const
{
  if (!ob)
    return false;
  RPSQT_WITH_LOCK();
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  auto it =  objbr_mapshownob.find(ob);
  if (it ==  objbr_mapshownob.end()) {
    if (pix)
      *pix = -1;
    return false;
  }
  if (pix)
    *pix = it->second;
  return true;
} // end RpsTemp_ObjectBrowser::refpersys_object_is_shown

void
RpsTemp_ObjectBrowser::add_shown_object(Rps_ObjectRef ob, std::string htmlsubtitle, int depth)
{
  if (!ob)
    return;
  RPSQT_WITH_LOCK();
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  int defdepth = default_display_depth();
  RPS_ASSERT(defdepth>0);
  if (depth<=0)
    depth=defdepth;
  struct shown_object_st shob;
  shob.shob_obref=ob;
  shob.shob_depth=depth;
  shob.shob_subtitle=htmlsubtitle;
  auto it = objbr_mapshownob.find(ob);
  if (it == objbr_mapshownob.end()) {
    int nbshownobjs = (int) objbr_mapshownob.size();
    objbr_mapshownob.insert({ob, nbshownobjs});
    objbr_shownobvect.push_back(shob);
  }
  else
    objbr_shownobvect[it->second] = shob;
  emit need_refresh_display();
} // end RpsTemp_ObjectBrowser::add_shown_object

void
RpsTemp_ObjectBrowser::remove_shown_object(Rps_ObjectRef ob)
{
  if (!ob)
    return;
  RPSQT_WITH_LOCK();
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  auto it = objbr_mapshownob.find(ob);
  if (it == objbr_mapshownob.end())
    return;
  int oix = it->second;
  RPS_ASSERT(oix>=0 && oix < (int)objbr_shownobvect.size());
  objbr_mapshownob.erase(it);
  objbr_shownobvect.erase(objbr_shownobvect.begin()+oix);
  for (int curix=oix; curix<(int)objbr_shownobvect.size(); curix++) {
    Rps_ObjectRef curob = objbr_shownobvect[curix].shob_obref;
    RPS_ASSERT(curob);
    objbr_mapshownob[curob] = curix;
  }
  emit need_refresh_display();
} // end RpsTemp_ObjectBrowser::remove_shown_object



void
RpsTemp_ObjectBrowser::refresh_object_browser(void)
{
  RPSQT_WITH_LOCK();
  std::lock_guard<std::recursive_mutex> curguard(objbr_mtx);
  int nbshob = (int) objbr_shownobvect.size();
  RPS_LOCALFRAME(/*descr:*/nullptr,
                 /*callerframe:*/nullptr,
		 Rps_ObjectRef obstrbuf;
		 Rps_ObjectRef obshown;
		 );
  QWidget* parwid = parentWidget();
  RpsTemp_MainWindow* mainwin = dynamic_cast<RpsTemp_MainWindow*>(window());
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::refresh_object_browser start parwid@" << parwid
		<< "€" << parwid->metaObject()->className());
  RPS_ASSERT(mainwin != nullptr);
  int winrank = mainwin->rank();
  RPS_DEBUG_LOG(GUI, "RpsTemp_MainWindow::refresh_object_browser winrank#"
		<< winrank << " nbshob=" << nbshob
		<< " cursorpos=" << cursor_position()
		<< " start from " << std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_MainWindow::refresh_object_browser start"));
  //// The shown objects should be GC marked thru the windows
  //// containing them, so we hope that adding a GC marking extra
  //// routine is not needed here.
  clear();
  if (nbshob==0) {
    setHtml(QString("<h1>object browser</h1>\n"));
    RPS_DEBUGNL_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser no object 'object browser'");
  }
  else if (nbshob==1) {
    setHtml(QString("<h1>browser for one object</h1>\n"));
    RPS_DEBUGNL_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser 'browser for one object'");
  }
  else {
    char bufnbshob[16];
    memset (bufnbshob, 0, sizeof(bufnbshob));
    snprintf (bufnbshob, sizeof(bufnbshob), "%d", nbshob);
    setHtml(QString("<h1>browser for ") + QString(bufnbshob) + QString(" objects</h1>\n"));
    RPS_DEBUGNL_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser 'browser for " << nbshob <<" objects'");
  }
  RPS_DEBUGNL_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser after title cursorpos=" << cursor_position()
		  << " window rank#" << winrank);
  _f.obstrbuf = Rps_PayloadStrBuf::make_string_buffer_object(&_);
  Rps_PayloadStrBuf*paylsbuf = _f.obstrbuf->get_dynamic_payload<Rps_PayloadStrBuf>();
  RPS_ASSERT (paylsbuf != nullptr);
  for (int obix=0; obix<nbshob; obix++) {
    paylsbuf->clear_buffer();
    std::ostream* poutsbuf = paylsbuf->output_string_stream_ptr();
    RPS_ASSERT(poutsbuf);
    _f.obshown = objbr_shownobvect[obix].shob_obref;
    int showdepth =  objbr_shownobvect[obix].shob_depth;
    std::string subtitle = objbr_shownobvect[obix].shob_subtitle;
    RPS_DEBUGNL_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser obix#" << obix
		    << " obshown=" << _f.obshown << " showdepth=" << showdepth
		    << " subtitle=" << subtitle
		    << " cursorpos=" << cursor_position()
		    << " poutsbuf/tellp:" << poutsbuf->tellp());
    move_cursor_at_end();
    insertHtml("<hr/>\n");
    move_cursor_at_end();
    show_one_object_in_frame(&_, objbr_shownobvect[obix]);
    *poutsbuf << std::flush;
    RPS_DEBUG_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser obix#" << obix
		  << " cursorpos=" << cursor_position()
		  << " poutsbuf/tellp:" << poutsbuf->tellp()
		  << " obshown=" << _f.obshown << " after show_one_object_in_frame from " << std::endl
		  << Rps_ShowCallFrame(&_) << std::endl
		  << RPS_FULL_BACKTRACE_HERE(1, "RpsTemp_ObjectBrowser::refresh_object_browser loop"));
  }
  RPS_DEBUG_LOG(GUI, "RpsTemp_ObjectBrowser::refresh_object_browser ending cursorpos=" << cursor_position() << " winrank#" << winrank
	      << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "ending RpsTemp_ObjectBrowser::refresh_object_browser")
	      << std::endl);
} // end RpsTemp_ObjectBrowser::refresh_object_browser

////////////////////////////////////////////////////////////////
///// the linedit Qt widget to enter some object by id or name
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



////////////////////////////////////////////////////////////////
RpsTemp_CommandEdit::RpsTemp_CommandEdit(QWidget*parent)
  : QTextEdit::QTextEdit(parent)
{
} // end RpsTemp_CommandEdit::RpsTemp_CommandEdit

void
RpsTemp_CommandEdit::garbage_collect_command_edit(Rps_GarbageCollector*gc)
{
} // end RpsTemp_CommandEdit::garbage_collect_command_edit


////////////////////////////////////////////////////////////////
void
rps_qtgui_init_progarg(int &argc, char**argv)
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
} // end rps_qtgui_init_progarg



void
rps_qtgui_run(void)
{
  RPS_INFORMOUT("rps_qtgui_run start:"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "start rps_qtgui_run"));
  RPS_ASSERT(rpsqt_app != nullptr);
  RPS_DEBUG_LOG(GUI, "rps_qtgui_run before one RpsTemp_Application::processEvents");
  /* In principle, these are useless. Since rpsqt_app->exec should do it. */
  RpsTemp_Application::processEvents();
  RPS_DEBUG_LOG(GUI, "rps_qtgui_run after one RpsTemp_Application::processEvents"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "in rps_qtgui_run"));
  RpsTemp_Application::sendPostedEvents();
  RPS_DEBUG_LOG(GUI, "rps_qtgui_run after one RpsTemp_Application::sendPostedEvents");
  usleep(1000);
  RPS_DEBUG_LOG(GUI, "rps_qtgui_run before exec"<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "rps_qtgui_run/before exec"));
  int ok = rpsqt_app->exec();
  RPS_DEBUG_LOG(GUI, "rps_qtgui_run after exec ok=" << ok<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "ending rps_qtgui_run"));
} //  end rps_qtgui_run

//// end of file qtgui-qrps.cc for refpersys.org

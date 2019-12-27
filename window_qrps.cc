/****************************************************************
 * file window_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the Qt5 code related to the Qt5 window
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
#include <sstream>

#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFile>
#include <QLabel>
#include <QDebug>

extern "C" const char rps_window_gitid[];
const char rps_window_gitid[]= RPS_GITID;

extern "C" const char rps_window_date[];
const char rps_window_date[]= __DATE__;


RpsQPixMap* RpsQPixMap::m_instance = nullptr;


RpsQWindow::RpsQWindow (QWidget *parent)
  : QMainWindow (parent)
{
  this->setupAppMenu ();
  this->setupHelpMenu ();

  qApp->setAttribute (Qt::AA_DontShowIconsInMenus, false);

  auto vbox = new QVBoxLayout (this);
  vbox->setSpacing (1);

  menuBar ()->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
  vbox->addWidget (menuBar ());

  setup_debug_widget();
  vbox->addWidget (&m_debug_widget);

  setLayout (vbox);
} // end RpsQWindow::RpsQWindow


void
RpsQWindow::setupAppMenu()
{
  auto pixmap = RpsQPixMap::instance();

  QAction *dump = new QAction(pixmap->get("RPS_ICON_DUMP"), "&Dump", this);
  QAction *gc = new QAction(
    pixmap->get("RPS_ICON_GC"),
    "&Garbage Collect",
    this
  );
  QAction *quit = new QAction(pixmap->get("RPS_ICON_QUIT"), "&Quit", this);
  QAction *exit = new QAction(pixmap->get("RPS_ICON_EXIT"), "e&Xit", this);
  QAction *close = new QAction(pixmap->get("RPS_ICON_CLOSE"), "&Close", this);
  QAction *newin = new QAction(pixmap->get("RPS_ICON_NEW"), "New &Window", this);

  quit->setShortcut (tr ("CTRL+Q"));
  exit->setShortcut (tr ("CTRL+X"));
  dump->setShortcut (tr ("CTRL+D"));
  gc->setShortcut (tr ("CTRL+G"));
  newin->setShortcut (tr ("CTRL+W")); //TODO: doesn't CTRL+N seem better?
  close->setShortcut (tr ("CTRL+C"));

  QMenu *app_menu;
  app_menu = menuBar ()->addMenu ("&App");
  app_menu->addAction (dump);
  app_menu->addAction (gc);
  app_menu->addAction (newin);
  app_menu->addSeparator ();
  app_menu->addAction (close);
  app_menu->addAction (quit);
  app_menu->addAction (exit);

  connect (dump, &QAction::triggered, this, &RpsQWindow::onMenuDump);
  connect (gc, &QAction::triggered, this, &RpsQWindow::onMenuGarbageCollect);
  connect (quit, &QAction::triggered, this, &RpsQWindow::onMenuQuit);
  connect (exit, &QAction::triggered, this, &RpsQWindow::onMenuExit);
  connect (newin, &QAction::triggered,
           dynamic_cast<RpsQApplication*>(RpsQApplication::instance()),
           &RpsQApplication::add_new_window);
  connect (close, &QAction::triggered, this, &RpsQWindow::onMenuClose);
} // end RpsQWindow::setupAppMenu


void RpsQWindow::setupHelpMenu()
{
  QPixmap about_px ("about_icon.png");
  QPixmap debug_px ("debug_icon.png");

  QAction *about_ax = new QAction (about_px, "&About", this);
  QAction *debug_ax = new QAction (debug_px, "&Debug", this);

  QMenu *help_menu;
  help_menu = menuBar ()->addMenu ("&Help");
  help_menu->addAction (about_ax);
  help_menu->addAction (debug_ax);

  connect (about_ax, &QAction::triggered, this, &RpsQWindow::onMenuAbout);
  connect (debug_ax, &QAction::triggered, this, &RpsQWindow::onMenuDebug);
} // end  RpsQWindow::setupHelpMenu


void
RpsQWindow::setup_debug_widget()
{
  m_debug_widget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_debug_widget.setReadOnly(true);
  m_debug_widget.setTextInteractionFlags(
    m_debug_widget.textInteractionFlags() | Qt::TextSelectableByKeyboard
  );
}


void
RpsQWindow::setup_debug_timer()
{
  connect(&m_debug_timer, SIGNAL(timeout()), this, SLOT(update_debug_widget()));
}


void
RpsQWindow::onMenuQuit()
{
  auto msg = QString("Are you sure you want to quit without dumping?");
  auto btn = QMessageBox::Yes | QMessageBox::No;
  auto reply = QMessageBox::question(this, "RefPerSys", msg, btn);

  if (reply == QMessageBox::Yes)
    QApplication::quit();
} // end  RpsQWindow::onMenuQuit


void
RpsQWindow::onMenuClose()
{
  auto app = dynamic_cast<RpsQApplication*> (RpsQApplication::instance());

  if (app->getWindowCount () > 1)
    {
      app->lowerWindowCount ();
      this->close();
    }
  else
    {
      this->onMenuQuit ();
    }
}


void
RpsQWindow::onMenuExit()
{
  rps_dump_into ();
  QApplication::quit ();
} // end RpsQWindow::onMenuExit

void
RpsQWindow::onMenuDump()
{
  rps_dump_into ();
} // end RpsQWindow::onMenuDump



void
RpsQWindow::onMenuGarbageCollect()
{
  rps_garbage_collect();
} // end RpsQWindow::onMenuGarbageCollect


void
RpsQWindow::onMenuAbout()
{
  std::ostringstream msg;
  msg << "RefPerSys Git ID: " << RpsColophon::git_id()
      << "\nBuild Date: " << RpsColophon::timestamp()
      << "\nMD5 Sum of Source: " << RpsColophon::source_md5()
      << "\nLast Git Commit: " << RpsColophon::last_git_commit()
      << "\nRefPerSys Top Directory: " << RpsColophon::top_directory()
      << "\n\nSee " << RpsColophon::website();

  QMessageBox::information (this, "About RefPerSys", msg.str().c_str());
} // end RpsQWindow::onMenuAbout


void
RpsQWindow::onMenuDebug()
{
  m_debug_timer.start(1000);
  update_debug_widget();
} // end RpsQWindow::onMenuDebug


void
RpsQWindow::update_debug_widget()
{
  QFile log ("_refpersys.log");

  if (log.open (QFile::ReadOnly | QFile::Text))
    {
      m_debug_widget.setPlainText (log.readAll());
      m_debug_widget.show();
      log.close();
    }

  else
    {
      qDebug() << "Failed to open debug log";
      return;
    }
}


//////////////////////////////////////// end of file window_qrps.cc


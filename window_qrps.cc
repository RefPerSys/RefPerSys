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

#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

extern "C" const char rps_window_gitid[];
const char rps_window_gitid[]= RPS_GITID;

extern "C" const char rps_window_date[];
const char rps_window_date[]= __DATE__;


RpsQWindow::RpsQWindow (QWidget *parent)
  : QMainWindow (parent)
{
  this->setupAppMenu ();
  this->setupHelpMenu ();

  qApp->setAttribute (Qt::AA_DontShowIconsInMenus, false);
} // end RpsQWindow::RpsQWindow


void
RpsQWindow::setupAppMenu()
{
  QPixmap dump_px ("dump_icon.png");
  QPixmap gc_px ("gc_icon.png");
  QPixmap quit_px ("quit_icon.png");
  QPixmap exit_px ("exit_icon.png");

  QAction *dump_ax = new QAction (dump_px, "&Dump", this);
  QAction *gc_ax = new QAction (gc_px, "&Garbage Collect", this);
  QAction *quit_ax = new QAction (quit_px, "&Quit", this);
  QAction *exit_ax = new QAction (exit_px, "e&Xit", this);
  QAction *close_ax = new QAction (exit_px, "&Close", this);
  QAction *newin_ax = new QAction (exit_px, "New &Window", this);

  quit_ax->setShortcut (tr ("CTRL+Q"));
  exit_ax->setShortcut (tr ("CTRL+X"));
  dump_ax->setShortcut (tr ("CTRL+D"));
  gc_ax->setShortcut (tr ("CTRL+G"));
  newin_ax->setShortcut (tr ("CTRL+W")); //TODO: doesn't CTRL+N seem better?
  close_ax->setShortcut (tr ("CTRL+C"));

  QMenu *app_menu;
  app_menu = menuBar ()->addMenu ("&App");
  app_menu->addAction (dump_ax);
  app_menu->addAction (gc_ax);
  app_menu->addAction (newin_ax);
  app_menu->addSeparator ();
  app_menu->addAction (close_ax);
  app_menu->addAction (quit_ax);
  app_menu->addAction (exit_ax);

  connect (dump_ax, &QAction::triggered, this, &RpsQWindow::onMenuDump);
  connect (gc_ax, &QAction::triggered, this, &RpsQWindow::onMenuGarbageCollect);
  connect (quit_ax, &QAction::triggered, this, &RpsQWindow::onMenuQuit);
  connect (exit_ax, &QAction::triggered, this, &RpsQWindow::onMenuExit);
  connect (newin_ax, &QAction::triggered,
           dynamic_cast<RpsQApplication*>(RpsQApplication::instance()),
           &RpsQApplication::add_new_window);
  connect (close_ax, &QAction::triggered, this, &RpsQWindow::onMenuClose);
} // end RpsQWindow::setupAppMenu


void RpsQWindow::setupHelpMenu()
{
  QPixmap about_px ("about_icon.png");

  QAction *about_ax = new QAction (about_px, "&About", this);

  QMenu *help_menu;
  help_menu = menuBar ()->addMenu ("&Help");
  help_menu->addAction (about_ax);

  connect (about_ax, &QAction::triggered, this, &RpsQWindow::onMenuAbout);
} // end  RpsQWindow::setupHelpMenu


void
RpsQWindow::onMenuQuit()
{
  auto msg = QString ("Are you sure you want to quit without dumping?");
  auto btn = QMessageBox::Yes | QMessageBox::No;
  auto reply = QMessageBox::question (this, "RefPerSys", msg, btn);

  if (reply == QMessageBox::Yes)
    QApplication::quit ();
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
  QString msg ("RefPerSys Git ID: ");
  msg.append (rps_gitid);
  msg.append ("\n");
  msg.append ("Build date: ");
  msg.append (rps_timestamp);
  msg.append ("\n");
  msg.append ("Md5sum of source: ");
  msg.append (rps_md5sum);
  msg.append ("\n");
  msg.append ("last git commit: ");
  msg.append(rps_lastgitcommit);
  msg.append ("\n");
  msg.append ("RefPerSys top directory: ");
  msg.append(rps_topdirectory);
  msg.append("\n");
  msg.append("see http://refpersys.org/\n");

  QMessageBox::information (this, "About RefPerSys", msg);
} // end RpsQWindow::onMenuAbout


//////////////////////////////////////// end of file window_qrps.cc

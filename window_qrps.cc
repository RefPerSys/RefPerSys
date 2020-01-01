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
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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
  : QMainWindow(parent),
    m_menu_bar(this)

{
  qApp->setAttribute (Qt::AA_DontShowIconsInMenus, false);

  auto vbox = new QVBoxLayout();
  vbox->setSpacing(1);
  vbox->addWidget(menuBar());

  setup_debug_widget();
  vbox->addWidget(&m_debug_widget);
} // end RpsQWindow::RpsQWindow


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


RpsQMenuAction::RpsQMenuAction(
  RpsQWindow* parent,
  RpsQWindowMenu menu,
  std::string icon,
  std::string title,
  std::string shortcut
)
  : m_parent(parent)
{
  auto pix = RpsQPixMap::instance()->get(icon);
  auto action = new QAction(pix, title.c_str(), m_parent);
  action->setShortcut(tr(shortcut.c_str()));

  auto item = m_parent->menuBar()->findChildren<QMenu*>().at(menu);
  item->addAction(action);
  m_parent->connect(
    action,
    &QAction::triggered,
    this,
    &RpsQMenuAction::on_trigger
  );
}


void RpsQMenuHelpAbout::on_trigger()
{
  std::ostringstream msg;
  msg << "RefPerSys Git ID: " << RpsColophon::git_id()
      << "\nBuild Date: " << RpsColophon::timestamp()
      << "\nMD5 Sum of Source: " << RpsColophon::source_md5()
      << "\nLast Git Commit: " << RpsColophon::last_git_commit()
      << "\nRefPerSys Top Directory: " << RpsColophon::top_directory()
      << "\n\nSee " << RpsColophon::website();

  QMessageBox::information (window(), "About RefPerSys", msg.str().c_str());
}


void RpsQMenuHelpDebug::on_trigger()
{
  auto wnd = window();
  wnd->m_debug_timer.start(1000);
  wnd->update_debug_widget();
}


void RpsQMenuAppQuit::on_trigger()
{
  auto msg = QString("Are you sure you want to quit without dumping?");
  auto btn = QMessageBox::Yes | QMessageBox::No;
  auto reply = QMessageBox::question(window(), "RefPerSys", msg, btn);

  if (reply == QMessageBox::Yes)
    QApplication::quit();
}


void RpsQMenuAppExit::on_trigger()
{
  rps_dump_into();
  QApplication::quit();
}


void RpsQMenuAppClose::on_trigger()
{
  auto app = window()->application();

  if (app->getWindowCount() > 1)
    {
      app->lowerWindowCount();
      window()->close();
    }
  else
    {
      auto msg = QString("Are you sure you want to quit without dumping?");
      auto btn = QMessageBox::Yes | QMessageBox::No;
      auto reply = QMessageBox::question(window(), "RefPerSys", msg, btn);

      if (reply == QMessageBox::Yes)
        QApplication::quit();
    }
}


void RpsQMenuAppDump::on_trigger()
{
  rps_dump_into();
}


void RpsQMenuAppGC::on_trigger()
{
  rps_garbage_collect();
}


void RpsQMenuAppNew::on_trigger()
{
  window()->application()->add_new_window();
}


RpsQWindowMenuBar::RpsQWindowMenuBar(RpsQWindow* parent)
  : m_parent(parent)
{
  auto app_menu = m_parent->menuBar()->addMenu("&App");
  m_menu_app_dump = std::make_shared<RpsQMenuAppDump>(m_parent);
  m_menu_app_gc = std::make_shared<RpsQMenuAppGC>(m_parent);
  m_menu_app_new = std::make_shared<RpsQMenuAppNew>(m_parent);
  app_menu->addSeparator();
  m_menu_app_close = std::make_shared<RpsQMenuAppClose>(m_parent);
  m_menu_app_quit = std::make_shared<RpsQMenuAppQuit>(m_parent);
  m_menu_app_exit = std::make_shared<RpsQMenuAppExit>(m_parent);

  m_parent->menuBar()->addMenu("&Help");
  m_menu_help_about = std::make_shared<RpsQMenuHelpAbout>(m_parent);
  m_menu_help_debug = std::make_shared<RpsQMenuHelpDebug>(m_parent);

  m_parent->menuBar()->setSizePolicy(
    QSizePolicy::Expanding,
    QSizePolicy::Expanding
  );
}

//////////////////////////////////////// end of file window_qrps.cc


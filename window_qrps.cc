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

RpsQWindow::RpsQWindow (QWidget *parent) 
    : QMainWindow (parent)
{
    QPixmap dump_px ("dump_icon.png");
    QPixmap gc_px ("gc_icon.png");
    QPixmap quit_px ("quit_icon.png");

    QAction *dump_ax = new QAction (dump_px, "&Dump", this);
    QAction *gc_ax = new QAction (gc_px, "&Garbage Collect", this);
    QAction *quit_ax = new QAction (quit_px, "&Quit", this);

    quit_ax->setShortcut (tr ("CTR+Q"));

    QMenu *app_menu;
    app_menu = menuBar ()->addMenu ("&App");
    app_menu->addAction (dump_ax);
    app_menu->addAction (gc_ax);
    app_menu->addSeparator ();
    app_menu->addAction (quit_ax);

    qApp->setAttribute (Qt::AA_DontShowIconsInMenus, false);

    connect (quit_ax, &QAction::triggered, qApp, &QApplication::quit);
} // end RpsQWindow::RpsQWindow 


//////////////////////////////////////// end of file window_qrps.cc

/****************************************************************
 * file tempgui-qrps.hh
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


#ifndef TEMPGUI_QRPS_INCLUDED
#define TEMPGUI_QRPS_INCLUDED

#include "refpersys.hh"
#include <QApplication>
#include <QTextBrowser>
#include <QtWidgets>

/***
  The Qt toolkit is known to run several threads internally. The
  current RefPerSys runtime and garbage collector is not multi-thread
  friendly. So by convention every Qt related code which deals with
  RefPerSys values should do that under protection of some
  mutex. Otherwise, non-reproducible crashes or unexpected behavior is
  very likely to happen.
 ***/
extern "C" std::recursive_mutex rpsqt_mtx;
#define RPSQT_WITH_LOCK() rpsqt_mtx.lock()
#define RPSQT_LOCKED(Foo) ({rpsqt_mtx.lock(); (Foo);})


class RpsTemp_Application : public QApplication {
Q_OBJECT
public slots:
  void do_dump(void);
  void do_exit(void);
  void do_quit(void);
public:
  RpsTemp_Application(int&argc, char**argv);
};				// end RpsTemp_Application

extern "C" RpsTemp_Application* rpsqt_app;

class RpsTemp_ObjectBrowser : public QTextBrowser {
  Q_OBJECT
public:
  RpsTemp_ObjectBrowser();
#warning class RpsTemp_ObjectBrowser is incomplete
};				// end RpsTemp_ObjectBrowser

class RpsTemp_MainWindow : public QMainWindow {
  Q_OBJECT
  int mainwin_rank;
  /// actions in the top menubar
  QAction* mainwin_dumpact;
  QAction* mainwin_quitact;
  QAction* mainwin_exitact;
  //// the central widget is an object browser
  RpsTemp_ObjectBrowser* mainwin_objbrowser;
protected:
  static std::set<RpsTemp_MainWindow*> mainwin_set_;
  void create_menus(void);
public:
  int rank() const { return mainwin_rank; };
  RpsTemp_ObjectBrowser* objbrowser() const { return mainwin_objbrowser; };
  RpsTemp_MainWindow();
};				// end class RpsTemp_MainWindow

extern "C" void rps_tempgui_init_progarg(int &argc, char**argv);
extern "C" void rps_tempgui_run(void);

#endif /*TEMPGUI_QRPS_INCLUDED*/
//// end of file tempgui_qrps.hh for refpersys.org

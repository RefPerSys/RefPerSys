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

extern std::ostream& operator << (std::ostream&, const QRect&);

class RpsTemp_Application : public QApplication {
Q_OBJECT
public slots:
  void do_dump(void);
  void do_exit(void);
  void do_quit(void);
  void do_new_window(void);
  void do_garbage_collect(void);
public:
  RpsTemp_Application(int&argc, char**argv);
protected:
  void xtra_gc_mark(Rps_GarbageCollector*gc);
};				// end RpsTemp_Application

extern "C" RpsTemp_Application* rpsqt_app;

class RpsTemp_ObjectCompleter;	// forward declaration

/// a single line entry with autocompletion for RefPerSys objects
class RpsTemp_ObjectLineEdit : public QLineEdit {
  ///- conventional field prefix: oblined_
  Q_OBJECT
  RpsTemp_ObjectCompleter*oblined_completer; 
public:
  RpsTemp_ObjectLineEdit(QWidget*parent=nullptr);
};				// end RpsTemp_ObjectLineEdit



class RpsTemp_ObjectCompleter : public QCompleter {
  Q_OBJECT
public:
  RpsTemp_ObjectCompleter(QObject*);
};				// end RpsTemp_ObjectCompleter

/// the object browser shows (using HTML) the content of RefPerSys objects
class RpsTemp_ObjectBrowser : public QTextBrowser {
  Q_OBJECT
  struct shown_object_st {
    Rps_ObjectRef shob_obref;	// the shown object
    int shob_depth;		// the display depth
    std::string shob_subtitle;	// the optional subtitle string
  };
  mutable std::mutex objbr_mtx;
  int objbr_defaultdepth;
  static constexpr int _objbr_maxdepth = 8;
  std::vector<shown_object_st> objbr_shownobvect;
  std::map<Rps_ObjectRef,int> objbr_mapshownob;
  /*** NOTICE
       We want to use
       https://doc.qt.io/qt-5/qtextedit.html#insertHtml, since a
       QTextBrowser inherits from QTextEdit
   ***/
public:
  RpsTemp_ObjectBrowser(QWidget*parent=nullptr);
  /// GC support
  void garbage_collect_object_browser(Rps_GarbageCollector*gc);
  /// fetch the default display depth
  int default_display_depth(void) const;
  /// put the default display depth
  void put_default_display_depth(int newdepth);
  bool refpersys_object_is_shown(Rps_ObjectRef ob) const;
public slots:
  /// Add at end a shown object, if it was not shown, or update its
  /// title and depth, if it was already shown
  void add_shown_object(Rps_ObjectRef ob, std::string htmlsubtitle=nullptr, int depth=0);
  /// Remove a shown object
  void remove_shown_object(Rps_ObjectRef ob);
  void refresh_object_browser(void);
signals:
  void need_refresh_display(void);
#warning class RpsTemp_ObjectBrowser is incomplete
};				// end RpsTemp_ObjectBrowser



class RpsTemp_MainWindow : public QMainWindow {
  Q_OBJECT
  ///- conventional field prefix: mainwin_
  int mainwin_rank;
  /// actions in the top menubar
  QAction* mainwin_dumpact;
  QAction* mainwin_quitact;
  QAction* mainwin_exitact;
  QAction* mainwin_newact;
  QAction* mainwin_garbcollact;
  //// the central windget is a frame
  QFrame* mainwin_centralframe;
  //// containing a vertical box
  QVBoxLayout* mainwin_vbox;
  //// .... containing an horizontal frame for showing objects
  QFrame* mainwin_showframe;
  QHBoxLayout*mainwin_showhbox;
  QLabel* mainwin_showlabel;
  RpsTemp_ObjectLineEdit* mainwin_shownobject;
  RpsTemp_ObjectCompleter* mainwin_showncompleter;
  //// .... containing an object browser
  RpsTemp_ObjectBrowser* mainwin_objbrowser;
protected:
  static std::set<RpsTemp_MainWindow*> mainwin_set_;
  void create_menus(void);
  void fill_vbox();
  void garbage_collect_main_window(Rps_GarbageCollector*gc);
public:
  static void garbage_collect_all_main_windows(Rps_GarbageCollector*gc);
  int rank() const { return mainwin_rank; };
  RpsTemp_ObjectBrowser* objbrowser() const { return mainwin_objbrowser; };
  RpsTemp_MainWindow();
public slots:
  void do_enter_shown_object(void);
};				// end class RpsTemp_MainWindow

extern "C" void rps_tempgui_init_progarg(int &argc, char**argv);
extern "C" void rps_tempgui_run(void);

#endif /*TEMPGUI_QRPS_INCLUDED*/
//// end of file tempgui_qrps.hh for refpersys.org

/****************************************************************
 * file qthead_qrps.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its internal Qt5/C++ declaring Qt classes
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

#ifndef QTHEAD_QRPS_INCLUDED
#define QTHEAD_QRPS_INCLUDED 1

// ensure that "refpersys.hh" has been included before
#ifndef REFPERSYS_INCLUDED
#error the refpersys.hh header should be included before this one
#endif /*REFPERSYS_INCLUDED*/


// By convention, only Qt related classes, using Q_OBJECT, should go
// into this file...


#include <QApplication>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTimer>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QCommandLineParser>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QFile>
#include <QLabel>
#include <QDebug>
#include <QFont>
#include <QMdiArea>
#include <QScreen>
#include <QDesktopWidget>
#include <QThread>

class RpsQApplication;
class RpsQWindow;
class RpsQObjectLineEdit;// a line edit for a RefPerSys object


//////////////////////////////////////////////////////////// RpsQApplication
//// our application class
class RpsQApplication
  : public QApplication
{
  friend void rps_run_application(int &argc, char **argv);
  friend bool rps_is_main_gui_thread(void);
  Q_OBJECT;
public:
  RpsQApplication (int &argc, char*argv[]); // constructor
  //// the json reading methods can fail by throwing some exception
  // read a json default file ~/refpersys-user.json:
  Json::Value read_user_json(void);
  // read the application default file app-refpersys.json:
  Json::Value read_application_json(void);
  // read some json file in the current working directory:
  Json::Value read_current_json(const std::string jsonpath);
  // get the nth window:
  RpsQWindow* getWindowPtr(int index);
  RpsQWindow& getWindow(int index)
  {
    auto w = getWindowPtr(index);
    if (!w)
      throw std::runtime_error("bad window index in RpsQApplication::getWindow");
    return *w;
  };
  static RpsQApplication* the_app(void)
  {
    return dynamic_cast<RpsQApplication*>(RpsQApplication::instance());
  };

public slots:
  void do_dump_state(QString dirpath=".");
  void do_dump_current_state();
  void do_add_new_window(void);
  void do_dump_then_exit(QString dirpath=".");
  void do_dump_current_then_exit(void);
  void do_remove_window_by_index(int index);
  void do_remove_window(RpsQWindow* win);

private:

  /// various ways to see the main GUI tread
  static QThread* app_mainqthread;
  static pthread_t app_mainselfthread;
  static std::thread::id app_mainthreadid;
  std::mutex app_mutex;
  std::vector <std::unique_ptr<RpsQWindow>> app_windvec;
  size_t app_wndcount;
};				// end of class RpsQApplication


///////////////////////////////////////////////////////////////////////////////
/// The line editor, with auto completion, for RefPerSys objects
//////////////////////////////////////////////////////////////////////////////


// the completer
class RpsQObjectCompleter : public QCompleter /// in file window_qrps.cc
{
  Q_OBJECT
private:
  QStringListModel qobcompl_strlistmodel;
public:
  RpsQObjectCompleter(QObject*parent = nullptr);
  static int constexpr max_nb_autocompletions = 32;
public slots:
  void update_for_text(const QString&);
};					    // end RpsQObjectCompleter

// the line edit
class RpsQObjectLineEdit : public QLineEdit /// in file window_qrps.cc
{
  Q_OBJECT
private:
  std::unique_ptr<RpsQObjectCompleter> qoblinedit_completer;
public:
  RpsQObjectLineEdit(const QString &contents= "",
                     const QString& placeholder= "", QWidget *parent = nullptr);
};				// end RpsQObjectLineEdit




///////////////////////////////////////////////////////////////////////////////
/// The dialog box for creating new classes.
//////////////////////////////////////////////////////////////////////////////
class RpsQCreateClassDialog : public QDialog
{
  Q_OBJECT
public:
  RpsQCreateClassDialog(RpsQWindow* parent);
  ~RpsQCreateClassDialog();

private slots:
  void on_ok_trigger();
  void on_cancel_trigger();


private:
  QVBoxLayout dialog_vbox;
  QHBoxLayout superclass_hbox;
  QLabel superclass_label;
  RpsQObjectLineEdit superclass_linedit;
  QHBoxLayout classname_hbox;
  QLabel classname_label;
  QLineEdit classname_linedit;
  QHBoxLayout button_hbox;
  QPushButton ok_button;
  QPushButton cancel_button;
};				// end RpsQCreateClassDialog


///////////////////////////////////////////////////////////////////////////////
/// The dialog box for creating new symbols.
//////////////////////////////////////////////////////////////////////////////
class RpsQCreateSymbolDialog : public QDialog
{
  Q_OBJECT
public:
  RpsQCreateSymbolDialog(RpsQWindow* parent);
  ~RpsQCreateSymbolDialog();

private slots:
  void on_ok_trigger();
  void on_cancel_trigger();


private:
  QVBoxLayout sydialog_vbox;
  QHBoxLayout syname_hbox;
  QLabel syname_label;
  QLineEdit syname_linedit;
  QCheckBox syname_weakchkbox;
  QHBoxLayout button_hbox;
  QPushButton ok_button;
  QPushButton cancel_button;
};				// end RpsQCreateSymbolDialog


///////////////////////////////////////////////////////////////////////////////
/// The dialog box for creating new named instances.
//////////////////////////////////////////////////////////////////////////////
class RpsQCreateNamedInstanceDialog : public QDialog
{
  Q_OBJECT
public:
  RpsQCreateNamedInstanceDialog(RpsQWindow* parent);
  ~RpsQCreateNamedInstanceDialog();

private slots:
  void on_ok_trigger();
  void on_cancel_trigger();


private:
  QVBoxLayout nidialog_vbox;
  QHBoxLayout niname_hbox;
  QLabel niname_label;
  QLineEdit niname_linedit;
  QHBoxLayout niclass_hbox;
  QLabel niclass_label;
  RpsQObjectLineEdit niclass_linedit;
  QHBoxLayout button_hbox;
  QPushButton ok_button;
  QPushButton cancel_button;
};				// end RpsQCreateNamedInstanceDialog


///////////////////////////////////////////////////////////////////////////////
/// The dialog box for creating new contributors.
//////////////////////////////////////////////////////////////////////////////
class RpsQCreateContributorDialog : public QDialog
{
  Q_OBJECT
public:
  RpsQCreateContributorDialog(RpsQWindow* parent);
  ~RpsQCreateContributorDialog();

private slots:
  void on_ok_trigger();
  void on_cancel_trigger();
};


//////////////////////////////////////////////////////////// RpsQWindow
//// our top window class
class RpsQWindow : public QMainWindow
{
  Q_OBJECT
public:
  RpsQWindow (QWidget *parent = nullptr, int rank=0);
private:
  int win_rank; // rank in application
  // see https://doc.qt.io/qt-5/qtwidgets-mainwindows-menus-example.html
  QMenu* win_app_menu;
  QMenu* win_create_menu;
  QMenu* win_help_menu;

  // for app menu
  QAction* win_apdump_action;
  QAction* win_apgc_action;
  QAction* win_apnewin_action;
  QAction* win_apclose_action;
  QAction* win_apquit_action;
  QAction* win_apexit_action;

  // for create menu
  QAction* win_crclass_action;
  QAction* win_crsymb_action;
  QAction* win_crnamedinstance_action;
  QAction* win_crcontrib_action;

  // for help menu
#warning RpsQWindow help menu missing

  /// the central widget
  QMdiArea* win_centralmdi;

public slots:
  void do_quit(void);
public:
  int window_rank() const
  {
    return win_rank;
  };
};				// end of RpsQWindow

#endif /*QTHEAD_QRPS_INCLUDED*/

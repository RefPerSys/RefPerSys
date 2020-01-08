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

class RpsQApplication;
class RpsQWindow;
class RpsQObjectLineEdit;// a line edit for a RefPerSys object


//////////////////////////////////////////////////////////// RpsQApplication
//// our application class
class RpsQApplication
  : public QApplication
{
  Q_OBJECT;
public:
  RpsQApplication (int &argc, char*argv[]); // constructor
  RpsQWindow* getWindowPtr(int index);
  RpsQWindow& getWindow(int index)
  {
    auto w = getWindowPtr(index);
    if (!w)
      throw std::runtime_error("bad window index in RpsQApplication::getWindow");
    return *w;
  };
  static RpsQApplication* the_app(void){
    return dynamic_cast<RpsQApplication*>(RpsQApplication::instance());
  };

public slots:
  void do_dump_state(QString dirpath=".");
  void do_add_new_window(void);
  void do_dump_then_exit(QString dirpath=".");
  void do_remove_window(int index);

private:

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


//////////////////////////////////////////////////////////// RpsQWindow
//// our top window class
class RpsQWindow : public QMainWindow
{
  Q_OBJECT
public:
  RpsQWindow (QWidget *parent = nullptr);

public slots:
  void do_quit(void);
};				// end of RpsQWindow

#endif /*QTHEAD_QRPS_INCLUDED*/

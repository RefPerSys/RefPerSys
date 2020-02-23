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
#include <QCheckBox>
#include <QCommandLineParser>
#include <QCompleter>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QFile>
#include <QFont>
#include <QLabel>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QStringListModel>
#include <QSettings>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

class RpsQApplication;
class RpsQWindow;
class RpsQObjectLineEdit;// a line edit for a RefPerSys object
class RpsQCommandTextEdit;
class RpsQOutputTextEdit;
class RpsQOutputTextDocument;

//////////////////////////////////////////////////////////// RpsQApplication
//// our application class
class RpsQApplication
  : public QApplication
{
  friend void rps_run_application(int &argc, char **argv);
  friend void rps_garbcoll_application(Rps_GarbageCollector&gc);
  friend bool rps_is_main_gui_thread(void);
  Q_OBJECT;
public:
  RpsQApplication (int &argc, char*argv[]); // constructor
  ~RpsQApplication();
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
  static QSettings* qt_settings(void)
  {
    auto ap = the_app();
    RPS_ASSERT (ap);
    return ap->app_settings;
  };
  void gc_mark(Rps_GarbageCollector&gc) const;

public slots:
  void do_dump_state(QString dirpath=".");
  void do_dump_current_state();
  void do_add_new_window(Rps_CallFrame*callerframe);
  void do_dump_then_exit(QString dirpath=".");
  void do_dump_current_then_exit(void);
  void do_remove_window_by_index(int index);
  void do_remove_window(RpsQWindow* win);

private:

  /// various ways to see the main GUI tread
  static QThread* app_mainqthread;
  static pthread_t app_mainselfthread;
  static std::thread::id app_mainthreadid;
  mutable std::mutex app_mutex;
  std::vector <QPointer<RpsQWindow>> app_windvec;
  size_t app_wndcount;
  QSettings* app_settings;
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
  void on_agree_change(int state);
  void on_fname_edit(const QString& text);
  void on_lname_edit(const QString& text);
  void on_email_edit(const QString& text);
  void on_ok_trigger();
  void on_cancel_trigger();

private:
  QVBoxLayout dialog_vbox;
  QHBoxLayout firstname_hbox;
  QHBoxLayout lastname_hbox;
  QHBoxLayout email_hbox;
  QHBoxLayout webpage_hbox;
  QHBoxLayout button_hbox;
  QLabel firstname_label;
  QLabel lastname_label;
  QLabel email_label;
  QLabel webpage_label;
  QLabel nb_label;
  QLineEdit firstname_edit;
  QLineEdit lastname_edit;
  QLineEdit email_edit;
  QLineEdit webpage_edit;
  QTextBrowser crcont_expltext;
  QCheckBox agree_chk;
  QPushButton ok_button;
  QPushButton cancel_button;
};				// end of RpsQCreateContributorDialog


///////////////////////////////////////////////////////////////////////////////
/// The dialog box for creating then running new temporary C++ plugins.
//////////////////////////////////////////////////////////////////////////////
class RpsQCreatePluginDialog : public QDialog
{
  Q_OBJECT
public:
  RpsQCreatePluginDialog(RpsQWindow* parent);
  ~RpsQCreatePluginDialog();

private slots:
  void on_ok_trigger();
  void on_cancel_trigger();

private:
  QVBoxLayout dialog_vbx;
  QHBoxLayout button_hbx;
  QLabel code_lbl;
  QTextEdit code_txt;
  QPushButton ok_btn;
  QPushButton cancel_btn;
  Rps_Id random_id;
public:
  std::string temporary_cplusplus_file_path()
  {
    return std::string("/tmp/rps") + random_id.to_string() + ".cc";
  }
  std::string temporary_plugin_file_path()
  {
    return std::string("/tmp/rps") + random_id.to_string() + ".so";
  }
  std::string temporary_function_name()
  {
    return std::string("rpstempf") + random_id.to_string();
  };
};				// end of RpsQCreatePluginDialog


///////////////////////////////////////////////////////////////////////////////
/// The dialog box for creating objects for closures; they are
/// collected in `set_of_core_functions` that is in mutable set
/// _6gxiw0snqrX01tZWW9
//////////////////////////////////////////////////////////////////////////////
class RpsQCreateClosureObjectDialog : public QDialog
{
  Q_OBJECT
public:
  RpsQCreateClosureObjectDialog(RpsQWindow* parent);
  ~RpsQCreateClosureObjectDialog ();
  static Rps_ObjectRef the_object_set_of_core_functions()
  {
    return RPS_ROOT_OB(_6gxiw0snqrX01tZWW9);
  };
  static Rps_ObjectRef the_core_function_class()
  {
    return RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS);
  };
  static Rps_ObjectRef the_comment_symbol()
  {
    return RPS_ROOT_OB(_0jdbikGJFq100dgX1n);
  }
private slots:
  void on_creatob_trigger();
  void on_copycod_trigger();
  void on_close_trigger();

private:
  QVBoxLayout crclo_dialog_vbox;
  QHBoxLayout crclo_comment_hbox;
  QLabel crclo_comment_label;
  QLineEdit crclo_comment_linedit;
  QHBoxLayout crclo_srcod_hbox;
  QLabel crclo_srcod_label;
  QTextEdit crclo_srcod_textedit;
  QHBoxLayout crclo_button_hbox;
  QPushButton crclo_creatob_btn;
  QPushButton crclo_copycod_btn;
  QPushButton crclo_close_btn;
};				// end of RpsQCreateClosureObjectDialog


//////////////////////////////////////////////////////////// RpsQCommandTextEdit
class RpsQCommandTextEdit : public QTextEdit
{
  Q_OBJECT;
public:
  RpsQCommandTextEdit(QWidget*parent);
  ~RpsQCommandTextEdit();
private:
  // Conventionally the object reference below is null or else a
  // transient RefPerSys object carrying a payload of
  // Rps_PayloadQt<RpsQCommandTextEdit>, pointing in C++ to this C++
  // object....
  Rps_ObjectRef cmdtxt_objref;
  /// each RefPerSys value could be displayed several times as a
  /// vector of text fragments, or something else so...
  std::map<Rps_Value,std::set<std::vector<QPointer<QObject>>>> cmdtxt_valmap;
public:
  void create_cmdedit_object(Rps_CallFrame*);
  void gc_mark(Rps_GarbageCollector&gc) const
  {
    if (cmdtxt_objref)
      cmdtxt_objref.gc_mark(gc);
    for (auto it : cmdtxt_valmap)
      it.first.gc_mark(gc);
  };
};				// end class RpsQCommandTextEdit


//////////////////////////////////////////////////////////// RpsQOutputTextDocument
class RpsQOutputTextDocument: public QTextDocument
{
  Q_OBJECT;
public:
  RpsQOutputTextDocument(RpsQWindow*parent);
  ~RpsQOutputTextDocument();
  RpsQWindow* out_window() const
  {
    return qobject_cast<RpsQWindow*>(parent());
  };
}; // end class RpsQOutputTextDocument

//////////////////////////////////////////////////////////// RpsQOutputTextEdit
class RpsQOutputTextEdit : public QTextEdit
{
  Q_OBJECT;
public:
  RpsQOutputTextEdit(QWidget*parent);
  ~RpsQOutputTextEdit();
private:
  // Conventionally the object reference below is null or else a
  // transient RefPerSys object carrying a payload of
  // Rps_PayloadQt<RpsQOutputTextEdit>, pointing in C++ to this C++
  // object....
  Rps_Value outptxt_objref;
  /// each RefPerSys value could be displayed several times as a
  /// vector of text fragments, or something else so...
  std::map<Rps_Value,std::set<std::vector<QPointer<QObject>>>> outptxt_valmap;
  static QTextCharFormat outptxt_int_qcfmt_;
  static QTextCharFormat outptxt_double_qcfmt_;
public:
  static QTextCharFormat int_text_format()
  {
    return  outptxt_int_qcfmt_;
  };
  static void initialize(void);
  // create a temporary RefPerSys object whose payload contains this output text edit
  void create_outpedit_object(Rps_CallFrame*);
  void gc_mark(Rps_GarbageCollector&gc) const
  {
    if (outptxt_objref)
      outptxt_objref.gc_mark(gc);
    for (auto it : outptxt_valmap)
      it.first.gc_mark(gc);
  };
};				// end class RpsQOutputTextEdit


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
  QAction* win_crplugin_action;
  QAction* win_crclosob_action;

  // for help menu
#warning RpsQWindow help menu missing


  QMdiArea* win_centralmdi;/// the central widget

  QMdiSubWindow* win_command_subwin; /// the command subwindow
  RpsQCommandTextEdit* win_command_textedit;/// the command text edit

  QMdiSubWindow* win_output_subwin; /// the output subwindow
  RpsQOutputTextEdit* win_output_textedit;/// the output text edit
  RpsQOutputTextDocument* win_output_textdoc; // the common output document

  // Conventionally the object reference below is null or else a
  // transient RefPerSys object carrying a payload of
  // Rps_PayloadQt<RpsQOutputTextEdit>, pointing in C++ to this C++
  // object....
  Rps_ObjectRef win_objref;

public slots:
  void do_quit(void);
public:
  int window_rank() const
  {
    return win_rank;
  };
  void gc_mark(Rps_GarbageCollector&gc) const;
  void create_winobj(Rps_CallFrame*callerframe);
};				// end of RpsQWindow


//// Qt related payload
template <class QtClass>
const std::string
Rps_PayloadQt<QtClass>::payload_type_name(void) const
{
  static std::mutex mtx;
  std::string str;
  std::lock_guard<std::mutex> gu(mtx);
  auto ow = owner();
  if (ow)
    {
      std::lock_guard<std::recursive_mutex> guow(*(ow->objmtxptr()));
      QtClass* qob = nullptr;
      if (ow->get_payload() == this && (qob=qtptr()) != nullptr)
        {
          str = std::string("Rps_PayloadQt:") + qob->metaObject()->className();
          return str;
        }
    }
  const char* cn = QtClass::staticMetaObject.className();
  str = std::string("Rps_PayloadQt/") + cn;
  return str;
} // end Rps_PayloadQt::payload_type_name


template <class QtClass>
Rps_PayloadQt<QtClass>&
Rps_PayloadQt<QtClass>::set_qtptr(QtClass* qptr)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> gu(*(owner()->objmtxptr()));
  _qtptr = qptr;
  return *this;
} // end Rps_PayloadQt<QtClass>::set_qtptr

template <class QtClass>
QtClass*
Rps_PayloadQt<QtClass>::qtptr(void) const
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> gu(*(owner()->objmtxptr()));
  return _qtptr;
} // end Rps_PayloadQt<QtClass>::qtptr

template <class QtClass>
Rps_PayloadQt<QtClass>::~Rps_PayloadQt()
{
  if (owner())
    {
      std::lock_guard<std::recursive_mutex> gu(*(owner()->objmtxptr()));
      if (_qtptr)
        {
          _qtptr->deleteLater();
        }
    }
} // end Rps_PayloadQt<QtClass>::~Rps_PayloadQt

extern template class Rps_PayloadQt<RpsQWindow>;
#endif /*QTHEAD_QRPS_INCLUDED*/

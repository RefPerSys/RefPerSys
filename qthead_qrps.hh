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

class RpsQApplication;
class RpsQWindow;
class RpsQObjectLineEdit;// a line edit for a RefPerSys object

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


///////////////////////////////////////////////////////////////////////////////
/// Singleton to cleanly manage GUI icons and other image assets.
//////////////////////////////////////////////////////////////////////////////
class RpsQPixMap
{
public:

  /// Gets the singleton instance.
  static inline RpsQPixMap* instance()
  {
    if (m_instance == nullptr)
      m_instance = new RpsQPixMap();

    return m_instance;
  }

  /// Registers a new image asset.
  inline void add(std::string id, std::string path)
  {
    m_pixmap[id] = QPixmap(path.c_str());
  }

  /// Gets a registered image asset.
  inline QPixmap& get(std::string id)
  {
    return m_pixmap[id];
  }

private:
  // Private constructor.
  inline RpsQPixMap()
    : m_pixmap()
  { }

  // Private destructor.
  inline ~RpsQPixMap()
  {
    delete m_instance;
  }

  // Singleton instance.
  static RpsQPixMap* m_instance;

  // Map of IDs and image assets.
  //
  // I've chosen an std::string for the image ID in order to avoid possible
  // enum collisions between different image asset classes.
  std::map<std::string, QPixmap> m_pixmap;
};



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

  size_t getWindowCount()
  {
    //return app_windvec.size () - 1; // offset for null element at index 0
    return app_wndcount;
  }

  void lowerWindowCount()
  {
    app_wndcount--;
  }

public slots:
  void dump_state(QString dirpath=".");
  void add_new_window(void);

private:
  void register_pixmap();

  std::mutex app_mutex;
  std::vector <std::unique_ptr<RpsQWindow>> app_windvec;
  size_t app_wndcount;
};				// end of class RpsQApplication


class RpsQWindow;

enum RpsQWindowMenu
{
  APP,
  CREATE,
  HELP,
};


///////////////////////////////////////////////////////////////////////////////
/// Abstract base class for all RpsQWindow menu actions.
///
/// This abstract class helps create a nice polymorphic object hierarchy of
/// menu actions for RpsQWindow instances.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAction : public QObject
{
  Q_OBJECT
public:
  /// Constructor
  RpsQMenuAction(
    RpsQWindow* parent,
    RpsQWindowMenu menu,
    std::string icon,
    std::string title,
    std::string shortcut = ""
  );

  /// Destructor
  inline virtual ~RpsQMenuAction()
  { }

protected:
  /// Accessor to get handle to parent window.
  inline RpsQWindow* window()
  {
    return m_parent;
  }

protected slots:
  /// Pure virtual slot for trigger action of derived menu action classes.
  virtual void on_trigger() = 0;

private:
  // Handle to parent window.
  RpsQWindow* m_parent;
};				// end RpsQMenuAction


///////////////////////////////////////////////////////////////////////////////
/// The Help | About menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuHelpAbout : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuHelpAbout(RpsQWindow* parent)
    : RpsQMenuAction(parent, RpsQWindowMenu::HELP, "RPS_ICON_ABOUT", "&About")
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuHelpAbout


///////////////////////////////////////////////////////////////////////////////
/// The Help | Debug menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuHelpDebug : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuHelpDebug(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::HELP,
        "RPS_ICON_DEBUG",
        "&Debug",
        "CTRL+D"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuHelpDebug




///////////////////////////////////////////////////////////////////////////////
/// The App | Quit menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAppQuit : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuAppQuit(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::APP,
        "RPS_ICON_QUIT",
        "&Quit",
        "CTRL+Q"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuAppQuit


///////////////////////////////////////////////////////////////////////////////
/// The App | Exit menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAppExit : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuAppExit(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::APP,
        "RPS_ICON_EXIT",
        "e&Xit",
        "CTRL+X"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuAppExit


///////////////////////////////////////////////////////////////////////////////
/// The App | Close menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAppClose : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuAppClose(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::APP,
        "RPS_ICON_CLOSE",
        "&Close",
        "CTRL+C"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuAppClose


///////////////////////////////////////////////////////////////////////////////
/// The App | Dump menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAppDump : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuAppDump(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::APP,
        "RPS_ICON_DUMP",
        "&Dump",
        "CTRL+D"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuAppDump


///////////////////////////////////////////////////////////////////////////////
/// The App | GC menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAppGC : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuAppGC(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::APP,
        "RPS_ICON_GC",
        "Collect &Garbage",
        "CTRL+G"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuAppGC


///////////////////////////////////////////////////////////////////////////////
/// The App | New Window menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuAppNew : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuAppNew(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::APP,
        "RPS_ICON_NEW",
        "New &Window",
        "CTRL+W"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuAppNew


///////////////////////////////////////////////////////////////////////////////
/// The Create | Class Window menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuCreateClass : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuCreateClass(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::CREATE,
        "RPS_ICON_CLASS",
        "&Class",
        "CTRL+L"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuCreateClass



///////////////////////////////////////////////////////////////////////////////
/// The Create | Symbol Window menu action for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////
class RpsQMenuCreateSymbol : public RpsQMenuAction
{
  Q_OBJECT
public:
  /// Constructor
  inline RpsQMenuCreateSymbol(RpsQWindow* parent)
    : RpsQMenuAction(
        parent,
        RpsQWindowMenu::CREATE,
        "RPS_ICON_CLASS",
        "&Symbol",
        "CTRL+S"
      )
  { }

protected slots:
  /// Overridden slot for the trigger action.
  void on_trigger();
};				// end RpsQMenuCreateSymbol


///////////////////////////////////////////////////////////////////////////////
/// The menu bar for RpsQWindow.
//////////////////////////////////////////////////////////////////////////////

#warning FIXME: RpsQWindowMenuBar should probably inherit from QMenuBar or be renamed to something else....
/// see https://doc.qt.io/qt-5/qmenubar.html
class RpsQWindowMenuBar
{
public:
  RpsQWindowMenuBar(RpsQWindow* parent);

private:
  RpsQWindow* menubar_parent;
  std::shared_ptr<RpsQMenuAppDump> m_menu_app_dump;
  std::shared_ptr<RpsQMenuAppGC> m_menu_app_gc;
  std::shared_ptr<RpsQMenuAppNew> m_menu_app_new;
  std::shared_ptr<RpsQMenuAppClose> m_menu_app_close;
  std::shared_ptr<RpsQMenuCreateClass> m_menu_create_class;
  std::shared_ptr<RpsQMenuCreateSymbol> m_menu_create_symbol;
  std::shared_ptr<RpsQMenuAppQuit> m_menu_app_quit;
  std::shared_ptr<RpsQMenuAppExit> m_menu_app_exit;
  std::shared_ptr<RpsQMenuHelpAbout> m_menu_help_about;
  std::shared_ptr<RpsQMenuHelpDebug> m_menu_help_debug;
};				// end RpsQWindowMenuBar



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
  virtual ~RpsQWindow()
  { };

  inline RpsQApplication* application()
  {
    return dynamic_cast<RpsQApplication*>(RpsQApplication::instance());
  }

private:
  void setup_debug_widget();
  void setup_debug_timer();

private:
  friend class RpsQMenuHelpDebug;

  RpsQWindowMenuBar m_menu_bar;
  QPlainTextEdit m_debug_widget;
  QTimer m_debug_timer;

signals:

private slots:
  void update_debug_widget();
};				// end of RpsQWindow

#endif /*QTHEAD_QRPS_INCLUDED*/

/****************************************************************
 * file window_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It holds the Qt5 code related to the Qt5 top windows
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


extern "C" const char rps_window_gitid[];
const char rps_window_gitid[]= RPS_GITID;

extern "C" const char rps_window_date[];
const char rps_window_date[]= __DATE__;



RpsQWindow::RpsQWindow (QWidget *parent)
  : QMainWindow(parent),
    // we explicitly initialize every pointer field, for ease of
    // debugging and more reproducible runs.  In principle this should
    // be useless...
    win_app_menu(nullptr),
    win_create_menu(nullptr),
    win_help_menu(nullptr),
    win_apdump_action(nullptr),
    win_apgc_action(nullptr),
    win_apnewin_action(nullptr),
    win_apquit_action(nullptr),
    win_apexit_action(nullptr),
    win_crclass_action(nullptr),
    win_crsymb_action(nullptr),
    win_centralmdi(nullptr)
{
  /// create the menus and their actions
  {
    auto mb = menuBar();
    win_app_menu = mb->addMenu("App");
    win_apdump_action = new QAction("&Dump", this);
    win_apdump_action->setStatusTip("dump state to current directory");
    win_apgc_action = new QAction("&Garbage collect", this);
    win_apgc_action->setStatusTip("run the precise garbage collector");
    win_apnewin_action = new QAction("new &Window", this);
    win_apnewin_action->setStatusTip("Create a new window");
    win_apclose_action = new QAction("&Close", this);
    win_apclose_action->setStatusTip("close the current window");
    win_apquit_action = new QAction("&Quit", this);
    win_apquit_action->setStatusTip("Quit without saving state");
    win_apexit_action = new QAction("e&Xit", this);
    win_apexit_action->setStatusTip("Exit after saving state");
    win_create_menu = mb->addMenu("Create");
    win_crclass_action = new QAction("create &Class",this);
    win_crclass_action->setStatusTip("create a new named class");
    win_crsymb_action = new QAction("create &Symbol", this);
    win_crsymb_action->setStatusTip("create a new symbol");
    win_help_menu = mb->addMenu("Help");
  }
  /// add the actions to their menu
  {
    win_app_menu->addAction(win_apdump_action);
    win_app_menu->addAction(win_apgc_action);
    win_app_menu->addAction(win_apnewin_action);
    win_app_menu->addAction(win_apclose_action);
    win_app_menu->addAction(win_apquit_action);
    win_app_menu->addAction(win_apexit_action);
    win_create_menu->addAction(win_crclass_action);
    win_create_menu->addAction(win_crsymb_action);
  }
  // our central widget
  win_centralmdi =  new QMdiArea(this);
  setCentralWidget(win_centralmdi);
  // connect the behavior
  connect(win_apdump_action, &QAction::triggered,
          RpsQApplication::the_app(),
          &RpsQApplication::do_dump_current_state);
  connect(win_apexit_action, &QAction::triggered,
          RpsQApplication::the_app(), &RpsQApplication::do_dump_current_then_exit);
  connect(win_apquit_action, &QAction::triggered,
          [=](void)
  {
    auto reply =
      QMessageBox::question(this, "RefPerSys",
                            "Quit without dump?",
                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
      QApplication::quit();
  });
  connect(win_apgc_action, &QAction::triggered,
          [=](void)
  {
    rps_garbage_collect();
  });
} // end RpsQWindow::RpsQWindow



////////////////////////////////////////////////////////////////
//// the dialog to create RefPerSys classes

RpsQCreateClassDialog::RpsQCreateClassDialog(RpsQWindow* parent)
  : QDialog(parent),
    dialog_vbox(),
    superclass_hbox(),
    superclass_label("superclass:", this),
    superclass_linedit("", "super", this),
    classname_hbox(),
    classname_label("class name:", this),
    classname_linedit(this),
    button_hbox(),
    ok_button("Create Class", this),
    cancel_button("cancel", this)
{
  // set widget names, useful for debugging, and later for style sheets.
  setObjectName("RpsQCreateClassDialog");
  dialog_vbox.setObjectName("RpsQCreateClassDialog_dialog_vbox");
  superclass_hbox.setObjectName("RpsQCreateClassDialog_dialog_vbox");
  superclass_label.setObjectName("RpsQCreateClassDialog_superclass_label");
  superclass_linedit.setObjectName("RpsQCreateClassDialog_superclass_linedit");
  classname_hbox.setObjectName("RpsQCreateClassDialog_classname_hbox");
  classname_label.setObjectName("RpsQCreateClassDialog_classname_label");
  classname_linedit.setObjectName("RpsQCreateClassDialog_classname_linedit");
  button_hbox.setObjectName("RpsQCreateClassDialog_button_hbox");
  ok_button.setObjectName("RpsQCreateClassDialog_ok_button");
  cancel_button.setObjectName("RpsQCreateClassDialog_cancel_button");
  RPS_INFORMOUT("RpsQCreateClassDialog @" << this);
  // set fonts of labels and linedits
  {
    auto labfont = QFont("Arial", 12);
    superclass_label.setFont(labfont);
    classname_label.setFont(labfont);
    auto editfont = QFont("Courier", 12);
    superclass_linedit.setFont(editfont);
    classname_linedit.setFont(editfont);
  }
  // ensure layout; maybe we should use style sheets?
  {
    dialog_vbox.addLayout(&superclass_hbox);
    superclass_hbox.addWidget(&superclass_label);
    superclass_hbox.addSpacing(2);
    superclass_hbox.addWidget(&superclass_linedit);
    dialog_vbox.addLayout(&classname_hbox);
    classname_hbox.addWidget(&classname_label);
    classname_hbox.addSpacing(2);
    classname_hbox.addWidget(&classname_linedit);
    dialog_vbox.addLayout(&button_hbox);
    button_hbox.addWidget(&ok_button);
    button_hbox.addSpacing(3);
    button_hbox.addWidget(&cancel_button);
    setLayout(&dialog_vbox);
  }
  // define behavior
  {
    connect(
      &ok_button,
      &QAbstractButton::clicked,
      this,
      &RpsQCreateClassDialog::on_ok_trigger
    );
    connect(
      &cancel_button,
      &QAbstractButton::clicked,
      this,
      &RpsQCreateClassDialog::on_cancel_trigger
    );
  }
} // end of RpsQCreateClassDialog::RpsQCreateClassDialog




RpsQCreateClassDialog::~RpsQCreateClassDialog()
{
} // end RpsQCreateClassDialog::~RpsQCreateClassDialog


void RpsQCreateClassDialog::on_ok_trigger()
{
  RPS_LOCALFRAME(Rps_ObjectRef(nullptr),//descriptor
                 nullptr,//parentframe
                 Rps_ObjectRef obsymb;
                 Rps_ObjectRef obnewclass;
                 Rps_ObjectRef obsuperclass;
                );
  // create new class
  std::string strsuperclass = superclass_linedit.text().toStdString();
  std::string strclassname = classname_linedit.text().toStdString();
  RPS_WARNOUT("untested RpsQCreateClassDialog::on_ok_trigger strsuperclass="
              << strsuperclass
              << ", strclassname=" << strclassname);
  try
    {
      _.obsuperclass = Rps_ObjectRef::find_object(&_, strsuperclass);
      RPS_INFORMOUT("RpsQCreateClassDialog::on_ok_trigger obsuperclass=" << _.obsuperclass);
      _.obnewclass = Rps_ObjectRef::make_named_class(&_, _.obsuperclass, strclassname);
      RPS_INFORMOUT("RpsQCreateClassDialog::on_ok_trigger obnewclass=" << _.obnewclass);
      std::ostringstream outs;
      outs << "created new class " << _.obnewclass << " named " << strclassname
           << " of superclass " << _.obsuperclass;
      std::string msg = outs.str();
      QMessageBox::information(parentWidget(), "Created Class", msg.c_str());
    }
  catch (const std::exception& exc)
    {
      RPS_WARNOUT("RpsQCreateClassDialog::on_ok_trigger exception " << exc.what());
      std::ostringstream outs;
      outs << "failed to create class named "
           << strclassname << " with superclass " << strsuperclass
           << std::endl;
      outs<< exc.what();
      QMessageBox::warning(parentWidget(), "Failed class creation", outs.str().c_str());
    }
  deleteLater(); // was close()
}		 // end RpsQCreateClassDialog::on_ok_trigger


void
RpsQCreateClassDialog::on_cancel_trigger()
{
  deleteLater(); // was close()
} // end RpsQCreateClassDialog::on_cancel_trigger



////////////////////////////////////////////////////////////////

/// the dialog to create RefPerSys symbols
RpsQCreateSymbolDialog::RpsQCreateSymbolDialog(RpsQWindow* parent)
  : QDialog(parent),
    sydialog_vbox(),
    syname_hbox(),
    syname_label("new symbol name:", this),
    syname_linedit(this),
    syname_weakchkbox("weak?", this),
    button_hbox(),
    ok_button("Create Symbol", this),
    cancel_button("cancel", this)
{
  // set widget names, useful for debugging, and later for style sheets.
  setObjectName("RpsQCreateSymbolDialog");
  sydialog_vbox.setObjectName("RpsQCreateSymbolDialog_sydialog_vbox");
  syname_hbox.setObjectName("RpsQCreateSymbolDialog_syname_hbox");
  syname_label.setObjectName("RpsQCreateSymbolDialog_syname_hbox");
  syname_linedit.setObjectName("RpsQCreateSymbolDialog_syname_linedit");
  syname_weakchkbox.setObjectName("RpsQCreateSymbolDialog_syname_weakchkbox");
  button_hbox.setObjectName("RpsQCreateSymbolDialog_button_hbox");
  ok_button.setObjectName("RpsQCreateSymbolDialog_ok_button");
  cancel_button.setObjectName("RpsQCreateSymbolDialog_cancel_button");
  RPS_INFORMOUT("RpsQCreateSymbolDialog @" << this);
  // set fonts of labels and linedits
  {
    auto labfont = QFont("Arial", 12);
    syname_label.setFont(labfont);
    auto editfont = QFont("Courier", 12);
    syname_linedit.setFont(editfont);
  }
  // ensure layout; maybe we should use style sheets?
  {
    sydialog_vbox.addLayout(&syname_hbox);
    syname_hbox.addWidget(&syname_label);
    syname_hbox.addSpacing(2);
    syname_hbox.addWidget(&syname_linedit);
    syname_hbox.addSpacing(2);
    syname_hbox.addWidget(&syname_weakchkbox);
    sydialog_vbox.addLayout(&button_hbox);
    button_hbox.addWidget(&ok_button);
    button_hbox.addSpacing(3);
    button_hbox.addWidget(&cancel_button);
    setLayout(&sydialog_vbox);
  }
  // define behavior
  {
    connect(
      &ok_button,
      &QAbstractButton::clicked,
      this,
      &RpsQCreateSymbolDialog::on_ok_trigger
    );
    connect(
      &cancel_button,
      &QAbstractButton::clicked,
      this,
      &RpsQCreateSymbolDialog::on_cancel_trigger
    );
  }
} // end RpsQCreateSymbolDialog::RpsQCreateSymbolDialog


RpsQCreateSymbolDialog::~RpsQCreateSymbolDialog()
{
} // end RpsQCreateSymbolDialog::~RpsQCreateSymbolDialog

void
RpsQCreateSymbolDialog::on_ok_trigger()
{
  RPS_LOCALFRAME(Rps_ObjectRef(nullptr),//descriptor
                 nullptr,//parentframe
                 Rps_ObjectRef obsymb;
                );
  std::string strsyname = syname_linedit.text().toStdString();
  RPS_WARNOUT("RpsQCreateSymbolDialog::on_ok_trigger strsyname=" << strsyname);
  try
    {
      bool isweak = syname_weakchkbox.isChecked();
      _.obsymb = Rps_ObjectRef::make_new_symbol(&_, strsyname, isweak);
      RPS_INFORMOUT("RpsQCreateSymbolDialog::on_ok_trigger created symbol " << _.obsymb
                    << " named " << strsyname);
      if (!_.obsymb)
        throw std::runtime_error(std::string("failed to create symbol:") + strsyname);
      if (!isweak)
        rps_add_root_object(_.obsymb);
      _.obsymb->put_space(Rps_ObjectRef::root_space());
      std::ostringstream outs;
      outs << "created new symbol " << _.obsymb << " named " << strsyname;
      std::string msg = outs.str();
      QMessageBox::information(parentWidget(), "Created Symbol", msg.c_str());
    }
  catch (const std::exception& exc)
    {
      RPS_WARNOUT("RpsQCreateSymbolDialog::on_ok_trigger exception " << exc.what());
      std::ostringstream outs;
      outs << "failed to create symbol named "
           << strsyname
           << std::endl;
      outs<< exc.what();
      QMessageBox::warning(parentWidget(), "Failed symbol creation", outs.str().c_str());
    }
  deleteLater();
} // end RpsQCreateSymbolDialog::on_ok_trigger

void
RpsQCreateSymbolDialog::on_cancel_trigger()
{
  deleteLater(); // was close()
} // end RpsQCreateSymbolDialog::on_cancel_trigger

////////////////////////////////////////////////////////////////

///// the completer for RefPerSys objects
RpsQObjectCompleter::RpsQObjectCompleter(QObject*parent)
  : QCompleter(parent),
    qobcompl_strlistmodel(this)
{
  setModel(&qobcompl_strlistmodel);
#warning incomplete RpsQObjectCompleter::RpsQObjectCompleter
  RPS_WARN("incomplete RpsQObjectCompleter::RpsQObjectCompleter");
} // end RpsQObjectCompleter::RpsQObjectCompleter

void
RpsQObjectCompleter::update_for_text(const QString&qstr)
{
  std::string str = qstr.toStdString();
  qobcompl_strlistmodel.setStringList(QStringList{});
  if (str.size() <= 2)
    {
      return;
    }
  else if (str[0] == '_' && isdigit(str[1]))
    {
      QStringList qslist;
      auto stopfun = [=,&qslist](const Rps_ObjectZone*obz)
      {
        RPS_ASSERT(obz);
        if (qslist.count() > max_nb_autocompletions+2)
          return true;
        Rps_Id obid = obz->oid();
        char idbuf[32];
        memset (idbuf, 0, sizeof(idbuf));
        obid.to_cbuf24(idbuf);
        QString qids(idbuf);
        qslist << qids;
        return false;
      };
      int nbcompl =
        Rps_ObjectZone::autocomplete_oid(str.c_str(), stopfun);
      if (nbcompl <= max_nb_autocompletions)
        qobcompl_strlistmodel.setStringList(qslist);
    }
  else if (isalpha(str[0]))
    {
      QStringList qslist;
      auto stopfun = [=,&qslist](const Rps_ObjectZone*obz, const std::string&nam)
      {
        RPS_ASSERT(obz);
        if (qslist.count() > max_nb_autocompletions+2)
          return true;
        QString qnams(nam.c_str());
        qslist << qnams;
        return false;
      };
      int nbcompl = Rps_PayloadSymbol::autocomplete_name(str.c_str(), stopfun);

      if (nbcompl <= max_nb_autocompletions)
        qobcompl_strlistmodel.setStringList(qslist);
    }
} // end RpsQObjectCompleter::update_for_text


///// the line edit for RefPerSys objects
RpsQObjectLineEdit::RpsQObjectLineEdit(const QString &contents,
                                       const QString& placeholder, QWidget *parent)
  : QLineEdit(contents, parent),
    qoblinedit_completer(nullptr)
{
  setPlaceholderText(placeholder);
  qoblinedit_completer = std::make_unique<RpsQObjectCompleter>(this);
  connect(this, SIGNAL(textEdited(const QString&)),
          qoblinedit_completer.get(), SLOT(update_for_text(const QString&)));
#warning incomplete RpsQObjectLineEdit::RpsQObjectLineEdit
  RPS_WARN("incomplete RpsQObjectLineEdit::RpsQObjectLineEdit");
} // end of RpsQObjectLineEdit::RpsQObjectLineEdit

//////////////////////////////////////// end of file window_qrps.cc


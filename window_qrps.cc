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
#include <iostream>
#include <fstream>
#include <QProcess>


extern "C" const char rps_window_gitid[];
const char rps_window_gitid[]= RPS_GITID;

extern "C" const char rps_window_date[];
const char rps_window_date[]= __DATE__;



RpsQWindow::RpsQWindow (QWidget *parent, int rank)
  : QMainWindow(parent),
    win_rank(rank),
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
    win_crnamedinstance_action(nullptr),
    win_crcontrib_action(nullptr),
    win_centralmdi(nullptr),
    win_command_subwin(nullptr),
    win_command_textedit(nullptr),
    win_output_subwin(nullptr),
    win_output_textedit(nullptr),
    win_objref(nullptr)
#warning win_objref should be known to the garbage collector
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
    win_crnamedinstance_action = new QAction("create &Named instance", this);
    win_crnamedinstance_action->setStatusTip("create a new named instance and its symbol");
    win_crcontrib_action = new QAction("create &Contributor", this);
    win_crcontrib_action->setStatusTip("create a new contributor");
    win_crplugin_action = new QAction("create &Plugin", this);
    win_crplugin_action->setStatusTip("Create a new C++ plugin");
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
    win_create_menu->addAction(win_crnamedinstance_action);
    win_create_menu->addAction(win_crcontrib_action);
    win_create_menu->addAction(win_crplugin_action);
  }
  // our central widget and related subwindows and subwidgets
  win_centralmdi =  new QMdiArea(this);
  setCentralWidget(win_centralmdi);
  win_centralmdi->tileSubWindows();
  win_command_subwin = new QMdiSubWindow(this);
  win_command_textedit = new RpsQCommandTextEdit(this);
  win_command_subwin->setWindowTitle("command");
  win_command_subwin->setWidget(win_command_textedit);
  win_centralmdi->addSubWindow(win_command_subwin);
  win_output_subwin = new QMdiSubWindow(this);
  win_output_textedit = new RpsQOutputTextEdit(this);
  win_output_subwin->setWindowTitle("output");
  win_output_subwin->setWidget(win_output_textedit);
  win_centralmdi->addSubWindow(win_output_subwin);

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
  connect(win_apnewin_action, &QAction::triggered,
          RpsQApplication::the_app(), &RpsQApplication::do_add_new_window);
  connect(win_crclass_action, &QAction::triggered,
          [=](void)
  {
    auto dia = new RpsQCreateClassDialog(this);
    dia->show();
  });
  connect(win_crsymb_action, &QAction::triggered,
          [=](void)
  {
    auto dia = new RpsQCreateSymbolDialog(this);
    dia->show();
  });

  connect(win_crnamedinstance_action, &QAction::triggered,
          [=](void)
  {
    auto dia = new RpsQCreateNamedInstanceDialog(this);
    dia->show();
  });

  connect(win_crcontrib_action, &QAction::triggered, [=](void)
  {
    auto dia = new RpsQCreateContributorDialog(this);
    dia->show();
  });

  connect(win_crplugin_action, &QAction::triggered, [=](void)
  {
    auto dia = new RpsQCreatePluginDialog(this);
    dia->show();
  });

  connect(win_apclose_action, &QAction::triggered, [=](void)
  {
    RpsQApplication::the_app()->do_remove_window_by_index(window_rank());
  });
#warning TODO: closing or deletion of RpsQWindow should remove it in application app_windvec....
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
  syname_label.setObjectName("RpsQCreateSymbolDialog_syname_label");
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

/// the dialog to create RefPerSys named instances
RpsQCreateNamedInstanceDialog::RpsQCreateNamedInstanceDialog(RpsQWindow* parent)
  : QDialog(parent),
    nidialog_vbox(),
    niname_hbox(),
    niname_label("new instance name:", this),
    niname_linedit(this),
    niclass_hbox(),
    niclass_label("class:", this),
    niclass_linedit("", "class of new instance", this),
    button_hbox(),
    ok_button("Create Named Instance", this),
    cancel_button("cancel", this)
{
  // set widget names, useful for debugging, and later for style sheets.
  setObjectName("RpsQCreateNamedInstanceDialog");
  nidialog_vbox.setObjectName("RpsQCreateNamedInstanceDialog_nidialog_vbox");
  niname_hbox.setObjectName("RpsQCreateNamedInstanceDialog_niname_hbox");
  niname_label.setObjectName("RpsQCreateNamedInstanceDialog_niname_label");
  niname_linedit.setObjectName("RpsQCreateNamedInstanceDialog_niname_linedit");
  niclass_label.setObjectName("RpsQCreateNamedInstanceDialog_niclass_label");
  button_hbox.setObjectName("RpsQCreateNamedInstanceDialog_button_hbox");
  ok_button.setObjectName("RpsQCreateNamedInstanceDialog_ok_button");
  cancel_button.setObjectName("RpsQCreateNamedInstanceDialog_cancel_button");
  RPS_INFORMOUT("RpsQCreateNamedInstanceDialog @" << this);
  // set fonts of labels and linedits
  {
    auto labfont = QFont("Arial", 12);
    niname_label.setFont(labfont);
    auto editfont = QFont("Courier", 12);
    niname_linedit.setFont(editfont);
  }
  // ensure layout; maybe we should use style sheets?
  {
    nidialog_vbox.addLayout(&niname_hbox);
    niname_hbox.addWidget(&niname_label);
    niname_hbox.addSpacing(2);
    niname_hbox.addWidget(&niname_linedit);
    nidialog_vbox.addLayout(&niclass_hbox);
    niclass_hbox.addSpacing(2);
    niclass_hbox.addWidget(&niclass_label);
    niclass_hbox.addWidget(&niclass_linedit);
    nidialog_vbox.addLayout(&button_hbox);
    button_hbox.addWidget(&ok_button);
    button_hbox.addSpacing(3);
    button_hbox.addWidget(&cancel_button);
    setLayout(&nidialog_vbox);
  }
  // define behavior
  {
    connect(
      &ok_button,
      &QAbstractButton::clicked,
      this,
      &RpsQCreateNamedInstanceDialog::on_ok_trigger
    );
    connect(
      &cancel_button,
      &QAbstractButton::clicked,
      this,
      &RpsQCreateNamedInstanceDialog::on_cancel_trigger
    );
  }
} // end RpsQCreateNamedInstanceDialog::RpsQCreateNamedInstanceDialog


RpsQCreateNamedInstanceDialog::~RpsQCreateNamedInstanceDialog()
{
} // end RpsQCreateNamedInstanceDialog::~RpsQCreateNamedInstanceDialog

void
RpsQCreateNamedInstanceDialog::on_ok_trigger()
{
  RPS_LOCALFRAME(Rps_ObjectRef(nullptr),//descriptor
                 nullptr,//parentframe
                 Rps_ObjectRef obsymb;
                 Rps_ObjectRef obclass;
                 Rps_ObjectRef obnewinst;
                );
  std::string strnisyname = niname_linedit.text().toStdString();
  std::string strniclaname = niclass_linedit.text().toStdString();
  RPS_WARNOUT("RpsQCreateNamedInstanceDialog::on_ok_trigger strnisyname=" << strnisyname
              << " strniclaname=" << strniclaname);
  try
    {
      _.obclass = Rps_ObjectRef::find_object(&_, strniclaname);
      RPS_INFORMOUT("RpsQCreateNamedInstanceDialog obclass=" << _.obclass);
      if (!_.obclass)
        throw RPS_RUNTIME_ERROR_OUT("create named instance: no class named " << strniclaname);
      if (!_.obclass->is_class())
        throw RPS_RUNTIME_ERROR_OUT("create named instance: invalid given class named " << strniclaname);
      if (_.obclass == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) // `int`
          || _.obclass == RPS_ROOT_OB(_36I1BY2NetN03WjrOv) // `symbol`
          || _.obclass == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) // `class`
          || _.obclass == RPS_ROOT_OB(_4jISxMJ4PYU0050nUl) // `closure`
          || _.obclass == RPS_ROOT_OB(_5CYWxcChKN002rw1fI) // `contributor_to_RefPerSys`
          || _.obclass == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) // `string`
          || _.obclass == RPS_ROOT_OB(_6JYterg6iAu00cV9Ye) // `set`
          || _.obclass == RPS_ROOT_OB(_6NVM7sMcITg01ug5TC) // `tuple`
          || _.obclass == RPS_ROOT_OB(_6XLY6QfcDre02922jz) // `value`
          || _.obclass == RPS_ROOT_OB(_98sc8kSOXV003i86w5) // `double`
         )
        throw RPS_RUNTIME_ERROR_OUT("create named instance: forbidden class named " << strniclaname);
      _.obsymb = Rps_ObjectRef::make_new_strong_symbol(&_, strnisyname);
      RPS_INFORMOUT("RpsQCreateNamedInstanceDialog::on_ok_trigger created symbol " << _.obsymb
                    << " named " << strnisyname
                    << " with obclass=" << _.obclass);
      if (!_.obsymb)
        throw std::runtime_error(std::string("failed to create symbol:") + strnisyname);
      rps_add_root_object(_.obsymb);
      _.obsymb->put_space(Rps_ObjectRef::root_space());
      _.obnewinst = Rps_ObjectRef::make_object(&_, _.obclass, Rps_ObjectRef::root_space());
      if (_.obclass == RPS_ROOT_OB(_0J1C39JoZiv03qA2HA)) // ̀ mutable_set` class
        _.obnewinst ->put_new_plain_payload<Rps_PayloadSetOb>();
      else if (_.obclass == RPS_ROOT_OB(_0J1C39JoZiv03qA2HA)) // ̀ mutable_vector` class
        _.obnewinst ->put_new_plain_payload<Rps_PayloadVectOb>();
      rps_add_root_object(_.obnewinst);
#warning should put payload for a few special cases of RpsQCreateNamedInstanceDialog
      auto sypayl = _.obsymb->get_dynamic_payload<Rps_PayloadSymbol>();
      RPS_ASSERT(sypayl != nullptr);
      sypayl->symbol_put_value(Rps_ObjectValue(_.obnewinst));
      RPS_INFORMOUT("RpsQCreateNamedInstanceDialog created new named instance "
                    << strnisyname << " as " << _.obnewinst
                    << " of class " << strniclaname);
      std::ostringstream outs;
      outs << "created new named instance " << strnisyname << " as " <<  _.obnewinst
           << " of class " << strniclaname << " " << _.obnewinst->get_class();
      std::string msg = outs.str();
      QMessageBox::information(parentWidget(), "Created Named Instance", msg.c_str());
    }
  catch (const std::exception& exc)
    {
      RPS_WARNOUT("RpsQCreateNamedInstanceDialog::on_ok_trigger exception " << exc.what());
      std::ostringstream outs;
      outs << "failed to create symbol named "
           << strnisyname
           << std::endl;
      outs << exc.what();
      QMessageBox::warning(parentWidget(), "Failed symbol creation", outs.str().c_str());
    }
  deleteLater();
} // end RpsQCreateNamedInstanceDialog::on_ok_trigger

void
RpsQCreateNamedInstanceDialog::on_cancel_trigger()
{
  deleteLater(); // was close()
} // end RpsQCreateNamedInstanceDialog::on_cancel_trigger


RpsQCreateContributorDialog::RpsQCreateContributorDialog(RpsQWindow* parent)
  : QDialog(parent),
    dialog_vbox(),
    firstname_hbox(),
    lastname_hbox(),
    email_hbox(),
    webpage_hbox(),
    button_hbox(),
    firstname_label("Contributor First Name*:", this),
    lastname_label("Contributor Last Name*:", this),
    email_label("Contributor Email*:", this),
    webpage_label("Contributor Web page:", this),
    nb_label("* indicates a required field", this),
    firstname_edit(this),
    lastname_edit(this),
    email_edit(this),
    webpage_edit(this),
    crcont_expltext(this),
    agree_chk("I have read the above notice and agree to its terms.", this),
    ok_button("Create Contributor", this),
    cancel_button("Cancel", this)
{
  // set widget names for debugging and future style sheets
  {
    dialog_vbox.setObjectName("RpsQCreateContributorDialog_dialog_vbox");
    firstname_hbox.setObjectName("RpsQCreateContributorDialog_firstname_hbox");
    lastname_hbox.setObjectName("RpsQCreateContributorDialog_lastname_hbox");
    email_hbox.setObjectName("RpsQCreateContributorDialog_email_hbox");
    webpage_hbox.setObjectName("RpsQCreateContributorDialog_email_webpage");
    button_hbox.setObjectName("RpsQCreateContributorDialog_button_hbox");
    firstname_label.setObjectName("RpsQCreateContributorDialog_firstname_label");
    lastname_label.setObjectName("RpsQCreateContributorDialog_lastname_label");
    email_label.setObjectName("RpsQCreateContributorDialog_email_label");
    webpage_label.setObjectName("RpsQCreateContributorDialog_webpage_label");
    nb_label.setObjectName("RpsQCreateContributorDialog_nb_label");
    firstname_edit.setObjectName("RpsQCreateContributorDialog_firstname_edit");
    lastname_edit.setObjectName("RpsQCreateContributorDialog_lastname_edit");
    email_edit.setObjectName("RpsQCreateContributorDialog_email_edit");
    webpage_edit.setObjectName("RpsQCreateContributorDialog_webpage_edit");
    crcont_expltext.setObjectName("RpsQCreateContributorDialog_crcont_expltext");
    agree_chk.setObjectName("RpsQCreateContributorDialog_agree_chk");
    ok_button.setObjectName("RpsQCreateContributorDialog_ok_button");
    cancel_button.setObjectName("RpsQCreateContributorDialog_cancel_button");
  }

  {
    auto screengeom = RpsQApplication::the_app()->desktop()->screenGeometry();
    int w = 700;
    int h = 500;
    if (w > (3*screengeom.width())/4)
      w = 3*screengeom.width()/4;
    else if (w < (screengeom.width())/2)
      w = 16 + (screengeom.width())/2;
    if (h >  (3*screengeom.height())/4)
      h = 3*screengeom.height()/4;
    else if (h < screengeom.height()/2)
      h = 16 + screengeom.height()/2;
    this->resize(w, h);
  }

  // set widget fonts
  {
    auto arial = QFont("Arial", 12);
    firstname_label.setFont(arial);
    lastname_label.setFont(arial);
    email_label.setFont(arial);
    webpage_label.setFont(arial);
    agree_chk.setFont(arial);
    ok_button.setFont(arial);
    cancel_button.setFont(arial);

    auto courier = QFont("Courier", 12);
    firstname_edit.setFont(courier);
    lastname_edit.setFont(courier);
    webpage_edit.setFont(courier);
    email_edit.setFont(courier);

    nb_label.setFont(QFont("Arial", 8));
  }

  // set widget tooltips
  {
    firstname_label.setToolTip("Your first or personal name is required");
    lastname_label.setToolTip("Your last or family name is required");
    email_label.setToolTip("Your e-mail address is required");
    webpage_label.setToolTip("The URL of your web page is optional");
    agree_chk.setToolTip("You must agree to the notice terms to continue");
  }

  /// fill HTML code of explanatory text with a raw C++ string literal
  crcont_expltext.setHtml(R"explaintext(
<!doctype html>
<html>
<head>
<title>RefPerSys personal data addition</title>
</head>

<body> 

<h1><a href='http://refpersys.org/'>RefPerSys</a> personal data
addition</h1> 

<h2>Please read this carefully</h2>

<p>This explains important things about your personal data in <a
href='http://refpersys.org/'>RefPerSys</a>, so <b>read all this
page</b> before clicking on the <i>Create Contributor</i> button
below. Following hyperlinks here would open your Web browser.</p>

<h2>General Points</h2>

<p>The <a href='http://refpersys.org/'>RefPerSys</a> system could use
some personal data related to yourself. The rationale for having such
limited personal data is given both in the <a
href='http://starynkevitch.net/Basile/refpersys-design.pdf'><tt>refpersys-design.pdf</tt>
draft</a>  document, and in the <a
href='https://gitlab.com/bstarynk/refpersys/-/wikis/Personal-data-in-RefPerSys'>Personal
data in RefPerSys</a> wikipage. You should have read and understood both of
these.</p>

<p>In Europe or for European citizens, the <a
href='https://gdpr-info.eu/'>General Data Protection Regulation</a>
<i>could</i> apply to such personal data. The human contributors of <a
href='http://refpersys.org/'>RefPerSys</a> are not lawyers, so they
don&apos;t really know the legal implications (perhaps the <i>GDPR</i>
is not even relevant here!), but are acting in good faith.</p>

<p><b>By completing this form</b> and clicking its <i>Create
Contributor</i> button, <b>you <i>explicitly acknowledge</i> that the
<i>Create Contributor</i> button below is adding some personal data
related to yourself into the <i>persistent heap</i> of <a
href='http://refpersys.org/'>RefPerSys</a></b>. You also acknowledge
understanding that removing your personal data from that persistent
heap is in practice not easily possible. You may later propagate this
modified persistent heap to <a
href='https://gitlab.com/bstarynk/refpersys'>our <tt>gitlab</tt>
repository</a>, and associated personal information, by running on
your free will the <tt>git commit -s</tt> command, followed by <tt>git
push</tt>. See <a href='http://git-scm.com/'><tt>git</tt> website</a>
for more information about <tt>git</tt>, and <a
href='https://gitlab.com/'>the <tt>gitlab</tt> website</a> for more
about <tt>gitlab</tt> and our <a
href='https://gitlab.com/bstarynk/refpersys'><i>RefPerSys</i>
repository</a> there.<p>

<p>Please be kind to <b>fill this form inputs with <i>sincere</i>
data</b>. You might use some pseudonym, if you wanted to, but the <a
href='http://refpersys.org/#team_id'><i>RefPerSys</i> development
team</a> prefers that you register, <b>on your free will</b>, your
actual first and last (family) names, a working email, and optionally
the real URL of your home page if you have one. No check is done on
the data you have input, so please take time to <b>verify your data
and fill it without mistakes</b>. Please check (e.g. by using <a
href='http://man7.org/linux/man-pages/man1/grep.1.html</a><tt>grep</tt></a>
on some <tt>persistore/*.json</tt> textual files containing persistent
data, then your favorite <a
href='https://en.wikipedia.org/wiki/Text_editor'>text editor</a> on
them) that your persisted data is correct before your following
<tt>git commit -s</tt> command.</p>

<h2>How your personal data is processed:</h2>

<p>Once you click on the <i>Create Contributor</i> button below, the
following steps should happen: 

<ul> 

<li>a permanent object <i>ObContrib</i>, reifying you, of
<i>RefPerSys</i> class <tt>contributor_to_RefPerSys</tt> (of oid
<i><tt>_5CYWxcChKN002rw1fI</tt></i>) is created.</li>

<li>the <i>RefPerSys</i> object <i>ObContrib</i> is filled with
attributes: <tt>first_name</tt> (of oid
<i><tt>_3N8vZ2Cw62z024XxCg</tt></i>), <tt>last_name</tt> (of oid
<i><tt>_6QAanFi9yLx00spBST</tt></i>), <tt>email</tt> (of oid
<i><tt>_0D6zqQNe4eC02bjfGs</tt></i>), optional <tt>home_page</tt> (of
oid <i><tt>_0LbCts6NacB03SMXz4</tt></i>) associated to strings.</li>

<li>that the object <i>ObContrib</i> is added as an element to the
<i><tt>_1wihX3eWD9o00QnxUX</tt></i> mutable set root object (that
object is the value of RefPerSys symbol <tt>our_contributors</tt>),
which collects all known contributors reifications.</li>

</ul> 

</p>

<h2>Fictitious Example:</h2

<p>A fictitious and inspirational example of personal data could be:
<ul>

<li><i>first name:</i> <tt style='background-color:pink'>John</tt></li>

<li><i>last name:</i> <tt style='background-color:pink'>Doe</tt></li>

<li><i>email:</i> <tt style='background-color:pink'>john.doe@fake.email</tt></li>

<li><i>web page:</i> <tt
style='background-color:pink'>http://example.net/john-doe.html</tt></li>

</ul><br/>

<b>Please adapt that example</b> to your personal data, and <b>check
your personal carefully</b> before clicking the <i>Create
Contributor</i> button, since changing your data is currently uneasy
(e.g. could require manual edition of some <tt>persistore/*.json</tt>
file). Clicking that <i>Create Contributor</i> button is giving
permission to update the persistent heap with your personal data.</p>

<p>Adding yourself as a contributor could require a working connection to
the Internet, to check your email and home page.</p>

<hr/>

<p>Thanks for your cooperation. The <a
href='mailto:team@refpersys.org'>RefPerSys team</a>.<p>

</body>

</html>
)explaintext");

  // set widget layouts
  {
    dialog_vbox.addLayout(&firstname_hbox);
    firstname_hbox.addWidget(&firstname_label);
    firstname_hbox.addSpacing(2);
    firstname_hbox.addWidget(&firstname_edit);

    dialog_vbox.addLayout(&lastname_hbox);
    lastname_hbox.addWidget(&lastname_label);
    lastname_hbox.addSpacing(2);
    lastname_hbox.addWidget(&lastname_edit);

    dialog_vbox.addLayout(&email_hbox);
    email_hbox.addWidget(&email_label);
    email_hbox.addSpacing(2);
    email_hbox.addWidget(&email_edit);
    
    dialog_vbox.addLayout(&webpage_hbox);
    webpage_hbox.addWidget(&webpage_label);
    webpage_hbox.addSpacing(2);
    webpage_hbox.addWidget(&webpage_edit);

    dialog_vbox.addWidget(&nb_label);
    dialog_vbox.addWidget(&crcont_expltext);
    dialog_vbox.addWidget(&agree_chk);

    dialog_vbox.addLayout(&button_hbox);
    button_hbox.addWidget(&ok_button);
    button_hbox.addSpacing(3);
    button_hbox.addWidget(&cancel_button);

    setLayout(&dialog_vbox);
  }

  // set explanatory placeholder text
  firstname_edit.setPlaceholderText("e.g. John");
  lastname_edit.setPlaceholderText("e.g. Doe");
  email_edit.setPlaceholderText("e.g. john.doe@fake.email");
  webpage_edit.setPlaceholderText("e.g. http://www.example.net/john-doe.html");

  // other settings
  {
    crcont_expltext.setReadOnly(true);
    crcont_expltext.setOpenExternalLinks(true);
    ok_button.setEnabled(false);
  }

  // connect widget slots
  {
    connect(
      &ok_button, &QAbstractButton::clicked, this,
      &RpsQCreateContributorDialog::on_ok_trigger
    );

    connect(
      &cancel_button, &QAbstractButton::clicked, this,
      &RpsQCreateContributorDialog::on_cancel_trigger
    );

    connect(
      &agree_chk, &QCheckBox::stateChanged, this,
      &RpsQCreateContributorDialog::on_agree_change
    );

    connect(
      &firstname_edit, &QLineEdit::textEdited, this,
      &RpsQCreateContributorDialog::on_fname_edit
    );

    connect(
      &lastname_edit, &QLineEdit::textEdited, this,
      &RpsQCreateContributorDialog::on_lname_edit
    );

    connect(
      &email_edit, &QLineEdit::textEdited, this,
      &RpsQCreateContributorDialog::on_email_edit
    );
  }
}  // end RpsQCreateContributorDialog::RpsQCreateContributorDialog()


RpsQCreateContributorDialog::~RpsQCreateContributorDialog()
{
}  // end RpsQCreateContributorDialog::~RpsQCreateContributorDialog()







void
RpsQCreateContributorDialog::on_ok_trigger()
{
  RPS_LOCALFRAME(Rps_ObjectRef(nullptr),//descriptor
                 nullptr,//parentframe
                 Rps_ObjectRef obcontrib; // the new contributor object
                 Rps_ObjectRef obsetcontributors; // the existing set of contributors
                 Rps_Value firstnamev; // first name, as a string
		 Rps_Value lastnamev; // last name, as a string
		 Rps_Value emailv; // email, as a string
		 Rps_Value webpagev; // home web page URL, as a string
		 Rps_Value gitidv; // the git id prefix, for tracability
		 );
  QString firstnameqs = firstname_edit.text();
  QString lastnameqs = lastname_edit.text();
  QString emailqs = email_edit.text();
  QString webpageqs = webpage_edit.text();

  std::string firstnamestr = firstnameqs.toStdString();
  std::string lastnamestr = lastnameqs.toStdString();
  std::string emailstr = emailqs.toStdString();
  std::string webpagestr = webpageqs.toStdString();

  RPS_INFORMOUT("RpsQCreateContributorDialog incomplete:"
		<< " firstnamestr=" << firstnamestr
		<< ", lastnamestr=" << lastnamestr
		<< ", emailstr=" << emailstr
		<< ", webpagestr=" << webpagestr
		);

  try {
    // validate the first name:
    {
      int firstix=0;
      int firstsiz = firstnameqs.size();
      if (firstsiz == 0)
	throw RPS_RUNTIME_ERROR_OUT("empty first name");
      for (QChar cfirst: firstnameqs) {
	if (firstix==0 && !cfirst.isLetter())
	  throw RPS_RUNTIME_ERROR_OUT("first character Unicode#" << cfirst.unicode()
				      << " in first name " << firstnamestr << " should be a letter");
	if (!cfirst.isLetter() && cfirst != '-' && cfirst != ' ')
	  throw RPS_RUNTIME_ERROR_OUT("invalid character Unicode#" << cfirst.unicode() << " in first name " << firstnamestr);
	if (!cfirst.isLetter() && firstix>0 && !firstnameqs[firstix-1].isLetter())
	  throw RPS_RUNTIME_ERROR_OUT("a non-letter in index " << firstix
				      << " of first name " << firstnamestr
				      << " cannot be preceded by a non-letter");
	if (firstix == firstsiz-1 && !(cfirst.isLetter() || cfirst == '.' || cfirst == '\''))
	  throw RPS_RUNTIME_ERROR_OUT("last character Unicode#" << cfirst.unicode()
				      << " in first name " << firstnamestr
				      << " should be a letter, or a dot or a quote");
	firstix++;
      };
    } // end of first name validation

    // validate the last name:
    {
      int lastix=0;
      int lastsiz = lastnameqs.size();
      if (lastsiz == 0)
	throw RPS_RUNTIME_ERROR_OUT("empty last name");
      for (QChar clast: lastnameqs) {
	if (lastix==0 && !clast.isLetter())
	  throw RPS_RUNTIME_ERROR_OUT("first character Unicode#" << clast.unicode()
				      << " in last name " << lastnamestr << " should be a letter");
	if (!clast.isLetter() && clast != '-' && clast != ' ')
	  throw RPS_RUNTIME_ERROR_OUT("invalid character Unicode#" << clast.unicode()
				      << " in last name " << lastnamestr);
	if (!clast.isLetter() && lastix>0 && !lastnameqs[lastix-1].isLetter())
	  throw RPS_RUNTIME_ERROR_OUT("a non-letter in index " << lastix
				      << " of last name " << lastnamestr
				      << " cannot be preceded by a non-letter");
	if (lastix == lastsiz-1 && !(clast.isLetter() || clast == '.' || clast == '\''))
	  throw RPS_RUNTIME_ERROR_OUT("last character Unicode#" << clast.unicode()
				      << " in last name " << lastnamestr
				      << " should be a letter, or a dot or a quote");
	lastix++;
      };
    } // end of last name validation


    // validate the email
    {
      int emailsiz = emailqs.size();
      int emailen = emailstr.size();
      if (emailsiz != emailen) // we have non ASCII characters
	throw RPS_RUNTIME_ERROR_OUT("non ASCII characters in email "<<emailstr);
      if (emailen < 4)
	throw RPS_RUNTIME_ERROR_OUT("too short email" << emailstr);
      int nbat = 0;
      int nbdot = 0;
      int nbdash = 0;
      int nbalnum = 0;
      int emailix = 0;
      for (char c: emailstr) {
	if (c=='@') {
	  if (nbat > 0)
	    throw RPS_RUNTIME_ERROR_OUT("more than one @ in email " << emailstr);
	  if (emailix >= emailen-1)
	    throw RPS_RUNTIME_ERROR_OUT("ending @ in email " << emailstr);
	  nbat++;
	} else if (c=='.')
	  nbdot++;
	else if (isalnum(c)) 
	  nbalnum++;
	else if (c == '-' || c=='+' || c=='_') {
	  if (emailix==0 || emailix>=emailen-2)
	    throw RPS_RUNTIME_ERROR_OUT("strange dash or plus in email " << emailstr);
	  nbdash++;
	}
	else
	  throw RPS_RUNTIME_ERROR_OUT("invalid character in email " << emailstr);
	emailix ++;
      }
      if (nbalnum < 2 || nbat == 0)
	throw  RPS_RUNTIME_ERROR_OUT("bizarre email " << emailstr);
    }

    /// validate home page
    if (!webpageqs.isEmpty()) {
      QUrl weburl (webpageqs);
      if (!weburl.isValid()) 
	throw  RPS_RUNTIME_ERROR_OUT("invalid homepage URL " << webpagestr);
      QNetworkRequest webreq(weburl);
      RPS_INFORMOUT("RpsQCreateContributorDialog after weburl for webpagestr:" << webpagestr);
      QNetworkAccessManager* webacc = new QNetworkAccessManager(this);
      RPS_INFORMOUT("RpsQCreateContributorDialog  webacc:" << webacc);
      RPS_ASSERT(webacc != nullptr);
      /// I am not sure that an HTTP request is happening here....
      auto webrepl = webacc->head(webreq);
      RPS_INFORMOUT("RpsQCreateContributorDialog  webrepl:" << webrepl
		    << ", webacc=" << webacc);
      if (!webrepl) {
	RPS_WARNOUT("failed to HTTP HEAD web page " << webpagestr);
	webacc->deleteLater();
	throw RPS_RUNTIME_ERROR_OUT("invalid homepage, HEAD request failed on " << webpagestr);
      };
      webacc->deleteLater();
      webrepl->deleteLater();
    }
    ///
    RPS_INFORMOUT("RpsQCreateContributorDialog should create contributor for:"
		  << " firstnamestr=" << firstnamestr
		  << ", lastnamestr=" << lastnamestr
		  << ", emailstr=" << emailstr
		  << ", webpagestr=" << webpagestr
		  );
    _.obcontrib =
      Rps_ObjectRef::make_object(&_, //
				 RPS_ROOT_OB(_5CYWxcChKN002rw1fI), //contributor_to_RefPerSys
				 Rps_ObjectRef::root_space());
#warning in commit 8ff0d00e733f508ee605dfcb the calls to Rps_StringValue are buggy
    RPS_ASSERT(!firstnamestr.empty());
    _.firstnamev = Rps_StringValue(firstnamestr);
    RPS_ASSERT(!lastnamestr.empty());
    _.lastnamev = Rps_StringValue(lastnamestr);
    RPS_ASSERT(!emailstr.empty());
    _.emailv = Rps_StringValue(emailstr);
    if (!webpagestr.empty()) {
      _.webpagev = Rps_StringValue(webpagestr);
      _.obcontrib->put_attr4(RPS_ROOT_OB(_3N8vZ2Cw62z024XxCg) /*=first_name*/, _.firstnamev,
			     RPS_ROOT_OB(_6QAanFi9yLx00spBST) /*=last_name*/, _.lastnamev,
			     RPS_ROOT_OB(_0D6zqQNe4eC02bjfGs) /*=email*/, _.emailv,
			     RPS_ROOT_OB(_0LbCts6NacB03SMXz4) /*=home_page*/, _.webpagev);
    }
    else
      _.obcontrib->put_attr3(RPS_ROOT_OB(_3N8vZ2Cw62z024XxCg) /*=first_name*/, _.firstnamev,
			     RPS_ROOT_OB(_6QAanFi9yLx00spBST) /*=last_name*/, _.lastnamev,
			     RPS_ROOT_OB(_0D6zqQNe4eC02bjfGs) /*=email*/, _.emailv);
    // in all cases, for tracability, we put a git_id attribute in
    // _.obcontrib; personal data are important enough to be
    // easily tracable....
    {
      char gitidbuf[24];
      memset(gitidbuf, 0, sizeof(gitidbuf));
      strncpy(gitidbuf, rps_gitid, sizeof(gitidbuf)-2);
      _.gitidv = Rps_StringValue(gitidbuf);
      _.obcontrib->put_attr(RPS_ROOT_OB(_0XMNvzdABUM03Bj7WP), // git_id
			    _.gitidv);
    }
    RPS_INFORMOUT("RpsQCreateContributorDialog created contributor " << _.obcontrib
		  << " with firstnamestr=" << firstnamestr
		  << ", lastnamestr=" << lastnamestr
		  << ", emailstr=" << emailstr
		  << ", webpagestr=" << webpagestr
		  << " at git id " << rps_gitid
		  );
    _.obsetcontributors = RPS_ROOT_OB(_1wihX3eWD9o00QnxUX); // our_contributors value
    //RPS_ASSERT (_.obsetcontributors == rpskob_1wihX3eWD9o00QnxUX)
    RPS_ASSERT(_.obsetcontributors
	       && _.obsetcontributors->get_class() == Rps_ObjectRef::the_mutable_set_class()
	       );
    {
      std::lock_guard<std::recursive_mutex> gu(*(_.obsetcontributors->objmtxptr()));
      auto setpayl = _.obsetcontributors->get_dynamic_payload<Rps_PayloadSetOb>();
      RPS_ASSERT(setpayl != nullptr);
      setpayl->add(_.obcontrib);
    }
    RPS_INFORMOUT("RpsQCreateContributorDialog added contributor " << _.obcontrib
		  << " to `our_contributors` " << _.obsetcontributors);

    std::string mailcmd;
    mailcmd = "mail -s 'new RefPerSys contributor' ";
    mailcmd += emailstr;
    FILE* mailpipe = popen(mailcmd.c_str(), "w");
    if (!mailpipe)
      throw  RPS_RUNTIME_ERROR_OUT("failed to popen: " << mailcmd
				   << ":" << strerror(errno));
    fprintf(mailpipe, "Hello %s %s,\n", firstnamestr.c_str(), lastnamestr.c_str());
    fprintf(mailpipe, "Welcome to RefPerSys - see http://refpersys.org/ for more.\n");
    std::ostringstream outs;
    outs << "Created new RefPerSys contributor reified as " <<  _.obcontrib << std::endl
	 << " with first-name: " << firstnamestr << std::endl
	 << " with last-name: " << lastnamestr << std::endl
	 << " with email: " << emailstr << std::endl;
    if (!webpagestr.empty())
      outs << " with home-page: " << webpagestr << std::endl;
    std::string msg = outs.str();
    fprintf (mailpipe, "%s\n\n", msg.c_str());
    fprintf (mailpipe,
	     "if your login shell is nearly POSIX compatible (e.g. is bash or zsh), consider adding\n"
	     "    export REFPERSYS_USER_OID=%s\n"
	     "to your ~/.bashrc or ~/.zshrc or interactive shell initialization.\n",
	     _.obcontrib->oid().to_string().c_str());
    fprintf (mailpipe, "Consider also contacting team@refpersys.org please.\n");
    fflush (mailpipe);
    RPS_INFORMOUT("RpsQCreateContributorDialog mailpipe fd=" << fileno(mailpipe)
		  << ", contrib=" << _.obcontrib
		  << ", email=" << emailstr);
    int mailcode = pclose(mailpipe);
    mailpipe = nullptr;
    if (mailcode >0)
      RPS_WARNOUT("failed to pclose " << mailcmd << " got code:" << mailcode);
    QMessageBox::information(parentWidget(), "Created Contributor", msg.c_str());      
  } catch (std::exception& exc) {
    RPS_WARNOUT(
		"RpsQCreateContributorDialog failed:"
		<< " firstnamestr=" << firstnamestr
		<< ", lastnamestr=" << lastnamestr
		<< ", emailstr=" << emailstr
		<< ", webpagestr=" << webpagestr
		<< std::endl
		<< "got exception:" << exc.what()
		);
  }

  deleteLater();
}  // end RpsQCreateContributorDialog::on_ok_trigger()


void
RpsQCreateContributorDialog::on_cancel_trigger()
{
  deleteLater();
} // end RpsQCreateContributorDialog::on_cancel_trigger


void 
RpsQCreateContributorDialog::on_agree_change(int state)
{
  // ensure that the contributor has provided her first name, last name, e-mail
  // and has checked the agreement checkbox

  if (state == Qt::Checked) {
    auto fname = firstname_edit.text();
    auto lname = lastname_edit.text();
    auto email = email_edit.text();

    ok_button.setEnabled(fname.size() && lname.size() && email.size());
  }
  else {
    ok_button.setEnabled(false);
  }
}  // end RpsQCreateContributorDialog::on_agree_change()


void
RpsQCreateContributorDialog::on_fname_edit(const QString& text)
{
  (void) text; // suppress unused parameter warning

  // ensure that the contributor has provided her first name, last name, e-mail
  // and has checked the agreement checkbox

  auto fname = firstname_edit.text();
  auto lname = lastname_edit.text();
  auto email = email_edit.text();
  auto agree = agree_chk.isChecked();

  ok_button.setEnabled(fname.size() && lname.size() && email.size() && agree);
}  // end RpsQCreateContributorDialog::on_fname_edit()


void
RpsQCreateContributorDialog::on_lname_edit(const QString& text)
{
  RpsQCreateContributorDialog::on_fname_edit(text);
}  // end RpsQCreateContributorDialog::on_lname_edit()


void
RpsQCreateContributorDialog::on_email_edit(const QString& text)
{
  RpsQCreateContributorDialog::on_fname_edit(text);
}  // end RpsQCreateContributorDialog::on_email_edit()


RpsQCreatePluginDialog::RpsQCreatePluginDialog(RpsQWindow* parent)
  : QDialog(parent), dialog_vbx(), button_hbx(), 
    code_lbl("Plugin Code:", this), 
    code_txt(this), ok_btn("Compile and Run", this), cancel_btn("Cancel", this)
{
    random_id = Rps_Id::random().to_string();
  
  // set widget names
  dialog_vbx.setObjectName("RpsQCreatePluginDialog_dialog_vbx");
  button_hbx.setObjectName("RpsQCreatePluginDialog_button_hbx");
  code_lbl.setObjectName("RpsQCreatePluginDialog_code_lbl");
  code_txt.setObjectName("RpsQCreatePluginDialog_code_txt");
  ok_btn.setObjectName("RpsQCreatePluginDialog_ok_btn");
  cancel_btn.setObjectName("RpsQCreatePlubinDialog_cancel_btn");

  // set widget fonts
  {
    auto arial = QFont("Arial", 12);
    code_lbl.setFont(arial);
    ok_btn.setFont(arial);
    cancel_btn.setFont(arial);

    auto courier = QFont("Courier", 12);
    code_txt.setFont(courier);


    std::ostringstream boilerplate;
    boilerplate << "// file /tmp/rps" << random_id << ".cc" << std::endl
                << "#include \"refpersys.hh\"" << std::endl << std::endl
                << "extern \"C\" void "
		<< (temporary_function_name()) << " (Rps_CallerFrame* caller);" << std::endl << std::endl
                << "void "  << (temporary_function_name()) << " (Rps_CallerFrame* caller) {"
                << std::endl << "  RPS_LOCALFRAME("
      /// see https://gitlab.com/bstarynk/refpersys/-/wikis/call-frames-in-RefPerSys
		<< "/*no descr:*/nullptr, caller," << std::endl
		<< "           Rps_Value val; // "  << std::endl
                << "           Rps_ObjectRef obattr; // " <<  std::endl
                << std::endl << "  );" << std::endl
                << "// body of " <<  (temporary_function_name()) << " ::::" << std::endl
		<< " ;" << std::endl
                << "} // end " <<  (temporary_function_name()) << std::endl << std::endl
                << "// end of file /tmp/rps" << random_id << ".cc" << std::endl;
    code_txt.setText(QString(boilerplate.str().c_str()));
  }

  // layout widgets


  dialog_vbx.addWidget(&code_lbl);
  dialog_vbx.addWidget(&code_txt);
  
  dialog_vbx.addLayout(&button_hbx);
  button_hbx.addWidget(&ok_btn);
  button_hbx.addSpacing(3);
  button_hbx.addWidget(&cancel_btn);

  setLayout(&dialog_vbx);

  // connect slots

  connect(&ok_btn, &QAbstractButton::clicked, this,
    &RpsQCreatePluginDialog::on_ok_trigger
  );

  connect(&cancel_btn, &QAbstractButton::clicked, this,
    &RpsQCreatePluginDialog::on_cancel_trigger
  );
}


RpsQCreatePluginDialog::~RpsQCreatePluginDialog()
{
}


void
RpsQCreatePluginDialog::on_ok_trigger()
{
  typedef void pluginsig_t (Rps_CallFrame*);
  RPS_LOCALFRAME(Rps_ObjectRef(nullptr),//descriptor
                 nullptr,//parentframe
		 );
  {
    auto screengeom = RpsQApplication::the_app()->desktop()->screenGeometry();
    int w = 700;
    int h = 500;
    if (w > (3*screengeom.width())/4)
      w = 3*screengeom.width()/4;
    else if (w < (screengeom.width())/2)
      w = 16 + (screengeom.width())/2;
    if (h >  (3*screengeom.height())/4)
      h = 3*screengeom.height()/4;
    else if (h < screengeom.height()/2)
      h = 16 + screengeom.height()/2;
    this->resize(w, h);
  }
  
  std::string code = code_txt.toPlainText().toStdString();
  auto srcpath = temporary_cplusplus_file_path();
  auto pluginpath = temporary_plugin_file_path();

  RPS_INFORMOUT("RpsQCreatePluginDialog srcpath="
		<< srcpath << ", pluginpath= "
		<< pluginpath << ", code:"
		<< std::endl << code << std::endl);

  {
    std::ofstream out;
    out.open(srcpath.c_str(), std::ios::out);
    out << code << std::endl;
    out.close();
  }

  QProcess proc;
  QStringList procargs;
  procargs << QString(temporary_cplusplus_file_path().c_str())
	   << QString(temporary_plugin_file_path().c_str());
  proc.setProgram("./build-temporary-plugin.sh");
  proc.setArguments(procargs);
  proc.waitForFinished();

  auto rc = proc.exitStatus();
  QString msg(proc.readAllStandardOutput());
  QString err(proc.readAllStandardError());

  RPS_INFORMOUT("RpsQCreatePluginDialog::on_ok_trigger(): exit code = "
                << rc << "; build msg = " << msg.toStdString());

  if (rc == 0) {
    try {
      void *dlh = dlopen(temporary_plugin_file_path().c_str(), RTLD_NOW | RTLD_GLOBAL);
      if (!dlh)
	throw RPS_RUNTIME_ERROR_OUT("dlopen of " << temporary_plugin_file_path()
				    << " failed:" << dlerror());
      void *funp = dlsym(dlh, temporary_function_name().c_str());
      if (!funp)
	throw RPS_RUNTIME_ERROR_OUT("dlsym of " << temporary_function_name()
				    << " in " << temporary_plugin_file_path()
				    << " failed:" << dlerror());
      (*reinterpret_cast<pluginsig_t*>(funp)) (&_);
      
      QMessageBox::information(this, "Success!", 
			       QString("The plugin %1 was successfully built and executed.").arg(temporary_plugin_file_path().c_str()));
      deleteLater();
    }
    catch (const std::exception& exc)
      {
	err += exc.what();
	RPS_WARNOUT("RpsQCreateClassDialog exception " << exc.what());
	QMessageBox::warning(this,
			     QString("Plugin %1 run failure!").arg(temporary_plugin_file_path().c_str()),
			     err);
      }
  }

  else {
    QMessageBox::warning(this, "Plugin build failure!", err);
  }
                
#warning TODO: Abhishek will fix the bug
  // notably in build-temporary-plugin.sh related to failing omake

} // end RpsQCreatePluginDialog::on_ok_trigger


void
RpsQCreatePluginDialog::on_cancel_trigger()
{
    deleteLater();
}


////////////////////////////////////////////////////////////////
RpsQCommandTextEdit::RpsQCommandTextEdit(QWidget*parent) : QTextEdit(parent) {
  setDocumentTitle("command");
} // end RpsQCommandTextEdit::RpsQCommandTextEdit

RpsQCommandTextEdit::~RpsQCommandTextEdit() {
} // end RpsQCommandTextEdit::~RpsQCommandTextEdit

////////////////////////////////////////////////////////////////
RpsQOutputTextEdit::RpsQOutputTextEdit(QWidget*parent) : QTextEdit(parent) {
  setDocumentTitle("output");
} // end RpsQOutputTextEdit::RpsQOutputTextEdit

RpsQOutputTextEdit::~RpsQOutputTextEdit() {
} // end RpsQOutputTextEdit::~RpsQOutputTextEdit

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


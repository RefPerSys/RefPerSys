// file misc-basile/q6refpersys.cc
// SPDX-License-Identifier: GPL-3.0-or-later

/***
    © Copyright 2024 - 2025 by Basile Starynkevitch
   program released under GNU General Public License v3+

   This is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   This q6refpersys program is an opensource Qt6 application (Qt is a
   graphical user toolkit for Linux; see https://www.qt.io/product/qt6
   ...) It is the interface to the RefPerSys inference engine on
   http://refpersys.org/ and communicates with the refpersys process
   using some JSONRPC protocol on named fifos. In contrast to
   refpersys itself, the q6refpersys process is short lived.

****/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#define UNUSED __attribute__((unused))
extern "C" const char myqr_git_id[];
extern "C" char myqr_host_name[64];

#ifndef GITID
#error GITID should be defined in compilation command
#endif



#include <QApplication>
#include <QProcess>
#include <QCommandLineParser>
#include <QDebug>
#include <QMainWindow>
#include <QMenuBar>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QSizePolicy>
#include <QLabel>
#include <QSocketNotifier>
//#include <QJsonValue>
#include <QtCore/QtCoreVersion>
//#include <QJsonValue>
#include <QtCore/qglobal.h>


// from jsoncpp (see https://github.com/open-source-parsers/jsoncpp)
#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"

#include <iostream>
#include <sstream>
#include <functional>
#include <mutex>
#include <deque>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


extern "C" char myqr_host_name[];
extern "C" char* myqr_progname;
extern "C" bool myqr_debug;
extern "C" std::string myqr_jsonrpc; // the FIFO prefix
extern "C" std::string myqr_refpersys_topdir;
extern "C" int myqr_jsonrpc_cmd_fd; /// written by RefPerSys, read by q6refpersys
extern "C" QSocketNotifier* myqr_notifier_jsonrpc_cmd;
extern "C" std::recursive_mutex myqr_mtx_jsonrpc_cmd;
extern "C" std::stringstream myqr_stream_jsonrpc_cmd;
extern "C" Json::CharReader* myqr_jsonrpc_reader;
extern "C" Json::CharReaderBuilder myqr_jsoncpp_reader_builder;

extern "C" QSocketNotifier* myqr_notifier_jsonrpc_out;
extern "C" int myqr_jsonrpc_out_fd; /// read by RefPerSys, written by q6refpersys
extern "C" std::recursive_mutex myqr_mtx_jsonrpc_out;
extern "C" std::deque<Json::Value> myqr_deque_jsonrpc_out;
extern "C" std::stringstream myqr_stream_jsonrpc_out;
extern "C" std::map<int,std::function<void(const Json::Value&res)>> myqr_jsonrpc_out_procmap;
extern "C" Json::StreamWriterBuilder myqr_jsoncpp_writer_builder;

extern "C" pid_t myqr_refpersys_pid;

extern "C" std::string myqr_json2str(const Json::Value&jv);

/// process the JSON recieved from refpersys
extern "C" void myqr_process_jsonrpc_from_refpersys(const Json::Value&js);


/// do a remote procedure call to RefPerSys using our JSONRPC variant
extern "C" void myqr_call_jsonrpc_to_refpersys
(const std::string& method,
 const Json::Value& args,
 const std::function<void(const Json::Value&res)>& resfun);

#define MYQR_FATALOUT_AT_BIS(Fil,Lin,Out) do {  \
    std::ostringstream outs##Lin;   \
    outs##Lin << Out << std::flush;   \
    qFatal("%s:%d: %s\n[git %s@%s] on %s",  \
     Fil, Lin, outs##Lin.str().c_str(),   \
     myqr_git_id, __DATE__" " __TIME__, \
     myqr_host_name);     \
    abort();          \
  } while(0)

#define MYQR_FATALOUT_AT(Fil,Lin,Out) \
  MYQR_FATALOUT_AT_BIS(Fil,Lin,Out)

#define MYQR_FATALOUT(Out) MYQR_FATALOUT_AT(__FILE__,__LINE__,Out)

#define MYQR_DEBUGOUT_AT_BIS(Fil,Lin,Out) do {  \
    if (myqr_debug)       \
      std::clog << Fil << ":" << Lin << " " \
    << Out << std::endl;    \
  } while(0)

#define MYQR_DEBUGOUT_AT(Fil,Lin,Out) \
  MYQR_DEBUGOUT_AT_BIS(Fil,Lin,Out)

#define MYQR_DEBUGOUT(Out) MYQR_DEBUGOUT_AT(__FILE__,__LINE__,Out)

extern "C" QApplication *myqr_app;

extern "C" {
  class MyqrMainWindow;
  class MyqrDisplayWindow;
};



////////////////////////////////////////////////////////////////
class MyqrMainWindow : public QMainWindow
{
  Q_OBJECT;
  //// the menubar
  QMenuBar* _mainwin_menubar;
  QMenu* _mainwin_appmenu;
  QAction* _mainwin_aboutact;
  QAction* _mainwin_aboutqtact;
  QMenu* _mainwin_editmenu;
  QAction* _mainwin_copyact;
  QAction* _mainwin_pasteact;
  /// the central widget is a vertical group box
  QGroupBox* _mainwin_centralgroup;
  QLabel*_mainwin_toplabel;
  QLineEdit*_mainwin_cmdline;
  QTextEdit*_mainwin_textoutput;
private slots:
  void about();
  void aboutQt();
public:
  static MyqrMainWindow*the_instance;
  explicit MyqrMainWindow(QWidget*parent = nullptr);
  virtual ~MyqrMainWindow();
  static constexpr int minimal_width = 512;
  static constexpr int minimal_height = 128;
  static constexpr int maximal_width = 2048;
  static constexpr int maximal_height = 1024;
};        // end MyqrMainWindow
MyqrMainWindow*MyqrMainWindow::the_instance;





////////////////////////////////////////////////////////////////
class MyqrDisplayWindow : public QMainWindow
{
  Q_OBJECT;
private slots:
  void about();
public:
  explicit MyqrDisplayWindow(QWidget*parent = nullptr);
  virtual ~MyqrDisplayWindow();
};        // end MyqrDisplayWindow

////////////////////////////////////////////////////////////////
extern "C" QProcess*myqr_refpersys_process;
//=============================================================

std::ostream& operator << (std::ostream&out, const QList<QString>&qslist)
{
  int nbl=0;
  for (const QString&qs: qslist)
    {
      if (nbl++ > 0)
        out << ' ';
      std::string s = qs.toStdString();
      bool needquotes=false;
      for (char c: s)
        {
          if (!isalnum(c)&& c!='_') needquotes=true;
        }
      if (needquotes)
        out << "'";
      for (QChar qc : qs)
        {
          switch (qc.unicode())
            {
            case '\\':
              out << "\\\\";
              break;
            case '\'':
              out << "\\'";
              break;
            case '\"':
              out << "\\\"";
              break;
            case '\r':
              out << "\\r";
              break;
            case '\n':
              out << "\\n";
              break;
            case '\t':
              out << "\\t";
              break;
            case '\v':
              out << "\\v";
              break;
            case '\f':
              out << "\\f";
              break;
            case ' ':
              out << " ";
              break;
            default:
              out << QString(qc).toStdString();
              break;
            }
        }
      if (needquotes)
        out << "'";
    }
  return out;
}// end operator << (std::ostream&out, const QList<QString>&qslist)

////////////////////////////////////////////////////////////////


MyqrMainWindow::MyqrMainWindow(QWidget*parent)
  : QMainWindow(parent),
    _mainwin_menubar(nullptr),
    _mainwin_appmenu(nullptr),
    _mainwin_aboutact(nullptr),
    _mainwin_aboutqtact(nullptr),
    _mainwin_editmenu(nullptr),
    _mainwin_copyact(nullptr),
    _mainwin_pasteact(nullptr),
    _mainwin_centralgroup(nullptr),
    _mainwin_toplabel(nullptr),
    _mainwin_cmdline(nullptr),
    _mainwin_textoutput(nullptr)
{
  if (the_instance != nullptr)
    MYQR_FATALOUT("duplicate MyqrMainWndow @" << (void*)the_instance
                  << " and this@" << (void*)this);
  _mainwin_menubar = menuBar();
  _mainwin_appmenu =_mainwin_menubar-> addMenu("App");
  _mainwin_aboutact = _mainwin_appmenu->addAction("About");
  QObject::connect(_mainwin_aboutact,&QAction::triggered,this,&MyqrMainWindow::about);
  _mainwin_aboutqtact = _mainwin_appmenu->addAction("About Qt");
  QObject::connect(_mainwin_aboutqtact,&QAction::triggered,this,&MyqrMainWindow::aboutQt);
  _mainwin_editmenu =_mainwin_menubar-> addMenu("Edit");
  _mainwin_copyact =  _mainwin_editmenu->addAction("Copy");
  _mainwin_pasteact = _mainwin_editmenu->addAction("Paste");
  _mainwin_centralgroup = new QGroupBox(this);
  {
    QVBoxLayout *vbox = new QVBoxLayout;
    _mainwin_centralgroup->setLayout(vbox);
    _mainwin_toplabel = new QLabel("q6refpersys");
    vbox->addWidget(_mainwin_toplabel);
    _mainwin_cmdline = new QLineEdit(_mainwin_centralgroup);
    _mainwin_cmdline->setFixedWidth(this->width()-16);
    vbox->addWidget(_mainwin_cmdline);
    _mainwin_textoutput = new QTextEdit();
    _mainwin_textoutput->setReadOnly(true);
    vbox->addWidget(_mainwin_textoutput);
  }
  setCentralWidget(_mainwin_centralgroup);
  the_instance = this;
  MYQR_DEBUGOUT("MyqrMainWndow the_instance@" << (void*)the_instance
                << " parent@" << (void*)parent);
  setMinimumWidth(minimal_width);
  setMinimumHeight(minimal_height);
  setMaximumWidth(maximal_width);
  setMaximumHeight(maximal_height);
  qDebug() << "incomplete MyqrMainWindow constructor "
           << __FILE__  ":" << (__LINE__-1);
#warning incomplete MyqrMainWindow constructor
} // end MyqrMainWindow constructor

MyqrDisplayWindow::MyqrDisplayWindow(QWidget*parent)
  : QMainWindow(parent)
{
} // end MyqrDisplayWindow constructor

void
MyqrDisplayWindow::about()
{
  qDebug() << " unimplemented MyqrDisplayWindow::about";
} // end MyqrDisplayWindow::about

MyqrMainWindow::~MyqrMainWindow()
{
  if (the_instance != this)
    MYQR_FATALOUT("corruption in MyqrMainWndow the_instance@" << (void*)the_instance
                  << " this@" << (void*)this);
  the_instance = nullptr;
} // end MyqrMainWindow destructor

MyqrDisplayWindow::~MyqrDisplayWindow()
{
} // end MyqrDisplayWindow destructor

void
MyqrMainWindow::aboutQt()
{
  MYQR_DEBUGOUT("unimplemented MyqrMainWindow::aboutQt");
#warning unimplemented MyqrMainWindow::aboutQt
} // end MyqrDisplayWindow::aboutQt

void
MyqrMainWindow::about()
{
  MYQR_DEBUGOUT("unimplemented MyqrMainWindow::about");
#warning unimplemented MyqrMainWindow::about
} // end MyqrDisplayWindow::aboutQt

void myqr_create_windows(const QString& geom);

void
myqr_create_windows(const QString& geom)
{
  MYQR_DEBUGOUT("incomplete myqr_create_windows geometry "
                << geom.toStdString() << ";");
  int w=0, h=0;
  const char*geomcstr = geom.toStdString().c_str();
  if (geomcstr != nullptr)
    {
      int p= -1;
      MYQR_DEBUGOUT("myqr_create_windows geomcstr='" << geomcstr << "'");
      if (sscanf(geomcstr, " %dx%d %n", &w, &h, &p) >= 2)
        {
          MYQR_DEBUGOUT("scanned w=" << w << " h=" << h
                        << " geomcstr='" << geomcstr << "' p=" << p);
          if (w > MyqrMainWindow::maximal_width)
            w= MyqrMainWindow::maximal_width;
          if (h > MyqrMainWindow::maximal_height)
            h=MyqrMainWindow::maximal_height;
        }
    }
  if (w< MyqrMainWindow::minimal_width)
    w=MyqrMainWindow::minimal_width;
  if (h< MyqrMainWindow::minimal_height)
    h= MyqrMainWindow::minimal_height;
  MYQR_DEBUGOUT("myqr_create_windows normalized w=" << w << ", h=" << h
                << " from  geomcstr='" << geomcstr << "'");
  auto mainwin = new MyqrMainWindow(nullptr);
  mainwin->resize(w,h);
  mainwin->show();
  MYQR_DEBUGOUT("myqr_create_windows incomplete mainwin@" << (void*)mainwin);
#warning incomplete myqr_create_windows
} // end myqr_create_windows

std::string myqr_refpersystop;

/// This function gets called when some bytes could be read on the
/// file descriptor (FIFO) from refpersys to GUI.
void
myqr_readable_jsonrpc_cmd(void)
{
  MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd start myqr_jsonrpc_cmd_fd="
                << myqr_jsonrpc_cmd_fd);
  /*** NOTE:

       We actually expect that most JSON objects sent by refpersys to
       this GUI process are short, typically a few dozen bytes
       each. And each JSON message is terminated by a formfeed (which is
       invalid in JSON, in C "\f") or a double newline, in C "\n\n" ...

       In rare cases refpersys might send a large JSON (over a
       kilobyte), but we expect this to be not frequent.  In other
       words, there is a lot of string copying happening here, to ease
       coding.
   ***/
  const std::lock_guard<std::recursive_mutex> lock(myqr_mtx_jsonrpc_cmd);
  constexpr unsigned jrbufsize= 2048;
  char buf [jrbufsize+4];
  memset (buf, 0, sizeof(buf));
  char errbuf[64];
  memset (errbuf, 0, sizeof(errbuf));
  errno = 0;
  ssize_t rdcnt = read(myqr_jsonrpc_cmd_fd, buf, jrbufsize);
  int rderr = errno;
  MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd read JSONRPC cmdfd#" <<
                myqr_readable_jsonrpc_cmd << " recieved "
                << rdcnt << " bytes in buffer of " << jrbufsize);
  if (rdcnt < 0 && rderr > 0)
    {
      strerror_r(rderr, errbuf, sizeof(errbuf));
      assert(errbuf[0] != (char)0);
    };
  MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd got rdcnt=" << rdcnt
                << (rderr?" : ":".")
                << (rderr?errbuf:""));
  if (rdcnt<0)
    {
      if (rderr == EWOULDBLOCK || rderr == EAGAIN || rderr == EINTR)
        return;
      MYQR_FATALOUT("myqr_readable_jsonrpc_cmd read fd#"
                    << myqr_jsonrpc_cmd_fd << " failed: " << errbuf);
    }
  else if (rdcnt==0)
    {
      MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd got EOF on fd#"
                    << myqr_jsonrpc_cmd_fd);
      myqr_jsonrpc_cmd_fd = -1;
      exit(EXIT_SUCCESS);
    }
  buf[rdcnt] = (char)0;
  MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd incomplete read " << rdcnt
                << " bytes:" << std::endl << buf);
  size_t oldcmdlen = myqr_stream_jsonrpc_cmd.tellp();
  MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd oldcmdlen:" << oldcmdlen);
  myqr_stream_jsonrpc_cmd.write(buf, rdcnt);
  char*begm = buf;
  while(begm && *begm)
    {
      char*ff = strchr(begm, '\f');
      char*nn = strstr(begm, "\n\n");
      char*eom = nullptr;
      if (!ff && !nn)
        {
          /// the JSON message is incomplete since not ended by formfeed
          /// or double-newline
          return;
        };
      if (ff != nullptr && nn != nullptr)
        {
          eom = (ff < nn)?ff:nn;
        }
      else if (ff != nullptr)
        eom = ff+1;
      else if (nn != nullptr)
        eom = nn+2;
      else
        {
          /// this should never happen
          MYQR_FATALOUT("myqr_readable_jsonrpc_cmd bug ff=" << ff << " nn=" << nn);
        };
      int deltaeom = eom - begm;
      assert(deltaeom > 0);
      std::string jsonstr;
      if (oldcmdlen>0)
        {
          jsonstr.assign(myqr_stream_jsonrpc_cmd.str());
          jsonstr.append(begm, deltaeom);
          oldcmdlen=0;
        }
      else
        jsonstr.assign(begm, deltaeom);
      begm = eom+1;
      MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd jsonstr=" << jsonstr);
      Json::Value jv;
      std::string errmsg;
      bool okparse = //
        myqr_jsonrpc_reader->parse(jsonstr.c_str(), &jsonstr.back(),
                                   &jv, &errmsg);
      if (!okparse)
        MYQR_FATALOUT("myqr_readable_jsonrpc_cmd failed to parse:"
                      << std::endl  << jsonstr
                      << std::endl << "error:" << errmsg);
      MYQR_DEBUGOUT("myqr_readable_jsonrpc_cmd parsed json:"
                    << std::endl << jv.asString() << std::endl);
      myqr_process_jsonrpc_from_refpersys(jv);
    };
} // end myqr_readable_jsonrpc_cmd



void
myqr_writable_jsonrpc_out(void)
{
  MYQR_DEBUGOUT("myqr_writable_jsonrpc_out unimplemented myqr_jsonrpc_out_fd="
                << myqr_jsonrpc_out_fd);
#warning unimplemented myqr_writable_jsonrpc_out
} // end myqr_writable_jsonrpc_out



/// a JSONRPC communication happens with RefPerSys
void
myqr_have_jsonrpc(const std::string&jsonrpc)
{
  MYQR_DEBUGOUT("myqr_have_jsonrpc incomplete " << jsonrpc);
  std::string jsonrpc_cmd = jsonrpc+".cmd"; /// written by RefPerSys, read by q6refpersys
  std::string jsonrpc_out = jsonrpc+".out"; /// read by RefPerSys, written by q6refpersys
  if (access(jsonrpc_cmd.c_str(), F_OK))
    {
      if (mkfifo(jsonrpc_cmd.c_str(), 0660)<0)
        MYQR_FATALOUT("failed to create command JSONRPC fifo " << jsonrpc_cmd << ":" << strerror(errno));
      else
        MYQR_DEBUGOUT("myqr_have_jsonrpc created command fifo " << jsonrpc_cmd);
    };
  if (access(jsonrpc_out.c_str(), F_OK))
    {
      if (mkfifo(jsonrpc_out.c_str(), 0660)<0)
        MYQR_FATALOUT("failed to create output JSONRPC fifo " << jsonrpc_out << ":" << strerror(errno));
      else
        MYQR_DEBUGOUT("myqr_have_jsonrpc created output fifo " << jsonrpc_out);
    };
  myqr_jsonrpc_cmd_fd = open(jsonrpc_cmd.c_str(), 0440 | O_CLOEXEC | O_NONBLOCK);
  if (myqr_jsonrpc_cmd_fd<0)
    MYQR_FATALOUT("failed to open command JSONRPC " << jsonrpc_cmd << " for reading:" << strerror(errno));
  else
    MYQR_DEBUGOUT("myqr_have_jsonrpc cmd fd#" << myqr_jsonrpc_cmd_fd);
  myqr_notifier_jsonrpc_cmd = new QSocketNotifier(myqr_jsonrpc_cmd_fd, QSocketNotifier::Read);
  QObject::connect(myqr_notifier_jsonrpc_cmd,&QSocketNotifier::activated,
                   myqr_readable_jsonrpc_cmd);
  myqr_jsonrpc_out_fd = open(jsonrpc_out.c_str(), 0660 | O_CLOEXEC | O_NONBLOCK);
  if (myqr_jsonrpc_out_fd<0)
    MYQR_FATALOUT("failed to open output JSONRPC " << jsonrpc_out << " for writing:" << strerror(errno));
  else
    MYQR_DEBUGOUT("myqr_have_jsonrpc out fd#" << myqr_jsonrpc_out_fd);
  myqr_notifier_jsonrpc_out = new QSocketNotifier(myqr_jsonrpc_out_fd, QSocketNotifier::Write);
  QObject::connect(myqr_notifier_jsonrpc_out,&QSocketNotifier::activated,myqr_writable_jsonrpc_out);
  if (setenv("REFPERSYS_JSONRPC", jsonrpc.c_str(), /*overwrite:*/(int)true))
    {
      MYQR_FATALOUT("failed to setenv REFPERSYS_JSONRPC to " << jsonrpc
                    << " :" << strerror(errno));
    }
  else
    {
      MYQR_DEBUGOUT("myqr_have_jsonrpc did setenv REFPERSYS_JSONRPC to "
                    << jsonrpc.c_str());
    };
  MYQR_DEBUGOUT("myqr_have_jsonrpc installed cmd: " << jsonrpc_cmd << " fd#" << myqr_jsonrpc_cmd_fd
                << " out: " << jsonrpc_out
                << " fd#" << myqr_jsonrpc_out_fd);
  MYQR_DEBUGOUT("myqr_have_jsonrpc ending jsonrpc:" << jsonrpc
                << " cmd.fd#" << myqr_jsonrpc_cmd_fd << " out.fd#" << myqr_jsonrpc_out_fd);
} // end myqr_have_jsonrpc



void
myqr_start_refpersys(const std::string& refpersysprog,
                     QStringList&arglist)
{
  std::string prog= refpersysprog;
  qint64 pid= 0;
  MYQR_DEBUGOUT("starting myqr_start_refpersys " << refpersysprog
                << " " << arglist << " from " << myqr_progname
                << " on " << myqr_host_name
                << " pid " << (int)getpid() << " git " << myqr_git_id);
  if (refpersysprog.empty())
    {
      prog = "refpersys";
    }
  else if (refpersysprog[0] == '-')
    {
      prog = "refpersys";
      arglist.prepend(QString(refpersysprog.c_str()));
    };
  MYQR_DEBUGOUT("myqr_start_refpersys prog=" << prog << " arglist=" << arglist
                << "before process creation from pid " << (int)getpid());
  myqr_refpersys_process = new QProcess();
  std::string progname;
  if (prog.find('/') == std::string::npos)
    {
      const char* path = getenv("PATH");
      const char*pc = nullptr;
      const char*colon = nullptr;
      const char*nextpc = nullptr;
      MYQR_DEBUGOUT("myqr_start_refpersys PATH=" << path
                    << " prog=" << prog);
      for (pc = path; pc && *pc; pc = nextpc)
        {
          colon = strchr(pc, ':');
          nextpc = colon?(colon+1):nullptr;
          std::string dir(pc, colon?(colon-pc):strlen(pc));
          std::string exepath = dir + "/" + prog;
          MYQR_DEBUGOUT("myqr_start_refpersys pc="
                        << pc << " dir=" << dir << " exepath=" << exepath);
          if (!access(exepath.c_str(), F_OK|X_OK))
            {
              progname = exepath;
              MYQR_DEBUGOUT("myqr_start_refpersys progname=" << progname);
              nextpc = nullptr;
              break;

            }
        };
      if (progname.empty())
        MYQR_FATALOUT("failed to find program " << prog
                      << " in PATH=" << path);
    }
  else
    {
      MYQR_DEBUGOUT("myqr_start_refpersys prog has slash " << prog);
      progname = prog;
    };
  myqr_refpersys_process->setProgram(QString(progname.c_str()));
  myqr_refpersys_process->setArguments(arglist);
  MYQR_DEBUGOUT("myqr_start_refpersys before starting " << progname
                << " with arguments " << arglist
                << " git " << myqr_git_id
                << " myqr_refpersys_process@" << (void*)myqr_refpersys_process);
  errno = 0;
  if (!myqr_refpersys_process->startDetached(&pid))
    {
      int e = errno;
      MYQR_FATALOUT("failed to start refpersys program: " << prog
                    << " from " << myqr_progname << " pid:" << getpid()
                    << " errno:" << strerror(e));
    };
  MYQR_DEBUGOUT("myqr_start_refpersys started " << prog
                << " with arguments " << arglist
                << " as pid " << pid);
  myqr_refpersys_pid = (pid_t) pid;
  usleep (320*1024); /// hopefully let the refpersys process run a little bit...
} // end myqr_start_refpersys



/// process the JSON recieved from refpersys
void
myqr_process_jsonrpc_from_refpersys(const Json::Value&js)
{
  MYQR_DEBUGOUT("myqr_process_jsonrpc_from_refpersys got JSON" << std::endl
                << myqr_json2str(js));
#warning myqr_process_jsonrpc_from_refpersys unimplemented
  MYQR_FATALOUT("unimplemented myqr_process_jsonrpc_from_refpersys"
                << std::endl
                << myqr_json2str(js));
} // end myqr_process_jsonrpc_from_refpersys




/// do a remote procedure call to RefPerSys using our JSONRPC variant
void
myqr_call_jsonrpc_to_refpersys
(const std::string& method,
 const Json::Value& args,
 const std::function<void(const Json::Value&res)>& resfun)
{
  Json::Value jresult;
  static int count;
  {
    std::lock_guard<std::recursive_mutex> lock(myqr_mtx_jsonrpc_out);
    Json::Value jreq(Json::objectValue);
    jreq["jsonrpc"] = "2.0";
    jreq["method"] = method;
    jreq["params"] = args;
    jreq["id"] = ++count;
    myqr_deque_jsonrpc_out.push_back(jreq);
    myqr_jsonrpc_out_procmap.insert_or_assign(count, resfun);
    MYQR_DEBUGOUT("myqr_call_jsonrpc_to_refpersys jreq:"
                  << myqr_json2str(jreq));

  }
  usleep(500);
  {
    std::lock_guard<std::recursive_mutex> lock(myqr_mtx_jsonrpc_out);
    if (!myqr_deque_jsonrpc_out.empty())
      {
        Json::Value joldreq = myqr_deque_jsonrpc_out.front();
        myqr_deque_jsonrpc_out.pop_front();
        myqr_stream_jsonrpc_out << myqr_json2str(joldreq)
                                << "\n\f" << std::flush;
      }
    std::string outs = myqr_stream_jsonrpc_out.str();
    size_t outslen = outs.size();
    errno = 0;
    ssize_t wcnt = write(myqr_jsonrpc_out_fd, outs.c_str(), outslen);
    MYQR_DEBUGOUT("myqr_call_jsonrpc_to_refpersys outslen:" << outslen
                  << " wcnt:" << wcnt << " outfd#" << myqr_jsonrpc_out_fd
                  << " errno:" << strerror(errno) << " pid:" << (int)getpid());
    if (wcnt>0)   // https://stackoverflow.com/a/4546562
      {
        if ((long)wcnt<(long)outslen)
          {
            outs.erase(0, wcnt);
            MYQR_DEBUGOUT("myqr_call_jsonrpc_to_refpersys outs becomes: '"
                          << outs << "'");
            myqr_stream_jsonrpc_out.str(outs);
          }
        else   // https://stackoverflow.com/a/20792
          {
            MYQR_DEBUGOUT("myqr_call_jsonrpc_to_refpersys outs cleared");
            myqr_stream_jsonrpc_out.str("");
          }
      }
#warning incomplete myqr_call_jsonrpc_to_refpersys
  }
} // end  myqr_call_jsonrpc_to_refpersys



std::string
myqr_json2str(const Json::Value&jv)
{
  Json::StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = " ";
  auto str = Json::writeString(builder, jv);
  return str;
} // end myqr_json2str

int
main(int argc, char **argv)
{
  myqr_progname = argv[0];
  {
    char*rfpt = getenv("REFPERSYS_TOPDIR");
    if (rfpt)
      {
        if (access(rfpt, R_OK))
          {
            int e=errno;
            std::clog << myqr_progname << " has bad REFPERSYS_TOPDIR="
                      << rfpt << ":" << strerror(e) << std::endl;
            exit(EXIT_FAILURE);
          }
        else
          myqr_refpersys_topdir = std::string(rfpt);
      }
    else
      {
        std::clog << myqr_progname << " needs a REFPERSYS_TOPDIR from environment"
                  << std::endl;
        exit(EXIT_FAILURE);
      };
  };
  for (int i=1; i<argc; i++)
    {
      if (!strcmp(argv[i], "-D") || !strcmp(argv[i], "--debug"))
        {
          qDebug().setVerbosity(QDebug::DefaultVerbosity);
          myqr_debug = true;
        }
    }
  gethostname(myqr_host_name, sizeof(myqr_host_name)-1);
  MYQR_DEBUGOUT("starting " << myqr_progname << " on " << myqr_host_name
                << " git " << myqr_git_id << " pid " << (int)getpid()
                << " argc=" << argc
                << " dynamic qVersion=" << qVersion());
  myqr_jsoncpp_reader_builder["collectComments"] = false;
  myqr_jsoncpp_reader_builder["rejectDupKeys"] = true;
  myqr_jsoncpp_writer_builder["commentStyle"] = "None";
  myqr_jsoncpp_writer_builder["indentation"] = "";
  myqr_jsonrpc_reader = myqr_jsoncpp_reader_builder.newCharReader();
  QCoreApplication::setApplicationName("q6refpersys");
  QCoreApplication::setApplicationVersion(QString("version ") + myqr_git_id
                                          + " " __DATE__ "@" __TIME__);
  QApplication the_app(argc, argv);
  MYQR_DEBUGOUT("the_app@" << (void*)&the_app << " argc:" << argc);
  QCommandLineParser cli_parser;
  cli_parser.addVersionOption();
  cli_parser.addHelpOption();
  QCommandLineOption debug_opt(QStringList() << "D" << "debug",
                               "show debugging messages");
  cli_parser.addOption(debug_opt);
  QCommandLineOption jsonrpc_opt{{"J", "jsonrpc"},
    "Use $JSONRPC.out and $JSONRPC.cmd fifos.\n"
    "Also sets the REFPERSYS_JSONRPC environment variable to $JSONRPC.", "JSONRPC"};
  cli_parser.addOption(jsonrpc_opt);
  QCommandLineOption geometry_opt{{"G", "geometry"},
    "Main window geometry is W*H,\n... e.g. --geometry 400x650", "WxH"};
  cli_parser.addOption(geometry_opt);
  QCommandLineOption refpersys_opt{"start-refpersys",
                                   "Start the given $REFPERSYS, defaulted to refpersys",
                                   "REFPERSYS", QString("refpersys")};
  cli_parser.addOption(refpersys_opt);
  cli_parser.process(the_app);
  MYQR_DEBUGOUT("main cli_parser@" << (void*)&cli_parser);
  QStringList args = cli_parser.positionalArguments();
  MYQR_DEBUGOUT("main args:" << args);
  myqr_app = &the_app;
  QString geomstr = cli_parser.value(geometry_opt);
  MYQR_DEBUGOUT("main geomstr:" << geomstr.toStdString());
  MYQR_DEBUGOUT("main debug:" << cli_parser.value(debug_opt).toStdString());
  MYQR_DEBUGOUT("main startrefpersys:" << cli_parser.value(refpersys_opt).toStdString()
                << (cli_parser.isSet(refpersys_opt)?" is set":" is not set"));
  myqr_create_windows(geomstr);
  if (cli_parser.isSet(jsonrpc_opt))
    myqr_have_jsonrpc(cli_parser.value(jsonrpc_opt).toStdString());
  if (cli_parser.isSet(refpersys_opt))
    {
      if (cli_parser.isSet(jsonrpc_opt))
        args += cli_parser.value(jsonrpc_opt);
      myqr_start_refpersys(cli_parser.value(refpersys_opt).toStdString(), args);
    };
  ///
  MYQR_DEBUGOUT("main jsonrpc_opt:" << cli_parser.value(jsonrpc_opt).toStdString()
                << " refpersys_opt: " <<  cli_parser.value(refpersys_opt).toStdString()
                << " jsonrpc:" << myqr_jsonrpc << " pid:" << myqr_refpersys_pid);
  if (cli_parser.isSet(jsonrpc_opt) && cli_parser.isSet(refpersys_opt))
    {
      Json::Value jargs(Json::objectValue);
      jargs["gitid"] = myqr_git_id;
      jargs["runtime_Qt"] = qVersion();
      jargs["compile_Qt"] = QTCORE_VERSION_STR;
      MYQR_DEBUGOUT("myqr_have_jsonrpc jargs is " << myqr_json2str(jargs));
      MYQR_DEBUGOUT("myqr_have_jsonrpc jargs=" << jargs << " call _VERSION" << " refpersys_pid:" << myqr_refpersys_pid);
      myqr_call_jsonrpc_to_refpersys("_VERSION", jargs,
                                     [&] (const Json::Value&jres)
      {
        MYQR_DEBUGOUT("myqr_have_jsonrpc _VERSION got " <<  myqr_json2str(jres)
                      << " with jargs " <<  myqr_json2str(jargs));
      });
    }
  MYQR_DEBUGOUT("main before exec");
  int execret = myqr_app->exec();
  MYQR_DEBUGOUT("main after exec execret=" << execret);
  if (myqr_refpersys_pid>0)
    {
      MYQR_DEBUGOUT("main kill with SIGTERM refpersys pid#" << myqr_refpersys_pid);
      errno = 0;
      if (kill(myqr_refpersys_pid, SIGTERM)<0)
        {
          MYQR_FATALOUT("failed to TERM refpersys pid#" << myqr_refpersys_pid
                        << ":" << strerror(errno));
        }
    }
  myqr_app = nullptr;
  MYQR_DEBUGOUT("main returns execret=" << execret);
  return execret;
} // end main



const char myqr_git_id[] = GITID;
char* myqr_progname;
char myqr_host_name[sizeof(myqr_host_name)];
std::string myqr_refpersys_topdir;
QApplication *myqr_app;
bool myqr_debug;
std::string myqr_jsonrpc;
Json::CharReaderBuilder myqr_jsoncpp_reader_builder;
Json::StreamWriterBuilder myqr_jsoncpp_writer_builder;
Json::CharReader* myqr_jsonrpc_reader;
int myqr_jsonrpc_cmd_fd = -1;
int myqr_jsonrpc_out_fd = -1;
std::recursive_mutex myqr_mtx_jsonrpc_cmd;
std::stringstream myqr_stream_jsonrpc_cmd;
QProcess*myqr_refpersys_process;
QSocketNotifier* myqr_notifier_jsonrpc_cmd;
QSocketNotifier* myqr_notifier_jsonrpc_out;
std::recursive_mutex myqr_mtx_jsonrpc_out;
std::deque<Json::Value> myqr_deque_jsonrpc_out;
std::stringstream myqr_stream_jsonrpc_out;
std::map<int,std::function<void(const Json::Value&res)>> myqr_jsonrpc_out_procmap;
pid_t myqr_refpersys_pid;


#include "_q6refpersys-moc.cc"

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make q6refpersys" ;;
 ** End: ;;
 **
 ****************/

/****************************************************************
 * file appli_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the Qt5 code related to the Qt5 application
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

#include <QCommandLineParser>

extern "C" const char rps_appli_gitid[];
const char rps_appli_gitid[]= RPS_GITID;

extern "C" const char rps_appli_date[];
const char rps_appli_date[]= __DATE__;

// these functions are here, because other files might not include <QString>
const Rps_String*
Rps_String::make(const QString&qs)
{
  return Rps_String::make(qs.toStdString());
} // end Rps_String::make(const QString&qs)

Rps_StringValue::Rps_StringValue(const QString&qs)
  : Rps_Value(Rps_String::make(qs), Rps_ValPtrTag{})
{
} // end of Rps_StringValue::Rps_StringValue(const QString&qs)

////////////////

static char rps_bufpath_homedir[384];

const char* rps_homedir(void)
{
  static std::mutex homedirmtx;
  std::lock_guard<std::mutex> gu(homedirmtx);
  if (RPS_UNLIKELY(rps_bufpath_homedir[0] == (char)0)) {
    const char*rpshome = getenv("REFPERSYS_HOME");
    const char*home = getenv("HOME");
    const char*path = rpshome?rpshome:home;
    if (!path)
      RPS_FATAL("no RefPerSys home ($REFPERSYS_HOME or $HOME)");
    char* rp = realpath(path, nullptr);
    if (!rp)
      RPS_FATAL("realpath failed on RefPerSys home %s - %m",
		       path);
    if (strlen(rp) >= sizeof(rps_bufpath_homedir) -1)
      RPS_FATAL("too long realpath %s on RefPerSys home %s", rp, path);
    strncpy(rps_bufpath_homedir, rp, sizeof(rps_bufpath_homedir) -1);
  }
  return rps_bufpath_homedir;
} // end rps_homedir

void
RpsQApplication::add_new_window(void)
{
  std::lock_guard gu(app_mutex);
  auto window = new RpsQWindow();
  window->setWindowTitle(QString("RefPerSys #%1").arg(app_windvec.size()));
  window->resize (640, 480); // TODO: get dimensions from $HOME/.RefPerSys
  window->show();
  app_windvec.emplace_back(window);
} // end of RpsQApplication::add_new_window

RpsQApplication::RpsQApplication(int &argc, char*argv[])
  : QApplication(argc, argv)
{
  setApplicationName("RefPerSys");
  setApplicationVersion(rps_lastgitcommit);
  app_windvec.reserve(16);
  app_windvec.push_back(nullptr); // we don't want a 0 index.
  add_new_window();
} // end of RpsQApplication::RpsQApplication

void
RpsQApplication::dump_state(QString dirpath)
{
  rps_dump_into(dirpath.toStdString());
} // end of RpsQApplication::dump_state


RpsQWindow* RpsQApplication::getWindowPtr(int ix)
{
  std::lock_guard gu(app_mutex);
  if (ix < 0)
    ix += app_windvec.size();
  if (ix <= 0 || ix > app_windvec.size())
    return nullptr;
  return app_windvec.at(ix).get();
}


void rps_run_application(int &argc, char **argv)
{
  bool batch = false;
  RPS_INFORM("rps_run_application: start of %s gitid %s host %s pid %d\n",
             argv[0], rps_gitid, rps_hostname(), (int)getpid());

  RpsQApplication app (argc, argv);
  std::string loadtopdir(rps_topdirectory);
  {
    QCommandLineParser argparser;
    argparser.setApplicationDescription("a REFlexive PERsistent SYStem");
    argparser.addHelpOption();
    argparser.addVersionOption();
    // refpersys home
    const QCommandLineOption rpshomeOption("refpersys-home",
                                           "RefPerSys homedir, default to $REFPERSYS_HOME or $HOME", "refpersys-home");
    argparser.addOption(rpshomeOption);
    
    // load directory
    const QCommandLineOption loadOption(QStringList() << "L" << "load",
                                        "The load directory", "load-dir");
    argparser.addOption(loadOption);
    // random oids
    const QCommandLineOption randoidOption("random-oid",
                                           "output some random oids", "nb-oids");
    argparser.addOption(randoidOption);
    // type information
    const QCommandLineOption typeOption("type-info", "Show type information.");
    argparser.addOption(typeOption);
    // batch flag
    const QCommandLineOption batchOption(QStringList() << "B" << "batch", "batch mode, without any windows");
    argparser.addOption(batchOption);
    //
    argparser.process(app);
    ///// --refpersys-home <dir>
    if (argparser.isSet(rpshomeOption))
      {
        const QString rhomqs = argparser.value(rpshomeOption);
	std::string rhompath = rhomqs.toStdString();
	struct stat rhomstat;
	memset (&rhomstat, 0, sizeof(rhomstat));
	if (stat(rhompath.c_str(), &rhomstat))
	  RPS_FATAL("failed to stat --refpersys-home %s: %m",
		    rhompath.c_str());
	if (!S_ISDIR(rhomstat.st_mode))
	  RPS_FATAL("given --refpersys-home %s is not a directory",
		    rhompath.c_str());
	if (rhomstat.st_mode & (S_IRUSR|S_IXUSR) !=  (S_IRUSR|S_IXUSR))
	  RPS_FATAL("given --refpersys-home %s is not user readable and executable",
		    rhompath.c_str());
	char*rhomrp = realpath(rhompath.c_str(), nullptr);
	if (!rhomrp)
	  RPS_FATAL("realpath failed on given --refpersys-home %s - %m",
		    rhompath.c_str());
	if (strlen(rhomrp) >= sizeof(rps_bufpath_homedir) -1)
	  RPS_FATAL("too long realpath %s on given --refpersys-home %s - %m",
		    rhomrp, rhompath.c_str());
	strncpy(rps_bufpath_homedir, rhomrp, sizeof(rps_bufpath_homedir) -1);
	free (rhomrp), rhomrp = nullptr;
      };
    RPS_INFORM("using %s as the RefPerSys home directory", rps_homedir());
    //// --load <dir>
    if (argparser.isSet(loadOption))
      {
        const QString loadpathqs = argparser.value(loadOption);
        loadtopdir = loadpathqs.toStdString();
      };
    /// --type-info
    if (argparser.isSet(typeOption))
      rps_print_types_info ();
    /// --batch or -B
    if (argparser.isSet(batchOption))
      batch = true;
    /// --random-oid <nbrand>
    if (argparser.isSet(randoidOption))
      {
        int nbrand = 5;
        const QString randoidqs = argparser.value(randoidOption);
        sscanf(randoidqs.toStdString().c_str(), "%d", &nbrand);
        if (nbrand <= 0) nbrand = 2;
        else if (nbrand > 100) nbrand = 100;
        RPS_INFORM("output of %d random objids generated on %.2f\n", nbrand,
                   rps_monotonic_real_time());
        printf("*    %-20s" "\t  %-19s" "   %-12s" "\t %-10s\n",
               " objid", "hi", "lo", "hash");
        printf("========================================================"
               "===========================\n");
        for (int ix = 0; ix<nbrand; ix++)
          {
            auto rid = Rps_Id::random();
            printf("! %22s" "\t  %19lld" " %12lld" "\t %10u\n",
                   rid.to_string().c_str(),
                   (long long) rid.hi(),
                   (long long) rid.lo(),
                   (unsigned) rid.hash());
          }
        printf("--------------------------------------------------------"
               "---------------------------\n");
        fflush(nullptr);
      }
  }
  rps_load_from (loadtopdir);
  if (!batch)
    (void) app.exec ();
} // end of rps_run_application

//////////////// moc generated file
#include "_qthead_qrps.inc.hh"

//////////////////////////////////////// end of file appli_qrps.cc

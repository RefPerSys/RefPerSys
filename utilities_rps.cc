/****************************************************************
 * file utilities_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has some utilities functions.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2024 The Reflective Persistent System Team
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
 *
 ******************************************************************************/

#include "refpersys.hh"

// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG gmp
//@@PKGCONFIG gmpxx
//@@PKGCONFIG glib-2.0
//@@PKGCONFIG cairo


#if RPS_WITH_FLTK
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Multi_Label.H>
#include <FL/Fl_Widget.H>
#if FLTK_API_VERSION >= 10400
#include <FL/Fl_Pack.H>
#include <FL/Fl_Flex.H>
#endif
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Box.H>
#endif // RPS_WITH_FLTK

#include "glib.h"
#include "libgccjit++.h"

extern "C" const char rps_utilities_gitid[];
const char rps_utilities_gitid[]= RPS_GITID;

extern "C" const char rps_utilities_date[];
const char rps_utilities_date[]= __DATE__;

extern "C" const char rps_utilities_shortgitid[];
const char rps_utilities_shortgitid[]= RPS_SHORTGITID;

extern "C" void rps_set_user_preferences(const char*path);

extern "C" char*rps_chdir_path_after_load;

static bool rps_flag_pref_help;

std::string rps_run_name;

/// https://lists.gnu.org/archive/html/lightning/2023-08/msg00004.html
/// see also file lightgen_rps.cc


static void rps_compute_program_invocation(int argc, char**argv);

// we may have a pair of FIFO to communicate with some external
// process (for graphical user interface), perhaps mini-edit-fltk on
// https://github.com/bstarynk/misc-basile/ ... The FIFO prefix is
// $FIFOPREFIX. The messages from the GUI user interface to RefPerSys
// are on $FIFOPREFIX.out; the messages from RefPerSys to that GUI
// user interface are on $FIFOPREFIX.cmd
static std::string rps_fifo_prefix;
char rps_bufpath_homedir[rps_path_byte_size];
char rps_debug_path[rps_path_byte_size];
static  struct rps_fifo_fdpair_st rps_fifo_pair;

static std::vector<std::string> rps_postponed_removed_files_vector;
static std::mutex rps_postponed_lock;

std::string rps_publisher_url_str;

static pthread_t rps_main_thread_handle;

extern "C" char*rps_pidfile_path;


int
rps_get_major_version(void)
{
  return RPS_MAJOR_VERSION_NUM;
} // end rps_get_major_version

int
rps_get_minor_version(void)
{
  return RPS_MINOR_VERSION_NUM;
} // end rps_get_minor_version

bool
rps_is_main_thread(void)
{
  return pthread_self() == rps_main_thread_handle;
} // end rps_is_main_thread


bool
rps_want_user_preferences_help(void)
{
  return rps_flag_pref_help;
} // end rps_want_user_preferences_help

static std::map<std::string,std::string> rps_dict_extra_arg;

void
rps_put_fifo_prefix(const char*pref)
{
  RPS_ASSERT(rps_is_main_thread());
  if (!rps_fifo_prefix.empty())
    return;
  if (pref && pref[0])
    rps_fifo_prefix = std::string(pref);
} // end rps_put_fifo_prefix

std::string
rps_get_fifo_prefix(void)
{
  return rps_fifo_prefix;
} // end rps_get_fifo_prefix

struct rps_fifo_fdpair_st
rps_get_gui_fifo_fds(void)
{
  if (rps_gui_pid)
    return rps_fifo_pair;
  else return {-1, -1};
} // end  rps_get_gui_fifo_fd

pid_t
rps_get_gui_pid(void)
{
  return rps_gui_pid;
} // end rps_get_gui_pid


static void
rps_remove_fifos(void)
{
  std::string cmdfifo = rps_fifo_prefix+".cmd";
  if (rps_is_fifo(cmdfifo))
    remove(cmdfifo.c_str());
  std::string outfifo = rps_fifo_prefix+".out";
  if (rps_is_fifo(outfifo))
    remove(outfifo.c_str());
} // end rps_remove_fifos

void
rps_do_create_fifos_from_prefix(void)
{
  int cmdfd= -1;
  int outfd= -1;
  bool rmatex = false;
  char cwdbuf[rps_path_byte_size+4];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  RPS_ASSERT(rps_is_main_thread());
  if (!getcwd(cwdbuf, rps_path_byte_size))
    strcpy(cwdbuf, "./");
  std::string cmdfifo = rps_fifo_prefix+".cmd";
  std::string outfifo = rps_fifo_prefix+".out";
  RPS_DEBUG_LOG(REPL, "rps_do_create_fifos_from_prefix " << rps_fifo_prefix
                << " in " << cwdbuf << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_do_create_fifos_from_prefix"));
  if (!rps_is_fifo(cmdfifo))
    {
      if (mkfifo(cmdfifo.c_str(), 0660)<0)
        RPS_FATALOUT("failed to create command FIFO " << cmdfifo << ":" << strerror(errno));
      RPS_INFORMOUT("created command FIFO " << cmdfifo
                    << " cwd:" << cwdbuf << " pid "
                    << (int)getpid() << " on " << rps_hostname()
                    << " git " << rps_shortgitid);
      rmatex = true;
    }
  RPS_DEBUG_LOG(REPL, "rps_do_create_fifos_from_prefix opening cmdfifo " << cmdfifo);
  cmdfd = open(cmdfifo.c_str(), 0660 | O_CLOEXEC | O_NONBLOCK);
  if (cmdfd<0)
    RPS_FATALOUT("failed to open command FIFO " << cmdfifo << ":" << strerror(errno));
  RPS_DEBUG_LOG(REPL, "rps_do_create_fifos_from_prefix cmdfd#" << cmdfd
                << " cmdfifo:" << cmdfifo);
  if (!rps_is_fifo(outfifo))
    {
      if (mkfifo(outfifo.c_str(), 0660)<0)
        RPS_FATALOUT("failed to create output FIFO " << outfifo << ":" << strerror(errno));
      RPS_INFORMOUT("created output FIFO " << outfifo
                    << " cwd:" << cwdbuf << " pid "
                    << (int)getpid() << " on " << rps_hostname()
                    << " git " << rps_shortgitid);
      rmatex = true;
    }
  RPS_DEBUG_LOG(REPL, "rps_do_create_fifos_from_prefix opening outfifo " << outfifo);
  outfd = open(outfifo.c_str(), 0440 | O_CLOEXEC | O_NONBLOCK);
  if (outfd<0)
    RPS_FATALOUT("failed to open output FIFO " << outfifo << ":" << strerror(errno));
  RPS_DEBUG_LOG(REPL, "rps_do_create_fifos_from_prefix outfd#" << outfd
                << " outfifo:" << outfifo);
  if (rmatex)
    atexit(rps_remove_fifos);
  rps_fifo_pair.fifo_ui_wcmd = cmdfd;
  rps_fifo_pair.fifo_ui_rout = outfd;
  RPS_INFORMOUT("RefPerSys did create (in pid " << (int)getpid()
                << ") command and output FIFOs to communicate with GUI" << std::endl
                << "… using for writing commands to GUI " << cmdfifo << " fd#" << cmdfd
                << std::endl
                << "… and for reading JSON output from GUI " << outfifo << " fd#" << outfd
                << "… git " << rps_shortgitid);
  RPS_POSSIBLE_BREAKPOINT();
} // end rps_do_create_fifos_from_prefix


const char*
rps_homedir(void)
{
  static std::mutex homedirmtx;
  std::lock_guard<std::mutex> gu(homedirmtx);
  if (RPS_UNLIKELY(rps_bufpath_homedir[0] == (char)0))
    {
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

const std::string&
rps_get_loaddir(void)
{
  return rps_my_load_dir;
} // end rps_get_loaddir


const char*
rps_hostname(void)
{
  static char hnambuf[80];
  if (RPS_UNLIKELY(!hnambuf[0]))
    gethostname(hnambuf, sizeof(hnambuf)-1);
  return hnambuf;
} // end rps_hostname


const char*
rps_get_extra_arg(const char*name)
{
  if (!name) return nullptr;
  bool is_good_name=isalpha(name[0]);
  for (const char*pc = name; is_good_name && *pc; pc++)
    is_good_name = isalnum(*pc) || *pc == '_';
  if (RPS_UNLIKELY(!is_good_name))
    return nullptr;
  std::string goodstr{name};
  auto it = rps_dict_extra_arg.find(goodstr);
  if (it == rps_dict_extra_arg.end())
    return nullptr;
  return it->second.c_str();
} // end rps_get_extra_arg

void
rps_emit_gplv3_copyright_notice_AT(std::ostream&outs, //
                                   const char*fil, int lin, const char*fromfunc,//
                                   std::string path, std::string linprefix, std::string linsuffix, std::string owner, std::string reason)
{
  outs << linprefix << "SPDX-License-Identifier: GPL-3.0-or-later"
       << linsuffix << std::endl;
  outs << linprefix
       << "GENERATED [GPLv3+] file " << path  << " / DO NOT EDIT!"
       << linsuffix << std::endl;
  outs << linprefix << "generating-git " << rps_shortgitid << linsuffix << std::endl;
  if (reason.length()>0)
    {
      outs << linprefix << "~" << reason << linsuffix << std::endl;
    };
  outs << linprefix
       << "This " << path << " file is generated by ..."
       << linsuffix << std::endl << linprefix << " the RefPerSys "
       << rps_get_major_version() << "." << rps_get_minor_version()
       << linsuffix << std::endl;
  outs << linprefix << "open source software.  See refpersys.org and" << linsuffix << std::endl;
  outs << linprefix << "contact team@refpersys.org"
       << linsuffix << std::endl;
  outs << linprefix
       << "This file is part of the Reflective Persistent System."
       << linsuffix << std::endl;
  {
    time_t nowtime = time(nullptr);
    struct tm nowtm = {};
    localtime_r(&nowtime, &nowtm);
    outs << linprefix
         << "Copyright (C) "
         << RPS_INITIAL_COPYRIGHT_YEAR
         << " - "
         << (nowtm.tm_year + 1900) << " "
         << ((owner.empty()) ? "The Reflective Persistent Team" : owner.c_str())
         << linsuffix << std::endl;
    outs << linprefix
         << " see refpersys.org and contact team@refpersys.org for more."
         << linsuffix << std::endl;
  }
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This program is free software: you can redistribute it and/or modify"
       << linsuffix << std::endl;
  outs << linprefix << "it under the terms of the GNU General Public License as published by"
       << linsuffix << std::endl;
  outs << linprefix << "the Free Software Foundation, either version 3 of the License, or"
       << linsuffix << std::endl;
  outs << linprefix << "(at your option) any later version."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This program is distributed in the hope that it will be useful,"
       << linsuffix << std::endl;
  outs << linprefix << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
       << linsuffix << std::endl;
  outs << linprefix << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
       << linsuffix << std::endl;
  outs << linprefix << "GNU General Public License for more details."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "You should have received a copy of the GNU "
       "General Public License"
       << linsuffix << std::endl;
  outs << linprefix << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
       << linsuffix << std::endl;
  outs << linprefix << "generated from git " << rps_shortgitid
       << " branch " << rps_gitbranch << linsuffix << std::endl;
  if (fil && lin>0 && fromfunc)
    {
      outs << linprefix << " emitted from " << fil << ":" << lin << linsuffix << std::endl;
      outs << linprefix << " by " << fromfunc << linsuffix << std::endl;
    }
} // end rps_emit_gplv3_copyright_notice_AT


void
rps_emit_lgplv3_copyright_notice_AT(std::ostream&outs,//
                                    const char*fil, int lin, const char*fromfunc, //
                                    std::string path, std::string linprefix, std::string linsuffix, std::string owner, std::string reason)
{
  outs << linprefix << "SPDX-License-Identifier: LGPL-3.0-or-later"
       << linsuffix << std::endl;
  outs << linprefix
       << "GENERATED [LGPLv3+] file " << path  << " / DO NOT EDIT!"
       << linsuffix << std::endl;
  outs << linprefix << "generating-git " << rps_shortgitid << linsuffix << std::endl;
  if (reason.length()>0)
    {
      outs << linprefix << "~" << reason << linsuffix << std::endl;
    };
  outs << linprefix
       << "This " << path << " file is generated by ..." << linsuffix << std::endl
       << linprefix << " the RefPerSys "
       << rps_get_major_version() << "." << rps_get_minor_version()
       <<  linsuffix << std::endl;
  outs << linprefix << "open source software.  See refpersys.org and contact"
       << linsuffix << std::endl;
  outs << linprefix << " team@refpersys.org ..."
       << linsuffix << std::endl;
  {
    time_t nowtime = time(nullptr);
    struct tm nowtm = {};
    localtime_r(&nowtime, &nowtm);
    outs << linprefix
         << "Copyright (C) "
         << RPS_INITIAL_COPYRIGHT_YEAR
         << " - "
         << (nowtm.tm_year + 1900)
         << ((owner.empty()) ? "The Reflective Persistent Team" : owner.c_str());
    outs << linsuffix << std::endl;
  }
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This file is free software: you can redistribute it and/or modify"
       << linsuffix << std::endl;
  outs << linprefix << "it under the terms of the GNU Lesser General Public License as"
       << linsuffix << std::endl;
  outs << linprefix << "published by the Free Software Foundation, either version 3 of the"
       << linsuffix << std::endl;
  outs << linprefix << "License, or (at your option) any later version."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "This file is distributed in the hope that it will be useful,"
       << linsuffix << std::endl;
  outs << linprefix << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
       << linsuffix << std::endl;
  outs << linprefix << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
       << linsuffix << std::endl;
  outs << linprefix << "GNU Lesser General Public License for more details."
       << linsuffix << std::endl;
  outs << linprefix << "_"
       << linsuffix << std::endl;
  outs << linprefix << "You should have received a copy of the GNU "
       "Lesser General Public License"
       << linsuffix << std::endl;
  outs << linprefix << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
       << linsuffix << std::endl;
  outs << linprefix << "generated from git " << rps_shortgitid
       << " branch " << rps_gitbranch << linsuffix << std::endl;
  if (fil && lin>0 && fromfunc)
    {
      outs << linprefix
           << " emitted from " << fil << ":" << lin
           << linsuffix << std::endl;
      outs << linprefix
           << " by " << fromfunc
           << linsuffix << std::endl;
    };
} // end rps_emit_lgplv3_copyright_notice_AT


////////////////
void
rps_print_types_info(void)
{
#define TYPEFMT_rps "%-58s:"
  printf(TYPEFMT_rps "   size  align   (bytes)\n", "**TYPE**");
  /////
#define EXPLAIN_TYPE(Ty) printf(TYPEFMT_rps " %5d %5d\n", #Ty,  \
        (int)sizeof(Ty), (int)alignof(Ty))
  /////
#define EXPLAIN_TYPE2(Ty1,Ty2) printf(TYPEFMT_rps " %5d %5d\n", \
              #Ty1 "," #Ty2,                                    \
              (int)sizeof(Ty1,Ty2),                             \
              (int)alignof(Ty1,Ty2))
  /////
#define EXPLAIN_TYPE3(Ty1,Ty2,Ty3)              \
  printf(TYPEFMT_rps " %5d %5d\n",              \
   #Ty1 "," #Ty2 ",\n"                          \
   "                     "#Ty3,                 \
   (int)sizeof(Ty1,Ty2,Ty3),                    \
   (int)alignof(Ty1,Ty2,Ty3))
  /////
#define EXPLAIN_TYPE4(Ty1,Ty2,Ty3,Ty4)          \
  printf(TYPEFMT_rps " %5d %5d\n",              \
   #Ty1 "," #Ty2 ",\n                " #Ty3     \
   "," #Ty4,                                    \
   (int)sizeof(Ty1,Ty2,Ty3,Ty4),                \
   (int)alignof(Ty1,Ty2,Ty3,Ty4))
  /////
  EXPLAIN_TYPE(int);
  EXPLAIN_TYPE(double);
  EXPLAIN_TYPE(char);
  EXPLAIN_TYPE(bool);
  EXPLAIN_TYPE(void*);
  EXPLAIN_TYPE(time_t);
  EXPLAIN_TYPE(pid_t);
  //opaque in glib: EXPLAIN_TYPE(GMainLoop);
  //opaque in glib: EXPLAIN_TYPE(GMainContext);
  EXPLAIN_TYPE(std::mutex);
  EXPLAIN_TYPE(std::shared_mutex);
  EXPLAIN_TYPE(std::recursive_mutex);
  EXPLAIN_TYPE(std::atomic<void*>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::recursive_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::string);
  EXPLAIN_TYPE(std::ostream);
  EXPLAIN_TYPE(std::ostringstream);
  EXPLAIN_TYPE(FILE);
  EXPLAIN_TYPE(std::vector<std::string>);
  EXPLAIN_TYPE(std::set<std::string>);
  EXPLAIN_TYPE2(std::map<Rps_ObjectRef, Rps_Value>);
  EXPLAIN_TYPE2(std::unordered_map<std::string, Rps_ObjectRef*>);
  printf("#### %s:%d\n", __FILE__, __LINE__);
  EXPLAIN_TYPE3(std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher>);
  EXPLAIN_TYPE3(std::variant<unsigned, std::function<Rps_Value(void*)>,
                std::function<int(void*,Rps_ObjectRef)>>);
  EXPLAIN_TYPE(Rps_Backtracer);
  EXPLAIN_TYPE(Rps_ClosureValue);
  EXPLAIN_TYPE(Rps_ClosureZone);
  EXPLAIN_TYPE(Rps_Double);
  EXPLAIN_TYPE(Rps_DoubleValue);
  EXPLAIN_TYPE(Rps_GarbageCollector);
  EXPLAIN_TYPE(Rps_HashInt);
  EXPLAIN_TYPE(Rps_Id);
  EXPLAIN_TYPE(Rps_ObjectRef);
  EXPLAIN_TYPE(Rps_ObjectValue);
  EXPLAIN_TYPE(Rps_ObjectZone);
  EXPLAIN_TYPE(Rps_Payload);
  EXPLAIN_TYPE(Rps_PayloadClassInfo);
  EXPLAIN_TYPE(Rps_PayloadSetOb);
  EXPLAIN_TYPE(Rps_PayloadVectOb);
  EXPLAIN_TYPE(Rps_QuasiZone);
  EXPLAIN_TYPE(Rps_SetOb);
  EXPLAIN_TYPE(Rps_SetValue);
  EXPLAIN_TYPE(Rps_String);
  EXPLAIN_TYPE(Rps_StringValue);
  EXPLAIN_TYPE(Rps_TupleOb);
  EXPLAIN_TYPE(Rps_TupleValue);
  EXPLAIN_TYPE(Rps_Type);
  EXPLAIN_TYPE(Rps_Value);
  EXPLAIN_TYPE(Rps_ZoneValue);
  ////
#if RPS_WITH_FLTK
  printf("\n\n===== FLTK widgets from %s:%d ====\n", __FILE__, __LINE__);
  EXPLAIN_TYPE(Fl_Box);
#if FLTK_API_VERSION >= 10400
  EXPLAIN_TYPE(Fl_Flex);
  EXPLAIN_TYPE(Fl_Pack);
#endif
  EXPLAIN_TYPE(Fl_Menu_Bar);
  EXPLAIN_TYPE(Fl_Text_Buffer);
  EXPLAIN_TYPE(Fl_Text_Editor);
  EXPLAIN_TYPE(Fl_Widget);
  EXPLAIN_TYPE(Fl_Window);
#endif /*RPS_WITH_FLTK*/
  ////
#undef EXPLAIN_TYPE4
#undef EXPLAIN_TYPE3
#undef EXPLAIN_TYPE
#undef TYPEFMT_rps
  putchar('\n');
  fflush(nullptr);
  std::cout << "@@°°@@ The tagged integer one hundred is "
            << Rps_Value::make_tagged_int(100)
            << std::endl
            << "... and the tagged integer minus one billion is "
            <<  Rps_Value::make_tagged_int(-1000000000)
            << " !!! " << std::endl;
} // end rps_print_types_info


static void
rps_show_version_handwritten_cplusplus_files(void)
{
  //// show gitid and date of individual handwritten *cc files, using dlsym
  //// since every file like utilities_rps.cc has rps_utilities_gitid and rps_utilities_date
  for (const char*const*curfileptr = rps_files;
       curfileptr && *curfileptr; curfileptr++)
    {
      char curbase[64];
      memset (curbase, 0, sizeof(curbase));
      int endpos = -1;
      const char*curfile = *curfileptr;
      if (!curfile)
        break;
      if (!isalpha(curfile[0]))
        continue;
      if (strchr(curfile, '/'))
        continue;
      if ((sscanf(curfile, "%60[a-zA-Z]_rps.cc%n", curbase, &endpos))<1
          || endpos<2 || curfile[endpos]!=(char)0)
        continue;
      const char* symgit = nullptr;
      const char* symdat = nullptr;
      const char* symshortgit = nullptr;
      {
        char cursymgit[80];
        char cursymdat[80];
        char cursymshortgit[80];
        memset (cursymgit, 0, sizeof(cursymgit));
        memset (cursymdat, 0, sizeof(cursymdat));
        memset (cursymshortgit, 0, sizeof(cursymshortgit));
        snprintf (cursymgit, sizeof(cursymgit), "rps_%s_gitid", curbase);
        snprintf (cursymshortgit, sizeof(cursymgit), "rps_%s_shortgitid", curbase);
        snprintf (cursymdat, sizeof(cursymdat), "rps_%s_date", curbase);
        symgit = (const char*)dlsym(rps_proghdl, cursymgit);
        if (!symgit || !isalnum(symgit[0]))
          continue;
        symshortgit = (const char*)dlsym(rps_proghdl, cursymshortgit);
        if (!symshortgit || !isalnum(symgit[0]))
          continue;
        symdat = (const char*)dlsym(rps_proghdl, cursymdat);
        if (!symdat || !isalnum(symdat[0]))
          continue;
        if (symgit && symshortgit
            && strncmp(symgit, symshortgit, sizeof(rps_utilities_shortgitid)-2))
          {
            /// this should not happen and is likely a bug in C++ files or build procedure
            RPS_POSSIBLE_BREAKPOINT();
            RPS_WARNOUT("perhaps corrupted " << curfile << " in topdir " << rps_topdirectory
                        << " with " << cursymgit << "=" << symgit
                        << " and " << cursymshortgit << "=" << symshortgit);
          }
      };
      if (symgit && isalnum(symgit[0]) && symdat && isalnum(symdat[0]))
        {
          char msgbuf[80];
          memset (msgbuf, 0, sizeof(msgbuf));
          if (snprintf(msgbuf, sizeof(msgbuf)-1, "%-20s git %.15s built %s",
                       curfile, symgit, symdat)>0)
            std::cout << "#" << msgbuf << std::endl;
        }
    }
} // end rps_show_version_handwritten_cplusplus_files

void
rps_show_version(void)
{
  int nbfiles=0;
  int nbsubdirs=0;
  for (auto pfiles=rps_files; *pfiles; pfiles++)
    nbfiles++;
  for (auto psubdirs=rps_subdirectories; *psubdirs; psubdirs++)
    nbsubdirs++;
  char exepath[256];
  memset (exepath, 0, sizeof(exepath));
  static char realexepath[PATH_MAX];
  memset (realexepath, 0, sizeof(realexepath));
  {
    ssize_t sz = readlink("/proc/self/exe", exepath, sizeof(exepath));
    RPS_ASSERT(sz>0 && exepath[0]);
  }
  {
    char*rp= realpath(exepath, realexepath);
    RPS_ASSERT(rp != nullptr);
  }
  std::cout << "RefPerSys "<< rps_get_major_version() << "."
            << rps_get_minor_version() //
            << ", an open source Artificial Intelligence system" << std::endl;
  std::cout << "\t  symbolic inference engine - work in progress..." << std::endl;
  std::cout << "version information:\n"
            << " major version: " << RPS_MAJOR_VERSION_NUM << std::endl
            << " minor version: " << RPS_MINOR_VERSION_NUM << std::endl
            << " program name: " << rps_progname << std::endl
            << " build time: " << rps_timestamp << std::endl
            << " top directory: " << rps_topdirectory << std::endl
            << " gitid: " << rps_gitid << std::endl
            << " short-gitid: " << rps_shortgitid << std::endl
            << " gitbranch: " << rps_gitbranch << std::endl
            << " last git tag: " << rps_lastgittag << std::endl
            << " last git commit: " << rps_lastgitcommit << std::endl
            << " md5sum of " << nbfiles << " source files: " << rps_md5sum << std::endl
            << " with " << nbsubdirs << " subdirectories." << std::endl
            << " GNU glibc: " << gnu_get_libc_version() << std::endl
            << " Glib: " << glib_major_version
            << "." << glib_minor_version
            << "." << glib_micro_version << std::endl
            << " executable: " << exepath;
  if (strcmp(exepath, realexepath))
    std::cout <<  " really " << realexepath;
#if RPS_WITH_FLTK
  std::cout << " FLTK (see fltk.org) ABI version:" << rps_fltk_get_abi_version()
            << std::endl;
  std::cout << " FLTK API version:" << rps_fltk_get_api_version()
            << std::endl;
#endif
  std::cout << " GCCJIT version:" << gcc_jit_version_major()
            << "." << gcc_jit_version_minor() << "." << gcc_jit_version_patchlevel() << std::endl;
  std::cout << std::endl
            /* TODO: near commit 191d55e1b31c, march 2023; decide
               which parser generator to really use... and drop the
               other one.  Non technical considerations,
               e.g. licensing, is important to some partners... */
            << " Gnu multi-precision library version: " << gmp_version
            << std::endl
            << " default GUI script: " << rps_gui_script_executable << std::endl
            << " Read Eval Print Loop: " << rps_repl_version() << std::endl
#if RPS_USE_CURL
            << " libCURL for web client: " << rps_curl_version() << std::endl
#endif /*RPS_USE_CURL*/
            ;
  ////
  RPS_POSSIBLE_BREAKPOINT();
  ////
  std::cout << " JSONCPP: " << JSONCPP_VERSION_STRING << std::endl
            << " GPP preprocessor command: " << rps_gpp_preprocessor_command << std::endl
            << " GPP preprocessor path: " << rps_gpp_preprocessor_realpath << std::endl
            << " GPP preprocessor version: " << rps_gpp_preprocessor_version << std::endl
            << " made with: " << rps_gnumakefile << std::endl
            << " running on: " << rps_hostname() << std::endl
            << " /proc/version:" << std::endl
            << " " << rps_get_proc_version() << std::endl
            << "This executable was built by "
            << rps_building_user_name
            << " of email " << rps_building_user_email << std::endl
            << "See refpersys.org and code on github.com/RefPerSys/RefPerSys"
            << std::endl;
  /////
  rps_show_version_handwritten_cplusplus_files();
  /////
  {
    char cwdbuf[rps_path_byte_size+4];
    memset (cwdbuf, 0, sizeof(cwdbuf));
    if (getcwd(cwdbuf, rps_path_byte_size))
      std::cout << " in: " << cwdbuf;
  };
  std::cout << std::endl << " C++ compiler: " << rps_cxx_compiler_version << std::endl
            << " free software license: GPLv3+, see https://gnu.org/licenses/gpl.html" << std::endl
            << "+++++ there is no WARRANTY, to the extent permitted by law ++++" << std::endl
            << "***** see also refpersys.org *****" << std::endl
            << "and github.com/RefPerSys/RefPerSys commit "
            << rps_shortgitid
            << std::endl << std::endl;
} // end rps_show_version

/// In a format string passed to strftime, replace .__ with the
/// centisecond fractional part of the time. See of course
/// http://man7.org/linux/man-pages/man3/strftime.3.html etc... Notice
/// that debugging facilities use that function, e.g. it gets called
/// from rps_debug_printf_at used by RPS_DEBUG_LOG and RPS_DEBUG_PRINTF
/// macros.
char *
rps_strftime_centiseconds(char *bfr, size_t len, const char *fmt, double tm)
{
  if (!bfr || !fmt || len<4)
    return nullptr;
  //
  memset (bfr, 0, len);
  //
  struct tm tmstruct;
  memset(&tmstruct, 0, sizeof (tmstruct));
  //
  time_t time = static_cast<time_t>(tm);
  strftime(bfr, len, fmt, localtime_r(&time, &tmstruct));
  //
  char *dotdunder = strstr(bfr, ".__");
  if (dotdunder)
    {
      double intpart = 0.0;
      double fraction = modf(tm, &intpart);

      char minibuf[16];
      memset(minibuf, 0, sizeof (minibuf));
      assert(fraction >= 0.0 && fraction < 1.0);

      snprintf(minibuf, sizeof (minibuf), "%.02f", fraction);
      minibuf[4] = (char)0;
      const char* dotminib = strchr(minibuf, '.');
      if (dotminib && dotminib<minibuf+sizeof(minibuf)-4)
        {
          strncpy(dotdunder, dotminib, 3);
        }
    }

  return bfr;
} // end rps_strftime_centiseconds



/// This rps_extend_env is called early from main.  It is extending
/// the Unix environment.
/// Try running ./refpersys "--run-after-load=env|grep REFPERSYS" --batch
void
rps_extend_env(void)
{
  static std::atomic<bool> extended;
  if (extended) return;
  RPS_ASSERT(rps_is_main_thread());
  extended = true;
  static char pidenv[64];
  snprintf(pidenv, sizeof(pidenv), "REFPERSYS_PID=%d", (int)getpid());
  putenv(pidenv);     // e.g. REFPERSYS_PID=2345
  static char shortgitenv[64];
  snprintf(shortgitenv, sizeof(shortgitenv), "REFPERSYS_SHORTGITID=%s", rps_shortgitid);
  putenv(shortgitenv);  // e.g. REFPERSYS_SHORTGITID=49466057bf7d+
  static char gitenv[128];
  snprintf(gitenv, sizeof(gitenv), "REFPERSYS_GITID=%s", rps_gitid);
  putenv(gitenv);  // e.g. REFPERSYS_GITID=494...90+ with 40 hexdigit
  static char topdirenv[384];
  snprintf(topdirenv, sizeof(topdirenv), "REFPERSYS_TOPDIR=%s", rps_topdirectory);
  putenv(topdirenv); // e.g. REFPERSYS_TOPDIR=$HOME/work/RefPerSys/
  if (!rps_fifo_prefix.empty())
    {
      static char fifoenv[256];
      snprintf(fifoenv, sizeof(fifoenv), "REFPERSYS_FIFO_PREFIX=%s", rps_fifo_prefix.c_str());
      putenv(fifoenv); // e.g. REFPERSYS_FIFO_PREFIX=$HOME/tmp/rpsfifo
    };
  if (!rps_run_name.empty())
    {
      static char runamenv[256];
      snprintf(runamenv, sizeof(runamenv), "REFPERSYS_RUN_NAME=%s", rps_run_name.c_str());
      putenv(runamenv);   // e.g. REFPERSYS_RUN_NAME=testflk3.x
    };
} // end rps_extend_env




void
rps_check_mtime_files(void)
{
  struct stat selfstat = {};
  if (stat("/proc/self/exe", &selfstat))
    RPS_FATAL("stat /proc/self/exe: %m");
  char exebuf[128];
  memset (exebuf, 0, sizeof(exebuf));
  if (readlink("/proc/self/exe", exebuf, sizeof(exebuf)-1)<0)
    RPS_FATAL("readlink /proc/self/exe: %m");
  for (const char*const*curpath = rps_files; *curpath; curpath++)
    {
      int lencurpath = strlen(*curpath);
      if (lencurpath < 6 || strstr(*curpath, "attic/"))
        continue;
      std::string curpathstr(*curpath);
      /// Files under webroot could be sent to browser, so we don't
      /// care about them being newer than executable....
      auto wrp = curpathstr.find("webroot/");
      if (wrp < curpathstr.size())
        continue;
      std::string curfullpathstr= std::string{rps_topdirectory} + "/" + curpathstr;
      struct stat curstat = {};
      if (stat(curfullpathstr.c_str(), &curstat))
        {
          RPS_WARNOUT("rps_check_mtime_files: stat " << curfullpathstr << " failed: " << strerror(errno));
          continue;
        };
      if (curstat.st_mtime > (time_t) rps_timelong)
        RPS_WARNOUT("rps_check_mtime_files: " << curfullpathstr.c_str()
                    << " is younger by "
                    << (curstat.st_mtime - (time_t) rps_timelong)
                    << " seconds than current executable " << exebuf
                    << ", so consider rebuilding with make");
    }
  char makecmd [128];
  memset (makecmd, 0, sizeof(makecmd));
  if (snprintf(makecmd, sizeof(makecmd), "make -t -C %s -q objects", rps_topdirectory) < (int)sizeof(makecmd)-1)
    {
      int bad = system(makecmd);
      if (bad)
        RPS_WARNOUT("rps_check_mtime_files: " << makecmd
                    << " failed with status# " << bad);
      else
        RPS_INFORMOUT("rps_check_mtime_files: did " << std::string(makecmd) << " successfully");
    }
  else
    RPS_FATAL("rps_check_mtime_files failed to construct makecmd in %s: %m",
              rps_topdirectory);
} // end rps_check_mtime_files




////////////////////////////////////////////////////////////////

static double rps_start_monotonic_time;
static double rps_start_wallclock_real_time;



/// rps_early_initialization is called by rps_parse_program_arguments which is called early from main.
static void
rps_early_initialization(int argc, char** argv)
{
  char*inside_emacs = getenv("INSIDE_EMACS"); /// GNU emacs is setting this
  rps_argc = argc;
  rps_argv = argv;
  rps_progname = argv[0];
  /// dlopen to self
  rps_proghdl = dlopen(nullptr, RTLD_NOW|RTLD_GLOBAL);
  if (!rps_proghdl)
    {
      char *err = dlerror();
      fprintf(stderr, "%s failed to dlopen whole program (%s)\n", rps_progname,
              err);
      syslog(LOG_ERR, "%s failed to dlopen whole program (%s)\n", rps_progname,
             err);
      exit(EXIT_FAILURE);
    };
  if (argc == 2 && !strcmp(argv[1], "--full-git"))   /// see also rps_parse1opt
    {
      printf("%s\n", rps_gitid);
      fflush(nullptr);
      exit(EXIT_SUCCESS);
    }
  else if (argc == 2 && !strcmp(argv[1], "--short-git"))  /// see also rps_parse1opt
    {
      printf("%s\n", rps_shortgitid);
      fflush(nullptr);
      exit(EXIT_SUCCESS);
    }
  rps_start_monotonic_time = rps_monotonic_real_time();
  rps_start_wallclock_real_time = rps_wallclock_real_time();
  if (!inside_emacs)
    {
      rps_stderr_istty = isatty(STDERR_FILENO);
      rps_stdout_istty = isatty(STDOUT_FILENO);
      std::cout << "RefPerSys outside of EMACS git " << RPS_SHORTGITID
                << " "<< (rps_stderr_istty?"tty stderr":"plain stderr")
                << " "<< (rps_stdout_istty?"tty stdout":"plain stdout")
                << " " << __FILE__ << ":" << __LINE__ << std::endl;
    }
  else   ////// called inside emacs
    {
      rps_stderr_istty = false; // INSIDE_EMACS
      rps_stdout_istty = false; // INSIDE_EMACS
      std::cout << "since INSIDE_EMACS is " << inside_emacs
                << " at " __FILE__ ":" << __LINE__ << std::endl
                << " disabling ANSI escapes from " << __FUNCTION__
                << " git " << RPS_SHORTGITID << std::endl;
    };
  if (uname (&rps_utsname))
    {
      fprintf(stderr, "%s: pid %d on %s failed to uname (%s:%d git %s): %s\n", rps_progname,
              (int) getpid(), rps_hostname(), __FILE__, __LINE__, RPS_SHORTGITID,
              strerror(errno));
      syslog(LOG_ERR,  "%s: pid %d on %s failed to uname (%s:%d git %s): %s\n", rps_progname,
             (int) getpid(), rps_hostname(), __FILE__, __LINE__, RPS_SHORTGITID,
             strerror(errno));
      exit(EXIT_FAILURE);
    };
  // compute the program invocation string
  rps_compute_program_invocation(argc, argv);
  rps_main_thread_handle = pthread_self();
  {
    char cwdbuf[rps_path_byte_size];
    memset (cwdbuf, 0, sizeof(cwdbuf));
    char tmbfr[64];   // the time buffer string
    memset(tmbfr, 0, sizeof (tmbfr));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)) || cwdbuf[0] == (char)0)
      strcpy(cwdbuf, "./");
    rps_now_strftime_centiseconds_nolen(tmbfr, "%Y, %b, %D %H:%M:%S.__ %Z");
    std::cout << std::endl << "** STARTING RefPerSys git " << rps_shortgitid << " on " << rps_hostname() << " pid#" << getpid()
              << " in " << cwdbuf << " at " << tmbfr << std::endl;
  }
  /// handle early a debug flag request
  if (argc > 1
      && !strncmp(argv[1], "--debug=", strlen("--debug=")))
    {
      rps_add_debug_cstr(argv[1]+strlen("--debug="));
    }
  else if (argc > 1 && argv[1][0]=='-' && argv[1][1]==RPSPROGOPT_DEBUG)
    {
      rps_add_debug_cstr(argv[1]+2);
    };
  // also use REFPERSYS_DEBUG
  {
    const char*debugenv = getenv("REFPERSYS_DEBUG");
    if (debugenv)
      rps_add_debug_cstr(debugenv);
  }
  // For weird reasons, the program arguments are parsed more than
  // once... We don't care that much in practice...
  RPS_ASSERT(argc>0);
  // we forcibly set the REFPERSYS_PID environment variable
  {
    static char envpid[32];
    if (snprintf(envpid, sizeof(envpid), "REFPERSYS_PID=%d", (int)getpid()) < 1)
      RPS_FATAL("failed to snprintf buffer for REFPERSYS_PID: %m");
    if (putenv(envpid))
      RPS_FATAL("failed to putenv %s %m", envpid);
  }
  /// disable ASLR programmatically if --no-aslr is passed ; this
  /// should ease low-level debugging with GDB
  /// https://en.wikipedia.org/wiki/Address_space_layout_randomization
  /// see https://askubuntu.com/a/507954/64680
  rps_disable_aslr = false;
  {
    for (int ix=1; ix<argc; ix++)
      {
        if (!strcmp(argv[ix], "--no-aslr"))
          rps_disable_aslr = true;
        else if (!strcmp(argv[ix], "-B") || !strcmp(argv[ix], "--batch"))
          rps_batch = true;
        else if (!strcmp(argv[ix], "--without-terminal"))
          rps_without_terminal_escape = true;
        else if (!strcmp(argv[ix], "--daemon"))
          {
            rps_daemonized = true;
            rps_syslog_enabled = true;
          }
        else if (!strcmp(argv[ix], "--syslog"))
          rps_syslog_enabled = true;
      }
    if (rps_disable_aslr)
      {
        if (personality(ADDR_NO_RANDOMIZE) == -1)
          RPS_FATAL("%s failed to disable ASLR: %m", rps_progname);
        else
          RPS_INFORM("%s disabled ASLR (git %s).", rps_progname, rps_gitid);
      }
  }
  Rps_Agenda::initialize();
  unsetenv("LANG");
  unsetenv("LC_ADDRESS");
  unsetenv("LC_ALL");
  unsetenv("LC_IDENTIFICATION");
  unsetenv("LC_MEASUREMENT");
  unsetenv("LC_MONETARY");
  unsetenv("LC_NAME");
  unsetenv("LC_NUMERIC");
  unsetenv("LC_NUMERIC");
  unsetenv("LC_PAPER");
  unsetenv("LC_TELEPHONE");
  unsetenv("LC_TIME");
  setenv("LANG", "C", (int)true);
  setenv("LC_ALL", "C.UTF-8", (int)true);
  std::setlocale(LC_ALL, "C.UTF-8");
  rps_backtrace_common_state =
    backtrace_create_state(rps_progname, (int)true,
                           Rps_Backtracer::bt_error_cb,
                           nullptr);
  if (!rps_backtrace_common_state)
    {
      fprintf(stderr, "%s failed to make backtrace state.\n", rps_progname);
      exit(EXIT_FAILURE);
    }
  pthread_setname_np(pthread_self(), "rps-main");
  // hack to handle debug flag as first program argument
  if (argc>1 && !strncmp(argv[1], "--debug=", strlen("--debug=")))
    rps_add_debug_cstr((argv[1]+strlen("--debug=")));
  if (argc>1 && !strncmp(argv[1], "-d", strlen("-d")))
    rps_add_debug_cstr((argv[1]+strlen("-d")));
  ///
  if (rps_syslog_enabled && rps_debug_flags != 0)
    openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
  RPS_INFORMOUT("done early initialization of RefPerSys process "
                << (int)getpid() << " on host " << rps_hostname()
                << " git " << rps_shortgitid);
} // end rps_early_initialization

////////////////////////////////////////////////////////////////
// Parse a single program option, skipping side effects when state is
// empty.
error_t
rps_parse1opt (int key, char *arg, struct argp_state *state)
{
  bool side_effect = state && (void*)state != RPS_EMPTYSLOT;
  RPS_POSSIBLE_BREAKPOINT();
  bool letterkey = (key>0 && key<256 && isalpha((char)key));
  RPS_DEBUG_LOG(PROGARG, "rps_parse1opt key#" << key
                << (letterkey?"'":"")
                << (letterkey? ((char)key) : ' ')
                << " arg:" << Rps_Cjson_String(arg)
                << (side_effect?".":"")
               );
  if (side_effect)
    RPS_DEBUG_LOG(PROGARG, "rps_parse1opt "
                  << RPS_OUT_PROGARGS(state->argc, state->argv)
                  << " argnum:" << state->arg_num
                  << " state.next:" << state->next
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,"rps_parse1opt"));
  switch (key)
    {
    case RPSPROGOPT_DEBUG:
    {
      rps_add_debug_cstr(arg);
    }
    return 0;
    case RPSPROGOPT_DEBUG_PATH:
    {
      if (side_effect)
        rps_set_debug_output_path(arg);
    }
    return 0;
    case RPSPROGOPT_FLTK:
    {
      RPS_DEBUG_LOG(PROGARG, "rps_parse1opt fltk arg=" << arg);
      rps_fltk_progoption(arg, state, side_effect);
    }
    return 0;
    case RPSPROGOPT_LOADDIR:
    {
      rps_my_load_dir = std::string(arg);
    }
    return 0;
    case RPSPROGOPT_COMMAND:
    {
      rps_command_vec.push_back(std::string(arg));
    }
    return 0;
    case RPSPROGOPT_INTERFACEFIFO:
    {
      rps_put_fifo_prefix(arg);
    }
    return 0;
    case RPSPROGOPT_BATCH:
    {
      rps_batch = true;
    }
    return 0;
    case RPSPROGOPT_JOBS:
    {
      int nbjobs = atoi(arg);
      if (nbjobs <= RPS_NBJOBS_MIN)
        nbjobs = RPS_NBJOBS_MIN;
      else if (nbjobs > RPS_NBJOBS_MAX)
        nbjobs = RPS_NBJOBS_MAX;
      rps_nbjobs = nbjobs;
    }
    return 0;
    case RPSPROGOPT_PUBLISH_ME:
    {
      if (!rps_publisher_url_str.empty())
        RPS_FATAL("cannot give twice the --publish-me <URL> option");
      rps_publisher_url_str = arg;
    }
    return 0;
    case RPSPROGOPT_PREFERENCES_HELP:
    {
    }
    return 0;
    case RPSPROGOPT_DUMP:
    {
      if (side_effect)
        rps_dumpdir_str = std::string(arg);
    }
    return 0;
    case RPSPROGOPT_CHDIR_BEFORE_LOAD:
    {
      char cwdbuf[rps_path_byte_size+4];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      if (side_effect)
        {
          if (chdir(arg))
            {
              RPS_FATALOUT("failed to chdir before loading to " << arg
                           << ":" << strerror(errno));
              char*cwd = getcwd(cwdbuf, rps_path_byte_size);
              if (!cwd)
                RPS_FATALOUT("failed to getcwd after chdir to " << arg);
              RPS_INFORMOUT("changed current directory before loading to "
                            << cwd);
            };
        }
      return 0;
    }
    case RPSPROGOPT_CHDIR_AFTER_LOAD:
    {
      if (side_effect)
        {
          rps_chdir_path_after_load = arg;
        }
      return 0;
    }
    case RPSPROGOPT_HOMEDIR:
    {
      struct stat rhomstat;
      memset (&rhomstat, 0, sizeof(rhomstat));
      if (stat(arg, &rhomstat))
        RPS_FATAL("failed to stat --refpersys-home %s: %m",
                  arg);
      if (!S_ISDIR(rhomstat.st_mode))
        RPS_FATAL("given --refpersys-home %s is not a directory",
                  arg);
      if ((rhomstat.st_mode & (S_IRUSR|S_IXUSR)) !=  (S_IRUSR|S_IXUSR))
        RPS_FATAL("given --refpersys-home %s is not user readable and executable",
                  arg);
      if (side_effect)
        {
          char*rhomrp = realpath(arg, nullptr);
          if (!rhomrp)
            RPS_FATAL("realpath failed on given --refpersys-home %s - %m",
                      arg);
          if (strlen(rhomrp) >= rps_path_byte_size -1)
            RPS_FATAL("too long realpath %s on given --refpersys-home %s - %m",
                      rhomrp, arg);
          strncpy(rps_bufpath_homedir, rhomrp, rps_path_byte_size -1);
          free (rhomrp), rhomrp = nullptr;
          RPS_INFORMOUT("set RefPerSys home directory to " << rps_bufpath_homedir);
        };
    }
    return 0;
    case RPSPROGOPT_RUN_NAME:
    {

      if (!rps_run_name.empty())
        RPS_FATALOUT("duplicate RefPerSys run name " << rps_run_name << " and " << std::string(arg));
      rps_run_name.assign(std::string(arg));
      RPS_INFORMOUT("set RefPerSys run name to " <<  Rps_QuotedC_String(rps_run_name));
    }
    return 0;
    case RPSPROGOPT_ECHO:
      /// example argument: --echo='Hello here'
    {
      if (!rps_run_name.empty())
        RPS_INFORMOUT(rps_run_name << " echo:" << std::string (arg)
                      << " git:" << rps_shortgitid);
      else
        RPS_INFORMOUT("echo:" << std::string(arg)
                      << " git:" << rps_shortgitid);
    }
    return 0;
    case RPSPROGOPT_USER_PREFERENCES:
      /// example argument: -U ~/myrefpersys.pref
      /// other example: --user-pref=$HOME/myrps.pref
    {
      if (access(arg, R_OK))
        rps_set_user_preferences(arg);
      else
        RPS_FATALOUT("missing user preferences file " << arg
                     << ":" << strerror(errno));
    }
    return 0;
    case RPSPROGOPT_RUN_DELAY:
    {
      int pos= -1;
      long dl= -1;
      /// example argument: --run-delay=45s for elapsed seconds
      if ((pos= -1), sscanf(arg, "%li s%n", &rps_run_delay, &pos) > 0
          && rps_run_delay>0 && pos>0)
        RPS_INFORMOUT("RefPerSys will run its agenda for "
                      <<  rps_run_delay
                      << " elapsed seconds.");
      ///
      /// example argument: --run-delay=10m for elapsed minutes
      else if ( ((pos= -1), (dl=0)),
                sscanf(arg, "%li m%n", &dl, &pos) > 0
                && dl>0 && pos>0)
        {
          rps_run_delay = dl*60;
          RPS_INFORMOUT("RefPerSys will run its agenda for "
                        << dl << " minutes so "
                        << rps_run_delay << " elapsed seconds");
        }

      ///
      /// example argument: --run-delay=3h for elapsed hours
      else if ( ((pos= -1), (dl=0)),
                sscanf(arg, "%li h%n", &dl, &pos) > 0
                && dl>0 && pos>0)
        {
          rps_run_delay = dl*3600;
          RPS_INFORMOUT("RefPerSys will run its agenda for "
                        << dl << " hours so " << rps_run_delay
                        << " elapsed seconds");
        }
      else
        RPS_FATAL("invalid --run-delay=%s argument.\n"
                  "\t (should be like 90s or 20m or 2h)",
                  arg);
    }
    return 0;
    case RPSPROGOPT_RANDOMOID:
    {
      int nbrand = atoi(arg);
      if (nbrand <= 0) nbrand = 2;
      else if (nbrand > 100) nbrand = 100;
      if (side_effect)
        {
          RPS_INFORM("output of %d random objids generated on %.2f\n", nbrand,
                     rps_wallclock_real_time());
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
    return 0;
    case RPSPROGOPT_TYPEINFO:
    {
      if (side_effect)
        rps_print_types_info ();
      rps_batch = true;
    }
    return 0;
    case RPSPROGOPT_SYSLOG:
    {
      if (side_effect && !rps_syslog_enabled)
        {
          rps_syslog_enabled = true;
          openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
          RPS_INFORM("using syslog");
        }
    }
    return 0;
    case RPSPROGOPT_NO_TERMINAL:
    {
      rps_without_terminal_escape = true;
    }
    return 0;
    case RPSPROGOPT_DAEMON:
    {
      rps_without_terminal_escape = true;
      char cwdbuf[rps_path_byte_size];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      if (side_effect)
        {
          if (!rps_syslog_enabled)
            {
              rps_syslog_enabled = true;
              openlog("RefPerSys", LOG_PERROR|LOG_PID, LOG_USER);
            };
          RPS_INFORM("using syslog with daemon");
          if (daemon(/*nochdir:*/1,
                                 /*noclose:*/0))
            RPS_FATAL("failed to daemon");
          rps_daemonized = true;
          const char*cw = getcwd(cwdbuf, sizeof(cwdbuf)-1);
          RPS_INFORM("daemonized pid %d in dir %s git %s",
                     (int)getpid(), cw, rps_shortgitid);
        };
    }
    return 0;
    case RPSPROGOPT_PID_FILE:
    {
      if (side_effect)
        rps_pidfile_path = arg;
    }
    return 0;
    case RPSPROGOPT_FULL_GIT:
    {
      if (side_effect)   /// see also rps_early_initialization
        {
          printf("%s\n", rps_gitid);
          fflush(nullptr);
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    case RPSPROGOPT_SHORT_GIT:
    {
      if (side_effect)   /// see also rps_early_initialization
        {
          printf("%s\n", rps_shortgitid);
          fflush(nullptr);
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    case RPSPROGOPT_NO_ASLR:
    {
      // was already handled
      RPS_ASSERT(rps_disable_aslr);
    }
    return 0;
    case RPSPROGOPT_NO_QUICK_TESTS:
    {
      rps_without_quick_tests = true;
    }
    return 0;
    case RPSPROGOPT_TEST_REPL_LEXER:
    {
      if (side_effect)
        {
          if (!rps_test_repl_string.empty())
            RPS_FATALOUT("only one --test-repl-lexer=TESTLEXSTRING can"
                         " be given, but already got "
                         << rps_test_repl_string);
          rps_test_repl_string = arg;
          RPS_INFORMOUT("will test the REPL lexer on:" << rps_test_repl_string
                        << std::endl << "... that is the " << rps_test_repl_string.size()
                        << " bytes string " << Rps_QuotedC_String(rps_test_repl_string));
        }
    }
    return 0;
    case RPSPROGOPT_DEBUG_AFTER_LOAD:
    {
      if (!rps_debugflags_after_load || side_effect)
        rps_debugflags_after_load = arg;
    }
    return 0;
    case RPSPROGOPT_EXTRA_ARG:
    {
      int eqnextpos= -1;
      char extraname[80];
      memset (extraname, 0, sizeof(extraname));
      if (sscanf(arg, "%72[A-Za-z0-9_]=%n", extraname, &eqnextpos) >= 1
          && isalpha(extraname[0])
          && eqnextpos > 1 && arg[eqnextpos-1] == '='
          && isalpha(extraname[0]))
        {
          for (const char*n = extraname; *n; n++)
            if (!isalnum(*n) && *n != '_')
              RPS_FATALOUT("invalid extra named argument " << extraname);
          if (rps_dict_extra_arg.find(extraname) != rps_dict_extra_arg.end())
            RPS_FATALOUT("extra named argument " << extraname
                         << " cannot be set more than once");
          std::string extraval{arg+eqnextpos};
          rps_dict_extra_arg.insert({extraname, extraval});
          RPS_INFORMOUT("set extra argument " << extraname
                        << " to '" << Rps_QuotedC_String(extraval)
                        << "'");
        }
      else
        RPS_FATALOUT("bad extra named argument " << arg
                     << " that is '" << Rps_QuotedC_String(arg)
                     << "' extra name is '" << Rps_QuotedC_String(extraname) << '"'
                    );
    }
    return 0;
    case RPSPROGOPT_RUN_AFTER_LOAD:
    {
      if (rps_run_command_after_load)
        RPS_FATALOUT("only one --run-after-load command can be given, not both " << rps_run_command_after_load
                     << " and " << arg);
      rps_run_command_after_load = arg;
    }
    return 0;
    case RPSPROGOPT_PLUGIN_AFTER_LOAD:
    {
      void* dlh = dlopen(arg, RTLD_NOW|RTLD_GLOBAL);
      if (!dlh)
        RPS_FATALOUT("failed to dlopen plugin " << arg << " : " << dlerror());
      const char* bnplug = basename(arg);
      Rps_Plugin curplugin(bnplug, dlh);
      RPS_INFORMOUT("loaded plugin#" << rps_plugins_vector.size() << " from " << arg << " from process pid#" << (int)getpid()
                    << " basenamed " << Rps_QuotedC_String(bnplug));
      rps_plugins_vector.push_back(curplugin);
    }
    return 0;
    case RPSPROGOPT_PLUGIN_ARG:
    {
      char plugname[80];
      char plugarg[128];
      memset (plugname, 0, sizeof(plugname));
      memset (plugarg, 0, sizeof(plugarg));
      if (!arg)
        RPS_FATALOUT("missing --plugin-arg");
      if (strlen(arg) >= sizeof(plugname) + sizeof(plugarg) - 1)
        RPS_FATALOUT("too long --plugin-arg" << arg
                     << " should be shorter than " << ( sizeof(plugname) + sizeof(plugarg)) << " bytes");
      if (sscanf(arg, "%78[a-zA-Z0-9_]:%126s", plugname, plugarg) < 2)
        RPS_FATALOUT("expecting --plugin-arg=<plugin-name>:<plugin-arg-string but got " << arg);
      int pluginix= -1;
      int plugcnt = 0;
      for (Rps_Plugin curplugin: rps_plugins_vector)
        {
          std::string curplugname = curplugin.plugin_name;
          int pluglenam= curplugname.length();
          if (pluglenam > 4 && curplugname.substr(pluglenam-3) == ".so")
            curplugname.erase(pluglenam-3);
          RPS_DEBUG_LOG (REPL, "plugin#" << plugcnt << " is named " << Rps_QuotedC_String(curplugname));
          if (curplugname == plugname)
            {
              pluginix = plugcnt;
              break;
            };
          plugcnt++;
        }
      if (pluginix<0)
        RPS_FATALOUT("--plugin-arg=" << plugname << ":" << plugarg
                     << " without such loaded plugin (loaded " << plugcnt << " plugins)");
      Rps_Plugin thisplugin = rps_plugins_vector[pluginix];
      rps_pluginargs_map[plugname] = std::string{plugarg};
      RPS_INFORMOUT("registering plugin argument of --plugin-arg " << arg
                    << " plugname=" << Rps_QuotedC_String(plugname)
                    << " plugarg=" << Rps_QuotedC_String(plugarg)
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "--plugin-arg processing"));
    }
    return 0;
    case RPSPROGOPT_CPLUSPLUSEDITOR_AFTER_LOAD:
    {
      RPS_DEBUG_LOG(CMD, "option --cplusplus-editor "
                    << (arg?" with '":" without ")
                    << (arg?arg:" argument")
                    << (arg?"'":" !!!")
                    << (side_effect?" side-effecting"
                        :" without side effect"));
      if (side_effect)
        {
          if (!arg || !arg[0])
            {
              char* editor = getenv("EDITOR");
              if (editor && editor[0])
                {
                  arg = editor;
                  RPS_INFORMOUT("using $EDITOR variable " << editor << " as C++ editor");
                }
            };
          if (!arg || !arg[0])
            RPS_FATALOUT("program option --cplusplus-editor-after-load"
                         " without explicit editor,\n"
                         "... and no $EDITOR environment variable");
          if (!rps_cpluspluseditor_str.empty())
            RPS_FATALOUT("program option --cplusplus-editor-after-load"
                         " given twice with "
                         << rps_cpluspluseditor_str << " and " << arg);
          rps_cpluspluseditor_str.assign(arg);
        };
    }
    return 0;
    case RPSPROGOPT_CPLUSPLUSFLAGS_AFTER_LOAD:
    {
      if (side_effect)
        {
          if (!rps_cplusplusflags_str.empty())
            RPS_FATALOUT("program option --cplusplus-flags-after-load given twice with "
                         << rps_cplusplusflags_str << " and " << arg);
          rps_cplusplusflags_str.assign(arg);
        }
    }
    return 0;
    case RPSPROGOPT_VERSION:
    {
      if (side_effect)
        {
          rps_show_version();
          exit(EXIT_SUCCESS);
        }
    }
    return 0;
    };        // end switch key
  return ARGP_ERR_UNKNOWN;
} // end rps_parse1opt

struct argp argparser_rps;



// rps_parse_program_arguments is called very early from main...
void
rps_parse_program_arguments(int &argc, char**argv)
{
  errno = 0;
  rps_early_initialization  (argc, argv);
  errno = 0;
  struct argp_state argstate;
  memset (&argstate, 0, sizeof(argstate));
  argparser_rps.options = rps_progoptions; // defined in main_rps.cc
  argparser_rps.parser = rps_parse1opt;
  argparser_rps.args_doc = " ; # ";
  argparser_rps.doc =
    "RefPerSys - an opensource Artificial Intelligence inference engine project,\n"
    " open science, for Linux/x86-64; see refpersys.org for more.\n"
    " (REFlexive PERsystem SYStem is GPLv3+ licensed free software)\n"
    " You should have received a copy of the GNU General Public License\n"
    " along with this program.  If not, see www.gnu.org/licenses\n"
    " *** NO WARRANTY, not even for FITNESS FOR A PARTICULAR PURPOSE ***\n"
    " +++!!! use at your own risk !!!+++\n"
    " (shortgitid " RPS_SHORTGITID " built at " __DATE__ ")\n"
    "\n Accepted program options are:\n";
  argparser_rps.children = nullptr;
  argparser_rps.help_filter = nullptr;
  argparser_rps.argp_domain = nullptr;
  int aix= -1;
  if (argp_parse(&argparser_rps, argc, argv, 0, &aix, nullptr))
    RPS_FATALOUT("failed to parse program arguments to " << argv[0]
                 << " at program argument index aix=" << aix);
  RPS_POSSIBLE_BREAKPOINT();
} // end rps_parse_program_arguments



/// most of the time this function is used thru RPS_OUT_PROGARGS macro
void
rps_output_program_arguments(std::ostream& out, int argc, const char*const*argv)
{
  for (int i=0; i<argc; i++)
    {
      if (i>0) out << ' ';
      const char*curparg = argv[i];
      if (!curparg)
        break;
      bool goodchar = true;
      for (const char* pc = curparg; goodchar && *pc; pc++)
        {
          if (isalnum(*pc) || *pc=='_' || *pc=='-' || *pc=='+'
              || *pc=='/' || *pc=='.' || *pc==',' || *pc==':'
              || *pc=='=' || *pc=='%' || *pc=='@')
            continue;
          else
            {
              goodchar = false;
              RPS_POSSIBLE_BREAKPOINT();
              break;
            }
        };
      if (goodchar)
        out << curparg;
      else
        out << Rps_QuotedC_String(curparg);
    };
  out << std::endl;
} // end rps_output_program_arguments

void
rps_compute_program_invocation(int argc, char**argv)
{
  std::ostringstream outs;
  rps_output_program_arguments(outs, argc, argv);
  outs.flush();
  std::string pstr = outs.str();
  size_t plen = pstr.size();
  rps_program_invocation = (char*)calloc(1, ((plen+20)|0x1f)+1);
  if (rps_program_invocation)
    strncpy(rps_program_invocation, pstr.c_str(), plen);
} // end rps_compute_program_invocation

const char*
rps_get_plugin_cstr_argument(const Rps_Plugin*plugin)
{
  if (!plugin)
    return nullptr;
  auto it = rps_pluginargs_map.find(plugin->plugin_name);
  if (it == rps_pluginargs_map.end())
    return nullptr;
  return it->second.c_str();
} // end rps_get_plugin_cstr_argument

void
rps_postponed_remove_file(const std::string& path)
{
  std::lock_guard<std::mutex> gu(rps_postponed_lock);
  if (rps_postponed_removed_files_vector.empty())
    atexit(rps_schedule_files_postponed_removal);
  rps_postponed_removed_files_vector.push_back(std::string(path));
} // end rps_postponed_remove_file

void
rps_schedule_files_postponed_removal(void)
{
  std::lock_guard<std::mutex> gu(rps_postponed_lock);
  if (rps_postponed_removed_files_vector.empty())
    return;
  FILE* pat = popen("/bin/at now + 5 minutes", "w");
  if (!pat)
    {
      RPS_WARNOUT("failed to open /bin/at now + 5 minutes :" << strerror(errno));
      return;
    };
  if (rps_syslog_enabled)
    syslog(LOG_NOTICE, "RefPerSys will later remove %d files (in five minutes, with /bin/at)", (int) rps_postponed_removed_files_vector.size());
  else
    RPS_INFORM("RefPerSys will later remove %d files (in five minutes, with /bin/at)", (int) rps_postponed_removed_files_vector.size());
  for  (auto rf: rps_postponed_removed_files_vector)
    {
      if (rps_syslog_enabled)
        syslog(LOG_NOTICE, "*rm  %s",  Rps_SingleQuotedC_String(rf).c_str());
      else
        printf(" *rm %s\n", Rps_SingleQuotedC_String(rf).c_str());
      fprintf(pat, "/bin/rm -f %s\n", Rps_SingleQuotedC_String(rf).c_str());
    }
  rps_postponed_removed_files_vector.clear();
  pclose(pat);
} // end rps_schedule_files_postponed_removal

////////////////
void
rps_fatal_stop_at (const char *filnam, int lin)
{
  static constexpr int skipfatal=2;
  assert(filnam != nullptr);
  assert (lin>=0);
  char errbuf[80];
  memset (errbuf, 0, sizeof(errbuf));
  char cwdbuf[rps_path_byte_size];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  if (!getcwd(cwdbuf, sizeof(cwdbuf)) || cwdbuf[0] == (char)0)
    strcpy(cwdbuf, "./");
  snprintf (errbuf, sizeof(errbuf), "FATAL STOP (%s:%d)", filnam, lin);
  /* we always syslog.... */
  syslog(LOG_EMERG, "RefPerSys fatal stop (%s:%d) git %s,\n"
         "... build %s pid %d on %s,\n"
         "... elapsed %.3f, process %.3f sec in %s\n%s%s%s%s",
         filnam, lin, rps_shortgitid,
         rps_timestamp, (int)getpid(), rps_hostname(),
         rps_elapsed_real_time(), rps_process_cpu_time(), cwdbuf,
         (rps_program_invocation?"... started as ":""),
         (rps_program_invocation?:""),
         (rps_run_name.empty()?"":" run "),
         rps_run_name.c_str());
  bool ontty = isatty(STDERR_FILENO);
  if (rps_debug_file)
    {
      fprintf(rps_debug_file, "\n*§*§* RPS FATAL %s:%d %s*§*§*\n", filnam, lin, rps_run_name.c_str());
      if (rps_program_invocation)
        fprintf(rps_debug_file,
                "… started as %s\n", rps_program_invocation);
    }
  if (!rps_syslog_enabled)
    fprintf(stderr, "\n" "%s%sRPS FATAL:%s\n"
            " RefPerSys gitid %s,\n"
            "\t built timestamp %s,\n"
            "\t on host %s, md5sum %s,\n"
            "\t elapsed %.3f, process %.3f sec in %s\n",
            ontty?RPS_TERMINAL_BOLD_ESCAPE:"",
            ontty?RPS_TERMINAL_BLINK_ESCAPE:"",
            ontty?RPS_TERMINAL_NORMAL_ESCAPE:"",
            rps_gitid, rps_timestamp, rps_hostname(), rps_md5sum,
            rps_elapsed_real_time(), rps_process_cpu_time(), cwdbuf);
  if (rps_debug_file && rps_debug_file != stderr && rps_debug_path[0])
    {
      fprintf(stderr, "*°* see debug output in %s\n", rps_debug_path);
      fprintf(rps_debug_file, "RefPerSys gitid %s built %s was started on %s pid %d as:\n",
              rps_shortgitid, rps_timestamp, rps_hostname(), (int)getpid());
      for (int aix=0; aix<rps_argc; aix++)
        {
          fputc(' ', rps_debug_file);
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'
                            || curarg[0]=='-' || curarg[0]=='=' || curarg[0]=='@';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            fputs(curarg, rps_debug_file);
          else
            fprintf(rps_debug_file, "'%s'", Rps_QuotedC_String(curarg).c_str());
        }
      fputc('\n', rps_debug_file);
    }
  fflush (stderr);
  fflush (rps_debug_file);
  if (rps_syslog_enabled)
    {
      std::ostringstream outl;
      auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                                 filnam, lin,
                                 skipfatal, "RefPerSys FATAL ERROR",
                                 &outl);
      backt.output(outl);
      outl << "===== end fatal error at " << filnam << ":" << lin
           << " ======" << std::endl << std::flush;
      outl << "RefPerSys gitid " << rps_shortgitid << " built " << rps_timestamp
           << " was started on " << rps_hostname() << " pid "
           << (int)getpid() << " as";
      if (!rps_run_name.empty())
        outl << " run " << rps_run_name;
      outl << ':' << std::endl;
      for (int aix=0; aix<rps_argc; aix++)
        {
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'
                            || curarg[0]=='-' || curarg[0]=='=' || curarg[0]=='@';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            outl << ' ' << curarg;
          else
            outl << ' ' << Rps_SingleQuotedC_String(curarg);
        }
      outl << std::endl << std::flush;
      syslog(LOG_EMERG, "RefPerSys fatal from %s", outl.str().c_str());
    }
  else
    {
      auto backt= Rps_Backtracer(Rps_Backtracer::FullOut_Tag{},
                                 filnam, lin,
                                 skipfatal, "RefPerSys FATAL ERROR",
                                 &std::clog);
      backt.output(std::clog);
      std::clog << "===== end fatal error at " << filnam << ":" << lin
                << " ======" << std::endl << std::flush;
      std::clog << "RefPerSys gitid " << rps_shortgitid << " built " << rps_timestamp;
      if (!rps_run_name.empty())
        std::clog << " run " << rps_run_name;
      std::clog << " was started on " << rps_hostname() << " pid " << (int)getpid() << " as:" << std::endl;
      for (int aix=0; aix<rps_argc; aix++)
        {
          const char*curarg = rps_argv[aix];
          bool isplainarg = isalnum(curarg[0]) || curarg[0]=='/'
                            || curarg[0]=='_' || curarg[0]=='.'  || curarg[0]=='-';
          for (const char*pc = curarg; *pc != (char)0 && isplainarg; pc++)
            isplainarg = *pc>' ' && *pc<(char)127
                         && *pc != '\'' && *pc != '\\' && *pc != '\"'
                         && isprint(*pc);
          if (isplainarg)
            std::clog << ' ' << curarg;
          else
            std::clog << ' ' << Rps_SingleQuotedC_String(curarg);
        }
      std::clog << std::endl << std::flush;
    }
  fflush(nullptr);
  rps_schedule_files_postponed_removal();
  abort();
} // end rps_fatal_stop_at


void rps_debug_warn_at(const char*file, int line)
{
  if (rps_syslog_enabled)
    {
      syslog(LOG_WARNING, "** REFPERSYS WARNING AT %s:%d (git %s pid %d) **", file, line,
             rps_shortgitid, (int)getpid());
    }
  else
    {
      std::cerr << std::flush;
      std::cerr << std::endl << "**!** REFPERSYS WARNING at " << file << ":" << line << std::endl;
    };
  if (rps_debug_file)
    {
      fprintf(rps_debug_file, "\n*** REFPERSYS WARNING at %s:%d ***\n", file, line);
      fflush(rps_debug_file);
    }
} // end rps_debug_warn_at


////////////////////////////////////////////////////////////////
// TIME ROUTINES
////////////////////////////////////////////////////////////////

double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
}

double rps_get_start_wallclock_real_time()
{
  return rps_start_wallclock_real_time;
}


void
rps_print_objectref(Rps_ObjectRef ob)
{
  std::cout << ob << std::endl;
} // end rps_print_objectref

////////////////////////////////////////////////////////////////
///// global roots for garbage collection and persistence

static std::set<Rps_ObjectRef> rps_object_root_set;
static std::mutex rps_object_root_mtx;
static std::unordered_map<Rps_Id,Rps_ObjectRef*,Rps_Id::Hasher> rps_object_global_root_hashtable;

void
rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (auto ob: rps_object_root_set)
    fun(ob);
} // end rps_each_root_object


void
rps_add_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_root_set.insert(ob);
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      *(rootit->second) = ob;
  }
} // end rps_add_root_object


bool
rps_remove_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  if (it == rps_object_root_set.end())
    return false;
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      (*(rootit->second)) = Rps_ObjectRef(nullptr);
  }
  rps_object_root_set.erase(it);
  return true;
} // end rps_remove_root_object

void
rps_initialize_roots_after_loading (Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_global_root_hashtable.max_load_factor(3.5);
  rps_object_global_root_hashtable.reserve(5*rps_hardcoded_number_of_roots()/4+3);
  ///
#define RPS_INSTALL_ROOT_OB(Oid) {              \
    const char*end##Oid = nullptr;              \
    bool ok##Oid = false;                       \
    Rps_Id id##Oid(#Oid, &end##Oid, &ok##Oid);  \
    RPS_ASSERT (end##Oid && !*end##Oid);        \
    RPS_ASSERT (ok##Oid);                       \
    RPS_ASSERT (id##Oid.valid());               \
    rps_object_global_root_hashtable[id##Oid]   \
      = &RPS_ROOT_OB(Oid);                      \
  };
#include "generated/rps-roots.hh"
} // end of rps_initialize_roots_after_loading

bool rps_is_root_object (const Rps_ObjectRef ob)
{
  if (!ob)
    return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  return it != rps_object_root_set.end();
} // end rps_is_root_object

std::set<Rps_ObjectRef>
rps_set_root_objects(void)
{
  std::set<Rps_ObjectRef> set;

  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (Rps_ObjectRef ob: rps_object_root_set)
    set.insert(ob);
  return set;
} // end rps_set_root_objects

unsigned
rps_nb_root_objects(void)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  return (unsigned) rps_object_root_set.size();
} // end rps_nb_root_objects


void
rps_add_constant_object(Rps_CallFrame*callframe, const Rps_ObjectRef argob)
{
  if (!argob)
    return;
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), //"constant"∈named_attribute
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obconst;
                           Rps_ObjectRef obsystem;
                           Rps_ObjectRef obnamedattr;
                           Rps_ObjectRef oboldroot;
                           Rps_Value oldsetv;
                           Rps_Value newsetv;
                );
  _f.obconst = argob;
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object start adding " << _f.obconst
                << " of class " <<  _f.obconst->get_class()
                << " in space " << _f.obconst->get_space() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_add_constant_object/start"));
  if (false
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_10YXWeY7lYc01RpQTA) //the_system_class∈class
      || _f.obconst == RPS_ROOT_OB(_1Io89yIORqn02SXx4p) //RefPerSys_system∈the_system_class
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) //int∈class
      || _f.obconst == RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC) //"code_module"∈named_attribute
      || _f.obconst == RPS_ROOT_OB(_3rXxMck40kz03RxRLM) //code_chunk∈class
      || _f.obconst == RPS_ROOT_OB(_3s7ztCCoJsj04puTdQ) //agenda∈class
      || _f.obconst == RPS_ROOT_OB(_3GHJQW0IIqS01QY8qD) //json∈class
      || _f.obconst == RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5) //symbol∈symbol
      || _f.obconst == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) //class∈class
      || _f.obconst == RPS_ROOT_OB(_4jISxMJ4PYU0050nUl) //closure∈class
      || _f.obconst == RPS_ROOT_OB(_4pSwobFHGf301Qgwzh) //named_attribute∈class
      || _f.obconst == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
      || _f.obconst == RPS_ROOT_OB(_5CYWxcChKN002rw1fI) //contributor_to_RefPerSys∈class
      || _f.obconst == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
      || _f.obconst == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
      || _f.obconst == RPS_ROOT_OB(_6fmq7pZkmNd03UyPuO) //class∈symbol
      || _f.obconst == RPS_ROOT_OB(_6gxiw0snqrX01tZWW9) //"set_of_core_functions"∈mutable_set
      || _f.obconst == RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) //immutable_instance∈class
      || _f.obconst == RPS_ROOT_OB(_6JYterg6iAu00cV9Ye) //set∈class
      || _f.obconst == RPS_ROOT_OB(_6NVM7sMcITg01ug5TC) //tuple∈class
      || _f.obconst == RPS_ROOT_OB(_6XLY6QfcDre02922jz) //value∈class
      || _f.obconst == RPS_ROOT_OB(_7OrPRWQEg2o043XvK2) //rps_routine∈class
      || _f.obconst == RPS_ROOT_OB(_7Y3AyF9gNx700bQJXc) //string_buffer∈class
      || _f.obconst == RPS_ROOT_OB(_8fYqEw8vTED03wsznt) //tasklet∈class
      || _f.obconst == RPS_ROOT_OB(_8J6vNYtP5E800eCr5q) //"initial_space"∈space
      || _f.obconst == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
      || _f.obconst == RPS_ROOT_OB(_9uwZtDshW4401x6MsY) //space∈symbol
      || _f.obconst == RPS_ROOT_OB(_9BnrMLXUhfG00llx8X) //function∈class
      || _f.obconst == RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS) //core_function∈class
     )
    {
      RPS_WARNOUT("cannot add core sacred root object as constant " << _f.obconst << " of class " << _f.obconst->get_class()
                  << " thread " << rps_current_pthread_name()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_add_constant_object")
                 );
      return;
    };

  _f.obsystem = RPS_ROOT_OB(_1Io89yIORqn02SXx4p); //RefPerSys_system∈the_system_class
  std::lock_guard<std::recursive_mutex> gu(*_f.obsystem->objmtxptr());
  _f.oldsetv = _f.obsystem->get_physical_attr
               (RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp)); // //"constant"∈named_attribute
  RPS_ASSERT(_f.oldsetv.is_set());
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst="
                << _f.obconst << " oldset=" << _f.oldsetv);
  RPS_POSSIBLE_BREAKPOINT();
  _f.newsetv = Rps_SetValue({_f.oldsetv, Rps_Value(_f.obconst)});
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst="
                << _f.obconst << " oldset=" << _f.oldsetv
                << " newset=" << _f.newsetv);
  RPS_ASSERT(_f.newsetv.is_set() && _f.newsetv.as_set()->cardinal() > 0);
  RPS_ASSERT(_f.newsetv.as_set()->cardinal() >= _f.oldsetv.as_set()->cardinal());
  /// update the set of contants
  _f.obsystem->put_attr(RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), // //"constant"∈named_attribute
                        _f.newsetv);
  RPS_DEBUG_LOG(REPL, "rps_add_constant_object obconst=" << _f.obconst
                << " of class " << _f.obconst->get_class() << " space " << _f.obconst->get_space()
                << std::endl
                << "... oldfsetv=" << _f.oldsetv << " newsetv=" << _f.newsetv << " in " << _f.obsystem
                << RPS_FULL_BACKTRACE_HERE(1, "rps_add_constant_object/ending"));
#pragma message "perhaps rps_add_constant_object should remove obconst from the set of roots?"
} // end rps_add_constant_object

void
rps_remove_constant_object(Rps_CallFrame*callframe, const Rps_ObjectRef argobconst)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_2aNcYqKwdDR01zp0Xp), //"constant"∈named_attribute
                           /*callerframe:*/callframe,
                           Rps_ObjectRef obconst;
                           Rps_ObjectRef obsystem;
                           Rps_ObjectRef obnamedattr;
                           Rps_ObjectRef oboldroot;
                           Rps_Value oldsetv;
                           Rps_Value newsetv;
                );
  _f.obconst = argobconst;
  if (false
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_10YXWeY7lYc01RpQTA) //the_system_class∈class
      || _f.obconst == RPS_ROOT_OB(_1Io89yIORqn02SXx4p) //RefPerSys_system∈the_system_class
      || _f.obconst == RPS_ROOT_OB(_2i66FFjmS7n03HNNBx) //space∈class
      || _f.obconst == RPS_ROOT_OB(_2A2mrPpR3Qf03p6o5b) //int∈class
      || _f.obconst == RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC) //"code_module"∈named_attribute
      || _f.obconst == RPS_ROOT_OB(_3rXxMck40kz03RxRLM) //code_chunk∈class
      || _f.obconst == RPS_ROOT_OB(_3s7ztCCoJsj04puTdQ) //agenda∈class
      || _f.obconst == RPS_ROOT_OB(_3GHJQW0IIqS01QY8qD) //json∈class
      || _f.obconst == RPS_ROOT_OB(_3Q3hJsSgCDN03GTYW5) //symbol∈symbol
      || _f.obconst == RPS_ROOT_OB(_41OFI3r0S1t03qdB2E) //class∈class
      || _f.obconst == RPS_ROOT_OB(_4jISxMJ4PYU0050nUl) //closure∈class
      || _f.obconst == RPS_ROOT_OB(_4pSwobFHGf301Qgwzh) //named_attribute∈class
      || _f.obconst == RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ) //object∈class
      || _f.obconst == RPS_ROOT_OB(_5CYWxcChKN002rw1fI) //contributor_to_RefPerSys∈class
      || _f.obconst == RPS_ROOT_OB(_5LMLyzRp6kq04AMM8a) //environment∈class
      || _f.obconst == RPS_ROOT_OB(_62LTwxwKpQ802SsmjE) //string∈class
      || _f.obconst == RPS_ROOT_OB(_6fmq7pZkmNd03UyPuO) //class∈symbol
      || _f.obconst == RPS_ROOT_OB(_6gxiw0snqrX01tZWW9) //"set_of_core_functions"∈mutable_set
      || _f.obconst == RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) //immutable_instance∈class
      || _f.obconst == RPS_ROOT_OB(_6JYterg6iAu00cV9Ye) //set∈class
      || _f.obconst == RPS_ROOT_OB(_6NVM7sMcITg01ug5TC) //tuple∈class
      || _f.obconst == RPS_ROOT_OB(_6XLY6QfcDre02922jz) //value∈class
      || _f.obconst == RPS_ROOT_OB(_7OrPRWQEg2o043XvK2) //rps_routine∈class
      || _f.obconst == RPS_ROOT_OB(_7Y3AyF9gNx700bQJXc) //string_buffer∈class
      || _f.obconst == RPS_ROOT_OB(_8fYqEw8vTED03wsznt) //tasklet∈class
      || _f.obconst == RPS_ROOT_OB(_8J6vNYtP5E800eCr5q) //"initial_space"∈space
      || _f.obconst == RPS_ROOT_OB(_98sc8kSOXV003i86w5) //double∈class
      || _f.obconst == RPS_ROOT_OB(_9uwZtDshW4401x6MsY) //space∈symbol
      || _f.obconst == RPS_ROOT_OB(_9BnrMLXUhfG00llx8X) //function∈class
      || _f.obconst == RPS_ROOT_OB(_9Gz1oNPCnkB00I6VRS) //core_function∈class
     )
    {
      RPS_WARNOUT("cannot remove core sacred root object as constant " << _f.obconst
                  << " thread " << rps_current_pthread_name()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_remove_constant_object")
                 );
      return;
    };
#pragma message "rps_remove_constant_object unimplemented"
  RPS_FATALOUT("rps_remove_constant_object unimplemented obconst=" << _f.obconst);
} // end rps_remove_constant_object

void
rps_initialize_symbols_after_loading(Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::recursive_mutex> gu(Rps_PayloadSymbol::symb_tablemtx);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.max_load_factor(2.5);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.reserve(5*rps_hardcoded_number_of_symbols()/4+3);
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name) {           \
    Rps_PayloadSymbol::symb_hardcoded_hashtable[#Name]  \
      = &RPS_SYMB_OB(Name);                             \
  };
#include "generated/rps-names.hh"
} // end of rps_initialize_symbols_after_loading

///////////////////////////////////////////////////////// debugging support
/// X macro tricks used below... see en.wikipedia.org/wiki/X_Macro

bool
rps_is_set_debug(const std::string &curlev)
{
  if (curlev.empty()) return false;
#define Rps_IS_SET_DEBUG(Opt) else if (curlev == #Opt)  \
    return  rps_debug_flags & (1 << RPS_DEBUG_##Opt);
  RPS_DEBUG_OPTIONS(Rps_IS_SET_DEBUG);
#undef Rps_IS_SET_DEBUG
  return false;
} // end rps_is_set_debug

Rps_Debug
rps_debug_of_string(const std::string &deblev)
{
  if (deblev.empty()) return RPS_DEBUG__NONE;
#define Rps_TEST_DEBUG(Opt) else if (deblev == #Opt) return RPS_DEBUG_##Opt;
  RPS_DEBUG_OPTIONS(Rps_TEST_DEBUG);
#undef Rps_TEST_DEBUG
  return RPS_DEBUG__NONE;
} // end rps_debug_of_string

bool
rps_set_debug_flag(const std::string &curlev)
{
  bool goodflag = false;
  if (curlev == "NEVER")
    {
      RPS_WARNOUT("forbidden debug level " << curlev);
    }
  else if (curlev == "help")
    {
      goodflag = true;
    }
  ///
  /* second X macro trick for processing several comma-separated debug flags, in all cases as else if branch  */
  ///
#define Rps_SET_DEBUG(Opt)            \
  else if (curlev == #Opt) {            \
    bool alreadygiven = rps_debug_flags & (1 << RPS_DEBUG_##Opt); \
    rps_debug_flags |= (1 << RPS_DEBUG_##Opt);        \
    goodflag = true;              \
    if (!alreadygiven)              \
      RPS_INFORMOUT("setting debugging flag " << #Opt);  }
  ///
  RPS_DEBUG_OPTIONS(Rps_SET_DEBUG);
#undef Rps_SET_DEBUG
  ////
  if (!goodflag)
    RPS_WARNOUT("unknown debug level " << curlev);
  return goodflag;
} // end rps_set_debug_flag

void
rps_set_debug(const std::string &deblev)
{
  static bool didhelp;
  if (deblev == "help" && !didhelp)
    {
      /* first X macro for help debug flag.... */
      didhelp = true;
      fprintf(stderr, "%s debugging options for git %s built at %s ...\n",
              rps_progname, rps_shortgitid, rps_timestamp);
      fprintf(stderr, "Comma separated debugging levels with -D<debug-level>\n"
              "\tor --debug=<debug-level> or --debug-after-load=<debug-level>:\n");

#define Rps_SHOW_DEBUG(Opt) fprintf(stderr, "\t%s\n", #Opt);
      RPS_DEBUG_OPTIONS(Rps_SHOW_DEBUG);
#undef Rps_SHOW_DEBUG
      fflush(nullptr);
    }
  else if (deblev.empty())
    {
      RPS_WARNOUT("empty debugging from " << RPS_FULL_BACKTRACE_HERE(1, "rps_set_debug/empty"));
    }
  else if (isdigit(deblev[0]))
    {
      char*pend = nullptr;
      long lev = strtol(&deblev[0], &pend, 0);
      if (pend && *pend != (char)0)
        RPS_WARNOUT("bad numerical debug level " << lev << " in " << deblev);
      rps_debug_flags = lev;
    }
  else
    {
      const char*comma=nullptr;
      for (const char*pc = deblev.c_str(); pc && *pc; pc = comma?(comma+1):nullptr)
        {
          comma = strchr(pc, ',');
          std::string curlev;
          if (comma && comma>pc)
            curlev = std::string(pc, comma-pc);
          else
            curlev = std::string(pc);
          if (!rps_set_debug_flag(curlev))
            RPS_FATALOUT("unexpected debug level " << curlev
                         << "; use --debug=help to get all known debug levels");
        };      // end for const char*pc ...

    } // else case, for deblev which is not help

  RPS_DEBUG_LOG(MISC, "rps_debug_flags=" << rps_debug_flags);
} // end rps_set_debug

void
rps_add_debug_cstr(const char*d)
{
  rps_set_debug(std::string(d));
} // end rps_add_debug_cstr


const char*
rps_cstr_of_debug(Rps_Debug dbglev)
{
  switch (dbglev)
    {
#define Rps_CSTR_DEBUG(Lev) case RPS_DEBUG_##Lev: return #Lev;
      RPS_DEBUG_OPTIONS(Rps_CSTR_DEBUG);
#undef Rps_CSTR_DEBUG
    default:
      ;
    }
  return nullptr;
} // end rps_cstr_of_debug

void
rps_output_debug_flags(std::ostream&out,  unsigned flags)
{
  if (!flags)
    flags = rps_debug_flags.load();
  out << flags << "=" ;
  int nbf = 0;
  //
#define SHOW_DBGFLAG(Lev)                       \
  do {                                          \
    if (flags & (1<< RPS_DEBUG_##Lev)) {        \
      if (nbf > 0)                              \
  out << ',';       \
      out << #Lev;                              \
      nbf++;                                    \
    }                                           \
  } while(0);
  ///
  RPS_DEBUG_OPTIONS(SHOW_DBGFLAG);
#undef SHOW_DBGFLAG
  out << std::flush;
} // end rps_output_debug_flags





////////////////////////////////////////////////////////////////

static std::recursive_mutex rps_aftevntloop_mtx;
static std::vector<std::function<void(void)>> rps_aftevntloop_vec;

void
rps_register_after_event_loop(std::function<void(void)>f)
{
  std::lock_guard<std::recursive_mutex> gu(rps_aftevntloop_mtx);
  rps_aftevntloop_vec.push_back(f);
} // end rps_register_after_event_loop

void
rps_run_after_event_loop(void)
{
  std::lock_guard<std::recursive_mutex> gu(rps_aftevntloop_mtx);
  for (std::function<void(void)> f: rps_aftevntloop_vec)
    f();
  rps_aftevntloop_vec.clear();
} // end rps_run_after_event_loop



////////////////////////////////////////////////////////////////
std::string
rps_stringprintf(const char*fmt, ...)
{
  va_list args;
  char smallbuf[128];
  memset (smallbuf, 0, sizeof(smallbuf));
  RPS_ASSERT(fmt);
  va_start(args, fmt);
  size_t l = vsnprintf(smallbuf, sizeof(smallbuf), fmt, args);
  va_end(args);
  if (l < sizeof(smallbuf)-4)
    {
      return std::string{smallbuf};
    }
  else
    {
      std::string res;
      size_t ml = ((l+4)|0xf)+1;
      char*buf = (char*)calloc(1, ml);
      if (!buf)
        RPS_FATALOUT("rps_stringprintf fmt " << fmt
                     << " fail to calloc " << ml << " bytes");
      va_start(args, fmt);
      size_t ll =  vsnprintf(buf, ml, fmt, args);
      RPS_ASSERT(ll == l);
      va_end(args);
      res=std::string(buf);
      free(buf);
      return res;
    }
} // end rps_stringprintf

//// end of file utilities_rps.cc

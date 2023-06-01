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
 *      © Copyright 2019 - 2023 The Reflective Persistent System Team
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


extern "C" const char rps_utilities_gitid[];
const char rps_utilities_gitid[]= RPS_GITID;

extern "C" const char rps_utilities_date[];
const char rps_utilities_date[]= __DATE__;

// we may have a pair of FIFO to communicate with some external
// process (for graphical user interface), perhaps mini-edit-fltk on
// https://github.com/bstarynk/misc-basile/ ... The FIFO prefix is
// $FIFOPREFIX. The messages from the GUI user interface to RefPerSys
// are on $FIFOPREFIX.out; the messages from RefPerSys to that GUI
// user interface are on $FIFOPREFIX.cmd
static std::string rps_fifo_prefix;
char rps_bufpath_homedir[rps_path_byte_size];
static  struct rps_fifo_fdpair_st rps_fifo_pair;

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
  RPS_ASSERT(rps_is_main_thread());
  std::string cmdfifo = rps_fifo_prefix+".cmd";
  std::string outfifo = rps_fifo_prefix+".out";
  if (!rps_is_fifo(cmdfifo))
    {
      if (mkfifo(cmdfifo.c_str(), 0660)<0)
        RPS_FATALOUT("failed to create command FIFO " << cmdfifo << ":" << strerror(errno));
      rmatex = true;
    }
  cmdfd = open(cmdfifo.c_str(), 0660 | O_CLOEXEC);
  if (cmdfd<0)
    RPS_FATALOUT("failed to open command FIFO " << cmdfifo << ":" << strerror(errno));
  if (!rps_is_fifo(outfifo))
    {
      if (mkfifo(outfifo.c_str(), 0660)<0)
        RPS_FATALOUT("failed to create output FIFO " << outfifo << ":" << strerror(errno));
      rmatex = true;
    }
  outfd = open(outfifo.c_str(), 0440 | O_CLOEXEC);
  if (outfd<0)
    RPS_FATALOUT("failed to open output FIFO " << outfifo << ":" << strerror(errno));
  if (rmatex)
    atexit(rps_remove_fifos);
  rps_fifo_pair.fifo_ui_wcmd = cmdfd;
  rps_fifo_pair.fifo_ui_rout = outfd;
  RPS_INFORMOUT("RefPerSys did create (in pid " << (int)getpid()
                << ") command and output FIFOs to communicate with GUI" << std::endl
                << "... using for written commands to GUI " << cmdfifo << " fd#" << cmdfd
                << std::endl
                << "... and for reading JSON output from GUI " << outfifo << " fd#" << outfd);
  usleep(1000);
} // end rps_do_create_fifos


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
  static char hnambuf[64];
  if (RPS_UNLIKELY(!hnambuf[0]))
    gethostname(hnambuf, sizeof(hnambuf)-1);
  return hnambuf;
} // end rps_hostname


void
rps_emit_gplv3_copyright_notice(std::ostream&outs, std::string path, std::string linprefix, std::string linsuffix)
{
  outs << linprefix
       << "GENERATED file " << path  << " / DO NOT EDIT!"
       << linsuffix << std::endl;
  outs << linprefix
       << "This file is part of the Reflective Persistent System."
       << linsuffix << std::endl;
  {
    time_t nowtime = time(nullptr);
    struct tm nowtm = {};
    localtime_r(&nowtime, &nowtm);
    outs << linprefix << " © Copyright " << RPS_INITIAL_COPYRIGHT_YEAR
         << " - " << (nowtm.tm_year+1900)
         << " The Reflective Persistent System Team."
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
} // end rps_emit_gplv3_copyright_notice


////////////////
void
rps_print_types_info(void)
{
#define TYPEFMT_rps "%-58s:"
  printf(TYPEFMT_rps "   size  align   (bytes)\n", "**TYPE**");
#define EXPLAIN_TYPE(Ty) printf(TYPEFMT_rps " %5d %5d\n", #Ty,		\
				(int)sizeof(Ty), (int)alignof(Ty))

#define EXPLAIN_TYPE2(Ty1,Ty2) printf(TYPEFMT_rps " %5d %5d\n",	\
				      #Ty1 "," #Ty2,		\
				      (int)sizeof(Ty1,Ty2),	\
				      (int)alignof(Ty1,Ty2))

#define EXPLAIN_TYPE3(Ty1,Ty2,Ty3) printf(TYPEFMT_rps " %5d %5d\n",	\
					  #Ty1 "," #Ty2 "," #Ty3,	\
					  (int)sizeof(Ty1,Ty2,Ty3),	\
					  (int)alignof(Ty1,Ty2,Ty3))
#define EXPLAIN_TYPE4(Ty1,Ty2,Ty3,Ty4) printf(TYPEFMT_rps " %5d %5d\n",	\
					      #Ty1 "," #Ty2 "," #Ty3 "," #Ty4, \
					      (int)sizeof(Ty1,Ty2,Ty3,Ty4), \
					      (int)alignof(Ty1,Ty2,Ty3,Ty4))
  EXPLAIN_TYPE(int);
  EXPLAIN_TYPE(double);
  EXPLAIN_TYPE(char);
  EXPLAIN_TYPE(bool);
  EXPLAIN_TYPE(void*);
  EXPLAIN_TYPE(std::mutex);
  EXPLAIN_TYPE(std::shared_mutex);
  EXPLAIN_TYPE(std::recursive_mutex);
  EXPLAIN_TYPE(std::atomic<void*>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::recursive_mutex>);
  EXPLAIN_TYPE(std::lock_guard<std::shared_mutex>);
  EXPLAIN_TYPE(std::string);
  EXPLAIN_TYPE(std::vector<std::string>);
  EXPLAIN_TYPE(std::set<std::string>);
  EXPLAIN_TYPE2(std::map<Rps_ObjectRef, Rps_Value>);
  EXPLAIN_TYPE2(std::unordered_map<std::string, Rps_ObjectRef*>);
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


void
rps_extend_env(void)
{
  static std::atomic<bool> extended;
  if (extended) return;
  RPS_ASSERT(rps_is_main_thread());
  extended = true;
  static char pidenv[64];
  snprintf(pidenv, sizeof(pidenv), "REFPERSYS_PID=%d", (int)getpid());
  putenv(pidenv);
  static char gitenv[64];
  snprintf(gitenv, sizeof(gitenv), "REFPERSYS_GITID=%s", rps_gitid);
  putenv(gitenv);
  static char topdirenv[384];
  snprintf(topdirenv, sizeof(topdirenv), "REFPERSYS_TOPDIR=%s", rps_topdirectory);
  putenv(topdirenv);
  static char fifoenv[256];
  if (!rps_fifo_prefix.empty())
    {
      snprintf(fifoenv, sizeof(fifoenv), "REFPERSYS_FIFO_PREFIX=%s", rps_fifo_prefix.c_str());
      putenv(fifoenv);
    }
} // end rps_extend_env




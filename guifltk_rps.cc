/****************************************************************
 * file guifltk_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the initial graphical user interface using FLTK 1.3
 *      (see fltk.org)
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2022 The Reflective Persistent System Team
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>

extern "C" std::string rps_dumpdir_str;
std::vector<char*> fltk_vector_arg_rps;

bool rps_fltk_gui;

Fl_Window* rps_fltk_mainwin;

class Fltk_Editor_rps : public Fl_Text_Editor {
  Fl_Text_Buffer*ed_tbuf;
public:
  Fltk_Editor_rps(int X,int Y,int W,int H);
  virtual ~Fltk_Editor_rps();
};				// end class Fltk_Editor_rps


Fltk_Editor_rps::Fltk_Editor_rps(int X,int Y,int W,int H)
  : Fl_Text_Editor(X,Y,W,H), ed_tbuf(nullptr) {
  ed_tbuf = new Fl_Text_Buffer();
  buffer (ed_tbuf);
} // end Fltk_Editor_rps::Fltk_Editor_rps

Fltk_Editor_rps::~Fltk_Editor_rps() {
  delete ed_tbuf;
} // end Fltk_Editor_rps::~Fltk_Editor_rps


int fltk_api_version_rps(void)
{
  return Fl::api_version();
}

void add_fltk_arg_rps(char*arg)
{
  if (fltk_vector_arg_rps.empty())
    fltk_vector_arg_rps.push_back((char*)rps_progname);
  RPS_ASSERT(!strncmp(arg,"--fltk",6));
  char*argtail = arg+6;
  char*colon = strchr(argtail, ':');
  if (colon)
    {
      *colon = (char)0;
      fltk_vector_arg_rps.push_back(argtail);
      fltk_vector_arg_rps.push_back(colon+1);
    }
  else
    fltk_vector_arg_rps.push_back(argtail);
} // end add_fltk_arg_rps



// This callback is invoked for dumping
static void
menub_dumpcbrps(Fl_Widget *w, void *)
{
  RPS_DEBUG_LOG(GUI, "menub_dumpcbrps start");
  char cwdbuf[128];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  getcwd(cwdbuf, sizeof(cwdbuf)-1);
  RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) GUI dump into %s\n"
             "... from current directory %s\n",
             (int)getpid(), rps_hostname(), rps_shortgitid,
             rps_dumpdir_str.c_str(), cwdbuf);
  rps_dump_into(rps_dumpdir_str);
} // end menub_dumpcbrps

// This callback is invoked for exiting after dump
static void
menub_exitcbrps(Fl_Widget *w, void *)
{
  char cwdbuf[128];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  getcwd(cwdbuf, sizeof(cwdbuf)-1);
  RPS_DEBUG_LOG(GUI, "menub_exitcbrps incomplete");
  RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) final dump into %s\n"
             "... from current directory %s\n",
             (int)getpid(), rps_hostname(), rps_shortgitid,
             rps_dumpdir_str.c_str(), cwdbuf);
  rps_dump_into(rps_dumpdir_str);
  exit(EXIT_SUCCESS);
#warning menub_dumpcbrps incomplete
} // end menub_exitcbrps

// This callback is invoked for quitting
static void
menub_quitcbrps(Fl_Widget *w, void *)
{
  RPS_DEBUG_LOG(GUI, "menub_quitcbrps incomplete");
  // we probably need a dialog to confirm quitting?
  RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) quit",
             (int)getpid(), rps_hostname(), rps_shortgitid);
  exit(EXIT_FAILURE);
#warning menub_quitcbrps incomplete
} // end menub_quitcbrps

void
guifltk_initialize_rps(void)
{
  Fl::args(fltk_vector_arg_rps.size(), fltk_vector_arg_rps.data());
  char titlbuf[128];
  memset(titlbuf, 0, sizeof(titlbuf));
  snprintf(titlbuf, sizeof(titlbuf), "refpersys-fltk/p%d-%s",
           (int)getpid(),
           rps_shortgitid);
  rps_fltk_mainwin = new Fl_Window(720, 460, titlbuf);
  Fl_Menu_Bar *menub = new Fl_Menu_Bar(0,0,720,20);
  menub->add("&App/&Dump", "^d", menub_dumpcbrps);
  menub->add("&App/e&Xit", "^x", menub_exitcbrps);
  menub->add("&App/&Quit", "^q", menub_quitcbrps);
#warning should create some Fltk_Editor_rps
  // ensure the editor follows the size of the mainwin
  rps_fltk_mainwin->end();
  int maxw = 3200, maxh = 1300;
  if (maxw > Fl::w())
    maxw = Fl::w()- 40;
  if (maxh > Fl::h())
    maxh = Fl::h() - 40;
  rps_fltk_mainwin->size_range(/*min dim w&h:*/ 330, 220,
      /*max dim w&h:*/ maxw, maxh,
      /*delta w&h:*/ 10, 10);
  rps_fltk_mainwin->show();
  RPS_DEBUG_LOG(GUI, "guifltk_initialize_rps: rps_fltk_mainwin@"
                << (void*)rps_fltk_mainwin
                << " titlbuf:" << titlbuf << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "guifltk_initialize_rps")
                << std::endl);
  RPS_DEBUG_LOG(GUI, "rps_fltk_mainwin@" << (void*)rps_fltk_mainwin);
} // end guifltk_initialize_rps


void
guifltk_run_application_rps (void)
{
  RPS_DEBUG_LOG(GUI, "guifltk_run_application_rps: rps_fltk_mainwin@"
                << (void*)rps_fltk_mainwin << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "guifltk_run_application_rps")
                << std::endl);
  Fl::run();
  RPS_DEBUG_LOG(GUI, "guifltk_run_application_rps ending");
} // end guifltk_run_application_rps

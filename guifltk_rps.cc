/****************************************************************
 * file guifltk_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the initial.
 graphical user interface using FLTK 1.3 (see fltk.org)
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

std::vector<char*> fltk_vector_arg_rps;

bool rps_fltk_gui;

Fl_Window* rps_fltk_mainwin;

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

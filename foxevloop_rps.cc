/****************************************************************
 * file foxevloop_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the high-level FOX event-loop related
 *      code. See https://fox-toolkit.org/
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

#include "headfox_rps.hh"



extern "C" const char rps_foxevloop_gitid[];
const char rps_foxevloop_gitid[]= RPS_GITID;

extern "C" const char rps_foxevloop_date[];
const char rps_foxevloop_date[]= __DATE__;

static std::atomic<bool> rps_running_fox;

extern "C" pthread_t rps_main_gui_pthread;
pthread_t rps_main_gui_pthread;
Rps_GuiPreferences rps_gui_pref;


bool
rps_is_main_gui_thread(void)
{
  return pthread_self() == rps_main_gui_pthread;
} // end rps_is_main_gui_thread

std::string
rps_fox_version(void)
{
  std::string r ("FOX");
  char buf[32];
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%ud.%ud.%ud",
           FX::fxversion[0], FX::fxversion[1], FX::fxversion[2]);
  return std::string{"FOX "} + buf;
} // end  rps_fox_version

void
rps_garbcoll_application(Rps_GarbageCollector&gc)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(gc.is_valid_garbcoll());
#warning incomplete rps_garbcoll_application
  RPS_WARNOUT("incomplete rps_garbcoll_application " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_garbcoll_application"));
} // end rps_garbcoll_application


void
rps_run_fox_gui(int &argc, char**argv)
{
  rps_main_gui_pthread = pthread_self();
  RPS_WARNOUT("incomplete rps_run_fox_gui argc=" << argc
              << " argv=" << Rps_Do_Output([=](std::ostream&out)
  {
    for (int ix=0; ix<argc; ix++)
      {
        out << " [" << ix << "]";
        if (argv[ix])
          out << "'" << argv[ix] << "'";
        else
          out << "*nil*";
      }
  })
      << std::endl
      << RPS_FULL_BACKTRACE_HERE(1, "rps_run_fox_gui"));
} // end rps_run_fox_gui

//// end of file foxevloop_rps.cc

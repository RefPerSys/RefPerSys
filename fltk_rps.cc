/****************************************************************
 * file fltk_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the FLTK graphical user interface related code. See http://fltk.org/
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


extern "C" const char rps_fltk_gitid[];
const char rps_fltk_gitid[]= RPS_GITID;

extern "C" const char rps_fltk_date[];
const char rps_fltk_date[]= __DATE__;

void
rps_garbcoll_application(Rps_GarbageCollector&gc)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  RPS_ASSERT(gc.is_valid_garbcoll());
#warning incomplete rps_garbcoll_application
  RPS_WARNOUT("incomplete rps_garbcoll_application " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_garbcoll_application"));
} // end rps_garbcoll_application

bool
rps_is_main_gui_thread(void)
{
#warning unimplemented rps_is_main_gui_thread
  RPS_FATAL("unimplemented rps_is_main_gui_thread");
} // end rps_is_main_gui_thread



void
rps_run_fltk_gui(int &argc, char**argv)
{
  for (int ix=0; ix<argc; ix++)
    RPS_DEBUG_LOG(REPL, "FLTK GUI arg [" << ix << "]: " << argv[ix]);
  RPS_FATAL("unimplemented rps_run_fltk_gui");
} // rps_run_fltk_gui

//////////////////////////////////////// end of file fltk_rps.cc

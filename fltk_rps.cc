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



/*********** 
 * Temporarily kept constant objects
 *
 * For a few weeks in may or june 2020, we mention here as comments
 * several constant objects.
 *
 *    rpskob_0rgijx7CCnq041IZEd, rpskob_0TwK4TkhEGZ03oTa5m,
 *    rpskob_1568ZHTl0Pa00461I2, rpskob_18DO93843oX02UWzq6,
 *    rpskob_1wihX3eWD9o00QnxUX, rpskob_1DUx3zfUzIb04lqNVt,
 *    rpskob_1Win5yzaf1L02cBUlV, rpskob_2KnFhlj8xW800kpgPt,
 *    rpskob_33DFyPOJxbF015ZYoi, rpskob_41OFI3r0S1t03qdB2E,
 *    rpskob_42cCN1FRQSS03bzbTz, rpskob_4ojpzRzyRWz02DNWMe,
 *    rpskob_4x9jd2yAe8A02SqKAx, rpskob_52zVxP3mTue034OWsD,
 *    rpskob_5nSiRIxoYQp00MSnYA, rpskob_6Wi00FwXYID00gl9Ma,
 *    rpskob_7oa7eIzzcxv03TmmZH, rpskob_8lKdW7lgcHV00WUOiT,
 *    rpskob_8KJHUldX8GJ03G5OWp, rpskob_9uwZtDshW4401x6MsY, etc..
 *
 ************/


//////////////////////////////////////// end of file fltk_rps.cc

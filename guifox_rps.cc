/****************************************************************
 * file guifox_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the initial graphical user interface using FOX toolkit 1.7.80
 *      (see fox-toolkit.org)
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

#include "fx.h"

bool rps_fox_gui;

std::vector<char*> fox_args_vect_rps;

class Fox_Main_Window_Rps: public FXMainWindow {
  FXDECLARE(Fox_Main_Window_Rps);
protected:
  FXMenuBar* fxmwin_menubar;
  Fox_Main_Window_Rps() {};
public:
  Fox_Main_Window_Rps(FXApp*);
  virtual ~Fox_Main_Window_Rps();
};				// end Fox_Main_Window_Rps

Fox_Main_Window_Rps::Fox_Main_Window_Rps(FXApp* ap):
  FXMainWindow(ap, FXString("foxrepersys")) {
} // end Fox_Main_Window_Rps::Fox_Main_Window_Rps

Fox_Main_Window_Rps::~Fox_Main_Window_Rps() {
  delete fxmwin_menubar;
} // end Fox_Main_Window_Rps::~Fox_Main_Window_Rps

// Map
FXDEFMAP(Fox_Main_Window_Rps) Fox_Main_Window_Map_rps[]={
  };

//  implementation
FXIMPLEMENT(Fox_Main_Window_Rps,FXMainWindow,Fox_Main_Window_Map_rps,ARRAYNUMBER(Fox_Main_Window_Map_rps))







static void delete_fox_app_on_exit_rps(void)
{
  FXApp* a= FXApp::instance();
  if (a)
    delete a;
} // end delete_fox_app_on_exit_rps

void
add_fox_arg_rps(char*arg)
{
  if (fox_args_vect_rps.empty())
    fox_args_vect_rps.push_back((char*)rps_progname);
  if (arg)
    fox_args_vect_rps.push_back(arg);
} // end add_fox_arg_rps

void
guifox_initialize_rps(void)
{
  if (fox_args_vect_rps.empty())
    fox_args_vect_rps.push_back((char*)rps_progname);
  fox_args_vect_rps.push_back(nullptr);
  /// the sole FXApp is accessible as FXApp::instance()
  auto ap = new FXApp("foxrefpersys", "refpersys.org");
  static int foxargc = fox_args_vect_rps.size()-1;
  ap->init(foxargc,fox_args_vect_rps.data());
  RPS_DEBUG_LOG(GUI, "guifox_initialize_rps ap@" << (void*)ap
		<< RPS_FULL_BACKTRACE_HERE(1, "guifox_initialize_rps"));
  atexit(delete_fox_app_on_exit_rps);
} // end guifox_initialize_rps


void
guifox_run_application_rps(void)
{
  auto mainwin=new Fox_Main_Window_Rps(FXApp::instance());
  RPS_DEBUG_LOG(GUI, "guifox_initialize_rps app@" << FXApp::instance()
		<< " mainwin@" << (void*)mainwin
		<< RPS_FULL_BACKTRACE_HERE(1, "guifox_initialize_rps"));
} // end of guifox_run_application_rps

//// end of file guifox_rps.cc

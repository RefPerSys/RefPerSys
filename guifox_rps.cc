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
  unsigned fxmwin_rank;
  static unsigned fxmwin_counter;
  Fox_Main_Window_Rps() {};
public:
  Fox_Main_Window_Rps(FXApp*);
  void output(std::ostream&out) const;
  virtual ~Fox_Main_Window_Rps();
  enum {
    ID_XXX=FXMainWindow::ID_LAST,
    /// TODO: we probably need our own IDs....
    ID_LAST
    };
#warning missing onCmdXXX(FXObject*,FXSelector,void*) declarations in Fox_Main_Window_Rps
};				// end Fox_Main_Window_Rps

std::ostream& operator << (std::ostream&out, const Fox_Main_Window_Rps&mwin)
{
  mwin.output(out);
  return out;
} // end operator << (std::ostream&out, const Fox_Main_Window_Rps&mwin)

unsigned Fox_Main_Window_Rps::fxmwin_counter=0;

void
Fox_Main_Window_Rps::output(std::ostream&out) const {
  out << "fox_main_window#" << fxmwin_rank
      << "{x=" << getX() << ",y=" << getY()  << ",w=" << getWidth()
      << ",h=" << getHeight();
  if (!isEnabled())
    out <<";disabled";
  if (!isActive())
    out << ";inactive";
  if (hasFocus())
    out << ";focused";
  if (shown())
    out << ";shown";
  else
    out << ";hidden";
  out << "}";
} // end Fox_Main_Window_Rps::output

Fox_Main_Window_Rps::Fox_Main_Window_Rps(FXApp* ap):
  FXMainWindow(ap, FXString("foxrepersys"),
	       (FXIcon*)nullptr,
	       (FXIcon*)nullptr,
	       DECOR_ALL,
	       100+((int)getpid()%256), //:x
	       100+((int)getpid()%256), //:y
	       600, //width
	       300 //height
	       ),
  fxmwin_menubar(nullptr),
  fxmwin_rank(++fxmwin_counter)
{
#warning Fox_Main_Window_Rps::Fox_Main_Window_Rps should build the fxmwin_menubar
  auto topdock=new FXDockSite(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  auto dragshell1=new FXToolBarShell(this,FRAME_RAISED);
  //// TODO: create the fxmwin_menubar
  fxmwin_menubar = new FXMenuBar(topdock,dragshell1, //
				 LAYOUT_DOCK_NEXT    //
				 |LAYOUT_SIDE_TOP    //
				 |LAYOUT_FILL_X      //
				 |FRAME_RAISED);
  RPS_DEBUG_LOG(GUI, "Fox_Main_Window_Rps::Fox_Main_Window_Rps#"
		<< fxmwin_rank
		<<" ap@"
		<< (void*)ap << " this@" << (void*)this
		<< " fxmwin_menubar@" << (void*)fxmwin_menubar
		<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "Fox_Main_Window_Rps::Fox_Main_Window_Rps"));
} // end Fox_Main_Window_Rps::Fox_Main_Window_Rps


Fox_Main_Window_Rps::~Fox_Main_Window_Rps() {
  RPS_DEBUG_LOG(GUI, "Fox_Main_Window_Rps::~Fox_Main_Window_Rps#"<<
		fxmwin_rank
		<< " this@" << (void*)this
		<< " fxmwin_menubar@" << (void*)fxmwin_menubar
		<< std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "Fox_Main_Window_Rps::~Fox_Main_Window_Rps"));
  delete fxmwin_menubar;
} // end Fox_Main_Window_Rps::~Fox_Main_Window_Rps

// Map
FXDEFMAP(Fox_Main_Window_Rps) Fox_Main_Window_Map_rps[]={
  /// FXMAPFUNC(<selector>,<id>,<handler>),
  };

//  implementation
FXIMPLEMENT(Fox_Main_Window_Rps,FXMainWindow,Fox_Main_Window_Map_rps,ARRAYNUMBER(Fox_Main_Window_Map_rps))


#warning missing Fox_Main_Window_Rps::onCmdXXX implementations
//// missing handlers, like
// long Fox_Main_Window_Rps::onCmdXXX(FXObject*,FXSelector,void*){
//  return 1;
//  }



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
  auto app = FXApp::instance();
  auto mainwin=new Fox_Main_Window_Rps(FXApp::instance());
  RPS_DEBUG_LOG(GUI, "guifox_run_application_rps app@" << (void*)app
		<< " mainwin@" << (void*)mainwin
		<< RPS_FULL_BACKTRACE_HERE(1, "guifox_run_application_rps"));
  mainwin->layout();
  RPS_DEBUG_LOG(GUI, "guifox_run_application_rps app@" << (void*)app
		<< " mainwin@" << (void*)mainwin << " after layout");
  mainwin->show();
  RPS_DEBUG_LOG(GUI, "guifox_run_application_rps app@" << (void*)app
		<< " mainwin=" << (*mainwin) << " after show");
  app->create();
  RPS_DEBUG_LOG(GUI, "guifox_initialize_rps created app@" << (void*)app << "before run"
		<< RPS_FULL_BACKTRACE_HERE(1, "guifox_run_application_rps before run"));
  int r = app->run();
  RPS_DEBUG_LOG(GUI, "guifox_run_application_rps did run app@" << (void*)app
		<< " r=" << r <<std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "ending guifox_run_application_rps"));
} // end of guifox_run_application_rps

//// end of file guifox_rps.cc

/****************************************************************
 * file foxsimpwin_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the low-level FOX graphical user interface related
 *      code, a single simple window. See http://fox-toolkit.org/
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



extern "C" const char rps_foxsimpwin_gitid[];
const char rps_foxsimpwin_gitid[]= RPS_GITID;

extern "C" const char rps_foxsimpwin_date[];
const char rps_foxlo_simpwin_date[]= __DATE__;

std::string rps_gui_dump_dir_str;

void
rps_set_gui_dump_dir(const std::string&str)
{
  RPS_DEBUG_LOG(GUI, "rps_set_gui_dump_dir " << str);
  rps_gui_dump_dir_str = str;
} // end rps_set_gui_dump_dir


FXDEFMAP(RpsGui_FoxSimpleWindow) RpsGui_FoxMapSimpleWindow[]=
{
  FXMAPFUNC(SEL_COMMAND,RpsGui_FoxSimpleWindow::ID_ABOUT,
            RpsGui_FoxSimpleWindow::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,RpsGui_FoxSimpleWindow::ID_HELP,
            RpsGui_FoxSimpleWindow::onCmdHelp),
};
// Object implementation
FXIMPLEMENT(RpsGui_FoxSimpleWindow,FXMainWindow,RpsGui_FoxMapSimpleWindow,ARRAYNUMBER(RpsGui_FoxMapSimpleWindow));


// Show help window, create it on-the-fly
long
RpsGui_FoxSimpleWindow::onCmdHelp(FXObject*,FXSelector,void*)
{
  RPS_WARNOUT("unimplemented RpsGui_FoxSimpleWindow::onCmdHelp"
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_FoxSimpleWindow::onCmdHelp"));
#warning unimplemented RpsGui_FoxSimpleWindow::onCmdHelp
  return 1;
} // end RpsGui_FoxSimpleWindow::onCmdHelp

// Show help window, create it on-the-fly
long
RpsGui_FoxSimpleWindow::onCmdAbout(FXObject*,FXSelector,void*)
{
  RPS_WARNOUT("unimplemented RpsGui_FoxSimpleWindow::onCmdAbout"
              << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_FoxSimpleWindow::onCmdAbout"));
#warning unimplemented RpsGui_FoxSimpleWindow::onCmdAbout
  return 1;
} // end RpsGui_FoxSimpleWindow::onCmdAbout

RpsGui_FoxSimpleWindow::RpsGui_FoxSimpleWindow(RpsGui_FoxApplication*app)
  : FXMainWindow(app, "refpersys-simple")
{
  RPS_DEBUG_LOG(GUI, "start constr RpsGui_FoxSimpleWindow this@" << (void*)this
                << " app:" << app
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_FoxSimpleWindow constr"));
} // end RpsGui_FoxSimpleWindow::RpsGui_FoxSimpleWindow




/// end of file foxsimpwin_rps.cc

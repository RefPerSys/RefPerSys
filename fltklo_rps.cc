/****************************************************************
 * file fltklo_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the low-level FLTK graphical user interface related
 *      code. See http://fltk.org/
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

#include "fltkhead_rps.hh"



extern "C" const char rps_fltklo_gitid[];
const char rps_fltklo_gitid[]= RPS_GITID;

extern "C" const char rps_fltklo_date[];
const char rps_fltklo_date[]= __DATE__;


int
RpsGui_Window::handle(int evtype)
{
  /* by default, FLTK understands the ESC key as
     closing. See https://www.fltk.org/doc-1.4/FAQ.html */
  if ((evtype == FL_KEYDOWN || evtype == FL_KEYUP))
    {
      if (Fl::event_key() == FL_Escape)
        {
          RPS_DEBUG_LOG(GUI, "GuiWindow ignore escape");
          return 1;
        }
    }
  return Fl_Double_Window::handle(evtype);
} // end RpsGui_Window::handle

Rps_ObjectRef
RpsGui_Window::owning_object(Rps_CallFrame*callframe) const
{
  return Rps_ObjectRef::find_object_by_oid(callframe, owning_oid());
} // end RpsGui_Window::owning_object

//////////////////////////////////////// end of file fltklo_rps.cc

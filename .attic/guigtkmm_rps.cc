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

/// /usr/include/gtkmm-3.0/gtkmm.h
#include <gtkmm.h>

bool rps_gtkmm_gui;

Glib::RefPtr<Gtk::Application> gtkmm_app_rps;

const char*
gtkmm_version_rps(void)
{
  static char vbuf[48];
  if (!vbuf[0])
    snprintf(vbuf, sizeof(vbuf), "gtkmm-%d.%d.%d",
             GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION);
  return vbuf;
} // end gtkmm_version_rps


void
add_gtkmm_arg_rps(char*arg)
{
  RPS_FATALOUT("unimplemented add_gtkmm_arg_rps arg:" << arg);
#warning add_gtkmm_arg_rps unimplemented
} // end add_gtkmm_arg_rps


void
gtkmm_initialize_app_rps(void)
{
  gtkmm_app_rps =  Gtk::Application::create("org.refpersys.gtkmm");
#warning gtkmm_initialize_app_rps incomplete
} // end gtkmm_initialize_app_rps

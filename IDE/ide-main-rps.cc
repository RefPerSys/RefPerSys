/****************************************************************
 * file ide-main-rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System IDE
 *
 *      It has the main function and related, program option parsing,
 *      code.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      team@refpersys.org
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

#include "ide-refpersys.hh"

#include "cxxopts.hpp"

///inspired by https://www.fltk.org/doc-1.3/basics.html

static Fl_Window*
makewin_iderps(void)
{
    Fl_Window *window = new Fl_Window(540,180);
    Fl_Box *box = new Fl_Box(20,40,440,100,"Hello, RefPerSys!");
    box->box(FL_UP_BOX);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labelsize(36);
    box->labeltype(FL_SHADOW_LABEL);
    window->end();
    return window;
} // end makewin_iderps

static double rps_start_monotonic_time;
double rps_elapsed_real_time(void)
{
  return rps_monotonic_real_time() - rps_start_monotonic_time;
}

int main(int argc, char **argv) {
  rps_start_monotonic_time = rps_monotonic_real_time();
    auto window = makewin_iderps();
    window->show(argc, argv);
    return Fl::run();
} // end main

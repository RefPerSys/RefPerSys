/****************************************************************
 * file repl_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the read-eval-print-loop code using GNU readline
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


extern "C" const char rps_repl_gitid[];
const char rps_repl_gitid[]= RPS_GITID;

extern "C" const char rps_repl_date[];
const char rps_repl_date[]= __DATE__;

void
rps_read_eval_print_loop(int &argc, char **argv)
{
  for (int ix=0; ix<argc; ix++)
    RPS_DEBUG_LOG(REPL, "REPL arg [" << ix << "]: " << argv[ix]);
#warning incomplete rps_read_eval_print_loop
  RPS_WARNOUT("incomplete rps_read_eval_print_loop " << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_read_eval_print_loop"));
} // end of rps_read_eval_print_loop

// end of file repl_rps.cc

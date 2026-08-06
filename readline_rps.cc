/****************************************************************
 * file readline_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 * It implements the GNU readline completion function, triggered by
 * the TAB or ESC keys on your keyboard.
 * Author(s):
 *      Basile Starynkevitch, France    <basile@starynkevitch.net>
 *      Niklas Rozencrantz, Sweden     <niklasr@protonmail.com>
 *
 *
 *      © Copyright (C) 2026 - 2026 The Reflective Persistent System Team
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
#include "readline/readline.h"

extern "C" const char rps_readline_gitid[];
const char rps_readline_gitid[]= RPS_GITID;


extern "C" const char rps_readline_shortgitid[];
const char rps_readline_shortgitid[]= RPS_SHORTGITID;


extern "C" const char rps_readline_basename[];
const char rps_readline_basename[]= RPS_BASENAME;

extern "C" const char rps_readline_baseid[];
const char rps_readline_baseid[]= RPS_BASEID;

extern "C" int rps_readline_tab(int, int);
extern "C" int rps_readline_esc(int, int);

/// initialization function called early
void
rps_readline_initialize(void)
{
  rl_readline_name = "refpersys";
  rl_initialize();
  rl_bind_key('\t', rps_readline_tab);
  rl_bind_key('\e', rps_readline_esc);
  RPS_INFORMOUT("readline initialized " << rl_library_version);
#warning incomplete rps_readline_initialize
} // end rps_readline_initialize

int
rps_readline_tab(int cnt, int key)
{
  RPS_UNIQUE_BREAKPOINT();
  RPS_ASSERT(rl_line_buffer);
  RPS_ASSERT(key=='\t');
  if (rl_point <= 0)		// first column
    return 1;
#warning incomplete rps_readline_tab
  return 0;
} // end rps_readline_tab

int
rps_readline_esc(int cnt, int key)
{
  RPS_UNIQUE_BREAKPOINT();
  RPS_ASSERT(rl_line_buffer);
  RPS_ASSERT(key=='\e');
  if (rl_point <= 0)		// first column
    return 1;
#warning incomplete rps_readline_esc
  return 0;
} // end rps_readline_esc

/// end of readline_rps.cc

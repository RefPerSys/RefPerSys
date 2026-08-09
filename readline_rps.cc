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


/// probably the readline prompt should be in a static buffer?
static char rps_readline_buf_prompt[64];
static std::mutex rps_readline_mtx_prompt;

void
rps_readline_set_prompt(const char*p)
{
  std::lock_guard<std::mutex> gu(rps_readline_mtx_prompt);
  memset(rps_readline_buf_prompt, 0, sizeof(rps_readline_buf_prompt));
  if (!p)
    return;
  RPS_ASSERT(strlen(p) < sizeof(rps_readline_buf_prompt));
  if (u8_check((const uint8_t*)p, strlen(p)))
    {
      RPS_UNIQUE_BREAKPOINT();
      RPS_WARNOUT("invalid UTF8 string " << (char*)p);
      return;
    };
  strncpy(rps_readline_buf_prompt, p, sizeof(rps_readline_buf_prompt));
} // end rps_readline_set_prompt

const std::string
rps_readline_fetch_string_prompt(void)
{
  std::lock_guard<std::mutex> gu(rps_readline_mtx_prompt);
  return std::string(rps_readline_buf_prompt);
} // end rps_readline_fetch_prompt

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
  if (rl_point <= 0)    // first column
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
  if (rl_point <= 0)    // first column
    return 1;
#warning incomplete rps_readline_esc
  return 0;
} // end rps_readline_esc

void
Rps_ReadlineTokenSource::fill_current_line_buffer(void)
{
#warning unimplemented Rps_ReadlineTokenSource::fill_current_line_buffer
  RPS_UNIQUE_BREAKPOINT();
  std::string prompt = rps_readline_fetch_string_prompt();
  RPS_DEBUG_LOG(REPL, "readline prompt=" << Rps_QuotedC_String(prompt));
  char* rl = readline(prompt.c_str());
  RPS_DEBUG_LOG(REPL, "did readline " << Rps_QuotedC_String(rl));
  RPS_FATALOUT("incomplete Readline fill_current_line_buffer @"
               << (void*)this << " rl=" << rl);
#warning incomplete Rps_ReadlineTokenSource::fill_current_line_buffer
} // end Rps_ReadlineTokenSource::fill_current_line_buffer


void
Rps_ReadlineTokenSource::output(std::ostream&out, unsigned depth,
                                unsigned maxdepth) const
{
  std::lock_guard<std::recursive_mutex> gu(toksrc_mtx);
  if (depth > maxdepth && &out != &std::cout &&
      &out != &std::cerr && &out != &std::clog)
    RPS_WARNOUT("Rps_ReadlineTokenSource " << name()
                << " depth=" << depth
                << " greater than maxdepth=" << maxdepth);
  out << "ReadlineTokenSource:" << name() << ".S#" << unique_number()
      << '@' << position_str() << " tok.cnt:" << token_count();
}; // end Rps_ReadlineTokenSource::output


Rps_ReadlineTokenSource::Rps_ReadlineTokenSource()
  : Rps_TokenSource(std::string{"*readline*"})
{
  static std::atomic_int cnt;
  RPS_ASSERT(cnt==0);
  RPS_UNIQUE_BREAKPOINT();
  cnt++;
} // end Rps_ReadlineTokenSource::Rps_ReadlineTokenSource



Rps_ReadlineTokenSource::~Rps_ReadlineTokenSource()
{
  RPS_UNIQUE_BREAKPOINT();
} // end Rps_ReadlineTokenSource destructor




bool
Rps_ReadlineTokenSource::get_line(void)
{
  std::lock_guard<std::recursive_mutex> gu(toksrc_mtx);
  RPS_UNIQUE_BREAKPOINT();
#warning Rps_ReadlineTokenSource::get_line unimplemented
  // TODO: use readline()
  starting_new_input_line();
  return true;
} // end Rps_ReadlineTokenSource::get_line

bool
Rps_ReadlineTokenSource::reached_end(void) const
{
#warning Rps_ReadlineTokenSource::reached_end unimplemented
  return false;
} // end Rps_ReadlineTokenSource::reached_end

void
Rps_ReadlineTokenSource::display(std::ostream&out) const
{
  std::lock_guard<std::recursive_mutex> gu(toksrc_mtx);
  output(out, 0, Rps_Value::debug_maxdepth);
  out << std::endl;
  if (reached_end())
    out <<  "°";
} // end Rps_ReadlineTokenSource::display

/// end of readline_rps.cc

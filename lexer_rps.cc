/****************************************************************
 * file lexer_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the lexer support for the Read Eval Print Loop
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2021 The Reflective Persistent System Team
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
#include "readline/history.h"

#include <wordexp.h>

extern "C" const char rps_lexer_gitid[];
const char rps_lexer_gitid[]= RPS_GITID;

extern "C" const char rps_lexer_date[];
const char rps_lexer_date[]= __DATE__;

Rps_TokenSource::Rps_TokenSource(std::string name)
  : toksrc_name(name), toksrc_line(0), toksrc_col(0), toksrc_linebuf{}
{
} // end Rps_TokenSource::Rps_TokenSource

Rps_TokenSource::~Rps_TokenSource()
{
  toksrc_name.clear();
  toksrc_line= -1;
  toksrc_col= -1;
  toksrc_linebuf.clear();
} // end Rps_TokenSource::~Rps_TokenSource

Rps_StreamTokenSource::Rps_StreamTokenSource(std::string path)
  : Rps_TokenSource(""), toksrc_input_stream()
{
  wordexp_t wx= {};
  int err = wordexp(path.c_str(), &wx, WRDE_SHOWERR);
  if (err)
    {
      RPS_WARNOUT("stream token source for '" << Rps_Cjson_String(path) << "' failed: error#" << err);
      throw std::runtime_error(std::string{"bad stream token source:"} + path);
    }
  if (wx.we_wordc == 0)
    {
      RPS_WARNOUT("no stream token source for '" << (Rps_Cjson_String(path)) << "'");
      throw std::runtime_error(std::string{"no stream token source:"} + path);
    }
  else if (wx.we_wordc > 1)
    {
      RPS_WARNOUT("ambiguous stream token source for '" << path << "' expanded to "
                  << wx.we_wordv[0] << " and " << wx.we_wordv[1]
                  << ((wx.we_wordc>2)?" etc...":" files"));
      throw std::runtime_error(std::string{"ambiguous stream token source:"} + path);
    };
  char* curword = wx.we_wordv[0];
  toksrc_input_stream.open(curword);
  set_name(std::string(curword));
} // end Rps_StreamTokenSource::Rps_StreamTokenSource


Rps_StreamTokenSource::~Rps_StreamTokenSource()
{
  toksrc_input_stream.close();
} // end ps_StreamTokenSource::~Rps_StreamTokenSource

bool
Rps_StreamTokenSource::get_line(void)
{
  std::getline( toksrc_input_stream, toksrc_linebuf);
  if (!toksrc_input_stream && toksrc_linebuf.empty()) return false;
  return true;
} // end Rps_StreamTokenSource::get_line



Rps_CinTokenSource::Rps_CinTokenSource()
  : Rps_TokenSource("-")
{
};				// end Rps_CinTokenSource::Rps_CinTokenSource

Rps_CinTokenSource::~Rps_CinTokenSource()
{
};	// end Rps_CinTokenSource::~Rps_CinTokenSource

bool
Rps_CinTokenSource::get_line(void)
{
  std::getline(std::cin, toksrc_linebuf);
  if (!std::cin && toksrc_linebuf.empty()) return false;
  return true;
} // end Rps_CinTokenSource::get_line

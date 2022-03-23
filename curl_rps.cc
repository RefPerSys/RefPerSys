/****************************************************************
 * file curl_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the libCURL related code for web related things. See
 *      https:///curl.haxx.se/libcurl/
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

#include "curl/curl.h"


extern "C" const char rps_curl_gitid[];
const char rps_curl_gitid[]= RPS_GITID;

extern "C" const char rps_curl_date[];
const char rps_curl_date[]= __DATE__;

std::string
rps_curl_version(void)
{
  char curlgitbuf[16];
  memset (curlgitbuf, 0, sizeof(curlgitbuf));
  strncpy(curlgitbuf, rps_curl_gitid, (3*sizeof(curlgitbuf))/4);
  std::string res("CURL ");
  char* cv = curl_version ();
  int nc = 0;
  for (char *pc = cv; *pc; pc++)
    {
      if (isspace(*pc))
        res += "\n\t";
      else
        res += *pc;
    }
  res += " git ";
  res += curlgitbuf;
  return res;
} // end rps_curl_version


/// end of file curl_rps.cc

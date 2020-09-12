/****************************************************************
 * file httpweb_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System. See refpersys.org
 *
 *      It has the web interface code, using libonion
 *      from https://github.com/davidmoreno/onion
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
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

#include "headweb_rps.hh"

/// our onion web server
Onion::Onion rps_onion_server;

static char* rps_onion_serverhost;
/// Called from main with an argument like "localhost:9090". Should
/// initialize the data structures to serve web requests.
void
rps_web_initialize_service(const char*servarg)
{
  RPS_ASSERT(servarg != nullptr);
  RPS_DEBUG_LOG(WEB, "rps_web_initialize_service start servarg=" << servarg
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_web_initialize_service"));
  int portnum = -1;
  static char serverbuf[80];
  memset(serverbuf, 0, sizeof(serverbuf));
  if (sscanf(servarg, "%72[a-zA-Z0-9.-]:%d",
             serverbuf, &portnum)>=2 && portnum>0)
    {
      if (serverbuf[0])
        rps_onion_server.setHostname(std::string{serverbuf});
      rps_onion_server.setPort(portnum);
      RPS_INFORMOUT("rps_web_initialize_service initialized Onion webserver on "
                    << serverbuf << ":" << portnum);
    }
  else
    RPS_FATALOUT("rps_web_initialize_service: bad server " << servarg);
} // end rps_web_initialize_service


void
rps_run_web_service()
{
  RPS_FATAL("unimplemented rps_run_web_service");
  /** we should add some URL, see examples in
      https://github.com/davidmoreno/onion/tree/master/tests/08-cpp/
      and
      https://github.com/davidmoreno/onion/tree/master/examples/cpp
      ...
  ***/
#warning rps_run_web_service unimplemented, should add URLs
} // end rps_run_web_service

///////// end of file web_rps.cc

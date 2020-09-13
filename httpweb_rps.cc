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

static const char* rps_onion_serverarg;
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
  rps_onion_serverarg = servarg;
} // end rps_web_initialize_service


void
rps_run_web_service()
{
  /** we should add some URL, see examples in
      https://github.com/davidmoreno/onion/tree/master/tests/08-cpp/
      and
      https://github.com/davidmoreno/onion/tree/master/examples/cpp
      ...
  ***/
#warning rps_run_web_service unimplemented, should add URLs and handlers
  /***
   *  The handler of some URLs would create a temporary object whose
   *  payload would be some Rps_PayloadWebex....  For other URL the
   *  handler should deliver static contents... We might consider
   *  having some templates, in the libonion sense...
   **/
  /// TODO: some Onion::Url should be declared here... see README.md
  Onion::Url rooturl(&rps_onion_server);
  rooturl.add("/img/refpersys_logo.svg",
              [&](Onion::Request &req, Onion::Response &res)
  {
    std::ifstream infil("webroot/img/refpersys_logo.svg");
    int nblin=0;
    for (std::string line; std::getline(infil, line); nblin++)
      res << line;
    RPS_DEBUG_LOG(WEB, "rps_run_web_service: sent "
                  << nblin << " lines for /img/refpersys_logo.svg");
    return OCS_PROCESSED;
  });
  ///
  /// TODO: Conventionally, URLs containing either .. or README.md
  /// should not be served.
} // end rps_run_web_service

void
Rps_PayloadWebex::gc_mark(Rps_GarbageCollector&gc) const
{
#warning Rps_PayloadWebex::gc_mark unimplemented
  RPS_FATALOUT("Rps_PayloadWebex::gc_mark unimplemented owner=" << owner());
} // end Rps_PayloadWebex::gc_mark

void
Rps_PayloadWebex::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
#warning Rps_PayloadWebex::dump_scan unimplemented
  RPS_FATALOUT("Rps_PayloadWebex::dump_scan unimplemented owner=" << owner());
} // end Rps_PayloadWebex::dump_scan


void
Rps_PayloadWebex::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_vectob in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
#warning Rps_PayloadWebex::dump_json_content unimplemented, should never be called
  RPS_FATALOUT("Rps_PayloadWebex::dump_scan unimplemented owner=" << owner());
} // end Rps_PayloadWebex::dump_json_content

Rps_PayloadWebex::Rps_PayloadWebex(Rps_ObjectZone*ownerobz,Onion::Request&req,Onion::Response&resp)
  : Rps_Payload(Rps_Type::PaylWebex, ownerobz)
{
  RPS_ASSERT(ownerobz && ownerobz->stored_type() == Rps_Type::Object);
#warning Rps_PayloadWebex::Rps_PayloadWebex unimplemented
  RPS_FATALOUT("Rps_PayloadWebex::Rps_PayloadWebex unimplemented owner=" << owner());
} // end Rps_PayloadWebex::Rps_PayloadWebex


Rps_PayloadWebex::~Rps_PayloadWebex()
{
#warning Rps_PayloadWebex::~Rps_PayloadWebex unimplemented
  RPS_FATALOUT("Rps_PayloadWebex::~Rps_PayloadWebex unimplemented owner=" << owner());
} // end  Rps_PayloadWebex::~Rps_PayloadWebex


///////// end of file httpweb_rps.cc

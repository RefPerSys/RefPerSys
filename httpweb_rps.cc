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
  if (!strcmp(servarg, "."))
    servarg = RPS_DEFAULT_WEB_HOST_PORT;
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
                    << serverbuf << ":" << portnum
                    << " using libonion " << onion_version());
    }
  else
    RPS_FATALOUT("rps_web_initialize_service: bad server " << servarg);
  rps_onion_serverarg = servarg;
} // end rps_web_initialize_service


void
rps_run_web_service()
{
  RPS_DEBUG_LOG(WEB, "start of rps_run_web_service from "
                << rps_current_pthread_name()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_web_service")
                << std::endl);
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
  /// FIXME: use rps_serve_onion_web here
  Onion::Url rooturl(&rps_onion_server);
  /// set the error handler
  auto errhandlerfun =
    [&](Onion::Request& req, Onion::Response&resp)->onion_connection_status
  {
    const std::string reqpath = req.path();
    const onion_request_flags reqflags = req.flags();
    const unsigned reqmethnum = reqflags & OR_METHODS;
    const char* reqmethname = onion_request_methods[reqmethnum];
    RPS_DEBUG_LOG(WEB, "Onion-internal-error from "
                  << rps_current_pthread_name()
                  << " for "
                  << reqmethname
                  << " of "
                  << reqpath
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "RefPerSys onion-internal-error")
                 );

    RPS_WARNOUT("ONION internal error for web request "
                << reqmethname
                << " of "
                << reqpath);

    resp << "<!DOCTYPE html>\n"
         << "<html><head><title>RefPerSys error</title></head>\n"
         << "<body><p><b>Backtrace:</b><br/><tt>";
    {
      std::ostringstream outs;
      outs  <<  RPS_FULL_BACKTRACE_HERE(1, "RefPerSys onion-internal-error");
      std::string outstr = outs.str();
      resp << Rps_Html_Nl2br_String(outstr);
    }
    resp << "</tt></p>\n";

    resp << "<p>For <tt>"
         << reqmethname << "</tt> of <tt>"
         << reqpath
         << "</tt></p>\n"
         << "</body></html>"
         << std::endl;

    return OCS_PROCESSED;
  };

  auto errh = Onion::Handler::make<Onion::HandlerFunction>(errhandlerfun);
  RPS_ASSERT(errh);
  rps_onion_server.setInternalErrorHandler(errh.get());

#warning we should use rps_serve_onion_web in rps_run_web_service
  rooturl.add("^",
              [&](Onion::Request &req, Onion::Response &res)
  {
    RPS_DEBUG_LOG(WEB, "𝜦-rps_run_web_service req@" << (void*)&req
                  << " res@" << (void*)&res
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "𝜦-rps_run_web_service"));
    auto onstat = rps_serve_onion_web((Rps_Value)(nullptr), &rooturl, &req, &res);
    RPS_DEBUG_LOG(WEB, "𝜦-rps_run_web_service onstat#" << (int) onstat);
    return onstat;
  });
  RPS_DEBUG_LOG(WEB, "rps_run_web_service added 𝜦, listening to onion server @" << &rps_onion_server);
  rps_onion_server.listen();
  ///
  /// TODO: Conventionally, URLs containing either .. or README.md
  /// should not be served.
  RPS_DEBUG_LOG(WEB, "end rps_run_web_service" << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_run_web_service-end"));
} // end rps_run_web_service

void
Rps_PayloadWebex::gc_mark(Rps_GarbageCollector&gc) const
{
  if (webex_state)
    webex_state.gc_mark(gc);
} // end Rps_PayloadWebex::gc_mark

void
Rps_PayloadWebex::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  // we don't persist the webex_state, so...
  return;
} // end Rps_PayloadWebex::dump_scan


void
Rps_PayloadWebex::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_vectob in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  // we don't persist the webex_state, so...
  return;
} // end Rps_PayloadWebex::dump_json_content

Rps_PayloadWebex::Rps_PayloadWebex(Rps_ObjectZone*ownerobz,uint64_t reqnum,Onion::Request&req,Onion::Response&resp)
  : Rps_Payload(Rps_Type::PaylWebex, ownerobz),
    webex_reqnum(reqnum),
    webex_startim(rps_monotonic_real_time()),
    webex_requ(&req),
    webex_resp(&resp),
    webex_state(nullptr),
    webex_numstate(0),
    webex_indent(0)
{
  RPS_ASSERT(ownerobz && ownerobz->stored_type() == Rps_Type::Object);
  if (RPS_DEBUG_ENABLED(WEB))
    {
      char thrname[24];
      memset(thrname, 0, sizeof(thrname));
      pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
      const std::string reqpath = webex_requ->path();
      const onion_request_flags reqflags= webex_requ->flags();
      const unsigned reqmethnum = reqflags&OR_METHODS;
      const char* reqmethname = onion_request_methods[reqmethnum];
      RPS_DEBUG_LOG(WEB, "creating payloadwebex@" << ((void*)this) << " reqnum#" << webex_reqnum
                    << " starting " << webex_startim
                    << " for " << reqmethname << " of " << reqpath << " thread " << thrname
                    << " owner " << owner()
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebex-cr"));
    }
} // end Rps_PayloadWebex::Rps_PayloadWebex


Rps_PayloadWebex::~Rps_PayloadWebex()
{
  if (RPS_DEBUG_ENABLED(WEB))
    {
      char thrname[24];
      memset(thrname, 0, sizeof(thrname));
      pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
      const std::string reqpath = webex_requ->path();
      const onion_request_flags reqflags= webex_requ->flags();
      const unsigned reqmethnum = reqflags&OR_METHODS;
      const char* reqmethname = onion_request_methods[reqmethnum];
      RPS_DEBUG_LOG(WEB, "destroying payloadwebex@" << ((void*)this) << " reqnum#" << webex_reqnum
                    << " starting " << webex_startim
                    << " for " << reqmethname << " of " << reqpath << " thread " << thrname
                    << " owner " << owner()
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebex-destr"));
    }
  webex_requ = nullptr;
  webex_resp = nullptr;
  webex_state = nullptr;
  webex_numstate = 0;
} // end  Rps_PayloadWebex::~Rps_PayloadWebex

////////////////////////////////////////////////////////////////

void
Rps_PayloadWebHandler::gc_mark(Rps_GarbageCollector&gc) const
{
  if (webh_gethandler)
    webh_gethandler.gc_mark(gc);
  if (webh_posthandler)
    webh_posthandler.gc_mark(gc);
} // end Rps_PayloadWebHandler::gc_mark

void
Rps_PayloadWebHandler::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (webh_gethandler)
    rps_dump_scan_value(du, webh_gethandler, 0);
  if (webh_posthandler)
    rps_dump_scan_value(du, webh_posthandler, 0);
  for (auto it: webh_dicthandler)
    rps_dump_scan_value(du, it.second, 0);
  RPS_DEBUG_LOG(DUMP, "done Rps_PayloadWebHandler::dump_scan owner=" << owner() << " pathelem='" << webh_pathelem);
} // end Rps_PayloadWebHandler::dump_scan


void
Rps_PayloadWebHandler::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_web_handler below
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  jv["payload"] = "web_handler";
  jv["webh_pathelem"] = webh_pathelem;
  if (webh_gethandler && rps_is_dumpable_value(du, webh_gethandler))
    jv["webh_gethandler"] = rps_dump_json_value(du, webh_gethandler);
  if (webh_posthandler && rps_is_dumpable_value(du, webh_posthandler))
    jv["webh_posthandler"] = rps_dump_json_value(du, webh_posthandler);
  Json::Value jseq(Json::arrayValue);
  for (auto it: webh_dicthandler)
    {
      if (rps_is_dumpable_value(du, it.second))
        {
          Json::Value jent(Json::objectValue);
          jent["webstr"] = it.first;
          jent["webhdl"] = rps_dump_json_value(du, it.second);
          jseq.append(jent);
        }
    }
  jv["webh_dicthandler"] = jseq;
} // end Rps_PayloadWebHandler::dump_json_content


/// loading of web_handler payload; see above
/// Rps_PayloadWebHandler::dump_json_content
void rpsldpy_web_handler(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_DEBUG_LOG(LOAD,"start rpsldpy_web_handler obz=" << obz
                << " jv=" << jv
                << " spacid=" << spacid
                << " lineno=" << lineno);
  auto paylwebh = obz->put_new_plain_payload<Rps_PayloadWebHandler>();
  RPS_ASSERT(paylwebh);
  {
    Json::Value jpathelem = jv["webh_pathelem"];
    if (!jpathelem.isString())
      RPS_FATALOUT("rpsldpy_web_handler: object " << obz->oid()
                   << " in space " << spacid << " lineno#" << lineno
                   << " has bad webh_pathelem in " << jv);
    std::string pathelemstr = jpathelem.asString();
    paylwebh->webh_pathelem = pathelemstr;
  }
  if (Json::Value jgethandler = jv["webh_gethandler"]; jgethandler.type() > Json::nullValue)
    paylwebh->webh_gethandler = Rps_Value(jgethandler,ld);
  if (Json::Value jposthandler = jv["webh_posthandler"]; jposthandler.type() > Json::nullValue)
    paylwebh->webh_posthandler = Rps_Value(jposthandler,ld);
  RPS_DEBUG_LOG(WEB, "rpsldpy_web_handler obz=" << obz
                << " webh_pathelem=" << paylwebh->webh_pathelem
                << " webh_gethandler=" << paylwebh->webh_gethandler
                << " webh_posthandler=" << paylwebh->webh_posthandler);
  if (Json::Value jdicthandler = jv["webh_dicthandler"]; jdicthandler.isObject())
    {
      for (Json::Value& jent : jdicthandler)
        {
          if (jent.isObject())
            {
              if (!jent.isMember("webstr") || !jent.isMember("webhdl"))
                continue;
              const Json::Value& jwebstr = jent["webstr"];
              if (!jwebstr.isString())
                continue;
              std::string webstr = jwebstr.asString();
              const Json::Value& jwebhdl = jent["webhdl"];
              auto vhdl = Rps_Value(jwebhdl,ld);
              RPS_DEBUG_LOG(WEB, "rpsldpy_web_handler obz=" << obz << " webstr=" << webstr << " vhdl=" << vhdl);
              paylwebh->webh_dicthandler.insert({webstr, vhdl});
            }
        }
    }
} // end rpsldpy_web_handler

////////////////

Rps_PayloadWebHandler::Rps_PayloadWebHandler(Rps_ObjectZone*ownerobz)
  : Rps_Payload(Rps_Type::PaylWebHandler, ownerobz),
    webh_pathelem(),
    webh_gethandler(nullptr),
    webh_posthandler(nullptr),
    webh_dicthandler()
{
  RPS_ASSERT(ownerobz && ownerobz->stored_type() == Rps_Type::Object);
  char thrname[24];
  memset(thrname, 0, sizeof(thrname));
  pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
  RPS_DEBUG_LOG(WEB, "creating payloadwebhandler@" << ((void*)this)
                << "' thread " << thrname
                << " for owner " << owner()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler-cr"));
} // end Rps_PayloadWebHandler::Rps_PayloadWebHandler


Rps_PayloadWebHandler::~Rps_PayloadWebHandler()
{
  if (RPS_DEBUG_ENABLED(WEB))
    {
      char thrname[24];
      memset(thrname, 0, sizeof(thrname));
      pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
      RPS_DEBUG_LOG(WEB, "destroying payloadwebhandler@" << ((void*)this)
                    << " owner: " << owner()
                    << " pathelem: '" << webh_pathelem
                    << std::endl
                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler-destr"));
    }
  webh_gethandler = nullptr;
  webh_posthandler = nullptr;
} // end  Rps_PayloadWebHandler::~Rps_PayloadWebHandler

bool
Rps_PayloadWebHandler::valid_path_element(const std::string&pathelem)
{
  if (pathelem.find(".."))
    return false;
  if (pathelem.find("README.md"))
    return false;
  RPS_ASSERT(pathelem.size() < webh_max_path_elem_size_);
  int pathlen = (int)pathelem.size();
  for (int ix=0; ix<pathlen; ix++)
    {
      const char c = pathelem[ix];
      if (isalnum(c) || c == '_'
          || (c=='.' && ix>0 && pathelem[ix-1] != '.')
          || (c=='-' && ix>0 && pathelem[ix-1] != '-'))
        continue;
      else
        return false;
    };
  return true;
} // end Rps_PayloadWebHandler::valid_path_element

void
Rps_PayloadWebHandler::put_path_element(const std::string&pathelem)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guwebh (*(owner()->objmtxptr()));
  char thrname[24];
  memset(thrname, 0, sizeof(thrname));
  pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
  if (!valid_path_element(pathelem))
    {
      RPS_WARNOUT("Rps_PayloadWebHandler::put_path_element for owner: "
                  << owner()
                  << " invalid path element " << Rps_Cjson_String(pathelem)
                  << " in thread " << thrname
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler::put_path_element"));
      return;
    }
  RPS_DEBUG_LOG(WEB, "Rps_PayloadWebHandler::put_path_element owner=" << owner()
                << " pathelem=" << Rps_Cjson_String(pathelem)
                << " thread=" << thrname);
  webh_pathelem = pathelem;
} // end Rps_PayloadWebHandler::put_path_element

void
Rps_PayloadWebHandler::put_get_handler(Rps_Value val)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guwebh (*(owner()->objmtxptr()));
  char thrname[24];
  memset(thrname, 0, sizeof(thrname));
  pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
  if (!val || val.is_closure())
    webh_gethandler = Rps_ClosureValue(val);
  else
    {
      RPS_WARNOUT("invalid get handler " << val << " for Rps_PayloadWebHandler owned by " << owner()
                  << " in thread " << thrname
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler::put_get_handler"));
      return;
    };
  RPS_DEBUG_LOG(WEB, "Rps_PayloadWebHandler::put_get_handler owner=" << owner()
                << " val=" << val << " thread:" << thrname);
} // end Rps_PayloadWebHandler::put_get_handler

void
Rps_PayloadWebHandler::put_post_handler(Rps_Value val)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guwebh (*(owner()->objmtxptr()));
  char thrname[24];
  memset(thrname, 0, sizeof(thrname));
  pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
  if (!val || val.is_closure())
    webh_posthandler = Rps_ClosureValue(val);
  else
    {
      RPS_WARNOUT("invalid post handler " << val << " for Rps_PayloadWebHandler owned by " << owner()
                  << " in thread " << thrname
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler::put_post_handler"));
      return;
    };
  RPS_DEBUG_LOG(WEB, "Rps_PayloadWebHandler::put_post_handler owner=" << owner()
                << " val=" << val << " thread:" << thrname);
} // end Rps_PayloadWebHandler::put_post_handler

void
Rps_PayloadWebHandler::add_dict_handler(const std::string& path, Rps_Value val)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guwebh (*(owner()->objmtxptr()));
  char thrname[24];
  memset(thrname, 0, sizeof(thrname));
  pthread_getname_np(pthread_self(),thrname,sizeof(thrname));
  if (!valid_path_element(path))
    {
      RPS_WARNOUT("invalid path " << Rps_Cjson_String(path) << " for Rps_PayloadWebHandler::add_dict_handler owned by " << owner()
                  << " val=" << val
                  << " in thread " << thrname
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler::add_dict_handler"));
      return;
    }
  if (val.is_empty() && (!val.is_closure() || !val.is_object()))
    {
      RPS_WARNOUT("for path " << Rps_Cjson_String(path) << " for Rps_PayloadWebHandler::add_dict_handler owned by " << owner()
                  << " invalid val=" << val
                  << " in thread " << thrname
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadWebHandler::add_dict_handler"));
      return;
    }
  webh_dicthandler.insert({path,val});
  RPS_DEBUG_LOG(WEB, "Rps_PayloadWebHandler::add_dict_handler owner=" << owner()
                << " path=" <<  Rps_Cjson_String(path)
                << " val=" << val << " thread:" << thrname);
} // end Rps_PayloadWebHandler::add_dict_handler

////////////////////////////////////////////////////////////////



onion_connection_status
rps_serve_onion_web(Rps_Value val, Onion::Url*purl, Onion::Request*prequ, Onion::Response*presp)
{
  RPS_ASSERT(purl != nullptr);
  RPS_ASSERT(prequ != nullptr);
  RPS_ASSERT(presp != nullptr);
  char thnambuf[16];
  memset (thnambuf, 0, sizeof(thnambuf));
  if (pthread_getname_np(pthread_self(), thnambuf, sizeof(thnambuf)))
    thnambuf[0] = '?';
  else
    thnambuf[sizeof(thnambuf)-1] = (char)0;
  static std::atomic<uint64_t> reqcount;
  uint64_t reqnum = 1+reqcount.fetch_add(1);
  const std::string reqpath =prequ->path();
  const onion_request_flags reqflags=prequ->flags();
  const unsigned reqmethnum = reqflags&OR_METHODS;
  const char* reqmethname = onion_request_methods[reqmethnum];
  RPS_DEBUG_LOG(WEB, "rps_serve_onion_web thread=" << thnambuf
                << " reqpath='" << Rps_Cjson_String(reqpath)
                << "' reqmethname=" << reqmethname
                << " reqnum#" << reqnum
                << " val=" << val << std::endl);
  /**
   * We first need to ensure that the URL does not contain neither
   * ".." nor "README.md" as a substring.  Then (for GET or HEAD) we
   * try to access a file starting from webroot. If the request path
   * is empty, we use "index.html".  If it is present, we serve it.
   **/
  if (reqmethnum == OR_GET || reqmethnum == OR_HEAD)
    {
      /// requests with ".." or "README.md" are rejected...
      if (reqpath.find("..")!=std::string::npos
          || reqpath.find("README.md")!=std::string::npos)
        {
          presp->setCode(HTTP_FORBIDDEN);
          std::ostringstream reqout;
          time_t now = time(nullptr);
          char nowbuf[48];
          memset(nowbuf, 0, sizeof(nowbuf));
          ctime_r(&now, nowbuf);
          reqout
              << "<!DOCTYPE HTML>\n"
              "<html><head>\n"
              "<title>RefPerSys forbids "
              << Rps_Html_String(reqpath)
              << "</title></head>\n"
              << "<body><h1>access to <tt>"
              << Rps_Html_String(reqpath)
              << "/<tt> forbidden</h1>\n"
              << "<p>For <tt>" << reqmethname << "</tt> request #" << reqnum
              << " for <tt>" << Rps_Html_String(reqpath)
              << "</tt></p>"
              ;
          reqout << "<p>From <a href='http://refpersys.org/'>RefPerSys</a> git <tt>"
                 << Rps_Html_String(rps_lastgitcommit) << "</tt>"
                 << " running on <tt>" << rps_hostname() << "</tt> pid "
                 << (int)getpid()
                 << " at " << Rps_Html_String(nowbuf) << ".</p>"
                 << "</body>\n</html>" << std::endl;
          std::string outstr = reqout.str();
          presp->setLength(outstr.size());
          RPS_DEBUG_LOG(WEB, "rps_serve_onion_web should send:" << std::endl
                        << reqout.str());
          presp->write(outstr.c_str(), outstr.size());
          return OCS_PROCESSED;
        }
      else
        {
          auto web_exchange_ob = RPS_ROOT_OB(_8zNtuRpzXUP013WG9S);
          std::string filpath =
            std::string{rps_topdirectory}
            + "/webroot/" + (reqpath.empty()?"index.html":reqpath);
          RPS_DEBUG_LOG(WEB, "rps_serve_onion_web filpath='"
                        << Rps_Cjson_String(filpath)
                        << "' reqnum#" << reqnum
                        << " for reqpath='" << Rps_Cjson_String(reqpath)
                        << "'");
          if (!access(filpath.c_str(), F_OK))
            {
              RPS_LOCALFRAME(/*descr:*/ web_exchange_ob,
                                        /*prev:*/nullptr,
                                        /*locals:*/);
              RPS_DEBUG_LOG(WEB, "rps_serve_onion_web reqnum#" << reqnum<< " should call rps_serve_onion_file" << std::endl
                            << RPS_FULL_BACKTRACE_HERE(1, "rps_serve_onion_web-servefile"));
              return rps_serve_onion_file(&_, val, purl, prequ, presp, reqnum, filpath);
            }
          else
            RPS_DEBUG_LOG(WEB, "rps_serve_onion_web notfound filpath=" << filpath
                          << " reqnum#" << reqnum << " " << strerror(errno));

        };
    }
  /**** TODO:
   *
   * If the URL ends with .thtml, it is supposed to be an
   * Onion template file.  We probably need to separate POST and GET
   * methods of HTTP requests.  We probably don't need to handle any
   * other request except HEAD.  Caveat, this rps_serve_onion_web
   * function might run in several threads started by the libonion
   * itself!
   ****/
  RPS_FATALOUT("unimplemented rps_serve_onion_web val: " << val << std::endl
               << "... purl@" << (void*)purl
               << " prequ@" << (void*)prequ
               << " presp@" << (void*)presp
               << " thread: " << thnambuf
               << " reqnum#" << reqnum << ' ' << reqmethname <<  " reqpath: '" << reqpath << "'"
              );
#warning rps_serve_onion_web unimplemented
} // end rps_serve_onion_web


onion_connection_status
rps_serve_onion_file(Rps_CallFrame*callframe, Rps_Value val, Onion::Url*purl, Onion::Request*preq, Onion::Response*pres, uint64_t reqnum, const std::string& filepath)
{
  RPS_ASSERT(callframe);
  RPS_ASSERT(purl != nullptr);
  RPS_ASSERT(preq != nullptr);
  const std::string reqpath =preq->path();
  const onion_request_flags reqflags=preq->flags();
  const unsigned reqmethnum = reqflags&OR_METHODS;
  const char* reqmethname = onion_request_methods[reqmethnum];
  const char*mime = onion_mime_get(filepath.c_str());
  RPS_DEBUG_LOG(WEB, "rps_serve_onion_file val=" << val << " mime=" << mime
                << " filepath=" << filepath
                << " reqnum#" << reqnum
                << " reqmethname=" << reqmethname
                << " reqpath='" << Rps_Cjson_String(reqpath) << "'"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_serve_onion_file"));
  std::ostringstream reqout;

  /****
   * TODO: most files, e.g. webroot/img/refpersys_logo.svg, should be
   * served as such. But we also need template files, with a file
   * suffix of .thtml, where some substitution occurs by sending
   * RefPerSys messages, etc...
   ***/
  if (mime && (reqmethnum==OR_GET || reqmethnum==OR_HEAD))
    {
      pres->setHeader("Content-Type:", mime);
      FILE *fil = fopen(filepath.c_str(), "r");
      if (!fil)
        RPS_FATALOUT("rps_serve_onion_file filepath=" << filepath
                     << " reqnum#" << reqnum
                     << " reqmethnum=" << reqmethname
                     << " reqpath='" << Rps_Cjson_String(reqpath) << "'"
                     << " fopen failed: " << strerror(errno));
      RPS_DEBUG_LOG(WEB, "rps_serve_onion_file fd#" << (fil?fileno(fil):-1)
                    << " for reqnum#" << reqnum
                    << ' ' << reqmethname
                    << " reqpath=" << Rps_Cjson_String(reqpath));
      struct stat filstat;
      memset(&filstat, 0, sizeof(filstat));
      if (fstat(fileno(fil), &filstat))
        RPS_FATALOUT("rps_serve_onion_file filepath=" << Rps_Cjson_String(filepath)
                     << " reqnum#" << reqnum
                     << " reqmethname=" << reqmethname
                     << " reqpath='" << Rps_Cjson_String(reqpath) << "'"
                     " fstat failed: " << strerror(errno));
      {
        char lenbuf[24];
        memset(lenbuf, 0, sizeof(lenbuf));
        snprintf(lenbuf, sizeof(lenbuf), "%ld", (long)filstat.st_size);
        pres->setHeader("Content-length:", lenbuf);
      }
      {
        char*linbuf=nullptr;
        ssize_t linlen=0;
        size_t linsiz=0;
        long curoff=0;
        for(;;)
          {
            curoff = ftell(fil);
            linlen = getline(&linbuf, &linsiz, fil);
            if (linlen <= 0)
              break;
            curoff += linlen;
            pres->write(linbuf, linlen);
            RPS_DEBUG_LOG(WEB, "rps_serve_onion_file val=" << val
                          << " fd#" << fileno(fil) << " curoff:" << curoff
                          << " linlen=" << linlen
                          << " reqnum#" << reqnum
                          << " wrote " << Rps_Cjson_String(std::string(linbuf,linlen)));
          };
        free(linbuf), linbuf=nullptr;
        RPS_DEBUG_LOG(WEB, "rps_serve_onion_file val=" << val
                      << " fd#" << fileno(fil)
                      << " ending off=" << ftell(fil)
                      << " for reqnum#" << reqnum
                      << " filepath=" << Rps_Cjson_String(filepath));
        fclose(fil);
      }
      RPS_DEBUG_LOG(WEB, "done rps_serve_onion_file val=" << val << " reqnum#" << reqnum
                    << " " << reqmethname << " reqpath='" << Rps_Cjson_String(reqpath) << "'"
                    << " filepath=" << Rps_Cjson_String(filepath)
                    << " mime=" << mime
                    << std::endl << "callframe:"
                    << Rps_ShowCallFrame(callframe));
      callframe->set_state_value(nullptr);
      callframe->set_state_rank(- __LINE__);
      callframe->clear_closure();
      return OCS_PROCESSED;
    }
  else
    {
      RPS_FATALOUT("unimplemented rps_serve_onion_file filepath=" << filepath
                   << " reqnum#" << reqnum
                   << " reqmethname=" << reqmethname
                   << " reqpath='" << reqpath << "'");
    }
} // end rps_serve_onion_file


///////// end of file httpweb_rps.cc

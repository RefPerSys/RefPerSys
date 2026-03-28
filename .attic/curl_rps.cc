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
 *      © Copyright 2019 - 2024 The Reflective Persistent System Team
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


// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG curlpp

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>




extern "C" const char rps_curl_gitid[];
const char rps_curl_gitid[]= RPS_GITID;

extern "C" const char rps_curl_date[];
const char rps_curl_date[]= __DATE__;

extern "C" const char rps_curl_shortgitid[];
const char rps_curl_shortgitid[]= RPS_SHORTGITID;


std::string
rps_curl_version(void)
{
  char curlgitbuf[64];
  memset (curlgitbuf, 0, sizeof(curlgitbuf));
  strncpy(curlgitbuf, rps_curl_gitid, (3*sizeof(curlgitbuf))/4);
  std::string res("CURL ");
  char* cv = curl_version ();
  for (char *pc = cv; *pc; pc++)
    {
      if (isspace(*pc))
        res += "\n\t";
      else
        res += *pc;
    }
  res += "; LibCurlPp " LIBCURLPP_VERSION;
  return res;
} // end rps_curl_version

void
rps_initialize_curl(void)
{
  curlpp::initialize(CURL_GLOBAL_ALL);
  atexit (curlpp::terminate);
} // end rps_initialize_curl

void
rps_curl_publish_me(const char*url)
{
  RPS_ASSERT(url != nullptr);
  const char* homedir=getenv("HOME");
  const char* repldb = RPS_DEBUG_ENABLED(REPL)?"REPL debugging":"no repldbg";
  RPS_INFORMOUT("rps_curl_publish_me start top url " << Rps_QuotedC_String(url)
                << " HOME=" << homedir << " " << repldb);
  RPS_DEBUG_LOG(REPL, "rps_curl_publish_me pid:" << (int)getpid());
  std::string gitname;
  std::string gitemail;
  /// parse our $HOME/.gitconfig for name and email
  {
    std::string path_gitconf= std::string(homedir) + "/.gitconfig";
    RPS_DEBUG_LOG(REPL, "rps_curl_publish_me path_gitconf=" << path_gitconf);
    FILE* fgitconf = fopen(path_gitconf.c_str(),  "r");
    if (!fgitconf)
      RPS_FATALOUT("failed to fopen git configure file " << path_gitconf.c_str() << ':' << strerror(errno));
    char linbuf[128];
    do
      {
        memset (linbuf, 0, sizeof(linbuf));
        char *curline = fgets(linbuf, sizeof(linbuf)-2, fgitconf);
        if (!curline)
          break;
        char *eol = strchr(curline, '\n');
        if (eol)
          *eol = (char)0;
        int col = 0;
        if ((col=-1), sscanf(curline, " name = %n", &col) >= 0 && col > 1 && gitname.empty())
          gitname = std::string(curline + col);
        else if ((col= -1), sscanf(curline, " email = %n", &col) >= 0
                 && col>1 && gitemail.empty())
          gitemail = std::string(curline + col);
      }
    while (!feof(fgitconf));
    fclose(fgitconf);
  };
  RPS_DEBUG_LOG(REPL, "rps_curl_publish_me gitname " << Rps_QuotedC_String(gitname)
                << " gitemail " << Rps_QuotedC_String(gitemail));
  /// first HTTP interaction GET - obtain the status as JSON
  std::string topurlstr {url};
  RPS_DEBUG_LOG(REPL, "rps_curl_publish_me topurlstr: " << topurlstr);
  std::string statusurlstr = topurlstr;
  int statuslen = statusurlstr.size();
  if (statuslen>0 && statusurlstr[statuslen-1]=='/')
    statusurlstr.resize(statuslen-1);
  statusurlstr += "/status";
  {
    RPS_DEBUG_LOG(REPL, "statusurlstr=" << Rps_QuotedC_String(statusurlstr));
    curlpp::options::Url mystaturl(statusurlstr);
    curlpp::Easy mystatusreq;
    mystatusreq.setOpt(mystaturl);
    std::list<std::string> statheaders;
    std::string headua("User-Agent:");
    headua += "RefPerSys/";
    headua += rps_shortgitid;
    statheaders.push_back(headua);
    mystatusreq.setOpt(new curlpp::options::HttpHeader(statheaders));
    std::ostringstream outs;
    curlpp::options::WriteStream ws(&outs);
    mystatusreq.setOpt(ws);
    RPS_DEBUG_LOG(REPL, "before performing GET request for status to "
                  << statusurlstr << " statheaders:" << std::endl
                  << statheaders);
    mystatusreq.perform();
    outs << std::flush;
    RPS_DEBUG_LOG(REPL, "status outs:" << outs.str());
    Json::Value jstatus;
    Json::CharReaderBuilder jsonreaderbuilder;
    std::unique_ptr<Json::CharReader> pjsonreader(jsonreaderbuilder.newCharReader());
    RPS_ASSERT(pjsonreader);
    std::string errstr;
    const char*startp = outs.str().c_str();
    const char*endp = startp + outs.str().size()-1;
    RPS_DEBUG_LOG(REPL, "status pjsonreader@" << (void*)pjsonreader.get()
                  << " startp:" << startp << "=" << (void*)startp
                  << " endp+" << (endp-startp));
    if (!pjsonreader->parse(startp, endp, &jstatus, &errstr))
      RPS_FATALOUT("failed to parse result of status web request to " << statusurlstr
                   << " got " << errstr << " parsing " << Rps_QuotedC_String(outs.str())
                   << " using JSONCPP " << JSONCPP_VERSION_STRING);
    RPS_DEBUG_LOG(REPL, "jstatus:" << jstatus);
    if (!jstatus.isObject())
      RPS_FATALOUT("status web request to " << statusurlstr
                   << " gave non-object " << jstatus);
  }
  ///
  /// should do an HTTP interaction POST sending our data and the
  /// obtained git of the web service
#warning missing C++ code in rps_curl_publish_me
  /** TODO:
   * This function should do one or a few HTTP requests to the web service running at given url.
   * Initially on http://refpersys.org:8080/ probably (or when debugging on
     http://localhost:8080/ ...)
   * Sending there the various public data in __timestamp.c probably as HTTP POST parameters
   * and probably the owner of the git, e.g. parse the .git/config file for its name and email in section user.
   **/
  RPS_FATALOUT("unimplemented rps_curl_publish_me function for " << url);
#warning rps_curl_publish_me incomplete, using CURL easy interface
} // end rps_curl_publish_me

/// end of file curl_rps.cc

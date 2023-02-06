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
 *      © Copyright 2019 - 2023 The Reflective Persistent System Team
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

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>




extern "C" const char rps_curl_gitid[];
const char rps_curl_gitid[]= RPS_GITID;

extern "C" const char rps_curl_date[];
const char rps_curl_date[]= __DATE__;

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
  res += "LibCurlPp " LIBCURLPP_VERSION;
  res += " git ";
  res += curlgitbuf;
  return res;
} // end rps_curl_version

void
rps_initialize_curl(void)
{
  curlpp::initialize(CURL_GLOBAL_ALL);
  atexit (curlpp::terminate);
} // end rps_initialize_curl

void
rps_publish_me(const char*url)
{
  RPS_ASSERT(url != nullptr);
  const char* homedir=getenv("HOME");
  RPS_INFORMOUT("rps_publish_me start top url '" << Rps_QuotedC_String(url) << "'" << " HOME=" << homedir);
  /// parse our $HOME/.gitconfig for name and email
  {
    std::string path_gitconf= std::string(homedir) + "/.gitconfig";
    std::string gitname;
    std::string gitemail; 
    FILE* fgitconf = fopen(path_gitconf.c_str(),  "r");
    if (!fgitconf)
      RPS_FATALOUT("failed to fopen git configure file " << path_gitconf.c_str() << ':' << strerror(errno));
    char linbuf[128];
    do {
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
    } while (!feof(fgitconf));
    fclose(fgitconf);
  };
  ///
  std::string topurlstr ({url});
  CURL* my_curl = curl_easy_init();
  if (!my_curl)
    RPS_FATALOUT("failed to curl_easy_init");
  curl_easy_setopt (my_curl, CURLOPT_USERAGENT,
		    "RefPerSys/" RPS_SHORTGITID);
  std::string statusurlstr = topurlstr + "/status";
  curl_mime *mime = curl_mime_init(my_curl);
  if (!mime)
    RPS_FATALOUT("failed to curl_mime_init");
 curl_mimepart *part = curl_mime_addpart(mime);
 if (!part)
   RPS_FATALOUT("failed to curl_mime_addpart");
  
  ////
  std::string versionurlstr = topurlstr + "/refpersys_version";
  curl_easy_setopt(my_curl, CURLOPT_URL, versionurlstr.c_str());
  /** TODO:
   * This function should do one or a few HTTP requests to the web service running at given url.
   * Initially on http://refpersys.org/ probably.
   * Sending there the various public data in __timestamp.c probably as HTTP POST parameters
   * and probably the owner of the git, e.g. parse the .git/config file for its name and email in section user.
   **/
  curl_easy_cleanup (my_curl);
  RPS_FATALOUT("unimplemented rps_publish_me function for " << url);
#warning rps_publish_me incomplete, using CURL easy interface
} // end rps_publish_me

/// end of file curl_rps.cc

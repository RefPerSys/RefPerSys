/****************************************************************
 * file store_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the persistent store, in HJSON format.
 *      See also http://hjson.org/ & https://github.com/hjson/hjson-cpp
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
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

#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>


extern "C" const char rps_store_gitid[];
const char rps_store_gitid[]= RPS_GITID;

extern "C" const char rps_store_date[];
const char rps_store_date[]= __DATE__;


// same as used in rps_manifest.hjson file
#define RPS_MANIFEST_FORMAT "RefPerSysFormat2019A"
//////////////////////////////////////////////// loader
class Rps_Loader
{
  std::string ld_topdir;
public:
  Rps_Loader(const std::string&topdir) :
    ld_topdir(topdir) {};
  void parse_manifest_file(void);
  std::string string_of_loaded_file(const std::string& relpath);
  std::string load_real_path(const std::string& path);
};				// end class Rps_Loader


std::string
Rps_Loader::load_real_path(const std::string& path)
{
  if (path.size() > 2 && path[0] == '/')
    {
      if (access(path.c_str(), R_OK))
        {
          int e = errno;
          RPS_WARN("loader cannot access %s - %s",
                   path.c_str(), strerror(e));
          throw std::runtime_error(path + ":" + strerror(e));
        }
      char*rp = realpath(path.c_str(), nullptr);
      if (!rp)
        throw std::runtime_error(std::string("realpath failed:") + path);
      std::string restr (rp);
      free (rp), rp = nullptr;
      return restr;
    }
  std::string candipath;// candidate path
  candipath = ld_topdir + "/" + path;
  if (!access(candipath.c_str(), R_OK))
    {
      char*rp = realpath(candipath.c_str(), nullptr);
      if (!rp)
        throw std::runtime_error(std::string("realpath failed:") + candipath);
      std::string restr (rp);
      free (rp), rp = nullptr;
      return restr;
    }
  throw std::runtime_error(std::string("cannot file load real path for ") + path);
#warning Rps_Loader::load_real_path is incomplete
} // end Rps_Loader::load_real_path



std::string
Rps_Loader::string_of_loaded_file(const std::string&relpath)
{
  constexpr size_t maxfilen = 1024*1024; // a megabyte
  std::string fullpath = load_real_path(relpath);
  std::string res;
  int lincnt=0;
  std::ifstream inp(fullpath);
  for (std::string linbuf; std::getline(inp, linbuf); )
    {
      lincnt++;
      if (u8_check(reinterpret_cast<const uint8_t*> (linbuf.c_str()),
                   linbuf.size()))
        {
          RPS_WARN("non UTF8 line#%d in %s:\n%s",
                   lincnt, fullpath.c_str(), linbuf.c_str());
          char errbuf[40];
          snprintf(errbuf, sizeof(errbuf), "non UTF8 line#%d", lincnt);
          throw std::runtime_error(std::string(errbuf) + " in " + fullpath);
        }
      res += linbuf;
      res += '\n';
    }
  return res;
} // end Rps_Loader::string_of_loaded_file

//////////////////////////////////////////////// dumper
class Rps_Dumper
{
  std::string du_topdir;
public:
  Rps_Dumper(const std::string&topdir) :
    du_topdir(topdir) {};
};				// end class Rps_Dumper


//////////////////////////////////////////////////////////////// dump
void rps_dump_into (const std::string dirpath)
{
  RPS_FATAL("unimplemented rps_dump_into '%s'", dirpath.c_str());
#warning rps_dump_into unimplemented
} // end of rps_dump_into

//////////////////////////////////////////////////////////////// load

void
Rps_Loader::parse_manifest_file(void)
{
  std::string manifpath = ld_topdir + "/" + RPS_MANIFEST_HJSON;
  if (access(manifpath.c_str(), R_OK))
    RPS_FATAL("Rps_Loader::parse_manifest_file cannot access %s - %m",
              manifpath.c_str());
  std::string manifstr = string_of_loaded_file(RPS_MANIFEST_HJSON);
  RPS_WARNOUT("Rps_Loader::parse_manifest_file should parse:"
              << std::endl
              << manifstr << std::endl);
  RPS_ASSERT (manifstr.size()>0);
#warning the below call to Hjson::Unmarshal fail so should be commented out
  Hjson::Value manifhjson
    = Hjson::Unmarshal(manifstr.c_str(), manifstr.size());
  if (manifhjson.type() != Hjson::Value::Type::MAP)
    RPS_FATAL("Rps_Loader::parse_manifest_file bad HJson type #%d",
              (int)manifhjson.type());
  if (manifhjson["format"].to_string() != RPS_MANIFEST_FORMAT)
    RPS_FATAL("manifest map in %s should have format: %s",
              manifpath.c_str (), RPS_MANIFEST_FORMAT);
  RPS_WARNOUT("Rps_Loader::parse_manifest_file incompletely parsed "
              << Hjson::MarshalJson(manifhjson));
#warning incomplete Rps_Loader::parse_manifest_file
} // end Rps_Loader::parse_manifest_file

void rps_load_from (const std::string& dirpath)
{
  RPS_WARN("unimplemented rps_load_from '%s'", dirpath.c_str());
  Rps_Loader loader(dirpath);
  loader.parse_manifest_file();
#warning rps_load_from unimplemented
} // end of rps_load_from

//////////////////////////////////////////////////////////// end of file store_rps.cc


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

class Rps_Loader
{
  std::string ld_topdir;
public:
  Rps_Loader(const std::string&topdir) :
    ld_topdir(topdir) {};
  void parse_manifest_file(void);
};				// end class Rps_Loader


class Rps_Dumper
{
  std::string du_topdir;
public:
  Rps_Dumper(const std::string&topdir) :
    du_topdir(topdir) {};
};				// end class Rps_Dumper



extern "C" const char rps_store_gitid[];
const char rps_store_gitid[]= RPS_GITID;

extern "C" const char rps_store_date[];
const char rps_store_date[]= __DATE__;

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
  RPS_WARN("Rps_Loader::parse_manifest_file should parse %s",
           manifpath.c_str());
} // end Rps_Loader::parse_manifest_file

void rps_load_from (const std::string& dirpath)
{
  RPS_WARN("unimplemented rps_load_from '%s'", dirpath.c_str());
  Rps_Loader loader(dirpath);
  loader.parse_manifest_file();
#warning rps_load_from unimplemented
} // end of rps_load_from

//////////////////////////////////////////////////////////// end of file store_rps.cc


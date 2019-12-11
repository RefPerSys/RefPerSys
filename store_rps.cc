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
  /// set of space ids
  std::set<Rps_Id> ld_spaceset;
  /// set of global roots id
  std::set<Rps_Id> ld_globrootsidset;
  /// mapping from plugins id to their dlopen-ed handle
  std::map<Rps_Id,void*> ld_pluginsmap;
  /// map of loaded objects
  std::map<Rps_Id,Rps_ObjectRef> ld_mapobjects;
  bool is_object_starting_line(Rps_Id spacid, unsigned lineno, const std::string&linbuf, Rps_Id*pobid);
  void parse_hjson_buffer_second_pass (Rps_Id spacid, unsigned lineno,
                                       Rps_Id objid, const std::string& objbuf);
public:
  Rps_Loader(const std::string&topdir) :
    ld_topdir(topdir) {};
  void parse_manifest_file(void);
  void first_pass_space(Rps_Id spacid);
  void second_pass_space(Rps_Id spacid);
  std::string string_of_loaded_file(const std::string& relpath);
  std::string space_file_path(Rps_Id spacid);
  std::string load_real_path(const std::string& path);
  Rps_ObjectRef find_object_by_oid(Rps_Id oid)
  {
    auto it = ld_mapobjects.find(oid);
    if (it != ld_mapobjects.end())
      return it->second;
    return Rps_ObjectRef(nullptr);
  };
  void load_all_state_files(void);
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
#warning Rps_Loader::load_real_path could be incomplete
} // end Rps_Loader::load_real_path




std::string
Rps_Loader::space_file_path(Rps_Id spacid)
{
  if (!spacid.valid())
    throw std::runtime_error("Rps_Loader::space_file_path invalid spacid");
  return std::string{"persistore/sp"} + spacid.to_string() + "-rps.hjson";
} // end Rps_Loader::space_file_path



bool
Rps_Loader::is_object_starting_line(Rps_Id spacid, unsigned lineno, const std::string&linbuf, Rps_Id*pobid)
{
  if (pobid)
    *pobid = Rps_Id(nullptr);
  if (linbuf.size() < strlen ("//+ob") + Rps_Id::nbchars)
    return false;
  if (linbuf[0] != '/' || linbuf[1] != '/'
      || linbuf[2] != '+'
      || linbuf[3] != 'o'
      || linbuf[4] != 'b')
    return false;
  const char*end=nullptr;
  bool ok=false;
  Rps_Id oid(linbuf.c_str()+strlen("//+ob"), &end, &ok);
  if (!end || (*end && !isspace(*end)))
    return false;
  if (!ok)
    return false;
  if (!oid.valid())
    return false;
  if (pobid)
    *pobid = oid;
  return true;
} // end Rps_Loader::is_object_starting_line

void
Rps_Loader::first_pass_space(Rps_Id spacid)
{
  auto spacepath = load_real_path(space_file_path(spacid));
  std::ifstream ins(spacepath);
  std::string prologstr;
  int obcnt = 0;
  int expectedcnt = 0;
  unsigned lincnt = 0;
  RPS_INFORM("Rps_Loader::first_pass_space start spacepath=%s", spacepath.c_str());
  for (std::string linbuf; std::getline(ins, linbuf); )
    {
      lincnt++;
      if (u8_check(reinterpret_cast<const uint8_t*> (linbuf.c_str()),
                   linbuf.size()))
        {
          RPS_WARN("non UTF8 line#%d in %s:\n%s",
                   lincnt, spacepath.c_str(), linbuf.c_str());
          char errbuf[40];
          snprintf(errbuf, sizeof(errbuf), "non UTF8 line#%d", lincnt);
          throw std::runtime_error(std::string(errbuf) + " in " + spacepath);
        }
      RPS_INFORM("lincnt#%d, lin.siz#%zd\n..linbuf:%s", lincnt,
                 linbuf.size(), linbuf.c_str());
      if (RPS_UNLIKELY(obcnt == 0))
        {
          prologstr += linbuf;
          prologstr += '\n';
        }
      Rps_Id curobjid;
      if (is_object_starting_line(spacid, lincnt, linbuf, &curobjid))
        {
          RPS_INFORMOUT("got ob spacid:" << spacid
                        << " linbuf: " << linbuf
                        << " lincnt#" << lincnt
                        << " curobjid:" << curobjid);
          if (RPS_UNLIKELY(obcnt == 0))
            {
              Hjson::Value prologhjson
                = Hjson::Unmarshal(prologstr.c_str(), prologstr.size());
              if (prologhjson.type() != Hjson::Value::Type::MAP)
                RPS_FATAL("Rps_Loader::first_pass_space %s bad HJson type #%d",
                          spacepath.c_str(), (int)prologhjson.type());
              RPS_INFORM("Rps_Loader::first_pass_space %s prologstr=<%s>\n"
                         ".. prologhjson=%s",
                         spacepath.c_str(), prologstr.c_str(), Hjson::Marshal(prologhjson).c_str());
              Hjson::Value formathjson = prologhjson["format"];
              if (formathjson.type() !=Hjson::Value::Type::STRING)
                RPS_FATALOUT("space file " << spacepath
                             << " with bad format type#" << (int)formathjson.type());
              if (formathjson.to_string() != RPS_MANIFEST_FORMAT)
                RPS_FATALOUT("space file " << spacepath
                             << "should have format: "
                             << RPS_MANIFEST_FORMAT
                             << " but got "
                             << (formathjson.to_string()));
              if (prologhjson["spaceid"].to_string() != spacid.to_string())
                RPS_FATAL("spacefile %s should have spaceid: '%s' but got '%s'",
                          spacepath.c_str (), spacid.to_string().c_str(),
                          prologhjson["spaceid"].to_string().c_str());
              Hjson::Value nbobjectshjson =  prologhjson["nbobjects"];
              expectedcnt =nbobjectshjson.to_int64();
            }
          Rps_ObjectRef obref(Rps_ObjectZone::make_loaded(curobjid, this));
          if (ld_mapobjects.find(curobjid) != ld_mapobjects.end())
            {
              RPS_WARN("duplicate object of oid %s in  line#%d in %s",
                       curobjid.to_string().c_str(), lincnt, spacepath.c_str());
              throw std::runtime_error(std::string("duplicate objid "
                                                   + curobjid.to_string() + " in " + spacepath));
            }
          ld_mapobjects.insert({curobjid,obref});
          obcnt++;
        }
    }
  if (obcnt != expectedcnt)
    {
      RPS_WARN("got %d objects in loaded space %s but expected %d of them",
               obcnt,  spacepath.c_str(), expectedcnt);
      throw std::runtime_error(std::string("unexpected object count in ")
                               + spacepath);
    }
  RPS_INFORMOUT("read " << obcnt
                << " objects while loading first pass of" << spacepath);
} // end Rps_Loader::first_pass_space


void
Rps_Loader::parse_hjson_buffer_second_pass (Rps_Id spacid, unsigned lineno,
    Rps_Id objid, const std::string& objbuf)
{
  RPS_INFORMOUT("parse_hjson_buffer_second_pass start spacid=" << spacid
                << " lineno=" <<lineno
                << " objid=" <<objid
                << " objbuf:\n" << objbuf);
  Hjson::Value objhjson
    = Hjson::Unmarshal(objbuf.c_str(), objbuf.size());
  if (objhjson.type() != Hjson::Value::Type::MAP)
    RPS_FATALOUT("parse_hjson_buffer_second_pass spacid=" << spacid
                 << " lineno:" << lineno
                 << " objid:" << objid
                 << " bad objbuf:" << std::endl
                 << objbuf);
  Hjson::Value oidhjson = objhjson["oid"];
  if (oidhjson.to_string() != objid.to_string())
    RPS_FATALOUT("parse_hjson_buffer_second_pass spacid=" << spacid
                 << " lineno:" << lineno
                 << " objid:" << objid
                 << " unexpected");
  auto obz = Rps_ObjectZone::find(objid);
  RPS_ASSERT (obz);
#warning incomplete Rps_Loader::parse_hjson_buffer_second_pass
// RPS_INFORMOUT("parse_hjson_buffer_second_pass spacid=" << spacid
//               << " lineno:" << lineno
//               << " objid:" << objid
//               << " objhjson: " << Hjson::Marshal(objhjson)
//               << std::endl
//               << " objbuf:" << std::endl
//               << objbuf << std::endl);
} // end of Rps_Loader::parse_hjson_buffer_second_pass


void
Rps_Loader::second_pass_space(Rps_Id spacid)
{
  auto spacepath = load_real_path(space_file_path(spacid));
  std::ifstream ins(spacepath);
  unsigned lincnt = 0;
  unsigned obcnt = 0;
  Rps_Id prevoid;
  unsigned prevlin=0;
  RPS_INFORM("Rps_Loader::second_pass_space start spacepath=%s", spacepath.c_str());
  std::string objbuf;
  for (std::string linbuf; std::getline(ins, linbuf); )
    {
      lincnt++;
      Rps_Id curobjid;
      if (is_object_starting_line(spacid,lincnt,linbuf,&curobjid))
        {
          if (objbuf.size() > 0 && prevoid && prevlin>0)
            {
              parse_hjson_buffer_second_pass(spacid, prevlin, prevoid, objbuf);
              prevoid = Rps_Id(nullptr);
            }
          objbuf.clear();
          objbuf = linbuf + '\n';
          obcnt++;
          prevoid = curobjid;
          prevlin = lincnt;
        }
      else if (objbuf.size() > 0)
        {
          objbuf += linbuf;
          objbuf += '\n';
        }
    } // end for getline
  if (objbuf.size() > 0 && prevoid && prevlin>0)
    {
      parse_hjson_buffer_second_pass(spacid, prevlin, prevoid, objbuf);
      prevoid = Rps_Id(nullptr);
    }
} // end of Rps_Loader::second_pass_space


void
Rps_Loader::load_all_state_files(void)
{
  int spacecnt1 = 0, spacecnt2 = 0;
  for (Rps_Id spacid: ld_spaceset)
    {
      first_pass_space(spacid);
      spacecnt1++;
    }
  RPS_INFORMOUT("loaded " << spacecnt1 << " space files in first pass");
  for (Rps_Id spacid: ld_spaceset)
    {
      second_pass_space(spacid);
      spacecnt2++;
    }
  RPS_INFORMOUT("loaded " << spacecnt1 << " space files in second pass with "
                << ld_mapobjects.size() << " objects");
} // end Rps_Loader::load_all_state_files


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
      if (RPS_UNLIKELY(res.size() > maxfilen))
        RPS_FATAL("too big file %zd of path %s", res.size(), fullpath.c_str());
    }
  return res;
} // end Rps_Loader::string_of_loaded_file



Rps_Value::Rps_Value(const Hjson::Value &hjv, Rps_Loader*ld)
  : Rps_Value(nullptr)
{
  RPS_ASSERT(ld != nullptr);
  std::int64_t i=0;
  double d=0.0;
  std::string str = "";
  std::size_t siz=0;
  Hjson::Value hjcomp;
  /// see https://github.com/hjson/hjson-cpp/issues/22
  /// so use https://github.com/bstarynk/hjson-cpp
  if (hjv.is_int64(&i))
    {
      *this = Rps_Value(i, Rps_IntTag{});
      return;
    }
  else if (hjv.is_double(&d))
    {
      *this = Rps_Value(d, Rps_DoubleTag{});
      return;
    }
  else if (hjv.is_null())
    {
      *this = Rps_Value(nullptr);
      return;
    }
  else if (hjv.is_string(&str))
    {
      /// should probably special-case when str looks like an objid
    }
  else if (hjv.is_map(&siz) && siz==1 && hjv.is_map_with_key("string")
           && (hjcomp=hjv["string"]).is_string(&str))
    {
      *this = Rps_StringValue(str);
      return;
    }
#warning Rps_Value::Rps_Value(const Hjson::Value &hjv, Rps_Loader*ld) unimplemented
  RPS_WARN("unimplemented Rps_Value::Rps_Value(const Hjson::Value &hjv, Rps_Loader*ld)");
} // end of Rps_Value::Rps_Value(const Hjson::value &hjv, Rps_Loader*ld)


Rps_ObjectRef::Rps_ObjectRef(const Hjson::Value &hjv, Rps_Loader*ld)
  : Rps_ObjectRef(nullptr)
{
  RPS_ASSERT(ld != nullptr);
  std::string str = "";
  Rps_Id oid;
  if (hjv.is_string(&str) && (oid = Rps_Id(str)).valid())
    {
      Rps_ObjectRef obr= ld->find_object_by_oid(oid);
      if (!obr)
        {
          RPS_WARNOUT("unknown oid " << oid);
          throw  std::runtime_error(std::string{"unknown oid "} + oid.to_string());
        }
      *this = obr;
      return;
    }
  RPS_WARN("unimplemented Rps_ObjectRef::Rps_ObjectRef(const Hjson::Value &hjv, Rps_Loader*ld)");
  throw  std::runtime_error("unimplemented Rps_ObjectRef::Rps_ObjectRef(const Hjson::Value &hjv, Rps_Loader*ld)");
#warning unimplemented Rps_ObjectRef::Rps_ObjectRef(const Hjson::Value &hjv, Rps_Loader*ld)
} // end Rps_ObjectRef::Rps_ObjectRef(const Hjson::Value &hjv, Rps_Loader*ld)

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
  if (manifstr.size() < 20)
    RPS_FATAL("Rps_Loader::parse_manifest_file nearly empty file %s",
              manifpath.c_str());
  Hjson::Value manifhjson
    = Hjson::Unmarshal(manifstr.c_str(), manifstr.size());
  if (manifhjson.type() != Hjson::Value::Type::MAP)
    RPS_FATAL("Rps_Loader::parse_manifest_file bad HJson type #%d",
              (int)manifhjson.type());
  if (manifhjson["format"].to_string() != RPS_MANIFEST_FORMAT)
    RPS_FATAL("manifest map in %s should have format: '%s' but got '%s'",
              manifpath.c_str (), RPS_MANIFEST_FORMAT,
              manifhjson["format"].to_string().c_str());
  /// parse spaceset
  {
    auto spsethjson = manifhjson["spaceset"];
    if (spsethjson.type() !=  Hjson::Value::Type::VECTOR)
      RPS_FATAL("manifest map in %s should have spaceset: [...]",
                manifpath.c_str ());
    size_t sizespset = spsethjson.size();
    for (int ix=0; ix<(int)sizespset; ix++)
      {
        std::string curspidstr = spsethjson[ix].to_string();
        Rps_Id curspid (curspidstr);
        RPS_ASSERT(curspid);
        ld_spaceset.insert({curspid});
      }
  }
  /// parse globalroots
  {
    auto globrootshjson = manifhjson["globalroots"];
    if (globrootshjson.type() !=  Hjson::Value::Type::VECTOR)
      RPS_FATAL("manifest map in %s should have globalroots: [...]",
                manifpath.c_str ());
    size_t sizeglobroots = globrootshjson.size();
    for (int ix=0; ix<(int)sizeglobroots; ix++)
      {
        std::string curgrootidstr = globrootshjson[ix].to_string();
        Rps_Id curgrootid (curgrootidstr);
        RPS_ASSERT(curgrootid);
        ld_globrootsidset.insert(curgrootid);
      }
  }
  /// parse plugins
  {
    auto pluginshjson = manifhjson["plugins"];
    if (pluginshjson.type() !=  Hjson::Value::Type::VECTOR)
      RPS_FATAL("manifest map in %s should have plugins: [...]",
                manifpath.c_str ());
    size_t sizeplugins = pluginshjson.size();
    for (int ix=0; ix<(int)sizeplugins; ix++)
      {
        std::string curpluginidstr = pluginshjson[ix].to_string();
        Rps_Id curpluginid (curpluginidstr);
        RPS_ASSERT(curpluginid && curpluginid.valid());
        std::string pluginpath = load_real_path(std::string{"plugins/rps"} + curpluginid.to_string() + "-mod.so");
        RPS_INFORMOUT("should load plugin #" << ix << " from " << pluginpath);
        void* dlh = dlopen(pluginpath.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!dlh)
          RPS_FATAL("failed to load plugin #%d file %s: %s",
                    ix, pluginpath.c_str(), dlerror());
        ld_pluginsmap.insert({curpluginid, dlh});
      }
  }
  ////
  RPS_INFORMOUT("Rps_Loader::parse_manifest_file parsed "
                << Hjson::MarshalJson(manifhjson));
#warning incomplete Rps_Loader::parse_manifest_file
} // end Rps_Loader::parse_manifest_file



void rps_load_from (const std::string& dirpath)
{
  RPS_WARN("unimplemented rps_load_from '%s'", dirpath.c_str());
  Rps_Loader loader(dirpath);
  loader.parse_manifest_file();
  loader.load_all_state_files();
#warning rps_load_from unimplemented
} // end of rps_load_from

//////////////////////////////////////////////////////////// end of file store_rps.cc


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
  /// dlsym and dlopen are not reentrant, so we need a mutex
  std::mutex ld_mtx;
  /// set of space ids
  std::set<Rps_Id> ld_spaceset;
  /// set of global roots id
  std::set<Rps_Id> ld_globrootsidset;
  /// mapping from plugins id to their dlopen-ed handle
  std::map<Rps_Id,void*> ld_pluginsmap;
  /// map of loaded objects
  std::map<Rps_Id,Rps_ObjectRef> ld_mapobjects;
  /// dictionnary of payload loaders - used as a cache to avoid most dlsym-s
  std::map<std::string,rpsldpysig_t*> ld_payloadercache;
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
    goto bad;
  if (!ok)
    goto bad;
  if (!oid.valid())
    goto bad;
  if (pobid)
    *pobid = oid;
  return true;
bad:
  RPS_WARNOUT("bad object starting line in space " << spacid << " line#" << lineno
              << ":" << std::endl
              << linbuf);
  return false;
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
      //RPS_INFORM("lincnt#%d, lin.siz#%zd\n..linbuf:%s", lincnt,
      //           linbuf.size(), linbuf.c_str());
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
  /// RPS_INFORMOUT("parse_hjson_buffer_second_pass start spacid=" << spacid
  ///               << " lineno=" <<lineno
  ///               << " objid=" <<objid
  ///               << " objbuf:\n" << objbuf);
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
  obz->loader_set_class (this, Rps_ObjectRef(objhjson["class"], this));
  obz->loader_set_mtime (this, objhjson["mtime"].to_double());
  if (objhjson.is_map_with_key("components"))
    {
      auto comphjson = objhjson["components"];
      std::size_t siz= 0;
      if (comphjson.is_vector(&siz))
        {
          obz->loader_reserve_comps(this, (unsigned)siz);
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto valcomp = Rps_Value(comphjson[ix], this);
              obz->loader_add_comp(this, valcomp);
            }
        }
    }
  if (objhjson.is_map_with_key("attributes"))
    {
      auto attrhjson = objhjson["attributes"];
      std::size_t siz= 0;
      if (attrhjson.is_vector(&siz))
        {
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto enthjson = attrhjson[ix];
              std::size_t entsiz= 0;
              if (enthjson.is_map(&entsiz) && entsiz>=2
                  && enthjson.is_map_with_key("at")
                  && enthjson.is_map_with_key("va")
                 )
                {
                  auto atobr =  Rps_ObjectRef(enthjson["at"], this);
                  auto atval = Rps_Value(enthjson["va"], this);
                  obz->loader_put_attr(this, atobr, atval);
                }
            }
        }
    }
  if (objhjson.is_map_with_key("payload"))
    {
      rpsldpysig_t*pldfun = nullptr;
      auto paylstr = objhjson["payload"].to_string();
      {
        std::lock_guard<std::mutex> gu(ld_mtx);
        auto ldit = ld_payloadercache.find(paylstr);
        if (RPS_UNLIKELY(ldit == ld_payloadercache.end()))
          {
            char firstc = paylstr.at(0);
            if (isalpha(firstc))
              {
                std::string symstr = std::string(RPS_PAYLOADING_PREFIX) + paylstr;
                void* symad = dlsym(rps_proghdl, symstr.c_str());
                if (!symad)
                  RPS_FATALOUT("cannot dlsym " << symstr << " for payload of objid:" <<  objid
                               << " lineno:" << lineno << ", spacid:" << spacid
                               << ":: " << dlerror());
                pldfun = (rpsldpysig_t*)symad;
                ld_payloadercache.insert({paylstr, pldfun});
              }
            else if (firstc=='_')
              {
                auto pyid = Rps_Id(paylstr);
                if (!pyid.valid())
                  RPS_FATALOUT("Rps_Loader::parse_hjson_buffer_second_pass spacid:" << spacid
                               << " lineno:" << lineno
                               << " objid:" << objid
                               << " invalid id payload:" << paylstr);
              }
            else
              RPS_FATALOUT("Rps_Loader::parse_hjson_buffer_second_pass spacid:" << spacid
                           << " lineno:" << lineno
                           << " objid:" << objid
                           << " invalid payload:" << paylstr);
          }
        else
          pldfun = ldit->second;
      };
#warning incomplete Rps_Loader::parse_hjson_buffer_second_pass for payload
      RPS_WARNOUT("Rps_Loader::parse_hjson_buffer_second_pass incomplete spacid=" << spacid
                  << " lineno:" << lineno
                  << " objid:" << objid
                  << " payload: " << paylstr
                  << std::endl);
      if (pldfun)
        {
          (*pldfun)(obz,this,objhjson,spacid,lineno);
        }
      else
        {
          RPS_FATALOUT("Rps_Loader::parse_hjson_buffer_second_pass in spacid=" << spacid
                       << " lineno:" << lineno
                       << " objid:" << objid
                       << " payload: " << paylstr
                       << " without loading function"
                       << std::endl);
        }
    }
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
  Hjson::Value hjvtype;
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
      if (str.size() == Rps_Id::nbchars + 1 && str[0] == '_'
          && std::all_of(str.begin()+1, str.end(),
                         [](char c)
      {
        return strchr(Rps_Id::b62digits, c) != nullptr;
        }))
      {
        *this = Rps_ObjectValue(Rps_ObjectRef(hjv, ld));
        return;
      }
      *this = Rps_StringValue(str);
      return;
    }
  else if (hjv.is_map(&siz) && siz==1 && hjv.is_map_with_key("string")
           && (hjcomp=hjv["string"]).is_string(&str))
    {
      *this = Rps_StringValue(str);
      return;
    }
  else if (hjv.is_map(&siz) &&  hjv.is_map_with_key("vtype")
           && (hjvtype=hjv["vtype"].is_string(&str)))
    {
      if (str == "set" && siz==2 && hjv.is_map_with_key("elem")
          && (hjcomp=hjv["elem"]).is_vector(&siz))
        {
          std::set<Rps_ObjectRef> setobr;
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto obrelem = Rps_ObjectRef(hjcomp[ix], ld);
              if (obrelem)
                setobr.insert(obrelem);
            }
          *this= Rps_SetValue(setobr);
          return;
        }
      else if (str == "tuple" && siz==2 && hjv.is_map_with_key("elem")
               && (hjcomp=hjv["comp"]).is_vector(&siz))
        {
          std::vector<Rps_ObjectRef> vecobr;
          vecobr.reserve(siz);
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto obrcomp = Rps_ObjectRef(hjcomp[ix], ld);
              vecobr.push_back(obrcomp);
            };
          *this= Rps_TupleValue(vecobr);
          return;
        }
      else if (str == "closure" && siz==3
               && hjv.is_map_with_key("fn")
               && hjv.is_map_with_key("env"))
        {
          auto hjfn = hjv["fn"];
          auto hjenv = hjv["env"];
          auto funobr = Rps_ObjectRef(hjfn, ld);
          if (hjenv.is_vector(&siz))
            {
              std::vector<Rps_Value> vecenv;
              vecenv.reserve(siz+1);
              for (int ix=0; ix <(int)siz; ix++)
                {
                  auto curval = Rps_Value(hjenv[ix], ld);
                  vecenv.push_back(curval);
                };
              *this = Rps_ClosureValue(funobr, vecenv);
              return;
            }
        }
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
  friend void rps_dump_into (const std::string dirpath);
  friend void rps_dump_scan_object(Rps_Dumper*, Rps_ObjectRef obr);
  friend void rps_dump_scan_value(Rps_Dumper*, Rps_Value obr, unsigned depth);
  friend Hjson::Value rps_dump_hjson_value(Rps_Dumper*, Rps_Value obr);
  friend Hjson::Value rps_dump_hjson_object_ref(Rps_Dumper*, Rps_ObjectRef obr);
  std::string du_topdir;
  std::unordered_map<Rps_Id, Rps_ObjectRef,Rps_Id::Hasher> du_mapobjects;
  std::deque<Rps_ObjectRef> du_scanque;
public:
  Rps_Dumper(const std::string&topdir) :
    du_topdir(topdir) {};
  void scan_object(const Rps_ObjectRef obr);
  void scan_value(const Rps_Value val, unsigned depth);
  Hjson::Value hjson_value(const Rps_Value val);
  Hjson::Value hjson_objectref(const Rps_ObjectRef obr);
  bool is_dumpable_objref(const Rps_ObjectRef obr);
  bool is_dumpable_value(const Rps_Value val);
};				// end class Rps_Dumper


void
Rps_Dumper::scan_object(const Rps_ObjectRef obr)
{
  if (!obr)
    return;
  RPS_FATALOUT("Rps_Dumper::scan_object unimplemented " << obr->oid());
#warning Rps_Dumper::scan_object unimplemented
} // end Rps_Dumper::scan_object

void
Rps_Dumper::scan_value(const Rps_Value val, unsigned depth)
{
  if (!val)
    return;
  RPS_FATALOUT("Rps_Dumper::scan_value unimplemented depth#" << depth);
#warning Rps_Dumper::scan_value unimplemented
} // end Rps_Dumper::scan_value

Hjson::Value
Rps_Dumper::hjson_value(Rps_Value val)
{
  if (is_dumpable_value(val))
    {
      RPS_FATALOUT("Rps_Dumper::hjson_value unimplemented");
#warning Rps_Dumper::hjson_value unimplemented
    }
  else return Hjson::Value(nullptr);
} // end Rps_Dumper::hjson_value

bool
Rps_Dumper::is_dumpable_objref(const Rps_ObjectRef obr)
{
  if (!obr)
    return false;
  if (du_mapobjects.find(obr->oid()) != du_mapobjects.end())
    return true;
  RPS_FATALOUT("Rps_Dumper::is_dumpable_objref partly unimplemented");
#warning Rps_Dumper::is_dumpable_objref partly unimplemented
} // end Rps_Dumper::is_dumpable_objref


bool
Rps_Dumper::is_dumpable_value(const Rps_Value val)
{
  if (!val) return true;
  if (val.is_int() || val.is_string() || val.is_set() || val.is_tuple())
    return true;
  if (val.is_object())
    return is_dumpable_objref(val.to_object());
  RPS_FATALOUT("Rps_Dumper::is_dumpable_value partly unimplemented");
#warning Rps_Dumper::is_dumpable_value partly unimplemented
} // end Rps_Dumper::is_dumpable_value

bool rps_is_dumpable_objref(Rps_Dumper*du, const Rps_ObjectRef obr)
{
  RPS_ASSERT(du != nullptr);
  return du->is_dumpable_objref(obr);
} // end rps_is_dumpable_objref

bool rps_is_dumpable_value(Rps_Dumper*du, const Rps_Value val)
{

  RPS_ASSERT(du != nullptr);
  return du->is_dumpable_value(val);
} // end rps_is_dumpable_value

void
rps_dump_scan_object(Rps_Dumper*du, const Rps_ObjectRef obr)
{
  RPS_ASSERT(du != nullptr);
  du->scan_object(obr);
} // end rps_dump_scan_object

void rps_dump_scan_value(Rps_Dumper*du, const Rps_Value val, unsigned depth)
{
  RPS_ASSERT(du != nullptr);
  du->scan_value(val, depth);
} // end rps_dump_scan_value

Hjson::Value
rps_dump_hjson_value(Rps_Dumper*du, Rps_Value val)
{
  RPS_ASSERT(du != nullptr);
  if (!val || !rps_is_dumpable_value(du,val))
    return Hjson::Value(nullptr);
  else return du->hjson_value(val);
} // end rps_dump_hjson_value

Hjson::Value
rps_dump_hjson_objectref(Rps_Dumper*du, Rps_ObjectRef obr)
{
  RPS_ASSERT(du != nullptr);
  if (!obr || !rps_is_dumpable_objref(du,obr))
    return Hjson::Value(nullptr);
  else
    return Hjson::Value(obr->oid().to_string());
} // end rps_dump_hjson_object_ref

void
Rps_TupleOb::dump_scan(Rps_Dumper*du, unsigned depth) const
{
  RPS_ASSERT(du != nullptr);
  for (auto obr: *this)
    du->scan_object(obr);
}

Hjson::Value
Rps_TupleOb::dump_hjson(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  RPS_FATAL("unimplemented Rps_TupleOb::dump_hjson");
#warning unimplemented Rps_TupleOb::dump_hjson
} // end Rps_TupleOb::dump_hjson


////////////////
void
Rps_SetOb::dump_scan(Rps_Dumper*du, unsigned depth) const
{
  RPS_ASSERT(du != nullptr);
  for (auto obr: *this)
    du->scan_object(obr);
}

Hjson::Value
Rps_SetOb::dump_hjson(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  RPS_FATAL("unimplemented Rps_SetOb::dump_hjson");
#warning unimplemented Rps_SetOb::dump_hjson
} // end Rps_SetOb::dump_hjson

////////////////
void
Rps_ClosureZone::dump_scan(Rps_Dumper*du, unsigned depth) const
{
  RPS_ASSERT(du != nullptr);
  auto obrcon = conn();
  du->scan_object(obrcon);
  if (du->is_dumpable_objref(obrcon))
    {
      for (auto v: *this)
        du->scan_value(v, depth+1);
    }
} // end Rps_ClosureZone::dump_scan



Hjson::Value
Rps_ClosureZone::dump_hjson(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  RPS_FATAL("unimplemented Rps_ClosureZone::dump_hjson");
#warning unimplemented Rps_ClosureZone::dump_hjson
} // end Rps_ClosureZone::dump_hjson


////////////////
void
Rps_ObjectZone::dump_scan(Rps_Dumper*du, unsigned depth) const
{
  RPS_ASSERT(du != nullptr);
  RPS_FATAL("unimplemented Rps_ObjectZone::dump_scan depth %u", depth);
#warning unimplemented Rps_ObjectZone::dump_scan
}

Hjson::Value
Rps_ObjectZone::dump_hjson(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  RPS_FATAL("unimplemented Rps_ObjectZone::dump_hjson");
#warning unimplemented Rps_ObjectZone::dump_hjson
} // end Rps_ObjectZone::dump_hjson

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
    {
      std::lock_guard<std::mutex> gu(ld_mtx);
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



/// loading of class information payload; see
/// Rps_PayloadClassInfo::dump_hjson_content in objects_rps.cc
void rpsldpy_class(Rps_ObjectZone*obz, Rps_Loader*ld, const Hjson::Value& hjv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(hjv.type() == Hjson::Value::Type::MAP);
  if (!hjv.is_map_with_key("superclass") || !hjv.is_map_with_key("methodict"))
    RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has incomplete payload"
                 << std::endl
                 << " hjv " <<Hjson::MarshalJson(hjv));
  auto paylclainf = obz->put_new_payload<Rps_PayloadClassInfo>();
  RPS_ASSERT(paylclainf != nullptr);
  auto obsuperclass = Rps_ObjectRef(hjv["superclass"], ld);
  RPS_ASSERT(obsuperclass);
  paylclainf->put_superclass(obsuperclass);
  Hjson::Value hjvmethodict = hjv["methodict"];
  size_t nbmeth = 0;
  if (!hjvmethodict.is_vector(&nbmeth))
    RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad methodict"
                 << std::endl
                 << " hjvmethodict " <<Hjson::MarshalJson(hjvmethodict));
  for (int methix=0; methix<(int)nbmeth; methix++)
    {
      size_t curlen=0;
      Hjson::Value hjvcurmeth = hjvmethodict[methix];
      if (!hjvcurmeth.is_map(&curlen) || curlen != 2
          || !hjvcurmeth.is_map_with_key("methosel")
          || !hjvcurmeth.is_map_with_key("methclos")
         )
        RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " has bad methodict entry#" << methix
                     << std::endl
                     << " hjvcurmeth " <<Hjson::MarshalJson(hjvcurmeth));
      auto obsel = Rps_ObjectRef(hjvcurmeth["methosel"], ld);
      auto valclo = Rps_Value(hjvcurmeth["methclos"], ld);
      if (!obsel || !valclo.is_closure())
        RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " with bad methodict entry#" << methix
                     << std::endl
                     << " hjvcurmeth " <<Hjson::MarshalJson(hjvcurmeth));
      paylclainf->put_own_method(obsel,valclo);
    }
} // end of rpsldpy_class




/// loading of set of objects payload
void rpsldpy_setob(Rps_ObjectZone*obz, Rps_Loader*ld, const Hjson::Value& hjv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(hjv.type() == Hjson::Value::Type::MAP);
  RPS_FATAL("rpsldpy_setob unimplemented");
#warning rpsldpy_setob unimplemented
} // end of rpsldpy_setob



//////////////////////////////////////////////////////////// end of file store_rps.cc


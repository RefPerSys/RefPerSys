/****************************************************************
 * file store_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for the persistent store, in JSON format.  See
 *      also http://json.org/ &
 *      https://github.com/open-source-parsers/jsoncpp/
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


// same as used in rps_manifest.json file
#define RPS_MANIFEST_FORMAT "RefPerSysFormat2019A"

Json::Value rps_string_to_json(const std::string&str)
{
  Json::CharReaderBuilder jsonreaderbuilder;
  std::unique_ptr<Json::CharReader> pjsonreader(jsonreaderbuilder.newCharReader());
  Json::Value jv;
  Json::String errstr;
  RPS_ASSERT(pjsonreader);
  if (!pjsonreader->parse(str.c_str(), str.c_str() + str.size(), &jv, &errstr))
    throw std::runtime_error(std::string("JSON parsing error:") + errstr);
  return jv;
} // end rps_string_to_json


std::string
rps_json_to_string(const Json::Value&jv)
{
  Json::StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = " ";
  auto str = Json::writeString(builder, jv);
  return str;
} // end rps_json_to_string

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
  void parse_json_buffer_second_pass (Rps_Id spacid, unsigned lineno,
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
  void load_install_roots(void);
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
} // end Rps_Loader::load_real_path




std::string
Rps_Loader::space_file_path(Rps_Id spacid)
{
  if (!spacid.valid())
    throw std::runtime_error("Rps_Loader::space_file_path invalid spacid");
  return std::string{"persistore/sp"} + spacid.to_string() + "-rps.json";
} // end Rps_Loader::space_file_path



bool
Rps_Loader::is_object_starting_line(Rps_Id spacid, unsigned lineno, const std::string&linbuf, Rps_Id*pobid)
{
  const char*reason = nullptr;
  if (pobid)
    *pobid = Rps_Id(nullptr);
  Rps_Id oid;
  const char*end=nullptr;
  bool ok=false;
  if (linbuf[0] != '/' || linbuf[1] != '/'
      || linbuf[2] != '+'
      || linbuf[3] != 'o'
      || linbuf[4] != 'b')
    return false;
  if (linbuf.size() < strlen ("//+ob") + Rps_Id::nbchars)
    {
      reason = "too short";
      goto bad;
    }
  {
    Rps_Id tempoid(linbuf.c_str()+strlen("//+ob"), &end, &ok);
    if (!end || (*end && !isspace(*end)))
      {
        reason= "too long";
        goto bad;
      }
    oid = tempoid;
  };
  if (!ok)
    {
      reason= "bad oid";
      goto bad;
    }
  if (!oid.valid())
    {
      reason= "invalid oid";
      goto bad;
    }
  if (pobid)
    *pobid = oid;
  return true;
bad:
  if (!reason)
    reason="???";
  RPS_WARNOUT("bad object starting line in space " << spacid << " line#" << lineno
              << " - " << reason
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
                        << " curobjid:" << curobjid
                        << " count:" << (obcnt+1));
          if (RPS_UNLIKELY(obcnt == 0))
            {
              Json::Value prologjson = rps_string_to_json(prologstr);
              if (prologjson.type() != Json::objectValue)
                RPS_FATAL("Rps_Loader::first_pass_space %s bad Json type #%d",
                          spacepath.c_str(), (int)prologjson.type());
              Json::Value formatjson = prologjson["format"];
              if (formatjson.type() !=Json::stringValue)
                RPS_FATALOUT("space file " << spacepath
                             << " with bad format type#" << (int)formatjson.type());
              if (formatjson.asString() != RPS_MANIFEST_FORMAT)
                RPS_FATALOUT("space file " << spacepath
                             << "should have format: "
                             << RPS_MANIFEST_FORMAT
                             << " but got "
                             << formatjson);
              if (prologjson["spaceid"].asString() != spacid.to_string())
                RPS_FATAL("spacefile %s should have spaceid: '%s' but got '%s'",
                          spacepath.c_str (), spacid.to_string().c_str(),
                          prologjson["spaceid"].asString().c_str());
              Json::Value nbobjectsjson =  prologjson["nbobjects"];
              expectedcnt =nbobjectsjson.asInt();
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
Rps_Loader::parse_json_buffer_second_pass (Rps_Id spacid, unsigned lineno,
    Rps_Id objid, const std::string& objbuf)
{
  /// RPS_INFORMOUT("parse_json_buffer_second_pass start spacid=" << spacid
  ///               << " lineno=" <<lineno
  ///               << " objid=" <<objid
  ///               << " objbuf:\n" << objbuf);
  Json::Value objjson = rps_string_to_json(objbuf);
  if (objjson.type() != Json::objectValue)
    RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                 << " lineno:" << lineno
                 << " objid:" << objid
                 << " bad objbuf:" << std::endl
                 << objbuf);
  Json::Value oidjson = objjson["oid"];
  if (oidjson.asString() != objid.to_string())
    RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                 << " lineno:" << lineno
                 << " objid:" << objid
                 << " unexpected");
  auto obz = Rps_ObjectZone::find(objid);
  RPS_ASSERT (obz);
  auto obzspace = Rps_ObjectZone::find(spacid);
  obz->loader_set_class (this, Rps_ObjectRef(objjson["class"], this));
  RPS_ASSERT (obzspace);
  obz->loader_set_space (this, obzspace);
  obz->loader_set_mtime (this, objjson["mtime"].asDouble());
  if (objjson.isMember("components"))
    {
      auto compjson = objjson["components"];
      int siz= 0;
      if (compjson.isArray())
        {
          siz = compjson.size();
          obz->loader_reserve_comps(this, (unsigned)siz);
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto valcomp = Rps_Value(compjson[ix], this);
              obz->loader_add_comp(this, valcomp);
            }
        }
    }
  if (objjson.isMember("attributes"))
    {
      auto attrjson = objjson["attributes"];
      int siz= 0;
      if (attrjson.isArray())
        {
          siz = attrjson.size();
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto entjson = attrjson[ix];
              int entsiz= 0;
              if (entjson.isObject()
                  && (entsiz=entjson.size()) >= 2
                  && entjson.isMember("at")
                  && entjson.isMember("va")
                 )
                {
                  auto atobr =  Rps_ObjectRef(entjson["at"], this);
                  auto atval = Rps_Value(entjson["va"], this);
                  obz->loader_put_attr(this, atobr, atval);
                }
            }
        }
    }
  if (objjson.isMember("payload"))
    {
      rpsldpysig_t*pldfun = nullptr;
      auto paylstr = objjson["payload"].asString();
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
                  RPS_FATALOUT("Rps_Loader::parse_json_buffer_second_pass spacid:" << spacid
                               << " lineno:" << lineno
                               << " objid:" << objid
                               << " invalid id payload:" << paylstr);
              }
            else
              RPS_FATALOUT("Rps_Loader::parse_json_buffer_second_pass spacid:" << spacid
                           << " lineno:" << lineno
                           << " objid:" << objid
                           << " invalid payload:" << paylstr);
          }
        else
          pldfun = ldit->second;
      };
      if (pldfun)
        {
          (*pldfun)(obz,this,objjson,spacid,lineno);
        }
      else
        {
          RPS_FATALOUT("Rps_Loader::parse_json_buffer_second_pass in spacid=" << spacid
                       << " lineno:" << lineno
                       << " objid:" << objid
                       << " payload: " << paylstr
                       << " without loading function"
                       << std::endl);
        }
    }
} // end of Rps_Loader::parse_json_buffer_second_pass



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
      if (linbuf.size() > 0 && linbuf[0] == '#')
        continue;
      Rps_Id curobjid;
      if (is_object_starting_line(spacid,lincnt,linbuf,&curobjid))
        {
          if (objbuf.size() > 0 && prevoid && prevlin>0)
            {
              try
                {
                  parse_json_buffer_second_pass(spacid, prevlin, prevoid, objbuf);
                }
              catch (const std::exception& exc)
                {
                  RPS_FATALOUT("failed second pass in " << spacid
                               << " prevoid:" << prevoid
                               << " line#" << prevlin
                               << std::endl
                               << "... got exception of type "
                               << typeid(exc).name()
                               << ":"
                               << exc.what());
                };
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
      try
        {
          parse_json_buffer_second_pass(spacid, prevlin, prevoid, objbuf);
        }
      catch (const std::exception& exc)
        {
          RPS_FATALOUT("failed second pass in " << spacid
                       << " prevoid:" << prevoid
                       << " line#" << prevlin
                       << std::endl
                       << "... got exception of type "
                       << typeid(exc).name()
                       << ":"
                       << exc.what());
        };
      prevoid = Rps_Id(nullptr);
    };
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



Rps_Value::Rps_Value(const Json::Value &jv, Rps_Loader*ld)
  : Rps_Value(nullptr)
{
  RPS_ASSERT(ld != nullptr);
  std::int64_t i=0;
  std::string str = "";
  std::size_t siz=0;
  Json::Value jcomp;
  Json::Value jvtype;
  if (jv.isInt64())
    {
      i = jv.asInt64();
      *this = Rps_Value(i, Rps_IntTag{});
      return;
    }
  else if (jv.isDouble())
    {
      double d = jv.asDouble();
      RPS_ASSERT(!std::isnan(d));
      *this = Rps_Value(d, Rps_DoubleTag{});
      return;
    }
  else if (jv.isNull())
    {
      *this = Rps_Value(nullptr);
      return;
    }
  else if (jv.isString())
    {
      str = jv.asString();
      if (str.size() == Rps_Id::nbchars + 1 && str[0] == '_'
          && std::all_of(str.begin()+1, str.end(),
                         [](char c)
      {
        return strchr(Rps_Id::b62digits, c) != nullptr;
        }))
      {
        *this = Rps_ObjectValue(Rps_ObjectRef(jv, ld));
        return;
      }
      *this = Rps_StringValue(str);
      return;
    }
  else if (jv.isObject() && jv.size()==1 && jv.isMember("string")
           && (jcomp=jv["string"]).isString())
    {
      str=jcomp.asString();
      *this = Rps_StringValue(str);
      return;
    }
  else if (jv.isObject() &&  jv.isMember("vtype")
           && ((jvtype=jv["vtype"]).isString())
           && !(str=jvtype.asString()).empty())
    {
      if (str == "set" && siz==2 && jv.isMember("elem")
          && (jcomp=jv["elem"]).isArray()
          && (siz=jcomp.size()))
        {
          std::set<Rps_ObjectRef> setobr;
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto obrelem = Rps_ObjectRef(jcomp[ix], ld);
              if (obrelem)
                setobr.insert(obrelem);
            }
          *this= Rps_SetValue(setobr);
          return;
        }
      else if (str == "tuple" && siz==2 && jv.isMember("elem")
               && (jcomp=jv["comp"]).isArray()
               && (siz=jcomp.size()))
        {
          std::vector<Rps_ObjectRef> vecobr;
          vecobr.reserve(siz);
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto obrcomp = Rps_ObjectRef(jcomp[ix], ld);
              vecobr.push_back(obrcomp);
            };
          *this= Rps_TupleValue(vecobr);
          return;
        }
      else if (str == "closure" && siz==3
               && jv.isMember("fn")
               && jv.isMember("env"))
        {
          auto jfn = jv["fn"];
          auto jenv = jv["env"];
          auto funobr = Rps_ObjectRef(jfn, ld);
          if (jenv.isArray())
            {
              auto siz = jenv.size();
              std::vector<Rps_Value> vecenv;
              vecenv.reserve(siz+1);
              for (int ix=0; ix <(int)siz; ix++)
                {
                  auto curval = Rps_Value(jenv[ix], ld);
                  vecenv.push_back(curval);
                };
              *this = Rps_ClosureValue(funobr, vecenv);
              return;
            }
        }
    }
#warning Rps_Value::Rps_Value(const Json::Value &jv, Rps_Loader*ld) unimplemented
  RPS_WARN("unimplemented Rps_Value::Rps_Value(const Json::Value &jv, Rps_Loader*ld)");
} // end of Rps_Value::Rps_Value(const Json::value &jv, Rps_Loader*ld)


Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)
  : Rps_ObjectRef(nullptr)
{
  RPS_ASSERT(ld != nullptr);
  std::string str = "";
  Rps_Id oid;
  if (jv.isString() && !((str=jv.asString()).empty()) && (oid = Rps_Id(str)).valid())
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
  RPS_WARN("partly unimplemented Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)");
  throw  std::runtime_error("partly unimplemented Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)");
#warning partly unimplemented Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)
} // end Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)





//////////////////////////////////////////////// dumper
class Rps_Dumper
{
  friend void rps_dump_into (const std::string dirpath);
  friend void rps_dump_scan_object(Rps_Dumper*, Rps_ObjectRef obr);
  friend void rps_dump_scan_value(Rps_Dumper*, Rps_Value obr, unsigned depth);
  friend Json::Value rps_dump_json_value(Rps_Dumper*, Rps_Value val);
  friend Json::Value rps_dump_json_objectref(Rps_Dumper*, Rps_ObjectRef obr);
  std::string du_topdir;
  std::recursive_mutex du_mtx;
  std::unordered_map<Rps_Id, Rps_ObjectRef,Rps_Id::Hasher> du_mapobjects;
  std::deque<Rps_ObjectRef> du_scanque;
  std::string du_tempsuffix;
  // we maintain the set of opened file paths, since they are opened
  // with the temporary suffix above, and renamed by
  // rename_opened_files below.
  std::set<std::string> du_openedpathset;
  // a random temporary suffix for written files
  static std::string make_temporary_suffix(void)
  {
    Rps_Id randid(nullptr);
    char buf[32];
    snprintf(buf, sizeof(buf), "%.7s-p%d%%",
             randid.to_string().c_str(), (int)getpid());
    return std::string(buf);
  };
private:
  std::string temporary_opened_path(const std::string& relpath) const
  {
    RPS_ASSERT(relpath.size()>0 && relpath[0] != '/');
    return du_topdir + "/" + relpath + du_tempsuffix;
  };
  void scan_roots(void);
  Rps_ObjectRef pop_object_to_scan(void);
  void scan_loop_pass(void);
  void scan_object_contents(Rps_ObjectRef obr);
  std::unique_ptr<std::ofstream> open_output_file(const std::string& relpath);
  void rename_opened_files(void);
public:
  std::string get_temporary_suffix(void) const
  {
    return du_tempsuffix;
  };
  std::string get_top_dir() const
  {
    return du_topdir;
  };
  Rps_Dumper(const std::string&topdir) :
    du_topdir(topdir), du_mtx(), du_mapobjects(), du_scanque(),
    du_tempsuffix(make_temporary_suffix()), du_openedpathset() {};
  void scan_object(const Rps_ObjectRef obr);
  void scan_value(const Rps_Value val, unsigned depth);
  Json::Value json_value(const Rps_Value val);
  Json::Value json_objectref(const Rps_ObjectRef obr);
  bool is_dumpable_objref(const Rps_ObjectRef obr);
  bool is_dumpable_value(const Rps_Value val);
};				// end class Rps_Dumper


void
Rps_Dumper::scan_object(const Rps_ObjectRef obr)
{
  if (!obr)
    return;
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  if (du_mapobjects.find(obr->oid()) != du_mapobjects.end())
    return;
  if (!obr->get_space()) // transient
    return;
  du_mapobjects.insert({obr->oid(), obr});
  du_scanque.push_back(obr);
  RPS_INFORMOUT("Rps_Dumper::scan_object adding oid " << obr->oid());
} // end Rps_Dumper::scan_object

void
Rps_Dumper::scan_value(const Rps_Value val, unsigned depth)
{
  if (!val || val.is_empty() || !val.is_ptr())
    return;
  val.to_ptr()->dump_scan(this,depth);
} // end Rps_Dumper::scan_value

Json::Value
Rps_Dumper::json_value(Rps_Value val)
{
  if (!val || val.is_empty())
    return Json::Value(nullptr);
  else if (val.is_int())
    return Json::Value(val.as_int());
  else if (val.is_ptr() && is_dumpable_value(val))
    {
      return val.to_ptr()->dump_json(this);
    }
  else
    return Json::Value(nullptr);
} // end Rps_Dumper::json_value

bool
Rps_Dumper::is_dumpable_objref(const Rps_ObjectRef obr)
{
  if (!obr)
    return false;
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  if (du_mapobjects.find(obr->oid()) != du_mapobjects.end())
    return true;
  auto obrspace = obr->get_space();
  if (!obrspace) // transient
    return false;
  return true;
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

std::unique_ptr<std::ofstream>
Rps_Dumper::open_output_file(const std::string& relpath)
{
  RPS_ASSERT(relpath.size()>1 && relpath[0] != '/');
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  if (RPS_UNLIKELY(du_openedpathset.find(relpath) != du_openedpathset.end()))
    {
      RPS_WARNOUT("duplicate opened dump file " << relpath);
      throw std::runtime_error(std::string{"duplicate opened dump file "} + relpath);
    }
  std::string tempathstr =  temporary_opened_path(relpath);
  auto poutf= std::make_unique<std::ofstream>(tempathstr);
  if (!poutf || !poutf->is_open())
    {
      RPS_WARNOUT("dump failed to open " << tempathstr);
      throw std::runtime_error(std::string{"duplicate failed to open "} + tempathstr + ":" + strerror(errno));
    }
  du_openedpathset.insert(relpath);
  return poutf;
} // end Rps_Dumper::open_output_file


void
Rps_Dumper::rename_opened_files(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  for (std::string curelpath: du_openedpathset)
    {
      std::string curpath = du_topdir + "/" + curelpath;
      if (!access(curpath.c_str(), F_OK))
        {
          std::string bak0path = curpath + "~";
          if (!access(bak0path.c_str(), F_OK))
            {
              std::string bak1path = bak0path + "~";
              (void) rename(bak0path.c_str(), bak1path.c_str());
            };
          if (rename(curpath.c_str(), bak0path.c_str()))
            RPS_WARNOUT("dump failed to backup " << curpath << " to " << bak0path << ":" << strerror(errno));
        };
      std::string tempath = temporary_opened_path(curelpath);
      if (rename(tempath.c_str(), curpath.c_str()))
        RPS_FATALOUT("dump failed to rename " << tempath << " as " << curpath);
    };
  du_openedpathset.clear();
} // end Rps_Dumper::rename_opened_files


//////////////// public interface to dumper::::
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

Json::Value
rps_dump_json_value(Rps_Dumper*du, Rps_Value val)
{
  RPS_ASSERT(du != nullptr);
  if (!val || !rps_is_dumpable_value(du,val))
    return Json::Value(nullptr);
  else return du->json_value(val);
} // end rps_dump_json_value

Json::Value
rps_dump_json_objectref(Rps_Dumper*du, Rps_ObjectRef obr)
{
  RPS_ASSERT(du != nullptr);
  if (!obr || !rps_is_dumpable_objref(du,obr))
    return Json::Value(nullptr);
  else
    return Json::Value(obr->oid().to_string());
} // end rps_dump_json_objectref

void
Rps_TupleOb::dump_scan(Rps_Dumper*du, unsigned) const
{
  RPS_ASSERT(du != nullptr);
  for (auto obr: *this)
    du->scan_object(obr);
}

Json::Value
Rps_TupleOb::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  auto jtup = Json::Value(Json::objectValue);
  jtup["vtype"] = Json::Value("tuple");
  auto jvec = Json::Value(Json::arrayValue);
  jvec.resize(cnt()); // reservation
  int ix=0;
  for (auto obr: *this)
    if (rps_is_dumpable_objref(du,obr))
      jvec[ix++] = rps_dump_json_objectref(du,obr);
  jvec.resize(ix);  // shrink to fit
  jtup["comp"] = jvec;
  return jtup;
} // end Rps_TupleOb::dump_json


////////////////
void
Rps_SetOb::dump_scan(Rps_Dumper*du, unsigned) const
{
  RPS_ASSERT(du != nullptr);
  for (auto obr: *this)
    du->scan_object(obr);
}

Json::Value
Rps_SetOb::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  auto  jset = Json::Value(Json::objectValue);
  jset["vtype"] = Json::Value("set");
  auto jvec = Json::Value(Json::arrayValue);
  for (auto obr: *this)
    if (rps_is_dumpable_objref(du,obr))
      jvec.append(rps_dump_json_objectref(du,obr));
  jset["elem"] = jvec;
  return jset;
} // end Rps_SetOb::dump_json

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



Json::Value
Rps_ClosureZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (!rps_is_dumpable_objref(du,conn()))
    return Json::Value(nullptr);
  auto  hjclo = Json::Value(Json::objectValue);
  hjclo["vtype"] = Json::Value("closure");
  hjclo["fn"] = rps_dump_json_objectref(du,conn());
  auto jvec = Json::Value(Json::arrayValue);
  for (Rps_Value sonval: *this)
    jvec.append(rps_dump_json_value(du,sonval));
  hjclo["env"] = jvec;
  return hjclo;
} // end Rps_ClosureZone::dump_json


////////////////
void
Rps_ObjectZone::dump_scan(Rps_Dumper*du, unsigned) const
{
  RPS_ASSERT(du != nullptr);
  rps_dump_scan_object(du,Rps_ObjectRef(this));
}

Json::Value
Rps_ObjectZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  return rps_dump_json_objectref(du,Rps_ObjectRef(this));
} // end Rps_ObjectZone::dump_json

//////////////////////////////////////////////////////////////// dump

void
Rps_Dumper::scan_roots(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  rps_each_root_object([=](Rps_ObjectRef obr)
  {
    rps_dump_scan_object(this,obr);
  });
} // end Rps_Dumper::scan_roots

Rps_ObjectRef
Rps_Dumper::pop_object_to_scan(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  if (du_scanque.empty())
    return Rps_ObjectRef(nullptr);
  auto obr = du_scanque.front();
  du_scanque.pop_front();
  return obr;
} // end Rps_Dumper::pop_object_to_scan


void
Rps_Dumper::scan_loop_pass(void)
{
  Rps_ObjectRef curobr;
  int count=0;
  // no locking here, it happens in pop_object_to_scan & scan_object_contents
  RPS_INFORMOUT("scan_loop_pass start");
  while ((curobr=pop_object_to_scan()))
    {
      count++;
      RPS_INFORMOUT("scan_loop_pass count#" << count
                    << " curobr=" << curobr->oid());
      scan_object_contents(curobr);
    };
  RPS_INFORMOUT("scan_loop_pass end count#" << count);
} // end Rps_Dumper::scan_loop_pass

void
Rps_Dumper::scan_object_contents(Rps_ObjectRef obr)
{
  obr->dump_scan_contents(this);
} // end Rps_Dumper::scan_object_contents



////////////////////////////////////////////////////////////////
void rps_dump_into (const std::string dirpath)
{
  std::string realdirpath;
  {
    char* rp = realpath(dirpath.c_str(), nullptr);
    if (!rp)
      {
        RPS_WARN("cannot dump into %s: %m", dirpath.c_str());
        throw std::runtime_error(std::string{"cannot dump into "} + dirpath);
      };
    realdirpath=rp;
    free (rp);
  }
  std::string cwdpath;
  {
    // not very good, but in practice good enough before bootstrapping
    // see https://softwareengineering.stackexchange.com/q/289427/40065
    char cwdbuf[256];
    memset(cwdbuf, 0, sizeof(cwdbuf));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
      RPS_FATAL("getcwd failed: %m");
    cwdpath = std::string(cwdbuf);
  }
  /// ensure that realdirpath exists
  {
    RPS_ASSERT(strrchr(realdirpath.c_str(), '/') != nullptr);
  }
  Rps_Dumper dumper(realdirpath);
  RPS_INFORMOUT("start dumping into " << dumper.get_top_dir()
                << " with temporary suffix " << dumper.get_temporary_suffix());
  try
    {
      if (realdirpath != cwdpath)
        {
          QDir realqdir{QString(realdirpath.c_str())};
          if (!realqdir.mkpath("."))
            {
              RPS_WARNOUT("failed to make dump directory " << realdirpath
                          << ":" << strerror(errno));
              throw std::runtime_error(std::string{"failed to make dump directory:"} + realdirpath);
            }
          else
            RPS_INFORMOUT("made real dump directory: " << realdirpath);
        }
      dumper.scan_roots();
      dumper.scan_loop_pass();
    }
  catch (const std::exception& exc)
    {
      RPS_WARNOUT("failure in dump to " << dumper.get_top_dir()
                  << std::endl
                  << "... got exception of type "
                  << typeid(exc).name()
                  << ":"
                  << exc.what());
      throw;
    };
  ///
  RPS_FATAL("unimplemented rps_dump_into '%s'", dirpath.c_str());
#warning rps_dump_into unimplemented
} // end of rps_dump_into

//////////////////////////////////////////////////////////////// load

void
Rps_Loader::parse_manifest_file(void)
{
  std::string manifpath = ld_topdir + "/" + RPS_MANIFEST_JSON;
  if (access(manifpath.c_str(), R_OK))
    RPS_FATAL("Rps_Loader::parse_manifest_file cannot access %s - %m",
              manifpath.c_str());
  std::string manifstr = string_of_loaded_file(RPS_MANIFEST_JSON);
  if (manifstr.size() < 20)
    RPS_FATAL("Rps_Loader::parse_manifest_file nearly empty file %s",
              manifpath.c_str());
  Json::Value manifjson = rps_string_to_json(manifstr);
  if (manifjson.type () != Json::objectValue)
    RPS_FATAL("Rps_Loader::parse_manifest_file wants a Json object in %s",
              manifpath.c_str());
  if (manifjson["format"].asString() != RPS_MANIFEST_FORMAT)
    RPS_FATAL("manifest map in %s should have format: '%s' but got:\n"
              "%s",
              manifpath.c_str (), RPS_MANIFEST_FORMAT,
              manifjson["format"].toStyledString().c_str());
  /// parse spaceset
  {
    auto spsetjson = manifjson["spaceset"];
    if (spsetjson.type() !=  Json::arrayValue)
      RPS_FATAL("manifest map in %s should have spaceset: [...]",
                manifpath.c_str ());
    size_t sizespset = spsetjson.size();
    for (int ix=0; ix<(int)sizespset; ix++)
      {
        std::string curspidstr = spsetjson[ix].asString();
        Rps_Id curspid (curspidstr);
        RPS_ASSERT(curspid);
        ld_spaceset.insert({curspid});
      }
  }
  /// parse globalroots
  {
    auto globrootsjson = manifjson["globalroots"];
    if (globrootsjson.type() !=  Json::arrayValue)
      RPS_FATAL("manifest map in %s should have globalroots: [...]",
                manifpath.c_str ());
    size_t sizeglobroots = globrootsjson.size();
    for (int ix=0; ix<(int)sizeglobroots; ix++)
      {
        std::string curgrootidstr = globrootsjson[ix].asString();
        Rps_Id curgrootid (curgrootidstr);
        RPS_ASSERT(curgrootid);
        ld_globrootsidset.insert(curgrootid);
      }
  }
  /// parse plugins
  {
    auto pluginsjson = manifjson["plugins"];
    if (pluginsjson.type() !=  Json::arrayValue)
      RPS_FATAL("manifest map in %s should have plugins: [...]",
                manifpath.c_str ());
    size_t sizeplugins = pluginsjson.size();
    {
      std::lock_guard<std::mutex> gu(ld_mtx);
      for (int ix=0; ix<(int)sizeplugins; ix++)
        {
          std::string curpluginidstr = pluginsjson[ix].asString();
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
  RPS_INFORMOUT("Rps_Loader::parse_manifest_file parsed:"
                << std::endl
                << manifjson
                << std::endl);
#warning incomplete Rps_Loader::parse_manifest_file
} // end Rps_Loader::parse_manifest_file



void Rps_Loader::load_install_roots(void)
{
  for (Rps_Id curootid: ld_globrootsidset)
    {
      std::lock_guard<std::mutex> gu(ld_mtx);
      Rps_ObjectRef curootobr = find_object_by_oid(curootid);
      RPS_ASSERT(curootobr);
      rps_add_root_object (curootobr);
    };
} // end Rps_Loader::load_install_roots

void rps_load_from (const std::string& dirpath)
{
  Rps_Loader loader(dirpath);
  try
    {
      loader.parse_manifest_file();
      loader.load_all_state_files();
      loader.load_install_roots();
    }
  catch (const std::exception& exc)
    {
      RPS_FATALOUT("failed to load " << dirpath
                   << "," << std::endl
                   << "... got exception of type "
                   << typeid(exc).name()
                   << ":"
                   << exc.what());
    }
#warning rps_load_from unimplemented
} // end of rps_load_from



/// loading of class information payload; see
/// Rps_PayloadClassInfo::dump_json_content in objects_rps.cc
void rpsldpy_class(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (!jv.isMember("superclass") || !jv.isMember("methodict"))
    RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has incomplete payload"
                 << std::endl
                 << " jv " << (jv));
  auto paylclainf = obz->put_new_payload<Rps_PayloadClassInfo>();
  RPS_ASSERT(paylclainf != nullptr);
  auto obsuperclass = Rps_ObjectRef(jv["superclass"], ld);
  RPS_ASSERT(obsuperclass);
  paylclainf->put_superclass(obsuperclass);
  Json::Value jvmethodict = jv["methodict"];
  unsigned nbmeth = 0;
  if (!jvmethodict.isArray())
    RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad methodict"
                 << std::endl
                 << " jvmethodict " <<(jvmethodict));
  nbmeth = jvmethodict.size();
  for (int methix=0; methix<(int)nbmeth; methix++)
    {
      size_t curlen=0;
      Json::Value jvcurmeth = jvmethodict[methix];
      if (!jvcurmeth.isObject()
          || !jvcurmeth.isMember("methosel")
          || !jvcurmeth.isMember("methclos")
         )
        RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " has bad methodict entry#" << methix
                     << std::endl
                     << " jvcurmeth " << (jvcurmeth));
      auto obsel = Rps_ObjectRef(jvcurmeth["methosel"], ld);
      auto valclo = Rps_Value(jvcurmeth["methclos"], ld);
      if (!obsel || !valclo.is_closure())
        RPS_FATALOUT("rpsldpy_class: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " with bad methodict entry#" << methix
                     << std::endl
                     << " jvcurmeth: " <<jvcurmeth);
      paylclainf->put_own_method(obsel,valclo);
    }
} // end of rpsldpy_class




/// loading of set of objects payload
void rpsldpy_setob(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_FATAL("rpsldpy_setob unimplemented");
#warning rpsldpy_setob unimplemented
} // end of rpsldpy_setob


/// loading of space payload
void rpsldpy_space(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  auto paylspace = obz->put_new_payload<Rps_PayloadSpace>();
  RPS_ASSERT(paylspace != nullptr);
} // end of rpsldpy_setob



//////////////////////////////////////////////////////////// end of file store_rps.cc


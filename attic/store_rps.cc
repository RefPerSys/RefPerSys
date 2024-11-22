/****************************************************************
 * file store_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
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
 *      © Copyright 2019 - 2022 The Reflective Persistent System Team
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
  JSONCPP_STRING errstr;
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
  /// dlsym and dlopen are not reentrant, so we need a mutex; is is
  /// recursive since we might lock it in a nested way
  std::recursive_mutex ld_mtx;
  /// set of space ids
  std::set<Rps_Id> ld_spaceset;
  /// set of global roots id
  std::set<Rps_Id> ld_globrootsidset;
  /// mapping from plugins id to their dlopen-ed handle
  std::map<Rps_Id,void*> ld_pluginsmap;
  /// map of loaded objects
  std::map<Rps_Id,Rps_ObjectRef> ld_mapobjects;
  /// double ended queue of todo chunks in second pass
  struct todo_st
  {
    double todo_addtime;
    std::function<void(Rps_Loader*)> todo_fun;
    todo_st() : todo_addtime(0.0), todo_fun() {};
    todo_st(double d, std::function<void(Rps_Loader*)> f)
      : todo_addtime(d), todo_fun(f) {};
  };
  std::deque<struct todo_st> ld_todoque;
  unsigned ld_todocount;
  static constexpr unsigned ld_maxtodo = 1<<20;
  /// dictionary of payload loaders - used as a cache to avoid most dlsym-s
  std::map<std::string,rpsldpysig_t*> ld_payloadercache;
  bool is_object_starting_line(Rps_Id spacid, unsigned lineno, const std::string&linbuf, Rps_Id*pobid);
  Rps_ObjectRef fetch_one_constant_at(const char*oid,int lin);
  void parse_json_buffer_second_pass (Rps_Id spacid, unsigned lineno,
                                      Rps_Id objid, const std::string& objbuf, unsigned count);
public:
  Rps_Loader(const std::string&topdir);
  ~Rps_Loader();
  void parse_manifest_file(void);
  void parse_user_manifest(const std::string&path);
  void first_pass_space(Rps_Id spacid);
  void initialize_constant_objects(void);
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
  void add_todo(const std::function<void(Rps_Loader*)>& todofun);
  // run some todo functions, return the number of remaining ones
  int run_some_todo_functions(void);
  void load_install_roots(void);
  unsigned nb_loaded_objects(void) const
  {
    return ld_mapobjects.size();
  };
};				// end class Rps_Loader

Rps_Loader::Rps_Loader(const std::string&topdir) :
  ld_topdir(topdir),
  ld_mtx(),
  ld_spaceset(),
  ld_globrootsidset(),
  ld_pluginsmap(),
  ld_mapobjects(),
  ld_todoque(),
  ld_todocount(0),
  ld_payloadercache()
{
  RPS_DEBUG_LOG(LOAD, "Rps_Loader constr topdir=" << topdir
                << " this@" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_Loader constr"));
} // end Rps_Loader::Rps_Loader

Rps_Loader::~Rps_Loader()
{
  RPS_DEBUG_LOG(LOAD, "Rps_Loader destr topdir=" << ld_topdir
                << " this@" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_Loader constr"));
} // end Rps_Loader::~Rps_Loader

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
      RPS_DEBUG_LOG(LOAD, "load_real_path path=" << path << " -> " << restr);
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
  RPS_DEBUG_LOG(LOAD, "first_pass_space start spacepath=" << spacepath);
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
      if (RPS_UNLIKELY(obcnt == 0))
        {
          prologstr += linbuf;
          prologstr += '\n';
        }
      Rps_Id curobjid;
      if (is_object_starting_line(spacid, lincnt, linbuf, &curobjid))
        {
          RPS_DEBUG_LOG(LOAD, "firstpass got ob spacid:" << spacid
                        << " linbuf: " << linbuf
                        << " lincnt#" << lincnt
                        << " curobjid:" << curobjid
                        << " count:" << (obcnt+1));
          if (RPS_UNLIKELY(obcnt == 0))
            {
              Json::Value prologjson;
              try
                {
                  prologjson = rps_string_to_json(prologstr);
                  if (prologjson.type() != Json::objectValue)
                    RPS_FATAL("Rps_Loader::first_pass_space %s line#%d bad Json type #%d",
                              spacepath.c_str(), (int)lincnt, (int)prologjson.type());
                }
              catch (std::exception& exc)
                {
                  RPS_FATALOUT("Rps_Loader::first_pass_space " << " spacepath:" << spacepath
                               << " line#" << lincnt
                               << " failed to parse: " << exc.what());
                };
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
  RPS_DEBUG_LOG(LOAD, "first_pass_space end spacepath=" << spacepath << " obcnt="<< obcnt << std::endl
                << "... read " << obcnt
                << " objects while loading first pass of " << spacepath);
} // end Rps_Loader::first_pass_space

void
Rps_Loader::add_todo(const std::function<void(Rps_Loader*)>& todofun)
{
  std::lock_guard<std::recursive_mutex> gu(ld_mtx);
  ld_todoque.push_back(todo_st{rps_elapsed_real_time(),todofun});
} // end Rps_Loader::add_todo


// return the number of remaining todo functions
int
Rps_Loader::run_some_todo_functions(void)
{
  double startim =  rps_elapsed_real_time();
  constexpr int dosteps = 24;
  constexpr double doelaps = 0.05;
  int count=0;
  /// run at least the front todo entry
  {
    todo_st td;
    {
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      bool emptyq = ld_todoque.empty();
      if (emptyq)
        return 0;
      td = ld_todoque.front();
      ld_todoque.pop_front();
      if (ld_todocount++ > ld_maxtodo)
        RPS_FATALOUT("too many " << ld_todocount << " loader todo functions");
    }
    td.todo_fun(this);
    count++;
  }
  /// run more entries provided they have been added before start
  while (count < dosteps && rps_elapsed_real_time() - startim < doelaps)
    {
      todo_st td;
      {
        std::lock_guard<std::recursive_mutex> gu(ld_mtx);
        bool emptyq = ld_todoque.empty();
        if (emptyq)
          return 0;
        td = ld_todoque.front();
        if (td.todo_addtime > startim)
          return ld_todoque.size();
        ld_todoque.pop_front();
        if (ld_todocount++ > ld_maxtodo)
          RPS_FATALOUT("too many " << ld_todocount << " loader todo functions");
      }
      td.todo_fun(this);
      count++;
    }
  /// finally
  {
    std::lock_guard<std::recursive_mutex> gu(ld_mtx);
    return ld_todoque.size();
  }
} // end of Rps_Loader::run_some_todo_functions



void
rps_load_add_todo(Rps_Loader*ld,const std::function<void(Rps_Loader*)>& todofun)
{
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(todofun);
  ld->add_todo(todofun);
} // end rps_load_add_todo

void
Rps_Loader::initialize_constant_objects(void)
{
  std::lock_guard<std::recursive_mutex> gu(ld_mtx);
#define RPS_INSTALL_CONSTANT_OB(Oid) \
  rpskob##Oid = fetch_one_constant_at(#Oid, __LINE__);
#include "generated/rps-constants.hh"
} // end of Rps_Loader::initialize_constant_objects

Rps_ObjectRef
Rps_Loader::fetch_one_constant_at(const char*oidstr, int lin)
{
  const char*end = nullptr;
  bool ok = false;
  Rps_Id id(oidstr, &end, &ok);
  RPS_ASSERT(end && *end==(char)0 && ok);
  auto it = ld_mapobjects.find(id);
  if (it == ld_mapobjects.end())
    {
      RPS_WARNOUT("failed to fetch constant " << oidstr
                  << " at line " << lin << " of generated/rps-constants.hh");
      return nullptr;
    }
  else
    return it->second;
} // end Rps_Loader::fetch_one_constant_at



////////////////
void
Rps_Loader::parse_json_buffer_second_pass (Rps_Id spacid, unsigned lineno,
    Rps_Id objid, const std::string& objbuf, unsigned count)
{
  RPS_DEBUG_LOG(LOAD, "parse_json_buffer_second_pass start spacid=" << spacid << " #" << count
                << " lineno=" <<lineno
                << " objid=" <<objid
                << " objbuf:\n" << objbuf);
  Json::Value objjson;
  try
    {
      objjson = rps_string_to_json(objbuf);
      if (objjson.type() != Json::objectValue)
        RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                     << " lineno:" << lineno
                     << " objid:" << objid
                     << " bad objbuf:" << std::endl
                     << objbuf);
    }
  catch (std::exception& exc)
    {
      RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                   << " lineno:" << lineno
                   << " objid:" << objid
                   << " parse failure "
                   << exc.what()
                   << " with objbuf:" << std::endl
                   << objbuf);
    }
  Json::Value oidjson = objjson["oid"];
  if (oidjson.asString() != objid.to_string())
    RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                 << " lineno:" << lineno
                 << " objid:" << objid
                 << " unexpected");
  auto obz = Rps_ObjectZone::find(objid);
  if (!obz)
    RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                 << " lineno:" << lineno
                 << " unknown objid:" << objid);
  auto obzspace = Rps_ObjectZone::find(spacid);
  obz->loader_set_class (this, Rps_ObjectRef(objjson["class"], this));
  RPS_ASSERT (obzspace);
  obz->loader_set_space (this, obzspace);
  obz->loader_set_mtime (this, objjson["mtime"].asDouble());
  if (objjson.isMember("comps"))
    {
      auto compjson = objjson["comps"];
      int siz= 0;
      if (compjson.isArray())
        {
          siz = compjson.size();
          RPS_DEBUG_LOG(LOAD, "parse_json_buffer_second_pass obz=" << obz << " comps#" << siz);
          obz->loader_reserve_comps(this, (unsigned)siz);
          for (int ix=0; ix<(int)siz; ix++)
            {
              auto valcomp = Rps_Value(compjson[ix], this);
              obz->loader_add_comp(this, valcomp);
            }
        }
      else
        RPS_WARNOUT("parse_json_buffer_second_pass spacid=" << spacid
                    << " lineno:" << lineno
                    << " objid:" << objid
                    << " bad compjson:" << compjson);
    }
  if (objjson.isMember("attrs"))
    {
      auto attrjson = objjson["attrs"];
      int siz= 0;
      if (attrjson.isArray())
        {
          siz = attrjson.size();
          RPS_DEBUG_LOG(LOAD, "parse_json_buffer_second_pass obz=" << obz << " attrs#" << siz);
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
      else RPS_WARNOUT("parse_json_buffer_second_pass spacid=" << spacid
                         << " lineno:" << lineno
                         << " objid:" << objid
                         << " bad attrjson:" << attrjson);
    }
  if (objjson.isMember("magicattr"))
    {
      RPS_DEBUG_LOG(LOAD, "parse_json_buffer_second_pass magicattr objid=" << objid);
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      char getfunambuf[sizeof(RPS_GETTERFUN_PREFIX)+8+Rps_Id::nbchars];
      memset(getfunambuf, 0, sizeof(getfunambuf));
      char obidbuf[32];
      memset (obidbuf, 0, sizeof(obidbuf));
      objid.to_cbuf24(obidbuf);
      strcpy(getfunambuf, RPS_GETTERFUN_PREFIX);
      strcat(getfunambuf+strlen(RPS_GETTERFUN_PREFIX), obidbuf);
      RPS_ASSERT(strlen(getfunambuf)<sizeof(getfunambuf)-4);
      void*funad = dlsym(rps_proghdl, getfunambuf);
      if (!funad)
        RPS_FATALOUT("cannot dlsym " << getfunambuf << " for magic attribute getter of objid:" <<  objid
                     << " lineno:" << lineno << ", spacid:" << spacid
                     << ":: " << dlerror());
      obz->loader_put_magicattrgetter(this, reinterpret_cast<rps_magicgetterfun_t*>(funad));
    }
  if (objjson.isMember("applying"))
    {
      RPS_DEBUG_LOG(LOAD, "parse_json_buffer_second_pass applying objid=" << objid);
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      char appfunambuf[sizeof(RPS_APPLYINGFUN_PREFIX)+8+Rps_Id::nbchars];
      memset(appfunambuf, 0, sizeof(appfunambuf));
      char obidbuf[32];
      memset (obidbuf, 0, sizeof(obidbuf));
      objid.to_cbuf24(obidbuf);
      strcpy(appfunambuf, RPS_APPLYINGFUN_PREFIX);
      strcat(appfunambuf+strlen(RPS_APPLYINGFUN_PREFIX), obidbuf);
      RPS_ASSERT(strlen(appfunambuf)<sizeof(appfunambuf)-4);
      void*funad = dlsym(rps_proghdl, appfunambuf);
      if (!funad)
        RPS_FATALOUT("cannot dlsym " << appfunambuf << " for applying function of objid:" <<  objid
                     << " lineno:" << lineno << ", spacid:" << spacid
                     << ":: " << dlerror());
      obz->loader_put_applyingfunction(this, reinterpret_cast<rps_applyingfun_t*>(funad));
    }
  if (objjson.isMember("payload"))
    {
      rpsldpysig_t*pldfun = nullptr;
      auto paylstr = objjson["payload"].asString();
      {
        std::lock_guard<std::recursive_mutex> gu(ld_mtx);
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
  RPS_DEBUG_LOG(LOAD, "parse_json_buffer_second_pass end objid=" << objid << " #" << count
                << std::endl);
} // end of Rps_Loader::parse_json_buffer_second_pass

////////////////////////////////////////////////////////////////


void
Rps_Loader::second_pass_space(Rps_Id spacid)
{
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::second_pass_space start spacid:" << spacid
                << std::endl << RPS_FULL_BACKTRACE_HERE(0, "RpsLoader::second_pass_space"));
  auto spacepath = load_real_path(space_file_path(spacid));
  std::ifstream ins(spacepath);
  unsigned lincnt = 0;
  unsigned obcnt = 0;
  Rps_Id prevoid;
  unsigned prevlin=0;
  std::string objbuf;
  for (std::string linbuf; std::getline(ins, linbuf); )
    {
      lincnt++;
      if (linbuf.size() > 0 && linbuf[0] == '#')
        continue;
      Rps_Id curobjid;
      if (is_object_starting_line(spacid,lincnt,linbuf,&curobjid))
        {
          obcnt++;
          RPS_DEBUG_LOG(LOAD, "secondpass lincnt="<< lincnt
                        << " curobjid=" << curobjid
                        << " obcnt=" << obcnt);
          if (objbuf.size() > 0 && prevoid && prevlin>0)
            {
              try
                {
                  parse_json_buffer_second_pass(spacid, prevlin, prevoid, objbuf, obcnt);
                }
              catch (const std::exception& exc)
                {
                  RPS_FATALOUT("failed second pass in space " << spacid
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
          parse_json_buffer_second_pass(spacid, prevlin, prevoid, objbuf, obcnt);
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
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::second_pass_space end spacid:" << spacid);
} // end of Rps_Loader::second_pass_space


void
Rps_Loader::load_all_state_files(void)
{
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::load_all_state_files start this@" << (void*)this
                << std::endl << RPS_FULL_BACKTRACE_HERE(0, "RpsLoader::load_all_state_files"));
  int spacecnt1 = 0, spacecnt2 = 0;
  int todocount = 0;
  for (Rps_Id spacid: ld_spaceset)
    {
      first_pass_space(spacid);
      spacecnt1++;
    }
  RPS_NOPRINTOUT("loaded " << spacecnt1 << " space files in first pass");
  initialize_constant_objects();
  /// conceptually, the second pass might be done in parallel
  /// (multi-threaded, with different threads working on different
  /// spaces), but this require more clever locking and
  /// synchronization, so might be done after reaching our milestone#2
  for (Rps_Id spacid: ld_spaceset)
    {
      run_some_todo_functions();
      second_pass_space(spacid);
      spacecnt2++;
    }
  while (run_some_todo_functions()>0)
    {
      // we sleep a tiny bit, so elapsed time is growing...
      usleep(20);
    };
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::load_all_state_files end this@" << (void*)this);
  RPS_INFORMOUT("loaded " << spacecnt1 << " space files in second pass with "
                << ld_mapobjects.size() << " objects and " << todocount << " todos" << std::endl);
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::load_all_state_files end this@" << (void*)this
                << std::endl << RPS_FULL_BACKTRACE_HERE(0, "RpsLoader::load_all_state_files"));
} // end Rps_Loader::load_all_state_files


std::string
Rps_Loader::string_of_loaded_file(const std::string&relpath)
{

  RPS_DEBUG_LOG(LOAD, "Rps_Loader::string_of_loaded_file start this@" << (void*)this
                << " relpath=" << relpath
                << std::endl << RPS_FULL_BACKTRACE_HERE(0, "RpsLoader::string_of_loaded_file"));
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
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::string_of_loaded_file end this@" << (void*)this);
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
      if (str.size() == Rps_Id::nbchars && str[0] == '_'
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
      else if (str == "instance" &&  jv.isMember("class")
               && (jcomp=jv["class"]).isString()
              )
        {
          *this = Rps_InstanceZone::load_from_json(ld, jv);
        }
      else if (str == "json" && jv.isMember("json")
              )
        {
          *this = Rps_JsonZone::load_from_json(ld, jv);
        }
      else if (str == "closure"
               && jv.isMember("fn")
               && jv.isMember("env"))
        {
          auto jfn = jv["fn"];
          auto jenv = jv["env"];
          RPS_NOPRINTOUT("closure jv=" << jv << std::endl << "jfn=" << jfn << std::endl << "jenv=" << jenv);
          auto funobr = Rps_ObjectRef(jfn, ld);
          RPS_NOPRINTOUT("closure funobr=" << funobr);
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
              Rps_ClosureValue thisclos(funobr, vecenv);;
              RPS_NOPRINTOUT("closure thisclos=" << thisclos);
              *this = thisclos;
              RPS_NOPRINTOUT("closure this is " << *this << std::endl << "jv=" << jv);
              if (jv.isMember("metaobj"))
                {
                  int32_t metark = jv["metarank"].asInt();
                  auto  metaobr = Rps_ObjectRef(jv["metaobj"], ld);
                  thisclos->put_persistent_metadata(metaobr, metark);
                };
              RPS_NOPRINTOUT("Rps_Value::Rps_Value closure is:" << *this
                             << std::endl << "jv=" << jv);
              return;
            }
          else
            RPS_WARNOUT("Rps_Value::Rps_Value bad closure funobr=" << funobr);
        }
      else
        RPS_WARNOUT("strange Rps_Value::Rps_Value str=" << str);
    }
#warning Rps_Value::Rps_Value(const Json::Value &jv, Rps_Loader*ld) unimplemented
  RPS_WARNOUT("unimplemented Rps_Value::Rps_Value(const Json::Value &jv, Rps_Loader*ld)" << std::endl
              << "jv=" << jv);
} // end of Rps_Value::Rps_Value(const Json::value &jv, Rps_Loader*ld)



Rps_InstanceZone*
Rps_InstanceZone::load_from_json(Rps_Loader*ld, const Json::Value& jv)
{
  Rps_InstanceZone*res = nullptr;
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.isObject());
  auto jclass = jv["iclass"];
  auto jsize = jv["isize"];
  Rps_ObjectRef obclass(jclass,ld);
  auto jattrs = jv["iattrs"];
  auto nbattrs = jattrs.size();
  auto jcomps = jv["icomps"];
  auto nbcomps = jcomps.size();
  std::map<Rps_ObjectRef,Rps_Value> attrmap;
  std::vector<Rps_Value> compvec;
  compvec.reserve(nbcomps);
  for (int aix = 0; aix<(int)nbattrs; aix++)
    {
      Json::Value jcurent = jattrs[aix];
      if (jcurent.isObject())
        {
          Json::Value jiat = jcurent["iat"];
          Json::Value jiva = jcurent["iva"];
          Rps_ObjectRef atob (jiat,ld);
          Rps_Value atvalv (jiva,ld);
          attrmap.insert({atob,atvalv});
        }
    }
  for (int cix=0; cix<(int)nbcomps; cix++)
    {
      Json::Value jcurcomp = jcomps[cix];
      Rps_Value compv (jcurcomp,ld);
      compvec.push_back(compv);
    }
  {
    RPS_ASSERT(obclass);
    std::lock_guard<std::recursive_mutex> guclass (*(obclass->objmtxptr()));
    auto clpayl = obclass->get_classinfo_payload();
    const Rps_SetOb*setat = nullptr;
    if (clpayl
        && (attrmap.empty() || (setat=clpayl->attributes_set())))
      {
        res = make_from_attributes_components(obclass, compvec, attrmap);
        return res;
      }
    else
      {
        int siz = jsize.asInt();
        res = make_incomplete_loaded(ld, obclass, siz);
        rps_load_add_todo(ld, [=](Rps_Loader*ld2)
        {
          res->fill_loaded_instance_from_json(ld2,obclass,jv);
        });
        return res;
      }
  }
} // end of Rps_InstanceZone::load_from_json



Rps_InstanceZone*
Rps_InstanceZone::make_incomplete_loaded(Rps_Loader*ld, Rps_ObjectRef classob, unsigned siz)
{
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(classob);
  Rps_InstanceZone*res = nullptr;
  res = rps_allocate_with_wordgap<Rps_InstanceZone,unsigned,Rps_ObjectRef,Rps_InstanceTag>
        ((siz*sizeof(Rps_Value))/sizeof(void*),
         siz, classob, Rps_InstanceTag{});
  return res;
} // end Rps_InstanceZone::make_incomplete_loaded



void
Rps_InstanceZone::fill_loaded_instance_from_json(Rps_Loader*ld,Rps_ObjectRef obclass, const Json::Value& jv)
{
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT((int)cnt() == jv["isiz"].asInt());
  auto jattrs = jv["iattrs"];
  auto nbattrs = jattrs.size();
  auto jcomps = jv["icomps"];
  auto nbcomps = jcomps.size();
  std::map<Rps_ObjectRef,Rps_Value> attrmap;
  std::vector<Rps_Value> compvec;
  compvec.reserve(nbcomps);
  for (int aix = 0; aix<(int)nbattrs; aix++)
    {
      Json::Value jcurent = jattrs[aix];
      if (jcurent.isObject())
        {
          Json::Value jiat = jcurent["iat"];
          Json::Value jiva = jcurent["iva"];
          Rps_ObjectRef atob (jiat,ld);
          Rps_Value atvalv (jiva,ld);
          attrmap.insert({atob,atvalv});
        }
    }
  for (int cix=0; cix<(int)nbcomps; cix++)
    {
      Json::Value jcurcomp = jcomps[cix];
      Rps_Value compv (jcurcomp,ld);
      compvec.push_back(compv);
    }
  {
    RPS_ASSERT(obclass);
    std::lock_guard<std::recursive_mutex> guclass (*(obclass->objmtxptr()));
    auto clpayl = obclass->get_classinfo_payload();
    const Rps_SetOb*attrset = nullptr;
    if (clpayl
        && (attrmap.empty() || (attrset=clpayl->attributes_set())))
      {
        // every attribute in attrmap should be known to the class
        for (auto it : attrmap)
          {
            Rps_ObjectRef curat = it.first;
            RPS_ASSERT(curat);
            RPS_ASSERT(it.second);
            if (!attrset->contains(curat))
              throw RPS_RUNTIME_ERROR_OUT("Rps_InstanceZone::make_from_attributes_components class " << obclass
                                          << " unexpected attribute " << curat);
          };
        Rps_Value*sonarr = raw_data_sons();
        for (auto it : attrmap)
          {
            Rps_ObjectRef curat = it.first;
            Rps_Value curval = it.second;
            int ix=attrset->element_index(curat);
            RPS_ASSERT(ix>=0);
            sonarr[2*ix] = curat;
            sonarr[2*ix+1] = curval;
          }
        for (int cix=0; cix<(int)nbcomps; cix++)
          {
            Rps_Value curcomp = compvec[cix];
            sonarr[2*nbattrs+cix] = curcomp;
          }
      }
    else
      {
        rps_load_add_todo(ld, [=](Rps_Loader*ld2)
        {
          this->fill_loaded_instance_from_json(ld2,obclass,jv);
        });
      }
  }
} // end Rps_InstanceZone::fill_loaded_instance_from_json




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
  RPS_WARNOUT("partly unimplemented Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)"
              << std::endl << " jv=" << jv
              << RPS_FULL_BACKTRACE_HERE(2,"strange ObjectRef::Rps_ObjectRef"));
  throw  std::runtime_error("partly unimplemented Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)");
#warning partly unimplemented Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)
} // end Rps_ObjectRef::Rps_ObjectRef(const Json::Value &jv, Rps_Loader*ld)





//////////////////////////////////////////////// dumper
class Rps_Dumper
{
  friend class Rps_PayloadSpace;
  friend double rps_dump_start_elapsed_time(Rps_Dumper*);
  friend double rps_dump_start_process_time(Rps_Dumper*);
  friend double rps_dump_start_wallclock_time(Rps_Dumper*);
  friend double rps_dump_start_monotonic_time(Rps_Dumper*);
  friend void rps_dump_into (const std::string dirpath, Rps_CallFrame*);
  friend void rps_dump_scan_code_addr(Rps_Dumper*, const void*);
  friend void rps_dump_scan_object(Rps_Dumper*, Rps_ObjectRef obr);
  friend void rps_dump_scan_space_component(Rps_Dumper*, Rps_ObjectRef obrspace, Rps_ObjectRef obrcomp);
  friend void rps_dump_scan_value(Rps_Dumper*, Rps_Value obr, unsigned depth);
  friend Json::Value rps_dump_json_value(Rps_Dumper*, Rps_Value val);
  friend Json::Value rps_dump_json_objectref(Rps_Dumper*, Rps_ObjectRef obr);
  std::string du_topdir;
  Json::StreamWriterBuilder du_jsonwriterbuilder;
  std::recursive_mutex du_mtx;
  std::unordered_map<Rps_Id, Rps_ObjectRef,Rps_Id::Hasher> du_mapobjects;
  std::deque<Rps_ObjectRef> du_scanque;
  std::string du_tempsuffix;
  long du_newobcount;		// counter for new dumped objects
  double du_startelapsedtime;
  double du_startprocesstime;
  double du_startwallclockrealtime;
  double du_startmonotonictime;
  Rps_CallFrame* du_callframe;
  struct du_space_st
  {
    Rps_Id sp_id;
    std::set<Rps_ObjectRef> sp_setob;
    du_space_st(Rps_Id id) : sp_id(id), sp_setob() {};
  };
  std::map<Rps_ObjectRef,std::shared_ptr<du_space_st>> du_spacemap; // map from spaces to objects inside
  std::set<Rps_ObjectRef> du_pluginobset;
  std::set<Rps_ObjectRef> du_constantobset;
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
  void scan_cplusplus_source_file_for_constants(const std::string&relfilename);
  void scan_every_cplusplus_source_file_for_constants(void);
  void write_all_space_files(void);
  void write_all_generated_files(void);
  void write_generated_roots_file(void);
  void write_generated_names_file(void);
  void write_generated_constants_file(void);
  void write_manifest_file(void);
  void write_space_file(Rps_ObjectRef spacobr);
  void scan_object_contents(Rps_ObjectRef obr);
  std::unique_ptr<std::ofstream> open_output_file(const std::string& relpath);
  void rename_opened_files(void);
  void scan_code_addr(const void*);
public:
  std::string get_temporary_suffix(void) const
  {
    return du_tempsuffix;
  };
  std::string get_top_dir() const
  {
    return du_topdir;
  };
  Rps_Dumper(const std::string&topdir, Rps_CallFrame*callframe);
  ~Rps_Dumper();
  Rps_CallFrame* dumper_call_frame(void) const
  {
    return du_callframe;
  };
  void scan_object(const Rps_ObjectRef obr);
  void scan_value(const Rps_Value val, unsigned depth);
  Json::Value json_value(const Rps_Value val);
  Json::Value json_objectref(const Rps_ObjectRef obr);
  bool is_dumpable_objref(const Rps_ObjectRef obr);
  bool is_dumpable_objattr(const Rps_ObjectRef obr);
  bool is_dumpable_value(const Rps_Value val);
  void scan_space_component(Rps_ObjectRef obrspace, Rps_ObjectRef obrcomp)
  {
    RPS_ASSERT(obrspace);
    RPS_ASSERT(obrcomp);
    std::lock_guard<std::recursive_mutex> gu(du_mtx);
    auto itspace = du_spacemap.find(obrspace);
    if (itspace == du_spacemap.end())
      {
        auto p = du_spacemap.emplace(obrspace,std::make_shared<du_space_st>(obrspace->oid()));
        itspace = p.first;
      };
    itspace->second->sp_setob.insert(obrcomp);
  };
};				// end class Rps_Dumper

Rps_Dumper::Rps_Dumper(const std::string&topdir, Rps_CallFrame*callframe) :
  du_topdir(topdir), du_jsonwriterbuilder(), du_mtx(), du_mapobjects(), du_scanque(),
  du_tempsuffix(make_temporary_suffix()),
  du_newobcount(0),
  du_startelapsedtime(rps_elapsed_real_time()),
  du_startprocesstime(rps_process_cpu_time()),
  du_startwallclockrealtime(rps_wallclock_real_time()),
  du_startmonotonictime(rps_monotonic_real_time()),
  du_callframe(callframe), du_openedpathset()
{
  du_jsonwriterbuilder["commentStyle"] = "None";
  du_jsonwriterbuilder["indentation"] = " ";
  RPS_DEBUG_LOG(DUMP, "Rps_Dumper constr topdir=" << topdir
                << " this@" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_Dumper constr"));
  RPS_ASSERT(rps_is_main_thread());
} // end Rps_Dumper::Rps_Dumper

Rps_Dumper::~Rps_Dumper()
{
  RPS_DEBUG_LOG(DUMP, "Rps_Dumper destr topdir=" << du_topdir
                << " this@" << (void*)this
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_Dumper destr"));
  RPS_ASSERT(rps_is_main_thread());
  du_callframe = nullptr;
} // end Rps_Dumper::~Rps_Dumper

double
rps_dump_start_elapsed_time(Rps_Dumper*du)
{
  if (!du)
    return NAN;
  return du->du_startelapsedtime;
} // end rps_dump_start_elapsed_time


double
rps_dump_start_process_time(Rps_Dumper*du)
{
  if (!du)
    return NAN;
  return du->du_startprocesstime;
} // end  rps_dump_start_process_time

double
rps_dump_start_wallclock_time(Rps_Dumper*du)
{
  if (!du)
    return NAN;
  return du->du_startwallclockrealtime;
} // end rps_dump_start_wallclock_time

double
rps_dump_start_monotonic_time(Rps_Dumper*du)
{
  if (!du)
    return NAN;
  return du->du_startmonotonictime;
} // end  rps_dump_start_monotonic_time

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
  if (obr->get_mtime() > rps_get_start_wallclock_real_time())
    {
      du_newobcount++;
      RPS_DEBUG_LOG(DUMP, "new object #" << du_newobcount << ": " << obr
                    << " with mtime " << obr->get_mtime()
                    << " and start wallclock " <<  rps_get_start_wallclock_real_time());
    }
  du_scanque.push_back(obr);
  //  RPS_INFORMOUT("Rps_Dumper::scan_object adding oid " << obr->oid());
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
    return Json::Value(Json::nullValue);
  else if (val.is_int())
    return Json::Value(Json::Int64(val.as_int()));
  else if (val.is_ptr() && is_dumpable_value(val))
    {
      return val.to_ptr()->dump_json(this);
    }
  else
    return Json::Value(Json::nullValue);
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
Rps_Dumper::is_dumpable_objattr(const Rps_ObjectRef obr)
{
  if (!obr)
    return false;
  if (!is_dumpable_objref(obr))
    return false;
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
#warning incomplete Rps_Dumper::is_dumpable_objattr
  return true;
} // end Rps_Dumper::is_dumpable_objattr


bool
Rps_Dumper::is_dumpable_value(const Rps_Value val)
{
  if (!val) return true;
  if (val.is_int() || val.is_string() || val.is_set() || val.is_tuple())
    return true;
  if (val.is_closure())
    {
      auto closv = val.as_closure();
      if (closv->is_transient()) return false;
      else
        return is_dumpable_objref(closv->conn());
    }
  else if (val.is_instance())
    {
      auto instv = val.as_instance();
      if (instv->is_transient()) return false;
      else
        return is_dumpable_objref(instv->conn());
    }
  if (val.is_object())
    return is_dumpable_objref(val.to_object());
  RPS_FATALOUT("Rps_Dumper::is_dumpable_value partly unimplemented for " << val);
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
Rps_Dumper::scan_cplusplus_source_file_for_constants(const std::string&relfilename)
{
  int nbconst = 0;
  RPS_DEBUG_LOG(DUMP, "start dumper scan_cplusplus_source_file_for_constants file " << relfilename
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_Dumper::scan_cplusplus_source_file_for_constants"));
  RPS_ASSERT(relfilename.size()>2 && isalpha(relfilename[0]));
  std::string fullpath = std::string(rps_topdirectory) + "/" + relfilename;
  std::ifstream ins(fullpath);
  unsigned lincnt = 0;
  for (std::string linbuf; std::getline(ins, linbuf); )
    {
      lincnt++;
      if (u8_check(reinterpret_cast<const uint8_t*> (linbuf.c_str()),
                   linbuf.size()))
        {
          RPS_WARNOUT("file " << fullpath << ", line " << lincnt
                      << " non UTF8:" << linbuf);
          continue;
        };
      const char*curpos = linbuf.c_str();
      char*foundpos = nullptr;
      while ((foundpos = strstr((char*)curpos, RPS_CONSTANTOBJ_PREFIX)) != nullptr)
        {
          const char*endpos=nullptr;
          bool ok=false;
          Rps_Id oid(foundpos + strlen(RPS_CONSTANTOBJ_PREFIX), &endpos, &ok);
          if (ok)
            {
              Rps_ObjectZone* curobz = Rps_ObjectZone::find(oid);
              Rps_ObjectRef obr(curobz);
              if (obr)
                {
                  scan_object(obr);
                  nbconst++;
                  std::lock_guard<std::recursive_mutex> gu(du_mtx);
                  if (du_constantobset.find(obr) != du_constantobset.end())
                    RPS_DEBUG_LOG(DUMP, "scan_cplusplus_source_file_for_constants const#" << nbconst
                                  << " is " << obr);
                  du_constantobset.insert(obr);
                }
              else
                RPS_WARNOUT("unknown object of oid " << oid
                            << " in file " << fullpath << " line " << lincnt);
              curpos = endpos;
            }
          else break;
        };
    }
  if (nbconst>0)
    RPS_INFORMOUT("found " << nbconst
                  << " constant[s] prefixed by " << RPS_CONSTANTOBJ_PREFIX
                  << " in file " << fullpath
                  << " of " << lincnt << " lines.");
  else
    RPS_DEBUG_LOG(DUMP, "scan_cplusplus_source_file_for_constants no constants in " << fullpath);
} // end Rps_Dumper::scan_cplusplus_source_file_for_constants


void
Rps_Dumper::scan_code_addr(const void*ad)
{
  /**
   * If we wanted to be more efficient, we should parse the
   * pseudo-file /proc/self/maps at start of dump. See
   * http://man7.org/linux/man-pages/man5/proc.5.html for more....
   **/
  if (!ad)
    return;
  Dl_info di;
  memset(&di, 0, sizeof(di));
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  if (!dladdr(ad, &di))
    return;
  if (!di.dli_fname)
    return;
  const char*lastslash = strrchr(di.dli_fname, '/');
  if (!lastslash)
    return;
  char idbuf[32];
  memset (idbuf, 0, sizeof(idbuf));
  int endpos = -1;
  if (sscanf(lastslash+1, "rps_%19[a-zA-Z0-9]-mod.so%n", idbuf, &endpos) >= 1
      && endpos>20)
    {
      const char* endid=nullptr;
      bool okid=false;
      Rps_Id plugid (idbuf, &endid, &okid);
      if (plugid.valid() && *endid == (char)0 && okid)
        {
          Rps_ObjectZone* obz = Rps_ObjectZone::find(plugid);
          if (obz)
            {
              Rps_ObjectRef plugobr(obz);
              if (du_pluginobset.find(plugobr)
                  == du_pluginobset.end())
                {
                  du_pluginobset.insert(plugobr);
                  scan_object(plugobr);
                }
            };
        }
    }
} // end of Rps_Dumper::scan_code_addr


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

bool rps_is_dumpable_objattr(Rps_Dumper*du, const Rps_ObjectRef obr)
{
  RPS_ASSERT(du != nullptr);
  return du->is_dumpable_objattr(obr);
} // end rps_is_dumpable_objattr

bool rps_is_dumpable_value(Rps_Dumper*du, const Rps_Value val)
{

  RPS_ASSERT(du != nullptr);
  return du->is_dumpable_value(val);
} // end rps_is_dumpable_value

void
rps_dump_scan_code_addr(Rps_Dumper*du, const void* ad)
{
  RPS_ASSERT(du != nullptr);
  if (ad)
    du->scan_code_addr(ad);
} // end rps_dump_scan_code_addr


void
rps_dump_scan_space_component(Rps_Dumper*du, Rps_ObjectRef obrspace, Rps_ObjectRef obrcomp)
{
  RPS_ASSERT(du != nullptr);
  du->scan_space_component(obrspace, obrcomp);
} // end rps_dump_scan_space_component

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
    return Json::Value(Json::nullValue);
  else return du->json_value(val);
} // end rps_dump_json_value

Json::Value
rps_dump_json_objectref(Rps_Dumper*du, Rps_ObjectRef obr)
{
  RPS_ASSERT(du != nullptr);
  if (!obr || !rps_is_dumpable_objref(du,obr))
    return Json::Value(Json::nullValue);
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
  if (!is_metatransient())
    du->scan_object(metaobject());
} // end Rps_ClosureZone::dump_scan



Json::Value
Rps_ClosureZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (!rps_is_dumpable_objref(du,conn()) || is_transient())
    return Json::Value(Json::nullValue);
  auto  hjclo = Json::Value(Json::objectValue);
  hjclo["vtype"] = Json::Value("closure");
  hjclo["fn"] = rps_dump_json_objectref(du,conn());
  auto jvec = Json::Value(Json::arrayValue);
  for (Rps_Value sonval: *this)
    jvec.append(rps_dump_json_value(du,sonval));
  hjclo["env"] = jvec;
  if (!is_metatransient())
    {
      auto md = get_metadata();
      Rps_ObjectRef metaobr = md.first;
      int32_t metarank = md.second;
      if (metaobr)
        {
          hjclo["metaobj"] = rps_dump_json_objectref(du,metaobr);
          hjclo["metarank"] = Json::Value(metarank);
        }
    }
  return hjclo;
} // end Rps_ClosureZone::dump_json

////////////////


void
Rps_InstanceZone::dump_scan(Rps_Dumper*du, unsigned depth) const
{
  RPS_ASSERT(du != nullptr);
  auto obrcon = conn();
  du->scan_object(obrcon);
  if (du->is_dumpable_objref(obrcon))
    {
      for (auto v: *this)
        du->scan_value(v, depth+1);
    }
  if (!is_metatransient())
    du->scan_object(metaobject());
} // end Rps_InstanceZone::dump_scan


Json::Value
Rps_InstanceZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  if (!rps_is_dumpable_objref(du,conn()) || is_transient())
    return Json::Value(Json::nullValue);
  auto  hjins = Json::Value(Json::objectValue);
  hjins["vtype"] = Json::Value("instance");
  hjins["isize"] = Json::Value(cnt());
  hjins["iclass"] = rps_dump_json_objectref(du,get_class());
  auto atset = set_attributes();
  const  Rps_Value* csons = const_sons();
  unsigned nbsons = cnt();
  int atlen = 0;
  int nbattrs = 0;
  if (atset)
    {
      atlen = atset->cardinal();
      auto jvattrs = Json::Value(Json::arrayValue);
      jvattrs.resize(atlen); // reservation
      int attrix = 0;
      for (Rps_ObjectRef obattr: *atset)
        {
          attrix++;
          if (attrix > (int)nbsons)
            break;
          if (!rps_is_dumpable_objref(du, obattr))
            {
              jvattrs.append(Json::Value(Json::nullValue));
              continue;
            }
          auto jent = Json::Value(Json::objectValue);
          jent["iat"] = rps_dump_json_objectref(du,obattr);
          jent["iva"] = rps_dump_json_value(du,csons[attrix-1]);
          jvattrs.append(jent);
          nbattrs++;
        }
      jvattrs.resize(nbattrs); // reservation
      hjins["iattrs"] = jvattrs;
      auto jvcomps = Json::Value(Json::arrayValue);
      jvcomps.resize(nbsons-nbattrs); // reservation
      for (int compix = attrix; compix<(int)nbsons; compix++)
        {
          jvcomps.append(rps_dump_json_value(du,csons[compix]));
        }
      hjins["icomps"] = jvcomps;
    }
  if (!is_metatransient())
    {
      auto md = get_metadata();
      Rps_ObjectRef metaobr = md.first;
      int32_t metarank = md.second;
      if (metaobr)
        {
          hjins["metaobj"] = rps_dump_json_objectref(du,metaobr);
          hjins["metarank"] = Json::Value(metarank);
        }
    }
  return hjins;
} // end Rps_InstanceZone::dump_json


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
  int nbroots = 0;
  RPS_DEBUG_LOG(DUMP, "dumper: scan_roots begin");
  rps_each_root_object([&](Rps_ObjectRef obr)
  {
    rps_dump_scan_object(this,obr);
    nbroots++;
  });

  RPS_DEBUG_LOG(DUMP, "dumper: scan_roots ends nbroots#" << nbroots);
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
  //  RPS_INFORMOUT("scan_loop_pass start");
  while ((curobr=pop_object_to_scan()))
    {
      count++;
      // RPS_INFORMOUT("scan_loop_pass count#" << count
      //              << " curobr=" << curobr->oid());
      scan_object_contents(curobr);
    };
  RPS_DEBUG_LOG(DUMP, "dumper: scan_loop_pass end count#" << count);
} // end Rps_Dumper::scan_loop_pass



void
Rps_Dumper::scan_object_contents(Rps_ObjectRef obr)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  obr->dump_scan_contents(this);
  Rps_ObjectRef spacobr(obr->get_space());
  rps_dump_scan_object(this,spacobr);
} // end Rps_Dumper::scan_object_contents



void
Rps_Dumper::scan_every_cplusplus_source_file_for_constants(void)
{
  for (const char*const*pcurfilename = rps_files; *pcurfilename; pcurfilename++)
    {
      const char*curpath = *pcurfilename;
      int lencurpath = strlen(curpath);
      if (lencurpath < 6 || strstr(curpath, "generated/") || strstr(curpath, "attic/"))
        continue;
      if (curpath[lencurpath-3] != '.') continue;
      if ((curpath[lencurpath-2] == 'h' && curpath[lencurpath-1] == 'h')
          || (curpath[lencurpath-2] == 'c' && curpath[lencurpath-1] == 'c'))
        {
          std::string relfilname = curpath;
          scan_cplusplus_source_file_for_constants(relfilname);
        };
    }
} // end of scan_every_cplusplus_source_file_for_constants

void
Rps_Dumper::write_all_space_files(void)
{
  RPS_DEBUG_LOG(DUMP, "dumper write_all_space_files start");
  std::set<Rps_ObjectRef> spaceset;
  {
    std::lock_guard<std::recursive_mutex> gu(du_mtx);
    for (auto it: du_spacemap)
      spaceset.insert(it.first);
  }
  int nbspace = 0;
  for (Rps_ObjectRef spacobr : spaceset)
    {
      write_space_file(spacobr);
      nbspace++;
    }
  RPS_INFORMOUT("wrote " << nbspace << " space files into " << du_topdir);
} // end Rps_Dumper::write_all_space_files

void
Rps_Dumper::write_generated_roots_file(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  RPS_DEBUG_LOG(DUMP, "dumper write_generated_roots_file start");
  auto rootpathstr = std::string{"generated/rps-roots.hh"};
  auto pouts = open_output_file(rootpathstr);
  rps_emit_gplv3_copyright_notice(*pouts, rootpathstr, "//: ", "");
  *pouts << std::endl
         << "#ifndef RPS_INSTALL_ROOT_OB" << std::endl
         << "#error RPS_INSTALL_ROOT_OB(Oid) macro undefined" << std::endl
         << "#endif /*undefined RPS_INSTALL_ROOT_OB*/" << std::endl << std::endl;
  int rootcnt = 0;
  //  std::ofstream& out = *pouts;
  rps_each_root_object([=, &pouts, &rootcnt](Rps_ObjectRef obr)
  {
    RPS_ASSERT(obr);
    (*pouts) << "RPS_INSTALL_ROOT_OB(" << obr->oid() << ") //";
    std::lock_guard<std::recursive_mutex> guobr(*(obr->objmtxptr()));
    Rps_ObjectRef obclass = obr->get_class();
    RPS_ASSERT(obclass);
    if (auto clapayl = obr->get_dynamic_payload<Rps_PayloadClassInfo>())
      {
        if (obclass == Rps_ObjectRef::the_class_class())
          {
            (*pouts) << clapayl->class_name_str() << "∈class";
          }
        else
          {
            auto claclapayl = obclass->get_dynamic_payload<Rps_PayloadClassInfo>();
            RPS_ASSERT(claclapayl);
            (*pouts) << clapayl->class_name_str() << "∈" << claclapayl->class_name_str();
          }
      }
    else if (auto symbpayl = obr->get_dynamic_payload<Rps_PayloadSymbol>())
      {
        if (obclass == Rps_ObjectRef::the_symbol_class())
          {
            (*pouts) << symbpayl->symbol_name()  << "∈symbol";
          }
        else
          {
            auto claclapayl = obclass->get_dynamic_payload<Rps_PayloadClassInfo>();
            RPS_ASSERT(claclapayl);
            (*pouts) << symbpayl->symbol_name() << "∈" << claclapayl->class_name_str();
          }
      }
    else
      {
        auto claclapayl = obclass->get_dynamic_payload<Rps_PayloadClassInfo>();
        Rps_Value nameval = obr->get_physical_attr(Rps_ObjectRef::the_name_object());
        RPS_ASSERT(claclapayl);
        if (nameval.is_string())
          (*pouts) << '"' << Rps_Cjson_String(nameval.to_cppstring()) << '"';
        (*pouts) << "∈" << claclapayl->class_name_str();
      };
    (*pouts) << std::endl;
    rootcnt++;
  });
  *pouts << std::endl
         << "#undef RPS_NB_ROOT_OB" << std::endl
         << "#define RPS_NB_ROOT_OB " << rootcnt << std::endl;
  *pouts << std::endl
         << "#undef RPS_INSTALL_ROOT_OB" << std::endl;
  *pouts << "/// end of RefPerSys roots file " << rootpathstr << std::endl;
  RPS_DEBUG_LOG(DUMP, "dumper write_generated_roots_file end rootcnt=" << rootcnt << std::endl);
} // end Rps_Dumper::write_generated_roots_file

void
Rps_Dumper::write_generated_names_file(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  auto rootpathstr = std::string{"generated/rps-names.hh"};
  RPS_DEBUG_LOG(DUMP, "dumper write_generated_names_file start");
  auto pouts = open_output_file(rootpathstr);
  rps_emit_gplv3_copyright_notice(*pouts, rootpathstr, "//: ", "");
  *pouts << std::endl
         << "#ifndef RPS_INSTALL_NAMED_ROOT_OB" << std::endl
         << "#error RPS_INSTALL_NAMED_ROOT_OB(Oid,Name) macro undefined" << std::endl
         << "#endif /*undefined RPS_INSTALL_NAMED_ROOT_OB*/" << std::endl << std::endl;
  int namecnt = 0;
  //  std::ofstream& out = *pouts;
  rps_each_root_object([=, &pouts, &namecnt](Rps_ObjectRef obr)
  {
    Rps_PayloadSymbol* cursym = obr->get_dynamic_payload<Rps_PayloadSymbol>();
    if (!cursym || cursym->symbol_is_weak())
      return;
    std::lock_guard<std::recursive_mutex> gu(*(obr->objmtxptr()));
    (*pouts) << "RPS_INSTALL_NAMED_ROOT_OB(" << obr->oid()
             << "," << (cursym->symbol_name()) << ")" << std::endl;
    namecnt++;
  });
  *pouts << std::endl
         << "#undef RPS_NB_NAMED_ROOT_OB" << std::endl
         << "#define RPS_NB_NAMED_ROOT_OB " << namecnt << std::endl;
  *pouts << std::endl
         << "#undef RPS_INSTALL_NAMED_ROOT_OB" << std::endl;
  *pouts << "/// end of RefPerSys roots file " << rootpathstr << std::endl;
  RPS_DEBUG_LOG(DUMP, "dumper write_generated_names_file end namecnt=" << namecnt << std::endl);

} // end Rps_Dumper::write_generated_names_file



void
Rps_Dumper::write_generated_constants_file(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  auto rootpathstr = std::string{"generated/rps-constants.hh"};
  RPS_DEBUG_LOG(DUMP, "dumper write_generated_constants_file start");
  auto pouts = open_output_file(rootpathstr);
  rps_emit_gplv3_copyright_notice(*pouts, rootpathstr, "//: ", "");
  unsigned constcnt = 0;
  *pouts << std::endl << "/// collection of constant objects, mentioned in C++ files, "<< std::endl
         << "/// .... prefixed with '"
         <<  RPS_CONSTANTOBJ_PREFIX << "' followed by an oid." << std::endl;
  *pouts << std::endl
         << "#ifndef RPS_INSTALL_CONSTANT_OB" << std::endl
         << "#error RPS_INSTALL_CONSTANT_OB(Oid) macro undefined" << std::endl
         << "#endif /*undefined RPS_INSTALL_CONSTANT_OB*/" << std::endl << std::endl;
  for (Rps_ObjectRef constobr : du_constantobset)
    {
      RPS_ASSERT(constobr);
      if (constcnt % 10 == 0)
        *pouts << std::endl;
      *pouts << "RPS_INSTALL_CONSTANT_OB(" << constobr->oid() << ")" << std::endl;
      constcnt ++;
    }
  *pouts << std::endl << "#undef RPS_INSTALL_CONSTANT_OB" << std::endl << std::endl;
  *pouts << std::endl
         << "#undef  RPS_NB_CONSTANT_OB" << std::endl
         << "#define RPS_NB_CONSTANT_OB " << constcnt << std::endl << std::endl;
  *pouts << "/// end of RefPerSys constants file " << rootpathstr << std::endl;
  RPS_DEBUG_LOG(DUMP, "dumper write_generated_constants_file end constcnt=" << constcnt << std::endl);
} // end Rps_Dumper::write_generated_constants_file



void
Rps_Dumper::write_all_generated_files(void)
{
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  write_generated_roots_file();
  write_generated_names_file();
  write_generated_constants_file();
  RPS_LOCALFRAME(/*descr:*/nullptr,
                           du_callframe,
                           Rps_Value dumpdirnamev;
                           Rps_Value tempsuffixv;
                           Rps_ObjectRef refpersysob;
                           Rps_ObjectRef gencodselob;
                           Rps_ObjectRef genstoreob;
                           Rps_Value mainv;
                           Rps_Value refpersysv;
                           Rps_Value xtrav;
                );
  _f.dumpdirnamev = Rps_StringValue(du_topdir);
  _f.tempsuffixv = Rps_StringValue(du_tempsuffix);
  _f.refpersysob = RPS_ROOT_OB(_1Io89yIORqn02SXx4p); //RefPerSys_system∈the_system_class
  _f.gencodselob = RPS_ROOT_OB(_5VC4IuJ0dyr01b8lA0); //generate_code∈named_selector
  try
    {
      _f.refpersysv = Rps_ObjectValue(_f.refpersysob);
      /* We create a temporary object to hold some "arbitrary"
      information about this particular generation */
      _f.genstoreob = Rps_ObjectRef::make_object(&_, Rps_ObjectRef::the_object_class());
      RPS_DEBUG_LOG(DUMP, "Rps_Dumper::write_all_generated_files before sending "<< _f.gencodselob << " to "
                    << _f.refpersysv << " with " << _f.dumpdirnamev << " & " << _f.tempsuffixv << " genstoreob=" << _f.genstoreob
                    << std::endl
                    << Rps_ShowCallFrame(&_));
      Rps_TwoValues two = _f.refpersysv.send3(&_, _f.gencodselob, _f.dumpdirnamev, _f.tempsuffixv, _f.genstoreob);
      _f.mainv = two.main();
      _f.xtrav  = two.xtra();
      RPS_DEBUG_LOG(DUMP, "Rps_Dumper::write_all_generated_files after sending "<< _f.gencodselob << " to "
                    << _f.refpersysv << " with " << _f.dumpdirnamev << " & " << _f.tempsuffixv
                    << std::endl << " --> mainv=" << _f.mainv << " & xtrav=" << _f.xtrav
                    << std::endl
                    << Rps_ShowCallFrame(&_));
      if (!_f.mainv && !_f.xtrav)
        {
          RPS_WARNOUT("Rps_Dumper::write_all_generated_files failed to send " << _f.gencodselob << " to "
                      << _f.refpersysob << " with " << _f.dumpdirnamev << " & " << _f.tempsuffixv
                      << std::endl << Rps_ShowCallFrame(&_)
                      << std::endl
                      << RPS_FULL_BACKTRACE_HERE(1,"Rps_Dumper::write_all_generated_files"));
        }
    }
  catch (std::exception&exc)
    {
      RPS_WARNOUT("Rps_Dumper::write_all_generated_files failed to generate_code on RefPerSys_system dumpdirnamev=" << _f.dumpdirnamev
                  << " tempsuffixv=" << _f.tempsuffixv
                  << " got exception " << exc.what()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1,"Rps_Dumper::write_all_generated_files"));
    }
} // end Rps_Dumper::write_all_generated_files


void
Rps_Dumper::write_manifest_file(void)
{
  std::unique_ptr<Json::StreamWriter> jsonwriter(du_jsonwriterbuilder.newStreamWriter());
  std::lock_guard<std::recursive_mutex> gu(du_mtx);
  RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file start");
  auto pouts = open_output_file(RPS_MANIFEST_JSON);
  rps_emit_gplv3_copyright_notice(*pouts, RPS_MANIFEST_JSON, "//!! ", "");
  Json::Value jmanifest(Json::objectValue);
  jmanifest["format"] = Json::Value (RPS_MANIFEST_FORMAT);
  {
    int nbroots=0;
    Json::Value jglobalroots(Json::arrayValue);
    rps_each_root_object([=,&jglobalroots,&nbroots](Rps_ObjectRef obr)
    {
      jglobalroots.append(Json::Value(obr->oid().to_string()));
      nbroots++;
    });
    jmanifest["globalroots"] = jglobalroots;
    RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file wrote " << nbroots << " global roots.");
  }
  {
    int nbspaces=0;
    Json::Value jspaceset(Json::arrayValue);
    for (auto it: du_spacemap)
      {
        RPS_ASSERT(it.first);
        jspaceset.append(Json::Value(it.first->oid().to_string()));
        nbspaces++;
      }
    jmanifest["spaceset"] = jspaceset;
    RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file wrote " << nbspaces << " spaces.");
  }
  {
    int nbconst=0;
    Json::Value jconstset(Json::arrayValue);
    for (Rps_ObjectRef obr: du_constantobset)
      {
        RPS_ASSERT(obr);
        jconstset.append(Json::Value(obr->oid().to_string()));
        nbconst++;
      }
    jmanifest["constset"] = jconstset;
    RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file wrote " << nbconst << " constants.");
  }
  {
    int nbplugins=0;
    Json::Value jplugins(Json::arrayValue);
    for (auto plugobr: du_pluginobset)
      {
        RPS_ASSERT(plugobr);
        jplugins.append(Json::Value(plugobr->oid().to_string()));
        nbplugins++;
      }
    jmanifest["plugins"] = jplugins;
    RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file wrote " << nbplugins << " plugins.");
  }
  {
    Json::Value jglobalnames(Json::arrayValue);
    int namecnt = 0;
    //  std::ofstream& out = *pouts;
    rps_each_root_object([=, &pouts, &namecnt, &jglobalnames](Rps_ObjectRef obr)
    {
      Rps_PayloadSymbol* cursym = obr->get_dynamic_payload<Rps_PayloadSymbol>();
      if (!cursym || cursym->symbol_is_weak())
        return;
      Json::Value jnaming(Json::objectValue);
      jnaming["nam"] = Json::Value(cursym->symbol_name());
      jnaming["obj"] = Json::Value(obr->oid().to_string());
      jglobalnames.append(jnaming);
      namecnt++;
    });
    jmanifest["globalnames"] = jglobalnames;
    RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file wrote " << namecnt << " global names.");
  }
  /// this is not used for loading, but could be useful for other purposes.
  jmanifest["origitid"] = Json::Value (rps_gitid);
  {
    time_t nowt = 0;
    time(&nowt);
    if (nowt > 0)
      {
        struct tm nowtm = {};
        memset (&nowtm, 0, sizeof(nowtm));
        if (gmtime_r(&nowt, &nowtm) != nullptr)
          {
            char datbuf[64];
            memset (datbuf, 0, sizeof(datbuf));
            if (strftime(datbuf, sizeof(datbuf), "%Y %b %d", &nowtm) && datbuf[0])
              jmanifest["dumpdate"] = Json::Value (datbuf);
          }
      }
  }
  jmanifest["progname"] = Json::Value (rps_progname);
  jmanifest["progtimestamp"] = Json::Value (rps_timestamp);
  jmanifest["progmd5sum"] = Json::Value(rps_md5sum);
  jsonwriter->write(jmanifest, pouts.get());
  *pouts << std::endl <<  std::endl << "//// end of RefPerSys manifest file" << std::endl;
  RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file ending ... " << rps_gitid << std::endl);
} // end Rps_Dumper::write_manifest_file


void
Rps_Dumper::write_space_file(Rps_ObjectRef spacobr)
{
  du_space_st* curspa = nullptr;
  {
    std::lock_guard<std::recursive_mutex> gu(du_mtx);
    curspa = du_spacemap[spacobr].get();
  }
  RPS_ASSERT(curspa);
  std::string curelpath;
  std::set<Rps_ObjectRef> curspaset;
  Rps_Id spacid;
  std::unique_ptr<std::ofstream> pouts;
  std::unique_ptr<Json::StreamWriter> jsonwriter(du_jsonwriterbuilder.newStreamWriter());
  {
    std::lock_guard<std::recursive_mutex> gu(du_mtx);
    spacid = curspa->sp_id;
    curelpath = std::string{"persistore/sp"} + spacid.to_string() + "-rps.json";
    pouts = open_output_file(curelpath);
    curspaset = curspa->sp_setob;
  }
  RPS_ASSERT(pouts);
  rps_emit_gplv3_copyright_notice(*pouts, curelpath, "//// ", "");
  *pouts << std::endl;
  // emit the prologue
  {
    *pouts << std::endl
           << "///!!! prologue of RefPerSys space file:" << std::endl;
    Json::Value jprologue(Json::objectValue);
    jprologue["format"] = Json::Value (RPS_MANIFEST_FORMAT);
    jprologue["spaceid"] = Json::Value (spacid.to_string());
    jprologue["nbobjects"] = Json::Value ((int)(curspaset.size()));
    jsonwriter->write(jprologue, pouts.get());
    *pouts << std::endl;
  }
  int count = 0;
  for (auto curobr: curspaset)
    {
      *pouts << std::endl << std::endl;
      ++count;
      *pouts << "//+ob" << curobr->oid().to_string() << std::endl;
      RPS_NOPRINTOUT("Rps_Dumper::write_space_file emits " << (curobr->oid().to_string())
                     << " of hi=" <<  (curobr->oid().hi())
                     << " #" << count);
      /// output a comment giving the class name for readability
      {
        Rps_ObjectRef obclass = curobr->get_class();
        Rps_ObjectRef obsymb;
        if (obclass)
          {
            RPS_NOPRINTOUT("Rps_Dumper::write_space_file obclass " << obclass->oid().to_string()
                           << " for obr " <<curobr->oid().to_string());
            usleep(1000);
            std::lock_guard<std::recursive_mutex> gu(*(obclass->objmtxptr()));
            auto classinfo = obclass->get_dynamic_payload<Rps_PayloadClassInfo>();
            if (classinfo)
              obsymb = classinfo->symbname();
          };
        if (obsymb)
          {
            RPS_NOPRINTOUT("Rps_Dumper::write_space_file obsymb " << obsymb->oid().to_string()
                           << " for obr " <<curobr->oid().to_string());
            std::lock_guard<std::recursive_mutex> gu(*(obsymb->objmtxptr()));
            auto symb = obsymb->get_dynamic_payload<Rps_PayloadSymbol>();
            if (symb)
              *pouts << "//∈" /*U+2208 ELEMENT OF*/
                     << symb->symbol_name() << std::endl;
          }
        else
          RPS_WARNOUT("Rps_Dumper::write_space_file no obsymb for obr "
                      <<curobr->oid().to_string());

      }
      Json::Value jobject(Json::objectValue);
      jobject["oid"] = Json::Value (curobr->oid().to_string());
      curobr->dump_json_content(this,jobject);
      jsonwriter->write(jobject, pouts.get());
      *pouts << std::endl;
      *pouts << "//-ob" << curobr->oid().to_string() << std::endl;
    }

  *pouts << std::endl << std::endl;
  *pouts << "//// end of RefPerSys generated space file " << curelpath << std::endl;
  RPS_DEBUG_LOG(DUMP, "dumper write_space_file end " << curelpath << " with " << count << " objects." << std::endl);
} // end Rps_Dumper::write_space_file



void
Rps_PayloadSpace::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
} // end Rps_PayloadSpace::dump_scan



////////////////////////////////////////////////////////////////
void rps_dump_into (std::string dirpath, Rps_CallFrame* callframe)
{
  RPS_LOCALFRAME(/*descr:*/nullptr, /*callerframe:*/callframe,
                           Rps_ObjectRef obdumper);
  double startelapsed = rps_elapsed_real_time();
  double startcputime = rps_process_cpu_time();
  RPS_DEBUG_LOG(DUMP, "rps_dump_into start dirpath=" << dirpath
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_dump_into"));
  if (dirpath.empty())
    dirpath = std::string(".");
  {
    DIR* d = opendir(dirpath.c_str());
    if (d)
      closedir(d);
    else
      {
        if (mkdir(dirpath.c_str(), 0750))
          RPS_WARN("failed to mkdir %s: %m", dirpath.c_str());
        else
          RPS_INFORM("made directory %s to dump into", dirpath.c_str());
      }
  }
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
    char cwdbuf[128];
    memset(cwdbuf, 0, sizeof(cwdbuf));
    if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
      RPS_FATAL("getcwd failed: %m");
    cwdpath = std::string(cwdbuf);
  }
  RPS_DEBUG_LOG(DUMP, "rps_dump_into realdirpath=" << realdirpath << " cwdpath=" << cwdpath);
  /// ensure that realdirpath exists
  {
    RPS_ASSERT(strrchr(realdirpath.c_str(), '/') != nullptr);
  }
  Rps_Dumper dumper(realdirpath, &_);
  RPS_INFORMOUT("start dumping into " << dumper.get_top_dir()
                << " with temporary suffix " << dumper.get_temporary_suffix());
  try
    {
      if (realdirpath != cwdpath)
        {
          if (!std::filesystem::create_directories(realdirpath
              + "/persistore"))
            {
              RPS_WARNOUT("failed to make dump sub-directory " << realdirpath
                          << "/persistore:" << strerror(errno));
              throw std::runtime_error(std::string{"failed to make dump directory:"} + realdirpath + "/persistore");
            }
          else
            RPS_INFORMOUT("made real dump sub-directory: " << realdirpath
                          << "/persistore");
          if (!std::filesystem::create_directories(realdirpath
              + "/generated"))
            {
              RPS_WARNOUT("failed to make dump sub-directory " << realdirpath
                          << "/generated:" << strerror(errno));
              throw std::runtime_error(std::string{"failed to make dump directory:"} + realdirpath + "/persistore");
            }
          else
            RPS_INFORMOUT("made real dump sub-directory: " << realdirpath
                          << "/generated");
        }
      dumper.scan_roots();
      dumper.scan_every_cplusplus_source_file_for_constants();
      dumper.scan_loop_pass();
      RPS_DEBUG_LOG(DUMP, "rps_dump_into realdirpath=" << realdirpath << " start writing "
                    << (rps_elapsed_real_time() - startelapsed) << " elapsed, "
                    << (rps_process_cpu_time() - startcputime)
                    << " cpu seconds." << std::endl
                    << Rps_ShowCallFrame(&_));
      dumper.write_all_space_files();
      dumper.write_all_generated_files();
      dumper.write_manifest_file();
      dumper.rename_opened_files();
      double endelapsed = rps_elapsed_real_time();
      double endcputime = rps_process_cpu_time();
      RPS_INFORMOUT("dump into " << dumper.get_top_dir()
                    << " completed in " << (endelapsed-startelapsed) << " wallclock, "
                    << (endcputime-startcputime) << " cpu seconds"
                    << " with " << dumper.du_newobcount
                    << " new objects dumped");
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
  RPS_DEBUG_LOG(LOAD, "loader parse_manifest_file start manifstr=" << manifstr);
  Json::Value manifjson;
  try
    {
      manifjson = rps_string_to_json(manifstr);
      if (manifjson.type () != Json::objectValue)
        RPS_FATAL("Rps_Loader::parse_manifest_file wants a Json object in %s",
                  manifpath.c_str());
    }
  catch  (const std::exception& exc)
    {
      RPS_FATALOUT("Rps_Loader::parse_manifest_file failed to parse: " << exc.what());
    };
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
    RPS_DEBUG_LOG(LOAD, "loader parse_manifest_file sizespset=" << sizespset);
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
    RPS_DEBUG_LOG(LOAD, "loader parse_manifest_file sizeglobroots=" << sizeglobroots);
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
    RPS_DEBUG_LOG(LOAD, "loader parse_manifest_file sizeplugins=" << sizeplugins);
    {
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      for (int ix=0; ix<(int)sizeplugins; ix++)
        {
          std::string curpluginidstr = pluginsjson[ix].asString();
          Rps_Id curpluginid (curpluginidstr);
          RPS_ASSERT(curpluginid && curpluginid.valid());
          std::string pluginsopath = load_real_path(std::string{"plugins/rps"} + curpluginid.to_string() + "-mod.so");
          std::string pluginsrcpath = load_real_path(std::string{"generated/rps"} + curpluginid.to_string() + "-mod.cc");
          RPS_INFORMOUT("should load plugin #" << ix << " from " << pluginsopath << " source " << pluginsrcpath);
          struct stat pluginsrcstat;
          memset (&pluginsrcstat, 0, sizeof(pluginsrcstat));
          if (!stat(pluginsrcpath.c_str(), &pluginsrcstat))
            {
              bool needbuild = false;
              struct stat pluginsostat;
              memset (&pluginsostat, 0, sizeof(pluginsostat));
              if (stat(pluginsopath.c_str(), &pluginsostat))
                {
                  RPS_INFORMOUT("should build missing plugin #" << ix
                                << " in " << pluginsopath
                                << " (" << strerror(errno) << ")"
                                << " from source " << pluginsrcpath);
                  needbuild = true;
                }
              else
                {
                  if (pluginsostat.st_mtim.tv_sec < pluginsrcstat.st_mtim.tv_sec)
                    needbuild = true;
                  else if (pluginsostat.st_mtim.tv_sec == pluginsrcstat.st_mtim.tv_sec
                           && pluginsostat.st_mtim.tv_nsec <=  pluginsrcstat.st_mtim.tv_nsec)
                    needbuild = true;
                  if (needbuild)
                    RPS_INFORMOUT("should rebuild plugin #" << ix << " into " << pluginsopath << " from source " << pluginsrcpath);
                }
              if (needbuild)
                {
                  std::string buildcmdstr;
                  buildcmdstr += rps_topdirectory;
                  buildcmdstr += "/do-build-refpersys-plugin";
                  buildcmdstr += " ";
                  buildcmdstr += pluginsrcpath;
                  buildcmdstr += " -o ";
                  buildcmdstr += pluginsopath;
                  RPS_INFORMOUT("before building plugin #" << ix << " using " << buildcmdstr);
                  fflush(nullptr);
                  int notok = system(buildcmdstr.c_str());
                  char exitbuf[16];
                  memset (exitbuf, 0, sizeof(exitbuf));
                  if (notok)
                    RPS_FATALOUT("failed to build plugin #" << ix << " using " << buildcmdstr
                                 << " - "
                                 << (WIFEXITED(notok)?"exited"
                                     :WIFSIGNALED(notok)?"signalled"
                                     :WIFSTOPPED(notok)?"stopped"
                                     :"!bizarre!")
                                 << " "
                                 << (WIFEXITED(notok)?(snprintf(exitbuf, sizeof(exitbuf), "%d", WEXITSTATUS(notok)), exitbuf)
                                     :WIFSIGNALED(notok)?strsignal(WTERMSIG(notok))
                                     :"???"));
                  else
                    {
                      if (!access(pluginsopath.c_str(), R_OK))
                        RPS_INFORMOUT("did build successfully plugin #" << ix << " " << pluginsopath);
                      else
                        RPS_FATALOUT("missing built plugin #" << ix << " " << pluginsopath << " : " << strerror(errno));
                    }
                  fflush(nullptr);
                }
            }
          void* dlh = dlopen(pluginsopath.c_str(), RTLD_NOW | RTLD_GLOBAL);
          if (!dlh)
            RPS_FATAL("failed to load plugin #%d file %s: %s",
                      ix, pluginsopath.c_str(), dlerror());
          else
            {
              ld_pluginsmap.insert({curpluginid, dlh});
              RPS_INFORMOUT("dlopen(3)-ed plugin #" << ix << " file " << pluginsopath);
            }
        }
    }
  }
  ////
  RPS_DEBUG_LOG(LOAD, "loader parse_manifest_file end" << std::endl);
} // end Rps_Loader::parse_manifest_file

void
Rps_Loader::parse_user_manifest(const std::string&umpath)
{
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::parse_user_manifest start umpath="
                << umpath);
  Json::Value manifjson;
  try
    {
      std::string manifstr = string_of_loaded_file(umpath);
      manifjson = rps_string_to_json(manifstr);
      if (manifjson.type () != Json::objectValue)
        RPS_FATAL("Rps_Loader::parse_user_manifest %s wants a Json object in %s",
                  umpath.c_str(), manifstr.c_str());
      if (manifjson["format"].asString() != RPS_MANIFEST_FORMAT)
        RPS_FATAL("user manifest map in %s should have format: '%s' but got:\n"
                  "%s",
                  umpath.c_str(), RPS_MANIFEST_FORMAT,
                  manifjson["format"].toStyledString().c_str());
      /// parse userroots
      {
        auto userrootsjson = manifjson["user_roots"];
        if (userrootsjson.type() !=  Json::arrayValue)
          RPS_FATAL("user manifest map in %s should have user_roots: [...]",
                    umpath.c_str ());
        size_t sizeuserroots = userrootsjson.size();
        RPS_DEBUG_LOG(LOAD, "loader parse_user_manifest sizeuserroots=" << sizeuserroots);
        for (int ix=0; ix<(int)sizeuserroots; ix++)
          {
            std::string curgrootidstr = userrootsjson[ix].asString();
            Rps_Id curgrootid (curgrootidstr);
            RPS_ASSERT(curgrootid);
            ld_globrootsidset.insert(curgrootid);
          }
      }
      /// parse user plugins
      {
        auto pluginsjson = manifjson["user_plugins"];
        if (pluginsjson.type() !=  Json::arrayValue)
          RPS_FATAL("user manifest map in %s should have user_plugins: [...]",
                    umpath.c_str ());
        size_t sizeplugins = pluginsjson.size();
        RPS_DEBUG_LOG(LOAD, "loader parse_user_manifest sizeplugins=" << sizeplugins);
        {
          std::lock_guard<std::recursive_mutex> gu(ld_mtx);
          for (int ix=0; ix<(int)sizeplugins; ix++)
            {
              std::string curpluginidstr = pluginsjson[ix].asString();
              Rps_Id curpluginid (curpluginidstr);
              RPS_ASSERT(curpluginid && curpluginid.valid());
              std::string pluginpath =
                load_real_path(std::string(rps_homedir())
                               +std::string{"/refpersys_plugins/rps"} + curpluginid.to_string() + "-mod.so");
              RPS_INFORMOUT("should load user plugin #" << ix << " from " << pluginpath);
              void* dlh = dlopen(pluginpath.c_str(), RTLD_NOW | RTLD_GLOBAL);
              if (!dlh)
                RPS_FATAL("failed to load user plugin #%d file %s: %s",
                          ix, pluginpath.c_str(), dlerror());
              ld_pluginsmap.insert({curpluginid, dlh});
            }
        }
      }
    }
  catch  (const std::exception& exc)
    {
      RPS_FATALOUT("Rps_Loader::parse_user_manifest failed to parse "
                   << umpath << ": " << exc.what());
    };

  RPS_INFORMOUT("parsed user manifest file " << umpath);
} // end Rps_Loader::parse_user_manifest


void Rps_Loader::load_install_roots(void)
{
  RPS_DEBUG_LOG(LOAD, "loader load_install_roots start");
  for (Rps_Id curootid: ld_globrootsidset)
    {
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      Rps_ObjectRef curootobr = find_object_by_oid(curootid);
      if (!curootobr)
        RPS_FATALOUT("load_install_roots: curootid " << curootid
                     << " not found");
      RPS_ASSERT(curootobr);
      rps_add_root_object (curootobr);
    };
  /// install the hard coded global roots
  int nbroots=0;
  {
    std::lock_guard<std::recursive_mutex> gu(ld_mtx);
#define RPS_INSTALL_ROOT_OB(Oid)    {			\
      const char *end##Oid = nullptr;			\
      bool ok##Oid = false;				\
      auto id##Oid = Rps_Id(#Oid, &end##Oid, &ok##Oid);	\
      RPS_ASSERT(end##Oid && *end##Oid == (char)0);	\
      RPS_ASSERT(id##Oid && id##Oid.valid());		\
      RPS_ROOT_OB(Oid) = find_object_by_oid(id##Oid);	\
      if (!RPS_ROOT_OB(Oid))				\
	RPS_WARN("failed to install root " #Oid);	\
      nbroots++;					\
    }
  };
#include "generated/rps-roots.hh"
  RPS_ASSERTPRINTF(nbroots == RPS_NB_ROOT_OB,
                   "load_install_roots nbroots=%u wanted %u",
                   nbroots, RPS_NB_ROOT_OB);
  RPS_DEBUG_LOG(LOAD, "loader load_install_roots nbroots=" << nbroots);
  ///
  /// install the hard coded symbols
  int nbsymb=0;
  {
    std::lock_guard<std::recursive_mutex> gu(ld_mtx);
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name)    {	\
      const char *end##Oid##Name = nullptr;		\
      bool ok##Oid##Name = false;			\
      auto id##Oid##Name = Rps_Id(#Oid,			\
				  &end##Oid##Name,	\
				  &ok##Oid##Name);	\
      RPS_ASSERT(end##Oid##Name				\
		 && *end##Oid##Name == (char)0);	\
      RPS_ASSERT(id##Oid##Name				\
		 && id##Oid##Name.valid());		\
      RPS_SYMB_OB(Name) =				\
	find_object_by_oid(id##Oid##Name);		\
      if (!RPS_SYMB_OB(Name))				\
	RPS_WARN("failed to install symbol "		\
		 #Oid " named " #Name);			\
      nbsymb++;						\
    };
#include "generated/rps-names.hh"
    RPS_ASSERT(nbsymb == RPS_NB_NAMED_ROOT_OB);
  }
  // final check
  RPS_ASSERT(rps_nb_root_objects() == RPS_NB_ROOT_OB);
  RPS_DEBUG_LOG(LOAD, "loader load_install_roots ending nbsymb=" << nbsymb << ", nbroots=" << nbroots << std::endl);
} // end Rps_Loader::load_install_roots




void rps_load_from (const std::string& dirpath)
{
  unsigned nbloaded = 0;
  double startrealt = rps_elapsed_real_time();
  double startcput = rps_process_cpu_time();
  double endrealt = NAN;
  double endcput = NAN;
  RPS_DEBUG_LOG(LOAD, "rps_load_from start dirpath=" << dirpath
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_load_from"));
  {
    Rps_Loader loader(dirpath);
    try
      {
        loader.parse_manifest_file();
        {
          std::string usermanifest{rps_homedir()};
          usermanifest += "/";
          usermanifest += RPS_USER_MANIFEST_JSON;
          if (!access(usermanifest.c_str(), R_OK))
            loader.parse_user_manifest(usermanifest);
        }
        loader.load_all_state_files();
        loader.load_install_roots();
        RPS_DEBUG_LOG(LOAD, "rps_load_from start dirpath=" << dirpath << " after load_install_roots");
        rps_initialize_roots_after_loading(&loader);
        rps_initialize_symbols_after_loading(&loader);
        nbloaded = loader.nb_loaded_objects();
        RPS_DEBUG_LOG(LOAD, "rps_load_from start dirpath=" << dirpath << " nbloaded=" << nbloaded);
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
  };
  RPS_ASSERT(nbloaded > 0);
  endrealt = rps_elapsed_real_time();
  endcput = rps_process_cpu_time();
  double realt = endrealt - startrealt;
  double cput = endcput - startcput;
  char realtbuf[32];
  char cputbuf[32];
  char realmicrobuf[32];
  char cpumicrobuf[32];
  memset(realtbuf, 0, sizeof(realtbuf));
  memset(cputbuf, 0, sizeof(cputbuf));
  memset(realmicrobuf, 0, sizeof(realmicrobuf));
  memset(cpumicrobuf, 0, sizeof(cpumicrobuf));
  snprintf(realtbuf, sizeof(realtbuf), "%.3f", realt);
  snprintf(cputbuf, sizeof(cputbuf), "%.3f", cput);
  snprintf(realmicrobuf, sizeof(realmicrobuf), "%.3f", (realt*1.0e6)/nbloaded);
  snprintf(cpumicrobuf, sizeof(cpumicrobuf), "%.3f", (cput*1.0e6)/nbloaded);
  RPS_INFORMOUT("rps_load_from completed" << std::endl
                << "... from directory " << dirpath
                << " with RefPerSys built " << rps_timestamp << std::endl
                << " lastgitcommit " << rps_lastgitcommit << std::endl
                << " md5sum " << rps_md5sum << std::endl
                << " loaded " << nbloaded << " objects in " << realtbuf << " elapsed, " << cputbuf << " cpu seconds" << std::endl
                << " so " << realmicrobuf << " elapsed µs/ob, " << cpumicrobuf << " cpu µs/ob, in "
                << Rps_QuasiZone::cumulative_allocated_wordcount() << " memory words."<< std::endl
                << "============================================================================="
                << std::endl << std::endl);
} // end of rps_load_from



/// loading of class information payload; see
/// Rps_PayloadClassInfo::dump_json_content in objects_rps.cc
void rpsldpy_classinfo(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (!jv.isMember("class_super") || !jv.isMember("class_methodict"))
    RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has incomplete payload"
                 << std::endl
                 << " jv " << (jv));
  auto paylclainf = obz->put_new_plain_payload<Rps_PayloadClassInfo>();
  RPS_ASSERT(paylclainf != nullptr);
  auto obsuperclass = Rps_ObjectRef(jv["class_super"], ld);
  RPS_ASSERT(obsuperclass);
  paylclainf->put_superclass(obsuperclass);
  if (jv.isMember("class_symb"))
    {
      Json::Value jclasssymb = jv["class_symb"];
      auto obsymb = Rps_ObjectRef(jclasssymb, ld);
      if (!obsymb)
        RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " has bad class_symb"
                     << std::endl
                     << " jclasssymb " <<(jclasssymb));
      paylclainf->loader_put_symbname(obsymb, ld);
    }
  Json::Value jvmethodict = jv["class_methodict"];
  unsigned nbmeth = 0;
  if (!jvmethodict.isArray())
    RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad class_methodict"
                 << std::endl
                 << " jvmethodict " <<(jvmethodict));
  nbmeth = jvmethodict.size();
  for (int methix=0; methix<(int)nbmeth; methix++)
    {
      Json::Value jvcurmeth = jvmethodict[methix];
      if (!jvcurmeth.isObject()
          || !jvcurmeth.isMember("methosel")
          || !jvcurmeth.isMember("methclos")
         )
        RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " has bad methodict entry#" << methix
                     << std::endl
                     << " jvcurmeth " << (jvcurmeth));
      auto obsel = Rps_ObjectRef(jvcurmeth["methosel"], ld);
      auto valclo = Rps_Value(jvcurmeth["methclos"], ld);
      if (!obsel || !valclo.is_closure())
        RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " with bad methodict entry#" << methix
                     << " for obsel=" << obsel
                     << ", valclo=" << valclo
                     << std::endl
                     << " jvcurmeth: " <<jvcurmeth);
      paylclainf->put_own_method(obsel,valclo);
    }
  if (jv.isMember("class_attrset"))
    {
      auto jvatset = jv["class_attrset"];
      auto valaset = Rps_Value(jvatset, ld);
      if (valaset.is_set())
        paylclainf->loader_put_attrset(valaset.as_set(), ld);
      else if (!valaset.is_empty())
        RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " with bad class_attrset" << std::endl
                     << " jvatset:" << jvatset);
    }
} // end of rpsldpy_classinfo




/// loading of vector of objects payload
void rpsldpy_vectob(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  Json::Value jvectob = jv["vectob"];
  unsigned nbelem = 0;
  if (!jvectob.isArray())
    RPS_FATALOUT("rpsldpy_vectob: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad vectob"
                 << std::endl
                 << " jvectob " <<(jvectob));
  auto paylvectob = obz->put_new_plain_payload<Rps_PayloadVectOb>();
  RPS_ASSERT(paylvectob != nullptr);
  nbelem = jvectob.size();
  paylvectob->reserve(nbelem);
  for (int elemix=0; elemix<(int)nbelem; elemix++)
    {
      Json::Value jvcurelem = jvectob[elemix];
      auto obelem = Rps_ObjectRef(jvcurelem, ld);
      paylvectob->push_back(obelem);
    }
} // end of rpsldpy_vectob


/// loading of vector of values payload
void rpsldpy_vectval(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  Json::Value jvectval = jv["vectval"];
  unsigned nbcomp = 0;
  if (!jvectval.isArray())
    RPS_FATALOUT("rpsldpy_vectval: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad vectval"
                 << std::endl
                 << " jvectval " <<(jvectval));
  auto paylvectval = obz->put_new_plain_payload<Rps_PayloadVectVal>();
  RPS_ASSERT(paylvectval != nullptr);
  nbcomp = jvectval.size();
  paylvectval->reserve(nbcomp);
  for (int compix=0; compix<(int)nbcomp; compix++)
    {
      Json::Value jvcurcomp =  jvectval[compix];
      auto compv = Rps_Value(jvcurcomp, ld);
      paylvectval->push_back(compv);
    }
} // end of rpsldpy_vectob

/// loading of set of objects payload
void rpsldpy_setob(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  Json::Value jsetob = jv["setob"];
  unsigned nbelem = 0;
  if (!jsetob.isArray())
    RPS_FATALOUT("rpsldpy_setob: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad setob"
                 << std::endl
                 << " jsetob " <<(jsetob));
  auto paylsetob = obz->put_new_plain_payload<Rps_PayloadSetOb>();
  RPS_ASSERT(paylsetob != nullptr);
  nbelem = jsetob.size();
  for (int elemix=0; elemix<(int)nbelem; elemix++)
    {
      Json::Value jvcurelem = jsetob[elemix];
      auto obelem = Rps_ObjectRef(jvcurelem, ld);
      if (obelem)
        paylsetob->add(obelem);
    }
} // end of rpsldpy_setob


/// loading of space payload
void rpsldpy_space(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_ASSERT(spacid);
  RPS_ASSERT(lineno>0);
  auto paylspace = obz->put_new_plain_payload<Rps_PayloadSpace>();
  RPS_ASSERT(paylspace != nullptr);
} // end of rpsldpy_setob


/// loading of symbol payload
void rpsldpy_symbol(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_NOPRINTOUT("rpsldpy_symbol: obz=" << obz->oid().to_string()
                 << " jv=" << jv
                 << " spacid=" << spacid.to_string()
                 << " lineno:" << lineno);
  const char* name = jv["symb_name"].asCString();
  bool weak = jv["symb_weak"].asBool();
  if (!Rps_PayloadSymbol::valid_name(name))
    RPS_FATALOUT("rpsldpy_symbol: object " << obz->oid()
                 << " in space " << spacid << " lineno#" << lineno
                 << " has bad name:"<< name);
  std::string namestr{name};
  Rps_PayloadSymbol* paylsymb = obz->put_new_plain_payload<Rps_PayloadSymbol>();
  paylsymb->load_register_name(namestr,ld,weak);
  auto jsymbval = jv["symb_val"];
  if (!jsymbval.isNull())
    paylsymb->symbol_put_value(Rps_Value(jsymbval,ld));
} // end rpsldpy_symbol

//////////////////////////////////////////////////////////// end of file store_rps.cc


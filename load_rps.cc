/****************************************************************
 * file load_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for loading the persistent store, in JSON
 *      format.  See also http://json.org/ &
 *      https://github.com/open-source-parsers/jsoncpp/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2025 The Reflective Persistent System Team
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
 *
 * Notice:
 *    Same code used to be in store_rps.cc file in july 2022.
 ******************************************************************************/

#include "refpersys.hh"



extern "C" const char rps_load_gitid[];
const char rps_load_gitid[]= RPS_GITID;

extern "C" const char rps_load_date[];
const char rps_load_date[]= __DATE__;

extern "C" const char rps_load_shortgitid[];
const char rps_load_shortgitid[]= RPS_SHORTGITID;

extern "C" char rps_loaded_directory[rps_path_byte_size];
char rps_loaded_directory[rps_path_byte_size];

Json::Value
rps_load_string_to_json(const std::string&str, const char*filnam, int lineno)
{
  Json::CharReaderBuilder jsonreaderbuilder;
  std::unique_ptr<Json::CharReader> pjsonreader(jsonreaderbuilder.newCharReader());
  Json::Value jv;
  JSONCPP_STRING errstr;
  RPS_ASSERT(pjsonreader);
  if (!pjsonreader->parse(str.c_str(), str.c_str() + str.size(), &jv, &errstr))
    {
      if (filnam != nullptr && lineno > 0)
        {
          RPS_WARNOUT("JSON parse failure (loading) at " << filnam << ":" << lineno
                      << std::endl << str);
        }
      throw std::runtime_error(std::string("JSON parsing error:") + errstr);
    }
  return jv;
} // end rps_load_string_to_json



//////////////////////////////////////////////// loader
class Rps_Loader
{
  std::string ld_topdir;
  double ld_startclock;
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
  void initialize_root_objects(void);
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
  void set_primitive_type_size_and_align(Rps_ObjectRef primtypob,
                                         unsigned sizeby, unsigned alignby);
  // run some todo functions, return the number of remaining ones
  int run_some_todo_functions(void);
  void load_install_roots(void);
  unsigned nb_loaded_objects(void) const
  {
    return ld_mapobjects.size();
  };
};        // end class Rps_Loader



Rps_Loader::Rps_Loader(const std::string&topdir) :
  ld_topdir(topdir),
  ld_startclock(rps_wallclock_real_time()),
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
  const char*oidstart = nullptr;
  const char*end = nullptr;
  const char*linestart = nullptr;
  char reasonbuf[64];
  Rps_Id oid;
  bool ok=false;
  char c = 0;
  int cix= -1;
  memset (reasonbuf, 0, sizeof(reasonbuf));
  size_t linelen = linbuf.size();
  if (pobid)
    *pobid = Rps_Id(nullptr);
  if (linelen < 8 || linbuf[0] != '/' || linbuf[1] != '/'
      || linbuf[2] != '+'
      || linbuf[3] != 'o'
      || linbuf[4] != 'b'
      || linbuf[5] != '_')
    return false;
  if (linelen < strlen ("//+ob") + Rps_Id::nbchars)
    {
      reason = "too short";
      goto bad;
    }
  linestart = linbuf.c_str();
  oidstart = linestart + strlen("//+ob");
  char oidbuf[Rps_Id::nbchars+8];
  memset (oidbuf, 0, sizeof(oidbuf));
  oidbuf[0] = '_';
  for (cix=1; cix<(int)Rps_Id::nbchars; cix++)
    {
      c = oidstart[cix];
      switch (c)
        {
        case '0' ... '9':
        case 'a' ... 'z':
        case 'A' ... 'Z':
          oidbuf[cix] = c;
          continue;
        default:
          memset (reasonbuf, 0, sizeof(reasonbuf));
          if (c > ' ' && c < 127)
            snprintf (reasonbuf, sizeof(reasonbuf)-1,
                      "bad character %c at column %d",
                      c, (int)(oidstart+cix-linestart));
          else
            snprintf (reasonbuf, sizeof(reasonbuf)-1,
                      "bad character code %d (\\%#02x) at colum %d",
                      (int)c, (int)c, (int)( oidstart+cix-linestart));
          reason = reasonbuf;
          goto bad;
        } // end case c
    };
  {
    Rps_Id tempoid(oidbuf, &end, &ok);
    if (!end || (*end && !isspace(*end) && *end != ':'))
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
                  prologjson = rps_load_string_to_json(prologstr);
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
              if (formatjson.asString() != RPS_MANIFEST_FORMAT
                  && formatjson.asString() != RPS_PREVIOUS_MANIFEST_FORMAT)
                RPS_FATALOUT("space file " << spacepath
                             << "should have format: "
                             << RPS_MANIFEST_FORMAT
                             << " or " << RPS_PREVIOUS_MANIFEST_FORMAT
                             << " but got "
                             << formatjson);
              if (prologjson["spaceid"].asString() != spacid.to_string())
                RPS_FATAL("spacefile %s should have spaceid: '%s' but got '%s'",
                          spacepath.c_str (), spacid.to_string().c_str(),
                          prologjson["spaceid"].asString().c_str());
              int majv = prologjson["rpsmajorversion"].asInt();
              int minv = prologjson["rpsminorversion"].asInt();
              if (majv != rps_get_major_version()
                  || minv != rps_get_minor_version())
                RPS_WARNOUT("space file " << spacepath
                            << " was dumped by RefPerSys " << majv << "." << minv
                            << " but is loaded by RefPerSys " << rps_get_major_version()
                            << "." << rps_get_minor_version());
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
Rps_Loader::initialize_root_objects(void)
{
  std::lock_guard<std::recursive_mutex> gu(ld_mtx);
#define RPS_INSTALL_ROOT_OB(Oid) do {   \
    if (!RPS_ROOT_OB(Oid))      \
      RPS_ROOT_OB(Oid)        \
  = find_object_by_oid(Rps_Id(#Oid));   \
    RPS_ASSERT(RPS_ROOT_OB(Oid));   \
  } while(0);
#include "generated/rps-roots.hh"
} // end Rps_Loader::initialize_root_objects




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
      objjson = rps_load_string_to_json(objbuf);
      if (objjson.type() != Json::objectValue)
        RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                     << " lineno:" << lineno
                     << " objid:" << objid
                     << " bad objbuf:" << std::endl
                     << objbuf
                     << std::endl << "... and objjson:" << objjson);
    }
  catch (std::exception& exc)
    {
      RPS_FATALOUT("parse_json_buffer_second_pass spacid=" << spacid
                   << " lineno:" << lineno
                   << " objid:" << objid
                   << " parse failure "
                   << exc.what()
                   << "... with objbuf:" << std::endl
                   << objbuf
                   << std::endl << "... and objjson:" << objjson);
    };
  //// now load the various JSON members
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
  double mtim =  objjson["mtime"].asDouble();
  if (mtim > this->ld_startclock + 300.0)
    {
      double cormtim = this->ld_startclock + 300.0;
      RPS_WARNOUT("parse_json_buffer_second_pass mtime of object " << objid
                  << " is too far in the future in  spacid=" << spacid
                  << " lineno:" << lineno
                  << " changed from " << mtim << " to " << cormtim);
      mtim = cormtim;
    }
  obz->loader_set_mtime (this,mtim);
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
                  RPS_ASSERT(atobr);
                  auto atval = Rps_Value(entjson["va"], this);
                  RPS_ASSERT(atval);
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
    };        // end with "magicattr" JSON member
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
    };        // end with "applying" JSON member
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
    }; //// end handling of "payload" JSON member
  if (obz->is_instance_of(RPS_ROOT_OB(_3O1QUNKZ4bU02amQus) //∈rps_routine
                         ))
    {
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      char appfunambuf[sizeof(RPS_APPLYINGFUN_PREFIX)+8+Rps_Id::nbchars];
      memset(appfunambuf, 0, sizeof(appfunambuf));
      char obidbuf[32];
      memset (obidbuf, 0, sizeof(obidbuf));
      obz->oid().to_cbuf24(obidbuf);
      strcpy(appfunambuf, RPS_APPLYINGFUN_PREFIX);
      strcat(appfunambuf+strlen(RPS_APPLYINGFUN_PREFIX), obidbuf);
      RPS_ASSERT(strlen(appfunambuf)<sizeof(appfunambuf)-4);
      void*funad = dlsym(rps_proghdl, appfunambuf);
      if (!funad)
        RPS_WARNOUT("cannot dlsym " << appfunambuf << " for applying function of objid:" <<  objid
                    << Rps_ObjectRef(obz)
                    << " lineno:" << lineno << ", spacid:" << spacid
                    << ":: " << dlerror());
      else
        obz->loader_put_applyingfunction(this, reinterpret_cast<rps_applyingfun_t*>(funad));
    };

  if (objjson.isMember("loadrout"))
    {
      auto loadroutstr = objjson["loadrout"].asString();
      std::lock_guard<std::recursive_mutex> gu(ld_mtx);
      if (loadroutstr.empty())
        RPS_WARNOUT("invalid loadrout for loading routine function of objid:" <<  objid
                    << Rps_ObjectRef(obz)
                    << " lineno:" << lineno << ", spacid:" << spacid
                    << std::endl << objjson);
      else
        {
          void*ldroutad = dlsym(rps_proghdl, loadroutstr.c_str());
          if (!ldroutad)
            RPS_WARNOUT("cannot dlsym " << loadroutstr
                        << " for loading routine function of objid:" <<  objid
                        << Rps_ObjectRef(obz)
                        << " lineno:" << lineno << ", spacid:" << spacid
                        << ":: " << dlerror());
          rpsldpysig_t*ldrout = (rpsldpysig_t*)ldroutad;
          (*ldrout)(obz, this, objjson, spacid, lineno);
        };
    };        // end if has "loadrout" member
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
  const char*thisprog = (rps_progexe[0]?rps_progexe
                         :rps_progname?rps_progname:"*RefPerSys*");
  RPS_ASSERT(thisprog != nullptr);
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::load_all_state_files start this@" << (void*)this
                << std::endl << RPS_FULL_BACKTRACE_HERE(0, "RpsLoader::load_all_state_files"));
  int spacecnt1 = 0, spacecnt2 = 0;
  Rps_Id initialspaceid("_8J6vNYtP5E800eCr5q"); //"initial_space"∈space
  first_pass_space(initialspaceid);
  spacecnt1++;
  initialize_root_objects();
  for (Rps_Id spacid: ld_spaceset)
    {
      if (spacid != initialspaceid)
        first_pass_space(spacid);
      spacecnt1++;
    }
  RPS_INFORM("%s loaded %d space files in first pass", thisprog, spacecnt1);
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
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::load_all_state_files end this@"
                << (void*)this);
  RPS_INFORM("%s loaded %d space files in first pass,\n"
             " %d space files in second passes,\n"
             " %ld objects from directory %s,\n"
             " in %.2f real sec (pid %ld on host %s git %s)",
             thisprog,
             spacecnt1, spacecnt2, (long)ld_mapobjects.size(),
             ld_topdir.c_str(), rps_wallclock_real_time() - ld_startclock,
             (long)getpid(), rps_hostname(), rps_shortgitid);
  RPS_DEBUG_LOG(LOAD, "Rps_Loader::load_all_state_files end this@"
                << (void*)this << std::endl
                << RPS_FULL_BACKTRACE_HERE(0, "RpsLoader::load_all_state_files"));
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
  std::size_t subsiz=0;
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
      if (str.size() == Rps_Id::nbchars && str[0] == '_' && isalnum(str[1])
          && std::all_of(str.begin()+1, str.end(),
                         [](char c)
      {
        return strchr(Rps_Id::b62digits, c) != nullptr;
        }))
      {
        *this = Rps_ObjectValue(Rps_ObjectRef(jv, ld));
        RPS_ASSERT(*this);
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
      siz = jv.size();
      RPS_POSSIBLE_BREAKPOINT();
      if (str == "set" && siz==2 && jv.isMember("elem")
          && (jcomp=jv["elem"]).isArray())
        {
          subsiz = jcomp.size();
          std::set<Rps_ObjectRef> setobr;
          for (int ix=0; ix<(int)subsiz; ix++)
            {
              auto obrelem = Rps_ObjectRef(jcomp[ix], ld);
              if (obrelem)
                setobr.insert(obrelem);
            }
          *this= Rps_SetValue(setobr);
          return;
        }
      else if (str == "tuple" && siz==2 && jv.isMember("elem")
               && (jcomp=jv["comp"]).isArray())
        {
          subsiz = jcomp.size();
          std::vector<Rps_ObjectRef> vecobr;
          vecobr.reserve(subsiz);
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
              subsiz = jenv.size();
              std::vector<Rps_Value> vecenv;
              vecenv.reserve(subsiz+1);
              for (int ix=0; ix <(int)subsiz; ix++)
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
            RPS_WARNOUT("Rps_Value::Rps_Value bad closure funobr=" << funobr << " jv=" << jv);
        }
      else
        RPS_WARNOUT("strange Rps_Value::Rps_Value siz=" << siz << " str=" << str << " jv=" << jv);
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
      manifjson = rps_load_string_to_json(manifstr);
      if (manifjson.type () != Json::objectValue)
        RPS_FATAL("Rps_Loader::parse_manifest_file wants a Json object in %s",
                  manifpath.c_str());
    }
  catch  (const std::exception& exc)
    {
      RPS_FATALOUT("Rps_Loader::parse_manifest_file failed to parse: " << exc.what());
    };
  if (manifjson["format"].asString() != RPS_MANIFEST_FORMAT && manifjson["format"].asString() != RPS_PREVIOUS_MANIFEST_FORMAT)
    RPS_FATAL("manifest map in %s should have format: '%s' or older '%s' but got:\n"
              "%s",
              manifpath.c_str (), RPS_MANIFEST_FORMAT, RPS_PREVIOUS_MANIFEST_FORMAT,
              manifjson["format"].toStyledString().c_str());
  int majv = manifjson["rpsmajorversion"].asInt();
  int minv = manifjson["rpsminorversion"].asInt();
  if (majv != rps_get_major_version() || minv != rps_get_minor_version())
    {
      RPS_WARNOUT("manifest path " << manifpath.c_str()
                  << " dumped by RefPerSys " << majv << "." << minv
                  << " but loaded by "
                  << rps_get_major_version() << "." << rps_get_minor_version());
    };
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
    if (sizeglobroots != RPS_NB_ROOT_OB)
      RPS_WARNOUT("in loaded manifest file " << manifpath
                  << " we have " << sizeglobroots << " globalroots "
                  << " for compiled RPS_NB_ROOT_OB=" << RPS_NB_ROOT_OB);
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
      manifjson = rps_load_string_to_json(manifstr);
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
#define RPS_INSTALL_ROOT_OB(Oid)    {                   \
      const char *end##Oid = nullptr;                   \
      bool ok##Oid = false;                             \
      auto id##Oid = Rps_Id(#Oid, &end##Oid, &ok##Oid); \
      RPS_ASSERT(end##Oid && *end##Oid == (char)0);     \
      RPS_ASSERT(id##Oid && id##Oid.valid());           \
      RPS_ROOT_OB(Oid) = find_object_by_oid(id##Oid);   \
      if (!RPS_ROOT_OB(Oid))                            \
  RPS_WARN("failed to install root " #Oid);             \
      nbroots++;                                        \
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
    //
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name)    {        \
      const char *end##Oid##Name = nullptr;             \
      bool ok##Oid##Name = false;                       \
      auto id##Oid##Name = Rps_Id(#Oid,                 \
          &end##Oid##Name,                              \
          &ok##Oid##Name);                              \
      RPS_ASSERT(end##Oid##Name                         \
     && *end##Oid##Name == (char)0);                    \
      RPS_ASSERT(id##Oid##Name                          \
     && id##Oid##Name.valid());                         \
      RPS_SYMB_OB(Name) =                               \
  find_object_by_oid(id##Oid##Name);                    \
      if (!RPS_SYMB_OB(Name))                           \
  RPS_WARN("failed to install symbol "                  \
     #Oid " named " #Name);                             \
      nbsymb++;                                         \
    };
    //
#include "generated/rps-names.hh"
    RPS_ASSERT(nbsymb == RPS_NB_NAMED_ROOT_OB);
  }
  // final check
  if (rps_nb_root_objects() != RPS_NB_ROOT_OB)
    RPS_FATALOUT("got rps_nb_root_objects()=" << rps_nb_root_objects()
                 << " expecting RPS_NB_ROOT_OB=" << RPS_NB_ROOT_OB
                 << " loading directory " << ld_topdir);
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
        rps_set_native_data_in_loader(&loader);
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
  strncpy(rps_loaded_directory, dirpath.c_str(), sizeof(rps_loaded_directory)-1);
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
void
rpsldpy_classinfo(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
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
      Json::Value jvcurmethent = jvmethodict[methix];
      if (!jvcurmethent.isObject()
          || !(jvcurmethent.isMember("methosel")
               && jvcurmethent.isMember("methclos"))
         )
        RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " has bad methodict entry#" << methix
                     << std::endl
                     << " jvcurmethent " << (jvcurmethent)
                     << std::endl << "... needs methosel & methclos"
                     << std::endl << "... isobject? " <<  jvcurmethent.isObject()
                     << " .. has-methosel?" << jvcurmethent.isMember("methosel")
                     << " .. has-methclos?" << jvcurmethent.isMember("methclos")
                    );
      auto obsel = Rps_ObjectRef(jvcurmethent["methosel"], ld);
      auto valclo = Rps_Value(jvcurmethent["methclos"], ld);
      if (!obsel || !valclo.is_closure())
        RPS_FATALOUT("rpsldpy_classinfo: object " << obz->oid()
                     << " in space " << spacid << " lineno#" << lineno
                     << " with bad methodict entry#" << methix
                     << " for obsel=" << obsel
                     << ", valclo=" << valclo
                     << std::endl
                     << " jvcurmethent: " <<jvcurmethent);
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
void
rpsldpy_vectob(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
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
void
rpsldpy_vectval(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
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
void
rpsldpy_setob(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
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
void
rpsldpy_space(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
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
void
rpsldpy_symbol(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
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


void
Rps_Loader::set_primitive_type_size_and_align(Rps_ObjectRef primtypob,
    unsigned sizeby, unsigned alignby)
{
  RPS_ASSERT(primtypob);
  RPS_ASSERT(primtypob->is_instance_of(RPS_ROOT_OB(_1XswYkom3Jm02YR3Vi))); //cplusplus_primitive_type∈class
  primtypob->loader_put_attr(this, rpskob_6EsfxShTuwH02waeLE, //!byte_alignment∈named_attribute
                             Rps_Value((intptr_t)sizeby));
  primtypob->loader_put_attr(this, rpskob_8IRzlYX53kN00tC3fG, //!byte_size∈named_attribute
                             Rps_Value((intptr_t)alignby));
} /* end Rps_Loader::set_primitive_type_size_and_align */


void
rps_set_native_data_in_loader(Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  RPS_WARNOUT("incomplete rps_set_native_data_in_loader" << std::endl
              << RPS_FULL_BACKTRACE_HERE(1, "rps_set_native_data_in_loader"));
  /* Some loaded objects are representing machine types, and their
     attributes _6EsfxShTuwH02waeLE !byte_alignment∈named_attribute
     and _8IRzlYX53kN00tC3fG !byte_size∈named_attribute need to be set
     to integers particular to this machine using alignof() and
     sizeof() C++ macros */
  /* TODO: use ld->set_primitive_type_size_and_align here */
  /* all the constants below have class cplusplus_primitive_type*/
  ld->set_primitive_type_size_and_align(rpskob_4V1oeUOvmxo041XLTm, //code_intptr_t
                                        sizeof(intptr_t),
                                        alignof(intptr_t)
                                       );
  ld->set_primitive_type_size_and_align(rpskob_4nZ0jIKUbGr01OixPV, //code_int
                                        sizeof(int),
                                        alignof(int)
                                       );
  ld->set_primitive_type_size_and_align(rpskob_3NYlqvmSuTm024LDuD, //code_long
                                        sizeof(long),
                                        alignof(long)
                                       );
#warning incomplete rps_set_native_data_in_loader
} // end rps_set_native_data_in_loader

//// end of file load_rps.cc

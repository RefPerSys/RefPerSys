/****************************************************************
 * file dump_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for dumping the persistent store, in JSON
 *      format.  See also http://json.org/ &
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
 *
 * Notice:
 *    Same code used to be in store_rps.cc file in july 2022.
 ******************************************************************************/

#include "refpersys.hh"



extern "C" const char rps_dump_gitid[];
const char rps_dump_gitid[]= RPS_GITID;

extern "C" const char rps_dump_date[];
const char rps_dump_date[]= __DATE__;

std::string
rps_dump_json_to_string(const Json::Value&jv)
{
  Json::StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = " ";
  auto str = Json::writeString(builder, jv);
  return str;
} // end rps_dump_json_to_string


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
      std::unique_lock<std::recursive_mutex> gurefpersysob(*(_f.refpersysob->objmtxptr()));

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
                      << _f.refpersysob << " (of class "
                      <<  _f.refpersysob->get_class() << " and "
                      << "payload type " << _f.refpersysob->payload_type_name() << ")"
                      << " genstoreob:" << _f.genstoreob
                      << " with " << _f.dumpdirnamev << " & " << _f.tempsuffixv
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
  jmanifest["jsoncpp-version"] = JSONCPP_VERSION_STRING;
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
  RPS_ASSERT(du_jsonwriterbuilder["indentation"] == std::string{" "});
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
    jprologue["jsoncpp-version"] =  JSONCPP_VERSION_STRING;
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
      *pouts << "{" << std::endl;
      *pouts << " \"oid\": \"" << curobr->oid().to_string() << '"' << ',' << std::endl;
      {
        double curmtim = curobr->ob_mtime;
        char mtimbuf[16];
        memset (mtimbuf, 0, sizeof(mtimbuf));
        snprintf (mtimbuf, sizeof(mtimbuf), "%.2f", curmtim);
        *pouts << " \"mtime\": " << mtimbuf << ',' << std::endl;
      }
      int countjat = 2; // both oid & mtime have been output
      int nbjat = jobject.size();
      Json::Value::Members jmembvec = jobject.getMemberNames();
      for (const std::string curmemstr : jmembvec)
        {
          const Json::Value& jcurmem = jobject[curmemstr];
          if (curmemstr != std::string{"oid"} && curmemstr != std::string{"mtime"})
            {
	      std::ostringstream outmem;
	      outmem << jcurmem << std::flush;
	      std::string outstr = outmem.str();
	      if (!outstr.empty() && outstr[outstr.size()-1] == '\n') {
		outstr.pop_back();
	      }
              *pouts << " \"" << curmemstr << "\" : ";
	      int cnt = 0;
	      for (char c : outstr) {
		if (c=='\n' && cnt>0)
		  *pouts << "\n  " << c;
		else
		  *pouts << c;
		cnt++;
	      }
              if (countjat+1 < nbjat)
                *pouts << ',' << std::endl;
              else
                *pouts << std::endl;
              countjat++;
            }
        }
      *pouts << "}" << std::endl;
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


#warning rpsapply_5Q5E0Lw9v4f046uAKZ should be installed as "generate_code°the_system_class"
/***
 * We need to manually edit the persistore/sp_8J6vNYtP5E800eCr5q-rps.json to avoid:::
** RefPerSys WARN! dump_rps.cc:1020:: Rps_Dumper::write_all_generated_files failed to send ◌_5VC4IuJ0dyr01b8lA0/generate_code to ◌_1Io89yIORqn02SXx4p/RefPerSys_system (of class ◌_10YXWeY7lYc01RpQTA/the_system_class and payload type symbol) genstoreob:◌_9wFykw9FYCj01qGGmB with "/tmp/refpersys-822d425661847d20+_144752" & "_2Z5MwS-p144753%"
***/

extern "C" rps_applyingfun_t rpsapply_5Q5E0Lw9v4f046uAKZ;
/// method generate_code°the_system_class
Rps_TwoValues
rpsapply_5Q5E0Lw9v4f046uAKZ(Rps_CallFrame*callerframe,
                            const Rps_Value arg0,
                            const Rps_Value arg1,
                            const Rps_Value arg2,
                            const Rps_Value arg3,
                            [[maybe_unused]] const std::vector<Rps_Value>* restargs)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(restargs == nullptr);
  RPS_LOCALFRAME(/*descr:*/nullptr, callerframe,
                           Rps_ObjectRef sysob;
                           Rps_Value dumpstrv;
                           Rps_Value suffixstrv;
                           Rps_ObjectRef dumpob;
                           Rps_Value closurev;
                );
  _f.sysob = arg0.as_object();
  _f.dumpstrv = arg1;
  _f.suffixstrv = arg2;
  _f.dumpob = arg3.as_object();
  _f.closurev = callerframe->call_frame_closure();
  char cwdbuf[128];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  const char*cwds = getcwd(cwdbuf, sizeof(cwdbuf)-1);
  if (!cwds)
    cwds = ".";
  RPS_WARNOUT("unimplemented rpsapply_5Q5E0Lw9v4f046uAKZ generate_code°the_system_class arg0=" << arg0 << " arg1=" << arg1 << " arg2=" << arg2 << " arg3=" << arg3
              << " closure:" << callerframe->call_frame_closure()
              << std::endl << RPS_FULL_BACKTRACE_HERE(1, "rpsapply_5Q5E0Lw9v4f046uAKZ generate_code°the_system_class")
              << std::endl << "cwd:" << cwds << " pid:" << (int)getpid() << " from " << (rps_is_main_thread()?"main":"other") << " thread");
  // arg0 is reciever, so _1Io89yIORqn02SXx4p⟦⏵RefPerSys_system∈the_system_class⟧
  RPS_ASSERT(arg0.is_object());
  // arg1 is the dumped directory string, e.g. ~/RefPerSys
  RPS_ASSERT(_f.dumpstrv.is_string());
  // arg2 is a temporary suffix like "_3MPAZx-p1084952%"
  RPS_ASSERT(_f.suffixstrv.is_string());
  // arg3 is a temporary dump object
  RPS_ASSERT(_f.dumpob);
#warning unimplemented rpsapply_5Q5E0Lw9v4f046uAKZ "generate_code°the_system_class"
  //// for inspiration read https://en.wikipedia.org/wiki/Quine_(computing)
  return {arg0,nullptr};
} // end  rpsapply_5Q5E0Lw9v4f046uAKZ generate_code°the_system_class

//// end of file dump_rps.cc

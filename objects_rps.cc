/****************************************************************
 * file objects_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      Low-level implementation of objects.
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

extern "C" const char rps_objects_gitid[];
const char rps_objects_gitid[]= RPS_GITID;

extern "C" const char rps_objects_date[];
const char rps_objects_date[]= __DATE__;


std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher> Rps_ObjectZone::ob_idmap_(50777);

std::map<Rps_Id,Rps_ObjectZone*> Rps_ObjectZone::ob_idbucketmap_[Rps_Id::maxbuckets];
std::recursive_mutex Rps_ObjectZone::ob_idmtx_;


void
Rps_ObjectRef::output(std::ostream&outs) const
{
  if (is_empty())
    outs << "__";
  else
    outs << obptr()->oid().to_string();
} // end Rps_ObjectRef::output

void
Rps_ObjectZone::val_output(std::ostream&out, unsigned int) const
{
  out << oid().to_string();
} // end Rps_ObjectZone::val_output

void
Rps_ObjectZone::register_objzone(Rps_ObjectZone*obz)
{
  RPS_ASSERT(obz != nullptr);
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  auto oid = obz->oid();
  if (ob_idmap_.find(oid) != ob_idmap_.end())
    RPS_FATALOUT("Rps_ObjectZone::register_objzone duplicate oid " << oid);
  ob_idmap_.insert({oid,obz});
  ob_idbucketmap_[oid.bucket_num()].insert({oid,obz});
} // end Rps_ObjectZone::register_objzone


Rps_Id
Rps_ObjectZone::fresh_random_oid(Rps_ObjectZone*ob)
{
  Rps_Id oid;
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  while(true)
    {
      oid = Rps_Id::random();
      if (RPS_UNLIKELY(ob_idmap_.find(oid) != ob_idmap_.end()))
        continue;
      ob_idmap_.insert({oid,ob});
      return oid;
    }
}


Rps_ObjectZone::Rps_ObjectZone(Rps_Id oid, bool dontregister)
  : Rps_ZoneValue(Rps_Type::Object),
    ob_oid(oid), ob_mtx(), ob_class(nullptr),
    ob_space(nullptr), ob_mtime(0.0),
    ob_attrs(), ob_comps(), ob_payload(nullptr),
    ob_magicgetterfun(nullptr),
    ob_applyingfun(nullptr)
{
  if (!dontregister)
    register_objzone(this);
} // end Rps_ObjectZone::Rps_ObjectZone


Rps_ObjectZone::~Rps_ObjectZone()
{
  //  RPS_INFORMOUT("destroying object " << oid());
  Rps_Id curid = oid();
  clear_payload();
  ob_attrs.clear();
  ob_comps.clear();
  ob_class.store(nullptr);
  ob_mtime.store(0.0);
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  ob_idmap_.erase(curid);
  ob_idbucketmap_[curid.bucket_num()].erase(curid);
} // end Rps_ObjectZone::~Rps_ObjectZone()

Rps_ObjectZone::Rps_ObjectZone() :
  Rps_ObjectZone::Rps_ObjectZone(fresh_random_oid(this), false)
{
} // end Rps_ObjectZone::Rps_ObjectZone


Rps_ObjectZone*
Rps_ObjectZone::make(void)
{
  Rps_ObjectZone*obz= Rps_QuasiZone::rps_allocate<Rps_ObjectZone,Rps_Id,bool>(Rps_Id(),true);
  Rps_Id oid = fresh_random_oid(obz);
  *(const_cast<Rps_Id*>(&obz->ob_oid)) = oid;
  return obz;
} // end Rps_ObjectZone::make


Rps_ObjectZone*
Rps_ObjectZone::make_loaded(Rps_Id oid, Rps_Loader* ld)
{
#warning Rps_ObjectZone::make_loaded might be incomplete
  RPS_ASSERT(oid.valid());
  RPS_ASSERT(ld != nullptr);
  Rps_ObjectZone*obz= Rps_QuasiZone::rps_allocate<Rps_ObjectZone,Rps_Id,bool>(oid, false);
  //RPS_INFORMOUT("make_loaded oid="<< oid << ", obz=" << (void*)obz);
  return obz;
} // end Rps_ObjectZone::make_loaded


Rps_ObjectZone*
Rps_ObjectZone::find(Rps_Id oid)
{
  if (!oid.valid())
    return nullptr;
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  auto obr = ob_idmap_.find(oid);
  if (obr != ob_idmap_.end())
    return obr->second;
  return nullptr;
} // end Rps_ObjectZone::find

void
Rps_ObjectZone::gc_mark(Rps_GarbageCollector&gc, unsigned) const
{
  if (is_gcmarked(gc)) return;
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  gc.mark_obj(this);
  const_cast<Rps_ObjectZone*>(this)->set_gcmark(gc);
} // end of Rps_ObjectZone::gc_mark

void
Rps_ObjectZone::mark_gc_inside(Rps_GarbageCollector&gc)
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
#warning perhaps the _gcinfo should be used here
  Rps_ObjectZone* obcla = ob_class.load();
  RPS_ASSERT(obcla != nullptr);
  gc.mark_obj(obcla);
  for (auto atit: ob_attrs)
    {
      gc.mark_obj(atit.first);
      if (atit.second.is_ptr())
        gc.mark_value(atit.second);
    }
  for (auto compv: ob_comps)
    {
      if (compv.is_ptr())
        gc.mark_value(compv);
    };
  Rps_Payload*payl = ob_payload.load();
  if (payl && payl->owner() == this)
    payl->gc_mark(gc);
} // end Rps_ObjectZone::mark_gc_inside

void
Rps_ObjectZone::dump_scan_contents(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  Rps_ObjectZone* obcla = ob_class.load();
  RPS_ASSERT(obcla != nullptr);
  rps_dump_scan_object(du, obcla);
  Rps_ObjectZone* obspace = ob_space.load();
  RPS_ASSERT(obspace != nullptr);
  rps_dump_scan_object(du, obspace);
  rps_dump_scan_space_component(du, Rps_ObjectRef(obspace), Rps_ObjectRef(this));
  for (auto atit: ob_attrs)
    {
      rps_dump_scan_object(du, atit.first);
      if (atit.second.is_ptr())
        rps_dump_scan_value(du, atit.second, 0);
    }
  for (auto compv: ob_comps)
    {
      if (compv.is_ptr())
        rps_dump_scan_value(du, compv, 0);
    };
  {
    rps_magicgetterfun_t*mgfun = ob_magicgetterfun.load();
    if (mgfun)
      rps_dump_scan_code_addr(du, reinterpret_cast<const void*>(mgfun));
  }
  {
    rps_applyingfun_t* apfun = ob_applyingfun.load();
    if (apfun)
      rps_dump_scan_code_addr(du, reinterpret_cast<const void*>(apfun));
  }
  Rps_Payload*payl = ob_payload.load();
  if (payl && payl->owner() == this)
    payl->dump_scan(du);
} // end Rps_ObjectZone::dump_scan_contents

void
Rps_ObjectZone::dump_json_content(Rps_Dumper*du, Json::Value&json) const
{
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(json.type() == Json::objectValue);
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  Rps_ObjectRef thisob(this);
  Rps_ObjectZone* obcla = ob_class.load();
  RPS_ASSERT(obcla != nullptr);
  json["class"] = rps_dump_json_objectref(du,obcla);
  json["mtime"] = Json::Value (get_mtime());
  /// magic getter function
  {
    rps_magicgetterfun_t*mgfun = ob_magicgetterfun.load();
    if (mgfun)
      {
        Dl_info di = {};
        if (dladdr((void*)mgfun, &di))
          {
            RPS_INFORMOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                          << " has magicgetter " << (void*)mgfun
                          << " dli_fname=" << (di.dli_fname?:"???")
                          << " dli_sname=" << (di.dli_sname?:"???"));
          }
        else
          {
            RPS_WARNOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                        << " has strange magicgetter " << (void*)mgfun);
          };
        json["magicattr"] = Json::Value(true);
      }
    else
      RPS_INFORMOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                    << " has no magicgetter");
  }
  /// applying function
  {
    rps_applyingfun_t*apfun = ob_applyingfun.load();
    if (apfun)
      {
        Dl_info di = {};
        if (dladdr((void*)apfun, &di))
          {
            RPS_INFORMOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                          << " has applyingfun " << (void*)apfun
                          << " dli_fname=" << (di.dli_fname?:"???")
                          << " dli_sname=" << (di.dli_sname?:"???"));
          }
        else
          {
            RPS_WARNOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                        << " has strange applyingfun " << (void*)apfun);
          };
        json["applying"] = Json::Value(true);
      }
    else
      RPS_INFORMOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                    << " has applying function");
  }
  /// attributes
  if (!ob_attrs.empty())
    {
      Json::Value jattrs(Json::arrayValue);
      for (auto atit: ob_attrs)
        {
          if (!rps_is_dumpable_objref(du,atit.first))
            continue;
          Json::Value jcurat(Json::objectValue);
          jcurat["at"] = rps_dump_json_objectref(du,atit.first);
          jcurat["va"] = rps_dump_json_value(du,atit.second);
          jattrs.append(jcurat);
        };
      json["attrs"] = jattrs;
    }
  if (!ob_comps.empty())
    {
      Json::Value jcomps(Json::arrayValue);
      for (auto compv: ob_comps)
        {
          jcomps.append(rps_dump_json_value(du,compv));
        };
      json["comps"] = jcomps;
    };
  Rps_Payload*payl = ob_payload.load();
  if (payl && payl->owner() == this)
    {
      json["payload"] = Json::Value(payl->payload_type_name());
      payl->dump_json_content(du,json);
    }
} // end Rps_ObjectZone::dump_json_contents

bool
Rps_ObjectZone::equal(const Rps_ZoneValue&zv) const
{
  if (zv.stored_type() == Rps_Type::Object)
    {
      auto othob = reinterpret_cast<const Rps_ObjectZone*>(&zv);
      return this == othob;
    }
  return false;
} // end of Rps_ObjectZone::equal

bool
Rps_ObjectZone::less(const Rps_ZoneValue&zv) const
{
  if (zv.stored_type() > Rps_Type::Object) return false;
  if (zv.stored_type() < Rps_Type::Object) return true;
  {
    auto othob = reinterpret_cast<const Rps_ObjectZone*>(&zv);
    return this->oid() < othob->oid();
  }
} // end of Rps_ObjectZone::less

std::string
Rps_ObjectZone::string_oid(void) const
{
  return oid().to_string();
}

int
Rps_ObjectZone::autocomplete_oid(const char*prefix,
                                 const std::function<bool(const Rps_ObjectZone*)>&stopfun)
{
  if (!prefix || prefix[0] != '_'
      || !isdigit(prefix[1]) || !isalnum(prefix[2]) || !isalnum(prefix[3]))
    return 0;
  char bufid[24];
  memset(bufid, 0, sizeof(bufid));
  int lastix=0;
  {
    int ix=0;
    bufid[0] = '_';
    for (ix=1; ix<(int)Rps_Id::nbchars; ix++)
      {
        if (!strchr(Rps_Id::b62digits, prefix[ix]))
          break;
        bufid[ix] = prefix[ix];
      }
    lastix = ix;
    for (ix=lastix; ix<(int)Rps_Id::nbchars; ix++)
      {
        bufid[ix] = '0';
      };
  }
  Rps_Id idpref(bufid);
  constexpr char lastdigit = Rps_Id::b62digits[sizeof(Rps_Id::b62digits)-1];
  for (int ix=lastix; ix<(int)Rps_Id::nbchars; ix++)
    {
      bufid[ix] = lastdigit;
    }
  Rps_Id idlast(bufid);
  int count = 0;
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  auto& curobuck = ob_idbucketmap_[idpref.bucket_num()];
  for (auto it = curobuck.lower_bound(idpref); it != curobuck.end(); it++)
    {
      Rps_Id curid = it->first;
      if (curid > idlast)
        break;
      count++;
      Rps_ObjectRef curobr = it->second;
      if (stopfun(curobr))
        break;
    }
  return count;
} // end Rps_ObjectZone::autocomplete_oid


////////////////////////////////////////////////////////////////
///// global roots for garbage collection and persistence

static std::set<Rps_ObjectRef> rps_object_root_set;
static std::mutex rps_object_root_mtx;
static std::unordered_map<Rps_Id,Rps_ObjectRef*,Rps_Id::Hasher> rps_object_global_root_hashtable;

void
rps_each_root_object (const std::function<void(Rps_ObjectRef)>&fun)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (auto ob: rps_object_root_set)
    fun(ob);
} // end rps_each_root_object


void
rps_add_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_root_set.insert(ob);
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      *(rootit->second) = ob;
  }
} // end rps_add_root_object


bool
rps_remove_root_object (const Rps_ObjectRef ob)
{
  if (!ob) return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  if (it == rps_object_root_set.end())
    return false;
  {
    auto rootit = rps_object_global_root_hashtable.find(ob->oid());
    if (RPS_UNLIKELY(rootit != rps_object_global_root_hashtable.end()))
      (*(rootit->second)) = Rps_ObjectRef(nullptr);
  }
  rps_object_root_set.erase(it);
  return true;
} // end rps_remove_root_object

void
rps_initialize_roots_after_loading (Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  rps_object_global_root_hashtable.max_load_factor(3.5);
  rps_object_global_root_hashtable.reserve(5*rps_hardcoded_number_of_roots()/4+3);
#define RPS_INSTALL_ROOT_OB(Oid) {		\
    const char*end##Oid = nullptr;		\
    bool ok##Oid = false;			\
    Rps_Id id##Oid(#Oid, &end##Oid, &ok##Oid);	\
    RPS_ASSERT (end##Oid && !*end##Oid);	\
    RPS_ASSERT (ok##Oid);			\
    RPS_ASSERT (id##Oid.valid());		\
    rps_object_global_root_hashtable[id##Oid]	\
      = &RPS_ROOT_OB(Oid);			\
  };
#include "generated/rps-roots.hh"
} // end of rps_initialize_roots_after_loading

bool rps_is_root_object (const Rps_ObjectRef ob)
{
  if (!ob)
    return false;
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  auto it = rps_object_root_set.find(ob);
  return it != rps_object_root_set.end();
} // end rps_is_root_object

std::set<Rps_ObjectRef>
rps_set_root_objects(void)
{
  std::set<Rps_ObjectRef> set;

  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  for (Rps_ObjectRef ob: rps_object_root_set)
    set.insert(ob);
  return set;
} // end rps_set_root_objects

unsigned
rps_nb_root_objects(void)
{
  std::lock_guard<std::mutex> gu(rps_object_root_mtx);
  return (unsigned) rps_object_root_set.size();
} // end rps_nb_root_objects



/***************** class info payload **********/

void
Rps_PayloadClassInfo::gc_mark(Rps_GarbageCollector&gc) const
{
  gc.mark_obj(pclass_super);
  gc.mark_obj(pclass_symbname);
  for (auto it: pclass_methdict)
    {
      gc.mark_obj(it.first);
      gc.mark_value(it.second, 1);
    }
} // end Rps_PayloadClassInfo::gc_mark

void
Rps_PayloadClassInfo::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  rps_dump_scan_object(du, pclass_super);
  rps_dump_scan_object(du, pclass_symbname);
  for (auto it : pclass_methdict)
    {
      rps_dump_scan_object(du, it.first);
      rps_dump_scan_value(du, it.second, 1);
    }
} // end Rps_PayloadClassInfo::dump_scan


void
Rps_PayloadClassInfo::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_class in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  if (pclass_symbname)
    {
      std::lock_guard<std::recursive_mutex> gu(*(pclass_symbname->objmtxptr()));
      auto symb = pclass_symbname->get_dynamic_payload<Rps_PayloadSymbol>();
      if (symb)
        {
          jv["class_name"] = Json::Value(symb->symbol_name());
          jv["class_symb"] = pclass_symbname.dump_json(du);
        }
    };
  jv["class_super"] = pclass_super.dump_json(du);
  auto jvvectmeth = Json::Value(Json::arrayValue);
  for (auto it : pclass_methdict)
    {
      auto jvcurmeth = Json::Value(Json::objectValue);
      jvcurmeth["methosel"] = rps_dump_json_objectref(du,it.first);
      jvcurmeth["methclos"] = rps_dump_json_value(du,it.second);
      jvvectmeth.append(jvcurmeth);
    }
  jv["class_methodict"] = jvvectmeth;
} // end Rps_PayloadClassInfo::dump_json_content

void
Rps_PayloadClassInfo::loader_put_symbname(Rps_ObjectRef obr, Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  pclass_symbname = obr;
} // end Rps_PayloadClassInfo::loader_put_symbname

void
Rps_PayloadClassInfo::put_symbname(Rps_ObjectRef obr)
{
  if (!obr)
    return;
  std::lock_guard<std::recursive_mutex> gu(*(obr->objmtxptr()));
  auto symb = obr->get_dynamic_payload<Rps_PayloadSymbol>();
  if (symb && symb->owner() == obr)
    {
      symb->symbol_put_value(owner());
      pclass_symbname = obr;
    }
} // end Rps_PayloadClassInfo::put_symbname


/***************** mutable set of objects payload **********/

void
Rps_PayloadSetOb::gc_mark(Rps_GarbageCollector&gc) const
{
  for (auto obr: psetob)
    gc.mark_obj(obr);
} // end Rps_PayloadSetOb::gc_mark

void
Rps_PayloadSetOb::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  for (auto obr: psetob)
    rps_dump_scan_object(du, obr);
} // end Rps_PayloadSetOb::dump_scan


void
Rps_PayloadSetOb::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_setob in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  Json::Value jarr(Json::arrayValue);
  for (auto obr: psetob)
    if (rps_is_dumpable_objref(du,obr))
      jarr.append(rps_dump_json_objectref(du,obr));
  jv["setob"] = jarr;
} // end Rps_PayloadSetOb::dump_json_content





/***************** mutable vector of objects payload **********/

void
Rps_PayloadVectOb::gc_mark(Rps_GarbageCollector&gc) const
{
  for (auto obr: pvectob)
    gc.mark_obj(obr);
} // end Rps_PayloadVectOb::gc_mark

void
Rps_PayloadVectOb::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  for (auto obr: pvectob)
    rps_dump_scan_object(du, obr);
} // end Rps_PayloadVectOb::dump_scan


void
Rps_PayloadVectOb::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_vectob in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  Json::Value jarr(Json::arrayValue);
  for (auto obr: pvectob)
    if (rps_is_dumpable_objref(du,obr))
      jarr.append(rps_dump_json_objectref(du,obr));
    else
      jarr.append(Json::Value(Json::nullValue));
  jv["vectob"] = jarr;
} // end Rps_PayloadVectOb::dump_json_content


/***************** space payload **********/

void
Rps_PayloadSpace::gc_mark(Rps_GarbageCollector&) const
{
} // end Rps_PayloadSpace::gc_mark

void
Rps_PayloadSpace::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_space in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
} // end Rps_PayloadSpace::dump_json_content


/***************** symbol payload **********/

std::recursive_mutex Rps_PayloadSymbol::symb_tablemtx;
std::map<std::string,Rps_PayloadSymbol*> Rps_PayloadSymbol::symb_table;
std::unordered_map<std::string,Rps_ObjectRef*> Rps_PayloadSymbol::symb_hardcoded_hashtable;

void
rps_initialize_symbols_after_loading(Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::recursive_mutex> gu(Rps_PayloadSymbol::symb_tablemtx);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.max_load_factor(2.5);
  Rps_PayloadSymbol::symb_hardcoded_hashtable.reserve(5*rps_hardcoded_number_of_symbols()/4+3);
#define RPS_INSTALL_NAMED_ROOT_OB(Oid,Name) {		\
    Rps_PayloadSymbol::symb_hardcoded_hashtable[#Name]	\
      = &RPS_SYMB_OB(Name);				\
  };
#include "generated/rps-names.hh"
} // end of rps_initialize_symbols_after_loading

bool
Rps_PayloadSymbol::valid_name(const char*str)
{
  if (!str || str==(const char*)RPS_EMPTYSLOT)
    return false;
  if (!isalpha(str[0]))
    return false;
  for (const char*pc = str+1; *pc; pc++)
    {
      if (isalnum(*pc))
        continue;
      else if (*pc == '$' || *pc == '_')
        {
          if (!isalnum(pc[-1]))
            return false;
        }
      else
        return false;
    };
  return true;
} // end Rps_PayloadSymbol::valid_name

Rps_PayloadSymbol::Rps_PayloadSymbol(Rps_ObjectZone*obz)
  : Rps_Payload(Rps_Type::PaylSymbol, obz),
    symb_name(), symb_data(nullptr), symb_is_weak(false)
{
} // end Rps_PayloadSymbol::Rps_PayloadSymbol

void
Rps_PayloadSymbol::load_register_name(const char*name, Rps_Loader*ld, bool weak)
{
  if (!valid_name(name))
    throw std::runtime_error(std::string("invalid symbol name:") + name);
  RPS_ASSERT(symb_name.empty());
  RPS_ASSERT(owner());
  RPS_ASSERT(ld != nullptr);
  std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
  symb_name.assign(name);
  if (RPS_UNLIKELY(symb_table.find(symb_name) != symb_table.end()))
    throw std::runtime_error(std::string("duplicate loaded symbol name:") + name + " for "
                             + owner()->oid().to_string());
  symb_table.insert({symb_name, this});
  symb_is_weak.store(weak);
  RPS_INFORMOUT("Rps_PayloadSymbol::load_register_name symb_name:" << symb_name
                << " " << (weak?"weak":"strong")
                << " owner:" << owner()->oid().to_string());
} // end Rps_PayloadSymbol::load_register_name

void
Rps_PayloadSymbol::gc_mark(Rps_GarbageCollector&gc) const
{
  auto symval = symbol_value();
  if (symval)
    gc.mark_value(symval);
} // end Rps_PayloadSymbol::gc_mark

void
Rps_PayloadSymbol::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  auto symval = symbol_value();
  if (symval)
    rps_dump_scan_value(du, symval, 0);
} // end Rps_PayloadSymbol::dump_scan

void
Rps_PayloadSymbol::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_symbol in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_ASSERT(owner());
  jv["symb_name"] = Json::Value(symb_name);
  Rps_Value symval = symbol_value();
  if (symval)
    jv["symb_val"] = rps_dump_json_value(du, symval);
  if (is_weak())
    jv["symb_weak"] = Json::Value(true);
  RPS_INFORMOUT("Rps_PayloadSymbol::dump_json_content owner=" << owner()->oid().to_string()
                << " jv=" << jv);
} // end Rps_PayloadSymbol::dump_json_content



void
Rps_PayloadSymbol::gc_mark_strong_symbols(Rps_GarbageCollector*gc)
{
  RPS_ASSERT(gc != nullptr);
  std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
  for (auto it: symb_table)
    {
      Rps_PayloadSymbol*cursymb = it.second;
      RPS_ASSERT(cursymb);
      if (cursymb->is_weak())
        continue;
      Rps_ObjectRef curown = cursymb->owner();
      if (!curown)
        continue;
      gc->mark_obj(curown);
    }
}

bool
Rps_PayloadSymbol::register_name(std::string name, Rps_ObjectRef obj, bool weak)
{
  if (!obj)
    return false;
  if (!valid_name(name))
    return false;
  std::lock_guard<std::recursive_mutex> gu(*(obj->objmtxptr()));
  if (obj->get_payload() != nullptr
      && !obj->has_erasable_payload()) return false;
  std::lock_guard<std::recursive_mutex> gusy(symb_tablemtx);
  if (symb_table.find(name) != symb_table.end())
    return false;
  Rps_PayloadSymbol* paylsymb =
    obj->put_new_plain_payload<Rps_PayloadSymbol>();
  paylsymb->symb_name = name;
  symb_table.insert({paylsymb->symb_name, paylsymb});
  paylsymb->symb_is_weak.store(weak);
  {
    auto symbit = symb_hardcoded_hashtable.find(name);
    if (RPS_UNLIKELY(symbit != symb_hardcoded_hashtable.end()))
      {
        *(symbit->second) = obj;
      }
  }
  RPS_INFORMOUT("Rps_PayloadSymbol::register_name name=" << name << " obj=" << obj->oid().to_string()
                << " " << (weak?"weak":"strong"));
  return true;
} // end Rps_PayloadSymbol::register_name



bool Rps_PayloadSymbol::forget_name(std::string name)
{
  if (!valid_name(name))
    return false;
  std::lock_guard<std::recursive_mutex> gusy(symb_tablemtx);
  auto it = symb_table.find(name);
  if (it == symb_table.end())
    return false;
  Rps_PayloadSymbol* sy = it->second;
  RPS_ASSERT(sy != nullptr);
  Rps_ObjectRef obj = sy->owner();
  if (!obj)
    return false;
  obj->clear_payload();
  symb_table.erase(it);
  {
    auto symbit = symb_hardcoded_hashtable.find(name);
    if (RPS_UNLIKELY(symbit != symb_hardcoded_hashtable.end()))
      {
        *(symbit->second) = Rps_ObjectRef(nullptr);
      }
  }
  return true;
} // end Rps_PayloadSymbol::forget_name

bool
Rps_PayloadSymbol::forget_object(Rps_ObjectRef obj)
{
  if (!obj)
    return false;
  std::lock_guard<std::recursive_mutex> gu(*(obj->objmtxptr()));
  Rps_PayloadSymbol* paylsymb = obj->get_dynamic_payload<Rps_PayloadSymbol>();
  if (!paylsymb)
    return false;
  std::lock_guard<std::recursive_mutex> gusy(symb_tablemtx);
  auto it = symb_table.find(paylsymb->symb_name);
  if (it == symb_table.end())
    return false;
  obj->clear_payload();
  symb_table.erase(it);
  return true;
} // end Rps_PayloadSymbol::forget_object

int
Rps_PayloadSymbol::autocomplete_name(const char*prefix, const std::function<bool(const Rps_ObjectZone*,const std::string&)>&stopfun)
{
  if (!valid_name(prefix))
    return 0;
  int count = 0;
  std::string prefixstr(prefix);
  int prefixlen = strlen(prefix);
  std::lock_guard<std::recursive_mutex> gusy(symb_tablemtx);
  for (auto it = symb_table.lower_bound(prefixstr); it != symb_table.end(); it++)
    {
      if (!it->second)
        continue;
      std::string curname = it->first;
      if (strncmp(prefix,curname.c_str(),prefixlen))
        break;
      count++;
      Rps_ObjectRef curobr = it->second->owner();
      if (stopfun(curobr,curname))
        break;
    }
  return count;
} // end Rps_PayloadSymbol::autocomplete_name

// end of file objects_rps.cc

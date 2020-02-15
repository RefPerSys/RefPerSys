/****************************************************************
 * file morevalues_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      Implementation related to more values (immutable instances, etc...)
 *      See also values_rps.cc file
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
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

extern "C" const char rps_morevalues_gitid[];
const char rps_morevalues_gitid[]= RPS_GITID;

extern "C" const char rps_morevalues_date[];
const char rps_morevalues_date[]= __DATE__;


/////////////////////////////////////////////////////// instances

const Rps_SetOb*
Rps_InstanceZone::class_attrset(Rps_ObjectRef obclass)
{
  if (!obclass || obclass.is_empty())
    return nullptr;
  std::lock_guard<std::recursive_mutex> gucla(*(obclass->objmtxptr()));
  auto paylcl = obclass->get_classinfo_payload();
  if (!paylcl)
    return nullptr;
  return paylcl->class_attrset();
} // end Rps_InstanceZone::class_attrset

void
Rps_InstanceZone::val_output(std::ostream& outs, unsigned depth) const
{
#warning unimplemented Rps_InstanceZone::val_output
  RPS_WARN("unimplemented Rps_InstanceZone::val_output");
} // end Rps_InstanceZone::val_output


Rps_ObjectRef
Rps_InstanceZone::compute_class(Rps_CallFrame*callerframe) const
{
  auto k_immutable_instance =
    RPS_ROOT_OB(_6ulDdOP2ZNr001cqVZ) /*=immutable_instance∈class*/;
  RPS_LOCALFRAME(k_immutable_instance,
                 callerframe,
                 Rps_ObjectRef obclass;
                );
  RPS_ASSERT(stored_type() == Rps_Type::Instance);
  _.obclass = get_class();
  RPS_ASSERT(_.obclass->get_classinfo_payload());
  return _.obclass;
} // end Rps_InstanceZone::compute_class



const Rps_SetOb*
Rps_InstanceZone::set_attributes(void) const
{
  Rps_ObjectRef obclass = get_class();
  auto setat = class_attrset(obclass);
  return setat;
} // end  Rps_InstanceZone::set_attributes


Rps_InstanceZone*
Rps_InstanceZone::make_from_attributes_components(Rps_ObjectRef classob,
    const std::vector<Rps_Value>& valvec,
    const std::map<Rps_ObjectRef,Rps_Value>& attrmap)
{
  Rps_InstanceZone*res = nullptr;
  RPS_ASSERT(classob);
  std::lock_guard<std::recursive_mutex> gucla(*(classob->objmtxptr()));
  auto clpayl = classob->get_classinfo_payload();
  if (!clpayl)
    throw RPS_RUNTIME_ERROR_OUT("Rps_InstanceZone::make_from_attributes_components with bad class:"
                                << classob);
  auto attrset = clpayl->attributes_set();
  if (!attrset)
    throw RPS_RUNTIME_ERROR_OUT("Rps_InstanceZone::make_from_attributes_components with class without attributes set:"
                                << classob);
  auto nbattrs = attrset->cardinal();
  auto nbcomps = valvec.size();
  auto physiz = 2*nbattrs+nbcomps; // physical allocated size
  if (RPS_UNLIKELY(physiz > maxsize)) // never happens in practice
    RPS_FATALOUT("Rps_InstanceZone::make_from_attributes_components too big, physical size="
                 << physiz << " for class " << classob);
  // every attribute in attrmap should be known to the class
  for (auto it : attrmap)
    {
      Rps_ObjectRef curat = it.first;
      RPS_ASSERT(curat);
      RPS_ASSERT(it.second);
      if (!attrset->contains(curat))
        throw RPS_RUNTIME_ERROR_OUT("Rps_InstanceZone::make_from_attributes_components class " << classob
                                    << " unexpected attribute " << curat);
    };
  res = rps_allocate_with_wordgap<Rps_InstanceZone,unsigned,Rps_ObjectRef,Rps_InstanceTag>
        ((physiz*sizeof(Rps_Value))/sizeof(void*),
         physiz, classob, Rps_InstanceTag{});
  Rps_Value*sonarr = res->raw_data_sons();
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
      Rps_Value curcomp = valvec[cix];
      sonarr[2*nbattrs+cix] = curcomp;
    }
  return res;
} // end Rps_InstanceZone::make_from_attributes_components




////////////////////////////////////////////////////// json

static void
rps_recursive_hash_json(const Json::Value&jv, std::uint64_t& h1,std::uint64_t& h2, unsigned depth)
{
  static constexpr unsigned maxrecurdepth=32;
  if (depth>maxrecurdepth) return;
  switch (jv.type())
    {
    case Json::nullValue:
      h1++;
      h2 -= depth;
      return;
    case Json::intValue:
    {
      auto i = jv.asInt64();
      h1 += ((31*depth) ^ (i % 200579));
      h2 ^= (i >> 48) - (i % 300593);
    }
    return;
    case Json::uintValue:
    {
      auto u = jv.asUInt64();
      h1 += ((17*depth) ^ (u % 200569));
      h2 ^= (u >> 42) + (u % 300557);
    }
    return;
    case Json::realValue:
    {
      double d = jv.asDouble();
      auto hd = std::hash<double> {}(d);
      h1 ^= 11 * (hd % 400597);
      h2 += hd;
    }
    return;
    case Json::stringValue:
    {
      const char* cs = jv.asCString();
      int64_t hs[2] = {0,0};
      int ln = rps_compute_cstr_two_64bits_hash(hs,cs);
      h1 ^= hs[0] + (31*ln);
      h2 -= hs[1] + (17*ln);
    }
    return;
    case Json::booleanValue:
    {
      if (jv.asBool())
        h1 += 23 + (h2 & 0xffff);
      else
        h2 += 13159 + (h1 & 0xffff);
    }
    return;
    case Json::arrayValue:
    {
      auto sz = jv.size();
      for (int ix=0; ix<(int)sz; ix++)
        {
          if (ix % 2)
            rps_recursive_hash_json(jv[ix], h1, h2, depth+1);
          else
            rps_recursive_hash_json(jv[ix], h2, h1, depth+1);
          h2 += ix;
          if (ix % 2 == 0)
            h1 ^= 13*ix;
          else
            h1 ^= ((h2+31*ix)&0xffff);
        }
    }
    return;
    case Json::objectValue:
    {
      auto mem = jv.getMemberNames();
      std::sort(mem.begin(), mem.end());
      int n = 0;
      for (const std::string& key: mem)
        {
          auto jmem=jv[key];
          int64_t hsk[2] = {0,0};
          int lnk = rps_compute_cstr_two_64bits_hash(hsk,key.c_str(),key.size());
          h1 ^= 11*hsk[0] + (317*lnk);
          h2 -= 7*hsk[1] + (331*lnk);
          if (n % 2)
            rps_recursive_hash_json(jmem, h1, h2, depth+1);
          else
            rps_recursive_hash_json(jmem, h2, h1, depth+1);
        }
    }
    return;
    default:
      RPS_FATALOUT("corrupted JSON type#" << jv.type() << " at depth " << depth);
    }
} // end of rps_recursive_hash_json

Rps_HashInt
Rps_JsonZone::compute_hash(void) const
{
  std::uint64_t h1=0, h2=0;
  rps_recursive_hash_json(_jsonval, h1, h2, 0);
  Rps_HashInt h = (h1 * 13151) ^ (h2 * 13291);
  if (RPS_UNLIKELY(h==0))
    h= (h1&0xffff) + (h2&0xfffff) + 17;
  return h;
} // end Rps_JsonZone::compute_hash


Rps_ObjectRef
Rps_JsonZone::compute_class(Rps_CallFrame*stkf __attribute__((unused))) const
{
  return RPS_ROOT_OB(_3GHJQW0IIqS01QY8qD); ////json∈class
} // end Rps_JsonZone::compute_class


Json::Value
Rps_JsonZone::dump_json(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  Json::Value jv(Json::objectValue);
  jv["vtype"] = "json";
  jv["json"] = _jsonval;
  return jv;
} // end Rps_JsonZone::dump_json


Rps_JsonZone*
Rps_JsonZone::make(const Json::Value& jv)
{
  return Rps_QuasiZone::rps_allocate<Rps_JsonZone,const Json::Value&>(jv);
} // end Rps_JsonZone::make

Rps_JsonZone*
Rps_JsonZone::load_from_json(Rps_Loader*ld, const Json::Value& jv)
{
  RPS_ASSERT(ld != nullptr);
  if (!jv.isObject() || !jv.isMember("json"))
    throw RPS_RUNTIME_ERROR_OUT("Rps_JsonZone::load_from_json bad jv=" << jv);
  return make(jv["json"]);
} // end Rps_JsonZone::load_from_json

void
Rps_JsonZone::val_output(std::ostream& outs, unsigned depth) const
{
  std::ostringstream tempouts;
  tempouts << _jsonval << std::endl;
  if (depth==0)
    outs << tempouts.str();
  else
    {
      auto srcstr = tempouts.str();
      const char*pc = nullptr;
      const char*eol = nullptr;
      for (pc = srcstr.c_str(); (eol=strchr(pc,'\n')); pc=eol+1)
        {
          std::string lin(pc, eol-pc);
          for (unsigned i=0; i<depth; i++) outs << ' ';
          outs << lin;
        }
    }
} // end Rps_JsonZone::val_output

bool
Rps_JsonZone::equal(const Rps_ZoneValue&zv) const
{
  if (zv.stored_type() == Rps_Type::Json)
    {
      auto othj = reinterpret_cast<const Rps_JsonZone*>(&zv);
      auto lh = lazy_hash();
      auto othlh = othj->lazy_hash();
      if (lh != 0 && othlh != 0 && lh != othlh) return false;
      return _jsonval == othj->_jsonval;
    }
  else return false;
} // end Rps_JsonZone::equal


bool
Rps_JsonZone::less(const Rps_ZoneValue&zv) const
{
  if (zv.stored_type() == Rps_Type::Json)
    {
      auto othj = reinterpret_cast<const Rps_JsonZone*>(&zv);
      return _jsonval < othj->_jsonval;
    }
  else return  Rps_Type::Json < zv.stored_type() ;
} // end Rps_JsonZone::less



////////////////////////////////////////////////////////////////
std::atomic<unsigned> Rps_QtPtrZone::qtptr_count;
// see gitlab.com/bstarynk/refpersys/-/wikis/adding-new-value-types-in-RefPerSys
Rps_HashInt
Rps_QtPtrZone::compute_hash(void) const
{
  auto rk = _qptr_rank;
  Rps_HashInt h = (31*(rk%173)) ^ (rk*2161);
  if (RPS_UNLIKELY(h==0))
    h = (rk&0xff) + 3;
  return h;
} // end Rps_QtPtrZone::compute_hash

Rps_ObjectRef
Rps_QtPtrZone::compute_class(Rps_CallFrame* stkf) const
{
  (void) stkf;
  return RPS_ROOT_OB(_3eg155drDR100uqE6R); // qtptr class
}


bool
Rps_QtPtrZone::less(const Rps_ZoneValue& zv) const
{
  if (zv.stored_type() == Rps_Type::QtPtr)
    {
      auto other = reinterpret_cast<const Rps_QtPtrZone*>(&zv);
      return _qptr_rank < other->_qptr_rank;
    }
  else
    return Rps_Type::QtPtr < zv.stored_type();
} // end Rps_QtPtrZone::less


bool
Rps_QtPtrZone::equal(const Rps_ZoneValue& zv) const
{
  return this == &zv;
} // end Rps_QtPtrZone::equal



void
Rps_QtPtrZone::val_output(std::ostream& outs, unsigned) const
{
  // the depth is not useful
  outs << "QtPtr#" <<  _qptr_rank;
} // end Rps_QtPtrZone::val_output


Rps_QtPtrZone*
Rps_QtPtrZone::make(const QPointer<QObject> qptr)
{
  return Rps_QuasiZone::rps_allocate1<Rps_QtPtrZone, const QPointer<QObject>>(qptr);
} // end Rps_QtPtrZone::make




/********************************************** end of file morevalues_rps.cc */

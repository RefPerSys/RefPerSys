/****************************************************************
 * file objects_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
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
 ******************************************************************************/


#include "refpersys.hh"

extern "C" const char rps_objects_gitid[];
const char rps_objects_gitid[]= RPS_GITID;

extern "C" const char rps_objects_date[];
const char rps_objects_date[]= __DATE__;

extern "C" const char rps_objects_shortgitid[];
const char rps_objects_shortgitid[]= RPS_SHORTGITID;


std::unordered_map<Rps_Id,Rps_ObjectZone*,Rps_Id::Hasher> Rps_ObjectZone::ob_idmap_(50777);

std::map<Rps_Id,Rps_ObjectZone*> Rps_ObjectZone::ob_idbucketmap_[Rps_Id::maxbuckets];
std::recursive_mutex Rps_ObjectZone::ob_idmtx_;



// Build an object from its existing string oid, or else fail with C++ exception
Rps_ObjectRef::Rps_ObjectRef(Rps_CallFrame*callerframe, const char*oidstr, Rps_ObjIdStrTag)
{
  if (!oidstr)
    throw RPS_RUNTIME_ERROR_OUT("Rps_ObjectRef: null oidstr");
  RPS_DEBUG_LOG(LOWREP, "Rps_ObjectRef oidstr=" << oidstr
                << " from " << Rps_ShowCallFrame(callerframe));
  const char*end = nullptr;
  bool ok=false;
  Rps_Id oid(oidstr,&end,&ok);
  if (!end || *end)
    throw RPS_RUNTIME_ERROR_OUT("Rps_ObjectRef: invalid compile-time oidstr=" << oidstr);
  *this = find_object_or_fail_by_oid(callerframe, oid);
} // end Rps_ObjectRef::Rps_ObjectRef(Rps_CallFrame*, constexpr const char*oidstr, Rps_ObjIdStrTag)


/// Static member function to compare two object references for display to humans
/// so if both have names, use them....
int
Rps_ObjectRef::compare_for_display(const Rps_ObjectRef leftob,
                                   const Rps_ObjectRef rightob)
{
  if (leftob.optr() == rightob.optr())
    return 0;
  if (leftob.is_empty())
    {
      if (rightob.is_empty())
        return 0;
      return -1;
    };
  if (rightob.is_empty())
    {
      if (leftob.is_empty())
        return 0;
      return 1;
    };
  Rps_Id leftid = leftob->oid();
  Rps_Id rightid = rightob->oid();
  RPS_ASSERT (leftid != rightid);
  /// these strings will hold a (non-empty) name if one is found.
  std::string sleftname;
  std::string srightname;
  {
    std::lock_guard<std::recursive_mutex> guleft(*leftob->objmtxptr());
    Rps_Value leftvalname =
      leftob->get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); // /name∈named_attribute
    if (leftvalname.is_string())
      sleftname = leftvalname.as_string()->cppstring();
  }
  {
    std::lock_guard<std::recursive_mutex> guright(*rightob->objmtxptr());
    Rps_Value rightvalname =
      rightob->get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); // /name∈named_attribute
    if (rightvalname.is_string())
      srightname = rightvalname.as_string()->cppstring();
  }
  if (!sleftname.empty() && !srightname.empty())
    {
      if (sleftname==srightname)
        {
          if (leftid < rightid)
            return -1;
          else
            return +1;
        }
      else
        {
          if (sleftname < srightname)
            return -1;
          else
            return +1;
        }
    };
  if (!sleftname.empty())
    return -1;
  else if (!srightname.empty())
    return +1;
  if (leftid < rightid)
    return -1;
  else
    return +1;
} // end of Rps_ObjectRef::compare_for_display



/// Output a reference for human display
void
Rps_ObjectRef::output(std::ostream&outs, unsigned depth, unsigned maxdepth) const
{
  //// See also class Rps_Object_Display in cmdrepl_rps.cc which uses
  //// this and the RPS_OBJECT_DISPLAY and RPS_OBJECT_DISPLAY_DEPTH
  //// macros of refpersys.hh.
  ////
  if (is_empty())
    outs << "__";
  else if (depth>maxdepth)
    outs << "??";
  else
    {
      std::lock_guard<std::recursive_mutex> gu(*_optr->objmtxptr());
      Rps_Value valname = obptr()->get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name
      outs << "◌" /*U+25CC DOTTED CIRCLE*/
           << obptr()-> oid().to_string();
      if (depth <= 1)
        outs << std::flush;
      if (valname.is_string())
        {
          outs << "/" << valname.as_cstring();
          if (depth <= 1)
            outs << std::flush;
        }
      else if (auto symbpayl = obptr()-> get_dynamic_payload<Rps_PayloadSymbol>())
        {
          outs << "!sy°" << symbpayl->symbol_name();
          if (depth <= 1)
            outs << std::flush;
        }
      else if (auto classpayl =  obptr()-> get_dynamic_payload<Rps_PayloadClassInfo>())
        {
          outs << "!cla°" << classpayl->class_name_str();
          if (depth <= 1)
            outs << std::flush;
        };
      if (depth <= 2)
        {
          Rps_ObjectRef obclass = obptr()-> get_class();
          if (obclass)
            {
              std::lock_guard<std::recursive_mutex> gucl(*obclass->objmtxptr());
              auto obclpayl = obclass->get_dynamic_payload<Rps_PayloadClassInfo>();
              if (obclpayl)
                {
                  outs << "∊" //U+220A SMALL ELEMENT OF
                       << obclpayl->class_name_str()
                       << std::flush;
                }
            };
          outs << std::flush;
        }
    };
} // end Rps_ObjectRef::output


const std::string
Rps_ObjectRef::as_string(void) const
{
  if (_optr == nullptr)
    return std::string{"__"};
  else if (_optr == RPS_EMPTYSLOT)
    return std::string{"_⁂_"}; //U+2042 ASTERISM
  const Rps_Id curoid = _optr->oid();
  std::lock_guard<std::recursive_mutex> gu(*_optr->objmtxptr());
  if (const Rps_PayloadSymbol*symbpayl
      = _optr->get_dynamic_payload<Rps_PayloadSymbol>())
    {
      const std::string& syna = symbpayl->symbol_name();
      if (!syna.empty())
        return std::string{"¤" /*U+00A4 CURRENCY SIGN*/} + syna;
    }
  else if (const Rps_PayloadClassInfo*classpayl
           =  _optr->get_dynamic_payload<Rps_PayloadClassInfo>())
    {
      const std::string clana = classpayl->class_name_str();
      if (!clana.empty())
        return std::string{"∋" /*U+220B CONTAINS AS MEMBER*/} + clana;
    }
  /** Up to commit 44bdcf1b86b983 dated Jul 16, 2023 we did
   ** incorrectly used the _4FBkYDlynyC02QtkfG "name"∈named_attribute
   ** instead of the _1EBVGSfW2m200z18rx name∈named_attribute **/
  else if (const Rps_Value namval
           = _optr->get_physical_attr(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)) /*name∈named_attribute*/)
    {
      if (namval.is_string())
        {
          std::string str {"⁑" /*U+2051 TWO ASTERISKS ALIGNED VERTICALLY*/};
          str.append  (namval.as_cppstring());
          str.append (":");
          str.append (curoid.to_string());
          return str;
        }
    }
  return curoid.to_string();
} // end Rps_ObjectRef::as_string



const std::string
Rps_ObjectZone::payload_type_name(void) const
{
  Rps_Payload* payl = get_payload();
  if (!payl)
    return "*no-payload*";
  if ((void*)payl == RPS_EMPTYSLOT)
    return "*empty-payload*";
  return payl-> payload_type_name();
} // end Rps_ObjectZone::payload_type_name

void
Rps_ObjectZone::val_output(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  if (depth > maxdepth)
    {
      out << "??";
      return;
    };
  out << oid().to_string();
  if (depth<2)
    {
      std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
      out << "⟦"; // U+27E6 MATHEMATICAL LEFT WHITE SQUARE BRACKET
      auto namit = ob_attrs.find(RPS_ROOT_OB(_1EBVGSfW2m200z18rx)); //name∈named_attribute);
      if (namit != ob_attrs.end())
        {
          Rps_Value namv = namit->second;
          if (namv.is_string())
            {
              out << "⏵"; // U+23F5 BLACK MEDIUM RIGHT-POINTING TRIANGLE
              out << namv.as_cstring();
            }
        }
      auto obcl = ob_class.load();
      if (obcl)
        {
          auto obclpayl = obcl->get_dynamic_payload<Rps_PayloadClassInfo>();
          if (obclpayl)
            {
              out << "∈"; //U+2208 ELEMENT OF
              out << obclpayl->class_name_str();
            }
        };
      out << "⟧"; // U+27E7 MATHEMATICAL RIGHT WHITE SQUARE BRACKET
    };
} // end Rps_ObjectZone::val_output


void
Rps_ObjectZone::register_objzone(Rps_ObjectZone*obz)
{
  RPS_ASSERT(obz != nullptr);
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  auto oid = obz->oid();
  RPS_DEBUG_LOG(LOWREP, "register_objzone obz=" << obz << " oid=" << oid
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "register_objzone"));
  if (ob_idmap_.find(oid) != ob_idmap_.end())
    RPS_FATALOUT("Rps_ObjectZone::register_objzone duplicate oid " << oid);
  ob_idmap_.insert({oid,obz});
  ob_idbucketmap_[oid.bucket_num()].insert({oid,obz});
} // end Rps_ObjectZone::register_objzone

Rps_Id
Rps_ObjectZone::fresh_random_oid(Rps_ObjectZone*obz)
{
  Rps_Id oid;
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  while(true)
    {
      oid = Rps_Id::random();
      if (RPS_UNLIKELY(ob_idmap_.find(oid) != ob_idmap_.end()))
        continue;
      if (obz)
        ob_idmap_.insert({oid,obz});
      RPS_DEBUG_LOG(LOWREP, "Rps_ObjectZone::fresh_random_oid obz=" << obz
                    << " -> oid=" << oid);
      return oid;
    }
}


Rps_ObjectZone::Rps_ObjectZone(Rps_Id oid, registermode_en regmod)
  : Rps_ZoneValue(Rps_Type::Object),
    ob_oid(oid), ob_mtx(), ob_class(nullptr),
    ob_space(nullptr), ob_mtime(0.0),
    ob_attrs(), ob_comps(), ob_payload(nullptr),
    ob_magicgetterfun(nullptr),
    ob_applyingfun(nullptr)
{
  RPS_DEBUG_LOG(LOWREP, "Rps_ObjectZone oid=" << oid << ' '
                << (regmod==OBZ_DONT_REGISTER?"non-":"") << "registering"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(2, "Rps_ObjectZone")
                << std::endl);
  if (regmod == OBZ_REGISTER)
    {
      register_objzone(this);
    }
  // In principle, the below initialization of ob_class should be
  // useless, because it should be done elsewhere. In practice, we
  // need it and prefer to initialize ob_class several times instead
  // of none.
  ///////
  // Every object should have a class, initially `object`; the
  // ob_class can later be replaced, but we need something which is
  // not null.... That atomic field could be later overwritten.
  ob_class.store(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)); //object∈class
} // end Rps_ObjectZone::Rps_ObjectZone


Rps_ObjectZone::~Rps_ObjectZone()
{
  //  RPS_INFORMOUT("destroying object " << oid());
  Rps_Id curid = oid();
  RPS_POSSIBLE_BREAKPOINT();
  clear_payload();
  ob_attrs.clear();
  ob_comps.clear();
  ob_class.store(nullptr);
  ob_mtime.store(0.0);
  std::lock_guard<std::recursive_mutex> gu(ob_idmtx_);
  RPS_DEBUG_LOG(LOWREP,"~Rps_ObjectZone curid=" << curid << " this=" << this);
  ob_idmap_.erase(curid);
  ob_idbucketmap_[curid.bucket_num()].erase(curid);
} // end Rps_ObjectZone::~Rps_ObjectZone()



Rps_ObjectZone::Rps_ObjectZone() :
  Rps_ObjectZone::Rps_ObjectZone(fresh_random_oid(this),
                                 Rps_ObjectZone::OBZ_DONT_REGISTER)
{
  RPS_DEBUG_LOG(LOWREP, "Rps_ObjectZone this=" << this
                << " oid=" << oid());
} // end Rps_ObjectZone::Rps_ObjectZone


void
Rps_ObjectZone::put_applying_function(rps_applyingfun_t*afun)
{
  auto oldappfun = ob_applyingfun.exchange(afun);
  if (oldappfun)
    {
      RPS_WARNOUT("Rps_ObjectZone::put_applying_function for oid=" << oid()
                  << " did overwrite applying function to " << afun
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "put_applying_function"));
    };
} // end Rps_ObjectZone::put_applying_function

Rps_ObjectZone*
Rps_ObjectZone::make(void)
{
  Rps_Id oid = fresh_random_oid(nullptr);
  RPS_DEBUG_LOG(LOWREP, "Rps_ObjectZone::make start oid=" << oid
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_ObjectZone::make start"));
  Rps_ObjectZone*obz= Rps_QuasiZone::rps_allocate<Rps_ObjectZone,Rps_Id,registermode_en>(oid,OBZ_REGISTER);
  *(const_cast<Rps_Id*>(&obz->ob_oid)) = oid;
  double rtime = rps_wallclock_real_time();
  obz->ob_mtime.store(rtime);
  RPS_DEBUG_LOG(LOWREP, "Rps_ObjectZone::make oid=" << oid
                << " obz=" << obz << " mtime=" << rtime);
  // Every object should have a class, initially `object`; the
  // ob_class can later be replaced, but we need something which is
  // not null.... That atomic field could be later overwritten.
  obz->ob_class.store(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)); //object∈class
  RPS_DEBUG_LOG(LOWREP, "Rps_ObjectZone::make oid=" << oid << " obz=" << obz
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_ObjectZone::make"));
  return obz;
} // end Rps_ObjectZone::make


Rps_ObjectZone*
Rps_ObjectZone::make_loaded(Rps_Id oid, Rps_Loader* ld)
{
#warning Rps_ObjectZone::make_loaded might be incomplete
  RPS_DEBUG_LOG(LOAD, "make_loaded oid="<< oid);
  RPS_ASSERT(oid.valid());
  RPS_ASSERT(ld != nullptr);
  Rps_ObjectZone*obz= Rps_QuasiZone::rps_allocate<Rps_ObjectZone,Rps_Id,registermode_en>(oid, OBZ_REGISTER);
  // Every object should have a class, initially `object`; the
  // ob_class can later be replaced, but we need something which is
  // not null.... The loader could later overwrite that.
  obz->ob_class.store(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)); //object∈class
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
Rps_ObjectZone::put_space(Rps_ObjectRef obr)
{
  if (obr)
    {
      if (obr->get_class() != RPS_ROOT_OB(_2i66FFjmS7n03HNNBx))
        throw std::runtime_error("invalid space object");
    };
  ob_space.store(obr);
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::put_space



void
Rps_ObjectZone::remove_attr(const Rps_ObjectRef obattr)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr.is_empty() || obattr->stored_type() != Rps_Type::Object)
    return;
  rps_magicgetterfun_t*getfun = obattr->ob_magicgetterfun.load();
  {
    if (RPS_UNLIKELY(getfun))
      throw RPS_RUNTIME_ERROR_OUT("cannot remove magic attribute " << obattr
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  ob_attrs.erase(obattr);
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::remove_attr


Rps_Value
Rps_ObjectZone::set_of_physical_attributes(void) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  unsigned nbat = ob_attrs.size();
  std::vector<Rps_ObjectRef> vecat;
  vecat.reserve(nbat);
  for (auto it : ob_attrs)
    vecat.push_back(it.first);
  return Rps_SetValue(vecat);
} // end of Rps_ObjectZone::set_of_physical_attributes



Rps_Value
Rps_ObjectZone::set_of_attributes([[maybe_unused]] Rps_CallFrame*stkf) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  RPS_ASSERT(!stkf || stkf->is_good_call_frame());
  return set_of_physical_attributes();
} // end of Rps_ObjectZone::set_of_attributes

unsigned
Rps_ObjectZone::nb_physical_attributes(void) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  return ob_attrs.size();
} // end Rps_ObjectZone::nb_physical_attributes

unsigned
Rps_ObjectZone::nb_attributes([[maybe_unused]] Rps_CallFrame*stkf) const
{
  RPS_ASSERT(!stkf || stkf->is_good_call_frame());
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  return ob_attrs.size();
} // end Rps_ObjectZone::nb_attributes

Rps_Value
Rps_ObjectZone::get_attr1(Rps_CallFrame*stkf,const Rps_ObjectRef obattr0) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return nullptr;
  Rps_Value val0;
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      val0 = (*getfun0)(stkf, *this, obattr0);
    else
      {
        auto it0 = ob_attrs.find(obattr0);
        if (it0 != ob_attrs.end())
          val0 = it0->second;
      }
  }
  return val0;
} // end Rps_ObjectZone::get_attr1


Rps_Value
Rps_ObjectZone::get_physical_attr(const Rps_ObjectRef obattr0) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return nullptr;
  Rps_Value val0;
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  auto it0 = ob_attrs.find(obattr0);
  if (it0 != ob_attrs.end())
    val0 = it0->second;
  return val0;
} // end Rps_ObjectZone::get_physical_attr



Rps_TwoValues
Rps_ObjectZone::get_attr2(Rps_CallFrame*stkf, const Rps_ObjectRef obattr0, const Rps_ObjectRef obattr1) const
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return Rps_TwoValues(nullptr,nullptr);
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return Rps_TwoValues(nullptr,nullptr);
  Rps_Value val0;
  Rps_Value val1;
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      val0 = (*getfun0)(stkf, *this, obattr0);
    else
      {
        auto it0 = ob_attrs.find(obattr0);
        if (it0 != ob_attrs.end())
          val0 = it0->second;
      }
  }
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      val0 = (*getfun1)(stkf, *this, obattr1);
    else
      {
        auto it1 = ob_attrs.find(obattr1);
        if (it1 != ob_attrs.end())
          val1 = it1->second;
      }
  }
  return Rps_TwoValues(val0, val1);
} // end Rps_ObjectZone::get_attr2


void
Rps_ObjectZone::put_attr(const Rps_ObjectRef obattr, const Rps_Value valattr)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr.is_empty() || obattr->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun = obattr->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
#warning debug stuff in ObjectZone::put_attr is temporary in end of jan 2025
  RPS_POSSIBLE_BREAKPOINT();
  RPS_DEBUG_LOG(REPL, "Rps_ObjectZone::put_attr/start *this="
                <<  Rps_ObjectRef(this) << std::endl
                << " obattr=" << obattr << " valattr=" << valattr
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_ObjectZone::put_attr")
                << RPS_OBJECT_DISPLAY(this));
  RPS_POSSIBLE_BREAKPOINT();
  if (valattr.is_empty())
    ob_attrs.erase(obattr);
  else
    ob_attrs.insert_or_assign(obattr, valattr);
  ob_mtime.store(rps_wallclock_real_time());
  RPS_DEBUG_LOG(REPL, "Rps_ObjectZone::put_attr/end"
                << RPS_OBJECT_DISPLAY(this));
} // end Rps_ObjectZone::put_attr


void
Rps_ObjectZone::put_attr2(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                          const Rps_ObjectRef obattr1, const Rps_Value valattr1)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr0
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr1
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  if (valattr0.is_empty())
    ob_attrs.erase(obattr0);
  else
    ob_attrs.insert_or_assign(obattr0, valattr0);
  if (valattr1.is_empty())
    ob_attrs.erase(obattr1);
  else
    ob_attrs.insert_or_assign(obattr1, valattr1);
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::put_attr2

void
Rps_ObjectZone::put_attr3(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                          const Rps_ObjectRef obattr1, const Rps_Value valattr1,
                          const Rps_ObjectRef obattr2, const Rps_Value valattr2)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr0
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr1
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr2.is_empty() || obattr2->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun2 = obattr2->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun2))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr2
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  if (valattr0.is_empty())
    ob_attrs.erase(obattr0);
  else
    ob_attrs.insert_or_assign(obattr0, valattr0);
  if (valattr1.is_empty())
    ob_attrs.erase(obattr1);
  else
    ob_attrs.insert_or_assign(obattr1, valattr1);
  if (valattr2.is_empty())
    ob_attrs.erase(obattr2);
  else
    ob_attrs.insert_or_assign(obattr2, valattr2);
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::put_attr3


void
Rps_ObjectZone::put_attr4(const Rps_ObjectRef obattr0, const Rps_Value valattr0,
                          const Rps_ObjectRef obattr1, const Rps_Value valattr1,
                          const Rps_ObjectRef obattr2, const Rps_Value valattr2,
                          const Rps_ObjectRef obattr3, const Rps_Value valattr3)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr0
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr1
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr2.is_empty() || obattr2->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun2 = obattr2->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun2))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr2
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr3.is_empty() || obattr3->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun3 = obattr3->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun3))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr3
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  if (valattr0.is_empty())
    ob_attrs.erase(obattr0);
  else
    ob_attrs.insert_or_assign(obattr0, valattr0);
  if (valattr1.is_empty())
    ob_attrs.erase(obattr1);
  else
    ob_attrs.insert_or_assign(obattr1, valattr1);
  if (valattr2.is_empty())
    ob_attrs.erase(obattr2);
  else
    ob_attrs.insert_or_assign(obattr2, valattr2);
  if (valattr3.is_empty())
    ob_attrs.erase(obattr3);
  else
    ob_attrs.insert_or_assign(obattr3, valattr3);
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::put_attr4


void
Rps_ObjectZone::exchange_attr(const Rps_ObjectRef obattr, const Rps_Value valattr, Rps_Value*poldval)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr.is_empty() || obattr->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun = obattr->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  Rps_Value oldval;
  if (poldval)
    {
      auto it = ob_attrs.find(obattr);
      if (it != ob_attrs.end())
        oldval = it->second;
    }
  if (valattr.is_empty())
    ob_attrs.erase(obattr);
  else
    ob_attrs.insert_or_assign(obattr, valattr);
  if (poldval)
    *poldval = oldval;
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::exchange_attr


void
Rps_ObjectZone::exchange_attr2(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                               const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr0
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr1
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  Rps_Value oldval0;
  Rps_Value oldval1;
  if (poldval0)
    {
      auto it = ob_attrs.find(obattr0);
      if (it != ob_attrs.end())
        oldval0 = it->second;
    }
  if (poldval1)
    {
      auto it = ob_attrs.find(obattr1);
      if (it != ob_attrs.end())
        oldval1 = it->second;
    }
  if (valattr0.is_empty())
    ob_attrs.erase(obattr0);
  else
    ob_attrs.insert_or_assign(obattr0, valattr0);
  if (valattr1.is_empty())
    ob_attrs.erase(obattr1);
  else
    ob_attrs.insert_or_assign(obattr1, valattr1);
  if (poldval0)
    *poldval0 = oldval0;
  if (poldval1)
    *poldval1 = oldval1;
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::exchange_attr2

void
Rps_ObjectZone::exchange_attr3(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                               const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1,
                               const Rps_ObjectRef obattr2, const Rps_Value valattr2, Rps_Value*poldval2)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr0
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr1
                                  << " in " << Rps_ObjectRef(this));
  }
  if (obattr2.is_empty() || obattr2->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun2 = obattr2->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun2))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr2
                                  << " in " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  Rps_Value oldval0;
  Rps_Value oldval1;
  Rps_Value oldval2;
  if (poldval0)
    {
      auto it = ob_attrs.find(obattr0);
      if (it != ob_attrs.end())
        oldval0 = it->second;
    }
  if (poldval1)
    {
      auto it = ob_attrs.find(obattr1);
      if (it != ob_attrs.end())
        oldval1 = it->second;
    }
  if (poldval2)
    {
      auto it = ob_attrs.find(obattr2);
      if (it != ob_attrs.end())
        oldval2 = it->second;
    }
  if (valattr0.is_empty())
    ob_attrs.erase(obattr0);
  else
    ob_attrs.insert_or_assign(obattr0, valattr0);
  if (valattr1.is_empty())
    ob_attrs.erase(obattr1);
  else
    ob_attrs.insert_or_assign(obattr1, valattr1);
  if (valattr2.is_empty())
    ob_attrs.erase(obattr2);
  else
    ob_attrs.insert_or_assign(obattr2, valattr2);
  if (poldval0)
    *poldval0 = oldval0;
  if (poldval1)
    *poldval1 = oldval1;
  if (poldval2)
    *poldval1 = oldval2;
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::exchange_attr3


void
Rps_ObjectZone::exchange_attr4(const Rps_ObjectRef obattr0, const Rps_Value valattr0, Rps_Value*poldval0,
                               const Rps_ObjectRef obattr1, const Rps_Value valattr1, Rps_Value*poldval1,
                               const Rps_ObjectRef obattr2, const Rps_Value valattr2, Rps_Value*poldval2,
                               const Rps_ObjectRef obattr3, const Rps_Value valattr3, Rps_Value*poldval3)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (obattr0.is_empty() || obattr0->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun0 = obattr0->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun0))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr0
                                  << " from " << Rps_ObjectRef(this));
  }
  if (obattr1.is_empty() || obattr1->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun1 = obattr1->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun1))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr1
                                  << " from " << Rps_ObjectRef(this));
  }
  if (obattr2.is_empty() || obattr2->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun2 = obattr2->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun2))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr2
                                  << " from " << Rps_ObjectRef(this));
  }
  if (obattr3.is_empty() || obattr3->stored_type() != Rps_Type::Object)
    return;
  {
    rps_magicgetterfun_t*getfun3 = obattr3->ob_magicgetterfun.load();
    if (RPS_UNLIKELY(getfun3))
      throw RPS_RUNTIME_ERROR_OUT("cannot put magic attribute " << obattr3
                                  << " from " << Rps_ObjectRef(this));
  }
  std::lock_guard gu(ob_mtx);
  Rps_Value oldval0;
  Rps_Value oldval1;
  Rps_Value oldval2;
  Rps_Value oldval3;
  if (poldval0)
    {
      auto it = ob_attrs.find(obattr0);
      if (it != ob_attrs.end())
        oldval0 = it->second;
    }
  if (poldval1)
    {
      auto it = ob_attrs.find(obattr1);
      if (it != ob_attrs.end())
        oldval1 = it->second;
    }
  if (poldval2)
    {
      auto it = ob_attrs.find(obattr2);
      if (it != ob_attrs.end())
        oldval2 = it->second;
    }
  if (poldval3)
    {
      auto it = ob_attrs.find(obattr3);
      if (it != ob_attrs.end())
        oldval3 = it->second;
    }
  if (valattr0.is_empty())
    ob_attrs.erase(obattr0);
  else
    ob_attrs.insert_or_assign(obattr0, valattr0);
  if (valattr1.is_empty())
    ob_attrs.erase(obattr1);
  else
    ob_attrs.insert_or_assign(obattr1, valattr1);
  if (valattr2.is_empty())
    ob_attrs.erase(obattr2);
  else
    ob_attrs.insert_or_assign(obattr2, valattr2);
  if (valattr3.is_empty())
    ob_attrs.erase(obattr3);
  else
    ob_attrs.insert_or_assign(obattr3, valattr3);
  if (poldval0)
    *poldval0 = oldval0;
  if (poldval1)
    *poldval1 = oldval1;
  if (poldval2)
    *poldval1 = oldval2;
  if (poldval3)
    *poldval1 = oldval3;
  ob_mtime.store(rps_wallclock_real_time());
} // end Rps_ObjectZone::exchange_attr4



//////////////// components
unsigned
Rps_ObjectZone::nb_physical_components(void) const
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  return ob_comps.size();
} // end Rps_ObjectZone::nb_physical_components

const std::vector<Rps_Value>
Rps_ObjectZone::vector_physical_components(void) const
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  return ob_comps;
} // end Rps_ObjectZone::vector_physical_components

unsigned
Rps_ObjectZone::nb_components([[maybe_unused]] Rps_CallFrame*stkf) const
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  unsigned nbcomp = ob_comps.size();
  return nbcomp;
} // end Rps_ObjectZone::nb_components

Rps_Value
Rps_ObjectZone::component_at ([[maybe_unused]] Rps_CallFrame*stkf, int rk, bool dontfail) const
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  unsigned nbcomp = ob_comps.size();
  if (rk<0) rk += nbcomp;
  if (rk>=0 && rk<(int)nbcomp)
    return ob_comps[rk];
  if (dontfail)
    return nullptr;
  throw std::range_error("Rps_ObjectZone::component_at index out of range");
} // end Rps_ObjectZone::component_at

Rps_Value
Rps_ObjectZone::replace_component_at ([[maybe_unused]] Rps_CallFrame*stkf, int rk,  Rps_Value comp0, bool dontfail)
{
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  unsigned nbcomp = ob_comps.size();
  if (rk<0) rk += nbcomp;
  if (rk>=0 && rk<(int)nbcomp)
    {
      Rps_Value oldv =  ob_comps[rk];
      ob_comps[rk] = comp0;
      touch_now();
      return oldv;
    }
  if (dontfail)
    throw std::range_error("Rps_ObjectZone::component_at index out of range");
  return nullptr;
} // end Rps_ObjectZone::replace_component_at

void
Rps_ObjectZone::append_comp1(Rps_Value comp0)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (RPS_UNLIKELY(comp0.is_empty()))
    comp0.clear();
  std::lock_guard gu(ob_mtx);
  ob_comps.push_back(comp0);
} // end Rps_ObjectZone::append_comp1



void
Rps_ObjectZone::append_comp2(Rps_Value comp0, Rps_Value comp1)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (RPS_UNLIKELY(comp0.is_empty()))
    comp0.clear();
  if (RPS_UNLIKELY(comp1.is_empty()))
    comp1.clear();
  std::lock_guard gu(ob_mtx);
  // we want to avoid too frequent resizes, so....
  if (RPS_UNLIKELY(ob_comps.capacity() < ob_comps.size() + 2))
    {
      auto newsiz = rps_prime_above(9*ob_comps.size()/8 + 2);
      ob_comps.reserve(newsiz);
    };
  ob_comps.push_back(comp0);
  ob_comps.push_back(comp1);
} // end Rps_ObjectZone::append_comp2



void
Rps_ObjectZone::append_comp3(Rps_Value comp0, Rps_Value comp1, Rps_Value comp2)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (RPS_UNLIKELY(comp0.is_empty()))
    comp0.clear();
  if (RPS_UNLIKELY(comp1.is_empty()))
    comp1.clear();
  if (RPS_UNLIKELY(comp2.is_empty()))
    comp2.clear();
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  // we want to avoid too frequent resizes, so....
  if (RPS_UNLIKELY(ob_comps.capacity() < ob_comps.size() + 3))
    {
      auto newsiz = rps_prime_above(9*ob_comps.size()/8 + 3);
      ob_comps.reserve(newsiz);
    };
  ob_comps.push_back(comp0);
  ob_comps.push_back(comp1);
  ob_comps.push_back(comp2);
} // end Rps_ObjectZone::append_comp3

void
Rps_ObjectZone::append_comp4(Rps_Value comp0, Rps_Value comp1, Rps_Value comp2, Rps_Value comp3)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  if (RPS_UNLIKELY(comp0.is_empty()))
    comp0.clear();
  if (RPS_UNLIKELY(comp1.is_empty()))
    comp1.clear();
  if (RPS_UNLIKELY(comp2.is_empty()))
    comp2.clear();
  if (RPS_UNLIKELY(comp3.is_empty()))
    comp3.clear();
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  // we want to avoid too frequent resizes, so....
  if (RPS_UNLIKELY(ob_comps.capacity() < ob_comps.size() + 4))
    {
      auto newsiz = rps_prime_above(9*ob_comps.size()/8 + 4);
      ob_comps.reserve(newsiz);
    };
  ob_comps.push_back(comp0);
  ob_comps.push_back(comp1);
  ob_comps.push_back(comp2);
  ob_comps.push_back(comp3);
} // end Rps_ObjectZone::append_comp4


void
Rps_ObjectZone::append_components(const std::initializer_list<Rps_Value>&compil)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  unsigned nbv = compil.size();
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  // we want to avoid too frequent resizes, so....
  if (RPS_UNLIKELY(ob_comps.capacity() < ob_comps.size() + nbv))
    {
      auto newsiz = rps_prime_above(9*ob_comps.size()/8 + nbv);
      ob_comps.reserve(newsiz);
    };
  for (Rps_Value v: compil)
    {
      if (RPS_UNLIKELY(v.is_empty()))
        v.clear();
      ob_comps.push_back(v);
    }
} // end Rps_ObjectZone::append_components


void
Rps_ObjectZone::append_components(const std::vector<Rps_Value>&compvec)
{
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  std::lock_guard<std::recursive_mutex> gu(ob_mtx);
  RPS_ASSERT(stored_type() == Rps_Type::Object);
  unsigned nbv = compvec.size();
  // we want to avoid too frequent resizes, so....
  if (RPS_UNLIKELY(ob_comps.capacity() < ob_comps.size() + nbv))
    {
      auto newsiz = rps_prime_above(9*ob_comps.size()/8 + nbv);
      ob_comps.reserve(newsiz);
    };
  for (Rps_Value v: compvec)
    {
      if (RPS_UNLIKELY(v.is_empty()))
        v.clear();
      ob_comps.push_back(v);
    }
} // end Rps_ObjectZone::append_components



////////////////////////////////////////////////////////////////
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
      Rps_ObjectRef obat = atit.first;
      Rps_Value valat = atit.second;
      if (!rps_is_dumpable_value(du, valat))
        continue;
      if (!rps_is_dumpable_objref(du, obat))
        continue;
      rps_dump_scan_object(du, obat);
      rps_dump_scan_value(du, valat, 0);
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
  RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content start thisob=" << thisob);
  json["class"] = rps_dump_json_objectref(du,obcla);
  {
    // It makes no sense to persist a time with a precision above the
    // centisecond. Since computers are not synchronized better than
    // that.
    double mt = get_mtime();
    char mtbuf[32];
    snprintf(mtbuf, sizeof(mtbuf), "%.2f", mt);
    double mtd = atof(mtbuf);
    json["mtime"] = Json::Value (mtd);
  }
  /// magic getter function
  {
    rps_magicgetterfun_t*mgfun = ob_magicgetterfun.load();
    if (mgfun)
      {
        Dl_info di = {};
        if (dladdr((void*)mgfun, &di))
          {
            RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                           << " has magicgetter " << (void*)mgfun
                           << " dli_fname=" << (di.dli_fname?:"???")
                           << " dli_sname=" << (di.dli_sname?:"???"));
            if (di.dli_sname && !strncmp(di.dli_sname, RPS_GETTERFUN_PREFIX, sizeof(RPS_GETTERFUN_PREFIX)-1)
                && di.dli_sname[sizeof(RPS_GETTERFUN_PREFIX)] == '_'
                && isdigit(di.dli_sname[sizeof(RPS_GETTERFUN_PREFIX)+1]))
              {
                const char* pend=nullptr;
                bool ok = false;
                Rps_Id oidfun(di.dli_sname+sizeof(RPS_GETTERFUN_PREFIX), &pend, &ok);
                if (ok && oidfun)
                  {
                    /* TODO: add more code */
#warning Rps_ObjectZone::dump_json_content should deal with rpsget_* function name (magic getter)
                    RPS_WARNOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob <<
                                " dli_sname:" << di.dli_sname
                                << " @@oidfun:" << oidfun << std::endl
                                << RPS_FULL_BACKTRACE_HERE(1, "Rps_ObjectZone::dump_json_content@@oidfun"));
                    json["magicgetter"] = oidfun.to_string();
                    /// TODO: FIXME the C++ (or GNU lightning?) code
                    /// of oidfun should be generated....
#warning the code of the magicgetter function should be somehow generated...
                  }
              };
          }
        else
          {
            RPS_WARNOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                        << " has strange magicgetter " << (void*)mgfun);
          };
        json["magicattr"] = Json::Value(true);
      }
    else
      RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
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
            RPS_DEBUG_LOG(DUMP, "Rps_ObjectZone::dump_json_content thisob=" << thisob
                          << " has applyingfun " << (void*)apfun
                          << " dli_fname=" << (di.dli_fname?:"???")
                          << " dli_sname=" << (di.dli_sname?:"???"));

            Dl_info di = {};
            if (dladdr((void*)apfun, &di))
              {
                RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                               << " has applyingfun " << (void*)apfun
                               << " dli_fname=" << (di.dli_fname?:"???")
                               << " dli_sname=" << (di.dli_sname?:"???"));
                if (di.dli_sname && !strncmp(di.dli_sname, RPS_APPLYINGFUN_PREFIX, sizeof(RPS_APPLYINGFUN_PREFIX)-1)
                    && di.dli_sname[sizeof(RPS_APPLYINGFUN_PREFIX)] == '_'
                    && isdigit(di.dli_sname[sizeof(RPS_APPLYINGFUN_PREFIX)+1]))
                  {
                    const char* pend=nullptr;
                    bool ok = false;
                    Rps_Id apoidfun(di.dli_sname+sizeof(RPS_APPLYINGFUN_PREFIX), &pend, &ok);
                    if (ok && apoidfun)
                      {
                        /* TODO: add more code */
#warning Rps_ObjectZone::dump_json_content should deal with rpsapply_* function name (applying function)
                        RPS_WARNOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob <<
                                    " applyingfun dli_sname:" << di.dli_sname
                                    << " @@apoidfun:" << apoidfun << std::endl
                                    << RPS_FULL_BACKTRACE_HERE(1, "Rps_ObjectZone::dump_json_content@@apoidfun"));
                        json["applyfun"] = apoidfun.to_string();
                      }
                  };
              }
            else // dladdr apfun failed
              {
                RPS_WARNOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                            << " has strange applyingfun " << (void*)apfun);
              };
            json["applying"] = Json::Value(true);
          }
        else
          RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob
                         << " has applying function");
      }
  }
  /// attributes
  RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob << ", attrs#" << ob_attrs.size());
  if (!ob_attrs.empty())
    {
      Json::Value jattrs(Json::arrayValue);
      for (auto atit: ob_attrs)
        {
          Rps_ObjectRef atob = atit.first;
          Rps_Value atval = atit.second;
          if (!rps_is_dumpable_objref(du,atob))
            continue;
          if (!rps_is_dumpable_objattr(du,atob))
            continue;
          if (!rps_is_dumpable_value(du,atval))
            continue;
          Json::Value jcurat(Json::objectValue);
          jcurat["at"] = rps_dump_json_objectref(du,atob);
          jcurat["va"] = rps_dump_json_value(du,atval);
          jattrs.append(jcurat);
        };
      json["attrs"] = jattrs;
    }
  ///
  RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content thisob=" << thisob << ", comps#" << ob_comps.size());
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
  RPS_NOPRINTOUT("Rps_ObjectZone::dump_json_content end thisob=" << thisob << std::endl);
} // end Rps_ObjectZone::dump_json_contents



////////////////////////////////////////////////////////////////
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


Rps_ObjectRef
Rps_ObjectZone::compute_class(Rps_CallFrame*) const
{
  return get_class();
} // end Rps_ObjectZone::compute_class



std::string
Rps_ObjectZone::string_oid(void) const
{
  return oid().to_string();
} // end Rps_ObjectZone::string_oid


int
Rps_ObjectZone::autocomplete_oid(const char*prefix,
                                 const std::function<bool(const Rps_ObjectZone*)>&stopfun)
{
  RPS_DEBUG_LOG(COMPL_REPL, "autocomplete_oid start prefix"
                << (prefix?"='":" ")
                << (prefix?:"*none*")
                << (prefix?"'":"."));
  if (!prefix || prefix[0] != '_'
      || !isdigit(prefix[1]) || !isalnum(prefix[2]) || !isalnum(prefix[3]))
    return 0;
  int prefixlen = (int) strlen(prefix);
  char bufid[24];
  memset(bufid, 0, sizeof(bufid));
  int lastix=0;
  {
    int ix=0;
    bufid[0] = '_';
    for (ix=1; ix<prefixlen && prefix[ix] != (char)0; ix++)
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
  RPS_DEBUG_LOG(COMPL_REPL, "autocomplete_oid bufid='" << bufid << "' idpref=" << idpref);
  constexpr char lastdigit = Rps_Id::b62digits[sizeof(Rps_Id::b62digits)-2];
  RPS_DEBUG_LOG(COMPL_REPL, "autocomplete_oid lastdigit=" << (lastdigit?:'?') << " of code " << (int)lastdigit);
  for (int ix=lastix; ix<(int)Rps_Id::nbchars; ix++)
    {
      bufid[ix] = lastdigit;
    }
  Rps_Id idlast(bufid);
  RPS_DEBUG_LOG(COMPL_REPL, "autocomplete_oid bufid='" << bufid
                << "', prefixlen=" << prefixlen
                << ", idpref=" << idpref << ", idlast=" << idlast);
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
  auto atset = attributes_set();
  if (atset)
    gc.mark_value(atset);
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
  auto atset = attributes_set();
  if (atset)
    rps_dump_scan_value(du,atset,1);
} // end Rps_PayloadClassInfo::dump_scan


void
Rps_PayloadClassInfo::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_classinfo in store_rps.cc
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
  auto atset = attributes_set();
  if (atset)
    jv["class_attrset"] = rps_dump_json_value(du,atset);
} // end Rps_PayloadClassInfo::dump_json_content


void
Rps_PayloadClassInfo::loader_put_symbname(Rps_ObjectRef obr, Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  pclass_symbname = obr;
} // end Rps_PayloadClassInfo::loader_put_symbname

void
Rps_PayloadClassInfo::loader_put_attrset(const Rps_SetOb*setob, Rps_Loader*ld)
{
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(!setob || setob->stored_type() == Rps_Type::Set);
  pclass_attrset.store(setob);
} // end Rps_PayloadClassInfo::loader_put_attrset

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

std::string
Rps_PayloadClassInfo::class_name_str(void) const
{
  auto obrown = owner();
  if (!obrown) return "";
  Rps_ObjectRef obsymb = symbname();
  if (!obsymb)
    return obrown->oid().to_string();
  {
    std::lock_guard<std::recursive_mutex> gusymb(*(obsymb->objmtxptr()));
    if (auto symbpayl = obsymb->get_dynamic_payload<Rps_PayloadSymbol>())
      return symbpayl->symbol_name();
  }
  return obrown->oid().to_string();
} // end Rps_PayloadClassInfo::class_name_str


Rps_SetValue
Rps_PayloadClassInfo::compute_set_of_own_method_selectors(Rps_CallFrame*callerframe) const
{
  Rps_ObjectRef ob_compute_set_of_own_method_selectors
    = RPS_ROOT_OB(_4bkpL4a6xlO00VnyM8);
  RPS_LOCALFRAME(ob_compute_set_of_own_method_selectors,
                 callerframe,
                 Rps_ObjectRef obcursel; //
                 Rps_ClosureValue curclos; //
                 Rps_SetValue setsel; //
                );
  std::set<Rps_ObjectRef> mutset;
  std::mutex mutlock;
  auto obrown = owner();
  if (!obrown)
    return Rps_SetValue();
  std::lock_guard<std::recursive_mutex> guown(*(obrown->objmtxptr()));
  _.set_additional_gc_marker([&](Rps_GarbageCollector*gc)
  {
    RPS_ASSERT(gc != nullptr);
    std::lock_guard<std::mutex> gu(mutlock);
    for (Rps_ObjectRef obr: mutset)
      {
        gc->mark_obj (obr);
      }
  });
  for (auto& it : pclass_methdict)
    {
      _f.obcursel = it.first;
      _f.curclos = it.second;
      if (!_f.obcursel)
        continue;
      if (!_f.curclos)
        continue;
      mutset.insert(_f.obcursel);
    };
  _f.setsel = Rps_SetValue(mutset);
  return _f.setsel;
} // end Rps_PayloadClassInfo::compute_set_of_own_method_selectors


void
Rps_PayloadClassInfo::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
  out << std::endl
      << BOLD_esc << "¤¤ class information payload ¤¤"
      << NORM_esc << std::endl;
  out << "*super-class:" << pclass_super << std::endl;
  out << "*symbol name:" << pclass_symbname << std::endl;
  /// show the method dictionary
  size_t nbmethod = pclass_methdict.size();
  if (nbmethod==0)
    out << BOLD_esc << "*no own method*" << NORM_esc << std::endl;
  else
    {
      std::vector<Rps_ObjectRef> selvect(nbmethod);
      for (auto it: pclass_methdict)
        {
          RPS_ASSERT(it.first);
          selvect.push_back(it.first);
        };
      if (nbmethod==1)
        {
          out << BOLD_esc << "*one own method*" << NORM_esc << std::endl;
          Rps_ObjectRef thesel = selvect[0];
          auto theit = pclass_methdict.find(thesel);
          RPS_ASSERT(theit != pclass_methdict.end());
          const Rps_ClosureValue theclos = theit->second;
          RPS_ASSERT(theclos.is_closure());
          out << BOLD_esc << "°" << NORM_esc << thesel
              << BOLD_esc << "→" // U+2192 RIGHTWARDS ARROW
              << NORM_esc << " ";
          out << Rps_OutputValue(theclos, depth+1, maxdepth) << std::endl;
        }
      else
        {
          out << BOLD_esc << "*" << nbmethod << " own methods*"
              << NORM_esc << std::endl;
          rps_sort_object_vector_for_display(selvect);
          for (int ix=0; ix<(int)nbmethod; ix++)
            {
              Rps_ObjectRef cursel = selvect[ix];
              auto curit = pclass_methdict.find(cursel);
              RPS_ASSERT(curit != pclass_methdict.end());
              const Rps_ClosureValue curclos = curit->second;
              RPS_ASSERT(curclos.is_closure());
              out << BOLD_esc << "°" << NORM_esc << cursel
                  << BOLD_esc << "→" // U+2192 RIGHTWARDS ARROW
                  << NORM_esc << " ";
              out << Rps_OutputValue(curclos, depth+1, maxdepth) << std::endl;
            }
        }
    } // end if nbmethod not 0
  /// show the attribute set (for classes of instances)
  {
    const Rps_SetOb* setattr = pclass_attrset.load();
    if (setattr != nullptr)
      {
        size_t nbattrset = setattr->cardinal();
        if (nbattrset>0)
          {
            if (nbattrset==1)
              out << BOLD_esc << "* one attribute set *" << NORM_esc
                  << std::endl;
            else
              out << BOLD_esc << "* " << nbattrset << " attributes set *"
                  << NORM_esc << std::endl;
            std::vector<Rps_ObjectRef> attrvect(nbattrset);
            for (auto atit : *setattr)
              {
                attrvect.push_back(*atit);
              };
            rps_sort_object_vector_for_display(attrvect);
            for (int ix=0; ix<(int)nbattrset; ix++)
              {
                Rps_ObjectRef obattr = attrvect[ix];
                RPS_ASSERT(obattr);
                out << BOLD_esc << "[!" << ix << "!]" << NORM_esc
                    << " " << obattr << std::endl;
              }
          }
        else
          out << BOLD_esc << "* no attribute set *" << NORM_esc
              << std::endl;
      }
  }
} // end Rps_PayloadClassInfo::output_payload






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


Rps_ObjectRef
Rps_PayloadSetOb::make_mutable_set_object(Rps_CallFrame*callerframe,
    Rps_ObjectRef classobarg,
    Rps_ObjectRef spaceobarg)
{
  RPS_LOCALFRAME(/*descr:*/RPS_ROOT_OB(_0J1C39JoZiv03qA2HA), //mutable_set∈class
                           callerframe,
                           Rps_ObjectRef classob;
                           Rps_ObjectRef spaceob;
                           Rps_ObjectRef resob;
                );
  if (classobarg.is_empty())
    _f.classob = RPS_ROOT_OB(_0J1C39JoZiv03qA2HA); // mutable_set∈class
  else
    _f.classob = classobarg;
  if (!spaceobarg.is_empty())
    _f.spaceob = spaceobarg;
  else
    _f.spaceob = nullptr;
  if (!_f.classob)
    throw std::runtime_error("make_mutable_set_object missing class");
  if (!_f.classob->is_class())
    {
      std::string classobstr = _f.classob.as_string();
      throw std::runtime_error(std::string{"make_mutable_set_object: bad class:"} + classobstr);
    };
  RPS_ASSERT(_f.classob);
  if (!_f.classob->is_subclass_of(RPS_ROOT_OB(_0J1C39JoZiv03qA2HA))) // mutable_set∈class
    {
      std::string classobstr = _f.classob.as_string();
      throw std::runtime_error(std::string{"make_mutable_set_object: invalid class:"} + classobstr);
    }
  if (_f.spaceob && !_f.spaceob->is_subclass_of(RPS_ROOT_OB(_2i66FFjmS7n03HNNBx))) //space∈class
    {
      std::string spaceobstr = _f.spaceob.as_string();
      throw std::runtime_error(std::string{"make_mutable_set_object: invalid space:"} + spaceobstr);
    }
  _f.resob = Rps_ObjectRef::make_object(&_, _f.classob, _f.spaceob);
  _f.resob->put_new_plain_payload<Rps_PayloadSetOb>();
  return _f.resob;
} // end Rps_PayloadSetOb::make_mutable_set_object


void
Rps_PayloadSetOb::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
  unsigned setcard = cardinal();
  if (setcard==0)
    {
      out << BOLD_esc << "¤¤ empty set object payload ¤¤" << NORM_esc
          << std::endl;
      return;
    }
  std::vector<Rps_ObjectRef> vectelem(setcard);
  for (auto setit: psetob)
    {
      Rps_ObjectRef curob = *setit;
      RPS_ASSERT(curob);
      vectelem.push_back(curob);
    };
  if (setcard==1)
    out << BOLD_esc << "¤¤ singleton set object payload ¤¤" << NORM_esc
        << std::endl;
  else
    {
      rps_sort_object_vector_for_display(vectelem);
      out << BOLD_esc << "¤¤ set object payload of "
          << setcard << " elements" << NORM_esc << std::endl;
    };
  for (int ix=0; ix<(int)setcard; ix++)
    {
      Rps_ObjectRef obelem = vectelem[ix];
      RPS_ASSERT(obelem);
      out << BOLD_esc << "[." << ix << ".]" << NORM_esc
          << " " << obelem << std::endl;
    };
} // end of Rps_PayloadSetOb::output_payload








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

void
Rps_PayloadVectOb::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
  unsigned vectsiz = size();
  if (vectsiz == 0)
    {
      out << BOLD_esc << "* empty object vector payload *" << NORM_esc << std::endl;
      return;
    }
  std::vector<Rps_ObjectRef> vectcomp(vectsiz);
  for (auto vectit: pvectob)
    {
      Rps_ObjectRef curob = *vectit;
      RPS_ASSERT(curob);
      vectcomp.push_back(curob);
    };
  if (vectsiz == 1)
    {
      out << BOLD_esc << "* singleton object vector payload *"
          << NORM_esc << std::endl;
    }
  else
    {
      out << BOLD_esc << "* vector of " << vectsiz << " objects payload *"
          << NORM_esc << std::endl;
    };
  for (unsigned ix=0; ix<vectsiz; ix++)
    {
      Rps_ObjectRef obcomp = vectcomp[ix];
      RPS_ASSERT(obcomp);
      out << BOLD_esc << "[" << ix << "]" << NORM_esc
          << " ";
      obcomp.output(out, depth+1, maxdepth);
      out << std::endl;
    }
} // end of Rps_PayloadVectOb::output_payload


/***************** mutable vector of values payload **********/

void
Rps_PayloadVectVal::gc_mark(Rps_GarbageCollector&gc) const
{
  for (auto compv: pvectval)
    if (compv)
      gc.mark_value(compv);
} // end Rps_PayloadVectVal::gc_mark

void
Rps_PayloadVectVal::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
  for (auto compv: pvectval)
    if (compv)
      rps_dump_scan_value(du, compv, 0);
} // end Rps_PayloadVectVal::dump_scan


void
Rps_PayloadVectVal::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  /// see function rpsldpy_vectob in store_rps.cc
  RPS_ASSERT(du != nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  Json::Value jarr(Json::arrayValue);
  for (auto compv: pvectval)
    if (rps_is_dumpable_value(du, compv))
      jarr.append(rps_dump_json_value(du,compv));
    else
      jarr.append(Json::Value(Json::nullValue));
  jv["vectval"] = jarr;
} // end Rps_PayloadVectVal::dump_json_content



const Rps_ClosureZone*
Rps_PayloadVectVal::make_closure_zone_from_vector(Rps_ObjectRef connob)
{
  if (!connob)
    return nullptr;
  std::lock_guard<std::recursive_mutex> gu(*(connob->objmtxptr()));
  return Rps_ClosureZone::make(connob, pvectval);
} // end Rps_PayloadVectVal::make_closure_zone_from_vector


const Rps_InstanceZone*
Rps_PayloadVectVal::make_instance_zone_from_vector(Rps_ObjectRef classob)
{
  if (!classob)
    return nullptr;
  std::lock_guard<std::recursive_mutex> gu(*(classob->objmtxptr()));
  if (!classob->is_class())
    return nullptr;
  return Rps_InstanceZone::make_from_components(classob, pvectval);
} // end Rps_PayloadVectVal::make_instance_zone_from_vector

void
Rps_PayloadVectVal::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  std::lock_guard<std::recursive_mutex> guown(*(owner()->objmtxptr()));
#warning incomplete Rps_PayloadVectVal::output_payload
} // end of Rps_PayloadVectVal::output_payload



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

void
Rps_PayloadSpace::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  RPS_ASSERT(depth <= maxdepth);
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  if (owner() == Rps_ObjectRef::root_space())
    out << BOLD_esc << "** root space payload **" << NORM_esc << std::endl;
  else
    out << BOLD_esc << "** space payload **" << NORM_esc << std::endl;
} // end Rps_PayloadSpace::output_payload

/***************** symbol payload **********/

std::recursive_mutex Rps_PayloadSymbol::symb_tablemtx;
std::map<std::string,Rps_PayloadSymbol*> Rps_PayloadSymbol::symb_table;
std::unordered_map<std::string,Rps_ObjectRef*> Rps_PayloadSymbol::symb_hardcoded_hashtable;

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


Rps_PayloadSymbol::~Rps_PayloadSymbol()
{
  std::lock_guard<std::recursive_mutex> gu(symb_tablemtx);
  RPS_DEBUG_LOG(LOWREP, "~Rps_PayloadSymbol symb_name='"
                << symb_name << "' owner=" << owner()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "~Rps_PayloadSymbol"));
  if (!symb_name.empty())
    symb_table.erase(symb_name);
} // end Rps_PayloadSymbol::~Rps_PayloadSymbol()



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
  RPS_DEBUG_LOG(LOAD,
                "Rps_PayloadSymbol::load_register_name symb_name:" << symb_name
                << " " << (weak?"weak":"strong")
                << " owner:" << owner()->oid().to_string() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Rps_PayloadSymbol::load_register_name"));
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
  RPS_NOPRINTOUT("Rps_PayloadSymbol::dump_json_content owner=" << owner()->oid().to_string()
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

const Rps_SetValue
Rps_PayloadSymbol::set_of_all_symbols(void)
{
  std::vector<Rps_ObjectRef> vecob;
  {
    std::lock_guard<std::recursive_mutex> gusy(symb_tablemtx);
    unsigned nbsymb = symb_table.size();
    vecob.reserve(nbsymb);
    for (auto it : symb_table)
      if (it.second && it.second->owner())
        vecob.push_back(it.second->owner());
  }
  return Rps_SetValue(vecob);
} // end Rps_PayloadSymbol::set_of_all_symbols


void
Rps_PayloadSymbol::output_payload(std::ostream&out, unsigned depth, unsigned maxdepth) const
{
  RPS_ASSERT(depth <= maxdepth);
  bool ontty =
    (&out == &std::cout)?isatty(STDOUT_FILENO)
    :(&out == &std::cerr)?isatty(STDERR_FILENO)
    :false;
  if (rps_without_terminal_escape)
    ontty = false;
  const char* BOLD_esc = (ontty?RPS_TERMINAL_BOLD_ESCAPE:"");
  const char* NORM_esc = (ontty?RPS_TERMINAL_NORMAL_ESCAPE:"");
  out << "*" << BOLD_esc << (is_weak()?"weak":"strong")
      << " symbol named " << symbol_name() << NORM_esc
      << " of value ";
  symbol_value().output(out, depth, maxdepth);
  out << " " << BOLD_esc << "*" << NORM_esc << std::endl;
} // end Rps_PayloadSymbol::output_payload



////////////////////////////////////////////////////////////////
Rps_ObjectRef
Rps_ObjectRef::find_object_by_string(Rps_CallFrame*callerframe, const std::string& str, Rps_ObjectRef::Find_Behavior_en behav)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callerframe,
                 Rps_ObjectRef obsymb;
                 Rps_ObjectRef obfound;
                );
  RPS_DEBUG_LOG(LOWREP, "find_object_by_string for str='" << str
                << "' from " << std::endl
                << Rps_ShowCallFrame(&_)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "find_object_by_string"));
  if (str.empty())
    {
      if (behav == Rps_Null_When_Missing)
        return Rps_ObjectRef(nullptr);
      throw std::runtime_error("empty string to Rps_ObjectRef::find_object_by_string");
    }
  if (isalpha(str[0]))
    {
      _f.obsymb = Rps_PayloadSymbol::find_named_object(str);
      RPS_DEBUG_LOG(LOWREP, "find_object_by_string for str='"
                    << str << "'  obsymb=" << _f.obsymb);
      if (!_f.obsymb)
        {
          RPS_DEBUG_LOG(LOWREP, "find_object_by_string for str='"
                        << str << "' SYMBOL NOT FOUND");
          if (behav == Rps_Null_When_Missing)
            return Rps_ObjectRef(nullptr);
          throw std::runtime_error("Rps_ObjectRef::find_object_by_string: no symbol named " + str);
        };
      auto symbpayl = _f.obsymb->get_dynamic_payload<Rps_PayloadSymbol>();
      RPS_ASSERT(symbpayl != nullptr);
      RPS_DEBUG_LOG(LOWREP, "find_object_by_string for str='"
                    << str << "' got symbol "
                    << _f.obsymb << " of oid " << _f.obsymb->oid()
                    << " and value:" << symbpayl->symbol_value());
      if (symbpayl->symbol_value().is_object())
        _f.obfound = symbpayl->symbol_value().as_object();
      if (!_f.obfound)
        _f.obfound = _f.obsymb;
    }
  else if (str[0] == '_')
    {
      Rps_Id id(str);
      RPS_DEBUG_LOG(LOWREP, "find_object_by_string for id=" << id);
      if (!id)
        {
          if (behav == Rps_Null_When_Missing)
            return Rps_ObjectRef(nullptr);
          throw std::runtime_error("Rps_ObjectRef::find_object_by_string: bad id " + str);
        };
      _f.obfound = Rps_ObjectRef(Rps_ObjectZone::find(id));
      RPS_DEBUG_LOG(LOWREP, "find_object_by_string for str='"
                    << str << "'  obfound=" << _f.obfound);
      if (!_f.obfound)
        {
          if (behav == Rps_Null_When_Missing)
            return Rps_ObjectRef(nullptr);
          throw std::runtime_error("Rps_ObjectRef::find_object_by_string: nonexistant id " + str);
        }
    }
  else
    {
      if (behav == Rps_Null_When_Missing)
        return Rps_ObjectRef(nullptr);
      throw std::runtime_error("bad string " + str + " to Rps_ObjectRef::find_object_by_string");
    }
  RPS_ASSERT(_f.obfound || behav == Rps_Null_When_Missing);
  return _f.obfound;
} // end Rps_ObjectRef::find_object_by_string


Rps_ObjectRef
Rps_ObjectRef::find_object_by_oid(Rps_CallFrame*callerframe, Rps_Id oid, Rps_ObjectRef::Find_Behavior_en behav)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callerframe,
                 Rps_ObjectRef obfound;
                );
  RPS_DEBUG_LOG(LOWREP, "find_object_by_oid oid=" << oid << " from "
                << Rps_ShowCallFrame(&_)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "find_object_by_oid"));
  if (!oid || !oid.valid())
    {
      if (behav == Rps_Null_When_Missing)
        return Rps_ObjectRef(nullptr);
      throw std::runtime_error("Rps_ObjectRef::find_object_by_oid: invalid or empty oid");
    }
  _f.obfound = Rps_ObjectRef(Rps_ObjectZone::find(oid));
  RPS_DEBUG_LOG(LOWREP, "find_object_by_oid oid=" << oid << " obfound="
                << _f.obfound
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "find_object_by_oid"));
  if (!_f.obfound)
    {
      if (behav == Rps_Null_When_Missing)
        return Rps_ObjectRef(nullptr);
      throw std::runtime_error(std::string{"Rps_ObjectRef::find_object: unknown id:"} + oid.to_string());
    }
  RPS_ASSERT(_f.obfound || behav == Rps_Null_When_Missing);
  return _f.obfound;
} // end Rps_ObjectRef::find_object_by_oid


Rps_ObjectRef
Rps_ObjectRef::really_find_object_by_oid(const Rps_Id& oid)
{
  return Rps_ObjectRef(Rps_ObjectZone::find(oid));
} // end Rps_ObjectRef::really_find_object_by_oid

Rps_ObjectRef
Rps_ObjectRef::make_named_class(Rps_CallFrame*callerframe, Rps_ObjectRef superclassarg, std::string name)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callerframe,
                 Rps_ObjectRef obthemutsetclasses;
                 Rps_ObjectRef obsuperclass;
                 Rps_ObjectRef obsymbol; // the symbol
                 Rps_ObjectRef obclass; //
                );
  _f.obthemutsetclasses =  RPS_ROOT_OB(_4DsQEs8zZf901wT1LH); //"the_mutable_set_of_classes"∈mutable_set
  _f.obsuperclass = superclassarg;
  RPS_INFORMOUT("Rps_ObjectRef::make_named_class obsuperclass="
                << _f.obsuperclass << ", name=" << name << " start");
  if (RPS_UNLIKELY(!_f.obsuperclass))
    {
      RPS_WARNOUT("make_named_class without superclass for name " << name);
      throw std::runtime_error(std::string("make_named_class without superclass"));
    }
  // the superclassob should be instance of `class` class
  if (RPS_UNLIKELY(_f.obsuperclass->get_class() != RPS_ROOT_OB(_41OFI3r0S1t03qdB2E)))
    {
      RPS_WARNOUT("make_named_class with invalid superclass " << _f.obsuperclass
                  << " for name " << name);

      throw std::runtime_error(std::string("make_named_class with invalid superclass"));
    };
  if (!Rps_PayloadSymbol::valid_name(name))
    {
      RPS_WARNOUT("make_named_class with superclass " << _f.obsuperclass
                  << " with invalid name " << name);
      throw std::runtime_error(std::string("make_named_class with invalid name"));
    }
  RPS_INFORMOUT("Rps_ObjectRef::make_named_class valid name=" << name);
  _f.obsymbol = Rps_PayloadSymbol::find_named_object(name);
  if (!_f.obsymbol)
    _f.obsymbol = Rps_ObjectRef::make_new_strong_symbol(&_, name);
  RPS_INFORMOUT("Rps_ObjectRef::make_named_class name=" << name <<", obsymbol=" << _f.obsymbol);
  std::unique_lock<std::recursive_mutex> gusymb (*(_f.obsymbol->objmtxptr()));
  // obsymbol should be of class `symbol`
  RPS_ASSERT(_f.obsymbol->get_class() == RPS_ROOT_OB(_36I1BY2NetN03WjrOv));
  RPS_INFORMOUT("Rps_ObjectRef::make_named_class good obsymbol=" << _f.obsymbol);
  auto paylsymbol = _f.obsymbol-> get_dynamic_payload<Rps_PayloadSymbol>();
  RPS_ASSERT (paylsymbol);
  _f.obclass = Rps_ObjectZone::make();
  RPS_INFORMOUT("Rps_ObjectRef::make_named_class name=" << name << ", paylsymbol=" << paylsymbol
                << ", obclass=" << _f.obclass);
  /// the class is class `class`
  _f.obclass->ob_class.store(RPS_ROOT_OB(_41OFI3r0S1t03qdB2E));
  auto paylclainf = _f.obclass->put_new_plain_payload<Rps_PayloadClassInfo>();
  paylclainf->put_superclass(_f.obsuperclass);
  paylclainf->put_symbname(_f.obsymbol);
  paylsymbol->symbol_put_value(_f.obclass);
  _f.obclass->put_space(RPS_ROOT_OB(_8J6vNYtP5E800eCr5q)); // the initial space
  _f.obsymbol->put_space(RPS_ROOT_OB(_8J6vNYtP5E800eCr5q)); // the initial space
  rps_add_root_object (_f.obclass);
  RPS_INFORMOUT("Rps_ObjectRef::make_named_class name="<< name
                << " gives obclass=" << _f.obclass);
  std::unique_lock<std::recursive_mutex> gumutsetcla (*(_f.obthemutsetclasses->objmtxptr()));
  auto paylsetcla = _f.obthemutsetclasses->get_dynamic_payload< Rps_PayloadSetOb>();
  RPS_ASSERT(paylsetcla != nullptr);
  paylsetcla->add(_f.obclass);
  return _f.obclass;
} // end Rps_ObjectRef::make_named_class



static std::mutex rps_symbol_mtx;
Rps_ObjectRef
Rps_ObjectRef::make_new_symbol(Rps_CallFrame*callerframe, std::string name, bool isweak)
{
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 callerframe,
                 Rps_ObjectRef obsymbol; // the symbol
                );
  std::lock_guard<std::mutex> gusymb (rps_symbol_mtx);
  if (!Rps_PayloadSymbol::valid_name(name))
    {
      RPS_WARNOUT("make_new_symbol with invalid name " << name);
      throw std::runtime_error(std::string("make_new_symbol with invalid name"));
    }
  if (Rps_PayloadSymbol::find_named_object(name))
    {
      RPS_WARNOUT("make_new_symbol with existing name " << name);
      throw std::runtime_error(std::string("make_new_symbol with existing name"));
    }
  _f.obsymbol = Rps_ObjectZone::make();
  _f.obsymbol->ob_class.store(RPS_ROOT_OB(_36I1BY2NetN03WjrOv)); // the `symbol` class
  Rps_PayloadSymbol::register_name(name, _f.obsymbol, isweak);
  RPS_NOPRINTOUT("Rps_ObjectRef::make_new_symbol name=" << name
                 << " gives obsymbol=" << _f.obsymbol);
  return _f.obsymbol;
} // end of Rps_ObjectRef::make_new_symbol

Rps_ObjectRef
Rps_ObjectRef::make_object(Rps_CallFrame*callerframe, Rps_ObjectRef classobarg, Rps_ObjectRef spaceobarg)
{
  RPS_LOCALFRAME(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ), //object∈class
                 callerframe,
                 Rps_ObjectRef classob; // the class
                 Rps_ObjectRef spaceob; // the space
                 Rps_ObjectRef resultob; // resulting object
                );
  _f.classob = classobarg;
  _f.spaceob = spaceobarg;
  if (_f.spaceob
      && RPS_UNLIKELY(!_f.spaceob->is_instance_of(RPS_ROOT_OB(_2i66FFjmS7n03HNNBx))))   //space∈class
    {
      RPS_WARNOUT("invalid spaceob " << _f.spaceob
                  << "for make_object");
      throw RPS_RUNTIME_ERROR_OUT("invalid spaceob " << _f.spaceob
                                  << "for make_object");
      if (!_f.classob
          || !_f.classob->is_subclass_of(RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)))   // object∈class
        {
          RPS_WARNOUT("invalid class " << _f.classob
                      << "for make_object");
          throw RPS_RUNTIME_ERROR_OUT("invalid class " << _f.classob
                                      << "for make_object");
        }
    };
  _f.resultob = Rps_ObjectZone::make();
  _f.resultob->ob_class.store(_f.classob);
  RPS_DEBUG_LOG(LOWREP, "make_object classob=" << _f.classob << " -> resultob=" << _f.resultob);
  _f.resultob->put_space(_f.spaceob);
  /// FIXME: perhaps we should send some `initialize_object` message?
  return _f.resultob;
} // end Rps_ObjectRef::make_object



void
Rps_ObjectRef::install_own_method(Rps_CallFrame*callerframe, Rps_ObjectRef obselarg, Rps_Value closvarg)
{
  /// FIXME: the frame descriptor is not very well choosen, but better than nothing....
  RPS_LOCALFRAME(RPS_ROOT_OB(_6JbWqOsjX5T03M1eGM), //closure_for_method_selector∈symbol
                 callerframe,
                 Rps_ObjectRef obclass; // the class
                 Rps_ObjectRef obsel; // the selector
                 Rps_Value closv; // the closure
                );
  _f.obclass = *this;
  _f.obsel = obselarg;
  _f.closv = closvarg;
  if (_f.obclass.is_empty())
    {
      RPS_WARNOUT("empty class for install_own_method of selector " << _f.obsel);
      throw RPS_RUNTIME_ERROR_OUT("empty class for install_own_method of selector " << _f.obsel);
    }
  std::unique_lock<std::recursive_mutex> guclass (*(_f.obclass->objmtxptr()));
  if (_f.obsel.is_empty())
    {
      RPS_WARNOUT("empty selector for install_own_method of class " << _f.obclass);
      throw RPS_RUNTIME_ERROR_OUT("empty selector for install_own_method of class " << _f.obclass);
    }
  auto paylcl = _f.obclass->get_classinfo_payload();
  if (!paylcl)
    {
      RPS_WARNOUT("bad class for install_own_method of class " << _f.obclass << " selector " << _f.obsel);
      throw RPS_RUNTIME_ERROR_OUT("bad class for install_own_method of class " << _f.obclass << " selector " << _f.obsel);
    }
  if (_f.closv.is_empty() || !_f.closv.is_closure())
    {
      RPS_WARNOUT("bad closure for install_own_method of class " << _f.obclass << " selector " << _f.obsel);
      throw RPS_RUNTIME_ERROR_OUT("bad closure for install_own_method of class " << _f.obclass << " selector " << _f.obsel);
    }
  paylcl->put_own_method(_f.obsel, _f.closv);
} // end Rps_ObjectRef::install_own_method


void
Rps_ObjectRef::install_own_2_methods(Rps_CallFrame*callerframe, Rps_ObjectRef obsel0arg, Rps_Value closv0arg, Rps_ObjectRef obsel1arg, Rps_Value closv1arg)
{
  /// FIXME: the frame descriptor is not very well choosen, but better than nothing....
  RPS_LOCALFRAME(RPS_ROOT_OB(_6JbWqOsjX5T03M1eGM), //closure_for_method_selector∈symbol
                 callerframe,
                 Rps_ObjectRef obclass; // the class
                 Rps_ObjectRef obsel0; // the selector #0
                 Rps_Value closv0; // the closure #0
                 Rps_ObjectRef obsel1; // the selector #1
                 Rps_Value closv1; // the closure #1
                );
  _f.obclass = *this;
  _f.obsel0 = obsel0arg;
  _f.closv0 = closv0arg;
  _f.obsel1 = obsel1arg;
  _f.closv1 = closv1arg;
  if (_f.obclass.is_empty())
    {
      RPS_WARNOUT("empty class for install_own_2_methods of selector#0 " << _f.obsel0 << ", selector#1 " << _f.obsel1);
      throw RPS_RUNTIME_ERROR_OUT("empty class for install_own_2_methods of selector#0 " << _f.obsel0 << ", selector#1 " << _f.obsel1);
    }
  std::unique_lock<std::recursive_mutex> guclass (*(_f.obclass->objmtxptr()));
  if (_f.obsel0.is_empty())
    {
      RPS_WARNOUT("empty selector#0 for install_own_2_methods of class " << _f.obclass);
      throw RPS_RUNTIME_ERROR_OUT("empty selector#0 for install_own_2_methods of class " << _f.obclass);
    }
  if (_f.obsel1.is_empty())
    {
      RPS_WARNOUT("empty selector#1 for install_own_2_methods of class " << _f.obclass);
      throw RPS_RUNTIME_ERROR_OUT("empty selector#1 for install_own_2_methods of class " << _f.obclass);
    }
  auto paylcl = _f.obclass->get_classinfo_payload();
  if (!paylcl)
    {
      RPS_WARNOUT("bad class for install_own_2_methods of class " << _f.obclass << " selector#0 " << _f.obsel0 << ", selector#1 " << _f.obsel1);
      throw RPS_RUNTIME_ERROR_OUT("bad class for install_own_2_methods of class " << _f.obclass << " selector#0 " << _f.obsel0 << ", selector#1 " << _f.obsel1);
    }
  if (_f.closv0.is_empty() || !_f.closv0.is_closure())
    {
      RPS_WARNOUT("bad closure#0 for install_2_methods of class " << _f.obclass << " selector#0 " << _f.obsel0);
      throw RPS_RUNTIME_ERROR_OUT("bad closure#0 for install_2_methods of class " << _f.obclass << " selector#0 " << _f.obsel0);
    }
  if (_f.closv1.is_empty() || !_f.closv1.is_closure())
    {
      RPS_WARNOUT("bad closure#1 for install_2_methods of class " << _f.obclass << " selector#1 " << _f.obsel1);
      throw RPS_RUNTIME_ERROR_OUT("bad closure#1 for install_2_methods of class " << _f.obclass << " selector#1 " << _f.obsel1);
    }
  paylcl->put_own_method(_f.obsel0, _f.closv0);
  paylcl->put_own_method(_f.obsel1, _f.closv1);
} // end Rps_ObjectRef::install_own_2_methods



void
Rps_ObjectRef::install_own_3_methods(Rps_CallFrame*callerframe, Rps_ObjectRef obsel0arg, Rps_Value clos0arg, Rps_ObjectRef obsel1arg, Rps_Value clos1arg, Rps_ObjectRef obsel2arg, Rps_Value clos2arg)
{
  /// FIXME: the frame descriptor is not very well choosen, but better than nothing....
  RPS_LOCALFRAME(RPS_ROOT_OB(_6JbWqOsjX5T03M1eGM), //closure_for_method_selector∈symbol
                 callerframe,
                 Rps_ObjectRef obclass; // the class
                 Rps_ObjectRef obsel0; // the selector #0
                 Rps_Value closv0; // the closure #0
                 Rps_ObjectRef obsel1; // the selector #1
                 Rps_Value closv1; // the closure #1
                 Rps_ObjectRef obsel2; // the selector #2
                 Rps_Value closv2; // the closure #2
                );
  _f.obclass = *this;
  _f.obsel0 = obsel0arg;
  _f.closv0 = clos0arg;
  _f.obsel1 = obsel1arg;
  _f.closv1 = clos1arg;
  _f.obsel2 = obsel2arg;
  _f.closv2 = clos2arg;
  if (_f.obclass.is_empty())
    {
      RPS_WARNOUT("empty class for install_own_3_methods of selector#0 " << _f.obsel0);
      throw RPS_RUNTIME_ERROR_OUT("empty class for install_own_3_methods of selector#0 " << _f.obsel0);
    }
  std::unique_lock<std::recursive_mutex> guclass (*(_f.obclass->objmtxptr()));
  if (_f.obsel0.is_empty())
    {
      RPS_WARNOUT("empty selector#0 for install_own_3_methods of class " << _f.obclass);
      throw RPS_RUNTIME_ERROR_OUT("empty selector#0 for install_own_3_methods of class " << _f.obclass);
    }
  if (_f.obsel1.is_empty())
    {
      RPS_WARNOUT("empty selector#1 for install_own_3_methods of class " << _f.obclass);
      throw RPS_RUNTIME_ERROR_OUT("empty selector#1 for install_own_3_methods of class " << _f.obclass);
    }
  if (_f.obsel2.is_empty())
    {
      RPS_WARNOUT("empty selector#2 for install_own_3_methods of class " << _f.obclass);
      throw RPS_RUNTIME_ERROR_OUT("empty selector#2 for install_own_3_methods of class " << _f.obclass);
    }
  auto paylcl = _f.obclass->get_classinfo_payload();
  if (!paylcl)
    {
      RPS_WARNOUT("bad class for install_own_3_methods of class " << _f.obclass << " selector#0 " << _f.obsel0 << ", selector#1 " << _f.obsel1 << ", selector#2 " << _f.obsel2);
      throw RPS_RUNTIME_ERROR_OUT("bad class for install_own_3_methods of class " << _f.obclass << " selector#0 " << _f.obsel0 << ", selector#1 " << _f.obsel1 << ", selector#2 " << _f.obsel2);
    }
  if (_f.closv0.is_empty() || !_f.closv0.is_closure())
    {
      RPS_WARNOUT("bad closure#0 for install_own_3_methods of class " << _f.obclass << " selector#0 " << _f.obsel0);
      throw RPS_RUNTIME_ERROR_OUT("bad closure#0 for install_own_3_methods of class " << _f.obclass << " selector#0 " << _f.obsel0);
    }
  if (_f.closv1.is_empty() || !_f.closv1.is_closure())
    {
      RPS_WARNOUT("bad closure#1 for install_own_3_methods of class " << _f.obclass << " selector#1 " << _f.obsel1);
      throw RPS_RUNTIME_ERROR_OUT("bad closure#1 for install_own_3_methods of class " << _f.obclass << " selector#1 " << _f.obsel1);
    }
  if (_f.closv2.is_empty() || !_f.closv2.is_closure())
    {
      RPS_WARNOUT("bad closure#2 for install_own_3_methods of class " << _f.obclass << " selector#2 " << _f.obsel1);
      throw RPS_RUNTIME_ERROR_OUT("bad closure#1 for install_own_3_methods of class " << _f.obclass << " selector#1 " << _f.obsel1);
    }
  paylcl->put_own_method(_f.obsel0, _f.closv0);
  paylcl->put_own_method(_f.obsel1, _f.closv1);
  paylcl->put_own_method(_f.obsel2, _f.closv2);
} // end Rps_ObjectRef::install_own_3_methods



// end of file objects_rps.cc

/****************************************************************
 * file lightgen_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for machine code generation using libgccjit.
 *      See also https://gcc.gnu.org/onlinedocs/jit/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2023 - 2024 The Reflective Persistent System Team
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
 *    See also companion file cppgen_rps.cc for C++ code generation.
 *    See https://lists.gnu.org/archive/html/lightning/2024-09/msg00010.html
 ************************************************************************/

#include "refpersys.hh"

#include "libgccjit.h"

#include "libgccjit++.h"

extern "C" const char rps_gccjit_gitid[];
const char rps_gccjit_gitid[]= RPS_GITID;

extern "C" const char rps_gccjit_date[];
const char rps_gccjit_date[]= __DATE__;

extern "C" const char rps_gccjit_shortgitid[];
const char rps_gccjit_shortgitid[]= RPS_SHORTGITID;


extern "C" gccjit::context rps_gccjit_top_ctxt;
gccjit::context rps_gccjit_top_ctxt;

extern "C" void rpsldpy_gccjit(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno);



/// payload for GNU libgccjit code generation:
class Rps_PayloadGccjit : public Rps_Payload
{
  friend void rpsldpy_gccjit(Rps_ObjectZone*obz, Rps_Loader*ld,
                             const Json::Value& jv, Rps_Id spacid,
                             unsigned lineno);
  /**
   * This class should store the data to generate code (probably a
   * dlopen-able plugin) from some libgccjit compatible and rather
   * portable representation.
   **/
  gccjit::context _gji_ctxt;  // child context for code generation
  /// We maintain a mapping between RefPerSys objects representing
  /// code and the gccjit::object-s for them.
  /// TODO: document the representation of GCCJIT code.
  std::map<Rps_ObjectRef, gccjit::object> _gji_rpsobj2jit;
  friend Rps_PayloadGccjit*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadGccjit,Rps_ObjectZone*>(Rps_ObjectZone*);
public:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadGccjit(Rps_ObjectZone*owner);
  Rps_PayloadGccjit(Rps_ObjectRef obr) :
    Rps_PayloadGccjit(obr?obr.optr():nullptr) {};
  virtual const std::string payload_type_name(void) const
  {
    return "gccjit";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
  virtual ~Rps_PayloadGccjit();
  gccjit::location make_csrc_location(const char*filename, int line, int col)
  {
    RPS_ASSERT(filename);
    RPS_ASSERT(filename[0] != '_');
    std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
    return  _gji_ctxt.gccjit::context::new_location(filename, line, col);
  };
  gccjit::location make_string_src_location(const std::string&filen, int line, int col)
  {
    RPS_ASSERT(!filen.empty());
    RPS_ASSERT(filen[0] != '_');
    std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
    return  _gji_ctxt.gccjit::context::new_location(filen.c_str(), line, col);
  };
  gccjit::location no_src_location()
  {
    return gccjit::location();
  };
  // an arbitrary refpersys object may represent a fictuous "source file"
  gccjit::location make_rpsobj_location(Rps_ObjectRef ob, int line, int col=0);
  void locked_register_object_jit(Rps_ObjectRef ob,  gccjit::object jit);
  void locked_unregister_object_jit(Rps_ObjectRef ob);
protected:
  void load_jit_json(Rps_Loader*ld, Rps_Id spacid, unsigned lineno, Json::Value&jseq);
  void raw_register_object_jit(Rps_ObjectRef ob,  const gccjit::object jit);
  void raw_unregister_object_jit(Rps_ObjectRef ob);
  ///
  /// Member functions for making a type; the raw versions don't lock
  /// the owner object, the locked ones do... Names are inspired by
  /// those in https://gcc.gnu.org/onlinedocs/jit/topics/types.html
  gccjit::type raw_get_gccjit_builtin_type(enum gcc_jit_types);
  gccjit::type locked_get_gccjit_builtin_type(enum gcc_jit_types);
  template <typename IntType> gccjit::type raw_get_gccjit_int_type()
  {
    RPS_ASSERT(owner());
    return _gji_ctxt.get_int_type<IntType>();
  };
  template <typename IntType> gccjit::type locked_get_gccjit_int_type()
  {
    RPS_ASSERT(owner());
    std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
    return _gji_ctxt.get_int_type<IntType>();
  };
  //// pointer, const, volatile, aligned types of a given type
  gccjit::type raw_get_gccjit_pointer_type(gccjit::type);
  gccjit::type locked_get_gccjit_pointer_type(gccjit::type);
  gccjit::type raw_get_gccjit_const_type(gccjit::type);
  gccjit::type locked_get_gccjit_const_type(gccjit::type);
  gccjit::type raw_get_gccjit_volatile_type(gccjit::type);
  gccjit::type locked_get_gccjit_volatile_type(gccjit::type);
  gccjit::type raw_get_gccjit_aligned_type(gccjit::type,size_t alignment );
  gccjit::type locked_get_gccjit_aligned_type(gccjit::type, size_t alignment);
  /// array type of a given type with or without location
  gccjit::type raw_new_gccjit_array_type(gccjit::type elemtype, int nbelem);
  gccjit::type locked_new_gccjit_array_type(gccjit::type elemtype, int nbelem);
  gccjit::type raw_new_gccjit_array_type(gccjit::type elemtype, int nbelem, gccjit::location loc);
  gccjit::type locked_new_gccjit_array_type(gccjit::type elemtype, int nbelem, gccjit::location loc);
  /// opaque struct of a given name with or without location
  gccjit::struct_ raw_new_gccjit_opaque_struct_type(const std::string&name);
  gccjit::struct_ locked_new_gccjit_opaque_struct_type(const std::string&name);
  gccjit::struct_ raw_new_gccjit_opaque_struct_type(const std::string&name, gccjit::location loc);
  gccjit::struct_ locked_new_gccjit_opaque_struct_type(const std::string&name, gccjit::location loc);
  /// opaque struct of a given object with or without location; in
  /// the locked member functions variants the RefPerSys object is
  /// registered...
  gccjit::struct_ raw_new_gccjit_opaque_struct_type(const Rps_ObjectRef ob);
  gccjit::struct_ locked_new_gccjit_opaque_struct_type(const Rps_ObjectRef ob);
  gccjit::struct_ raw_new_gccjit_opaque_struct_type(const Rps_ObjectRef ob, gccjit::location loc);
  gccjit::struct_ locked_new_gccjit_opaque_struct_type(const Rps_ObjectRef ob, gccjit::location loc);
  ////
};        // end class Rps_PayloadGccjit



Rps_PayloadGccjit::Rps_PayloadGccjit(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylGccjit,owner), // is that thread-safe?
    _gji_ctxt(rps_gccjit_top_ctxt),
    _gji_rpsobj2jit()
{
#warning incomplete Rps_PayloadGccjit::Rps_PayloadGccjit
} // end of Rps_PayloadGccjit::Rps_PayloadGccjit



///////////// builtin types
gccjit::type
Rps_PayloadGccjit::raw_get_gccjit_builtin_type(enum gcc_jit_types gcty)
{
  RPS_ASSERT(owner());
  return _gji_ctxt.get_type(gcty);
} // end Rps_PayloadGccjit::raw_get_gccjit_builtin_type

gccjit::type
Rps_PayloadGccjit::locked_get_gccjit_builtin_type(enum gcc_jit_types gcty)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_builtin_type(gcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_builtin_type



///////////// pointer types
gccjit::type
Rps_PayloadGccjit::raw_get_gccjit_pointer_type(gccjit::type srcty)
{
  RPS_ASSERT(owner());
  return srcty.get_pointer();
} // end Rps_PayloadGccjit::raw_get_gccjit_pointer_type

gccjit::type
Rps_PayloadGccjit::locked_get_gccjit_pointer_type(gccjit::type srcty)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_pointer_type(srcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_pointer_type




///////////// constant types
gccjit::type
Rps_PayloadGccjit::raw_get_gccjit_const_type(gccjit::type srcty)
{
  RPS_ASSERT(owner());
  return srcty.get_const();
} // end Rps_PayloadGccjit::raw_get_gccjit_const_type

gccjit::type
Rps_PayloadGccjit::locked_get_gccjit_const_type(gccjit::type srcty)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_const_type(srcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_const_type


///////////// volatile types
gccjit::type
Rps_PayloadGccjit::raw_get_gccjit_volatile_type(gccjit::type srcty)
{
  RPS_ASSERT(owner());
  return srcty.get_volatile();
} // end Rps_PayloadGccjit::raw_get_gccjit_volatile_type

gccjit::type
Rps_PayloadGccjit::locked_get_gccjit_volatile_type(gccjit::type srcty)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_volatile_type(srcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_volatile_type

///////////// aligned types, the alignment is a power of two
gccjit::type
Rps_PayloadGccjit::raw_get_gccjit_aligned_type(gccjit::type srcty, size_t alignment)
{
  RPS_ASSERT(owner());
  RPS_ASSERT((alignment-1)&alignment==0);
  return srcty.get_aligned(alignment);
} // end Rps_PayloadGccjit::raw_get_gccjit_aligned_type

gccjit::type
Rps_PayloadGccjit::locked_get_gccjit_aligned_type(gccjit::type srcty, size_t alignment)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_aligned_type(srcty, alignment);
} // end Rps_PayloadGccjit::locked_get_gccjit_aligned_type

/// Array types are global but could have a location
gccjit::type
Rps_PayloadGccjit::raw_new_gccjit_array_type(gccjit::type elemtype, int nbelem)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(nbelem>=0);
  return  _gji_ctxt.new_array_type(elemtype,nbelem);
}// end Rps_PayloadGccjit::raw_new_gccjit_array_type

gccjit::type
Rps_PayloadGccjit::locked_new_gccjit_array_type(gccjit::type elemtype, int nbelem)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_array_type(elemtype,nbelem);
} // end Rps_PayloadGccjit::locked_new_gccjit_array_type

gccjit::type
Rps_PayloadGccjit::raw_new_gccjit_array_type(gccjit::type elemtype, int nbelem, gccjit::location loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(nbelem>=0);
  return  _gji_ctxt.new_array_type(elemtype,nbelem,loc);
}// end Rps_PayloadGccjit::raw_new_gccjit_array_type

gccjit::type
Rps_PayloadGccjit::locked_new_gccjit_array_type(gccjit::type elemtype, int nbelem, gccjit::location loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_array_type(elemtype,nbelem,loc);
} // end Rps_PayloadGccjit::locked_new_gccjit_array_type


/// Opaque struct types are global but could have a location
gccjit::struct_
Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type(const std::string&strname)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(!strname.empty());
  return  _gji_ctxt.new_opaque_struct_type(strname);
}// end Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type

gccjit::struct_
Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type(const std::string&strname)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_opaque_struct_type(strname);
} // end Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type

gccjit::struct_
Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type(const std::string&strname, gccjit::location loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(!strname.empty());
  return  _gji_ctxt.new_opaque_struct_type(strname,loc);
}// end Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type

gccjit::struct_
Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type(const std::string&strname, gccjit::location loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_opaque_struct_type(strname,loc);
} // end Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type



/// Opaque struct types are global; they could be defined by a
/// RefPerSys object, but could have a location
gccjit::struct_
Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type(const Rps_ObjectRef ob)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(ob);
  return  _gji_ctxt.new_opaque_struct_type(ob->oid().to_string());
}// end Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type

gccjit::struct_
Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type(const Rps_ObjectRef ob)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  gccjit::struct_ newst= raw_new_gccjit_opaque_struct_type(ob);
  raw_register_object_jit(ob,newst);
  return newst;
} // end Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type



////////////////////////////////////////////////////////////////
////// managing RefPerSys objects and their gccjit
void
Rps_PayloadGccjit::raw_register_object_jit(Rps_ObjectRef ob,  const gccjit::object jit)
{
  RPS_ASSERT(ob);
  RPS_ASSERT(owner());
  _gji_rpsobj2jit.insert({ob,jit});
} // end protected Rps_PayloadGccjit::raw_register_object_jit

void
Rps_PayloadGccjit::locked_register_object_jit(Rps_ObjectRef ob,  gccjit::object jit)
{
  RPS_ASSERT(ob);
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  std::lock_guard<std::recursive_mutex> guob(*ob->objmtxptr());
  raw_register_object_jit(ob, jit);
} // end Rps_PayloadGccjit::locked_register_object_jit

void
Rps_PayloadGccjit::raw_unregister_object_jit(Rps_ObjectRef ob)
{
  RPS_ASSERT(ob);
  RPS_ASSERT(owner());
  _gji_rpsobj2jit.erase(ob);
} // end protected Rps_PayloadGccjit::raw_unregister_object_jit


void
Rps_PayloadGccjit::locked_unregister_object_jit(Rps_ObjectRef ob)
{
  RPS_ASSERT(ob);
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  std::lock_guard<std::recursive_mutex> guob(*ob->objmtxptr());
  raw_unregister_object_jit(ob);
} // end Rps_PayloadGccjit::locked_unregister_object_jit


gccjit::location
Rps_PayloadGccjit::make_rpsobj_location(Rps_ObjectRef ob, int line, int col)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  RPS_ASSERT(ob);
  std::lock_guard<std::recursive_mutex> guob(*ob->objmtxptr());
  char cbuf[24];
  memset(cbuf, 0, sizeof(cbuf));
  ob->oid().to_cbuf24(cbuf);
  return  _gji_ctxt.gccjit::context::new_location(cbuf, line, col);
} // end Rps_PayloadGccjit::make_rpsobj_location

/* Should transform a JSON value into data relevant to GCCJIT and
   create if needed the necessary code. */
void
Rps_PayloadGccjit::load_jit_json(Rps_Loader*ld, Rps_Id spacid, unsigned lineno, Json::Value&jseq)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  if (jseq.type() != Json::objectValue)
    {
      RPS_WARNOUT("Rps_PayloadGccjit::load_jit_json bad jseq=" << jseq
                  << "in spacid=" << spacid
                  << " lineno=" << lineno);
    };
  RPS_FATALOUT("unimplemented Rps_PayloadGccjit::load_jit_json spacid="
               << spacid << " lineno=" << lineno << " jseq=" << jseq);
#warning unimplemented load_jit_json
} // end Rps_PayloadGccjit::load_jit_json


void
Rps_PayloadGccjit::gc_mark(Rps_GarbageCollector&gc) const
{
  for (auto it: _gji_rpsobj2jit)
    {
      Rps_ObjectRef obr = it.first;
      RPS_ASSERT(obr);
      obr->gc_mark(gc);
    }
#warning incomplete Rps_PayloadGccjit::gc_mark
} // end Rps_PayloadGccjit::gc_mark

void
Rps_PayloadGccjit::dump_scan(Rps_Dumper*du) const
{
  for (auto it: _gji_rpsobj2jit)
    {
      Rps_ObjectRef obr = it.first;
      RPS_ASSERT(obr);
      rps_dump_scan_object(du, obr);
    };
#warning incomplete Rps_PayloadGccjit::dump_scan
} // end Rps_PayloadGccjit::dump_scan

void
Rps_PayloadGccjit::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  Json::Value jarr(Json::arrayValue);
  for (auto it: _gji_rpsobj2jit)
    {
      Rps_ObjectRef obr = it.first;
      RPS_ASSERT(obr);
      Json::Value job = rps_dump_json_objectref(du, obr);
      /// TODO: dump obr and add it somehow to jv
      if (job)
        {
          jarr.append(job);
        }
    };
  jv["payload"] = "gccjit"; // since rpsldpy_gccjit exists
  jv["jitseq"] = jarr;
#warning incomplete Rps_PayloadGccjit::dump_json_content
} // end Rps_PayloadGccjit::dump_json_content

Rps_PayloadGccjit::~Rps_PayloadGccjit()
{
  _gji_rpsobj2jit.clear();
} // end of Rps_PayloadGccjit::~Rps_PayloadGccjit

/// loading of Gccjit payload; see above
///Rps_PayloadGccjit::dump_json_content

void rpsldpy_gccjit(Rps_ObjectZone*obz, Rps_Loader*ld, const Json::Value& jv, Rps_Id spacid, unsigned lineno)
{
  RPS_ASSERT(obz != nullptr);
  RPS_ASSERT(ld != nullptr);
  RPS_ASSERT(obz->get_payload() == nullptr);
  RPS_ASSERT(jv.type() == Json::objectValue);
  RPS_DEBUG_LOG(LOAD,"start rpsldpy_gccjit obz=" << obz
                << " jv=" << jv
                << " spacid=" << spacid
                << " lineno=" << lineno);
  auto paylgccj= obz->put_new_plain_payload<Rps_PayloadGccjit>();
  Json::Value jseq = jv["jitseq"];
  RPS_ASSERT(paylgccj);
  /// should iterate on jseq and create appropriate libgccjit data
  if (jseq.type() == Json::arrayValue)
    {
      unsigned seqsiz = jseq.size();
      for (unsigned ix=0; ix<seqsiz; ix++)
        {
          Json::Value jvcurelem = jseq[ix];
          paylgccj->load_jit_json(ld,spacid, lineno, jvcurelem);
        }
    }
  else
    {
      char spacename[24];
      memset (spacename, 0, sizeof(spacename));
      spacid.to_cbuf24(spacename);
      RPS_WARN("bad jitseq JSON attribute for gccjit data in space %s:%d",
               spacename, lineno);
    }
#warning incomplete rpsldpy_gccjit
} // end of rpsldpy_gccjit


void
rps_gccjit_initialize(void)
{
  RPS_ASSERT(rps_is_main_thread());
  rps_gccjit_top_ctxt = gccjit::context::acquire();
} // end rps_gccjit_initialize

static volatile std::atomic_flag rps_gccjit_finalized = ATOMIC_FLAG_INIT;

/// the finalize routine is called thru at exit
void rps_gccjit_finalize(void)
{
  if (std::atomic_flag_test_and_set(&rps_gccjit_finalized))
    return;
  rps_gccjit_top_ctxt.gccjit::context::release();
#warning rps_gccjit_finalize incomplete
} // end rps_gccjit_finalize

// end of file gccjit_rps.cc

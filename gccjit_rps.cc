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

// the C++ API of libgccjit is likely to be deprecated in Dec. 2024
// See https://gcc.gnu.org/pipermail/jit/2024q4/001955.html
//#include "libgccjit++.h"

extern "C" const char rps_gccjit_gitid[];
const char rps_gccjit_gitid[]= RPS_GITID;

extern "C" const char rps_gccjit_date[];
const char rps_gccjit_date[]= __DATE__;

extern "C" const char rps_gccjit_shortgitid[];
const char rps_gccjit_shortgitid[]= RPS_SHORTGITID;

/// We use the C API to GCCJIT and conventionally explicit with struct
/// keyword all the opaque structures in it.
extern "C" struct gcc_jit_context* rps_gccjit_top_ctxt;
struct gcc_jit_context* rps_gccjit_top_ctxt;

extern "C" const std::string rps_gccjit_prefix_struct;
const std::string rps_gccjit_prefix_struct="_rps_STRUCT";

extern "C" const std::string rps_gccjit_prefix_field;
const std::string rps_gccjit_prefix_field="_rps_FIELD";

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
  struct gcc_jit_context* _gji_ctxt;  // child context for code generation
  /// We maintain a mapping between RefPerSys objects representing
  /// code and the gccjit::object-s for them.
  /// TODO: document the representation of GCCJIT code.
  std::map<Rps_ObjectRef, struct gcc_jit_object*> _gji_rpsobj2jit;
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
  struct gcc_jit_location* make_csrc_location(const char*filename, int line, int col)
  {
    RPS_ASSERT(filename);
    RPS_ASSERT(filename[0] != '_');
    std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
    return gcc_jit_context_new_location(_gji_ctxt, filename, line, col);
  };
  struct gcc_jit_location* make_string_src_location(const std::string&filen, int line, int col)
  {
    RPS_ASSERT(!filen.empty());
    RPS_ASSERT(filen[0] != '_');
    std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
    return  make_csrc_location(filen.c_str(), line, col);
  };
  struct gcc_jit_location* no_src_location(void) const
  {
    return nullptr;
  };
  // an arbitrary refpersys object may represent a fictuous "source file"
  struct gcc_jit_location* make_rpsobj_location(Rps_ObjectRef ob, int line, int col=0);
  void locked_register_object_jit(Rps_ObjectRef ob,  struct gcc_jit_object* jit);
  void locked_unregister_object_jit(Rps_ObjectRef ob);
protected:
  void load_jit_json(Rps_Loader*ld, Rps_Id spacid, unsigned lineno, Json::Value&jseq);
  void raw_register_object_jit(Rps_ObjectRef ob, struct gcc_jit_object* jit);
  void raw_unregister_object_jit(Rps_ObjectRef ob);
  ///
  //////////////// GCCJIT TYPES
  /// Member functions for making a type; the raw versions don't lock
  /// the owner object, the locked ones do... Names are inspired by
  /// those in https://gcc.gnu.org/onlinedocs/jit/topics/types.html
  struct gcc_jit_type* raw_get_gccjit_builtin_type(enum gcc_jit_types);
  struct gcc_jit_type* locked_get_gccjit_builtin_type(enum gcc_jit_types);
  //// pointer, const, volatile, aligned types of a given type
  struct gcc_jit_type* raw_get_gccjit_pointer_type(struct gcc_jit_type*);
  struct gcc_jit_type* locked_get_gccjit_pointer_type(struct gcc_jit_type*);
  struct gcc_jit_type* raw_get_gccjit_const_type(struct gcc_jit_type*);
  struct gcc_jit_type* locked_get_gccjit_const_type(struct gcc_jit_type*);
  struct gcc_jit_type* raw_get_gccjit_volatile_type(struct gcc_jit_type*);
  struct gcc_jit_type* locked_get_gccjit_volatile_type(struct gcc_jit_type*);
  struct gcc_jit_type* raw_get_gccjit_aligned_type(struct gcc_jit_type*,size_t alignment);
  struct gcc_jit_type* locked_get_gccjit_aligned_type(struct gcc_jit_type*, size_t alignment);
  /// array type of a given type with or without location
  struct gcc_jit_type* raw_new_gccjit_array_type(struct gcc_jit_type* elemtype, int nbelem, struct gcc_jit_location* loc=nullptr);
  struct gcc_jit_type* locked_new_gccjit_array_type(struct gcc_jit_type* elemtype, int nbelem, struct gcc_jit_location* loc=nullptr);
  /// NB. The opaque struct below can be given a vector of fields using raw_gccjit_set_struct_fields or locked_gccjit_set_strict_fields
  /// opaque struct of a given name with or without location
  struct gcc_jit_struct* raw_new_gccjit_opaque_struct(const std::string&name, struct gcc_jit_location*loc=nullptr);
  struct gcc_jit_struct* locked_new_gccjit_opaque_struct(const std::string&name, struct gcc_jit_location* loc=nullptr);
  /// opaque struct of a given object with or without location; in
  /// the locked member functions variants the RefPerSys object is
  /// registered...
  struct gcc_jit_struct* raw_new_gccjit_opaque_struct(const Rps_ObjectRef ob, struct gcc_jit_location* loc=nullptr);
  struct gcc_jit_struct* locked_new_gccjit_opaque_struct(const Rps_ObjectRef ob, struct gcc_jit_location* loc=nullptr);
  /// create a field
  struct gcc_jit_field* raw_new_gccjit_field(struct gcc_jit_type* type, const std::string&name, struct gcc_jit_location* loc=nullptr);
  struct gcc_jit_field* locked_new_gccjit_field(struct gcc_jit_type* type, const std::string&name, struct gcc_jit_location* loc=nullptr);
  /// create a field
  struct gcc_jit_field* raw_new_gccjit_field(struct gcc_jit_type* type, const Rps_ObjectRef obf, struct gcc_jit_location* loc=nullptr);
  struct gcc_jit_field* locked_new_gccjit_field(struct gcc_jit_type* type, const Rps_ObjectRef obf, struct gcc_jit_location* loc=nullptr);
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
struct gcc_jit_type*
Rps_PayloadGccjit::raw_get_gccjit_builtin_type(enum gcc_jit_types gcty)
{
  RPS_ASSERT(owner());
  return gcc_jit_context_get_type(_gji_ctxt,gcty);
} // end Rps_PayloadGccjit::raw_get_gccjit_builtin_type

struct gcc_jit_type*
Rps_PayloadGccjit::locked_get_gccjit_builtin_type(enum gcc_jit_types gcty)
{
  RPS_ASSERT(owner());
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_builtin_type(gcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_builtin_type



///////////// pointer types
struct gcc_jit_type*
Rps_PayloadGccjit::raw_get_gccjit_pointer_type(struct gcc_jit_type* srcty)
{
  RPS_ASSERT(owner());
  return gcc_jit_type_get_pointer(srcty);
} // end Rps_PayloadGccjit::raw_get_gccjit_pointer_type

struct gcc_jit_type*
Rps_PayloadGccjit::locked_get_gccjit_pointer_type(struct gcc_jit_type* srcty)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_pointer_type(srcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_pointer_type




///////////// constant types
struct gcc_jit_type*
Rps_PayloadGccjit::raw_get_gccjit_const_type(struct gcc_jit_type* srcty)
{
  RPS_ASSERT(owner());
  return gcc_jit_type_get_const(srcty);
} // end Rps_PayloadGccjit::raw_get_gccjit_const_type

struct gcc_jit_type*
Rps_PayloadGccjit::locked_get_gccjit_const_type(struct gcc_jit_type* srcty)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_const_type(srcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_const_type


///////////// volatile types
struct gcc_jit_type*
Rps_PayloadGccjit::raw_get_gccjit_volatile_type(struct gcc_jit_type* srcty)
{
  RPS_ASSERT(owner());
  return gcc_jit_type_get_volatile(srcty);
} // end Rps_PayloadGccjit::raw_get_gccjit_volatile_type

struct gcc_jit_type*
Rps_PayloadGccjit::locked_get_gccjit_volatile_type(struct gcc_jit_type* srcty)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_volatile_type(srcty);
} // end Rps_PayloadGccjit::locked_get_gccjit_volatile_type

///////////// aligned types, the alignment is a power of two
struct gcc_jit_type*
Rps_PayloadGccjit::raw_get_gccjit_aligned_type(struct gcc_jit_type* srcty, size_t alignment)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(((alignment-1)&alignment)==0);
  return gcc_jit_type_get_aligned(srcty,alignment);
} // end Rps_PayloadGccjit::raw_get_gccjit_aligned_type

struct gcc_jit_type*
Rps_PayloadGccjit::locked_get_gccjit_aligned_type(struct gcc_jit_type* srcty, size_t alignment)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_get_gccjit_aligned_type(srcty, alignment);
} // end Rps_PayloadGccjit::locked_get_gccjit_aligned_type


struct gcc_jit_type*
Rps_PayloadGccjit::raw_new_gccjit_array_type(struct gcc_jit_type* elemtype, int nbelem, struct gcc_jit_location* loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(nbelem>=0);
  return  gcc_jit_context_new_array_type(_gji_ctxt,loc,elemtype,nbelem);
}// end Rps_PayloadGccjit::raw_new_gccjit_array_type

struct gcc_jit_type*
Rps_PayloadGccjit::locked_new_gccjit_array_type(struct gcc_jit_type* elemtype, int nbelem, struct gcc_jit_location* loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_array_type(elemtype,nbelem,loc);
} // end Rps_PayloadGccjit::locked_new_gccjit_array_type


/// Opaque struct types are global but could have a location
struct gcc_jit_struct*
  Rps_PayloadGccjit::raw_new_gccjit_opaque_struct(const std::string&strname, struct gcc_jit_location*loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(!strname.empty());
  return  gcc_jit_context_new_opaque_struct(_gji_ctxt,loc,strname.c_str());
}// end Rps_PayloadGccjit::raw_new_gccjit_opaque_struct_type

struct gcc_jit_struct*
  Rps_PayloadGccjit::locked_new_gccjit_opaque_struct(const std::string&strname, struct gcc_jit_location* loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_opaque_struct(strname,loc);
} // end Rps_PayloadGccjit::locked_new_gccjit_opaque_struct



/// Opaque struct types are global; they could be defined by a
/// RefPerSys object, but could have a location
struct gcc_jit_struct*
  Rps_PayloadGccjit::raw_new_gccjit_opaque_struct(const Rps_ObjectRef ob, struct gcc_jit_location* loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(ob);
  /// the prefix is rps_stru defined before class Rps_PayloadGccjit
  return  gcc_jit_context_new_opaque_struct(_gji_ctxt, loc, (rps_gccjit_prefix_struct+ob->oid().to_string()).c_str());
} // end Rps_PayloadGccjit::raw_new_gccjit_opaque_struct

struct gcc_jit_struct*
  Rps_PayloadGccjit::locked_new_gccjit_opaque_struct(const Rps_ObjectRef ob, struct gcc_jit_location* loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  struct gcc_jit_struct* newst= raw_new_gccjit_opaque_struct(ob, loc);
  raw_register_object_jit(ob,gcc_jit_type_as_object(gcc_jit_struct_as_type (newst)));
  return newst;
} // end Rps_PayloadGccjit::locked_new_gccjit_opaque_struct_type


struct gcc_jit_field*
Rps_PayloadGccjit::raw_new_gccjit_field(struct gcc_jit_type* type, const std::string&name, struct gcc_jit_location* loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(!name.empty());
  return  gcc_jit_context_new_field(_gji_ctxt, loc, type, name.c_str());
} // end Rps_PayloadGccjit::raw_new_gccjit_field

struct gcc_jit_field*
Rps_PayloadGccjit::raw_new_gccjit_field(struct gcc_jit_type* type, const Rps_ObjectRef obf, struct gcc_jit_location* loc)
{
  RPS_ASSERT(owner());
  RPS_ASSERT(obf);
  std::string fstr = rps_gccjit_prefix_field+obf->oid().to_string();
  return  gcc_jit_context_new_field(_gji_ctxt, loc, type, fstr.c_str());
} // end Rps_PayloadGccjit::raw_new_gccjit_field

struct gcc_jit_field*
Rps_PayloadGccjit::locked_new_gccjit_field(struct gcc_jit_type* type, const std::string&name, struct gcc_jit_location* loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_field(type, name, loc);
} // end Rps_PayloadGccjit::locked_new_gccjit_field


struct gcc_jit_field*
Rps_PayloadGccjit::locked_new_gccjit_field(struct gcc_jit_type* type, const Rps_ObjectRef obf, struct gcc_jit_location* loc)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  return raw_new_gccjit_field(type, obf, loc);
} // end Rps_PayloadGccjit::locked_new_gccjit_field


////////////////////////////////////////////////////////////////
////// managing RefPerSys objects and their gccjit
void
Rps_PayloadGccjit::raw_register_object_jit(Rps_ObjectRef ob,  struct gcc_jit_object* jit)
{
  RPS_ASSERT(ob);
  RPS_ASSERT(owner());
  _gji_rpsobj2jit.insert(std::pair{ob,jit});
} // end protected Rps_PayloadGccjit::raw_register_object_jit

void
Rps_PayloadGccjit::locked_register_object_jit(Rps_ObjectRef ob,  struct gcc_jit_object* jit)
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


struct gcc_jit_location*
Rps_PayloadGccjit::make_rpsobj_location(Rps_ObjectRef ob, int line, int col)
{
  std::lock_guard<std::recursive_mutex> guown(*owner()->objmtxptr());
  RPS_ASSERT(ob);
  std::lock_guard<std::recursive_mutex> guob(*ob->objmtxptr());
  char cbuf[24];
  memset(cbuf, 0, sizeof(cbuf));
  ob->oid().to_cbuf24(cbuf);
  return  gcc_jit_context_new_location(_gji_ctxt, cbuf, line, col);
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
  rps_gccjit_top_ctxt = gcc_jit_context_acquire  ();
} // end rps_gccjit_initialize

static volatile std::atomic_flag rps_gccjit_finalized = ATOMIC_FLAG_INIT;

/// the finalize routine is called thru at exit
void rps_gccjit_finalize(void)
{
  if (std::atomic_flag_test_and_set(&rps_gccjit_finalized))
    return;
  gcc_jit_context_release(rps_gccjit_top_ctxt);
#warning rps_gccjit_finalize incomplete
} // end rps_gccjit_finalize

// end of file gccjit_rps.cc

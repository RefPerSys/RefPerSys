/****************************************************************
 * file lightgen_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for machine code generation using GNU
 *      lightning.  See also https://www.gnu.org/software/lightning/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2023 - 2023 The Reflective Persistent System Team
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
 ************************************************************************/

#include "refpersys.hh"

/// GNU lightning implementation header.
/// See https://www.gnu.org/software/lightning/

/// a GNU lightning library after 2.2.2 (or GNU lightning commit
/// 3b0fff9206a458d7e11db of August 21, 2023) is required.


/// GNU lightning
#include "lightning.h"

extern "C" const char rps_lightgen_gitid[];
const char rps_lightgen_gitid[]= RPS_GITID;

extern "C" const char rps_lightgen_date[];
const char rps_lightgen_date[]= __DATE__;




/// According to www.gnu.org/software/lightning/manual/lightning.html
/// every GNU lightning macro uses the _jit identifier... The type of
/// that identifier is a pointer to the abstract jit_state_t ...

/// temporary payload for GNU lightning code generation:
class Rps_PayloadLighntingCodeGen : public Rps_Payload
{
  friend Rps_PayloadLighntingCodeGen*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadLighntingCodeGen,Rps_ObjectZone*>(Rps_ObjectZone*);
  virtual ~Rps_PayloadLighntingCodeGen();
  jit_state_t* lightg_jist;
#warning PayloadLighntingCodeGen may need some jit_state_t* pointer
protected:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadLighntingCodeGen(Rps_ObjectZone*owner);
  Rps_PayloadLighntingCodeGen(Rps_ObjectRef obr) :
    Rps_PayloadLighntingCodeGen(obr?obr.optr():nullptr) {};
  virtual const std::string payload_type_name(void) const
  {
    return "lightning_code_generator";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
};        // end class Rps_PayloadLightningCodeGen

Rps_PayloadLighntingCodeGen::Rps_PayloadLighntingCodeGen(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylLightCodeGen,owner), lightg_jist(nullptr)
{
} // end of Rps_PayloadLighntingCodeGen::Rps_PayloadLighntingCodeGen

Rps_PayloadLighntingCodeGen::~Rps_PayloadLighntingCodeGen()
{
} // end destructor Rps_PayloadLighntingCodeGen::~Rps_PayloadLighntingCodeGen

void
Rps_PayloadLighntingCodeGen::gc_mark(Rps_GarbageCollector&gc) const
{
} // end of Rps_PayloadLighntingCodeGen::gc_mark

void
Rps_PayloadLighntingCodeGen::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du);
  RPS_POSSIBLE_BREAKPOINT();
} // end Rps_PayloadLighntingCodeGen::dump_scan

void
Rps_PayloadLighntingCodeGen::dump_json_content(Rps_Dumper*du, Json::Value&jv) const
{
  RPS_ASSERT(du);
  RPS_POSSIBLE_BREAKPOINT();
} // end Rps_PayloadLighntingCodeGen::dump_json_content

//// return true on successful code generation
bool
rps_generate_lightning_code(Rps_CallFrame*callerframe,
                            Rps_ObjectRef obmodule)
{
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(obmodule);
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obmodule;
                 Rps_ObjectRef obgenerator;
                );
  _f.obmodule = obmodule;
  std::lock_guard<std::recursive_mutex> gumodule(*obmodule->objmtxptr());
  _f.obgenerator =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_6SM7PykipQW01HVClH) //midend_lightning_code_generator∈class
                              );
  _f.obgenerator->put_new_plain_payload<Rps_PayloadLighntingCodeGen>();
  _f.obgenerator->put_attr(RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC), //"code_module"∈named_attribute
                           _f.obmodule);
  RPS_FATALOUT("unimplemented rps_generate_lightning_code obmodule="
               << obmodule << " obgenerator=" << _f.obgenerator);
#warning unimplemented rps_generate_lightning_code
} // end rps_generate_lightning_code

#warning incomplete lightgen_rps.cc file

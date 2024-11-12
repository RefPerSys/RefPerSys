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

/// temporary payload for GNU libgccjit code generation:
class Rps_PayloadGccjitCodeGen : public Rps_Payload
{
  friend Rps_PayloadGccjitCodeGen*
  Rps_QuasiZone::rps_allocate1<Rps_PayloadGccjitCodeGen,Rps_ObjectZone*>(Rps_ObjectZone*);
public:
  virtual void gc_mark(Rps_GarbageCollector&gc) const;
  virtual void dump_scan(Rps_Dumper*du) const;
  virtual void dump_json_content(Rps_Dumper*, Json::Value&) const;
  inline Rps_PayloadGccjitCodeGen(Rps_ObjectZone*owner);
  Rps_PayloadGccjitCodeGen(Rps_ObjectRef obr) :
    Rps_PayloadGccjitCodeGen(obr?obr.optr():nullptr) {};
  virtual const std::string payload_type_name(void) const
  {
    return "gccjitcode_generator";
  };
  virtual uint32_t wordsize(void) const
  {
    return sizeof(*this)/sizeof(void*);
  };
  virtual bool is_erasable(void) const
  {
    return false;
  };
};				// end class Rps_PayloadGccjitCodeGen

Rps_PayloadGccjitCodeGen::Rps_PayloadGccjitCodeGen(Rps_ObjectZone*owner)
  : Rps_Payload(Rps_Type::PaylGccjitCodeGen,owner)
{
} // end of Rps_PayloadGccjitCodeGen::Rps_PayloadGccjitCodeGen

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
#warning rps_gccjit_finalize unimplemented
} // end rps_gccjit_finalize

// end of file gccjit_rps.cc

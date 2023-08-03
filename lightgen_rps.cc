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

/// GNU lightning implementation header

/// FIXME for https://lists.gnu.org/archive/html/lightning/2023-08/msg00004.html
#define protected lightning_protected
#include "jit_private.h"
#undef protected

extern "C" const char rps_lightgen_gitid[];
const char rps_lightgen_gitid[]= RPS_GITID;

extern "C" const char rps_lightgen_date[];
const char rps_lightgen_date[]= __DATE__;


/// https://lists.gnu.org/archive/html/lightning/2023-08/msg00004.html
extern "C" const int rps_gnulightning_jitstate_size;
const int rps_gnulightning_jitstate_size = sizeof(jit_state);
extern "C" const int rps_gnulightning_jitstate_align;
const int rps_gnulightning_jitstate_align = alignof(jit_state);

/// According to www.gnu.org/software/lightning/manual/lightning.html
/// every GNU lightning macro uses the _jit identifier... The type of
/// that identifier is a pointer to the abstract jit_state_t ...

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
#warning TODO: improve class of generator in rps_generate_lightning_code
                               /* FIXME: we probably need a lightning_generator class */
                               RPS_ROOT_OB(_5yhJGgxLwLp00X0xEQ)//object∈class
                              );
  RPS_FATALOUT("unimplemented rps_generate_lightning_code obmodule="
               << obmodule << " obgenerator=" << _f.obgenerator);
#warning unimplemented rps_generate_lightning_code
} // end rps_generate_lightning_code

#warning incomplete lightgen_rps.cc file

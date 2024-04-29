/****************************************************************
 * file cppgen_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the code for C++ code generation.
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
 *    See also companion file lightgen_rps.cc for GNU lightning
 *    code generation.
 ******************************************************************************/

#include "refpersys.hh"



extern "C" const char rps_cppgen_gitid[];
const char rps_cppgen_gitid[]= RPS_GITID;

extern "C" const char rps_cppgen_date[];
const char rps_cppgen_date[]= __DATE__;

//// return true on successful C++ code generation
bool
rps_generate_cplusplus_code(Rps_CallFrame*callerframe,
                            Rps_ObjectRef obmodule)
{
  RPS_LOCALFRAME(nullptr,
                 callerframe,
                 Rps_ObjectRef obmodule;
                 Rps_ObjectRef obgenerator;
                 Rps_ObjectRef obincludeset;
                 Rps_ObjectRef obcurinclude;
                 Rps_ClosureValue vclos;
                 Rps_Value vinclude;
                 Rps_Value vold;
                 Rps_Value vmainres;
                 Rps_Value vxtrares;
                 Rps_Value vtype;
                );
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_ASSERT(obmodule);
  _f.obmodule = obmodule;
  std::lock_guard<std::recursive_mutex> gumodule(*obmodule->objmtxptr());
  _f.obgenerator =
    Rps_ObjectRef::make_object(&_,
                               RPS_ROOT_OB(_2yzD3HZ6VQc038ekBU)//midend_cplusplus_code_generator∈class
                              );
  _f.obgenerator->put_attr(RPS_ROOT_OB(_2Xfl3YNgZg900K6zdC), //"code_module"∈named_attribute
                           _f.obmodule);
  /**
     The "include" attribute of the module describes how would the
     #include-s in the generated C++ file be obtained and generated.

     We need to document more.... **/
  _f.vinclude =
    _f.obmodule->get_attr1(&_,
                           RPS_ROOT_OB(_0XF2N1YQ87p02GXXir)); //"include"∈named_attribute
  RPS_DEBUG_LOG(CODEGEN,"rps_generate_cplusplus_code start obmodule=" << _f.obmodule
                << " obgenerator=" << _f.obgenerator << " vinclude=" << _f.vinclude);
  /**
     TODO complete here:

     if vinclude is a set or sequence, use it;
     if it is a closure, apply it to the module and the generator

     WHAT ELSE

     How to deal with the (varying) set of types, of C++ routines, of
     C++ struct-s or class-es

  **/
  if (_f.vinclude.is_closure())
    {
      _f.vold = _f.vinclude;
      _f.vclos = Rps_ClosureValue(_f.vinclude);
      Rps_TwoValues tv = //
        _f.vclos.apply2(&_, _f.obmodule, _f.obgenerator);
      _f.vinclude = tv.main();
      _f.vxtrares = tv.xtra();
      RPS_DEBUG_LOG(CODEGEN,
                    "rps_generate_cplusplus_code computed include "
                    << _f.vinclude << " with closure=" << _f.vold
                    << " obmodule=" << _f.obmodule
                    << " obgenerator=" << _f.obgenerator);
    }
  else
    RPS_DEBUG_LOG(CODEGEN,
                  "rps_generate_cplusplus_code plain include "
                  << _f.vinclude << " obmodule=" << _f.obmodule
                  << " obgenerator=" << _f.obgenerator);
  if (_f.vinclude.is_set())
    {
      unsigned cardinclset = _f.vinclude.as_set()->cardinal();
#warning  rps_generate_cplusplus_code should handle set of includes
      throw RPS_RUNTIME_ERROR_OUT("rps_generate_cplusplus_code unimplemented set vinclude:"
                                  << _f.vinclude << " obmodule=" << _f.obmodule
                                  << " obgenerator=" << _f.obgenerator);
    }
  else if (_f.vinclude.is_tuple())
    {
      unsigned lenincltup = _f.vinclude.as_tuple()->size();
#warning  rps_generate_cplusplus_code should handle tuple of includes
      throw RPS_RUNTIME_ERROR_OUT("rps_generate_cplusplus_code unimplemented tuple vinclude:"
                                  << _f.vinclude<< " obmodule=" << _f.obmodule
                                  << " obgenerator=" << _f.obgenerator);
    }
  else
    RPS_FATALOUT("in rps_generate_cplusplus_code obmodule="
                 << obmodule << " obgenerator=" << _f.obgenerator
                 << "unexpected include=" << _f.vinclude);
#warning missing code in rps_generate_cplusplus_code
} // end rps_generate_cplusplus_code


#warning incomplete cppgen_rps.cc file

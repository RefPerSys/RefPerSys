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

void
rps_gccjit_initialize(void)
{
  RPS_ASSERT(rps_is_main_thread());
  rps_gccjit_top_ctxt = gccjit::context::acquire();
} // end rps_gccjit_initialize

// end of file gccjit_rps.cc

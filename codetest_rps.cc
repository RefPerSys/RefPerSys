/****************************************************************
 * file RefPerSys/codetest_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has some testing code, to be called only using dlsym
 *
 * Author(s):
 *      Basile Starynkevitch, France <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 The Reflective Persistent System Team
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

extern "C" const char rps_codetest_gitid[];
const char rps_codetest_gitid[]= RPS_GITID;


extern "C" const char rps_codetest_shortgitid[];
const char rps_codetest_shortgitid[]= RPS_SHORTGITID;


extern "C" const char rps_codetest_basename[];
const char rps_codetest_basename[]= RPS_BASENAME;

extern "C" const char rps_codetest_baseid[];
const char rps_codetest_baseid[]= RPS_BASEID;

extern "C" void rps_codetest_fun_from(const char*fil, int lin)
__attribute__((cold));

void
rps_codetest_fun_from(const char*fil, int lin)
{
  RPS_INFORMOUT("rps_codetest_fun_from fil="
                << Rps_QuotedC_String(fil)
                << " lin=" << lin
                << std::endl
                << RPS_FULL_BACKTRACE(1, "rps_codetest_fun_from"));
} // end rps_codetest_fun_from


/// end file RefPerSys/codetest_rps.cc

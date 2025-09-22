/****************************************************************
 * file scripting_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It provides support for scripting (after loading the heap)
 *
 * Author(s):
 *      Basile STARYNKEVITCH (France) <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2025 - 2025 The Reflective Persistent System Team
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
 *********************************************************************/


#include "refpersys.hh"

// comment for our do-scan-refpersys-pkgconfig.c utility
//@@PKGCONFIG gmp
//@@PKGCONFIG gmpxx

extern "C" const char rps_scripting_gitid[];
const char rps_scripting_gitid[]= RPS_GITID;

extern "C" const char rps_scripting_date[];
const char rps_scripting_date[]= __DATE__;

extern "C" const char rps_scripting_shortgitid[];
const char rps_scripting_shortgitid[]= RPS_SHORTGITID;

extern "C" const char rps_scripting_timestamp[];
const char rps_scripting_timestamp[]= __TIMESTAMP__;

#warning missing code in scripting_rps.cc

extern "C" void rps_scripting_help(void);
extern "C" void rps_scripting_add_script(const char*);

extern "C" const char rps_scripting_help_english_text[];


/// vector of real path to script files
static std::vector<const char*> rps_scripts_vector;

const char rps_scripting_help_english_text[] =
  R"help(
A script file is a textual file.
All its initial lines before a line containing REFPERSYS_SCRIPT are ignored.
Hence these initial lines could contain some shell script, etc.
)help"
  ;
#warning more text needed inside rps_scripting_help_english_text



void
rps_scripting_help(void)
{
  RPS_FATALOUT("unimplemented rps_scripting_help" << std::endl
	       << rps_scripting_help_english_text);
#warning rps_scripting_help unimplemented
} // end rps_scripting_help


void
rps_scripting_add_script(const char*path)
{
  if (access(path, R_OK))
    RPS_FATALOUT("script file " << Rps_QuotedC_String(path)
		 << " is not accessible: "
		 << strerror(errno));
  char*rp = realpath(path, nullptr);
  if (rp == path) /*same pointer*/
    rp = strdup(path);
  if (!rp)
    RPS_FATALOUT("realpath(3) of "
		 <<  Rps_QuotedC_String(path) << " failed: "
		 << strerror(errno));
  if (!rps_is_main_thread())
    RPS_FATALOUT("adding script file " << rp << " from non main thread");
  rps_scripts_vector.push_back(rp);
  RPS_INFORMOUT("added script file #" << rps_scripts_vector.size()
		<< ": " << rp);
} // end rps_scripting_add_script


void
rps_run_scripts_after_load(Rps_CallFrame* caller)
{
  if (rps_scripts_vector.empty())
    return;
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
		 caller,
		 Rps_Value strv;
		);
  RPS_WARNOUT("unimplemented rps_run_scripts_after_load for "
	      << rps_scripts_vector.size() << " scripts"
	      << std::endl
	      << RPS_FULL_BACKTRACE_HERE(1, "rps_run_scripts_after_load"));
  /// TODO: loop on the script vector and handle exceptions
#warning unimplemented rps_run_scripts_after_load
} // end rps_run_scripts_after_load

//// end of file scripting_rps.cc

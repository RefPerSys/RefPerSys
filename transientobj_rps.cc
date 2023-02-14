/****************************************************************
 * file transientobj_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      Low-level implementation of transient objects and payloads
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2023 The Reflective Persistent System Team
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

extern "C" const char rps_transientobj_gitid[];
const char rps_transientobj_gitid[]= RPS_GITID;

extern "C" const char rps_transientobj_date[];
const char rps_transientobj_date[]= __DATE__;



////////////////////////////////////////////////////////////////
////// trensient unix process payload
Rps_PayloadUnixProcess::Rps_PayloadUnixProcess(Rps_ObjectZone*owner)  // See PaylUnixProcess
  : Rps_Payload(Rps_Type::PaylUnixProcess,owner),
    unixproc_pid(0),
    unixproc_exe(),
    unixproc_argv()
{
} // end constructor Rps_PayloadUnixProcess

Rps_PayloadUnixProcess::Rps_PayloadUnixProcess(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylUnixProcess,owner),
    unixproc_pid(0),
    unixproc_exe(),
    unixproc_argv()
{
  RPS_FATALOUT("cannot load payload of unix process for owner " << owner);
} // end constructor Rps_PayloadUnixProcess

Rps_PayloadUnixProcess::~Rps_PayloadUnixProcess()
{
} // end destructor Rps_PayloadUnixProcess

void
Rps_PayloadUnixProcess::dump_scan(Rps_Dumper*du)  const
{
  // do nothing, this payload for unix process is transient!
  RPS_ASSERT(du);
} // end Rps_PayloadUnixProcess::dump_scan


void
Rps_PayloadUnixProcess::dump_json_content(Rps_Dumper*du, Json::Value&)  const
{
  // do nothing, this payload for unix process is transient!
  RPS_ASSERT(du);
} // end Rps_PayloadUnixProcess::dump_scan

bool
Rps_PayloadUnixProcess::is_erasable(void) const
{
  return false;
} // end Rps_PayloadUnixProcess::is_erasable

void
Rps_PayloadUnixProcess::gc_mark(Rps_GarbageCollector&gc) const
{
} // end Rps_PayloadUnixProcess::gc_mark


/// static member function to create a dormant (potential, not yet forked) unix process object
Rps_ObjectRef
Rps_PayloadUnixProcess::make_dormant_unix_process_object(Rps_CallFrame*callerframe,
    const std::string& exec, const std::vector<std::string>& progargs)
{
  if (exec.empty())
    throw std::runtime_error("no executable given to make_dormant_unix_process");
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_ROOT_OB(_61uFnhRCXfe00Mir2n), //unix_process∈class
                 callerframe,
                 Rps_ObjectRef obres;
                );
  char *realpath = ::realpath(exec.c_str(), nullptr);
  if (!realpath)
    throw RPS_RUNTIME_ERROR_OUT("cannot make_dormant_unix_process_object from executable " << Rps_QuotedC_String(exec)
                                << " without a real path");
  _f.obres = Rps_ObjectRef::make_object(&_, RPS_ROOT_OB(_61uFnhRCXfe00Mir2n)); //unix_process∈class
  Rps_PayloadUnixProcess* payl = _f.obres->put_new_plain_payload<Rps_PayloadUnixProcess>();
  payl->unixproc_exe = std::string(realpath);
  payl->unixproc_pid = 0;
  payl->unixproc_argv.clear();
  for (std::string curarg: progargs)
    payl->unixproc_argv.push_back(curarg);
  free (realpath);
  return _f.obres;
} // end Rps_PayloadUnixProcess::make_dormant_unix_process_object


///////////////////////////////////////
///// transient popened file payload
Rps_PayloadPopenedFile::Rps_PayloadPopenedFile(Rps_ObjectZone*owner)  // See PaylPopenedFile
  : Rps_Payload(Rps_Type::PaylPopenedFile,owner)
{
} // end constructor Rps_PayloadPopenedFile

Rps_PayloadPopenedFile::Rps_PayloadPopenedFile(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylPopenedFile,owner)
{
  RPS_FATALOUT("cannot load payload of popened file for owner " << owner);
} // end constructor Rps_PayloadUnixProcess

Rps_PayloadPopenedFile::~Rps_PayloadPopenedFile()
{
} // end destructor Rps_PayloadPopenedFile


void
Rps_PayloadPopenedFile::dump_scan(Rps_Dumper*du)  const
{
  // do nothing, this payload for unix process is transient!
  RPS_ASSERT(du);
} // end Rps_PayloadPopenedFile::dump_scan


void
Rps_PayloadPopenedFile::dump_json_content(Rps_Dumper*du, Json::Value&)  const
{
  // do nothing, this payload for unix process is transient!
  RPS_ASSERT(du);
} // end Rps_PayloadPopenedFile::dump_scan

bool
Rps_PayloadPopenedFile::is_erasable(void) const
{
  return false;
} // end Rps_PayloadPopenedFile::is_erasable

void
Rps_PayloadPopenedFile::gc_mark(Rps_GarbageCollector&gc) const
{
} // end Rps_PayloadPopenedFile::gc_mark


#warning transientobj_rps.cc is nearly empty

/*** end of file transientobj_rps.cc ***/

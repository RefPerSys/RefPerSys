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
 *      © Copyright (C) 2023 - 2025 The Reflective Persistent System Team
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

extern "C" const char rps_transientobj_shortgitid[];
const char rps_transientobj_shortgitid[]= RPS_SHORTGITID;

////////////////////////////////////////////////////////////////
////// trensient unix process payload
Rps_PayloadUnixProcess::Rps_PayloadUnixProcess(Rps_ObjectZone*owner)  // See PaylUnixProcess
  : Rps_Payload(Rps_Type::PaylUnixProcess,owner),
    _unixproc_pid(0),
    _unixproc_exe(),
    _unixproc_argv(),
    _unixproc_closure(),
    _unixproc_inputclos(),
    _unixproc_outputclos(),
    _unixproc_pipeinputfd(-1),
    _unixproc_pipeoutputfd(-1),
    _unixproc_cpu_time_limit(0),
    _unixproc_elapsed_time_limit(0),
    _unixproc_start_time(0),
    _unixproc_as_mb_limit(0),
    _unixproc_fsize_mb_limit(0),
    _unixproc_core_mb_limit(0),
    _unixproc_forbid_core(false),
    _unixproc_nofile_limit(0)
{
} // end constructor Rps_PayloadUnixProcess


/// needed but never called
Rps_PayloadUnixProcess::Rps_PayloadUnixProcess(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylUnixProcess,owner),
    _unixproc_pid(0),
    _unixproc_exe(),
    _unixproc_argv(),
    _unixproc_closure(),
    _unixproc_inputclos(),
    _unixproc_outputclos(),
    _unixproc_pipeinputfd(-1),
    _unixproc_pipeoutputfd(-1),
    _unixproc_cpu_time_limit(0),
    _unixproc_elapsed_time_limit(0),
    _unixproc_start_time(0),
    _unixproc_as_mb_limit(0),
    _unixproc_fsize_mb_limit(0),
    _unixproc_core_mb_limit(0),
    _unixproc_forbid_core(false),
    _unixproc_nofile_limit(0)
{
  RPS_FATALOUT("cannot load payload of unix process for owner " << owner);
} // end constructor Rps_PayloadUnixProcess

Rps_PayloadUnixProcess::~Rps_PayloadUnixProcess()
{
} // end destructor Rps_PayloadUnixProcess

void
Rps_PayloadUnixProcess::add_process_argument(const std::string& arg)
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  _unixproc_argv.push_back(arg);
} // end Rps_PayloadUnixProcess::add_process_argument

void
Rps_PayloadUnixProcess::forbid_core_dump(void)
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  pid_t pid = _unixproc_pid.load();
  _unixproc_forbid_core.store(true);
  if (pid >0)
    {
      struct rlimit newlim= {.rlim_cur=0, .rlim_max= RLIM_INFINITY};
      struct rlimit oldlim= {.rlim_cur=0, .rlim_max= 0};
      prlimit(pid, RLIMIT_CORE, &newlim, &oldlim);
    };
} // end Rps_PayloadUnixProcess::forbid_core_dump


unsigned
Rps_PayloadUnixProcess::core_megabytes_limit(unsigned newlimit)
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  pid_t pid = _unixproc_pid.load();
  _unixproc_forbid_core.store(false);
  if (pid >0)
    {
      struct rlimit newlim= {.rlim_cur=((newlimit>0)?(newlimit<<20):RLIM_INFINITY),
               .rlim_max= RLIM_INFINITY
      };
      struct rlimit oldlim= {.rlim_cur=0, .rlim_max= 0};
      (void)prlimit(pid, RLIMIT_CORE, &newlim, &oldlim);
      if (oldlim.rlim_cur < RLIM_INFINITY)
        return oldlim.rlim_cur>>20;
    }
  return _unixproc_core_mb_limit.exchange(newlimit);
} // end Rps_PayloadUnixProcess::core_megabytes_limit

unsigned
Rps_PayloadUnixProcess::address_space_megabytes_limit(unsigned newlimit)
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  pid_t pid = _unixproc_pid.load();
  if (pid >0)
    {
      struct rlimit newlim= {.rlim_cur=(newlimit?(newlimit<<20):RLIM_INFINITY),
               .rlim_max= RLIM_INFINITY
      };
      struct rlimit oldlim= {.rlim_cur=0, .rlim_max= 0};
      if (!prlimit(pid, RLIMIT_AS, &newlim, &oldlim))
        {
          _unixproc_as_mb_limit.store(oldlim.rlim_cur>>20);
          return oldlim.rlim_cur>>20;
        }
    };
  return _unixproc_as_mb_limit.exchange(newlimit);
} // end Rps_PayloadUnixProcess::address_space_megabytes_limit


unsigned
Rps_PayloadUnixProcess::file_size_megabytes_limit(unsigned newlimit)
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  pid_t pid = _unixproc_pid.load();
  if (pid >0)
    {
      struct rlimit newlim= {.rlim_cur=(newlimit?(newlimit<<20):RLIM_INFINITY),
               .rlim_max= RLIM_INFINITY
      };
      struct rlimit oldlim= {.rlim_cur=0, .rlim_max= 0};
      if (!prlimit(pid, RLIMIT_FSIZE, &newlim, &oldlim))
        {
          _unixproc_fsize_mb_limit.store(oldlim.rlim_cur>>20);
          return oldlim.rlim_cur>>20;
        }
    };
  return _unixproc_fsize_mb_limit.exchange(newlimit);
} // end Rps_PayloadUnixProcess::address_space_megabytes_limit


unsigned
Rps_PayloadUnixProcess::nofile_limit(unsigned newlimit)
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  pid_t pid = _unixproc_pid.load();
  if (pid >0)
    {
      struct rlimit newlim= {.rlim_cur=(newlimit?newlimit:RLIM_INFINITY),
               .rlim_max= RLIM_INFINITY
      };
      struct rlimit oldlim= {.rlim_cur=0, .rlim_max= 0};
      if (!prlimit(pid, RLIMIT_NOFILE, &newlim, &oldlim))
        {
          _unixproc_fsize_mb_limit.store(oldlim.rlim_cur);
          return oldlim.rlim_cur;
        }
    };
  return _unixproc_fsize_mb_limit.exchange(newlimit);
} // end Rps_PayloadUnixProcess::nofile_limit



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
  if (_unixproc_closure)
    {
      RPS_ASSERT(_unixproc_closure.is_closure());
      _unixproc_closure.gc_mark(gc,1);
    }
} // end Rps_PayloadUnixProcess::gc_mark

const Rps_ClosureValue
Rps_PayloadUnixProcess::get_process_closure(void) const
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  return _unixproc_closure;
} // end Rps_PayloadUnixProcess::get_process_closure


void
Rps_PayloadUnixProcess::put_process_closure(Rps_ClosureValue closv)
{
  if (!closv || !closv.is_closure()) return;
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  _unixproc_closure = closv;
} // end Rps_PayloadUnixProcess::put_process_closure

const Rps_ClosureValue
Rps_PayloadUnixProcess::get_input_closure(void) const
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  return _unixproc_inputclos;
} // end Rps_PayloadUnixProcess::get_input_closure


void
Rps_PayloadUnixProcess::put_input_closure(Rps_ClosureValue closv)
{
  if (!closv || !closv.is_closure()) return;
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  _unixproc_inputclos = closv;
} // end Rps_PayloadUnixProcess::put_input_closure

const Rps_ClosureValue
Rps_PayloadUnixProcess::get_output_closure(void) const
{
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  return _unixproc_outputclos;
} // end Rps_PayloadUnixProcess::get_output_closure


void
Rps_PayloadUnixProcess::put_output_closure(Rps_ClosureValue closv)
{
  if (!closv || !closv.is_closure()) return;
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  _unixproc_outputclos = closv;
} // end Rps_PayloadUnixProcess::put_output_closure



/// static member function to create a dormant (potential, not yet forked) unix process object
Rps_ObjectRef
Rps_PayloadUnixProcess::make_dormant_unix_process_object(Rps_CallFrame*callerframe,
    const std::string& exec)
{
  if (exec.empty())
    throw std::runtime_error("no executable given to make_dormant_unix_process");
  RPS_ASSERT(callerframe && callerframe->is_good_call_frame());
  RPS_LOCALFRAME(RPS_ROOT_OB(_61uFnhRCXfe00Mir2n), //unix_process∈class
                 callerframe,
                 Rps_ObjectRef obres;
                );
  std::string realexestr;
  if (exec.find("/") > 0)
    {
      char *realexepath = ::realpath(exec.c_str(), nullptr);
      if (!realexepath)
        throw RPS_RUNTIME_ERROR_OUT("cannot make_dormant_unix_process_object from executable " << Rps_QuotedC_String(exec)
                                    << " without a real path for " << exec);
      realexestr = std::string{realexepath};
      free (realexepath);
    }
  else
    {
      const char*curpath = getenv("PATH");
      RPS_ASSERT(curpath);
      const char*pc= curpath;
      const char*next = nullptr;
      do
        {
          std::string dir;
          next = nullptr;
          const char*colon = strchr(pc, ':');
          if (colon)
            {
              next = colon+1;
              dir = std::string(pc, colon-1);
              if (dir.empty())
                dir= std::string{"."};
              dir += '/';
            }
          else
            {
              dir = std::string(pc);
              next= nullptr;
            };
          std::string curexepath = dir + exec;
          if (!access(curexepath.c_str(), X_OK))
            {
              realexestr = curexepath;
              break;
            };
          pc = next;
        }
      while(realexestr.empty());
    };
  _f.obres = Rps_ObjectRef::make_object(&_, RPS_ROOT_OB(_61uFnhRCXfe00Mir2n)); //unix_process∈class
  Rps_PayloadUnixProcess* payl = //
    _f.obres->put_new_plain_payload<Rps_PayloadUnixProcess>(); //
  payl->_unixproc_exe = realexestr;
  payl->_unixproc_argv.push_back(exec);
  return _f.obres;
} // end Rps_PayloadUnixProcess::make_dormant_unix_process_object

std::set<Rps_PayloadUnixProcess*>
Rps_PayloadUnixProcess::set_of_runnable_processes;

std::mutex
Rps_PayloadUnixProcess::mtx_of_runnable_processes;
std::deque<Rps_PayloadUnixProcess*>
Rps_PayloadUnixProcess::queue_of_runnable_processes;

void
Rps_PayloadUnixProcess::gc_mark_active_processes(Rps_GarbageCollector&gc)
{
  std::lock_guard<std::mutex> gu(mtx_of_runnable_processes);
  /// Both set_of_runnable_processes and queue_of_runnable_processes
  /// should contain the same objects, but for sure we want to mark
  /// both.
  for (Rps_PayloadUnixProcess*paylup : set_of_runnable_processes)
    {
      paylup->owner()->gc_mark(gc);
    }
  for (Rps_PayloadUnixProcess*paylup : queue_of_runnable_processes)
    {
      paylup->owner()->gc_mark(gc);
    }
} // end Rps_PayloadUnixProcess::gc_mark_active_processes

void
Rps_PayloadUnixProcess::start_process(Rps_CallFrame*callframe)
{
  RPS_ASSERT(!callframe || callframe->is_good_call_frame());
  std::lock_guard<std::mutex> rungu(mtx_of_runnable_processes);
  std::lock_guard<std::recursive_mutex> gu(*owner()->objmtxptr());
  if (_unixproc_pid.load()>0)
    {
      RPS_WARNOUT("already running Rps_PayloadUnixProcess owned by " << owner()
                  << std::endl << Rps_ShowCallFrame(callframe));
      throw std::runtime_error("already running Rps_PayloadUnixProcess");
    }
  queue_of_runnable_processes.push_back(this);
  rps_postpone_child_process();
  /// code in eventloop_rps.cc should be related.
} // end Rps_PayloadUnixProcess::start_process

void
Rps_PayloadUnixProcess::do_on_active_process_queue(std::function<void(Rps_ObjectRef, Rps_CallFrame*,void*)> fun,
    Rps_CallFrame*callframe, void*client_data)
{
  std::lock_guard<std::mutex> gu(mtx_of_runnable_processes);
  RPS_ASSERT(!callframe || callframe->is_good_call_frame());
  for (Rps_PayloadUnixProcess*paylup : queue_of_runnable_processes)
    {
      Rps_ObjectRef obown = paylup->owner();
      std::lock_guard<std::recursive_mutex> gu(*obown->objmtxptr());
      RPS_DEBUG_LOG(REPL,
                    "Rps_PayloadUnixProcess::do_on_active_process_queue obown="
                    << obown << std::endl
                    << Rps_ShowCallFrame(callframe)
                    << std::endl
                    <<  RPS_FULL_BACKTRACE_HERE(1,"Rps_PayloadUnixProcess::do_on_active_process_queue"));
      fun(obown,callframe,client_data);
    }
} // end Rps_PayloadUnixProcess::do_on_active_process_queue

///////////////////////////////////////
///// transient popened file payload
Rps_PayloadPopenedFile::Rps_PayloadPopenedFile(Rps_ObjectZone*owner, const std::string command, bool reading)  // See PaylPopenedFile
  : Rps_Payload(Rps_Type::PaylPopenedFile,owner),
    _popened_cmd(command),
    _popened_to_read(reading),
    _popened_file(nullptr)
{
} // end constructor Rps_PayloadPopenedFile

//// needed but never called
Rps_PayloadPopenedFile::Rps_PayloadPopenedFile(Rps_ObjectZone*owner, Rps_Loader*ld)
  : Rps_Payload(Rps_Type::PaylPopenedFile,owner),
    _popened_cmd(),
    _popened_to_read(true),
    _popened_file(nullptr)
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


#warning transientobj_rps.cc is probably incomplete

/*** end of file transientobj_rps.cc ***/

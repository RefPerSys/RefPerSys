/****************************************************************
 * file eventloop_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the event loop and some JSONRPC code to user-interface
 *      to an external process (using JSONRPC protocol)
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2022 - 2024 The Reflective Persistent System Team
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
 ***************************************************************************0***/


#include "refpersys.hh"


extern "C" const char rps_eventloop_gitid[];
const char rps_eventloop_gitid[]= RPS_GITID;

extern "C" const char rps_eventloop_date[];
const char rps_eventloop_date[]= __DATE__;

extern "C" const char rps_eventloop_shortgitid[];
const char rps_eventloop_shortgitid[]= RPS_SHORTGITID;

// default or initial delay to poll(2) in milliseconds.
#define RPS_EVENT_DEFAULT_POLL_DELAY_MILLISEC 1600

enum self_pipe_code_en
{
  SelfPipe__NONE=0,
  SelfPipe_Dump = 'D',
  SelfPipe_GarbColl = 'G',
  SelfPipe_Process = 'P',
  SelfPipe_Quit = 'Q',
  SelfPipe_Exit = 'X',
};



#define RPS_MAXPOLL_FD 128

//extern "C" std::atomic<bool> rps_stop_event_loop_flag;

std::atomic<bool> rps_stop_event_loop_flag;

int rps_poll_delay_millisec;


#define RPS_EVENTLOOPDATA_MAGIC 814538509 /*0x308cdf0d*/
struct event_loop_data_st
{
  unsigned eld_magic;   // should be RPS_EVENTLOOPDATA_MAGIC
  int eld_polldelaymillisec;
  unsigned eld_lastix;
  std::recursive_mutex eld_mtx;
  double eld_startelapsedtime; // start real time of event loop
  double eld_startcputime; // start CPU time of event loop
  Rps_EventHandler_sigt* eld_handlarr[RPS_MAXPOLL_FD+1];
  const char*eld_explarr[RPS_MAXPOLL_FD+1];
  struct pollfd eld_pollarr[RPS_MAXPOLL_FD+1];
  void* eld_datarr[RPS_MAXPOLL_FD+1];
  int eld_sigfd;  // file descriptor from signalfd(2)
  int eld_timfd;        // file descriptor from timerfd_create(2)
  int eld_selfpipereadfd; // self pipe, reading end
  int eld_selfpipewritefd; // self pipe, writing end
  std::deque<unsigned char> eld_selfpipefifo;
  std::atomic<bool> eld_eventloopisactive;
  std::atomic<long> eld_nbloops;
  std::vector<std::function<void(struct pollfd*, int& npoll, Rps_CallFrame*)>> eld_prepollvect;
};

extern "C" struct event_loop_data_st rps_eventloopdata;
struct event_loop_data_st rps_eventloopdata;

/// for debugging purposes, a string explaining the rps_eventloopdata
extern "C" std::string rps_eventloop_explstring(void);

static std::mutex rps_jsonrpc_mtx; /// common mutex for below buffers
#warning TODO: maybe command and response should use std::stringstream?
static std::stringbuf rps_jsonrpc_cmdbuf; /// buffer for commands written to JSONRPC GUI process
static std::stringbuf rps_jsonrpc_rspbuf; /// buffer for responses read from JSONRPC GUI process
static std::stringstream rps_jsonrpc_rspstream; // should be used
#warning use rps_jsonrpc_rspstream below

extern "C" void rps_sigfd_read_handler(Rps_CallFrame*cf, int fd, void* data);
extern "C" void rps_timerfd_read_handler(Rps_CallFrame*cf, int fd, void* data);
extern "C" void rps_fifo_read_handler(Rps_CallFrame*cf, int fd, void* data);
extern "C" void rps_fifo_write_handler(Rps_CallFrame*cf, int fd, void* data);
extern "C" void rps_self_pipe_read_handler(Rps_CallFrame*cf, int fd, void* data);
extern "C" void rps_self_pipe_write_handler(Rps_CallFrame*cf, int fd, void* data);
/**
 * We probably want to use the pipe to self trick.
 * https://www.sitepoint.com/the-self-pipe-trick-explained/
 *
 * in cooperation with Rps_PayloadUnixProcess::start_process
 **/
static std::atomic<bool> event_loop_is_active;

static std::atomic<long> event_nbloops;


extern "C" void rps_self_pipe_write_byte(unsigned char b);

static void handle_self_pipe_byte_rps(unsigned char b);



void
rps_do_stop_event_loop(void)
{
  RPS_DEBUG_LOG(REPL, "rps_do_stop_event_loop thread:"
                <<  rps_current_pthread_name()
                << RPS_FULL_BACKTRACE_HERE(1, "rps_do_stop_event_loop"));
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  rps_stop_event_loop_flag.store(true);
  if (rps_fltk_enabled ())
    {
      rps_fltk_stop();
    }
} // end rps_do_stop_event_loop

extern "C" void rps_jsonrpc_initialize(void);


void
rps_self_pipe_write_byte(unsigned char b)
{
  RPS_ASSERT(b != (char)0);
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  rps_eventloopdata.eld_selfpipefifo.push_back(b);
} // end rps_self_pipe_write_byte


int
rps_register_event_loop_prepoller(std::function<void (struct pollfd*, int npoll, Rps_CallFrame*)> fun)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  int ln = (int) rps_eventloopdata.eld_prepollvect.size();
  if (ln > 1000)
    RPS_FATALOUT("too many event loop prepoller " << ln);
  for (int i=0; i<ln; i++)
    {
      if (!rps_eventloopdata.eld_prepollvect[i])
        {
          rps_eventloopdata.eld_prepollvect[i] = fun;
          return i;
        }
    }
  rps_eventloopdata.eld_prepollvect.push_back(fun);
  return rps_eventloopdata.eld_prepollvect.size();
} // end rps_register_event_loop_prepoller


void
rps_unregister_event_loop_prepoller(int rank)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  if (rank<0 || rank>(int) rps_eventloopdata.eld_prepollvect.size())
    {
      RPS_WARNOUT("invalid rank to rps_unregister_event_loop_prepoller " << rank);
      return;
    };
  rps_eventloopdata.eld_prepollvect[rank] = nullptr;
} // end rps_unregister_event_loop_prepoller

bool rps_event_loop_get_entry(int ix,
                              Rps_EventHandler_sigt**pfun,
                              struct pollfd*po, const char**pexpl, void**pdata)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  if (pfun)
    *pfun = nullptr;
  if (po)
    {
      *po =  pollfd {};
    };
  if (pexpl)
    *pexpl = nullptr;
  if (pdata)
    *pdata = nullptr;
  if (ix<0 || ix>(int) rps_eventloopdata.eld_lastix)
    return false;
  if (pfun)
    *pfun = rps_eventloopdata.eld_handlarr[ix];
  if (po)
    *po = rps_eventloopdata.eld_pollarr[ix];
  if (pexpl)
    *pexpl = rps_eventloopdata.eld_explarr[ix];
  if (pdata)
    *pdata = rps_eventloopdata.eld_datarr[ix];
  return true;
} // end rps_event_loop_get_entry

void
rps_event_loop_add_input_fd_handler (int fd,
                                     Rps_EventHandler_sigt*f,
                                     const char* explanation,
                                     void*data)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  unsigned lastfd = rps_eventloopdata.eld_lastix;
  RPS_ASSERT(lastfd < RPS_MAXPOLL_FD);
  rps_eventloopdata.eld_pollarr[lastfd].fd = fd;
  rps_eventloopdata.eld_pollarr[lastfd].events = POLLIN;
  rps_eventloopdata.eld_explarr[lastfd] = explanation;
  rps_eventloopdata.eld_datarr[lastfd] = data;
  rps_eventloopdata.eld_lastix = lastfd+1;
  if (rps_fltk_enabled())
    {
      rps_fltk_add_input_fd(fd, f, explanation, (int)lastfd);
    };
  RPS_DEBUG_LOG(REPL, "rps_event_loop_add_input_fd_handler fd#" << fd
                << " f@" << (void*)f
                << " expl:" << explanation
                << " data@" << (void*)data);
} // end rps_event_loop_add_input_fd_handler

void
rps_event_loop_add_output_fd_handler (int fd,
                                      Rps_EventHandler_sigt*f,
                                      const char* explanation,
                                      void*data)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  unsigned lastfd = rps_eventloopdata.eld_lastix;
  RPS_ASSERT(lastfd < RPS_MAXPOLL_FD);
  rps_eventloopdata.eld_pollarr[lastfd].fd = fd;
  rps_eventloopdata.eld_pollarr[lastfd].events = POLLOUT;
  rps_eventloopdata.eld_explarr[lastfd] = explanation;
  rps_eventloopdata.eld_datarr[lastfd] = data;
  rps_eventloopdata.eld_lastix = lastfd+1;
  if (rps_fltk_enabled())
    {
      rps_fltk_add_output_fd(fd, f, explanation, (int)lastfd);
    }
  RPS_DEBUG_LOG(REPL, "rps_event_loop_add_output_fd_handler fd#" << fd
                << " f@" << (void*)f
                << " expl:" << explanation
                << " data@" << (void*)data);
} // end rps_event_loop_add_output_fd_handler

void
rps_event_loop_remove_input_fd_handler(int fd)
{
  Rps_EventHandler_sigt* new_handlarr[RPS_MAXPOLL_FD+1];
  const char*new_explarr[RPS_MAXPOLL_FD+1];
  struct pollfd new_pollarr[RPS_MAXPOLL_FD+1];
  void* new_datarr[RPS_MAXPOLL_FD+1];
  memset (new_handlarr, 0, sizeof(new_handlarr));
  memset (new_explarr, 0, sizeof(new_explarr));
  memset (new_pollarr, 0, sizeof(new_pollarr));
  memset (new_datarr, 0, sizeof(new_datarr));
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  unsigned lastix = rps_eventloopdata.eld_lastix;
  unsigned newlastix = 0;
  for (int ix=0; ix<(int)lastix; ix++)
    {
      Rps_EventHandler_sigt* curphdlr=nullptr;
      struct pollfd curpfd= {};
      const char*curexpl=nullptr;
      void*curdata = nullptr;
      if (rps_event_loop_get_entry(ix, &curphdlr, &curpfd, &curexpl, &curdata))
        {
          if (curpfd.fd == fd && curpfd.events == POLLIN)
            continue;
          new_handlarr[newlastix] = curphdlr;
          new_pollarr[newlastix] = curpfd;
          new_explarr[newlastix] = curexpl;
          new_datarr[newlastix] = curdata;
          newlastix++;
        }
      else
        continue;
    };
  // the erasing below is in theory useless...
  memset (rps_eventloopdata.eld_handlarr, 0, sizeof(rps_eventloopdata.eld_handlarr));
  memset (rps_eventloopdata.eld_explarr, 0, sizeof(rps_eventloopdata.eld_explarr));
  memset (rps_eventloopdata.eld_pollarr, 0, sizeof(rps_eventloopdata.eld_pollarr));
  memset (rps_eventloopdata.eld_datarr, 0, sizeof(rps_eventloopdata.eld_datarr));
  // copy the new things
  memcpy (rps_eventloopdata.eld_handlarr, new_handlarr, newlastix*sizeof(new_handlarr[0]));
  memcpy (rps_eventloopdata.eld_explarr, new_explarr, newlastix*sizeof(new_explarr[0]));
  memcpy (rps_eventloopdata.eld_pollarr, new_pollarr, newlastix*sizeof(new_pollarr[0]));
  memcpy (rps_eventloopdata.eld_datarr, new_datarr, newlastix*sizeof(new_datarr[0]));
  rps_eventloopdata.eld_lastix = newlastix;
  RPS_DEBUG_LOG(REPL, "rps_event_loop_remove_input_fd_handler fd#" << fd);
  if (rps_fltk_enabled())
    rps_fltk_remove_input_fd(fd);
} // end rps_event_loop_remove_input_fd_handler

void
rps_self_pipe_read_handler(Rps_CallFrame*cf, int fd, void* data)
{
  RPS_ASSERT(rps_is_main_thread());
  unsigned char buf[128];
  RPS_ASSERT(fd == rps_eventloopdata.eld_selfpipereadfd);
  RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
  memset(buf, 0, sizeof(buf));
  int nbr = read(fd, buf, sizeof(buf));
  RPS_DEBUG_LOG(REPL, "rps_self_pipe_read_handler fd#" << fd << " nbr=" << nbr
                << " buf=" << buf << "." << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_self_pipe_read_handler"));
  if (nbr > 0)
    {
      std::function<void(Rps_GarbageCollector*)> gcmarker
        = [&] (Rps_GarbageCollector*gc)
      {
        RPS_ASSERT(gc->is_valid_garbcoll());
        gc->mark_call_stack(cf);
      };
      for (int i = 0; i<nbr; i++)
        {
          unsigned char b=buf[i];
          switch (b)
            {
            case SelfPipe_Dump:
              rps_dump_into (rps_get_loaddir());
              break;
            case SelfPipe_GarbColl:
              rps_garbage_collect(&gcmarker);
              break;
            case SelfPipe_Quit:
              rps_stop_event_loop_flag.store(true);
              break;
            case SelfPipe_Exit:
              rps_dump_into (rps_get_loaddir());
              rps_stop_event_loop_flag.store(true);
              break;
            case SelfPipe_Process:
#warning should call something from transientobj_rps.cc to perhaps fork a process related to some Rps_PayloadUnixProcess
            /* TODO: see Rps_PayloadUnixProcess::queue_of_runnable_processes
            in transientobj_rps.cc; the dormant processes should somehow
            be started by fork/exec */
            default:
              RPS_FATALOUT("unexpected byte " << (char)b << "#" << (unsigned)b << " on self pipe");
            };
        };
    }
} // end rps_self_pipe_read_handler

void
rps_self_pipe_write_handler(Rps_CallFrame*cf, int fd, void* data)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_DEBUG_LOG(REPL, "rps_self_pipe_write_handler fd#" << fd
                << RPS_FULL_BACKTRACE_HERE(1, "rps_self_pipe_write_handler"));
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  RPS_ASSERT(fd == rps_eventloopdata.eld_selfpipewritefd);
  while (!rps_eventloopdata.eld_selfpipefifo.empty())
    {
      char buf[4] = {0,0,0,0};
      char c = rps_eventloopdata.eld_selfpipefifo.back();
      buf[0] = c;
      errno = 0;
      ssize_t nw = write(fd, buf, 1);
      if (nw>0)
        {
          RPS_DEBUG_LOG(REPL, "rps_self_pipe_write_handler wrote " << buf << " to fd#" << fd);
          rps_eventloopdata.eld_selfpipefifo.pop_back();
        }
      else
        {
          RPS_DEBUG_LOG(REPL, "rps_self_pipe_write_handler write-fail fd#" << fd << ":" << strerror(errno));
          break;
        }
    }
} // end rps_self_pipe_write_handler

void
rps_event_loop_remove_output_fd_handler(int fd)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  unsigned lastfd = rps_eventloopdata.eld_lastix;
  Rps_EventHandler_sigt* new_handlarr[RPS_MAXPOLL_FD+1];
  const char*new_explarr[RPS_MAXPOLL_FD+1];
  struct pollfd new_pollarr[RPS_MAXPOLL_FD+1];
  void* new_datarr[RPS_MAXPOLL_FD+1];
  memset (new_handlarr, 0, sizeof(new_handlarr));
  memset (new_explarr, 0, sizeof(new_explarr));
  memset (new_pollarr, 0, sizeof(new_pollarr));
  memset (new_datarr, 0, sizeof(new_datarr));
  unsigned new_lastfd=0;
  unsigned lastix = rps_eventloopdata.eld_lastix;
  unsigned newlastix = 0;
  for (int ix=0; ix<(int)lastix; ix++)
    {
      Rps_EventHandler_sigt* curphdlr=nullptr;
      struct pollfd curpfd= {0};
      const char*curexpl=nullptr;
      void*curdata = nullptr;
      if (rps_event_loop_get_entry(ix, &curphdlr, &curpfd, &curexpl, &curdata))
        {
          if (curpfd.fd == fd && curpfd.events == POLLOUT)
            continue;
          new_handlarr[newlastix] = curphdlr;
          new_pollarr[newlastix] = curpfd;
          new_explarr[newlastix] = curexpl;
          new_datarr[newlastix] = curdata;
          newlastix++;
        }
      else
        continue;
    };
  // the erasing below is in theory useless...
  memset (rps_eventloopdata.eld_handlarr, 0,
          sizeof(rps_eventloopdata.eld_handlarr));
  memset (rps_eventloopdata.eld_explarr, 0,
          sizeof(rps_eventloopdata.eld_explarr));
  memset (rps_eventloopdata.eld_pollarr, 0,
          sizeof(rps_eventloopdata.eld_pollarr));
  memset (rps_eventloopdata.eld_datarr, 0,
          sizeof(rps_eventloopdata.eld_datarr));
  // copy the new things
  memcpy (rps_eventloopdata.eld_handlarr, new_handlarr,
          newlastix*sizeof(new_handlarr[0]));
  memcpy (rps_eventloopdata.eld_explarr, new_explarr,
          newlastix*sizeof(new_explarr[0]));
  memcpy (rps_eventloopdata.eld_pollarr, new_pollarr,
          newlastix*sizeof(new_pollarr[0]));
  memcpy (rps_eventloopdata.eld_datarr, new_datarr,
          newlastix*sizeof(new_datarr[0]));
  rps_eventloopdata.eld_lastix = newlastix;
  RPS_DEBUG_LOG(REPL, "rps_event_loop_remove_output_fd_handler fd#" << fd);
  if (rps_fltk_enabled())
    rps_fltk_remove_output_fd(fd);
} // end rps_event_loop_remove_output_fd_handler

bool
rps_is_fifo(std::string path)
{
  struct stat s = {};
  if (!stat(path.c_str(), &s))
    return (s.st_mode & S_IFMT) == S_IFIFO;
  return false;
} // end rps_is_fifo

void
rps_initialize_pipe_to_self_in_event_loop(void)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  /**
   * create the pipe to self and install handlers for it
   **/
  {
    int pipefdarr[2] = {-1, -1};
    if (pipe2(pipefdarr, O_CLOEXEC) <0)
      RPS_FATALOUT("rps_initialize_event_loop failed to create pipe to self:"
                   << strerror(errno));
    rps_eventloopdata.eld_selfpipereadfd = pipefdarr[0];
    RPS_ASSERT(rps_eventloopdata.eld_selfpipereadfd > 0);
    rps_eventloopdata.eld_selfpipewritefd = pipefdarr[1];
    RPS_ASSERT(rps_eventloopdata.eld_selfpipewritefd > 0);
    rps_event_loop_add_input_fd_handler(rps_eventloopdata.eld_selfpipereadfd,
                                        rps_self_pipe_read_handler,
                                        "selfpiperead",
                                        nullptr);;
    rps_event_loop_add_input_fd_handler(rps_eventloopdata.eld_selfpipewritefd,
                                        rps_self_pipe_write_handler,
                                        "selfpipewrite",
                                        nullptr);
    RPS_DEBUG_LOG(REPL, "rps_initialize_pipe_to_self_in_event_loop eld_selfpipereadfd#"
                  << (rps_eventloopdata.eld_selfpipereadfd)
                  << " eld_selfpipewritefd#"
                  << (rps_eventloopdata.eld_selfpipewritefd)
                  << " rps_poll_delay_millisec=" << rps_poll_delay_millisec
                  << " in thread " << rps_current_pthread_name()
                  << std::endl
                  << RPS_FULL_BACKTRACE_HERE(1, "rps_initialize_pipe_to_self_event_loop"));
  }
} // end rps_initialize_pipe_to_self_in_event_loop

void
rps_initialize_signalfd_in_event_loop(void)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  sigset_t msk= {};
  sigemptyset(&msk);
  sigaddset(&msk, SIGCHLD);
  sigaddset(&msk, SIGINT);
  sigaddset(&msk, SIGTERM);
  sigaddset(&msk, SIGXCPU);
  sigaddset(&msk, SIGALRM);
  sigaddset(&msk, SIGVTALRM);
  rps_eventloopdata.eld_sigfd = signalfd(-1, &msk, SFD_CLOEXEC);
  if (rps_eventloopdata.eld_sigfd<=0)
    RPS_FATALOUT("failed to call signalfd:" << strerror(errno));
  RPS_DEBUG_LOG(REPL, "rps_initialize_signalfd_in_event_loop thread "
                << rps_current_pthread_name()
                << " sigfd#" << rps_eventloopdata.eld_sigfd
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_initialize_signalfd_in_event_loop")
               );
  rps_event_loop_add_input_fd_handler(rps_eventloopdata.eld_sigfd, rps_sigfd_read_handler,
                                      "signalfd", nullptr);
} // end rps_initialize_signalfd_in_event_loop

void
rps_initialize_timerfd_in_event_loop(void)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  rps_eventloopdata.eld_timfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
  if (rps_eventloopdata.eld_timfd<=0)
    RPS_FATALOUT("failed to call timerfd:" << strerror(errno));
  RPS_DEBUG_LOG(REPL, "rps_initialize_timerfd_in_event_loop thread "
                << rps_current_pthread_name()
                << " timerfd#" << rps_eventloopdata.eld_timfd
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_initialize_timerfd_in_event_loop")
               );
  rps_event_loop_add_input_fd_handler(rps_eventloopdata.eld_sigfd, rps_timerfd_read_handler,
                                      "timerfd", nullptr);
} // end rps_initialize_timerfd_in_event_loop

void
rps_initialize_jsonfifo_in_event_loop(void)
{
  struct rps_fifo_fdpair_st fdp = rps_get_gui_fifo_fds();
  if (fdp.fifo_ui_wcmd <= 0)
    RPS_FATALOUT("invalid command FIFO fd " << fdp.fifo_ui_wcmd
                 << " with FIFO prefix " << rps_get_fifo_prefix());
  if (fdp.fifo_ui_rout <= 0)
    RPS_FATALOUT("invalid output FIFO fd " << fdp.fifo_ui_rout
                 << " with FIFO prefix " << rps_get_fifo_prefix());
} // end rps_initialize_jsonfifo_in_event_loop

/////////// the toplevel event loop initialization
void
rps_initialize_event_loop(void)
{
  /// rps_initialize_event_loop should be called once, early, from the
  /// main thread.  It is called after the persistent heap load.
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  static int count;
  if (!rps_is_main_thread())
    RPS_FATALOUT("rps_initialize_event_loop should be called only from the main thread");
  if (count++ > 0)
    RPS_FATALOUT("rps_initialize_event_loop should be called once");
  rps_eventloopdata.eld_magic = RPS_EVENTLOOPDATA_MAGIC;
  rps_eventloopdata.eld_polldelaymillisec = (rps_debug_flags != 0)?666:111;
  rps_eventloopdata.eld_lastix = 0;
  rps_eventloopdata.eld_startelapsedtime = -1.0/1024;
  rps_eventloopdata.eld_startcputime =  -1.0/1024;
  memset (rps_eventloopdata.eld_handlarr, 0,
          sizeof(rps_eventloopdata.eld_handlarr));
  memset (rps_eventloopdata.eld_explarr, 0,
          sizeof(rps_eventloopdata.eld_explarr));
  memset (rps_eventloopdata.eld_pollarr, 0,
          sizeof(rps_eventloopdata.eld_pollarr));
  memset (rps_eventloopdata.eld_datarr, 0,
          sizeof(rps_eventloopdata.eld_datarr));
  rps_eventloopdata.eld_sigfd = -1;
  rps_eventloopdata.eld_timfd = -1;
  rps_eventloopdata.eld_selfpipereadfd = -1;
  rps_eventloopdata.eld_selfpipewritefd = -1;
  rps_eventloopdata.eld_selfpipefifo.clear();
  rps_eventloopdata.eld_eventloopisactive = false;
  rps_eventloopdata.eld_nbloops = 0;
  rps_eventloopdata.eld_prepollvect.clear();
  ///
  RPS_DEBUG_LOG(REPL, "rps_initialize_event_loop starting "
                << (rps_fltk_enabled()?"with FLTK":"without-fltk")
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_initialize_event_loop*start"));
  rps_initialize_pipe_to_self_in_event_loop();
  rps_initialize_signalfd_in_event_loop();
  rps_initialize_timerfd_in_event_loop();
  if (!rps_get_fifo_prefix().empty())
    rps_initialize_jsonfifo_in_event_loop();
  if (rps_poll_delay_millisec==0)
    rps_poll_delay_millisec = RPS_EVENT_DEFAULT_POLL_DELAY_MILLISEC;
  RPS_DEBUG_LOG(REPL, "rps_initialize_event_loop ended "
                << (rps_fltk_enabled()?"with FLTK":"without-fltk")
                << " polldelay=" << rps_poll_delay_millisec << " ms"
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_initialize_event_loop*ended"));
} // end rps_initialize_event_loop



/**
   Function rps_jsonrpc_initialize is called once from main, when
   rps_fifo_prefix is not empty. The FIFOs have been created in
   main_rps.cc...
***/
void
rps_jsonrpc_initialize(void)
{
  RPS_ASSERT(!rps_get_fifo_prefix().empty());
  struct rps_fifo_fdpair_st fdp = rps_get_gui_fifo_fds();
  if (fdp.fifo_ui_wcmd <= 0)
    RPS_FATALOUT("invalid command FIFO fd " << fdp.fifo_ui_wcmd
                 << " with FIFO prefix " << rps_get_fifo_prefix());
  if (fdp.fifo_ui_rout <= 0)
    RPS_FATALOUT("invalid output FIFO fd " << fdp.fifo_ui_rout
                 << " with FIFO prefix " << rps_get_fifo_prefix());
  RPS_DEBUG_LOG(REPL, "rps_jsonrpc_initialize FIFO prefix "
                << rps_get_fifo_prefix()
                << " wcmdfd#" << fdp.fifo_ui_wcmd
                << " routfd#" << fdp.fifo_ui_rout
                << RPS_FULL_BACKTRACE_HERE(1, "rps_jsonrpc_initialize")
                << std::endl << " in thread " << rps_current_pthread_name());
#warning maybe incomplete rps_jsonrpc_initialize
  /**
   *  TODO: we need to document the JSONRPC protocol between the GUI
   *  software and RefPerSys
   *
   *  A needed request is the _VERSION one....
   *
  **/
  RPS_DEBUG_LOG(REPL, "ending rps_jsonrpc_initialize with fifo prefix "
                << rps_get_fifo_prefix() //
                << " and wcmd.fd#" << fdp.fifo_ui_wcmd //
                << " and rout.fd#" << fdp.fifo_ui_rout
                << " thread:" << rps_current_pthread_name());
} // end rps_jsonrpc_initialize

std::string
rps_eventloop_explstring(void)
{
  std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
  char numbuf[32];
  memset(numbuf, 0, sizeof(numbuf));
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  std::ostringstream out;
  double elapsed_age = rps_elapsed_real_time() - rps_eventloopdata.eld_startelapsedtime;
  double cpu_age = rps_process_cpu_time() - rps_eventloopdata.eld_startcputime;
  snprintf(numbuf, sizeof(numbuf), "%.2f", elapsed_age);
  out << numbuf << " elap";
  snprintf(numbuf, sizeof(numbuf), "%.3f", cpu_age);
  out << "," << numbuf << " cpu time";
#warning rps_eventloop_explstring very incomplete
  out << std::flush;
  return out.str();
} // end rps_eventloop_explstring




////////////////////////////////////////////////////////////////
/* TODO: an event loop using poll(2) and also handling SIGCHLD using
   https://man7.org/linux/man-pages/man2/signalfd.2.html
 */
void
rps_event_loop(void)
{
  /// see https://man7.org/linux/man-pages/man2/poll.2.html
  int nbfdpoll = 0; /// number of polled file descriptors, second argument to poll(2)
  long pollcount = 0;
  double startelapsedtime = rps_elapsed_real_time();
  double startcputime = rps_process_cpu_time();
  RPS_DEBUG_LOG(REPL, "rps_event_loop starting elapsedtime=" << startelapsedtime
                << " cputime=" << startcputime
                << " thread:" << rps_current_pthread_name() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop/start"));
  std::array<std::function<void(Rps_CallFrame*, int/*fd*/, short /*revents*/)>,RPS_MAXPOLL_FD+1> handlarr;
  ///
  /// check that rps_event_loop is called exactly once from main
  /// thread, and after rps_initialize_event_loop...
  {
    static int nbcall;
    if (!rps_is_main_thread())
      RPS_FATALOUT("rps_event_loop should be called only once, and from the main thread");
    if (nbcall++>0)
      RPS_FATALOUT("rps_event_loop has already been called " << nbcall << " times");
    /// The rps_poll_delay_millisec should have been set by a prior call
    /// ... to rps_initialize_event_loop above.
    if (rps_poll_delay_millisec<=0)
      RPS_FATALOUT("the poll event loop has not being properly initialized");
  };
  if (rps_fltk_enabled ())
    RPS_FATALOUT("rps_event_loop incompatible with FLTK");
  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/rps_curthread_callframe, //
                 /** locals **/
                 Rps_Value valarr[RPS_MAXPOLL_FD+1];
                 Rps_ClosureValue closarr[RPS_MAXPOLL_FD+1];
                );
#warning incomplete rps_event_loop
  const char*explarr[RPS_MAXPOLL_FD+1];
  memset (explarr, 0, sizeof(explarr));
#warning TODO: consider using rps_timer ...?
  /*** give output
   ***/
  RPS_INFORMOUT("starting rps_event_loop in pid " << (long)getpid() << std::endl
                << "... on " << rps_hostname() << " thread " << rps_current_pthread_name()
                << " git " << rps_shortgitid << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop")
               );
  while (!rps_stop_event_loop_flag.load())
    {
      char elapsbuf[32];
      memset(elapsbuf, 0, sizeof(elapsbuf));
      struct pollfd pollarr[RPS_MAXPOLL_FD+1];
      memset ((void*)&pollarr, 0, sizeof(pollarr));
#define EXPLAIN_EVFD_AT(Fil,Lin,Ix,Expl) do { explarr[Ix] = Fil ":" #Lin " " Expl; } while(0)
#define EXPLAIN_EVFD_ATBIS(Fil,Lin,Ix,Expl)  EXPLAIN_EVFD_AT(Fil,Lin,Ix,Expl)
#define EXPLAIN_EVFD_RPS(Ix,Expl) EXPLAIN_EVFD_ATBIS(__FILE__,__LINE__,(Ix),Expl)
      int loopcnt=1+ event_nbloops.fetch_add(1);
      if ((loopcnt-1) % 16 == 0)
        {
          RPS_DEBUG_LOG(REPL, "looping rps_event_loop #" << loopcnt
                        << " elapsed:" << rps_elapsed_real_time()
                        << " thread:" << rps_current_pthread_name()
                        << ((rps_fltk_enabled())?" with FLTK": " without-fltk")
                        << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop/looping"));
        }
      else
        RPS_DEBUG_LOG(REPL, "looping rps_event_loop #" << loopcnt
                      << " elapsed:" << rps_elapsed_real_time());
      memset ((void*)&pollarr, 0, sizeof(pollarr));
      nbfdpoll=0;
      struct rps_fifo_fdpair_st fdp = rps_get_gui_fifo_fds();
      if (fdp.fifo_ui_wcmd >0)
        {
          /// JSONRPC commands written from RefPerSys to the GUI process...
          RPS_ASSERT(nbfdpoll<RPS_MAXPOLL_FD);
          int pix = nbfdpoll++;
          pollarr[pix].fd = fdp.fifo_ui_wcmd;
          pollarr[pix].events = POLLOUT;
          EXPLAIN_EVFD_RPS(pix, "JsonRpc commands to GUI");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            RPS_ASSERT(rev == POLLOUT);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            RPS_DEBUG_LOG(REPL, "should write JSONRPC to GUI on cmdfd#" << fd);
            RPS_FATALOUT("missing code to handle JSONRPC output commands to fd#"
                         << fd << " pix#" << pix);
            /* TODO: write the bytes that are in rps_jsonrpc_cmdbuf */
#warning missing code to handle JsonRpc output to the GUI process
          };
        };
      if (fdp.fifo_ui_rout>0)
        {
          /// JSONRPC responses/events sent by the GUI process to RefPerSys
          RPS_ASSERT(nbfdpoll<RPS_MAXPOLL_FD);
          int pix = nbfdpoll++;
          pollarr[pix].fd = fdp.fifo_ui_rout;
          pollarr[pix].events = POLLIN;
          EXPLAIN_EVFD_RPS(pix, "JsonRpc responses from GUI");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            constexpr unsigned bufusefulen = 1024;
            char buf[bufusefulen+8];
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            RPS_ASSERT(rev == POLLIN);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            /* TODO: should read(2) */
            memset(buf, 0, sizeof(buf));
            RPS_DEBUG_LOG(REPL, "reading JSONRPC from GUI on outfd#" << fd);
            errno = 0;
            int nbr = read(fd, buf, bufusefulen);
            if (nbr < 0)
              return;
            if (nbr == 0)
              {
                RPS_FATALOUT("missing code to handle JSONRPC input EOF from fd#"
                             << fd << " pix#" << pix);
#warning missing code to handle JsonRpc EOF from GUI process
              };
            buf[nbr] = (char)0;
            RPS_DEBUG_LOG(REPL, "got " << nbr << " bytes from JSONRPC fd#" << fd << std::endl
                          << buf << std::endl);
            /* TODO: append the bytes we did read to
               rps_jsonrpc_rspbuf; by convention a double newline or a
               formfeed is ending the JSON message. */
            {
              std::lock_guard<std::mutex> gu(rps_jsonrpc_mtx);
#warning oldbufsiz need a code review
              std::size_t oldbufsiz= rps_jsonrpc_rspbuf.in_avail();
              std::size_t oldprevix = (oldbufsiz>0)?(oldbufsiz-1):0;
              auto newsiz = rps_jsonrpc_rspbuf.sputn(buf, nbr);
              bool again = false;
              char* curp = buf;
              do
                {
                  again = false;
                  /// Find a double newline or formfeed at index. If there
                  /// is one (or more) it is terminating a message...
                  /* Invariant: there cannot be any double newline of
                  formfeed before, since it would have been already
                  processed. */
                  {
                    char*ffbuf = strchr(curp, '\f');
                    char*nl2buf = strstr(curp, "\n\n");
                    char*eombuf = nullptr;
                    if (ffbuf && nl2buf)
                      eombuf = (ffbuf>nl2buf)?ffbuf:nl2buf;
                    else if (ffbuf) eombuf = ffbuf;
                    else if (nl2buf) eombuf = nl2buf+1;
                    if (eombuf)
                      {
                        int eombufoff = eombuf - buf;
                        again = true;
                        RPS_FATALOUT("missing code JSONRPC input eombufoff:" << eombufoff);
#warning should build a string with the JSON message, then decode and process that JSON
                      }
                  }
                }
              while (again);
              /// oldprevix
#warning missing code to handle JsonRpc input from the GUI process
            }
            RPS_FATALOUT("missing code to handle JSONRPC input responses from fd#"
                         << fd << " pix#" << pix
                         << " did read " << nbr << " bytes");
          };
        };

      if (rps_eventloopdata.eld_timfd>0)
        {
          /// timers transformed to data with timerfd_create(2)
          RPS_ASSERT(nbfdpoll<RPS_MAXPOLL_FD);
          int pix = nbfdpoll++;
          pollarr[pix].fd = rps_eventloopdata.eld_timfd;
          pollarr[pix].events = POLLIN;
          EXPLAIN_EVFD_RPS(pix, "timerfd");
          handlarr[pix] = [&](Rps_CallFrame*cf, int fd, short rev)
          {
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            RPS_ASSERT(rev == POLLIN);
            uint64_t timbuf[16];
            memset (&timbuf, 0, sizeof(timbuf));
            int nbr = read(fd, (void*)&timbuf, sizeof(timbuf));
            if (nbr>0)
              {
                RPS_DEBUG_LOG(REPL, "eventloop read " << nbr << " bytes on timerfd, so " << (nbr/sizeof(std::uint64_t)) << " int64s: "
                              << Rps_Do_Output([=](std::ostream& out)
                {
                  for (int ix=0; ix< (int) (nbr/sizeof(std::uint64_t)); ix++)
                    out << ' ' << timbuf[ix];
                }
                                              )
                    << std::flush);
              }
#warning some missing code to handle timerfd...
          };
        };
      if (rps_eventloopdata.eld_selfpipereadfd>0)
        {
          RPS_ASSERT(nbfdpoll<RPS_MAXPOLL_FD);
          int pix = nbfdpoll++;
          pollarr[pix].fd = rps_eventloopdata.eld_selfpipereadfd;
          pollarr[pix].events = POLLIN;
          EXPLAIN_EVFD_RPS(pix, "self_pipe_read_fd");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            unsigned char buf[128];
            RPS_ASSERT(fd == rps_eventloopdata.eld_selfpipereadfd);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            RPS_ASSERT(rev == POLLIN);
            /* TODO: should read(2) */
            memset(buf, 0, sizeof(buf));
            int nbr = read(fd, buf, sizeof(buf));
            if (nbr > 0)
              for (int i = 0; i<nbr; i++)
                handle_self_pipe_byte_rps(buf[i]);
          };
        };
      bool wantselfwrite = false;
      {
        std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
        wantselfwrite = !rps_eventloopdata.eld_selfpipefifo.empty();
      }
      if (rps_eventloopdata.eld_selfpipewritefd>0 && wantselfwrite)
        {
          RPS_ASSERT(nbfdpoll<RPS_MAXPOLL_FD);
          int pix = nbfdpoll++;
          pollarr[pix].fd = rps_eventloopdata.eld_selfpipewritefd;
          pollarr[pix].events = POLLOUT;
          EXPLAIN_EVFD_RPS(pix, "self_pipe_write_fd");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            RPS_ASSERT(fd == rps_eventloopdata.eld_selfpipewritefd);
            RPS_ASSERT(rev == POLLOUT);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
            while (!rps_eventloopdata.eld_selfpipefifo.empty())
              {
                unsigned char b = rps_eventloopdata.eld_selfpipefifo.back();
                if (write(fd, &b, 1) > 0)
                  {
                    RPS_DEBUG_LOG(REPL, "eventloop wrote self pipe " << b
                                  << " to fd#" << fd);
                    rps_eventloopdata.eld_selfpipefifo.pop_back();
                  }
                else
                  {
                    RPS_DEBUG_LOG(REPL, "eventloop failed to write self pipe " << b
                                  << " to fd#" << fd << ":" << strerror(errno));
                    break;
                  }
              };
          };
        };
      bool debugpoll = RPS_DEBUG_ENABLED(REPL) //
                       || RPS_DEBUG_ENABLED(EVENT_LOOP) //
                       || RPS_DEBUG_ENABLED(GUI);
      if (debugpoll)
        {
          char pidbuf[16];
          memset(pidbuf, 0, sizeof(pidbuf));
          if (event_nbloops.load() % 10)
            snprintf(pidbuf, sizeof(pidbuf)-1, " pid %d", (int)getpid());
          for (int pix=0; pix<nbfdpoll; pix++)
            {
              std::string evstr;
              if (pollarr[pix].events & POLLIN)
                evstr += " POLLIN";
              if (pollarr[pix].events & POLLOUT)
                evstr += " POLLOUT";
              if (pollarr[pix].events & POLLPRI)
                evstr += " POLLPRI";
              if (pollarr[pix].events & POLLRDHUP)
                evstr += " POLLRDHUP";
              if (pollarr[pix].events & POLLERR)
                evstr += " POLLERR";
              if (pollarr[pix].events & POLLHUP)
                evstr += " POLLHUP";
              if (pollarr[pix].events & POLLNVAL)
                evstr += " POLLNVAL";
              rps_debug_printf_at(__FILE__,__LINE__,__FUNCTION__,RPS_DEBUG__EVERYTHING,
                                  "poll[%d] loop%ld:fd#%d:%s,%s%s\n",
                                  pix, event_nbloops.load(), pollarr[pix].fd,
                                  explarr[pix], evstr.c_str(), pidbuf);
            }
        };
      errno = 0;
      if (Rps_Agenda::agenda_timeout > 0
          && rps_elapsed_real_time() >= Rps_Agenda::agenda_timeout)
        {
          RPS_INFORMOUT("stopping agenda mechanism because of agenda timeoutafter "
                        << pollcount << " polling." << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop/timeout"));
          rps_stop_agenda_mechanism();
          break;
        };
      fflush(nullptr);
      // call the registered event prepoller
      {
        std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
        for (auto fun : rps_eventloopdata.eld_prepollvect)
          {
            if (fun)
              fun(pollarr, nbfdpoll, &_);
          }
      }
      errno = 0;
      int respoll = poll(pollarr, nbfdpoll, 20 + (rps_poll_delay_millisec*(debugpoll?4:1)));
      pollcount++;
      if (pollcount %2 && debugpoll)
        snprintf(elapsbuf, sizeof(elapsbuf), " elti: %.3fs", rps_elapsed_real_time());
      RPS_DEBUG_LOG(REPL, "rps_event_loop pollcount#"  << pollcount ///
                    << " respoll=" << respoll
                    << " nbfdpoll=" << nbfdpoll
                    << " elapsed time:" << rps_elapsed_real_time());
      if (respoll>0)
        {
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,__FUNCTION__,RPS_DEBUG__EVERYTHING,
                                "respoll=%d loop%ld%s\n", respoll, event_nbloops.load(), elapsbuf);
          int nbrev=0;
          for (int pix=0; pix<nbfdpoll; pix++)
            {
              if (pollarr[pix].revents != 0)
                {
                  nbrev++;
                  if (debugpoll)
                    {
                      std::string evstr;
                      if (pollarr[pix].revents & POLLIN)
                        evstr += " POLLIN";
                      if (pollarr[pix].revents & POLLOUT)
                        evstr += " POLLOUT";
                      if (pollarr[pix].revents & POLLPRI)
                        evstr += " POLLPRI";
                      if (pollarr[pix].revents & POLLRDHUP)
                        evstr += " POLLRDHUP";
                      if (pollarr[pix].revents & POLLERR)
                        evstr += " POLLERR";
                      if (pollarr[pix].revents & POLLHUP)
                        evstr += " POLLHUP";
                      if (pollarr[pix].revents & POLLNVAL)
                        evstr += " POLLNVAL";
                      rps_debug_printf_at(__FILE__,__LINE__,__FUNCTION__,RPS_DEBUG__EVERYTHING,
                                          "polled[%d]:fd#%d:%s>%s\n",
                                          pix, pollarr[pix].fd, explarr[pix], evstr.c_str());
                    }
                  if (handlarr[pix])
                    handlarr[pix](&_, pollarr[pix].fd, pollarr[pix].revents);
                  else
                    usleep((5+(pix & 0xf))*1024);
                };
            };
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,__FUNCTION__,RPS_DEBUG__EVERYTHING,
                                "respoll=%d nbrev=%d event_nbloops=%ld\n",
                                respoll, nbrev, event_nbloops.load());
        }
      else if (respoll==0)   // timed out poll
        {
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,__FUNCTION__,RPS_DEBUG__EVERYTHING,
                                "poll timeout loop%ld\n", event_nbloops.load());
        }
      else if (errno != EINTR)
        RPS_FATALOUT("rps_event_loop failure : " << strerror(errno));
      else
        {
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,__FUNCTION__,
                                RPS_DEBUG__EVERYTHING,
                                "poll interrupt loop%ld\n", event_nbloops.load());
        };
      if (Rps_Agenda::agenda_timeout > 0
          && rps_elapsed_real_time() >= Rps_Agenda::agenda_timeout + 2.0)
        {
          RPS_INFORMOUT("stopping event loop because of agenda timeout after "
                        << pollcount << " polling." << std::endl
                        << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop/agenda-timeout"));
          rps_stop_event_loop_flag.store(true);
        };
      fflush(nullptr);
    };       // end while not rps_stop_event_loop_flag
  {
    std::lock_guard<std::recursive_mutex> gu(rps_eventloopdata.eld_mtx);
    rps_eventloopdata.eld_eventloopisactive.store(true);
  }
  /*TODO: cooperation with transientobj_rps.cc ... */
#warning incomplete rps_event_loop see related file transientobj_rps.cc, missing code
  /*TODO: use Rps_PayloadUnixProcess::do_on_active_process_queue to collect file descriptors inside such payloads */

  double endelapsedtime=rps_elapsed_real_time();
  double endcputime=rps_process_cpu_time();
  RPS_INFORMOUT("ended rps_event_loop " << event_nbloops.load() << " times in pid " << (int)getpid() << " on " << rps_hostname()
                << " in " << (endelapsedtime-startelapsedtime) << " elapsed and "
                << (endcputime-startcputime) << " cpu seconds"
                << " git " << rps_shortgitid << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop")
               );
  RPS_DEBUG_LOG(REPL, "rps_event_loop ended elapsedtime=" << startelapsedtime
                << " cputime=" << startcputime
                << " thread:" << rps_current_pthread_name() << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop/ended"));
#undef EXPLAIN_EVFD_AT
#undef EXPLAIN_EVFD_ATBIS
#undef EXPLAIN_EVFD_RPS
} // end rps_event_loop


bool
rps_is_active_event_loop(void)
{
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  return rps_eventloopdata.eld_eventloopisactive.load();
} // end rps_is_active_event_loop

void
rps_sigfd_read_handler(Rps_CallFrame*cf, int fd, void* data)
{
  RPS_DEBUG_LOG (REPL, "rps_sigfd_read_handler fd#" << fd
                 << " thread:" << rps_current_pthread_name()
                 << " data:" << data
                 << RPS_FULL_BACKTRACE_HERE(1, "rps_sigfd_read_handler"));
  RPS_ASSERT (rps_eventloopdata.eld_sigfd>0);
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
  struct signalfd_siginfo infsig;
  memset(&infsig, 0, sizeof(infsig));
  RPS_ASSERT(fd == rps_eventloopdata.eld_sigfd);
  RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
  int nbr = read(fd, (void*)&infsig, sizeof(infsig));
  if (nbr != sizeof(infsig))
    RPS_FATALOUT("rps_sigfd_read_handler read failure on fd#" << fd
                 << " got " << nbr << " bytes, expecting " << sizeof(infsig)
                 << ":" << strerror(errno));
  std::int32_t scod= infsig.ssi_code;
  pid_t origpid= infsig.ssi_pid;
  std::int32_t status= infsig.ssi_status;
  int signum= infsig.ssi_signo;
  RPS_DEBUG_LOG(REPL, "rps_sigfd_read_handler signal#" << signum << ":" << strsignal(signum)
                << " scod:" << scod << " origpid:"<< origpid << " status:" << status);
  switch (infsig.ssi_signo)
    {
    case SIGTERM:
    {
      RPS_INFORMOUT("rps_sigfd_read_handler got SIGTERM from pid " << origpid);
#warning on SIGTERM should schedule a final dump
      rps_do_stop_event_loop();
      /* TODO: perhaps use rps_register_after_event_loop for the final dump? */
    };
    break;
    case SIGINT:
    {
      RPS_INFORMOUT("rps_sigfd_read_handler got SIGINT from pid " << origpid);
      rps_do_stop_event_loop();
    };
    break;
    case SIGQUIT:
    {
      RPS_INFORMOUT("rps_sigfd_read_handler got SIGQUIT from pid " << origpid);
      rps_do_stop_event_loop();
    };
    break;
    case SIGCHLD:
    {
      RPS_INFORMOUT("rps_sigfd_read_handler got SIGCHLD from pid " << origpid
                    << " status:" << status);
    };
    break;
    case SIGUSR1:
    {
      RPS_INFORMOUT("rps_sigfd_read_handler got SIGUSR1 from pid " << origpid);
#warning on SIGUSR1 should schedule a temporary dump
    };
    break;
    default:
      RPS_FATALOUT("rps_sigfd_read_handler got unexpected signal#" << signum << ":" << strsignal(signum));
    };
} // end rps_sigfd_read_handler



void
rps_timerfd_read_handler(Rps_CallFrame*cf, int fd, void* data)
{
  RPS_DEBUG_LOG (REPL, "rps_timerfd_read_handler fd#" << fd
                 << " data:" << data
                 << " thread:" << rps_current_pthread_name() << std::endl
                 << RPS_FULL_BACKTRACE_HERE(1, "rps_timerfd_read_handler"));
  RPS_ASSERT(rps_eventloopdata.eld_magic == RPS_EVENTLOOPDATA_MAGIC);
#warning unimplemented rps_timerfd_read_handler
  RPS_FATALOUT("unimplemented rps_timerfd_read_handler fd#" << fd);
} // end rps_timerfd_read_handler



void
handle_self_pipe_byte_rps(unsigned char b)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_DEBUG_LOG(REPL, "handle_self_pipe_byte_rps b=" << (char)b
                << "#" << (unsigned)b
                << " thread:" << rps_current_pthread_name());
  switch (b)
    {
    case SelfPipe_Dump:
      rps_dump_into (rps_get_loaddir());
      break;
    case SelfPipe_GarbColl:
      rps_garbage_collect();
      break;
    case SelfPipe_Quit:
      rps_do_stop_event_loop();
      break;
    case SelfPipe_Exit:
      rps_dump_into (rps_get_loaddir());
      rps_do_stop_event_loop();
      break;
    case SelfPipe_Process:
#warning should call something from transientobj_rps.cc to perhaps fork a process related to some Rps_PayloadUnixProcess
    /* TODO: see Rps_PayloadUnixProcess::queue_of_runnable_processes
    in transientobj_rps.cc; the dormant processes should somehow
    be started by fork/exec */
    default:
      RPS_FATALOUT("unexpected byte " << (char)b << "#" << (unsigned)b << " on self pipe");
    };
} // end handle_self_pipe_byte_rps

bool
rps_event_loop_is_running(void)
{
  return event_loop_is_active.load();
} // end rps_event_loop_is_running



// Give the counter for the loop, or -1 if it is
// not running.
long
rps_event_loop_counter(void)
{
  if (rps_stop_event_loop_flag.load())
    return -1;
  if (event_loop_is_active.load())
    return event_nbloops.load();
  return -1L;
} // end rps_event_loop_counter

void
rps_postpone_dump(void)
{
  RPS_DEBUG_LOG(REPL, "rps_postpone_dump thread:"
                << rps_current_pthread_name());
  rps_self_pipe_write_byte(SelfPipe_Dump);
} // end rps_postpone_dump

void
rps_postpone_garbage_collection(void)
{
  RPS_DEBUG_LOG(REPL, "rps_postpone_garbage_collection thread:"
                << rps_current_pthread_name());
  rps_self_pipe_write_byte(SelfPipe_GarbColl);
} // end rps_postpone_garbage_collection

void
rps_postpone_quit(void)
{
  RPS_DEBUG_LOG(REPL, "rps_postpone_quit thread:"
                << rps_current_pthread_name());
  rps_self_pipe_write_byte(SelfPipe_Quit);
} // end rps_postpone_quit

void
rps_postpone_exit_with_dump(void)
{
  RPS_DEBUG_LOG(REPL, "rps_postpone_exit_with_dump thread:"
                << rps_current_pthread_name());
  rps_self_pipe_write_byte(SelfPipe_Exit);
} // end rps_postpone_exit_with_dump


void
rps_postpone_child_process(void)
{
  RPS_DEBUG_LOG(REPL, "rps_postpone_child_process thread:"
                << rps_current_pthread_name());
  rps_self_pipe_write_byte(SelfPipe_Process);
} // end rps_postpone_child_process

/// end of file eventloop_rps.cc

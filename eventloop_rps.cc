/****************************************************************
 * file eventloop_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the JSONRPC code to user-interface to an external process (using JSONRPC protocol)
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2022 - 2023 The Reflective Persistent System Team
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

static int sigfd;		// file descriptor for https://man7.org/linux/man-pages/man2/signalfd.2.html
static int timfd;		// file descriptor for https://man7.org/linux/man-pages/man2/timerfd_create.2.html
/**
 * We probably want to use the pipe to self trick.
 * https://www.sitepoint.com/the-self-pipe-trick-explained/
 *
 * in cooperation with Rps_PayloadUnixProcess::start_process
 **/
static int self_pipe_read_fd, self_pipe_write_fd;
static std::deque<unsigned char> self_pipe_fifo;
static std::mutex self_pipe_mtx;

static std::atomic<bool> event_loop_is_active;

static std::atomic<long> nbloops;

enum self_pipe_code_en
{
  SelfPipe__NONE=0,
  SelfPipe_Dump = 'D',
  SelfPipe_GarbColl = 'G',
  SelfPipe_Quit = 'Q',
  SelfPipe_Exit = 'X',
};

extern "C" void rps_self_pipe_write_byte(unsigned char b);

static void handle_self_pipe_byte_rps(unsigned char b);

//extern "C" std::atomic<bool> rps_stop_event_loop_flag;

std::atomic<bool> rps_stop_event_loop_flag;

int rps_poll_delay_millisec = 1500;

#define RPS_MAXPOLL_FD 128


void
rps_do_stop_event_loop(void)
{
  rps_stop_event_loop_flag.store(true);
} // end rps_do_stop_event_loop

extern "C" void jsonrpc_initialize_rps(void);


void
rps_self_pipe_write_byte(unsigned char b)
{
  RPS_ASSERT(b != (char)0);
  std::lock_guard<std::mutex> gu(self_pipe_mtx);
  self_pipe_fifo.push_back(b);
} // end rps_self_pipe_write_byte

bool
rps_is_fifo(std::string path)
{
  struct stat s = {};
  if (!stat(path.c_str(), &s))
    return (s.st_mode & S_IFMT) == S_IFIFO;
  return false;
} // end rps_is_fifo

void
rps_initialize_event_loop(void)
{
  static int count;
  if (!rps_is_main_thread())
    RPS_FATALOUT("rps_initialize_event_loop should be called only from the main thread");
  if (count++ > 0)
    RPS_FATALOUT("rps_initialize_event_loop should be called once");
  /**
   * create the pipe to self
   **/
  {
    int pipefdarr[2] = {-1, -1};
    if (pipe2(pipefdarr, O_CLOEXEC) <0)
      RPS_FATALOUT("rps_initialize_event_loop failed to create pipe to self:" << strerror(errno));
    self_pipe_read_fd = pipefdarr[0];
    RPS_ASSERT(self_pipe_read_fd > 0);
    self_pipe_write_fd = pipefdarr[1];
  }
} // end rps_initialize_event_loop

/**
   Function jsonrpc_initialize_rps is called once from main, when
   rps_fifo_prefix is not empty. The FIFOs have been created in
   main_rps.cc...
***/
void
jsonrpc_initialize_rps(void)
{
  RPS_ASSERT(!rps_get_fifo_prefix().empty());
  struct rps_fifo_fdpair_st fdp = rps_get_gui_fifo_fds();
  if (fdp.fifo_ui_wcmd <= 0)
    RPS_FATALOUT("invalid command FIFO fd " << fdp.fifo_ui_wcmd << " with FIFO prefix " << rps_get_fifo_prefix());
  if (fdp.fifo_ui_rout <= 0)
    RPS_FATALOUT("invalid output FIFO fd " << fdp.fifo_ui_rout << " with FIFO prefix " << rps_get_fifo_prefix());
#warning unimplemented  jsonrpc_initialize_rps
  /** TODO: we probably want to make a first JsonRpc with some meta
   *  data, e.g. a JSON grouping information from __timestamp.c and we
   *  need to document the JSONRPC protocol between the GUI software
   *  and RefPerSys
   *
   * That JSONRPC protocol should also be able to "disconnect" from
   * the RefPerSys process.
  **/
  RPS_FATALOUT("unimplemented jsonrpc_initialize_rps with fifo prefix "
               << rps_get_fifo_prefix() << " and wcmd.fd#" << fdp.fifo_ui_wcmd  << " and rout.fd#" << fdp.fifo_ui_rout);
} // end jsonrpc_initialize_rps

/* TODO: an event loop using poll(2) and also handling SIGCHLD using
   https://man7.org/linux/man-pages/man2/signalfd.2.html
 */
void
rps_event_loop(void)
{
  static int nbcall;
  int nbpoll=0;
  double startelapsedtime=rps_elapsed_real_time();
  double startcputime=rps_process_cpu_time();
  std::array<std::function<void(Rps_CallFrame*, int/*fd*/, short /*revents*/)>,RPS_MAXPOLL_FD+1> handlarr;
  if (!rps_is_main_thread())
    RPS_FATALOUT("rps_event_loop should be called only from the main thread");
  if (nbcall++>0)
    RPS_FATALOUT("rps_event_loop has already been called " << nbcall << " times");

  RPS_LOCALFRAME(RPS_CALL_FRAME_UNDESCRIBED,
                 /*callerframe:*/RPS_NULL_CALL_FRAME, //
                 /** locals **/
                 Rps_Value valarr[RPS_MAXPOLL_FD+1];
                 Rps_ClosureValue closarr[RPS_MAXPOLL_FD+1];
                );
#warning incomplete rps_event_loop
  sigset_t msk= {};
  sigemptyset(&msk);
  sigaddset(&msk, SIGCHLD);
  sigaddset(&msk, SIGTERM);
  sigaddset(&msk, SIGXCPU);
  sigaddset(&msk, SIGALRM);
  sigaddset(&msk, SIGVTALRM);
  sigfd = signalfd(-1, &msk, SFD_CLOEXEC);
  if (sigfd<=0)
    RPS_FATALOUT("failed to call signalfd:" << strerror(errno));
  timfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
  if (timfd<=0)
    RPS_FATALOUT("failed to call timerfd:" << strerror(errno));
#warning TODO: consider using rps_timer ...?
  /*** give output
   ***/
  RPS_INFORMOUT("starting rps_event_loop in pid " << (int)getpid() << " on " << rps_hostname()
                << " git " << rps_shortgitid << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop")
               );
  while (!rps_stop_event_loop_flag.load())
    {
      struct pollfd pollarr[RPS_MAXPOLL_FD+1];
      memset ((void*)&pollarr, 0, sizeof(pollarr));
      const char*explarr[RPS_MAXPOLL_FD+1];
      memset (explarr, 0, sizeof(explarr));
#define EXPLAIN_EVFD_AT(Fil,Lin,Ix,Expl) do { explarr[Ix] = Fil ":" #Lin " " Expl; } while(0)
#define EXPLAIN_EVFD_ATBIS(Fil,Lin,Ix,Expl)  EXPLAIN_EVFD_AT(Fil,Lin,Ix,Expl)
#define EXPLAIN_EVFD_RPS(Ix,Expl) EXPLAIN_EVFD_ATBIS(__FILE__,__LINE__,(Ix),Expl)
      nbloops.fetch_add(1);
      memset ((void*)&pollarr, 0, sizeof(pollarr));
      nbpoll=0;
      struct rps_fifo_fdpair_st fdp = rps_get_gui_fifo_fds();
      if (fdp.fifo_ui_wcmd >0)
        {
          /// JSONRPC commands written from RefPerSys to the GUI process...
          /// could copy paste most of the few lines below
          RPS_ASSERT(nbpoll<RPS_MAXPOLL_FD);
          int pix = nbpoll++;
          pollarr[pix].fd = fdp.fifo_ui_wcmd;
          pollarr[pix].events = POLLOUT;
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            RPS_FATALOUT("missing code to handle JsonRpc output to fd#" << fd << " pix#" << pix);
#warning missing code to handle JsonRpc output to the GUI process
          };
        };
      if (fdp.fifo_ui_rout>0)
        {
          /// JSONRPC responses/events sent by the GUI process to RefPerSys
          RPS_ASSERT(nbpoll<RPS_MAXPOLL_FD);
          int pix = nbpoll++;
          pollarr[pix].fd = fdp.fifo_ui_rout;
          pollarr[pix].events = POLLIN;
          EXPLAIN_EVFD_RPS(pix, "JsonRpc responses from GUI");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            char buf[1024];
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            /* TODO: should read(2) */
            memset(buf, 0, sizeof(buf));
            int nbr = read(fd, buf, sizeof(buf));
            RPS_FATALOUT("missing code to handle JsonRpc input from fd#" << fd << " pix#" << pix);
#warning missing code to handle JsonRpc input from the GUI process
          };
        };
      if (sigfd>0)
        {
          /// signals transformed to data with signalfd(2)
          RPS_ASSERT(nbpoll<RPS_MAXPOLL_FD);
          int pix = nbpoll++;
          pollarr[pix].fd = sigfd;
          pollarr[pix].events = POLLIN;
          EXPLAIN_EVFD_RPS(pix, "signalfd");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            struct signalfd_siginfo infsig;
            memset(&infsig, 0, sizeof(infsig));
            RPS_ASSERT(fd ==  pollarr[pix].fd && fd == sigfd);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            int nbr = read(fd, (void*)&infsig, sizeof(infsig));
            if (nbr != sizeof(infsig))
              RPS_FATALOUT("signalfd read failure on fd#" << fd << " pix#" << pix << " got " << nbr << " bytes, expecting " << sizeof(infsig)
                           << ":" << strerror(errno));
            std::int32_t scod= infsig.ssi_code;
            pid_t origpid= infsig.ssi_pid;
            std::int32_t status= infsig.ssi_status;
            int signum= infsig.ssi_signo;
            RPS_DEBUG_LOG(REPL, "eventloop sigfd signal#" << signum << ":" << strsignal(signum)
                          << " scod:" << scod << " origpid:"<< origpid << " status:" << status);
            switch (infsig.ssi_signo)
              {
              case SIGTERM:
              {
                RPS_INFORMOUT("event loop got SIGTERM from pid " << origpid);
                rps_stop_event_loop_flag.store(true);
              };
              break;
              case SIGQUIT:
              {
                RPS_INFORMOUT("event loop got SIGQUIT from pid " << origpid);
                rps_stop_event_loop_flag.store(true);
              };
              break;
              case SIGINT:
              {
                RPS_INFORMOUT("event loop got SIGINT from pid " << origpid);
              };
              break;
              case SIGCHLD:
              {
                RPS_INFORMOUT("event loop got SIGCHLD from pid " << origpid << " status:" << status);
              };
              break;
              default:
                RPS_FATALOUT("unexpected signal#" << signum << ":" << strsignal(signum));
              };
#warning missing code to handle signalfd...
          };
        }
      if (timfd>0)
        {
          /// timers transformed to data with timerfd_create(2)
          RPS_ASSERT(nbpoll<RPS_MAXPOLL_FD);
          int pix = nbpoll++;
          pollarr[pix].fd = timfd;
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
                  for (int ix=0; ix< (nbr/sizeof(std::uint64_t)); ix++)
                    out << ' ' << timbuf[ix];
                }
                                              )
                    << std::flush);
                RPS_FATALOUT("unimplemented timerfd handling of " << (nbr/sizeof(std::uint64_t)) << " int64s.");
              }
#warning missing code to handle timerfd...
          };
        };
      if (self_pipe_read_fd>0)
        {
          RPS_ASSERT(nbpoll<RPS_MAXPOLL_FD);
          int pix = nbpoll++;
          pollarr[pix].fd = self_pipe_read_fd;
          pollarr[pix].events = POLLIN;
          EXPLAIN_EVFD_RPS(pix, "self_pipe_read_fd");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            unsigned char buf[128];
            RPS_ASSERT(fd == self_pipe_read_fd);
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
        std::lock_guard<std::mutex> gu(self_pipe_mtx);
        wantselfwrite = !self_pipe_fifo.empty();
      }
      if (self_pipe_write_fd>0 && wantselfwrite)
        {
          RPS_ASSERT(nbpoll<RPS_MAXPOLL_FD);
          int pix = nbpoll++;
          pollarr[pix].fd = self_pipe_write_fd;
          pollarr[pix].events = POLLOUT;
          EXPLAIN_EVFD_RPS(pix, "self_pipe_write_fd");
          handlarr[pix] = [&](Rps_CallFrame* cf, int fd, short rev)
          {
            RPS_ASSERT(fd == self_pipe_write_fd);
            RPS_ASSERT(rev == POLLOUT);
            RPS_ASSERT(cf != nullptr && cf->is_good_call_frame());
            std::lock_guard<std::mutex> gu(self_pipe_mtx);
            while (!self_pipe_fifo.empty())
              {
                unsigned char b = self_pipe_fifo.back();
                if (write(fd, &b, 1) > 0)
                  self_pipe_fifo.pop_back();
                else
                  break;
              };
          };
        };
      bool debugpoll = RPS_DEBUG_ENABLED(REPL) //
                       || RPS_DEBUG_ENABLED(EVENT_LOOP) //
                       || RPS_DEBUG_ENABLED(GUI);
      if (debugpoll)
        {
          for (int pix=0; pix<nbpoll; pix++)
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
              rps_debug_printf_at(__FILE__,__LINE__,RPS_DEBUG__EVERYTHING,
                                  "poll[%d] loop%ld:fd#%d:%s,%s\n",
                                  pix, nbloops.load(), pollarr[pix].fd, explarr[pix], evstr.c_str());
            }
        };
      errno = 0;
      int respoll = poll(pollarr, nbpoll, (rps_poll_delay_millisec*(debugpoll?3:1)));
      if (respoll>0)
        {
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,RPS_DEBUG__EVERYTHING,
                                "respoll=%d loop%ld\n", respoll, nbloops.load());
          int nbrev=0;
          for (int pix=0; pix<nbpoll; pix++)
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
                      rps_debug_printf_at(__FILE__,__LINE__,RPS_DEBUG__EVERYTHING,
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
            rps_debug_printf_at(__FILE__,__LINE__,RPS_DEBUG__EVERYTHING,
                                "respoll=%d nbrev=%d nbloops=%ld\n",
                                respoll, nbrev, nbloops.load());
        }
      else if (respoll==0)   // timed out poll
        {
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,RPS_DEBUG__EVERYTHING,
                                "poll timeout loop%ld\n", nbloops.load());
        }
      else if (errno != EINTR)
        RPS_FATALOUT("rps_event_loop failure : " << strerror(errno));
      else
        {
          if (debugpoll)
            rps_debug_printf_at(__FILE__,__LINE__,RPS_DEBUG__EVERYTHING,
                                "poll interrupt loop%ld\n", nbloops.load());
        };
      fflush(nullptr);
    };		   // end while not rps_stop_event_loop_flag
  /*TODO: cooperation with transientobj_rps.cc ... */
#warning incomplete rps_event_loop see related file transientobj_rps.cc, missing code
  /*TODO: use Rps_PayloadUnixProcess::do_on_active_process_queue to collect file descriptors inside such payloads */

  double endelapsedtime=rps_elapsed_real_time();
  double endcputime=rps_process_cpu_time();
  RPS_INFORMOUT("ended rps_event_loop " << nbloops.load() << " times in pid " << (int)getpid() << " on " << rps_hostname()
                << " in " << (endelapsedtime-startelapsedtime) << " elapsed and "
                << (endcputime-startcputime) << " cpu seconds"
                << " git " << rps_shortgitid << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "rps_event_loop")
               );
#undef EXPLAIN_EVFD_AT
#undef EXPLAIN_EVFD_ATBIS
#undef EXPLAIN_EVFD_RPS
} // end rps_event_loop

void
handle_self_pipe_byte_rps(unsigned char b)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_DEBUG_LOG(REPL, "handle_self_pipe_byte_rps b=" << (char)b << "#" << (unsigned)b);
  switch (b)
    {
    case SelfPipe_Dump:
      rps_dump_into (rps_get_loaddir());
      break;
    case SelfPipe_GarbColl:
      rps_garbage_collect();
      break;
    case SelfPipe_Quit:
      rps_stop_event_loop_flag.store(true);
      break;
    case SelfPipe_Exit:
      rps_dump_into (rps_get_loaddir());
      rps_stop_event_loop_flag.store(true);
      break;
    default:
      RPS_FATALOUT("unexpected byte " << (char)b << "#" << (unsigned)b << " on self pipe");
    };
} // end handle_self_pipe_byte_rps

bool
rps_event_loop_is_running(void)
{
  if (rps_stop_event_loop_flag.load())
    return false;
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
    return nbloops.load();
  return -1L;
} // end rps_event_loop_counter

void
rps_postpone_dump(void)
{
  rps_self_pipe_write_byte(SelfPipe_Dump);
} // end rps_postpone_dump

void
rps_postpone_garbage_collection(void)
{
  rps_self_pipe_write_byte(SelfPipe_GarbColl);
} // end rps_postpone_garbage_collection

void
rps_postpone_quit(void)
{
  rps_self_pipe_write_byte(SelfPipe_Quit);
} // end rps_postpone_quit

void
rps_postpone_exit_with_dump(void)
{
  rps_self_pipe_write_byte(SelfPipe_Exit);
} // end rps_postpone_exit_with_dump

/// end of file eventloop_rps.cc

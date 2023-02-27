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

//extern "C" std::atomic<bool> rps_stop_event_loop_flag;

std::atomic<bool> rps_stop_event_loop_flag;

const int rps_poll_delay_millisec = 500;

#define RPS_MAXPOLL_FD 128


void
rps_do_stop_event_loop(void)
{
  rps_stop_event_loop_flag.store(true);
} // end rps_do_stop_event_loop

extern "C" void jsonrpc_initialize_rps(void);

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
  RPS_WARNOUT("unimplemented rps_initialize_event_loop");
#warning unimplemented rps_initialize_event_loop
  /** TODO:
   *
   * create the pipe to self
   **/
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
  struct pollfd pollarr[RPS_MAXPOLL_FD+1];
  memset ((void*)&pollarr, 0, sizeof(pollarr));
  std::array<std::function<void(int/*fd*/, short /*revents*/)>,RPS_MAXPOLL_FD+1> handlarr;
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
  timfd = timerfd_create(CLOCK_REALTIME_ALARM, TFD_CLOEXEC);
  if (timfd<=0)
    RPS_FATALOUT("failed to call timerfd:" << strerror(errno));
  while (!rps_stop_event_loop_flag.load())
    {
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
          handlarr[pix] = [&](int fd, short rev)
          {
            RPS_ASSERT(fd ==  pollarr[pix].fd);
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
          handlarr[pix] = [&](int fd, short rev)
          {
            char buf[1024];
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            /* TODO: should read(2) */
            memset(buf, 0, sizeof(buf));
            int nbr = read(fd, buf, sizeof(buf));
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
          handlarr[pix] = [&](int fd, short rev)
          {
            struct signalfd_siginfo infsig;
            memset(&infsig, 0, sizeof(infsig));
            RPS_ASSERT(fd ==  pollarr[pix].fd && fd == sigfd);
            int nbr = read(fd, (void*)&infsig, sizeof(infsig));
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
          handlarr[pix] = [&](int fd, short rev)
          {
            RPS_ASSERT(fd ==  pollarr[pix].fd);
            uint64_t timbuf[16];
            memset (&timbuf, 0, sizeof(timbuf));
            int nbr = read(fd, (void*)&timbuf, sizeof(timbuf));
#warning missing code to handle timerfd...
          };
        };
      errno = 0;
      int respoll = poll(pollarr, nbpoll, rps_poll_delay_millisec);
      if (respoll>0)
        {
          for (int pix=0; pix<nbpoll; pix++)
            {
              if (pollarr[pix].revents > 0)
                {
                  if (handlarr[pix])
                    handlarr[pix](pollarr[pix].fd, pollarr[pix].revents);
                };
            }
        }
      else if (respoll==0)   // timed out poll
        {
        }
      else if (errno != EINTR)
        RPS_FATALOUT("rps_event_loop failure : " << strerror(errno));
    };		   // end while not rps_stop_event_loop_flag
  /*TODO: cooperation with transientobj_rps.cc ... */
#warning incomplete rps_event_loop see related file transientobj_rps.cc, missing code
  /*TODO: use Rps_PayloadUnixProcess::do_on_active_process_queue to collect file descriptors inside such payloads */
  RPS_FATALOUT("unimplemented rps_event_loop");
} // end rps_event_loop




/// end of file eventloop_rps.cc

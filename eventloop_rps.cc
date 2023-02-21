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


extern "C" const char rps_jsonrpc_gitid[];
const char rps_jsonrpc_gitid[]= RPS_GITID;

extern "C" const char rps_jsonrpc_date[];
const char rps_jsonrpc_date[]= __DATE__;

static int sigfd;		// file descriptor for https://man7.org/linux/man-pages/man2/signalfd.2.html

extern "C" void jsonrpc_initialize_rps(void);

bool
rps_is_fifo(std::string path)
{
  struct stat s = {};
  if (!stat(path.c_str(), &s))
    return (s.st_mode & S_IFMT) == S_IFIFO;
  return false;
} // end rps_is_fifo

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
#warning unimplemented rps_event_loop
  RPS_ASSERT(rps_is_main_thread());
  sigset_t msk={};
  sigemptyset(&msk);
  sigaddset(&msk, SIGCHLD);
  sigaddset(&msk, SIGTERM);
  sigaddset(&msk, SIGXCPU);
  sigaddset(&msk, SIGALRM);
  sigaddset(&msk, SIGVTALRM);
  sigfd = signalfd(-1, &msk, SFD_CLOEXEC);
  if (sigfd<0)
    RPS_FATALOUT("failed to call signalfd:" << strerror(errno));
#warning related file transientobj_rps.cc
  RPS_FATALOUT("unimplemented rps_event_loop");
} // end rps_event_loop




/// end of file eventloop_rps.cc

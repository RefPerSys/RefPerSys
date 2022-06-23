/****************************************************************
 * file jsonrpc_rps.cc
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
 *      © Copyright 2022 The Reflective Persistent System Team
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

extern "C" void jsonrpc_initialize_rps(void);
extern "C" void jsonrpc_run_application_rps(void);

/**
   Function jsonrpc_initialize_rps is called once from main, when
   rps_fifo_prefix is not empty. It is expected to create the FIFOs if
   they dont exist ....
***/
void
jsonrpc_initialize_rps(void)
{
#warning unimplemented  jsonrpc_initialize_rps
  RPS_FATALOUT("unimplemented jsonrpc_initialize_rps with fifo prefix "
	       << rps_get_fifo_prefix());
} // end jsonrpc_initialize_rps

void
jsonrpc_run_application_rps(void)
{
#warning jsonrpc_run_application_rps unimplemented
  RPS_FATALOUT("unimplemented jsonrpc_run_application_rps with fifo prefix "
	       << rps_get_fifo_prefix());
} // end jsonrpc_run_application_rps

/// end of file jsonrpc_rps.cc

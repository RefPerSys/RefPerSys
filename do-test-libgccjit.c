/// file do-test-libgccjit.c in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///
/// Purpose: configure-time testing for the RefPerSys inference
/// engine of the libgccjit library see https://gcc.gnu.org/onlinedocs/jit/
///
/// this file is compiled by do-configure-refpersys executable into a
/// do-test-libgccjit.so plugin to be dlopen-ed by the same
/// do-configure-refpersys executable...
///
/// Author(s):
///      Basile Starynkevitch <basile@starynkevitch.net>

/// License: GPLv3+ (file COPYING-GPLv3)
///    This software is free software: you can redistribute it and/or modify
///    it under the terms of the GNU General Public License as published by
///    the Free Software Foundation, either version 3 of the License, or
///   (at your option) any later version.
///
///  This program is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details or the Lesser
///  General Public License.
///
///  You should have received a copy of the GNU General Public License
///  along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/
#include <sys/utsname.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "libgccjit.h"

#ifndef RPSJIT_GITID
#error RPSJIT_GITID should be set at compilation time
#endif

const char rpsjit_gitid[] = RPSJIT_GITID;
const char rpsjit_build[] = __DATE__ "@" __TIME__;

extern const char rpsconf_gitid[];

#ifndef LIBGCCJIT_HAVE_REFLECTION
#error libgccjit is required to have LIBGCCJIT_HAVE_REFLECTION provided in GCC 12
#endif /* LIBGCCJIT_HAVE_REFLECTION */

void
rpsjit_test (void)
{
#warning rpsjit_test incomplete
  fprintf (stderr, "rpsjit_test incomplete at %s:%d\n", __FILE__, __LINE__);
  exit (EXIT_FAILURE);
}				/* end rpsjit_test */



void
rpsjit_initialize (void)
{
#warning unimplemented rpsjit_initialize
}				/* end rpsjit_initialize */

void
rpsjit_finalize (void)
{
#warning unimplemented rpsjit_finalize
}				/* end rpsjit_finalize */

const char *
rpsjit_get_version_string (void)
{
  static char buf[128];
  memset (buf, 0, sizeof (buf));
  snprintf (buf, sizeof (buf), "%d.%d.%d",
	    gcc_jit_version_major (), gcc_jit_version_minor (),
	    gcc_jit_version_patchlevel ());
  return buf;
}				/* end rpsjit_version_string */

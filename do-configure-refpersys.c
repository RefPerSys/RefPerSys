/// file do-configure-refpersys.c in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///
/// Purpose: build-time configuration of the RefPerSys inference
/// engine.
///
/// Caveat: this program should run quickly and consume few memory. So
/// we never call free here!
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
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <dlfcn.h>

#ifndef WITHOUT_READLINE
#include "readline/readline.h"
#endif

char *
my_readline (const char *prompt)
{
#ifndef WITHOUT_READLINE
  return readline (prompt);
#else
  char linebuf[512];
  memset (linebuf, 0, linebuf);
  puts (prompt);
  fflush (stdout);
  char *p = fgets (linebuf, sizeof (linebuf), stdin);
  if (!p)
    return NULL;
  linebuf[sizeof (linebuf) - 1] = (char) 0;
  char *res = strdup (linebuf);
  if (!res)
    {
      perror ("my_readline");
      exit (EXIT_FAILURE);
    };
  return res;
#endif // WITHOUT_READLINE
}				// end my_readline


int
main (int argc, char **argv)
{
}				/* end main */


/// eof do-configure-refpersys.c

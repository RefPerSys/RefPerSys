/// file do-scan-pkgconfig.c in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///

///
/// Purpose: scan C++ code for //!PKGCONFIG comments

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
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#ifndef MAX_PROG_ARGS
#define MAX_PROG_ARGS 1024
#endif

// maximal path length of given C++ file arguments
#ifndef MY_PATH_MAXLEN
#define MY_PATH_MAXLEN 384
#endif

// maximal line width in bytes
#ifndef MY_LINE_MAXLEN
#define MY_LINE_MAXLEN 512
#endif

#ifndef MY_HEAD_LINES_THRESHOLD
#define MY_HEAD_LINES_THRESHOLD 256
#endif

#ifndef GIT_ID
#error GIT_ID should be defined at compilation time
#endif

const char *prog_name;
//// working directory
char my_cwd_buf[MY_PATH_MAXLEN];

//// host name
char my_host_name[80];

void
usage (void)
{
  puts ("# C++ scanning utility program for refpersys.org");
  printf ("%s usage:              #from [%s:%d]\n", prog_name,
	  __FILE__, __LINE__ - 1);
  puts ("\t --version             # show version");
  puts ("\t --help                # this help");
  puts ("\t <C++-files>           # sequence of *.cc files to scan");
  puts ("# a file foo.cc is scanned for //!PKGCONFIG <package-name> lines");
  puts ("# a _foo_pkgconfig.mk textual file is generated for GNU make");
  puts ("# that file defines a foo_PKGCONFIG variable for GNU make");
  puts ("# GPLv3+ licensed, so no warranty");
  printf ("# scanning first %d lines in given files\n",
	  MY_HEAD_LINES_THRESHOLD);
  printf ("# lines of at most %d bytes\n", MY_LINE_MAXLEN);
}				/* end usage */


void
process_source_file (const char *origpath)
{
  char pathbuf[MY_PATH_MAXLEN];
  char linebuf[MY_LINE_MAXLEN + 4];
  memset (pathbuf, 0, sizeof (pathbuf));
  memset (linebuf, 0, sizeof (linebuf));
  assert (origpath != NULL);
  if (strlen (origpath) >= MY_PATH_MAXLEN)
    {
      fprintf (stderr, "%s given too long path %s (max is %d) [%s:%d]\n",
	       prog_name, origpath, MY_PATH_MAXLEN, __FILE__, __LINE__ - 2);
      fflush (NULL);
      exit (EXIT_FAILURE);
    };
  strcpy (pathbuf, origpath);
  FILE *f = fopen (pathbuf, "r");
  if (!f)
    {
      fprintf (stderr, "%s failed to open %s: %m [%s:%d]\n",
	       prog_name, pathbuf, __FILE__, __LINE__ - 2);
      fflush (NULL);
      exit (EXIT_FAILURE);
    }
  int linenum = 0;
  do
    {
      memset (linebuf, 0, sizeof (linebuf));
      if (!fgets (linebuf, MY_LINE_MAXLEN, f))
	break;
      linenum++;
      if (linenum > MY_HEAD_LINES_THRESHOLD)
	break;
#warning missing code to scan line in process_source_file
    }
  while (!feof (f));
  fclose (f);
}				/* end process_source_file */

int
main (int argc, char **argv)
{
  prog_name = argv[0];
  if (argc >= 2 && !strcmp (argv[1], "--help"))
    {
      usage ();
      return 0;
    };
  if (argc >= 2 && !strcmp (argv[1], "--version"))
    {
      printf ("%s version gitid %s built on %s:%s\n",
	      prog_name, GIT_ID, __DATE__, __TIME__);
      return 0;
    };
  memset (my_cwd_buf, 0, sizeof (my_cwd_buf));
  if (!getcwd (my_cwd_buf, sizeof (my_cwd_buf)))
    {
      fprintf (stderr, "%s failed to getcwd (%m) [%s:%d]\n",
	       prog_name, __FILE__, __LINE__ - 1);
      exit (EXIT_FAILURE);
    };
  memset (my_host_name, 0, sizeof (my_host_name));
  if (gethostname (my_host_name, sizeof (my_host_name) - 1))
    {
      fprintf (stderr, "%s failed to gethostname (%m) [%s:%d]\n",
	       prog_name, __FILE__, __LINE__ - 1);
      exit (EXIT_FAILURE);
    };
  assert (sizeof (my_cwd_buf) == MY_PATH_MAXLEN);
  if (my_cwd_buf[MY_PATH_MAXLEN - 2])
    {
      my_cwd_buf[MY_PATH_MAXLEN - 1] = (char) 0;
      fprintf (stderr,
	       "%s failed too long current working directory %s [%s:%d]\n",
	       prog_name, my_cwd_buf, __FILE__, __LINE__ - 1);
      exit (EXIT_FAILURE);
    };
  if (argc > MAX_PROG_ARGS)
    {
      fprintf (stderr,
	       "%s (from C file %s) limits MAX_PROG_ARGS to %d\n"
	       "... but %d are given! Edit it and recompile!\n", argv[0],
	       __FILE__, MAX_PROG_ARGS, argc);
      exit (EXIT_FAILURE);
    };
  printf ("# generated by %s on %s from %d files git %s [%s:%d]\n",
	  prog_name, my_host_name, argc - 1, GIT_ID, __FILE__, __LINE__ - 2);
  {
    time_t nowt = 0;
    time (&nowt);
    char timbuf[MY_HEAD_LINES_THRESHOLD];
    memset (timbuf, 0, sizeof (timbuf));
    strftime (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S %Z",
	      localtime (&nowt));
    printf ("# generated at %s\n", timbuf);
  }
  for (int i = 1; i < argc; i++)
    process_source_file (argv[i]);
  fflush(NULL);
  return 0;
}				/* end function main */


/// end of file do-scan-pkgconfig.c

/// file do-scan-refpersys-pkgconfig.c in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///

///
/// Purpose: utility to scan C++ code for //@PKGCONFIG comments

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
#include <ctype.h>
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


/// Caution, this MY_HEAD_LINES_THRESHOLD should also be
/// BP_HEAD_LINES_THRESHOLD in do-build-refpersys-plugin.cc ... better
/// yet, add a --print-head-lines-threshold argument to this program,
/// or even inside refpersys heap and in some generated/*.h file.

#ifndef MY_HEAD_LINES_THRESHOLD
#define MY_HEAD_LINES_THRESHOLD 512
#endif

#ifndef MY_MAX_PACKAGE
#define MY_MAX_PACKAGE 32
#endif

#ifndef GIT_ID
#error GIT_ID should be defined at compilation time
#endif

const char *prog_name;
//// working directory
char my_cwd_buf[MY_PATH_MAXLEN];


bool my_raw_mode;

//// host name
char my_host_name[80];

#define noprintf(Fmt,...) do { if (false) printf(Fmt, __VA_ARGS__); }while(0)

void
usage (void)
{
  puts ("# C++ scanning utility program for refpersys.org");
  printf ("%s usage:              #from [%s:%d]\n", prog_name,
          __FILE__, __LINE__ - 1);
  puts ("\t --version             # show version");
  puts ("\t --help                # this help");
  puts ("\t --raw                 # generate the list of packages only\n"
        "\t                       # one per line");
  puts ("\t <C++-files>           # sequence of *.cc files to scan");
  puts ("# a file foo.cc is scanned for //!PKGCONFIG <package-name> lines");
  puts ("# a _foo_pkgconfig.mk textual file is generated for GNU make");
  puts ("# that file defines a foo_PKGCONFIG variable for GNU make");
  puts ("# GPLv3+ licensed, so no warranty");
  printf ("# scanning first %d lines in given files\n",
          MY_HEAD_LINES_THRESHOLD);
  printf ("# lines of at most %d bytes\n", MY_LINE_MAXLEN);
  printf ("# at most %d packages per scanned source file\n", MY_MAX_PACKAGE);
}       /* end usage */


void
process_source_file (const char *origpath)
{
  char pathbuf[MY_PATH_MAXLEN];
  char linebuf[MY_LINE_MAXLEN + 4];
  // for a dir/foo_rps.cc file the naked basename is foo_rps
  char my_naked_basename[MY_PATH_MAXLEN];
  char pkgbuf[80];
  char *pkgarr[MY_MAX_PACKAGE];
  int nbpkg = 0;
  memset (pathbuf, 0, sizeof (pathbuf));
  memset (linebuf, 0, sizeof (linebuf));
  memset (pkgarr, 0, sizeof (pkgarr));
  memset (pkgbuf, 0, sizeof (pkgbuf));
  memset (my_naked_basename, 0, sizeof (my_naked_basename));
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
  noprintf ("# [%s:%d] reading pathbuf=%s git %s\n",
            __FILE__, __LINE__ - 1, pathbuf, GIT_ID);
  char *lastdot = strrchr (pathbuf, '.');
  char *lastslash = strrchr (pathbuf, '/');
  /// the asm volatile is to ease debugging and gdb breakpoints
  asm volatile ("nop; nop; nop; nop");
  if (lastslash && lastdot && lastdot > lastslash)
    {
      strncpy (my_naked_basename, lastslash + 1, lastdot - lastslash - 1);
      noprintf ("# [%s:%d] pathbuf=%s lastdot=%s lastslash=%s nakedbase=%s\n",
                __FILE__, __LINE__ - 1, pathbuf, lastdot, lastslash,
                my_naked_basename);
    }
  else if (!lastslash && lastdot)
    {
      assert (lastdot > pathbuf && lastdot < pathbuf + MY_PATH_MAXLEN);
      strncpy (my_naked_basename, pathbuf, lastdot - pathbuf);
      noprintf ("# [%s:%d] pathbuf=%s NOlastslash lastdot=%s nakedbase=%s\n",
                __FILE__, __LINE__ - 1, pathbuf, lastdot, my_naked_basename);
    };
  if (!my_naked_basename[0])
    {
      fprintf (stderr,
               "%s failed to compute naked basename for %s [%s:%d git %s]\n",
               prog_name, pathbuf, __FILE__, __LINE__ - 1, GIT_ID);
      exit (EXIT_FAILURE);
    };
  for (const char *pc = my_naked_basename; *pc; pc++)
    {
      if (!isalnum (*pc) && *pc != '_')
        {
          fprintf (stderr,
                   "%s for file %s has naked basename with invalid char %c\n",
                   prog_name, pathbuf, *pc);
          fprintf (stderr,
                   "the naked basename %s should be a valid C identifier\n",
                   my_naked_basename);
          exit (EXIT_FAILURE);
        }
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
      if (linebuf[0] != '/' && linebuf[1] != '/')
        continue;
      memset (pkgbuf, 0, sizeof (pkgbuf));
      assert (sizeof (pkgbuf) > 64);
      if (sscanf (linebuf, "//@@PKGCONFIG %64[A-Za-z.+0-9-]", pkgbuf) > 0
          && isalpha (pkgbuf[0]))
        {
          assert (strlen (pkgbuf) < sizeof (pkgbuf) - 1);
          if (nbpkg > MY_MAX_PACKAGE)
            {
              fprintf (stderr,
                       "%s: too many (%d) packages in source file %s [%s:%d git %s]\n",
                       prog_name, nbpkg, pathbuf,
                       __FILE__, __LINE__ - 2, GIT_ID);
              exit (EXIT_FAILURE);
            };
          char *pkgname = strdup (pkgbuf);
          if (!pkgname)
            {
              fprintf (stderr,
                       "%s: failed to strdup %s (%m) [%s:%d git %s]\n",
                       prog_name, pkgbuf, __FILE__, __LINE__ - 1, GIT_ID);
              exit (EXIT_FAILURE);
            };
          pkgarr[nbpkg++] = pkgname;
        }
    }
  while (!feof (f));
  fclose (f);
  if (nbpkg == 0)
    {
      if (!my_raw_mode)
        printf ("# source file %s without //@@PKGCONFIG comments\n", pathbuf);
    }
  else
    {
      if (!my_raw_mode)
        {
          printf ("# source file %s with %d //@@PKGCONFIG comment lines\n",
                  pathbuf, nbpkg);
          printf ("PKGLIST_%s=", my_naked_basename);
          for (int i = 0; i < nbpkg; i++)
            {
              if (i > 0)
                putchar (' ');
              fputs (pkgarr[i], stdout);
            }
          putchar ('\n');
        };
      for (int i = 0; i < nbpkg; i++)
        {
          if (my_raw_mode)
            printf ("%s\n", pkgarr[i]);
          else
            printf ("PACKAGES_LIST += %s\n", pkgarr[i]);
        }
      putchar ('\n');
      putchar ('\n');
    };
  fflush (NULL);
}       /* end process_source_file */

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
  if (argc >= 2 && !strcmp (argv[1], "--raw"))
    my_raw_mode = true;
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
  if (!my_raw_mode)
    {
      printf ("# generated by %s on %s from %d files git %s [%s:%d]\n",
              prog_name, my_host_name, argc - 1, GIT_ID, __FILE__,
              __LINE__ - 2);
      {
        time_t nowt = 0;
        time (&nowt);
        char timbuf[MY_HEAD_LINES_THRESHOLD];
        memset (timbuf, 0, sizeof (timbuf));
        strftime (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S %Z",
                  localtime (&nowt));
        printf ("# generated at %s\n", timbuf);
        printf ("PACKAGES_LIST=\n");
      }
    };
  for (int i = (my_raw_mode?2:1); i < argc; i++)
    process_source_file (argv[i]);
  fflush (NULL);
  return 0;
}       /* end function main */


/// end of file do-scan-refpersys-pkgconfig.c

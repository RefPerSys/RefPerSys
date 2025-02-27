/// file do-configure-refpersys.c in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 - 2025 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///
/// Purpose: build-time configuration of the RefPerSys inference
/// engine.
///
/// Caveat: this program should run quickly and consumes and leaks a
/// few memory. So we never call free here!
///
/// Convention: global names are prefixed with rpsconf_
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
#include <dlfcn.h>
#include <stdlib.h>

#ifndef RPSCONF_WITHOUT_NCURSES
#include "ncurses.h"
WINDOW *rpsconf_ncurses_window;
#endif /*RPSCONF_WITHOUT_NCURSES */

#ifndef RPSCONF_WITHOUT_READLINE
#include "readline/readline.h"
#endif /*RPSCONF_WITHOUT_READLINE */

#ifndef RPSCONF_WITHOUT_GCCJIT
#include <libgccjit.h>
#endif /*RPSCONF_WITHOUT_GCCJIT */

#ifndef RPSCONF_MAX_PROG_ARGS
#define RPSCONF_MAX_PROG_ARGS 1024
#endif /* RPSCONF_MAX_PROG_ARGS */

#ifndef RPSCONF_PATH_MAXLEN
#define RPSCONF_PATH_MAXLEN 384
#endif /* RPSCONF_PATH_MAXLEN */

#ifndef RPSCONF_GIT_ID
// fictional GIT_ID could be "b32bcbf69070+"
#error RPSCONF_GIT_ID should be defined thru compilation command "short git id"
#endif /* RPSCONF_GIT_ID */

#ifndef RPSCONF_OPERSYS
// usually RPSCONF_OPERSYS would be "GNU_Linux" given by:
/////  /bin/uname -o | /bin/sed 1s/[^a-zA-Z0-9_]/_/g
#error RPSCONF_OPERSYS should be defined thru compilation command "operating system"
#endif /* RPSCONF_OPERSYS */

#ifndef RPSCONF_ARCH
// example RPSCONF_ARCH could be "aarch64" or "x86_64" given by /bin/uname -m
#error RPSCONF_ARCH should be defined thru compilation command "machine architecture"
#endif

#ifndef RPSCONF_HOST
// fictional HOST might be "refpersys.com" given by /bin/hostname -f
#error HOST should be defined thru compilation command "hostname"
#endif

const char rpsconf_gitid[] = RPSCONF_GIT_ID;
const char rpsconf_opersys[] = RPSCONF_OPERSYS;
const char rpsconf_arch[] = RPSCONF_ARCH;
const char rpsconf_host[] = RPSCONF_HOST;

#define RPSCONF_OK 0
#define RPSCONF_FAIL -1

const char *rpsconf_prog_name;
bool rpsconf_verbose = 1; /* will be set later with command line flag */
bool rpsconf_failed;

#ifndef RPSCONF_WITHOUT_GCCJIT
gcc_jit_context *rpsconf_gccjit_ctxt;
gcc_jit_result *rpsconf_gccjit_result;
#endif /*RPSCONF_WITHOUT_GCCJIT */

//// working directory
char rpsconf_cwd_buf[RPSCONF_PATH_MAXLEN];

//// host name
char rpsconf_host_name[80];

//// the below arguments are kept, they should be rewritten in
//// _config-refpersys.mk in a GNU make friendly way.

char *rpsconf_preprocessor_args[RPSCONF_MAX_PROG_ARGS];
int rpsconf_preprocessor_argcount;
char *rpsconf_compiler_args[RPSCONF_MAX_PROG_ARGS];
int rpsconf_compiler_argcount;
char *rpsconf_linker_args[RPSCONF_MAX_PROG_ARGS];
int rpsconf_linker_argcount;

/* absolute path to C and C++ GCC compilers */
const char *rpsconf_c_compiler;
const char *rpsconf_cpp_compiler;

/* absolute path to libgccjit include directory */
const char *rpsconf_libgccjit_include_dir;

/* strdup-ed string of the person building RefPerSys (or null): */
const char *rpsconf_builder_person;

/* strdup-ed string of the email of the person building RefPerSys (or null): */
const char *rpsconf_builder_email;

/* absolute path to fltk-config utility */
const char *rpsconf_fltk_config;




#ifndef MAX_REMOVED_FILES
#define MAX_REMOVED_FILES 4096
#endif

#ifndef RPSCONF_BUFFER_SIZE
#define RPSCONF_BUFFER_SIZE 1024
#endif /*RPSCONF_BUFFER_SIZE */

const char *rpsconf_files_to_remove_at_exit[MAX_REMOVED_FILES];
int rpsconf_removed_files_count;

/// return a malloced path to a temporary textual file inside /tmp
char *rpsconf_temporary_textual_file (const char *prefix, const char *suffix,
                                      int lineno);
/// return a malloced path to a temporary binary file in the current directory
char *rpsconf_temporary_binary_file (const char *prefix, const char *suffix,
                                     int lineno);

/// emit the configure-refperys.mk file to be included in GNUmakefile
void rpsconf_emit_configure_refpersys_mk (void);

/* see gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html */
#ifdef __GNUC__
#define RPSCONF_ATTR_PRINTF(x, y) __attribute__((format(printf, 3, 4)))
#else
#define RPSCONF_ATTR_PRINTF(x, y)
#endif

static void
rpsconf_diag__ (const char *, int, const char *, ...)
RPSCONF_ATTR_PRINTF (3, 4);
static int rpsconf_cc_set (const char *);
static void rpsconf_cc_test (const char *);

void rpsconf_try_then_set_cxx_compiler (const char *cxx);
void rpsconf_should_remove_file (const char *path, int lineno);


/* Wrapper macro around rpsconf_diag__() */
#define RPSCONF_DIAG(msg, ...)         \
  rpsconf_diag__(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/*
 * Interface for rpsconf_trash
 */
struct rpsconf_trash
{
  const char *pathv_[4096];
  int pathc_;
  char state_;
};

static struct rpsconf_trash *rpsconf_trash_get_ (void);
static void rpsconf_trash_push_ (const char *, int);
static void rpsconf_trash_exit (void);
#define rpsconf_trash_push(path) rpsconf_trash_push_((path), __LINE__)

struct rpsconf_trash *rpsconf_trash_get_ (void)
{
  static struct rpsconf_trash ctx;
  static bool init = false;

  if (!init)
    {
      ctx.pathc_ = 0;
      ctx.state_ = EXIT_SUCCESS;
      memset (ctx.pathv_, 0, sizeof (ctx.pathv_));
      init = true;
    }

  return &ctx;
}

void
rpsconf_trash_push_ (const char *path, int line)
{
  struct rpsconf_trash *ctx;

  assert (path != NULL && *path != '\0');
  if (access (path, F_OK) == -1)
    return;

  ctx = rpsconf_trash_get_ ();
  if (ctx->pathc_ > (int) sizeof (ctx->pathv_))
    {
      fprintf (stderr, "%s: %s: too many files to remove [%s:%d]\n",
               rpsconf_prog_name, path, __FILE__, line);
      ctx->state_ = EXIT_FAILURE;
      exit (ctx->state_);
    }

  ctx->pathv_[ctx->pathc_++] = path;
}

void
rpsconf_trash_exit (void)
{
  struct rpsconf_trash *ctx;
  int i;

  ctx = rpsconf_trash_get_ ();
  if (ctx->state_ == EXIT_FAILURE)
    {
      fprintf (stderr,
               "%s: exit failure: not removing %d files [%s:%d]\n",
               rpsconf_prog_name, ctx->pathc_, __FILE__, __LINE__ - 2);
      return;
    }

  fprintf (stderr, "%s: removing %d files at exit [%s:%d]\n",
           rpsconf_prog_name, ctx->pathc_, __FILE__, __LINE__ - 2);

  for (i = 0; i < ctx->pathc_; i++)
    unlink (ctx->pathv_[i]);
}

/* End rpsconf_trash interface */


/// return a malloced path to a temporary textual file
char *
rpsconf_temporary_textual_file (const char *prefix,
                                const char *suffix, int lineno)
{
  char buf[256];
  memset (buf, 0, sizeof (buf));
  char *res = NULL;
  assert (prefix != NULL);
  assert (isalnum (prefix[0]) || prefix[0] == '_');
  for (const char *p = prefix; p && *p; p++)
    assert (isalnum (*p) || *p == '_' || *p == '-');
  if (!suffix)
    suffix = "";
  int suflen = strlen (suffix);
  assert (strlen (prefix) + suflen < sizeof (buf) + 16);
  snprintf (buf, sizeof (buf), "/tmp/%s-l%d_XXXXXX", prefix, lineno);
  int fd = mkostemp (buf, suflen);
  if (fd < 0)
    {
      fprintf (stderr,
               "%s failed to mkostemps from %s:%d\n", rpsconf_prog_name,
               __FILE__, lineno);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  strcat (buf, suffix);
  res = strdup (buf);
  if (!res)
    {
      fprintf (stderr, "%s failed to strdup temporary file path %s" //
               " from %s:%d (%s)\n",
               rpsconf_prog_name, buf, __FILE__, lineno, strerror (errno));
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  close (fd);
  printf ("%s temporary textual file is %s [%s:%d]\n",
          rpsconf_prog_name, res, __FILE__, lineno);
  return res;
}       /* end rpsconf_temporary_textual_file */

/// return a malloced path to a temporary binary file in the current directory
char *
rpsconf_temporary_binary_file (const char *prefix, const char *suffix,
                               int lineno)
{
  char buf[256];
  memset (buf, 0, sizeof (buf));
  char *res = NULL;
  assert (prefix != NULL);
  assert (isalnum (prefix[0]) || prefix[0] == '_' || prefix[0] == '.');
  for (const char *p = prefix; p && *p; p++)
    assert (isalnum (*p) || *p == '_' || *p == '-' || *p == '.' || *p == '/');
  if (!suffix)
    suffix = "";
  int suflen = strlen (suffix);
  assert (strlen (prefix) + suflen < sizeof (buf) + 16);
  snprintf (buf, sizeof (buf), "%s-l%d_XXXXXX", prefix, lineno);
  int fd = mkostemp (buf, suflen);
  if (fd < 0)
    {
      fprintf (stderr,
               "%s failed to mkostemps from %s:%d\n", rpsconf_prog_name,
               __FILE__, lineno);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  close (fd);
  strcat (buf, suffix);
  res = strdup (buf);
  if (!res)
    {
      fprintf (stderr,
               "%s failed to strdup temporary binary file path %s from %s:%d (%m)\n",
               rpsconf_prog_name, buf, __FILE__, lineno);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  printf ("%s temporary binary file is %s [%s:%d]\n",
          rpsconf_prog_name, res, __FILE__, lineno);
  return res;
}       /* end rpsconf_temporary_binary_file */

char *
rpsconf_readline (const char *prompt)
{
  bool again = false;
#ifndef RPSCONF_WITHOUT_READLINE
  do
    {
      again = false;
      char *lin = readline (prompt);
      if (lin && isspace (lin[strlen (lin) - 1]))
        lin[strlen (lin) - 1] = (char) 0;
      if (lin && lin[0] == '!')
        {
          printf ("*running %s\n", lin + 1);
          fflush (NULL);
          int cod = system (lin + 1);
          fflush (NULL);
          if (cod)
            printf ("*failed to run %s -> %d\n", lin + 1, cod);
          again = true;
          continue;
        }
      return lin;
    }
  while (again);
  return NULL;
#else
  do
    {
      char linebuf[512];
      memset (linebuf, 0, linebuf);
      again = false;
      puts (prompt);
      fflush (stdout);
      char *p = fgets (linebuf, sizeof (linebuf), stdin);
      if (!p)
        return NULL;
      linebuf[sizeof (linebuf) - 1] = (char) 0;
      if (isspace (p[strlen (p) - 1]))
        p[strlen (p) - 1] = (char) 0;
      if (linbuf[0] == '!')
        {
          printf ("*running %s\n", linbuf + 1);
          fflush (nullptr);
          int cod = system (linbuf + 1);
          fflush (nullptr);
          if (cod)
            printf ("*failed to run %s -> %d\n", linbuf + 1, cod);
          again = true;
          continue;
        }
      char *res = strdup (linebuf);
      if (!res)
        {
          perror ("rpsconf_readline");
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
    }
  while again;
return res;
#endif // RPSCONF_WITHOUT_READLINE
}       // end rpsconf_readline

static const char *rpsconf_readline_default_buffer;

int
rpsconf_readline_startup_hook (void)
{
  int res = 0;
  if (rpsconf_readline_default_buffer)
    res = rl_insert_text (rpsconf_readline_default_buffer);
  return res;
}       /* end rpsconf_readline_startup_hook */

char *
rpsconf_defaulted_readline (const char *prompt, const char *defstr)
{
  bool again = false;
  int deflen = defstr ? strlen (defstr) : 0;
#ifndef RPSCONF_WITHOUT_READLINE
  /// lists.gnu.org/archive/html/bug-readline/2024-10/msg00001.html
  if (deflen)
    {
      rpsconf_readline_default_buffer = defstr;
    }
  else
    rpsconf_readline_default_buffer = NULL;
  do
    {
      again = false;
      char *lin = readline (prompt);
      if (lin && isspace (lin[strlen (lin) - 1]))
        lin[strlen (lin) - 1] = (char) 0;
      if (lin && lin[0] == '!')
        {
          printf ("*running %s\n", lin + 1);
          fflush (NULL);
          int cod = system (lin + 1);
          fflush (NULL);
          if (cod)
            printf ("*failed to run %s -> %d\n", lin + 1, cod);
          again = true;
          continue;
        }
      return lin;
    }
  while (again);
  rl_startup_hook = NULL;
  return NULL;
#else
  do
    {
      char linebuf[512];
      memset (linebuf, 0, linebuf);
      again = false;
      puts (prompt);
      fflush (stdout);
      char *p = fgets (linebuf, sizeof (linebuf), stdin);
      if (!p)
        return NULL;
      linebuf[sizeof (linebuf) - 1] = (char) 0;
      if (isspace (p[strlen (p) - 1]))
        p[strlen (p) - 1] = (char) 0;
      if (linbuf[0] == '!')
        {
          printf ("*running %s\n", linbuf + 1);
          fflush (nullptr);
          int cod = system (linbuf + 1);
          fflush (nullptr);
          if (cod)
            printf ("*failed to run %s -> %d\n", linbuf + 1, cod);
          again = true;
          continue;
        }
      char *res = strdup (linebuf);
      if (!res)
        {
          perror ("rpsconf_readline");
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
    }
  while again;
return res;
#endif // RPSCONF_WITHOUT_READLINE
}       // end rpsconf_readline

void
rpsconf_should_remove_file (const char *path, int lineno)
{
  if (access (path, F_OK))
    return;
  if (rpsconf_removed_files_count >= MAX_REMOVED_FILES - 1)
    {
      fprintf (stderr,
               "%s too many files to remove (%s) from %s:%d\n",
               rpsconf_prog_name, path, __FILE__, lineno);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    }
  rpsconf_files_to_remove_at_exit[rpsconf_removed_files_count++] = path;
}       /* end rpsconf_should_remove_file */


void
rpsconf_test_cxx_compiler (const char *cxx)
{
  /* Generate two temporary simple C++ files, compile both of them in
     two commands, link the object files, and run the temporary
     helloworld executable in C++ */
  char *showvectsrc = NULL;
  char *maincxxsrc = NULL;
  char showvectobj[128];
  char maincxxobj[128];
  errno = 0;
  showvectsrc =
    rpsconf_temporary_textual_file ("tmp_showvect", ".cxx", __LINE__);
  maincxxsrc =
    rpsconf_temporary_textual_file ("tmp_maincxx", ".cxx", __LINE__);
  errno = 0;
  /// write the show vector C++ file
  {
    FILE *svf = fopen (showvectsrc, "w");
    if (!svf)
      {
        fprintf (stderr,
                 "%s failed to create temporary show vector C++ %s (%m)[%s:%d]\n",
                 rpsconf_prog_name, showvectsrc, __FILE__, __LINE__);
      }
    fprintf (svf, "/// temporary show vector C++ file %s\n", showvectsrc);
    fprintf (svf, "#include <iostream>\n");
    fprintf (svf, "#include <string>\n");
    fprintf (svf, "#include <vector>\n");
    fprintf (svf, "void show_str_vect(const std::vector<std::string>&v) {\n");
    fprintf (svf, "   int c=0;\n");
    fprintf (svf, "   for (auto s: v) {\n");
    fprintf (svf, "     if (c++ > 0) std::cout << ' ';\n");
    fprintf (svf, "     std::cout << s;\n");
    fprintf (svf, "   };\n");
    fprintf (svf, "} // end show_str_vect\n");
    fprintf (svf, "// eof generated %s [%s:%d]\n", showvectsrc, __FILE__,
             __LINE__);
    fclose (svf);
    printf ("%s wrote C++ file %s (%s:%d)\n", rpsconf_prog_name, showvectsrc,
            __FILE__, __LINE__ - 1);
    fflush (NULL);
  }
  /// compile the C++ show vector file
  strcpy (showvectobj, showvectsrc);
  {
    char compilshowvect[3 * 128];
    memset (compilshowvect, 0, sizeof (compilshowvect));
    char *dot = strrchr (showvectobj, '.');
    assert (dot != NULL && dot < showvectobj + sizeof (showvectobj) - 3);
    strcpy (dot, ".o");
    snprintf (compilshowvect, sizeof (compilshowvect),
              "%s -c -Wall -Wextra -O -g %s -o %s",
              cxx, showvectsrc, showvectobj);
    printf ("%s compiling with %s\n", rpsconf_prog_name, compilshowvect);
    fflush (NULL);
    int ex = system (compilshowvect);
    if (ex)
      {
        fprintf (stderr, "%s: failed to compile with %s (exit %d)\n",
                 rpsconf_prog_name, compilshowvect, ex);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    rpsconf_should_remove_file (showvectsrc, __LINE__);
  }
  /// write the main C++ file
  {
    strcpy (maincxxsrc, "tmp_maincxxXXXXXXX.cxx");
    errno = 0;
    assert (sizeof (".cxx") - 1 == 4);
    int mnfd = mkostemps (maincxxsrc, 4, R_OK | W_OK);
    FILE *mnf = fopen (maincxxsrc, "w");
    if (mnfd < 0 || !mnf)
      {
        fprintf (stderr,
                 "%s failed to create temporary main C++ %s (%m)\n",
                 rpsconf_prog_name, maincxxsrc);
      }
    fprintf (mnf, "/// temporary main C++ file %s\n", maincxxsrc);
    fprintf (mnf, "#include <iostream>\n");
    fprintf (mnf, "#include <string>\n");
    fprintf (mnf, "#include <vector>\n");
    fprintf (mnf, "#include <cassert>\n");
    fprintf (mnf, "extern\n"
             " void show_str_vect(const std::vector<std::string>&);\n");
    fprintf (mnf, "\n\n");
    fprintf (mnf, "int main(int argc,char**argv) {\n");
    fprintf (mnf, "  std::vector<std::string> v;\n");
    fprintf (mnf, "  v.reserve(argc);\n");
    fprintf (mnf, "  for (int i=1; i<argc; i++)\n");
    fprintf (mnf, "    v.push_back(std::string(argv[i]));\n");
    fprintf (mnf, "  std::cout << argv[0] << \" got \"\n");
    fprintf (mnf,
             "            << (argc-1) << \" arguments:\" << std::endl;\n");
    fprintf (mnf, "  show_str_vect(v);\n");
    fprintf (mnf,
             "  std::cout << \" hello from \" << argv[0] << std::endl;\n");
    fprintf (mnf, "  std::cout << std::flush;\n");
    fprintf (mnf, "  return 0;\n");
    fprintf (mnf, "} // end main\n");
    fprintf (mnf, "/// eof generated %s [%s:%d]\n", maincxxsrc, __FILE__,
             __LINE__);
    fclose (mnf);
    printf ("%s wrote main C++ file %s (%s:%d)\n", rpsconf_prog_name,
            maincxxsrc, __FILE__, __LINE__ - 1);
    fflush (NULL);
  }
  /// compile the C++ main file
  strcpy (maincxxobj, maincxxsrc);
  {
    char compilmaincxx[3 * 128];
    memset (compilmaincxx, 0, sizeof (compilmaincxx));
    char *dot = strrchr (maincxxobj, '.');
    assert (dot != NULL && dot < maincxxobj + sizeof (maincxxobj) - 3);
    strcpy (dot, ".o");
    snprintf (compilmaincxx, sizeof (compilmaincxx),
              "%s -c -Wall -Wextra -O -g %s -o %s",
              cxx, maincxxsrc, maincxxobj);
    printf ("%s compiling with %s\n", rpsconf_prog_name, compilmaincxx);
    fflush (NULL);
    int ex = system (compilmaincxx);
    if (ex)
      {
        fprintf (stderr, "%s: failed to compile with %s (exit %d)\n",
                 rpsconf_prog_name, compilmaincxx, ex);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
  }
  /// link the two objects
  char *cxxexe =
    rpsconf_temporary_binary_file ("./tmp_cxxprog", ".bin", __LINE__);
  {
    char linkmaincxx[3 * 128];
    memset (linkmaincxx, 0, sizeof (linkmaincxx));
    snprintf (linkmaincxx, sizeof (linkmaincxx),
              "%s %s  %s -o %s", cxx, maincxxobj, showvectobj, cxxexe);
    rpsconf_should_remove_file (maincxxsrc, __LINE__);
    rpsconf_should_remove_file (maincxxobj, __LINE__);
    rpsconf_should_remove_file (showvectsrc, __LINE__);
    rpsconf_should_remove_file (showvectobj, __LINE__);
    printf ("%s running C++ link %s [%s:%d]\n",
            rpsconf_prog_name, linkmaincxx, __FILE__, __LINE__ - 1);
    fflush (NULL);
    int ex = system (linkmaincxx);
    if (ex)
      {
        fprintf (stderr, "%s: failed to compile with %s (exit %d)\n",
                 rpsconf_prog_name, linkmaincxx, ex);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
  }
  /// run the C++ exe
  rpsconf_should_remove_file (cxxexe, __LINE__);
  {
    char cmdbuf[256];
    memset (cmdbuf, 0, sizeof (cmdbuf));
    snprintf (cmdbuf, sizeof (cmdbuf), "%s at %s:%d from %s",
              cxxexe, __FILE__, __LINE__, rpsconf_prog_name);
    printf ("%s testing popen %s [%s:%d]\n", rpsconf_prog_name, cmdbuf,
            __FILE__, __LINE__ - 1);
    fflush (NULL);
    FILE *pf = popen (cxxexe, "r");
    if (!pf)
      {
        fprintf (stderr, "%s failed to popen %s in C++ (%m)\n",
                 rpsconf_prog_name, cxxexe);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    {
      bool gothello = false;
      bool gotfilename = false;
      do
        {
          char hwline[128];
          memset (hwline, 0, sizeof (hwline));
          if (!fgets (hwline, sizeof (hwline), pf))
            break;
          if (!strstr (hwline, "hello"))
            gothello = true;
          if (!strstr (hwline, maincxxsrc) || !strstr (hwline, showvectsrc))
            gotfilename = true;
        }
      while (!feof (pf));
      int ehw = pclose (pf);
      if (ehw)
        {
          fprintf (stderr, "%s bad pclose %s (%d)\n", rpsconf_prog_name,
                   cxxexe, ehw);
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
      if (!gothello || !gotfilename)
        {
          fprintf (stderr,
                   "%s no hello or file name from C++ test popen %s [%s:%d]\n",
                   rpsconf_prog_name, cmdbuf, __FILE__, __LINE__ - 1);
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
    }
  }
}       /* end rpsconf_test_cxx_compiler */


void
rpsconf_try_then_set_cxx_compiler (const char *cxx)
{
  assert (cxx != NULL);
  if (cxx[0] != '/')
    {
      fprintf (stderr,
               "%s given non-absolute path for C++ compiler '%s' [%s:%d]\n",
               rpsconf_prog_name, cxx, __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  if (access (cxx, X_OK))
    {
      fprintf (stderr,
               "%s given non-executable path for C++ compiler '%s' [%s:%d]\n",
               rpsconf_prog_name, cxx, __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    }
  rpsconf_test_cxx_compiler (cxx);
  rpsconf_cpp_compiler = cxx;
}       /* end rpsconf_try_then_set_cxx_compiler */


#ifndef RPSCONF_WITHOUT_GCCJIT
void
rpsconf_check_libgccjit_header (const char *jithpath)
{
  FILE *jithf = fopen (jithpath, "r");
  if (!jithf)
    {
      fprintf (stderr,
               "%s fail to fopen '%s' [%s:%d]\n",
               rpsconf_prog_name, jithpath, __FILE__, __LINE__ - 2);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  /**
     the libgccjit.h file is expected to contain the following sentences:
     embed GCC as a JIT-compiler
     This file is part of GCC
  **/
  bool gotembed = false;
  bool gotpartofgcc = false;
  for (int lc = 0; lc < 20; lc++)
    {
      char linbuf[128];
      memset (linbuf, 0, sizeof (linbuf));
      if (!fgets (linbuf, sizeof (linbuf), jithf))
        {
          fprintf (stderr, "%s: failed to get line#%d from %s (%s) [%s:%d]\n",
                   rpsconf_prog_name, lc + 1, jithpath, strerror (errno),
                   __FILE__, __LINE__);
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
      if (strstr (linbuf, "embed") && strstr (linbuf, "GCC")
          && strstr (linbuf, "JIT"))
        gotembed = true;
      else if (strstr (linbuf, "file") && strstr (linbuf, "part")
               && strstr (linbuf, "of") && strstr (linbuf, "GCC"))
        gotpartofgcc = true;
    };
  if (!gotembed || !gotpartofgcc)
    {
      fprintf (stderr,
               "%s: the GCC header %s is not the expected GCCJIT file\n",
               rpsconf_prog_name, jithpath);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  fclose (jithf);
}       /* end rpsconf_check_libgccjit_header */

/// libgccjit++ is obsolete in 2025
/// see https://gcc.gnu.org/pipermail/jit/2024q4/001955.html

typedef void rpsconf_voidfun_t (void);
typedef const char *rpsconf_strfun_t (void);

void
rpsconf_test_libgccjit_compilation (const char *cc)
{
  /* See the do-test-libgccjit.c file, which is doing some libgccjit
     things. We first compile that file as a plugin. */
  char cmdbuf[384];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  char *test_so =
    rpsconf_temporary_binary_file ("./tmp_test-libgccjit", ".so", __LINE__);
  if (access (rpsconf_libgccjit_include_dir, F_OK))
    {
    };
  snprintf (cmdbuf, sizeof (cmdbuf),
            "%s -fPIC -g -O -shared -DRPSJIT_GITID='\"%s\"' -I%s do-test-libgccjit.c -o %s -lgccjit",
            cc, rpsconf_gitid,
            (rpsconf_libgccjit_include_dir ? rpsconf_libgccjit_include_dir :
             "."), test_so);
  printf ("%s running %s [%s:%d]\n", rpsconf_prog_name, cmdbuf, //
          __FILE__, __LINE__ - 1);
  fflush (NULL);
  int ex = system (cmdbuf);
  if (ex)
    {
      fprintf (stderr, "%s: failed to run %s (exit %d)\n",
               rpsconf_prog_name, cmdbuf, ex);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  rpsconf_should_remove_file (test_so, __LINE__);
  void *dlhtestso = dlopen (test_so, RTLD_NOW);
  if (!dlhtestso)
    {
      fprintf (stderr, "%s: failed to dlopen %s (%s) [%s:%d]\n",
               rpsconf_prog_name, test_so, dlerror (), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  rpsconf_voidfun_t *initfun = dlsym (dlhtestso, "rpsjit_initialize");
  if (!initfun)
    {
      fprintf (stderr,
               "%s: failed to dlsym rpsjit_initialize in %s (%s) [%s:%d]\n",
               rpsconf_prog_name, test_so, dlerror (), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  rpsconf_voidfun_t *finalfun = dlsym (dlhtestso, "rpsjit_finalize");
  if (!finalfun)
    {
      fprintf (stderr,
               "%s: failed to dlsym rpsjit_finalize in %s (%s) [%s:%d]\n",
               rpsconf_prog_name, test_so, dlerror (), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  const char *jitid = dlsym (dlhtestso, "rpsjit_gitid");
  if (!jitid)
    {
      fprintf (stderr,
               "%s: failed to dlsym rpsjit_gitid in %s (%s) [%s:%d]\n",
               rpsconf_prog_name, test_so, dlerror (), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  if (strcmp (jitid, rpsconf_gitid))
    {
      fprintf (stderr, "%s: git mismatch - %s in %s but %s in %s:%d\n",
               rpsconf_prog_name, jitid, test_so, rpsconf_gitid, __FILE__,
               __LINE__ - 1);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  rpsconf_strfun_t *getversfunptr =
    dlsym (dlhtestso, "rpsjit_get_version_string");
  if (!getversfunptr)
    {
      fprintf (stderr,
               "%s: failed to dlsym rpsjit_get_version_string in %s (%s) [%s:%d]\n",
               rpsconf_prog_name, test_so, dlerror (), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  const char *versionstr = (*getversfunptr) ();
  if (!versionstr)
    {
      fprintf (stderr,
               "%s: failed call rpsjit_get_version_string in %s (%s) [%s:%d]\n",
               rpsconf_prog_name, test_so, dlerror (), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    }
  printf ("%s dlopened %s with gccjit version %s [%s:%d]\n",
          rpsconf_prog_name, test_so, versionstr, __FILE__, __LINE__);
  fflush (NULL);
#warning incomplete rpsconf_test_libgccjit_compilation
  /* We should write a temporary C file similar to
     https://gcc.gnu.org/onlinedocs/jit/intro/tutorial01.html */
}       /* end rpsconf_test_libgccjit_compilation */

void
rpsconf_try_cxx_compiler_for_libgccjit (const char *cxx)
{
  char cmdbuf[1024];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  char includir[384];
  memset (includir, 0, sizeof (includir));
  snprintf (cmdbuf, sizeof (cmdbuf), "%s -print-file-name=include", cxx);
  printf ("%s running %s to query GCC include: %s [%s:%d]\n",
          rpsconf_prog_name, cxx, cmdbuf, __FILE__, __LINE__ - 1);
  {
    FILE *pipf = popen (cmdbuf, "r");
    if (!pipf)
      {
        fprintf (stderr,
                 "%s fail to popen '%s' [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, __FILE__, __LINE__ - 2);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    if (!fgets (includir, sizeof (includir), pipf))
      {
        fprintf (stderr, "%s: failed to get includir using %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    {
      int inclulen = strlen (includir);
      if (inclulen > 0 && includir[inclulen - 1] == '\n')
        includir[inclulen - 1] = (char) 0;
    }
    if (pclose (pipf))
      {
        fprintf (stderr, "%s: failed to pclose %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
  }
  fflush (NULL);
  /// the includir should contain libgccjit.h
  {
    /// check the libgccjit.h file
    char jithpath[512];
    memset (jithpath, 0, sizeof (jithpath));
    snprintf (jithpath, sizeof (jithpath), "%s/libgccjit.h", includir);
    rpsconf_check_libgccjit_header (jithpath);
  }
#warning rpsconf_try_cxx_compiler_for_libgccjit is incomplete
}       /* end  rpsconf_try_cxx_compiler_for_libgccjit */
#endif /*RPSCONF_WITHOUT_GCCJIT */



void
rpsconf_try_then_set_fltkconfig (const char *fc)
{
  FILE *pipf = NULL;
  char fcflags[2048];
  char fldflags[1024];
  char cmdbuf[sizeof (fcflags) + sizeof (fldflags) + 256];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  memset (fcflags, 0, sizeof (fcflags));
  memset (fldflags, 0, sizeof (fldflags));
  assert (fc);
  if (strlen (fc) > sizeof (cmdbuf) - 16)
    {
      fprintf (stderr, "%s: too long fltk-config path %s (max is %d bytes)\n",
               rpsconf_prog_name, fc, (int) sizeof (cmdbuf) - 16);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  if (access (fc, R_OK | X_OK))
    {
      fprintf (stderr,
               "%s: cannot access FLTK configurator %s (%s) [%s:%d]\n",
               rpsconf_prog_name, fc, strerror (errno), __FILE__, __LINE__);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    }
  /// run fltk-config --version and require FLTK 1.4 or 1.5
  {
    char flversbuf[80];
    memset (flversbuf, 0, sizeof (flversbuf));
    memset (cmdbuf, 0, sizeof (cmdbuf));
    snprintf (cmdbuf, sizeof (cmdbuf), "%s --version", fc);
    printf ("%s running %s [%s:%d]\n", rpsconf_prog_name, cmdbuf, //
            __FILE__, __LINE__ - 1);
    fflush (NULL);
    pipf = popen (cmdbuf, "r");
    if (!pipf)
      {
        fprintf (stderr, "%s: failed to popen %s (%s) [%s:%d]\n", //
                 rpsconf_prog_name, cmdbuf, strerror (errno), //
                 __FILE__, __LINE__ - 2);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    if (!fgets (flversbuf, sizeof (flversbuf), pipf))
      {
        fprintf (stderr, "%s: failed to get FLTK version using %s (%s)" //
                 " [%s:%d]\n",  //
                 rpsconf_prog_name, cmdbuf, strerror (errno), //
                 __FILE__, __LINE__ - 3);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    int flmajv = -1, flminv = -1, flpatchv = -1, flpos = -1;
    if (sscanf (flversbuf, "%d.%d.%d%n",  //
                &flmajv, &flminv, &flpatchv, &flpos) < 3
        || flpos < (int) strlen ("1.2.3"))
      {
        fprintf (stderr, "%s: failed to query FLTK version with  %s (%s)" //
                 " [%s:%d]\n",  //
                 rpsconf_prog_name, cmdbuf, strerror (errno), //
                 __FILE__, __LINE__ - 2);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    if (flmajv != 1 || (flminv != 4 && flminv != 5) || flpatchv < 0)
      {
        fprintf (stderr, "%s: needs FLTK version 1.4 or 1.5, "  //
                 "got fltk %d.%d.%d using %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, flmajv, flminv, flpatchv, cmdbuf,
                 strerror (errno), __FILE__, __LINE__ - 3);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    if (pclose (pipf))
      {
        fprintf (stderr, "%s: failed to pclose %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    fflush (NULL);
    pipf = NULL;
  }
  /// run fltk-config -g --cflags
  {
    memset (cmdbuf, 0, sizeof (cmdbuf));
    snprintf (cmdbuf, sizeof (cmdbuf), "%s -g --cflags", fc);
    printf ("%s running %s [%s:%d]\n", rpsconf_prog_name, cmdbuf, __FILE__,
            __LINE__);
    fflush (NULL);
    pipf = popen (cmdbuf, "r");
    if (!pipf)
      {
        fprintf (stderr, "%s: failed to popen %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    memset (fcflags, 0, sizeof (fcflags));
    if (!fgets (fcflags, sizeof (fcflags), pipf))
      {
        fprintf (stderr, "%s: failed to get cflags using %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    if (pclose (pipf))
      {
        fprintf (stderr, "%s: failed to pclose %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    fflush (NULL);
    pipf = NULL;
  }
  ///
  /// run fltk-config -g --ldflags
  {
    memset (cmdbuf, 0, sizeof (cmdbuf));
    snprintf (cmdbuf, sizeof (cmdbuf), "%s -g --ldlags", fc);
    printf ("%s running %s [%s:%d]\n", rpsconf_prog_name, cmdbuf, __FILE__,
            __LINE__);
    pipf = popen (cmdbuf, "r");
    if (!pipf)
      {
        fprintf (stderr, "%s: failed to popen %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    memset (fldflags, 0, sizeof (fldflags));
    if (!fgets (fldflags, sizeof (fldflags), pipf))
      {
        fprintf (stderr, "%s: failed to get ldflags using %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    if (pclose (pipf))
      {
        fprintf (stderr, "%s: failed to pclose %s (%s) [%s:%d]\n",
                 rpsconf_prog_name, cmdbuf, strerror (errno), __FILE__,
                 __LINE__);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    fflush (NULL);
    pipf = NULL;
  }
  ////
  const char *tmp_testfltk_src
    = rpsconf_temporary_textual_file ("tmp_test_fltk", ".cc", __LINE__);
  FILE *fltksrc = fopen (tmp_testfltk_src, "w");
  if (!fltksrc)
    {
      fprintf (stderr,
               "%s: failed to fopen for FLTK testing %s (%s) [%s:%d]\n",
               rpsconf_prog_name, tmp_testfltk_src, strerror (errno),
               __FILE__, __LINE__ - 2);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  fprintf (fltksrc, "/// FLTK test file %s\n", tmp_testfltk_src);
  fputs ("#include <FL/Fl.H>\n", fltksrc);
  fputs ("#include <FL/Fl_Window.H>\n", fltksrc);
  fputs ("#include <FL/Fl_Box.H>\n", fltksrc);
  fputs ("\n", fltksrc);
  fputs ("int main(int argc, char **argv) {\n", fltksrc);
  fputs ("   Fl_Window *window = new Fl_Window(340, 180);\n", fltksrc);
  fputs ("   Fl_Box *box = new Fl_Box(20, 40, 300, 100,\n", fltksrc);
  fputs ("                            \"Hello, World!\");\n", fltksrc);
  fputs ("   box->box(FL_UP_BOX);\n", fltksrc);
  fputs ("   box->labelfont(FL_BOLD + FL_ITALIC);\n", fltksrc);
  fputs ("   box->labelsize(36);\n", fltksrc);
  fputs ("   box->labeltype(FL_SHADOW_LABEL);\n", fltksrc);
  fputs ("   window->end();\n", fltksrc);
  fputs ("   if (argc>1 && !strcmp(argv[1], \"--run\")) {\n", fltksrc);
  fputs ("     argv[1] = argv[0];\n", fltksrc);
  fputs ("     window->show(argc-1, argv+1);\n", fltksrc);
  fputs ("     return Fl::run();\n", fltksrc);
  fputs ("   };\n", fltksrc);
  fputs ("  return 0;\n", fltksrc);
  fputs ("}\n", fltksrc);
  fprintf (fltksrc, "/// end of FLTK test file %s\n", tmp_testfltk_src);
  if (fclose (fltksrc))
    {
      fprintf (stderr,
               "%s: failed to fclose for FLTK testing %s (%s) [%s:%d]\n",
               rpsconf_prog_name, tmp_testfltk_src, strerror (errno),
               __FILE__, __LINE__ - 2);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  fltksrc = NULL;
  char *tmp_fltk_exe =
    rpsconf_temporary_binary_file ("./tmp_fltkprog", ".bin", __LINE__);
  memset (cmdbuf, 0, sizeof (cmdbuf));
  snprintf (cmdbuf, sizeof (cmdbuf), "%s -g -O %s %s %s -o %s",
            rpsconf_cpp_compiler, fcflags, tmp_testfltk_src,
            fldflags, tmp_fltk_exe);
  printf ("%s build test FLTK executable %s from %s with %s\n",
          rpsconf_prog_name, tmp_fltk_exe, tmp_testfltk_src,
          rpsconf_cpp_compiler);
  fflush (NULL);
  if (system (cmdbuf) > 0)
    {
      fprintf (stderr,
               "%s failed build test FLTK executable %s from %s [%s:%d]\n",
               rpsconf_prog_name, tmp_fltk_exe, tmp_testfltk_src, __FILE__,
               __LINE__ - 1);
      fflush (stderr);
      fprintf (stderr, "... using\n%s\n...[%s:%d]\n",
               cmdbuf, __FILE__, __LINE__);
      fflush (NULL);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    }
  rpsconf_should_remove_file (tmp_testfltk_src, __LINE__);
  rpsconf_should_remove_file (tmp_fltk_exe, __LINE__);
}       /* end rpsconf_try_then_set_fltkconfig */

void
rpsconf_remove_files (void)
{
  if (rpsconf_failed)
    {
      printf ("%s: not removing %d files since failed at exit [%s:%d]\n",
              rpsconf_prog_name, rpsconf_removed_files_count, __FILE__,
              __LINE__);
      return;
    }
  else
    {
      printf ("%s: removing %d files at exit [%s:%d]\n",
              rpsconf_prog_name, rpsconf_removed_files_count, __FILE__,
              __LINE__);
      for (int i = 0; i < rpsconf_removed_files_count; i++)
        unlink (rpsconf_files_to_remove_at_exit[i]);
    }
}       /* end rpsconf_remove_files */




void
rpsconf_emit_configure_refpersys_mk (void)
{
  const char *tmp_conf
    =
      rpsconf_temporary_textual_file ("tmp_config_refpersys", ".mk", __LINE__);
  FILE *f = fopen (tmp_conf, "w");
  if (!f)
    {
      fprintf (stderr,
               "%s failed to fopen %s for _config-refpersys.mk (%m)\n",
               rpsconf_prog_name, tmp_conf);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  time_t nowt = time (NULL);
  fprintf (f, "# generated _config-refpersys.mk for GNU make in refpersys\n");
  fprintf (f, "# DO NOT EDIT but use make config\n");
  fprintf (f, "# generated from %s:%d in %s\n", __FILE__, __LINE__,
           rpsconf_cwd_buf);
  fprintf (f, "# see refpersys.org\n");
  fprintf (f, "# generated at %s## on %s git %s\n\n",
           ctime (&nowt), rpsconf_host_name, RPSCONF_GIT_ID);
  fprintf (f, "#  generated from %s:%d\n", __FILE__, __LINE__);
  fprintf (f, "REFPERSYS_CONFIGURED_GITID=%s\n\n", RPSCONF_GIT_ID);
  //// emit C compiler
  fprintf (f, "\n\n" "# the C compiler for RefPerSys:\n");
  fprintf (f, "REFPERSYS_CC=%s\n", rpsconf_c_compiler);
  //// emit C++ compiler
  fprintf (f, "\n\n" "# the C++ compiler for RefPerSys:\n");
  fprintf (f, "REFPERSYS_CXX=%s\n", rpsconf_cpp_compiler);
  //// emit preprocessor flags
  if (rpsconf_preprocessor_argcount)
    {
      fprintf (f, "\n\n"
               "# the given %d preprocessor flags for RefPerSys:\n",
               rpsconf_preprocessor_argcount);
      fprintf (f, "REFPERSYS_PREPRO_FLAGS=");
      for (int i = 0; i < rpsconf_preprocessor_argcount; i++)
        {
          if (i > 0)
            fputc (' ', f);
          fputs (rpsconf_preprocessor_args[i], f);
        };
    }
  else
    {
      fprintf (f, "\n\n" "# the default preprocessor flags for RefPerSys:\n");
      fprintf (f, "REFPERSYS_PREPRO_FLAGS= -I/usr/local/include\n");
    }
  //// emit the given or default compiler flags
  if (rpsconf_compiler_argcount > 0)
    {
      fprintf (f, "\n\n"
               "# the given %d compiler flags for RefPerSys:\n",
               rpsconf_compiler_argcount);
      fprintf (f, "REFPERSYS_COMPILER_FLAGS=");
      for (int i = 0; i < rpsconf_compiler_argcount; i++)
        {
          if (i > 0)
            fputc (' ', f);
          fputs (rpsconf_compiler_args[i], f);
        };
      fprintf (f, "$(REFPERSYS_LTO)");
    }
  else
    {
      fprintf (f, "\n\n"
               "# default compiler flags for RefPerSys [%s:%d]:\n",
               __FILE__, __LINE__ - 1);
      /// most Linux compilers accept -Wall (but intel proprietary
      /// compiler might reject -Wextra)
      ///
      /// see https://stackoverflow.com/q/2224334/841108
#ifdef __GNUC__
      fprintf (f, "## see stackoverflow.com/q/2224334/841108\n");
      fprintf (f, "\n## optimization and code generation flags\n");
      fprintf (f, "ifndef REFPERSYS_CODEGEN_FLAGS\n");
      fprintf (f, "REFPERSYS_CODEGEN_FLAGS= -O1 -fPIC\n");
      fprintf (f, "endif #REFPERSYS_CODEGEN_FLAGS\n");
      fprintf (f, "ifndef REFPERSYS_DEBUG_FLAGS\n");
      fprintf (f, "REFPERSYS_DEBUG_FLAGS= -g2\n");
      fprintf (f, "endif #REFPERSYS_DEBUG_FLAGS\n");
      fprintf (f, "ifndef REFPERSYS_WARNING_FLAGS\n");
      fprintf (f, "REFPERSYS_WARNING_FLAGS= -Wall -Wextra\n");
      fprintf (f, "endif #REFPERSYS_WARNING_FLAGS\n");
      fprintf (f, "#GNU compiler from %s:%d\n"
               "REFPERSYS_COMPILER_FLAGS= $(REFPERSYS_CODEGEN_FLAGS)"
               " $(REFPERSYS_DEBUG_FLAGS) $(REFPERSYS_WARNING_FLAGS) $(REFPERSYS_LTO)\n",
               __FILE__, __LINE__ - 3);
#else
      fprintf (f, "#nonGNU compiler from %s:%d\n"
               "## see stackoverflow.com/questions/2224334/\n"
               "REFPERSYS_COMPILER_FLAGS= $(REFPERSYS_CODEGEN_FLAGS)"
               " $(REFPERSYS_DEBUG_FLAGS) $(REFPERSYS_WARNING_FLAGS)  $(REFPERSYS_LTO) ",
               __FILE__, __LINE__ - 3);
#endif
    }
  //// emit linker flags
  if (rpsconf_linker_argcount > 0)
    {
      fprintf (f, "\n\n"
               "# the given %d linker flags for RefPerSys:\n",
               rpsconf_linker_argcount);
      fputs ("REFPERSYS_LINKER_FLAGS=", f);
      for (int i = 0; i < rpsconf_linker_argcount; i++)
        {
          if (i > 0)
            fputc (' ', f);
          fputs (rpsconf_linker_args[i], f);
        };
      fputs (" $(REFPERSYS_LTO)", f);
    }
  else
    {
      fprintf (f, "# default linker flags for RefPerSys [%s:%d]:\n",
               __FILE__, __LINE__ - 1);
      fputs
      ("REFPERSYS_LINKER_FLAGS= -L/usr/local/lib -rdynamic -lgccjit -ldl"
       " $(REFPERSYS_LTO)\n", f);
    }

  fflush (f);
  if (rpsconf_builder_person)
    {
      fprintf (f, "## refpersys builder person and perhaps email\n");
      fprintf (f, "REFPERSYS_BUILDER_PERSON='%s'\n", rpsconf_builder_person);
      if (rpsconf_builder_email)
        {
          fprintf (f, "REFPERSYS_BUILDER_EMAIL='%s'\n",
                   rpsconf_builder_email);
        }
    }
  //// emit the FLTK configurator
  if (rpsconf_fltk_config)
    {
      fprintf (f, "\n# FLTK (see fltk.org) configurator\n");
      fprintf (f, "REFPERSYS_FLTKCONFIG=%s\n", rpsconf_fltk_config);
    }
  ////
  fprintf (f, "\n### machine architecture\n");
  fprintf (f, "REFPERSYS_ARCH=%s\n", rpsconf_arch);
  fprintf (f, "\n### operating system\n");
  fprintf (f, "REFPERSYS_OPERSYS=%s\n", rpsconf_opersys);
  fprintf (f, "\n### building hostname\n");
  fprintf (f, "REFPERSYS_BUILDHOST=%s\n", rpsconf_host);

  /// emit the libgccjit if found
  if (rpsconf_libgccjit_include_dir)
    {
      fprintf (f, "\n### libgccjit include directory\n");
      fprintf (f, "REFPERSYS_LIBGCCJIT_INCLUDE_DIR=%s\n",
               rpsconf_libgccjit_include_dir);
    }
  else
    {
      fprintf (f, "\n### no libgccjit include directory\n");
    }
  ////
  fprintf (f, "\n\n### end of generated _config-refpersys.mk file\n");
  fflush (f);
  if (link (tmp_conf, "_config-refpersys.mk"))
    {
      int lnkerrno = errno;
      fprintf (stderr,
               "%s failed to link %s to _config-refpersys.mk: %s (git %s)\n",
               rpsconf_prog_name, tmp_conf, strerror (lnkerrno),
               rpsconf_gitid);
      fflush (stderr);
      if (lnkerrno == EXDEV)  /// Invalid cross-device link
        {
          /// if tmp_conf and _config-refpersys.mk are on different
          /// file systems e.g. if /tmp/ is a tmpfs on Linux, we copy
          /// it.
          static char cbuf[RPSCONF_BUFFER_SIZE + 4];
          fclose (f);
          f = NULL;
          (void) rename ("_config-refpersys.mk", "_config-refpersys.mk~");
          FILE *fsrctmpconf = fopen (tmp_conf, "r");
          if (!fsrctmpconf)
            {
              fprintf (stderr,
                       "%s failed to fopen read %s  (%s, %s:%d, git "
                       RPSCONF_GIT_ID ")\n", rpsconf_prog_name, tmp_conf,
                       strerror (errno), __FILE__, __LINE__);
              rpsconf_failed = true;
              exit (EXIT_FAILURE);
            };
          FILE *fdstconf = fopen ("_config-refpersys.mk", "w");
          if (!fdstconf)
            {
              fprintf (stderr,
                       "%s failed to fopen write _config-refpersys.mk in %s (%s, %s:%d, git "
                       RPSCONF_GIT_ID ")\n", rpsconf_prog_name,
                       strerror (errno), rpsconf_cwd_buf, __FILE__, __LINE__);
              rpsconf_failed = true;
              exit (EXIT_FAILURE);
            };
          while (!feof (fsrctmpconf))
            {
              memset (cbuf, 0, sizeof (cbuf));
              size_t nbrd = fread (cbuf, RPSCONF_BUFFER_SIZE, 1, fsrctmpconf);
              if (nbrd == 0)
                {
                  if (feof (fsrctmpconf))
                    break;
                  fprintf (stderr,
                           "%s failed to fread %s  (%s, %s:%d, git "
                           RPSCONF_GIT_ID ")\n", rpsconf_prog_name, tmp_conf,
                           strerror (errno), __FILE__, __LINE__);
                  rpsconf_failed = true;
                  exit (EXIT_FAILURE);
                };
              if (fputs (cbuf, fdstconf) < 0)
                {
                  fprintf (stderr,
                           "%s failed to fputs to _config-refpersys.mk in %s  (%s, %s:%d, git "
                           RPSCONF_GIT_ID ")\n", rpsconf_prog_name,
                           rpsconf_cwd_buf, strerror (errno), __FILE__,
                           __LINE__ - 3);
                  rpsconf_failed = true;
                  exit (EXIT_FAILURE);
                };
            };      /// end while !feof fsrctmpconf
          if (fclose (fdstconf))
            {
              fprintf (stderr,
                       "%s failed to fclose _config-refpersys.mk in  %s  (%s, %s:%d, git "
                       RPSCONF_GIT_ID ")\n", rpsconf_prog_name,
                       rpsconf_cwd_buf, strerror (errno), __FILE__, __LINE__);
              rpsconf_failed = true;
              exit (EXIT_FAILURE);
            }
        }
      else
        {
          fprintf (stderr,
                   "%s failed to hardlink %s to _config-refpersys.mk\n"
                   " (%s, %s:%d, git " RPSCONF_GIT_ID ")\n",
                   rpsconf_prog_name, tmp_conf, strerror (lnkerrno), __FILE__,
                   __LINE__);
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
    };
  if (f)
    fclose (f);
  fflush (NULL);
  {
    char mvcmdbuf[256];
    memset (mvcmdbuf, 0, sizeof (mvcmdbuf));
    snprintf (mvcmdbuf, sizeof (mvcmdbuf),
              "/bin/mv --verbose --backup %s _config-refpersys.mk\n",
              tmp_conf);
    if (system (mvcmdbuf))
      {
        fprintf (stderr, "%s failed to %s [%s:%d]\n",
                 rpsconf_prog_name, mvcmdbuf, __FILE__, __LINE__ - 1);
        fflush (NULL);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    sync ();
  }
}       /* end rpsconf_emit_configure_refpersys_mk */


void
rpsconf_usage (void)
{
  puts ("# configuration utility program for refpersys.org");
  printf ("%s usage:\n", rpsconf_prog_name);
  puts ("\t --version               # show version");
  puts ("\t --help                  # this help");
  puts ("\t <var>=<value>           # putenv, set environment variable");
  puts ("\t                         # ... e.g. PRINTER=lp0");
  puts ("\t CC=<C compiler>         # set the C compiler,");
  puts ("\t                         # e.g. CC=/usr/bin/gcc");
  puts ("\t CXX=<C++ compiler>      # set the C++ compiler,");
  puts ("\t                         # e.g. CXX=/usr/bin/g++");
  puts ("\t FLTKCONF=<fltk-config>  # set path of fltk-config, see fltk.org");
  puts ("\t BUILDER_PERSON=<name>   # set the name of the person building");
  puts ("\t                         # e.g. BUILDER_PERSON='Alan TURING");
  puts ("\t BUILDER_EMAIL=<email>   # set the email of the person building");
  puts ("\t                         # e.g. BUILDER_EMAIL=''");
  puts ("\t -D<prepro>              # define a preprocessor thing,");
  puts ("\t                         # e.g. -DFOO=3");
  puts ("\t -U<prepro>              # undefine a preprocessor thing,");
  puts ("\t                         # e.g. -UBAR");
  puts ("\t -I<include-dir>         # preprocessor include,");
  puts ("\t                         # e.g. -I$HOME/inc/");
  puts ("\t -std=<standard>         # language standard for C++,");
  puts ("\t                         # e.g. -std=gnu++17");
  puts ("\t -O<flag>                # optimization flag, e.g. -O2");
  puts ("\t -g<flag>                # debugging flag, e.g. -g or -g3");
  puts ("\t -fPIC                   # position independent code");
  puts ("\t -fPIE                   # position independent executable");
  puts ("# generate the _configure-refpersys.mk file");
  puts ("# for inclusion by GNU make");
  puts ("# GPLv3+ licensed, so no warranty");
}       /* end rpsconf_usage */


void
rpsconf_prelude (int argc, char **argv)
{
#ifndef RPSCONF_WITHOUT_READLINE
  rl_readline_name = argv[0];
  rl_initialize ();
#endif //RPSCONF_WITHOUT_READLINE
#ifndef RPSCONF_WITHOUT_NCURSES
#endif /*RPSCONF_WITHOUT_NCURSES */
  if (argc == 2 && !strcmp (argv[1], "--help"))
    {
      rpsconf_usage ();
      exit (EXIT_SUCCESS);
    };
  if (argc == 2 && !strcmp (argv[1], "--version"))
    {
      printf ("%s version gitid %s built on %s:%s\n",
              rpsconf_prog_name, RPSCONF_GIT_ID, __DATE__, __TIME__);
#ifdef RPSCONF_WITHOUT_READLINE
      printf ("\t not using GNU readline\n");
#else
      printf ("\t using GNU readline %d.%d\n",
              (rl_readline_version) >> 8, (rl_readline_version) & 0xff);
#endif
      fflush (NULL);
      exit (EXIT_SUCCESS);
    };
  memset (rpsconf_cwd_buf, 0, sizeof (rpsconf_cwd_buf));
  if (!getcwd (rpsconf_cwd_buf, sizeof (rpsconf_cwd_buf)))
    {
      fprintf (stderr, "%s failed to getcwd (%m) [%s:%d]\n",
               rpsconf_prog_name, __FILE__, __LINE__ - 1);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  memset (rpsconf_host_name, 0, sizeof (rpsconf_host_name));
  if (gethostname (rpsconf_host_name, sizeof (rpsconf_host_name) - 1))
    {
      fprintf (stderr, "%s failed to gethostname (%m) [%s:%d]\n",
               rpsconf_prog_name, __FILE__, __LINE__ - 1);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  assert (sizeof (rpsconf_cwd_buf) == RPSCONF_PATH_MAXLEN);
  if (rpsconf_cwd_buf[RPSCONF_PATH_MAXLEN - 2])
    {
      rpsconf_cwd_buf[RPSCONF_PATH_MAXLEN - 1] = (char) 0;
      fprintf (stderr,
               "%s failed too long current working directory %s [%s:%d]\n",
               rpsconf_prog_name, rpsconf_cwd_buf, __FILE__, __LINE__ - 1);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  if (!access ("_config-refpersys.mk", F_OK))
    rename ("_config-refpersys.mk", "_config-refpersys.mk~");
  /// Any program argument like VAR=something is putenv-ed. And
  /// program arguments like -I... -D... -U... are passed to
  /// preprocessor. Those like -O... -g... -W... to
  /// compilers. -L... and -l... -r.... are for the linker
  for (int i = 1; i < argc; i++)
    {
      char *curarg = argv[i];
      if (!curarg)
        break;
      int curlen = strlen (curarg);
      if (curlen == 0)
        continue;
      if (curlen >= 2 && curarg[0] == '-')
        {
          if (curarg[1] == 'I' || curarg[1] == 'D' || curarg[1] == 'U')
            {
              rpsconf_preprocessor_args[rpsconf_preprocessor_argcount++] =
                curarg;
              continue;
            };
          if (curarg[1] == 'O' || curarg[1] == 'g')
            {
              rpsconf_compiler_args[rpsconf_compiler_argcount++] = curarg;
              continue;
            };
          /// -std=gnu77 affects compiler and preprocessor
          if (!strncmp (curarg, "-std=", 5))
            {
              rpsconf_preprocessor_args[rpsconf_preprocessor_argcount++] =
                curarg;
              rpsconf_compiler_args[rpsconf_compiler_argcount++] = curarg;
              continue;
            }
          /// -fPIC and -fPIE affects compiler and linker
          /// -flto and -fwhopr affects compiler and linker
          if (!strcmp (curarg, "-flto") || !strcmp (curarg, "-fwhopr")
              || !strcmp (curarg, "-fPIC") || !strcmp (curarg, "-fPIE"))
            {
              rpsconf_compiler_args[rpsconf_compiler_argcount++] = curarg;
              rpsconf_linker_args[rpsconf_linker_argcount++] = curarg;
              continue;
            }
        }
      if (!isalpha (curarg[0]))
        break;
      char *pc = NULL;
      for (pc = curarg; *pc && (isalnum (*pc) || *pc == '_'); pc++);
      if (*pc == '=')
        putenv (curarg);
    };
}       /* end rpsconf_prelude */




int
main (int argc, char **argv)
{
  rpsconf_prog_name = argv[0];
  rpsconf_prelude (argc, argv);
  atexit (rpsconf_remove_files);
  printf ("%s: configurator program for RefPerSys inference engine\n",
          rpsconf_prog_name);
  printf ("%s: [FRENCH]\n\t programme de configuration du\n"  //
          "\t moteur d'inférences RefPerSys\n", rpsconf_prog_name);
  printf ("\t cf refpersys.org & github.com/RefPerSys/RefPerSys\n");
  printf ("\t   REFlexive PERsistent SYStem\n");
  printf ("\t Contact: Basile STARYNKEVITCH,\n" //
          "\t 8 rue de la Faïencerie,\n" //
          "\t 92340 Bourg-la-Reine\n" //
          "\t (France)\n");
  fflush (NULL);
  printf ("%s: when asked for a file path, you can run a shell command ...\n"
          "... if your input starts with an exclamation point\n",
          rpsconf_prog_name);
  printf
  ("\t When asked for file paths, you are expected to enter an absolute one,\n"
   "\t for example /etc/passwd\n"
   "\t if you enter something starting with ! it is a shell command\n"
   "\t which is run and the question is repeated.\n");
  fflush (NULL);
  if (argc > RPSCONF_MAX_PROG_ARGS)
    {
      fprintf (stderr,
               "%s (from C file %s) limits RPSCONF_MAX_PROG_ARGS to %d\n"
               "... but %d are given! Edit it and recompile!\n",
               argv[0], __FILE__, RPSCONF_MAX_PROG_ARGS, argc);
      rpsconf_failed = true;
      exit (EXIT_FAILURE);
    };
  printf
  ("\nThe C and C++ compilers (maybe $CC and $CXX) should be preferably\n"
   "from gcc.gnu.org (or at least compatible)\n");
  fflush (NULL);
  char *cc = getenv ("CC");
  if (!cc)
    {
      if (!access ("/usr/bin/gcc", F_OK))
        cc =
          rpsconf_defaulted_readline ("C compiler [default /usr/bin/gcc]: ",
                                      "/usr/bin/gcc");
      else
        cc = rpsconf_readline ("C compiler [default /usr/bin/gcc]: ");
    };
  if (!cc)
    cc = "/usr/bin/gcc";

  if (rpsconf_cc_set (cc) == RPSCONF_FAIL)
    exit (EXIT_FAILURE);

  char *cxx = getenv ("CXX");
  if (!cxx)
    {
      if (!access ("/usr/bin/g++", F_OK))
        cxx =
          rpsconf_defaulted_readline ("C++ compiler [default /usr/bin/g++:",
                                      "/usr/bin/g++");
      else
        cxx = rpsconf_readline ("C++ compiler [default /usr/bin/g++]: ");
    };

  if (!cxx)
    cxx = "/usr/bin/g++";

  rpsconf_try_then_set_cxx_compiler (cxx);
  rpsconf_try_cxx_compiler_for_libgccjit (cc);
  rpsconf_test_libgccjit_compilation (cc);
  rpsconf_builder_person =
    rpsconf_readline ("person building RefPerSys (eg Alan TURING):");
  if (rpsconf_builder_person && isspace (rpsconf_builder_person[0]))
    {
      free ((void *) rpsconf_builder_person);
      rpsconf_builder_person = NULL;
    };
  if (rpsconf_builder_person)
    {
      rpsconf_builder_email =
        rpsconf_readline
        ("email of person building (e.g. alan.turing@princeton.edu):");
      bool goodemail = rpsconf_builder_email != NULL
                       && isalnum (rpsconf_builder_email[0]);
      const char *pc = rpsconf_builder_email;
      for (pc = rpsconf_builder_email; *pc && goodemail && *pc != '@'; pc++)
        {
          if (!isalnum (*pc) && *pc != '+' && *pc != '-' && *pc != '_'
              && *pc != '.')
            goodemail = false;
        };
      if (goodemail && *pc == '@')
        pc++;
      else
        goodemail = false;
      int nbdots = 0;
      for (pc = pc;
           *pc && goodemail && (isalnum (*pc) || strchr ("+-_.:", *pc)); pc++)
        {
          if (*pc == '.')
            nbdots++;
        };
      if (nbdots == 0)
        goodemail = false;
      if (!goodemail)
        {
          free ((void *) rpsconf_builder_email);
          rpsconf_builder_email = NULL;
        }
    }
  errno = 0;

  rpsconf_fltk_config = getenv ("FLTKCONFIG");
  if (!rpsconf_fltk_config)
    {
      printf ("\nFLTK is a graphical toolkit from www.fltk.org\n"
              "\t providing a configurator script\n");
      fflush (stdout);
      rpsconf_fltk_config = rpsconf_readline ("FLTK configurator:");
      if (access (rpsconf_fltk_config, X_OK))
        {
          fprintf (stderr,
                   "%s bad FLTK configurator %s (%s) [%s:%d]\n",
                   rpsconf_prog_name,
                   rpsconf_fltk_config ? rpsconf_fltk_config : "???",
                   strerror (errno), __FILE__, __LINE__ - 3);
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        }
    }

  if (access ("generated/rpsdata.h", R_OK))
    {
      char datapath[RPSCONF_PATH_MAXLEN];
      memset (datapath, 0, sizeof (datapath));
      snprintf (datapath, sizeof (datapath) - 1,
                "rpsdata_%s_%s.h", rpsconf_opersys, rpsconf_arch);
      if (symlink (datapath, "generated/rpsdata.h"))
        {
          fprintf (stderr,
                   "%s failed symlink %s to %s : %s [%s:%d]\n",
                   rpsconf_prog_name, "generated/rpsdata.h", datapath,
                   strerror (errno), __FILE__, __LINE__ - 3);
          rpsconf_failed = true;
          exit (EXIT_FAILURE);
        };
      if (rpsconf_verbose)
        {
          printf ("%s symlinked %s to %s [%s:%d]\n",
                  rpsconf_prog_name, "generated/rpsdata.h", datapath,
                  __FILE__, __LINE__ - 2);
          fflush (NULL);
        }
    }
  else
    {
      if (rpsconf_verbose)
        {
          printf ("%s accessed generated/rpsdata.h [%s:%d]\n",
                  rpsconf_prog_name, __FILE__, __LINE__ - 2);
        }
    };
  ///emit file config-refpersys.mk to be included by GNU make
  rpsconf_emit_configure_refpersys_mk ();
  fprintf (stderr,
           "[%s:%d] perhaps missing code to emit some refpersys-config.h....\n"
           "git %s opersys %s arch %s host %s in %s on %s\n",
           __FILE__, __LINE__ - 2,
           rpsconf_gitid, rpsconf_opersys, rpsconf_arch, rpsconf_host,
           rpsconf_cwd_buf, rpsconf_host_name);
  return 0;
#warning TODO perhaps we should emit also a refpersys-config.h file
  /// that hypothetical refpersys-config.h would be included by refpersys.hh
}       /* end main */







////////////////////////////////
/*
 * Helper Functions
 */

/*
 * Function: rpsconf_diag__
 *   Prints a diagnostic message to stderr, with optional verbosity.
 *
 * Inputs:
 *   - file: source file path, provided by __FILE__
 *   - line: source file line number, provided by __LINE__
 *   - fmt: formatted message string
 *
 * Outputs:
 *   Prints diagnostic message on to stderr
 *
 * Preconditions:
 *   - file is not null
 *   - file is not an empty string
 *   - line > 0
 *   - fmt is not null
 *   - fmt is not an empty string
 *
 * Postconditions:
 *   None
 */
void
rpsconf_diag__ (const char *file, int line, const char *fmt, ...)
{
  va_list ap;

  assert (file != NULL && *file != '\0');
  assert (line > 0);
  assert (fmt != NULL && *fmt != '\0');

  va_start (ap, fmt);
  (void) fprintf (stderr, "%s: error: ", rpsconf_prog_name);
  (void) vfprintf (stderr, fmt, ap);

  if (rpsconf_verbose)
    (void) fprintf (stderr, " [%s:%d]\n", file, line);
  else
    (void) fputs ("\n", stderr);

  va_end (ap);
}

/*
 * Function: rpsconf_cc_test
 *   Attempts to compile and run a simple "Hello, world" C program.
 *
 * Inputs:
 *   - cc: absolute path to C compiler
 *
 * Outputs:
 *   None
 *
 * Preconditions:
 *   - cc is not null
 *
 * Postconditions:
 *   None
 */
void
rpsconf_cc_test (const char *cc)
{
  char *helloworldsrc = NULL;
  char helloworldbin[128];
  memset (helloworldbin, 0, sizeof (helloworldbin));
  helloworldsrc =
    rpsconf_temporary_textual_file ("tmp_helloworld_", ".c", __LINE__);
  FILE *hwf = fopen (helloworldsrc, "w");
  if (!hwf)
    {
      fprintf (stderr,
               "%s failed to create temporary hello world C %s (%m)\n",
               rpsconf_prog_name, helloworldsrc);
    }
  fprintf (hwf, "/// temporary hello world C file %s\n", helloworldsrc);
  fprintf (hwf, "#include <stdio.h>\n");
  fprintf (hwf, "void say_hello(const char*c) {\n");
  fprintf (hwf, " printf(\"hello from %%s in %%s:%%d\\n\",\n");
  fprintf (hwf, "        c, __FILE__, __LINE__);\n");
  fprintf (hwf, "} //end say_hello\n");
  fprintf (hwf, "\n\n");
  fprintf (hwf, "int main(int argc,char**argv) { say_hello(argv[0]); }\n");
  fclose (hwf);
  snprintf (helloworldbin, sizeof (helloworldbin), "./%s",
            basename (helloworldsrc));
  char *lastdot = strrchr (helloworldbin, '.');
  if (lastdot)
    strcpy (lastdot, ".bin");
  else
    strcat (helloworldbin, ".bin");
  assert (lastdot);
  {
    char helloworldcompile[512];
    memset (helloworldcompile, 0, sizeof (helloworldcompile));
    snprintf (helloworldcompile, sizeof (helloworldcompile),
              "%s -Wall -O %s -o %s", cc, helloworldsrc, helloworldbin);
    printf ("trying %s\n", helloworldcompile);
    fflush (NULL);
    int e = system (helloworldcompile);
    if (e)
      {
        fprintf (stderr,
                 "%s failed to compile hello world in C : %s exited %d\n",
                 rpsconf_prog_name, helloworldcompile, e);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    rpsconf_should_remove_file (helloworldsrc, __LINE__);
    rpsconf_should_remove_file (helloworldbin, __LINE__);
    printf ("popen-eing %s\n", helloworldbin);
    fflush (NULL);
    FILE *pf = popen (helloworldbin, "r");
    if (!pf)
      {
        fprintf (stderr, "%s failed to popen hello world in C %s (%m)\n",
                 rpsconf_prog_name, helloworldbin);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    bool gothello = false;
    int nblin = 0;
    do
      {
        char hwline[128];
        memset (hwline, 0, sizeof (hwline));
        if (!fgets (hwline, sizeof (hwline), pf))
          break;
        nblin++;
        if (strstr (hwline, "hello"))
          gothello = true;
      }
    while (!feof (pf));
    if (!gothello)
      {
        fprintf (stderr,
                 "%s popen %s without hello but read %d lines from popen [%s:%d]\n",
                 rpsconf_prog_name, helloworldbin, nblin, __FILE__,
                 __LINE__ - 1);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      }
    int ehw = pclose (pf);
    if (ehw)
      {
        fprintf (stderr, "%s bad pclose %s (%d)\n",
                 rpsconf_prog_name, helloworldbin, ehw);
        rpsconf_failed = true;
        exit (EXIT_FAILURE);
      };
    printf ("%s: tested hello world C compilation and run [%s:%d]\n",
            rpsconf_prog_name, __FILE__, __LINE__);
  }
}       /* end rpsconf_cc_test */


/*
 * Function: rpsconf_cc_set
 *   Sets the path to the C compiler after checking that it works.
 *
 * Inputs:
 *   - cc: absolute path to C compiler
 *
 * Outputs:
 *   - RPSCONF_OK:   function succeeded
 *   - RPSCONF_FAIL: function failed
 *
 * Preconditions:
 *   - cc is not null
 *   - cc is not an empty string
 *   - cc is an absolute path
 *
 * Postconditions:
 *   None
 */
int
rpsconf_cc_set (const char *cc)
{
  assert (cc != NULL);
  if (*cc == '\0')
    {
      fprintf (stderr, "C compiler path not specified, using /usr/bin/gcc\n");
      cc = "/usr/bin/gcc";
    };

  if (*cc != '/')
    {
      RPSCONF_DIAG ("%s: C compiler path not absolute", cc);
      return RPSCONF_FAIL;
    };

  errno = 0;
  if (access (cc, X_OK))
    {
      RPSCONF_DIAG ("%s: C compiler path not executable", cc);
      return RPSCONF_FAIL;
    };

  rpsconf_cc_test (cc);
  rpsconf_c_compiler = cc;
  return RPSCONF_OK;
}       /* end rpsconf_cc_set */


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make -C $REFPERSYS_TOPDIR/. do-configure-refpersys" ;;
 ** End: ;;
 ****************/

/// end of file do-configure-refpersys.c

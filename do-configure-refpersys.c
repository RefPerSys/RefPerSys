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
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef WITHOUT_READLINE
#include "readline/readline.h"
#endif

#ifndef MAX_PROG_ARGS
#define MAX_PROG_ARGS 1024
#endif

#ifndef MY_PATH_MAXLEN
#define MY_PATH_MAXLEN 384
#endif

#ifndef GIT_ID
#error GIT_ID should be defined at compilation time
#endif

const char *prog_name;
bool failed;

//// working directory
char my_cwd_buf[MY_PATH_MAXLEN];

//// host name
char my_host_name[80];

//// the below arguments are kept, they should be rewritten in
//// _config-refpersys.mk in a GNU make friendly way.

char *preprocessor_args[MAX_PROG_ARGS];
int preprocessor_argcount;
char *compiler_args[MAX_PROG_ARGS];
int compiler_argcount;
char *linker_args[MAX_PROG_ARGS];
int linker_argcount;

/* absolute path to C and C++ compiler */
const char *c_compiler;
const char *cpp_compiler;

/* strdup-ed string of the person building RefPerSys (or null): */
const char *builder_person;

/* strdup-ed string of the email of the person building RefPerSys (or null): */
const char *builder_email;

/* absolute path to fltk-config utility */
const char *fltk_config;


/* absolute path to Miller&Auroux Generic preprocessor */
const char *gpp;

/* absolute path to ninja builder (see ninja-build.org) */
const char *ninja_builder;

#ifndef MAX_REMOVED_FILES
#define MAX_REMOVED_FILES 4096
#endif

const char *files_to_remove_at_exit[MAX_REMOVED_FILES];
int removedfiles_count;

/// return a malloced path to a temporary textual file inside /tmp
char *temporary_textual_file (const char *prefix, const char *suffix,
			      int lineno);
/// return a malloced path to a temporary binary file in the current directory
char *temporary_binary_file (const char *prefix, const char *suffix,
			     int lineno);

/// emit the configure-refperys.mk file to be included in GNUmakefile 
void emit_configure_refpersys_mk (void);

static void rps_conf_try_then_set_c_compiler (const char *);
static void rps_conf_try_compile_run_hello_world_in_c (const char *);

void try_then_set_cxx_compiler (const char *cxx);
void should_remove_file (const char *path, int lineno);




/// return a malloced path to a temporary textual file
char *
temporary_textual_file (const char *prefix, const char *suffix, int lineno)
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
	       "%s failed to mkostemps from %s:%d\n", prog_name, __FILE__,
	       lineno);
      failed = true;
      exit (EXIT_FAILURE);
    };
  strcat (buf, suffix);
  res = strdup (buf);
  if (!res)
    {
      fprintf (stderr,
	       "%s failed to strdup temporay file path %s from %s:%d (%m)\n",
	       prog_name, buf, __FILE__, lineno);
      failed = true;
      exit (EXIT_FAILURE);
    };
  close (fd);
  printf ("%s temporary textual file is %s [%s:%d]\n",
	  prog_name, res, __FILE__, lineno);
  return res;
}				/* end temporary_textual_file */

/// return a malloced path to a temporary binary file in the current directory
char *
temporary_binary_file (const char *prefix, const char *suffix, int lineno)
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
	       "%s failed to mkostemps from %s:%d\n", prog_name, __FILE__,
	       lineno);
      failed = true;
      exit (EXIT_FAILURE);
    };
  close (fd);
  strcat (buf, suffix);
  res = strdup (buf);
  if (!res)
    {
      fprintf (stderr,
	       "%s failed to strdup temporary binary file path %s from %s:%d (%m)\n",
	       prog_name, buf, __FILE__, lineno);
      failed = true;
      exit (EXIT_FAILURE);
    };
  printf ("%s temporary binary file is %s [%s:%d]\n",
	  prog_name, res, __FILE__, lineno);
  return res;
}				/* end temporary_binary_file */

char *
my_readline (const char *prompt)
{
  bool again = false;
#ifndef WITHOUT_READLINE
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
	  perror ("my_readline");
	  failed = true;
	  exit (EXIT_FAILURE);
	};
    }
  while again;
  return res;
#endif // WITHOUT_READLINE
}				// end my_readline

void
should_remove_file (const char *path, int lineno)
{
  if (access (path, F_OK))
    return;
  if (removedfiles_count >= MAX_REMOVED_FILES - 1)
    {
      fprintf (stderr,
	       "%s too many files to remove (%s) from %s:%d\n",
	       prog_name, path, __FILE__, lineno);
      failed = true;
      exit (EXIT_FAILURE);
    }
  files_to_remove_at_exit[removedfiles_count++] = path;
}				/* end should_remove_file */



void
test_cxx_compiler (const char *cxx)
{
  /* Generate two temporary simple C++ files, compile both of them in
     two commands, link the object files, and run the temporary
     helloworld executable in C++ */
  char *showvectsrc = NULL;
  char *maincxxsrc = NULL;
  char showvectobj[128];
  char maincxxobj[128];
  errno = 0;
  showvectsrc = temporary_textual_file ("tmp_showvect", ".cxx", __LINE__);
  maincxxsrc = temporary_textual_file ("tmp_maincxx", ".cxx", __LINE__);
  errno = 0;
  /// write the show vector C++ file
  {
    FILE *svf = fopen (showvectsrc, "w");
    if (!svf)
      {
	fprintf (stderr,
		 "%s failed to create temporary show vector C++ %s (%m)[%s:%d]\n",
		 prog_name, showvectsrc, __FILE__, __LINE__);
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
    printf ("%s wrote C++ file %s (%s:%d)\n", prog_name, showvectsrc,
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
    printf ("%s compiling with %s\n", prog_name, compilshowvect);
    fflush (NULL);
    int ex = system (compilshowvect);
    if (ex)
      {
	fprintf (stderr, "%s: failed to compile with %s (exit %d)\n",
		 prog_name, compilshowvect, ex);
	failed = true;
	exit (EXIT_FAILURE);
      };
    should_remove_file (showvectsrc, __LINE__);
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
		 prog_name, maincxxsrc);
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
    printf ("%s wrote main C++ file %s (%s:%d)\n", prog_name, maincxxsrc,
	    __FILE__, __LINE__ - 1);
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
    printf ("%s compiling with %s\n", prog_name, compilmaincxx);
    fflush (NULL);
    int ex = system (compilmaincxx);
    if (ex)
      {
	fprintf (stderr, "%s: failed to compile with %s (exit %d)\n",
		 prog_name, compilmaincxx, ex);
	failed = true;
	exit (EXIT_FAILURE);
      };
  }
  /// link the two objects
  char *cxxexe = temporary_binary_file ("./tmp_cxxprog", ".bin", __LINE__);
  {
    char linkmaincxx[3 * 128];
    memset (linkmaincxx, 0, sizeof (linkmaincxx));
    snprintf (linkmaincxx, sizeof (linkmaincxx),
	      "%s %s  %s -o %s", cxx, maincxxobj, showvectobj, cxxexe);
    should_remove_file (maincxxsrc, __LINE__);
    should_remove_file (maincxxobj, __LINE__);
    should_remove_file (showvectsrc, __LINE__);
    should_remove_file (showvectobj, __LINE__);
    printf ("%s running C++ link %s [%s:%d]\n",
	    prog_name, linkmaincxx, __FILE__, __LINE__ - 1);
    fflush (NULL);
    int ex = system (linkmaincxx);
    if (ex)
      {
	fprintf (stderr, "%s: failed to compile with %s (exit %d)\n",
		 prog_name, linkmaincxx, ex);
	failed = true;
	exit (EXIT_FAILURE);
      };
  }
  /// run the C++ exe
  should_remove_file (cxxexe, __LINE__);
  {
    char cmdbuf[256];
    memset (cmdbuf, 0, sizeof (cmdbuf));
    snprintf (cmdbuf, sizeof (cmdbuf), "%s at %s:%d from %s",
	      cxxexe, __FILE__, __LINE__, prog_name);
    printf ("%s testing popen %s [%s:%d]\n", prog_name, cmdbuf,
	    __FILE__, __LINE__ - 1);
    fflush (NULL);
    FILE *pf = popen (cxxexe, "r");
    if (!pf)
      {
	fprintf (stderr, "%s failed to popen %s in C++ (%m)\n",
		 prog_name, cxxexe);
	failed = true;
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
	  fprintf (stderr, "%s bad pclose %s (%d)\n", prog_name, cxxexe, ehw);
	  failed = true;
	  exit (EXIT_FAILURE);
	};
      if (!gothello || !gotfilename)
	{
	  fprintf (stderr,
		   "%s no hello or file name from C++ test popen %s [%s:%d]\n",
		   prog_name, cmdbuf, __FILE__, __LINE__ - 1);
	  failed = true;
	  exit (EXIT_FAILURE);
	};
    }
  }
}				/* end test_cxx_compiler */


void
try_then_set_cxx_compiler (const char *cxx)
{
  if (cxx[0] != '/')
    {
      fprintf (stderr,
	       "%s given non-absolute path for C++ compiler '%s' [%s:%d]\n",
	       prog_name, cxx, __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    };
  if (access (cxx, X_OK))
    {
      fprintf (stderr,
	       "%s given non-executable path for C++ compiler '%s' [%s:%d]\n",
	       prog_name, cxx, __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  test_cxx_compiler (cxx);
  cpp_compiler = cxx;
}				/* end try_then_set_cxx_compiler */


void
try_then_set_fltkconfig (const char *fc)
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
	       prog_name, fc, (int) sizeof (cmdbuf) - 16);
      failed = true;
      exit (EXIT_FAILURE);
    };
  if (access (fc, R_OK | X_OK))
    {
      fprintf (stderr,
	       "%s: cannot access FLTK configurator %s (%s) [%s:%d]\n",
	       prog_name, fc, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  /// run fltk-config -g --cflags
  memset (cmdbuf, 0, sizeof (cmdbuf));
  snprintf (cmdbuf, sizeof (cmdbuf), "%s -g --cflags", fc);
  printf ("%s running %s [%s:%d]\n", prog_name, cmdbuf, __FILE__, __LINE__);
  fflush (NULL);
  pipf = popen (cmdbuf, "r");
  if (!pipf)
    {
      fprintf (stderr, "%s: failed to popen %s (%s) [%s:%d]\n",
	       prog_name, cmdbuf, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  if (!fgets (fcflags, sizeof (fcflags), pipf))
    {
      fprintf (stderr, "%s: failed to get cflags using %s (%s) [%s:%d]\n",
	       prog_name, cmdbuf, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  if (pclose (pipf))
    {
      fprintf (stderr, "%s: failed to pclose %s (%s) [%s:%d]\n",
	       prog_name, cmdbuf, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  fflush (NULL);
  pipf = NULL;
  /// run fltk-config -g --ldflags
  memset (cmdbuf, 0, sizeof (cmdbuf));
  snprintf (cmdbuf, sizeof (cmdbuf), "%s -g --ldlags", fc);
  printf ("%s running %s [%s:%d]\n", prog_name, cmdbuf, __FILE__, __LINE__);
  pipf = popen (cmdbuf, "r");
  if (!pipf)
    {
      fprintf (stderr, "%s: failed to popen %s (%s) [%s:%d]\n",
	       prog_name, cmdbuf, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  if (!fgets (fldflags, sizeof (fldflags), pipf))
    {
      fprintf (stderr, "%s: failed to get ldflags using %s (%s) [%s:%d]\n",
	       prog_name, cmdbuf, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  if (pclose (pipf))
    {
      fprintf (stderr, "%s: failed to pclose %s (%s) [%s:%d]\n",
	       prog_name, cmdbuf, strerror (errno), __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    }
  fflush (NULL);
  pipf = NULL;
  const char *tmp_testfltk_src
    = temporary_textual_file ("tmp_test_fltk", ".cc", __LINE__);
  FILE *fltksrc = fopen (tmp_testfltk_src, "w");
  if (!fltksrc)
    {
      fprintf (stderr,
	       "%s: failed to fopen for FLTK testing %s (%s) [%s:%d]\n",
	       prog_name, tmp_testfltk_src, strerror (errno), __FILE__,
	       __LINE__ - 2);
      failed = true;
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
	       prog_name, tmp_testfltk_src, strerror (errno), __FILE__,
	       __LINE__ - 2);
      failed = true;
      exit (EXIT_FAILURE);
    };
  fltksrc = NULL;
  char *tmp_fltk_exe =
    temporary_binary_file ("./tmp_fltkprog", ".bin", __LINE__);
  memset (cmdbuf, 0, sizeof (cmdbuf));
  snprintf (cmdbuf, sizeof (cmdbuf), "%s -g -O %s %s %s -o %s",
	    cpp_compiler, fcflags, tmp_testfltk_src, fldflags, tmp_fltk_exe);
  printf ("%s build test FLTK executable %s from %s with %s\n", prog_name,
	  tmp_fltk_exe, tmp_testfltk_src, cpp_compiler);
  fflush (NULL);
  if (system (cmdbuf) > 0)
    {
      fprintf (stderr,
	       "%s failed build test FLTK executable %s from %s [%s:%d]\n",
	       prog_name, tmp_fltk_exe, tmp_testfltk_src, __FILE__,
	       __LINE__ - 1);
      fflush (stderr);
      fprintf (stderr, "... using\n%s\n...[%s:%d]\n",
	       cmdbuf, __FILE__, __LINE__);
      fflush (NULL);
      failed = true;
      exit (EXIT_FAILURE);
    }
  should_remove_file (tmp_testfltk_src, __LINE__);
  should_remove_file (tmp_fltk_exe, __LINE__);
}				/* end try_then_set_fltkconfig */

void
remove_files (void)
{
  if (failed)
    {
      printf ("%s: not removing %d files since failed at exit [%s:%d]\n",
	      prog_name, removedfiles_count, __FILE__, __LINE__);
      return;
    }
  else
    {
      printf ("%s: removing %d files at exit [%s:%d]\n",
	      prog_name, removedfiles_count, __FILE__, __LINE__);
      for (int i = 0; i < removedfiles_count; i++)
	unlink (files_to_remove_at_exit[i]);
    }
}				/* end remove_files */




void
emit_configure_refpersys_mk (void)
{
  const char *tmp_conf
    = temporary_textual_file ("tmp_config_refpersys", ".mk", __LINE__);
  FILE *f = fopen (tmp_conf, "w");
  if (!f)
    {
      fprintf (stderr,
	       "%s failed to fopen %s for _config-refpersys.mk (%m)\n",
	       prog_name, tmp_conf);
      failed = true;
      exit (EXIT_FAILURE);
    };
  time_t nowt = time (NULL);
  fprintf (f, "# generated _config-refpersys.mk for GNU make in refpersys\n");
  fprintf (f, "# DO NOT EDIT but use make config\n");
  fprintf (f, "# generated from %s:%d in %s\n", __FILE__, __LINE__,
	   my_cwd_buf);
  fprintf (f, "# see refpersys.org\n");
  fprintf (f, "# generated at %s## on %s git %s\n\n",
	   ctime (&nowt), my_host_name, GIT_ID);
  fprintf (f, "#  generated from %s:%d\n", __FILE__, __LINE__);
  fprintf (f, "REFPERSYS_CONFIGURED_GITID=%s\n\n", GIT_ID);
  //// emit C compiler
  fprintf (f, "\n\n" "# the C compiler for RefPerSys:\n");
  fprintf (f, "REFPERSYS_CC=%s\n", c_compiler);
  //// emit C++ compiler
  fprintf (f, "\n\n" "# the C++ compiler for RefPerSys:\n");
  fprintf (f, "REFPERSYS_CXX=%s\n", cpp_compiler);
  //// emit preprocessor flags
  if (preprocessor_argcount)
    {
      fprintf (f, "\n\n"
	       "# the given %d preprocessor flags for RefPerSys:\n",
	       preprocessor_argcount);
      fprintf (f, "REFPERSYS_PREPRO_FLAGS=");
      for (int i = 0; i < preprocessor_argcount; i++)
	{
	  if (i > 0)
	    fputc (' ', f);
	  fputs (preprocessor_args[i], f);
	};
    }
  else
    {
      fprintf (f, "\n\n" "# the default preprocessor flags for RefPerSys:\n");
      fprintf (f, "REFPERSYS_PREPRO_FLAGS= -I/usr/local/include\n");
    }
  //// emit the given or default compiler flags
  if (compiler_argcount > 0)
    {
      fprintf (f, "\n\n"
	       "# the given %d compiler flags for RefPerSys:\n",
	       compiler_argcount);
      fprintf (f, "REFPERSYS_COMPILER_FLAGS=");
      for (int i = 0; i < compiler_argcount; i++)
	{
	  if (i > 0)
	    fputc (' ', f);
	  fputs (compiler_args[i], f);
	};
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
      fprintf (f, "#GNU compiler from %s:%d\n"
	       "REFPERSYS_COMPILER_FLAGS= -Og -g -fPIC -Wall -Wextra\n",
	       __FILE__, __LINE__-2);
#else
      fprintf (f, "#nonGNU compiler from %s:%d\n"
	       "## see stackoverflow.com/questions/2224334/\n"
	       "REFPERSYS_COMPILER_FLAGS= -O0 -g -fPIC -Wall",
	       __FILE__, __LINE__-3);
#endif
    }
  //// emit linker flags
  if (linker_argcount > 0)
    {
      fprintf (f, "\n\n"
	       "# the given %d linker flags for RefPerSys:\n",
	       linker_argcount);
      fprintf (f, "REFPERSYS_LINKER_FLAGS=");
      for (int i = 0; i < linker_argcount; i++)
	{
	  if (i > 0)
	    fputc (' ', f);
	  fputs (linker_args[i], f);
	};
    }
  else
    {
      fprintf (f, "# default linker flags for RefPerSys [%s:%d]:\n",
	       __FILE__, __LINE__ - 1);
      fprintf (f,
	       "REFPERSYS_LINKER_FLAGS= -L/usr/local/lib -rdynamic -ldl\n");
    }
  //// emit the generic preprocessor
  fprintf (f,
	   "\n\n"
	   "# the Generic Preprocessor for RefPerSys (see logological.org/gpp):\n");
  fprintf (f, "REFPERSYS_GPP=%s\n", realpath (gpp, NULL));
  /// emit the ninja builder
  fprintf (f, "\n\n" "# ninja builder from ninja-build.org\n");
  fprintf (f, "REFPERSYS_NINJA=%s\n", realpath (ninja_builder, NULL));
  if (builder_person) {
    fprintf(f, "## refpersys builder person and perhaps email\n");
    fprintf(f, "REFPERSYS_BUILDER_PERSON='%s'\n", builder_person);
    if (builder_email) {
      fprintf(f, "REFPERSYS_BUILDER_EMAIL='%s'\n", builder_email);
    }
  }
  //// emit the FLTK configurator
  if (fltk_config) {
    fprintf (f, "\n# FLTK (see fltk.org) configurator\n");
    fprintf (f, "REFPERSYS_FLTKCONFIG=%s\n", fltk_config);
  }
  ////
  fprintf (f, "\n\n### end of generated _config-refpersys.mk file\n");
  fflush (f);
  if (!link (tmp_conf, "_config-refpersys.mk"))
    {
      fprintf (stderr, "%s failed to link %s to _config-refpersys.mk (%m)\n",
	       prog_name, tmp_conf);
      failed = true;
      exit (EXIT_FAILURE);
    };
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
		 prog_name, mvcmdbuf, __FILE__, __LINE__ - 1);
	fflush (NULL);
	failed = true;
	exit (EXIT_FAILURE);
      }
    sync ();
  }
}				/* end emit_configure_refpersys_mk */


void
usage (void)
{
  puts ("# configuration utility program for refpersys.org");
  printf ("%s usage:\n", prog_name);
  puts ("\t --version               # show version");
  puts ("\t --help                  # this help");
  puts ("\t <var>=<value>           # putenv, set environment variable");
  puts ("\t                         # ... e.g. PRINTER=lp0");
  puts ("\t CC=<C compiler>         # set the C compiler,");
  puts ("\t                         # e.g. CC=/usr/bin/gcc");
  puts ("\t CXX=<C++ compiler>      # set the C++ compiler,");
  puts ("\t                         # e.g. CXX=/usr/bin/g++");
  puts ("\t NINJA=<ninja-builder>   # set builder from ninja-build.org");
  puts ("\t                         # e.g. NINJA=/usr/bin/ninja");
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
}				/* end usage */

int
main (int argc, char **argv)
{
  prog_name = argv[0];
  if (argc == 2 && !strcmp (argv[1], "--help"))
    {
      usage ();
      return 0;
    };
  if (argc == 2 && !strcmp (argv[1], "--version"))
    {
      printf ("%s version gitid %s built on %s:%s\n",
	      prog_name, GIT_ID, __DATE__, __TIME__);
    };
  memset (my_cwd_buf, 0, sizeof (my_cwd_buf));
  if (!getcwd (my_cwd_buf, sizeof (my_cwd_buf)))
    {
      fprintf (stderr, "%s failed to getcwd (%m) [%s:%d]\n",
	       prog_name, __FILE__, __LINE__ - 1);
      failed = true;
      exit (EXIT_FAILURE);
    };
  memset (my_host_name, 0, sizeof (my_host_name));
  if (gethostname (my_host_name, sizeof (my_host_name) - 1))
    {
      fprintf (stderr, "%s failed to gethostname (%m) [%s:%d]\n",
	       prog_name, __FILE__, __LINE__ - 1);
      failed = true;
      exit (EXIT_FAILURE);
    };
  assert (sizeof (my_cwd_buf) == MY_PATH_MAXLEN);
  if (my_cwd_buf[MY_PATH_MAXLEN - 2])
    {
      my_cwd_buf[MY_PATH_MAXLEN - 1] = (char) 0;
      fprintf (stderr,
	       "%s failed too long current working directory %s [%s:%d]\n",
	       prog_name, my_cwd_buf, __FILE__, __LINE__ - 1);
      failed = true;
      exit (EXIT_FAILURE);
    };
  atexit (remove_files);
  printf ("%s: when asked for a file path, you can run a shell command ...\n"
	  "... if your input starts with an exclamation point\n", prog_name);
  if (argc > MAX_PROG_ARGS)
    {
      fprintf (stderr,
	       "%s (from C file %s) limits MAX_PROG_ARGS to %d\n"
	       "... but %d are given! Edit it and recompile!\n",
	       argv[0], __FILE__, MAX_PROG_ARGS, argc);
      failed = true;
      exit (EXIT_FAILURE);
    };
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
	      preprocessor_args[preprocessor_argcount++] = curarg;
	      continue;
	    };
	  if (curarg[1] == 'O' || curarg[1] == 'g')
	    {
	      compiler_args[compiler_argcount++] = curarg;
	      continue;
	    };
	  /// -std=gnu77 affects compiler and preprocessor
	  if (!strncmp (curarg, "-std=", 5))
	    {
	      preprocessor_args[preprocessor_argcount++] = curarg;
	      compiler_args[compiler_argcount++] = curarg;
	      continue;
	    }
	  /// -fPIC and -fPIE affects compiler and linker
	  /// -flto and -fwhopr affects compiler and linker
	  if (!strcmp (curarg, "-flto") || !strcmp (curarg, "-fwhopr")
	      || !strcmp (curarg, "-fPIC") || !strcmp (curarg, "-fPIE"))
	    {
	      compiler_args[compiler_argcount++] = curarg;
	      linker_args[linker_argcount++] = curarg;
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
  char *cc = getenv ("CC");
  if (!cc)
    cc = my_readline ("C compiler, preferably gcc:");
  if (!cc)
    cc = "/usr/bin/gcc";
  rps_conf_try_then_set_c_compiler (cc);
  char *cxx = getenv ("CXX");
  if (!cxx)
    cxx = my_readline ("C++ compiler:");
  try_then_set_cxx_compiler (cxx);
  builder_person =
    my_readline ("person building RefPerSys (eg Alan TURING):");
  if (builder_person && isspace (builder_person[0]))
    {
      free ((void *) builder_person);
      builder_person = NULL;
    };
  if (builder_person)
    {
      builder_email =
	my_readline
	("email of person building (e.g. alan.turing@princeton.edu):");
      bool goodemail = builder_email != NULL && isalnum (builder_email[0]);
      const char *pc = builder_email;
      for (pc = builder_email; *pc && goodemail && *pc != '@'; pc++)
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
	  free ((void *) builder_email);
	  builder_email = NULL;
	}
    }
  errno = 0;
  gpp = getenv ("GPP");
  if (!gpp)
    {
      puts
	("Generic Preprocessor (by Tristan Miller and Denis Auroux, see logological.org/gpp ...)");
      gpp = my_readline ("Generic Preprocessor full path:");
      if (access (gpp, X_OK))
	{
	  fprintf (stderr,
		   "%s bad Generic Preprocessor %s (%s) [%s:%d]\n",
		   prog_name, gpp ? gpp : "???", strerror (errno),
		   __FILE__, __LINE__ - 3);
	  failed = true;
	  exit (EXIT_FAILURE);
	}
    };
  assert (gpp != NULL);

  ninja_builder = getenv ("NINJA");
  if (!ninja_builder)
    {
      ninja_builder = my_readline ("ninja builder:");
      if (access (ninja_builder, X_OK))
	{
	  fprintf (stderr,
		   "%s bad ninja builder %s (%s) [%s:%d]\n",
		   prog_name, ninja_builder ? ninja_builder : "???",
		   strerror (errno), __FILE__, __LINE__ - 3);
	  failed = true;
	  exit (EXIT_FAILURE);
	}
    };
  fltk_config = getenv ("FLTKCONFIG");
  if (!fltk_config)
    {
      fltk_config = my_readline ("FLTK configurator:");
      if (access (fltk_config, X_OK))
	{
	  fprintf (stderr,
		   "%s bad FLTK configurator %s (%s) [%s:%d]\n",
		   prog_name, fltk_config ? fltk_config : "???",
		   strerror (errno), __FILE__, __LINE__ - 3);
	  failed = true;
	  exit (EXIT_FAILURE);
	}
    }
  ///emit file config-refpersys.mk to be included by GNU make 
  emit_configure_refpersys_mk ();
  fprintf (stderr,
	   "[%s:%d] perhaps missing code to emit some refpersys-config.h....\n",
	   __FILE__, __LINE__);
  return 0;
#warning TODO perhaps we should emit also a refpersys-config.h file
  /// that hypothetical refpersys-config.h would be included by refpersys.hh
}				/* end main */

/*
 * Helper Functions
 */

void
rps_conf_try_compile_run_hello_world_in_c (const char *cc)
{
  char *helloworldsrc = NULL;
  char helloworldbin[128];
  memset (helloworldbin, 0, sizeof (helloworldbin));
  helloworldsrc = temporary_textual_file ("tmp_helloworld_", ".c", __LINE__);
  FILE *hwf = fopen (helloworldsrc, "w");
  if (!hwf)
    {
      fprintf (stderr,
	       "%s failed to create temporary hello world C %s (%m)\n",
	       prog_name, helloworldsrc);
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
		 prog_name, helloworldcompile, e);
	failed = true;
	exit (EXIT_FAILURE);
      };
    should_remove_file (helloworldsrc, __LINE__);
    should_remove_file (helloworldbin, __LINE__);
    printf ("popen-eing %s\n", helloworldbin);
    fflush (NULL);
    FILE *pf = popen (helloworldbin, "r");
    if (!pf)
      {
	fprintf (stderr, "%s failed to popen hello world in C %s (%m)\n",
		 prog_name, helloworldbin);
	failed = true;
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
		 prog_name, helloworldbin, nblin, __FILE__, __LINE__ - 1);
	failed = true;
	exit (EXIT_FAILURE);
      }
    int ehw = pclose (pf);
    if (ehw)
      {
	fprintf (stderr, "%s bad pclose %s (%d)\n",
		 prog_name, helloworldbin, ehw);
	failed = true;
	exit (EXIT_FAILURE);
      };
    printf ("%s: tested hello world C compilation and run [%s:%d]\n",
	    prog_name, __FILE__, __LINE__);
  }
}				/* end rps_conf_try_compile_run_hello_world_in_c */


/*
 * Function: rps_conf_try_then_set_c_compiler
 *
 * Inputs:
 *   cc - absolute path to C compiler
 *
 * Outputs:
 *   None
 *
 * Preconditions:
 *   1. cc is not null
 *
 * Postconditions:
 *   None
 */
void
rps_conf_try_then_set_c_compiler (const char *cc)
{
  assert(cc != NULL);

  errno = 0;
  if (cc[0] != '/')
    {
      fprintf (stderr,
	       "%s given non-absolute path for C compiler '%s' [%s:%d]\n",
	       prog_name, cc, __FILE__, __LINE__);
      failed = true;
      exit (EXIT_FAILURE);
    };
  errno = 0;
  if (access (cc, X_OK))
    {
      fprintf (stderr,
	       "%s given non-executable path for C compiler '%s' [%s:%d] %s\n",
	       prog_name, cc, __FILE__, __LINE__ - 1, strerror (errno));
      failed = true;
      exit (EXIT_FAILURE);
    }
  errno = 0;
  rps_conf_try_compile_run_hello_world_in_c (cc);
  c_compiler = cc;
}				/* end rps_conf_try_then_set_c_compiler */


/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make do-configure-refpersys" ;;
 ** End: ;;
 ****************/

/// end of file do-configure-refpersys.c

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
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#ifndef WITHOUT_READLINE
#include "readline/readline.h"
#endif

#ifndef MAX_PROG_ARGS
#define MAX_PROG_ARGS 1024
#endif

const char *prog_name;


//// the below arguments are kept, they should be rewritten in
//// config-refpersys.mk in a GNU make friendly way.

char *preprocessor_args[MAX_PROG_ARGS];
int preprocessor_argcount;
char *compiler_args[MAX_PROG_ARGS];
int compiler_argcount;
char *linker_args[MAX_PROG_ARGS];
int linker_argcount;

/* absolute path to C and C++ compiler */
const char *c_compiler;
const char *cpp_compiler;

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

void try_c_compiler (const char *cc);
void try_then_set_cxx_compiler (const char *cxx);
void should_remove_file (const char *path, int lineno);

char *
temporary_textual_file (const char *prefix, const char *suffix, int lineno)
{
  char buf[256];
  memset (buf, 0, sizeof (buf));
  char *res = NULL;
  assert (prefix != NULL);
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
      exit (EXIT_FAILURE);
    };
  strcat (buf, suffix);
  res = strdup (buf);
  if (!res)
    {
      fprintf (stderr,
	       "%s failed to strdup temporay file path %s from %s:%d (%m)\n",
	       prog_name, buf, __FILE__, lineno);
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
  for (const char *p = prefix; p && *p; p++)
    assert (isalnum (*p) || *p == '_' || *p == '-');
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
      exit (EXIT_FAILURE);
    };
  printf ("%s temporary binary file is %s [%s:%d]\n",
	  prog_name, res, __FILE__, lineno);
  return res;
}				/* end temporary_binary_file */

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
      exit (EXIT_FAILURE);
    }
  files_to_remove_at_exit[removedfiles_count++] = path;
}				/* end should_remove_file */

void
try_compile_run_hello_world_in_c (const char *cc)
{
  char *helloworldsrc = NULL;
  char helloworldbin[sizeof (helloworldsrc) + 8];
  helloworldsrc = temporary_textual_file ("helloworld_", ".c", __LINE__);
  FILE *hwf = fopen (helloworldsrc, "w");
  if (!hwf)
    {
      fprintf (stderr,
	       "%s failed to create temporary hello world C %s (%m)\n",
	       prog_name, helloworldsrc);
    }
  should_remove_file (helloworldsrc, __LINE__);
  fprintf (hwf, "/// temporary hello world C file %s\n", helloworldsrc);
  fprintf (hwf, "#include <stdio.h>\n");
  fprintf (hwf, "void say_hello(const char*c) {\n");
  fprintf (hwf, " printf(\"hello from %%s in %%s:%%d\\n\",\n");
  fprintf (hwf, "        c, __FILE__, __LINE__);\n");
  fprintf (hwf, "} //end say_hello\n");
  fprintf (hwf, "\n\n");
  fprintf (hwf, "int main(int argc,char**argv) { say_hello(argv[0]); }\n");
  fclose (hwf);
  snprintf (helloworldbin, sizeof (helloworldbin), "./%s", helloworldsrc);
  char *lastdot = strrchr (helloworldbin, '.');
  if (lastdot)
    strcpy (lastdot, ".bin");
  else
    strcat (helloworldbin, ".bin");
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
	exit (EXIT_FAILURE);
      };
    should_remove_file (helloworldbin, __LINE__);
    printf ("popen-eing %s\n", helloworldbin);
    fflush (NULL);
    FILE *pf = popen (helloworldbin, "r");
    if (!pf)
      {
	fprintf (stderr, "%s failed to popen hello world in C %s (%m)\n",
		 prog_name, helloworldbin);
	exit (EXIT_FAILURE);
      };
    char hwline[128];
    memset (hwline, 0, sizeof (hwline));
    fgets (hwline, sizeof (hwline), pf);
    if (!strstr (hwline, "hello"))
      {
	fprintf (stderr, "%s bad read from popen %s got:%s\n",
		 prog_name, helloworldbin, hwline);
	exit (EXIT_FAILURE);
      }
    int ehw = pclose (pf);
    if (ehw)
      {
	fprintf (stderr, "%s bad pclose %s (%d)\n",
		 prog_name, helloworldbin, ehw);
	exit (EXIT_FAILURE);
      };
    printf ("%s: tested hello world C compilation and run [%s:%d]\n",
	    prog_name, __FILE__, __LINE__);
  }
}				/* end try_compile_run_hello_world_in_c */


void
try_then_set_c_compiler (const char *cc)
{
  if (cc[0] != '/')
    {
      fprintf (stderr,
	       "%s given non-absolute path for C compiler %s [%s:%d]\n",
	       prog_name, cc, __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    };
  if (access (cc, F_OK | X_OK))
    {
      fprintf (stderr,
	       "%s given non-executable path for C compiler %s [%s:%d]\n",
	       prog_name, cc, __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    }
  try_compile_run_hello_world_in_c (cc);
  c_compiler = cc;
}				/* end try_then_set_c_compiler */

void
test_cxx_compiler (const char *cxx)
{
  /* Generate two temporary simple C++ files, compile both of them in
     two commands, link the object files, and run the temporary
     helloworld executable in C++ */
  char showvectsrc[128];
  char maincxxsrc[128];
  char showvectobj[128];
  char maincxxobj[128];
  memset (showvectsrc, 0, sizeof (showvectsrc));
  memset (maincxxsrc, 0, sizeof (maincxxsrc));
  strcpy (showvectsrc, "tmp_showvectXXXXXXX.cxx");
  /// write the show vector C++ file
  {
    errno = 0;
    assert (sizeof (".cxx") - 1 == 4);
    int svfd = mkostemps (showvectsrc, 4, R_OK | W_OK);
    if (svfd < 0)
      {
	fprintf (stderr,
		 "%s failed to create using mkostemps temporary show vector C++ %s (%m)[%s:%d]\n",
		 prog_name, showvectsrc, __FILE__, __LINE__);
      };
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
    fprintf (svf, "} // eof %s \n", showvectsrc);
    fclose (svf);
    printf ("%s wrote C++ file %s (%s:%d)\n", prog_name, showvectsrc,
	    __FILE__, __LINE__ - 1);
    should_remove_file (showvectsrc, __LINE__);
    fflush (NULL);
  }
  /// compile the C++ show vector file
  strcpy (showvectobj, showvectsrc);
  {
    char compilshowvect[2 * sizeof (showvectsrc) + 128];
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
	exit (EXIT_FAILURE);
      };
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
	     "            << (argc-1) << \" arguments:\" << std::endl\n");
    fprintf (mnf, "  show_str_vect(v);\n");
    fprintf (mnf, "  std::cout << std::flush;\n");
    fprintf (mnf, "  return 0;\n");
    fprintf (mnf, "} // end main\n");
    fprintf (mnf, "/// eof %s\n", maincxxsrc);
    fclose (mnf);
    printf ("%s wrote main C++ file %s (%s:%d)\n", prog_name, maincxxsrc,
	    __FILE__, __LINE__ - 1);
    fflush (NULL);
  }
  /// compile the C++ main file
  strcpy (maincxxobj, maincxxsrc);
  {
    char compilmaincxx[2 * sizeof (maincxxsrc) + 128];
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
	exit (EXIT_FAILURE);
      };
  }
}				/* end test_cxx_compiler */

void
try_then_set_cxx_compiler (const char *cxx)
{
  if (cxx[0] != '/')
    {
      fprintf (stderr,
	       "%s given non-absolute path for C++ compiler %s [%s:%d]\n",
	       prog_name, cxx, __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    };
  if (access (cxx, F_OK | X_OK))
    {
      fprintf (stderr,
	       "%s given non-executable path for C++ compiler %s [%s:%d]\n",
	       prog_name, cxx, __FILE__, __LINE__);
      exit (EXIT_FAILURE);
    }
  test_cxx_compiler (cxx);
}				/* end try_then_set_cxx_compiler */

void
remove_files (void)
{
  for (int i = 0; i < removedfiles_count; i++)
    unlink (files_to_remove_at_exit[i]);
}				/* end remove_files */

int
main (int argc, char **argv)
{
  prog_name = argv[0];
  atexit (remove_files);
  if (argc > MAX_PROG_ARGS)
    {
      fprintf (stderr,
	       "%s (from C file %s) limits MAX_PROG_ARGS to %d\n"
	       "... but %d are given! Edit it and recompile!\n",
	       argv[0], __FILE__, MAX_PROG_ARGS, argc);
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
	      || !strcmp (curarg, "-fPIC"))
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
  try_then_set_c_compiler (cc);
  char *cxx = getenv ("CXX");
  if (!cxx)
    cxx = my_readline ("C++ compiler:");
  try_then_set_cxx_compiler (cxx);
#warning TODO write the refpersys-config.mk file for GNU make
}				/* end main */


/// eof do-configure-refpersys.c

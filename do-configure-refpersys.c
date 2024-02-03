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
#include <stdbool.h>
#include <stdlib.h>
#include <dlfcn.h>

#ifndef WITHOUT_READLINE
#include "readline/readline.h"
#endif

#ifndef MAX_PROG_ARGS
#define MAX_PROG_ARGS 1024
#endif

#ifndef MY_PATH_MAXLEN
#define MY_PATH_MAXLEN 384
#endif

const char *prog_name;

//// working directory
char my_cwd_buf[MY_PATH_MAXLEN];

//// host name
char my_host_name[80];

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

/// emit the configure-refperys.mk file to be included in GNUmakefile 
void emit_configure_refpersys_mk (void);

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
  char *showvectsrc = NULL;
  char *maincxxsrc = NULL;
  char showvectobj[128];
  char maincxxobj[128];
  showvectsrc = temporary_textual_file ("tmp_showvect", ".cxx", __LINE__);
  maincxxsrc = temporary_textual_file ("tmp_maincxx", ".cxx", __LINE__);
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
	exit (EXIT_FAILURE);
      };
  }
  /// run the C++ exe
  should_remove_file (cxxexe, __LINE__);
  printf ("%s testing popen %s [%s:%d]\n", prog_name, cxxexe,
	  __FILE__, __LINE__ - 1);
  fflush (NULL);
  FILE *pf = popen (cxxexe, "r");
  if (!pf)
    {
      fprintf (stderr, "%s failed to popen %s in C++ (%m)\n",
	       prog_name, cxxexe);
      exit (EXIT_FAILURE);
    };
  {
    bool gothello = false;
    do
      {
	char hwline[128];
	memset (hwline, 0, sizeof (hwline));
	if (!fgets (hwline, sizeof (hwline), pf))
	  break;
	if (!strstr (hwline, "hello"))
	  gothello = true;
	{
	  fprintf (stderr, "%s bad read from popen %s got:%s\n",
		   prog_name, cxxexe, hwline);
	  exit (EXIT_FAILURE);
	}
      }
    while (!feof (pf));
    int ehw = pclose (pf);
    if (ehw)
      {
	fprintf (stderr, "%s bad pclose %s (%d)\n", prog_name, cxxexe, ehw);
	exit (EXIT_FAILURE);
      };
    if (!gothello)
      {
	fprintf (stderr, "%s no hello from C++ test popen %s [%s:%d]\n",
		 prog_name, cxxexe, __FILE__, __LINE__ - 1);
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

void
emit_configure_refpersys_mk (void)
{
  const char *tmp_conf
    = temporary_textual_file ("tmp_config_refpersys", ".mk", __LINE__);
  FILE *f = fopen (tmp_conf, "w");
  if (!f)
    {
      fprintf (stderr, "%s failed to fopen %s for config-refpersys.mk (%m)\n",
	       prog_name, tmp_conf);
      exit (EXIT_FAILURE);
    };
  time_t nowt = time (NULL);
  fprintf (f, "# generated config-refpersys.mk for GNU make in refpersys\n");
  fprintf (f, "# DO NOT EDIT but use make config\n");
  fprintf (f, "# generated from %s:%d in %s\n", __FILE__, __LINE__,
	   my_cwd_buf);
  fprintf (f, "# see refpersys.org\n");
  fprintf (f, "# generated %s\n", ctime (&nowt));
  //// emit C compiler
  fprintf (f, "\n\n" "# the C compiler for RefPerSys:\n");
  fprintf (f, "REFPERSYS_CC=%s\n", c_compiler);
  //// emit C++ compiler
  fprintf (f, "\n\n" "# the C++ compiler for RefPerSys:\n");
  fprintf (f, "REFPERSYS_CXX=%s\n", cpp_compiler);
  //// emit preprocessor flags
  fprintf (f, "\n\n"
	   "# the %d preprocessor flags for RefPerSys:\n",
	   preprocessor_argcount);
  fprintf (f, "REFPERSYS_PREPRO_FLAGS=");
  for (int i = 0; i < preprocessor_argcount; i++)
    {
      if (i > 0)
	fputc (' ', f);
      fputs (preprocessor_args[i], f);
    };
  //// emit compiler flags
  fprintf (f, "\n\n"
	   "# the %d compiler flags for RefPerSys:\n", compiler_argcount);
  fprintf (f, "REFPERSYS_COMPILER_FLAGS=");
  for (int i = 0; i < compiler_argcount; i++)
    {
      if (i > 0)
	fputc (' ', f);
      fputs (compiler_args[i], f);
    };
  //// emit linker flags
  fprintf (f, "\n\n"
	   "# the %d linker flags for RefPerSys:\n", linker_argcount);
  fprintf (f, "REFPERSYS_LINKER_FLAGS=");
  for (int i = 0; i < linker_argcount; i++)
    {
      if (i > 0)
	fputc (' ', f);
      fputs (linker_args[i], f);
    };
  fprintf (f, "\n\n### end of generated config-refpersys.mk file\n");
  fflush (f);
  if (!link (tmp_conf, "refpersys-config.mk"))
    {
      fprintf (stderr, "%s failed to link %s to config-refpersys.mk (%m)\n",
	       prog_name, tmp_conf);
      exit (EXIT_FAILURE);
    };
  fclose (f);
  fflush (NULL);
}				/* end emit_configure_refpersys_mk */


int
main (int argc, char **argv)
{
  prog_name = argv[0];
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
  ///emit file config-refpersys.mk to be included by GNU make 
  emit_configure_refpersys_mk ();
  fprintf (stderr,
	   "[%s:%d] perhaps missing code to emit some refpersys-config.h....\n",
	   __FILE__, __LINE__);
  return 0;
#warning TODO perhaps we should emit also a refpersys-config.h file
  /// that hypothetical refpersys-config.h would be included by refpersys.hh
}				/* end main */


/// eof do-configure-refpersys.c

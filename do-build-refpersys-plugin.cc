/// file do-build-refpersys-plugin.cc in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 - 2025 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///      including Basile Starynkevitch
///
/// Purpose: build a plugin for RefPerSys
///
/// Caveat: this do-build-refpersys-plugin program should run quickly
/// and uses GNU make or ninja from ninja-build.org.  It could leak
/// memory.
///
/// invocation: do-build-refpersys-plugin <plugin-c++-source> -o <plugin-shared-object>
/// e.g. do-build-refpersys-plugin plugins_dir/foo.cc -o /tmp/foo.so
/// other program options are:
///   ./do-build-refpersys-plugin --version | -V #give also defaults
///     ./do-build-refpersys-plugin --verbose | -v #verbose execution
///     ./do-build-refpersys-plugin --output=PLUGIN | -o PLUGIN #output generated .so
///     ./do-build-refpersys-plugin --dirobj=OBJ_DIR | -d OBJ_DIR #directory for object files
///     ./do-build-refpersys-plugin --shell=CMD | -S CMD #run shell command
///     ./do-build-refpersys-plugin --plugin-src=DIR_NAME | -s DIR_NAME #plugin source directory
///     ./do-build-refpersys-plugin --help | -h #this help
///     ./do-build-refpersys-plugin --ninja=NINJAFILE | -N NINJAFILE #add to generated ninja-build script
///     ./do-build-refpersys-plugin --symlink=SYMLINK  #add a symlink of plugin
///
///// The C++ plugin sources may contain comments driving the compilation

///
/// Author(s):
///      Basile Starynkevitch <basile@starynkevitch.net>

/// License: GPLv3+ (file COPYING-GPLv3)
///    This software is free software: you can redistribute it and/or modify
///    it under the terms of the GNU General Public License as published by
///    the Free Software Foundation, either version 3 of the License, or
///   (at your option) any later version.
///

#define _GNU_SOURCE 1
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <cassert>
#include <set>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>


/// GDB debugging of C++ see https://www.youtube.com/watch?v=jc_uzRWSrvw



//// www.gnu.org/software/guile/ version 3
#include <libguile.h>
///
#define BP_HEAD_LINES_THRESHOLD 512
#define BP_MAX_OPTIONS 48

/// a macro to ease GDB breakpoint
#define BP_NOP_BREAKPOINT() do { \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop");} \
  while(0)

#pragma message "compiling " __FILE__

#warning perhaps replace pkg-config with "https://github.com/pkgconf/pkgconf"

#warning fix issue #24 on github.com/RefPerSys/RefPerSys

extern "C" {
#include "__timestamp.c"
  const char bp_git_id[]=GIT_ID;
  const char bp_timestamp[]= __TIMESTAMP__;
  char bp_hostname[128];
  const char* bp_progname;
  int bp_argc_prog;
  char**bp_argv_prog;
  const char** bp_env_prog;
  const char* bp_plugin_binary; // generated shared object
  const char* bp_plugin_symlink; // generated symlink to plugin
  std::string bp_srcdir;  // plugin source directory
  void bp_initialize_guile_scheme(void);
#warning bp_srcdir plugin source directory incompletely handled
  // if only once C++ source given set bp_srcdir to its dirname(3)
  std::string bp_objdir; // plugin object directory
  std::vector<std::string> bp_vect_cpp_sources; // vector of C++ sources
  std::string bp_temp_ninja;  // temporary generated file for ninja-build.org
  std::set<std::string> bp_set_objects; // set of object files
  std::map<std::string,int> bp_map_ixobj; // mapping from objects to
  // indexes in
  // bp_vect_cpp_sources
  bool bp_verbose;
  struct option* bp_options_ptr;
};        // end extern "C"

struct option bp_options_arr[BP_MAX_OPTIONS] =
{
  {
    .name= "verbose", // --verbose | -v
    // so GNU make uses --trace
    .has_arg= no_argument,
    .flag= nullptr,
    .val= 'v',
  },
  {
    .name= "version", // --version | -V
    .has_arg= no_argument,
    .flag= nullptr,
    .val= 'V',
  },
  {
    .name= "output",  // --output=PLUGIN | -o PLUGIN
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 'o',
  },
  {
    .name= "input",  // --input=C++-source | -i C++-source
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 'i',
  },
  {
    .name= "dirobj",  // --dirobj=OBJECT_DIR | -d OBJECT_DIR
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 'd',
  },
  {
    .name= "guile", // --guile=GUILE_CODE | -G GUILE_CODE
    .has_arg= required_argument,
    .flag= nullptr,
    .val = 'G',
  },
  {
    .name= "shell", // --shell="command" | -S cmd
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 'S',
  },
  {
    .name= "plugin-source", // --plugin-source="DIR" | -s DIR_NAME
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 's',
  },
  {
    .name= "symlink",  // --symlink=SYMLINK | -L SYMLINK
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 'L',
  },
  {
    .name= nullptr,
    .has_arg= no_argument,
    .flag= nullptr,
    .val= 0
  },
};        // end bp_options_arr


void
bp_version (void)
{
  std::cout << bp_progname << " version " << bp_git_id
            << " built " __DATE__ "@" << __TIME__ << " [refpersys.org]"
            << std::endl
            << " tool source <" << __FILE__ ":" << __LINE__ << ">"
            << std::endl;
  std::cout << "\t top directory " << rps_topdirectory << std::endl;
  std::cout << "\t GNUmakefile " << rps_gnumakefile << std::endl;
  std::cout << "\t timestamp: " << rps_timestamp  <<std::endl;
  std::cout << "\t gnu-make is " << rps_gnu_make
            << "::" << rps_gnu_make_version  << std::endl;
  std::cout << "# run " << bp_progname  <<" --help for details." << std::endl;
  std::cout << "\t\t see refpersys.org and github.com/RefPerSys/RefPerSys" << std::endl;
} // end bp_version




void
bp_usage(void)
{
  std::string bp_spaces(strlen(bp_progname), ' ');
  std::cout << "usage: " << bp_progname
            << " <plugin-source-code> ... -o <plugin-shared-object>" << std::endl;
  std::cout << '\t' << bp_progname << " --version | -V #give also defaults" << std::endl;
  std::cout << '\t' << bp_progname << " --verbose | -v #verbose execution" << std::endl;
  std::cout << '\t' << bp_progname << " --input=CXXSOURCE | -i CXXSOURCE #input C++ file" << std::endl;
  std::cout << '\t' << bp_progname << " --output=PLUGIN | -o PLUGIN #output generated .so" << std::endl;
  std::cout << '\t' << bp_progname << " --symlink=SYMLINK | -L SYMLINK #symlink the generated plugin .so" << std::endl;
  std::cout << '\t' << bp_progname << " --dirobj=OBJ_DIR | -d OBJ_DIR #directory for object files" << std::endl;
  std::cout << '\t' << bp_progname << " --shell=CMD | -S CMD #run shell command" << std::endl;
  std::cout << '\t' << bp_progname << " --plugin-src=DIR_NAME | -s DIR_NAME #plugin source directory" << std::endl;
  std::cout << '\t' << bp_progname << " --guile=GUILE_CODE | -G GUILE_CODE #GUILE code for GNU make" << std::endl;
  std::cout << '\t' << bp_spaces << "if GUILE_CODE starts with a left-paren, space or semicolon, it is passed to the interpreter inside make"
            << std::endl;
  std::cout << '\t' << bp_spaces << "if GUILE_CODE starts with a pipe or exclamation, it is popen-ed"
            << std::endl;
  std::cout << '\t' << bp_spaces << "otherwise GUILE_CODE is a file with GNU Guile Scheme code"
            << std::endl;

  std::cout << '\t' << bp_progname << " --help | -h #this help" << std::endl;
  std::cout << "\t #from " << __FILE__ << ':' << __LINE__ << " git " << bp_git_id << std::endl;
  std::cout << "\t #see refpersys.org and github.com/RefPerSys/RefPerSys" << std::endl;
  std::cout << "\t #uses $RPSPLUGIN_CXXFLAGS and $RPSPLUGIN_LDFLAGS if provided"
            << std::endl;
  std::cout << "\t #the C++ plugin sources may contain comments to drive the compilation" << std::endl;
  std::cout << "\t\t\t //@PKGCONFIG <package-name>   #e.g.  ////@PKGCONFIG sfml-graphics" <<std::endl;
  std::cout << "\t\t\t //@NINJA.<tag> up to //@ENDNINJA.<tag> #e.g. //@NINJA.foo ... //@ENDNINJA.foo copy lines to ninja file" <<std::endl;
  std::cout << "\t\t\t //@OBJECT <object-file>       #eg //@OBJECT /usr/local/lib/libnwcc.o to add a new object file" <<std::endl;
} // end bp_usage



void
bp_add_cplusplus_source(const char*path)
{
  assert(path);
  if (access(path, R_OK))
    {
      std::clog << bp_progname
                << " : failed to access input source " << path
                << " - " << strerror(errno)
                << std::endl;
      exit (EXIT_FAILURE);
    };
  const char* dot = strrchr(path, '.');
  if (!dot)
    {
      std::clog << bp_progname
                << " :  input source " <<  path << " without dot"
                << std::endl;
      exit (EXIT_FAILURE);
    };
  if (strcmp(dot+1, "cc") && strcmp(dot+1, "cxx") && strcmp(dot+1, "cpp")
      && !strcmp(dot+1, "C"))
    {
      std::clog << bp_progname
                << " :  input source " <<  path << " has bad extension "
                << (dot+1) << " expecting cc, cxx, cpp, C"
                << std::endl;
      exit (EXIT_FAILURE);
    }
  char* rsrc = realpath(path, nullptr);
  if (rsrc==path)
    rsrc = strdup(path);
  FILE* filsrc = fopen(rsrc,"r");
  if (!filsrc)
    {
      std::clog << bp_progname
                << " : failed to open input source " << rsrc
                << " - " << strerror(errno)
                << std::endl;
      exit(EXIT_FAILURE);
    };
  bool withspdx = false;
  bool with2slash = false;
  int nblin= 0;
  do
    {
      char linbuf[256];
      memset (linbuf, 0, sizeof(linbuf));
      char* l = fgets(linbuf, sizeof(linbuf)-2, filsrc);
      if (!l)
        break;
      nblin++;
      if (strstr(linbuf, "//"))
        with2slash = true;
      else if (strstr(linbuf, "SPDX-"))
        withspdx = true;
    }
  while (!feof(filsrc) && nblin<BP_HEAD_LINES_THRESHOLD);
  fclose(filsrc);
  if (!with2slash && !withspdx)
    {
      std::clog << bp_progname
                << " : input source " << path << " really " << rsrc
                << " has no C++ // comments or SPDX line in its first "
                << BP_HEAD_LINES_THRESHOLD << " lines" << std::endl;
    };
  bp_vect_cpp_sources.push_back(rsrc);
  int srcix=1+(int)bp_vect_cpp_sources.size();
  if (bp_verbose)
    std::cout << bp_progname << " : input#" << srcix
              << " is " << rsrc << std::endl;
} // end bp_add_cplusplus_source

void
bp_prog_options(int argc, char**argv)
{
  int opt= 0;
  int ix= 0;
  pthread_setname_np(pthread_self(), "bldrpsplug");
  bp_vect_cpp_sources.reserve(argc);
  BP_NOP_BREAKPOINT();
  do
    {
      opt = getopt_long(argc, argv, "Vhvi:s:o:N:S:d:G:L:", bp_options_ptr, &ix);
      BP_NOP_BREAKPOINT();
      if (opt == -1)
        break;
      if (ix >= argc)
        break;
      BP_NOP_BREAKPOINT();
      switch (opt)
        {
        case 'V':       // --version
          bp_version();
          exit(EXIT_SUCCESS);
          break;
        case 'h':     // --help
          bp_usage();
          exit(EXIT_SUCCESS);
          break;
        case 'v':     // --verbose
          bp_verbose= true;
          break;
        case 'd':
        {
          static char dirbuf[1024];
          struct stat dirstat = {};
          if (stat(optarg, &dirstat) == 0)
            {
              if (!S_ISDIR(dirstat.st_mode))
                {
                  std::clog << bp_progname
                            << " : specified object directory " << optarg
                            << " is not a directory." << std::endl;
                  exit(EXIT_FAILURE);
                };
            }
          else      // stat failed, try mkdir
            {
              if (mkdir(optarg, 0700))
                {
                  std::clog << bp_progname
                            << " : failed to mkdir object directory "
                            << optarg
                            << " - " << strerror(errno)
                            << std::endl;
                  exit(EXIT_FAILURE);
                }
            };
          char*dirpath = realpath(optarg, dirbuf);
          if (!dirpath)
            {
              std::clog << bp_progname
                        << " : failed to get real path of object directory "
                        << optarg
                        << " - " << strerror(errno)
                        << std::endl;
              exit(EXIT_FAILURE);
            }
          bp_objdir = dirpath;
        }
        break;
        case 's': // --plugin-src=SRC_DIR
        {
          static char dirbuf[1024];
          struct stat dirstat = {};
          if (stat(optarg, &dirstat) == 0)
            {
              if (!S_ISDIR(dirstat.st_mode))
                {
                  std::clog << bp_progname
                            << " : specified plugin source directory " << optarg
                            << " is not a directory." << std::endl;
                  exit(EXIT_FAILURE);
                };
            }
          else      // stat failed, try mkdir
            {
              if (mkdir(optarg, 0700))
                {
                  std::clog << bp_progname
                            << " : failed to mkdir plugin source directory "
                            << optarg
                            << " - " << strerror(errno)
                            << std::endl;
                  exit(EXIT_FAILURE);
                }
            };
          char*dirpath = realpath(optarg, dirbuf);
          if (!dirpath)
            {
              std::clog << bp_progname
                        << " : failed to get real path of plugin source directory "
                        << optarg
                        << " - " << strerror(errno)
                        << std::endl;
              exit(EXIT_FAILURE);
            }
          bp_srcdir = dirpath;
        }
        break;
        case 'i':   // --input=C++source
          bp_add_cplusplus_source(optarg);
          break;
        case 'o':   // --output=PLUGIN
        {
          char bufbak[384];
          memset (bufbak, 0, sizeof(bufbak));
          bp_plugin_binary = optarg;
          if (!access(optarg, F_OK) && strlen(optarg)<sizeof(bufbak)-2)
            {
              int n = snprintf(bufbak, sizeof(bufbak), "%s~", optarg);
              if (n >= (int)sizeof(bufbak)-2)
                {
                  std::clog << bp_progname
                            << " : too long output "
                            << optarg << std::endl;
                  exit(EXIT_FAILURE);
                };
              if (!rename(optarg, bufbak) && bp_verbose)
                printf("%s renamed old plugin: %s -> %s\n", bp_progname, optarg, bufbak);
            };
        }
        break;
        case 'L':   // --symlink=SYMLINK
        {
          bp_plugin_symlink = optarg;
        }
        break;
        case 'S':   // --shell COMMAND
        {
          printf("%s running shell %s [%s:%d]\n",
                 bp_progname, optarg, __FILE__, __LINE__-1);
          fflush(nullptr);
          int shfail = system(optarg);
          fflush(nullptr);
          if (shfail!=0)
            {
              fprintf(stderr,
                      "%s failed to run shell command %s (%d) [%s:%d]\n",
                      bp_progname, optarg,
                      shfail, __FILE__, __LINE__-2);
              exit(EXIT_FAILURE);
            };
          if (bp_verbose)
            {
              printf("%s did run shell %s [%s:%d]\n",
                     bp_progname, optarg, __FILE__, __LINE__-1);
            };
          fflush(nullptr);
        }
        break;
        case 'G': // --guile GUILECODE
        {
          if (bp_verbose)
            {
              printf("%s is running GUILE Scheme code %s [%s:%d]\n",
                     bp_progname, optarg,  __FILE__, __LINE__-1);
            }
          fflush(nullptr);
          /// https://lists.gnu.org/archive/html/guile-user/2025-05/msg00005.html
          SCM guilexp = scm_c_read_string(optarg);
          SCM resguile = scm_primitive_eval(guilexp);
          fflush(nullptr);
          if (bp_verbose)
            {
              char*strguile = scm_to_utf8_string(resguile);
              printf("%s evaluated GUILE code %s as %s [%s:%d]\n",
                     bp_progname, optarg, strguile,  __FILE__, __LINE__-1);
            };
          fflush(nullptr);
        }
        break;
        } // end switch opt
    }
  while (opt > 0);
  fflush(nullptr);
  BP_NOP_BREAKPOINT();
  char cwdbuf[384];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  const char*cwd = getcwd(cwdbuf, sizeof(cwdbuf)-2);
  BP_NOP_BREAKPOINT();
  if (!cwd) {
    fprintf(stderr,
	    "%s failed to getcwd %s [%s:%d]\n",
	    bp_progname, strerror(errno),
	    __FILE__, __LINE__-2);
    fflush(stderr);
    exit(EXIT_FAILURE);
  };
  if (cwd && bp_verbose) {
    printf("%s running in %s git %15s [%s:%d]\n",
	   bp_progname, cwd, bp_git_id, __FILE__, __LINE__-1);
    fflush(stdout);
    BP_NOP_BREAKPOINT();
  };
  while (optind < argc)
    {
      BP_NOP_BREAKPOINT();
      std::string curarg=argv[optind];
      BP_NOP_BREAKPOINT();
      bp_add_cplusplus_source(curarg.c_str());
      /// dont bother freeing rp....
      optind++;
    };        // end while(optind<argc)
  BP_NOP_BREAKPOINT();
  ////
  if (bp_vect_cpp_sources.empty())
    {
      fprintf(stderr,
              "%s got no plugin C++ source [%s:%d]\n",
              bp_progname, __FILE__, __LINE__-2);
      exit(EXIT_FAILURE);
    };
  // default the source directory to the directory of first C++ plugin source if none given
  if (bp_srcdir.empty())
    {
      //// dirname(3) is modifying the path!
      char srcbuf[PATH_MAX];
      memset(srcbuf, 0, sizeof(srcbuf));
      strncpy(srcbuf, bp_vect_cpp_sources[0].c_str(), sizeof(srcbuf)-1);
      char*dn = dirname(srcbuf);
      char rpbuf[PATH_MAX];
      memset(rpbuf, 0, sizeof(rpbuf));
      char*rp = realpath(dn, rpbuf);
      if (!rp)
        {
          fprintf(stderr,
                  "%s failed to call realpath of %s (%s) [%s:%d]\n",
                  bp_progname, dn, strerror(errno),
                  __FILE__, __LINE__-2);
          exit(EXIT_FAILURE);
        };
      char* duprp = strdup(rp); // not freed
      if (!duprp)
        {
          fprintf(stderr,
                  "%s failed to strdup of %s (%s) [%s:%d]\n",
                  bp_progname, rp, strerror(errno),
                  __FILE__, __LINE__-2);
          exit(EXIT_FAILURE);
        }
      bp_srcdir.assign(duprp);
      BP_NOP_BREAKPOINT();
      if (bp_verbose)
        {
          printf("%s defaulted plugin source directory to %s [%s:%d]\n",
                 bp_progname, bp_srcdir.c_str(), __FILE__, __LINE__-1);
        }
    };
  BP_NOP_BREAKPOINT();
} // end bp_prog_options


/// by convention Scheme primitives for GNU guile are prefixed by bpscm
static SCM
bpscm_false0(void)
{
  //// just in case to use the breakpoint below from GDB
  BP_NOP_BREAKPOINT();
  return SCM_BOOL_F;
} // end bpscm_false0

static SCM
bpscm_false1(void)
{
  //// just in case to use the breakpoint below from GDB
  BP_NOP_BREAKPOINT();
  return SCM_BOOL_F;
} // end bpscm_false1

static SCM
bpscm_git_id(void)
{
  BP_NOP_BREAKPOINT();
  return scm_from_utf8_string(bp_git_id);
} // end bpscm_git_id

static SCM
bpscm_nb_cppsources(void)
{
  return scm_from_int((int)bp_vect_cpp_sources.size());
} // end bpscm_nb_cppsources

static SCM
bpscm_get_cpp_source(SCM rkv)
{
  if (scm_is_integer(rkv))
    {
      int32_t irk = scm_to_int32(rkv);
      int32_t nbcpp = (int32_t)bp_vect_cpp_sources.size();
      if (irk<0)
        irk += nbcpp;
      if (irk>0 && irk<nbcpp)
        return  scm_from_utf8_string (bp_vect_cpp_sources[irk].c_str());
    };
  return  SCM_BOOL_F;
} // end bpscm_get_cpp_source

void
bp_initialize_guile_scheme(void)
{
  scm_c_define_gsubr ("bpscm:false0", /*nbreq:*/0, /*nbopt:*/0, /*nbrest:*/0,
                      (scm_t_subr)bpscm_false0);
  scm_c_define_gsubr ("bpscm:false1", /*nbreq:*/0, /*nbopt:*/0, /*nbrest:*/0,
                      (scm_t_subr)bpscm_false1);
  scm_c_define_gsubr ("bpscm:git_id", /*nbreq:*/0, /*nbopt:*/0, /*nbrest:*/0,
                      (scm_t_subr)bpscm_git_id);
  scm_c_define_gsubr ("bpscm:nb_cppsources", /*nbreq:*/0, /*nbopt:*/0, /*nbrest:*/0,
                      (scm_t_subr)bpscm_nb_cppsources);
  scm_c_define_gsubr ("bpscm:get_cpp_source", /*nbreq:*/1, /*nbopt:*/0, /*nbrest:*/0,
                      (scm_t_subr)bpscm_get_cpp_source);
} // end bp_initialize_guile_scheme

////////////////////////////////////////////////////////////////
int
main(int argc, char**argv, const char**env)
{
  bp_progname = argv[0];
  bp_argc_prog = argc;
  bp_argv_prog = argv;
  bp_env_prog = env;
  bp_options_ptr = bp_options_arr;
  scm_init_guile();
  bp_initialize_guile_scheme();
  BP_NOP_BREAKPOINT();
  std::string bp_first_base;
#warning do-build-refpersys-plugin should be much improved and fixed

  ///TODO to accept secondary source files for the plugin and more
  ///program options and improve GNUmakefile
  memset (bp_hostname, 0, sizeof(bp_hostname));
  gethostname(bp_hostname, sizeof(bp_hostname)-1);
  if (argc<2)
    {
      bp_usage();
      return 0;
    };
  if (argc>1 && !strcmp(argv[1], "--version"))
    {
      bp_version();
      return 0;
    }
  else if (argc>1 && !strcmp(argv[1], "--help"))
    {
      bp_usage();
      return 0;
    }
  else if (argc>1 &&
           (!strcmp(argv[1], "--verbose") || !strcmp(argv[1], "-v")))
    {
      bp_verbose = true;
    };
  bp_prog_options(argc, argv);
  if (chdir(rps_topdirectory))
    {
      std::clog << bp_progname << " failed to chdir " << rps_topdirectory
                << " : " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
    };
  BP_NOP_BREAKPOINT();
  if (bp_verbose)
    {
      char cwdbuf[256];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      const char*cwd = getcwd(cwdbuf, sizeof(cwdbuf)-2);
      std::cout << "Running on " << bp_hostname;
      for (int ix=0; ix<argc; ix++)
        {
          std::cout << ' ' << argv[ix];
        };
      std::cout << " git " << bp_git_id << " in "
                << (cwd?cwd:"./") << std::endl;
    };
  BP_NOP_BREAKPOINT();
  std::set<std::string> bp_base_src_set;
  if (bp_vect_cpp_sources.empty())
    {
      std::clog << bp_progname << " : no C++ source files given" << std::endl;
      exit(EXIT_FAILURE);
    }
  assert(bp_first_base.empty());
  for (std::string cursrc: bp_vect_cpp_sources)
    {
      const char*lastslash = nullptr;
      char buf[128];
      memset (buf, 0, sizeof(buf));
      lastslash = strrchr(cursrc.c_str(), (int) '/');
      if (lastslash)
        {
          sscanf(lastslash+1, "%100[A-Za-z0-9_+-]", buf);
        }
      else
        sscanf(cursrc.c_str(), "%100[A-Za-z0-9_+-]", buf);
      std::string bufstr{buf};
      if (bp_base_src_set.find(bufstr) != bp_base_src_set.end())
        {
          std::clog << bp_progname << " : the base name " << bufstr << " appears more than once, last in C++ file " << cursrc
                    << " [" << __FILE__ << ":" << __LINE__ <<"]" <<std::endl;
          exit(EXIT_FAILURE);
        };
      if (bp_first_base.empty())
        bp_first_base = bufstr;
      bp_base_src_set.insert({bufstr,cursrc});
    };
  /// run the script to build the plugin
  char buildcmd[1024];
  memset (buildcmd, 0, sizeof(buildcmd));
  ///
  /// special case for a single C++ plugin source
  if (bp_vect_cpp_sources.size() == 1)
    {
      const std::string thecppsrc= bp_vect_cpp_sources[0];
      BP_NOP_BREAKPOINT();
      if (bp_verbose)
        {
          std::clog << "#" << bp_progname << " single C++ source is "
                    << thecppsrc
                    << " [" << __FILE__ << ":" << __LINE__ <<"]"
                    << std::endl;
        };
      if (access(thecppsrc.c_str(), R_OK))
        {
          int e=errno;
          std::clog << "#" << bp_progname << " missing single C++ source is "
                    << thecppsrc << " " << strerror(e)
                    << " [" << __FILE__ << ":" << __LINE__ <<"]"
                    << std::endl;
        }
      BP_NOP_BREAKPOINT();
      if (bp_verbose)
        snprintf (buildcmd, sizeof(buildcmd)-1, "%s --trace -C %s "
                  "one-plugin"
                  " REFPERSYS_PLUGIN_SOURCE='%s' REFPERSYS_PLUGIN_SHARED_OBJECT='%s'",
                  rps_gnu_make,
                  rps_topdirectory,
                  thecppsrc.c_str(),
                  bp_plugin_binary);
      else
        snprintf (buildcmd, sizeof(buildcmd)-1, "%s -C %s one-plugin"
                  " REFPERSYS_PLUGIN_SOURCE='%s' REFPERSYS_PLUGIN_SHARED_OBJECT='%s'",
                  rps_gnu_make,
                  rps_topdirectory,
                  thecppsrc.c_str(),
                  bp_plugin_binary);
      // Nota Bene: we assume that both the plugin source and shared
      // object don't contain pathological characters, including
      // quotes, spaces, control-characters or backslashes.
    }
  else        // several C++ plugin sources; in that case the
    // GNUmakefile should know about them
    {
      BP_NOP_BREAKPOINT();
      if (bp_verbose)
        snprintf (buildcmd, sizeof(buildcmd)-1, "%s --trace -C %s %s",
                  rps_gnu_make,
                  rps_topdirectory,
                  bp_plugin_binary);
      else
        snprintf (buildcmd, sizeof(buildcmd)-1, "%s -C %s %s",
                  rps_gnu_make,
                  rps_topdirectory,
                  bp_plugin_binary);
    };
  if (strlen(buildcmd)> sizeof(buildcmd)-5)
    {
      std::clog << bp_progname << " : too wide build command " << buildcmd << std::endl;
      exit(EXIT_FAILURE);
    };
  printf("%s [%s:%d|%.8s] running GNU make in %s as \n  %s"
         "\n (plugin binary %s, %d sources starting with %s)\n",
         bp_progname,
         __FILE__, __LINE__-2, bp_git_id,
         rps_topdirectory,
         buildcmd,  bp_plugin_binary,
         (int)bp_vect_cpp_sources.size(),
         bp_vect_cpp_sources.at(0).c_str());
  fflush (nullptr);
  BP_NOP_BREAKPOINT();
  int ex = system(buildcmd);
  sync ();
  if (ex)
    {
      char cwdbuf[256];
      memset(cwdbuf, 0, sizeof(cwdbuf));
      if (!getcwd(cwdbuf, sizeof(cwdbuf)-3) || cwdbuf[sizeof(cwdbuf)-3])
        strcpy(cwdbuf, "./");
      std::clog << bp_progname << " fail to run " << buildcmd << " in " << cwdbuf
                << " =" << ex << " [" <<__FILE__ << ":" << __LINE__ -2 << "]" << std::endl;
      return ex;
    };
  char *rp = realpath(bp_plugin_binary, NULL);
  if (!access(rp, R_OK) && bp_plugin_symlink)
    {
      char oldsymlink[384];
      memset (oldsymlink, 0, sizeof(oldsymlink));
      ssize_t rlsz = readlink(bp_plugin_symlink, oldsymlink, sizeof(oldsymlink)-1);
      if (rlsz > 0 && (int)rlsz <  (int)sizeof(oldsymlink)-2 && !strcmp(oldsymlink, rp))
        {
          if (bp_verbose)
            printf("%s: symlink %s -> %s already exists\n",
                   bp_progname, bp_plugin_symlink, rp);
          fflush(stdout);
        }
      else
        {
          int syok = 0;
          if (!access(bp_plugin_symlink, R_OK))
            std::clog << bp_progname << " fail to symlink "
                      << bp_plugin_symlink << " -> " << rp
                      << " (existing file)" << std::endl;
          else
            syok = symlink(rp, bp_plugin_symlink);
          if (syok)
            std::clog << bp_progname << " fail to symlink "
                      << bp_plugin_symlink << " -> " << rp
                      << " : " << strerror(errno) << std::endl;
          else
            {
              if (bp_verbose)
                printf("%s: symlinked %s -> %s\n",
                       bp_progname, bp_plugin_symlink, rp);
              fflush(stdout);
            }
        }
    }
  /// temporary files should be removed using at(1) utility in ten minutes
  /// see https://linuxize.com/post/at-command-in-linux/
  if (!bp_temp_ninja.empty())
    {
      char atcmd[80];
      memset (atcmd, 0, sizeof(atcmd));
      strcpy(atcmd, "/bin/at now + 10 minutes");
      FILE *p = popen(atcmd, "w");
      if (!p)
        {
          fprintf(stderr, "%s won't remove later file %s\n",
                  bp_progname, bp_temp_ninja.c_str());
          return 0;
        }
      fprintf (p, "/bin/rm -f '%s'\n", bp_temp_ninja.c_str());
      pclose(p);
      if (bp_verbose)
        {
          printf("%s: will remove ninja temporary script %s in ten minutes thru /bin/at\n",
                 bp_progname, bp_temp_ninja.c_str());
        }
    }
  fflush(nullptr);
  bp_options_ptr = nullptr;
  /// synchronize the disk
  sync();
  return 0;
} // end main



/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && make do-build-refpersys-plugin" ;;
 ** End: ;;
 ****************/

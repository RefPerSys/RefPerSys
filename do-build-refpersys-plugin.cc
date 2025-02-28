/// file do-build-refpersys-plugin.cc in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 - 2025 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///
/// Purpose: build a plugin for RefPerSys
///
/// Caveat: this do-build-refpersys-plugin program should run quickly
/// and uses ninja from ninja-build.org. It could leak memory.
///
/// invocation: do-build-refpersys-plugin <plugin-c++-source> -o <plugin-shared-object>
/// e.g. do-build-refpersys-plugin plugins_dir/foo.cc -o /tmp/foo.so
/// other program options are:
/// 	./do-build-refpersys-plugin --version | -V #give also defaults
///     ./do-build-refpersys-plugin --verbose | -v #verbose execution
///     ./do-build-refpersys-plugin --output=PLUGIN | -o PLUGIN #output generated .so
///     ./do-build-refpersys-plugin --dirobj=OBJ_DIR | -d OBJ_DIR #directory for object files
///     ./do-build-refpersys-plugin --shell=CMD | -S CMD #run shell command
///     ./do-build-refpersys-plugin --plugin-src=DIRNAME | -s DIRNAME #plugin source directory
///     ./do-build-refpersys-plugin --help | -h #this help
///     ./do-build-refpersys-plugin --ninja=NINJAFILE | -N NINJAFILE #add to generated ninja-build script
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


///
#define BP_HEAD_LINES_THRESHOLD 512
#define BP_MAX_OPTIONS 32

#warning perhaps replace pkg-config with "https://github.com/pkgconf/pkgconf"

extern "C" {
#include "__timestamp.c"
  const char bp_git_id[]=GIT_ID;
  char bp_hostname[128];
  const char* bp_progname;
  int bp_argc_prog;
  char**bp_argv_prog;
  const char** bp_env_prog;
  const char* bp_plugin_binary; // generated shared object
  std::string bp_srcdir;  // plugin source directory
#warning bp_srcdir plugin source directory incompletely handled
  // if only once C++ source given set bp_srcdir to its dirname
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
    .name= "plugin-source", // --plugin-source="DIR" | -s DIRNAME
    .has_arg= required_argument,
    .flag= nullptr,
    .val= 's',
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
  std::cout << '\t' << bp_progname << " --output=PLUGIN | -o PLUGIN #output generated .so" << std::endl;
  std::cout << '\t' << bp_progname << " --dirobj=OBJ_DIR | -d OBJ_DIR #directory for object files" << std::endl;
  std::cout << '\t' << bp_progname << " --shell=CMD | -S CMD #run shell command" << std::endl;
  std::cout << '\t' << bp_progname << " --plugin-src=DIRNAME | -s DIRNAME #plugin source directory" << std::endl;
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
bp_prog_options(int argc, char**argv)
{
  int opt= 0;
  int ix= 0;
  do
    {
      opt = getopt_long(argc, argv, "Vhvs:o:N:S:d:", bp_options_ptr, &ix);
      if (ix >= argc)
        break;
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
                  std::cerr << bp_progname
                            << " : specified object directory " << optarg
                            << " is not a directory." << std::endl;
                  exit(EXIT_FAILURE);
                };
            }
          else      // stat failed, try mkdir
            {
              if (mkdir(optarg, 0700))
                {
                  std::cerr << bp_progname
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
              std::cerr << bp_progname
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
                  std::cerr << bp_progname
                            << " : specified plugin soursr directory " << optarg
                            << " is not a directory." << std::endl;
                  exit(EXIT_FAILURE);
                };
            }
          else      // stat failed, try mkdir
            {
              if (mkdir(optarg, 0700))
                {
                  std::cerr << bp_progname
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
              std::cerr << bp_progname
                        << " : failed to get real path of plugin source directory "
                        << optarg
                        << " - " << strerror(errno)
                        << std::endl;
              exit(EXIT_FAILURE);
            }
          bp_srcdir = dirpath;
        }
        break;
        case 'o':   // --output=PLUGIN
        {
          char bufbak[384];
          memset (bufbak, 0, sizeof(bufbak));
          bp_plugin_binary = optarg;
          if (!access(optarg, F_OK) && strlen(optarg)<sizeof(bufbak)-2)
            {
              snprintf(bufbak, sizeof(bufbak), "%s~", optarg);
              if (!rename(optarg, bufbak) && bp_verbose)
                printf("%s renamed old plugin: %s -> %s\n", bp_progname, optarg, bufbak);
            };
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
#warning GUILECODE not handled
	  }
	  break;
        } // end switch opt
    }
  while (opt > 0 && ix < argc);
  fflush(nullptr);
  asm volatile ("nop; nop; nop; nop");
  while (optind < argc)
    {
      std::string curarg=argv[optind];
      int srcix=1+(int)bp_vect_cpp_sources.size();
      if (access(curarg.c_str(), R_OK))
        {
          fprintf(stderr,
                  "%s failed to access plugin source#%d %s (%s) [%s:%d]\n",
                  bp_progname, srcix, curarg.c_str(),
                  strerror(errno), __FILE__, __LINE__-2);
          exit(EXIT_FAILURE);
        };
      for (char c: curarg)
        {
          if (isspace(c) || !isprint(c))
            {
              fprintf(stderr,
                      "%s bad plugin source#%d has forbidden character %0ux (%s) [%s:%d]\n",
                      bp_progname, srcix, (unsigned)c, curarg.c_str(),
                      __FILE__, __LINE__-2);
              exit(EXIT_FAILURE);
            };
        }
      if (bp_verbose)
        {
          printf("%s adding plugin C++ source file#%d %s\n",
                 bp_progname, srcix, curarg.c_str());
          fflush(nullptr);
        };
      bp_vect_cpp_sources.push_back(curarg);
      optind++;
    };        // end while(optind<argc)
  asm volatile ("nop; nop; nop; nop");
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
      char*dn = dirname(const_cast<char*>(bp_vect_cpp_sources[0].c_str()));
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
      bp_srcdir.assign(rp);
      if (bp_verbose)
        {
          printf("%s defaulted plugin source directory to %s [%s:%d]\n",
                 bp_progname, bp_srcdir.c_str(), __FILE__, __LINE__-1);
        }
    };
} // end bp_prog_options



////////////////////////////////////////////////////////////////
int
main(int argc, char**argv, const char**env)
{
  bp_options_ptr = bp_options_arr;
  std::string bp_first_base;
#warning do-build-refpersys-plugin should be much improved
  ///TODO to accept secondary source files for the plugin and more
  ///program options and improve GNUmakefile
  bp_progname = argv[0];
  bp_argc_prog = argc;
  bp_argv_prog = argv;
  bp_env_prog = env;
  memset (bp_hostname, 0, sizeof(bp_hostname));
  gethostname(bp_hostname, sizeof(bp_hostname)-1);
  char symlkbuf[384];
  memset(symlkbuf, 0, sizeof(symlkbuf));
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
  asm volatile ("nop; nop; nop; nop");
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
      std::cout << " git " << bp_git_id << " in " << (cwd?cwd:"./") << std::endl;
    };
  asm volatile ("nop; nop; nop; nop");
  std::set<std::string> bp_base_src_set;
  if (bp_vect_cpp_sources.empty())
    {
      std::cerr << bp_progname << " : no C++ source files given" << std::endl;
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
          std::cerr << bp_progname << " : the base name " << bufstr << " appears more than once, last in C++ file " << cursrc
                    << " [" << __FILE__ << ":" << __LINE__ <<"]" <<std::endl;
          exit(EXIT_FAILURE);
        };
      if (bp_first_base.empty())
        bp_first_base = bufstr;
      bp_base_src_set.insert({bufstr,cursrc});
    };
  /// run the script to build the plugin
  {
    char buildcmd[384];
    memset (buildcmd, 0, sizeof(buildcmd));
    if (bp_verbose)
      snprintf (buildcmd, sizeof(buildcmd), "%s -v -C %s %s",
                rps_plugin_builder,
                rps_topdirectory,
                bp_plugin_binary);
    else
      snprintf (buildcmd, sizeof(buildcmd), "%s -C %s %s",
                rps_plugin_builder,
                rps_topdirectory,
                bp_plugin_binary);
    printf("%s [%s:%d] running gmake as \n  %s"
           "\n (plugin binary %s, %d sources starting with %s)\n",
           bp_progname,
           __FILE__, __LINE__-2,
           buildcmd,  bp_plugin_binary,
           (int)bp_vect_cpp_sources.size(),
           bp_vect_cpp_sources.at(0).c_str());
    fflush (nullptr);
    int ex = system(buildcmd);
    sync ();
    if (ex)
      return ex;
  }
  /// temporary files should be removed using at(1) utility in ten minutes
  /// see https://linuxize.com/post/at-command-in-linux/
  {
    char atcmd[80];
    memset (atcmd, 0, sizeof(atcmd));
    snprintf(atcmd, sizeof(atcmd), "/bin/at now + 10 minutes");
    FILE *p = popen(atcmd, "w");
    if (!p)
      {
        fprintf(stderr, "%s won't remove later file %s\n",
                bp_progname, bp_temp_ninja.c_str());
        return 0;
      }
    fprintf (p, "/bin/rm -f '%s'\n", bp_temp_ninja.c_str());
    if (symlkbuf[0])
      fprintf(p, "/bin/rm -f '%s'\n", symlkbuf);
    pclose(p);
  }
  if (bp_verbose)
    {
      printf("%s: will remove ninja temporary script %s in ten minutes thru /bin/at\n",
             bp_progname, bp_temp_ninja.c_str());
      if (symlkbuf[0])
        printf("%s: will remove symlink %s in ten minutes thru /bin/at\n",
               bp_progname, symlkbuf);
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
 ** compile-command: "make do-build-refpersys-plugin" ;;
 ** End: ;;
 ****************/

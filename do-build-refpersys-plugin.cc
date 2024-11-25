/// file do-build-refpersys-plugin.cc in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///
/// Purpose: build a plugin for RefPerSys
///
/// Caveat: this do-build-refpersys-plugin program should run quickly
/// and uses ninja from ninja-build.org
///
/// invocation: do-build-refpersys-plugin <plugin-c++-source> -o <plugin-shared-object>
/// e.g. do-build-refpersys-plugin plugins_dir/foo.cc -o /tmp/foo.so
///
/// Author(s):
///      Basile Starynkevitch <basile@starynkevitch.net>

/// License: GPLv3+ (file COPYING-GPLv3)
///    This software is free software: you can redistribute it and/or modify
///    it under the terms of the GNU General Public License as published by
///    the Free Software Foundation, either version 3 of the License, or
///   (at your option) any later version.
///

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <set>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>


///
#define BP_HEAD_LINES_THRESHOLD 512

#warning perhaps replace pkg-config with "https://github.com/pkgconf/pkgconf"

extern "C" {
#include "__timestamp.c"
  const char bp_git_id[]=GIT_ID;
  const char* bp_progname;
  const char** bp_envprog;
  std::vector<std::string> bp_vect_cpp_sources;
  const char* bp_plugin_binary;
  std::string bp_base;
  std::string bp_temp_ninja;  // temporary generated file for ninja-build.org
  std::set<std::string> bp_set_objects;
  bool bp_verbose;
  FILE* bp_ninja_file;
  std::vector<std::string> bp_vect_ninja;
  struct option bp_options[] =
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
      .name= "ninja", // --ninja=NINJA | -N NINJA
      .has_arg= required_argument,
      .flag= nullptr,
      .val = 'N',
    },
    {
      .name= "shell", // --shell="command" | -S cmd
      .has_arg= required_argument,
      .flag= nullptr,
      .val= 'S',
    },
    {
      .name= nullptr,
      .has_arg= no_argument,
      .flag= nullptr,
      .val= 0
    },
  };        // end bp_options
};

void
bp_version (void)
{
  std::cerr << bp_progname << " version " << bp_git_id
            << " built " __DATE__ "@" << __TIME__ << " [refpersys.org]"
            << std::endl
            << " tool source <" << __FILE__ ":" << __LINE__ << ">"
            << std::endl;
  std::cerr << "\t using builder " << rps_ninja_builder << " " << rps_ninja_version << std::endl;
  std::cerr << "\t top directory " << rps_topdirectory << std::endl;
  std::cerr << "\t GNUmakefile " << rps_gnumakefile << std::endl;
  std::cerr << "\t timestamp: " << rps_timestamp  <<std::endl;
  std::cerr << "\t gnu-make is " << rps_gnu_make
            << "::" << rps_gnu_make_version  << std::endl;
  std::cerr << "\t ninja is " << rps_ninja_builder << "::" << rps_ninja_version << std::endl;
  std::cerr << "# run " << bp_progname  <<" --help for details." << std::endl;
  std::cerr << "\t\t see refpersys.org and github.com/RefPerSys/RefPerSys" << std::endl;
} // end bp_version




void
bp_usage(void)
{
  std::cerr << "usage: " << bp_progname
            << " <plugin-source-code> ... -o <plugin-shared-object>" << std::endl;
  std::cerr << '\t' << bp_progname << " --version | -V #give also defaults" << std::endl;
  std::cerr << '\t' << bp_progname << " --verbose | -v #verbose execution" << std::endl;
  std::cerr << '\t' << bp_progname << " --output=PLUGIN | -o PLUGIN #output generated .so" << std::endl;
  std::cerr << '\t' << bp_progname << " --shell=CMD | -S CMD #run shell command" << std::endl;
  std::cerr << '\t' << bp_progname << " --help | -h #this help" << std::endl;
  std::cerr << '\t' << bp_progname << " --ninja=NINJAFILE | -N NINJAFILE #add to generated ninja-build script" << std::endl;
  std::cerr << "\t\t #from " << __FILE__ << ':' << __LINE__ << " git " << bp_git_id << std::endl;
  std::cerr << "\t\t see refpersys.org and github.com/RefPerSys/RefPerSys" << std::endl;
  std::cerr << "\t\t uses $RPSPLUGIN_CXXFLAGS and $RPSPLUGIN_LDFLAGS if provided"
            << std::endl;
} // end bp_usage



void
bp_complete_ninja(FILE*f, const std::string& src)
{
  std::ifstream inp(src);
  int lineno=0;
  do
    {
      char linbuf[256];
      memset (linbuf, 0, sizeof(linbuf));
      inp.getline(linbuf, sizeof(linbuf)-2);
      if (!inp)
        break;
      lineno++;
      /// handle @PKGCONFIG lines, followed by one name of pkg-config managed packages
      char*pk = strstr(linbuf, "@PKGCONFIG");
      if (pk)
        {
          char*n = pk + strlen("@PKGCONFIG");
          char pkgname[64];
          memset(pkgname, 0, sizeof(pkgname));
          if (sscanf(n, " %60[a-zA-Z0-9._+-]", pkgname) >1 && pkgname[0])
            {
              char cmd[100];
              memset(cmd, 0, sizeof(cmd));
              char inpbuf[384];
              memset(inpbuf, 0, sizeof(inpbuf));
              snprintf(cmd, sizeof(cmd), "pkg-config --cflags %s", pkgname);
              FILE*p = popen(cmd, "r");
              if (!p)
                {
                  std::cerr << bp_progname << " : failed to run "
                            << cmd
                            << " ["<< src << ":" << lineno << "]" << std::endl;
                  exit(EXIT_FAILURE);
                };
              if (!fgets(inpbuf, sizeof(inpbuf)-2, p))
                {
                  std::cerr << bp_progname << " : failed to get line ("
                            << strerror(errno)
                            << ") ["<< src << ":" << lineno << "]" << std::endl;
                  exit(EXIT_FAILURE);
                }
              fprintf(f, "# for package %s [%s:%d]\n", pkgname,
                      __FILE__, __LINE__-1);
              fprintf(f, "cflags = $cflags %s\n", inpbuf);
              if (pclose(p))
                {
                  std::cerr << bp_progname << " : failed to pclose "
                            << cmd
                            << " ["<< src << ":" << lineno << "]" << std::endl;
                  exit(EXIT_FAILURE);
                }
              p = nullptr;
              snprintf(cmd, sizeof(cmd), "pkg-config --libs %s", pkgname);
              p = popen(cmd, "r");
              if (!p)
                {
                  std::cerr << bp_progname << " : failed to run "
                            << cmd
                            << " ["<< src << ":" << lineno << "]" << std::endl;
                  exit(EXIT_FAILURE);
                };
              if (!fgets(inpbuf, sizeof(inpbuf)-2, p))
                {
                  std::cerr << bp_progname << " : failed to get line from command "
                            << cmd << " (" << strerror(errno) << ")"
                            << " in "<< src << ":" << lineno << std::endl;
                  exit(EXIT_FAILURE);
                }
              fprintf(f, "# for package %s [%s:%d]\n", pkgname,
                      __FILE__, __LINE__-1);
              fprintf(f, "ldflags = $ldflags %s\n", inpbuf);
              if (pclose(p))
                {
                  std::cerr << bp_progname << " : failed to pclose "
                            << cmd
                            << " ["<< src << ":" << lineno << "]" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
          continue;
        }
      /// handle @NINJA lines, followed by one name, then insert all
      /// the lines up to @ENDNINJA in the generated ninja file...
      /**
       * for example a line
       *
       *  ///@NINJA.foo
       *
       *  up to the line
       *
       *  //-   @ENDNINJA.foo
       *
       **/
      char*nj = strstr(linbuf, "@NINJA");
      if (nj)
        {
          char name[64];
          memset (name, 0, sizeof (name));
          char*n = nj + strlen("@NINJA");
          if (sscanf(n, ".%60[a-zA-z0-9_]", name) >0 && name[0])
            {
              char endline[80];
              memset (endline, 0, sizeof(endline));
              snprintf(endline, sizeof(endline), "@ENDNINJA.%s", name);
              fprintf(f, "///@NINJA.%s at %s:%d [%s:%d]\n",
                      name, src.c_str(), lineno, __FILE__, __LINE__-1);
              while (inp)
                {
                  memset (linbuf, 0, sizeof(linbuf));
                  inp.getline(linbuf, sizeof(linbuf)-2);
                  if (!inp)
                    break;
                  lineno++;
                  if (strstr(linbuf, endline))
                    break;
                  fputs(linbuf, f);
                };
              fprintf(f, "///@ENDNINJA.%s at %s:%d\n",
                      name, src.c_str(), lineno);
            }
          else
            {
              std::cerr << bp_progname << " : bad @NINJA "
                        << " ["<< src << ":" << lineno << "]" << std::endl;
              exit(EXIT_FAILURE);
            }
          continue;
        };
      char*ob = strstr(linbuf, "@OBJECT");
      if (ob)
        {
          char objpath[256];
          memset (objpath, 0, sizeof(objpath));
          if (sscanf(ob+strlen("@OBJECT"), " %200[a-zA-Z0-9./_+-]",
                     objpath) >0
              && objpath[0])
            {
              std::string objpstr{objpath};
              bp_set_objects.insert(objpstr);
            }

        }

    }
  while (inp);
  fprintf(f, "\n\n##/ %d objects from [%s:%d]\n", (int)bp_set_objects.size(),
          __FILE__, __LINE__-1);
#warning incomplete function bp_complete_ninja
  /* TODO: improve this thng to maintain a set of source files and
     generate a better ninja file */
  for (std::string ob: bp_set_objects)
    {
      std::string src = ob;
      assert(src.size()>=3);
      src.pop_back();
      src.pop_back();
      src.append(".cc");
      fprintf(f, "\n"
              "build %s : R_CXX %s\n", ob.c_str(), src.c_str());
      fprintf(f, "  base_in=%s\n", basename(src.c_str()));
      fprintf(f, "  base_out=%s\n", basename(ob.c_str()));
    }
  fprintf(f, "\n\n##/ final from [%s:%d]\n", __FILE__, __LINE__);
  fprintf(f, "build %s : R_LINKSHARED",
          bp_plugin_binary);
  for (std::string ob: bp_set_objects)
    fprintf(f, " %s", ob.c_str());
  fputc('\n', f);
  fflush (f);
} // end bp_complete_ninja


void
bp_write_prologue_ninja(const char*njpath)
{
  fprintf(bp_ninja_file, "# generated ninja file %s for the ninja-build.org tool\n", njpath);
  fprintf(bp_ninja_file, "# for the refpersys.org project\n");
  fprintf(bp_ninja_file, "# generator <%s:%d> git %s\n",
          __FILE__,  __LINE__-1, bp_git_id);
  fprintf(bp_ninja_file, "# %d refpersys C++ source plugin files\n",
          (int) bp_vect_cpp_sources.size());
  fprintf(bp_ninja_file, "# refpersys generated plugin %s\n",
          bp_plugin_binary);
  fprintf(bp_ninja_file, "ninja_required_version = 1.10\n");
  fflush(bp_ninja_file);
  fprintf(bp_ninja_file, "refpersys_plugin_sources =");
  for (std::string& src: bp_vect_cpp_sources)
    fprintf(bp_ninja_file, " %s", src.c_str());
  fputc('\n', bp_ninja_file);
  fprintf(bp_ninja_file, "refpersys_plugin_binary = %s\n", bp_plugin_binary);
  fprintf(bp_ninja_file, "cplusplus_sources = $refpersys_plugin_sources\n");
  char objbuf[128];
  memset (objbuf, 0, sizeof(objbuf));
  const char* lastdot = strrchr(bp_plugin_binary, '.');
  if (lastdot)
    {
      int l= (int)(lastdot - bp_plugin_binary);
      int i=0;
      for (i=0; i<(int)sizeof(objbuf)-4 && i<l ; i++)
        objbuf[i] = bp_plugin_binary[i];
      objbuf[i++] = '.';
      objbuf[i++] = 'o';
      fprintf(bp_ninja_file, "object_files =\n");
      bp_set_objects.insert(std::string(objbuf));
    }
  fprintf(bp_ninja_file, "#from %s:%d\n", __FILE__, __LINE__);
  fprintf(bp_ninja_file, "cxx = %s\n", rps_cxx_compiler_realpath);
  fprintf(bp_ninja_file, "cxxflags = -Wall -Wextra -I%s",
          rps_topdirectory);
  {
    const char*envcxx = getenv("RPSPLUGIN_CXXFLAGS");
    if (envcxx)
      fprintf(bp_ninja_file, " %s", envcxx);
  }
  fprintf(bp_ninja_file, " $rps_cxx_compiler_flags\n");
  fprintf(bp_ninja_file, "ldflags = -rdynamic -L/usr/local/lib");
  {
    const char*envld = getenv("RPSPLUGIN_LDFLAGS");
    if (envld)
      fprintf(bp_ninja_file, " %s", envld);
  }
  fprintf(bp_ninja_file, "\n\n"
          "rule R_CXX\n"
          "  deps = gcc\n"
          "  depfile = Make-dependencies/__$base_in.mkd\n"
          "  command = $cxx $cxxflags -c $in -MD -MF Make-dependencies/__$base_in.mkd -o $out\n\n");
  fprintf(bp_ninja_file, "\n"
          "rule R_LINKSHARED\n"
          "  command = $cxx -rdynamic -shared $in -o $out\n");
  fprintf(bp_ninja_file, "\n""#end prologue from <%s:%d>\n\n",
          __FILE__, __LINE__-1);
} // end bp_write_prologue_ninja


void
bp_include_ninja(FILE*njf)
{
  char linbuf[512];
  for (std::string&str : bp_vect_ninja)
    {
      fprintf(njf, "##included NINJA file %s\n", str.c_str());
      FILE*inf = fopen(str.c_str(), "r");
      int lincnt=0;
      do
        {
          memset(linbuf, 0, sizeof(linbuf));
          if (!fgets(linbuf, sizeof(linbuf)-1, inf))
            break;
          lincnt++;
          if (fputs(linbuf, njf)==EOF)
            {
              fprintf(stderr,
                      "%s failed to copy line#%d of ninja file %s (%s)\n",
                      bp_progname, lincnt, str.c_str(), strerror(errno));
              exit(EXIT_FAILURE);
            }
        }
      while(!feof(inf));
      fprintf(njf, "##end of included NINJA file %s\n\n", str.c_str());
      fflush(njf);
      if (bp_verbose)
        printf("%s did include NINJA file %s [%s:%d]\n",
               bp_progname, str.c_str(), __FILE__, __LINE__-1);
    }
  fflush(nullptr);
} // end bp_include_ninja

void
bp_prog_options(int argc, char**argv)
{
  int opt= 0;
  int ix= 0;
  do
    {
      opt = getopt_long(argc, argv, "Vhvo:N:S:", bp_options, &ix);
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
        case 'o':   // --output=PLUGIN
        {
          char bufbak[256];
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
        case 'N':   // --ninja NINJA
        {
          int njix = 1+(int)bp_vect_ninja.size();
          if (access(optarg, R_OK))
            {
              fprintf(stderr,
                      "%s failed to access NINJA file #%d %s (%s) [%s:%d]\n",
                      bp_progname, njix, optarg,
                      strerror(errno), __FILE__, __LINE__-2);
              exit(EXIT_FAILURE);
            };
          bp_vect_ninja.push_back(std::string(optarg));
          if (bp_verbose)
            {
              printf("%s: adding NINJA file #%d %s\n",
                     bp_progname, njix, optarg);
              fflush(nullptr);
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
        } // end switch opt
    }
  while (opt > 0 && ix < argc);
  fflush(nullptr);
  asm volatile ("nop; nop");
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
                      "%s bad plugin source#%d %s (%s) [%s:%d]\n",
                      bp_progname, srcix, curarg.c_str(),
                      strerror(errno), __FILE__, __LINE__-2);
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
    };
} // end bp_prog_options

int
main(int argc, char**argv, const char**env)
{
#warning do-build-refpersys-plugin should be much improved
  ///TODO to accept secondary source files for the plugin and more
  ///program options and improve GNUmakefile
  bp_progname = argv[0];
  bp_envprog = env;
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
      bp_base.assign(buf);
    }
  {
    char temp[128];
    snprintf (temp, sizeof(temp), "/tmp/%s_XXXXXX.ninja", bp_base.c_str());
    int fd = mkstemps(temp, strlen(".ninja"));
    bp_temp_ninja.assign(temp);
    errno = 0;
    bp_ninja_file = fdopen(fd, "w");
    if (!bp_ninja_file)
      {
        std::cerr << bp_progname << " cannot open generated ninja file " << temp
                  << " fd#" << fd
                  << " for plugin  " << bp_plugin_binary
                  << " : " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
      }
    bp_write_prologue_ninja(temp);
    if (!bp_vect_ninja.empty())
      bp_include_ninja(bp_ninja_file);
    bp_complete_ninja(bp_ninja_file, bp_plugin_binary);
  }
  fprintf(bp_ninja_file, "\ndefault %s\n", bp_plugin_binary);
  fprintf(bp_ninja_file, "\n###generated by %s:%d git %s\n",
          __FILE__, __LINE__-1, bp_git_id);
  fprintf(bp_ninja_file, "\n#end of generated ninja file %s\n", bp_temp_ninja.c_str());
  fclose(bp_ninja_file);
  fflush(nullptr);
  /// run the ninja command to build the plugin
  {
    char ninjacmd[384];
    memset (ninjacmd, 0, sizeof(ninjacmd));
    if (bp_verbose)
      snprintf (ninjacmd, sizeof(ninjacmd), "%s -v -C %s -f %s %s",
                rps_ninja_builder,
                rps_topdirectory,
                bp_temp_ninja.c_str(),
                bp_plugin_binary);
    else
      snprintf (ninjacmd, sizeof(ninjacmd), "%s -C %s -f %s %s",
                rps_ninja_builder,
                rps_topdirectory,
                bp_temp_ninja.c_str(),
                bp_plugin_binary);
    printf("%s [%s:%d] running\n  %s\n (plugin binary %s)\n", bp_progname,
           __FILE__, __LINE__-1,
           ninjacmd,  bp_plugin_binary);
    fflush (nullptr);
    int ex = system(ninjacmd);
    sync ();
    if (ex)
      return ex;
  }
  /// temporary files should be removed using at utility in ten minutes
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
    fprintf (p, "/bin/rm -f '%s'", bp_temp_ninja.c_str());
    pclose(p);
  }
  if (bp_verbose)
    printf("%s: will remove ninja temporary script %s in ten minutes thru /bin/at\n",
           bp_progname, bp_temp_ninja.c_str());
  fflush(nullptr);
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

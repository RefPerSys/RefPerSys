/// file do-build-plugin.cc in refpersys.org
/// SPDX-License-Identifier: GPL-3.0-or-later
///
/// Description:
///     This file is part of the Reflective Persistent System.

///      © Copyright 2024 The Reflective Persistent System Team
///      team@refpersys.org & http://refpersys.org/
///
/// Purpose: build a plugin for RefPerSys
///
/// Caveat: this program should run quickly and uses ninja.
///
/// invocation: do-build-plugin <plugin-c++-source> <plugin-shared-object>
/// e.g. do-build-plugin plugins_dir/foo.cc /tmp/foo.so
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

extern "C" {
#include "__timestamp.c"
  const char bp_git_id[]=GIT_ID;
  const char* bp_progname;
  const char* bp_plugin_source;
  const char* bp_plugin_binary;
  std::string bp_base;
  std::string bp_temp_ninja;
  FILE* bp_ninja_file;
};


void
bp_version (void)
{
  std::cerr << bp_progname << " version " << bp_git_id
            << " built " __DATE__ "@" << __TIME__ << " [refpersys.org]"
            << std::endl;
  std::cerr << "\t using " << rps_ninja_builder << " " << rps_ninja_version << std::endl;
  std::cerr << "# run " << bp_progname <<" --help for details." << std::endl;
} // end bp_version




void
bp_usage(void)
{
  std::cerr << "usage: " << bp_progname
            << " <plugin-source-code> <plugin-shared-object>" << std::endl;
  std::cerr << bp_progname << " --version" << std::endl;
  std::cerr << bp_progname << " --help" << std::endl;
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
              fgets(inpbuf, sizeof(inpbuf)-2, p);
              fprintf(f, "# for package %s\n", pkgname);
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
              fgets(inpbuf, sizeof(inpbuf)-2, p);
              fprintf(f, "# for package %s\n", pkgname);
              fprintf(f, "ldflags = $ldflags %s\n", inpbuf);
              if (pclose(p))
                {
                  std::cerr << bp_progname << " : failed to pclose "
                            << cmd
                            << " ["<< src << ":" << lineno << "]" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
        }
    }
  while (inp);
} // end bp_complete_ninja




int
main(int argc, char**argv)
{
  bp_progname = argv[0];
  if (argc<2)
    {
      bp_usage();
      return 0;
    };
  if (argc>1 && !strcmp(argv[1], "--version"))
    {
      bp_version();
      return 0;
    };
  if (argc>1 && !strcmp(argv[1], "--help"))
    {
      bp_usage();
      return 0;
    };
  if (argc != 3)
    {
      bp_usage();
      std::cerr << bp_progname << " needs two arguments." << std::endl;
      exit(EXIT_FAILURE);
    };
  bp_plugin_source = (const char*) argv[1];
  bp_plugin_binary = (const char*)argv[2];
  if (access(bp_plugin_source, R_OK))
    {
      std::cerr << bp_progname << " cannot read source file "
                << bp_plugin_source << " : " << strerror(errno) << std::endl;
    }
  {
    const char*lastslash = nullptr;
    char buf[128];
    memset (buf, 0, sizeof(buf));
    lastslash = strrchr(bp_plugin_source, (int) '/');
    if (lastslash)
      {
        sscanf(lastslash+1, "%100[A-Za-z0-9_+-]", buf);
      }
    else
      sscanf(bp_plugin_source, "%100[A-Za-z0-9_+-]", buf);
    bp_base.assign(buf);
  }
  {
    char temp[128];
    snprintf (temp, sizeof(temp), "/tmp/%s_XXXXXX", bp_base.c_str());
    int fd = mkstemp(temp);
    bp_temp_ninja.assign(temp);
    bp_temp_ninja += ".ninja";
    bp_ninja_file = fdopen(fd, "w");
    fprintf(bp_ninja_file, "# generated ninja file %s\n", temp);
    fprintf(bp_ninja_file, "# for refpersys.org\n");
    fprintf(bp_ninja_file, "# generator %s git %s\n", __FILE__, bp_git_id);
    fprintf(bp_ninja_file, "# refpersys source plugin %s\n",
            bp_plugin_source);
    fprintf(bp_ninja_file, "# refpersys generated plugin %s\n",
            bp_plugin_binary);
    fprintf(bp_ninja_file, "ninja_required_version 1.10\n");
    fflush(bp_ninja_file);
    fprintf(bp_ninja_file, "default %s\n", bp_plugin_binary);
    fprintf(bp_ninja_file, "deps = gcc\n");
    fprintf(bp_ninja_file, "cxx = %s\n", rps_cxx_compiler_realpath);
    fprintf(bp_ninja_file, "cflags = -Wall -Wextra -I%s %s\n",
            rps_topdirectory, rps_cxx_compiler_flags);
    fprintf(bp_ninja_file, "ldflags = -rdynamic -L/usr/local/lib\n");
    fprintf(bp_ninja_file, "\n\n"
            "rule CC\n"
            "  depfile = $out.mkd\n"
            "  command = $cxx $cflags -c $in -MD -MF $out.mkd -o $out\n");
    fprintf(bp_ninja_file, "\n"
            "rule LINKSO\n"
            "  command $cxx -rdynamic -shared $in -o $out\n");
    bp_complete_ninja(bp_ninja_file, bp_plugin_source);
  }
  fprintf(bp_ninja_file, "\n#end of file %s\n", bp_base.c_str());
  fclose(bp_ninja_file);
  return 0;
} // end main



/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make do-build-plugin" ;;
 ** End: ;;
 ****************/

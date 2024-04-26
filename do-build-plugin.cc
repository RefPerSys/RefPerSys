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
#include <cassert>
#include <set>
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
  std::set<std::string> bp_set_objects;
  FILE* bp_ninja_file;
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
              fgets(inpbuf, sizeof(inpbuf)-2, p);
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
              fgets(inpbuf, sizeof(inpbuf)-2, p);
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
  for (std::string ob: bp_set_objects)
    {
      std::string src = ob;
      assert(src.size()>=3);
      src.pop_back();
      src.pop_back();
      src.append(".cc");
      fprintf(f, "\n"
              "build %s : CC %s\n", ob.c_str(), src.c_str());
      fprintf (f, "object_files =$object_files %s\n", ob.c_str());
    }
  fprintf(f, "\n\n##/ final from [%s:%d]\n", __FILE__, __LINE__);
  fprintf(f, "build %s : LINKSHARED $object_files\n",
          bp_plugin_binary);
} // end bp_complete_ninja


void
bp_write_prologue_ninja(const char*njpath)
{
  fprintf(bp_ninja_file, "# generated ninja file %s for the ninja-build.org tool\n", njpath);
  fprintf(bp_ninja_file, "# for the refpersys.org project\n");
  fprintf(bp_ninja_file, "# generator <%s:%d> git %s\n",
          __FILE__,  __LINE__-1, bp_git_id);
  fprintf(bp_ninja_file, "# refpersys source plugin %s\n",
          bp_plugin_source);
  fprintf(bp_ninja_file, "# refpersys generated plugin %s\n",
          bp_plugin_binary);
  fprintf(bp_ninja_file, "ninja_required_version = 1.10\n");
  fflush(bp_ninja_file);
  fprintf(bp_ninja_file, "refpersys_plugin_source = %s\n", bp_plugin_source);
  fprintf(bp_ninja_file, "refpersys_plugin_binary = %s\n", bp_plugin_binary);
  fprintf(bp_ninja_file, "cplusplus_sources = $refpersys_plugin_source\n");
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
          "rule LINKSHARED\n"
          "  command = $cxx -rdynamic -shared $in -o $out\n");
  fprintf(bp_ninja_file, "\n""#end prologue from <%s:%d>\n\n",
          __FILE__, __LINE__-1);
} // end bp_write_prologue_ninja

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
      exit(EXIT_FAILURE);
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
    snprintf (temp, sizeof(temp), "/tmp/%s_XXXXXX.ninja", bp_base.c_str());
    int fd = mkstemps(temp, strlen(".ninja"));
    bp_temp_ninja.assign(temp);
    errno = 0;
    bp_ninja_file = fdopen(fd, "w");
    if (!bp_ninja_file)
      {
        std::cerr << bp_progname << " cannot open generated ninja file " << temp
                  << " fd#" << fd
                  << " for plugin source " << bp_plugin_source
                  << " : " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
      }
    bp_write_prologue_ninja(temp);
    bp_complete_ninja(bp_ninja_file, bp_plugin_source);
  }
  fprintf(bp_ninja_file, "\ndefault %s\n", bp_plugin_binary);
  fprintf(bp_ninja_file, "\n#end of generated ninja file %s\n", bp_temp_ninja.c_str());
  fclose(bp_ninja_file);
  /// run the ninja command to build the plugin
  {
    char ninjacmd[256];
    memset (ninjacmd, 0, sizeof(ninjacmd));
    snprintf (ninjacmd, sizeof(ninjacmd), "%s -C %s -f %s %s",
              rps_ninja_builder,
              rps_topdirectory,
              bp_temp_ninja.c_str(),
              bp_plugin_binary);
    printf("%s running\n  %s\n (source %s)\n", bp_progname, ninjacmd,
           bp_plugin_source);
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
  /// synchronize the disk
  sync();
  return 0;
} // end main



/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make do-build-plugin" ;;
 ** End: ;;
 ****************/

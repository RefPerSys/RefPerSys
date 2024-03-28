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
#include <vector>
#include <string>
#include <cstring>
#include <stdlib.h>

extern "C" {
#include "__timestamp.c"
  const char bp_git_id[]=GIT_ID;
  const char* bp_progname;
};


void bp_version (void)
{
  std::cerr << bp_progname << " version " << bp_git_id
	    << " built " __DATE__ "@" << __TIME__ << " [refpersys.org]"
	    << std::endl;
} // end bp_version

void bp_usage(void)
{
  std::cerr << "usage: " << bp_progname
	    << " <plugin-c++-code> <plugin-shared-object>" << std::endl;
  std::cerr << bp_progname << " --version" << std::endl;
  std::cerr << bp_progname << " --help" << std::endl;
}

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
} // end main



/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make do-build-plugin" ;;
 ** End: ;;
 ****************/

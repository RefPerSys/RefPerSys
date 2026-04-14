// file RefPerSys/tools/fox-refpersys.cc
// SPDX-License-Identifier: GPL-3.0-or-later

/***
    © Copyright (C) 2026 by Basile STARYNKEVITCH, France
   program released under GNU General Public License v3+

   This is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   This fox-refpersys program is an opensource FOX toolkit application
   (FOX is a graphical user toolkit for Linux; see
   https://fox-toolkit.org/ ...) It is the interface to the
   RefPerSys inference engine on http://refpersys.org/ and
   communicates with the refpersys process using some JSONRPC2
   protocol on named fifos. In contrast to refpersys itself, the
   fox-refpersys process is short lived.

****/


///// We may want to generate Qt6 temporary C++ code which has to
///// contain the declarations then compile that code into a dlopen-ed
///// plugin....  So we remember the first and last lines of this very
///// C++ source file q6refpersys.cc to be replicated in generated C++
///// code by this utility, to be compiled by it (in temporary C++
///// files) into a temporary C++ plugin.

////////
extern "C" const int foxrps_first_decl_line, foxrps_last_decl_line;
const int foxrps_first_decl_line = __LINE__ -2;

extern "C" const char foxrps_self_file[];

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#define UNUSED __attribute__((unused))
extern "C" const char foxrps_git_id[];
extern "C" const char foxrps_shortgitid[];
extern "C" char foxrps_host_name[];

#ifndef GITID
#error GITID should be defined in compilation command
#endif


#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#define UNUSED __attribute__((unused))
extern "C" const char foxrps_git_id[];
extern "C" const char foxrps_shortgitid[];
extern "C" char foxrps_host_name[];

#ifndef GITID
#error GITID should be defined in compilation command
#endif

//// from generated __timestamp.c
extern "C" const char rps_timestamp[];
extern "C" const unsigned long rps_timelong;
extern "C" const char rps_topdirectory[];
extern "C" const char rps_gitid[];
extern "C" const char rps_qt6moc[];
extern "C" const char rps_shortgitid[];
extern "C" const char rps_gitbranch[];
extern "C" const char rps_lastgittag[];
extern "C" const char rps_lastgitcommit[];
extern "C" const char rps_md5sum[];
extern "C" const char*const rps_files[];
extern "C" const char*const rps_subdirectories[];
extern "C" /// see https://www.gnu.org/software/make/ - a builder tool
extern "C" const char rps_gnumakefile[];
extern "C" const char rps_gnu_make[];
extern "C" const char rps_gnu_make_version[];
extern "C" const char rps_gnu_make_features[];
extern "C" /// see https://www.gnu.org/software/bison/ - a parser generator
extern "C" const char rps_gnu_bison[];
extern "C" const char rps_gnu_bison_version[];
extern "C" /// carburetta.com is a lexer & parser generator
extern "C" /// cf github.com/kingletbv/carburetta
extern "C" const char rps_carburetta[];
extern "C" const char rps_carburetta_version[];
extern "C" const char rps_gui_script_executable[];
extern "C" const char rps_building_user_name[];
extern "C" const char rps_building_user_email[];
extern "C" const char rps_building_host[];
extern "C" const char rps_building_operating_system[];
extern "C" const char rps_building_opersysname[];
extern "C" const char rps_building_machine[];
extern "C" const char rps_building_machname[];
extern "C" const char rps_plugin_builder[];
extern "C" const char rps_cxx_compiler_realpath[];
extern "C" const char rps_cxx_compiler_version[];
// end from __timestamp.c

#include <FXJSON.h>




#define FOXRPS_BREAKPOINT_AT(Fil,Lin) do {    \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"); \
    asm volatile ("__" SELF_BASENAME "_brk_" #Lin ": nop\n");    \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"); \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"); \
 } while(0)

#define FOXRPS_BREAKPOINT_AT_BIS(Fil,Lin) \
  FOXRPS_BREAKPOINT_AT(Fil,Lin)

#define FOXRPS_BREAKPOINT() FOXRPS_BREAKPOINT_AT_BIS(__FILE__,__LINE__)

/// fatal unrecoverable errors
#define FOXRPS_FATALOUT_AT_BIS(Fil,Lin,Out) do {		\
  std::clog <<  "FOXRPS FATAL: " << Out << std::flush		\
      << Fil<<":"<< Lin<< "::"<< __FUNCTION__			\
	      <<  "git:" << foxrps_shortgitid			\
	      << " host " << foxrps_host_name<< std::endl;	\
    abort();							\
  } while(0)

#define FOXRPS_FATALOUT_AT(Fil,Lin,Out) \
  FOXRPS_FATALOUT_AT_BIS(Fil,Lin,Out)

#define FOXRPS_FATALOUT(Out) FOXRPS_FATALOUT_AT(__FILE__,__LINE__,Out)

/// serious warnings
#define FOXRPS_WARNOUT_AT_BIS(Fil,Lin,Out) do {   \
  std::cerr << "FOXRPS WARNING: " << Out << std::flush		\
      << Fil<<":"<< Lin<< "::"<< __FUNCTION__			\
	      <<  "git:" << foxrps_shortgitid			\
	      << " host " << foxrps_host_name<< std::endl;	\
  } while(0)

#define FOXRPS_WARNOUT_AT(Fil,Lin,Out) \
  FOXRPS_WARNOUT_AT_BIS(Fil,Lin,Out)

#define FOXRPS_WARNOUT(Out) FOXRPS_WARNOUT_AT(__FILE__,__LINE__,Out)

#define FOXRPS_DEBUGOUT_AT_BIS(Fil,Lin,Out) do {	\
    if (foxrps_debug)					\
      std::clog << Fil << ":" << Lin			\
		<< "::"<< __FUNCTION__ << " "		\
		<< Out << std::endl;			\
  } while(0)

#define FOXRPS_DEBUGOUT_AT(Fil,Lin,Out) \
  FOXRPS_DEBUGOUT_AT_BIS(Fil,Lin,Out)

#define FOXRPS_DEBUGOUT(Out) FOXRPS_DEBUGOUT_AT(__FILE__,__LINE__,Out)

#error fox-refpersys.cc very incomplete

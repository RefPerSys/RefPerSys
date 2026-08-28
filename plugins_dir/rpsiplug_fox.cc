// file RefPerSys/plugins_dir/rpsiplug_fox.cc
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
***/

#pragma message "compiling " __FILE__ " at " __DATE__ "@" __TIME__

#include "refpersys.hh"

#include "fx.h"

#include "fxver.h"


extern "C" void
rps_do_interactive_plugin(const char*arg)
{
  ///
  if (FOX_MAJOR != fxversion[0] && FOX_MINOR != fxversion[1])
    RPS_FATALOUT("incompatible FOX-toolkit versions" <<std::endl
		 << " fox compiled version:" << FOX_MAJOR << "."
		 << FOX_MINOR << "." << FOX_LEVEL
		 << " linked " << fxversion[0] << "." << fxversion[1]
		 << "." << fxversion[2]);
  RPS_FATALOUT("unimplemented fox rps_do_interactive_plugin arg="
	       << Rps_QuotedC_String(arg)
	       << " fox compiled version:" << FOX_MAJOR << "."
	       << FOX_MINOR << "." << FOX_LEVEL
	       << " linked " << (int)(fxversion[0])
	       << "." << (int)(fxversion[1])
	       << "." << (int)(fxversion[2]));
} // end rps_do_interactive_plugin

#pragma message "done compiling " __FILE__ " at " __DATE__ "@" __TIME__
/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && make plugins_dir/rpsiplug_fox.so" ;;
 ** End: ;;
 ****************/


///////////////// end of file RefPerSys/plugins_dir/rpsiplug_fox.cc

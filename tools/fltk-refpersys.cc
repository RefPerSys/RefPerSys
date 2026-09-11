// file RefPerSys/tools/fltk-refpersys.cc
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

   This fltk-refpersys program is an opensource FLTK toolkit application
   (FLTK is a graphical user toolkit for Linux; see
   https://fltk.org/ ...) It is the interface to the
   RefPerSys inference engine on http://refpersys.org/ and
   communicates with the refpersys process using some JSONRPC2
   protocol on named fifos. In contrast to refpersys itself, the
   fox-refpersys process is short lived.

****/


///// We may want to generate FOX toolkit temporary C++ code which has to
///// contain the declarations then compile that code into a dlopen-ed
///// plugin....  So we remember the first and last lines of this very
///// C++ source file fltk-refpersys.cc to be replicated in generated C++
///// code by this utility, to be compiled by it (in temporary C++
///// files) into a temporary C++ plugin.

////////
extern "C" const int fltkrps_first_decl_line, fltkrps_last_decl_line;
const int fltkrps_first_decl_line = __LINE__ -2;

extern "C" const char fltkrps_self_file[];
extern "C" const char fltkrps_self_basename[];


#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <iostream>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include <assert.h>

//// copied from FLTK texteditor-with-dynamic-colors.cxx

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Editor.H>




extern "C" const char fltkrps_git_id[];
extern "C" const char fltkrps_shortgitid[];
extern "C" char fltkrps_host_name[];
extern "C" int fltkrps_argc;
extern "C" char** fltkrps_argv;
extern "C" bool fltkrps_with_debug;
extern "C" void* fltkrps_dlh;
#ifndef GITID
#error GITID should be defined in compilation command
#endif



//// from generated __buildinfo.c
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




#define FLTKRPS_BREAKPOINT_AT(Fil,Lin) do {    \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"); \
    asm volatile ("_" SELF_BASEID "_brk_" #Lin ": nop; nop\n");   \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"); \
    asm volatile ("nop; nop; nop; nop; nop; nop; nop; nop;\n"); \
 } while(0)

#define FLTKRPS_BREAKPOINT_AT_BIS(Fil,Lin) \
  FLTKRPS_BREAKPOINT_AT(Fil,Lin)

#define FLTKRPS_BREAKPOINT() FLTKRPS_BREAKPOINT_AT_BIS(__FILE__,__LINE__)

/// fatal unrecoverable errors
#define FLTKRPS_FATALOUT_AT_BIS(Fil,Lin,Out) do {        \
  std::clog <<  "FLTKRPS FATAL: " << Out << std::flush   \
      << Fil<<":"<< Lin<< "::"<< __FUNCTION__           \
        <<  "git:" << fltkrps_shortgitid                 \
        << " host " << fltkrps_host_name<< std::endl;    \
  FLTKRPS_BREAKPOINT_AT_BIS(Fil,Lin);                    \
    abort();                                            \
  } while(0)

#define FLTKRPS_FATALOUT_AT(Fil,Lin,Out) \
  FLTKRPS_FATALOUT_AT_BIS(Fil,Lin,Out)

#define FLTKRPS_FATALOUT(Out) FLTKRPS_FATALOUT_AT(__FILE__,__LINE__,Out)

/// serious warnings
#define FLTKRPS_WARNOUT_AT_BIS(Fil,Lin,Out) do {         \
  std::cerr << "FLTKRPS WARNING: " << Out << std::flush  \
      << Fil<<":"<< Lin<< "::"<< __FUNCTION__           \
        <<  "git:" << fltkrps_shortgitid                 \
        << " host " << fltkrps_host_name<< std::endl;    \
  FLTKRPS_BREAKPOINT_AT_BIS(Fil,Lin);                    \
  } while(0)

#define FLTKRPS_WARNOUT_AT(Fil,Lin,Out) \
  FLTKRPS_WARNOUT_AT_BIS(Fil,Lin,Out)

#define FLTKRPS_WARNOUT(Out) FLTKRPS_WARNOUT_AT(__FILE__,__LINE__,Out)

#define FLTKRPS_DEBUGOUT_AT_BIS(Fil,Lin,Out) do {        \
    if (fltkrps_with_debug)                              \
      std::clog << Fil << ":" << Lin                    \
    << "::"<< __FUNCTION__ << " "   \
    << Out << std::endl;      \
    FLTKRPS_BREAKPOINT_AT_BIS(Fil,Lin);      \
  } while(0)

#define FLTKRPS_DEBUGOUT_AT(Fil,Lin,Out) \
  FLTKRPS_DEBUGOUT_AT_BIS(Fil,Lin,Out)

#define FLTKRPS_DEBUGOUT(Out) FLTKRPS_DEBUGOUT_AT(__FILE__,__LINE__,Out)

// Custom class to demonstrate a specialized text editor
class FltkRpsEditor : public Fl_Text_Editor {

    Fl_Text_Buffer *tbuff;      // text buffer
    Fl_Text_Buffer *sbuff;      // style buffer

    // Modify callback handler
    void ModifyCallback(int pos,        // position of update
                        int nInserted,  // number of inserted chars
                        int nDeleted,   // number of deleted chars
                        int,            // number of restyled chars (unused here)
                        const char*) {  // text deleted (unused here)

        // Nothing inserted or deleted?
        if (nInserted == 0 && nDeleted == 0) return;

        // Characters inserted into tbuff?
        //     Insert same number of chars into style buffer..
        //
        if (nInserted > 0) {
            char *style = new char[nInserted + 1];  // temp buffer
            memset(style, 'A', nInserted);          // init style to "A"s
            style[nInserted] = '\0';                // terminate string
            sbuff->insert(pos, style);              // insert "A"s into style buffer
            delete[] style;                         // done with temp buffer..
        }

        // Characters deleted from tbuff?
        //    Delete same number of chars from style buffer..
        //
        if ( nDeleted > 0 ) {
            sbuff->remove(pos, pos + nDeleted);
            return;     // nothing more to do; deleting won't affect our single char coloring
        }

        // Focus on characters inserted
        int start  = pos;
        int end    = pos + nInserted;
        //DEBUG fprintf(stderr, "add_modify_callback(): start/end=%d/%d, text='%.*s'\n", start, end, (end-start), tbuff->address(start));

        // SIMPLE EXAMPLE:
        //     Color the digits 0-4 in green, 5-9 in red.
        //
        for ( int i=start; i<end; i++ ) {
            unsigned int c = tbuff->char_at(i);
            if      ( strchr("01234", c) ) sbuff->replace(i, i+1, "B");   // style 'B' (green)
            else if ( strchr("56789", c) ) sbuff->replace(i, i+1, "C");   // style 'C' (red)
            else                           sbuff->replace(i, i+1, "A");   // style 'A' (black)
        }
    }

    static void ModifyCallback_STATIC(int pos,                 // position of update
                                      int nInserted,           // number of inserted chars
                                      int nDeleted,            // number of deleted chars
                                      int nRestyled,           // number of restyled chars
                                      const char *deletedText, // text deleted
                                      void *cbarg) {           // callback data
        FltkRpsEditor *med = (FltkRpsEditor*)cbarg;
        med->ModifyCallback(pos, nInserted, nDeleted, nRestyled, deletedText);
    }

public:
    FltkRpsEditor(int X,int Y,int W,int H) : Fl_Text_Editor(X,Y,W,H) {
        // Style table for the respective styles
        static const Fl_Text_Editor::Style_Table_Entry stable[] = {
           // FONT COLOR      FONT FACE   FONT SIZE
           // --------------- ----------- --------------
           {  FL_BLACK,       FL_COURIER, 14 }, // A - Black
           {  FL_DARK_GREEN,  FL_COURIER, 14 }, // B - Green
           {  FL_RED,         FL_COURIER, 14 }, // C - Red
        };
        tbuff = new Fl_Text_Buffer();    // text buffer
        sbuff = new Fl_Text_Buffer();    // style buffer
        buffer(tbuff);
        int stable_size = sizeof(stable)/sizeof(stable[0]);
        highlight_data(sbuff, stable, stable_size, 'A', 0, 0);
        tbuff->add_modify_callback(ModifyCallback_STATIC, (void*)this);
    }

    void text(const char* val) {
        tbuff->text(val);
    }
};

int main() {
   Fl_Window *win = new Fl_Window(720, 480, "FLTKRPS Text Editor With Dynamic Coloring");
   FltkRpsEditor  *med = new FltkRpsEditor(10,10,win->w()-20,win->h()-20);
   // Initial text in editor.
   med->text("In this editor, digits 0-4 are shown in green, 5-9 shown in red.\n"
             "So here's some numbers 0123456789.\n"
             "Coloring is handled automatically by the add_modify_callback().\n"
             "\n"
             "You can type here to test. ");
   win->resizable(med);
   win->show();
   return(Fl::run());
}

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd $REFPERSYS_TOPDIR && make fltk-refpersys" ;;
 ** End: ;;
 **
 ****************/

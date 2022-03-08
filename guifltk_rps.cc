/****************************************************************
 * file guifltk_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the initial graphical user interface using FLTK 1.3
 *      (see fltk.org)
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2022 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "refpersys.hh"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Display.H>

extern "C" std::string rps_dumpdir_str;
std::vector<char*> fltk_vector_arg_rps;

bool rps_fltk_gui;

class Fltk_MainWindow_rps;
class Fltk_Editor_rps;
class Fltk_Browser_rps;

// below its menubar, main windows have a tile
class Fltk_MainTile_rps : public Fl_Tile
{
  /* FIXME: we need to have some Fltk_Editor_rps (for the main window
     editor buffer), some text buffer and two Fltk_Browser_rps sharing
     the same main window browser buffer */
  Fltk_MainWindow_rps* mtil_mainwin;
  Fltk_Editor_rps* mtil_editor;
  Fltk_Browser_rps*mtil_top_browser;
  Fltk_Browser_rps*mtil_bottom_browser;
public:
#warning very incomplete Fltk_MainTile_rps
  Fltk_MainTile_rps(Fltk_MainWindow_rps*mainwin, int X, int Y, int W, int H);
  ~Fltk_MainTile_rps();
};				// end Fltk_MainTile_rps

class Fltk_MainWindow_rps : public Fl_Window
{
  static int mainw_count;
  Fl_Menu_Bar mainw_menub; // a menu bar
  Fl_Text_Buffer mainw_editorbuf; // a buffer for edited commands
  Fl_Text_Buffer mainw_browserbuf; // another buffer for browsed output, shared by two browser widgets in the tile
  /* the tile shows one editor, for our mainw_editorbuf and two browsers sharing the same mainw_browserbuf */
  Fltk_MainTile_rps mainw_tile;
  int mainw_rank;
  char mainw_title[80];
  static constexpr int mainw_menuheight = 20;
  static std::set<Fltk_MainWindow_rps*>set_mainw;
public:
  virtual void resize(int X, int Y, int W, int H);
  Fl_Text_Buffer*editor_buffer()
  {
    return &mainw_editorbuf;
  };
  Fl_Text_Buffer*browser_buffer()
  {
    return &mainw_browserbuf;
  };
  Fltk_MainWindow_rps(int X, int Y);
  virtual ~Fltk_MainWindow_rps();
  int rank() const
  {
    return mainw_rank;
  };
};				// end Fltk_MainWindow_rps
int Fltk_MainWindow_rps::mainw_count;
std::set<Fltk_MainWindow_rps*>Fltk_MainWindow_rps::set_mainw;

class Fltk_Editor_rps : public Fl_Text_Editor
{
  Fltk_MainWindow_rps* editor_mainwin;
public:
  Fltk_Editor_rps(Fltk_MainWindow_rps*mainwin, int X,int Y,int W,int H);
  virtual ~Fltk_Editor_rps();
};				// end class Fltk_Editor_rps

class Fltk_Browser_rps : public Fl_Text_Display
{
  Fltk_MainWindow_rps* browser_mainwin;
public:
  Fltk_Browser_rps(Fltk_MainWindow_rps*mainwin, int X,int Y,int W,int H);
  virtual ~Fltk_Browser_rps();
};				// end class Fltk_Browser_rps


static void menub_dumpcbrps(Fl_Widget *w, void *);
static void menub_exitcbrps(Fl_Widget *w, void *);
static void menub_quitcbrps(Fl_Widget *w, void *);
static void menub_copycbrps(Fl_Widget *w, void *);
static void menub_pastecbrps(Fl_Widget *w, void *);
static void menub_quitcbrps(Fl_Widget *w, void *);
static void menub_makewincbrps(Fl_Widget *w, void *);

Fltk_MainWindow_rps::Fltk_MainWindow_rps(int W, int H)
  : Fl_Window(W,H),
    mainw_menub(1,1,W-2, mainw_menuheight),
    mainw_tile(this, 1,mainw_menuheight+1, W, H -mainw_menuheight-1),
    mainw_editorbuf(),  mainw_browserbuf(),
    mainw_rank(0)
{
  memset (mainw_title, 0, sizeof(mainw_title));
  mainw_rank = 1+Fltk_MainWindow_rps::mainw_count++;
  mainw_menub.add("&App/&Dump", "^d", menub_dumpcbrps);
  mainw_menub.add("&App/e&Xit", "^x", menub_exitcbrps);
  mainw_menub.add("&App/&Quit", "^q", menub_quitcbrps);
  mainw_menub.add("&App/&Make Window", "^q", menub_makewincbrps, this);
  mainw_menub.add("&Edit/&Copy", "^c", menub_copycbrps);
  mainw_menub.add("&Edit/&Paste", "^p", menub_pastecbrps);
  set_mainw.insert(this);
  snprintf(mainw_title, sizeof(mainw_title),
           "RefPerSys %s p%d git %s #%d", rps_hostname(), (int)getpid(),
           rps_shortgitid, mainw_rank);
  label (mainw_title);
  RPS_DEBUG_LOG(GUI, "made Fltk_MainWindow_rps @"
                << (void*)this << "#" << mainw_rank
                << " title:" << mainw_title);
} // end Fltk_MainWindow_rps::Fltk_MainWindow_rps


Fltk_MainWindow_rps::~Fltk_MainWindow_rps()
{
  RPS_DEBUG_LOG(GUI, "delete Fltk_MainWindow_rps#" << mainw_rank);
  set_mainw.erase(this);
} // end Fltk_MainWindow_rps::~Fltk_MainWindow_rps

Fltk_MainTile_rps::Fltk_MainTile_rps(Fltk_MainWindow_rps*mainwin,
                                     int X, int Y, int W, int H)
  :  Fl_Tile(X,Y,W,H), mtil_mainwin(mainwin),
     mtil_editor(nullptr),
     mtil_top_browser(nullptr), mtil_bottom_browser(nullptr)
{
  mtil_editor = new  Fltk_Editor_rps(mtil_mainwin,X,Y,W,H/3);
  mtil_top_browser = new Fltk_Browser_rps(mtil_mainwin,X,Y+H/3,W,H/3);
  mtil_bottom_browser =  new Fltk_Browser_rps(mtil_mainwin,X,Y+2*H/3,W,H/3);
#warning should create the Fltk_Editor_rps and the two Fltk_Browser_rps
}; // end Fltk_MainTile_rps::Fltk_MainTile_rps

Fltk_MainTile_rps::~Fltk_MainTile_rps()
{
  delete mtil_editor;
  delete mtil_top_browser;
  delete mtil_bottom_browser;
  mtil_mainwin = nullptr;
  mtil_editor = nullptr;
  mtil_top_browser = nullptr;
  mtil_bottom_browser = nullptr;
};				// end Fltk_Maintile_rps::~Fltk_MainTile_rps()

void
Fltk_MainWindow_rps::resize(int X, int Y, int W, int H)
{
  Fl_Window::resize(X,Y,W,H);
  mainw_menub.resize(1,1,W-2,mainw_menuheight);
  mainw_tile.resize(1,1,W-2,H-mainw_menuheight-1);
  RPS_DEBUG_LOG(GUI, "resize Fltk_MainWindow_rps#" << mainw_rank  << " X="<< X << ", Y="<< Y
                << ", W=" << W << ", H=" << H
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"Fltk_MainWindow_rps::resize"));
} // end Fltk_MainWindow_rps::resize

Fltk_Editor_rps::Fltk_Editor_rps(Fltk_MainWindow_rps*mainwin,int X,int Y,int W,int H)
  : Fl_Text_Editor(X,Y,W,H),  editor_mainwin(mainwin)
{
  assert (mainwin != nullptr);
} // end Fltk_Editor_rps::Fltk_Editor_rps

Fltk_Editor_rps::~Fltk_Editor_rps()
{
  editor_mainwin = nullptr;
} // end Fltk_Editor_rps::~Fltk_Editor_rps


Fltk_Browser_rps::Fltk_Browser_rps(Fltk_MainWindow_rps*mainwin,int X,int Y,int W,int H)
  : Fl_Text_Display(X,Y,W,H),  browser_mainwin(mainwin)
{
  assert (mainwin != nullptr);
} // end Fltk_Browser_rps::Fltk_Browser_rps

Fltk_Browser_rps::~Fltk_Browser_rps()
{
  browser_mainwin = nullptr;
} // end Fltk_Browser_rps::~Fltk_Browser_rps


int fltk_api_version_rps(void)
{
  return Fl::api_version();
}

void add_fltk_arg_rps(char*arg)
{
  if (fltk_vector_arg_rps.empty())
    fltk_vector_arg_rps.push_back((char*)rps_progname);
  RPS_ASSERT(!strncmp(arg,"--fltk",6));
  char*argtail = arg+6;
  char*colon = strchr(argtail, ':');
  if (colon)
    {
      *colon = (char)0;
      fltk_vector_arg_rps.push_back(argtail);
      fltk_vector_arg_rps.push_back(colon+1);
    }
  else
    fltk_vector_arg_rps.push_back(argtail);
} // end add_fltk_arg_rps



// This callback is invoked for dumping
static void
menub_dumpcbrps(Fl_Widget *w, void *)
{
  RPS_DEBUG_LOG(GUI, "menub_dumpcbrps start");
  char cwdbuf[128];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
    RPS_FATAL("getcwd failed %m");
  RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) GUI dump into %s\n"
             "... from current directory %s\n",
             (int)getpid(), rps_hostname(), rps_shortgitid,
             rps_dumpdir_str.c_str(), cwdbuf);
  rps_dump_into(rps_dumpdir_str);
} // end menub_dumpcbrps

// This callback is invoked for exiting after dump
static void
menub_exitcbrps(Fl_Widget *w, void *)
{
  char cwdbuf[128];
  memset(cwdbuf, 0, sizeof(cwdbuf));
  if (!getcwd(cwdbuf, sizeof(cwdbuf)-1))
    RPS_FATAL("getcwd failed %m");
  RPS_DEBUG_LOG(GUI, "menub_exitcbrps incomplete");
  RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) final dump into %s\n"
             "... from current directory %s\n",
             (int)getpid(), rps_hostname(), rps_shortgitid,
             rps_dumpdir_str.c_str(), cwdbuf);
  rps_dump_into(rps_dumpdir_str);
  exit(EXIT_SUCCESS);
#warning menub_dumpcbrps incomplete
} // end menub_exitcbrps

// This callback is invoked for quitting
static void
menub_quitcbrps(Fl_Widget *w, void *)
{
  RPS_DEBUG_LOG(GUI, "menub_quitcbrps incomplete");
  // we probably need a dialog to confirm quitting?
#warning menub_quitcbrps incomplete
  RPS_INFORM("RefPerSys (pid %d on %s shortgit %s) quit",
             (int)getpid(), rps_hostname(), rps_shortgitid);
  exit(EXIT_FAILURE);
} // end menub_quitcbrps

// This callback is invoked for making a new window
static void
menub_makewincbrps(Fl_Widget *w, void *)
{
  Fltk_MainWindow_rps* freshwin = new Fltk_MainWindow_rps(720, 460);
  freshwin->show();
  RPS_DEBUG_LOG(GUI, "menub_makewincbrps made freshwin@" << (void*)freshwin
                << "#" << freshwin->rank());
} // end menub_makewincbrps

// This callback is invoked for copy
static void
menub_copycbrps(Fl_Widget *w, void *)
{
  RPS_DEBUG_LOG(GUI, "menub_copycbrps incomplete");
#warning menub_copycbrps incomplete
} // end menub_copycbrps

// This callback is invoked for quitting
static void
menub_pastecbrps(Fl_Widget *w, void *)
{
  RPS_DEBUG_LOG(GUI, "menub_pastecbrps incomplete");
#warning menub_pastecbrps incomplete
} // end menub_pastecbrps

void
guifltk_initialize_rps(void)
{
  Fl::args(fltk_vector_arg_rps.size(), fltk_vector_arg_rps.data());
  auto mwin = new Fltk_MainWindow_rps(720, 460);
#warning should create some Fltk_Editor_rps
  // ensure the editor follows the size of the mainwin
  int maxw = 3200, maxh = 1300;
  if (maxw > Fl::w())
    maxw = Fl::w()- 40;
  if (maxh > Fl::h())
    maxh = Fl::h() - 40;
  mwin->size_range(330, 220, //: min dim w & h
                   maxw, maxh, //: max dim w & h
                   /*delta w&h:*/ 10, 10);
  mwin->show();
  RPS_DEBUG_LOG(GUI, "guifltk_initialize_rps: mwin@"
                << (void*)mwin << "#" << mwin->rank()
                << RPS_FULL_BACKTRACE_HERE(1, "guifltk_initialize_rps")
                << std::endl);
} // end guifltk_initialize_rps


void
guifltk_run_application_rps (void)
{
  RPS_DEBUG_LOG(GUI, "guifltk_run_application_rps: " << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "guifltk_run_application_rps")
                << std::endl);
  Fl::run();
  RPS_DEBUG_LOG(GUI, "guifltk_run_application_rps ending");
} // end guifltk_run_application_rps

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
#include <FL/names.h>
#include <ctype.h>

extern "C" std::string rps_dumpdir_str;
std::vector<char*> fltk_vector_arg_rps;
extern const char*event_name_fltkrps(int event);

bool rps_fltk_gui;

class Fltk_MainWindow_rps;
class Fltk_Editor_rps;
class Fltk_Browser_rps;
class RpsFltk_EditorSource;

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
  static constexpr double mtil_min_ratio = 0.1;
  static constexpr double mtil_max_ratio = 1.0 - mtil_min_ratio;
  static constexpr double mtil_initial_ratio = 1.0 / 3.0;
  double mtil_ratio_editor; // > mtil_min_ratio and mtil_max_ratio;
  double mtil_ratio_top; //  > mtil_min_ratio and mtil_max_ratio;
public:
#warning very incomplete Fltk_MainTile_rps
  virtual void resize(int X, int Y, int W, int H);
  virtual int handle(int event);
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
  void flush()
  {
    Fl_Window::flush();
  };
  virtual int handle(int event);
};				// end Fltk_MainWindow_rps



int Fltk_MainWindow_rps::mainw_count;
std::set<Fltk_MainWindow_rps*>Fltk_MainWindow_rps::set_mainw;





class Fltk_Editor_rps : public Fl_Text_Editor
{
  Fltk_MainWindow_rps* editor_mainwin;
  friend class RpsFltk_EditorSource;
public:
  Fltk_Editor_rps(Fltk_MainWindow_rps*mainwin, int X,int Y,int W,int H);
  Fltk_MainWindow_rps* mainwin()
  {
    return editor_mainwin;
  };
  int text_editor_handle(int event)
  {
    return Fl_Text_Editor::handle(event);
  };
  //// https://groups.google.com/u/1/g/fltkgeneral/c/61nWL2ryFts
  virtual int handle(int event);
  /* The prettify function is colorizing and syntax-hightlighting the
     editor buffer.  It probably should be called no more than five
     times per second, e.g. with some delay after each key event. */
  void prettify(Rps_CallFrame*caller=nullptr);
  virtual ~Fltk_Editor_rps();
};				// end class Fltk_Editor_rps


class RpsFltk_EditorSource : public Rps_TokenSource
{
  Fltk_Editor_rps* edsrc_editor;
  int edsrc_position;
protected:
  virtual bool get_line();
public:
  RpsFltk_EditorSource(Fltk_Editor_rps*editor);
  virtual ~RpsFltk_EditorSource();
};				// end RpsFltk_EditorSource

class Fltk_Browser_rps : public Fl_Text_Display
{
  Fltk_MainWindow_rps* browser_mainwin;
public:
  Fltk_Browser_rps(Fltk_MainWindow_rps*mainwin, int X,int Y,int W,int H, bool top);
  virtual ~Fltk_Browser_rps();
};				// end class Fltk_Browser_rps




static  int editor_keyfuncbrps(int key, Fl_Text_Editor*ed);
static void menub_refreshcbrps(Fl_Widget *w, void *);
static void menub_dumpcbrps(Fl_Widget *w, void *);
static void menub_exitcbrps(Fl_Widget *w, void *);
static void menub_quitcbrps(Fl_Widget *w, void *);
static void menub_copycbrps(Fl_Widget *w, void *);
static void menub_pastecbrps(Fl_Widget *w, void *);
static void menub_quitcbrps(Fl_Widget *w, void *);
static void menub_makewincbrps(Fl_Widget *w, void *);
static void editorbufmodify_cbrps(int pos, int nInserted, int nDeleted,
                                  int nRestyled, const char* deletedText,
                                  void* cbArg);


Fltk_MainWindow_rps::Fltk_MainWindow_rps(int W, int H)
  : Fl_Window(W,H),
    mainw_menub(1,1,W-2, mainw_menuheight),
    mainw_tile(this, 1,mainw_menuheight+1, W, H -mainw_menuheight-1),
    mainw_editorbuf(),  mainw_browserbuf(),
    mainw_rank(0)
{
  memset (mainw_title, 0, sizeof(mainw_title));
  mainw_rank = 1+Fltk_MainWindow_rps::mainw_count++;
  mainw_menub.add("&App/&Refresh", "^r", menub_refreshcbrps, this);
  mainw_menub.add("&App/&Dump", "^d", menub_dumpcbrps);
  mainw_menub.add("&App/e&Xit", "^x", menub_exitcbrps);
  mainw_menub.add("&App/&Quit", "^q", menub_quitcbrps);
  mainw_menub.add("&App/&Make Window", "^m", menub_makewincbrps, this);
  mainw_menub.add("&Edit/&Copy", "^c", menub_copycbrps);
  mainw_menub.add("&Edit/&Paste", "^p", menub_pastecbrps);
  set_mainw.insert(this);
  mainw_editorbuf.text("//⁑ editor\n");
  mainw_browserbuf.text("▼\n");
  mainw_editorbuf.add_modify_callback(editorbufmodify_cbrps, this);
  snprintf(mainw_title, sizeof(mainw_title),
           "RefPerSys %s p%d git %s #%d", rps_hostname(), (int)getpid(),
           rps_shortgitid, mainw_rank);
  label (mainw_title);
  mainw_tile.show();
  RPS_DEBUG_LOG(GUI, "made Fltk_MainWindow_rps @"
                << (void*)this << "#" << mainw_rank
                << " title:" << mainw_title);
} // end Fltk_MainWindow_rps::Fltk_MainWindow_rps


int
Fltk_MainWindow_rps::handle(int ev)
{
  RPS_DEBUG_LOG(GUI, "Fltk_MainWindow_rps::handle ev=" << ev
                << ":" << event_name_fltkrps(ev) << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "Fltk_MainWindow_rps::handle"));
  int h = Fl_Window::handle(ev);
  usleep (1000);
  RPS_DEBUG_LOG(GUI, "end Fltk_MainWindow_rps::handle ev=" << ev
                << ":" << event_name_fltkrps(ev)
                << " h=" << h << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "end Fltk_MainWindow_rps::handle"));
  return h;
} // end Fltk_MainWindow_rps::handle

Fltk_MainWindow_rps::~Fltk_MainWindow_rps()
{
  RPS_DEBUG_LOG(GUI, "delete Fltk_MainWindow_rps#" << mainw_rank);
  set_mainw.erase(this);
} // end Fltk_MainWindow_rps::~Fltk_MainWindow_rps



Fltk_MainTile_rps::Fltk_MainTile_rps(Fltk_MainWindow_rps*mainwin,
                                     int X, int Y, int W, int H)
  :  Fl_Tile(X,Y,W,H), mtil_mainwin(mainwin),
     mtil_editor(nullptr),
     mtil_top_browser(nullptr), mtil_bottom_browser(nullptr),
     mtil_ratio_editor(mtil_initial_ratio),
     mtil_ratio_top(mtil_initial_ratio)
{
  RPS_DEBUG_LOG(GUI, "made Fltk_MainTile_rps @" << (void*)this
                << " in mainwin#" << mainwin->rank());
  mtil_editor =
    new Fltk_Editor_rps(mtil_mainwin,X,Y,W,H*mtil_ratio_editor);
  mtil_editor->show();
  mtil_top_browser = //
    new Fltk_Browser_rps(mtil_mainwin,X,
                         Y+H*mtil_ratio_editor,W,
                         H*(mtil_ratio_editor + mtil_ratio_top), true);
  mtil_bottom_browser = //
    new Fltk_Browser_rps(mtil_mainwin,
                         X,
                         Y+H*(mtil_ratio_editor+mtil_ratio_top),
                         W,
                         H*(mtil_ratio_editor+mtil_ratio_top),
                         false);
  mtil_top_browser->show();
  RPS_DEBUG_LOG(GUI, "Fltk_MainTile_rps @" << (void*)this
                << " mainwin#" << mainwin->rank()
                << " mtil_editor@" << (void*)mtil_editor
                << " mtil_top_browser@" << (void*)mtil_top_browser
                << " mtil_bottom_browser@" << (void*)mtil_bottom_browser);
  mtil_bottom_browser->show();
  RPS_DEBUG_LOG(GUI, "end Fltk_MainTile_rps @" << (void*)this
                << " mainwin#" << mainwin->rank() << std::endl);
}; // end Fltk_MainTile_rps::Fltk_MainTile_rps

void
Fltk_MainTile_rps::resize(int X, int Y, int W, int H)
{
  RPS_ASSERT(mtil_mainwin != nullptr);
  RPS_DEBUG_LOG(GUI, "Fltk_MainTile_rps @" << (void*)this
                << " mainwin#" << mtil_mainwin->rank()
                << " resize X=" << X
                << ", Y=" << Y
                << ", W=" << W
                << ", H=" << H
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"Fltk_MainTile_rps::resize"));
  Fl_Tile::resize(X, Y, W, H);
} // end Fltk_MainTile_rps::resize



int
Fltk_MainTile_rps::handle(int event)
{
  RPS_DEBUG_LOG(GUI, "Fltk_MainTile_rps::handle @" << (void*)this
                << " mainwin#" << mtil_mainwin->rank()
                << " at <x=" <<  mtil_mainwin->x()
                << ", y=" << mtil_mainwin->y()
                << ", w=" << mtil_mainwin->w()
                << ", h="  << mtil_mainwin->h() << ">"
                << " event#" << event << ":" << event_name_fltkrps(event)
                << " evx=" << Fl::event_x() << ", evy=" << Fl::event_y()
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"Fltk_MainTile_rps::event"));
  int h = Fl_Tile::handle(event);
  RPS_DEBUG_LOG(GUI, "Fltk_MainTile_rps::handle @" << (void*)this
                << " event#" << event << ":" << event_name_fltkrps(event) << " -> h=" << h);
  return h;
} // end Fltk_MainTile_rps::handle



Fltk_MainTile_rps::~Fltk_MainTile_rps()
{
  RPS_DEBUG_LOG(GUI, "destroy Fltk_MainTile_rps @" << (void*)this
                << " in mainwin#" << mtil_mainwin->rank());
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
  RPS_DEBUG_LOG(GUI, "start resize Fltk_MainWindow_rps#" << mainw_rank  << " X="<< X << ", Y="<< Y
                << ", W=" << W << ", H=" << H << ", "
                << ((this->shown())?"shown":"hidden"));
  Fl_Window::resize(X,Y,W,H);
  mainw_menub.resize(1,1,W-2,mainw_menuheight);
  mainw_tile.resize(1,mainw_menuheight,W-2,H-mainw_menuheight-1);
  mainw_menub.show();
  mainw_tile.show();
  RPS_DEBUG_LOG(GUI, "resize Fltk_MainWindow_rps#" << mainw_rank  << " X="<< X << ", Y="<< Y
                << ", W=" << W << ", H=" << H << ", "
                << ((this->shown())?"shown":"hidden")
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1,"Fltk_MainWindow_rps::resize")
                << std::endl);
} // end Fltk_MainWindow_rps::resize

Fltk_Editor_rps::Fltk_Editor_rps(Fltk_MainWindow_rps*mainwin,int X,int Y,int W,int H)
  : Fl_Text_Editor(X,Y,W,H),  editor_mainwin(mainwin)
{
  RPS_ASSERT (mainwin != nullptr);
  buffer (mainwin->editor_buffer());
  RPS_DEBUG_LOG(GUI, "made Fltk_Editor_rps @" << (void*)this
                << " in mainwin#" << mainwin->rank());
  color (fl_lighter(fl_lighter(FL_YELLOW)));
  add_key_binding(/*key:*/0,
                          /*state:*/0,
                          editor_keyfuncbrps);

} // end Fltk_Editor_rps::Fltk_Editor_rps

RpsFltk_EditorSource::RpsFltk_EditorSource(Fltk_Editor_rps*editor)
  : Rps_TokenSource("*editor*"), edsrc_editor(editor), edsrc_position(0)
{
  RPS_ASSERT(editor);
  RPS_ASSERT(editor->mainwin());
  char nambuf[32];
  memset(nambuf, 0, sizeof(nambuf));
  snprintf(nambuf, sizeof(nambuf), "editor#%d", editor->mainwin()->rank());
  set_name(std::string{nambuf});
} // end RpsFltk_EditorSource::RpsFltk_EditorSource

RpsFltk_EditorSource::~RpsFltk_EditorSource()
{
} // end RpsFltk_EditorSource::~RpsFltk_EditorSource

const char*
event_name_fltkrps(int event)
{
  if (event >= 0 && event < sizeof(fl_eventnames)/sizeof(fl_eventnames[0]))
    return fl_eventnames[event];
  static char evnambuf[32];
  memset (evnambuf, 0, sizeof(evnambuf));
  snprintf(evnambuf, sizeof(evnambuf), "FlStrangeEvent%d", event);
  return evnambuf;
} // end event_name_fltkrps

bool
RpsFltk_EditorSource::get_line(void)
{
  RPS_ASSERT(edsrc_editor != nullptr);
  RPS_ASSERT(edsrc_position > 0);
  /// should get the FLTK text buffer...
  RPS_FATAL("unimplemented RpsFltk_EditorSource");
#warning unimplemented RpsFltk_EditorSource::get_line
  /***
   * This function should get line by line from the editor's
   * buffer. So fill toksrc_linebuf and update toksrc_line and
   * toksrc_col
   ***/
  return false;
} // end RpsFltk_EditorSource::get_line

int
Fltk_Editor_rps::handle(int event)
{
  // https://groups.google.com/u/1/g/fltkgeneral/c/61nWL2ryFts
  int h = 0;
  RPS_DEBUG_LOG(GUI, "Fltk_Editor_rps::handle event=" << event
                << ":" <<  event_name_fltkrps(event)
                << RPS_FULL_BACKTRACE_HERE(1,"Fltk_Editor_rps::handle"));
  if (event == FL_KEYUP || event == FL_KEYDOWN)
    {
      const char*ktext = Fl::event_text();
      bool specialkey = false;
      if (Fl::event_key() == FL_Escape)
        {
          ktext = "ESC";
          specialkey = true;
        }
      else if (Fl::event_key() == FL_Tab)
        {
          ktext = "TAB";
          specialkey = true;
        }
      else if (Fl::event_key() == FL_BackSpace)
        {
          ktext = "BACKSPACE";
          specialkey = false;
        }
      else if (Fl::event_key() > FL_F && Fl::event_key() < FL_F+10)
        {
          char kbuf[8];
          memset (kbuf, 0, sizeof(kbuf));
          snprintf(kbuf, sizeof(kbuf), "F%d", Fl::event_key() - FL_F);
          specialkey = true;
          ktext = kbuf;
        }
      if (!specialkey)
        h = text_editor_handle(event);
      char hexk[8];
      memset (hexk, 0, sizeof(hexk));
      snprintf(hexk, sizeof(hexk), "%#x", Fl::event_key());
      RPS_DEBUG_LOG(GUI, "handled keyboard event=" << event
                    << ":" <<  event_name_fltkrps(event)
                    << " text='" << Rps_QuotedC_String(ktext)
                    << "' key=" << Fl::event_key() << "=" << hexk
                    << ((isprint( Fl::event_key()))?"=":"!")
                    << ((isprint( Fl::event_key()))?((char) Fl::event_key()):' ')
                    << " insertpos:" << insert_position()
                    << " line:" << buffer()->count_lines(0, insert_position())
                    << " -> h=" << h
                    << std::endl);
    }
  else
    {

      RPS_DEBUG_LOG(GUI, "Fltk_Editor_rps::handle non-key event=" << event << ":" << event_name_fltkrps(event)
                    <<  std::endl
                    << RPS_FULL_BACKTRACE_HERE(1,"Fltk_Editor_rps::handle"));
    }
  RPS_DEBUG_LOG(GUI, "handled event=" << event
                << ":" <<  event_name_fltkrps(event) << " -> h=" << h
                << std::endl);
  return h;
} // end Fltk_Editor_rps::handle



/* The prettify function is colorizing and syntax-hightlighting the
   editor buffer. */
void
Fltk_Editor_rps::prettify(Rps_CallFrame*caller)
{
  RPS_ASSERT(rps_is_main_thread());
  RPS_LOCALFRAME(nullptr,caller,
                 Rps_LexTokenValue tokval);
  RPS_ASSERT(mainwin());
  RPS_DEBUG_LOG(GUI,
                "Fltk_Editor_rps::prettify start in mainwin#"
                << mainwin()->rank());
#warning incomplete Fltk_Editor_rps::prettify
} // end Fltk_Editor_rps::prettify

// A key binding for our editor. Should handle at least the TAB key for autocompletion.
int
editor_keyfuncbrps(int key, Fl_Text_Editor*txed)
{
  RPS_ASSERT(txed != nullptr);
  Fltk_Editor_rps* ed = dynamic_cast<Fltk_Editor_rps*>(txed);
  RPS_DEBUG_LOG(GUI, "editor_keyfuncbrps key="
                << key << " ed@" << (void*) ed
                << " in mainwin#" << ed->mainwin()->rank() << std::endl
		<< RPS_FULL_BACKTRACE_HERE(1, "editor_keyfuncbrps"));
  // should return 1 to accept the key....
  return 0;
#warning unimplemented and unbound editor_keyfuncbrps
} // end editor_keyfuncbrps


Fltk_Editor_rps::~Fltk_Editor_rps()
{
  RPS_DEBUG_LOG(GUI, "destroy Fltk_Editor_rps @" << (void*)this
                << " in mainwin#" << editor_mainwin->rank());
  editor_mainwin = nullptr;
} // end Fltk_Editor_rps::~Fltk_Editor_rps


Fltk_Browser_rps::Fltk_Browser_rps(Fltk_MainWindow_rps*mainwin,int X,int Y,int W,int H, bool istop)
  : Fl_Text_Display(X,Y,W,H),  browser_mainwin(mainwin)
{
  RPS_ASSERT (mainwin != nullptr);
  RPS_DEBUG_LOG(GUI, "made Fltk_Browser_rps @" << (void*)this << " "
                << (istop?"top":"bottom")
                << " in mainwin#" << mainwin->rank());
  buffer (mainwin->browser_buffer());
  if (istop)
    color (fl_lighter(fl_lighter(fl_lighter(FL_CYAN))));
  else
    color (fl_lighter(fl_lighter(fl_lighter(FL_MAGENTA))));
} // end Fltk_Browser_rps::Fltk_Browser_rps

Fltk_Browser_rps::~Fltk_Browser_rps()
{

  RPS_DEBUG_LOG(GUI, "destroy Fltk_Browser_rps @" << (void*)this
                << " in mainwin#" << browser_mainwin->rank());
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
menub_refreshcbrps(Fl_Widget *w, void *ad)
{
  Fltk_MainWindow_rps* win = reinterpret_cast<Fltk_MainWindow_rps*>(ad);
  RPS_DEBUG_LOG(GUI, "menub_refreshcbrps start win#" << win->rank() << " w@" << (void*)w);
  win->show();
  win->flush();
  RPS_DEBUG_LOG(GUI, "menub_refreshcbrps ending win#" << win->rank());
  RPS_ASSERT(win != NULL && w != NULL);
} // end menub_refreshcbrps

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

static void
editorbufmodify_cbrps(int pos, int nInserted, int nDeleted,
                      int nRestyled, const char* deletedText,
                      void* cbArg)
{
  Fltk_MainWindow_rps*mwin = (Fltk_MainWindow_rps*)cbArg;
  RPS_ASSERT(mwin != nullptr);
  RPS_DEBUG_LOG(GUI, "editormodify_cbrps pos=" << pos
                << " nInserted=" << nInserted
                << " nDeleted=" << nDeleted
                << " nRestyled=" << nRestyled
                << std::endl
                << "... deletedText=" << Rps_QuotedC_String(deletedText)
                << std::endl
                << " mwin#" << mwin->rank()
                << ' ' << ((mwin->shown())?"shown":"hidden")
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "editormodify_cbrps"));
} // end editorbufmodify_cbrps


void
guifltk_run_application_rps (void)
{
  RPS_DEBUG_LOG(GUI, "guifltk_run_application_rps: " << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "guifltk_run_application_rps")
                << std::endl);
  Fl::run();
  RPS_DEBUG_LOG(GUI, "guifltk_run_application_rps ending");
} // end guifltk_run_application_rps

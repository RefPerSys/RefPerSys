/****************************************************************
 * file fltksimpwin_rps.cc
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the low-level FLTK graphical user interface related
 *      code, a single simple window. See http://fltk.org/
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 - 2020 The Reflective Persistent System Team
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

#include "headfltk_rps.hh"



extern "C" const char rps_fltksimpwin_gitid[];
const char rps_fltksimpwin_gitid[]= RPS_GITID;

extern "C" const char rps_fltksimpwin_date[];
const char rps_fltklo_simpwin_date[]= __DATE__;

std::string rps_gui_dump_dir_str;
////////////////////////////////////////////////////////////////
///// to show widgets for debugging
void
RpsGui_ShowWidget::output(std::ostream*pout) const
{
  if (pout == nullptr)
    return;
  if (shown_widget == nullptr)
    {
      *pout << "?nullptr-widget?";
      return;
    }
  else if ((void*)shown_widget == RPS_EMPTYSLOT)
    {
      *pout << "?empty-widget?";
      return;
    }
  else
    {
      unsigned wtype = shown_widget->type();
      switch (wtype)
        {
        case RpsGuiTy_SimpleWindow:
          *pout << "rps-simple-window";
          break;
        case RpsGuiTy_MenuBar:
          *pout << "rps-menubar";
          break;
        case RpsGuiTy_Pack:
          *pout << "rps-pack";
          break;

          ////////////////////////////////

#ifdef FL_NORMAL_BUTTON
        case FL_NORMAL_BUTTON:
          *pout << "normal-button";
          break;
#endif
          //
#ifdef FL_TOGGLE_BUTTON
        case FL_TOGGLE_BUTTON:
          *pout << "toggle-button";
          break;
#endif
          //
#ifdef FL_RADIO_BUTTON
        case FL_RADIO_BUTTON:
          *pout << "radio-button";
          break;
#endif
          //
#ifdef FL_HIDDEN_BUTTON
        case FL_HIDDEN_BUTTON:
          *pout << "hidden-button";
          break;
#endif
          //
#ifdef FL_HOLD_BROWSER
        case FL_HOLD_BROWSER:
          *pout << "hold-browser";
          break;
#endif
          //
#ifdef FL_MULTI_BROWSER
        case FL_MULTI_BROWSER:
          *pout << "multi-browser";
          break;
#endif
          //
#ifdef FL_SELECT_BROWSER
        case FL_SELECT_BROWSER:
          *pout << "select-browser";
          break;
#endif
          //
#ifdef FL_NORMAL_COUNTER
        case FL_NORMAL_COUNTER:
          *pout << "normal-counter";
          break;
#endif
          //
#ifdef FL_SIMPLE_COUNTER
        case FL_SIMPLE_COUNTER:
          *pout << "simple-counter";
          break;
#endif
          //
#ifdef FL_WINDOW
        case FL_WINDOW:
          *pout << "window";
          break;
#endif
          //
#ifdef FL_FILL_DIAL
        case FL_FILL_DIAL:
          *pout << "fill-dial";
          break;
#endif
          //
#ifdef FL_LINE_DIAL
        case FL_LINE_DIAL:
          *pout << "line-dial";
          break;
#endif
          //
#ifdef FL_DOUBLE_DIAL
        case FL_DOUBLE_DIAL:
          *pout << "double-dial";
          break;
#endif
          //
#ifdef FL_FLOAT_INPUT
        case FL_FLOAT_INPUT:
          *pout << "float-input";
          break;
#endif
          //
#ifdef FL_INT_INPUT
        case FL_INT_INPUT:
          *pout << "int-input";
          break;
#endif
          //
#ifdef FL_MULTILINE_INPUT
        case FL_MULTILINE_INPUT:
          *pout << "multiline-input";
          break;
#endif
          //
#ifdef FL_SECRET_INPUT
        case FL_MULTILINE_INPUT:
          *pout << "secret-input";
          break;
#endif
          //
#ifdef FL_NORMAL_OUTPUT
        case FL_NORMAL_OUTPUT:
          *pout << "normal-output";
          break;
#endif
          //
#ifdef FL_TOGGLE_BUTTON
        case FL_TOGGLE_BUTTON:
          *pout << "toggle-button";
          break;
#endif
          //
#ifdef FL_RADIO_BUTTON
        case FL_RADIO_BUTTON:
          *pout << "radio-button";
          break;
#endif
          //
#ifdef FL_HIDDEN_BUTTON
        case FL_HIDDEN_BUTTON:
          *pout << "hidden-button";
          break;
#endif
          //
#ifdef FL_SQUARE_CLOCK
        case FL_SQUARE_CLOCK:
          *pout << "square-clock";
          break;
#endif
          //
#ifdef FL_ROUND_CLOCK
        case FL_ROUND_CLOCK:
          *pout << "round-clock";
          break;
#endif

        /////// default case
        default:
          *pout << "fltk-type#" << wtype;
        };			// end switch wtype
      {
        char adbuf[32];
        memset(adbuf, 0, sizeof(adbuf));
        snprintf(adbuf, sizeof(adbuf), "@%p", (void*)shown_widget);
        *pout << adbuf;
      }
      const char* wlabel = shown_widget->label();
      if (wlabel && wlabel[0])
        *pout << "/" << wlabel << "\\";
    }
} // end RpsGui_ShowWidget::output




////////////////
RpsGui_MenuBar::RpsGui_MenuBar(int X, int Y, int W, int H, const char*lab)
  : Fl_Menu_Bar(X,Y,W,H,lab)
{
  Fl_Widget::type((uchar)RpsGuiTy_MenuBar);
  RPS_DEBUG_LOG(GUI, "RpsGui_MenuBar new this=" << RpsGui_ShowFullWidget(this)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_MenuBar-new"));
};				// end RpsGui_MenuBar::RpsGui_MenuBar



////////////////
RpsGui_MenuBar::~RpsGui_MenuBar()
{
  RPS_DEBUG_LOG(GUI, "RpsGui_MenuBar destr this=" << RpsGui_ShowFullWidget(this)
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_MenuBar-destr"));
};				// end RpsGui_MenuBar::~RpsGui_MenuBar



void
rps_set_gui_dump_dir(const std::string&str)
{
  RPS_DEBUG_LOG(GUI, "rps_set_gui_dump_dir " << str);
  rps_gui_dump_dir_str = str;
} // end rps_set_gui_dump_dir

RpsGui_SimpleWindow::RpsGui_SimpleWindow(int w, int h, const std::string& lab)
  : Fl_Double_Window(w,h),
    guiwin_menubar(nullptr),
    guiwin_label(lab)
{
} // end RpsGui_SimpleWindow::RpsGui_SimpleWindow w,h,lab

RpsGui_SimpleWindow::RpsGui_SimpleWindow(int x, int y, int w, int h, const std::string& lab)
  : Fl_Double_Window(x,y,w,h),
    guiwin_menubar(nullptr),
    guiwin_label(lab)
{
} // end RpsGui_SimpleWindow::RpsGui_SimpleWindow x,y,w,h,lab

RpsGui_SimpleWindow::~RpsGui_SimpleWindow()
{
} // end RpsGui_SimpleWindow::~RpsGui_SimpleWindow

int
RpsGui_SimpleWindow::handle(int evtype)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  /* by default, FLTK understands the ESC key as
     closing. See https://www.fltk.org/doc-1.4/FAQ.html */
  if ((evtype == FL_KEYDOWN || evtype == FL_KEYUP))
    {
      if (Fl::event_key() == FL_Escape)
        {
          RPS_DEBUG_LOG(GUI, "SimpleGuiWindow " << RpsGui_ShowFullWidget<RpsGui_SimpleWindow>(this) << " ignore escape");
          return 1;
        }
    }
  else if (evtype == FL_HIDE)
    {
      RPS_DEBUG_LOG(GUI, "SimpleGuiWindow " << RpsGui_ShowFullWidget<RpsGui_SimpleWindow>(this) << " got hide");
      if (top_window() == this)
        Fl::delete_widget(this);
    }
  else RPS_DEBUG_LOG(GUI, "SimpleGuiWindow " << RpsGui_ShowFullWidget<RpsGui_SimpleWindow>(this) << " evtype#" << evtype);
  return Fl_Double_Window::handle(evtype);
}; // end RpsSimpleGui_Window::handle


void
RpsGui_SimpleWindow::initialize_menubar(void)
{
  int width= w();
  int height= h();
  RPS_DEBUGNL_LOG(GUI, "RpsGui_SimpleWindow::initialize_menubar this:" << RpsGui_ShowWidget(this)
                  << ",  w=" << width << ", h=" << height
                  << std::endl << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_SimpleWindow::initialize_menubar"));
  begin();
  // Notice that coordinates of FLTKwidgets are relative to their
  // containing window, not to parent widget.
  guiwin_menubar =
    new RpsGui_MenuBar(guiwin_border,guiwin_border,width-right_menu_gap-guiwin_border,menu_height-guiwin_border);
  guiwin_menubar->add("&App/Dump",  FL_F+1, RpsGui_SimpleWindow::menu_dump_cb);
  guiwin_menubar->add("&App/e&Xit",  "^x", RpsGui_SimpleWindow::menu_exit_cb);
  guiwin_menubar->add("&App/&Quit",  "^q", RpsGui_SimpleWindow::menu_quit_cb);
  guiwin_menubar->color(fl_rgb_color(255,228,225)); // MistyRose
  guiwin_menubar->box(FL_BORDER_BOX);
  guiwin_menubar->show();
  end(); // putting this before the guiwin_menubar->show() show does not change anything...
  this->show();
  RPS_DEBUG_LOG(GUI, "RpsGui_SimpleWindow::initialize_menubar this:"
                << RpsGui_ShowFullWidget(this) << std::endl
                << "... guiwin_menubar:" << RpsGui_ShowFullWidget(guiwin_menubar) << std::endl
                << "... of parent:" << RpsGui_ShowFullWidget(guiwin_menubar->parent())
                << std::endl);
} // end RpsGui_SimpleWindow::initialize_menubar

void
RpsGui_SimpleWindow::menu_dump_cb(Fl_Widget*widg, void*ptr)
{
  RPS_DEBUG_LOG(GUI, "RpsGui_SimpleWindow::menu_dump_cb widg:" << RpsGui_ShowWidget(widg) << ", ptr@" << ptr
                << " guidumpdir=" << rps_gui_dump_dir_str
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_SimpleWindow::menu_dump_cb"));
  RPS_ASSERT(rps_is_main_gui_thread());
  auto dumpdir = rps_gui_dump_dir_str.empty()?".":rps_gui_dump_dir_str;
  rps_dump_into (dumpdir);
  char cwdbuf[128];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  getcwd(cwdbuf, sizeof(cwdbuf));
  RPS_INFORMOUT("RefPerSys dumped into " << dumpdir << " with current directory being "
                << cwdbuf << ", widg:" << RpsGui_ShowWidget(widg));
} // end RpsGui_SimpleWindow::menu_dump_cb



void
RpsGui_SimpleWindow::menu_exit_cb(Fl_Widget*widg, void*ptr)
{
  RPS_DEBUG_LOG(GUI, "RpsGui_SimpleWindow::menu_exit_cb widg:" << RpsGui_ShowWidget(widg) << " ptr@" << ptr
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_SimpleWindow::menu_exit_cb"));
  RPS_ASSERT(rps_is_main_gui_thread());
  char cwdbuf[128];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  getcwd(cwdbuf, sizeof(cwdbuf));
  auto dumpdir = rps_gui_dump_dir_str.empty()?".":rps_gui_dump_dir_str;
  RPS_INFORMOUT("RefPerSys exiting, will dump into " << dumpdir << " with current directory being " << cwdbuf);
  rps_dump_into (dumpdir);
  RPS_INFORMOUT("RefPerSys dumped before exit into " << dumpdir << " with current directory being " << cwdbuf);
  exit(EXIT_SUCCESS);
} // end RpsGui_SimpleWindow::menu_exit_cb


void
RpsGui_SimpleWindow::menu_quit_cb(Fl_Widget*widg, void*ptr)
{
  RPS_DEBUG_LOG(GUI, "RpsGui_SimpleWindow::menu_quit_cb widg:" << RpsGui_ShowWidget(widg) << ", ptr@" << ptr
                << std::endl
                << RPS_FULL_BACKTRACE_HERE(1, "RpsGui_SimpleWindow::menu_quit_cb"));
  char cwdbuf[128];
  memset (cwdbuf, 0, sizeof(cwdbuf));
  getcwd(cwdbuf, sizeof(cwdbuf));
  auto dumpdir = rps_gui_dump_dir_str.empty()?".":rps_gui_dump_dir_str;
  int quitchoice =
    fl_choice ("Quit without dumping into '%s'\nfrom '%s' ?",
               "cancel",
               "quit",
               "dump",
               dumpdir.c_str(), cwdbuf);
  RPS_DEBUG_LOG(GUI, "RpsGui_SimpleWindow::menu_quit_cb quitchoice=" << quitchoice
                << " dumpdir=" << dumpdir
                << " cwdbuf=" << cwdbuf << std::endl
                << ".. widg:" << RpsGui_ShowWidget(widg));
  switch (quitchoice)
    {
    case 0: // cancel
      return;
    case 1: // quit
      exit(EXIT_SUCCESS);
    case 2: // dump
    {
      RPS_INFORMOUT("RefPerSys quitting, will dump into " << dumpdir
                    << " with current directory being " << cwdbuf);
      rps_dump_into (dumpdir);
      RPS_INFORMOUT("RefPerSys dumped before quitting into " << dumpdir
                    << " with current directory being " << cwdbuf);
      exit(EXIT_SUCCESS);
    }
    break;
    default:
      RPS_FATALOUT("menu_quit_cb: unexpected choice #" << quitchoice);
    };
} // end RpsGui_SimpleWindow::menu_quit_cb



/******* end of file fltksimpwin_rps.cc ******/

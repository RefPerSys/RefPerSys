/****************************************************************
 * file fltkhead_rps.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is the public C++ header file of the FLTK based GUI
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



#ifndef FLTKHEAD_RPS_INCLUDED
#define FLTKHEAD_RPS_INCLUDED

#include "refpersys.hh"

/// for FLTK - see https://www.fltk.org/

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>

extern "C" void rps_fltk_event_loop(Rps_CallFrame*cf);

extern "C" void rps_fltk_stop_event_loop(void);

extern "C" void rps_fltk_initialize(int &argc, char**argv);



enum RpsGui_WinTypes
{
  /// every FLTK widget has a type. Please grep the FLTK header files for "RESERVED_TYPE".
  RpsGuiType_CommandWindow = FL_DOUBLE_WINDOW+2,
  RpsGuiType_OutputWindow,
};

class RpsGui_Window: public Fl_Double_Window
{
  static std::set<RpsGui_Window*> _set_of_gui_windows_;
public:
  virtual int handle(int);
protected:
  Fl_Menu_Bar *guiwin_menubar;
  Rps_Id guiwin_ownoid;
  std::string guiwin_label;
  virtual void initialize_menubar(void) =0;
  RpsGui_Window(int w, int h, const std::string& lab);
  RpsGui_Window(int x, int y, int w, int h, const std::string& lab);
public:
  virtual ~RpsGui_Window();
  Rps_Id owning_oid() const
  {
    return guiwin_ownoid;
  };
  Rps_ObjectRef owning_object(Rps_CallFrame*) const;
  void set_owning_object(Rps_CallFrame*, Rps_ObjectRef obr);
  void clear_owning_object(void);
  const std::string label_str(void) const
  {
    return guiwin_label;
  };
};

class RpsGui_CommandWindow
  : public RpsGui_Window
{
  friend  void rps_fltk_initialize(int &,char**);
  virtual void initialize_menubar(void);
  static void menu_dump_cb(Fl_Widget*, void*);
  static void menu_exit_cb(Fl_Widget*, void*);
  static void menu_quit_cb(Fl_Widget*, void*);
public:
  RpsGui_CommandWindow(int w, int h, const std::string& lab);
  RpsGui_CommandWindow(int x, int y, int w, int h, const std::string& lab);
  virtual ~RpsGui_CommandWindow();
};				// end class RpsGui_CommandWindow

class RpsGui_OutputWindow
  : public RpsGui_Window
{
  friend  void rps_fltk_initialize(int &,char**);
  virtual void initialize_menubar(void);
}; // end class RpsGui_OutputWindow

class RpsGui_OutputText
{
};				// end class RpsGui_OutputText

class RpsGui_InputCommand
{
};				// end class RpsGui_InputCommand

#endif /*FLTKHEAD_RPS_INCLUDED*/
// end of file fltkhead_rps.hh */

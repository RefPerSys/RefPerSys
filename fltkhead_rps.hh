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

enum RpsGui_WinTypes
{
  /// every FLTK widget has a type. Please grep the FLTK header files for "RESERVED_TYPE".
  RpsGuiType_CommandWindow = FL_DOUBLE_WINDOW+2,
  RpsGuiType_OutputWindow,
};

class RpsGui_Window: public Fl_Double_Window
{
public:
  virtual int handle(int);
protected:
  Rps_Id guiwin_ownoid;
  std::string guiwin_label;
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
};

class RpsGui_CommandWindow
  : public RpsGui_Window
{
};				// end class RpsGui_CommandWindow

class RpsGui_OutputWindow
  : public RpsGui_Window
{
}; // end class RpsGui_OutputWindow

class RpsGui_OutputText
{
};				// end class RpsGui_OutputText

class RpsGui_InputCommand
{
};				// end class RpsGui_InputCommand

#endif /*FLTKHEAD_RPS_INCLUDED*/
// end of file fltkhead_rps.hh */

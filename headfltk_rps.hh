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
#include <FL/Fl_Pack.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/fl_ask.H>

//////////////// forward declaration of classes
class RpsGui_ShowWidget;
class RpsGui_Window;
class RpsGui_CommandWindow;
class RpsGui_OutputWindow;
class RpsGui_OutputTextBuffer;
class RpsGui_InputCommand;
////////////////

extern "C" void rps_fltk_event_loop(Rps_CallFrame*cf);

extern "C" void rps_fltk_stop_event_loop(void);

extern "C" void rps_fltk_initialize(int &argc, char**argv);

template<class FltkWidgetClass>
static inline bool
rps_fltk_get_window_geometry(const FltkWidgetClass*widg, int &x, int &y, int &w, int &h, float*scaleptr=nullptr)
{
  if (!widg)
    return false;
  int xoff= -1, yoff= -1;
  Fl_Window* window = widg->top_window_offset(xoff, yoff);
  if (!window || xoff<0 || yoff<0)
    return false;
  int widg_w= widg->w();
  int widg_h= widg->h();
  int wind_screen= window->screen_num();
  if (wind_screen < 0)
    return false;
  float scr_scale = Fl::screen_scale(wind_screen);
  if (scr_scale <= 0.0 || std::isnan(scr_scale))
    return false;
  if (scr_scale != 1.0)
    {
      widg_w = (int)(widg_w*scr_scale);
      widg_h = (int)(widg_h*scr_scale);
    }
  if (scaleptr)
    *scaleptr = scr_scale;
  x= xoff;
  y= yoff;
  w= widg_w;
  h= widg_h;
  return true;
} // end rps_fltk_get_window_geometry<FltkWidgetClass>


template<class FltkWidgetClass>
static inline std::string
rps_fltk_geometry_string(const FltkWidgetClass*widg)
{
  if (widg == nullptr)
    return "";
  int x= -1, y= -1, w= 0, h=0;
  float scale=1.0;
  if (rps_fltk_get_window_geometry<FltkWidgetClass>(widg, x, y, w, h, &scale))
    {
      char geombuf[80];
      memset (geombuf, 0, sizeof(geombuf));
      if (scale != 1.0)
        snprintf (geombuf, sizeof(geombuf), "[x=%d,y=%d,w=%d,h=%d,scale=%.3f]", x, y, w, h, scale);
      else
        snprintf (geombuf, sizeof(geombuf), "[x=%d,y=%d,w=%d,h=%d]", x, y, w, h);
      return std::string(geombuf);
    };
  return "[??]";
} // end rps_fltk_geometry_string


/*** Below class is mostly to ease debugging, e.g. as
     RPS_DEBUG_LOG(GUI, "some widget=" << RpsGui_ShowWidget(widget));
 ***/
class RpsGui_ShowWidget
{
  const Fl_Widget* shown_widget;
public:
  RpsGui_ShowWidget(const Fl_Widget*widg = nullptr) : shown_widget(widg) {};
  RpsGui_ShowWidget(const Fl_Widget &widg) : RpsGui_ShowWidget(&widg) {};
  virtual ~RpsGui_ShowWidget()
  {
    shown_widget=nullptr;
  };
  RpsGui_ShowWidget(const RpsGui_ShowWidget&) = delete;
  virtual void output (std::ostream* pout) const;
  void output (std::ostream& out) const
  {
    output(&out);
  };
  const Fl_Widget* widget() const
  {
    return shown_widget;
  };
};				// end of RpsGui_ShowWidget

inline std::ostream&
operator << (std::ostream& out, const RpsGui_ShowWidget&shw)
{
  shw.output(out);
  return out;
} // end output operator <<  RpsGui_ShowWidget


template <class FltkWidgetClass>
class RpsGui_ShowFullWidget : public RpsGui_ShowWidget
{
public:
  RpsGui_ShowFullWidget(const FltkWidgetClass*widg = nullptr)
    : RpsGui_ShowWidget(widg) {};
  RpsGui_ShowFullWidget(const FltkWidgetClass* &widg)
    : RpsGui_ShowWidget(widg) {};
  virtual ~RpsGui_ShowFullWidget() {};
  RpsGui_ShowFullWidget(const RpsGui_ShowFullWidget&) = delete;
  virtual void output (std::ostream* pout) const
  {
    if (!pout) return;
    RpsGui_ShowWidget::output(pout);
    *pout << rps_fltk_geometry_string<FltkWidgetClass>(dynamic_cast<const FltkWidgetClass*>(widget()));
  };
};				// end  RpsGui_ShowFullWidget

template <class FltkWidgetClass>
static inline
std::ostream&operator << (std::ostream& out, const RpsGui_ShowFullWidget<FltkWidgetClass>&shw)
{
  shw.output(&out);
  return out;
} // end output operator << on RpsGui_ShowFullWidget

///

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
  static constexpr int right_menu_gap = 16;
  static constexpr int menu_height = 20;
  Fl_Pack* cmdwin_pack;
  friend  void rps_fltk_initialize(int &,char**);
  virtual void initialize_menubar(void);
  virtual void initialize_pack(void);
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
  RpsGui_OutputTextBuffer*outwin_buffer;
  /// we probably want two instances of Fl_Text_Editor inside some box sharing the outwin_buffer above.
  Fl_Text_Editor* outwin_uppereditor;
  Fl_Text_Editor* outwin_lowereditor;
  friend  void rps_fltk_initialize(int &,char**);
  virtual void initialize_menubar(void);
public:
  virtual ~RpsGui_OutputWindow();
  RpsGui_OutputWindow(int w, int h, const std::string& lab);
  RpsGui_OutputWindow(int x, int y, int w, int h, const std::string& lab);
}; // end class RpsGui_OutputWindow



class RpsGui_OutputTextBuffer : public Fl_Text_Buffer
{
};				// end class RpsGui_OutputTextBuffer

class RpsGui_InputCommand
{
};				// end class RpsGui_InputCommand

#endif /*FLTKHEAD_RPS_INCLUDED*/
// end of file fltkhead_rps.hh */

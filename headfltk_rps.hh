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

// see https://www.fltk.org/doc-1.4/osissues.html#osissues_unix and
// https://stackoverflow.com/questions/61931734/fltk-1-4-widget-position-w-r-t-x11-root-window
#include <FL/platform.H>

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
class RpsGui_MenuBar;
////////////////

extern "C" void rps_fltk_event_loop(Rps_CallFrame*cf);

extern "C" void rps_fltk_stop_event_loop(void);

extern "C" void rps_fltk_initialize(int &argc, char**argv, Rps_CallFrame*curframe);

extern "C" void
rps_fltk_add_delayed_todo(Rps_CallFrame*curframe, double delay,
                          const std::function<void(Rps_CallFrame*,void*,void*)>& todo,
                          void*arg1, void*arg2);
extern "C" void
rps_fltk_add_delayed_closure(Rps_CallFrame*curframe, double delay,
                             Rps_ClosureValue closv,
                             Rps_Value arg1v, Rps_Value arg2v);

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
  const char* visiblestr =
    (widg->visible_r())
    ?"•"://U+2022 BULLET
    (widg->visible_r())?"▫"// U+25AB WHITE SMALL SQUARE
    : "◌"; //U+25CC DOTTED CIRCLE
  if (rps_fltk_get_window_geometry<FltkWidgetClass>(widg, x, y, w, h, &scale))
    {
      char geombuf[80];
      memset (geombuf, 0, sizeof(geombuf));
      if (scale != 1.0)
        snprintf (geombuf, sizeof(geombuf), "[x=%d,y=%d,w=%d,h=%d,scale=%.3f%s]", x, y, w, h, scale, visiblestr);
      else
        snprintf (geombuf, sizeof(geombuf), "[x=%d,y=%d,w=%d,h=%d%s]", x, y, w, h, visiblestr);
      return std::string(geombuf);
    };
  return "[??]";
} // end rps_fltk_geometry_string


//////////
// NOTE ON ABOVE:
// Why do we need templates for rps_fltk_geometry_string() and
// rps_fltk_get_window_geometry()? Could we not do something like below, using
// only the Fl_Widget base class?
// Aslo, aren't floating point comparisons unsafe?

#pragma message "the RpsGui_Geometry class should follow the rule of five"
// see https://cpppatterns.com/patterns/rule-of-five.html
class RpsGui_Geometry
{
public:
  RpsGui_Geometry()
    : gm_x(0)
    , gm_y(0)
    , gm_height(0)
    , gm_width(0)
    , gm_scale(0.0f)
  { }

  RpsGui_Geometry(int x, int y, int width, int height, float scale)
    : gm_x(x)
    , gm_y(y)
    , gm_width(width)
    , gm_height(height)
    , gm_scale(scale)
  { }

  ~RpsGui_Geometry() { }

  inline int x() const
  {
    return gm_x;
  }
  inline void x(int val)
  {
    gm_x = val;
  }
  inline int y() const
  {
    return gm_y;
  }
  inline void y(int val)
  {
    gm_y = val;
  }
  inline int width() const
  {
    return gm_width;
  }
  inline void width(int val)
  {
    gm_width = val;
  }
  inline int height() const
  {
    return gm_height;
  }
  inline void height(int val)
  {
    gm_height = val;
  }
  inline float scale() const
  {
    return gm_scale;
  }
  inline void scale(float val)
  {
    gm_scale = val;
  }

  std::string str() const
  {
    std::stringstream ss;
    ss << "[x = " << x() << ", y = " << y() << ", w = " << width()
       << ", h = " << height();

    if (scale() != 1.0)
      ss << ", scale = " << scale();

    return ss.str();
  }

private:
  int gm_x;
  int gm_y;
  int gm_height;
  int gm_width;
  float gm_scale;
};


static void
rpsgui_get_window_geometry(const Fl_Widget* wx, RpsGui_Geometry& gm)
{
  RPS_ASSERT (wx);

  int xoff = -1, yoff = -1;
  auto wnd = wx->top_window_offset(xoff, yoff);
  RPS_ASSERT (wnd && xoff >= 0 && yoff >= 0);

  gm.x(xoff);
  gm.y(yoff);

  auto scrnum = wnd->screen_num();
  RPS_ASSERT (scrnum >= 0);

  gm.scale(Fl::screen_scale(scrnum));
  RPS_ASSERT (!(gm.scale() <= 0.0f || std::isnan(gm.scale())));

  gm.width(wx->w() * gm.scale());
  gm.height(wx->h() * gm.scale());
}

////// END NOTE

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




////////////////////////////////////////////////////////////////

enum RpsGui_WinTypes
{
  /// every FLTK widget has a type. Please grep the FLTK header files for "RESERVED_TYPE".
  RpsGuiType_CommandWindow = FL_DOUBLE_WINDOW+2,
  RpsGuiType_OutputWindow,
  RpsGuiType_MenuBar,
  RpsGuiType_Pack,

  //////////// SUGGESTION
  // Don't we need to maintain a consistent naming scheme, as in the following?
  // Perhaps a consistent naming scheme would be even more important in case of
  // generated code.
  //
  // I can see why we need enumerators for the Command and Output windows, but I
  // am not sure whether the menu bar and pack widgets should be considered as
  // part of the WinTypes enumeration, since they are not windows per se, but
  // rather are widgets embedded within a double window.

  RpsGui_WinTypes_CommandWindow = FL_DOUBLE_WINDOW + 2,
  RpsGui_WinTypes_OutputWindow,
  RpsGui_WinTypes_MenuBar,
  RpsGui_WinTypes_Pack,

  /////////////////// END SUGGESTION
};

class RpsGui_Window: public Fl_Double_Window
{
  static std::set<RpsGui_Window*> _set_of_gui_windows_;
public:
  virtual int handle(int);
  virtual uchar type() const =0;
  static constexpr int guiwin_border = 5;
protected:
  RpsGui_MenuBar *guiwin_menubar;
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
};				// end class RpsGui_Window

////////////////
class RpsGui_MenuBar : public Fl_Menu_Bar
{
public:
  virtual uchar type(void) const
  {
    return RpsGuiType_MenuBar;
  };
  RpsGui_MenuBar(int X, int Y, int W, int H, const char*lab=nullptr);
  virtual ~RpsGui_MenuBar();
};				// end class RpsGui_MenuBar

class RpsGui_Pack : public Fl_Pack
{
public:
  virtual uchar type(void) const
  {
    return RpsGuiType_Pack;
  };
  RpsGui_Pack(int X, int Y, int W, int H, const char*lab=nullptr);
  virtual ~RpsGui_Pack();
};				// end class RpsGui_Pack


////////////////
class RpsGui_CommandWindow
  : public RpsGui_Window
{
  static constexpr int right_menu_gap = 16;
  static constexpr int menu_height = 20;
  Fl_Pack* cmdwin_pack;
  friend  void rps_fltk_initialize(int &,char**, Rps_CallFrame*);
  virtual void initialize_menubar(void);
  virtual void initialize_pack(void);
  static void menu_dump_cb(Fl_Widget*, void*);
  static void menu_exit_cb(Fl_Widget*, void*);
  static void menu_quit_cb(Fl_Widget*, void*);
public:
  RpsGui_CommandWindow(int w, int h, const std::string& lab);
  RpsGui_CommandWindow(int x, int y, int w, int h, const std::string& lab);
  virtual ~RpsGui_CommandWindow();
  virtual uchar type(void) const
  {
    return RpsGuiType_CommandWindow;
  };
};				// end class RpsGui_CommandWindow

class RpsGui_OutputWindow
  : public RpsGui_Window
{
  RpsGui_OutputTextBuffer*outwin_buffer;
  /// we probably want two instances of Fl_Text_Editor inside some box sharing the outwin_buffer above.
  Fl_Text_Editor* outwin_uppereditor;
  Fl_Text_Editor* outwin_lowereditor;
  friend  void rps_fltk_initialize(int &,char**, Rps_CallFrame*);
  virtual void initialize_menubar(void);
public:
  virtual ~RpsGui_OutputWindow();
  virtual uchar type(void) const
  {
    return RpsGuiType_OutputWindow;
  };
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

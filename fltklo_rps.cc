/****************************************************************
 * file fltklo_rps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the low-level FLTK graphical user interface related
 *      code. See http://fltk.org/
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



extern "C" const char rps_fltklo_gitid[];
const char rps_fltklo_gitid[]= RPS_GITID;

extern "C" const char rps_fltklo_date[];
const char rps_fltklo_date[]= __DATE__;

std::set<std::unique_ptr<RpsGui_Window>> RpsGui_Window::_set_of_gui_windows_;

RpsGui_Window::RpsGui_Window(int w, int h, const std::string& lab)
  : Fl_Double_Window(w,h), guiwin_ownoid(), guiwin_label(lab)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  label(guiwin_label.c_str());
  _set_of_gui_windows_.insert(std::unique_ptr<RpsGui_Window>(this));
}; // end RpsGui_Window::RpsGui_Window

RpsGui_Window::RpsGui_Window(int x, int y, int w, int h, const std::string& lab)
  : Fl_Double_Window(x,y,w,h), guiwin_ownoid(), guiwin_label(lab)
{
  RPS_ASSERT(rps_is_main_gui_thread());
  label(guiwin_label.c_str());
  _set_of_gui_windows_.insert(std::unique_ptr<RpsGui_Window>(this));
};				// end RpsGui_Window::RpsGui_Window


RpsGui_Window::~RpsGui_Window()
{
  RPS_ASSERT(rps_is_main_gui_thread());
  if (guiwin_ownoid)
    clear_owning_object();
  _set_of_gui_windows_.erase(std::unique_ptr<RpsGui_Window>(this));
  if (_set_of_gui_windows_.empty())
    rps_fltk_stop_event_loop();
};				// end RpsGui_Window::~RpsGui_Window

void
RpsGui_Window::clear_owning_object(void)
{
  if (!guiwin_ownoid)
    return;
  Rps_ObjectRef obr = Rps_ObjectRef::really_find_object_by_oid(guiwin_ownoid);
  if (!obr)
    return;
  guiwin_ownoid = Rps_Id(nullptr);
  std::unique_lock<std::recursive_mutex> guobr (*(obr->objmtxptr()));
  auto winpayl = obr->get_dynamic_payload<Rps_PayloadWindow>();
  if (!winpayl)
    return;
  if (winpayl->owner() != obr)
    return;
  obr->clear_payload();
  obr->gui_window_reset_class(this);
}; // end RpsGui_Window::clear_owning_object

int
RpsGui_Window::handle(int evtype)
{
  /* by default, FLTK understands the ESC key as
     closing. See https://www.fltk.org/doc-1.4/FAQ.html */
  if ((evtype == FL_KEYDOWN || evtype == FL_KEYUP))
    {
      if (Fl::event_key() == FL_Escape)
        {
          RPS_DEBUG_LOG(GUI, "GuiWindow " << label_str() << " ignore escape");
          return 1;
        }
    }
  else if (evtype == FL_HIDE)
    {
      RPS_DEBUG_LOG(GUI, "GuiWindow " << label_str() << " got hide");
    }
  else RPS_DEBUG_LOG(GUI, "GuiWindow " << label_str() << " evtype#" << evtype);
  return Fl_Double_Window::handle(evtype);
}; // end RpsGui_Window::handle

Rps_ObjectRef
RpsGui_Window::owning_object(Rps_CallFrame*callframe) const
{
  return Rps_ObjectRef::find_object_by_oid(callframe, owning_oid());
}; // end RpsGui_Window::owning_object

RpsGui_CommandWindow::RpsGui_CommandWindow(int w, int h, const std::string& lab)
  : RpsGui_Window(w,h,lab)
{
  RPS_DEBUG_LOG(GUI, "creating RpsGui_CommandWindow w=" << w << ", h=" << h
                << ", lab=" << lab << " this@" << (void*)this);
};				// end RpsGui_CommandWindow::RpsGui_CommandWindow

RpsGui_CommandWindow::RpsGui_CommandWindow(int x, int y, int w, int h, const std::string& lab)
  : RpsGui_Window(x,y,w,h,lab)
{
  RPS_DEBUG_LOG(GUI, "creating RpsGui_CommandWindow x=" << x << ", y=" << y
                << " w=" << w << ", h=" << h
                << ", lab=" << lab << " this@" << (void*)this);
}; // end RpsGui_CommandWindow::RpsGui_CommandWindow


RpsGui_CommandWindow::~RpsGui_CommandWindow()
{
  RPS_DEBUG_LOG(GUI, "destroying RpsGui_CommandWindow@" << (void*)this);
}; // end RpsGui_CommandWindow::~RpsGui_CommandWindow

Rps_PayloadWindow::~Rps_PayloadWindow()
{
}; // end Rps_PayloadWindow::~Rps_PayloadWindow

bool
Rps_PayloadWindow::is_erasable() const
{
  return true;
}; // end Rps_PayloadWindow::is_erasable()

void
Rps_PayloadWindow::gc_mark(Rps_GarbageCollector&gc) const
{
} // end Rps_PayloadWindow::gc_mark

void
Rps_PayloadWindow::dump_scan(Rps_Dumper*du) const
{
  RPS_ASSERT(du != nullptr);
} // end Rps_PayloadWindow::dump_scan

void
Rps_PayloadWindow::dump_json_content(Rps_Dumper*du, Json::Value&) const
{
  RPS_ASSERT(du != nullptr);
} // end Rps_PayloadWindow::dump_json_content

//////////////////////////////////////// end of file fltklo_rps.cc

/****************************************************************
 * file gui_qrps.cc
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the Qt5 Graphical User Interface Q_OBJECT code
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2019 The Reflective Persistent System Team
 *      team@refpersys.org
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
#ifndef REFPERSYS_INCLUDED
#error header refpersys.hh should be included before this one
#endif /*REFPERSYS_INCLUDED*/

#ifndef QTGUIQRPS_INCLUDED
#define QTGUIQRPS_INCLUDED 1

#include <QApplication>
#include <QWidget>
#include <QtWidgets>


/// think of future Qt6 compatibility

class RpsWindow;
class RpsApplication : public QApplication {
  Q_OBJECT
public:
  // the vector of windows
  std::vector<std::unique_ptr<RpsWindow>> app_winvec;
  RpsApplication(int &argc, char **argv);
  virtual ~RpsApplication();
};				// end RpsApplication


class RpsWindow : public QWindow {
  Q_OBJECT
  QVBoxLayout topvbox;
public:
  RpsWindow();
  virtual ~RpsWindow();
};				// end RpsWindow


#endif /*QTGUIQRPS_INCLUDED*/
/************* end  of file qtgui_qrps.hh ************/

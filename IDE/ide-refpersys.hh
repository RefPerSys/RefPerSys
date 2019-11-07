/****************************************************************
 * file ide-refpersys.hh
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is its only C++ header file for the temporary IDE
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



#ifndef IDEREFPERSYS_INCLUDED
#define IDEREFPERSYS_INCLUDED

// A name containing `unsafe` refers to something which should be used
// with great caution.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#if __cplusplus < 201412L
#error expecting C++17 standard
#endif

#include <set>
#include <map>
#include <variant>
#include <unordered_map>
#include <new>
#include <random>
#include <iostream>
#include <limits>
#include <initializer_list>
#include <algorithm>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <functional>

// FLTKL 1.3 - see www.fltk.org
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
////////////////

/// HJSON-CPP, installed from https://github.com/hjson/hjson-cpp
#include "hjson/hjson.h"

#include "refpersys.hh"

#endif /*IDEREFPERSYS_INCLUDED*/
////////// end of file ide-refpersys.hh

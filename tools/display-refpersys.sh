#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
#      © Copyright 2025 - 2025 The Reflective Persistent System Team
#      team@refpersys.org & http://refpersys.org/
#
# contributed by Basile Starynkevitch
# License:
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

if [ -z "$REFPERSYS_TOPDIR" ]; then
    echo $0 script is missing REFPERSYS_TOPDIR > /dev/stderr
    exit 1
fi

if [ ! -d "$REFPERSYS_TOPDIR" ]; then
    echo $0 script with bad REFPERSYS_TOPDIR non-directory $REFPERSYS_TOPDIR > /dev/stderr
    exit 1
fi

cd $REFPERSYS_TOPDIR

if [ ! -f "refpersys.hh" ]; then
    echo $0 script in REFPERSYS_TOPDIR $REFPERSYS_TOPDIR there is no refpersys.hh header > /dev/stderr
    exit 1
fi

if [ ! -x "refpersys" ]; then
    echo $0 script in REFPERSYS_TOPDIR $REFPERSYS_TOPDIR there is no refpersys executable > /dev/stderr
    exit 1
fi

echo $0 script in  REFPERSYS_TOPDIR $REFPERSYS_TOPDIR is incomplete  > /dev/stderr
exit 1

## we probably want to compile then use plugins_dir/rpsplug_display.cc

## should probably use getopts 

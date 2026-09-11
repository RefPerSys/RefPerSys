#!/bin/bash
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

/usr/bin/gmake -j4 refpersys 2>&1 > $HOME/tmp/making-refpersys.out

if [ ! -x "refpersys" ]; then
    echo $0 script in REFPERSYS_TOPDIR $REFPERSYS_TOPDIR there is no refpersys executable > /dev/stderr
    exit 1
fi


#echo $0 script has $# argc

if [ $# != 1 ]; then
    echo $0 script in   REFPERSYS_TOPDIR $REFPERSYS_TOPDIR needs one argument could be --help
    exit 2
fi

if [ "$1" = "--help" ]; then
   echo $0 script needs an argument, either --help or the name or objid of some object
   exit 0
fi


## we probably want to compile then use plugins_dir/rpsplug_display.cc
/usr/bin/gmake do-build-refpersys-plugin 2>&1 > $HOME/tmp/making-rps-plugin-builder.out

./do-build-refpersys-plugin -v plugins_dir/rpsplug_display.cc -o plugins_dir/rpsplug_display.so --symlink=/tmp/rpsplug_display.so 2>&1 > $HOME/tmp/making-rpsplug_display.out

./refpersys --plugin-after-load=/tmp/rpsplug_display.so \
            --batch --plugin-arg=rpsplug_display:$1 \
	    --extra=from=$0 --extra=lineno=$LINENO  -run=display



#### end of file RefPerSys/tools/display-refpersys.sh

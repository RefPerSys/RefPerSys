#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# file RefPerSys/test_dir/007display.bash
#  © Copyright (C) 2026 The Reflective Persistent System Team
#      team@refpersys.org & http://refpersys.org/
#  contributed by Basile Starynkevitch, France

# License:
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This GNU bash script is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

disp=$1

if [ -z "$disp" ]; then
    echo $0 should be used with an argument the object to display > /dev/stderr
    echo $0 got none > /dev/stderr
    exit 1
fi

if [ -n "$REFPERSYS_TOPDIR" ]; then
    cd  "$REFPERSYS_TOPDIR"  && /bin/pwd
fi

if [ ! -x refpersys ]; then
    /usr/bin/gmake -j3 refpersys
fi

if [ ! -x refpersys ]; then
    echo 'no refpersys executable in ' $(/bin/pwd) > /dev/stderr
    exit 1
fi
if [ ! -x do-build-refpersys-plugin ]; then
    /usr/bin/gmake  do-build-refpersys-plugin
fi

if [ -f /tmp/rpsplug_display.so ]; then
    /bin/rm -vf /tmp/rpsplug_display.so
fi


if [ ! -x do-build-refpersys-plugin ]; then
    echo 'no do-build-build-refpersys-plugin executable in ' $(/bin/pwd) > /dev/stderr
    echo 'try running make do-build-refpersys-plugin' > /dev/stderr
    exit 1
fi

./do-build-refpersys-plugin --verbose \
			    --input=plugins_dir/rpsplug_display.cc \
			    --output=plugins_dir/rpsplug_display.so \
			    --symlink=/tmp/rpsplug_display.so

## printf is builtin in bash
printf "\n\n%s: after do-build-refpersys-plugin pid %d\n" $0 $$

/bin/ls -ltf ./refpersys $(/bin/pwd)/plugins_dir/rpsplug_display.{cc,so} /tmp/rpsplug_display.so

echo

./refpersys -AREPL --script=$(/usr/bin/realpath $0) --batch \
	    --plugin-after-load=/tmp/rpsplug_display.so \
	    --plugin-arg=rpsplug_display:$disp

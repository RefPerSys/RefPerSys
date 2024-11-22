#!/bin/bash -x
# SPDX-License-Identifier: GPL-3.0-or-later
# script file create-refpersys-root-class.sh in RefPerSys - see http://refpersys.org/
#
#      © Copyright 2023 The Reflective Persistent System Team
#      team@refpersys.org & http://refpersys.org/
#
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

### invocation as
##     ./create-refpersys-root-class.sh <newclassname> <superclass>

function show_usage() {
    printf "%s: usage\n" $0
    printf "  %s <new-class-name> <super-class>\n" $1
    printf "See refpersys.org\n"
}

if [ $# -lt 2 ]; then
    show_usage
    exit
fi

if [ $1 == "--help" ]; then
    show_usage
    exit
fi

if [ ! -f "refpersys.hh" ]; then
    printf "%s: no refpersys.hh in current directory %s\n" $0 $(/bin/pwd)
    exit 1
fi

if [ ! -f "plugins/rpsplug_createclass.cc" ]; then
    printf "%s: no plugins/rpsplug_createclass.cc in current directory %s\n" $0 $(/bin/pwd)
    exit 1
fi

make refpersys do-build-refpersys-plugin
./do-build-refpersys-plugin plugins/rpsplug_createclass.cc -o /tmp/rpsplug_createclass.so


if [ ! -f /tmp/rpsplug_createclass.so ]; then
    printf "%s: failed to build plugin %s in  current directory %s\n" $0 /tmp/rpsplug_createclass.so $(/bin/pwd)
    exit 1
fi


./refpersys --plugin-after-load=/tmp/rpsplug_createclass.so \
            --plugin-arg=rpsplug_createclass:$1 \
            --extra=super=$2 \
	    --extra=root=true \
            --batch --dump=.

# enf of script create-refpersys-root-class.sh

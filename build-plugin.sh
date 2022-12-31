#!/bin/bash
# file build-plugin.sh in RefPerSys - see http://refpersys.org/
# Author(s):
#      Basile Starynkevitch <basile@starynkevitch.net>
#      Abhishek Chakravarti <abhishek@taranjali.org>
#      Nimesh Neema <nimeshneema@gmail.com>
#
#      © Copyright 2020 - 2022 The Reflective Persistent System Team
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
##     ./build-plugin.sh <C++-plugin-source> <plugin-sharedobject>

cppfile=$1
pluginfile=$2
declare curdate;
curdate=$(date +%c);
printf "start %s at %s: C++ file %s, plugin file %s\n" $0 "$curdate" $cppfile $pluginfile > /dev/stderr
logger --id=$$ -s  -t "$0:" "starting" cppfile= $1 pluginfile= $2 curdate= $curdate
eval $(make print-plugin-settings)

## check that we have the necessary shell variables set in above eval
if [ -z "$RPSPLUGIN_CXX" ]; then
    echo RPSPLUGIN_CXX missing in $0 > /dev/stderr
    exit 1
fi
if [ -z "$RPSPLUGIN_CXXFLAGS" ]; then
    echo RPSPLUGIN_CXXFLAGS missing in $0 > /dev/stderr
    exit 1
fi
if [ -z "$RPSPLUGIN_LDFLAGS" ]; then
    echo RPSPLUGIN_LDFLAGS missing in $0 > /dev/stderr
    exit 1
fi

## run the compiler suitably
logger --id=$$ -s  -t $0 running: "$RPSPLUGIN_CXX $RPSPLUGIN_CXXFLAGS -Wall -fPIC -shared $cppfile $RPSPLUGIN_LDFLAGS -o $pluginfile"
## 
exec $RPSPLUGIN_CXX $RPSPLUGIN_CXXFLAGS -Wall -fPIC -shared $cppfile $RPSPLUGIN_LDFLAGS -o $pluginfile 

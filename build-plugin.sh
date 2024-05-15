#!/bin/bash
# file build-plugin.sh in RefPerSys - see http://refpersys.org/
# Author(s):
#      Basile Starynkevitch <basile@starynkevitch.net>
#      Abhishek Chakravarti <abhishek@taranjali.org>
#      Nimesh Neema <nimeshneema@gmail.com>
#
#      © Copyright 2020 - 2024 The Reflective Persistent System Team
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

## same as MY_HEAD_LINES_THRESHOLD in do-scan-pkgconfig.c
MY_HEAD_LINES_THRESHOLD=384

cppfile=$1
pluginfile=$2
declare curdate;
curdate=$(date +%c);
printf "start %s at %s: C++ file %s, plugin file %s in %s\n" $0 \
       "$curdate" $cppfile $pluginfile "$(/bin/pwd)" > /dev/stderr
/usr/bin/logger --id=$$ -s  -t "$0:" "starting" cppfile= $1 pluginfile= $2 curdate= $curdate
eval $(gmake print-plugin-settings)

### plugincppflags contain compiler flags
### pluginlinkerflags contain linker flags

if /usr/bin/fgrep -q '@RPSCOMPILEFLAGS=' $cppfile ; then
    plugincppflags=$(/bin/head -$MY_HEAD_LINES_THRESHOLD $cppfile | /usr/bin/gawk --source '/@RPSCOMPILEFLAGS=/ { for (i=2; i<=NF; i++) print $i; }')
else
    plugincppflags=()
fi


## ugly hack needed in March 2024 after commit 456fcb27bc57f
if [ -f /usr/include/jsoncpp/json/json.h ]; then
    plugincppflags+=" -I/usr/include/jsoncpp"
fi

if /usr/bin/fgrep -q '@RPSLIBES=' $cppfile ; then
    pluginlinkerflags=$(/bin/head -$MY_HEAD_LINES_THRESHOLD $cppfile | /usr/bin/gawk --source '/@RPSLIBES=/ { for (i=2; i<=NF; i++) print $i; }')
else
    pluginlinkerflags=()
fi

if  /usr/bin/fgrep -q '//@@PKGCONFIG' $cppfile ; then
    local pkglist=$(./do-scan-pkgconfig $cppfile)
    plugincppflags="$plugincppflags $(pkg-config --cflags $pkglist)"
    pluginlinkerflags="$pluginlinkerflags $(pkg-config --libs $pkglist)"
fi

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
logger --id=$$ -s  -t $0 running: "$RPSPLUGIN_CXX $RPSPLUGIN_CXXFLAGS  $plugincppflags -Wall -fPIC -shared $cppfile $RPSPLUGIN_LDFLAGS  $pluginlinkerflags -o $pluginfile"
## 
$RPSPLUGIN_CXX $RPSPLUGIN_CXXFLAGS $plugincppflags -Wall -Wextra -I. -fPIC -shared $cppfile $RPSPLUGIN_LDFLAGS \
	       $pluginlinkerflags -o $pluginfile  || ( \
						       printf "\n%s failed to compile RefPerSys plugin %s to %s \
			   			       (cxxflags %s cppflags %s ldflags %s linkerflags %s)\n" \
							      $0 $cppfile $pluginfile "$RPSPLUGIN_CXXFLAGS" "$plugincppflags" \
							      "$RPSPLUGIN_LDFLAGS" "$pluginlinkerflags"> /dev/stderr; \
						       /usr/bin/logger --id=$$ -s  -t $0 -puser.warning \
								       "$0 failed to compile RefPerSys plugin $cppfile to $pluginfile\n" ; \
						       exit 1)

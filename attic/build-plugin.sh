#!/bin/dash
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

### example
##  To compile the C++ plugin source in file  plugins_dir/rpsplug_createclass.cc
##  into the dlopen-able shared object /tmp/rpsplug_createclass.so
##  run the following command
##     ./build-plugin.sh plugins_dir/rpsplug_createclass.cc \
##                       /tmp/rpsplug_createclass.so
##  a later invocation of refpersys is explained in a C++ comment in
##  this C++ plugin file plugins_dir/rpsplug_createclass.cc
## the C++ plugin source may contain comments driving this compilation.


## TODO: in commit 3dc7163fa129 of May 15, 2024 this is a buggy
## script. I (Basile S.) hope for some Python3 expert to replace it
## with a more robust Python script which would scan the C++ source
## code comments....

## same as MY_HEAD_LINES_THRESHOLD in do-scan-pkgconfig.c
MY_HEAD_LINES_THRESHOLD=512

if [ "$1" = '--help' ]; then
    echo $0 usage to compile a RefPerSys plugin C++ code on Linux:
    echo $0 "<plugin-C++-source-file>" "<output-shared-object>"
    echo for example: $0 plugins_dir/rpsplug_createclass.cc /tmp/rpsplug_createclass.so
    echo a later execution is: ./refpersys --plugin-after-load=/tmp/rpsplug_createclass.so
    echo and is given in that plugins_dir/rpsplug_createclass.cc file :
    /bin/head -20 plugins_dir/rpsplug_createclass.cc
    exit
fi

/usr/bin/logger -i -s --id=$$ --tag refpersys-build-plugin  --priority user.info "$0" "$@"

cppfile=$1
pluginfile=$2
declare curdate;
curdate=$(date +%c);
declare pkglist;
pkglist=""

printf "start %s at %s: C++ file %s, plugin file %s in %s\n" $0 \
       "$curdate" $cppfile $pluginfile "$(/bin/pwd)" > /dev/stderr

if [ -z "$REFPERSYS_TOPDIR" ]; then
    printf "%s: missing REFPERSYS_TOPDIR\n" $0 > /dev/stderr
    exit 1
fi

/usr/bin/logger --id=$$ -s  -t "$0:" "starting" cppfile= $1 pluginfile= $2 curdate= $curdate REFPERSYS_TOPDIR= $REFPERSYS_TOPDIR cwd $(/bin/pwd)

eval $(/usr/bin/make print-plugin-settings)

### plugincppflags contain compiler flags
### pluginlinkerflags contain linker flags
#### Scan the C++ source files for comments giving compiler and linker flags.
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
    pkglist=$($REFPERSYS_TOPDIR/do-scan-pkgconfig --raw $cppfile)
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
               $pluginlinkerflags -o $pluginfile  || \
    ( \
      printf "\n%s failed to compile RefPerSys plugin %s to %s\n \
                (°cxxflags %s\n °cppflags %s\n °ldflags %s\n °linkerflags %s\n °pkg-list %s\n)\n" \
             $0 $cppfile $pluginfile "$RPSPLUGIN_CXXFLAGS" "$plugincppflags" \
             "$RPSPLUGIN_LDFLAGS" "$pluginlinkerflags" "$pkglist" > /dev/stderr ; \
      /usr/bin/logger --id=$$ -s  -t $0 -puser.warning \
                      "$0 failed to compile RefPerSys plugin $cppfile to $pluginfile in $(/bin/pwd)\n" ; \
      exit 1 )

## end of file RefPerSys/build-plugin.sh

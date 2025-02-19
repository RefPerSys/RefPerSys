#!/bin/bash -x
# SPDX-License-Identifier: GPL-3.0-or-later
# A shell script to create the code_int object reifying the int type
#      © Copyright 2025 Basile STARYNKEVITCH 
#      see team@refpersys.org & http://refpersys.org/
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

rps_scriptname=$0
if [ -z "$REFPERSYS_TOPDIR" ]; then
    printf "%s: without REFPERSYS_TOPDIR\n" $rps_scriptname > /dev/stderr
    exit 1
fi
cd "$REFPERSYS_TOPDIR"
if [ ! -f "refpersys.hh" ]; then
    printf "%s: in %s no refpersys.hh header file\n" $rps_scriptname $(/bin/pwd) > /dev/stderr
    exit 1
fi
make -j3 refpersys || exit 1
rps_persistore="$(/usr/bin/realpath persistore)"

if /bin/grep -rwl code_int persistore/ ; then
    printf "%s: already known code_int in %s\n" \
	   $rps_scriptname $rps_persistore ;
    exit 0
fi

make plugins_dir/rpsplug_create_cplusplus_primitive_type.so   || exit 1

./refpersys --plugin-after-load=plugins_dir/rpsplug_create_cplusplus_primitive_type.so \
	    --plugin-arg=rpsplug_create_cplusplus_primitive_type:code_int \
	    --extra=super='cplusplus_primitive_type' \
	    --extra=comment='the native int type' \
	    --batch --dump=.

if  ! /bin/grep -rwl code_int persistore/ > /dev/null ; then
    printf "%s: no code_int in %s\n" \
	   $rps_scriptname $rps_persistore ;
    exit 1
fi

printf "%s: the store in %s contains code_int\n" \
       $rps_scriptname $rps_persistore
printf "%s: dont forget to edit rps_set_native_data_in_loader in load_rps.cc\n" \
       $rps_scriptname
exit 0

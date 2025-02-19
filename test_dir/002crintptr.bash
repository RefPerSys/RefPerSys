#!/bin/bash -x
# SPDX-License-Identifier: GPL-3.0-or-later
# aé shell script to create the intptr object reifying the intptr type
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

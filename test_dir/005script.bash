#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# file RefPerSys/test_dir/005script.bash
#  © Copyright (C) 2025 - 2026 The Reflective Persistent System Team
#      team@refpersys.org & http://refpersys.org/

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

## CPU time soft and hard limits in seconds
ulimit -S -t 5
ulimit -H -t 8
## memory limit in kilobytes (512 Mbytes)
ulimit -S -m 512000
## file size limits (in half kilobytes blocks)
ulimit -S -f 32768

echo running refpersys -AREPL --script=$0 --user-pref=. --batch --run-name=005script

./refpersys -AREPL --script=$0 --user-pref=. --batch --run-name=005script
exit $?

## for GDB use
## gdb --args ./refpersys -AREPL --script=test_dir/005script.bash --user-pref=. --batch --run-name 005script

## magic string should not be commented
## see scripting_rps.cc file
REFPERSYS_SCRIPT carbon
## etc

@display RefPerSys_system

@display 1 + 2
# end of file test_dir/005script.bash in refpersys

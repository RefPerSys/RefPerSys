#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
#  © Copyright (C) 2025 - 2025 The Reflective Persistent System Team
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
if [ ! -x refpersys ]; then
    echo 'no refpersys executable in ' $(/bin/pwd) > /dev/stderr
    exit 1
fi
./refpersys -AREPL --script=$0 --batch --run-name=005script
exit $?

## for GDB use
## gdb --args ./refpersys -AREPL --script=test_dir/005script.bash --batch --run-name 005script
## magic string REFPERSYS_SCRIPT carbon
## etc



# end of file test_dir/005script.bash in refpersys

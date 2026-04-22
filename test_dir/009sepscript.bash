#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# file RefPerSys/test_dir/009sepscript.bash
#  © Copyright (C) 2026 The Reflective Persistent System Team
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
ulimit -S -t 15
ulimit -H -t 18
## memory limit in kilobytes (2 Gbytes)
ulimit -S -m $[[2 * 1024]]
## file size limits (in half kilobytes blocks)
ulimit -S -f 32768

echo running refpersys -AREPL --script=test_dir/009sepscript.rps --user-pref=. --batch --run-name=009sepscript

./refpersys -AREPL --script=test_dir/009sepscript.rps --user-pref=. --batch --run-name=009sepscript

exit $?

## for Emacs:
## Local Variables: ;;
## compile-command: "cd $REFPERSYS_TOPDIR && test_dir/009sepscript.bash" ;;
## End: ;;
####### eof RefPerSys/test_dir/009sepscript.bash

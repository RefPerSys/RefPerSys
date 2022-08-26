################################################################################
# file tools/msg.sh
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Description:
#      This file is part of the Reflective Persistent System.
#
#      It implements the shell script functions to display timestamped and
#      coloured messages to stdout.
#
# Author(s):
#      Basile Starynkevitch <basile@starynkevitch.net>
#      Abhishek Chakravarti <abhishek@taranjali.org>
#      Nimesh Neema <nimeshneema@gmail.com>
#
#      © Copyright 2020 The Reflective Persistent System Team
#      team@refpersys.org & http://refpersys.org/
#
# License:
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
################################################################################


msg_ok()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[0;32m OK \033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1"
}


msg_info()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[0;34mINFO\033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1"
}


msg_warn()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[1;33mWARN\033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1"
}


msg_fail()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf                                                          \
            '[\033[1;31mFAIL\033[0m] \033[0;35m%s\033[0m: %s...\n'      \
            "$ts"                                                       \
            "$1"                                                        \
            2>&1

        exit 1
}


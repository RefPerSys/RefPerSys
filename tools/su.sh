################################################################################
# file tools/su.sh
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Description:
#      This file is part of the Reflective Persistent System.
#
#      It defines the SU shell script variable as an abstraction over either
#      sudo or doas. Requires that the tools/msg.sh file be sourced.
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

# su command for client code
export SU

# set SU to either sudo or doas, depending on which is available. sudo takes
# preference.
if sudo -V >/dev/null 2>&1; then
	SU=sudo
elif doas -L >/dev/null 2>&1; then
	SU=doas
else
	msg_fail 'neither sudo nor doas found; install one of the two'
fi

################################################################################
# file tools/os.sh
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Description:
#      This file is part of the Reflective Persistent System.
#
#      It verifies that a supported Linux distribution is running on the host
#      machine.
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



# Linux distribution
export DISTRO

# Determine kernel
kernel=$(uname -s | tr '[:upper:]' '[:lower:]')

# Ensure we're using a Linux kernel
if [ "$kernel" != linux ]
then
	msg_fail "please use a Linux distribution"
fi

# Ensusre /etc/os-release is available
emsg='failed to determine Linux distribution'
[ -f /etc/os-release ] || msg_fail "$emsg"

# Determine Linux distribution
DISTRO=$(grp 'NAME' /etc/os-release \
	| head -n 1 \
	| cut -d '=' -f 2 \
	| tr -d '"' \
	| cut -d ' ' -f 1 \
	| tr '[:upper:]' '[:lower:]')

# Ensure we're using either Debian or Ubuntu
emsg="unsupported Linux distribution: $DISTRO"
[ "$DISTRO" != debian ]	\
	&& [ "$DISTRO" != ubuntu ] \
	&& msg_fail "$emsg"
	

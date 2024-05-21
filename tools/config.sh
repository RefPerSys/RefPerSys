#!/bin/sh

#
# SPDX-License-Identifier: GPL-3.0-or-later
# RefPerSys file gui-script-refpersys.sh - see refpersys.org
#
#      Copyright 2022 The Reflective Persistent System Team
#      <http://refpersys.org>
#
# Description:
#
#      This file is part of the Reflective Persistent System.  It may
#      be started by the refpersys executable to provide some
#      graphical user interface
#
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
#

#
# Print a timestamped and coloured success message on to stderr.
#
msg_ok()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[0;32m OK \033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1" 1>&2
}

#
# Print a timestamped and coloured informational message on to stderr.
#
msg_info()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[0;34mINFO\033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1" 1>&2
}

#
# Print a timestamped and coloured warning message on to stderr.
#
msg_warn()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf '[\033[1;33mWARN\033[0m] \033[0;35m%s\033[0m: %s...\n' "$ts" "$1" 1>&2
}

#
# Print a timestamped and coloured error message on to stderr and exit with error code 1.
#
msg_fail()
{
        ts=$(date +'%b %d %H:%M:%S')
        printf                                                          \
            '[\033[1;31mFAIL\033[0m] \033[0;35m%s\033[0m: %s...\n'      \
            "$ts"                                                       \
            "$1"                                                        \
            1>&2

        exit 1
}

#
# Ensure that we're running a supported Linux distribution.
# As of now, we support only Debian/Ubuntu, but other Linux
# distros are in the pipeline.
#
check_os()
{
	emsg="only Debian/Ubuntu supported"

	kernel=$(uname -s | tr '[:upper:]' '[:lower:]')
	test "$kernel" = linux || msg_fail "$emsg"

	test -f /etc/os-release || msg_fail "$emsg"
	distro=$(grep 'NAME' /etc/os-release \
	    | head -n 1                      \
	    | cut -d '=' -f 2                \
	    | tr -d '"'                      \
	    | cut -d ' ' -f 1                \
	    | tr '[:upper:]' '[:lower:]')

	test "$distro" = debian        \
	    || test "$distro" = ubuntu \
	    || msg_fail "$emsg"
}

#
# Parse the command line options passed to the configure script.
#
parse_flags()
{
	echo 'TODO'
}

#
# Parse the command line arguments passed to the configure script.
#
parse_args()
{
	echo 'TODO'
}

#
# Main entry point for the configuration script.  We parse the command
# line options and arguments passed to this script after checking that
# a supported host OS environment is present.
#
main()
{
	check_os
	parse_flags "$@"
	parse_args "$@"
}

main "$@"


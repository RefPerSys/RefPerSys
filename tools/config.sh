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
# NOTE: Code adapted from the Fifth Estate cvs/01.src/build source tree,
# which is released under the BSD 2-Clause License.
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
        printf                                                  \
            '[\033[1;31mFAIL\033[0m] \033[0;35m%s\033[0m: %s\n' \
            "$ts"                                               \
            "$1"                                                \
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
# Callback function to handle `make help`.
#
run_help()
{
	echo 'usage:'
	printf '\t%s\t\t%s\n' '$ make help' 'Show this help message'
	printf '\t%s\t\t%s\n' '$ make intro' 'Show introductory notes'
	printf '\t%s\t\t%s\n' '$ make config' 'Generate config.mk file'
	printf '\t%s\t\t%s\n' '$ make check' 'Check build prerequisites'
	printf '\t%s\t\t%s\n' '$ make build' 'Build target objects'
	printf '\t%s\t\t%s\n' '$ make test' 'Run regression tests'
	printf '\t%s\t\t%s\n' '# make install' 'Install target objects'
	printf '\t%s\t%s\n' '# make uninstall' 'Uninstall target objects'
	printf '\t%s\t\t%s\n' '# make dist' 'Prepare release tarball'
	printf '\t%s\t\t%s\n' '$ make clean' 'Remove build artefacts'
}

#
# Parse the command line options passed to the configure script.
#
parse_flags()
{
	OPT_VERBOSE=0
	while getopts ':v' opt; do
		case $opt in
			v)
				OPT_VERBOSE=$((OPT_VERBOSE+1))
				;;

			:)	
				msg_fail "-OPTARG: missing argument"
				;;
			?)
				msg_fail "-$OPTARG: unknown option"
				;;
		esac
	done

	test "$OPT_VERBOSE" -gt 1 && msg_fail '-v: excess count'
}

#
# Parse the command line arguments passed to the configure script.
#
parse_args()
{
	# Reset to allow next calls to getopts
	shift $((OPTIND-1))
	OPTIND=1

	# Redirect control to the appropriate command handling function
	case $1 in
		help)   shift; run_help "$@";;
		intro)  shift; run_intro "$@";;
		config) shift; run_config "$@";;
		check)  shift; run_check "$@";;
		dist)   shift; run_dist "$@";;
		clean)  shift; run clean "$@";;
		*)      msg_fail "$1: unknown argument";;
	esac
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


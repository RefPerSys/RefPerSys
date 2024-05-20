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
# Ensure that we're running a supported Linux distribution.
# As of now, we support only Debian/Ubuntu, but other Linux
# distros are in the pipeline.
#
check_os()
{
	echo 'TODO'
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


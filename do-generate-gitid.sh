#!/bin/sh

#% SPDX-License-Identifier: GPL-3.0-or-later
# RefPerSys file do-generate-gitid.sh - see refpersys.org
###
# it just emits a string with the full git commit id, appending + if
# the git status is not clean.
#%
#%      © Copyright 2020-2022 The Reflective Persistent System Team
#%      <http://refpersys.org>
#%
#% Description:
#%
#%      This file is part of the Reflective Persistent System.
#%      This script is used by the Makefile and emits a string with the gitid
#%
#%
#% License:
#%    This program is free software: you can redistribute it and/or modify
#%    it under the terms of the GNU General Public License as published by
#%    the Free Software Foundation, either version 3 of the License, or
#%    (at your option) any later version.
#%
#%    This program is distributed in the hope that it will be useful,
#%    but WITHOUT ANY WARRANTY; without even the implied warranty of
#%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#%    GNU General Public License for more details.
#%
#%    You should have received a copy of the GNU General Public License
#%    along with this program.  If not, see <http://www.gnu.org/licenses/>.

usage()
{
	echo "usage: ./do-generate-gitid.sh [-hlsv]"
	echo "options:"
	echo "  -h  show usage"
	echo "  -l  show license" 
	echo "  -s  print short git ID" 
	echo "  -v  show version" 
}

license()
{
	cat ./COPYING-SHORT
}

version()
{
	maj="$(grep -R RPS_MAJOR_VERSION . \
		| head -n 1 \
		| cut -d ' ' -f 3)"

	min="$(grep -R RPS_MINOR_VERSION . \
		| head -n 1 \
		| cut -d ' ' -f 3)"

	echo "v${maj}.${min}"
}

getid()
{
    	gitid="$(git log --format=oneline -q -1 \
		| cut '-d ' -f 1 \
		| tr -d '\n')"
}

isdirty()
{
	if git status | grep -q "nothing to commit" ; then
    		dirty=""
	else
    		dirty="+"
	fi
}

output()
{
	if [ "$1" = "-s" ]; then
    		printf "%.12s%s\n" "${gitid}" "${dirty}"
	else
    		printf "%s%s\n" "${gitid}" "${dirty}"
	fi
}

main()
{
	getid
	isdirty
	output "$@"
	license
}

main "$@"


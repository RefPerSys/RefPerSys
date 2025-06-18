#!/bin/gawk
# SPDX-License-Identifier: GPL-3.0-or-later
#
# © Copyright (C) 2025 - 2025, Basile Starynkevitch and the
# forum@refpersys.org mailing list contributors
#
# This file rps-generate-linking-command.awk is part of the Reflexive
# Persistent System (aka RefPerSys); it just emits The C bytes
# containing the linking command
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
BEGIN{
    doecho=0;
    nbc=0;
    printf "///start rps-generate-linking-command\n";
}

/Linking refpersys/{
    doecho=1;
    next;
}

/Linked refpersys/{
    doecho=0;
    next;
}

//{
    if (doecho) {
	lin=$0;
	w=length(lin);
	for (i=0; i<w; i++) {
	    #c=sprintf(lin[i],"%c");
	    c=substr(lin,i,1);
	    if (match (c, /[::alnum::]/)) {
		printf "'%c', ", c;
	    }
	    else {
		printf "'\\%#x', //%c\n", (c), c;
	    }
	}
	nbc++;
    };
    next;
}

END{
    printf "///done rps-generate-linking-command %d lines echoed %d\n", NR, nbc;
}
##
##    This internal script generates some C string data containing the
##    linking command.  It should be invoked by GNU make only.
##    

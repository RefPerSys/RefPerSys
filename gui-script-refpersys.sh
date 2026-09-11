#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# RefPerSys file gui-script-refpersys.sh - see refpersys.org
#
#
#
#      © Copyright (C) 2022 - 2025 The Reflective Persistent System
#      Team <http://refpersys.org>
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
if [ -f $HOME/refpersys-gui-config.sh ]; then
    source $HOME/refpersys-gui-config.sh
fi


## in june 2022 a possible GUI script might involve executable
## obtained from
## github.com/bstarynk/misc-basile/blob/master/fltk-mini-edit.cc near
## its commit fe1779bcc2
if [ ! -f "$REFPERSYS_GUI_SCRIPT" ]; then
    printf "%s: missing REFPERSYS_GUI_SCRIPT\n" $0 2>&1
    exit 1
fi
    
exec $REFPERSYS_GUI_SCRIPT

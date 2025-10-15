#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
#  © Copyright (C) 2025 - 2025 The Reflective Persistent System Team
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
./refpersys -AREPL --script=$(/usr/bin/realpath $0) --batch --run-name=006echo
exit $?

## for GDB use
## gdb --args ./refpersys -AREPL --script=$(/usr/bin/realpath test_dir/006echo.bash) --batch --run-name 005script
## magic string REFPERSYS_SCRIPT echo

(This is from the tragedy le Cid of Corneille, in 1637 so public domain)

Ô rage ! Ô désespoir ! Ô vieillesse ennemie !
N’ai-je donc tant vécu que pour cette infamie ?
Et ne suis-je blanchi dans les travaux guerriers
Que pour voir en un jour flétrir tant de lauriers ?
Mon bras, qu’avec respect toute l’Espagne admire,
Mon bras, qui tant de fois a sauvé cet empire,
Tant de fois affermi le trône de son roi,
Trahit donc ma querelle, et ne fait rien pour moi ?
Ô cruel souvenir de ma gloire passée !
Œuvre de tant de jours en un jour effacée !


For every alpha in the set of relative integers alpha plus zero is alpha
∀ α ∈ ℤ  α + 0 = α

# end of file test_dir/006echo.bash in refpersys

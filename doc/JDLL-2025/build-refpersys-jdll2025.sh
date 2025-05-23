#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# file doc/JDLL-2025/build-refpersys-jdll2025.sh

#
##      © Copyright (C) 2025 Basile STARYNKEVITCH
##

# in http://github.com/RefPerSys/RefPerSys/
cd $REFPERSYS_TOPDIR/doc/JDLL-2025
if ! [ -f jdll2025-RefPerSys-Starynkevitch.tex ]; then
    printf "%s: missing jdll2025-RefPerSys-Starynkevitch.tex in %s\n" \
	   $0 "$(/bin/pwd)"
    exit 1
fi
/bin/mv *.pdf /tmp 
for svgfile in *.svg ; do
    svgbase=$(basename $svgfile .svg)
    inkscape  --export-filename=$svgbase.pdf $svgfile
    inkscape  --export-filename=$svgbase.eps $svgfile
done
for dotfile in *.dot ; do
    dotbase=$(basename $dotfile .dot)
    dot -v -Teps -o $dotbase.eps  $dotfile
    dot -v -Tpdf -o $dotbase.pdf  $dotfile
    dot -v -Tsvg -o $dotbase.svg  $dotfile
done
lualatex --shell-escape  --halt-on-error jdll2025-RefPerSys-Starynkevitch
lualatex --shell-escape  --halt-on-error jdll2025-RefPerSys-Starynkevitch

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#% Local Variables: ;;
#% compile-command: "./build-refpersys-jdll2025.sh" ;;
#% End: ;;
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

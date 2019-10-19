#!/bin/bash -x
echo $0 starting in $(pwd)
rm -vf  refpersys-design.{aux,bcf,blg,log,run.xml,toc}
for svgfile in ../CC-BY-SA-icon.svg ; do
    svgbase=$(basename $svgfile .svg)
    inkscape --without-gui --export-pdf=$svgbase.pdf $svgfile
    inkscape --without-gui --export-eps=$svgbase.eps $svgfile
done
lualatex --shell-escape  --halt-on-error refpersys-design
biber refpersys-design
lualatex --shell-escape  --halt-on-error refpersys-design
[ -f refpersys-design.idx ] && texindy -v -C utf8 -I latex refpersys-design.idx
if lualatex --shell-escape  --halt-on-error refpersys-design ; then
    [ -x $HOME/bin/install-refpersys-design-report.sh ] && $HOME/bin/install-refpersys-design-report.sh
fi

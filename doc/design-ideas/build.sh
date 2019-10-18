#!/bin/bash -x
echo $0 starting in $(pwd)
rm -vf  refpersys-design.{aux,bcf,blg,log,run.xml,toc}
lualatex --shell-escape  --halt-on-error refpersys-design
biber refpersys-design
lualatex --shell-escape  --halt-on-error refpersys-design
[ -f refpersys-design.idx ] && texindy -v -C utf8 -I latex refpersys-design.idx
if lualatex --shell-escape  --halt-on-error refpersys-design ; then
    [ -x $HOME/bin/install-refpersys-design-report.sh ] && $HOME/bin/install-refpersys-design-report.sh
fi

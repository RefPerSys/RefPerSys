#!/bin/bash -x
echo $0 starting in $(pwd)
lualatex --shell-escape  --halt-on-error refpersys-design
biber refpersys-design
lualatex --shell-escape  --halt-on-error refpersys-design
[ -f refpersys-design.idx ] && texindy -v -C utf8 -I latex refpersys-design.idx
lualatex --shell-escape  --halt-on-error refpersys-design

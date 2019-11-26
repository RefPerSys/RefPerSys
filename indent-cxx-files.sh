#!/bin/bash
# RefPerSys file indent-cxx-files.sh -- see refpersys.org
# see https://unix.stackexchange.com/a/122848/50557
for f in "$@"; do
    cp -a $f $f~%
    ${ASTYLE:-astyle} ${ASTYLEFLAGS:- -v -s2 --style=gnu} $f
done

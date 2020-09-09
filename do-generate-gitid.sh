#!/bin/bash
# RefPerSys file generate-gitid.sh - see refpersys.org
###
# it just emits a string with the full git commit id, appending + if
# the git status is not clean.
if git status|grep -q 'nothing to commit' ; then
    endgitid=''
else
    endgitid='+'
fi

if [ "$1" = "-s" ]; then
    printf "%.16s%s\n" $(git log --format=oneline -q -1 | cut '-d '  -f1 | tr -d '\n') $endgitid     
else
    (git log --format=oneline -q -1 | cut '-d '  -f1 | tr -d '\n';
     echo $endgitid)
fi

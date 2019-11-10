#!/bin/bash
# RefPerSys file IDE/generate-gitid.sh

if git status|grep -q 'nothing to commit' ; then
    endgitid=''
else
    endgitid='+'
fi
(git log --format=oneline -q -1 | cut '-d '  -f1 | tr -d '\n';
     echo $endgitid)  

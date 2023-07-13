#!/bin/bash
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8
printf "// generated Python file %s by %s -- DONT EDIT - see refpersys.org\n" $1 $0

date +"RPSPY_TIMESTAMP=\"%c\"%nRPSPY_TIMELONG=%s;"
printf "RPSPY_TOPDIRECTORY=\"%s\";\n" $(realpath $(pwd))

if git status|grep -q 'nothing to commit' ; then
    endgitid='";'
else
    endgitid='+";'
fi
(echo -n 'RPSPY_GITID="'; 
 git log --format=oneline -q -1 | cut '-d '  -f1 | tr -d '\n';
     echo $endgitid)  

(echo -n 'RPSPY_LASTGITTAG="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"\\\\'; echo '";')

(echo -n 'RPSPY_LASTGITCOMMIT="' ; \
 git log --format=oneline --abbrev=12 --abbrev-commit -q  \
     | head -1 | tr -d '\n\r\f\"\\\\' ; \
 echo '";') 

(echo -n 'RPSPY_MD5SUM="' ; cat $(tar tf /tmp/refpersys-$$.tar.gz | grep -v '/$') | md5sum | tr -d '\n -'  ;  echo '";')

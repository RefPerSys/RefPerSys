#!/bin/bash
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8
printf "// generated file %s -- DONT EDIT - see refpersys.org\n" $1
date +"const char rps_timestamp[]=\"%c\";%nconst unsigned long rps_timelong=%sL;"
printf "const char rps_topdirectory[]=\"%s\";\n" $(realpath $(pwd))

if git status|grep -q 'nothing to commit' ; then
    endgitid='";'
else
    endgitid='+";'
fi
(echo -n 'const char rps_gitid[]="'; 
 git log --format=oneline -q -1 | cut '-d '  -f1 | tr -d '\n';
     echo $endgitid)  

(echo -n 'const char rps_lastgittag[]="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"\\\\'; echo '";')

(echo -n 'const char rps_lastgitcommit[]="' ; \
 git log --format=oneline --abbrev=12 --abbrev-commit -q  \
     | head -1 | tr -d '\n\r\f\"\\\\' ; \
 echo '";') 

git archive -o /tmp/refpersys-$$.tar.gz HEAD 
trap "/bin/rm /tmp/refpersys-$$.tar.gz" EXIT INT 

cp -va /tmp/refpersys-$$.tar.gz $HOME/tmp/refpersys.tar.gz >& /dev/stderr

(echo -n 'const char rps_md5sum[]="' ; cat $(tar tf /tmp/refpersys-$$.tar.gz | grep -v '/$') | md5sum | tr -d '\n -'  ;  echo '";')

(echo  'const char*const rps_files[]= {' ; tar tf /tmp/refpersys-$$.tar.gz | grep -v '/$' | tr -s " \n"  | sed 's/^\(.*\)$/ "\1\",/';  echo ' (const char*)0} ;')

(echo  'const char*const rps_subdirectories[]= {' ; tar tf /tmp/refpersys-$$.tar.gz | grep  '/$' | tr -s " \n"  | sed 's/^\(.*\)$/ "\1\",/';  echo ' (const char*)0} ;')

printf "const char rps_makefile[]=\"%s\";\n"   $(realpath Makefile)

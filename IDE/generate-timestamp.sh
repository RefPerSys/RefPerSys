#!/bin/bash
printf "// generated file %s -- DONT EDIT\n" $1
date +"const char rpside_date[]=\"%c\";%nconst unsigned long rpside_timelong=%sL;%n"
printf "const char rpside_directory[]=\"%s\";\n" $(pwd)
printf "const char rps_directory[]=\"%s\";\n" $(cd ..; pwd)

if git status|grep -q 'nothing to commit' ; then
    endgitid='";'
else
    endgitid='+";'
fi
(echo -n 'const char rpside_gitid[]="'; 
 git log --format=oneline -q -1 | cut '-d '  -f1 | tr -d '\n';
     echo $endgitid)  

(echo -n 'const char rpside_lastgittag[]="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"\\\\'; echo '";')

(echo -n 'const char rpside_lastgitcommit[]="' ; \
 git log --format=oneline --abbrev=12 --abbrev-commit -q  \
     | head -1 | tr -d '\n\r\f\"\\\\' ; \
 echo '";') 

printf "const char rpside_makefile[]=\"%s\";\n"   $(realpath Makefile)

#!/bin/bash
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8
printf "// generated C file %s by %s -- DONT EDIT - see refpersys.org\n" $1 $0
printf "// to restore %s from a  %%%%__timestamp.c%%%% file edit there lines with @REFPERSYS_HOME.\n" $1
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

printf "const char rps_makefile[]=\"%s\";\n"   $(realpath GNUmakefile)

printf "const char rps_gui_script_executable[]=\"%s\";\n" $(realpath gui-script-refpersys.sh)

printf "const char rps_building_user_name[]=\"%s\";\n" "$(git config user.name)"

printf "const char rps_building_user_email[]=\"%s\";\n" $(git config user.email)

printf "const char rps_building_host[]=\"%s\";\n" $(/bin/uname -n)

printf "const char rps_building_operating_system[]=\"%s\";\n" $(/bin/uname -o)

printf "const char rps_building_opersysname[]=\"%s\";\n" $(/bin/uname -o | /bin/sed 's/[^A-Za-z0-9]/_/')

printf "const char rps_building_machine[]=\"%s\";\n" $(/bin/uname -m)

printf "const char rps_building_machname[]=\"%s\";\n" $(/bin/uname -m | /bin/sed 's/[^A-Za-z0-9]/_/')

printf "/// see also GNUmakefile for refpersys.org;\n"
### some things are generated in GNUmakefile 

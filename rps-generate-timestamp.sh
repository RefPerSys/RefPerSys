#!/bin/bash

#
# Copyright 2020 - 2024, Basile Starynkevitch and the forum@refpersys.org
# mailing list contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This file is part of the Reflexive Persistent System (aka RefPerSys);
# it just emits a string with the full git commit id, appending + if
# the git status is not clean.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

##
##    This internal script generates some C file containing timestamp
##    related constants for Linux related to the RefPerSys inference
##    engine free software project on refpersys.org.  It should be
##    invoked by GNU make only.
##    

if [ -z $GPP ]; then
    GPP=$(which gpp)
    if [ -z $GPP ]; then
	printf "%s missing gpp\n" $0 2>&1
	exit 1
    fi
    export GPP
fi
printf "/// invocation: %s %s in %s\n" $0 "$*" "$(realpath $(pwd))"
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8
printf "// generated C file %s by %s -- DONT EDIT - see refpersys.org\n" $1 $0
printf "// to restore %s from a  %%%%__timestamp.c%%%% file edit there lines with @REFPERSYS_HOME.\n" $1

printf "// environment is:\n"
printenv | /bin/sed 's:^://~ :'
printf "\n\n"
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

printf 'const char rps_shortgitid[] = "%s";\n' "$(./rps-generate-gitid.sh -s)"

printf 'const char rps_gitbranch[] = "%s";\n' "$(git branch --show-current)"

(echo -n 'const char rps_lastgittag[]="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"\\\\'; echo '";')

(echo -n 'const char rps_lastgitcommit[]="' ; \
 git log --format=oneline --abbrev=12 --abbrev-commit -q  \
     | head -1 | tr -d '\n\r\f\"\\\\' ; \
 echo '";') 

git archive -o /tmp/refpersys-$$.tar.gz HEAD 
trap "/bin/rm /tmp/refpersys-$$.tar.gz /tmp/refpersys-$$.toc" EXIT INT 

cp -va /tmp/refpersys-$$.tar.gz $HOME/tmp/refpersys.tar.gz >& /dev/stderr

/bin/tar tf /tmp/refpersys-$$.tar.gz | /bin/grep -v 'attic' >  /tmp/refpersys-$$.toc

printf "\n/// refpersys.toc:\n"
/bin/sed s:^:///:g /tmp/refpersys-$$.toc
printf "//// end refpersys.toc\n"

(echo -n 'const char rps_md5sum[]="' ; cat $(/bin/grep -v '/$' /tmp/refpersys-$$.toc ) | /usr/bin/md5sum | /usr/bin/tr -d '\n -'  ;  echo '";')

(echo  'const char*const rps_files[]= {'; /bin/egrep -v '/$\|attic/' /tmp/refpersys-$$.toc  | /usr/bin/tr -s " \n"  | /usr/bin/sed 's/^\(.*\)$/ "\1\",/';  echo ' (const char*)0} ;')

(echo  'const char*const rps_subdirectories[]= {' ; /bin/grep  '/$' /tmp/refpersys-$$.toc | tr -s " \n"  | sed 's/^\(.*\)$/ "\1\",/';  echo ' (const char*)0} ;')

printf "const char rps_gnumakefile[]=\"%s\";\n"   $(realpath GNUmakefile)

printf "const char rps_gnu_make[]=\"%s\";\n" $(/bin/which gmake)

printf "const char rps_gnu_make_version[]=\"%s\";\n" "$(gmake --version | /bin/head -1)"

printf "const char rps_gnu_bison[]=\"%s\";\n" $(/bin/which bison)

printf "const char rps_gnu_bison_version[]=\"%s\";\n" "$(bison --version | /bin/head -1)"

printf "const char rps_gui_script_executable[]=\"%s\";\n" $(realpath gui-script-refpersys.sh)

printf "const char rps_building_user_name[]=\"%s\";\n" "$(git config user.name)"

printf "const char rps_building_user_email[]=\"%s\";\n" $(git config user.email)

printf "const char rps_building_host[]=\"%s\";\n" $(/bin/uname -n)

printf "const char rps_building_operating_system[]=\"%s\";\n" $(/bin/uname -o)

printf "const char rps_building_opersysname[]=\"%s\";\n" $(/bin/uname -o | /bin/sed 's/[^A-Za-z0-9]/_/')

printf "const char rps_building_machine[]=\"%s\";\n" $(/bin/uname -m)

printf "const char rps_building_machname[]=\"%s\";\n" $(/bin/uname -m | /bin/sed 's/[^A-Za-z0-9]/_/')

printf "const char rps_plugin_builder[]=\"%s\";\n" $(realpath do-build-refpersys-plugin)

printf "const char rps_cxx_compiler_realpath[]=\"%s\";\n" $(realpath $CXX)

printf "const char rps_cxx_compiler_version[]=\"%s\";\n" "$($CXX --version | /bin/head -1)"

printf "const char rps_cxx_compiler_flags[]=\"%s\";\n" "$CXXFLAGS"

printf "const char rps_gpp_preprocessor_command[]=\"%s\";\n" $GPP

printf "const char rps_gpp_preprocessor_realpath[]=\"%s\";\n" $(realpath $GPP)

printf "const char rps_gpp_preprocessor_version[]=\"%s\";\n" "$($GPP --version | /bin/head -1)"

printf "const char rps_ninja_builder[]=\"%s\";\n" "$REFPERSYS_NINJA"

printf "const char rps_ninja_version[]=\"%s\";\n" "$($REFPERSYS_NINJA --version)"

printf "/// see also GNUmakefile in %s for refpersys.org;\n" $PWD

printf "/// this is generated by %s in %s git %s\n" $0 $PWD $(./rps-generate-gitid.sh -s)
## end of file rps-generate-timestamp.sh

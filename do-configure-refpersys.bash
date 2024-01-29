#!/bin/bash
##
##      © Copyright 2024 The Reflective Persistent System Team
##
## License:
##
##    This do-configure-refpersys.bash script is free software: you
##    can redistribute it and/or modify it under the terms of the GNU
##    General Public License as published by the Free Software
##    Foundation, either version 3 of the License, or (at your option)
##    any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <http://www.gnu.org/licenses/>

script_name=$0
declare -a files_to_remove
## first step: ask for a C compiler

function try_c_compiler() { # $1 is the C compiler to try
    # first step, compile a simple hello world program
    local csrc textexe
    csrc=/tmp/helloworld$$.c
    testexe=/tmp/helloworld$$.bin
    echo '/// sample helloworld.c' > $csrc
    printf '#include <stdio.h>\n' >> $csrc
    printf 'int main(int argc, char**argv) {\n' >> $csrc
    printf '  printf("hello from %%s HERE %%s\\n", argv[0], HERE);\n' >> $csrc
    printf '  return 0;\n' >> $csrc
    printf '}\n /// eof helloworld.c\n' >> $csrc
    $1 -DHERE=\"$script_name\" -O -g -o $testexe $csrc
    if [ $? -ne 0 ]; then
	printf '%s: failed to compile %s with %s\n' $script_name $csrc $1 > /dev/stderr
	exit 1
    fi
    files_to_remove+=$csrc
    files_to_remove+=$testexe
}
    
function ask_c_compiler() {
    local c rc
    echo Enter path of C99 compiler "preferably GCC"
    read -e -i /usr/bin/gcc -p "C compiler: " c
    echo Using $c as the C99 compiler
    $c --version
    try_c_compiler $c
}


## second step: ask for a C++ compiler
#function ask_cpp_compiler() {
#}


ask_c_compiler

echo $0 should remove $files_to_remove
for f in $files_to_remove ; do
    /bin/rm -vf $f
done

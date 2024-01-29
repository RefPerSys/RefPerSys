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
## first step: ask for a C compiler

function try_c_compiler() { # $1 is the C compiler to try
    # first step, compile a simple hello world program
    echo '/// sample helloworld.c' > helloworld.c
    printf '#include <stdio.h>\n' >> helloworld.c
    printf 'int main(int argc, char**argv) {\n' >> helloworld.c
    printf '  printf("hello from %%s HERE %%s\\n", argv[0], HERE);\n' >> helloworld.c
    printf '  return 0;\n' >> helloworld.c
    printf '}\n /// eof helloworld.c\n' >> helloworld.c
    $1 -DHERE=\"$script_name\" -O -g -o helloworld.test helloworld.c
}
    
function ask_c_compiler() {
    local c rc
    echo Enter path of C99 compiler "preferably GCC"
    read -e -i /usr/bin/gcc -p "C compiler:" c
    rc=$(realpath $c)
    echo Using $rc as the C99 compiler
    try_c_compiler $rc
}


## second step: ask for a C++ compiler
#function ask_cpp_compiler() {
#}


ask_c_compiler

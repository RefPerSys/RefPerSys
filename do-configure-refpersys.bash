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
    local csrc othercsrc textexe
    csrc=$(/usr/bin/mktemp tmp_helloworld.XXXXX.c)
    testexe=$(/usr/bin/mktemp --dry-run tmp_helloworld.XXXXX.bin)
    printf '/// sample helloworld C program %s from %s\n' $csrc $script_name >> $csrc
    printf '#include <stdio.h>\n' >> $csrc
    printf 'int main(int argc, char**argv) {\n' >> $csrc
    printf '  printf("hello from %%s HERE %%s\\n", argv[0], HERE);\n' >> $csrc
    printf '  return 0;\n' >> $csrc
    printf '}\n /// eof %s\n' $csrc >> $csrc
    echo $0 running $1 -DHERE=\"$script_name\" -O -g -o $testexe $csrc
    $1 -DHERE=\"$script_name\" -O -g -o $testexe $csrc
    if [ $? -ne 0 ]; then
	printf '%s: failed to C compile %s into %s with %s\n' $script_name $csrc $testexe $1 > /dev/stderr
	exit 1
    fi
    files_to_remove+=($csrc $testexe)
    #second step, compile a two files hello world
    csrc=$(/usr/bin/mktemp tmp_otherfirsthelloworld.XXXXX.c)
    othercsrc=$(/usr/bin/mktemp tmp_othersecondhelloworld.XXXXX.c)
    otherexe=$(/usr/bin/mktemp tmp_otherhelloworld.XXXXX.bin)
    firstobj=$(/usr/bin/basename $csrc .c).o
    otherobj=$(/usr/bin/basename $othercsrc .c).o
    printf '/// sample C program %s from %s\n' $csrc $script_name >> $csrc
    printf '#include <stdio.h>\n' >> $csrc
    printf 'void sayhello(const char*c) {\n' >> $csrc
    printf '     printf("hello from %%s HERE %%s\\n", c, HERE);\n' >> $csrc
    printf '}\n // end sayhello\n/// eof %s\n' $csrc >> $csrc
    printf '/// sample main C file %s from %s\n' $othercsrc $script_name >> $othercsrc
    printf 'extern void sayhello(const char*c);\n' >> $othercsrc
    printf 'int main(int argc,char**argv) {\n' >> $othercsrc
    printf '   if (argc>0) sayhello(argv[0]);\n' >> $othercsrc
    printf '   return 0;\n' >> $othercsrc >> $othercsrc
    printf '}\n /// end main\n///// eof %s\n' $othercsrc >> $othercsrc
    echo $script_name running $1 -DHERE=\"$script_name\" -O -g -Wall -c $csrc
    $1 -DHERE=\"$script_name\" -O -g -Wall -c $csrc
    if [ $? -ne 0 ]; then
	printf '%s: failed to C compile %s into object file with %s\n' $script_name $csrc $1 > /dev/stderr
	exit 1
    fi
    echo $script_name running $1 -DHERE=\"$script_name\" -O -g -Wall  -c $othercsrc
    $1 -DHERE=\"$script_name\" -O -g -Wall  -c $othercsrc
    if [ $? -ne 0 ]; then
	printf '%s: failed to C compile %s into object file with %s\n' $script_name $othercsrc $1 > /dev/stderr
	exit 1
    fi
    files_to_remove+=($csrc $othercsrc)
    echo $script_name running $1 -O -g $firstobj $otherobj -o $otherexe
    $1 -O -g $firstobj $otherobj -o $otherexe
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

function try_cplusplus_compiler { # $1 is the C++ compiler to try
    # first step, compile a simple hello world program
    local cpsrc textexe
    cpsrc=$(/usr/bin/mktemp tmp_helloworldcxx.XXXXX.cc)
    testexe=$(/usr/bin/mktemp --dry-run tmp_helloworldcxx.XXXXX.bin)
    printf '/// sample helloworld C++ program %s from %s\n' $cpsrc $script_name >> $cpsrc
    printf '#include <iostream>\n' >> $cpsrc
    printf 'int main(int argc, char**argv) {\n' >> $cpsrc
    printf '  std::out << "hello from " << argv[0]\n' >> $cpsrc
    printf '    << "  HERE=" << HERE << std::endl;\n' >> $cpsrc
    printf '  return 0;\n' >> $cpsrc
    printf '}\n /// eof %s\n' $cpsrc >> $cpsrc
    $1 -DHERE=\"$script_name\" -O -g -o $testexe $cpsrc
    if [ $? -ne 0 ]; then
	printf '%s: failed to C++ compile %s into %s with %s\n' $script_name $csrc $testexe $1 > /dev/stderr
	exit 1
    fi
    files_to_remove+=($cpsrc $testexe)
}

#function ask_cpp_compiler() {
#}


ask_c_compiler

### TODO: not working so well in commit 8f0f3c2715a8 (Jan 29, 2024)
echo $0 should remove ${#files_to_remove[@]} files from ${files_to_remove}
for f in ${files_to_remove} ; do
    /bin/rm -vf $f
done

# RefPerSys REPL

## Background

The initial idea for the creation of a user interface for RefPerSys involved the
development of a GUI built with the Qt 5 framework. However, experience showed
that integration with the Qt 5 framework was very challenging, and would require
much more in-depth focus on the internals of Qt than is desirable.

As a consequence, we started looking for other options on developing the
RefPerSys. A GUI is now being developed using the Fast Light Toolkit (FLTK),
while in tandem we are also considering the development of a command line read
eval print loop (REPL).

The RefPerSys REPL aims to be a "mirror" of the GUI on the command line. Of
course, instead of traditional GUI actions and conventions, we will be need to
type out commands on the REPL. The syntax of these commands is LISP-like and
inspired by the Guile REPL.

## List of Commands

  * `(help [cmd])` - displays general help for for a given `cmd`
  * `(version)` - displays the current RefPerSys version information
  * `(license)` - displays the license details of RefPerSys
  * `(syslog [priority])` - shows RefPerSys `syslog` entries for an optional
    priority
  * `(garbage-collect)` - invokes the RefPerSys garbage collector
  * `(oid-random)` - generates a random object ID
  * `(state-load [dir])` - loads the persistent heap from `dir` (or current
    working directory)
  * `(state-dump [dir])` - saves the persistent heap to `dir` directory (or
    current working directory)
  * `(object-inspect oid [prop])` - shows the details of an object with a given
    `oid`, or only for a specific property
  * `(class-create)` - creates a new class
  * `(symbol-create)` - creates a new symbol
  * `(named-instance-create)` - creates a new named instance
  * `(contributor-create)` - creates a new contributor
  * `(plugin-create)` - creates a new plugin

## Example Usage

Corresponding to the commands listed above, here are examples of how such
commands would be used in the REPL.

### (help [cmd])
  
  * `(help)` - displays general help message
  * `(help version)` - displays help message for the `(version)` command
  * `(help license)` - displays help message for the `(license)` command
  * `(help help)` - displays help message for the `(help)` command
  * `(help object-inspect)` - displays help message for the `(object-inspect)`
    command

### (version)

  * `(vesrsion)` - displays version message, with an output similar to that
    provided by invoking `./repfersys --version`.

    > RefPerSys, an Artificial Intelligence system - work in progress...
    > version information:
    > program name: ./refpersys
    > build time: Wednesday 15 July 2020 09:51:35 AM IST
    > top directory: /home/abhishek/work/bstarynk/refpersys
    > git id: ba10b056a074c420d894db21b540e392b9b41fb2+
    > last git tag: heads/fltk-branch
    > last git commit: ba10b056a074 adding end_of_main symbol for backtracking
    > purposes
    > md5sum of 62 source files: d38507e6abe83ac635e1d62074d39514
    > with 7 subdirectories.
    > GNU glibc 2.30
    > Graphical User Interface using FLTK git ba10b056a074c420d894db21, ABI:10400,
    > API:10400
    > Read Eval Print Loop using REPL git ba10b056a074c420d894db2 ReadLine 8.0
    > libCURL for web using CURL git ba10b056a074c420d894db21b540e392b9b41fb2;
    > libcurl/7.68.0 OpenSSL/1.1.1g zlib/1.2.11 brotli/1.0.7 libidn2/2.3.0
    > libpsl/0.21.0 (+libidn2/2.3.0) libssh2/1.8.0 nghttp2/1.40.0 librtmp/2.3
    > made with: /home/abhishek/work/bstarynk/refpersys/Makefile
    > running on refpersys in /home/abhishek/work/bstarynk/refpersys
    > C++ compiler: g++ (Debian 9.3.0-12) 9.3.0
    > free software license: GPLv3+, see https://gnu.org/licenses/gpl.html
    > +++++ there is no WARRANTY, to the extent permitted by law ++++
    > ***** see also http://refpersys.org/ *****


### (license)
TODO

### (syslog [priority])
TODO

### (garbage-collect)
TODO

### (oid-random)
TODO

### (state-load [dir])
TODO

### (state-dump [dir])
TODO

### (object=inspect oid [prop])
TODO

### (class-create)
TODO

### (symbol-create)
TODO

### (named-instance-create)
TODO

### (contributor-create)
TODO

### (plugin-create)
TODO

## EBNF Grammar
TODO

command := help | version | license | syslog | garbage-collect | oid-random
	| state-load | state-dump | object-inspect | class-create
	| symbol-create | named-instance-create | contributor-create
	| plugin-create ;

help := "(", "help", [opt-help-cmd], ")" ;

opt-help-cmd := "help" | "version" | "license" | "syslog" 
	| "garbage-collect" | "oid-random" | "state-load" | "state-dump"
	| "object-inspect" | "class-create" | "symbol-create"
	| "named-instance-create" | "contributor-create" | "plugin-create" ;

version := "(", "version", ")" ;

license := "(", "license", ")" ;

syslog := "(", "syslog", [opt-syslog-priority], ")" ;

opt-syslog-priority := "emerg" | "alert" | "crit" | "err" | "warning"
	| "notice" | "info" | "debug" ;

garbage-collect := "(", "garbage-collect", ")" ;

oid-random := "(", "oid-random", ")" ;

state-load := "(", "state-load", [opt-state-dir], ")" ;

state-dump := "(", "state-dump", [opt-state-dir], ")" ;

opt-state-dir := ? valid Linux file path ? ;

object-inspect := "(", "object-inspect", oid, [opt-object-prop], ")" ;

class-create := "(", oid, ")";

symbol-create := "(", oid, [opt-symbol-name], ")";

contributor-create := "(", contributor-name, contributor-email,
	[opt-contributor-url], ")";

oid := "_", { letter | digit };

letter := "A" | "B" | "C" | "D" | "E" | "F" | "G"
       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
       | "V" | "W" | "X" | "Y" | "Z" | "a" | "b"
       | "c" | "d" | "e" | "f" | "g" | "h" | "i"
       | "j" | "k" | "l" | "m" | "n" | "o" | "p"
       | "q" | "r" | "s" | "t" | "u" | "v" | "w"
       | "x" | "y" | "z" ;

digit := "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

## Implementation

TODO


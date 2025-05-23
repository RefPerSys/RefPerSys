# RefPerSys inference engine


This [free software](https://en.wikipedia.org/wiki/Free_software)
project has its source code on https://github.com/RefPerSys/RefPerSys
with an obsolete variant on https://gitlab.com/bstarynk/refpersys/
which might not maintained in 2024. It is its own web site on
http://refpersys.org/ where more details are given.

*RefPerSys* is aiming to become a free software symbolic artificial
intelligence system or [inference
engine](https://en.wikipedia.org/wiki/Inference_engine), e.g. a better
alternative to [CLIPSrules](https://clipsrules.net/).

Contributions to *RefPerSys* are welcome. Contact by email
[Basile STARYNKEVITCH](http://starynkevitch.net/Basile/) (near Paris, France)
at `basile@starynkevitch.net` and `b.starynkevitch@gmail.com`
or Abhishek Chakravarti (Kolkata, India) at `chakravarti.avishek@gmail.com`.
Basile can also be contacted
by snail mail (8 rue de la Faïencerie, 92340 Bourg-la-Reine, France)
or (on French office hours) by Whatapp to +33 6 8501 followed by
the four digits product of seven and 337.

There used to have several variants of [RefPerSys](http://refpersys.org/).
But unless we get help (including funding) and contributors we focus on [RefPerSys in C++](https://github.com/RefPerSys/RefPerSys/)
since commit 559ea329f46a (2024, Nov, 20).

[HorizonEurope](https://ec.europa.eu/info/funding-tenders/opportunities/portal/screen/programmes/horizon)
or [ITEA](https://itea4.org/) consortia interested by
[RefPerSys](http://refpersys.org/) are welcome to contact us. Likewise for students or academics or passionate open source developers 
wanting to contribute.

## A research project

The Reflective Persistent System language is a **research project**,
taking many good ideas from
[Bismon](http://github.com/bstarynk/bismon), sharing a lot of goals
(*except* static source code analysis) with it but avoiding bad ideas
from it.

For Linux/x86-64 only, unless someone provides us `ssh` remote access
to some other Linux 64 bits system. Don't even think of running that
on non-Linux systems, unless you provide patches for that. And we need
a 64 bits processor. If you can give us `ssh` access to non x86-64
sixty-four bits Linux machines (multi-core, at least 32Gbytes of RAM,
at least 128Gbytes of disk space, with [GCC](https://gcc.gnu.org/)
installed for C and C++) please contact us.

We have multi-threading in mind, but in some limited way. We think of
a pool of a few dozen Pthreads at most (but not of a thousand
Pthreads).

We absolutely want to avoid any
[GIL](https://en.wikipedia.org/wiki/Global_interpreter_lock)

Don't expect anything useful from RefPerSys before at least 2024. But
you could have fun sharing our ideas and experimenting yours.

A rewrite of RefPerSys in C was attempted on [refpersys-in-c](https://github.com/RefPerSys/refpersys-in-c) but is abandonned 
in commit 559ea329f46a28 (nov. 2024).

We considered previously to use the garbage collector from [Ravenbrook
MPS](https://github.com/Ravenbrook/mps).

**Don't expect RefPerSys to be a mature project.** It is not in October 2024.

## environment variables

Your `$EDITOR` should give a valid editor path (e.g. `/usr/bin/emacsclient` or `/usr/bin/gedit` or `/usr/bin/vim` ...)

Your `$HOME` should be valid.

Your `$PATH` should be valid and give access to your `refpersys` and
(when relevant) `lto-refpersys` executables. A possibility could be to
have (or create) your `$HOME/bin/` directory (appearing in your
`$PATH`) and symbolic links inside it. Your `$PATH` should give access
to [GNU make](https://www.gnu.org/software/make/) as `make` (a simpler
make utility, including BSD make, is not ok). the
[`git`](http://git-scm.com/) version control command should be
accessible and likewise for other [GNU](https://www.gnu.org/software/)
utilities.

The GNU `make` software should have version at least 4 and be
configured to use [GNU
guile](https://www.gnu.org/software/guile/). See [GNU make bug
#66658](https://savannah.gnu.org/bugs/index.php?66658).

Conventionally any *RefPerSys* specific environment variable (outside
of the usual environment variables mentioned above) is starting with
`REFPERSYS_` .... Some of them are set by the `refpersys` executable before forking any command:


**The `$REFPERSYS_TOPDIR` should contain the path of the RefPerSys
source directory**. So if you did some command like `git clone
https://github.com/RefPerSys/RefPerSys` in your `$HOME/work/`
directory you need to `export REFPERSYS_TOPDIR=$HOME/work/RefPerSys`
(probably in some of your ̀`$HOME/.bashrc` or `$HOME/.zshrc` or
`$HOME/.zshenv` shell files) ...

### internally set environment variables.

When the `refpersys` executable is forking a process or running a
command, the following environment variables have been set (by its
`rps_extend_env` C++ function):

* `REFPERSYS_TOPDIR` to the top directory e.g. `$HOME/work/RefPerSys`
* `REFPERSYS_RUN_NAME` to the run name (if given with `--run-name=NAME`)
* `REFPERSYS_GITID` to the complete git id
  (e.g. `d65f8c47aed61d31454b3612a33a30a308660d31+`).<br/>
  The suffix `+` indicates that your *RefPerSys source code*
  has been *modified locally*
  and is newer that what you are reading on this
  https://github.com/RefPerSys/RefPerSys webpage.
* `REFPERSYS_PID` to the process id e.g. `1653963`
* `REFPERSYS_SHORTGITID` to a shortened git id e.g. `d65f8c47aed6+`


## design ideas

Some draft design ideas are written in the [RefPerSys design
draft](http://starynkevitch.net/Basile/refpersys-design.pdf) which is
very incomplete **work in progress**.

If you happen to know about any research call for proposals or funding
opportunities e.g. thru some [HorizonEurope
consortium](https://research-and-innovation.ec.europa.eu/funding/funding-opportunities/funding-programmes-and-open-calls/horizon-europe_en)
in Europe (Euro zone) about this (e.g. related to [artificial
intelligence](https://en.wikipedia.org/wiki/Artificial_intelligence)
goals) and [open source](https://en.wikipedia.org/wiki/Open_source)
please mention them to [Basile
Starynkevitch](http://starynkevitch.net/Basile/) (France) by email to
`basile@starynkevitch.net` (personal email) or by snail mail, e.g. postcards are also welcome.

## persistent values

Like *Bismon*, RefPerSys is managing an evolving,
[persistable](https://en.wikipedia.org/wiki/Persistence_(computer_science)),
[heap](https://en.wikipedia.org/wiki/Memory_management#HEAP) of
[dynamically](https://en.wikipedia.org/wiki/Dynamic_programming_language)
typed,
[garbage-collected](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)),
values, exactly like Bismon does (see *§2 Data and its persistence in
Bismon* of the [Bismon draft
report](http://starynkevitch.net/Basile/bismon-chariot-doc.pdf)...).
The
[semantics](https://en.wikipedia.org/wiki/Semantics_(computer_science))
-but *not* the syntax- of values is on purpose close to those of
[Lisp](https://en.wikipedia.org/wiki/Lisp_(programming_language)),
Python, Scheme, JavaScript, Go, or even Java, etc....  Most of these
RefPerSys values are
[immutable](https://en.wikipedia.org/wiki/Immutable_object); for
example boxed strings, sets -with [dichotomic
search](https://en.wikipedia.org/wiki/Dichotomic_search) inside them-
or tuples of references to objects,
[closures](https://en.wikipedia.org/wiki/Closure_(computer_programming)),
etc ...- But some of these RefPerSys values are *mutable* objects, and
by *convention* every mutable value is called an *object*. Each
mutable object has its own lock, and any access or update of mutable
data inside objects is generally made under its lock. By exception,
some very few, and very often accessed, mutable fields inside objects
(e.g. their class) are
[atomic](https://en.cppreference.com/w/cpp/atomic) pointers, for
performance reasons. Objects have (exactly like in Bismon) attributes,
components, and some optional payload. An attribute is an association
between an object (called the key of that attribute) and some
RefPerSys arbitrary non-nil value (called the value of that
attribute), and each object has its mutable associative table of
attributes. A component is an arbitrary RefPerSys value, and each
object has some mutable vector of them.  The payload is any additional
mutable data (e.g. a string buffer, an mutable vector or hashtable of
values, some class metadata, etc...), owned by the object. So the data
model of a RefPerSys object is as flexible as the data model of
JavaScript. However, RefPerSys objects have a *mutable* class defining
their *behavior* (not their fields, which are represented as
attributes) so used for [dynamic message
dispatching](https://en.wikipedia.org/wiki/Dynamic_dispatch).


## Code representation and generation

See separate [CODE-REPR.md](CODE-REPR.md) markup file.

## Worker threads and agenda of tasklets

*RefPerSys* will have a small *fixed* set of worker threads (perhaps a
dozen of them), each running some *agenda loop*; we would have some
central data structure (called *the agenda*, like in
[Bismon](http://github.com/bstarynk/bismon) (see §1.7 of the [Bismon
draft
report](http://starynkevitch.net/Basile/bismon-chariot-doc.pdf)...)
organizing runnable *tasklets* (e.g. a few FIFO queues of them). A
tasklet should conceptually run quickly (in a few milliseconds) and is
allowed to add or remove runnable tasklets (including itself) to the
agenda. Each worker thread is looping: fetching a runnable tasklet
from the agenda, then running that tasklet.

## License and copyright

This free software project is
[GPLv3+](https://www.gnu.org/licenses/gpl.html) licensed and
© copyrighted (C) 2019 - 2025 by the RefPerSys team, currently made of:

 *      Basile Starynkevitch <basile@starynkevitch.net>,
        8 rue de la Faïencerie
        92340 Bourg-la-Reine
        France
        homepage http://starynkevitch.net/Basile/
                near Paris, France. So usual timezone `TZ=MEST`

 *      Abhishek Chakravarti <ack@fifthestate.co.in>
        Kolkata, India

 *      Nimesh Neema <nimeshneema@gmail.com>
 
 *      Niklas Rozencrantz in Stockholm, Sweden.

Some files might be "borrowed" from other similar GPLv3+ licensed
projects (notably from [Bismon](http://github.com/bstarynk/bismon)...)
and could retain their original copyright owner.

## Contributing

Please ask, by email, the above RefPerSys team for C++ coding
conventions before starting non-trivial contributions to the C++
runtime of RefPerSys.  If you are contributing to its C++ runtime,
please **run `make clean` after any `git pull`**. You generally should run `make config` 

The GPLv3+ license of RefPerSys is unlikely to change before 2025 (and
probably even after).

## Generated output.

RefPerSys could be patched and extended to generate proprietary code
or data. In 2023 some authors (including Basile Starynkevitch) are not
interested to add such a features. Others authors (in India) are
interested to add that. Their contributions are pending (in
october 2024).

## File conventions

The RefPerSys runtime is implemented in C++17, with hand-written C++
code in `*_rps.cc`, and has a single C++ header file `refpersys.hh`.
We don't claim to be C++ gurus. Most C++ experts could write more
genuine C++ code than we do and will find our C++ code pityful. We
just want our runtime to work, not to serve as an example of well
written C++17 code.

The prefered C++ compiler (in 2023Q2) for *RefPerSys* is
[GCC](http://gcc.gnu.org/) version [13](https://gcc.gnu.org/gcc-13/) or
(preferably in 2024Q3) [14](https://gcc.gnu.org/gcc-14).

It could be worthwhile to sometimes compile *RefPerSys* with `clang++`
(see http://clang.llvm.org/ for more). In practice `make clean` then
`make RPS_BUILD_CXX=clang++`. The [Clang static
analyzer](https://clang-analyzer.llvm.org/) could be useful, but
expect a lot of warnings, since C++ dont have [flexible array
members](https://en.wikipedia.org/wiki/Flexible_array_member) but we
need something similar.

*RefPerSys* may later also use generated C++ code in some `_*.cc`
file, some generated C code in some `_*.c` and generated C or C++
headers in some `_*.h` files. By convention, files starting with an
underscore are generated (but they may, or not, being git
versioned). Some generated C++ files which are `git add`-ed are under
`generated/` subdirectory.

A *RefPerSys* generated C++ file should be generated from some
*RefPerSys* object (its generator).

We could need later some C++ generating program (maybe similar in
spirit to Bismon's
[BM_makeconst.cc](https://github.com/bstarynk/bismon/blob/master/BM_makeconst.cc). it
would then be named `rps_*` for the executable, and fits in a single
self-sufficient `rps_*.cc` C++ file. Perhaps we'll later have some
`rps_makeconst` executable to generate some C++, and its source in
some `rps_makeconst.cc`. So the convention is that any future C++
generating source code is in some `rps_*.cc` C++ file. In commit
`65a8f84aeffc9ba4e468` or newer the dumping facility is scanning
hand-written C++ source files to emit `generated/rps-constants.hh`

## Terminology and conventions

[RefPerSys](http://refpersys.org/) aims to become an
**[homoiconic](https://en.wikipedia.org/wiki/Homoiconicity) system** : we
hope to generate most of its C++ source code (under `generated/`
subdirectory), and explicitly represent the generated code as objects.

A *plugin* is some [shared object](https://en.wikipedia.org/wiki/Library_(computing)#Shared_libraries) 
(some `*.so` [ELF](https://wiki.osdev.org/ELF) file) loaded by 
[dlopen(3)](https://man7.org/linux/man-pages/man3/dlopen.3.html). 
The C++ code of a plugin is hand-written, with the hope of needing less and less of them 
(and have them replaced by automatically generated module code). We don't bother
[dlclose(3)](https://man7.org/linux/man-pages/man3/dlclose.3.html)-ing plugins.


A binary *module* is a shared object whose C++ code is generated by
[RefPerSys](http://refpersys.org/) at dump time. The generated C++
code is called the source *module* (unrelated to C++20 modules). It is
conventionally named `generated/_`*objid*`.cc`

Binary modules are conventionally named `generated/__rps_*.so`, see in
our `GNUmakefile` the line around comment `**generated binary modules`

See our C++ utility `do-build-refpersys-plugin.cc` to understand compiling
conventions in binary modules or plugins. In particular, if the first
fifty (50) lines of a generated C++ file contain - probably inside a
comment - @RPSCOMPILEFLAGS= that is used to compile the plugin. If
they contain @RPSLIBES= that is used to link the plugin.

## Building and dependencies.

A [transpiler](https://en.wikipedia.org/wiki/Source-to-source_compiler) for *RefPerSys* is being developed on https://github.com/bstarynk/misc-basile 
files `transpiler-refpersys.{cc,hh}`

The [build automation](https://en.wikipedia.org/wiki/Build_automation)
tool used here is [GNU make](https://www.gnu.org/software/make/) since
commit `6d56f50660c7cc41b9` (it was
[omake](https://github.com/ocaml-omake/omake) before).


The GNU [lightning](https://www.gnu.org/software/lightning/) library
is need for machine code generation. You may want to compile it from
source code and configure this *lightning* library with
```
./configure  --with-gnu-ld  --enable-disassembler   --enable-devel-disassembler \
  --enable-devel-get-jit-size   --disable-silent-rules CFLAGS='-O2 -g2'
```
then the usual build commands.

You should have compiled and installed Ian Taylor's
[libbacktrace](https://github.com/ianlancetaylor/libbacktrace),
e.g. under `/usr/local/`. You may need to add `/usr/local/lib/` in
your `/etc/ld.so.conf` and run `ldconfig -v -a` after installation of
that `libbacktrace`.

The [JsonCPP](https://github.com/open-source-parsers/jsoncpp/) and and
also a [mail](https://linux.die.net/man/1/mail) command in your
`$PATH`.

To install the dependencies on a recent [Debian](https://debian.org/) 12 *bookworm* or
[Ubuntu](https://ubuntu.com/) 22 system, you could run the following
steps

* `sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test` (for Ubuntu 20.04)
* `sudo apt install -y gcc-12 g++-12 clang-14 libc++-11-dev libc++abi-11-dev` (for Ubuntu 22.04)
* `sudo apt install libunistring-dev`
* `sudo apt install libjsoncpp-dev`
* `sudo apt-get install libssl-dev`
* `sudo apt install bisonc++ bisonc++-doc`
* `sudo apt install ccache g++ make build-essential remake gdb automake`
* `sudo apt install ttf-unifont ttf-mscorefonts-installer unifont msttcorefonts fonts-ubuntu fonts-tuffy fonts-spleen fonts-roboto fonts-recommended fonts-yanone-kaffeesatz fonts-play fonts-eurofurence fonts-ecolier-court fonts-dejavu fonts-croscore fonts-cegui fonts-inter fonts-inconsolata`
* `git clone https://github.com/ianlancetaylor/libbacktrace.git`
* `cd libbacktrace`
* `./configure`
* `make`
* `make install`

### Important simple functions

Some C++ code is important since shared between RefPerSys and the
`guifltk-refpersys` program.  In particular, the code of
`rps_compute_cstr_two_64bits_hash` should not be changed after
mid-september 2023.  It uses GNU libunistring and is shared (in file
`jsonrpsfltk.cc` of the `guifltk-refpersys`


### Related files

See also some files from
[misc-basile](https://github.com/bstarynk/misc-basile) in particular
`build-with-guile.c`

### Build instructions

You need a recent C99 compiler e.g. `gcc` (at least  [GCC
12](https://gcc.gnu.org/gcc-12/)). Check with `gcc --version`.

You need a recent C++17 compiler such as `g++` (We use 
[GCC 13](https://gcc.gnu.org/gcc-13/) or [GCC 14](https://gcc.gnu.org/gcc-14/)
and sometimes [`clang++`](http://clang.llvm.org/) whose warnings are different. 
Look into, and perhaps improve, our `GNUmakefile`. Check with `g++ --version`. Build using `make -j 3` or more.
 
You may need the [ninja](https://ninja-build.org/) build utility; in commit 559ea329f46a and before 
it is used by our `do-refpersys-build-plugin` utility.

You need the `glibmm` from GTK suite. And some `gtkmm` for some RefPerSys plugins.

You also should do a  `make clean` after any `git pull` and often redo a `make config`...


Your Unix environment should contain a `REFPERSYS_TOPDIR` shell
variable. The author has `env REFPERSYS_TOPDIR=$HOME/RefPerSys/` and
did `git clone` this into that `$REFPERSYS_TOPDIR`.

You need a Linux `pkg-config` utility.

You first need to `make config`. I recommend having the *GNU readline* library.

You then build with `make -j4 refpersys && make all`


### GUI libraries

Debugging them is painful (or my code using GUI libraries) because of
lack of DWARF debugging information in many GUI libraries (and even
for my bugs, it is easier to have GUI libraries compiled with `-O2
-g`).  If you are familiar with [FLTK](https://www.fltk.org/) and able
and allowed to download its source code and configure it using
`'./configure' 'CFLAGS=-O2 -fPIC -g -Wall -Wextra' 'CXXFLAGS=-O2 -g
-fPIC -Wall -Wextra' '--disable-static' '--enable-shared'
'--enable-debug' '--with-abiversion' '--with-optim=-g -O2 -fPIC'
'--sysconfdir=/etc/local/'` please send us an email.

## Garbage collection

*RefPerSys* is a multi-threaded and garbage-collected system. We are
fully aware that multi-thread friendly and efficient garbage
collection is a very difficult topic.

The reader unaware of garbage collection terminology (*precise*
vs. *conservative* GC, *tracing garbage collection*, *copying* GC, GC
roots, GC locals, *mark and sweep* GC, *incremental* GC, *write
barrier*) is advised to read the [GC handbook](http://gchandbook.org/)
and is expected to have read *very carefully* the [Tracing Garbage
Collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)
wikipage.

We have considered to use [Ravenbrook
MPS](https://www.ravenbrook.com/project/mps/). Unfortunately for us,
that very good GC implementation seems unmaintained, and with almost a
hundred thousand lines of code is very difficult to grasp, understand,
and adopt. Finally, using MPS is not reasonable in our eyes.

We also did consider using [Boehm
GC](http://www.hboehm.info/gc/). That conservative GC is really simple
to use (basically, use `GC_MALLOC` instead of `malloc`, etc...) and is
[C++ friendly](https://stackoverflow.com/a/8019003/841108). However,
it is rather slow (even for allocations of GC-ed zones, and we would
have *many* of them) and might be quite unsuitable for programs having
*lots* of [circular
references](https://en.wikipedia.org/wiki/Circular_reference), and
reflexive programs have lots of them.


### Garbage collection ideas

So we probably are heading towards developing our own *precise* and
multi-thread friendly GC (hopefully "better" than Boehm, but worse
than MPS), with the following ideas:

* *local roots* in the local frame are *explicit*, like in Bismon
  (`LOCALFRAME_BM` macro of
  [bismon/cmacros_BM.h](https://github.com/bstarynk/bismon/blob/master/cmacros_BM.h))
  or Ocaml (see its [§20.5 *Living in harmony with the garbage
  collector*](http://caml.inria.fr/pub/docs/manual-ocaml/intfc.html)
  and `CAMLlocal*` and `CAMLparam*` and `CAMLreturn*` macros). The
  local call frame is conventionally reified as the `_` local
  variable, so an [automatic
  variable](https://en.wikipedia.org/wiki/Automatic_variable) GC-ed
  pointer `foo` is coded `_f.foo` in our C++ runtime. A local frame in
  RefPerSys should be declared in C++ using `RPS_LOCALFRAME`.  By
  convention, and for readability, use `RPS_NULL_CALL_FRAME` in C++
  code when the caller frame argument of invocation of C++ macro
  `RPS_LOCALFRAME` is statically null, and
  `RPS_CALL_FRAME_UNDESCRIBED` when its descriptor is not given.
  

* our garbage collector manages *memory zones* inside a set of
  `mmap`-ed *memory blocks* : either small blocks of a megaword that
  is 8 megabytes (i.e. `RPS_SMALL_BLOCK_SIZE`), or large blocks of 8
  megawords (i.e. `RPS_LARGE_BLOCK_SIZE`). Values are inside such
  memory zones. Mutable objects may contain -perhaps indirectly-
  pointers to *quasivalues* (notably in their payload), that is to
  garbage collected zones which are not first-class values. A typical
  example of quasivalue could be some bucket in some (fully
  RefPerSys-implemented) array hash table (appearing as the payload of
  some object), in which buckets would be some small and mutable
  dynamic arrays of entries with colliding hashes. Such buckets indeed
  garbage collected zones, but are not themselves values (since they
  are mutable, but not reified as objects).

* The GC allocation operations are explicitly given the pointer to the
  local frame (i.e. `&_`, named `RPS_CURFRAME`), which is linked to
  the previous call frame and so on. That pointer is passed to *every*
  routine needing the GC (i.e. allocating or mutating values); only
  functions which don't allocate or mutate (e.g. [accessor or getter
  functions](https://en.wikipedia.org/wiki/Mutator_method)) can avoid
  getting that local frame pointer.
  
* The C++ runtime and plugins, and any code  generated in RefPerSys (i.e. modules)
or with [libgccjit](https://gcc.gnu.org/onlinedocs/jit/), should
*explicitly* be in [A-normal
form](https://en.wikipedia.org/wiki/A-normal_form). So coding `z =
f(g(x),y)` is forbidden in C++ (where `f` and `g` are C++ functions
using the GC). Instead, reserve a local slot such as `_.tmp1` in the
local frame, then code ``` _.tmp1 = g(RPS_CURFRAME, _.x); _.z =
f(RPS_CURFRAME, _.tmp1, _.y); ```
In less pedantic terms, we should do **only one call** (to GC-aware
functions) **or one allocation per statement**; and **every such
call** to some allocation primitive, or to a GC-aware function,
**should pass the `RPS_CURFRAME`** and **use `RPL_LOCALFRAME` in the
calling function**.
  
* A [*write barrier*](https://en.wikipedia.org/wiki/Write_barrier)
  should be called after object or quasivalue updates, and before any
  other allocation or update of some other object, value, or
  quasivalue. In practice, code
  `_.foo.rps_write_barrier(RPS_CURFRAME)` or more simply
  `_.foo.RPS_WRITE_BARRIER()`

* Every garbage-collection aware thread (a thread allocating GC-ed
  values, mutating GC-ed quasivalues or objects, running the GC
  forcibly) should call quite often, typically once per few
  milliseconds, the `Rps_GarbageCollector::maybe_garbcoll` routine. If
  this is not possible (e.g. before a potentially blocking `read` or
  `poll` system call), special precautions should be taken. Forgetting
  to call that `maybe_garbcoll` function often enough (typically every
  few milliseconds) could maybe crash the system.

* Consequently, as a rule of thumb, any routine which can directly *or
  indirectly* allocate GC-ed values or quasi-values, or directly *or
  indirectly* mutate GC-ed values or quasi-values, should take a
  calling callframe argument. We might need to consider: putting that
  specific `callframe` argument in some *global* register, using GCC
  `register` ... `asm` extension to [define global register
  variables](https://gcc.gnu.org/onlinedocs/gcc/Global-Register-Variables.html)
  and compile with the `-ffixed-`*reg* [code generation
  option](https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html).
  By coding convention, that calling callframe argument should be
  preferably named `callingfra`, and should be the *first* argument of
  every function or methods (member functions in C++ classes)
  requiring the GC.


### useful references

For Bismon, see http://github.com/bstarynk/bismon and read its [draft
Bismon report](http://starynkevitch.net/Basile/chariot-bismon-doc.pdf).

For the C++17 language, see this [C++ reference](https://en.cppreference.com/w/cpp).

For Linux programming, see [Advanced Linux
Programming](http://www.makelinux.net/alp/) and the
[syscalls(2)](http://man7.org/linux/man-pages/man2/syscalls.2.html)
`man` page.

For [GCC](http://gcc.gnu.org/), see notably its [Invoking
GCC](https://gcc.gnu.org/onlinedocs/gcc/Invoking-GCC.html) chapter.

For garbage collection, read Paul Wilson's [Uniprocessor Garbage
Collection
Techniques](https://www3.nd.edu/~dthain/courses/cse40243/spring2006/gc-survey.pdf)
old paper, then read the [GC handbook](http://gchandbook.org/)

## useful and relevant libraries

We already need the following libraries:

* [libunistring](https://www.gnu.org/software/libunistring/) for [UTF-8](https://en.wikipedia.org/wiki/UTF-8) support, since [UTF-8 is everywhere](http://utf8everywhere.org/)
* [libbacktrace](https://github.com/ianlancetaylor/libbacktrace) for [backtraces](https://en.wikipedia.org/wiki/Stack_trace) is essential. We hope to have rules mentioning and querying the call stack.
* [libgccjit](https://gcc.gnu.org/onlinedocs/jit/) for code generation.

We may want to use, either soon or within a few years, (usually after 2022) interesting C or C++ libraries such as:

* [libonion](https://www.coralbits.com/libonion/) or [Wt](https://www.webtoolkit.eu/wt) should be very soon (even in 2019) useful for the web interface
* [libevent](http://libevent.org/) or [libev](http://software.schmorp.de/pkg/libev.html) for some [event loop](https://en.wikipedia.org/wiki/Event_loop) (quite soon).
* [TensorFlow](https://www.tensorflow.org/) for [machine learning](https://en.wikipedia.org/wiki/Machine_learning) purposes
* [Gudhi](http://gudhi.gforge.inria.fr/) for [topological data analysis](https://en.wikipedia.org/wiki/Topological_data_analysis)
* [libcurl](https://curl.haxx.se/libcurl/) for [HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol) client 
* [GMPlib](https://gmplib.org/) for [Arbitrary Precision Arithmetic](https://en.wikipedia.org/wiki/Arbitrary-precision_arithmetic) or *Bignums*.
* [0mq](http://zeromq.org/) for distributed messaging, in relation with [distributed computing](https://en.wikipedia.org/wiki/Distributed_computing) and [message passing](https://en.wikipedia.org/wiki/Message_passing) approaches.
* [JsonCPP](https://github.com/open-source-parsers/jsoncpp) could be useful for [JSON](http://json.org/).
* [POCO](https://pocoproject.org/) is a useful C++ generic framework library, and [Qt](http://qt.io/) might also be useful, even without its GUI aspect.

We should list other libraries interesting for us here, just in case (to avoid forgetting them).

## some contributors

Thanks to Niklas Rosencrantz (Sweden) (he is `montao` on github) for several contributions.
Thanks to Abhishek Chkravarti (India) (he is `achakravarti` on github) for several contributions.

Other contributors, please email `basile@starynkevitch.net` about you.

## HTTP service

We are adding HTTP service in *RefPerSys*. So
[libonion](https://github.com/davidmoreno/onion) is required.  For
many months, we just hope to use `http://localhost:9090/` in a recent
(e.g. Firefox 80) web browser.

We really need to be able to show a demo of RefPerSys on a laptop
*without* Internet connection. So all required resources should be
copied here, under `webroot/`. Be careful about copyright and
licensing issues.

### Web conventions.

The `webroot/` subdirectory holds resources useful for HTTP
requests. In particular the following subdirectories:

* `webroot/css/` for -hand-written- style sheets.

* `webroot/img/` for additional images. Prefer SVG or PNG formats.

* `webroot/js/` for JavaScript code.

### Dependency installation notes (Ubuntu 20.04 Focal Fossa)

  - apt install make
  - apt install pkg-config
  - apt install libcurl4-openssl-dev
  - apt install zlib1g-dev
  - apt install libreadline-dev
  - apt install libjsoncpp-dev
  - apt install qt5-default
  - apt install cmake # we probably want to remove this dependency
  - apt install build-essential


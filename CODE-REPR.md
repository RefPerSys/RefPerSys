# Code representation inside the RefPerSys inference engine

A lot of code should be representable by [RefPerSys](http://refpersys.org/) data (objects,
instances, tuples, etc...) and generated lazily (at least at load
time).

The internal code representation should be conceptually close to
[gccjit](https://gcc.gnu.org/onlinedocs/jit/). But it is represented
by [RefPerSys](https://github.com/RefPerSys/RefPerSys/) data.

Machine code (as
[ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)
plugins) is generated with the
[libgccjit](https://gcc.gnu.org/onlinedocs/jit/) (in shared object
files, to be reused when possible) in file `gccjit_rps.cc` from
RefPerSys data. This library wraps the [GCC](https://gcc.gnu.org/)
middle-end and back-end so is capable of optimizations but may take a
significant amount of CPU time and resources to emit code (later to be
[dlopen(3)](https://man7.org/linux/man-pages/man3/dlopen.3.html)-ed by
RefPerSys).

Non-optimized machine code (*without* debugging
[DWARF](https://en.wikipedia.org/wiki/DWARF) data) can be quickly
generated by `lightgen_rps.cc` using [GNU
lightning](https://www.gnu.org/software/lightning/) library.

C++ code is generated in file `cppgen_rps.cc` and we hope to generate
more of it (replacing hand-written C++ code with generated one).


The generated code is (on Linux), [position-independent
code](https://en.wikipedia.org/wiki/Position-independent_code), in
[shared objects](https://en.wikipedia.org/wiki/Shared_library)
loadable with
[dlopen(3)](https://man7.org/linux/man-pages/man3/dlopen.3.html) and
[ELF](https://man7.org/linux/man-pages/man5/elf.5.html) format. When
possible, it contains [DWARF](https://en.wikipedia.org/wiki/DWARF)
debugging data (e.g. usable by
[libbacktrace](https://github.com/ianlancetaylor/libbacktrace) or the
[GDB](https://www.gnu.org/software/gdb/) debugger).

The generated code is either useful in RefPerSys or could be generated
for other programs or processes.

For RefPerSys consumed and generated code, we need to follow its
coding conventions tied to persistence and garbage collection. But the
generated machine code itself is not expected to be deleted by the
garbage collector. It would be forgotten (if provably useless) at dump
time.

Our `gccjit_rps.cc` file defines a `Rps_PayloadGccjit` payload class.

## [gccjit](https://gcc.gnu.org/onlinedocs/jit/) friendly data representations

### "source" locations and conventions.

In [gccjit](https://gcc.gnu.org/onlinedocs/jit/) a source location of
type
[`gcc_jit_location`](https://gcc.gnu.org/onlinedocs/jit/topics/locations.html)
refers to a "source file position". When the source file exists, it is
a path, line, column position. Source locations are optional.

By convention a [RefPerSys](https://github.com/RefPerSys/RefPerSys/)
file path, when referring to some file existing in the file system,
should not start with an `_` (underscore). If a file path does start
with an underscore, refer preferably to its absolute path or at least
use the `./_` prefix.

When the "source" file starts with an underscore, it should be the
objid of some *existing* RefPerSys object.  Hence `Rps_PayloadGccjit`
has member functions `make_csrc_location`, `make_string_src_location`,
and `make_rpsobj_location`.

### top-level representations.

Outside of "source" locations described above, the generated code is
obtained from mutable objects of superclass `_8kK8HUqCBlj02e7YGQ` =
*`code_class`* and immutable instances of superclass
`_7VRJKpau2Nn04oeUER` = *`code_instance`*.

Subclasses of *`code_class`* (i.e. `_8kK8HUqCBlj02e7YGQ`...) are used to represent types,
fields, functions, blocks, assignable lvalues, parameters, primitive
operators (e.g. floating point addition or integer multiplication).

Subclasses of *`code_instance`* (i.e. `_7VRJKpau2Nn04oeUER`) are used
to represent statements (added to blocks), rvalues so expressions and
most constants... RefPerSys literal strings, doubles, tagged integers
represent literal constants...

#### code type representations

They are either predefined types (for `int` etc....) or user defined types.

##### predefined type representations

They all are created (once and for all each) with our
plugins_dir/rpsplug_create_cplusplus_predefined_type.cc
# Code representation inside the RefPerSys inference engine

A lot of code should be representable by [RefPerSys](http://refpersys.org/) data (objects,
instances, tuples, etc...) and generated lazily (at least at load
time).

The internal representation should be conceptually close to
[gccjit](https://gcc.gnu.org/onlinedocs/jit/). But it is represented
by [RefPerSys](https://github.com/RefPerSys/RefPerSys/) data.

It is generated in file `gccjit_rps.cc`

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
C++ class
[`gccjit::location`](https://gcc.gnu.org/onlinedocs/jit/cp/topics/locations.html)
refers to a "source file position". When the source file exists, it is
a path, line, column position. Source locations are optional.

By convention a [RefPerSys](https://github.com/RefPerSys/RefPerSys/)
file path, when referring to some file existing in the file system,
should not start with an `_` (underscore). If it does, refer
preferably to its absolute path or at least use the `./_` prefix.

When the "source" file starts with an underscore, it should be the
objid of some *existing* RefPerSys object.  Hence `Rps_PayloadGccjit`
has member functions 


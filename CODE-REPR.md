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

For RefPerSys consumed code, we need to follow coding conventions tied
to persistence and garbage collection.
# Code representation inside the RefPerSys inference engine

A lot of code should be representable by RefPerSys data (objects,
instances, tuples, etc...) and generated lazily (at least at load
time).

The internal representation should be conceptually close to
[gccjit](https://gcc.gnu.org/onlinedocs/jit/).

It is generated in file `gccjit_rps.cc` 
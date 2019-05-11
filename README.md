# refpersys

The Reflective Persistent System language is a **research project**, taking many good ideas from [Bismon](http://github.com/bstarynk/bismon), sharing a lot of goals (except static source code analysis) with it but avoiding bad ideas from it.

For Linux/x86-64 only. Don't even think of running that on non-Linux
systems, unless you provide patches for that. And we need a 64 bits
processor.

We have multi-threading in mind, but in some limited way. We think of
a pool of a few dozen Pthreads at most (but not of a thousand
Pthreads).

We absolutely want to avoid any
[GIL](https://en.wikipedia.org/wiki/Global_interpreter_lock)


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

This *research* project is
[GPLv3+](https://www.gnu.org/licenses/gpl.html) licensed and
copyrighted by the RefPerSys team, currently made of:

 *      Basile Starynkevitch <basile@starynkevitch.net>, homepage http://starynkevitch.net/Basile/
 *      Niklas Rosencrantz <niklasro@gmail.com>
 *      Abhishek Chakravarti <abhishek@taranjali.org>

## Contributing

Please refer to the `CONTRIBUTING.md` file for coding conventions.

## File conventions

The Refpersys runtime is implemented in C++17, with hand-written C++
code in `*_rps.cc`, and has a single C++ header file `refpersys.hh`.
We don't claim to be C++ gurus. Most C++ experts could write more
genuine C++ code than we do and will find our C++ code pityful. We
just want our runtime to work, not to serve as an example of well
written C++17 code.

It may later also use generated C++ code in some `_*.cc` file, some
generated C code in some `_*.c` and generated C or C++ headers in some
`_*.h` files. By convention, files starting with an underscore are
generated (but they may, or not, being git versioned).

We could need later some C++ generating program (maybe similar in
spirit to Bismon's
[BM_makeconst.cc](https://github.com/bstarynk/bismon/blob/master/BM_makeconst.cc). it
would then be named `rps_*` for the executable, and fits in a single
self-sufficient `rps_*.cc` C++ file. Perhaps we'll later have some
`rps_makeconst` executable to generate some C++, and its source in
some `rps_makeconst.cc`. So the convention is that any future C++
generating source code is in some `rps_*.cc` C++ file.


## Building and dependencies.

The [build automation](https://en.wikipedia.org/wiki/Build_automation)
tool used here is [omake](https://github.com/ocaml-omake/omake)
because [`omake`
has](http://projects.camlcity.org/projects/dl/omake-0.10.3/doc/html/omake-quickstart.html)
*automatic dependency analysis* and *content-based dependency
analysis* (the later being an unusual feature - most build automation
tools using modification time of files).


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
and adopt. So MPS specific code is wrapped around `#ifdef
RPS_HAVE_MPS` ... `#endif`.

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
  pointer `foo` is coded `_.foo` in our C++ runtime. A local frame in
  RefPerSys should be declared in C++ using `RPS_LOCALFRAME`.

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
  
* The C++ runtime, and any code generated in RefPerSys, should
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

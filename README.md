# refpersys

The Reflective Persistent System language is a **research project**, taking many good ideas from [Bismon](http://github.com/bstarynk/bismon), sharing a lot of goals (except static source code analysis) with it but avoiding bad ideas from it.

For Linux/x86-64 only.

We have multi-threading in mind, but in some limited way. We think of
a pool of a few dozen Pthreads at most (but not of a thousand
Pthreads).

We absolutely want to avoid any
[GIL](https://en.wikipedia.org/wiki/Global_interpreter_lock)


## License

This project is GPLv3+ licensed, but contain some code from
[MPS](https://www.ravenbrook.com/project/mps) which has some MIT-like
license.

## Building and dependencies.

[GNU `make`](https://www.gnu.org/software/make/) 4 is required. And
[GCC](http://gcc.gnu.org/) 8 -or later- or some recent C++14 compiler.

One must explicitly pull in the submodule as follows

```
git submodule init
git submodule update
```

Use `make` or `make -j` to build this project.

As usual, run `make -p` to understand the built-in rules and take
advantage of them.

To debug the build machinery, consider using
[remake](http://bashdb.sourceforge.net/remake/), probably as `remake
-x`

Later, we might change the build automation, perhaps to
[omake](http://projects.camlcity.org/projects/omake.html), since
`omake` uses file contents, not file modification times, for build
decision.

## Contributing

Please refer to the `CONTRIBUTING.md` file for coding conventions.

## File conventions

The Refpersys runtime is implemented in C++, with hand-written C++
code in `src/*.cc`, hand-written C++ headers in `inc/*.h`

It may later also use generated C++ code in some `_*.cc` file, some
generated C code in some `_*.c` and generated C or C++ headers in some
`_*.h` files.

The Refpersys runtime is using [Ravenbrook
MPS](https://www.ravenbrook.com/project/mps) (a sophisticated garbage
collection framework). MPS related files are under `mps/`.

The subdirectory `mps/` is obtained from MPS 1.117.0; That `mps/`
contains all the files from `mps-kit-1.117.0/code/`

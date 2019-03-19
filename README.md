# refpersys

The Reflective Persistent System language

For Linux/x86-64 only.

## License

This project is GPLv3+ licensed, but contain some code from
[MPS](https://www.ravenbrook.com/project/mps) which has some MIT-like
license.

## Building and dependencies.

[GNU `make`](https://www.gnu.org/software/make/) 4 is required. And
[GCC](http://gcc.gnu.org/) 8 -or later- or some recent C++14 compiler.

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

It may later also use generated C++ code in some `_*.cc` file and generated
C++ headers in some `_*.h` files.

The Refpersys runtime is using [Ravenbrook
MPS](https://www.ravenbrook.com/project/mps) (a sophisticated garbage
collection framework). MPS related files are under `mps/`.

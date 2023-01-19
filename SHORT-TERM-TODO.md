# short term todo list for *RefPerSys* #

This file `SHORT-TERM-TODO.md` (using
[markdown](https://en.wikipedia.org/wiki/Markdown) syntax) documents
technical small tasks related to [RefPerSys](http://refpersys.org/)

The intuition is to mention here a few things which should take less
than a few days of work.

## TODO List

For *RefPerSys* near commit `7bc44857b94bd031` (jan. 2023)

### create a mutable set of *RefPerSys* classes

We need a predefined root object, of *RefPerSys* class `mutable_set`
(i.e. `_0J1C39JoZiv03qA2HA` ...), to be named
`the_mutable_set_of_classes` whose payload is a mutable set (i.e. in
C++ parlance `Rps_PayloadSetOb`). It should be filled with all classes
objects. The routine `Rps_ObjectRef::make_named_class` should be
improved to fill `the_mutable_set_of_classes`

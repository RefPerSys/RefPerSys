# short term todo list for *RefPerSys* #

This file `SHORT-TERM-TODO.md` (using
[markdown](https://en.wikipedia.org/wiki/Markdown) syntax) documents
technical small tasks related to [RefPerSys](http://refpersys.org/)

The intuition is to mention here a few things which should take less
than a few days of work.

## TODO List

For *RefPerSys* near commit `dd3b0d0c848a441f` (jan. 2023)

The predefined root object `_4DsQEs8zZf901wT1LH` informally named
`the_mutable_set_of_classes` should have its naming symbol
`the_mutable_set_of_classes` and should be filled with all classes
objects. The routine `Rps_ObjectRef::make_named_class` should be
improved to fill `the_mutable_set_of_classes`

## DONE  List

### create a mutable set of *RefPerSys* classes

We have the predefined root object `_4DsQEs8zZf901wT1LH`, of *RefPerSys* class `mutable_set`
 to be named
`the_mutable_set_of_classes` whose payload is a mutable set (i.e. in
C++ parlance `Rps_PayloadSetOb`). 

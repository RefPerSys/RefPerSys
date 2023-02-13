# short term todo list for *RefPerSys* #

This file `SHORT-TERM-TODO.md` (using
[markdown](https://en.wikipedia.org/wiki/Markdown) syntax) documents
technical small tasks related to [RefPerSys](http://refpersys.org/)
(an acronym for *Ref*lexive *Per*sistent *Sys*tem) and its open source
code on https://github.com/RefPerSys/RefPerSys

The intuition is to mention here a few things which should take less
than a few days of work.


## TODO List


### context

For *RefPerSys* near commit `85667c8506a34a` (feb. 2023). Please email
[refpersys-forum@framalistes.org](mailto:refpersys-forum@framalistes.org)
or at least contact [Basile
Starynkevitch](http://starynkevitch.net/Basile/) by SMS, WhatsApp, or
private email to
[basile@starynkevitch.net](mailto:basile@starynkevitch.net) if you
want to contribute.


Please mention *RefPerSys* to colleagues, students, teachers et al...

### small useful TODO coding tasks

Improve the REPL parsing so that `make test03` works

Improve *RefPerSys* so that its `--publish-me=http://localhost:8086/`
program option works. See function `rps_publish_me` in file
`curl_rps.cc`

Review file `refpersys.hh` for spelling mistakes in comments.

Not sure if we want to remove there `class Rps_PayloadWebPi` and
`PaylWebHandler` and `PaylWebex` and simulatnously remove the
persistent objects and classes related to Web from the persistent
store....

Add transient (non-dumped) payloads for: *forked* Unix processes,
`popen`-ed file handles, smart pointers to C++ input or output
streams and the corresponding RefPerSys classes. The handling of
forked Unix processes should be known to our `agenda_rps.cc`
machinery. These payloads may need C++ locking or synchronizing
facilities (e.g. `std::mutex` or `std::condition_variable`-s ...). It
is suggested to add a new C++ file (perhaps named
`transientobj_rps.cc` for our *transient RefPerSys objects and
payloads* ...) for them.

Use the `create-refpersys-root-class.sh` shell script to create
RefPerSys classes related to Unix processes, `popen`-ed file handles
etc...

## DONE  List

The predefined root object `_4DsQEs8zZf901wT1LH` informally named
`the_mutable_set_of_classes` should have its naming symbol
`the_mutable_set_of_classes` and should be filled with all classes
objects. The routine `Rps_ObjectRef::make_named_class` has been
improved to fill `the_mutable_set_of_classes`


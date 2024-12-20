# short term todo list for *RefPerSys* #

This file `SHORT-TERM-TODO.md` (using
[markdown](https://en.wikipedia.org/wiki/Markdown) syntax) documents
technical small tasks related to [RefPerSys](http://refpersys.org/)
(an acronym for *Ref*lexive *Per*sistent *Sys*tem) and its open source
code on https://github.com/RefPerSys/RefPerSys

The intuition is to mention here a few things which should take less
than a few days or weeks of work.


## TODO List

Improve both GNUmakefile and do-build-refpersys-plugin.cc to have secondary
plugin C++ files

### context

For *RefPerSys* near commit [d53653c37f7a](https://github.com/RefPerSys/RefPerSys/commit/d53653c37f7a5dd0817a5f86256ef6f095944e25)  (i.e. `d53653c37f7a`, May 2023). Please email
[refpersys-forum@framalistes.org](mailto:refpersys-forum@framalistes.org)
or at least contact [Basile
Starynkevitch](http://starynkevitch.net/Basile/) by SMS, WhatsApp, or
private email to
[basile@starynkevitch.net](mailto:basile@starynkevitch.net)
and [b.starynkevitch@gmail.com](mailto:b.starynkevitch@gmail.com) if you
want to contribute.

(I Basile can also be reached during French office hours by Whatapp
 to +33 6 8501 followed by the four digits product of 7 and 337)


Please mention *RefPerSys* and the http://refpersys.org/ URL to colleagues, students, teachers et al...

### small useful TODO coding tasks

Improve the REPL parsing so that `make test03` works


Add a file `machlearn_rps.cc` including `mlpack.hpp` from https://www.mlpack.org/

In commit c4935a77e45f3 (June 14, 2023) the `make test01`
fails. Probably `Rps_ObjectZone::is_instance_of` and or
`Rps_ObjectZone::is_subclass_of` are buggy. This is corrected in
commit 889f895c0b (Jan 17, 2024)

Improve *RefPerSys* so that its `--publish-me=http://localhost:8086/`
program option works. See function `rps_curl_publish_me` in file
`curl_rps.cc`

Review file `refpersys.hh` for spelling mistakes in comments.

Not sure if we want to remove there `class Rps_PayloadWebPi` and
`PaylWebHandler` and `PaylWebex` and simulatnously remove the
persistent objects and classes related to Web from the persistent
store....

Add some `start` *RefPerSys* symbol and code for `the_agenda` which
starts the agenda by sending that `start` selector to `the_agenda`
after load.

Add transient payloads for smart pointers to C++ input or output
streams and the corresponding RefPerSys classes. The handling of
forked Unix processes should be known to our `agenda_rps.cc`
machinery. These payloads may need C++ locking or synchronizing
facilities (e.g. `std::mutex` or `std::condition_variable`-s
...). Complete file `transientobj_rps.cc` for our *transient RefPerSys
objects and payloads* ...) for them.

Code the `rps_event_loop` around a
[poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html) system
call. It should cooperate with `Rps_PayloadUnixProcess` and manage
both processes and file descriptors handled by that event loop. These
file descriptors may be related to JSONRPC service, etc...



Use [libgccjit](https://gcc.gnu.org/onlinedocs/jit/)


### possible improvements

Define some different plugins for the user interface. So remove
all dependencies on X11, readline and replace them with other plugins (not
the ones in plugins_dir) dealing with user interfacve.

## DONE  List

Added transient (non-dumped) payloads for: *forked* Unix processes,
`popen`-ed file handles

Use the `create-refpersys-root-class.sh` shell script to create
RefPerSys classes related to Unix processes, `popen`-ed file handles
etc...

The predefined root object `_4DsQEs8zZf901wT1LH` informally named
`the_mutable_set_of_classes` should have its naming symbol
`the_mutable_set_of_classes` and should be filled with all classes
objects. The routine `Rps_ObjectRef::make_named_class` has been
improved to fill `the_mutable_set_of_classes`


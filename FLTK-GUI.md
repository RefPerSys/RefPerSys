# the temporary FLTK graphical interface of RefPerSys

This is related to the [RefPerSys](http://refpersys.org/) project and
its [`fltk-branch` for
`git`](https://gitlab.com/bstarynk/refpersys/-/tree/fltk-branch). We
want to use [FLTK](https://www.fltk.org/) version
[1.4](https://www.fltk.org/doc-1.4/), a *simple* graphical user
toolkit on Linux.

In the long term (after 2023), we should consider a web based user
interface, in particular using
[HTTPS](https://en.wikipedia.org/wiki/HTTPS) and *generating*
[HTML5](https://en.wikipedia.org/wiki/HTML5) and
[AJAX](https://en.wikipedia.org/wiki/Ajax_(programming)) so
[JavaScript](https://en.wikipedia.org/wiki/JavaScript) code. The
[Bismon](https://github.com/bstarynk/bismon/) project demonstrates
that it is doable, but a *lot* of work (so a lot of human time) and
require know-how that we *collectively* don't currently master in
mid-2020.

## why switch from Qt5 to FLTK1.4?

The [Qt](http://qt.io/) is a very powerful GUI toolkit, but too
difficult to code for and providing features (cross-platform
compatibility to other operating systems and [display
servers](https://en.wikipedia.org/wiki/Display_server) such as
[Wayland](https://en.wikipedia.org/wiki/Wayland_(display_server_protocol))
we don't really care about in 2020 or 2021.

The `master` branch of [RefPerSys](http://refpersys.org/) is using Qt5
but has several issues
(e.g. [#31](https://gitlab.com/bstarynk/refpersys/-/issues/31) or
[#26](https://gitlab.com/bstarynk/refpersys/-/issues/26) or
[#32](https://gitlab.com/bstarynk/refpersys/-/issues/32) ...) that we
failed to correct in a reasonable time.

**The *Qt* toolkit is also very big.** Compiling it takes a night of computer time.

For a few years, we can afford targetting only
[Xorg](https://en.wikipedia.org/wiki/X.Org_Server) running on Linux,
for UTF-8 encoded languages written from left to right. This excludes
human languages like *Japanese* or *Hebrew* and switching to other
display servers such as *Wayland*.

The *Qt* toolkit has a very powerful object model requiring a specific
meta-program generating C++ code, the [Qt
`moc`](https://doc.qt.io/qt-5/moc.html). This is in tension with our
own *RefPerSys* object model inspired by
[ObjVLisp](http://stephane.ducasse.free.fr/Web/ArchivedLectures/Lectures-Reflective-0001/allPapers.pdf).

The *Qt5* toolkit is also starting **several threads** behind our
back, and this is **annoying** (and hostile) for any kind of [tracing
garbage
collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection),
since precise garbage collection in multi-threaded approaches is a
difficult topic (see the [GC handbook](http://gchandbook.org/) for
details).. In particular, we want to *generate* garbage collection
supporting routines and consider (for later) a [copying generational
garbage
collector](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science))
for immutable values. Past experience on [GCC
MELT](http://starynkevitch.net/Basile/gcc-melt/) is then relevant.

In contrast **the *FLTK* toolkit is much smaller**, looks indeed less
*sexy* (much less eye candy), but is a simple toolkit (on Linux,
*only* for [X11](https://en.wikipedia.org/wiki/X_Window_System) and
that means [Xorg](https://www.x.org/) in 2020...) which can be
compiled from source code in a dozen of minutes. The underlying object
model is just the C++ one. And FLTK is *single-threaded* for its GUI.

If later we discover that we need to
[fork](https://en.wikipedia.org/wiki/Fork_(software_development)) the
FLTK toolkit for the specific needs of
[RefPerSys](http://refpersys.org/), it is doable. Doing the same for
Qt won't be reasonable.

The FLTK toolkit would enable us to easily code a
[Centaur](https://www-sop.inria.fr/croap/centaur/centaur.html)-inspired
user interface, with again the idea of *explicitly* representing both
[abstract syntax
tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree)s and their
transformations during editing. The insight is to use [bidirectional
transformation](https://en.wikipedia.org/wiki/Bidirectional_transformation)s
between *RefPerSys* values and what appears on the screen. Papers such
as [this one](https://dl.acm.org/doi/pdf/10.1145/2784731.2784750) or
[that one](https://dl.acm.org/doi/pdf/10.1145/1929501.1929520) provide
some theoretical foundations.


## Technical considerations

The most important one is that FLTK [event
loop](https://en.wikipedia.org/wiki/Event_loop) may be recursive
(e.g. for [modal dialogs](https://en.wikipedia.org/wiki/Modal_window)
...). This should be specially taken care of to be friendly with our
garbage collector (where we hope to *eventually* introduce copying
techniques à la [Cheney's
algorithm](https://en.wikipedia.org/wiki/Cheney%27s_algorithm) ....).

Practically speaking, we probably want to write our own event loop
which uses
[`Rps_CallFrame*`](https://gitlab.com/bstarynk/refpersys/-/wikis/call-frames-in-RefPerSys)
and deals with a [FIFO
queue](https://en.wikipedia.org/wiki/Queue_(abstract_data_type)) of a
[tagged union](https://en.wikipedia.org/wiki/Tagged_union) of either
[C++ closures](https://stackoverflow.com/q/12635184/841108) or
[*RefPerSys*
ones](https://gitlab.com/bstarynk/refpersys/-/wikis/RefPerSys-code-for-applicable-functions). Of
course these closures are queued with a few arguments to be applied to
them. This mechanism is a simple [TODO
list](https://en.wikipedia.org/wiki/Time_management#Implementation_of_goals)
and compatible with the [FLTK
policy](https://www.fltk.org/doc-1.4/group__fl__del__widget.html) of
never *immediately* deleting any FLTK object inside an FLTK
[callback](https://en.wikipedia.org/wiki/Callback_(computer_programming)).

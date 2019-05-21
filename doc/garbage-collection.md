# RefPerSys garbage collection (GC)

[Garbage
collection](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science))
is a well understood topic, and has some well established
terminology. It is tightly related to [dynamic memory
allocation](https://en.wikipedia.org/wiki/Memory_management#ALLOCATION). RefPerSys
uses mostly [tracing garbage
collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)
techniques. Efficient GC requires making several important trade-offs,
so is more an art than a science.

For basic GC concepts and terminology, read first Paul Wilson's
[*Uniprocessor Garbage Collection
Techniques*](https://www.cs.rice.edu/~javaplt/311/Readings/wilson92uniprocessor.pdf)
survey [1992
IWMM](https://link.springer.com/chapter/10.1007/BFb0017182) paper. Then read [The GC handbook](http://gchandbook.org/) (ISBN: 978-1420082791) or at least its predecessor: Richard Jones' [Garbage Collection](https://www.cs.kent.ac.uk/people/staff/rej/gcbook/) (ISBN: 0-471-94148-4) book.

A RefPerSys garbage collected memory zone is an instance of some
subclass of `Rps_ZoneValue`, which conceptually is a [tagged
union](https://en.wikipedia.org/wiki/Tagged_union), discriminated by
some `Rp_Type` enum. The pointer to such a GC-allocated zone is called
a *quasi-value* (so by extension, that memory zone is also called
*quasi-value*). Of course, first class RefPerSys values and objects
are also quasi-values (but genuine values can also be [*tagged
integers*](https://en.wikipedia.org/wiki/Tagged_pointer)).

A *mutator thread* (see
[this](https://www.researchgate.net/profile/Thierry_Le_Sergent/publication/221032842_Incremental_Multi-threaded_Garbage_Collection_on_Virtual_Shared_Memory_Architectures)
paper) is a RefPerSys Posix or C++ thread which uses the garbage
collector (either for *allocating* new quasi-values, or for *updating*
existing ones). Our GC is a *precise* garbage collector, so needs to
know and scan all the quasi-values on the [call
stack](https://en.wikipedia.org/wiki/Call_stack). Because there is no
way to reliably get each call frame and all the quasi-values in them,
the calling frame is a required argument (conventionally named
`callingfra`) to every allocating or updating primitive that mutator
threads could call, and our call frames are actually reified as
instances of subclasses of `Rps_CallFrameZone`.


## file refpersys/IDE/Makefile
CCACHE= ccache
CXX= $(CCACHE) g++
DIALECTFLAGS= -std=gnu++17
ASTYLE= astyle
ASTYLEFLAGS= 
OPTIMFLAGS= -g -O1
INCLFLAGS= -I/usr/local/include
WARNFLAGS= -Wall -Wextra 
CXXFLAGS= $(DIALECTFLAGS) $(OPTIMFLAGS) $(shell fltk-config --cxxflags) $(INCLFLAGS) -I.. $(WARNFLAGS)
LIBES=  -lhjson -lunistring -lbacktrace -ldl $(shell fltk-config --ldflags) -lpthread
LDFLAGS +=  -L/usr/local/lib  -rdynamic


SOURCES= $(wildcard ide-*-rps.cc) ../primes_rps.cc  ../utilities_rps.cc

HEADERS= $(wildcard ide-*-rps.hh)
OBJECTS= $(patsubst %.cc, %.o, $(SOURCES))

## see also http://make.mad-scientist.net/papers/advanced-auto-dependency-generation
MAKEDEPS= $(patsubst %.cc, %.mkd, $(SOURCES))
GITID := $(shell ./generate-gitid.sh)

.PHONY: all clean indent


all: ide-refpersys

clean:
	$(RM) *.o *~ *% *.orig *.mkd _timestamp* ide-refpersys
	$(RM) ../*.o ../*~ ../*% ../*.orig ../*.mkd ../_timestamp* 

_timestamp.c:
	./generate-timestamp.sh $@ > $@-tmp
	mv $@-tmp $@

%.o %.mkd: %.cc
	$(COMPILE.cc) -DRPS_GITID=\"$(GITID)\" $< -MT $@ -MMD -MP -MF $(basename $^).mkd -o $@

ide-refpersys: $(OBJECTS) _timestamp.o
	mv -vf _timestamp.c _timestamp.c~
	$(LINK.cc) _timestamp.o $(OBJECTS) $(LIBES) -o $@

indent:
	for ccfil in $(SOURCES); do \
	  $(ASTYLE) $(ASTYLEFLAGS) $$ccfil ; \
	done
	for hhfil in $(HEADERS); do \
	  $(ASTYLE) $(ASTYLEFLAGS) $$hhfil ; \
	done

-include $(wildcard *.mkd)

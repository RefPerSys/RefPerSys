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
LIBES=  -lhjson -lunistring -lbacktrace -ldl $(shell fltk-config --ldflags)
LDFLAGS +=  -L/usr/local/lib  -rdynamic


SOURCES= $(wildcard ide-*-rps.cc)
HEADERS= $(wildcard ide-*-rps.hh)
OBJECTS= $(patsubst %.cc, %.o, $(SOURCES)) 
.PHONY: all clean indent

all: ide-refpersys

clean:
	$(RM) *.o *~ *.orig ide-refpersys

_timestamp.c:
	./generate-timestamp.sh $@ > $@-tmp
	mv $@-tmp $@

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

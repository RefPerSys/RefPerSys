
## Description:
##      This file is part of the Reflective Persistent System. refpersys.org
##
##      It is its Makefile, for the GNU make automation builder.
##
## Author(s):
##      Basile Starynkevitch <basile@starynkevitch.net>
##      Abhishek Chakravarti <abhishek@taranjali.org>
##      Nimesh Neema <nimeshneema@gmail.com>
##
##      © Copyright 2020 The Reflective Persistent System Team
##      team@refpersys.org
##
## License:
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <http://www.gnu.org/lice

.PHONY: all clean

RPS_GIT_ID:= $(shell ./generate-gitid.sh)

RPS_CORE_HEADERS:= $(sort $(wildcard *_rps.hh))
RPS_CORE_SOURCES:= $(sort $(wildcard *_rps.cc))
RPS_CORE_OBJECTS = $(patsubst %.cc, %.o, $(RPS_CORE_SOURCES))

RPS_QT_HEADERS:= $(sort $(wildcard *_qrps.hh))
RPS_QT_SOURCES:= $(sort $(wildcard *_qrps.cc))
RPS_QT_OBJECTS = $(patsubst %.cc, %.o, $(RPS_QT_SOURCES))
RPS_QT_MOC = moc

RPS_BUILD_CCACHE = ccache
RPS_BUILD_CC = gcc
RPS_BUILD_CXX = g++
RPS_BUILD_DIALECTFLAGS = -std=gnu++17
RPS_BUILD_WARNFLAGS = -Wall -Wextra
RPS_BUILD_OPTIMFLAGS = -O1 -g3
RPS_BUILD_CODGENFLAGS = -fPIC

RPS_INCLUDE_DIRS = /usr/local/include /usr/include 
RPS_INCLUDE_FLAGS = $(patsubst %, -I %, $(RPS_INCLUDE_DIRS))

RPS_PKG_CONFIG = pkg-config
RPS_PKG_NAMES = Qt5Core Qt5Gui Qt5Widgets Qt5Network jsoncpp
RPS_PKG_CFLAGS:= $(shell $(RPS_PKG_CONFIG) --cflags $(RPS_PKG_NAMES))
RPS_PKG_LIBS:= $(shell $(RPS_PKG_CONFIG) -- libs $(RPS_PKG_NAMES)) -lbacktrace -ldl

RM= rm -f
MV= mv
CC = $(RPS_BUILD_CCACHE) $(RPS_BUILD_CC)
CXX = $(RPS_BUILD_CCACHE) $(RPS_BUILD_CXX)
CXXFLAGS += $(RPS_BUILD_DIALECTFLAGS) $(RPS_BUILD_OPTIMFLAGS) \
            $(RPS_BUILD_CODGENFLAGS) \
	    $(RPS_BUILD_WARNFLAGS) $(RPS_BUILD_INCLUDE_FLAGS) \
	    $(RPS_PKG_CFLAGS) -DRPS_GITID=\"$(RPS_GIT_ID)\"

all: refpersys

refpersys: $(RPS_CORE_OBJECTS) $(RPS_QT_OBJECTS) __timestamp.o
	$(LINK.cc) -rdynamic \
            $(RPS_CORE_OBJECTS) $(RPS_QT_OBJECTS) __timestamp.o \
           -o $@
	$(MV) --backup __timestamp.c __timestamp.c~
	$(RM) __timestamp.o

$(RPS_CORE_OBJECTS): $(RPS_CORE_HEADERS) $(RPS_CORE_SOURCES)

$(RPS_QT_OBJECTS): _qthead_qrps.inc.hh $(RPS_QT_SOURCES)

_qthead_qrps.inc.hh: $(RPS_QT_HEADERS)
	$(RPS_QT_MOC) $(RPS_PKG_CFLAGS) -DRPS_GITID=\"$(RPS_GIT_ID)\" $< -o $@

%.o: %.cc refpersys.hh.gch
	$(COMPILE.cc) -o $@ $<

# https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html
refpersys.hh.gch: refpersys.hh
	$(COMPILE.cc) -c -o $@ $^

clean:
	$(RM) *.o *.orig *~ refpersys *.gch

__timestamp.c:
	./generate-timestamp.sh > $@-tmp
	$(MV) --backup $@-tmp $@
## eof Makefile


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
##      © Copyright 2019 - 2020 The Reflective Persistent System Team
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

.PHONY: all objects clean fullclean redump altredump print-temporary-plugin-settings indent test01 test02 test03

RPS_GIT_ID:= $(shell ./generate-gitid.sh)
RPS_SHORTGIT_ID:= $(shell ./generate-gitid.sh -s)

RPS_CORE_HEADERS:= $(sort $(wildcard *_rps.hh))
RPS_CORE_SOURCES:= $(sort $(wildcard *_rps.cc))
RPS_CORE_OBJECTS = $(patsubst %.cc, %.o, $(RPS_CORE_SOURCES))
RPS_SANITIZED_CORE_OBJECTS = $(patsubst %.cc, %.sanit.o, $(RPS_CORE_SOURCES))
RPS_DEBUG_CORE_OBJECTS = $(patsubst %.cc, %.dbg.o, $(RPS_CORE_SOURCES))


#RPS_BUILD_CCACHE?= ccache
RPS_BUILD_CCACHE= 
RPS_BUILD_CC?= gcc
RPS_BUILD_CXX?= g++
RPS_BUILD_DIALECTFLAGS = -std=gnu++17
RPS_BUILD_WARNFLAGS = -Wall -Wextra
override RPS_BUILD_OPTIMFLAGS ?= -Og -g3
RPS_BUILD_DEBUGFLAGS = -O0 -fno-inline -g3
RPS_BUILD_CODGENFLAGS = 
RPS_BUILD_SANITFLAGS = -fsanitize=address
RPS_INCLUDE_DIRS = /usr/local/include /usr/include /usr/include/jsoncpp
RPS_INCLUDE_FLAGS = $(patsubst %, -I %, $(RPS_INCLUDE_DIRS))
RPS_BUILD_INCLUDE_FLAGS=  -I . $(RPS_INCLUDE_FLAGS)

RPS_ALTDUMPDIR_PREFIX?= /tmp/refpersys-$(RPS_SHORTGIT_ID)

RPS_PKG_CONFIG=  pkg-config
RPS_PKG_NAMES= jsoncpp readline libcurl
RPS_PKG_CFLAGS:= $(shell $(RPS_PKG_CONFIG) --cflags $(RPS_PKG_NAMES))
RPS_PKG_LIBS:= $(shell $(RPS_PKG_CONFIG) --libs $(RPS_PKG_NAMES))

LIBES= $(RPS_PKG_LIBS) -lunistring -lbacktrace -ldl
RM= rm -f
MV= mv
CC= $(RPS_BUILD_CCACHE) $(RPS_BUILD_CC)
CXX= $(RPS_BUILD_CCACHE) $(RPS_BUILD_CXX)
LINK.cc= $(RPS_BUILD_CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
CXXFLAGS= $(RPS_BUILD_DIALECTFLAGS) $(RPS_BUILD_OPTIMFLAGS) \
            $(RPS_BUILD_CODGENFLAGS) \
	    $(RPS_BUILD_WARNFLAGS) $(RPS_BUILD_INCLUDE_FLAGS) \
	    $(RPS_PKG_CFLAGS) \
            -DRPS_GITID=\"$(RPS_GIT_ID)\"

LDFLAGS += -rdynamic -pthread -L /usr/local/lib -L /usr/lib

-include $(wildcard $$HOME/build-refpersys.mk)
all:
	if [ -f refpersys ] ; then  $(MV) -f --backup refpersys refpersys~ ; fi
	$(RM) __timestamp.o __timestamp.c
	sync &
	$(MAKE) -$(MAKEFLAGS) refpersys
	sync

refpersys: $(RPS_CORE_OBJECTS) __timestamp.o
	$(LINK.cc) $(RPS_CORE_OBJECTS) __timestamp.o \
           $(LIBES) -o $@-tmp
	$(MV) --backup $@-tmp $@
	$(MV) --backup __timestamp.c __timestamp.c~
	$(RM) __timestamp.o

sanitized-refpersys:  $(RPS_SANITIZED_CORE_OBJECTS) __timestamp.o
	$(LINK.cc)  $(RPS_BUILD_SANITFLAGS) \
           $(RPS_SANITIZED_CORE_OBJECTS) __timestamp.o \
           $(LIBES) -o $@-tmp
	$(MV) --backup $@-tmp $@
	$(MV) --backup __timestamp.c __timestamp.c~
	$(RM) __timestamp.o


## the below target don't work yet
#- dbg-refpersys:  $(RPS_DEBUG_CORE_OBJECTS) __timestamp.o
#-         env RPS_BUILD_OPTIMFLAGS='$(RPS_BUILD_DEBUGFLAGS)' $(MAKE) $(MAKEFLAGS) -e  $(RPS_DEBUG_CORE_OBJECTS) 
#-         $(LINK.cc)  $(RPS_BUILD_DEBUGFLAGS) \
#-            $(RPS_DEBUG_CORE_OBJECTS)  __timestamp.o \
#-            $(LIBES) -o $@-tmp
#-         $(MV) --backup $@-tmp $@
#-         $(MV) --backup __timestamp.c __timestamp.c~
#-         $x(RM) __timestamp.o

objects:  $(RPS_CORE_OBJECTS) 

$(RPS_CORE_OBJECTS): $(RPS_CORE_HEADERS) $(RPS_CORE_SOURCES)


%.o: %.cc refpersys.hh.gch
	$(COMPILE.cc) -o $@ $<

%.sanit.o: %.cc refpersys.hh.sanit.gch
	$(COMPILE.cc) $(RPS_BUILD_SANITFLAGS) -o $@ $<

%.dbg.o: %.cc refpersys.hh.dbg.gch
	$(COMPILE.cc) $(RPS_BUILD_DEBUGFLAGS) -o $@ $<

%.ii: %.cc refpersys.hh.gch
	$(COMPILE.cc) -C -E $< | sed s:^#://#:g > $@

# see https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html 

refpersys.hh.gch: refpersys.hh oid_rps.hh $(wildcard generated/rps*.hh)
	$(COMPILE.cc) -c -o $@ $<
refpersys.hh.sanit.gch: refpersys.hh oid_rps.hh $(wildcard generated/rps*.hh)
	$(COMPILE.cc)  $(RPS_BUILD_SANITFLAGS) -c -o $@ $<
refpersys.hh.dbg.gch: refpersys.hh oid_rps.hh $(wildcard generated/rps*.hh)
	$(COMPILE.cc)  $(RPS_BUILD_DEBUGFLAGS) -c -o $@ $<





clean:
	$(RM) *.o *.orig *~ refpersys sanitized-refpersys *.gch *~
	$(RM) _*.hh _*.cc _timestamp_rps.* generated/*~
	$(RM) persistore/*~ persistore/*%
	$(RM) *.ii
	$(RM) *% core vgcore*

fullclean:
	$(RPS_BUILD_CCACHE) -C
	$(MAKE) clean

__timestamp.c:
	./generate-timestamp.sh $@  > $@-tmp
	printf 'const char rps_cxx_compiler_version[]="%s";\n' "$$($(RPS_BUILD_CXX) --version | head -1)" >> $@-tmp 
	$(MV) --backup $@-tmp $@


## for temporary plugins, see build-temporary-plugin.sh
print-temporary-plugin-settings:
	@printf "RPSPLUGIN_CXX='%s'\n" $(RPS_BUILD_CXX)
	@printf "RPSPLUGIN_CXXFLAGS='%s'\n" "$(CXXFLAGS)"
	@printf "RPSPLUGIN_LDFLAGS='%s'\n"  "-rdynamic -pthread -L /usr/local/lib -L /usr/lib $(LIBES)"

## astyle indenting
indent:
	./indent-cxx-files.sh $(RPS_CORE_HEADERS) \
		$(RPS_CORE_SOURCES) 

## redump target
redump: refpersys
	./refpersys --dump=. --batch
	@if git diff -U1|grep '^[+-] ' | grep -v origitid ; then \
	  printf "make redump changed in %s\n" $$(pwd) ; \
          git diff ; \
        else \
	  git checkout rps_manifest.json ; \
            printf "make redump reached fixpoint in %s\n" $$(pwd) ; \
        fi

## alternate redump target
altredump:  ./refpersys
	./refpersys --dump=$(RPS_ALTDUMPDIR_PREFIX)_$$$$ --batch 2>&1 | tee  $(RPS_ALTDUMPDIR_PREFIX).$$$$.out


check:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --log-file=valgrind.log   \
		./refpersys

# Target to facilitate git push to both origin and GitHub mirrors
gitpush:
	@git push origin
ifeq ($(shell git remote | grep github), github)
	@git push github
else
	@echo "Add github remote as git@github.com:RefPerSys/RefPerSys.git"
	@printf "using: %s\n" 'git remote add --mirror=push github git@github.com:RefPerSys/RefPerSys.git'
endif
	@printf "\n%s git-pushed commit %s of RefPerSys, branch %s ...\n" \
	        "$$(git config --get user.email)" "$$(./generate-gitid.sh -s)" "$$(git branch | fgrep '*')"
	@git log -1 --format=oneline --abbrev=12 --abbrev-commit -q | head -1


################################################################
#### simple tests
test01: ./refpersys
#was    ./refpersys -dGUI --display=object
	@echo missing test01 ; exit 1

test02: ./refpersys
	@echo missing test02 ; exit 1

test03: ./refpersys
	@echo missing test03 ; exit 1
## eof Makefile


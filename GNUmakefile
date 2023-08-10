#!/usr/bin/gmake
## Description:
##      This file is part of the Reflective Persistent System. refpersys.org
##
##      It is its GNUmakefile, for the GNU make automation builder.
##
## Author(s):
##      Basile Starynkevitch <basile@starynkevitch.net>
##      Abhishek Chakravarti <abhishek@taranjali.org>
##      Nimesh Neema <nimeshneema@gmail.com>
##      Abdullah Siddiqui <siddiquiabdullah92@gmail.com>
##
##      © Copyright 2019 - 2023 The Reflective Persistent System Team
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

.PHONY: all debug objects lto clean plugin fullclean redump undump altredump print-plugin-settings indent \
   test00 test01 test02 test03 test04 test05 test06 test07 test08 test09 \
   test01b \
   test-load \
   analyze gitpush gitpush2 withclang 




## tell GNU make to export all variables by default
export

### The optional file $HOME/.refpersys.mk should contain definitions like
###     # file ~/.refpersys.mk
###     RPS_BUILD_CC= gcc-12
###     RPS_BUILD_CXX= g++-12
###     RPS_BUILD_GNU_LIGHTNING_SOURCEDIR= /usr/src/Libs/lightning
###  and perhaps
###     RPS_BUILD_XTRA_CFLAGS= -pg
### This enables changing C and C++ compiler versions
ifndef RPS_BUILD_CC
-include $(shell /bin/ls ~/.refpersys.mk)
endif

ifndef RPS_BUILD_GNU_LIGHTNING_SOURCEDIR
$(error undefined RPS_BUILD_GNU_LIGHTNING_SOURCEDIR GNU make variable in ~/.refpersys.mk)
endif

RPS_GIT_ID:= $(shell ./do-generate-gitid.sh)
RPS_SHORTGIT_ID:= $(shell ./do-generate-gitid.sh -s)

RPS_GIT_ORIGIN := $(shell git remote -v | grep "RefPerSys/RefPerSys.git" | head -1 | awk '{print $$1}')
RPS_GIT_MIRROR := $(shell git remote -v | grep "bstarynk/refpersys.git" | head -1 | awk '{print $$1}')

RPS_CORE_HEADERS:= $(sort $(wildcard *_rps.hh))
RPS_CORE_SOURCES:= $(sort $(filter-out $(wildcard *gui*.cc *main*.cc), $(wildcard *_rps.cc)))
#RPS_JSONRPC_SOURCES:=  $(sort $(wildcard *jsonrpc*_rps.cc))

# for the GNU bison parser generator
RPS_BISON_SOURCES:=  $(sort $(wildcard [a-z]*_rps.yy))
RPS_BISON_CPPFILES= $(patsubst %.yy,_%.cc,$(RPS_BISON_SOURCES))

RPS_ARCH := $(shell /bin/uname -m)
RPS_OPERSYS := $(shell /bin/uname -o | /bin/sed 1s/[^a-zA-Z0-9_]/_/g )

## bisonc++ parser generator obsolete in commit 51fd71d8e750 (March 2023)
# note: the ANTLR parser generator is obsolete at commit  427be821cb (March 2023)
## we did consider using the lemon parser generator inside sqlite.org
## https://www.sqlite.org/lemon.html up to commit 51fd71d8e75

RPS_COMPILER_TIMER:= /usr/bin/time --append --format='%C : %S sys, %U user, %E elapsed; %M RSS' --output=_build.time
RPS_CORE_OBJECTS = $(patsubst %.cc, %.o, $(RPS_CORE_SOURCES))
RPS_JSONRPC_OBJECTS = $(patsubst %.cc, %.o, $(RPS_JSONRPC_SOURCES))
RPS_BISON_OBJECTS = $(patsubst %.yy, %.o, $(RPS_BISON_SOURCES))


#RPS_SANITIZED_CORE_OBJECTS = $(patsubst %.cc, %.sanit.o, $(RPS_CORE_SOURCES))
#RPS_SANITIZED_BISON_OBJECTS = $(patsubst %.yy, %.sanit.o, $(RPS_BISON_SOURCES))
RPS_DEBUG_CORE_OBJECTS = $(patsubst %.cc, %.dbg.o, $(RPS_CORE_SOURCES))
RPS_LTO_CORE_OBJECTS = $(patsubst %.cc, %.lto.o, $(RPS_CORE_SOURCES))
RPS_LTO_BISON_OBJECTS = $(patsubst %.yy, %.lto.o, $(RPS_BISON_SOURCES))

#RPS_JSONRPC_CXXFLAGS = $(shell pkg-config  --cflags jsoncpp)
#RPS_JSONRPC_LIBES = $(shell pkg-config --libs jsoncpp)

#RPS_BUILD_CCACHE?= ccache
RPS_BUILD_CCACHE=
# the GCC compiler, see gcc.gnu.org
## for some reason GCC 9 dont compile
ifndef RPS_BUILD_CC
RPS_BUILD_CC?= gcc-12
endif

ifndef RPS_BUILD_CXX
RPS_BUILD_CXX?= g++-12
endif

RPS_BUILD_CXX_REALPATH= $(realpath $(RPS_BUILD_CXX))

ifndef RPS_BUILD_COMPILER_FLAGS
RPS_BUILD_COMPILER_FLAGS?= -std=gnu++17
endif

## GNU bison (parser generator) ; see www.gnu.org/software/bison/
RPS_BUILD_BISON= bison

## Generic preprocessor - see https://logological.org/gpp
RPS_GPP= gpp

ifndef RPS_INCLUDE_DIRS
RPS_INCLUDE_DIRS ?= /usr/local/include /usr/include /usr/include/jsoncpp
endif

ifndef RPS_INCLUDE_FLAGS
RPS_INCLUDE_FLAGS ?= $(patsubst %, -I%, $(RPS_INCLUDE_DIRS))
endif

ifndef RPS_BUILD_INCLUDE_FLAGS
RPS_BUILD_INCLUDE_FLAGS?=  -I. $(RPS_INCLUDE_FLAGS)
endif


RPS_BUILD_DIALECTFLAGS = -std=gnu++17
RPS_BUILD_WARNFLAGS = -Wall -Wextra
override RPS_BUILD_OPTIMFLAGS ?= -O1 -g3
RPS_BUILD_DEBUGFLAGS = -O0 -fno-inline -g3
RPS_BUILD_CODGENFLAGS = -fPIC
RPS_BUILD_LTOFLAGS = -O2 -g3 -flto
#RPS_BUILD_SANITFLAGS = -fsanitize=address
#RPS_INCLUDE_DIRS = /usr/local/include /usr/include /usr/include/jsoncpp
#RPS_INCLUDE_FLAGS = $(patsubst %, -I %, $(RPS_INCLUDE_DIRS))
#RPS_BUILD_INCLUDE_FLAGS=  -I . $(RPS_INCLUDE_FLAGS)

RPS_ALTDUMPDIR_PREFIX?= /tmp/refpersys-$(RPS_SHORTGIT_ID)

RPS_PKG_CONFIG=  pkg-config
RPS_CURLPP_CONFIG= curlpp-config

## useful packages and libraries:
##
## * JSONCPP from github.com/open-source-parsers/jsoncpp
##   to process JSON format in C++
##
## * libcurl from https://curl.se/libcurl/ to access web services
##
## * curlpp on www.curlpp.org/ to wrap them as C++ code
##
## * zlib from http://www.zlib.net/ as compression utilities
##
## * GNU lightning from https://www.gnu.org/software/lightning/ to generate code
##
## * libunistring to process Unicode or UTF-8 strings
##
## * libbacktrace from github.com/ianlancetaylor/libbacktrace to
##   inspect the call stack

RPS_PKG_NAMES= jsoncpp libcurl zlib lightning libssh2
RPS_PKG_CFLAGS:= $(shell $(RPS_CURLPP_CONFIG) --cflags) \
                 $(shell $(RPS_PKG_CONFIG) --cflags $(RPS_PKG_NAMES))

RPS_PKG_LIBS:=  $(shell $(RPS_PKG_CONFIG) --libs $(RPS_PKG_NAMES))

LIBES=  -lunistring -lbacktrace -lpthread -ldl
RM= /bin/rm -f
MV= /bin/mv
CC= $(RPS_BUILD_CCACHE) $(RPS_BUILD_CC)
CXX= $(RPS_BUILD_CCACHE) $(RPS_BUILD_CXX)
LINK.cc= $(RPS_BUILD_CXX)
CXXFLAGS= $(RPS_BUILD_DIALECTFLAGS) $(RPS_BUILD_OPTIMFLAGS) \
            $(RPS_BUILD_CODGENFLAGS) \
	    $(RPS_BUILD_WARNFLAGS) $(RPS_BUILD_INCLUDE_FLAGS) \
            -I/usr/include/jsoncpp  \
	    $(RPS_PKG_CFLAGS) \
            -DRPS_ARCH=\"$(RPS_ARCH)\" -DRPS_HAVE_ARCH_$(RPS_ARCH) \
            -DRPS_OPERSYS=\"$(RPS_OPERSYS)\" -DRPS_HAVE_OPERSYS_$(RPS_OPERSYS) \
            -DRPS_GITID=\"$(RPS_GIT_ID)\" \
            -DRPS_SHORTGITID=\"$(RPS_SHORTGIT_ID)\" \
            -I$(realpath $(RPS_BUILD_GNU_LIGHTNING_SOURCEDIR)/include/lightning/) \
            $(RPS_BUILD_COMPILER_FLAGS) $(RPS_BUILD_XTRA_CFLAGS)

LDFLAGS += -rdynamic -pthread -L /usr/local/lib -L /usr/lib

-include $(wildcard $$HOME/build-refpersys.mk)


all:
	@echo RefPerSys has RPS_ARCH: $(RPS_ARCH) and RPS_OPERSYS: $(RPS_OPERSYS)
	if [ -f refpersys ] ; then  $(MV) -f -v --backup refpersys refpersys~ ; fi
	$(RM) __timestamp.o __timestamp.c
	$(MAKE) -$(MAKEFLAGS) refpersys
	@echo all make target syncing
	sync

.SECONDARY:  __timestamp.c

debug:
	@echo making debug version of refpersys
	if [ -f refpersys ] ; then  $(MV) -f -v --backup refpersys refpersys~ ; fi
	$(RM) __timestamp.o
	$(MAKE)  -$(MAKEFLAGS) RPS_BUILD_OPTIMFLAGS='-Og -g3' refpersys


lto: 
	@echo making LTO optimized version of refpersys
	if [ -f refpersys-lto ] ; then  $(MV) -f -v --backup refpersys-lto refpersys-lto~ ; fi
	$(RM) __timestamp.o
	$(MAKE)  -$(MAKEFLAGS) refpersys-lto

refpersys: main_rps.o $(RPS_CORE_OBJECTS) $(RPS_BISON_OBJECTS)  __timestamp.o
	@echo $@: RPS_COMPILER_TIMER= $(RPS_COMPILER_TIMER)
	@echo $@: RPS_BUILD_CODGENFLAGS= $(RPS_BUILD_CODGENFLAGS)
	@echo $@: RPS_CORE_OBJECTS= $(RPS_CORE_OBJECTS)
	@echo $@: RPS_BISON_OBJECTS= $(RPS_BISON_OBJECTS)
	@echo $@: LIBES= $(LIBES)
	-sync
	$(RPS_COMPILER_TIMER) $(LINK.cc) -DREFPERYS_BUILD $(RPS_BUILD_CODGENFLAGS)  $(RPS_BUILD_XTRA_CFLAGS) -rdynamic -pie -Bdynamic \
                              main_rps.o $(RPS_CORE_OBJECTS)  $(RPS_BISON_OBJECTS)   __timestamp.o \
                 $(shell $(RPS_CURLPP_CONFIG) --libs) \
	         $(LIBES) $(RPS_PKG_LIBS)  -o $@-tmp
	$(MV) --backup $@-tmp $@
	$(MV) --backup __timestamp.c __timestamp.c~
	$(RM) __timestamp.o
	-sync

refpersys-lto: main_rps.lto.o $(RPS_LTO_CORE_OBJECTS) $(RPS_LTO_BISON_OBJECTS)  __ltotimestamp.o
	@echo $@: RPS_COMPILER_TIMER= $(RPS_COMPILER_TIMER)
	@echo $@: RPS_BUILD_CODGENFLAGS= $(RPS_BUILD_CODGENFLAGS)
	@echo $@: RPS_BUILD_LTOFLAGS= $(RPS_BUILD_LTOFLAGS)
	@echo $@: RPS_LTO_CORE_OBJECTS= $(RPS_LTO_CORE_OBJECTS)
	@echo $@: RPS_LTO_BISON_OBJECTS= $(RPS_LTO_BISON_OBJECTS)
	@echo $@: LIBES= $(LIBES)
	-sync
	$(RPS_COMPILER_TIMER) $(LINK.cc) -DREFPERYS_BUILD $(RPS_BUILD_CODGENFLAGS)  $(RPS_BUILD_XTRA_CFLAGS) $(RPS_BUILD_LTOFLAGS) -rdynamic -pie -Bdynamic \
                              main_rps.lto.o $(RPS_LTO_CORE_OBJECTS)  $(RPS_LTO_BISON_OBJECTS)   __ltotimestamp.o \
                 $(shell $(RPS_CURLPP_CONFIG) --libs) \
	         $(LIBES) $(RPS_PKG_LIBS)  -o $@-tmp
	$(MV) --backup $@-tmp $@
	$(MV) --backup __ltotimestamp.c __ltotimestamp.c~
	$(RM) __ltotimestamp.o
	-sync

#sanitized-refpersys:  main_rps.sanit.o $(RPS_SANITIZED_CORE_OBJECTS)  $(RPS_SANITIZED_BISON_OBJECTS) __timestamp.o
#       $(RPS_COMPILER_TIMER) $(LINK.cc)  $(RPS_BUILD_SANITFLAGS) \
#          $(RPS_SANITIZED_CORE_OBJECTS)  __timestamp.o \
#          $(LIBES) -o $@-tmp
#       $(MV) --backup $@-tmp $@
#       $(MV) --backup __timestamp.c __timestamp.c~
#       $(RM) __timestamp.o

## it is assumed that clang and clang++ are some symbolic links to
## clang compiler in the user's $PATH .... (suppose it has $HOME/bin/ ...)
## for example: ln -s /usr/bin/clang-15 $HOME/bin/clang
withclang:
	$(MAKE) clean
	env RPS_BUILD_CC=clang RPS_BUILD_CXX=clang++ $(MAKE) refpersys
## the below target don't work yet
#- dbg-refpersys:  $(RPS_DEBUG_CORE_OBJECTS) __timestamp.o
#-         env RPS_BUILD_OPTIMFLAGS='$(RPS_BUILD_DEBUGFLAGS)' $(MAKE) $(MAKEFLAGS) -e  $(RPS_DEBUG_CORE_OBJECTS) 
#-         $(LINK.cc)  $(RPS_BUILD_DEBUGFLAGS) \
#-            $(RPS_DEBUG_CORE_OBJECTS)   $(RPS_DEBUG_BISON_OBJECTS) __timestamp.o \
#-            $(LIBES) -o $@-tmp
#-         $(MV) --backup $@-tmp $@
#-         $(MV) --backup __timestamp.c __timestamp.c~
#-         $x(RM) __timestamp.o

objects:  $(RPS_CORE_OBJECTS)  $(RPS_BISON_OBJECTS) 

$(RPS_CORE_OBJECTS): $(RPS_CORE_HEADERS) $(RPS_CORE_SOURCES)

%.o: %.cc refpersys.hh.gch
	$(RPS_COMPILER_TIMER) $(COMPILE.cc) -o $@ $<
	sync


%.lto.o: %.cc refpersys.hh.gch
	$(RPS_COMPILER_TIMER) $(COMPILE.cc) $(RPS_BUILD_LTOFLAGS) -o $@ $<
	sync



#%.sanit.o: %.cc refpersys.hh.sanit.gch
#	$(RPS_COMPILER_TIMER) 	$(COMPILE.cc) $(RPS_BUILD_SANITFLAGS) -o $@ $<

%.dbg.o: %.cc refpersys.hh.dbg.gch
	$(RPS_COMPILER_TIMER) $(COMPILE.cc) $(RPS_BUILD_DEBUGFLAGS) -o $@ $<

%.lto.o: %.cc refpersys.hh.dbg.gch
	$(RPS_COMPILER_TIMER) $(COMPILE.cc) $(RPS_BUILD_LTOFLAGS) -o $@ $<

%.ii: %.cc refpersys.hh.gch
	$(COMPILE.cc) -C -E $< | sed s:^#://#:g > $@
	astyle -v -s2 --style=gnu $@

%.cc: %.yy
	/usr/bin/printf "GNU make generating %s from %s\n" $@ $<
	/usr/bin/printenv
	$(RPS_COMPILER_TIMER) $(RPS_BUILD_BISON) $(RPS_BUILD_BISON_FLAGS) --output=$@ $<

%_gpprps.cc: %_gpprps.yy
	/usr/bin/printf "GNU make generating using GPP+BISON %s from %s\n" $@ $<
# GPP macro start sequence is @gpprps: (including at sign and colon)
# GPP macro end sequence for a call without arguments is the empty string
# GPP argument start sequence is the leftparen
# GPP argument separator is the comma
# GPP argument end sequence is the rightparen
	$(RPS_COMPILER_TIMER) $(RPS_GPP) -U "@gpprps:"    ""    "("    ","  ")"  "("  ")" "$#" "\\"   \
			      -M  "@gppmacrps:"  "\n"  " "   " "    "\n"   "("   ")"   \
		              -x -o _$<
	$(RPS_COMPILER_TIMER) $(RPS_BUILD_BISON) $(RPS_BUILD_BISON_FLAGS) --output=$@ _$<

%.cc: %.yyy
	$(RPS_COMPILER_TIMER) $(RPS_BISONCPP) --show-filenames --verbose --thread-safe $<

# see https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html 

refpersys.hh.gch: refpersys.hh oid_rps.hh $(wildcard generated/rps*.hh)
	$(RPS_COMPILER_TIMER) $(COMPILE.cc) -c -o $@ $<
#refpersys.hh.sanit.gch: refpersys.hh oid_rps.hh $(wildcard generated/rps*.hh)
#	$(RPS_COMPILER_TIMER) $(COMPILE.cc)  $(RPS_BUILD_SANITFLAGS) -c -o $@ $<
refpersys.hh.dbg.gch: refpersys.hh oid_rps.hh $(wildcard generated/rps*.hh)
	$(RPS_COMPILER_TIMER) $(COMPILE.cc)  $(RPS_BUILD_DEBUGFLAGS) -c -o $@ $<



################
clean:
	if [ -f __timestamp.c ]; then /bin/sed '1,$$s:/home/[a-zA-Z]*:@REFPERSYS_HOME:g' __timestamp.c > %%__timestamp.c%% ; fi
	$(RM) *.o *.orig *~ refpersys *.gch *~ _build.time
	$(RM) sanitized-refpersys
	$(RM) refpersys-lto
	$(RM) *.so
	$(RM) *.moc.hh
	$(RM) _*.hh _*.cc __timestamp.* generated/*~ _*.mk
	$(RM) persistore/*~ persistore/*%
	$(RM) plugins/*~  plugins/*% plugins/*.cc.orig plugins/*.so *.so
	$(RM) generated/__rps*.so
	$(RM) *.ii
	$(RM) [a-z]*% core vgcore*
	$(RM) -rf bld
	$(RM) $(patsubst %.yy, %.cc, $(RPS_BISON_SOURCES)) \
	      $(patsubst %.yy, %.output, $(RPS_BISON_SOURCES)) \
	      *.tmp


## TODO: we might need to have some conventions, maybe comments like ///|| in the
## first fifty lines of generated C++ to give extra compilation flags
## or extra libraries in generated binary modules.
### **generated binary modules.
## Nota Bene: see Rps_Dumper class and dump_rps.cc
generated/__rps_$(RPS_ARCH)_$(RPS_OPERSYS)_%-mod.so: generated/_%.cc refpersys.hh.gch refpersys.hh build-plugin.sh
	 ./build-plugin.sh $< $@


## usual invocation: make plugin RPS_PLUGIN_SOURCE=/tmp/foo.cc RPS_PLUGIN_SHARED_OBJECT=/tmp/foo.so
## see also our ./build-plugin.sh script
plugin: | ./build-plugin.sh
	if [ -n "$(RPS_PLUGIN_SOURCE)" ]; then echo missing RPS_PLUGIN_SOURCE > /dev/stderr ; exit 1; fi
	if [ -n "$(RPS_PLUGIN_SHARED_OBJECT)" ]; then echo missing RPS_PLUGIN_SHARED_OBJECT  > /dev/stderr ; exit 1; fi
	./build-plugin.sh $(RPS_PLUGIN_SOURCE) $(RPS_PLUGIN_SHARED_OBJECT)


fullclean:
	$(RPS_BUILD_CCACHE) -C
	$(MAKE) clean

__timestamp.c: GNUmakefile do-generate-timestamp.sh
	echo $@:
	printf '/// $@ generated by do-generate-timestamp.sh\n' >> $@-tmp
	./do-generate-timestamp.sh $@  >> $@-tmp
	printf '/// below is generated in GNUmakefile by %s\n' $(realpath GNUmakefile) >> $@-tmp
	printf 'const char rps_cxx_compiler_command[]="%s";\n' $(RPS_BUILD_CXX) >> $@-tmp
	printf 'const char rps_cxx_compiler_realpath[]="%s";\n' '$(shell /bin/which $(RPS_BUILD_CXX))' >> $@-tmp
	printf 'const char rps_cxx_compiler_version[]="%s";\n' "$$($(RPS_BUILD_CXX) --version | head -1)" >> $@-tmp
	printf 'const char rps_build_xtra_cflags[]="%s";\n' '$(RPS_BUILD_XTRA_CFLAGS)' >> $@-tmp
	printf 'const int rps_is_link_time_optimized = 0;\n' >> $@-tmp
	printf '\n/// the GNU bison parser generator from www.gnu.org/software/bison/\n' >> $@-tmp
	printf 'const char rps_gnubison_command[]="%s";\n' "$(RPS_BUILD_BISON)" >> $@-tmp
	printf 'const char rps_gnubison_realpath[]="%s";\n' '$(shell /bin/which $(RPS_BUILD_BISON))' >> $@-tmp
	printf 'const char rps_gnubison_version[]="%s";\n' "$$($(RPS_BUILD_BISON) --version | head -1)" >> $@-tmp
	printf '\n/// the generic preprocessor from logological.org/gpp\n' >> $@-tmp
	printf 'const char rps_gpp_command[]="%s";\n' '$(RPS_GPP)' >> $@-tmp
	printf 'const char rps_gpp_realpath[]="%s";\n' '$(shell /bin/which $(RPS_GPP))' >> $@-tmp
	printf 'const char rps_gpp_version[]="%s";\n' "$$($(RPS_GPP) --version | head -1)" >> $@-tmp
	printf '\n\n' >> $@-tmp
	printf '/// end of generated file $@\n' >> $@-tmp
	$(MV) --backup $@-tmp $@


__ltotimestamp.c: GNUmakefile do-generate-timestamp.sh
	echo $@:
	printf '/// $@ generated by do-generate-timestamp.sh\n' >> $@-tmp
	./do-generate-timestamp.sh $@  >> $@-tmp
	printf '/// below is generated in GNUmakefile by %s\n' $(realpath GNUmakefile) >> $@-tmp
	printf 'const char rps_cxx_compiler_command[]="%s";\n' $(RPS_BUILD_CXX) >> $@-tmp
	printf 'const char rps_cxx_compiler_realpath[]="%s";\n' '$(shell /bin/which $(RPS_BUILD_CXX))' >> $@-tmp
	printf 'const char rps_cxx_compiler_version[]="%s";\n' "$$($(RPS_BUILD_CXX) --version | head -1)" >> $@-tmp
	printf 'const char rps_build_xtra_cflags[]="%s %s";\n' '$(RPS_BUILD_XTRA_CFLAGS) $(RPS_BUILD_LTOFLAGS)' >> $@-tmp
	printf 'const int rps_is_link_time_optimized = 1;\n' >> $@-tmp
	printf 'const char rps_gnubison_command[]="%s";\n' "$(RPS_BUILD_BISON)" >> $@-tmp
	printf 'const char rps_gnubison_realpath[]="%s";\n' '$(shell /bin/which $(RPS_BUILD_BISON))' >> $@-tmp
	printf 'const char rps_gnubison_version[]="%s";\n' "$$($(RPS_BUILD_BISON) --version | head -1)" >> $@-tmp
	printf 'const char rps_shortgitid[] = "%s";\n' "$(RPS_SHORTGIT_ID)" >> $@-tmp
	printf '/// end of generated file $@\n' >> $@-tmp
	$(MV) --backup $@-tmp $@


## for plugins, see build-plugin.sh
print-plugin-settings:
	@printf "RPSPLUGIN_CXX='%s'\n" $(RPS_BUILD_CXX)
	@printf "RPSPLUGIN_CXXFLAGS='%s'\n" "$(CXXFLAGS)"
	@printf "RPSPLUGIN_LDFLAGS='%s'\n"  "-rdynamic -pthread -L /usr/local/lib -L /usr/lib $(LIBES)"

## astyle indenting
indent:
	./indent-cxx-files.sh $(RPS_CORE_HEADERS) \
		$(RPS_CORE_SOURCES) 
	./indent-cxx-files.sh $(sort $(wildcard plugins/*.cc))

## redump target
redump: refpersys
	./refpersys --dump=. --batch
	@if git diff -U1|grep '^[+-] ' | grep -v origitid ; then \
	  printf "make redump changed in %s git %s\n" $$(pwd)  $(RPS_SHORTGIT_ID); \
          git diff ; \
        else \
	  git checkout rps_manifest.json ; \
            printf "make redump reached fixpoint in %s git %s\n" $$(pwd) $(RPS_SHORTGIT_ID) ; \
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
	@echo RefPerSys git pushing.... ; grep -2 url .git/config
	@git push origin
ifeq ($(shell git remote | grep github), github)
	@git push github
else
	@echo "Add github remote as git@github.com:RefPerSys/RefPerSys.git"
	@printf "using: %s\n" 'git remote add --mirror=push github git@github.com:RefPerSys/RefPerSys.git'
endif
	@printf "\n%s git-pushed commit %s of RefPerSys, branch %s ...\n" \
	        "$$(git config --get user.email)" "$$(./do-generate-gitid.sh -s)" "$$(git branch | fgrep '*')"
	@git log -1 --format=oneline --abbrev=12 --abbrev-commit -q | head -1
	if [ -x $$HOME/bin/push-refpersys ]; then $$HOME/bin/push-refpersys $(shell /bin/pwd) $(RPS_SHORTGIT_ID); fi

gitpush2:
ifeq ($(RPS_GIT_ORIGIN), )
	git remote add origin https://github.com/RefPerSys/RefPerSys.git
	echo "Added GitHub repository as remote, run make gitpush2 again..."
else
	git push $(RPS_GIT_ORIGIN) master
endif
ifeq ($(RPS_GIT_MIRROR), )
	git remote add mirror https://gitlab.com/bstarynk/refpersys.git
	echo "Added GitLab repository as remote, run make gitpush2 again..."
else
	git push $(RPS_GIT_MIRROR) master
endif


## undump target, use git to checkout what last redump changed....
## see https://devtutorial.io/how-to-get-a-list-of-the-changed-files-in-git-p1201.html
undump:
	./undump-refpersys.sh


analyze:
	make clean
	mkdir -p bld
	scan-build -v -V -o bld make -j4


################################################################
#### simple tests
test00: ./refpersys
	@printf '\n\n\n////test00 first\n'
	./refpersys  -AREPL  --test-repl-lexer 'show help' -B
	@printf '\n\n\n////test00 second\n'
	./refpersys  -AREPL  --test-repl-lexer 'show RefPerSys_system' -B
	@printf '\n\n\n////test00 third\n'
	./refpersys  -AREPL  --test-repl-lexer 'show (1 + 2)' -B
	@printf '\n\n\n////test00 help REPL command\n'
	./refpersys -AREPL -c help -B
	@printf '\n\n\n////test00 FINISHED¤\n'

test01: ./refpersys
	@echo test01 testing simple show help with a lot of debug
	./refpersys -AREPL -c 'show help' -B
	@printf '\n\n\n////test01 FINISHED¤\n'

test01b: debug
	./refpersys -AREPL,LOW_REPL  -c 'show help' -B
	@printf '\n\n\n////test01b FINISHED¤\n'

test02: ./refpersys
	./refpersys -AREPL  -c 'show RefPerSys_system' -B
	@printf '\n\n\n////test02 FINISHED¤\n'

test03: ./refpersys
	./refpersys -AREPL  -c 'show 1 + 2' -B
	@printf '\n\n\n////test03 FINISHED¤\n'

test04: ./refpersys
	./refpersys -AREPL  -c 'show  1 * 2 + 3 * 4' -B
	@printf '\n\n\n////test04 FINISHED¤\n'

test05: ./refpersys
	./refpersys -AREPL  -c 'show (1 + 2) ' -B
	@printf '\n\n\n////test05 FINISHED¤\n'

test06: ./refpersys
	./refpersys -AREPL  -c 'show 1' -B
	@printf '\n\n\n////test06 FINISHED¤\n'

test07: ./refpersys
	@echo missing test07 ; exit 1

test08: ./refpersys
	@echo missing test08 ; exit 1

test09: ./refpersys
	@echo missing test09 ; exit 1

test-load: ./refpersys
	./refpersys --batch
	@printf '\n\n\n////test-load FINISHED¤\n'
## eof GNUmakefile


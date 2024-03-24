#!/usr/bin/gmake
## SPDX-License-Identifier: GPL-3.0-or-later
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
##      © Copyright 2019 - 2024 The Reflective Persistent System Team
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
##    along with this program.  If not, see <http://www.gnu.org/licenses/>



## tell GNU make to export all variables by default
export

RPS_GIT_ID:= $(shell ./do-generate-gitid.sh)
RPS_SHORTGIT_ID:= $(shell ./do-generate-gitid.sh -s)
#                                                                
.DEFAULT_GOAL: refpersys
.PHONY: all config objects clean gitpush gitpush2 print-plugin-settings indent redump plugins

SYNC=/bin/sync

## a formatter to restrict width to 75 columns
FMT=/usr/bin/fmt

## a C++ indenter and its flags
ASTYLE=/usr/bin/astyle
ASTYLEFLAGS= --verbose --style=gnu  --indent=spaces=2  --convert-tabs

-include _config-refpersys.mk

### Human hand-written C++ sources
REFPERSYS_HUMAN_CPP_SOURCES=$(wildcard *_rps.cc)

### corresponding object files
REFPERSYS_HUMAN_CPP_OBJECTS=$(patsubst %.cc, %.o, $(REFPERSYS_HUMAN_CPP_SOURCES))

### Generated C++ sources
REFPERSYS_GENERATED_CPP_SOURCES := $(wildcard generated/*.cc)

### corresponding object files
REFPERSYS_GENERATED_CPP_OBJECTS=$(patsubst %.cc, %.o, $(REFPERSYS_GENERATED_CPP_SOURCES))

### required libraries not being known to pkg-config
## unistring is https://www.gnu.org/software/libunistring/
## backtrace is https://github.com/ianlancetaylor/libbacktrace (also inside GCC source)
REFPERSYS_NEEDED_LIBRARIES= -lunistring -lbacktrace

### desired plugins (their basename under plugins/)
REFPERSYS_DESIRED_PLUGIN_BASENAMES= rpsplug_simpinterp
################
all:

	@/usr/bin/printf "make features: %s\n" "$(.FEATURES)" | $(FMT)
	$(MAKE) do-configure-refpersys
	@/usr/bin/printf "hand-written C++ code: %s\n" "$(REFPERSYS_HUMAN_CPP_SOURCES)" | $(FMT)
	@if [ ! -f _config-refpersys.mk ]; then \
	   echo missing _config-refpersys.mk for GNUmakefile > /dev/stderr; \
	   echo run $(MAKE) config > /dev/stderr ; \
	   exit 1 ; \
	fi
	$(MAKE) refpersys
	$(MAKE) plugins


.SECONDARY:  __timestamp.c  #gramrepl_rps.yy gramrepl_rps.cc  gramrepl_rps.hh
	$(SYNC)

config: do-configure-refpersys do-scan-pkgconfig GNUmakefile
	./do-configure-refpersys
	$(MAKE) _scanned-pkgconfig.mk

do-configure-refpersys: do-configure-refpersys.c |GNUmakefile do-generate-gitid.sh
	$(CC) -Wall -Wextra -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" \
              -O -g $^ -o $@ -lreadline
## if readline library is unavailable add -DWITHOUT_READLINE above

do-scan-pkgconfig: do-scan-pkgconfig.c |GNUmakefile do-generate-gitid.sh
	$(CC) -Wall -Wextra -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" \
              -O -g $^ -o $@

clean:
	$(RM) tmp* *~ *.o do-configure-refpersys refpersys
	$(RM) *% %~
	$(RM) *.orig
	$(RM) */*~ */*.orig
	$(RM) */*.so

-include _scanned-pkgconfig.mk

_scanned-pkgconfig.mk: $(REFPERSYS_HUMAN_CPP_SOURCES) |GNUmakefile do-scan-pkgconfig
	./do-scan-pkgconfig refpersys.hh $(REFPERSYS_HUMAN_CPP_SOURCES) > $@

__timestamp.c: do-generate-timestamp.sh GNUmakefile
	echo MAKE is $(MAKE)
	env MAKE=$(shell /bin/which gmake) CXX=$(REFPERSYS_CXX) ./do-generate-timestamp.sh $@ > $@

__timestamp.o: __timestamp.c |GNUmakefile
	$(CC) -fPIC -c -O -g -Wall -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" $^ -o $@

refpersys: $(REFPERSYS_HUMAN_CPP_OBJECTS)  $(REFPERSYS_GENERATED_CPP_OBJECTS) __timestamp.o |  GNUmakefile
	@if [ -z "$(REFPERSYS_CXX)" ]; then echo should make config ; exit 1; fi
	@echo RefPerSys human C++ source files $(REFPERSYS_HUMAN_CPP_SOURCES)
#       @echo RefPerSys human C++ object files $(REFPERSYS_HUMAN_CPP_OBJECTS)
	@echo RefPerSys generated C++ files $(REFPERSYS_GENERATED_CPP_SOURCES)
#	@echo RefPerSys generated C++ object files $(REFPERSYS_GENERATED_CPP_OBJECTS)
	@echo PACKAGES_LIST is $(PACKAGES_LIST)
	$(MAKE) $(REFPERSYS_HUMAN_CPP_OBJECTS) $(REFPERSYS_GENERATED_CPP_OBJECTS) __timestamp.o
	@if [ -x $@ ]; then /bin/mv -v --backup $@ $@~ ; fi
	$(REFPERSYS_CXX) -rdynamic -o $@ $(REFPERSYS_HUMAN_CPP_OBJECTS) $(REFPERSYS_GENERATED_CPP_OBJECTS) __timestamp.o \
              $(REFPERSYS_NEEDED_LIBRARIES) \
	      $(shell pkg-config --libs $(sort $(PACKAGES_LIST))) -ldl
	@/bin/mv -v --backup __timestamp.c __timestamp.c%
	@/bin/rm -vf __timestamp.o

plugins: refpersys $(patsubst %, plugins/%.so, $(REFPERSYS_DESIRED_PLUGIN_BASENAMES)) |GNUmakefile

plugins/%.so: plugins/%.cc refpersys.hh build-plugin.sh |GNUmakefile
	@printf "RefPerSys-gnumake building plugin %s from source %s in %s\n" "$@"  "$<"  "$$(/bin/pwd)"
	env PATH=$$PATH $(shell $(MAKE) print-plugin-settings) ./build-plugin.sh $< $@

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
	$(SYNC)

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
	$(SYNC)
%_rps.o: %_rps.cc refpersys.hh
	echo dollar-less-F is $(<F)
	echo basename-dollar-less-F is $(basename $(<F))
	echo pkglist is $(PKGLIST_$(basename $(<F)))	
	$(REFPERSYS_CXX) $(REFPERSYS_PREPRO_FLAGS) $(REFPERSYS_COMPILER_FLAGS) \
	       $(shell pkg-config --cflags $(PKGLIST_refpersys)) \
               $(shell pkg-config --cflags $(PKGLIST_$(basename $(<F)))) \
               -DRPS_THIS_SOURCE=\"$<\" -DRPS_GITID=\"$(RPS_GIT_ID)\"  \
               -DRPS_SHORTGITID=\"$(RPS_SHORTGIT_ID)\" \
	       -c -o $@ $<
	$(SYNC)

## for plugins, see build-plugin.sh
print-plugin-settings:
	@printf "RPSPLUGIN_CXX='%s'\n" $(REFPERSYS_CXX)
	@printf "RPSPLUGIN_CXXFLAGS='%s'\n" "$(REFPERSYS_PREPRO_FLAGS) $(REFPERSYS_COMPILER_FLAGS)"
	@printf "RPSPLUGIN_LDFLAGS='%s'\n"  "-rdynamic -pthread -L /usr/local/lib -L /usr/lib $(LIBES)"

indent:
	$(ASTYLE) $(ASTYLEFLAGS) refpersys.hh
	$(ASTYLE) $(ASTYLEFLAGS) oid_rps.hh
	$(ASTYLE) $(ASTYLEFLAGS) inline_rps.hh
	for f in $(REFPERSYS_HUMAN_CPP_SOURCES) ; do \
	    $(ASTYLE) $(ASTYLEFLAGS) $$f ; done
	for p in $(patsubst %, plugins/%.cc, $(REFPERSYS_DESIRED_PLUGIN_BASENAMES)) ; do \
	    $(ASTYLE) $(ASTYLEFLAGS) $$p ; done

## redump target
redump: refpersys
	./refpersys --dump=. --batch --run-name=$@
	@if git diff -U1|grep '^[+-] ' | grep -v origitid ; then \
	  printf "make redump changed in %s git %s\n" $$(pwd)  $(RPS_SHORTGIT_ID); \
          git diff ; \
        else \
	  git checkout rps_manifest.json ; \
            printf "make redump reached fixpoint in %s git %s\n" $$(pwd) $(RPS_SHORTGIT_ID) ; \
        fi
	$(SYNC)

## alternate redump target
altredump:  ./refpersys
	./refpersys --dump=$(RPS_ALTDUMPDIR_PREFIX)_$$$$ --batch --run-name=$@ 2>&1 | tee  $(RPS_ALTDUMPDIR_PREFIX).$$$$.out
	$(SYNC)

## eof GNUmakefile


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
RPS_MAKE:= $(MAKE)
RPS_BISON := /usr/bin/bison
RPS_HOST := $(shell /bin/hostname -f)
RPS_ARCH := $(shell /bin/uname -m)
RPS_OPERSYS := $(shell /bin/uname -o | /bin/sed 1s/[^a-zA-Z0-9_]/_/g )
RPS_ATSHARP := $(shell printf '@#')
#                                                                
.DEFAULT_GOAL: refpersys
.PHONY: all config objects clean distclean gitpush gitpush2 \
        print-plugin-settings indent redump clean-plugins plugins \
        test00 test01 test01b test01c test01d test01e test01f \
        test02 test03 test05 test06 test07 test07a test08 test09 test-load \
	testfltk1 testfltk2 testfltk3 testfltk4

SYNC=/bin/sync

## a formatter to restrict width to 75 columns
FMT=/usr/bin/fmt

## a C++ indenter and its flags
ASTYLE=/usr/bin/astyle
ASTYLEFLAGS= --verbose --style=gnu  --indent=spaces=2  --convert-tabs

CFLAGS= -O -g

-include _config-refpersys.mk

### Human hand-written C++ sources
REFPERSYS_HUMAN_CPP_SOURCES=$(wildcard *_rps.cc)

### corresponding object files
REFPERSYS_HUMAN_CPP_OBJECTS=$(patsubst %.cc, %.o, $(REFPERSYS_HUMAN_CPP_SOURCES))

### Generated C++ sources
REFPERSYS_GENERATED_CPP_SOURCES := $(wildcard generated/*.cc)

### corresponding object files
REFPERSYS_GENERATED_CPP_OBJECTS=$(patsubst %.cc, %.o, $(REFPERSYS_GENERATED_CPP_SOURCES))

## altredump is dumping into....
RPS_ALTDUMPDIR_PREFIX?= /tmp/refpersys-$(RPS_SHORTGIT_ID)

### required libraries not being known to pkg-config
## unistring is https://www.gnu.org/software/libunistring/
## backtrace is https://github.com/ianlancetaylor/libbacktrace (also inside GCC source)
## libgccjit is https://gcc.gnu.org/onlinedocs/jit/
REFPERSYS_NEEDED_LIBRARIES= -lunistring -lbacktrace -lgccjit

### desired plugins (their basename under plugins_dir/)
REFPERSYS_DESIRED_PLUGIN_BASENAMES= rpsplug_simpinterp

all:
	@if [ -z "$(REFPERSYS_TOPDIR)" ]; then \
		REFPERSYS_TOPDIR="$(pwd)"; \
		/usr/bin/printf "missing REFPERSYS_TOPDIR, using default\n" > /dev/stderr; \
	fi

	@/usr/bin/printf "make features: %s\n" "$(.FEATURES)" | $(FMT)
	$(MAKE) do-configure-refpersys
	@/usr/bin/printf "hand-written C++ code: %s\n" "$(REFPERSYS_HUMAN_CPP_SOURCES)" | $(FMT)
	@if [ ! -f _config-refpersys.mk ]; then \
	   echo missing _config-refpersys.mk for GNUmakefile > /dev/stderr; \
	   echo run $(MAKE) config > /dev/stderr ; \
	   exit 1 ; \
	fi
	$(MAKE) refpersys
	@/usr/bin/printf "\n\n\nMaking RefPerSys plugins\n\n"
	$(MAKE) plugins


.SECONDARY:  __timestamp.c  #gramrepl_rps.yy gramrepl_rps.cc  gramrepl_rps.hh
	$(SYNC)

config: do-configure-refpersys do-scan-pkgconfig GNUmakefile
	./do-configure-refpersys
	$(MAKE) _scanned-pkgconfig.mk

do-configure-refpersys: do-configure-refpersys.c |GNUmakefile do-generate-gitid.sh
	$(CC) -Wall -Wextra -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" \
              $(CFLAGS) $^ -o $@ -lreadline
## if GNU readline library is unavailable add -DWITHOUT_READLINE above
## and remove the -lreadline above

do-scan-pkgconfig: do-scan-pkgconfig.c |GNUmakefile do-generate-gitid.sh
	$(CC) -Wall -Wextra -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" \
              $(CFLAGS) $^ -o $@

do-build-plugin: do-build-plugin.cc __timestamp.c
	$(CXX) -Wall -Wextra  -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" -O -g $^ -o $@



clean: clean-plugins
	$(RM) tmp* *~ *.o do-configure-refpersys do-build-plugin refpersys
	$(RM) *% %~
	$(RM) *.gch
	$(RM) *.orig
	$(RM) */*~ */*% */*.orig
	$(RM) */*.so
	$(RM) *.ii

clean-plugins:
	$(RM) -v plugins_dir/*.o
	$(RM) -v plugins_dir/*.so
	$(RM) -v plugins_dir/_*
	$(RM) -v _rpsplug* */_rpsplug*

distclean: clean
	$(RM) build.time  _config-refpersys.mk  _scanned-pkgconfig.mk  __timestamp.*
-include _scanned-pkgconfig.mk

_scanned-pkgconfig.mk: $(REFPERSYS_HUMAN_CPP_SOURCES) |GNUmakefile do-scan-pkgconfig
	./do-scan-pkgconfig refpersys.hh $(REFPERSYS_HUMAN_CPP_SOURCES) > $@

__timestamp.c: do-generate-timestamp.sh GNUmakefile
	@echo MAKE is "$(MAKE)" CXX is "$(REFPERSYS_CXX)" GPP is "$(REFPERSYS_GPP)" and "$(GPP)"
	+env "MAKE=$(shell /bin/which gmake)" "CXX=$(REFPERSYS_CXX)" "GPP=$(REFPERSYS_GPP)" "CXXFLAGS=$(REFPERSYS_PREPRO_FLAGS) $(REFPERSYS_COMPILER_FLAGS)" ./do-generate-timestamp.sh $@ > $@

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
	$(REFPERSYS_CXX) -rdynamic -o $@ \
             $(REFPERSYS_HUMAN_CPP_OBJECTS) \
             $(REFPERSYS_GENERATED_CPP_OBJECTS) __timestamp.o \
	      $(shell $(REFPERSYS_CXX) -print-file-name=libbacktrace.a) \
              -L/usr/local/lib $(REFPERSYS_NEEDED_LIBRARIES) \
              $(shell $(REFPERSYS_FLTKCONFIG) -g --ldflags) \
              $(shell pkg-config --libs $(sort $(PACKAGES_LIST))) -ldl
	@/bin/mv -v --backup __timestamp.c __timestamp.c%
	@/bin/rm -vf __timestamp.o

%.ii: %.cc | refpersys.hh GNUmakefile

plugins: refpersys $(patsubst %, plugins_dir/%.so, $(REFPERSYS_DESIRED_PLUGIN_BASENAMES)) |GNUmakefile build-plugin.sh do-scan-pkgconfig

plugins_dir/%.so: plugins_dir/%.cc refpersys.hh build-plugin.sh |GNUmakefile
	@printf "\n\nRefPerSys-gnumake building plugin %s from source %s in %s\n" "$@"  "$<"  "$$(/bin/pwd)"
	@printf "RefPerSys-gnumaking plugin %s MAKE is %s RPS_MAKE is %s\n" "$@" "$(MAKE)" "$(RPS_MAKE)"
	env PATH=$$PATH $(shell $(RPS_MAKE) -s print-plugin-settings) ./build-plugin.sh $< $@

plugins_dir/_rpsplug_gramrepl.yy: attic/gramrepl_rps.yy.gpp refpersys.hh refpersys |GNUmakefile _config-refpersys.mk  _scanned-pkgconfig.mk
	@printf "RefPerSys-gnumake building plugin GNU bison code %s from %s using $(REFPERSYS_GPP) in %s\n" "$@"  "$<"  "$$(/bin/pwd)"
	$(REFPERSYS_GPP) -x -I generated/ -I . \
            -DRPS_SHORTGIT="$(RPS_SHORTGIT_ID)" \
            -DRPS_HOST=$(RPS_HOST) \
            -DRPS_ARCH=$(RPS_ARCH) \
            -DRPS_OPERSYS=$(RPS_OPERSYS) \
            -DRPS_GPP_INPUT="$<"    -DRPS_GPP_OUTPUT="$@"    \
            -DRPS_GPP_INPUT_BASENAME="$(basename $<)" \
            -U  '@&'  '&@'  '('  '&,'  ')'  '('  ')' '$(RPS_ATSHARP)'   '\\'  \
            -o $@ $<


plugins_dir/_rpsplug_gramrepl.cc: plugins_dir/_rpsplug_gramrepl.yy
	$(RPS_BISON) --verbose --no-lines --warnings=all --color=tty \
                     --language=c++ --debug  --token-table \
                     --header=plugins_dir/_rpsplug_gramrepl.hh \
                     --output=$@ \
                   $<

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
	echo pkglist-refpersys is $(PKGLIST_refpersys)
	echo pkglist-$(basename $(<F)) is $(PKGLIST_$(basename $(<F)))	
	$(REFPERSYS_CXX) $(REFPERSYS_PREPRO_FLAGS) $(REFPERSYS_COMPILER_FLAGS) \
	       $(shell pkg-config --cflags $(PKGLIST_refpersys)) \
               $(shell pkg-config --cflags $(PKGLIST_$(basename $(<F)))) \
               -DRPS_THIS_SOURCE=\"$<\" -DRPS_GITID=\"$(RPS_GIT_ID)\"  \
               -DRPS_SHORTGITID=\"$(RPS_SHORTGIT_ID)\" \
	       -c -o $@ $<
	$(SYNC)

%_rps.ii:  %_rps.cc refpersys.hh $(wildcard generated/rps*.hh) | GNUmakefile _config-refpersys.mk
	echo dollar-less-F is $(<F)
	echo basename-dollar-less-F is $(basename $(<F))
	echo pkglist-refpersys is $(PKGLIST_refpersys)
	echo pkglist-$(basename $(<F)) is $(PKGLIST_$(basename $(<F)))
	$(REFPERSYS_CXX) -C -E $(REFPERSYS_PREPRO_FLAGS) $(REFPERSYS_COMPILER_FLAGS) \
	       $(shell pkg-config --cflags $(PKGLIST_refpersys)) \
               $(shell pkg-config --cflags $(PKGLIST_$(basename $(<F)))) \
               -DRPS_THIS_SOURCE=\"$<\" -DRPS_GITID=\"$(RPS_GIT_ID)\"  \
               -DRPS_SHORTGITID=\"$(RPS_SHORTGIT_ID)\" \
	       $< | /bin/sed 's:^#://#:g' | $(ASTYLE) $(ASTYLEFLAGS)  > $@

fltk_rps.o: fltk_rps.cc refpersys.hh  $(wildcard generated/rps*.hh) | GNUmakefile _config-refpersys.mk
	echo dollar-less-F is $(<F)
	echo basename-dollar-less-F is $(basename $(<F))
	echo pkglist-refpersys is $(PKGLIST_refpersys)
	echo pkglist-$(basename $(<F)) is $(PKGLIST_$(basename $(<F)))
	$(REFPERSYS_CXX) $(REFPERSYS_PREPRO_FLAGS) \
            $(REFPERSYS_COMPILER_FLAGS) \
	       $(shell pkg-config --cflags $(PKGLIST_refpersys)) \
               $(shell pkg-config --cflags $(PKGLIST_$(basename $(<F)))) \
               -DRPS_THIS_SOURCE=\"$<\" -DRPS_GITID=\"$(RPS_GIT_ID)\"  \
               -DRPS_SHORTGITID=\"$(RPS_SHORTGIT_ID)\" \
            -DRPS_SHORTGIT="$(RPS_SHORTGIT_ID)" \
            -DRPS_HOST=$(RPS_HOST) \
            -DRPS_ARCH=$(RPS_ARCH) \
            -DRPS_OPERSYS=$(RPS_OPERSYS) \
	    $(shell $(REFPERSYS_FLTKCONFIG) -g --cflags) \
	       -c -o $@ $<
	$(SYNC)

fltk_rps.ii:  fltk_rps.cc refpersys.hh  $(wildcard generated/rps*.hh) | GNUmakefile _config-refpersys.mk
	echo dollar-less-F is $(<F)
	echo basename-dollar-less-F is $(basename $(<F))
	echo pkglist-refpersys is $(PKGLIST_refpersys)
	echo pkglist-$(basename $(<F)) is $(PKGLIST_$(basename $(<F)))
	$(REFPERSYS_CXX) -C -E $(REFPERSYS_PREPRO_FLAGS) \
            $(REFPERSYS_COMPILER_FLAGS) \
	       $(shell pkg-config --cflags $(PKGLIST_refpersys)) \
               $(shell pkg-config --cflags $(PKGLIST_$(basename $(<F)))) \
               -DRPS_THIS_SOURCE=\"$<\" -DRPS_GITID=\"$(RPS_GIT_ID)\"  \
               -DRPS_SHORTGITID=\"$(RPS_SHORTGIT_ID)\" \
            -DRPS_SHORTGIT="$(RPS_SHORTGIT_ID)" \
            -DRPS_HOST=$(RPS_HOST) \
            -DRPS_ARCH=$(RPS_ARCH) \
            -DRPS_OPERSYS=$(RPS_OPERSYS) \
	    $(shell $(REFPERSYS_FLTKCONFIG) -g --cflags) \
	       $< | /bin/sed 's:^#://#:g'| $(ASTYLE) $(ASTYLEFLAGS)  > $@

%.ii.o: %.ii | GNUmakefile  _config-refpersys.mk
	echo dollar-less-F is $(<F)
	echo basename-dollar-less-F is $(basename $(<F))
	echo pkglist-refpersys is $(PKGLIST_refpersys)
	echo pkglist-$(basename $(<F)) is $(PKGLIST_$(basename $(<F)))
	$(REFPERSYS_CXX) -c -std=gnu++17 -g -O $< -o $@

## for plugins, see build-plugin.sh
print-plugin-settings:
	@printf "RPSPLUGIN_CXX='%s'\n" "$(REFPERSYS_CXX)"
	@printf "RPSPLUGIN_CXXFLAGS='%s'\n" "$(REFPERSYS_PREPRO_FLAGS) $(REFPERSYS_COMPILER_FLAGS) $(shell pkg-config --cflags $(PKGLIST_refpersys))"
	@printf "RPSPLUGIN_LDFLAGS='%s'\n"  "-rdynamic -pthread -L /usr/local/lib -L /usr/lib $(LIBES)"

indent:
	$(ASTYLE) $(ASTYLEFLAGS) refpersys.hh
	$(ASTYLE) $(ASTYLEFLAGS) oid_rps.hh
	$(ASTYLE) $(ASTYLEFLAGS) inline_rps.hh
	for f in $(REFPERSYS_HUMAN_CPP_SOURCES) ; do \
	    $(ASTYLE) $(ASTYLEFLAGS) $$f ; done
	for p in $(patsubst %, plugins_dir/%.cc, $(REFPERSYS_DESIRED_PLUGIN_BASENAMES)) ; do \
	    $(ASTYLE) $(ASTYLEFLAGS) $$p ; done

## redump target
redump: refpersys
	./refpersys --dump=. --batch --run-name=$@
	@if git diff -U1|grep '^[+-] ' | grep -v 'origitid|//: gen' ; then \
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

################################################################
#### simple tests
test00: refpersys
	@printf '\n\n\n////test00 first\n'
	./refpersys  -AREPL  --test-repl-lexer 'show help' -B --run-name=$@.1
	@printf '\n\n\n////test00 second\n'
	./refpersys  -AREPL  --test-repl-lexer 'show RefPerSys_system' -B --run-name=$@.2
	@printf '\n\n\n////test00 third\n'
	./refpersys  -AREPL  --test-repl-lexer 'show (1 + 2)' -B --run-name=$@.3
	@printf '\n\n\n////test00 help REPL command\n'
	./refpersys -AREPL -c help -B
	@printf '\n\n\n////test00 FINISHED¤\n'

test01: refpersys
	@echo test01 testing simple show help with a lot of debug
	./refpersys -AREPL -c 'show help' -B --run-name=$@
	@printf '\n\n\n////test01 FINISHED¤\n'

test01b: refpersys
	./refpersys -AREPL,LOW_REPL  -c 'show help' -B --run-name=$@
	@printf '\n\n\n////test01b FINISHED¤\n'

test01c: refpersys
	@printf '\n\n\n//+ test01c !parse_sum 1 + 2\n'
	./refpersys -AREPL,LOW_REPL  -c '!parse_sum 1 + 2' -B --run-name=$@
	@printf '\n\n\n////test01c FINISHED¤\n'

test01d: refpersys
	@printf '\n\n\n//+ test01d !parse_sum 1 + 2 + 3\n'
	./refpersys -AREPL,LOW_REPL  -c '!parse_sum 1 + 2 + 3' -B --run-name=$@
	@printf '\n\n\n////test01d FINISHED¤\n'

test01e: refpersys
	@printf '\n\n\n//+ test01e !parse_sum 1 + 2 * 3\n'
	./refpersys -AREPL,LOW_REPL  -c '!parse_sum 1 + 2 * 3' -B --run-name=$@
	@printf '\n\n\n////test01e FINISHED¤\n'

### notice the space after the 3 below
test01f: refpersys
	./refpersys -AREPL,LOW_REPL  -c '!parse_primary 3 ' -B --run-name=$@
	@printf '\n\n\n////test01f FINISHED¤\n'


test02: refpersys
	./refpersys -AREPL  -c 'show RefPerSys_system' -B --run-name=$@
	@printf '\n\n\n////test02 FINISHED¤\n'

test03: refpersys
	./refpersys -AREPL  -c 'show 1 + 2' -B --run-name=$@
	@printf '\n\n\n////test03 FINISHED¤\n'

test04: refpersys
	./refpersys -AREPL  -c 'show  1 * 2 + 3 * 4' -B --run-name=$@
	@printf '\n\n\n////test04 FINISHED¤\n'

test05: refpersys
	./refpersys -AREPL  -c 'show (1 + 2) ' -B --run-name=$@
	@printf '\n\n\n////test05 FINISHED¤\n'

test06: refpersys
	./refpersys -AREPL  -c 'show 1' -B --run-name=$@
	@printf '\n\n\n////test06 FINISHED¤\n'

test07: refpersys
	./refpersys -AREPL -B -c '!parse_term 1' --run-name=$@.1
	./refpersys -AREPL -B -c '!parse_sum 1 + 2' --run-name=$@.2
	@printf '\n\n\n////test07 FINISHED¤\n'

test07a: refpersys
	./refpersys -AREPL -B -c '!parse_term 1' --run-name=$@
	@printf '\n\n\n////test07a FINISHED¤\n'

test08: refpersys
	@echo missing test08 ; exit 1

test09: refpersys
	@echo missing test09 ; exit 1

test10: refpersys 
	./refpersys -AREPL --run-name=$@ --run-delay=6s
	@printf '\n\n\n////test10 FINISHED¤\n'
test-load: refpersys
	./refpersys --batch
	@printf '\n\n\n////test-load FINISHED¤\n'

## testing the FLTK graphical interface
testfltk1: refpersys
	@printf '%s git %s\n' $@ $(RPS_SHORTGIT_ID)
	./refpersys -AREPL --run-name=$@ --run-delay=6s  --fltk
	@printf '\n\n\n////testfltk1 FINISHED git %s¤\n' $(RPS_SHORTGIT_ID)

testfltk2: refpersys
	@printf '%s git %s\n' $@ $(RPS_SHORTGIT_ID)
	./refpersys -dPROGARG -AREPL --run-delay=14s --fltk -bg ivory --run-name=$@
	@printf '\n\n\n////testfltk2 FINISHED git %s¤\n' $(RPS_SHORTGIT_ID)

testfltk3: refpersys
	@printf '%s git %s\n' $@ $(RPS_SHORTGIT_ID)
	./refpersys -dPROGARG -AREPL --run-name=$@ --run-delay=29s  --fltk -bg lightpink
	@printf '\n\n\n////testfltk3 FINISHED git %s¤\n' $(RPS_SHORTGIT_ID)

testfltk4: refpersys
	@printf '%s git %s\n' $@ $(RPS_SHORTGIT_ID)
	./refpersys -dPROGARG -AREPL --run-name=$@ --run-delay=15m  --fltk -bg peachpuff --echo="hello from $@"
	@printf '\n\n\n////testfltk4 FINISHED git %s¤\n' $(RPS_SHORTGIT_ID)

## eof GNUmakefile


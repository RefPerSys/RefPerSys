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
.PHONY: all config objects clean gitpush gitpush2

SYNC=/bin/sync
FMT=/usr/bin/fmt
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
	$(RM) */*~

-include _scanned-pkgconfig.mk

_scanned-pkgconfig.mk: $(REFPERSYS_HUMAN_CPP_SOURCES) |GNUmakefile do-scan-pkgconfig
	./do-scan-pkgconfig refpersys.hh $(REFPERSYS_HUMAN_CPP_SOURCES) > $@

__timestamp.c: do-generate-timestamp.sh |GNUmakefile
	./do-generate-timestamp.sh > $@

refpersys: 
	@if [ -z "$(REFPERSYS_CXX)" ]; then echo should make config ; exit 1; fi
	@echo RefPerSys human C++ source files $(REFPERSYS_HUMAN_CPP_SOURCES)
#       @echo RefPerSys human C++ object files $(REFPERSYS_HUMAN_CPP_OBJECTS)
	@echo RefPerSys generated C++ files $(REFPERSYS_GENERATED_CPP_SOURCES)
#	@echo RefPerSys generated C++ object files $(REFPERSYS_GENERATED_CPP_OBJECTS)
	@echo PACKAGES_LIST is $(PACKAGES_LIST)
	$(MAKE) $(REFPERSYS_HUMAN_CPP_OBJECTS) $(REFPERSYS_GENERATED_CPP_OBJECTS) __timestamp.o
	$(REFPERSYS_CXX) $(REFPERSYS_HUMAN_CPP_OBJECTS) $(REFPERSYS_GENERATED_CPP_OBJECTS) -rdynamic \
              $(REFPERSYS_NEEDED_LIBRARIES) \
	      $(shell pkg-config --libs $(sort $(PACKAGES_LIST))) -ldl



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
## eof GNUmakefile


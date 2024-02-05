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

#                                                                
.PHONY: all config objects clean gitpush gitpush2

SYNC=/bin/sync
FMT=/usr/bin/fmt
-include config-refpersys.mk

all:

	@/usr/bin/printf "make features: %s\n" "$(.FEATURES)" | $(FMT)
	$(MAKE) do-configure-refpersys
	@/usr/bin/printf "hand-written C++ code: %s\n" "$(REFPERSYS_CPPSOURCES)" | $(FMT)
	@if [ ! -f config-refpersys.mk ]; then \
	   echo missing config-refpersys.mk for GNUmakefile > /dev/stderr; \
	   echo run $(MAKE) config > /dev/stderr ; \
	   exit 1 ; \
	fi
	$(MAKE) refpersys


### Human hand-written C++ sources
REFPERSYS_HUMAN_CPP_SOURCES=$(wildcard *_rps.cc)

### corresponding object files
REFPERSYS_HUMAN_CPP_OBJECTS=$(patsubst %.cc, %.o, $(REFPERSYS_HUMAN_CPP_SOURCES))

### Generated C++ sources
REFPERSYS_GENERATED_CPP_SOURCES := $(wildcard generated/*.cc)

### corresponding object files
REFPERSYS_GENERATED_CPP_OBJECTS=$(patsubst %.cc, %.o, $(REFPERSYS_GENERATED_CPP_SOURCES))

config: do-configure-refpersys GNUmakefile
	./do-configure-refpersys 

do-configure-refpersys: do-configure-refpersys.c |GNUmakefile do-generate-gitid.sh
	$(CC) -Wall -Wextra -DGIT_ID=\"$(shell ./do-generate-gitid.sh -s)\" -O -g $^ -o $@ -lreadline
## if readline library is unavailable add -DWITHOUT_READLINE above


clean:
	$(RM) tmp* *~ *.o do-configure-refpersys refpersys
	$(RM) *% %~
	$(RM) */*~

refpersys: $(REFPERSYS_HUMAN_CPP_OBJECTS) $(REFPERSYS_GENERATED_CPP_OBJECTS)
	@echo RefPerSys human C++ source files $(REFPERSYS_HUMAN_CPP_SOURCES)
	@echo RefPerSys human C++ object files $(REFPERSYS_HUMAN_CPP_OBJECTS)

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
## eof GNUmakefile


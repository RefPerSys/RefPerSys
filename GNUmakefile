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
.PHONY: all config objects clean gitpush

-include config-refpersys.mk

all:

	/usr/bin/printf "make features %s\n" $(.FEATURES)
	/usr/bin/printf "hand-written C++ code %s\n" $(REFPERSYS_CPPSOURCES)
	@if [ ! -f config-refpersys.mk ]; then \
	   echo missing config-refpersys.mk for GNUmakefile > /dev/stderr; \
	   echo run $(MAKE) config > /dev/stderr ; \
	   exit 1 ; \
	fi
	$(MAKE) refpersys


### Human hand-written C++ sources
REFPERSYS_CPPSOURCES := $(wildcard *_rps.cc)

config: do-configure-refpersys GNUmakefile
	./do-configure-refpersys 

do-configure-refpersys: do-configure-refpersys.c |GNUmakefile
	$(CC) -Wall -Wextra -O -g $^ -o $@ -lreadline
## if readline library is unavailable add -DWITHOUT_READLINE above


clean:
	$(RM) tmp* *~ *.o do-configure-refpersys refpersys

## eof GNUmakefile


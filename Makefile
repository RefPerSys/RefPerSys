#
# File: refpersys/Makefile
#
# Description:
# 	This file is part of the Reflective Persistent System. It defines the
# 	build rules.
#
# Author(s):
# 	Basile Starynkevitch <basile@starynkevitch.net>
# 	Niklas Rosencrantz <niklasro@gmail.com>
# 	Abhishek Chakravarti <abhishek@taranjali.org>
#
# Copyright:
# 	(c) 2019 The Reflective Persistent System Team
# 	<https://refpersys.gitlab.io>
#
# License:
# 	Released under the GNU General Public License version 3 (GPLv3)
# 	<http://opensource.org/licenses/GPL-3.0>. See the accompanying LICENSE
# 	file for complete licensing details.
####
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################

REFPERSYS_SOURCES := src/iface.cc src/refpersys.cc src/util.cc src/types.cc
REFPERSYS_OBJECTS := $(patsubst src/%.cc, src/%.o, $(REFPERSYS_SOURCES))




# define commands
CXX  = g++
RM = rm -vf
MV = mv -vf
AS = astyle
CMD_LASTCOMMIT = git log --format=oneline --abbrev=12 \
	         --abbrev-commit -q | head -1




# No coverage, we know that Refpersys is likely to have some code
# which in practice cannot be reached.
OPTIMFLAGS= -g -O1
WARNFLAGS= -Wall -Wextra
INCFLAGS= -I. -Iinc/ -Imps/
CXXFLAGS:= $(OPTIMFLAGS) $(WARNFLAGS) $(INCFLAGS)
CFLAGS:=  $(OPTIMFLAGS) $(WARNFLAGS) $(INCFLAGS)
LDFLAGS:= -pthread
LIBES:=  -lpthread -ldl -lm
ASFLAGS:= --style=gnu -s2
.PHONY: all clean

## the default target
all: refpersys

# rule to build refpersys executable
refpersys: $(REFPERSYS_OBJECTS) mps/mps.o _timestamp.o
	$(LINK.cc) $^ -o $@ $(LIBES)
	$(MV) _timestamp.c _timestamp.c~
	$(RM) _timestamp.o

## automatic makefile dependencies, see
## https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html
-include $(wildcard src/*.mkd)
-include $(wildcard mps/*.mkd)

_timestamp.c:
	echo '// generated temporary' $@ 'file - DONT EDIT' > $@-tmp
	$(CMD_LASTCOMMIT) | awk 'BEGIN {print ""} {print "const char rps_git_commit[] = \"" $$0 "\";"} END {}' >> $@-tmp
	date +'const char rps_build_timestamp[] = "%c %Z";%n' >> $@-tmp
	mv $@-tmp $@

src/%.o: src/%.cc
	$(COMPILE.cc) -o $@ $< -MMD -MT $@ -MF $(patsubst src/%.o, src/%.mkd, $@)

mps/%.o: mps/code/%.c
	$(COMPILE.c)  -o $@ $< -MMD -MT $@ -MF $(patsubst src/%.o, src/%.mkd, $@)

# rule to clean build artefacts
clean:
	$(RM) refpersys *.o */*.o *~ */*~ _*.h _*.c _*.cc
	$(RM) */*.mkd *-tmp
	$(RM) */*.orig

# rule to beautify files to GNU styling standards
indent:
	$(AS) $(ASFLAGS) inc/*.h src/*.cc


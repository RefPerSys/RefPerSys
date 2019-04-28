##***************************************************************
## file Makefile
##
## Description:
##      This file is part of the Reflective Persistent System.
##      It is its Makefile, soon completely obsolete
##      since we really use omake
##
## Author(s):
##      Basile Starynkevitch <basile@starynkevitch.net>
##      Niklas Rosencrantz <niklasro@gmail.com>
##      Abhishek Chakravarti <abhishek@taranjali.org>
##
##      © Copyright 2019 The Reflective Persistent System Team
##      <https://refpersys.gitlab.io>
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
##    along with this program.  If not, see <http://www.gnu.org/licenses/>.
##***************************************************************************

OMAKE= omake
.PHONY: all clean indent archive directclean
# if you don't have omake, use "make refpersys" and "make directclean"

# if you don't have ccache, run without it using make CCACHE=
CCACHE= ccache
CXX= $(CCACHE) g++
RM= rm -vf

################
### keep carefully these variables in sync with those in OMakefile
DIALECTFLAGS = -std=gnu++17
OPTIMFLAGS = -O1 -g
WARNFLAGS = -Wall -Wextra
CXXFLAGS += $(DIALECTFLAGS) $(OPTIMFLAGS) $(WARNFLAGS)

LDFLAGS += -L/usr/local/lib  -rdynamic -lmps -lunistring -lpthread

INCLUDES += /usr/local/include
REFPERSYS_BASE_FILES = 				\
   main_rps					\
   objects_rps					\
   random_rps					\
   garbcoll_rps					\
   values_rps
REFPERSYS_BASE_HEADERS = inc-refpersys
### end of variables to be kept in sync with those in OMakefile
################

all:
	$(OMAKE) all

clean:
	$(OMAKE) clean

indent: # should have an omake target like that doing
	for f in [a-z]*.cc [a-z]*.hh ; do \
	  astyle --style=gnu -s2 $$f ; done

archive: # should have an omake target like that doing
	@gitid=$$(git log --format=oneline --abbrev=12 --abbrev-commit -q  \
     | head -1 | tr -d '\n\r\f\"\\\\' | cut -d' ' -f1); \
	echo gitid is $$gitid; \
	dirpath=$$(date +"refpersys-%Y-%b-%d-$$gitid"); \
	echo dirpath is $$dirpath; \
	outarchive="/tmp/basile-$$dirpath.tar"; \
	rm -vf "$$outarchive"* ; \
	echo outarchive is $$outarchive; \
	git archive --prefix "$$dirpath/" -o "$$outarchive" HEAD; \
	git log > "/tmp/$$dirpath.gitlog" ; \
	tar -f "$$outarchive" -C /tmp --append "$$dirpath.gitlog" ; \
	echo archive done in  "$$outarchive"; \
	bzip2 -v9  "$$outarchive"; echo ; \
	tar tvf  "$$outarchive.bz2" ; echo ; \
	ls -lh   "$$outarchive.bz2"

### to avoid using omake, one could run 'make refpersys'; in the
### future, omake is really preferable because it builds targets based
### on content of files, not on modification time of files. And this
### would become important once refpersys is *generating* C++ code.
refpersys: $(patsubst %,%.o,$(REFPERSYS_BASE_FILES))
	$(LINK.cc) -o $@ $^ $(LDFLAGS)

%_rps.o: %_rps.cc $(patsubst %,%.hh,$(REFPERSYS_BASE_HEADERS))
	$(COMPILE.cc)  -o $@ $<

directclean:
	$(RM) *.o *~ *% *.orig refpersys core

## end of refpersys Makefile

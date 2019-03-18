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




# define directory paths
DIR_BLD  = bld
DIR_BIN  = $(DIR_BLD)/bin
DIR_COV  = $(DIR_BLD)/cov
DIR_INC  = inc
DIR_SRC  = src
DIR_TEST = test




# define commands
CMD_CC  = g++
CMD_SO  = $(CMD_CC)
CMD_LD  = $(CMD_CC)
CMD_LASTCOMMIT = git log --format=oneline --abbrev=12 \
	         --abbrev-commit -q | head -1




# define command options.

# No coverage, we know that Refpersys is likely to have some code
# which in practice cannot be reached.
OPT_CC  = -c -fPIC -Wall -g -O0
OPT_SO  = -shared -g -O2
OPT_LD  = -Wall -g -O2
OPT_COV = -o $(DIR_BLD)


# define inputs
INP_LD = $(DIR_SRC)/refpersys.cc


OUT_GENFILE = $(DIR_INC)/.version.gen.h



# rule to build refparsys executable
all:
	rm -rf $(DIR_BLD)
	mkdir $(DIR_BLD)
	$(CMD_LASTCOMMIT) | awk 'BEGIN {print ""} {print "#define RPS_VERSION_LASTCOMMIT  \"" $$0 "\""} END {}' > $(OUT_GENFILE)
	$(CMD_LD) $(OPT_LD) $(INP_LD) -o $(DIR_BLD)/refpersys




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
#
# 	BY CONTINUING TO USE AND/OR DISTRIBUTE THIS FILE, YOU ACKNOWLEDGE THAT
# 	YOU HAVE UNDERSTOOD THESE LICENSE TERMS AND ACCEPT THEM.
#




	# define directory paths
DIR_BLD  = bld
DIR_BIN  = $(DIR_BLD)/bin
DIR_COV  = $(DIR_BLD)/cov
DIR_SRC  = src
DIR_TEST = test




	# define commands
CMD_CC  = gcc
CMD_SO  = $(CMD_CC)
CMD_LD  = $(CMD_CC)
CMD_COV = gcov
CMD_RUN = ./$(OUT_LD)




	# define command options
OPT_CC  = -c -fPIC -std=c99 -Wall -g -O0 -coverage
OPT_SO  = -shared -g -O1 -coverage
OPT_LD  = -std=c99 -Wall -g -O0 -coverage
OPT_COV = -o $(DIR_BLD)





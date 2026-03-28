# man/Makefile - recursive Makefile for man/ assets
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2019 Basile Starynkevitch <basile@starynkevitch.net>


# Recursive makefile for building man pages
# This will be generated later by a configure script

.PHONY: all

#MAN= obj/Rps::Midend::Cmd.3pm  obj/rpsmidend.1 obj/rpsprovision.8
MAN= obj/rpsmidend.1 obj/rpsprovision.8

all: ${MAN}


#obj/Rps::Midend::Cmd.3pm: Rps::Midend::Cmd.3pm
#	cat $< | awk 'NF' > $@

obj/rpsmidend.1: rpsmidend.1
	cat $< | awk 'NF' > $@

obj/rpsprovision.8: rpsprovision.8
	cat $< | awk 'NF' > $@

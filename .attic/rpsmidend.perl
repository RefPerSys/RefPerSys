#! /usr/bin/env perl
# -*- mode: perl -*-
# bin/rpsmidend - midend class utility
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2023 RefPerSys team - see refpersys.org 


## This script should build refpersys and appropriate plugins, then
## run refpersys to build objects, classes, named attributes,
## ... required to generate C++ code.  Some or most of them are
## RefPerSys root objects....

## Conventionally, immutable instances for middle-end code
## representations of expressions have two slots unused for code
## generation, but usable for metadata...

## Conventionally, objects for middle-end code generations are locked
## and collected by code generation ...

use 5.36.0;

use FindBin;
use lib "$FindBin::Bin/../lib";
use Rps::Midend::Cmd;


#
# Main entry point of script.
#

sub main()
{
    if (Rps::Midend:::Cmd::parse() < 1) {
	goto fail;
    }

    # TODO
    
  fail:
    Rps::Midend::Info::usage();
    exit(1);
}

main();

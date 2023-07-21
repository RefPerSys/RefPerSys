#! /usr/bin/env perl

#
# Import required libraries. We're using Perl 5.36 to ensure a stricter syntax
# and because it's readily available on Linux distros. The Getopt::Long library
# handles CLI argument parsing, and on Ubuntu can be obtained by installing the
# `libgetopt-long-descriptive-perl` package.
#
use 5.36.0;
use Getopt::Long;

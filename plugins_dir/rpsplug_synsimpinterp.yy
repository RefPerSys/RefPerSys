/**
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * File plugins_dir/rpsplug_synsimpinterp.yy for RefPerSys (C++) ; see refpersys.org;
 *
 * To be processed by bisonc++ from https://fbb-git.gitlab.io/bisoncpp/ by Frank B.Brokken
 *
 *      © Copyright 2025 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 * 
 **/
%require "3.7"
%language "c++"

%define api.value.type {Rps_Value}
%define api.symbol.prefix {RPS_SYNPARSYMB_}
%define api.token.prefix {RPS_SYNPARSTOKEN_}
%{
//// prologue from plugins_dir/rpsplug_synsimpinterp.yy
#define _GNU_SOURCE
#pragma GCC message "prologue of rpsplug_synsimpinterp.yy"
#include "refpersys.hh"
%}


%code requires // header file
{
/* generated header from plugins_dir/rpsplug_synsimpinterp.yy */
#pragma GCC message "header of rpsplug_synsimpinterp.yy"
}


%code  // code file
{
/* generated code from plugins_dir/rpsplug_synsimpinterp.yy */
#pragma GCC message "code of rpsplug_synsimpinterp.yy"
}



%token END_OF_INPUT 0;

%token LEFT_BRACE RIGHT_BRACE

%token OBJID

%token NAME

%token INTEGER

/* this is temporary */
%start simple

%%

simple: INTEGER {
};

%%

/* epilogue from  plugins_dir/rpsplug_synsimpinterp.yy */
/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_simpinterp.so && /bin/ln -svf $(/bin/pwd)/plugins_dir/rpsplug_simpinterp.so /tmp/" ;;
 ** End: ;;
 ****************/
/**
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * File iniparser_rps.yy for RefPerSys (C++) ; see refpersys.org;
 *
 * To be processed by GNU bison.
 *
 *      © Copyright 2022 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 * 
 **/
%require "3.7"
%language "c++"

%define api.value.type variant
%define api.symbol.prefix {RPS_INIPARSYMB_}
%define api.token.prefix {RPS_INIPARSTOKEN_}
%{
//// prologue from iniparser_rps.yy
%}


%code requires // header file
{
/* generated header from iniparser_rps.yy */
#include "refpersys.hh"
}


%code  // code file
{
/* generated code from iniparser_rps.yy */
#include "refpersys.hh"
}



%token END_OF_INPUT 0;

%token LEFT_BRACE RIGHT_BRACE

%token OBJID

%token NAME

%token INTEGER

%start simple

%%

simple: INTEGER {
};

%%

/* epilogue from iniparser_rps.yy */
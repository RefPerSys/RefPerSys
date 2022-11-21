/*** GNU bison parser for RefPerSys REPL -*- C++ -*-
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *
 *      It has the GNU bison grammar for the Read Eval Print Loop
 *
 * Author(s):
 *
 * © Copyright 2022 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.2"
%debug
%language "C++"

%define api.token.constructor
%define api.value.type variant
 //%define api.value.automove
%define api.location.file none
%define parse.assert
%define api.prefix {RpsGr_}
%start grammar_top
%locations

%header

%code requires {
  // require part of grammar_rps.yy (GPLv3+ licensed)
  // see refpersys.org
  // goes into generated header file
  #include "refpersys.hh"

  // end of require part of grammar_rps.yy
}

%code {
  // code part of grammar_rps.yy (GPLv3+ licensed)
  // see refpersys.org
  // goes into generated implementation file
  #include "refpersys.hh"

  // end of code part of grammar_rps.yy (GPLv3+ licensed)
}


%%
 /**
  *  grammar part of grammar_rps.yy (GPLv3+ licensed)
  * see refpersys.org
  **/

%nterm <int> grammar_top;

grammar_top:
    %empty {
      /*empty for grammar_top*/
      0;
    }
;

%%

/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "make grammar_rps.cc && make -j5" ;;
 ** End: ;;
 ****************/

/// end of file grammar_rps.yy


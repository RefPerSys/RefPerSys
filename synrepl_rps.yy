/****************************************************************
 * file synrepl_rps.yy
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      It is the grammar of the read-eval-print loop for GNU Bison.
 *      See https://www.gnu.org/software/bison/ for more about Bison.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *      Nimesh Neema <nimeshneema@gmail.com>
 *
 *      © Copyright 2020 The Reflective Persistent System Team
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
 ******************************************************************************/

%require "3.2"
%debug
%language "c++"
%define api.token.constructor
%define api.value.type variant
%define api.value.automove
%define api.location.file none
%define parse.assert
%locations

%code requires // *.hh
{
/// header part generated from synrepl_rps.yy
///      © Copyright 2020 The Reflective Persistent System Team
///     team@refpersys.org & http://refpersys.org/
///

#include "refpersys.hh"
}

%code // *.cc
{
/// code part generated from synrepl_rps.yy
///      © Copyright 2020 The Reflective Persistent System Team
///     team@refpersys.org & http://refpersys.org/
///
}


%%
/*** prologue part of synrepl_rps.yy ***/
%token <int> RPSY_INT;
%token RPSY_EOF 0;

%type <int> rpsy_int_literal;

%%
/*** grammar part of synrepl_rps.yy ***/
rpsy_int_literal: RPSY_INT
                  { /*useless rpsy_int_literal*/ $$ = $1; }
;

%%
/*** action part of synrepl_rps.yy ***/
/******************************************************************
 * file gramrepl_antlr_rps.g4
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description:
 *      This file is part of the Reflective Persistent System.
 *      © Copyright 2023 The Reflective Persistent System Team
 *      team@refpersys.org & http://refpersys.org/
 *
 * You can consider RefPerSys as either GPLv3+ or LGPLv3+ licensed (at
 * your choice)
 *
 * License: GPLv3+ (file COPYING-GPLv3)
 *    This software is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 * Alternative license: LGPLv3+ (file COPYING-LGPLv3)
 *    This software is is free software: you can
 *    redistribute it and/or modify it under the terms of the GNU
 *    Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/


/// the name of the grammar has to be the base name of this file
grammar gramrepl_antlr_rps;

// the start symbol is the REPL command

repl_command : 'show' val_expr
   {
   /* C++ code for show REPL */
#warning missing C++ code for show command (ANTLR)
   }
	     | 'in' obj_expr 'put' obj_expr ':' val_expr
   {
   /* C++ code for ANTLR in .. put */
#warning missing C++ code for in ... put ... ':' ... command (ANTLR)
   }
	     | 'in' obj_expr 'rem' obj_expr
   {
   /* C++ code for ANTLR in .. rem */
#warning missing C++ code for in ... rem ... command (ANTLR)
   }
	     | 'in' obj_expr 'append' val_expr
   {
   /* C++ code for ANTLR in ... append ... command */
#warning missing C++ code for in ... append ... command (ANTLR)
   }
	     ;

val_expr : INT
   {
   /* C++ code for ANTLR integer value expr */
#warning missing C++ code for integer value expr (ANTLR)
   }
   
         /// later | STRING
	 
	 | DOUBLE
   {
   /* C++ code for ANTLR integer value expr */
#warning missing C++ code for double value expr (ANTLR)
   }
	 | obj_expr
   {
   /* C++ code for ANTLR object value expr */
#warning missing C++ code for object value expr (ANTLR)
   }
	 ;

obj_expr: OBJID
   {
   /* C++ code for ANTLR objectexpr by OBJID */
#warning missing C++ code for object expr by OBJID (ANTLR)
   }
   | NAME
   {
   /* C++ code for ANTLR objectexpr by NAME */
#warning missing C++ code for object expr by NAME (ANTLR)
   }
   
;


INT: [0-9]+
   {
   /* C++ code for ANTLR literal INT */
#warning missing C++ code for literal INT (ANTLR)
   }
;

DOUBLE: [0-9]+ '.' [0-9]+
   {
   /* C++ code for ANTLR literal DOUBLE */
#warning missing C++ code for literal DOUBLE (ANTLR)
   }
;



OBJID: '_' [A-Za-z0-9]*
   {
   /* C++ code for ANTLR literal OBJID */
#warning missing C++ code for literal OBJID (ANTLR)
   }
;


NAME: [A-Z][a-z][A-Za-z0-9]*
   {
   /* C++ code for ANTLR literal NAME */
#warning missing C++ code for literal NAME (ANTLR)
   }
;
/* file RefPerSys/gramrepl_rps.yy for GNU bison*/
%require "3.8"
%debug
%language "c++"

%code requires {
// requires part of RefPerSys/gramrepl_rps.yy
#pragma message "require part of RefPerSys/gramrepl_rps.yy"
#include "refpersys.hh"
}

%code {
#pragma message "%code part of RefPerSys/gramrepl_rps.yy"
}

%define api.value.type {Rps_Value}

%token <Rps_Id> OID;
%token <intptr_t> INT;
%token <double> DOUBLE;
%token <std::string> STRING;
%type <Rps_Value> repl_atom;

%parse-param {Rps_TokenSource*tksrc, Rps_CallFrame*callframe}

%start repl_atom

%%

repl_atom: INT {
#pragma message "INT rule for repl_atom"
  $$ = Rps_Value::make_tagged_int($1);
};

repl_atom: DOUBLE {
#pragma message "DOUBLE rule for repl_atom"
  $$ = Rps_DoubleValue($1);
};

repl_atom: STRING {
#pragma message "STRING rule for repl_atom"
  $$ = Rps_StringValue($1);
};

repl_atom: OID {
#pragma message "OID rule for repl_atom"
  $$ = Rps_ObjectValue(Rps_ObjectRef(find_object_by_oid($1)));
};

%%
#pragma message "trailer of gramrepl_rps.yy"
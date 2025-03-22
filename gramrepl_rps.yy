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

%token <Rps_Id> RPSTOK_OID;
%token <intptr_t> RPSTOK_INT;
%token <double> RPSTOK_DOUBLE;
%token <std::string> RPSTOK_STRING;
%type <Rps_Value> repl_atom;

//%parse-param {Rps_TokenSource*tksrc, Rps_CallFrame*callframe}

%start repl_atom

%%

repl_atom: RPSTOK_INT {
#pragma message "RPSTOK_INT rule for repl_atom"
  $$ = Rps_Value::make_tagged_int($1);
};

repl_atom: RPSTOK_DOUBLE {
#pragma message "RPSTOK_DOUBLE rule for repl_atom"
  $$ = Rps_DoubleValue($1);
};

repl_atom: RPSTOK_STRING {
#pragma message "RPSTOK_STRING rule for repl_atom"
  $$ = Rps_StringValue($1);
};

repl_atom: RPSTOK_OID {
#pragma message "RPSTOK_OID rule for repl_atom"
  $$ = Rps_ObjectValue(Rps_ObjectRef::find_object_by_oid($1));
};

%%
#pragma message "trailer of gramrepl_rps.yy"
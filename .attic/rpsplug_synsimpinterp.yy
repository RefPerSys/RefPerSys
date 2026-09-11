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
%filenames _rpsplug_synsimpterp_
%baseclass-preinclude "refpersys.hh"
%debug
%default-actions warn
%class-name RpsSyn_Parser
%error-verbose
%print-tokens
%required-tokens 2
%thread-safe
%parsefun-source _rpsplug_synsimpinterp_parser_.cc


%token END_OF_INPUT;

%token LEFT_BRACE RIGHT_BRACE

%token OBJID

%token NAME

%token INTEGER

/* this is temporary */
%start simple

%%

simple: INTEGER {
};



/* epilogue from  plugins_dir/rpsplug_synsimpinterp.yy */
/****************
 **                           for Emacs...
 ** Local Variables: ;;
 ** compile-command: "cd ..; make plugins_dir/rpsplug_simpinterp.so && /bin/ln -svf $(/bin/pwd)/plugins_dir/rpsplug_simpinterp.so /tmp/" ;;
 ** End: ;;
 ****************/
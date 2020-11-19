# RefPerSys primordial Read Eval Print Loop

## lexical conventions

See C++ lexer function `rps_repl_lexer` in file `repl_rps.cc`. That
lexer function returns a pair of values: first is the lexical kind,
second is the semantic value of the lexed token.

It is not possible to enter a fresh objid. Only *existing* objids can
be typed.

## Features considered

1. Dump persistent heap
2. Generate relevant code files from persistent heap
3. Create new transient/persistent objects
4. View properties of existing objects
5. Mark objects as persistent or transient (or vice versa)
6. Create objects representing code plugins
7. Run plugin objects


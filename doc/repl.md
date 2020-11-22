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


## Syntax considerations

As suggested in the e-mail exchange, we would be considering all REPL commands
to be starting with a verb, followed by a subject, and then parameters. The verb
 and subject are mandatorily required, whereas the parameters are optional.

Since we may have commands running across several line, and since we are **not**
considering whitespace (including newline characters) to be significant, we need
a way to indicate the termination of a command. In our initial discussion it was
suggested that we use two consecutive semicolons; however, some discussion is
required on how the REPL distinguishes `;;` in infinite for loops in code chunks
and command termination.

The general syntax for REPL commands would thus be:
```
<verb> <subject> <flags>

```


## Extensible syntax

The *`<verb>`* above would be an object, often a RefPerSys symbol,
having as some specific attribute `repl_command` whose associated
value is a RefPerSys closure parsing the rest of the command.

## Concrete examples

1. Creating a persistent object of class "symbol" with name "comment" can be
   achieved in two ways, namely by using either the object ID or name of the
   class "symbol".

  * `create-object name=comment class=symbol`
  * `create-object name=comment class=_36I1BY2NetN03WjrOv`


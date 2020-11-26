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


## List of RefPerSys functions acception a call frame

  * Rps_ObjectRef::Rps_ObjectRef(
	Rps_CallFrame*callerframe, 
	const char*oidstr, 
	Rps_ObjIdStrTag);

  * static Rps_ObjectRef Rps_ObjectRef::find_object_by_string(
	Rps_CallFrame*callerframe,  
	const std::string& str, 
	bool dontfail=false);

  * static Rps_ObjectRef Rps_ObjectRef::find_object_by_oid(
	Rps_CallFrame*callerframe, 
	Rps_Id oid, 
	bool dontfail=false);

  * static Rps_ObjectRef Rps_ObjectRef::make_named_class(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef superclassob, 
	std::string name);

  * static Rps_ObjectRef Rps_ObjectRef::make_new_symbol(
	Rps_CallFrame*callerframe, 
	std::string name, 
	bool isweak);

  * static Rps_ObjectRef Rps_ObjectRef::make_new_strong_symbol(
	Rps_CallFrame*callerframe, 
	std::string name);

  * static Rps_ObjectRef Rps_ObjectRef::make_new_weak_symbol(
	Rps_CallFrame*callerframe, 
	std::string name);

  * static Rps_ObjectRef Rps_ObjectRef::make_object(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef classob, 
	Rps_ObjectRef spaceob=nullptr);

  * static Rps_ObjectRef Rps_ObjectRef::make_mutable_set_object(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef spaceob=nullptr);

  * void Rps_ObjectRef::install_own_method(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef obsel, 
	Rps_Value closv);

  * void Rps_ObjectRef::install_own_2_methods(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef obsel0, 
	Rps_Value closv0, 
	Rps_ObjectRef obsel1, 
	Rps_Value closv1);

  * void Rps_ObjectRef::install_own_3_methods(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef obsel0, 
	Rps_Value closv0, 
	Rps_ObjectRef obsel1, 
	Rps_Value closv1, 
	Rps_ObjectRef obsel2, 
	Rps_Value closv2);


  * Rps_ObjectFromOidRef::Rps_ObjectFromOidRef(
	Rps_CallFrame*callerframe, 
	const char*oidstr);

  * inline Rps_Value::Rps_Value(
	const void*ptr, 
	Rps_CallFrame*cframe);

  * Rps_ClosureValue Rps_Value::closure_for_method_selector(
	Rps_CallFrame*cframe, 
	Rps_ObjectRef obselector) const;

  * Rps_ObjectRef Rps_Value::compute_class(
	Rps_CallFrame*) const;

  * Rps_Value Rps_Value::get_attr(
	Rps_CallFrame*stkf, 
	const Rps_ObjectRef obattr) const;

  * inline bool Rps_Value::is_instance_of(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef obclass) const;

  * inline bool Rps_Value::is_subclass_of(
	Rps_CallFrame*callerframe, 
	Rps_ObjectRef obsuperclass) const;

  * Rps_TwoValues Rps_Value::send0(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel) const;

  * Rps_TwoValues Rps_Value::send1(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        Rps_Value arg0) const;

  * Rps_TwoValues Rps_Value::send2(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        Rps_Value arg0, 
	const Rps_Value arg1) const;

  * Rps_TwoValues Rps_Value::send3(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1, 
	const Rps_Value arg2) const;

  * Rps_TwoValues Rps_Value::send4(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1,
        const Rps_Value arg2, 
	const Rps_Value arg3) const;

  * Rps_TwoValues Rps_Value::send5(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1,
        const Rps_Value arg2, 
	const Rps_Value arg3,
        const Rps_Value arg4) const;

  * Rps_TwoValues Rps_Value::send6(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1,
        const Rps_Value arg2, 
	const Rps_Value arg3,
        const Rps_Value arg4, 
	const Rps_Value arg5) const;

  * Rps_TwoValues Rps_Value::send7(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1,
        const Rps_Value arg2, 
	const Rps_Value arg3,
        const Rps_Value arg4, 
	const Rps_Value arg5,
        const Rps_Value arg6) const;

  * Rps_TwoValues Rps_Value::send8(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1,
        const Rps_Value arg2, 
	const Rps_Value arg3,
        const Rps_Value arg4, 
	const Rps_Value arg5,
        const Rps_Value arg6, 
	const Rps_Value arg7) const;

  * Rps_TwoValues Rps_Value::send9(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const Rps_Value arg0, 
	const Rps_Value arg1,
        const Rps_Value arg2, 
	const Rps_Value arg3,
        const Rps_Value arg4, 
	const Rps_Value arg5,
        const Rps_Value arg6, 
	const Rps_Value arg7,
        const Rps_Value arg8) const;

  * Rps_TwoValues Rps_Value::send_vect(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const std::vector<Rps_Value>& argvec) const;

  * Rps_TwoValues Rps_Value::send_ilist(
	Rps_CallFrame*cframe, 
	const Rps_ObjectRef obsel,
        const std::initializer_list<Rps_Value>& argil) const;


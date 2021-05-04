# RefPerSys primordial Read Eval Print Loop

## lexical conventions

See C++ lexer function `rps_repl_lexer` in file `repl_rps.cc`. That
lexer function returns a pair of values: first is the lexical kind,
second is the semantic value of the lexed token.

It is not possible to enter a fresh objid. Only *existing* objids can
be typed.

Lexing of delimiters is done thru the `_627ngdqrVfF020ugC5` object (a
`string_dictionnary` named `repl_delim` and mapping delimiters to
values describing them).

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

Since we may have commands running across several lines, and since we are **not**
considering whitespace (including newline characters) to be significant, we need
a way to indicate the termination of a command. In our initial discussion it was
suggested that we use two consecutive semicolons; however, some discussion is
required on how the REPL distinguishes `;;` in infinite for loops in code chunks
and command termination.

The general syntax for REPL commands would thus be:
```
<verb> <subject> <flags>

```

The REPL syntax should be designed to be easy to parse. It probably
should be some [LL(1)](https://en.wikipedia.org/wiki/LL_parser)
syntax.

## Extensible syntax

The *`<verb>`* above would be an object, often a RefPerSys symbol,
having as some specific attribute `repl_command_parser` whose associated
value is a RefPerSys closure parsing the rest of the command.

EDIT:

Although we had initially considered using verbs and subjects in the REPL, on
discussion it was decided that a better approach would be to consider a syntax
similar to Python functions. With such a syntax, every command would be a
function, which in turn would be associated with a RefPerSys closure.


## Expressions

**INCOMPLETE**

We need expressions evaluated to values, and other expressions
evaluated to objects. Since objects and values are different, we have
two kind of expressions: those giving a value, and those evaluated to
an object.

### Value expressions

Scalars like integers `-10` or `0x1f` or `2300` are values.
Scalars like doubles `3.2` or `-2.0e1` are boxed double values.
Scalars like strings `"abc"` or `"twø\nlines"` are strings.


### Object expressions
* Addition of two or more objects: 
  - Addition is denoted by the operator `+`
  - `o1 + o2 + o3 + ... <on>` where `o1`, `o2`, `o3` are object operands
  - Whitespace is not significant between the operator and operand
  - Associative property holds, i.e. `o1 + (o2 + o3) == (o1 + o2) + o3`

* Subtraction of two or more objects: 
  - Subtraction is denoted by the operator `-`
  - `o1 - o2 - o3 + ... <on>` where `o1`, `o2`, `o3` are object operands
  - Whitespace is not significant between the operator and operand
  - Associative property does not hold, i.e. `o1 - (o2 - o3) != (o1 - o2) - o3`

* Multiplication of two or more objects:
  - Multiplication is denoted by the operator `*`
  - `o1 * o2 * o3 * ... <on>` where `o1`, `o2`, `o3` are objects
  - Whitespace is not significant between the operator and operand
  - Associative property holds, i.e. `o1 * (o2 * o3) == (o1 * o2) * o3`

* Division of two or more objects: div <obj1> <obj2> ... <objn>
  - Division is denoted by the operator `/`
  - `o1 / o2 / o3 / ... <on>` where `o1`, `o2`, `o3` are object operands
  - Whitespace is not significant between the operator and operand
  - Associative property does not hold, i.e. `o1 / (o2 / o3) != (o1 / o2) / o3`

* Modulo operator (only two objects):
  - Computing the modulo is denoted by the operator `%`
  - `o1 % o2 % o3 % ... <on>` where `o1`, `o2`, `o3` are object operands
  - Whitespace is not significant between the operator and operand
  - Associative property does not hold, i.e. `o1 % (o2 % o3) != (o1 % o2) % o3`

### Logical expressions evaluation to true or false
* We need objects representing TRUE and FALSE, or we could use boxed values 1
  and 0
* Less than: lt <obj1> <obj2>
* less than equal: lte <obj1> <obj2>
* equal: eq <obj1> <obj2>
* greater than: gt <obj1> <obj2>
* greater than equal: gte <obj1> <obj2>

#### Combining object expressions
* add <obj1> (mul <obj2> <obj3> (sub <obj4> <obj5>)), equivalent to
  obj1 + (obj2 * obj3 * (obj4 - obj5))
* The normal rules of precedence apply when evaluating the order of the terms in
  an expression.
* Hence, the order of evaluation would be, in terms of precedence, brackets,
  division, multiplication, addition, subtraction.

## Representation of sets and tuples
* Sets: {o1, o2, o3 ...} where o1, o2 and o3 are discrete objects
* Tuples: [o1, o2, o3 ...} where o1, o2 and o3 may or may not be discrete
  objects

## Syntactic conventions for EBNF

In the syntax below, braces are for repetition, and quotes are for
literals (or keywords, lexed as objects). Brackets denote optional
syntax. The vertical bar means "or" metasyntax. and ∅ denotes the
empty sequence.  See also
[EBNF](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form)
wikipedia page.

## EBNF


expression = disjunction { `or` disjunction }

disjunction = conjunction { `and` conjunction }

conjunction = comparand [ compare-operator comparand ]

compare-operator = `<` | `>` | `<=` | `>=` | `=` | `==` | `<` | `>`

comparand = factor { multiplicative-operator factor }

multiplicative-operator = `*` | `/` | `%`

additive-operator = `+` | `-`

factor = primary { primary-complement }
term = factor [ additive-operator term ]

primary = object-ref | string-literal | float-literal | `(` expression `)` | set-expr | tuple-expr

object-ref = oid | named-object

primary-complement = field-selection | message-send

field-selection = . object-ref | `.` `(` expression `)`

message-send = `=>` object-ref `(` arglist `)`

arglist = ∅ | expression { `,` expression }

set-expr = `{` arglist `}`

tuple-expr = `[` arglist `]`

## Concrete examples of commands

0. Help for possible commands

  * `help` should list the possible commands

1. Dump the persistent heap:

  * `dump !` will dump to override the loaded heap
  * `dump .` will dump to the current working directory
  * `dump "/tmp/dump"` will dump to the newly made directory `/tmp/dump`

2. Show a given object or value

  * `show` *expression*
  * `show _0jdbikGJFq100dgX1n` will show the details of object with OID `_0jdbikGJFq100dgX1n`.
  * `show comment` will show the details of the object with the symbol name "comment".
  * `show "foo"` is showing the string value `"foo"`.
  * `show 123` is showing the tagged integer 123.
  * `show 1+2` could evaluate that sum and display 3.
  * `?X.class == _41OFI3r0S1t03qdB2E => show ?X` will list all objects belonging to the class `_41OFI3r0S1t03qdB2E`.
    We are keeping this form for a future date (perhaps 2022).

3. Put an attribute inside some existing object

  * `put` *object-expression* `:` *attribute-expression* `=` *value-expression*

4. Remove an attribute from some existing object

  * `remove` *object-expression* `:` *attribute-expression*

5. Append a component into some existing object

  * `append` *object-expression* `:` *value-expression*

6. Add a new persistent root object

  * `add_root` *object-expression*

7. Remove an existing persistent root object

  * `remove_root` *object-expression*

8. Make a new strong symbol

  * `make_symbol` *name*

9. Generate C++ code described by some existing object

  * `generate_code` *code-object*
  
1. **Probably wrong** Creating a persistent object of class "symbol" with name "comment" can be
   achieved in two ways, namely by using either the object ID or name of the
   class "symbol".

  * `create_object name=comment class=symbol`
  * `create_object name=comment class=_36I1BY2NetN03WjrOv`

EDIT:

With the new syntax in consideration, we would have instead:
  * `create_object(name=comment, class=symbol)`
  * `create_object(name=comment, class=_36I1BY2NetN03Wjr0v)`

As before, we can refer to objects either by their object ID or by their name.

### Examples required

  * Creating a new object named `sample_object` of class `symbol` with a
    comment:
    - object_new(name=sample_object, class=symbol, comment="Sample object");

  * Creating the same object as above, except that we now refer to the class by
    its object ID:
    - object_new(name=sample_object, class=_36I1BY2NetN03Wjr0v, comment="Sample
      object");
  
  * Querying the newly created `sample_object` for its object ID:
    - sample_object.oid();

  * Adding a key-value attribute pair of objects to the newly created
    `sample_object`:
    - sample_object.add(attribute=[sample_key_object, sample_key_value]);

  * Adding a list of key-value attribute pairs of objects to the newly created
    `sample_object`:
    - sample_object.add(attribute={[sample_key_object1, sample_key_value1],
	sample_key_object2, sample_key_value2]});

  * Removing attribute with key sample_key_object from `sample_object`:
    - sample_object.remove(attribute=sample_key_object);

  * Add a component to an object
  * Remove a component from an object
  * Send a message to an object
  * Apply a closure to an object
  * Create an object with a payload
  * Create a new class
  * Create a code chunk object

  


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

  * inline void Rps_GarbageCollector::mark_call_stack(
	Rps_CallFrame*topframe);

  * virtual Rps_ObjectRef Rps_ZoneValue::compute_class(
	Rps_CallFrame*stkf) const =0;

  * virtual Rps_ObjectRef Rps_String::compute_class(
	Rps_CallFrame*stkf) const;

  * virtual Rps_ObjectRef Rps_Double::compute_class(
	Rps_CallFrame*stkf) const;

  * virtual Rps_ObjectRef Rps_ObjectZone::compute_class(
	Rps_CallFrame*stkf) const;
  
  * unsigned Rps_ObjectZone::nb_attributes(
	Rps_CallFrame*stkf) const;

  * Rps_Value Rps_ObjectZone::get_attr1(
	Rps_CallFrame*stkf,
	const Rps_ObjectRef obattr0) const;

  * Rps_TwoValues Rps_ObjectZone::get_attr2(
	Rps_CallFrame*stkf,
	const Rps_ObjectRef obattr0, 
	const Rps_ObjectRef obattr1) const;

  * unsigned Rps_ObjectZone::nb_components(
	Rps_CallFrame*stkf) const;

  * Rps_Value Rps_ObjectZone::component_at(
	Rps_CallFrame*stkf, 
	int rk, 
	bool dontfail=false) const;

  * Rps_Value Rps_ObjectZone::instance_from_components(
	Rps_CallFrame*stkf, 
	Rps_ObjectRef obinstclass) const;
  
  * virtual Rps_ObjectRef Rps_SetOb::compute_class(
	Rps_CallFrame*stkf) const;

  * virtual Rps_ObjectRef Rps_TupleOb::compute_class(
	Rps_CallFrame*stkf) const;

  * virtual Rps_ObjectRef Rps_ClosureZone::compute_class(
	Rps_CallFrame*stkf) const;
  
  * inline Rps_TwoValues Rps_ClosureZone::apply0(
	Rps_CallFrame*callerframe) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply1(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply2(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply3(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply4(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply5(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3, 
	const Rps_Value arg4) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply6(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3, 
	const Rps_Value arg4,
        const Rps_Value arg5) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply7(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3, 
	const Rps_Value arg4,
        const Rps_Value arg5, 
	const Rps_Value arg6) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply8(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3, 
	const Rps_Value arg4,
        const Rps_Value arg5, 
	const Rps_Value arg6,
        const Rps_Value arg7) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply9(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3, 
	const Rps_Value arg4,
        const Rps_Value arg5, 
	const Rps_Value arg6,
        const Rps_Value arg7, 
	const Rps_Value arg8) const;

  * inline Rps_TwoValues Rps_ClosureZone::apply10(
	Rps_CallFrame*callerframe, 
	const Rps_Value arg0,
        const Rps_Value arg1, 
	const Rps_Value arg2,
        const Rps_Value arg3, 
	const Rps_Value arg4,
        const Rps_Value arg5, 
	const Rps_Value arg6,
        const Rps_Value arg7, 
	const Rps_Value arg8, 
	const Rps_Value arg9) const;

  * Rps_TwoValues Rps_ClosureZone::apply_vect(
	Rps_CallFrame*callerframe, 
	const std::vector<Rps_Value>& argvec) const;

  * Rps_TwoValues Rps_ClosureZone::apply_ilist(
	Rps_CallFrame*callerframe, 
	const std::initializer_list<Rps_Value>& argil) const;





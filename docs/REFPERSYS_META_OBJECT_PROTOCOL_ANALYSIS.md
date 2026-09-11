# RefPerSys Meta-Object Protocol Analysis

## Executive Summary

This document provides comprehensive analysis of RefPerSys's meta-object protocol, the system's most sophisticated and unique capability. The meta-object protocol enables objects to introspect and modify themselves, create dynamic class hierarchies, and integrate with code generation systems. This protocol forms the foundation of RefPerSys's reflective and self-modifying nature.

## Table of Contents

1. [Meta-Object System Architecture](#meta-object-system-architecture)
2. [Core Data Structures](#core-data-structures)
3. [Bidirectional Object-Meta-Object Relationships](#bidirectional-object-meta-object-relationships)
4. [Reflective Capabilities Catalog](#reflective-capabilities-catalog)
5. [Inheritance and Class Relationships](#inheritance-and-class-relationships)
6. [Method Dispatch and Dynamic Invocation](#method-dispatch-and-dynamic-invocation)
7. [Property Access and Modification](#property-access-and-modification)
8. [Metadata Management System](#metadata-management-system)
9. [Implementation Analysis](#implementation-analysis)
10. [Integration with Other Subsystems](#integration-with-other-subsystems)

---

## Meta-Object System Architecture

### Overview

RefPerSys implements a sophisticated meta-object protocol where every object can serve as both a regular object and a meta-object for other objects. This creates a fully reflective system where objects can:

- **Introspect** their own structure, class, and relationships
- **Modify** their class hierarchy and method definitions at runtime
- **Trigger** code generation based on structural changes
- **Persist** their reflective state across sessions
- **Integrate** with plugins for extended meta-object behaviors

### Fundamental Principles

1. **Every Object is Potentially a Meta-Object**: Objects can hold metadata about other objects through the meta-object protocol
2. **Dynamic Class Hierarchy**: Classes can be created, modified, and extended at runtime
3. **Method as First-Class Objects**: Methods are closures that can be manipulated like any other object
4. **Persistent Reflection**: Meta-object state is preserved through serialization
5. **Plugin Extensibility**: Meta-object behavior can be extended through the plugin system

---

## Core Data Structures

### Rps_PayloadClassInfo: The Meta-Object Foundation

The `Rps_PayloadClassInfo` class in [`refpersys.hh:4743`](refpersys.hh:4743) is the central data structure for meta-object functionality:

```cpp
class Rps_PayloadClassInfo : public Rps_Payload {
    // the superclass
    Rps_ObjectRef pclass_super;
    
    // the dictionary from selector to own methods
    std::map<Rps_ObjectRef,Rps_ClosureValue> pclass_methdict;
    
    // the optional name (a symbol)
    Rps_ObjectRef pclass_symbname;
    
    // cached attribute set
    mutable std::atomic<const Rps_SetOb*> pclass_attrset;
};
```

#### Key Components

1. **`pclass_super`**: References the superclass, enabling inheritance hierarchies
2. **`pclass_methdict`**: Maps method selectors to closure implementations
3. **`pclass_symbname`**: Optional symbolic name for the class
4. **`pclass_attrset`**: Cached attribute set for performance optimization

### Object Zone Meta-Data

Every object in the system has access to meta-object capabilities through the `Rps_ObjectZone` class in [`refpersys.hh:3402`](refpersys.hh:3402):

```cpp
class Rps_ObjectZone : public Rps_ZoneValue {
    // Meta-object management
    std::atomic<Rps_ObjectZone*> _treemetaob;     // The meta-object
    std::atomic<int32_t> _treemetarank;           // Meta-object rank
    std::atomic<bool> _treemetatransient;         // Transient flag
    
    // Core operations
    Rps_ObjectZone* metaobject(void) const;
    int32_t metarank(void) const;
    std::pair<Rps_ObjectZone*,int32_t> const get_metadata(void) const;
};
```

---

## Bidirectional Object-Meta-Object Relationships

### The Meta-Object Hierarchy

The relationship between objects and their meta-objects creates a sophisticated hierarchy:

```
Object A (user data)
  └── metaobject() → Object B (class definition)
       └── metaobject() → Object C (metaclass)
            └── metaobject() → Object D (meta-metaclass)
                 └── ...
```

### Implementation Details

The meta-object relationship is managed through atomic operations for thread safety in [`refpersys.hh:4139`](refpersys.hh:4139):

```cpp
Rps_ObjectZone* metaobject(void) const {
    return _treemetaob.load();
}

int32_t metarank(void) const {
    return _treemetarank.load();
}
```

### Metadata Storage Operations

The system provides sophisticated metadata management in [`inline_rps.hh:1953`](inline_rps.hh:1953):

```cpp
void put_metadata(Rps_ObjectRef obr, int32_t num=0, bool transient=false) {
    _treemetaob.store(obr);
    _treemetarank.store(num);
    _treemetatransient.store(transient);
}

std::pair<Rps_ObjectZone*,int32_t> swap_metadata(Rps_ObjectRef obr, int32_t num=0, bool transient=false) {
    auto oldobz = _treemetaob.load();
    auto oldnum = _treemetarank.load();
    put_metadata(obr, num, transient);
    return std::pair<Rps_ObjectZone*,int32_t> {oldobz, oldnum};
}
```

---

## Reflective Capabilities Catalog

### Type Introspection

#### Class Discovery
Objects can discover their class and test inheritance relationships:

```cpp
// Check if object is instance of a class
inline bool is_instance_of(Rps_ObjectRef obclass) const;

// Test subclass relationships
inline bool is_subclass_of(Rps_ObjectRef obsuperclass) const;
```

#### Implementation Analysis
The `is_subclass_of` implementation in [`inline_rps.hh:1691`](inline_rps.hh:1691) provides sophisticated inheritance testing:

```cpp
bool Rps_ObjectZone::is_subclass_of(Rps_ObjectRef obsuperclass) const {
    // Protection against infinite loops
    for (int cnt = 0; cnt < maximal_inheritance_depth; cnt++) {
        // Get current class and check against target
        Rps_ObjectRef obcurclass = this;
        Rps_ObjectRef obthisclass = get_class();
        
        while (obcurclass) {
            auto curclasspayl = obcurclass->get_dynamic_payload<Rps_PayloadClassInfo>();
            if (curclasspayl) {
                if (obcurclass == obsuperclass) return true;
                obcurclass = curclasspayl->superclass();
            }
        }
    }
    return false;
}
```

### Method Discovery and Invocation

#### Method Dictionary Access
Classes maintain method dictionaries that support both lookup and modification:

```cpp
// Find method in class hierarchy
Rps_ClosureValue find_own_method(Rps_ObjectRef obsel) const {
    auto it = pclass_methdict.find(obsel);
    if (it != pclass_methdict.end())
        return it->second;
    return Rps_ClosureValue(nullptr);
}

// Add method to class
void put_own_method(Rps_ObjectRef obsel, Rps_ClosureValue clov);

// Remove method from class
void remove_own_method(Rps_ObjectRef obsel);
```

#### Method Dispatch Patterns
The system supports sophisticated method dispatch through the `send0` operation in [`refpersys.hh:1572`](refpersys.hh:1572):

```cpp
Rps_TwoValues send0(Rps_CallFrame*cframe, const Rps_ObjectRef obsel) const;
```

This enables dynamic method invocation where the method selector can be determined at runtime.

---

## Inheritance and Class Relationships

### Dynamic Class Creation

Classes can be created dynamically using the `make_named_class` method in [`objects_rps.cc:2478`](objects_rps.cc:2478):

```cpp
Rps_ObjectRef Rps_ObjectRef::make_named_class(Rps_CallFrame*callerframe, 
                                            Rps_ObjectRef superclassarg, 
                                            std::string name) {
    // Validate superclass
    if (!superclassarg || !superclassarg->is_class())
        throw std::runtime_error("make_named_class with invalid superclass");
    
    // Create new class object
    Rps_ObjectZone* obclass = Rps_ObjectZone::make();
    auto paylclainf = obclass->put_new_plain_payload<Rps_PayloadClassInfo>();
    paylclainf->put_superclass(superclassarg);
    
    return Rps_ObjectRef(obclass);
}
```

### Superclass Management

The system supports dynamic superclass manipulation:

```cpp
// Get superclass
Rps_ObjectRef superclass() const {
    return pclass_super;
}

// Set superclass
void put_superclass(Rps_ObjectRef obr) {
    pclass_super = obr;
}
```

### Inheritance Depth Management

To prevent infinite recursion in inheritance chains, the system implements depth limiting in [`inline_rps.hh:1722`](inline_rps.hh:1722):

```cpp
// Protection against infinite inheritance loops
if (RPS_UNLIKELY(cnt > (int)Rps_Value::maximal_inheritance_depth)) {
    RPS_WARNOUT("too deep (" << cnt << ") inheritance for " << Rps_ObjectRef(this)
                << " of class " << obinitclass 
                << " in Rps_ObjectZone::is_subclass_of call#" << curcallcnt);
    throw RPS_RUNTIME_ERROR_OUT("too deep inheritance");
}
```

---

## Method Dispatch and Dynamic Invocation

### Method Lookup Algorithm

The method lookup follows a sophisticated algorithm:

1. **Own Methods**: Check the class's own method dictionary
2. **Superclass Chain**: Recursively check superclass method dictionaries
3. **Method Resolution**: Return first matching method or nullptr

### Dynamic Method Installation

Plugins can dynamically add methods to classes as demonstrated in [`plugins_dir/rpsplug_createclass.cc:94`](plugins_dir/rpsplug_createclass.cc:94):

```cpp
// Create new class
_f.obnewclass = Rps_ObjectRef::make_named_class(&_, _f.obsuperclass, 
                                                std::string{plugarg});

// Add methods dynamically
_f.obnewclass->put_own_method(selector, closure);
```

### Closure-Based Methods

Methods are implemented as closures (`Rps_ClosureValue`), enabling:

- **First-class status**: Methods can be passed as arguments
- **Dynamic creation**: Methods can be created and modified at runtime
- **Closure state**: Methods can maintain internal state
- **Lazy evaluation**: Method bodies can be evaluated on demand

---

## Property Access and Modification

### Attribute Management

Classes maintain attribute sets for property management:

```cpp
// Get class attribute set
const Rps_SetOb* class_attrset(void) const {
    return pclass_attrset.load();
}

// Set class attribute set
void put_attrset(const Rps_SetOb* setob) {
    pclass_attrset.store(setob);
}
```

### Property Access Patterns

The system supports dynamic property access through:

1. **Attribute Sets**: Collections of property definitions
2. **Getter/Setter Methods**: Dynamic property access
3. **Property Inheritance**: Properties inherited through class hierarchy

### Implementation in Class Creation

Class creation includes attribute set management as seen in [`objects_rps.cc:2526`](objects_rps.cc:2526):

```cpp
auto paylclainf = _f.obclass->put_new_plain_payload<Rps_PayloadClassInfo>();
paylclainf->put_superclass(_f.obsuperclass);
paylclainf->put_symbname(_f.obsymbol);
```

---

## Metadata Management System

### Transient vs Persistent Metadata

The meta-object system distinguishes between transient and persistent metadata:

- **Transient Metadata**: Temporary meta-object information not saved to disk
- **Persistent Metadata**: Meta-object information saved in object serialization

### Meta-Rank System

The `metarank()` value enables:

1. **Meta-Object Hierarchy**: Multiple levels of meta-object nesting
2. **Performance Optimization**: Quick access to meta-object information
3. **Cache Management**: Efficient meta-object lookup and caching

### Atomic Operations for Thread Safety

All meta-object operations use atomic operations for thread safety:

```cpp
std::atomic<Rps_ObjectZone*> _treemetaob;
std::atomic<int32_t> _treemetarank;
std::atomic<bool> _treemetatransient;
```

This ensures that meta-object modifications are atomic and visible across threads.

---

## Implementation Analysis

### Memory Management

The meta-object system integrates with RefPerSys's garbage collection:

```cpp
virtual void gc_mark(Rps_GarbageCollector&gc, unsigned depth=0) const {
    // Mark meta-object
    if (!is_metatransient())
        gc.mark_obj(metaobject());
    
    // Mark class information
    auto paylclinfo = get_dynamic_payload<Rps_PayloadClassInfo>();
    if (paylclinfo) {
        gc.mark_obj(paylclinfo->superclass());
        gc.mark_obj(paylclinfo->class_symbol());
    }
}
```

### Persistence Integration

Meta-object state is preserved through serialization in [`dump_rps.cc:772`](dump_rps.cc:772):

```cpp
auto md = get_metadata();
Rps_ObjectRef metaobr = md.first;
if (!metaobr.is_empty()) {
    hjins["metaobj"] = rps_dump_json_objectref(du, metaobr);
    hjins["metarank"] = Json::Value(md.second);
}
```

### Performance Optimizations

Several performance optimizations are implemented:

1. **Lazy Loading**: Method dictionaries loaded on demand
2. **Caching**: Attribute sets cached in atomic pointers
3. **Atomic Operations**: Thread-safe operations without locks
4. **Depth Limiting**: Protection against infinite recursion

---

## Integration with Other Subsystems

### Code Generation Integration

Meta-objects integrate with code generation systems:

- **cppgen**: C++ code generation from meta-object definitions
- **lightgen**: GNU Lightning code generation
- **gccjit**: GCC libgccjit code generation

### Plugin System Integration

Plugins can extend meta-object functionality:

```cpp
// Plugin creates class with meta-object capabilities
_f.obnewclass = Rps_ObjectRef::make_named_class(&_, _f.obsuperclass, 
                                                std::string{plugarg});

// Plugin adds custom meta-object behavior
_f.obnewclass->put_metadata(custom_meta_object, rank, transient);
```

### REPL Integration

The REPL provides interactive meta-object manipulation:

```cpp
// REPL command creation with meta-object context
_f.obclass = _f.inst->get_class();
_f.obmeta = _f.inst->metaobject();
metark = _f.inst->metarank();
```

---

## Developer Guidelines

### Best Practices

1. **Use Atomic Operations**: Always use atomic operations for meta-object modifications
2. **Implement Depth Limits**: Always implement protection against infinite recursion
3. **Cache Wisely**: Use lazy loading and caching for performance
4. **Thread Safety**: Ensure meta-object operations are thread-safe
5. **Error Handling**: Implement proper error handling for meta-object operations

### Common Patterns

1. **Dynamic Class Creation**: Use `make_named_class` for runtime class creation
2. **Method Injection**: Use `put_own_method` for dynamic method addition
3. **Meta-Object Hierarchy**: Use `metarank()` for meta-object level management
4. **Property Management**: Use attribute sets for property management

### Performance Considerations

1. **Minimize Meta-Object Chain Depth**: Keep meta-object hierarchies shallow
2. **Cache Attribute Sets**: Use `pclass_attrset` for attribute caching
3. **Use Transient Metadata**: Use transient metadata for temporary information
4. **Lazy Evaluation**: Defer expensive operations until needed

---

## Conclusion

RefPerSys's meta-object protocol represents a sophisticated and powerful approach to reflective programming. The system enables dynamic class creation, method manipulation, and self-modification while maintaining thread safety and persistence. The bidirectional object-meta-object relationships create a fully reflective environment where objects can introspect and modify themselves and their relationships.

The protocol's integration with code generation, plugins, and persistence creates a comprehensive platform for meta-programming that is unique in its scope and capability. Understanding this protocol is essential for leveraging RefPerSys's full potential as a reflective, self-modifying system.

---

*This document is part of the RefPerSys documentation suite. For additional information, see the Developer Guide and subsystem-specific documentation.*
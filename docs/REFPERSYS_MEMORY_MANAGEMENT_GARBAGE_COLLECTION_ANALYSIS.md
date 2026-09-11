# RefPerSys Memory Management & Garbage Collection Analysis

## Overview

RefPerSys implements a custom **precise multi-threaded garbage collector** that manages memory for all heap-allocated objects, values, and zones. The system uses a **mark-sweep algorithm** with write barriers to ensure thread safety and precise garbage collection.

## Architecture Overview

### Core Components

1. **Rps_GarbageCollector Class**: Main GC controller with singleton pattern
2. **Rps_QuasiZone**: Base class for all GC-managed memory blocks
3. **Zone Types**: Different memory allocation strategies (ObjectZone, ValueZone, etc.)
4. **Write Barriers**: Automatic tracking of object modifications
5. **Call Frame Integration**: GC-aware function call stack management

### Memory Organization

- **Zones**: Memory blocks allocated with word alignment and gap management
- **Buckets**: Objects organized by ID hash buckets for efficient lookup
- **Root Objects**: Always-reachable objects that anchor the object graph
- **Thread-Local Storage**: Per-thread call frames and random number generators

## Key Classes and Structures

### Rps_GarbageCollector

```cpp
class Rps_GarbageCollector {
    static std::atomic<Rps_GarbageCollector*> gc_this_;
    static std::atomic<uint64_t> gc_count_;
    
    std::mutex gc_mtx;
    std::atomic<bool> gc_running;
    unsigned gc_magic;
    std::function<void(Rps_GarbageCollector*)> gc_rootmarkers;
    std::deque<Rps_ObjectRef> gc_obscanque;
    uint64_t gc_nbscan, gc_nbmark, gc_nbdelete, gc_nbroots;
    double gc_startrealtime, gc_startelapsedtime, gc_startprocesstime;
};
```

**Responsibilities:**
- Singleton GC instance management
- Root marking coordination
- Mark-sweep algorithm execution
- Statistics collection and reporting

### Rps_QuasiZone

```cpp
class Rps_QuasiZone : public Rps_TypedZone {
    static std::recursive_mutex qz_mtx;
    static std::vector<Rps_QuasiZone*> qz_zonvec;
    static uint32_t qz_cnt;
    static std::atomic<uint64_t> qz_alloc_cumulw;
    
    uint32_t qz_rank;
    volatile std::atomic_uint16_t qz_gcinfo;
};
```

**Key Methods:**
- `clear_all_gcmarks()`: Resets GC marks across all zones
- `every_zone()`: Iterates over all zones for sweep phase
- `rps_allocate<>()`: Template-based zone allocation with type safety

### Memory Allocation Strategy

- **Word-Aligned Allocation**: All zones aligned to `rps_allocation_unit` (2*sizeof(void*))
- **Gap Management**: Optional word gaps for ABI alignment requirements
- **Cumulative Tracking**: Global counter of allocated words since process start
- **Type-Safe Templates**: Compile-time type checking for zone allocation

## Critical Algorithms

### Mark-Sweep Algorithm

```cpp
void Rps_GarbageCollector::run_gc(void) {
    gc_running.store(true);
    Rps_QuasiZone::run_locked_gc(*this, [](Rps_GarbageCollector& gc) {
        Rps_QuasiZone::clear_all_gcmarks(gc);
        gc.mark_gcroots();
        Rps_PayloadSymbol::gc_mark_strong_symbols(&gc);
        
        while (!gc.gc_obscanque.empty()) {
            auto obfront = gc.gc_obscanque.front();
            gc.gc_obscanque.pop_front();
            obfront->mark_gc_inside(gc);
            gc.gc_nbscan++;
        }
    });
    
    Rps_QuasiZone::every_zone(*this, [](Rps_GarbageCollector& gc, Rps_QuasiZone* qz) {
        gc.gc_nbmark++;
        if (qz->is_gcmarked(gc)) return;
        delete qz;
        gc.gc_nbdelete++;
    });
    gc_running.store(false);
}
```

**Algorithm Phases:**

1. **Clear Phase**: Reset all GC marks across all zones
2. **Mark Phase**: Traverse from roots, marking reachable objects
3. **Sweep Phase**: Delete unmarked zones

### Root Marking Strategy

```cpp
void Rps_GarbageCollector::mark_gcroots(void) {
    // Mark root objects from global registry
    rps_each_root_object([this](Rps_ObjectRef obr) {
        this->mark_root_objectref(obr);
    });
    
    // Mark application-specific roots
    rps_garbcoll_application(*this);
    
    // Mark hardcoded root objects
    #define RPS_INSTALL_ROOT_OB(Oid) { \
        if (RPS_ROOT_OB(Oid)) \
            this->mark_root_objectref(RPS_ROOT_OB(Oid)); \
    };
    #include "generated/rps-roots.hh"
    
    // Mark hardcoded symbols
    #define RPS_INSTALL_NAMED_ROOT_OB(Oid,Nam) { \
        if (RPS_SYMB_OB(Nam)) \
            this->mark_root_objectref(RPS_SYMB_OB(Nam)); \
    };
    #include "generated/rps-names.hh"
    
    // Mark constants
    #define RPS_INSTALL_CONSTANT_OB(Oid) { \
        if (rpskob##Oid) \
            this->mark_root_objectref(rpskob##Oid); \
    };
    #include "generated/rps-constants.hh"
    
    // Mark active processes
    Rps_PayloadUnixProcess::gc_mark_active_processes(*this);
    
    if (gc_rootmarkers) gc_rootmarkers(this);
}
```

### Object Marking Logic

```cpp
void Rps_ObjectZone::mark_gc_inside(Rps_GarbageCollector& gc) {
    std::lock_guard<std::recursive_mutex> gu(ob_mtx);
    
    // Mark class
    Rps_ObjectZone* obcla = ob_class.load();
    if (obcla) gc.mark_obj(obcla);
    
    // Mark attributes
    for (auto atit: ob_attrs) {
        gc.mark_obj(atit.first);  // attribute name
        if (atit.second.is_ptr()) gc.mark_value(atit.second);
    }
    
    // Mark components
    for (auto compv: ob_comps) {
        if (compv.is_ptr()) gc.mark_value(compv);
    }
    
    // Mark payload
    Rps_Payload* payl = ob_payload.load();
    if (payl && payl->owner() == this) payl->gc_mark(gc);
}
```

## Interface Contracts

### Public GC API

```cpp
// Top-level GC invocation
extern "C" void rps_garbage_collect(std::function<void(Rps_GarbageCollector*)>* fun=nullptr);

// GC control functions
extern "C" void rps_forbid_garbage_collection(void);
extern "C" void rps_allow_garbage_collection(void);
extern "C" bool rps_gc_forbidden(void);
```

### Zone Allocation Interface

```cpp
// Template-based allocation
template <typename ZoneClass, class ...Args>
static ZoneClass* rps_allocate(Args... args);

// Allocation with word gap
template <typename ZoneClass, class ...Args>
static ZoneClass* rps_allocate_with_wordgap(unsigned wordgap, Args... args);
```

### Root Object Management

```cpp
extern "C" void rps_each_root_object(const std::function<void(Rps_ObjectRef)>& fun);
extern "C" void rps_add_root_object(const Rps_ObjectRef);
extern "C" bool rps_remove_root_object(const Rps_ObjectRef);
extern "C" bool rps_is_root_object(const Rps_ObjectRef);
```

## Integration Points

### Call Frame Integration

```cpp
class Rps_ProtoCallFrame {
    void gc_mark_frame(Rps_GarbageCollector* gc);
    void set_additional_gc_marker(const std::function<void(Rps_GarbageCollector*)>& gcmarkfun);
};
```

**Integration Pattern:**
- Call frames automatically mark their state during GC
- Additional markers can be registered for local variables
- Thread-local call frame pointers ensure proper stack tracing

### Symbol Table Integration

```cpp
class Rps_PayloadSymbol {
    static void gc_mark_strong_symbols(Rps_GarbageCollector* gc);
    void gc_mark() const;
};
```

**Integration Pattern:**
- Strong symbols are always marked as roots
- Weak symbols may be collected if not otherwise reachable
- Symbol values are marked recursively

### Payload System Integration

Each payload type implements:
```cpp
virtual void gc_mark(Rps_GarbageCollector& gc) const = 0;
virtual void dump_scan(Rps_Dumper* du) const = 0;
```

**Integration Pattern:**
- Payloads mark their contained objects/values
- Different payload types have specialized marking logic
- Payload ownership verified before marking

## Performance Characteristics

### Time Complexity

- **Mark Phase**: O(live_objects + live_values)
- **Sweep Phase**: O(total_zones)
- **Root Marking**: O(number_of_roots)

### Space Complexity

- **Mark Bits**: 1 bit per zone (stored in qz_gcinfo)
- **Scan Queue**: O(max_live_objects) in worst case
- **Zone Overhead**: 16 bytes minimum per zone

### Memory Allocation

- **Alignment**: 16-byte word alignment for all zones
- **Size Tracking**: Cumulative word count since process start
- **Gap Management**: Configurable word gaps for alignment

### Threading Performance

- **Mutex Usage**: Recursive mutexes for object access
- **Atomic Operations**: Lock-free counters and flags where possible
- **Thread Safety**: All GC operations are thread-safe

## Design Rationale

### Why Mark-Sweep?

- **Precise Collection**: Can identify and collect all unreachable objects
- **Multi-Threaded Safety**: Works correctly in multi-threaded environment
- **Incremental Potential**: Can be extended to incremental collection
- **Memory Efficiency**: No fragmentation issues like mark-compact

### Why Singleton GC Instance?

- **Global Coordination**: Single point of control for all GC operations
- **Statistics Collection**: Centralized performance monitoring
- **Thread Safety**: Avoids race conditions between multiple GC instances

### Why Zone-Based Allocation?

- **Type Safety**: Compile-time type checking for allocations
- **Memory Efficiency**: Word-aligned allocations minimize padding
- **GC Integration**: Built-in GC marking and sweeping support
- **Extensibility**: Easy to add new zone types

### Why Write Barriers?

- **Automatic Tracking**: No manual intervention required
- **Thread Safety**: Atomic operations ensure consistency
- **Performance**: Minimal overhead for object modifications

### Why Call Frame Integration?

- **Precise Roots**: Call stack is accurately traced
- **Local Variable Support**: Can mark local variables in frames
- **Exception Safety**: Works correctly across exceptions

## Critical Design Decisions

1. **Precise vs. Conservative**: Chose precise collection for better memory management
2. **Mark-Sweep vs. Copying**: Mark-sweep avoids copying overhead and works with C++ objects
3. **Threading Model**: Multi-threaded with mutexes for safety
4. **Root Management**: Explicit root registration with automatic symbol/constant marking
5. **Zone Architecture**: Type-safe zone allocation with inheritance hierarchy

## Future Evolution Opportunities

1. **Generational Collection**: Add young/old generation separation
2. **Concurrent Marking**: Parallel marking phase
3. **Incremental Collection**: Reduce pause times
4. **Compaction**: Add mark-compact for fragmentation control
5. **NUMA Awareness**: Optimize for multi-socket systems

This memory management system provides a solid foundation for RefPerSys's object-oriented persistence model, ensuring efficient memory usage while maintaining thread safety and precise garbage collection semantics.
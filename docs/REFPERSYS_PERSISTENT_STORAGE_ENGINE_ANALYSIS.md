# RefPerSys Persistent Storage Engine Technical Analysis

## Executive Summary

The RefPerSys Persistent Storage Engine is a sophisticated system that provides reliable, versioned serialization and deserialization of the entire object system. It implements a JSON-based format with space organization, manifest management, and seamless integration with the plugin system. The engine supports both forward and backward compatibility while maintaining performance through space-based organization and incremental loading.

## 1. Architecture Overview

### 1.1 System Design Philosophy

The Persistent Storage Engine follows several key design principles:

- **Space-Based Organization**: Objects are grouped into logical "spaces" with separate files
- **Version Compatibility**: Forward and backward compatibility across RefPerSys versions
- **Human-Readable Format**: JSON-based serialization for debuggability and version control
- **Atomic Operations**: Two-phase loading with consistency checks
- **Plugin Integration**: Seamless plugin loading and initialization during state restoration

### 1.2 Core Components

#### Rps_Dumper - Serialization Engine
**Purpose**: Converts in-memory objects to persistent JSON format
**Location**: `dump_rps.cc` lines 82-203

```cpp
class Rps_Dumper {
    std::string du_topdir;                                    // Target directory
    std::unordered_map<Rps_Id, Rps_ObjectRef,Rps_Id::Hasher> du_mapobjects; // Object registry
    std::deque<Rps_ObjectRef> du_scanque;                     // Processing queue
    std::map<Rps_ObjectRef,std::shared_ptr<du_space_st>> du_spacemap; // Space organization
    // ... additional state for serialization
};
```

#### Rps_Loader - Deserialization Engine
**Purpose**: Reconstructs in-memory objects from persistent JSON format
**Location**: `load_rps.cc` lines 80-143

```cpp
class Rps_Loader {
    std::string ld_topdir;                     // Source directory
    std::set<Rps_Id> ld_spaceset;              // Discovered spaces
    std::map<Rps_Id,void*> ld_pluginsmap;      // Loaded plugins
    std::map<Rps_Id,Rps_ObjectRef> ld_mapobjects; // Object registry
    std::deque<struct todo_st> ld_todoque;     // Post-load processing
    // ... additional state for loading
};
```

## 2. Storage Format and Schema

### 2.1 JSON Format Specification

#### Space File Structure
Each space is stored in a separate file with the pattern: `persistore/sp{SPACEID}-rps.json`

```json
{
  "format": "RefPerSysFormat2024A",
  "spaceid": "_8J6vNYtP5E800eCr5q",
  "jsoncppversion": "1.9.6",
  "nbobjects": 276,
  "rpsmajorversion": 0,
  "rpsminorversion": 6
}
//+ob_OBJECTID:optional_name
{
  "oid": "OBJECTID",
  "mtime": 1234567890.12,
  "class": "CLASS_OID",
  "attrs": [
    {
      "at": "ATTRIBUTE_OID",
      "va": "VALUE"
    }
  ],
  "comps": [
    "COMPONENT_VALUE"
  ],
  "payload": "payload_type"
}
//-ob_OBJECTID
```

#### Object Serialization Schema
Each object is serialized with the following structure:

```json
{
  "oid": "STRING",           // Object identifier (base62 encoded)
  "mtime": FLOAT,            // Modification time
  "class": "STRING",         // Class object reference
  "attrs": [                 // Attributes array
    {
      "at": "STRING",        // Attribute key object reference
      "va": VALUE           // Attribute value (type-specific)
    }
  ],
  "comps": [VALUE],          // Components array
  "payload": "STRING",       // Payload type identifier
  // Payload-specific fields based on type
}
```

#### Value Type Encoding
- **Integers**: Direct JSON numeric values
- **Strings**: JSON strings or `{"string": "content"}` format
- **Objects**: Base62-encoded OID strings
- **Sets**: `{"vtype": "set", "elem": [OIDS]}`
- **Tuples**: `{"vtype": "tuple", "comp": [VALUES]}`
- **Closures**: `{"vtype": "closure", "fn": OBJECT, "env": [VALUES]}`
- **Instances**: `{"vtype": "instance", "class": OBJECT, "iattrs": [...], "icomps": [...]}`

### 2.2 Manifest File Management

#### MANIFEST.rps Structure
The manifest file provides global system metadata and plugin information:

```json
{
  "format": "RefPerSysFormat2024A",
  "jsoncppversion": "1.9.6",
  "rpsmajorversion": 0,
  "rpsminorversion": 6,
  "shortgitid": "commit_hash",
  "gitbranch": "branch_name",
  "globalroots": [OID_ARRAY],      // Root objects
  "spaceset": [OID_ARRAY],         // Space identifiers
  "constset": [OID_ARRAY],         // Constant objects
  "plugins": [OID_ARRAY],          // Plugin objects
  "globalnames": [                 // Name mappings
    {
      "nam": "symbol_name",
      "obj": "OBJECT_OID"
    }
  ],
  "dumpdate": "2024 date string",
  "progname": "refpersys",
  "progtimestamp": "build timestamp",
  "progmd5sum": "binary checksum"
}
```

## 3. Serialization Mechanism (Dumping)

### 3.1 Dumping Process Flow

#### Step 1: Dumper Initialization
```cpp
Rps_Dumper::Rps_Dumper(const std::string&topdir, Rps_CallFrame*callframe)
```
- Validates target directory
- Creates temporary file suffix
- Initializes timing metrics
- Sets up JSON writer configuration

#### Step 2: Root Object Scanning
```cpp
void Rps_Dumper::scan_roots(void)
```
- Discovers all root objects via `rps_each_root_object()`
- Populates initial scan queue
- Records global root set

#### Step 3: Constant Discovery
```cpp
void Rps_Dumper::add_constants_known_from_RefPerSys_system(void)
void Rps_Dumper::scan_every_source_file_for_constants(void)
```
- Extracts constants from RefPerSys_system object
- Scans source files for `RPS_CONSTANTOBJ_PREFIX` annotations
- Builds complete constant object set

#### Step 4: Object Graph Traversal
```cpp
void Rps_Dumper::scan_loop_pass(void)
```
- Processes scan queue iteratively
- Discovers referenced objects via `scan_object_contents()`
- Handles circular references automatically
- Organizes objects by space

#### Step 5: File Generation
```cpp
void Rps_Dumper::write_all_space_files(void)
void Rps_Dumper::write_all_generated_files(void)
void Rps_Dumper::write_manifest_file(void)
```
- Writes individual space files
- Generates C++ header files for roots, names, constants, data
- Creates manifest file with global metadata
- Performs atomic file operations with backup

### 3.2 Space-Based Organization

#### Space Mapping
```cpp
struct du_space_st {
    Rps_Id sp_id;                    // Space identifier
    std::set<Rps_ObjectRef> sp_setob; // Objects in space
};
```

Each discovered space gets its own JSON file containing only objects belonging to that space. This provides:
- **Parallel Loading**: Different spaces can be loaded in parallel
- **Scalability**: Large object graphs can be split across files
- **Modularity**: Related objects can be grouped logically

#### Atomic File Operations
```cpp
void Rps_Dumper::rename_opened_files(void)
```
- Writes all files with temporary suffix
- Creates backup copies of existing files
- Atomically renames temporary files to final names
- Ensures consistency even on system crash

## 4. Deserialization Mechanism (Loading)

### 4.1 Loading Process Flow

#### Step 1: Manifest Processing
```cpp
void Rps_Loader::parse_manifest_file(void)
```
- Parses MANIFEST.rps file
- Discovers available spaces
- Identifies global roots and constants
- Processes plugin information
- Validates format compatibility

#### Step 2: First Pass Loading
```cpp
void Rps_Loader::first_pass_space(Rps_Id spacid)
```
- Parses space file prologue
- Creates placeholder objects for all OIDs
- Validates object count and basic structure
- Builds object ID mapping

#### Step 3: Root and Constant Initialization
```cpp
void Rps_Loader::initialize_root_objects(void)
void Rps_Loader::initialize_constant_objects(void)
```
- Links root objects to global registry
- Resolves constant object references
- Validates completeness

#### Step 4: Second Pass Loading
```cpp
void Rps_Loader::second_pass_space(Rps_Id spacid)
```
- Re-parses space files for detailed content
- Populates object attributes and components
- Loads payload-specific data
- Links object relationships
- Handles magic attributes and applying functions

#### Step 5: Plugin Loading
```cpp
void Rps_Loader::parse_manifest_file(void)  // plugin section
```
- Discovers required plugins from manifest
- Compiles plugins if source is newer than binary
- Loads plugins via `dlopen()`
- Registers plugin-provided functionality

### 4.2 Two-Pass Loading Architecture

#### Rationale
The two-pass loading approach solves several challenges:

1. **Forward References**: Objects may reference objects defined later in file
2. **Lazy Loading**: Payload loading can be deferred until needed
3. **Consistency**: All objects exist before relationship resolution
4. **Recovery**: Failed loads can be detected early

#### Implementation
```cpp
void Rps_Loader::parse_json_buffer_second_pass(Rps_Id spacid, unsigned lineno,
    Rps_Id objid, const std::string& objbuf, unsigned count)
```
- Parses complete JSON object content
- Resolves all object references
- Loads payload-specific data
- Applies magic attributes and functions
- Handles special cases like applying functions

### 4.3 Todo Queue System

#### Deferred Processing
```cpp
void Rps_Loader::add_todo(const std::function<void(Rps_Loader*)>& todofun)
int Rps_Loader::run_some_todo_functions(void)
```

Post-load processing is handled via a todo queue:
- **Lazy Loading**: Complex objects load incrementally
- **Dependency Resolution**: Objects with unknown dependencies queued
- **Performance**: Processing spread across time to avoid blocking
- **Concurrency**: Multiple todo items can be processed in parallel

## 5. Object Lifecycle Mapping

### 5.1 Creation to Storage Pipeline

```
Object Creation → GC Tracking → Space Assignment → Dumping Queue → 
JSON Serialization → Space File Writing → Manifest Update → Atomic Commit
```

#### Key Stages:
1. **Object Creation**: New objects assigned OIDs and placed in appropriate spaces
2. **GC Integration**: Objects tracked by garbage collector with write barriers
3. **Space Assignment**: Objects automatically assigned to spaces based on creation context
4. **Scan Trigger**: Objects discovered during dump scanning process
5. **Serialization**: Object contents converted to JSON format
6. **File Writing**: Atomic file operations with temporary suffixes
7. **Commit**: Files renamed to final names, ensuring consistency

### 5.2 Storage to Creation Pipeline

```
File Discovery → Manifest Parse → Space File Parse → Object Reconstruction → 
Relationship Linking → Plugin Loading → Root Installation → System Activation
```

#### Key Stages:
1. **File Discovery**: System locates manifest and space files
2. **Manifest Parse**: Global metadata and plugin requirements determined
3. **Space Parse**: Objects discovered and placeholders created
4. **Object Reconstruction**: Full object contents loaded with proper typing
5. **Relationship Linking**: All object references resolved
6. **Plugin Loading**: Required plugins compiled and loaded
7. **Root Installation**: System roots installed in global registry
8. **System Activation**: RefPerSys system fully operational

## 6. Checkpointing and Recovery Strategies

### 6.1 Atomic Operations

#### File-Level Atomicity
- All writes use temporary file suffixes
- Existing files backed up before replacement
- Atomic `rename()` operations for final commit
- Directory operations checked for success

#### Consistency Guarantees
- Manifest always reflects actual state
- Space files contain complete object sets
- Plugin binaries match source timestamps
- No partial state exposure to users

### 6.2 Recovery Mechanisms

#### Version Compatibility
```cpp
if (formatjson.asString() != RPS_MANIFEST_FORMAT
    && formatjson.asString() != RPS_PREVIOUS_MANIFEST_FORMAT)
```
- Supports current and previous format versions
- Graceful degradation for older formats
- Warning messages for version mismatches

#### Corruption Handling
- JSON parsing errors caught and reported with context
- Invalid object structures detected and fail-fast
- Missing plugins cause loading failure (fail-safe)
- Backup files preserved for manual recovery

#### Performance Recovery
- Large object graphs load incrementally
- Todo queue prevents blocking on complex objects
- Space-based parallel loading possible
- Memory usage monitored during loading

## 7. Performance Characteristics

### 7.1 Time Complexity Analysis

#### Serialization (Dumping)
- **Object Discovery**: O(n) where n = total objects
- **JSON Generation**: O(n) with constant factors per object type
- **File Writing**: O(n) with I/O costs
- **Space Organization**: O(n log n) for space mapping

#### Deserialization (Loading)
- **Manifest Parse**: O(1) - constant size
- **First Pass**: O(n) - linear scan for object discovery
- **Second Pass**: O(n) - detailed object reconstruction
- **Plugin Loading**: O(p log p) where p = number of plugins

### 7.2 Space Complexity

#### Memory Usage
- **Dumper**: O(n) for object registry and scan queue
- **Loader**: O(n) for object registry and todo queue
- **Peak Memory**: Approximately 2x object count during loading
- **Streaming**: Large spaces can be processed streamingly

#### Storage Efficiency
- **JSON Overhead**: Minimal due to compact OID encoding
- **Space Files**: Organized for efficient incremental loading
- **Compression**: Could be added but currently uses plain text
- **Version Control**: Git-friendly format for change tracking

### 7.3 Scalability Considerations

#### Large System Support
- **Space Division**: Object graphs can be split across multiple spaces
- **Parallel Loading**: Different spaces can be loaded concurrently
- **Incremental Dumps**: Only modified spaces need updating
- **Plugin Isolation**: Plugin failures don't affect core system

#### Performance Optimization
- **Write Barriers**: Minimize GC impact during dumping
- **Batching**: Multiple objects processed together
- **Caching**: Plugin loader functions cached to avoid repeated dlsym
- **Lazy Loading**: Complex objects loaded only when needed

## 8. Integration Points

### 8.1 Garbage Collector Integration

#### Write Barriers
```cpp
#define RPS_WRITE_BARRIER() \
    /* Called after object modifications */
    _.foo.RPS_WRITE_BARRIER()
```

Objects modified during dumping are tracked by GC to ensure consistency.

#### Call Frame Integration
```cpp
Rps_Dumper(const std::string&topdir, Rps_CallFrame*callframe)
```

All dumping operations run within proper GC call frames for safety.

### 8.2 Plugin System Integration

#### Dynamic Plugin Loading
```cpp
std::string pluginsopath = load_real_path(std::string{"plugins/rps"} + curpluginid.to_string() + "-mod.so");
void* dlh = dlopen(pluginsopath.c_str(), RTLD_NOW | RTLD_GLOBAL);
```

Plugins are automatically discovered, compiled, and loaded during state restoration.

#### Plugin State Preservation
- Plugin object state saved in space files
- Plugin binary state preserved through source regeneration
- Plugin initialization functions called during loading

### 8.3 Code Generation Integration

#### Generated File Management
```cpp
void Rps_Dumper::write_generated_roots_file(void)
void Rps_Dumper::write_generated_constants_file(void)
```

Generated C++ files provide compile-time access to persistent objects.

#### Runtime Code Integration
```cpp
void*funad = dlsym(rps_proghdl, appfunambuf);
obz->loader_put_applyingfunction(this, reinterpret_cast<rps_applyingfun_t*>(funad));
```

Applying functions linked via dlsym for runtime code execution.

## 9. Design Rationale

### 9.1 Format Choice: JSON

#### Advantages
- **Human Readable**: Easy debugging and version control
- **Standard Library**: JsonCpp provides robust parsing
- **Tool Support**: Compatible with standard JSON tools
- **Extensible**: Easy to add new object types

#### Trade-offs
- **Size**: Larger than binary formats
- **Parse Overhead**: Slower than binary parsing
- **Type Safety**: Runtime validation required

### 9.2 Space-Based Organization

#### Benefits
- **Modularity**: Related objects grouped logically
- **Scalability**: Large systems split across files
- **Parallelism**: Different spaces load concurrently
- **Maintainability**: Easy to understand system structure

#### Implementation Complexity
- **Space Assignment**: Automatic vs manual allocation
- **Cross-References**: Objects in different spaces can reference each other
- **Migration**: Moving objects between spaces requires coordination

### 9.3 Two-Pass Loading

#### Problem Solved
- **Forward References**: Objects may reference objects defined later
- **Circular References**: Objects may have mutual references
- **Lazy Dependencies**: Some objects depend on system initialization

#### Performance Impact
- **Parse Cost**: Files parsed twice (acceptable for human-readable format)
- **Memory Usage**: Object placeholders held during first pass
- **Complexity**: More complex than single-pass loading

## 10. Error Handling and Diagnostics

### 10.1 Validation Mechanisms

#### Format Validation
```cpp
if (formatjson.asString() != RPS_MANIFEST_FORMAT
    && formatjson.asString() != RPS_PREVIOUS_MANIFEST_FORMAT)
```

Strict validation ensures data integrity and provides clear error messages.

#### Consistency Checks
- Object count validation
- OID uniqueness verification
- Class reference validation
- Plugin availability checks

### 10.2 Error Recovery

#### Graceful Degradation
- Warning messages for non-critical issues
- Fallback to previous format versions
- Partial system operation when possible

#### Failure Isolation
- Plugin failures don't crash system
- Corrupt space files don't affect other spaces
- Recovery possible from backup files

### 10.3 Debugging Support

#### Comprehensive Logging
```cpp
RPS_DEBUG_LOG(DUMP, "dumper write_manifest_file start");
RPS_DEBUG_LOG(LOAD, "Rps_Loader::second_pass_space start spacid:" << spacid);
```

Extensive debug logging throughout the process with category-based filtering.

#### Context Information
- File paths and line numbers for errors
- Object IDs and relationships
- Plugin loading status
- Performance metrics

## 11. Future Enhancements

### 11.1 Performance Improvements
- **Binary Format**: Optional binary serialization for performance
- **Compression**: Transparent compression for large spaces
- **Streaming**: Process large spaces without loading entirely
- **Parallel Processing**: Multi-threaded loading of independent spaces

### 11.2 Reliability Enhancements
- **Checksums**: File integrity verification
- **Incremental Dumps**: Only save changed objects
- **Journaling**: Transaction log for crash recovery
- **Migration Tools**: Automated format upgrade utilities

### 11.3 Integration Improvements
- **Distributed Storage**: Remote space file access
- **Database Backend**: Optional database storage
- **Cloud Integration**: Cloud storage synchronization
- **API Integration**: External system connectivity

## Conclusion

The RefPerSys Persistent Storage Engine provides a robust, scalable, and maintainable solution for object persistence. Its JSON-based format ensures human readability and version control compatibility, while its space-based organization and two-pass loading enable efficient handling of large, complex object graphs. The integration with the plugin system and garbage collector demonstrates sophisticated system design that prioritizes both functionality and performance.

The engine's design choices reflect careful consideration of trade-offs between readability, performance, and maintainability, resulting in a system that is both powerful enough for symbolic AI applications and accessible for debugging and evolution.
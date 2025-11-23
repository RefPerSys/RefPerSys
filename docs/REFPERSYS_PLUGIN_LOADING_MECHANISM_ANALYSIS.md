# RefPerSys Plugin Loading Mechanism Analysis

## Overview

The RefPerSys plugin loading mechanism provides a dynamic extension system that allows runtime loading of compiled C++ code modules. This system enables the system to be extended with new functionality without recompiling the core engine, supporting both manifest-driven automatic loading and command-line manual loading.

## Architecture Overview

### Core Components

1. **Rps_Plugin Structure**: Represents a loaded plugin with name and dynamic library handle
2. **Plugin Discovery System**: Manifest-based and command-line plugin specification
3. **Dynamic Loading Engine**: Uses POSIX dlopen/dlsym for runtime linking
4. **Standardized Interface**: All plugins implement `rps_do_plugin` function
5. **Argument Passing**: Plugin-specific arguments via global map

### Key Classes and Structures

#### Rps_Plugin
```cpp
struct Rps_Plugin {
    std::string plugin_name;    // Plugin identifier (e.g., "rpsplug_display")
    void* plugin_dlh;          // dlopen handle (RTLD_GLOBAL scope)
};
```

#### Plugin Loading Infrastructure
- **rps_plugins_vector**: Global vector of loaded Rps_Plugin instances
- **rps_pluginargs_map**: Map from plugin name to argument string
- **RPS_PLUGIN_INIT_NAME**: Macro defining "rps_do_plugin" entry point

## Plugin Discovery and Loading Mechanism

### Manifest-Driven Loading

Plugins are primarily discovered through JSON manifest files:

1. **System Manifest** (`rps_manifest.json`): Contains "plugins" array with plugin IDs
2. **User Manifest** (`~/.refpersys.json`): Optional user-specific plugins

#### Manifest Format
```json
{
  "plugins": ["_0abcd123", "_1efgh456", "_2ijkl789"]
}
```

### Loading Sequence

1. **Manifest Parsing**: `Rps_Loader::parse_manifest_file()` reads plugin IDs
2. **Path Construction**: Builds paths like `plugins/rps{PID}-mod.so` and `generated/rps{PID}-mod.cc`
3. **Build Verification**: Compares source and binary timestamps
4. **Automatic Compilation**: Uses `do-build-refpersys-plugin` if binary is stale
5. **Dynamic Loading**: `dlopen(path, RTLD_NOW | RTLD_GLOBAL)`
6. **Handle Storage**: Stores dlopen handle in `ld_pluginsmap`

### Command-Line Loading

Plugins can also be loaded manually via command-line options:

- `--plugin-after-load=PATH`: Load plugin from specified path
- `--plugin-arg=NAME:ARG`: Pass arguments to named plugin

## Standardized Plugin Interface

### Required Entry Point

All plugins must export a C-linkable function:

```cpp
extern "C" void rps_do_plugin(const Rps_Plugin* plugin);
```

### Plugin Structure Template

```cpp
#include "refpersys.hh"

extern "C" const char rpsplug_example_shortgitid[];
const char rpsplug_example_shortgitid[] = RPS_SHORTGIT;

void rps_do_plugin(const Rps_Plugin* plugin) {
    RPS_LOCALFRAME(/*descr:*/ nullptr, /*callerframe:*/ nullptr,
                   // local variables
                   Rps_ObjectRef obj;
                  );

    const char* plugarg = rps_get_plugin_cstr_argument(plugin);
    // Plugin implementation
}
```

## Plugin Lifecycle and Initialization Sequences

### 1. Discovery Phase
- Manifest parsing identifies plugin IDs
- Plugin paths constructed from ID patterns
- Source/binary timestamp comparison

### 2. Compilation Phase (if needed)
- Automatic build using `do-build-refpersys-plugin`
- Source: `generated/rps{PID}-mod.cc`
- Output: `plugins/rps{PID}-mod.so`
- Build command: `make one-plugin REFPERSYS_PLUGIN_SOURCE=...`

### 3. Loading Phase
- `dlopen()` with `RTLD_NOW | RTLD_GLOBAL` flags
- Global symbol visibility for cross-plugin communication
- Handle stored in loader's plugin map

### 4. Initialization Phase
- `dlsym()` locates `rps_do_plugin` function
- Function called with `Rps_Plugin*` parameter
- Plugin executes in loaded application context

### 5. Runtime Phase
- Plugin has full access to RefPerSys object graph
- Can create/modify objects, register symbols
- Memory managed by garbage collector

## Dynamic Loading via dlopen() with Standardized Interfaces

### dlopen Flags and Behavior

```cpp
void* dlh = dlopen(plugin_path, RTLD_NOW | RTLD_GLOBAL);
```

- **RTLD_NOW**: Resolve all symbols immediately (fail fast)
- **RTLD_GLOBAL**: Make symbols visible to subsequently loaded libraries
- **Thread Safety**: Protected by loader mutex during loading

### Symbol Resolution

```cpp
void* symad = dlsym(dlh, RPS_PLUGIN_INIT_NAME);
rps_plugin_init_sig_t* pluginit = reinterpret_cast<rps_plugin_init_sig_t*>(symad);
```

### Error Handling

- `dlerror()` checked after dlopen/dlsym operations
- Fatal errors on loading failures
- Plugin loading is atomic (all-or-nothing)

## Cross-Plugin Communication Patterns

### Shared Symbol Space

- **RTLD_GLOBAL** enables symbol sharing between plugins
- Plugins can call functions from other loaded plugins
- Global RefPerSys symbols accessible to all plugins

### Object Graph Communication

- Plugins communicate through shared object references
- Global symbol table (`RPS_SYMB_OB`) for named object access
- Root object registry for system-wide state

### Memory Management Integration

- All plugin-allocated objects subject to garbage collection
- `RPS_LOCALFRAME` ensures proper stack rooting
- Cross-plugin object references automatically tracked

## Extension Point Definitions and Implementations

### Primary Extension Points

1. **Object Creation**: Plugins can create new classes, instances, symbols
2. **REPL Commands**: Extend read-eval-print-loop functionality
3. **Code Generation**: Add new code generation backends
4. **Serialization**: Custom payload types with load/save logic

### Plugin Categories

#### Creation Plugins
- `rpsplug_createclass`: Define new object classes
- `rpsplug_createsymbol`: Register named symbols
- `rpsplug_create_cplusplus_code_class`: C++ code generation classes

#### Operational Plugins
- `rpsplug_display`: Object display functionality
- `rpsplug_simpinterp`: Simple interpreter operations
- `rpsplug_thesetreploper`: REPL operator definitions

#### Infrastructure Plugins
- `rpsplug_installrootoid`: Root object installation
- `rpsplug_root2const`: Convert roots to constants

## Interface Contracts

### Plugin to System Contracts

#### Required Functions
- `rps_do_plugin(const Rps_Plugin*)`: Main entry point
- Optional: Custom payload loaders (`rpsldpy_*` functions)

#### Argument Access
```cpp
const char* rps_get_plugin_cstr_argument(const Rps_Plugin* plugin);
const char* rps_get_extra_arg(const char* name);
```

#### Object Creation
```cpp
Rps_ObjectRef::make_object(Rps_CallFrame*, Rps_ObjectRef classob, ...);
Rps_ObjectRef::find_object_by_string(Rps_CallFrame*, const std::string&, ...);
```

### System to Plugin Contracts

#### Memory Management
- Automatic garbage collection of plugin objects
- `RPS_LOCALFRAME` for stack rooting
- `Rps_ObjectRef` smart pointers

#### Thread Safety
- Single-threaded plugin execution
- Loader mutex protects loading operations
- Object graph operations are thread-safe

## Performance Characteristics

### Loading Performance
- **Manifest Parsing**: O(n) where n = number of plugins
- **dlopen Overhead**: ~100μs per plugin (Linux typical)
- **Symbol Resolution**: Cached after first dlsym call

### Memory Overhead
- **Per Plugin**: ~4KB for dlopen metadata + code size
- **Global Symbol Table**: Shared across all plugins
- **RTLD_GLOBAL**: Increases symbol lookup time marginally

### Runtime Performance
- **Function Calls**: Direct C++ calls (no overhead)
- **Object Access**: Through smart pointer indirection
- **Garbage Collection**: Plugin objects participate fully

## Design Rationale

### Why dlopen()?
- **POSIX Standard**: Portable across Unix-like systems
- **Runtime Flexibility**: Load code without restarting
- **Symbol Isolation**: Each plugin in separate namespace
- **Memory Efficiency**: Code shared when possible

### Why RTLD_GLOBAL?
- **Cross-Plugin Calls**: Enable plugin interoperability
- **System Integration**: Access to RefPerSys internals
- **Extension Composability**: Plugins can build on each other

### Why Manifest-Driven?
- **Reproducible Builds**: Declarative plugin specification
- **Version Management**: Plugins versioned with system
- **Automatic Dependencies**: Build system handles compilation

### Why Single Entry Point?
- **Simplicity**: One function to implement
- **Consistency**: Uniform plugin interface
- **Debugging**: Clear execution path

## Integration Points

### With Persistent Storage
- Plugin objects serialized in persistent store
- Custom payload types with load/save functions
- Manifest versioning ensures compatibility

### With Code Generation
- Plugins can define new code generation targets
- Runtime code generation integration
- JIT compilation support

### With REPL System
- Command extensions via plugin-defined operators
- Interactive plugin development
- Runtime system extension

### With Garbage Collection
- Plugin objects participate in GC cycles
- Proper rooting prevents premature collection
- Memory safety guarantees

## Critical Algorithms

### Plugin Loading Algorithm
```
for each plugin_id in manifest.plugins:
    source_path = "generated/rps" + plugin_id + "-mod.cc"
    binary_path = "plugins/rps" + plugin_id + "-mod.so"

    if needs_rebuild(source_path, binary_path):
        build_plugin(source_path, binary_path)

    dlhandle = dlopen(binary_path, RTLD_NOW|RTLD_GLOBAL)
    store_handle(plugin_id, dlhandle)

    init_func = dlsym(dlhandle, "rps_do_plugin")
    init_func(plugin_struct)
```

### Argument Resolution
```
plugin_args = parse_command_line("--plugin-arg=NAME:VALUE")
for each loaded_plugin:
    if plugin.name == requested_name:
        rps_pluginargs_map[plugin.name] = value
```

## Future Evolution

### Potential Enhancements
- **Plugin Dependencies**: Explicit dependency declarations
- **Version Compatibility**: Semantic versioning checks
- **Hot Reloading**: Runtime plugin replacement
- **Sandboxing**: Restricted plugin execution environments

### Compatibility Considerations
- **ABI Stability**: dlopen requires compatible C++ ABIs
- **Symbol Conflicts**: RTLD_GLOBAL can cause naming collisions
- **Memory Leaks**: Plugin unloading not currently supported

This plugin system provides a robust foundation for RefPerSys extensibility while maintaining system integrity and performance.
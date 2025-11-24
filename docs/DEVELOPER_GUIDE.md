# RefPerSys Developer Guide

## Table of Contents
1. [Project Overview](#project-overview)
2. [Build Process](#build-process)
3. [Code Organization](#code-organization)
4. [Key Concepts](#key-concepts)
5. [Core Object Model and Architecture](#core-object-model-and-architecture)
   - [Core Object Model and Architecture](#core-object-model-and-architecture-1)
   - [Object Identity and OID System](#object-identity-and-oid-system)
   - [Class System and Inheritance](#class-system-and-inheritance)
   - [Attribute and Method Management](#attribute-and-method-management)
   - [Memory Management and Garbage Collection](#memory-management-and-garbage-collection)
6. [Program Startup and Initialization](#program-startup-and-initialization)
7. [Command-Line Interface](#command-line-interface)
8. [Operational Modes](#operational-modes)
9. [Core Subsystem Initialization](#core-subsystem-initialization)
10. [Development Guidelines](#development-guidelines)

---

## Project Overview

RefPerSys (Reflective Persistent System) is a **free software symbolic artificial intelligence system and inference engine** designed as a research project to become a better alternative to CLIPS rules engine. It aims to be a **reflective persistent system** with a dynamically typed, garbage-collected value heap.

### Key Characteristics

- **Purpose**: Symbolic AI system and inference engine
- **License**: GPLv3+ (free software)
- **Primary Language**: C++17
- **Target Platform**: Linux/x86-64 (with possibility of extension to other 64-bit Linux systems)
- **Architecture**: Homoiconic system with automatic code generation capabilities
- **Memory Management**: Custom multi-threaded precise garbage collector
- **Persistence**: Evolving, persistable heap with garbage collection

### Research Goals

The project serves as a research platform for:
- Developing better alternatives to existing rule engines
- Exploring homoiconic programming languages
- Implementing reflective, persistent programming systems
- Advanced symbolic artificial intelligence techniques
- Code generation and automatic programming

### System Design Philosophy

- **Reflective**: The system can inspect and modify its own structure
- **Persistent**: Objects and data survive program termination
- **Garbage Collected**: Automatic memory management with precise GC
- **Multi-threaded**: Designed for concurrent execution with controlled thread pools
- **Homoiconic**: Code and data have the same representation, enabling self-generation

---

## Build Process

### System Requirements

#### Required Software
- **Operating System**: Linux (x86-64 architecture preferred)
- **C++ Compiler**: GCC 13+ or GCC 14/15 (GCC 15 preferred)
- **Build System**: GNU make 4+ with Guile integration
- **Version Control**: Git

#### Required Libraries
- **GNU lightning**: For machine code generation
- **libunistring**: For UTF-8 support
- **libbacktrace**: For stack traces and debugging
- **libgccjit**: For JIT code generation
- **libreadline**: For REPL functionality
- **libjsoncpp**: For JSON processing

#### Optional Libraries
- **FLTK**: For graphical user interface
- **libonion**: For HTTP service
- **GTKmm**: For additional GUI components

### Build Configuration

#### Environment Setup
```bash
# Set the RefPerSys top directory
export REFPERSYS_TOPDIR=$HOME/work/RefPerSys

# Ensure your PATH includes make, git, and RefPerSys binaries
export PATH=$HOME/bin:$PATH
```

#### Build Dependencies Installation (Debian/Ubuntu)
```bash
# Core development tools
sudo apt install -y gcc-15 g++-15 libgccjit-15-dev clang-19
sudo apt build-dep g++-15
sudo apt install libunistring-dev libjsoncpp-dev libssl-dev
sudo apt install bisonc++ bisonc++-doc ccache g++ make build-essential
sudo apt install ttf-unifont ttf-mscorefonts-installer

# Build and install libbacktrace
git clone https://github.com/ianlancetaylor/libbacktrace.git
cd libbacktrace
./configure
make && sudo make install
sudo ldconfig -v -a
```

### Build Steps

#### Initial Configuration
```bash
# Navigate to RefPerSys source directory
cd $REFPERSYS_TOPDIR

# Run configuration (generates _config-refpersys.mk)
make config
```

#### Standard Build
```bash
# Build RefPerSys core and all components
make -j4 refpersys && make all

# Alternative: Build with optimization
make clean
make -j4 refpersys
```

#### Build Variants

**Raw Build (no GUI)**
```bash
# Build without FLTK dependencies
make raw-refpersys
```

**LTO Build (Link-Time Optimization)**
```bash
# Build with link-time optimization
make lto-refpersys
```

**Individual Plugin Build**
```bash
# Build specific plugin
make REFPERSYS_PLUGIN_SOURCE=plugins_dir/your_plugin.cc one-plugin
```

**Debug Build**
```bash
# Build with debug symbols
make clean
make -j4 refpersys DEBUG=1
```

### Post-Build Verification
```bash
# Run basic tests
make test00
make test01
make test03

# Run FLTK tests (if GUI enabled)
make testfltk1
```

### Build Troubleshooting

**Common Issues**:
- Missing `REFPERSYS_TOPDIR` environment variable
- Insufficient dependencies
- GCC version compatibility (use GCC 13+)
- Missing Guile integration in GNU make

**Solutions**:
```bash
# Verify environment
echo $REFPERSYS_TOPDIR

# Check GNU make version and Guile support
make --version
make --print-data-base | grep -i guile

# Clean rebuild
make clean
make config
make -j4 refpersys && make all
```

---

## Code Organization

The RefPerSys codebase follows a modular C++17 architecture with clear separation of concerns:

### Root Directory Structure

```
RefPerSys/
├── *.cc              # Hand-written C++ source files
├── *.hh              # C++ header files
├── GNUmakefile       # Build automation
├── refpersys.hh      # Main header file
├── generated/        # Auto-generated code
├── plugins_dir/      # Plugin system
├── doc/              # Documentation
├── test_dir/         # Test suite
└── tools/            # Build tools
```

### Core Source Files

#### Main C++ Implementation Files
- **`main_rps.cc`**: Entry point and main program logic
- **`garbcoll_rps.cc`**: Custom garbage collector implementation
- **`load_rps.cc`**: Persistent state loading and serialization
- **`output_rps.cc`**: Output and dumping functionality
- **`objects_rps.cc`**: Object system and value management
- **`scalar_rps.cc`**: Scalar value types and operations

#### Specialized Components
- **`repl_rps.cc`**: Read-Eval-Print Loop implementation
- **`eventloop_rps.cc`**: Event handling and threading
- **`lexer_rps.cc`**: Lexical analysis for parsing
- **`parsrepl_rps.cc`**: Parser for REPL commands
- **`cppgen_rps.cc`**: C++ code generation
- **`gccjit_rps.cc`**: GCC JIT integration for machine code
- **`lightgen_rps.cc`**: Lightning-based code generation

#### GUI Components
- **`fltk_rps.cc`**: FLTK-based graphical interface
- **`cmdrepl_rps.cc`**: Command-line REPL interface

### Generated Code Directory (`generated/`)

Auto-generated files for:
- **Constant definitions**: `rps-constants.hh`
- **Name mappings**: `rps-names.hh`
- **Parser implementations**: `rps-parser-decl.hh`, `rps-parser-impl.cc`
- **Root object definitions**: `rps-roots.hh`
- **Platform-specific data**: `rpsdata_GNU_Linux_*.h`

### Plugin System (`plugins_dir/`)

Extensible plugin architecture with shared objects:

#### Current Plugins
- **`rpsplug_createclass.cc`**: Dynamic class creation
- **`rpsplug_createnamedattribute.cc`**: Attribute management
- **`rpsplug_createsymbol.cc`**: Symbol creation and handling
- **`rpsplug_display.cc`**: Display and visualization
- **`rpsplug_cplusplustypes.cc`**: C++ type system integration

#### Plugin Development
- Hand-written C++ source files
- Compiled as shared objects (`.so` files)
- Loaded dynamically via `dlopen(3)`
- Define new RefPerSys classes and operations

### Configuration and Build System

#### Build Files
- **`GNUmakefile`**: Primary build automation using GNU make
- **`tools/do-configure-refpersys.c`**: Configuration tool
- **`do-scan-refpersys-pkgconfig.c`**: Dependency scanning
- **`do-build-refpersys-plugin.cc`**: Plugin compilation utilities

#### Generated Build Files
- **`_config-refpersys.mk`**: Generated configuration
- **`_scanned-pkgconfig.mk`**: Dependency information
- **`Make-dependencies/`**: Build dependency tracking

### Documentation (`doc/`)

#### Architecture Documentation
- **`garbage-collection.md`**: GC implementation details
- **`primordial-persistence.md`**: Persistence mechanisms
- **`repl.md`**: REPL implementation guide
- **`fcgi.md`**: Web interface documentation

#### Research Papers
- **`IntelliSys2021/`**: Conference papers and presentations
- **`ecai-2020/`**: ECAI 2020 submissions
- **`iccait-2021/`**: Research publications

### Testing (`test_dir/`)

Comprehensive test suite:
- **Lexical analysis tests**: `001test.bash`
- **Core system tests**: `002crintptr.bash`, `003crint.bash`
- **Value system tests**: `004crscaltypes.bash`
- **Script execution tests**: `005script.bash`

### Legacy and Archive (`attic/`)

Historical code and alternative implementations:
- Previous GUI implementations (GTK, Qt, Fox)
- Web interface prototypes
- Build system alternatives
- Parser generator experiments

---

## Key Concepts

### Garbage Collection System

RefPerSys implements a **custom precise multi-threaded garbage collector** with the following characteristics:

- **Precise GC**: Knows exact structure of all objects
- **Generational**: Separate handling for new and old objects
- **Multi-threaded**: Supports concurrent mutator threads
- **Write barriers**: Track modifications for incremental collection
- **Local frames**: Explicit call frame management

### Memory Management

#### Memory Zones
- **Small blocks**: 8 MB zones for frequently allocated objects
- **Large blocks**: 64 MB zones for bigger objects
- **mmap-based**: Direct memory mapping for performance

#### Quasi-values vs Values
- **Quasi-values**: GC-managed memory zones
- **Values**: First-class RefPerSys values (including tagged integers)
- **Objects**: Mutable values with explicit class definitions

### Persistent Storage

#### Dump and Load Mechanism
- **Snapshot-based**: Periodic state serialization
- **Incremental updates**: Efficient change tracking
- **Version compatibility**: Forward and backward compatibility
- **Format evolution**: Structured evolution of stored data

### Code Generation

#### Multiple Code Generation Strategies
1. **GCC JIT**: High-quality optimized code via libgccjit
2. **GNU Lightning**: Fast code generation for simple cases
3. **C++ Generation**: Template-based C++ code generation
4. **Plugin System**: Dynamic shared object generation

### Homoiconic Design

#### Self-Referential Structure
- **AST Representation**: Abstract syntax trees as first-class objects
- **Code as Data**: Programs can inspect and modify themselves
- **Automatic Generation**: System generates its own components
- **Reflective Operations**: Runtime introspection and modification

---

## Core Object Model and Architecture

RefPerSys implements a sophisticated object-oriented architecture that integrates symbolic AI concepts with practical system design. The object model follows a **homoiconic** design philosophy where code and data are represented uniformly as objects.

### Core Object Model and Architecture

The system architecture consists of fundamental components working together to provide a reflective, persistent programming environment:

#### Base Classes and Structures

**`Rps_ObjectZone`** - The Core Object Class
- **Purpose**: The fundamental mutable object type in RefPerSys
- **Location**: `objects_rps.cc`, `refpersys.hh`
- **Key Characteristics**:
  - Allocated in GC-managed memory zones
  - Each object has its own recursive mutex for thread safety
  - Immutable OID assigned at creation for persistent identity
  - Contains attributes, components, and optional payload data

```cpp
class Rps_ObjectZone {
    Rps_Id _ob_oid;                    // Unique object identifier
    std::recursive_mutex* _ob_mutex;   // Per-object lock
    std::atomic<Rps_ObjectZone*> _ob_class; // Class reference (atomic for performance)
    // Attribute storage, components, payload...
};
```

**`Rps_Value`** - First-Class Value System
- **Purpose**: Represent first-class RefPerSys values in a single word
- **Location**: `refpersys.hh` lines 1732-1912
- **Design**: Tagged union using type tag to distinguish between different value types
- **Value Types** include:
  - Payloads (negative values): Class information, mutable sets, vectors, symbols
  - Special values: Tagged integers, nil values
  - Boxed values: Strings, doubles, sets, tuples, objects, closures, instances

**`Rps_ObjectRef`** - Object Reference Management
- **Purpose**: C++ smart pointer-like reference to objects
- **Location**: `refpersys.hh` lines 1376-1581
- **Features**: Rule of Five, thread safety with atomic operations, GC integration, type safety

### Object Identity and OID System

#### Rps_Id - Unique Object Identifiers

RefPerSys uses **128-bit unique object identifiers** to ensure persistent object identity across system restarts:

```cpp
class Rps_Id {
    uint64_t _id_hi;    // High 64 bits
    uint64_t _id_lo;    // Low 64 bits
    
    // Base-62 encoding for string representation
    static constexpr const char b62digits[] = 
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
};
```

#### Key Features:
- **Uniqueness**: Cryptographically strong random generation using `Rps_Random::random_64u()`
- **Persistence**: OIDs remain stable across system restarts
- **String Representation**: Base-62 encoded for compact display
- **Hashing**: Custom hash function for efficient lookup in hash maps

#### Object Identity Management:
```cpp
std::unordered_map<Rps_Id, Rps_ObjectZone*, Rps_Id::Hasher> 
    Rps_ObjectZone::ob_idmap_;

std::map<Rps_Id, Rps_ObjectZone*> 
    Rps_ObjectZone::ob_idbucketmap_[Rps_Id::maxbuckets];
```

- **`find_object_by_oid()`**: Fast lookup by OID
- **`find_object_by_string()`**: Lookup by string representation
- **`really_find_object_by_oid()`**: Internal lookup with no fallbacks

### Class System and Inheritance

#### Class Hierarchy

RefPerSys uses a **single inheritance system** with root classes:

- **`the_object_class()`**: Superclass of all objects
- **`the_class_class()`**: Class of all classes  
- **`the_symbol_class()`**: Class of all symbols
- **`the_mutable_set_class()`**: Class of mutable sets

#### Class Creation Process

Classes are created dynamically using the plugin system:

```cpp
static Rps_ObjectRef make_named_class(
    Rps_CallFrame*callerframe, 
    Rps_ObjectRef superclassob, 
    std::string name);
```

The process involves:
1. Create class object with `make_named_class()`
2. Set class name attribute
3. Create corresponding symbol object  
4. Add to global class registry
5. Optionally mark as root or constant

#### Rps_PayloadClassInfo - Class Metadata

Each class stores metadata through the `Rps_PayloadClassInfo` plugin:
- **`name`**: Human-readable class name
- **`comment`**: Documentation or description
- **`symbol`**: Symbol object representing the class name
- **Method Dictionary**: Class-specific methods

#### Inheritance System:
- **Single Inheritance**: Each class has exactly one superclass
- **Method Resolution**: Dynamic method lookup through class hierarchy
- **Instance Checking**: `is_instance_of()` and `is_subclass_of()` methods

### Attribute and Method Management

#### Attribute Storage System

Attributes provide flexible object state management:

- **Key**: Object reference (attribute identifier)
- **Value**: Arbitrary `Rps_Value`
- **Storage**: Mutable associative table per object
- **Access**: Locked operations for thread safety

Core operations:
```cpp
Rps_Value get_attr(Rps_CallFrame*stkf, const Rps_ObjectRef obattr) const;
void put_attr(const Rps_ObjectRef obattr, const Rps_Value val);
```

#### Method System

Methods are installed as closures attached to class objects:

**Method Representation**:
- **Selectors**: Objects that identify methods
- **Closures**: First-class functions stored as values
- **Installation**: Class-level method registration

**Method Installation**:
```cpp
void install_own_method(
    Rps_CallFrame*callerframe, 
    Rps_ObjectRef obsel,     // Method selector
    Rps_Value closv);        // Closure value

void install_own_2_methods(
    Rps_CallFrame*callerframe, 
    Rps_ObjectRef obsel0, Rps_Value closv0,
    Rps_ObjectRef obsel1, Rps_Value closv1);
```

**Method Invocation**:
- **Message Sending**: `send0()` through `send9()` methods for 0-9 arguments
- **Vector Support**: `send_vect()` for dynamic argument lists
- **A-Normal Form**: Single call per statement for GC integration

#### Closure System

**`Rps_ClosureZone`** provides first-class function representation:
- **Environment**: Captured variables
- **Function**: Code to execute
- **Application**: `apply0()` through `apply10()` methods

### Memory Management and Garbage Collection

RefPerSys implements a **custom precise multi-threaded garbage collector** with sophisticated memory management:

#### Garbage Collector Architecture

**`Rps_GarbageCollector`** features:
- **Precise GC**: Knows exact object layout for accurate collection
- **Generational**: Separate handling for young/old objects
- **Multi-threaded**: Concurrent mutator support with proper synchronization
- **Write Barriers**: Track modifications for incremental collection

#### Collection Algorithm

The GC uses multiple strategies:
- **Cheney-like copying** for immutable values
- **Tri-color mark-and-sweep** for objects
- **Atomic operations** for thread safety:
```cpp
std::atomic<Rps_GarbageCollector*> gc_this_;
std::atomic<uint64_t> gc_count_;
```

#### Memory Zones

**Zone Allocation Strategy**:
- **Small Blocks**: 8 MB zones (`RPS_SMALL_BLOCK_SIZE`) for frequently allocated objects
- **Large Blocks**: 64 MB zones (`RPS_LARGE_BLOCK_SIZE`) for bigger objects  
- **mmap-based**: Direct memory mapping for performance

**Zone Types**:
- **`Rps_QuasiZone`**: GC-managed memory zones
- **`Rps_ZoneValue`**: Base class for all GC values
- **`Rps_ObjectZone`**: Object-specific zones

#### GC Integration Requirements

**Call Frame Integration**:
```cpp
class Rps_CallFrame {
    void gc_mark_frame(Rps_GarbageCollector* gc);
    // Extra data for local variables
    void* cfram_xtradata;
};
```

**Write Barriers**:
```cpp
#define RPS_WRITE_BARRIER() \
    /* Called after object modifications */
    _.foo.RPS_WRITE_BARRIER()
```

**Essential GC Functions**:
- **Frame Passing**: All GC functions need call frame parameter
- **Periodic Collection**: `maybe_garbcoll()` called every few milliseconds
- **Write Barriers**: Called after object updates

#### Performance Optimizations

- **Generational Collection**: Most objects die young, reducing GC overhead
- **Write Barriers**: Enable incremental collection without stopping the world
- **Thread-Local Caches**: Reduce synchronization overhead
- **Zone-based Allocation**: Fast allocation from pre-managed memory
- **Lock-Free Operations**: Atomic operations where possible

This object model provides the foundation for RefPerSys's symbolic AI capabilities, enabling flexible knowledge representation while maintaining performance and safety in a multi-threaded environment.

---

## Program Startup and Initialization

The RefPerSys startup sequence is a carefully orchestrated 9-phase process that initializes multiple subsystems in a specific order to ensure proper system operation. The main entry point is defined in `main_rps.cc` at line 1745.

### Main Function Execution Flow

#### 1. **Early Initialization Phase** (Lines 1747-1850)
- **Program Name Setup**: Store program name from `argv[0]`
- **Thread Naming**: Set main thread name to "rps--main" using `pthread_setname_np`
- **Locale Processing**: Handle `--locale` argument early and set process locale
- **Version Display**: Handle `--version` flag
- **Program Invocation**: Store full program invocation string
- **Environment Validation**: Ensure `$REFPERSYS_TOPDIR` is set
- **Source Validation**: Verify main source file references `refpersys.org`
- **System Information**: Read `/proc/version` and `/proc/self/exe`
- **Preference File Check**: Validate existence of preference file

#### 2. **Command-Line Argument Parsing** (Line 1849)
- **Early Parsing**: Parse all program arguments using `argp` library
- **Argument Storage**: Store parsed arguments in global variables
- **Mode Determination**: Determine operational mode based on flags

#### 3. **System Setup Phase** (Lines 1860-1920)
- **PID File Creation**: Write process ID to specified file if `--pid-file` given
- **TTY Detection**: Check if stdout/stderr are TTYs for terminal formatting
- **Startup Banner**: Display startup information if running interactively
- **Environment Extension**: Call `rps_extend_env()` to extend environment
- **Breakpoint Setup**: Set up potential breakpoints for debugging

#### 4. **Core Subsystem Initialization** (Lines 1924-1941)
- **QuasiZone Initialization**: Initialize memory management system
- **File Timestamp Checking**: Check modification times of files
- **CURL Initialization**: Initialize web client library (if enabled)
- **GCCJIT Initialization**: Initialize GCC Just-In-Time compilation
- **Load Directory Setup**: Determine and set the persistent state load directory

#### 5. **Persistent State Loading** (Line 1941)
- **Primary Loading**: Load persistent state from `rps_my_load_dir`
- **Root Object Initialization**: Initialize hardcoded root objects
- **Symbol Initialization**: Initialize named symbols and constants

#### 6. **Post-Load Setup** (Lines 1944-1962)
- **Directory Change**: Change to post-load directory if specified
- **Exit Handler Registration**: Register `rps_exiting()` with `atexit()`
- **User Preferences**: Parse user preferences if not already done
- **Event Loop Initialization**: Initialize event loop system (if not in batch mode)

#### 7. **Application Execution** (Line 1962)
- **Loaded Application**: Run the loaded application logic
- **Plugin Loading**: Load and initialize plugins specified by `--plugin-after-load`
- **REPL Commands**: Execute commands specified by `--command` flags

#### 8. **Mode-Specific Execution** (Lines 1964-1989)
- **Non-Batch Mode**: Initialize appropriate user interface
  - **FLTK Mode**: If FLTK is enabled, initialize and run FLTK GUI
  - **Event Loop Mode**: Run the main event loop for REPL mode
- **Post-Event Loop**: Execute cleanup and finalization code

#### 9. **Shutdown Phase** (Lines 1992-2067)
- **State Dumping**: Dump persistent state if `--dump` directory specified
- **Debug Output**: Final debug information and statistics
- **Cleanup**: Close debug files and perform final cleanup
- **Exit**: Return exit code

### Initialization Priorities

The initialization sequence prioritizes:
1. **System stability** through careful validation and error handling
2. **Flexibility** through multiple operational modes
3. **Extensibility** through the plugin system
4. **Debugging capability** through comprehensive logging and diagnostic features
5. **Performance** through multi-threaded operation and efficient memory management

Understanding this startup sequence is crucial for developers who wish to extend RefPerSys, debug issues, or integrate it into larger systems.

---

## Command-Line Interface

RefPerSys uses the **GNU argp** (argument parser) library for command-line argument handling, providing over 30 options for comprehensive system control and configuration.

### Parsing System Architecture

- **Parser Function**: `rps_parse1opt()` in `utilities_rps.cc` (line 1220)
- **Main Parser**: `rps_parse_program_arguments()` in `utilities_rps.cc` (line 1730)
- **Option Table**: `rps_progoptions[]` in `main_rps.cc` (lines 115-484)

### Essential Command-Line Options

#### Mode Selection
- **`--batch` / `-B`**: Run in batch mode without user interface
- **`--fltk[=GUI_PREFS]`**: Enable FLTK graphical interface
- **`--interface-fifo=PREFIX`**: Use FIFO-based JSONRPC interface

#### Debug and Development
- **`--debug=FLAGS`**: Set debug flags (use `--debug=help` for list)
- **`--debug-after-load=FLAGS`**: Set debug flags after successful load
- **`--debug-path=FILE`**: Output debug messages to specified file
- **`--type-info`**: Display type information and test tagged integers

#### File and Directory Options
- **`--load=DIR`**: Load persistent state from specified directory
- **`--dump=DIR`**: Dump persistent state to specified directory
- **`--chdir-before-load=DIR`**: Change directory before loading
- **`--chdir-after-load=DIR`**: Change directory after loading

#### Plugin and Code Options
- **`--plugin-after-load=PLUGIN`**: Load specified plugin after startup
- **`--plugin-arg=PLUGIN_NAME:PLUGIN_ARG`**: Pass argument to loaded plugin
- **`--cplusplus-editor-after-load=EDITOR`**: Edit C++ code with specified editor
- **`--cplusplus-flags-after-load=FLAGS`**: Set compilation flags for C++ code

#### System Options
- **`--jobs=NBJOBS`**: Set number of worker threads (default 5, min 3, max 24)
- **`--run-delay=TIME`**: Run for limited real time (e.g., "50s", "2m", "5h")
- **`--daemon`**: Run as daemon using `daemon(3)`
- **`--syslog`**: Use system logging
- **`--no-aslr`**: Disable Address Space Layout Randomization
- **`--no-terminal`**: Disable ANSI terminal escape codes

#### Information Options
- **`--version`**: Display version information and exit
- **`--full-git`**: Display full git commit ID
- **`--short-git`**: Display short git commit ID
- **`--random-oid=N`**: Print N random object identifiers
- **`--test-repl-lexer=STRING`**: Test REPL lexer on given string

### Argument Processing Flow

1. **Early Processing**: Handle `--help`, `--version`, and `--locale` before full parsing
2. **Full Parsing**: Use `argp_parse()` to process all arguments
3. **Validation**: Validate argument combinations and values
4. **Storage**: Store parsed values in global variables for later use
5. **Mode Determination**: Determine operational mode based on parsed arguments

### Debug Categories

The debug system supports multiple categories for fine-grained control:
- `CMD`: Command processing
- `CODEGEN`: Code generation
- `COMPL_REPL`: REPL completion
- `DUMP`: State dumping
- `EVENT_LOOP`: Event loop operations
- `GARBAGE_COLLECTOR`: Garbage collection
- `GUI`: Graphical interface
- `LOAD`: State loading
- `LOWREP`: Low-level REPL operations
- `MISC`: Miscellaneous operations
- `MSGSEND`: Message sending
- `PARSE`: Parsing operations
- `REPL`: Read-Eval-Print Loop

Use `--debug=help` to see all available debug flags.

---

## Operational Modes

RefPerSys supports five distinct operational modes, each optimized for different use cases and deployment scenarios.

### 1. Batch Mode (`--batch`)

**Characteristics:**
- No user interface (neither graphical nor REPL)
- Runs automated processing tasks
- Exits after completing specified operations
- Suitable for server environments and automated processing

**Typical Use Cases:**
- Data processing pipelines
- Automated testing
- Server-side operations
- Batch conversions

**Example Usage:**
```bash
./refpersys --batch --load=/data/input --dump=/data/output --plugin-after-load=process_plugin
```

### 2. FLTK GUI Mode (`--fltk`)

**Characteristics:**
- Full graphical user interface using FLTK toolkit
- Windowed environment with menus and dialogs
- Interactive object manipulation
- Real-time debugging and visualization

**Components:**
- Main application window
- Object browser and inspector
- REPL console integrated in GUI
- Debug message display
- File dialogs and preferences

**Example Usage:**
```bash
./refpersys --fltk=default.prefs
```

### 3. REPL Mode (Default Interactive Mode)

**Characteristics:**
- Command-line Read-Eval-Print Loop
- Interactive object manipulation
- Real-time expression evaluation
- Comprehensive command set

**Features:**
- Object creation and manipulation
- Class and method definition
- Code compilation and execution
- Debug and inspection commands
- File operations

**Example Usage:**
```bash
./refpersys --load=/my/project
# Interactive session starts automatically
```

### 4. JSONRPC FIFO Mode (`--interface-fifo=PREFIX`)

**Characteristics:**
- Headless operation with external GUI communication
- Uses named pipes (FIFOs) for JSON-RPC communication
- Separate GUI process communicates via structured messages
- Suitable for remote or embedded GUI applications

**Communication Protocol:**
- `PREFIX.cmd`: Commands written by GUI, read by RefPerSys
- `PREFIX.out`: Output written by RefPerSys, read by GUI
- JSON-RPC message format for structured communication

**Example Usage:**
```bash
./refpersys --interface-fifo=/tmp/rps_gui
```

### 5. Daemon Mode (`--daemon`)

**Characteristics:**
- Runs as system daemon
- Detaches from controlling terminal
- Suitable for system services
- Can be managed by system service managers

**Example Usage:**
```bash
./refpersys --daemon --syslog --load=/service/data
```

### Mode Selection and Compatibility

- **Batch mode** is exclusive - no user interface components are initialized
- **FLTK GUI mode** and **REPL mode** are mutually exclusive
- **JSONRPC FIFO mode** can combine with batch processing
- **Daemon mode** can combine with any other mode except FLTK GUI

### Performance Considerations

- **Batch mode**: Optimal for performance-critical automated processing
- **FLTK GUI mode**: Additional overhead for GUI rendering, suitable for interactive development
- **REPL mode**: Balanced between interactivity and performance
- **JSONRPC FIFO mode**: Network overhead for IPC, suitable for distributed systems
- **Daemon mode**: Minimal overhead for long-running services

---

## Core Subsystem Initialization

The core subsystem initialization follows a carefully designed sequence to ensure proper system operation and dependency management.

### Initialization Order and Dependencies

#### 1. **Rps_QuasiZone::initialize()** (Line 1924)
- **Purpose**: Initializes the memory management system
- **Dependencies**: None (first subsystem to initialize)
- **Implementation**: Sets up garbage collection infrastructure
- **Components**: Memory zones, allocation arenas, write barriers

#### 2. **rps_check_mtime_files()** (Line 1925)
- **Purpose**: Checks modification times of key files
- **Dependencies**: File system access
- **Implementation**: Validates system consistency and detects stale builds
- **Critical Files**: Source files, configuration files, plugin timestamps

#### 3. **rps_initialize_curl()** (Line 1927) [Conditional]
- **Purpose**: Initializes CURL library for web operations
- **Dependencies**: Optional - only if network functionality required
- **Implementation**: Sets up HTTP client functionality
- **Features**: SSL support, cookie handling, multi-part forms

#### 4. **rps_gccjit_initialize()** (Line 1930)
- **Purpose**: Acquires GCC JIT compilation context
- **Dependencies**: libgccjit library
- **Implementation**: Enables on-the-fly code generation
- **Capabilities**: High-quality optimized code generation

#### 5. **rps_load_from()** (Line 1941)
- **Purpose**: Loads persistent state from disk
- **Dependencies**: Memory management system (QuasiZone)
- **Implementation**: Initializes all loaded objects and their relationships
- **Process**: Manifest processing, object reconstruction, relationship restoration

#### 6. **rps_initialize_roots_after_loading()**
- **Purpose**: Initializes hardcoded root objects
- **Dependencies**: Persistent state loaded
- **Implementation**: Sets up garbage collection roots
- **Components**: System-level objects, fundamental types, root namespace

#### 7. **rps_initialize_symbols_after_loading()**
- **Purpose**: Initializes named symbols
- **Dependencies**: Root objects initialized
- **Implementation**: Sets up symbol tables and bindings
- **Features**: Global symbol resolution, constant definitions

#### 8. **rps_set_native_data_in_loader()**
- **Purpose**: Sets platform-specific data in the loader
- **Dependencies**: All core systems initialized
- **Implementation**: Configures system-dependent information
- **Data Types**: Platform constants, endianness, pointer sizes

#### 9. **rps_initialize_event_loop()** (Line 1961) [Conditional]
- **Purpose**: Initializes the event loop system
- **Dependencies**: All subsystems loaded
- **Implementation**: Sets up file descriptor monitoring
- **Features**: Signal handling, timer management, I/O multiplexing

### Plugin Initialization Sequence

The plugin system follows a separate initialization pathway:

1. **Plugin Loading**: Load shared object files with `dlopen()`
2. **Symbol Resolution**: Find `rps_init_plugin` function in each plugin
3. **Plugin Initialization**: Call each plugin's initialization function
4. **Plugin Registration**: Register plugin-provided classes, methods, and operations

### Critical Dependencies and Error Handling

#### Failure Recovery
- **Early failures**: Exit immediately with descriptive error messages
- **Plugin failures**: Continue with core functionality, log plugin errors
- **Configuration errors**: Graceful degradation where possible

#### Resource Management
- **Memory allocation**: All initialization uses GC-aware allocation
- **File handles**: Proper cleanup on initialization failure
- **Network resources**: Conditional initialization with fallback

### Performance Optimization

#### Initialization Time
- **Lazy loading**: Optional subsystems initialized on-demand
- **Parallel initialization**: Independent subsystems initialized concurrently
- **Caching**: Compiled configurations and precomputed values

#### Memory Usage
- **Zone-based allocation**: Efficient memory layout from startup
- **Minimal footprint**: Only required subsystems initialized
- **Early GC**: Garbage collection begins during initialization

---

## Development Guidelines

### Code Style and Conventions

#### C++ Standards
- **Language**: C++17 (with gnu++2c extensions)
- **Style**: GNU coding style with spaces=2 indentation
- **Memory**: Manual memory management with custom GC integration
- **Concurrency**: Thread-safe design with explicit synchronization

#### File Naming Conventions
- **Hand-written sources**: `*_rps.cc` pattern
- **Generated sources**: `_*.cc` prefix or `generated/` directory
- **Headers**: `*.hh` pattern with single main header `refpersys.hh`

### GC Integration Guidelines

#### Function Requirements
- **Allocation functions**: Must accept `callingfra` parameter
- **Write barriers**: Call `RPS_WRITE_BARRIER()` after modifications
- **Frame management**: Use `RPS_LOCALFRAME` for function entry/exit
- **Periodic collection**: Call `Rps_GarbageCollector::maybe_garbcoll`

#### Coding Patterns
```cpp
// Standard GC-aware function pattern
Rps_Value some_function(Rps_CallFrame* callingfra, /* other params */) {
    RPS_LOCALFRAME(callingfra);
    // Function body with GC operations
    return result;
}
```

### Plugin Development

#### Plugin Structure
```cpp
// Standard plugin entry point
extern "C" {
    void rps_init_plugin(Rps_PluginLoader* loader) {
        // Define classes, methods, and operations
        // Register with the plugin loader
    }
}
```

#### Compilation
- Compile as shared object (`-fPIC -shared`)
- Link against RefPerSys core and required libraries
- Use plugin-specific compiler flags

### Testing and Debugging

#### Test Categories
- **Unit tests**: Individual component testing
- **Integration tests**: System interaction testing
- **Performance tests**: Garbage collection and memory usage
- **GUI tests**: FLTK interface testing

#### Debug Techniques
- **libbacktrace**: Stack trace generation
- **GDB integration**: DWARF debugging information
- **Verbose output**: Debug flags for detailed logging
- **Memory debugging**: GC state inspection

### Contributing Guidelines

#### Before Contributing
1. Contact the development team via email
2. Understand coding conventions and GC requirements
3. Run `make clean` after any `git pull`
4. Execute `make config` after major changes

#### Development Workflow
1. **Setup**: Clone repository and configure environment
2. **Build**: Standard build with `make -j4 refpersys && make all`
3. **Test**: Run appropriate test suite
4. **Debug**: Use built-in debugging and logging facilities
5. **Submit**: Email patches or pull requests to maintainers

---

## Conclusion

RefPerSys represents a sophisticated research platform for symbolic AI and reflective programming systems. Its custom garbage collector, homoiconic design, and powerful code generation capabilities make it unique among existing rule engines and AI systems.

The project is actively developed by an international team and welcomes contributions from researchers and developers interested in advanced AI systems, garbage collection, and reflective programming paradigms.

For more detailed information, consult the README.md file, examine the source code directly, or contact the development team at basile@starynkevitch.net.
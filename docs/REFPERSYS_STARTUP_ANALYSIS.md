# RefPerSys Program Entry Point and Initialization Sequence Analysis

This document provides a detailed technical analysis of the RefPerSys program's main entry point, initialization sequence, command-line argument handling, and operational modes.

## Table of Contents

1. [Main Function Analysis](#main-function-analysis)
2. [Initialization Sequence](#initialization-sequence)
3. [Command-Line Argument Handling](#command-line-argument-handling)
4. [Main Modes of Operation](#main-modes-of-operation)
5. [Key Subsystems](#key-subsystems)

## Main Function Analysis

### Function Signature and Initial Setup

The main entry point is defined in `main_rps.cc` at line 1745:

```cpp
int main (int argc, char** argv)
```

### Step-by-Step Execution Flow

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

## Initialization Sequence

### Core Initialization Order

1. **Rps_QuasiZone::initialize()** (Line 1924)
   - Initializes the memory management system
   - Sets up garbage collection infrastructure

2. **rps_check_mtime_files()** (Line 1925)
   - Checks modification times of key files
   - Validates system consistency

3. **rps_initialize_curl()** (Line 1927) [Conditional]
   - Initializes CURL library for web operations
   - Sets up HTTP client functionality

4. **rps_gccjit_initialize()** (Line 1930)
   - Acquires GCC JIT compilation context
   - Enables on-the-fly code generation

5. **rps_load_from()** (Line 1941)
   - Loads persistent state from disk
   - Initializes all loaded objects and their relationships

6. **rps_initialize_roots_after_loading()**
   - Initializes hardcoded root objects
   - Sets up garbage collection roots

7. **rps_initialize_symbols_after_loading()**
   - Initializes named symbols
   - Sets up symbol tables and bindings

8. **rps_set_native_data_in_loader()**
   - Sets platform-specific data in the loader
   - Configures system-dependent information

9. **rps_initialize_event_loop()** (Line 1961) [Conditional]
   - Initializes the event loop system
   - Sets up file descriptor monitoring

### Plugin Initialization Sequence

1. **Plugin Loading**: Load shared object files with `dlopen()`
2. **Symbol Resolution**: Find `rps_init_plugin` function in each plugin
3. **Plugin Initialization**: Call each plugin's initialization function
4. **Plugin Registration**: Register plugin-provided classes, methods, and operations

## Command-Line Argument Handling

### Parsing System

RefPerSys uses the **GNU argp** (argument parser) library for command-line argument handling:

- **Parser Function**: `rps_parse1opt()` in `utilities_rps.cc` (line 1220)
- **Main Parser**: `rps_parse_program_arguments()` in `utilities_rps.cc` (line 1730)
- **Option Table**: `rps_progoptions[]` in `main_rps.cc` (lines 115-484)

### Key Command-Line Options

#### Mode Selection
- `--batch` / `-B`: Run in batch mode without user interface
- `--fltk[=GUI_PREFS]`: Enable FLTK graphical interface
- `--interface-fifo=PREFIX`: Use FIFO-based JSONRPC interface

#### Debug and Development
- `--debug=FLAGS`: Set debug flags (use `--debug=help` for list)
- `--debug-after-load=FLAGS`: Set debug flags after successful load
- `--debug-path=FILE`: Output debug messages to specified file
- `--type-info`: Display type information and test tagged integers

#### File and Directory Options
- `--load=DIR`: Load persistent state from specified directory
- `--dump=DIR`: Dump persistent state to specified directory
- `--chdir-before-load=DIR`: Change directory before loading
- `--chdir-after-load=DIR`: Change directory after loading

#### Plugin and Code Options
- `--plugin-after-load=PLUGIN`: Load specified plugin after startup
- `--plugin-arg=PLUGIN_NAME:PLUGIN_ARG`: Pass argument to loaded plugin
- `--cplusplus-editor-after-load=EDITOR`: Edit C++ code with specified editor
- `--cplusplus-flags-after-load=FLAGS`: Set compilation flags for C++ code

#### System Options
- `--jobs=NBJOBS`: Set number of worker threads (default 5, min 3, max 24)
- `--run-delay=TIME`: Run for limited real time (e.g., "50s", "2m", "5h")
- `--daemon`: Run as daemon using `daemon(3)`
- `--syslog`: Use system logging
- `--no-aslr`: Disable Address Space Layout Randomization
- `--no-terminal`: Disable ANSI terminal escape codes

#### Information Options
- `--version`: Display version information and exit
- `--full-git`: Display full git commit ID
- `--short-git`: Display short git commit ID
- `--random-oid=N`: Print N random object identifiers
- `--test-repl-lexer=STRING`: Test REPL lexer on given string

### Argument Processing Flow

1. **Early Processing**: Handle `--help`, `--version`, and `--locale` before full parsing
2. **Full Parsing**: Use `argp_parse()` to process all arguments
3. **Validation**: Validate argument combinations and values
4. **Storage**: Store parsed values in global variables for later use
5. **Mode Determination**: Determine operational mode based on parsed arguments

## Main Modes of Operation

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

### 5. Daemon Mode (`--daemon`)

**Characteristics:**
- Runs as system daemon
- Detaches from controlling terminal
- Suitable for system services
- Can be managed by system service managers

## Key Subsystems

### Event Loop System

**Purpose**: Central event handling and coordination
**Implementation**: `eventloop_rps.cc`
**Components**:
- File descriptor monitoring using `poll(2)`
- Signal handling via `signalfd`
- Timer management via `timerfd`
- Integration with FLTK event loop
- JSONRPC communication handling

### Agenda System

**Purpose**: Task scheduling and worker thread management
**Implementation**: `agenda_rps.cc`
**Components**:
- Priority-based task queue
- Worker thread pool (configurable size)
- Garbage collection coordination
- Process and file descriptor monitoring

### Plugin System

**Purpose**: Dynamic loading and initialization of extensions
**Implementation**: Main entry points in `main_rps.cc`
**Features**:
- Shared object loading via `dlopen()`
- Plugin initialization function calling
- Plugin argument passing
- Plugin registration and management

### REPL System

**Purpose**: Interactive command-line interface
**Implementation**: `repl_rps.cc`, `lexer_rps.cc`, `cmdrepl_rps.cc`
**Components**:
- Lexical analysis for command parsing
- Expression evaluation and execution
- Command processing and object manipulation
- Environment and variable management

### Garbage Collection System

**Purpose**: Automatic memory management
**Implementation**: `garbcoll_rps.cc`
**Features**:
- Mark-and-sweep garbage collection
- Root object tracking
- Reference counting for cyclic detection
- Integration with persistent storage

### Persistent Storage System

**Purpose**: Save and restore system state
**Implementation**: `load_rps.cc`, `dump_rps.cc`
**Features**:
- JSON-based serialization
- Manifest file management
- Object relationship preservation
- Platform-specific data handling

### Code Generation System

**Purpose**: Dynamic code compilation and execution
**Components**:
- **GCCJIT Integration**: `gccjit_rps.cc` - Just-In-Time compilation
- **GNU Lightning**: `lightgen_rps.cc` - Lightweight code generation
- **C++ Generation**: `cppgen_rps.cc` - C++ code template generation

## Development and Debugging Features

### Debug System

**Debug Categories**:
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

### Breakpoint System

**Assembly Breakpoints**: Strategic NOP instructions for debugging
**Macro Support**: `RPS_POSSIBLE_BREAKPOINT()` throughout codebase
**GDB Integration**: Special symbols for debugging and self-modification

### Logging and Diagnostics

**Multiple Output Destinations**:
- Standard error (default)
- Log files (via `--debug-path`)
- System logs (via `--syslog`)
- FLTK GUI debug display

**Diagnostic Features**:
- Backtrace generation
- Memory usage statistics
- Performance timing
- Object reference counting

## Conclusion

The RefPerSys startup sequence is a carefully orchestrated process that initializes multiple subsystems in a specific order to ensure proper system operation. The modular design allows for different operational modes while maintaining core functionality across all modes. The extensive command-line argument system provides flexibility for various use cases, from interactive development to automated processing and system service operation.

The initialization sequence prioritizes:
1. **System stability** through careful validation and error handling
2. **Flexibility** through multiple operational modes
3. **Extensibility** through the plugin system
4. **Debugging capability** through comprehensive logging and diagnostic features
5. **Performance** through multi-threaded operation and efficient memory management

Understanding this startup sequence is crucial for developers who wish to extend RefPerSys, debug issues, or integrate it into larger systems.
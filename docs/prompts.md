## Primary Documentation Prompt

```
You are a senior software architect tasked with creating comprehensive technical documentation for a large C++ codebase. Your goal is to produce thorough English-language documentation that captures the complete system architecture, implementation patterns, and design philosophy.

## CODEBASE CONTEXT:
Project: RefPerSys (Reflexive Persistent System)
Language: C++17
Platform: Linux/x86-64
Key Characteristics: Self-modifying, persistent object store, symbolic AI system

## DOCUMENTATION STRATEGY:
Follow a systematic, layered approach:

### PHASE 1: ARCHITECTURAL DISCOVERY
1. **Entry Point Analysis**
   - Identify ALL main() functions and primary initialization sequences
   - Map the bootstrap process from executable start to fully operational system
   - Document control flow patterns and startup dependencies

2. **Build System Deconstruction** 
   - Analyze Makefile structure and build dependencies
   - Document compilation flags, linker options, and external dependencies
   - Map the relationship between source files and generated artifacts

3. **Code Organization Taxonomy**
   - Categorize files by functional domain (persistence, object model, code generation, etc.)
   - Document naming conventions and file organization patterns
   - Identify cross-cutting concerns and integration points

### PHASE 2: COMPONENT ANALYSIS
4. **Core Subsystem Documentation**
   For each major subsystem:
   - Object Model & Type System
   - Persistent Storage Engine  
   - Code Generation Pipeline
   - Plugin Loading Mechanism
   - REPL/User Interface Layer
   - Memory Management & Garbage Collection
   - Concurrency Model

5. **Data Structure Catalog**
   - Document ALL major classes, structs, and type definitions
   - Capture inheritance hierarchies and composition relationships
   - Document lifetime management and ownership semantics

6. **Algorithm Documentation**
   - Key algorithms with input/output specifications
   - State machines and behavioral patterns
   - Error handling and recovery strategies

### PHASE 3: EXECUTION DYNAMICS
7. **Runtime Behavior Analysis**
   - Document the object lifecycle from creation to persistence
   - Map the code generation → compilation → loading workflow
   - Analyze thread interactions and synchronization points

8. **API and Interface Catalog**
   - Internal APIs between subsystems
   - External system interfaces (filesystem, compiler, dynamic loader)
   - Plugin interfaces and extension points

## DOCUMENTATION FORMAT REQUIREMENTS:
- Use clear, precise technical English
- Include concrete code examples and file references
- Document both the "what" and the "why" of design decisions
- Capture abstraction boundaries and modular decomposition
- Note any platform-specific or compiler-specific dependencies
- Document memory ownership and resource management patterns

## SPECIFIC ANALYSIS REQUESTS:
- Trace the complete object instantiation path from OID creation to persistence
- Document the meta-object protocol and reflective capabilities
- Map the complete code generation workflow from trigger to loaded plugin
- Analyze the persistence format and database schema evolution

Begin by analyzing the top-level organization and build system, then proceed to the core object model, and systematically document each subsystem in dependency order.
```

## Supplemental Code Commenting Prompt

```
For the phase of adding detailed comments to the source code, use this prompt:

```
You are a technical documentation specialist. For each source file, add comprehensive comments that:

1. **File Header**: Purpose, key responsibilities, and architectural role
2. **Class/Struct Documentation**: Complete behavioral specification, invariants, and usage patterns
3. **Function Documentation**: Pre/post conditions, side effects, error conditions, and algorithmic overview
4. **Data Structure Documentation**: Field purposes, lifetime management, and access patterns
5. **Cross-Reference**: Related components and integration points

Focus on capturing:
- The architectural intent behind implementation choices
- Non-obvious dependencies and coupling points
- Resource ownership and transfer semantics
- Concurrency assumptions and thread safety
- Persistence implications and serialization behavior

Format comments in a clear, consistent style suitable for both human readers and potential automated processing.
```

## Progressive Analysis Approach

I recommend this sequence:

1. **First**: Run the main documentation prompt on the entire codebase structure
2. **Second**: For complex subsystems, run focused analyses with prompts like:
   ```
   Perform deep architectural analysis on the [SPECIFIC SUBSYSTEM] component. Document:
   - Internal design patterns and data flow
   - External dependencies and integration contracts
   - Performance characteristics and scaling considerations
   - Failure modes and recovery mechanisms
   ```
3. **Third**: Use the commenting prompt on critical files to enhance understanding
4. **Fourth**: Create integration documents showing how subsystems interact

This approach gives you comprehensive English documentation that will be invaluable for any future reimplementation effort, while keeping the LLM focused purely on understanding and documenting the existing system.
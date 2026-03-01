# Implementation description

Important note: Currently AbcKit supports JS, ArkTS and static ArkTS, but **static ArkTS support is experimental**.
Compiled JS and ArkTS are stored in "dynamic" `abc` file format and static ArkTS in "static" `abc` file format.
AbcKit works with these file formats using "dynamic" and "static" runtimes.

Please take a look at [cookbook](mini_cookbook.md) beforehand to find out how API looks like from user' point of view.

1. [Two types of `abc` files](#two-types-of-abc-files)
2. [C API and C++ implementation](#c-api-and-c-implementation)
3. [Control flow by components](#control-flow-by-components)
4. [Control flow by source files](#control-flow-by-source-files)
5. [Dispatch between dynamic and static file formats](#dispatch-between-dynamic-and-static-file-formats)
6. [Data structures (context) and opaque pointers](#data-structures-context-and-opaque-pointers)
7. [Data structures (context) implementation](#data-structures-context-implementation)
8. [Includes naming conflict](#includes-naming-conflict)
9. [Bytecode<-->IR](#bytecode--ir)

## Two types of abc files

**AbcKit supports two types of `abc` files**: dynamic and static.
Depending on `abc` file type different components are used, so there are two kinds of:

1. `panda::panda_file` and `ark::panda_file`
2. `panda::abc2program` and `ark::abc2program`
3. `panda::pandasm` and `ark::pandasm`
4. IR builder: `libabckit::IrBuilderDynamic` and `ark::compiler::IrBuilder`
5. Codegen: `libabckit::CodeGenDynamic` and `libabckit::CodeGenStatic`

NOTE: `panda::` is dynamic runtime namespace, `ark::` is static runtime namespace

And only one static runtime compiler is used for AbcKit graph representation: `ark::compiler::Graph`.

## C and C++ API

AbcKits provides two kinds of API : C API and C++ API.
### C API
All public API is stored in `./include/c` folder and has such structure:

```
include/c/
├── abckit.h                  // Entry point API
├── metadata_core.h           // API for language-independent metadata inspection/transformation
├── extensions
│   ├── arkts
│   │   └── metadata_arkts.h  // API for language-specific (ArkTS and static ArkTS) metadata inspection/transformation
│   └── js
│       └── metadata_js.h     // API for language-specific (JS) metadata inspection/transformation
├── ir_core.h                 // API for language-independent graph inspection/transformation
├── isa
│   ├── isa_dynamic.h         // API for language-specific (JS and ArkTS) graph inspection/transformation
│   └── isa_static.h          // API for language-specific (static ArkTS) graph inspection/transformation (This header is now hidden)
├── statuses.h                // List of error codes
```

Abckit APIs are pure C functions, all implementations are stored in `./src/` folder and written in C++

### C++ API

C++ API is stored in `./include/cpp` folder and has such structure:

```
include/cpp/
├── abckit_cpp.h              // C++ API entry point
├── headers/
│   ├── file.h                // File operations
│   ├── graph.h               // Graph operations
│   ├── basic_block.h         // Basic block operations
│   ├── instruction.h         // Instruction operations
│   ├── literal.h             // Literal operations
│   ├── value.h               // Value operations
│   ├── type.h                // Type operations
│   ├── dynamic_isa.h         // Dynamic ISA operations
│   ├── config.h              // Configuration
│   ├── utils.h               // Utility functions
│   ├── base_classes.h        // Base classes
│   ├── base_concepts.h       // Base concepts
│   ├── core/                 // Language-independent APIs
│   ├── arkts/                // ArkTS-specific APIs
│   └── js/                   // JS-specific APIs
```

C++ API provides a higher-level, object-oriented interface.

## Control flow by components

1. When `openAbc` is called:
   1. Read `abc` file into `panda_file`
   2. Convert `panda_file` into `pandasm` using `abc2program`
2. Abckit metadata API inspects/transforms `pandasm` program
3. When `createGraphFromFunction` is called:
   1. `pandasm` program is converted back into `panda_file` (because current IR builders support only `panda_file` input)
   2. IR builder builds `ark::compiler::Graph`
4. Abckit graph API inspects/transforms `ark::compiler::Graph`
5. When `functionSetGraph` is called, `codegen` generates `pandasm` bytecode from `ark::compiler::Graph` and replaces original code of function
6. When `writeAbc` is called, transformed `pandasm` program is emitted into `abc` file

```
                                                                                ──────────────────────────────────────────────────────────────────────────────────────────────────
                                                                                |                                                                                                /\
                                                                                \/                                                                                               |
x.abc────>(ark/panda)::panda_file───>(ark/panda)::abc2program────>(ark/panda)::pandasm────>(ark/panda)::panda_file───>(ark/panda)::ir_builder────>ark::compiler::Graph──>(ark/panda::)codegen
                                                                    |            /\              |                                                   /\           |
                                                                    |             |              |                                                   |            |
                                                                    |             |              |                                                   |            |
                                                                (abckit metadata API)            |                                                   |            \/
                                                                                                 ──────────>(ark/panda)::RuntimeIface───────────────>(abckit IR API)
```

## Control flow by source files

1. C API declarations
2. C++ implementations for above APIs, in most cases implementation is just dispatch between dynamic and static runtimes
3. Runtime specific implementation:
   1. Dynamic runtime implementation
   2. Static runtime implementation
4. APIs consumes and produces pointers to opaque `AbckitXXX` structures

```

                                                  |───────────────────────────────────────────|
                                                  |     4. Data structures (context) (./src)  |
                                                  |  metadata_inspect_impl.h                  |
                                                  |  ir_impl.h                                |
                                                  |───────────────────────────────────────────|
                                                                    /\
                                                                    |
                                                                    \/
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
                                                                    /\
                                                                    |
                                                                    \/
  |──────────────────────────────────────|        |───────────────────────────────────────────|        |─────────────────────────────────────────────────────────────────|
  |     1. API declarations (./include/c)|        |     2. API implementation (./src/)        |        |     3.1 Dynamic runtime implementation (./src/adapter_dynamic/) |
  |  abckit.h                            |        |  abckit_impl.cpp                          |        |  abckit_dynamic.cpp                                             |
  |  metadata_core.h                     |        |  metadata_(inspect|modify)_impl.cpp       |        |  metadata_(inspect|modify)_dynamic.cpp                          |
  |  ir_core.h                           |        |  ir_impl.cpp                              |        |                                                                 |
  |  extensions/arkts/metadata_arkts.h   |───────>|  metadata_arkts_(inspect|modify)_impl.cpp |───────>|                                                                 |
  |  extensions/js/metadata_js.h         |        |  metadata_js_(inspect|modify)_impl.cpp    |        |                                                                 |
  |  isa/isa_dynamic.h                   |        |  isa_dynamic_impl.cpp                     |        |                                                                 |
  |  isa/isa_static.h                    |        |  isa_static_impl.cpp                      |        |                                                                 |
  |──────────────────────────────────────|        |───────────────────────────────────────────|        |─────────────────────────────────────────────────────────────────|
                                                                    |
                                                                    \/
                                        |───────────────────────────────────────────────────────────────|
                                        |     3.2 Static runtime implementation (./src/adapter_static/) |
                                        |  abckit_static.cpp                                            |
                                        |  metadata_(inspect|modify)_static.cpp                         |
                                        |  ir_static.cpp                                                |
                                        |───────────────────────────────────────────────────────────────|

```

## Dispatch between dynamic and static file formats

Lots of APIs are able to work with both dynamic and static file formats depending on source language of `abc` file.
Runtime specific implementations are stored in `./src/adapter_dynamic/` and `./src/adapter_static/`

For example here is `functionGetName()` API implementation (`./src/metadata_inspect_impl.cpp`):

```cpp
    switch (function->module->target) {
        case ABCKIT_TARGET_JS:
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionGetNameDynamic(function);
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionGetNameStatic(function);
    }
```

So depending on function's source language one of two functions is called:

- `FunctionGetNameDynamic()` from `./src/adapter_dynamic/metadata_inspect_dynamic.cpp` <-- this is implementation specific for dynamic file format
- `FunctionGetNameStatic()` from `./src/adapter_static/metadata_inspect_static.cpp` <-- this is implementation specific for static file format

## Data structures (context) and opaque pointers

Abckit C API consumes and returns pointers to opaque `AbckitXXX` structures.
User has forward declaration types for these structures (implementation is hidden from user and stored in `./src/metadata_inspect_impl.h`),
so user can only receive pointer from API and pass it to another API and can't modify it manually.

For example here is type forward declaration from `./include/c/metadata_core.h`:

```
typedef struct AbckitLiteral AbckitLiteral;
```

And here is implementation from `./src/metadata_inspect_impl.h`:

```
struct AbckitLiteral {
    AbckitFile *file;
    libabckit::pandasm_Literal* val;
};
```

## Data structures (context) implementation

### Metadata

On `openAbc` API call abckit doing next steps:

1. Open `abc` file with `panda_file`
2. Convert opened panda file into `pandasm` program with `abc2program`
3. **Greedily** traverse all `pandasm` structures and create related `AbckitXXX` structures

Implementation of all `AbckitXXX` data structures is stored in `metadata_inspect_impl.h` and `ir_core.h`.
Top level data structure is `AbckitFile`, user receives `AbckitFile*` pointer after `openAbc` call.

`AbckitXXX` metadata structures has "tree structure" which matches source program structure, for example:

1. `AbckitFile` owns verctor of `unique_ptr<AbckitCoreModule>` (each module usually corresponds to one source file)
2. `AbckitCoreModule` owns vectors of `unique_ptr<AbckitCoreNamespace>` (top level namespaces),
   `unique_ptr<AbckitCoreClass>` (top level classes), `unique_ptr<AbckitCoreFunction>` (top level functions)
3. `AbckitNamespace` owns vectors of `unique_ptr<AbckitCoreNamespace>` (namespaces nested in namespace),
   `unique_ptr<AbckitCoreClass>` (classes nested in namespace), `unique_ptr<AbckitCoreFunction>` (top level namespace functions)
4. `AbckitCoreClass` owns vector of `unique_ptr<AbckitCoreFunction>` (class methods)
5. `AbckitCoreFunction` owns vector of `unique_ptr<AbckitCoreFunction>` (for function nested in other functions)

### Graph

On `createGraphFromFunction` API call abckit doing next steps:

1. Emit `panda_file` from `pandasm` function (this is needed because currently `ir_builder` supports only `panda_file` input)
2. Build `ark::compiler::Graph` using `IrBuilder`
3. **Greedily traverse `ark::compiler::Graph` and create related `AbckitXXX` structures**

`AbckitXXX` graph structures are: `AbckitGraph`, `AbckitBasicBock`, `AbckitInst` and `AbckitIrInterface`.
For each `ark::compiler::Graph` basic block and instruction, `AbckitBasicBock` and `AbckitInst` are created.
`AbckitGraph` contains such maps:

```cpp
std::unordered_map<ark::compiler::BasicBlock *, AbckitBasicBlock *> implToBB;
std::unordered_map<ark::compiler::Inst *, AbckitInst *> implToInst;
```

So we can get from internal graph implementation related `AbckitXXX` structure.

## Includes naming conflict

**Important implementation restriction:** libabckit includes headers from both dynamic and static runtimes,
so during build clang must be provided with include paths (`-I`) to both runtimes' folders.
But there are a lot of files and folders with same names in two runtimes, it causes naming conflict for `#include`s.
Thats why no file in AbcKit includes headers from both runtimes:
- there are `./src/adapter_dynamic/` folder for files which includes dynamic runtime headers
- and `./src/adapter_static/` folder for files which includes static runtime headers

### Wrappers

But for some cases we need to work with two runtimes in single file, for example:

- When we generate `ark::compiler::Graph` from `panda::panda_file::Function`
- Or when we generate `panda::pandasm::Function` from `ark::compiler::Graph`

For such cases we are using **wrappers** (they are stored in `./src/wrappers/`).
On next picture (arrows show includes direction) you can see how:

- `metadata_inspect_dynamic.cpp` includes dynamic runtime headers and also uses static graph (via `graph_wrapper.h`)
- `graph_wrapper.cpp` includes static runtime headers and also uses dynamic `panda_file` (via `abcfile_wrapper.h`)

```
       metadata_inspect_dynamic.cpp ──>graph_wrapper.h<────|
       |                                                   |
       \/                                                  |
DynamicRuntime                                             |<───graph_wrapper.cpp──>StaticRuntime
       /\                                                  |
       |                                                   |
      abcfile_wrapper.cpp────────────>abcfile_wrapper.h<───|
```

If you follow arrows from above picture you can see that no file includes both static and dynamic runtimes' headers

## Bytecode <──> IR

abckit uses `ark::compiler::Graph` for internal graph representation,
so both dynamic and static `pandasm` bytecodes are converted into single `IR`

Bytecode <──> IR transformation approach is the same as for bytecode optimizer,
IR builder converts bytecode into IR and codegen converts IR into bytecode.

- There are two IR builders: `libabckit::IrBuilderDynamic` and `ark::compiler::IrBuilder`
- Two bytecode optimizers: `libabckit::CodeGenDynamic` and `libabckit::CodeGenStatic`
- Two runtime interfaces: `libabckit::AbckitRuntimeAdapterDynamic` and `libabckit::AbckitRuntimeAdapterStatic`

Runtime interface is part of static compiler, it is needed to abstract IR builder from it's source.
Each user of IR builder should provide it's own runtime interface (by design of compiler).

IR interface (`AbckitIrInterface`) idea is also taken from bytecode optimizer, it is needed to store relation between `panda_file` entity names and offsets.
Instance of IR interface is:

1. Created when function's bytecode converted into IR
2. Stored inside `AbckitGraph` instance
3. Used inside abckit API implementations to obtain `panda_file` entity from instruction's immediate offsets

### IR builder

For static bytecode abckit just reuses IrBuilder from static runtime.

For dynamic bytecode abckit uses fork of IrBuilder from dynamic runtime with various changes.
For dynamic bytecode almost all instructions are just converted into intrinsic calls (for example `Intrinsic.callthis0`).

### Codegen

For static bytecode abckit uses fork of bytecode optimizer codegen from static runtime with various changes.
For dynamic bytecode abckit uses fork of bytecode optimizer codegen from dynamic runtime with various changes.

### Compiler passes

There are additional graph clean up and optimization passes applied during graph creation and bytecode generation,
so if you do like this: `graph1`->`bytecode`->`graph2` without any additional changes,
`graph1` **may be different** from `graph2`!

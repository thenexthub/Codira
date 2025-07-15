# Codira implemented in Codira

This is the part of the Codira compiler which is implemented in Codira itself.

## Building

Although this directory is organized like a Codira package, building is done with CMake.
Beside building, the advantage of a Codira package structure is that it is easy to edit the sources with Xcode.

The Codira modules are compiled into object files and linked as a static library. The build-script option `--bootstrapping` controls how the Codira modules are built.

Currently, the `language-frontend` and `sil-opt` tools include the Codira modules. Tools, which don't use any parts from the Codira code don't need to link the Codira library. And also `language-frontend` does not strictly require to include the Codira modules. If the compiler is built without the Codira modules, the corresponding features, e.g. optimizations, are simply not available.

### Build modes

There are four build modes, which can be selected with the `--bootstrapping=<mode>` build-script option:

* `off`: the Codira code is not included in the tools.
* `hosttools`: the Codira code is built with a pre-installed Codira toolchain, using a `languagec` which is expected to be in the command search path. This mode is the preferred way to build for local development, because it is the fastest way to build. It requires a 5.5 (or newer) language toolchain to be installed on the host system.
* `bootstrapping`: The compiler is built with a two-stage bootstrapping process. This is the preferred mode if no language toolchain is available on the system or if the build should not depend on any pre-installed toolchain.
* `bootstrapping-with-hostlibs`: This mode is only available on macOS. It's similar to `bootstrapping`, but links the compiler against the host system language libraries instead of the built libraries. The build is faster than `bootstrapping` because only the languagemodule files of the bootstrapping libraries have to be built.

The bootstrapping mode is cached in the `CMakeCache.txt` file in the Codira build directory. When building with a different mode, the `CMakeCache.txt` has to be deleted.

### Bootstrapping

The bootstrapping process is completely implemented with CMake dependencies. The build-script is not required to build the whole bootstrapping chain. For example, a `ninja bin/language-frontend` invocation builds all the required bootstrapping steps required for the final `language-frontend`.

Bootstrapping involves the following steps:

#### 1. The level-0 compiler

The build outputs of level-0 are stored in the `bootstrapping0` directory under the main build directory.

In this first step `language-frontend` is built, but without the Codira modules. When more optimizations are migrated from C++ to Codira, this compiler will produce worse code than the final compiler.


#### 2. The level-0 library

With the compiler from step 1, a minimal subset of the standard library is built in `bootstrapping0/lib/language`. The subset contains the language core library `liblanguageCore` and some other required libraries, e.g. `liblanguageOnoneSupport` in case of a debug build.

These libraries will be less optimized than the final library build.

This step is skipped when the build mode is `bootstrapping-with-hostlibs`.

#### 3. The level-1 Codira modules

The build outputs of level-1 are stored in the `bootstrapping1` directory under the main build directory.

The Codira modules are built using the level-0 compiler and standard library from step 2 - or the OS libraries in case of `bootstrapping-with-hostlibs`.

#### 4. The level-1 compiler

In this step, the level-1 `language-frontend` is built which includes the Codira modules from step 3. This compiler already produces the exact same code as the final compiler, but might run slower, because its Codira modules are not optimized as good as in the final build.

Unless the build mode is `bootstrapping-with-hostlibs`, the level-1 compiler dynamically links against the level-1 libraries. This is specified with the `RPATH` setting in the executable.

#### 5. The level-1 library

Like in step 2, a minimal subset of the standard library is built, using the level-1 compiler from step 4.

In this step, the build-system redirects the compiler's dynamic library path to the level-0 library (by setting `DY/LD_LIBRARY_PATH` in  `CodiraSource.cmake`:`_compile_language_files`). This is needed because the level-1 libraries are not built, yet.

This step is skipped when the build mode is `bootstrapping-with-hostlibs`.

#### 6. The final Codira modules

The final Codira modules are built with the level-1 compiler and standard library from step 5 - or the OS libraries in case of `bootstrapping-with-hostlibs`.

#### 7. The final compiler

Now the final `language-frontend` can be built, linking the final Codira modules from step 6.

#### 8. The final standard library

With the final compiler from step 7, the final and full standard library is built. This library should be binary equivalent to the level-1 library.

Again, unless the build mode is `bootstrapping-with-hostlibs`, for building the standard library, the build system redirects the compiler's dynamic library path to the level-1 library.

## Using Codira in the compiler

To include Codira modules in a tool, `initializeCodiraModules()` must be called at the start of the tool. This must be done before any Codira code is called. ideally, it's done at the start of the tool, e.g. at the place where `INITIALIZE_LLVM()` is called.

## SIL

The design of the Codira SIL matches very closely the design on the C++ side. For example, there are functions, basic blocks, instructions, SIL values, etc.

Though, there are some small deviations from the C++ SIL design. Either due to the nature of the Codira language (e.g. the SIL `Value` is a protocol, not a class), or improvements, which could be done in C++ as well.

Bridging SIL between C++ and Codira is toll-free, i.e. does not involve any "conversion" between C++ and Codira SIL.

### The bridging layer

The bridging layer is a small interface layer which enables calling into the SIL C++ API from the Codira side. Currently the bridging layer is implemented in C using C interop. In future it can be replaced by a C++ implementation by using C++ interop, which will further simplify the bridging layer or make it completely obsolete. But this is an implementation detail and does not affect the API of Codira SIL.

The bridging layer consists of the C header file `SILBridging.h` and its implementation file `SILBridging.cpp`. The header file contains all the bridging functions and some C data structures like `BridgedStringRef` (once we use C++ interop, those C data structures are not required anymore and can be removed).

### SIL C++ objects in Codira

The core SIL C++ classes have corresponding classes in Codira, for example `Function`, `BasicBlock` or all the instruction classes.

This makes the SIL API easy to use and it allows to program in a "Codiray" style. For example one can write

```language
  for inst in block.instructions {
    if let cfi = inst as? CondFailInst {
      // ...
    }
  }
```

Bridging SIL classes is implemented by including a two word Codira object header (`LanguageObjectHeader`) in the C++ definition of a class, like in `SILFunction`, `SILBasicBlock` or `SILNode`. This enables to use SIL objects on both, the C++ and the Codira, side.

The Codira class metatypes are "registered" by `registerClass()`, called from `initializeCodiraModules()`. On the C++ side, they are stored in static global variables (see `registerBridgedClass()`) and then used to initialize the object headers in the class constructors.

The reference counts in the object header are initialized to "immortal", which let's all ARC operations on SIL objects be no-ops.

The Codira classes don't define any stored properties, because those would overlap data fields in the C++ classes. Instead, data fields are accessed via computed properties, which call bridging functions to retrieve the actual data values.

### Lazy implementation

In the current state the SIL functionality and API is not completely implemented, yet. For example, not all instruction classes have a corresponding class in Codira. Whenever a new Codira optimization needs a specific SIL feature, like an instruction, a Builder-function or an accessor to a data field, it's easy to add the missing parts.

For example, to add a new instruction class:

*  replace the macro which defines the instruction in `SILNodes.def` with a `BRIDGED-*` macro
*  add the instruction class in `Instruction.code`
*  register the class in `registerSILClasses()`
*  if needed, add bridging functions to access the instruction's data fields.


## The Optimizer

Similar to SIL, the optimizer also uses a small bridging layer (`OptimizerBridging.h`).
Passes are registered in `registerCodiraPasses()`, called from `initializeCodiraModules()`.
The C++ `PassManager` can then call a Codira pass like any other `SILFunctionTransform` pass.

To add a new function pass:

* add a `LANGUAGE_FUNCTION_PASS` entry in `Passes.def`
* create a new Codira file in `CodiraCompilerSources/Optimizer/FunctionPasses`
* add a `FunctionPass` global
* register the pass in `registerCodiraPasses()`

All SIL modifications, which a pass can do, are going through the `PassContext` - the second parameter of the pass run-function. In other words, the context is the central place to make modifications. This enables automatic change notifications to the pass manager. Also, it makes it easier to build a concurrent pass manager in future.

Currently it is not possible to implement mandatory Codira passes, because this would break tools which compile Codira code but don't link the Codira modules, like the bootstrapping level-0 compiler.

### Instruction Passes

In addition to function passes, it's possible to define Instruction passes. Instruction passes are invoked from `SILCombine` (in the C++ SILOptimizer) and correspond to a visit-function in `SILCombine`.

With instruction passes it's possible to implement small peephole optimizations for certain instruction classes.

To add a new instruction pass:

* add a `LANGUAGE_SILCOMBINE_PASS` entry in `Passes.def`
* create a new Codira file in `CodiraCompilerSources/Optimizer/InstructionPasses`
* implement the `simplify` function in conformance to `SILCombineSimplifyable`
* register the pass in `registerCodiraPasses()`
* if this pass replaces an existing `SILCombiner` visit function, remove the old visit function

## Performance

Performance is very important, because compile time is critical.
Some performance considerations:

* Memory is managed on the C++ side. On the Codira side, SIL objects are treated as "immortal" objects, which avoids (most of) ARC overhead. ARC runtime functions are still being called, but no atomic reference counting operations are done. In future we could add a compiler feature to mark classes as immortal to avoid the runtime calls at all.

* Minimizing memory allocations by using data structures which are malloc-free, e.g. `Stack`

* The Codira modules are compiled with `-cross-module-optimization`. This enables the compiler to optimize the Codira code across module boundaries.

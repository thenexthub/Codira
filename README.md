# Codira Programming Language Compiler

## Introduction

Codira is a omni-purpose programming language designed for all-scenario application development, balancing development efficiency and runtime performance while providing a great programming experience. 
Codira features concise and efficient syntax, multi-paradigm programming, and type safety.

This repository provides the source code for the Codira compiler, which consists of two main parts: the compiler frontend and modified open-source components. The latter includes the LLVM backend, opt optimizer, llc, ld linker, and debugger.

## Architecture

**Architecture Description**

- **Compiler Frontend**: Responsible for converting Codira source code from text to intermediate representation, including lexical, syntax, macro, and semantic analysis, ensuring code structure and semantics are correct, and preparing for backend code generation. This module depends on mingw-w64 to support Windows platform capabilities, enabling users to generate executable binaries that can call Windows APIs. It also relies on libboundscheck for safe function library access.

  - **Lexer** breaks down Codira source code into meaningful tokens.

  - **Parser** builds an Abstract Syntax Tree (AST) according to Codira grammar rules to reflect program structure.

  - **Semantic** performs type checking, type inference, and scope analysis on the AST to ensure semantic correctness.

  - **Mangler** handles symbol name mangling for Codira, and includes a demangler tool for reverse parsing.

  - **Package Management** manages and loads code modules, handles dependencies and namespace isolation, and supports multi-module collaborative development. This module uses the flatbuffer library for serialization and deserialization.

  - **Macro** handles macro expansion, processing macro definitions and calls for code generation and reuse.

  - **Condition Compile**: Conditional compilation allows compiling based on predefined or custom conditions; incremental compilation speeds up builds using previous compilation cache files.

  - **CHIR**: CHIR (Codira High Level IR) converts the AST to an intermediate representation and performs optimizations.

  - **Codegen**: Translates the intermediate representation (CHIR) to LLVM IR, preparing for target machine code (LLVM BitCode) generation.

- **Virtual Machine**: Includes the compiler backend and related toolchain. The backend receives the intermediate representation from the frontend, optimizes it, generates target platform machine code, and links it into executable files.

  - **Optimisators**: Performs various optimizations on IR, such as constant folding and loop optimization, to improve code efficiency and quality.

  - **Omni Runtime**: Converts optimized IR to target platform machine code, supporting different hardware architectures.

  - **Linker**: Links multiple object files and libraries into the final executable, resolving symbol references and generating deployable program artifacts.

  - **debugger**: Provides debugging capabilities for the Codira language.

- **OS**: The Codira compiler and LLVM toolchain currently support Windows x86-64, Linux x86-64/AArch64, and Mac x86/arm64. 

## Directory Structure

```text
compiler/
├── cmake                       # CMake scripts for build assistance
├── demangler                   # Symbol demangling
├── doc                         # Documentation
├── figures                     # Documentation images
├── include                     # Header files
├── integration_build           # Codira SDK integration build scripts
├── schema                      # FlatBuffers schema files for serialization
├── src                         # Compiler source code
│   ├── AST                     # Abstract Syntax Tree
│   ├── Basic                   # Compiler basic components
│   ├── CHIR                    # High-level Intermediate representation and optimization
│   ├── CodeGen                 # Code generation (CHIR to IR)
│   ├── ConditionalCompilation  # Conditional compilation
│   ├── Driver                  # Compiler driver (frontend/backend orchestration)
│   ├── Frontend                # Compiler instance and workflow
│   ├── FrontendTool            # Compiler instance for external tools
│   ├── IncrementalCompilation  # Incremental compilation
│   ├── Lex                     # Lexical analysis
│   ├── Macro                   # Macro expansion
│   ├── main.cpp                # Compiler entry point
│   ├── Mangle                  # Symbol mangling
│   ├── MetaTransformation      # Metaprogramming plugins
│   ├── Modules                 # Module management
│   ├── Option                  # Compiler options
│   ├── Parse                   # Syntax analysis
│   ├── Sema                    # Semantic analysis
│   └── Utils                   # Utilities
├── third_party                 # Third-party build scripts and patch files
│   ├── cmake                   # Third-party CMake scripts
│   ├── llvmPatch.diff          # LLVM backend patch (includes llvm and cjdb sources)
│   └── flatbufferPatch.diff    # Flatbuffer source patch
├── unittests                   # Unit tests
└── utils                       # Auxiliary tools
```

## Constraints

Currently, building Codira compiler artifacts directly in the Windows environment is not supported. Instead, you need to generate compiler artifacts that can run on Windows through cross-compilation in a Linux environment.

## Platform Support Roadmap

- Build Platform Evolution: Planned support for Windows Native builds of compiler artifacts in 2025 Q4.

- Compiler Runtime Platform Evolution: Planned support for running the compiler on the OHOS(PC) platform in 2026 Q2.

- Codira Application Runtime Platform Evolution: Planned support for OHOS-ARM32 core features on 2025.10.20, reflection and dynamic loading、some compiler Optimization features will support on 2025 Q4.

## Building from Source

> **Note:**
>
> This section describes how to build the Codira compiler from source. 
> If you only want to use the compiler to build Codira code or projects, skip this section and download the release package from the GitHub Releases.

### Preparation

For environment requirements and software dependencies on each platform, see the [Standalone Build Guide](doc/Standalone_Build_Guide.md).

Clone the source code:

```shell
git clone https://gitcode.com/thenexthub/Codira.git
```

### Build Steps

```shell
cd compiler
python3 build.py clean
python3 build.py build -t release
python3 build.py install
```

1. The `clean` command removes temporary files from the workspace.
2. The `build` command starts compilation. The `-t` or `--build-type` option specifies the build type: `release`, `debug`, or `relwithdebinfo`.
3. The `install` command installs the build artifacts to the `output` directory.

The `output` directory structure:

```text
./output
├── bin
│   ├── codec                      # Codira compiler executable
│   └── codec-frontend -> codec    # Codira compiler frontend executable (symlink)
├── envsetup.sh                    # Environment setup script
├── include                        # Public headers for the frontend
├── lib                            # Compiler libraries (by target platform)
├── modules                        # Standard library codeo files (by target platform)
├── runtime                        # Runtime libraries
├── third_party                    # Third-party binaries and libraries (e.g., flatbuffers)
└── tools                          # Codira tools
```

On Linux, run `source ./output/envsetup.sh` to set up the environment, then use `cjc -v` to check the compiler version and platform info:

```shell
source ./output/envsetup.sh
cjc -v
```

Example output:

```text
Codira Compiler: x.xx.xx (codenative)
Target: xxxx-xxxx-xxxx
```

### Run Unittest

Unit tests are built by default. After a successful build, run:

```shell
python3 build.py test
```

### More Build Options

For more build options, please refer to the [build.py build script](./build.py) or use the `--help` option:

```shell
python3 build.py --help
```

For more platform-specific build information, see the [Standalone Build Guide](doc/Standalone_Build_Guide.md).

## License

This project is licensed under [Apache-2.0 with Runtime Library Exception](./LICENSE). Feel free to use and contribute!


## Open Source Software Statement

| Software Name       | License                              | Usage Description                                                                                                                       | Main Component              | Usage Methods                               |
|---------------------|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|-----------------------------|---------------------------------------------|
| mingw-w64           | Zope Public License V2.1             | The Codira Windows SDK includes some static libraries from Mingw, linked with Codira-generated objects to produce Windows executables   | Compiler                    | Integrated into the Codira binary release  |                                                                                         | Compiler                    | Integrated into the Codira binary release  |
| flatbuffers         | Apache License V2.0                  | Used for serialization/deserialization of cjo files and macros                                                                          | Compiler & StdLib(std.ast)  | Integrated into the Codira binary release  |
| libboundscheck      | Mulan Permissive Software License V2 | Used for safe function implementations in the compiler and related code                                                                 | Compiler, StdLib, Extension | Integrated into the Codira binary release  |

## Contribution

We welcome contributions from developers in any form, including but not limited to code, documentation, issues, and more.
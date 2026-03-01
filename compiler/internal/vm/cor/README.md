Codira Omni Runtime (COR)
========================

COR is a low-level retargetable code generator. It translates a
[target-independent intermediate representation](docs/ir.md)
into executable machine code.

Status
------

COR currently supports enough functionality to run a wide variety
of programs, including all the functionality needed to execute
WebAssembly (MVP and various extensions like SIMD), although it needs to be
used within an external WebAssembly embedding such as Wasmtime to be part of a
complete WebAssembly implementation. It is also usable as a backend for
non-WebAssembly use cases: for example, there is an effort to build a [Rust
compiler backend] using COR.

COR is production-ready, and is used in production in several places, all
within the context of Wasmtime. It is carefully fuzzed as part of Wasmtime with
differential comparison against V8 and the executable Wasm spec, and the
register allocator is separately fuzzed with symbolic verification. There is an
active effort to formally verify COR's instruction-selection backends. We
take security seriously and have a [security policy] as a part of Bytecode
Alliance.

COR has four backends: x86-64, aarch64 (aka ARM64), s390x (aka IBM
Z) and riscv64. All backends fully support enough functionality for Wasm MVP, and
x86-64 and aarch64 fully support SIMD as well. On x86-64, COR supports
both the System V AMD64 ABI calling convention used on many platforms and the
Windows x64 calling convention. On aarch64, COR supports the standard
Linux calling convention and also has specific support for macOS (i.e., M1 /
Apple Silicon).
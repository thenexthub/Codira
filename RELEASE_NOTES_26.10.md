# Codira 26.10 release notes

Build of October 2026 (`main` at `4d27456303`). 73 commits since 13 October 2026,
including the merge commits.

This release takes Codira from a language whose front end type-checked much more
than its back end could compile to one where the main language features run end
to end, with compile-time proofs, several new targets and a faster link step.
Everything listed under the headings before "Experimental and unfinished" is on
`main` and passed the repository's five pre-commit gates (check, clippy, format,
doc tests, tests). Unfinished and experimental work is listed separately and
plainly in [Experimental and unfinished](#experimental-and-unfinished), in
[Known limitations](#known-limitations) and in [Not in this release](#not-in-this-release).

The workspace version in `Cargo.toml` is still `0.6.0-dev`; "26.10" is the
release label used by the README.

## Highlights

- **Proofs at compile time.** `requires`, `ensures`, loop `invariant` and
  `decreases`, `ghost` state and refinement types are proven by Z3 with exact
  bit-vector semantics, or rejected with a counterexample that can become a
  runnable test. `@noalloc`, `@nopanic`, `@noblock` and `@realtime` are proven
  over the whole call graph.
- **The type system caught up with the syntax.** Generics, enums, `match`,
  optionals, closures, `for` loops and units of measure all compile and run.
- **More targets.** Fourteen native targets, standalone executables, freestanding
  WebAssembly, NVIDIA GPUs (PTX), quantum circuits (QIR and OpenQASM), Verilog for
  FPGAs, and a Cranelift backend that runs a program without LLVM or a linker.
- **A purpose-built linker for Codira libraries.** `codira_image` writes
  `.codiralib` files directly instead of calling LLD: a link step that took 3.4 to
  5.9 ms in-process now takes 8 to 330 microseconds.
- **Zero-knowledge proofs of constant-time functions.** `codira attest` turns an
  `oblivious` function into a circuit and proves its result with Groth16.
- **Hot reload keeps more state.** Renamed and retyped fields migrate, and a whole
  GC heap can be saved and restored.

## Language

- **Contracts.** `requires`, `ensures` (with `result`), `invariant`, `decreases`,
  `ghost let`, and refinement types on parameters and results. Integers are
  modelled as bit-vectors of their own width, so wrapping overflow is visible to
  the proof. A refuted contract fails the build with a concrete counterexample. A
  proof that runs out of solver budget says so instead of passing.
- **Synthesis.** `??` holes and `choose x where p` are filled only with
  expressions Z3 proves against the contract (`codira synth`).
- **Secrets.** `secret`, `declassify`, `oblivious` and `ct_select`: the compiler
  rejects a branch, index or loop bound that depends on a secret, and `ct_select`
  compiles to a select instruction, not a branch.
- **Algebraic effects.** `effect`, `uses`, `perform`, `handle` and `resume` compile
  and run, with handlers that resume or abort across call frames.
- **Structured concurrency.** `scope`, `spawn` and bounded channels run tasks on a
  worker pool and join every task on every exit path.
- **Loops.** `for` over ranges (`a..b`, `a..=b`) and arrays, and `continue`. Array
  indexing is bounds-checked: an out-of-range index stops the program with its
  source location. `&&` and `||` short-circuit.
- **Optionals.** `T?`, `nil`, `if let`, `while let`, postfix `!` (panics with its
  location on `nil`), `== nil`, and the implicit conversion from `T` to `T?`.
- **`match`.** Over integers, floats, `bool`, strings, tuples, optionals and enums,
  with exhaustiveness checking by the full usefulness algorithm. The error names a
  value that is not covered.
- **Enums.** Variants with payloads, `Shape.Circle(r)` and `Shape.Point`, nested
  patterns, `if let` and `while let` on variants, `==` and `!=` for enums with
  number or `bool` payloads, and methods through `extend`. Enums work in struct
  fields, arrays and across hot reload.
- **Generics.** Generic functions, structs and classes compile and run: each
  instantiation is compiled on its own, with its own runtime type information.
- **Closures and function values.** Named functions and closure literals as values,
  in locals, fields and arrays, called with `f(x)`. Captured by value at creation;
  captured GC references stay alive. A function value never holds a code pointer,
  so a value created before a hot reload runs the new code.
- **Units of measure.** `f64 unit(m / s)` is checked at compile time and erased, so
  it costs nothing at run time.
- **Low level.** `@asm` inline assembly (one variant per architecture), machine
  memory intrinsics (stack slots, atomics, fences, non-temporal access), region
  arenas with byte quotas, OS pages with W^X protection, and fuel (`--fuel`) that
  bounds native code: every call and loop iteration costs one unit.
- **Compile-time evaluation** folds casts and narrow integers exactly as the
  machine does.

## Verification and proof tooling

- `@noalloc`, `@nopanic`, `@noblock` and `@realtime`, with the chain of calls
  behind a violation (for example: "`tick` calls `step` (line 8), which allocates:
  class `Particle` constructed at line 12"). Integer division counts as a possible
  panic unless the solver proves a non-zero divisor.
- The contract verifier now models `for` loops and `continue` soundly. Before this
  release a proof through either could be wrong.
- `@rewrite(from: ..., to: ...)` declares an equation for the optimizer; the
  translation validator proves it before it is used, and a false or unprovable
  rule fails the build.
- `codira certify` re-checks a library's contract proofs from SMT-LIB2 queries and
  code hashes with Z3 alone, without the compiler's front end. Build with
  `--certify` to produce the certificate.
- `codira api-diff` decides which public API changes are breaking, deciding
  contract changes by Z3 implication, and `--check` refuses a version bump that is
  too small.
- `codira test` and `codira query`: a refuted contract becomes a runnable test, and
  queries search by what code returns.

## Targets and backends

- **Native.** Fourteen targets, each held to the data layout LLVM actually uses
  (tested against LLVM's own layouts), `--target-cpu` with `native` resolved to a
  concrete CPU, and a `[native]` manifest table for C, C++ and assembly sources in
  a package (C is inlined into the Codira that calls it).
- **Standalone executables:** `codira build --exe`.
- **WebAssembly:** `--target wasm32-unknown-unknown` writes a module that imports
  nothing.
- **Cranelift:** `codira run --jit` runs a program without LLVM or a linker. It
  refuses constructs it does not know instead of miscompiling them; programs with
  generics, closures, optionals or enums run on LLVM.
- **GPU:** `@target(gpu)` compiles to NVPTX; a Codira vector add runs on a GPU.
- **Quantum:** `@target(qpu)` compiles to circuits; `codira quantum` builds, runs
  (with a simulator that agrees with Qiskit) and imports them. QIR and OpenQASM 2/3
  in both directions.
- **FPGA:** `@target(fpga)` writes Verilog-2001 for combinational integer
  functions, with two's-complement semantics. Checked with Yosys down to generic
  gates; no vendor flow has run it.
- **Zero knowledge:** `codira attest build|prove|verify` elaborates an `oblivious`
  function (8 to 64-bit integers and `bool`) into an R1CS circuit with exact
  integer semantics, emits iden3 `.r1cs`/`.wtns` and Circom, and proves with
  Groth16 over BN254. The setup is a single-party trusted setup, so these proofs
  are for development and testing.

## Linking

- `codira_image` writes `.codiralib` libraries directly. It is the default on
  Windows hosts; `CODIRA_DIRECT_LINK=0` or `CODIRA_LINK_MODE=in-process|subprocess`
  select LLD. ELF shared objects are supported behind `CODIRA_DIRECT_ELF=1`.
- It declines anything it does not model (unknown relocations, TLS, imports,
  `[native]` objects, debug info, symbol-table requests) and the build falls back
  to LLD with no change in behaviour.
- Measured (release build, interleaved runs): the link step alone is 8 to 330
  microseconds against 3.4 to 5.9 ms for in-process LLD. A rebuild after an edit has
  a median of about 4.2 to 5.3 ms against about 10.6 ms, and its best case of
  3.6 ms fits the 5 ms budget.
- Output files are written to a temporary file and renamed into place, so the
  runtime never reads a torn file. In debug builds every library is also linked by
  LLD and compared; 238 libraries matched byte for byte.
- A fuzz test mutates PE and ELF objects and requires the writer to decline or
  produce an image that validates (about 1.5 million mutants without a failure).

## Runtime and hot reload

- **Heap snapshots:** the whole GC heap can be written to a byte stream and
  restored in another runtime running the same program (`save_heap` and
  `restore_heap`). A layout mismatch or a type with raw pointers is refused.
- **Field migration:** fields are matched by name; `@renamed(from: "old")` keeps a
  renamed field's value; retyped numeric fields convert with saturating semantics
  (never wrapping).
- **GC:** size-class blocks without a lock or a hash map on the fast path, and a
  heap cap per runtime that collects before it gives up.
- **Reload safety:** a failed relink leaves the heap alone; filesystem events no
  longer panic the runtime; on Windows a file renamed onto a loaded library is now
  treated as a change to it.

## Interop and tooling

- `codira import-c` turns a C header into Codira declarations (read by Clang);
  `codira bindgen` writes C, Rust, Go and TypeScript bindings; Bun can import a
  `.code` file directly.
- `codira run` builds and runs a program and shows what `println` printed.
- The build daemon keeps the compiler warm; a build with nothing changed is
  answered in about 1.4 ms inside the daemon.
- The Windows console is no longer probed by spawning `cmd /C ver` on every build
  (about 21 ms per invocation).

## Bug fixes

- Float `!=` is true when an operand is NaN.
- Compile-time folding computes casts and narrow integers as the machine does.
- The contract verifier no longer accepts some proofs through `for` loops and
  `continue` (see above).
- Translation validation reports a counterexample only after running both bodies
  on it.
- A `match` the backend cannot lower is a diagnostic, not a compiler crash.
- A data-layout bug in the optimizer pass corrupted heap objects holding nested
  aggregates (for example an optional followed by a tuple): offsets were folded
  with LLVM's default layout, not the target's.
- A type variable bound to a type parameter now unifies with it.
- A fresh clone builds without the separately kept book.
- Format arguments inside log and anyhow macros are inlined, satisfying a lint
  that newer dependency versions turn on.

## Experimental and unfinished

These are on `main` but are not finished or have not been validated:

- **AArch64 MTE tagging (`--sanitize=memtag`).** Emits tagging code for memory from
  region arenas, OS pages and `stack_alloc`. It has never run on MTE hardware or
  under QEMU, so tag enforcement is unverified. The software capability mode (a
  CHERI-style model) is not finished, and a Morello target is still refused.
- **Funis**, a fork of the mold linker (MIT, by Rui Ueyama), is in the workspace as
  `crates/funis*`. The compiler does not use it yet: the in-process library mode is
  unfinished and it is not wired into the linker. Standalone, on Codira-sized links
  it measured 1.7 to 1.9 times faster than `ld.lld` (process start included, loaded
  machine); see its own repository.
- **`codira_ueval`**, scaffolding for an evaluation harness, with no measurements.
- **Z3 start-up cost.** Contract checking costs about 40 ms for a small function
  because every obligation starts a fresh solver. A faster configuration is known
  and measured but not yet applied.

## Known limitations

- **No trait or bound system.** An operation on a value of a type-parameter type
  beyond move, pass, return and store is a type error saying bounds do not exist
  yet. `enum Option[T]` (a generic enum) is a diagnostic. A generic function cannot
  be used as a value.
- **The standard library is mostly not usable from a program.** `std/` is written
  in features that only now compile, and about 2,650 semantic diagnostics remain in
  it. There is no `String`, no `async`/`await` and no `trait`.
- **Closures:** a channel cannot be captured; a closure body in verified code is
  reported as unverified; function values are not comparable.
- **Enums** cannot be printed, braced variants are rejected, and removing a variant
  makes a later `match` on a stored value of it stop with its location.
- **Hot reload has no update admission.** An update is not checked before it is
  applied, and nothing yet guarantees that no active frame belongs to a function
  being replaced. Appending enum variants is safe; reordering them changes what
  stored values mean.
- **Cranelift** does not support generics, closures, optionals or enums.
- **`@deadline(...)`** is rejected: it needs a cycle model of the target CPU.
- **Stack overflow** is not ruled out by `@nopanic` or `@realtime`.
- **Windows:** `codira.exe` loads `libz3.dll` through the normal search path; a
  different Z3 earlier on `PATH` (for example MinGW's) makes the compiler crash on
  the first contract. Keep the matching DLL next to the executable.
- **Direct linking** covers PE libraries (default) and ELF (opt-in; 89 of 93 corpus
  programs, the rest need a PLT). Mach-O and WebAssembly still use their linkers.
- **`codira attest`** supports integers up to 64 bits and `bool`, with no floats,
  arrays or heap objects; `@target(fpga)` is combinational only.

## Not in this release

Unfinished work that is kept in branches and is **not** part of 26.10:

- `select` and session types.
- Loop schedules (`schedule ... for`): the dependence check that must refuse an
  unsafe reordering failed its test, so it is not merged.
- Multi-shot continuations and `sample`/`observe`: the statistical checks against
  closed-form posteriors fail.
- Proof-gated live updates (relational prover and runtime admission protocol). A
  first draft of the accompanying paper is in `paper/proof-gated-updates/`; its
  evaluation sections are placeholders.

## Upgrading

- Windows hosts now write `.codiralib` with `codira_image`. To get LLD's output for
  any reason, set `CODIRA_DIRECT_LINK=0`.
- A `let` or parameter pattern must now be irrefutable (a literal, variant or `nil`
  pattern is a type error there; use `match` or `if let`).
- Contracts are proven with exact bit-vector semantics, so code that relied on a
  contract holding "modulo overflow" now shows new counterexamples.
- A generic function never appears in the exported function list or the dispatch
  table; calling `invoke` on a public generic function fails.

## Credits

Funis starts from [mold](https://github.com/rui314/mold) by Rui Ueyama (MIT). The
original licence and credit are kept with the Funis crates and in the Funis
repository.

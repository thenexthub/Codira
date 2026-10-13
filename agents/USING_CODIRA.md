<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: August 6, 2026

Functionality: Instructions for AI coding agents — how to USE the Codira
programming language to write projects (NOT how to build the compiler).
 -->

# Using Codira — Agent Instructions

This document teaches an AI coding agent how to **use** the Codira programming
language to write and run programs. It is *not* about building the compiler;
for compiler-internals work see `spec/LANGUAGE_SPEC.md` §12-§15 and the
`crates/**` sources.

Codira is an ahead-of-time (AOT) compiled, statically typed, hot-reloadable
systems language with an LLVM backend. It compiles to natively executable
machine code, cross-compiles, and ships as reusable libraries.

> **Ground truth** — Language details change fast. Always prefer
> `spec/LANGUAGE_SPEC.md` over this guide when they disagree. This guide is a
> friendly summary of the current (v26.8.1) surface.

---

## 1. What Codira is not

- No `null` / undefined reference value. Absence is spelled `Type?` and handled
  explicitly.
- No template angle brackets — generics use **square brackets** `[T]`.
- `;` is *optional* at the end of a line-terminated statement. The parser is
  not indentation-sensitive; it just treats `;` as always-legal.
- No borrow checker / ownership model. `struct` = value (copied on pass),
  `class` = GC-managed reference (shared mutability).
- No standalone executable by default — you produce a Codira "library"
  (`*.codiralib`) and run it via the `codira start` CLI.

---

## 2. Project layout

A Codira project is a directory holding the source plus a manifest. Example:

```
my_program/
├── src/
│   └── mod.code      # module source (must end in `.code`)
└── codira.toml       # manifest
```

- Source files always end in **`.code`**. Multi-word file names use
  underscores. One file = one module; directories nest modules.
- The manifest (`codira.toml`) declares package metadata:
  ```toml
  [package]
  name="my_program"
  authors=[]
  version="0.1.0"
  ```
- Compiling yields `target/mod.codiralib`; `codira start` invokes a chosen
  entry-point function.

### Scaffold a project

```bash
codira new my_program      # creates a new directory + project
codira init .             # initializes the current directory (or [path])
```

The generated `src/mod.code` starts as:

```
public func main() -> f64 {
    3.14159
}
```

---

## 3. Writing Codira source

### Lexical rules

Full reserved keyword list (the modern, non-Rust surface):

```
as break class comptime effect else enum extend extern false for
func handle if import in init internal let loop macro match never
nil override perform private public resume return root self static
struct super true trait uses var where while
```

The path separator is **`.`** (like Python/Swift), not `::`.

### Functions

```codira
func add(a: i32, b: i32) -> i32 {
    a + b
}
```

Visibility: `private` (default — write nothing), `internal` (same package),
`public` (exported):

```codira
public func distance_squared(a: Point, b: Point) -> f64 {
    let dx = a.x - b.x
    let dy = a.y - b.y
    dx * dx + dy * dy
}
```

Implicit return — the last expression is the return value; `return x` also works.

### Bindings

```codira
let pi = 3.14159     // immutable
var counter = 0       // mutable
counter += 1
```

`var` replaces `let mut`; there is no standalone `mut` keyword.

### Records (value) vs classes (reference)

```codira
// Value type: copied on assignment/pass, value semantics.
struct Point {
    x: f64
    y: f64
}

// Reference type: heap-allocated, garbage collected, shared mutability.
class Actor {
    var position: Point
    var health: f64
}
```

### Methods via `extend` (was `impl`)

The type body holds **fields only**; methods and `init` live in an `extend` block:

```codira
extend Actor {
    init(position: Point) {
        self.position = position
    }
    func take_damage(var self, amount: f64) {
        self.health -= amount
    }
}
```

### Enums & pattern matching

```codira
enum Shape {
    Circle(radius: f64)
    Rectangle(width: f64, height: f64)
}

func area(shape: Shape) -> f64 {
    match shape {
        Shape.Circle(radius) -> 3.14159 * radius * radius
        Shape.Rectangle(width, height) -> width * height
    }
}
```

`match` must be exhaustive (or include a `_` wildcard arm).

### Generics

```codira
struct Box[T] {
    value: T
}

func first[T](items: [T]) -> T? {
    if items.is_empty() { nil } else { items[0] }
}
```

### Optional types

```codira
func find(items: [Value], target: i32) -> i32? {
    for item in items {
        if item == target { return item }
    }
    nil
}

if let value = find(xs, 7) {
    print(value)
}
```

### Contracts and verification

Every contract is proven at compile time by Z3; a clause that cannot be
proven is a compile error with a counterexample, never a silent pass.

```codira
type Positive = i64 { v | v > 0 }        // refinement type

func div(a: i64, b: i64) -> i64
    requires b != 0                          // proven at every call site
    requires a != -9223372036854775807 - 1 || b != -1
{
    a / b
}

func half(x: Positive) -> i64
    ensures result >= 0 && result < x        // `result` = the return value
{
    x / 2
}

func sum_to(n: i64) -> i64
    requires n >= 0 && n <= 1000
    ensures result >= 0
{
    ghost let limit = n                      // verification-only binding
    var i = 0
    var s = 0
    while i < n
        invariant i >= 0 && i <= limit
        invariant s >= 0 && s <= i * 1000
        decreases n - i                      // proves the loop terminates
    {
        i += 1
        s += i
    }
    s
}
```

- Integers are checked with the semantics the compiled code has: fixed
  width, wrapping. `ensures result > x` for `x + 1` fails at `x = i64::MAX`.
- Inside `ensures`, a parameter means its value on entry.
- A function with any contract must also prove its divisions safe (no zero
  divisor, no signed `MIN / -1`).
- `ghost let` bindings and all clauses are erased before code generation;
  executable code may not read a ghost binding, and clauses may not call,
  assign or loop.
- Contract clauses support integer/bool parameters, locals, literals,
  arithmetic, comparisons, `&&`/`||`/`!`, `if`/`else` and `as`. Anything that
  mentions a struct field, array, string, float or a call is reported as
  uncheckable (an error).
- `@runtime_checked` on a function makes its `requires` clauses run on entry
  (a violation traps), so a call site the solver cannot *decide* is accepted.
  A call site it *refutes* is still an error.

### Modules & imports

```codira
import math
import collections.list
import geometry.{ Point, Vector as Vec2 }

root.tools.clamp(x, 0, 1)     // `root` = package root
super.helper()                 // parent module
```

---

## 4. Building and running

```bash
codira run                                       # build, then run `main`
codira run other_entry                           # build, then run `other_entry`
codira build                                     # compile -> target/mod.codiralib
codira start target/mod.codiralib main          # run an already-built library
```

`codira run` takes the same `--manifest-path`, `-O 0..3` and `--color`
options as `build`. If the build reports an error, nothing is run. After the
entry point returns, `run`/`start` print its value if it is a primitive
(`bool`, any integer or float type); a `()` entry point prints nothing.

Printing is built in -- no import, no `extern` declaration:

```codira
public func main() {
    let x = 5
    println("x = ", x)           // x = 5
    print("no newline ", 1.5)    // arguments are written back to back
    println()                    // just a newline
}
```

`print`/`println` accept any number of integers (any width), floats, `bool`s
and `str`s; anything else is a compile error. A program that defines its own
function named `print` calls its own. See `examples/hello`.

Useful `build` options: `-O 0..3` (default `2`), `--emit-ir`, `--watch`,
`--target <triple>` (cross-compile), `--manifest-path <path>`.

Hot reload: Codira is designed so a running application can swap function
bodies without a manual stop/restart cycle.

### Targets

`--target` accepts: `x86_64-pc-windows-msvc`, `aarch64-pc-windows-msvc`,
`x86_64-unknown-linux-gnu`, `x86_64-unknown-linux-musl`,
`aarch64-unknown-linux-gnu`, `aarch64-unknown-linux-musl`,
`riscv64gc-unknown-linux-gnu` (lp64d), `x86_64-unknown-freebsd`,
`aarch64-unknown-freebsd`, `x86_64-apple-darwin`, `aarch64-apple-darwin`,
`x86_64-apple-ios`, `aarch64-apple-ios`, `aarch64-apple-ios-sim`. Every one is
tested to produce an object of its own format and architecture, and every
non-Apple target is tested to link into a library from a Windows host; Apple
targets need the platform SDK to link. Running the output is tested on the
host only.

### Inline assembly: `@asm`

Declare a body-less `extern func` with one `@asm` variant per architecture.
Calls are replaced by the assembly itself, so a call costs exactly the
instructions written:

```codira
@asm(arch: "x86_64", "lea ($1,$2), $0", constraints: "=r,r,r", pure: true)
@asm(arch: "aarch64", "add $0, $1, $2", constraints: "=r,r,r", pure: true)
extern func add(a: i64, b: i64) -> i64;

@asm(arch: "x86_64", "rdtsc; shl $$32, %rdx; or %rdx, %rax", constraints: "={rax},~{rdx}")
@asm(arch: "aarch64", "mrs $0, cntvct_el0", constraints: "=r")
extern func cycle_counter() -> u64;
```

The template and constraints use LLVM's inline-assembly syntax: `$0`, `$1`...
name operands (outputs first), `$$` is a literal `$`, `=r` is an output, `r`
an input, `{reg}` pins a register, `~{reg}` declares a clobber, and a digit
ties an input to an output (`"=r,0"`). `dialect: "intel"` switches x86
templates to Intel syntax. `pure: true` lets the optimizer merge or drop calls
whose results are unused; without it every call is kept, in order. A variant
without `arch:` applies to every architecture. Operands are integers and
floats. The compiler rejects mismatched operand counts, unsupported operand
types and a missing variant for the target before LLVM sees the assembly.

### Machine-level memory intrinsics

Declared in an `extern "codira-intrinsic"` block and emitted in place, with
addresses as `usize`:

- `stack_alloc(size, align) -> usize`: memory in the calling function's
  frame, freed when it returns. Constant sizes get one slot in the entry block.
- `load_nt_<t>`/`store_nt_<t>`: non-temporal accesses that bypass the cache.
- `prefetch_read`, `prefetch_write`, `prefetch_read_once`.
- `fence_acquire`, `fence_release`, `fence_acq_rel`, `fence_seq_cst`,
  `compiler_fence`.
- `atomic_load_<t>`, `atomic_store_<t>`, `atomic_cas_<t>(addr, expected, new)`,
  and `atomic_<op>_<t>(addr, value)` returning the old value, for `add sub and
  or xor swap max min` (signedness from the type: `atomic_max_i64` compares
  signed). All sequentially consistent.
- `cache_line_size() -> usize`, `frame_address() -> usize`.

`<t>` is `u8`/`i8` through `u64`/`i64` (`f32`/`f64` for non-temporal and atomic
loads and stores). As with `load_*`/`store_*`, nothing checks the address.

### Quantum kernels (`codira quantum`)

A function marked `@target(qpu)` is a quantum kernel: the compiler runs its
body at compile time and turns it into a circuit. Gates come from an
`extern "codira-quantum"` block -- print the full block with
`codira quantum prelude` and paste it into the file.

```codira
extern "codira-quantum" {
    func h(q: i64);
    func cx(control: i64, target: i64);
    func measure(q: i64, c: i64) -> bool;
}

@target(qpu)
func ghz(n: i64) {
    h(0);
    let mut i = 1;
    while i < n {
        cx(i - 1, i);
        i += 1;
    }
    let mut j = 0;
    while j < n {
        measure(j, j);
        j += 1;
    }
}
```

```bash
codira quantum run ghz.code --param n=5 --shots 1000 --seed 7     # built-in simulator
codira quantum run ghz.code --param n=5 --exact                    # exact probabilities
codira quantum build ghz.code --param n=5 --emit qir -o ghz.ll     # QIR base profile (text)
codira quantum build ghz.code --param n=5 --emit qir-bc -o ghz.bc  # QIR bitcode (QDK / Azure)
codira quantum build ghz.code --param n=5 --emit qasm3             # OpenQASM 3 (also qasm2)
codira quantum import circuit.qasm                                 # Qiskit export -> .code kernel
codira quantum run circuit.qasm --shots 100                        # run a .qasm/.ll/.bc directly
```

Rules that matter when writing kernels:

- Parameters must be known at compile time: pass `--param name=value`.
- Loops need compile-time bounds (they are unrolled; a fuel limit stops
  runaway loops with an error).
- You may ignore or return a `measure` result, but never branch on it or
  compute with it: the QIR base profile has no measurement feedback, and the
  compiler rejects it with the source line.
- Kernels may call other kernels and ordinary helper functions; ordinary
  functions may not call gates.
- Kernels use integers, floats, bools, `let`/`let mut`, `if`, `loop`/`while`
  and calls. Arrays, tuples, structs and methods are not supported inside
  kernels yet.
- Don't name a variable after a gate (`x`, `t`, `s`, ...): it shadows the
  gate function.

From Python, `bindings/python/codira_quantum` converts kernels to and from
Qiskit circuits and produces QIR bitcode; see `crates/codira_quantum/README.md`
for the full surface, the profile limits, and what is verified.

### C, C++ and assembly in a package: `[native]`

```toml
[native]
sources = ["c/kernels.c", "c/table.cpp", "c/crc.S"]
include = ["c/include"]
defines = ["SCALE=3"]
flags = ["-O3"]
libs = []
```

```codira
extern "C" {
    func c_mul_add(a: i64, b: i64) -> i64;
}
public func mul_add(a: i64, b: i64) -> i64 { c_mul_add(a, b) }
```

`codira build` compiles C (`.c`) and C++ (`.cc/.cpp/.cxx`) with Clang to LLVM
bitcode and links it into each module that calls it **before** that module
is optimized, so LLVM inlines C into its Codira callers (tested: at `-O2` a C
helper leaves no call behind). Assembly (`.s`, or `.S`/`.asm` through the
preprocessor) is assembled to objects and linked. An `extern "C"` function the
native code defines is called directly, not through the runtime's dispatch
table. On Windows such an assembly links the DLL C runtime and gets a real
entry point, so C++ static constructors run.

Clang is `CODIRA_CLANG`, else the one in the LLVM installation the compiler was
built against, else `clang` on `PATH`. Outputs are cached in
`target/native/` and rebuilt when the source, a header it includes, or the
flags change. Limits: each module is its own library, so two modules that use
one C file each get their own copy of its globals; watch mode and the daemon
compile native sources only when the package is loaded; C++ exceptions must
not escape into Codira code.

### Bun

`bindings/bun` is a Bun plugin: with `preload = ["codira-bun/register"]` in
`bunfig.toml`, `import { add } from "./native/src/mod.code"` builds the
package, binds that module's `@export("C")` functions through `bun:ffi` and
returns them as named exports. Each build is loaded from a private copy named
after its hash, so `bun --hot` sees rebuilds. Underneath it uses
`codira bindgen --runtime bun --module <file> --named-exports`, and
`--print-library` to find the built library.

---

## 5. End-to-end status today

Verified through the full pipeline (parser → HIR → LLVM → link → run):

- `struct`, `class`, `func`, `let`/`var`, tuples and tuple patterns
- field access, arithmetic, comparisons, `as` casts
- `if`/`else`, `while`, `loop`/`break`, function calls, methods via `extend`,
  `Type.assoc_fn()` static calls
- arrays: literals, indexing, element assignment
- `str` literals, and `print`/`println` (`codira run` shows the output)
- imports and visibility (`public`/`internal`/`private`)

Contracts (`requires`, `ensures`, `invariant`, `decreases`, `ghost let`,
refinement types on parameters and returns) are checked statically by Z3 in
`codira check` and `codira build`; see "Contracts and verification" above for
what the checker models and what it reports as uncheckable.

Runtime (hosts embedding `codira_runtime`):

- The GC heap is a segregated size-class allocator with per-thread bump
  allocation; `Runtime::gc_collect()` collects objects not reachable from
  host-rooted handles. There are no stack maps: never collect while another
  thread runs Codira code.
- `Runtime::builder(lib).heap_limit(bytes)` caps the GC heap. An allocation
  that would exceed it first collects (scanning the allocating thread's stack
  conservatively; only while one thread allocates) and retries; if memory is
  still short the process aborts with a message naming the type and limit.
  It caps the GC heap only, not raw allocations.

Parsed but **not yet lowered** -- the compiler stops with "... is not
supported by the code generator yet" naming the construct: `match`, closures,
postfix `!`, `perform`/`handle`/`spawn`, channel operators. Not available at
all yet: `for` loops, `enum`, `trait`, `T?`/`nil` at runtime, generic
instantiation (generic code type-checks but does not build), `String`, and
using the standard library from a program.

---

## 6. Reference material (in this repository)

- `spec/LANGUAGE_SPEC.md` — the canonical, current language spec.
- `spec/HERACLES_Codira_Examples.code` — self-healing framework examples
  (`@heal(...)`, refinement types, `supervisor`).
- `book/src/**` — chapter-based tutorial (getting started, hot-reload, arrays,
  structs).
- `examples/**` — runnable Codira (and Rust, C++, C) example projects.
- `std/` — the Codira standard library source (`*.code`).

Always cross-check written code against `spec/LANGUAGE_SPEC.md` and the parser
tests in `crates/codira_syntax/src/tests.rs`.

---

## 7. Agent coding rules

1. **Use `.code` files.** Start with a single `src/mod.code`; nest modules via
   subdirectories when a package grows.
2. **Prefer explicit types** at module and interop boundaries (inference works
   but less clarity).
3. **No `null`** — design around `Type?` optionals and explicit error returns.
4. **No `::`, no `<>`** — use `.` paths and `[T]` generics on args.
5. **Semicolons** are optional; keep lines mostly semicolon-free unless two
   statements share one line.
6. **Verify** with `codira build` and `codira start` rather than guessing.
7. When this guide and the spec disagree, **obey `spec/LANGUAGE_SPEC.md`**.

Happy coding in Codira!
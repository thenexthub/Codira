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

### Loops and arrays

```codira
for i in 0..10 { }            // 0, 1, ..., 9
for i in 1..=n { }            // 1, ..., n (stops at the type's max, never wraps)
for x in xs { }               // each element of the array `xs`
for (a, b) in pairs { }       // tuple patterns destructure each element
while cond { if skip { continue } }
loop { if done { break } }

let xs = [3, 1, 4]
xs.len()                      // 3, as usize
xs[i]                         // bounds-checked: an out-of-range index panics
xs.unchecked_get(i)           // no check, for hot loops with a proven index
```

A range (`a..b`, `a..=b`) is only valid as a `for` loop's iterable for now --
there is no range value type. `continue` is a contextual keyword: it means
"next iteration" only when it stands alone (before `}`, `;` or a line break).
`&&` and `||` short-circuit.

An out-of-range index prints `panic at mod.code:7:13: index out of bounds: the
length is 3 but the index is -1` to stderr and ends the program with exit
status 101. A panic cannot be caught from Codira yet.

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

### Algebraic effects: `effect`, `uses`, `perform`, `handle`, `resume`

An effect declares operations; a function that may perform them says so
with `uses`; a `handle` expression supplies the implementation.

```codira
effect Fail {
    func fail(code: i32) -> i32
}

effect Ask {
    func ask(hint: i32) -> i32
}

func check(x: i32) uses Fail -> i32 {
    if x < 0 { perform Fail.fail(x) } else { x }
}

func compute(x: i32) uses Ask, Fail -> i32 {
    check(perform Ask.ask(x)) * 2
}

public func run(x: i32) -> i32 {
    handle {
        handle compute(x) {
            Ask.ask(h) -> resume(h + 1)      // resumes: `perform` returns h + 1
        }
    } {
        Fail.fail(code) -> code * 1000       // aborts: this is the value of the handle
    }
}
```

Rules (checked by the compiler):

- A `perform E.op(..)`, or a call to a function with `uses E`, must be inside
  a `handle` with an `E` arm or in a function that itself declares `uses E`.
  An arm is not covered by its own `handle` (it runs where the `handle` is).
- An arm `E.op(a, b) -> body` binds one name (or `_`, or a tuple pattern) per
  parameter of `op`; their types come from the effect. A `handle` implements
  every operation of each effect it names.
- Handlers are one-shot: `resume(v)` returns `v` to the `perform` and must be
  the last thing the arm does (tail position, through blocks and `if`
  branches). An arm that finishes without resuming abandons the handled
  computation -- through any number of call frames -- and its value becomes
  the value of the whole `handle`. `resume()` for an operation without a
  result.
- Innermost handler wins.
- Version-1 restrictions: an arm cannot use local variables or parameters of
  the enclosing function (no closures yet -- pass values through the
  operation's parameters), cannot `return` or `break` out of the enclosing
  function, and a function with `uses` can only be called from its own
  module and cannot be `@export`ed.
- A `uses` function can still be called from the host (`Runtime::invoke`);
  performing an effect there, with no handler, executes a trap instead of
  continuing.

Cost: evidence passing. Each `uses` effect is one hidden pointer parameter;
`perform` is one load and one indirect call; after a `perform` or a call that
passed evidence, one load and branch per handler in scope. Code that uses no
effects pays nothing.

### Structured concurrency: `scope`, `spawn`, channels

```codira
func square(jobs: Channel[i32], results: Channel[i32]) {
    let x = <-jobs              // receive: waits while the channel is empty
    results <- x * x            // send: waits while the channel is full
}

public func sum_of_squares(n: i32) -> i32 {
    scope {
        let jobs = channel(64)      // bounded; the element type is inferred
        let results = channel(64)
        let i = 0
        while i < n { spawn square(jobs, results); i += 1 }
        i = 0
        while i < n { jobs <- i; i += 1 }
        let sum = 0
        i = 0
        while i < n { sum += <-results; i += 1 }
        sum
    }                               // every task has finished here
}
```

- `scope { .. }` owns tasks: every task `spawn`ed in it has finished before
  control leaves it, by its end, `return`, `break`, or an effect handler
  abandoning the computation. `scope` is a contextual keyword (only before
  `{`; `if scope { .. }` still tests a variable).
- `spawn f(args)` runs a call to a function of the same module on the
  runtime's worker pool. It must be inside a `scope` (of the same function;
  a handler arm counts as its own function); `f` takes only integers,
  floats, `bool` and `Channel[T]` and declares no effects. Arguments are
  evaluated by the spawner; the result is discarded -- results come back
  through channels.
- `let ch = channel(capacity)` creates a bounded blocking `Channel[T]` of
  integers, floats or `bool`, owned by the enclosing `scope` and freed when
  it ends. A channel is only *used*: `ch <- v`, `<-ch`, or passed to a
  function or `spawn`. It cannot be stored, returned, copied into another
  binding or made the value of a block -- that is what keeps it from
  outliving its scope.
- Tasks share nothing but channels, so they cannot race on memory.
- The pool keeps `available_parallelism()` workers running and starts a
  replacement when one blocks on a channel or a nested scope, so blocked
  tasks cannot starve the ones that would unblock them. Mismatched sends and
  receives still block forever, as with any blocking channel.
- Output printed by a task is not captured by `capture_output` (it is
  per-thread); a task's `print` goes to standard output.
- A secret (`secret T`) cannot be sent on a channel: what a channel delivers
  is public.

#### `select`: wait on several channel operations

```codira
func collect(jobs: Channel[i64], results: Channel[i64], quit: Channel[bool]) -> i64 {
    let total = 0
    let sent = 0
    let idle = 0
    let x = 7
    select {
        v = <-jobs -> total += v,       // receive, bind the value in this arm
        <-quit -> {},                   // receive, discard the value
        results <- x -> sent += 1,      // send
        default -> idle += 1,           // optional: makes the select non-blocking
    }
    let first = select {                // a select is an expression
        a = <-jobs -> a,
        b = <-results -> b * 2,
    }
    total + first
}
```

- `select` is a contextual keyword (only before `{`; `if select { .. }`
  tests a variable). Arms are separated by commas (optional after a block)
  and use `->` like `match`. An arm's operation is `name = <-ch`, `<-ch`,
  `ch <- value` or `default`; `name` is a new binding visible only in that
  arm's body.
- Semantics (Go's): every arm's channel and sent value is evaluated first,
  top to bottom. Then the select waits until at least one operation can
  proceed (a receive: the channel has a value; a send: it has room),
  performs exactly one and runs that arm. When several are ready, one is
  chosen uniformly at random, so a ready arm is never starved by the
  others. `default` runs when none is ready, and then the select never
  waits.
- The arms' bodies have one type, which is the select's value.
- Rejected at compile time: a select with no arms (it would wait forever),
  more than one `default`, an arm that is not a channel operation, a binding
  on anything but a receive, arms of different types.
- A waiting select sleeps (no polling) and counts as blocked for the worker
  pool, like a blocked send or receive. The verifier treats the chosen arm
  as unknown and a received value as unconstrained. Cranelift (`--jit`)
  declines `select`, as it declines channels.

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

### Synthesis: `??` and `choose`

```codira
func inc(x: i64) -> i64
    requires x < 1000
    ensures result == x + 1
{
    ??                          // codira synth writes `(x + 1)` here
}

func complement(n: i64) -> i64
    requires n >= 0 && n <= 10
{
    choose y where y + n == 10  // codira synth writes `(-n + 10)`
}
```

```bash
codira synth                                  # search, prove, rewrite the source
codira synth --function inc --dry-run         # print instead of writing
codira synth --oracle "my-llm-cli --model x"  # ask an external program
```

A hole is a compile error until it is filled. `codira synth` searches small
expression shapes (constants, linear combinations of parameters, `if a < b`
with linear arms, comparisons for `bool` holes) by counterexample-guided
synthesis, and writes a candidate only after Z3 proves every contract
obligation of the function with it. With `--oracle` the command receives the
function and every rejected answer on stdin and prints one expression; each
answer is spliced in and re-verified, and only a proven one is kept
(`--attempts`, default 3). The compiler opens no network connection itself.
Holes are integer or `bool`; templates use only the function's parameters.

### Secrets and constant-time code

```codira
oblivious func ct_max(a: u64, b: u64) -> u64 {   // every value is secret
    ct_select(a > b, a, b)                         // a select, never a branch
}

@constant_time
func ct_equal(a: secret u64, b: secret u64) -> secret bool {
    let diff: secret u64 = a ^ b
    diff == 0
}

public func equal(a: u64, b: u64) -> bool {
    declassify(ct_equal(a, b))                     // the only way out
}
```

- `secret T` marks a parameter, result or `let`. A secret may not reach a
  non-secret result, parameter, `let`, field/array store or `print` without
  `declassify(..)`; values assigned under a secret `if`/`while` are secret too.
- In `@constant_time` / `oblivious` functions the compiler rejects branching on
  a secret (`if`, `while`, the left side of `&&`/`||`), indexing by a secret,
  `/` or `%` on a secret, and passing a secret to a function that is not
  constant-time. `ct_select(c, a, b)` chooses without a branch.
- What is guaranteed: those source rules, and that `ct_select` becomes one
  LLVM `select` marked `!unpredictable` with no conditional branch (checked
  in tests at `-O0`, `-O2`, `-O3`). What is not: the LLVM backend may still
  lower a `select` to a branch (`!unpredictable` asks it not to); multiplies,
  shifts and loads at public addresses are assumed constant-time; speculative
  execution is out of scope.

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

### Tests, counterexamples and semantic search

```bash
codira test                       # build src/ + tests/, run every @test
codira test parse                 # only tests whose name contains "parse"
codira check                      # also writes tests/generated/ (see below)
codira query "result < 0"         # functions that can return a negative
codira query "x > 0 && result == 0"
```

A test is a `public` function marked `@test` with no parameters, returning
`bool` (passes when `true`) or `()` (passes unless it traps or aborts). Tests
can live in `src/` or in any `.code` file under `tests/`; each runs in its own
process, so a trap or abort fails only that test.

When `codira check` refutes a contract it writes
`tests/generated/<fn>_counterexample.code`: a `@test` that calls the function
with the counterexample Z3 found (for a postcondition it returns the clause
evaluated on the real result, so it fails exactly while the bug exists). The
file is removed again once the contract is proven. `codira test` runs such
tests even while the contract is unproven -- that is what they are for.

`codira query` answers from the function *bodies* (callees summarised by
their contracts), with a witness input for each match. Values the verifier
does not model (struct fields, arrays, loop variables without an invariant)
are unconstrained, so a match there can be a value the program never
produces; a function is never missed because of that.

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
`--target <triple>` (cross-compile), `--manifest-path <path>`, and:

- `--mode dev|release` (default `release`). `dev` skips Eidos, the
  mid-level optimizer (inlining, unrolling, equality saturation and
  compile-time evaluation of whole functions), for faster edit-build
  cycles. `-O` still controls LLVM's optimizer separately.
- `--target-cpu <name>`: an LLVM CPU name for the target (`znver4`,
  `skylake`, `apple-m1`, ...) or `native` for the machine running the
  compiler. An unknown name is an error that lists the CPUs the target does
  know. `native` is resolved to the concrete host CPU and its features
  before the build starts, and is refused when cross-compiling.

Eidos's search is bounded per region by an optional `[eidos]` table in
`codira.toml`. Every bound counts work, never time, so the same sources
always build to the same program:

```toml
[eidos]
max_egraph_nodes_per_region = 10000   # hard cap on e-graph nodes (1..=1000000)
max_egraph_iterations = 8             # rewrite rounds (<= 1000)
exact_extraction_budget = 10000       # exact-extraction search steps; 0 = greedy only
```

Unknown keys are errors. `egraph_timeout_ms` is refused because it would
make the output depend on machine speed. `ilp_extractor_fallback` is
refused because there is no ILP solver; use `exact_extraction_budget`
instead.

Hot reload: Codira is designed so a running application can swap function
bodies without a manual stop/restart cycle.

### Fast development runs: `codira run --jit`

`codira run --jit` compiles the program with the Cranelift backend and runs it
in the compiler's own process: no LLVM optimization pipeline, no linker. It
covers the scalar core of the language (integers up to 64 bits, floats,
`bool`; `let`/`var` and assignment; operators and casts; `if`, `loop`,
`while`, `break`, `return`; calls; `print`/`println` of scalars and string
literals). A program that uses anything else runs on the LLVM backend instead,
with a note naming what Cranelift does not compile yet. Measured: a
three-function program compiles in about 2.2 ms (minimum of 25, in a debug
build of the compiler). `CODIRA_JIT_TIMINGS=1` prints the compile time.

### Standalone executables: `codira build --exe`

```bash
codira build --exe -o app.exe      # one file; runs with nothing else installed
./app.exe                          # `main` returning an integer sets the exit code
```

The executable is the Codira runtime (`codira-launcher`, shipped next to
`codira`) with the package's compiled libraries appended. On its first run it
unpacks them once into a temporary directory named after their hash; later
runs load them directly. `main` may return nothing (exit code 0), an integer
(the exit code) or `bool` (true is success). Host target only for now.

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

`wasm32-unknown-unknown` and `wasm32-wasip1` build `target/<module>.wasm`, a
freestanding WebAssembly module that imports nothing and exports the
module's `@export("C")` functions (and its linear memory):

```codira
@export("C")
public func add(a: i32, b: i32) -> i32 { a + b }
```

```bash
codira build --target wasm32-unknown-unknown   # -> target/mod.wasm
```

There is no Codira runtime inside a WebAssembly host, so only functions over
integers, floats and booleans that need nothing from it can be built: no
classes or other garbage-collected values, no calls into other modules, no
`extern` functions. A package that needs any of those fails the build with
the reason. Modules are tested by running them in an independent engine
(wasmi) with no imports.

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

### Importing a C header: `codira import-c`

```bash
codira import-c c/include/kernels.h -I c/include -o src/kernels.code
```

Clang reads the header exactly as a C compiler would (macros, typedefs,
includes) and `import-c` writes an `extern "C"` block for every function the
header itself declares whose types Codira can carry: integers, floats,
`_Bool`, and pointers, which become `usize` addresses. Structs and unions by
value, variadic functions, `static`/`inline` functions and `long double` are
listed in a comment with the reason instead. `--target` reads the header for
another target (`long` is 32 bits on Windows); `--cxx` reads it as C++.

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
- `if`/`else`, `while`, `loop`, `for` over ranges and arrays, `break`,
  `continue`, short-circuit `&&`/`||`, function calls, methods via `extend`,
  `Type.assoc_fn()` static calls
- arrays: literals, bounds-checked indexing and element assignment, `len()`,
  `unchecked_get(i)` (arrays of tuples type-check but do not link yet: the
  runtime has no type info for tuple element types)
- `str` literals, and `print`/`println` (`codira run` shows the output)
- imports and visibility (`public`/`internal`/`private`)
- algebraic effects and structured concurrency (see below)

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
- The runtime provides raw memory services a program reaches with
  `extern "C"` declarations (wrapped in `std/memory/region.code` and
  `std/memory/page.code`): region arenas with byte quotas and bulk reset
  (`codira_region_*`), OS pages with W^X protection, huge pages and icache
  flushing (`codira_page_*`, `codira_icache_flush`), and aligned allocation.
  Addresses are `usize`; failures return 0/false. Region memory is untraced.
- Fuel: `codira build --fuel` instruments every function entry and loop
  iteration to decrement a counter; `codira start --fuel <n>` /
  `codira run --fuel <n>` (or `RuntimeBuilder::fuel(n)`) sets the budget.
  Running out prints a message and ends the call (a `FuelExhausted` fault a
  HERACLES guard catches, with `--heal`). Measured cost: about 2-12% in loops.
  Uninstrumented code is not metered.

Algebraic effects (`effect`, `uses`, `perform`, `handle`, `resume`) with
one-shot, tail-resumptive or aborting handlers, nested handlers, and aborts
across call frames; see "Algebraic effects" above for the version-1
restrictions (no captures in arms, `uses` functions callable only within
their module).

Structured concurrency (`scope`, `spawn`, `channel(n)`, `ch <- v`, `<-ch`)
on a worker pool; see "Structured concurrency" above for what may cross
between tasks (scalars and channels only), and `select` over channel
operations (fair, with an optional `default`).

Parsed but **not yet lowered** -- the compiler stops with "... is not
supported by the code generator yet" naming the construct: `match`, closures,
postfix `!`, postfix `^`. Not available at
all yet: `enum`, `trait`, `T?`/`nil` at runtime, generic
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
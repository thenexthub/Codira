<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: October 14, 2026

Functionality: Codira Programming Language
-->

# Codira Programming Language
### Stable Version: 26.10
### Build: October 14, 2026.

_Codira_ is an ahead-of-time compiled, statically typed language for high
performance systems, built around three ideas: the compiler proves what it
can about your code, a running program can be changed without restarting
it, and one source can target CPUs, GPUs, quantum processors and more.

## Features

- **Proven contracts** - `requires`, `ensures`, loop `invariant`/`decreases`,
  `ghost` state and refinement types are discharged by Z3 at compile time.
  A proven clause costs nothing at run time; a refuted one fails the build
  with a counterexample, which `codira test` can turn into a runnable test.

- **Synthesis** - `??` holes and `choose x where p` are filled only with
  expressions Z3 proves against the contract (`codira synth`).

- **Constant-time code** - `secret` values and `oblivious` functions: the
  compiler rejects any branch, index or loop bound that depends on a secret,
  and `ct_select` compiles to a select, never a branch.

- **Allocation-, panic- and block-free code** - `@noalloc`, `@nopanic`,
  `@noblock` and `@realtime` are proven over the whole call graph; a broken
  promise is reported with the chain of calls behind it.

- **Units of measure** - `f64 unit(m / s)` is checked at compile time and
  erased, so it costs nothing at run time.

- **Algebraic effects and structured concurrency** - `effect`, `perform`,
  `handle`, `resume`; `scope`, `spawn` and bounded channels on a worker pool,
  with every task joined before its scope ends.

- **First class hot reloading** - change code and data layouts of a live
  program. Objects survive layout changes (renamed fields with
  `@renamed(from:)`, retyped fields with defined conversions), and the whole
  GC heap can be saved and restored into another runtime.

- **A verifying optimizer** - Eidos optimizes with equality saturation and
  checks its transformations by translation validation; library authors can
  ship `@rewrite` rules that are proven before they are used.

- **Many targets from one source** - fourteen native targets held to LLVM's
  data layouts, standalone executables (`codira build --exe`), freestanding
  WebAssembly, NVIDIA GPUs (`@target(gpu)`, PTX), quantum kernels
  (`@target(qpu)`, QIR and OpenQASM), inline assembly (`@asm`), and a
  Cranelift backend that runs a program without LLVM or a linker
  (`codira run --jit`).

- **Interop** - C, C++ and assembly sources in a package, with C inlined into
  the Codira that calls it; `codira import-c` reads C headers through Clang;
  `codira bindgen` writes C, Rust, Go and TypeScript bindings; Bun imports a
  `.code` file directly.

- **Bounded execution** - fuel meters native code (`--fuel`), a runtime can
  cap its GC heap, and region arenas have byte quotas.

- **Tooling** - a language server, a build daemon that keeps the compiler
  warm, `codira test`, `codira query` (search by what code returns) and
  `codira quantum`.

What runs end to end today, and what is parsed but not yet compiled, is
listed precisely in [`agents/USING_CODIRA.md`](agents/USING_CODIRA.md)
("End-to-end status today"). Enums, generic instantiation, closures,
`select`, session types, proof-carrying artifacts, zero-knowledge circuits,
Verilog output and experimental hardware memory safety (ARM MTE, a
CHERI-style capability mode) are in progress.

## Example

<!-- inline HTML is intentionally used to add the id. This allows retrieval of the HTML -->
<pre language="codira">
<code id="code-sample">// A contract the compiler proves with Z3: no runtime check is emitted.
func clamp(x: i64, lo: i64, hi: i64) -&gt; i64
    requires lo &lt;= hi
    ensures lo &lt;= result &amp;&amp; result &lt;= hi
{
    if x &lt; lo { lo } else if x &gt; hi { hi } else { x }
}

// Units of measure are checked at compile time and erased.
type Speed = f64 unit(m / s);

func speed(d: f64 unit(m), t: f64 unit(s)) -&gt; Speed {
    d / t
}

// Structured concurrency: every spawned task is joined before `scope` ends.
func square(jobs: Channel[i64], results: Channel[i64]) {
    let x = &lt;-jobs
    results &lt;- x * x
}

public func sum_of_squares(n: i64) -&gt; i64 {
    scope {
        let jobs = channel(64)
        let results = channel(64)
        for _ in 0..n { spawn square(jobs, results) }
        for i in 0..n { jobs &lt;- i }
        var sum = 0
        for _ in 0..n { sum += &lt;-results }
        sum
    }
}

// Proven allocation-, panic- and block-free, with bounded loops.
@realtime
func triangle(n: i64) -&gt; i64 {
    var total = 0
    for i in 0..n { total += i }
    total
}

public func main() -&gt; i64 {
    let v = speed(100.0, 8.0)
    clamp(sum_of_squares(10) + triangle(4), 0, 1000)
}</code>
</pre>


## Building from Source

Make sure you have the following dependencies installed on your machine:

* Rust (the version pinned in `rust-toolchain`; `rustup` picks it up
  automatically)
* LLVM 22.1 (a full dev distribution with `llvm-config` and the LLD static
  libraries). Codira reaches it through `llvm-sys` 221, so point
  `LLVM_SYS_221_PREFIX` at its install root, the directory whose `bin` holds
  `llvm-config`:
  * Debian/Ubuntu: `sudo bash scripts/install-llvm.sh` installs LLVM 22 from
    apt.llvm.org; then `export LLVM_SYS_221_PREFIX=/usr/lib/llvm-22`.
  * macOS: `brew install llvm@22`; then
    `export LLVM_SYS_221_PREFIX="$(brew --prefix llvm@22)"`.
  * Windows: extract the LLVM project's
    `clang+llvm-22.1.8-x86_64-pc-windows-msvc.tar.xz` release archive (for
    example to `C:\llvm22`) and set `LLVM_SYS_221_PREFIX` to that directory,
    or build LLVM 22.1 with MLIR and LLD from source with
    `pwsh build-llvm/build.ps1 -InstallDir C:\llvm22\built`.
* Z3 (`libz3` -- used by `codira_smt` for refinement-type checking and
  translation validation). Set `Z3_LIB_DIR` to the directory containing
  `libz3.so` (`libz3.lib` on Windows), or `Z3_ROOT` to a Z3 install prefix.
  On Debian/Ubuntu, `sudo apt install libz3-dev` puts it in the multiarch
  library directory, e.g. `/usr/lib/x86_64-linux-gnu`. On Windows,
  `choco install z3` needs neither variable: Chocolatey's install location is
  searched by default.

Clone the source code:

```bash
git clone https://github.com/theomnira/codira.git
cd codira
```

The doc tests in `crates/codira/tests/book.rs` are generated from the mdbook
in `book/`, which is not part of the repository. Without it the build prints a
warning and skips them.

Use `cargo` to build a release version

```bash
cargo build --release
```

## Language server

Codira contains support for the lsp protocol, start the executable using:

```bash
codira language-server
```

Alternatively, you can install editor-specific extensions.

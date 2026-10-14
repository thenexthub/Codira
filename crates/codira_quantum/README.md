<!--
Copyright (c) 2026 Omnira CJSC
Author: Tunjay Akbarli
Date: October 13, 2026

Functionality: Design, surface, profile limits and verification status of
Codira's quantum support (`codira_quantum`, `codira quantum`, and the
Python binding in bindings/python/codira_quantum).
-->

# Quantum kernels in Codira

`codira_quantum` turns Codira functions marked `@target(qpu)` into quantum
circuits, writes those circuits as **QIR** (Base Profile, LLVM IR text and
bitcode) and **OpenQASM 2.0 / 3.0**, reads OpenQASM and QIR back in, and runs
circuits on a built-in state-vector simulator. A small Python package moves
circuits between Codira, **Qiskit**, and the **QDK / Azure Quantum** (which
take QIR).

```
 .code  ──elaborate──▶  Circuit IR  ──▶ QIR text / bitcode ──▶ QDK, Azure Quantum, qir-runner, pyqir
 (@target(qpu))            │  ▲      ──▶ OpenQASM 2 / 3     ──▶ Qiskit, QDK (openqasm)
                           │  └──────── OpenQASM 2 / 3, QIR  ◀── Qiskit qasm2/qasm3.dumps, Q# QIR
                           └──▶ state-vector simulator (counts, exact probabilities)
```

## 1. The surface

No new syntax. A kernel is an ordinary function with the attribute
`@target(qpu)` (spec section 14 already parses it). The gates are declared
once per program in an `extern "codira-quantum"` block; `codira quantum
prelude` prints the full block (it is `elaborate::PRELUDE`).

```codira
extern "codira-quantum" {
    func h(q: i64);
    func cx(control: i64, target: i64);
    func cp(lambda: f64, control: i64, target: i64);
    func measure(q: i64, c: i64) -> bool;
    func pi() -> f64;
    // ... the full list: `codira quantum prelude`
}

func pow2(e: i64) -> i64 { 1 << e }

@target(qpu)
func qft(n: i64) {
    let mut j = n - 1;
    while j >= 0 {
        h(j);
        let mut k = j - 1;
        while k >= 0 {
            cp(pi() / (pow2(j - k) as f64), k, j);
            k = k - 1;
        }
        j = j - 1;
    }
    let mut i = 0;
    while i < n / 2 { swap(i, n - 1 - i); i += 1; }
}
```

Gates: `id h x y z s sdg t tdg sx rx ry rz p u` (one qubit; angles first),
`cx cy cz ch swap cp crx cry crz` (two; control first), `ccx cswap` (three),
plus `measure(q, c)`, `reset(q)`, `barrier()`, `pi()`, and
`alloc_qubits(n)` / `alloc_bits(n)` to force a register size (registers
otherwise grow to the highest index used). Qubits and classical bits are
plain `i64` indices into one register each (`q`, `c`).

**Why an `extern` block and not a std module.** The HIR already lowers
`extern "..." { .. }` blocks into ordinary, name-resolved, type-checked
function items -- that is how `extern "codira-intrinsic"` works -- so `h(0.5)`
is a type error from the normal checker before elaboration runs, and
`codira_hir` needed no change to recognise the gates: the block's ABI string
is read off the syntax tree by `elaborate::is_quantum_intrinsic`. A std
module would need cross-package import of a module whose functions have no
bodies to compile, which is more machinery for the same declarations.

### Elaboration (compile time)

`elaborate::elaborate` interprets the kernel's HIR body
(`codira_hir::Body`, the same `Expr`/`Statement`/`Pat` + inference result
that `codira_hir::mir_lower` reads) with every value known:

- `i64`/`f64`/`bool` literals, parameters and locals; `let`, `let mut`,
  `=` and compound assignment;
- unary and binary operators -- **checked**: overflow, division by zero and
  out-of-range shifts are errors, never wrapped values;
- `as` casts between integers, floats and bools;
- `if`/`else` on compile-time conditions, `loop`/`while` with
  `break`/`return`, unrolled under a deterministic fuel budget (loop
  iterations + calls; default 10,000,000, `--fuel` on the CLI) and a call
  depth limit (256) -- no wall clock anywhere;
- calls to other `@target(qpu)` functions (which emit gates) and to ordinary
  Codira functions (which compute values but may not emit gates).

Kernel parameters are compile-time values: `--param n=5` on the CLI,
parsed by the parameter's declared type. A missing parameter is an error
naming the flag to pass.

**Runtime-dependent control flow is rejected.** `measure` returns a value
that is only known when the circuit runs. It may be ignored or returned;
branching on it, computing with it, comparing it or passing it to a call is
an error that names the qubit and points at the source line:

```
k.code:44:8: in `feedback`: this `if` condition depends on the measurement of qubit 0
(into bit 0), which is only known at run time; the QIR base profile has no measurement
feedback, so kernels cannot branch on, compute with, or pass on measurement results
```

Not supported inside kernels yet: arrays, tuples, structs, methods, strings
(each is a clear error, not a miscompile).

## 2. Targets and their limits

### QIR Base Profile (`--emit qir`, `--emit qir-bc`)

Written to the QIR Alliance Base Profile specification
(`qir-alliance/qir-spec`, `specification/profiles/Base_Profile.md`), checked
line by line against the spec text:

- entry point `define i64 @<kernel>() #0` with four blocks: `entry`
  (`__quantum__rt__initialize(ptr null)`), `body` (reversible QIS calls only),
  `measurements` (`irreversible` calls only), `output` (recording only, then
  `ret i64 0`), joined by unconditional `br`;
- qubits and results as `inttoptr (i64 N to ptr)` constants (opaque pointers,
  LLVM 22); `__quantum__qis__*__body`/`__adj` declarations returning `void`;
  `mz` carries `"irreversible"` and a `writeonly` result parameter;
- output: one `__quantum__rt__array_record_output` per classical register
  (inside a `__quantum__rt__tuple_record_output` when there are several),
  then `__quantum__rt__result_record_output` per bit, each with its own
  label constant (Q#'s scheme: `0_a`, `1_a0r`, ...); array order is result
  index order, i.e. bit 0 first;
- entry attributes `"entry_point"`, `"qir_profiles"="base_profile"`,
  `"output_labeling_schema"="codira_v1"`, `"required_num_qubits"`,
  `"required_num_results"`;
- module flags `qir_major_version` (behavior Error), `qir_minor_version`
  (behavior Max -- written as raw metadata because inkwell's `FlagBehavior`
  has no Max), `dynamic_qubit_management` and `dynamic_result_management`
  (Error, `i1 false`).

QIS names are the ones the QDK (`qsharp` 1.31) emits: `h x y z s s__adj t
t__adj rx ry rz cx cy cz ccx swap mz`. Everything else is decomposed (`sx` as
`h s h`, `u` as `rz ry rz`, `ch` as Q#'s own decomposition, controlled
rotations as `R(a/2) cx R(-a/2) cx`, `cswap` as `cx ccx cx`); each
decomposition is compared with the gate's unitary in the tests.

Every module is run through the LLVM verifier, and the returned text and
bitcode are each parsed back by LLVM 22 and verified again before they are
handed out.

**Base-profile limits** (enforced by `Circuit::check_base_profile`, each with
its own error): no operation on a qubit after it has been measured (no
mid-circuit measurement, no measurement feedback); every classical bit is
measured exactly once; `reset` only before a qubit's first use (then it is a
no-op and dropped); barriers are dropped. The QIR major version defaults to 2
-- what the current spec assigns to opaque-pointer QIR. Q# 1.x tooling still
labels its own typed-pointer output 1; `--qir-major 1` writes that number for
consumers that insist on it (the pointers stay opaque: LLVM 22 cannot write
typed pointers).

### OpenQASM 2.0 / 3.0 (`--emit qasm2`, `--emit qasm3`)

QASM 2 uses Qiskit's `qelib1.inc` names (`u3`, `u1`, `cu1`, plus `sx`,
`swap`, `cswap`, `crx`, `cry` which Qiskit's `qelib1.inc` adds); QASM 3 uses
`stdgates.inc` and the builtin `U`. Angles are printed with shortest
round-trip formatting, so import(emit(c)) == c bit for bit (tested).

### Importers

- **OpenQASM 2/3** (`qasm::import`): register declarations in both
  spellings, broadcasting over whole registers, `measure` in both spellings,
  `reset`, `barrier`, `gphase` (dropped), parameter expressions (`pi`, `tau`,
  `+ - * / ^ **`, `sin cos tan exp ln sqrt ...`), and user `gate`
  definitions, expanded at each call. A built-in library defines the Qiskit
  gates outside the IR's set (`rzz rxx ryy rzx cu3 cu csx cs csdg ccz sxdg
  iswap ecr dcx r u0`), each checked against an independently written
  matrix. Refused, with the line number: `if`, loops, `def`, `input`, gate
  modifiers (`ctrl @` ...), `opaque`, physical qubits `$n`, unknown includes.
- **QIR** (`qir::import_qir_text`, `qir::import_qir_bitcode`): reads this
  crate's output and Q#'s typed-pointer QIR 1.0 (a verbatim `qsharp` 1.31
  module is a test fixture; LLVM 22's parser no longer reads typed pointers,
  so the reader is textual). Refuses conditional branches and dynamic qubit
  allocation.
- `codira quantum import file.qasm` writes a `.code` file: the prelude plus
  one `@target(qpu)` kernel replaying the circuit with flat indices (the
  original register layout is kept in a comment).

## 3. The simulator

`sim` holds `2^n` complex `f64` amplitudes, up to 26 qubits (1 GiB). Qubit
`k` is bit `k` of a basis index, and outcomes are written the way Qiskit's
`get_counts()` writes them (highest bit first, last register leftmost), so
results compare with Qiskit directly. Sampling uses xoshiro256** seeded
through SplitMix64: a given `(circuit, shots, seed)` gives the same counts on
every platform. Measurement-terminal circuits are sampled from one final
state; others (mid-circuit measurement, reset) are simulated per shot from a
shared prefix, collapsing on each measurement. `--exact` prints the exact
outcome distribution instead of samples.

## 4. CLI

```
codira quantum build <entry> [--emit qir|qir-bc|qasm2|qasm3] [-o file] [--param n=v]... [--kernel k] [--qir-major 2]
codira quantum run   <entry> [--shots 1024] [--seed 0] [--exact] [--json] [--param n=v]...
codira quantum import <file.qasm|file.ll|file.bc> [-o out.code] [--name kernel]
codira quantum prelude
```

`<entry>` is a `.code` file (`file.code::kernel` picks a kernel; with
several kernels and no choice, `main` is used) or a `.qasm`/`.ll`/`.bc`
circuit file, which is imported and used directly.

## 5. Python: Qiskit and QDK

`bindings/python/codira_quantum` (pure Python, drives the `codira` CLI):
`to_qiskit(entry, params=...)` returns a `qiskit.QuantumCircuit` (through
OpenQASM 2 by default; `qasm_version=3` needs `qiskit-qasm3-import`),
`from_qiskit(circuit)` writes a `.code` kernel (through `qasm2.dumps`),
`to_qir(entry)` returns base-profile bitcode bytes, plus `to_qasm`, `run`
and `probabilities`.

## 6. What is verified, and how

Rust (`cargo test -p codira_quantum`):

- simulator against hand-derived values: single-gate amplitudes, unitarity of
  every gate matrix, control ordering, Toffoli truth table, Bell and 5-qubit
  GHZ amplitudes, teleportation (exact, and sampled through mid-circuit
  measurement), Deutsch–Jozsa (constant vs balanced), Grover on 2 qubits
  (certainty) and 3 qubits (25/32 and 121/128 from sin²((2k+1)θ)), the QFT
  entry by entry against its definition, QFT round trip on every basis state,
  seed determinism;
- QASM: exact round trip in both dialects; verbatim Qiskit 2.5.2
  `qasm2.dumps` / `qasm3.dumps` output imports and both agree; library gates
  against matrices; every rejection carries its line;
- QIR: every piece the Base Profile requires is present; only `call`/`br`/
  `ret` in the entry point; every decomposition equals its gate up to global
  phase; text and bitcode round trips preserve the exact distribution; Q#'s
  QIR imports;
- elaboration over real Codira source parsed and type-checked by
  `codira_hir`: Bell, GHZ(n) from a parameter, the QFT written in Codira
  (helper function, cast, `pi()`, nested loops) against its definition,
  Grover with kernel-to-kernel calls against the formula, float/bool
  parameters, and every promised diagnostic (measurement feedback with its
  line, missing/invalid/unknown parameters, fuel, recursion depth, gates
  outside kernels, bad indices, overflow, `extern "C"` calls), and imported
  circuits elaborating back to themselves.

Against the real libraries (`bindings/python/codira_quantum/tests`,
`python -m unittest discover -s tests`), run in throwaway virtualenvs on
**Python 3.12 and Python 3.14** with qiskit 2.5.2, qiskit-qasm3-import,
qsharp 1.31.0, pyqir 0.12.6 and qirrunner 0.9.7 -- all 10 pass on both:

- **Qiskit, exact**: Bell, GHZ(5) and a kernel using every Codira gate,
  through `to_qiskit` (OpenQASM 2 and OpenQASM 3), have exactly the outcome
  distribution Qiskit's `Statevector` gives (1e-9); a 5-qubit, two-register
  Qiskit circuit using `rzz rxx ryy rzx cu csx sxdg ccz iswap ecr dcx cs
  csdg r u p cp crx cry crz ch cswap ccx` and a decomposed `QFTGate`, through
  `from_qiskit`, have exactly Qiskit's distribution in Codira;
- **Qiskit, sampled**: 20,000 seeded shots from Codira's simulator fall
  within 5 sigma of Qiskit's exact probabilities on every outcome, and the
  same seed gives the same counts;
- **pyqir**: Codira's bitcode parses, verifies, has one entry point with
  `required_num_qubits`/`required_num_results`, `qir_profiles=base_profile`,
  all four module flags and the expected declarations;
- **qir-runner**: executes Codira's bitcode -- a deterministic kernel gives
  the predicted bits on every shot, Bell shots are always correlated with
  both outcomes present, and 4,000 shots of the all-gates kernel fall within
  5 sigma of Codira's exact distribution;
- **QDK (`qsharp`)**: the QDK's own OpenQASM 3 compiler compiles Codira's
  OpenQASM 3 to (typed-pointer) QIR, Codira imports that QIR, and the result
  has exactly the distribution of the original kernel; the QDK simulator
  sees the Bell correlation.

**Not verified:** submission to Azure Quantum (needs credentials; no Azure
target has run this bitcode), anything on hardware, and whether consumers
that only read QIR 1.0 typed-pointer bitcode (older LLVM) accept LLVM 22's
opaque-pointer bitcode -- `--qir-major 1` changes the version flag, not the
pointer representation.

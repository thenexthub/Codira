# Codira research agenda: proof-carrying performance, physics and updates

Status: working document, 2026-10-19. This is a plan for what to invent, not a
description of what exists. Each program says what is already built, what is
prior art (so we claim only what is new), the first experiment that could show
it does not work, and what would make us stop.

## The thesis

Languages have optimised for what a program *means* and, separately, for how
fast a compiler can make it run. Codira already has an unusual asset: a compiler
that proves things with an SMT solver and re-checks them (contracts, effect
summaries, translation validation, certificates), a runtime that can replace
code while it runs, and back ends for substrates that normally need separate
languages (CPU, GPU, FPGA, zero-knowledge circuits, quantum circuits).

The agenda is one idea applied six ways: **a program should carry checkable
evidence of everything that matters about it, and the system that runs it should
re-check that evidence itself.** Meaning, speed, energy, equivalence across
hardware, and fitness to be updated are all claims; today most of them are
assertions in a benchmark chart or a code review. We want them to be artefacts
that can be wrong in a way a machine detects.

What "ahead of its time" can honestly mean: not a prediction of 2076, but a set
of bets chosen because they are limited today by tooling and discipline rather
than by physics or mathematics, each with an experiment that can fail soon.

## What already exists (the platform)

On `main` as of 26.10: contracts proven by Z3 with exact bit-vector semantics;
effect summaries (`@noalloc`, `@nopanic`, `@noblock`, `@realtime`); `codira
certify` (proofs re-checked with Z3 alone); `codira api-diff`; translation
validation of optimizer rewrites and proven `@rewrite` rules; an e-graph
optimizer (Eidos); units of measure; `oblivious` functions lowered to
constant-time code, to zero-knowledge circuits (`codira attest`) and to Verilog;
quantum circuits (`@target(qpu)`, simulator, QIR, OpenQASM); GPU (PTX); hot
reload with typed heap migration and snapshots; a direct image writer.
Unfinished: the relational prover and runtime admission for updates (the paper),
SIMD and the performance tooling (in progress), traits and a usable standard
library (in progress).

## P1. Performance as a checked contract

**Idea.** Attach a *performance contract* to a kernel and have the compiler
prove or measure it: not "this is fast" but "this loop nest moves at least L
bytes between memory levels for any schedule, and the generated code moves at
most 1.3 L; therefore it is within a factor 1.3 of the best any compiler could
do on this machine model". Regressions become build errors.

**Prior art, so we do not overclaim.** Hong and Kung's red-blue pebble game gave
I/O lower bounds (1981). IOLB derives parametric data-movement lower bounds
automatically for affine programs (Olivry, Langou, Pouchet, Sadayappan,
Rastello, PLDI 2020). The roofline model bounds attainable throughput by
arithmetic intensity. Resource-analysis and WCET tools prove upper bounds on
cost. None of these is a language feature checked against the compiled code.

**What would be new.** (1) The bound is part of the program, written beside the
function, checked on every build and recorded in the certificate. (2) The
machine model (peak FMA throughput, cache sizes, bandwidth, measured on the
machine and versioned) is a first-class object the compiler reads. (3) The
compiler reports *why* a kernel is not at its roof (memory- or compute-bound,
vector width used against available, spills, branches left). (4) The achieved
traffic comes from an exact counter, not a guess (below).

**Built on.** `for` loops, the contract verifier (it already proves loop bounds
and index ranges), the schedule and SIMD tracks, the roofline microbenchmarks.

**First experiment (can fail).** Ten affine kernels (dot, saxpy, matvec, matmul,
stencils, transpose, triangular solve). Derive a data-movement lower bound with
a small IOLB-style analysis for the subset of loops the language supports.
Count achieved traffic exactly with a deterministic cache simulator driven by
the kernel's address trace (hardware counters are not reliably available on the
development machine). Compare. **It fails if** the derived bounds are looser
than 3x for most kernels (a bound that loose guides nothing), or if the
simulator's miss counts do not predict measured time ranking across schedules.

**Stop if** bounds are only derivable for toy kernels and the supported subset
cannot express what people write.

## P2. Optimality-certified code generation

**Idea.** The e-graph optimizer explores many equivalent programs; extract the
cheapest under an exact microarchitectural cost model, and emit a certificate
that no equivalent term *in the explored space* is cheaper, plus a translation
validation proof that the chosen code equals the source.

**Prior art.** Superoptimization (Massalin; STOKE searches stochastically),
equality saturation with extraction (egg, Tate et al.), ILP-based extraction,
llvm-mca and uiCA-style throughput models, certifying algorithms (certificates
of optimality are an active topic in pseudo-Boolean solving).

**What would be new.** An optimality claim relative to a stated space and model,
checked independently, in a production compiler, with equivalence proven by the
same solver. The honest wording is "optimal in this rewrite space under this
model", never "optimal".

**First experiment.** For basic blocks of 5 to 40 instructions from the SIMD
kernels, compare the model's predicted throughput with measured throughput
across 20 candidate schedules (rank correlation). **It fails if** the rank
correlation is below about 0.8 on this machine: a model that cannot rank
candidates cannot justify an optimality claim. Then check the exact extraction
beats the greedy extractor Eidos uses today by a margin worth the cost.

## P3. One function, every substrate, proven equal

**Idea.** An `oblivious` pure function lowers to native constant-time code, GPU,
FPGA, a zero-knowledge circuit (done), an FHE circuit, and a *reversible circuit*
that becomes a quantum oracle, from one analysis, with a checked equivalence
between every pair of lowerings.

**Prior art.** CirC (a shared compiler infrastructure for proof systems);
ReVerC (a verified compiler from a classical language to reversible circuits
with ancilla clean-up proofs); Quipper's automatic lifting to reversible
circuits; Bennett's reversible computation and pebbling trade-offs between
space and recomputation.

**What would be new.** The combination: constant-time native, ZK, hardware,
and quantum-oracle lowerings under one front-end check, with each lowering
proven equivalent to the source by the SMT layer already in the compiler, and
the results hot-swappable at run time. Nobody wants five languages for one
function.

**Built on.** `oblivious` analysis, `codira_circuits` (ZK, Verilog), the
quantum crate and its simulator, the translation validator.

**First experiment.** Twenty functions (8 to 16 bit arithmetic, comparison,
`ct_select`, small loops) through five lowerings. Equivalence checked
exhaustively on 8-bit inputs and by SMT for wider ones. Measure circuit size
growth. **It fails if** the reversible lowering needs more qubits or Toffoli
gates than a hand-written oracle by a large factor, or if uncomputation cannot
be proven for functions with loops.

## P4. Physics-typed computation

**Idea.** Time, energy, bytes moved and *bits erased* as quantities in the type
system, checked the way units of measure are checked today. A reversible subset
(no information erasure) is tracked, and its cost is accounted in Landauer
units (kT ln 2 per erased bit) so that a program's thermodynamic floor is a
number the compiler can print.

**Prior art.** Reversible languages (Janus, Eel and other energy-aware
partially reversible languages), adiabatic and reversible hardware research
(Frank and others), units of measure (F#).

**What would be new.** Integration with a real optimizing compiler, a proof
layer, and calibration against measured energy on real hardware, with the
accounting visible in ordinary code and not in a separate research language.

**First experiment.** Add an erasure count to the type checker for the
integer-only subset and report it for ten functions; compare with the number of
bits a hand analysis says must be erased. Energy claims need hardware counters
(RAPL on a platform that exposes them) and are **not** made until measured.
**It fails if** the erasure accounting is too coarse to distinguish a reversible
rewrite from its irreversible original.

## P5. Living programs with proofs (the paper in `paper/`)

**Idea.** An update, a specialised version compiled for a hot value, or a
fleet-wide patch is admitted only with a certificate the running process
re-checks, swapped at a safe point, and rolled back automatically on a fault.
This is already specified (RFC 3 and `paper/proof-gated-updates`); the new
extension is *speculative specialisation*: runtime counters find a value that
never changes, the daemon compiles a guarded specialised version, the validator
proves it equal under the guard, and the dispatch table swaps it in, so an
ahead-of-time compiler gets just-in-time quality with every swap proven.

**Prior art.** Dynamic software updating (Ginseng, Kitsune, Rubah), regression
verification, translation validation (Alive2), JIT speculation with deopt.

**First experiment.** The three workloads and update corpus the evaluation track
is building. **It fails if** most realistic updates fall outside what the
prover can handle (coverage below about half) or the safe-point protocol blocks
most updates.

## P6. An oracle proposes, the solver decides

**Idea.** Search, learned models and language models propose rewrites,
schedules, invariants and patches; nothing is accepted until the compiler proves
it. This makes untrusted proposers safe to use at scale and turns the
compiler's proofs into a training signal and a filter.

**Prior art.** CEGIS-style synthesis, learned optimizers, AI-assisted program
repair, certified patches.

**First experiment.** Use a cost model to propose schedules for the SIMD
kernels and count how often an unsound proposal reaches the verifier (it must
never get past it). Measure end-to-end speedup of the accepted proposals
against the hand-tuned schedule. **It fails if** the proof step rejects most
proposals (the proposer is useless) or admits one wrong one (the checker is).

## P7. Long horizon (speculative, no experiment yet)

Targets beyond the von Neumann machine: in-memory and analog compute,
neuromorphic hardware, quantum-classical co-compilation, adiabatic reversible
CPUs. The only claim made here is architectural: a small, proof-carrying
semantic core with substrate back ends is the right shape to adopt such targets
without rewriting programs. We have no evidence yet; P3 and P4 are the steps
that would produce some.

## Sequencing and dependencies

1. Finish the platform the others stand on: SIMD and perf tooling (P1 needs the
   roofline report), traits and a usable standard library, the update prover and
   runtime (P5), faster Z3 use (every proof-heavy program above).
2. P1 and P3 first among the new programs: both are buildable on what exists, and
   both have crisp first experiments.
3. P2 needs a trustworthy cost model, which P1's measurements supply.
4. P4 after P3 (it shares the reversible lowering).
5. P6 once P1 and P2 exist (they provide the cost model and the verifier).

## How we avoid fooling ourselves

- Every claim names its model, its machine and its caveats; "optimal" always
  means "within this space under this model".
- Every program has an experiment above that can fail. If one fails, the plan
  changes and this file says so.
- Novelty is stated as a combination, never as a first, until a literature search
  shows nothing equivalent. The related-work notes in each section are from
  searches on 2026-10-19 and are not exhaustive.
- Measurements follow the rules in the build notes: minimum and median over at
  least 25 interleaved runs, load stated, commit recorded.

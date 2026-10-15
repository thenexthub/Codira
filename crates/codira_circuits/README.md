# codira_circuits

Circuits from Codira functions. One elaborator turns a function into a
word-level circuit; two back ends consume it:

- **`codira attest`**: zero-knowledge proofs for `oblivious func`s. The
  circuit becomes an R1CS over the BN254 scalar field (written as an iden3
  `.r1cs` file and as Circom 2 source), and arkworks proves it with Groth16.
- **`@target(fpga)`**: synthesizable Verilog-2001, one module per function,
  written by `codira build` as `target/<function>.v`.

## The subset

A circuit function takes and returns integers of 8, 16, 32 or 64 bits
(signed or unsigned; `usize`/`isize` are the host's 64 bits) and `bool`s.
Inside, it may use:

- literals, parameters, `let`/`var`, assignment and compound assignment;
- `+ - *` (wrapping, exactly as native code), `& | ^ ~ !`, `<< >>` (`>>` is
  arithmetic on signed types), `== != < <= > >=` (signed or unsigned by
  type), unary `-`, and `as` between integer types and from `bool`;
- `ct_select(c, a, b)` and `declassify(e)`;
- calls to other Codira functions (inlined; recursion only to a compile-time
  depth);
- `if`, `&&`, `||` on compile-time conditions (only the branch taken is
  elaborated); in `@target(fpga)` functions also on inputs (both branches
  become a multiplexer);
- `while`, `loop` and `for i in a..b` / `a..=b` whose bounds are known at
  compile time (unrolled), `break`, `continue`, and `return` where no
  input-dependent condition guards it;
- `/` and `%` on compile-time values.

Everything else is refused with the file, line, column and the reason:
floats, 128-bit integers, strings, arrays, tuples, structs, classes, methods,
effects, `print`, concurrency, `extern` functions, division of an input,
loops with input-dependent bounds, and, for `attest`, any branch on an input
(an `oblivious` function chooses with `ct_select`) or a secret passed to a
function that is not `oblivious`/`@constant_time`.

Shifts by the width or more: native code leaves them undefined (LLVM
`poison`); every circuit back end gives 0 (`<<`, `>>` on unsigned) or all
sign bits (`>>` on signed), which is SMT-LIB's and Verilog's rule. Keep shift
amounts below the width when the native result matters.

## Exact integers in a prime field

The field wraps at a 254-bit prime, the machine at `2^w`. Inputs are
decomposed into bits on entry (that range-checks them); sums, differences,
negations and constant products stay linear combinations with a tracked
bound and cost nothing; when bits or the exact value are needed the value is
decomposed again, which reduces it modulo `2^w`. Constraints per operator,
measured on 64-bit operands (`r1cs::tests::constraint_counts_per_operator`):

| operator | constraints |
|---|---|
| input | 65 |
| `+`, `-`, unary `-`, `~`, `* constant`, narrowing `as` | 0 (deferred) |
| reduction of a value below `2^m` | `m + 1` (66 after an add, 130 after a multiply) |
| `*` | 1 |
| `&`, `|`, `^` | 64 |
| `<<`, `>>` by a constant | 0 |
| `<<`, `>>` by a variable | 450 (a barrel shifter, plus the out-of-range case) |
| `==`, `!=` | 2 |
| `<`, `<=`, `>`, `>=` | 66 (signed and unsigned) |
| `ct_select` | 1 |
| output | 1 |

## `codira attest`

```bash
codira attest build score.code::score --out-dir keys    # .r1cs .circom .pk .vk
codira attest prove score.code::score --input x=-7 --input y=1234 \
    --pk keys/score.pk -o score.proof                    # prints "score reveals ..."
codira attest verify --vk keys/score.vk --proof score.proof
codira attest verify --vk keys/score.vk --proof score.proof --reveal 99   # INVALID
```

This is the roadmap's `attest score(x, y) reveal r`: the inputs are private
witnesses, the function's result is the single public input, and `verify`
needs only the verifying key and the proof. It is a CLI rather than syntax
because proving is something a prover does to a program, not something the
program does; a statement form would need a grammar change shared with
other tracks, and nothing in the language could consume its result yet.

Setup is circuit-specific and run by one party (`build`), who could forge
proofs for that circuit. A deployment that needs more can run a multi-party
ceremony with snarkjs over the emitted `.r1cs`.

## What is verified, and how

- The IR evaluator against an independent integer reference for every
  operator: exhaustively on all pairs of 8-bit values (signed and unsigned),
  and on edge cases (0, 1, -1, MIN, MAX, MIN+1, MAX-1, alternating bits,
  shifts by 63, 64 and more) plus random values on 64-bit ones.
- The R1CS witness on the same inputs: every constraint satisfied and the
  public output equal to the reference.
- Every operator on six integer types, plus casts between them, against the
  same Codira functions compiled by LLVM and run natively (more than 100,000
  comparisons; `codira/tests/circuits.rs`).
- Groth16 (arkworks 0.5, BN254): proofs verify; a wrong revealed value, a
  proof for other inputs, and a corrupted proof are rejected; keys and
  proofs round-trip through their encodings; the CLI flow end to end.
- Tampered witnesses (a wrong output, a non-boolean bit, the unreduced
  field product) fail the constraint check.
- External tools, run from a scratch directory, not installed system-wide
  (the tests report a skip when they are absent):
  - snarkjs 0.7 `wtns check` accepts Codira's own `.r1cs` and `.wtns`
    files; circom 2 (the `circom2` npm WASM build) compiles the emitted
    Circom, its witness generator computes the same output as native code,
    and snarkjs checks that witness against circom's R1CS
    (`CODIRA_ZK_TOOLS=<dir with node_modules>`).
  - Yosys (YoWASP's WebAssembly build from pip, `CODIRA_YOSYS=<path to
    yowasp-yosys>`) reads and elaborates the emitted Verilog, evaluates it on
    sampled vectors before and after `synth -noabc`, and matches native code.

Not verified: no event-driven simulator (Icarus, Verilator) and no FPGA
vendor tool has run the Verilog; Yosys's ABC technology-mapping step does not
run under YoWASP on Windows, so synthesis stops at Yosys's generic gates; no
snarkjs-generated Groth16 proof has been checked against arkworks' keys (the
formats differ); no multi-party setup.

## FHE

Fully homomorphic encryption (`encrypted`) is out of scope here, but it needs
exactly what this crate produces: `ir::Circuit` is a fixed, data-independent
DAG of integer and boolean operations. Zama's `tfhe` crate is the candidate
back end: its radix integers provide `+ - * & | ^ << >>` and comparisons,
and `if_then_else` is the multiplexer.

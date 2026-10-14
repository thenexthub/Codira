//! Copyright (c) 2026 Omnira CJSC. All Rights Reserved.
//! Author: Tunjay Akbarli
//! Date: September 15, 2026
//!
//! `codira_tv` -- SMT-backed **translation validation** for `codira_mir`.
//!
//! After an optimizer (the e-graph rewriter, `simplify`, CSE, constant
//! folding, ...) rewrites a function [`Body`], this crate *proves* that the
//! rewrite preserved semantics -- or produces a concrete counterexample
//! input on which the two bodies disagree. It makes the compiler
//! self-skeptical: the pass manager does not have to trust its own
//! optimizer, it can ask for a proof after every rewrite.
//!
//! # Prior art, and how this differs
//!
//! Translation validation is the classic alternative to verifying a
//! compiler once and for all: instead, validate each *individual*
//! translation the compiler actually performed.
//!
//! * Pnueli, Siegel, Singerman, *Translation Validation* (TACAS 1998) --
//!   introduced the idea for a synchronous-language-to-C compiler.
//! * Necula, *Translation Validation for an Optimizing Compiler* (PLDI 2000) --
//!   scaled it to GCC's intraprocedural optimizations via symbolic evaluation
//!   and a bespoke prover.
//! * Alive/Alive2 (Lopes et al., PLDI 2021) -- SMT-based refinement checking
//!   for LLVM IR, run offline over the LLVM test suite as an external tool.
//!
//! `codira_tv` follows the same architecture as Alive2 (symbolically
//! evaluate both programs into solver terms, assert result disagreement,
//! ask for UNSAT), but with a different deployment model: it validates the
//! **mid-level structured IR** (region-based `cf.if`, not a flat CFG),
//! **online**, **per-function**, as an always-available **library** the
//! pass manager can call after any rewrite -- rather than an offline batch
//! tool over a test corpus. Anything it cannot encode it reports honestly
//! as [`Verdict::Unsupported`] instead of guessing.
//!
//! # The validated fragment
//!
//! Loop-free, call-free, tuple-free, param-free bodies over `Int`/`Bool`:
//! straight-line SSA plus (arbitrarily nested) `cf.if`. This is exactly
//! the fragment the algebraic-rewrite passes actually touch; loops, calls,
//! floats, strings, and tuples yield `Unsupported` with a reason.
//!
//! # Soundness decisions (read this before trusting a verdict)
//!
//! The encoding is **linear integer arithmetic over unbounded Z3 `Int`**,
//! plus `Bool`. `codira_smt` also offers 64-bit bitvectors, which would
//! model wrapping `i64` bit-precisely, but this crate does not use them:
//! everything below is a statement about the unbounded encoding, and the
//! caveats it carries are real. Consequences, op by op:
//!
//! * **Unbounded integers vs. wrapping `i64`.** `codira_mir::fold` defines
//!   `Int` as wrapping two's-complement `i64`; Z3 `Int` is unbounded. A
//!   [`Verdict::Proven`] therefore certifies equivalence **for all executions
//!   in which no intermediate value overflows `i64`** -- the standard
//!   translation-validation caveat, surfaced explicitly as `Proven {
//!   modulo_overflow: true }` whenever the bodies contain any overflow-capable
//!   op (`Add`/`Sub`/`Mul`/`Neg`/`Shl`, or `Div`/`Rem` with divisor `-1`).
//!   Bodies built only from comparisons, boolean connectives, `Div`/`Rem`/`Shr`
//!   by benign constants, and constants are modeled *exactly*, and get `Proven
//!   { modulo_overflow: false }`.
//! * **`Mul`**: linear arithmetic only -- multiplication is encoded when at
//!   least one operand is a compile-time constant (emitted as `c * x`, a linear
//!   term). Variable-times-variable is `Unsupported`.
//! * **`Div`/`Rem`**: only by a *nonzero constant* `c`. Rust/i64 division
//!   truncates toward zero (not Euclidean, not floor), so it is encoded
//!   precisely with fresh quotient/remainder variables and a sign case-split:
//!   `x = q*c + r`, with `x >= 0 -> 0 <= r < |c|` and `x < 0 -> -|c| < r <= 0`.
//!   These constraints are total and functional (for every `x` a unique `(q,
//!   r)` exists), so they neither block nor admit spurious counterexamples.
//!   Division by a variable, or by constant zero (an unconditional evaluation
//!   error), is `Unsupported`. `c == -1` sets the overflow caveat (`i64::MIN /
//!   -1` errors in `fold`, which the unbounded model cannot see).
//! * **`Shl` by constant `k` in `0..64`**: `x << k` is `x * 2^k` in the
//!   unbounded model; shifted-out bits fall under the overflow caveat.
//! * **`Shr` by constant `k` in `0..64`**: arithmetic right shift of `i64`
//!   **is** floor division by `2^k` (not truncated division!), and floor
//!   division is encoded exactly: `x = q * 2^k + r`, `0 <= r < 2^k`. This op is
//!   exact -- it never sets the overflow caveat. Non-constant or out-of-range
//!   shift amounts are `Unsupported`.
//! * **`BitAnd`/`BitOr`/`BitXor`**: folded when both operands are constants;
//!   otherwise `Unsupported` (not expressible in LIA without bitvectors).
//! * **`If`**: each `cf.if` is encoded with a fresh variable `v` constrained by
//!   `cond -> v = then` and `!cond -> v = else` (with `<->` via mutual
//!   implication for boolean results) rather than with `codira_smt`'s `ite`
//!   builders. Again total and functional, so satisfiability is preserved in
//!   both directions.
//! * **Error behavior**: within the accepted fragment every dynamic `FoldError`
//!   is ruled out syntactically (nonzero constant divisors, in-range constant
//!   shift amounts) except `i64::MIN / -1`, which is subsumed by the overflow
//!   caveat. Equivalence is therefore plain value equality over the modeled
//!   executions.
//!
//! # Argument sorts
//!
//! `OpKind::Arg(i)` is untyped in the IR, so each argument's sort is
//! inferred from its **use sites** across *both* bodies (an argument fed
//! to `Add` is an integer; an `If` condition is a boolean; `Eq`/`Ne`
//! propagate the other operand's sort). Conflicting uses yield
//! `Unsupported`; arguments with no constraining use default to `Int`.
//! Both bodies are encoded over the *same* argument variables, which is
//! what makes the equivalence query meaningful.
//!
//! # Counterexamples
//!
//! A satisfiable disagreement query is a refutation **in the unbounded
//! model**. That is not yet a counterexample: an `Int` variable can take a
//! value no `i64` holds, and an execution whose intermediate values leave
//! `i64` range behaves differently once it wraps. So a witness is reported
//! only after it has been **confirmed by running both bodies** on it under
//! the real semantics (`codira_mir::fold_op`, via the `eval` module):
//!
//! 1. Read each argument's value out of the solver's model
//!    (`codira_smt::Solver::model`). An argument whose model value does not fit
//!    `i64` has no real counterpart, and the candidate is dropped.
//! 2. Evaluate `before` and `after` on those values. The candidate is a witness
//!    only if **both evaluate to a value and the values differ**. An evaluation
//!    error on either side (`i64::MIN / -1`) is not counted as a difference:
//!    "one side has no value" is not the claim a witness makes.
//! 3. If the model's candidate is not confirmed, fall back to **greedy bounded
//!    probing**: re-check satisfiability with each argument pinned to small
//!    candidate values in turn. The model is one arbitrary satisfying
//!    assignment and may sit out of range while a small one exists; the probe
//!    finds those. Its result goes through the same confirmation as step 2.
//!
//! If no candidate is confirmed the verdict is still [`Verdict::Refuted`]
//! (the solver's answer stands), with `args: None`. That outcome is
//! expected, not exceptional: when the bodies differ *only* on values or
//! executions outside `i64` range, they agree on every real input and no
//! true witness exists. The validator does not constrain argument
//! variables to `i64` range, so it reports such a pair as `Refuted` --
//! rejecting a rewrite that was in fact fine, which is the safe direction
//! for a validator to be wrong in.

mod encode;
mod eval;
mod infer;

use codira_mir::{verify_body, Attr, Body};
use codira_smt::{BoolExpr, CheckOutcome, Context, SatResult, Solver};

use crate::encode::{iff, Encoder, Sym};

/// The outcome of validating one rewrite. See the crate docs for the exact
/// guarantees behind each variant.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Verdict {
    /// The two bodies are semantically equivalent.
    ///
    /// `modulo_overflow: true` means the proof holds in the unbounded
    /// integer model, i.e. for every execution in which no intermediate
    /// value leaves `i64` range (the bodies contain at least one
    /// overflow-capable op). `false` means every op involved is modeled
    /// exactly and the proof is unconditional.
    Proven { modulo_overflow: bool },
    /// The solver found an assignment on which the encodings disagree.
    ///
    /// `args`, when present, is a **confirmed** witness: both bodies were
    /// evaluated on it under the real wrapping-`i64` semantics and
    /// produced different values. `args[i]` is the value of `Arg(i)`
    /// (booleans encoded as `0`/`1`; arguments referenced by neither body
    /// filled with `0`).
    ///
    /// `None` means no candidate was confirmed (or witness recovery is
    /// disabled). The disagreement then stands in the unbounded model
    /// only; the bodies may agree on every real `i64` input. See the
    /// crate docs, "Counterexamples".
    Refuted { args: Option<Vec<i64>> },
    /// The bodies fall outside the encodable fragment; `reason` says
    /// exactly which construct was the obstacle.
    Unsupported { reason: String },
    /// Z3 could not decide the query; `reason` says why (`resource limit`
    /// when the deterministic budget ran out). Never a proof.
    Unknown { reason: String },
}

/// A configurable validator.
///
/// The only knob today is whether to recover a concrete counterexample on
/// refutation. A per-query *timeout* is deliberately absent, though
/// `codira_smt::Solver::set_timeout_ms` exists: a wall-clock budget would
/// let the machine's load decide between `Proven` and `Unknown`, and a
/// verdict that feeds compiler output has to be a function of its inputs
/// alone.
#[derive(Debug, Clone)]
pub struct Validator {
    search_witness: bool,
}

impl Validator {
    /// A validator with default settings (witness recovery enabled).
    pub fn new() -> Self {
        Validator {
            search_witness: true,
        }
    }

    /// Disables witness recovery: refutations come back as
    /// `Refuted { args: None }` after a single solver call, with no model
    /// read and no probing.
    pub fn without_witness_search(mut self) -> Self {
        self.search_witness = false;
        self
    }

    /// Validates that `after` computes the same result as `before` on all
    /// (non-overflowing) inputs. See [`Verdict`] and the crate docs.
    pub fn validate(&self, before: &Body, after: &Body) -> Verdict {
        // Structurally malformed IR has no semantics to compare; report it
        // rather than encoding garbage.
        if let Err(e) = verify_body(before) {
            return Verdict::Unsupported {
                reason: format!("before body fails structural verification: {e}"),
            };
        }
        if let Err(e) = verify_body(after) {
            return Verdict::Unsupported {
                reason: format!("after body fails structural verification: {e}"),
            };
        }

        let arg_sorts = match infer::infer_arg_sorts(&[before, after]) {
            Ok(sorts) => sorts,
            Err(reason) => return Verdict::Unsupported { reason },
        };

        let ctx = Context::new();
        let mut enc = Encoder::new(&ctx, &arg_sorts);

        let sym_before = match enc.encode_body(before) {
            Ok(sym) => sym,
            Err(reason) => return Verdict::Unsupported { reason },
        };
        let sym_after = match enc.encode_body(after) {
            Ok(sym) => sym,
            Err(reason) => return Verdict::Unsupported { reason },
        };

        // The equivalence query: do the encodings (which are functional in
        // the shared argument variables) admit an argument assignment on
        // which the results differ? UNSAT <=> no counterexample <=> the
        // rewrite is proven (in the unbounded model).
        let disagree = match (sym_before, sym_after) {
            (Sym::Int(b, _), Sym::Int(a, _)) => b.eq(a).not(),
            (Sym::Bool(b, _), Sym::Bool(a, _)) => iff(b, a).not(),
            _ => {
                return Verdict::Unsupported {
                    reason: "before and after bodies compute results of different sorts \
                             (int vs bool)"
                        .to_string(),
                }
            }
        };

        let solver = ctx.solver();
        for c in &enc.constraints {
            solver.assert(*c);
        }
        solver.assert(disagree);

        match solver.check_detailed() {
            CheckOutcome::Unsat => Verdict::Proven {
                modulo_overflow: enc.overflow_possible,
            },
            CheckOutcome::Sat => Verdict::Refuted {
                args: if self.search_witness {
                    find_witness(&ctx, &enc, &solver, disagree, before, after)
                } else {
                    None
                },
            },
            // "Could not decide" is its own answer: neither a proof nor a
            // refutation may be read into it.
            CheckOutcome::Unknown(reason) => Verdict::Unknown {
                reason: reason.to_string(),
            },
        }
    }
}

impl Default for Validator {
    fn default() -> Self {
        Self::new()
    }
}

/// Validates a single rewrite with default settings. The library entry
/// point a pass manager calls after an optimizer touches a body.
pub fn validate_rewrite(before: &Body, after: &Body) -> Verdict {
    Validator::new().validate(before, after)
}

/// One candidate counterexample: a value for every argument either body
/// references, in ascending argument order. Not yet a witness -- see
/// [`confirm_witness`].
type Candidate = Vec<(u32, Attr)>;

/// Recovers a confirmed counterexample for a satisfiable disagreement
/// query (crate docs, "Counterexamples"). `solver` must be the solver
/// whose last `check` returned `Sat` for exactly that query, with nothing
/// asserted since: its model is what is read.
fn find_witness<'ctx>(
    ctx: &'ctx Context,
    enc: &Encoder<'_, 'ctx>,
    solver: &Solver<'ctx>,
    disagree: BoolExpr<'ctx>,
    before: &Body,
    after: &Body,
) -> Option<Vec<i64>> {
    // Sorted so the probe fixes arguments in a stable order: the map's
    // iteration order must not be able to change which witness is found.
    let mut arg_syms: Vec<(u32, Sym<'ctx>)> = enc.args.iter().map(|(i, s)| (*i, *s)).collect();
    arg_syms.sort_by_key(|(i, _)| *i);

    model_candidate(solver, &arg_syms)
        .and_then(|candidate| confirm_witness(before, after, &candidate))
        .or_else(|| {
            // The model is one arbitrary satisfying assignment; it can sit
            // outside i64 range, or on an overflowing execution, while a
            // small real counterexample exists. The probe looks for that.
            probe_candidate(ctx, enc, disagree, &arg_syms)
                .and_then(|candidate| confirm_witness(before, after, &candidate))
        })
}

/// Reads every argument's value out of the solver's model.
///
/// `None` if the solver has no model, or if any argument's value cannot be
/// read as its real type: `Model::eval_int` declines a value that does not
/// fit `i64`, and such a value is no `i64` input at all. `codira_smt`
/// evaluates with model completion on, so an argument the model leaves
/// unconstrained still gets a definite value rather than a miss.
fn model_candidate<'ctx>(
    solver: &Solver<'ctx>,
    arg_syms: &[(u32, Sym<'ctx>)],
) -> Option<Candidate> {
    let model = solver.model()?;
    arg_syms
        .iter()
        .map(|(index, sym)| {
            let value = match sym {
                Sym::Int(term, _) => Attr::Int(model.eval_int(*term)?),
                Sym::Bool(term, _) => Attr::Bool(model.eval_bool(*term)?),
            };
            Some((*index, value))
        })
        .collect()
}

/// The gate every reported witness passes: runs both bodies on the
/// candidate under the real semantics and returns the witness vector only
/// if both produce a value and the values differ.
///
/// The solver's word is not enough. Its model satisfies the *unbounded*
/// encoding, so the disagreement it shows may exist only because some
/// intermediate value left `i64` range; wrapped, the bodies can agree. An
/// evaluation error on either side also yields `None`: a witness claims
/// two different results, and an erroring body has no result to differ.
fn confirm_witness(before: &Body, after: &Body, candidate: &[(u32, Attr)]) -> Option<Vec<i64>> {
    // Dense argument vector indexed by argument number; arguments no body
    // references cannot influence either result, so 0 is as good a value
    // as any there.
    let len = candidate
        .iter()
        .map(|(i, _)| *i as usize + 1)
        .max()
        .unwrap_or(0);
    let mut args = vec![Attr::Int(0); len];
    for (index, value) in candidate {
        args[*index as usize] = value.clone();
    }

    let differ = match (
        eval::eval_body(before, &args)?,
        eval::eval_body(after, &args)?,
    ) {
        (Attr::Int(b), Attr::Int(a)) => b != a,
        (Attr::Bool(b), Attr::Bool(a)) => b != a,
        // Results of different (or non-int/bool) shapes are outside what
        // the equivalence query compared; nothing is confirmed.
        _ => return None,
    };
    if !differ {
        return None;
    }

    args.iter()
        .map(|arg| match arg {
            Attr::Int(v) => Some(*v),
            Attr::Bool(b) => Some(i64::from(*b)),
            _ => None,
        })
        .collect()
}

/// Candidate values tried (in order) for each integer argument by the
/// fallback probe. Small values first: they are the counterexamples
/// humans want to read, and the ones rewrite bugs almost always admit.
const PROBE_CANDIDATES: &[i64] = &[
    0, 1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, 8, -8, 9, -9, 10, -10, 15, -15, 16, -16,
    17, 31, 32, 33, 63, 64, 100, -100, 127, 128, 255, 256, 1000, -1000,
];

/// Greedy bounded probe (see crate docs): fixes arguments one at a time to
/// the first candidate value that keeps the disagreement satisfiable.
/// Each probe is a fresh solver over the same assertions -- cheap at these
/// formula sizes, and it leaves the caller's solver and its model alone.
///
/// A probe that comes back `Unknown` is treated as "not this value": the
/// candidate it yields is confirmed by evaluation before anyone sees it,
/// so an undecided probe can cost a witness but never invent one.
fn probe_candidate<'ctx>(
    ctx: &'ctx Context,
    enc: &Encoder<'_, 'ctx>,
    disagree: BoolExpr<'ctx>,
    arg_syms: &[(u32, Sym<'ctx>)],
) -> Option<Candidate> {
    let check_with = |fixed: &[(Sym<'ctx>, i64)]| -> bool {
        let solver = ctx.solver();
        for c in &enc.constraints {
            solver.assert(*c);
        }
        solver.assert(disagree);
        for (sym, value) in fixed {
            let pin = match sym {
                Sym::Int(term, _) => term.eq(ctx.int_lit(*value)),
                Sym::Bool(term, _) => {
                    if *value != 0 {
                        *term
                    } else {
                        term.not()
                    }
                }
            };
            solver.assert(pin);
        }
        solver.check() == SatResult::Sat
    };

    let mut fixed: Vec<(Sym<'ctx>, i64)> = Vec::with_capacity(arg_syms.len());
    let mut values: Candidate = Vec::with_capacity(arg_syms.len());
    for (index, sym) in arg_syms {
        let candidates: &[i64] = match sym {
            Sym::Int(..) => PROBE_CANDIDATES,
            Sym::Bool(..) => &[1, 0],
        };
        let mut chosen = None;
        for &candidate in candidates {
            fixed.push((*sym, candidate));
            if check_with(&fixed) {
                chosen = Some(candidate);
                break;
            }
            fixed.pop();
        }
        let value = chosen?;
        values.push((
            *index,
            match sym {
                Sym::Int(..) => Attr::Int(value),
                Sym::Bool(..) => Attr::Bool(value != 0),
            },
        ));
    }
    Some(values)
}

#[cfg(test)]
mod tests {
    use codira_mir::OpKind;

    use super::*;

    /// before: x - 1        after: x + 1
    fn sub_one_and_add_one() -> (Body, Body) {
        let mut before = Body::new();
        let x = before.push(OpKind::Arg(0), []);
        let one = before.push(OpKind::Const(Attr::Int(1)), []);
        before.push(OpKind::Sub, [x, one]);

        let mut after = Body::new();
        let x = after.push(OpKind::Arg(0), []);
        let one = after.push(OpKind::Const(Attr::Int(1)), []);
        after.push(OpKind::Add, [x, one]);
        (before, after)
    }

    #[test]
    fn confirm_accepts_a_candidate_the_bodies_differ_on() {
        let (before, after) = sub_one_and_add_one();
        assert_eq!(
            confirm_witness(&before, &after, &[(0, Attr::Int(5))]),
            Some(vec![5])
        );
    }

    #[test]
    fn confirm_rejects_a_candidate_the_bodies_agree_on() {
        // The gate itself, with no solver involved: a candidate on which
        // the bodies agree must never come out as a witness, whoever
        // proposed it.
        let (before, _) = sub_one_and_add_one();
        assert_eq!(
            confirm_witness(&before, &before, &[(0, Attr::Int(5))]),
            None
        );
    }

    #[test]
    fn confirm_rejects_a_disagreement_that_exists_only_unbounded() {
        // before: (x * 2) > i64::MAX        after: false
        // At x = 2^62 the unbounded model says 2^63 > i64::MAX, i.e. true
        // vs false. Really, x * 2 wraps to i64::MIN and both are false.
        let mut before = Body::new();
        let x = before.push(OpKind::Arg(0), []);
        let two = before.push(OpKind::Const(Attr::Int(2)), []);
        let doubled = before.push(OpKind::Mul, [x, two]);
        let max = before.push(OpKind::Const(Attr::Int(i64::MAX)), []);
        before.push(OpKind::Gt, [doubled, max]);

        let mut after = Body::new();
        after.push(OpKind::Const(Attr::Bool(false)), []);

        assert_eq!(
            confirm_witness(&before, &after, &[(0, Attr::Int(1 << 62))]),
            None
        );
    }

    #[test]
    fn confirm_rejects_a_candidate_either_body_errors_on() {
        // before: x / -1        after: x
        // i64::MIN / -1 is an evaluation error, not a value that differs.
        let mut before = Body::new();
        let x = before.push(OpKind::Arg(0), []);
        let minus_one = before.push(OpKind::Const(Attr::Int(-1)), []);
        before.push(OpKind::Div, [x, minus_one]);

        let mut after = Body::new();
        after.push(OpKind::Arg(0), []);

        assert_eq!(
            confirm_witness(&before, &after, &[(0, Attr::Int(i64::MIN))]),
            None
        );
        assert_eq!(
            confirm_witness(&before, &after, &[(0, Attr::Int(3))]),
            Some(vec![3])
        );
    }

    #[test]
    fn confirm_fills_unreferenced_arguments_and_encodes_booleans() {
        // Only Arg(2) is referenced; Arg(0)/Arg(1) are padding.
        let mut before = Body::new();
        before.push(OpKind::Arg(2), []);
        let mut after = Body::new();
        let c = after.push(OpKind::Arg(2), []);
        after.push(OpKind::Not, [c]);

        assert_eq!(
            confirm_witness(&before, &after, &[(2, Attr::Bool(true))]),
            Some(vec![0, 0, 1])
        );
    }
}

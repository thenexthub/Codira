//! Copyright (c) 2026 Omnira CJSC
//!
//! End-to-end translation-validation tests: build `before`/`after` bodies
//! with the public `codira_mir` API, structurally verify them (every body
//! a pass would hand the validator must pass `verify_body`), then check
//! the verdict.
//!
//! These run unconditionally, the same way `codira_smt`'s own tests do:
//! Z3 is a hard build-time requirement of that crate (its build script
//! fails the build if no install is found), so if this test binary linked
//! at all, the solver is present.

use codira_mir::{fold_op, verify_body, Attr, Body, OpId, OpKind, Region};
use codira_tv::{validate_rewrite, Validator, Verdict};

fn arg(body: &mut Body, i: u32) -> OpId {
    body.push(OpKind::Arg(i), [])
}

fn int(body: &mut Body, v: i64) -> OpId {
    body.push(OpKind::Const(Attr::Int(v)), [])
}

/// Verifies both bodies (the validator's contract is verified IR) and
/// runs the default validator.
fn validate(before: &Body, after: &Body) -> Verdict {
    verify_body(before).expect("before body must be structurally valid");
    verify_body(after).expect("after body must be structurally valid");
    validate_rewrite(before, after)
}

/// Runs `body` on concrete arguments under the real semantics
/// (`codira_mir::fold_op`: wrapping `i64`). Written here, against the
/// public `codira_mir` API, rather than borrowed from `codira_tv`: a
/// witness is only worth asserting on if the check is not the validator
/// marking its own homework. Panics if the body has no value on `args`.
fn eval(body: &Body, args: &[Attr]) -> Attr {
    let mut values: Vec<(OpId, Attr)> = Vec::new();
    let value_of = |values: &[(OpId, Attr)], id: OpId| -> Attr {
        let (_, value) = values
            .iter()
            .find(|(seen, _)| *seen == id)
            .expect("operands refer to earlier ops");
        value.clone()
    };
    for (id, op) in body.iter() {
        let value = match &op.kind {
            OpKind::Arg(i) => args[*i as usize].clone(),
            OpKind::If => {
                let taken = match value_of(&values, op.operands[0]) {
                    Attr::Bool(true) => &op.regions[0],
                    Attr::Bool(false) => &op.regions[1],
                    other => panic!("cf.if condition must be a bool, got {other:?}"),
                };
                eval(&taken.body, args)
            }
            kind => {
                let operands: Vec<Attr> = op
                    .operands
                    .iter()
                    .map(|operand| value_of(&values, *operand))
                    .collect();
                fold_op(kind, &operands).expect("body must evaluate on these arguments")
            }
        };
        values.push((id, value));
    }
    let (_, result) = values.pop().expect("body must not be empty");
    result
}

/// The property that makes a witness a witness: both bodies evaluate on
/// it, and to different values.
fn assert_witness_differs(before: &Body, after: &Body, args: &[Attr]) {
    assert_ne!(
        eval(before, args),
        eval(after, args),
        "bodies must disagree on the reported witness {args:?}"
    );
}

#[test]
fn add_zero_is_identity() {
    // before: x + 0        after: x
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let zero = int(&mut before, 0);
    before.push(OpKind::Add, [x, zero]);

    let mut after = Body::new();
    arg(&mut after, 0);

    // Add is overflow-capable in the wrapping-i64 semantics (vacuously
    // here, but the encoder is deliberately syntactic about the caveat).
    assert_eq!(
        validate(&before, &after),
        Verdict::Proven {
            modulo_overflow: true
        }
    );
}

#[test]
fn mul_two_equals_shl_one() {
    // before: x * 2        after: x << 1
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let two = int(&mut before, 2);
    before.push(OpKind::Mul, [x, two]);

    let mut after = Body::new();
    let x2 = arg(&mut after, 0);
    let one = int(&mut after, 1);
    after.push(OpKind::Shl, [x2, one]);

    assert_eq!(
        validate(&before, &after),
        Verdict::Proven {
            modulo_overflow: true
        }
    );
}

#[test]
fn add_then_sub_cancels() {
    // before: (a + b) - b        after: a
    let mut before = Body::new();
    let a = arg(&mut before, 0);
    let b = arg(&mut before, 1);
    let sum = before.push(OpKind::Add, [a, b]);
    before.push(OpKind::Sub, [sum, b]);

    let mut after = Body::new();
    arg(&mut after, 0);

    assert_eq!(
        validate(&before, &after),
        Verdict::Proven {
            modulo_overflow: true
        }
    );
}

#[test]
fn sub_one_is_not_add_one() {
    // before: x - 1        after: x + 1   -- wrong on every input.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let one = int(&mut before, 1);
    before.push(OpKind::Sub, [x, one]);

    let mut after = Body::new();
    let x2 = arg(&mut after, 0);
    let one2 = int(&mut after, 1);
    after.push(OpKind::Add, [x2, one2]);

    match validate(&before, &after) {
        Verdict::Refuted { args: Some(w) } => {
            assert_eq!(w.len(), 1, "one argument, one witness value");
            // Any value refutes this one; sanity-check the witness anyway.
            let x = w[0];
            assert_ne!(x.wrapping_sub(1), x.wrapping_add(1));
            assert_witness_differs(&before, &after, &[Attr::Int(x)]);
        }
        other => panic!("expected Refuted with a witness, got {other:?}"),
    }
}

#[test]
fn if_with_identical_branches_collapses() {
    // before: if (c) { a } else { a }        after: a
    // Exercises the fresh-variable ite encoding and bool argument
    // inference (c is only ever used as a cf.if condition).
    let mut then_region = Body::new();
    arg(&mut then_region, 1);
    let mut else_region = Body::new();
    arg(&mut else_region, 1);

    let mut before = Body::new();
    let c = arg(&mut before, 0);
    before.push_with_regions(
        OpKind::If,
        [c],
        [Region::new(then_region), Region::new(else_region)],
    );

    let mut after = Body::new();
    arg(&mut after, 1);

    // No arithmetic anywhere: the proof is exact, no overflow caveat.
    assert_eq!(
        validate(&before, &after),
        Verdict::Proven {
            modulo_overflow: false
        }
    );
}

#[test]
fn loops_are_unsupported() {
    // before: while (v < 10) { v = v + 1 } starting at 0        after: 10
    // The rewrite happens to be sound, but loops are outside the
    // fragment -- the honest answer is Unsupported, not Proven.
    let mut cond = Body::new();
    let cv = cond.push(OpKind::BlockArg(0), []);
    let limit = int(&mut cond, 10);
    cond.push(OpKind::Lt, [cv, limit]);

    let mut loop_body = Body::new();
    let bv = loop_body.push(OpKind::BlockArg(0), []);
    let one = int(&mut loop_body, 1);
    let next = loop_body.push(OpKind::Add, [bv, one]);
    loop_body.push(OpKind::Yield, [next]);

    let mut before = Body::new();
    let init = int(&mut before, 0);
    before.push_with_regions(
        OpKind::While,
        [init],
        [Region::with_args(1, cond), Region::with_args(1, loop_body)],
    );

    let mut after = Body::new();
    int(&mut after, 10);

    match validate(&before, &after) {
        Verdict::Unsupported { reason } => {
            assert!(
                reason.contains("loop"),
                "reason should name loops: {reason}"
            );
        }
        other => panic!("expected Unsupported for a loop, got {other:?}"),
    }
}

#[test]
fn division_by_variable_is_unsupported() {
    // before: x / y        after: x
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let y = arg(&mut before, 1);
    before.push(OpKind::Div, [x, y]);

    let mut after = Body::new();
    arg(&mut after, 0);

    match validate(&before, &after) {
        Verdict::Unsupported { reason } => assert!(
            reason.contains("non-constant divisor"),
            "reason should name the divisor: {reason}"
        ),
        other => panic!("expected Unsupported for variable division, got {other:?}"),
    }
}

#[test]
fn nonlinear_multiplication_is_unsupported() {
    // before: x * y        after: y * x -- true, but not in LIA's power.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let y = arg(&mut before, 1);
    before.push(OpKind::Mul, [x, y]);

    let mut after = Body::new();
    let y2 = arg(&mut after, 1);
    let x2 = arg(&mut after, 0);
    after.push(OpKind::Mul, [y2, x2]);

    match validate(&before, &after) {
        Verdict::Unsupported { reason } => assert!(
            reason.contains("nonlinear"),
            "reason should say nonlinear: {reason}"
        ),
        other => panic!("expected Unsupported for x*y, got {other:?}"),
    }
}

#[test]
fn div_then_mul_roundtrip_is_refuted() {
    // A deliberately wrong "optimization": (x / 2) * 2  ==>  x.
    // Wrong for every odd x; the truncated-division encoding must catch
    // it and the witness must be odd (which odd value is the solver's
    // choice, so the test pins the property, not the number).
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let two = int(&mut before, 2);
    let half = before.push(OpKind::Div, [x, two]);
    let two_again = int(&mut before, 2);
    before.push(OpKind::Mul, [half, two_again]);

    let mut after = Body::new();
    arg(&mut after, 0);

    match validate(&before, &after) {
        Verdict::Refuted { args: Some(w) } => {
            assert_eq!(w.len(), 1);
            let x = w[0];
            assert!(x % 2 != 0, "only odd inputs refute x/2*2 == x, got {x}");
            assert_ne!((x / 2) * 2, x, "witness must actually disagree");
            assert_witness_differs(&before, &after, &[Attr::Int(x)]);
        }
        other => panic!("expected Refuted with an odd witness, got {other:?}"),
    }
}

#[test]
fn shr_is_floor_not_truncated_division() {
    // x >> 1  is NOT  x / 2: they differ on negative odd x (floor vs
    // truncation). The two encodings are distinct on purpose; the
    // validator must refute this plausible-looking "strength reduction".
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let one = int(&mut before, 1);
    before.push(OpKind::Shr, [x, one]);

    let mut after = Body::new();
    let x2 = arg(&mut after, 0);
    let two = int(&mut after, 2);
    after.push(OpKind::Div, [x2, two]);

    match validate(&before, &after) {
        Verdict::Refuted { args: Some(w) } => {
            let x = w[0];
            assert!(x < 0 && x % 2 != 0, "only negative odd x differ, got {x}");
            assert_ne!(x >> 1, x / 2);
            assert_witness_differs(&before, &after, &[Attr::Int(x)]);
        }
        other => panic!("expected Refuted, got {other:?}"),
    }
}

#[test]
fn not_eq_equals_ne() {
    // before: !(a == b)        after: a != b
    // Pure boolean/comparison body: the proof carries no overflow caveat.
    let mut before = Body::new();
    let a = arg(&mut before, 0);
    let b = arg(&mut before, 1);
    let eq = before.push(OpKind::Eq, [a, b]);
    before.push(OpKind::Not, [eq]);

    let mut after = Body::new();
    let a2 = arg(&mut after, 0);
    let b2 = arg(&mut after, 1);
    after.push(OpKind::Ne, [a2, b2]);

    assert_eq!(
        validate(&before, &after),
        Verdict::Proven {
            modulo_overflow: false
        }
    );
}

#[test]
fn conflicting_argument_sorts_are_unsupported() {
    // before uses arg0 as an integer (x + 1); after uses it as a boolean
    // (x && true). Sort inference must refuse rather than guess.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let one = int(&mut before, 1);
    before.push(OpKind::Add, [x, one]);

    let mut after = Body::new();
    let x2 = arg(&mut after, 0);
    let t = after.push(OpKind::Const(Attr::Bool(true)), []);
    after.push(OpKind::And, [x2, t]);

    match validate(&before, &after) {
        Verdict::Unsupported { reason } => assert!(
            reason.contains("both an integer and a boolean"),
            "reason should describe the sort conflict: {reason}"
        ),
        other => panic!("expected Unsupported for a sort conflict, got {other:?}"),
    }
}

#[test]
fn witness_search_can_be_disabled() {
    // Same refutation as sub_one_is_not_add_one, but with witness recovery
    // turned off: still Refuted, just without a concrete witness. `None`
    // is the contract of `without_witness_search`, not a missed witness.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let one = int(&mut before, 1);
    before.push(OpKind::Sub, [x, one]);

    let mut after = Body::new();
    let x2 = arg(&mut after, 0);
    let one2 = int(&mut after, 1);
    after.push(OpKind::Add, [x2, one2]);

    verify_body(&before).unwrap();
    verify_body(&after).unwrap();
    assert_eq!(
        Validator::new()
            .without_witness_search()
            .validate(&before, &after),
        Verdict::Refuted { args: None }
    );
}

#[test]
fn single_argument_witness_comes_from_the_model() {
    // before: x == 123456789        after: false
    // Exactly one input refutes this, and it is nowhere near the small
    // values a bounded probe tries: only the solver's model can name it.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let k = int(&mut before, 123_456_789);
    before.push(OpKind::Eq, [x, k]);

    let mut after = Body::new();
    after.push(OpKind::Const(Attr::Bool(false)), []);

    let verdict = validate(&before, &after);
    assert_eq!(
        verdict,
        Verdict::Refuted {
            args: Some(vec![123_456_789])
        }
    );
    assert_witness_differs(&before, &after, &[Attr::Int(123_456_789)]);
}

#[test]
fn multi_argument_witness_is_confirmed() {
    // before: a + b        after: a - b   -- differ unless 2b wraps to 0.
    let mut before = Body::new();
    let a = arg(&mut before, 0);
    let b = arg(&mut before, 1);
    before.push(OpKind::Add, [a, b]);

    let mut after = Body::new();
    let a2 = arg(&mut after, 0);
    let b2 = arg(&mut after, 1);
    after.push(OpKind::Sub, [a2, b2]);

    match validate(&before, &after) {
        Verdict::Refuted { args: Some(w) } => {
            assert_eq!(w.len(), 2, "two arguments, two witness values");
            assert_ne!(w[1], 0, "b == 0 makes the bodies agree, got {w:?}");
            assert_ne!(w[0].wrapping_add(w[1]), w[0].wrapping_sub(w[1]));
            assert_witness_differs(&before, &after, &[Attr::Int(w[0]), Attr::Int(w[1])]);
        }
        other => panic!("expected Refuted with a witness, got {other:?}"),
    }
}

#[test]
fn mixed_bool_and_int_witness_is_confirmed() {
    // before: if (c) { a } else { b }        after: a
    // Differs exactly when c is false and a != b; exercises the 0/1
    // encoding of a boolean argument next to integer ones.
    let mut then_region = Body::new();
    arg(&mut then_region, 1);
    let mut else_region = Body::new();
    arg(&mut else_region, 2);

    let mut before = Body::new();
    let c = arg(&mut before, 0);
    before.push_with_regions(
        OpKind::If,
        [c],
        [Region::new(then_region), Region::new(else_region)],
    );

    let mut after = Body::new();
    arg(&mut after, 1);

    match validate(&before, &after) {
        Verdict::Refuted { args: Some(w) } => {
            assert_eq!(w.len(), 3);
            assert_eq!(w[0], 0, "only c == false refutes, got {w:?}");
            assert_ne!(w[1], w[2], "a == b makes the bodies agree, got {w:?}");
            assert_witness_differs(
                &before,
                &after,
                &[Attr::Bool(w[0] != 0), Attr::Int(w[1]), Attr::Int(w[2])],
            );
        }
        other => panic!("expected Refuted with a witness, got {other:?}"),
    }
}

#[test]
fn unreferenced_arguments_are_padded_with_zero() {
    // Only Arg(2) is referenced: the witness is still indexed by argument
    // number, with the untouched slots at 0.
    let mut before = Body::new();
    let x = arg(&mut before, 2);
    let one = int(&mut before, 1);
    before.push(OpKind::Add, [x, one]);

    let mut after = Body::new();
    arg(&mut after, 2);

    match validate(&before, &after) {
        Verdict::Refuted { args: Some(w) } => {
            assert_eq!(w.len(), 3);
            assert_eq!((w[0], w[1]), (0, 0));
            assert_witness_differs(
                &before,
                &after,
                &[Attr::Int(w[0]), Attr::Int(w[1]), Attr::Int(w[2])],
            );
        }
        other => panic!("expected Refuted with a witness, got {other:?}"),
    }
}

#[test]
fn out_of_range_model_value_is_not_reported_as_a_witness() {
    // before: x > i64::MAX        after: false
    // The unbounded model refutes this with x >= 2^63. No i64 is that
    // large: on every real input both bodies are false, so ANY reported
    // witness would be a lie. The verdict stays Refuted (the validator
    // does not bound argument variables -- see the crate docs), but
    // without a witness.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let max = int(&mut before, i64::MAX);
    before.push(OpKind::Gt, [x, max]);

    let mut after = Body::new();
    after.push(OpKind::Const(Attr::Bool(false)), []);

    assert_eq!(validate(&before, &after), Verdict::Refuted { args: None });

    // The claim above, checked at the boundary rather than assumed.
    for x in [i64::MIN, -1, 0, 1, i64::MAX] {
        assert_eq!(
            eval(&before, &[Attr::Int(x)]),
            eval(&after, &[Attr::Int(x)])
        );
    }
}

#[test]
fn overflow_only_disagreement_is_not_reported_as_a_witness() {
    // before: (x * 2) > i64::MAX        after: false
    // Unbounded, x >= 2^62 refutes it -- and 2^62 is a perfectly good
    // i64, so the model value may well be in range. But the real x * 2
    // wraps and is never above i64::MAX: the bodies agree on every input.
    // Only re-evaluating the candidate catches that.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let two = int(&mut before, 2);
    let doubled = before.push(OpKind::Mul, [x, two]);
    let max = int(&mut before, i64::MAX);
    before.push(OpKind::Gt, [doubled, max]);

    let mut after = Body::new();
    after.push(OpKind::Const(Attr::Bool(false)), []);

    assert_eq!(validate(&before, &after), Verdict::Refuted { args: None });

    for x in [i64::MIN, -1, 0, 1, 1 << 62, (1 << 62) + 1, i64::MAX] {
        assert_eq!(
            eval(&before, &[Attr::Int(x)]),
            eval(&after, &[Attr::Int(x)])
        );
    }
}

#[test]
fn equivalent_pair_still_validates() {
    // before: x == 123456789        after: !(x != 123456789)
    // The same shape as the single-argument refutation above, but
    // actually equivalent: reading models must not have loosened proofs.
    let mut before = Body::new();
    let x = arg(&mut before, 0);
    let k = int(&mut before, 123_456_789);
    before.push(OpKind::Eq, [x, k]);

    let mut after = Body::new();
    let x2 = arg(&mut after, 0);
    let k2 = int(&mut after, 123_456_789);
    let ne = after.push(OpKind::Ne, [x2, k2]);
    after.push(OpKind::Not, [ne]);

    assert_eq!(
        validate(&before, &after),
        Verdict::Proven {
            modulo_overflow: false
        }
    );
}

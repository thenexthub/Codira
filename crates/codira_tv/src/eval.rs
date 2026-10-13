//! Copyright (c) 2026 Omnira CJSC. All Rights Reserved.
//!
//! Concrete evaluation of a loop-free [`Body`] on given argument values,
//! under the **real** semantics (`codira_mir::fold_op`: wrapping `i64`,
//! checked division and shifts) rather than the solver's unbounded model.
//!
//! This is what makes a reported counterexample trustworthy. A solver
//! model only says the two bodies disagree *in the encoding*; the encoding
//! is exact only where no intermediate value overflows `i64`. Running both
//! bodies here, on the model's argument values, is the check that the
//! disagreement survives in the semantics the compiler actually emits.
//!
//! `codira_comptime` has a full interpreter, but it deliberately refuses a
//! top-level `core.arg` (a runtime argument has no compile-time value), so
//! it cannot run a body on witness inputs without wrapping it in a
//! generator call. This evaluator covers exactly the validated fragment
//! instead, with the same evaluation model: every op of a region is
//! evaluated once, in order (so a dead op that errors is still an error),
//! and `cf.if` evaluates only the branch taken. Every pure op delegates to
//! [`fold_op`], so there is no second definition of the arithmetic here.

use codira_mir::{fold_op, Attr, Body, OpId, OpKind};
use rustc_hash::FxHashMap;

/// Evaluates `body` with `Arg(i)` bound to `args[i]`.
///
/// `None` means the body has no value on these inputs: an op failed
/// (`fold_op` error -- division overflow, a sort mismatch, ...), an
/// argument index is out of range, or the body leaves the fragment. The
/// caller treats that as "nothing confirmed", never as a result.
pub(crate) fn eval_body(body: &Body, args: &[Attr]) -> Option<Attr> {
    let mut values: FxHashMap<OpId, Attr> = FxHashMap::default();
    for (id, op) in body.iter() {
        let value = match &op.kind {
            OpKind::Arg(i) => args.get(*i as usize)?.clone(),
            OpKind::If => {
                let [cond] = op.operands.as_slice() else {
                    return None;
                };
                let [then_region, else_region] = op.regions.as_slice() else {
                    return None;
                };
                let region = match values.get(cond)? {
                    Attr::Bool(true) => then_region,
                    Attr::Bool(false) => else_region,
                    _ => return None,
                };
                eval_body(&region.body, args)?
            }
            // Constants and every pure op. Loops, calls, tuples, casts and
            // references come back `NotFoldable`, which lands in the same
            // `None` as a genuine evaluation error.
            kind => {
                let operands = op
                    .operands
                    .iter()
                    .map(|operand| values.get(operand).cloned())
                    .collect::<Option<Vec<_>>>()?;
                fold_op(kind, &operands).ok()?
            }
        };
        values.insert(id, value);
    }
    values.remove(&body.result()?)
}

#[cfg(test)]
mod tests {
    use codira_mir::Region;

    use super::*;

    fn int(body: &mut Body, v: i64) -> OpId {
        body.push(OpKind::Const(Attr::Int(v)), [])
    }

    #[test]
    fn arithmetic_wraps_like_fold() {
        // x + 1 at i64::MAX wraps to i64::MIN: the real semantics, not the
        // unbounded model's 2^63.
        let mut body = Body::new();
        let x = body.push(OpKind::Arg(0), []);
        let one = int(&mut body, 1);
        body.push(OpKind::Add, [x, one]);
        assert_eq!(
            eval_body(&body, &[Attr::Int(i64::MAX)]),
            Some(Attr::Int(i64::MIN))
        );
    }

    #[test]
    fn if_evaluates_only_the_taken_branch() {
        // if (c) { 7 } else { 1 / (x + 1) }: at x = -1 the else branch
        // divides by zero, which must only matter when c is false.
        let mut then_body = Body::new();
        int(&mut then_body, 7);
        let mut else_body = Body::new();
        let one = int(&mut else_body, 1);
        let x = else_body.push(OpKind::Arg(1), []);
        let x_plus_one = else_body.push(OpKind::Add, [x, one]);
        else_body.push(OpKind::Div, [one, x_plus_one]);

        let mut body = Body::new();
        let c = body.push(OpKind::Arg(0), []);
        body.push_with_regions(
            OpKind::If,
            [c],
            [Region::new(then_body), Region::new(else_body)],
        );

        assert_eq!(
            eval_body(&body, &[Attr::Bool(true), Attr::Int(-1)]),
            Some(Attr::Int(7))
        );
        assert_eq!(eval_body(&body, &[Attr::Bool(false), Attr::Int(-1)]), None);
        assert_eq!(
            eval_body(&body, &[Attr::Bool(false), Attr::Int(0)]),
            Some(Attr::Int(1))
        );
    }

    #[test]
    fn errors_and_missing_arguments_have_no_value() {
        // x / -1 at i64::MIN is fold's DivisionOverflow.
        let mut body = Body::new();
        let x = body.push(OpKind::Arg(0), []);
        let minus_one = int(&mut body, -1);
        body.push(OpKind::Div, [x, minus_one]);
        assert_eq!(eval_body(&body, &[Attr::Int(i64::MIN)]), None);
        assert_eq!(eval_body(&body, &[Attr::Int(6)]), Some(Attr::Int(-6)));
        assert_eq!(eval_body(&body, &[]), None, "Arg(0) is unbound");
        assert_eq!(
            eval_body(&body, &[Attr::Bool(true)]),
            None,
            "a boolean fed to Div is a sort mismatch, not a value"
        );
        assert_eq!(eval_body(&Body::new(), &[]), None, "empty body");
    }
}

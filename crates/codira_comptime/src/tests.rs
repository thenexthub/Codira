use codira_mir::{Attr, Body, OpKind, Region};

use crate::{eval_body, EvalError, Env, Value};

#[test]
fn comptime_two_plus_two_is_four() {
    // The literal example from spec/KGEN_SUPERSET_ARCHITECTURE.md §5:
    // `comptime { 2 + 2 }` should evaluate to 4 at compile time.
    let mut body = Body::new();
    let a = body.push(OpKind::Const(Attr::Int(2)), []);
    let b = body.push(OpKind::Const(Attr::Int(2)), []);
    body.push(OpKind::Add, [a, b]);

    let result = eval_body(&body, &Env::new()).unwrap();
    assert_eq!(result, Value::Int(4));
}

#[test]
fn comptime_arithmetic_precedence_via_explicit_ops() {
    // `(3 * 4) - 5`
    let mut body = Body::new();
    let three = body.push(OpKind::Const(Attr::Int(3)), []);
    let four = body.push(OpKind::Const(Attr::Int(4)), []);
    let mul = body.push(OpKind::Mul, [three, four]);
    let five = body.push(OpKind::Const(Attr::Int(5)), []);
    body.push(OpKind::Sub, [mul, five]);

    assert_eq!(eval_body(&body, &Env::new()).unwrap(), Value::Int(7));
}

#[test]
fn comptime_if_picks_then_branch() {
    // `if 1 < 2 { 10 } else { 20 }`
    let mut body = Body::new();
    let one = body.push(OpKind::Const(Attr::Int(1)), []);
    let two = body.push(OpKind::Const(Attr::Int(2)), []);
    let cond = body.push(OpKind::Lt, [one, two]);

    let mut then_body = Body::new();
    then_body.push(OpKind::Const(Attr::Int(10)), []);
    let mut else_body = Body::new();
    else_body.push(OpKind::Const(Attr::Int(20)), []);

    body.push_with_regions(
        OpKind::If,
        [cond],
        [Region::new(then_body), Region::new(else_body)],
    );

    assert_eq!(eval_body(&body, &Env::new()).unwrap(), Value::Int(10));
}

#[test]
fn comptime_if_picks_else_branch() {
    // `if false { 10 } else { 20 }`
    let mut body = Body::new();
    let cond = body.push(OpKind::Const(Attr::Bool(false)), []);
    let mut then_body = Body::new();
    then_body.push(OpKind::Const(Attr::Int(10)), []);
    let mut else_body = Body::new();
    else_body.push(OpKind::Const(Attr::Int(20)), []);

    body.push_with_regions(
        OpKind::If,
        [cond],
        [Region::new(then_body), Region::new(else_body)],
    );

    assert_eq!(eval_body(&body, &Env::new()).unwrap(), Value::Int(20));
}

#[test]
fn generic_param_specialization_binds_n() {
    // Elaborating `SIMD[f32, N]`'s hypothetical bounds-check body with N=4:
    // `N == 4`
    let mut body = Body::new();
    let n_ref = body.push(OpKind::ParamRef("N".into()), []);
    let four = body.push(OpKind::Const(Attr::Int(4)), []);
    body.push(OpKind::Eq, [n_ref, four]);

    let mut env = Env::new();
    env.bind("N", Value::Int(4));

    assert_eq!(eval_body(&body, &env).unwrap(), Value::Bool(true));
}

#[test]
fn unbound_param_ref_is_a_clean_error_not_a_panic() {
    let mut body = Body::new();
    body.push(OpKind::ParamRef("T".into()), []);

    let err = eval_body(&body, &Env::new()).unwrap_err();
    assert_eq!(err, EvalError::UnresolvedParam("T".into()));
}

#[test]
fn division_by_zero_is_a_clean_error_not_a_panic() {
    let mut body = Body::new();
    let one = body.push(OpKind::Const(Attr::Int(1)), []);
    let zero = body.push(OpKind::Const(Attr::Int(0)), []);
    body.push(OpKind::Div, [one, zero]);

    assert_eq!(eval_body(&body, &Env::new()).unwrap_err(), EvalError::DivideByZero);
}

#[test]
fn loops_are_honestly_unsupported_not_silently_wrong() {
    let mut body = Body::new();
    body.push_with_regions(OpKind::For, [], []);

    assert_eq!(
        eval_body(&body, &Env::new()).unwrap_err(),
        EvalError::UnsupportedOp("cf.for")
    );
}

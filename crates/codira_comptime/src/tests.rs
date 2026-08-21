use codira_mir::{Attr, Body, Generator, GeneratorParam, OpKind, Region};

use crate::{elaborate, eval_body, EvalError, Env, Value};

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

// ---------------------------------------------------------------------------
// Elaborator tests
// ---------------------------------------------------------------------------

#[test]
fn elaboration_preserves_runtime_argument() {
    // `fn add[N](x) = x + N`, elaborated with N=5.
    // The runtime argument `x` must survive elaboration untouched -- it is
    // not something elaboration is allowed to specialize away.
    let mut body = Body::new();
    let x = body.push(OpKind::Arg(0), []);
    let n = body.push(OpKind::ParamRef("N".into()), []);
    body.push(OpKind::Add, [x, n]);

    let generator = Generator {
        name: "add".into(),
        params: vec![GeneratorParam { name: "N".into() }],
        body,
    };

    let elaborated = elaborate(&generator, &[("N".into(), Value::Int(5))]);

    // The elaborated body still genuinely depends on a runtime argument:
    // evaluating it with no argument binding must fail with
    // RuntimeArgument, not silently produce a wrong constant.
    assert_eq!(
        eval_body(&elaborated, &Env::new()).unwrap_err(),
        EvalError::RuntimeArgument(0)
    );
}

#[test]
fn elaboration_folds_pure_generator_to_a_single_constant() {
    // `fn double[N]() = N * 2`, elaborated with N=5 -- no runtime
    // arguments at all, so the *entire* body should collapse to one
    // `Const(10)` op instead of retaining the multiplication. This is the
    // concrete "does less work than naive substitution" claim from the
    // module doc / architecture doc §7.
    let mut body = Body::new();
    let n = body.push(OpKind::ParamRef("N".into()), []);
    let two = body.push(OpKind::Const(Attr::Int(2)), []);
    body.push(OpKind::Mul, [n, two]);

    let generator = Generator {
        name: "double".into(),
        params: vec![GeneratorParam { name: "N".into() }],
        body,
    };

    let elaborated = elaborate(&generator, &[("N".into(), Value::Int(5))]);

    assert_eq!(eval_body(&elaborated, &Env::new()).unwrap(), Value::Int(10));
    // The performance claim, checked structurally: the naive substitution
    // would still contain 3 ops (ParamRef-turned-Const, the literal 2, and
    // the Mul); constant folding collapses that to exactly 1.
    assert_eq!(elaborated.iter().count(), 1);
}

#[test]
fn elaboration_inlines_the_taken_if_branch() {
    // `fn pick[Flag]() = if Flag { 1 } else { 2 + 2 }`, elaborated with
    // Flag=true. The whole `if` should disappear -- only the `then`
    // branch's (folded) value should remain reachable.
    let mut body = Body::new();
    let flag = body.push(OpKind::ParamRef("Flag".into()), []);

    let mut then_body = Body::new();
    then_body.push(OpKind::Const(Attr::Int(1)), []);

    let mut else_body = Body::new();
    let a = else_body.push(OpKind::Const(Attr::Int(2)), []);
    let b = else_body.push(OpKind::Const(Attr::Int(2)), []);
    else_body.push(OpKind::Add, [a, b]);

    body.push_with_regions(
        OpKind::If,
        [flag],
        [Region::new(then_body), Region::new(else_body)],
    );

    let generator = Generator {
        name: "pick".into(),
        params: vec![GeneratorParam { name: "Flag".into() }],
        body,
    };

    let elaborated = elaborate(&generator, &[("Flag".into(), Value::Bool(true))]);

    assert_eq!(eval_body(&elaborated, &Env::new()).unwrap(), Value::Int(1));
    // Only the taken branch's single op should have been spliced in --
    // the untaken `else` branch's ops must not appear in the result body.
    assert_eq!(elaborated.iter().count(), 1);
}

#[test]
fn elaboration_supports_partial_specialization() {
    // Binding nothing leaves `param.ref` exactly as written -- elaboration
    // with an empty binding set must not fabricate a value for an
    // unresolved parameter.
    let mut body = Body::new();
    body.push(OpKind::ParamRef("T".into()), []);

    let generator = Generator {
        name: "identity".into(),
        params: vec![GeneratorParam { name: "T".into() }],
        body,
    };

    let elaborated = elaborate(&generator, &[]);

    assert_eq!(
        eval_body(&elaborated, &Env::new()).unwrap_err(),
        EvalError::UnresolvedParam("T".into())
    );
}

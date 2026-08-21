use codira_hir_input::WithFixture;
use codira_mir::OpKind;

use crate::{
    mock::MockDatabase, mir_lower::lower_function_to_generator, HirDatabase, ModuleDef, Package,
};

/// Parses `content`, finds the first function declared in it, and lowers
/// it to a `codira_mir::Generator`. Panics (via the trailing `.unwrap()`
/// left to callers) if there's no function or lowering bails -- tests
/// below assert on the `Option` directly where a `None` is expected.
fn lower_first_fn(content: &str) -> Option<codira_mir::Generator> {
    let db = MockDatabase::with_files(content);
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) => Some(f),
            _ => None,
        })
        .expect("no function found in source");
    lower_function_to_generator(&db, func)
}

#[test]
fn lowers_generic_param_reference() {
    // `func add[N](x: i64) -> i64 { x + N }`
    let generator = lower_first_fn("func add[N](x: i64) -> i64 { x + N }")
        .expect("expected a lowerable generator");

    assert_eq!(generator.name, "add");
    assert_eq!(generator.params.len(), 1);
    assert_eq!(generator.params[0].name, "N");

    // The body should be exactly one Add op referencing an Arg and a
    // ParamRef -- i.e. real, not just "didn't crash".
    let result = generator.body.result().expect("body has a result");
    let op = generator.body.get(result);
    assert_eq!(op.kind, OpKind::Add);
    assert_eq!(op.operands.len(), 2);

    let lhs_kind = &generator.body.get(op.operands[0]).kind;
    let rhs_kind = &generator.body.get(op.operands[1]).kind;
    assert_eq!(*lhs_kind, OpKind::Arg(0));
    assert_eq!(*rhs_kind, OpKind::ParamRef("N".into()));
}

#[test]
fn lowers_let_bindings_and_if() {
    // `func clamp_low[Lo](x: i64) -> i64 { let y = x; if y < Lo { Lo } else { y } }`
    let generator = lower_first_fn(
        "func clamp_low[Lo](x: i64) -> i64 { let y = x; if y < Lo { Lo } else { y } }",
    )
    .expect("expected a lowerable generator");

    let result = generator.body.result().expect("body has a result");
    let op = generator.body.get(result);
    assert_eq!(op.kind, OpKind::If);
    assert_eq!(op.regions.len(), 2);
    // `then` branch is just `Lo` -> a single ParamRef op.
    let then_result = op.regions[0].body.result().expect("then has a result");
    assert_eq!(
        op.regions[0].body.get(then_result).kind,
        OpKind::ParamRef("Lo".into())
    );
    // `else` branch is `y`, which was let-bound to `x` (Arg(0)).
    let else_result = op.regions[1].body.result().expect("else has a result");
    assert_eq!(op.regions[1].body.get(else_result).kind, OpKind::Arg(0));
}

#[test]
fn bails_out_on_function_calls() {
    // Calling another function needs a `Call` op codira_mir doesn't have
    // yet -- this must return `None`, not a wrong/partial generator.
    let generator = lower_first_fn(
        r#"
        func helper() -> i64 { 1 }
        func uses_helper() -> i64 { helper() }
        "#,
    );
    // `lower_first_fn` finds the *first* declared function (`helper`,
    // which lowers fine); assert on the second explicitly instead.
    assert!(generator.is_some(), "helper() itself should lower fine");

    let db = MockDatabase::with_files(
        r#"
        func helper() -> i64 { 1 }
        func uses_helper() -> i64 { helper() }
        "#,
    );
    let uses_helper = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) if f.name(&db).to_string() == "uses_helper" => Some(f),
            _ => None,
        })
        .expect("uses_helper not found");
    assert!(lower_function_to_generator(&db, uses_helper).is_none());
}

// ---------------------------------------------------------------------------
// Salsa query tests: HirDatabase::mir_generator / elaborate_generator
// ---------------------------------------------------------------------------

#[test]
fn elaborate_generator_query_produces_correct_result() {
    // `func double[N]() -> i64 { N * 2 }`, elaborated with N=21 via the
    // salsa query (not the bare `codira_comptime::elaborate` function
    // directly) should fully constant-fold to 42, same as the
    // hand-built-Generator tests in codira_comptime itself -- this is the
    // proof the query wiring doesn't change the answer.
    let db = MockDatabase::with_files("func double[N]() -> i64 { N * 2 }");
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) => Some(f),
            _ => None,
        })
        .expect("no function found");

    let bindings = vec![("N".into(), codira_comptime::Value::Int(21))];
    let body = db
        .elaborate_generator(func, bindings)
        .expect("expected an elaborated body");

    assert_eq!(
        codira_comptime::eval_body(&body, &codira_comptime::Env::new()).unwrap(),
        codira_comptime::Value::Int(42)
    );
}

#[test]
fn elaborate_generator_query_is_memoized() {
    // The concrete "incremental caching" claim from architecture doc
    // §4.1: calling the salsa query twice with identical inputs must
    // return the *same* Arc allocation (a cache hit), not recompute and
    // allocate a fresh one.
    let db = MockDatabase::with_files("func double[N]() -> i64 { N * 2 }");
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) => Some(f),
            _ => None,
        })
        .expect("no function found");

    let bindings = vec![("N".into(), codira_comptime::Value::Int(21))];
    let first = db
        .elaborate_generator(func, bindings.clone())
        .expect("expected an elaborated body");
    let second = db
        .elaborate_generator(func, bindings)
        .expect("expected an elaborated body");

    assert!(
        std::sync::Arc::ptr_eq(&first, &second),
        "salsa should have served the second call from cache instead of recomputing"
    );
}

#[test]
fn elaborate_generator_query_distinguishes_different_bindings() {
    // A different cache key (different N) must produce a genuinely
    // different, correctly-computed result -- not an incorrectly reused
    // cache entry from a different specialization.
    let db = MockDatabase::with_files("func double[N]() -> i64 { N * 2 }");
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) => Some(f),
            _ => None,
        })
        .expect("no function found");

    let body_21 = db
        .elaborate_generator(func, vec![("N".into(), codira_comptime::Value::Int(21))])
        .unwrap();
    let body_5 = db
        .elaborate_generator(func, vec![("N".into(), codira_comptime::Value::Int(5))])
        .unwrap();

    assert_eq!(
        codira_comptime::eval_body(&body_21, &codira_comptime::Env::new()).unwrap(),
        codira_comptime::Value::Int(42)
    );
    assert_eq!(
        codira_comptime::eval_body(&body_5, &codira_comptime::Env::new()).unwrap(),
        codira_comptime::Value::Int(10)
    );
}

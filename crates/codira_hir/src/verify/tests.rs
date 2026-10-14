//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - End-to-end tests of contract verification: source text in, Z3 verdicts and
//!   diagnostics out.

use codira_hir_input::WithFixture;

use super::{ObligationKind, Value, Verdict, VerificationReport};
use crate::{mock::MockDatabase, utils::tests::diagnostics, HirDatabase, ModuleDef, Package};

/// The verification report of the function named `name` in `source`.
fn report(source: &str, name: &str) -> VerificationReport {
    let (db, _) = MockDatabase::with_single_file(source);
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|def| match def {
            ModuleDef::Function(f) if f.name(&db).to_string() == name => Some(f),
            _ => None,
        })
        .unwrap_or_else(|| panic!("no function `{name}` in fixture"));
    (*db.verify_function(func)).clone()
}

fn all_proven(report: &VerificationReport) -> bool {
    !report.obligations.is_empty() && report.failures().next().is_none()
}

fn refuted<'a>(
    report: &'a VerificationReport,
    kind: &ObligationKind,
) -> Option<&'a super::Counterexample> {
    report.obligations.iter().find_map(|o| match &o.verdict {
        Verdict::Refuted(cex) if &o.kind == kind => Some(cex),
        _ => None,
    })
}

fn input(cex: &super::Counterexample, name: &str) -> i128 {
    cex.inputs
        .iter()
        .find(|(n, _)| n == name)
        .and_then(|(_, v)| v.as_i128())
        .unwrap_or_else(|| panic!("no integer input `{name}` in {cex:?}"))
}

#[test]
fn wrapping_negation_refutes_abs_with_the_minimum_value() {
    let r = report(
        r#"
    func abs(x: i64) -> i64
        ensures result >= 0
    {
        if x < 0 { -x } else { x }
    }
    "#,
        "abs",
    );
    let cex = refuted(&r, &ObligationKind::Postcondition).expect("must be refuted");
    assert_eq!(input(cex, "x"), i128::from(i64::MIN));
    assert_eq!(cex.call("abs"), "abs(x: -9223372036854775808)");
    assert_eq!(cex.inputs[0].1.to_source(), "(-9223372036854775807 - 1)");
}

#[test]
fn a_precondition_excluding_the_minimum_proves_abs() {
    let r = report(
        r#"
    func abs(x: i64) -> i64
        requires x > -9223372036854775807 - 1
        ensures result >= 0
    {
        if x < 0 { -x } else { x }
    }
    "#,
        "abs",
    );
    assert!(all_proven(&r), "{r:#?}");
}

#[test]
fn early_returns_are_each_checked_against_the_postcondition() {
    let r = report(
        r#"
    func clamp(x: i64) -> i64
        ensures result >= 0 && result <= 10
    {
        if x < 0 { return 0; }
        if x > 10 { return 10; }
        x
    }
    "#,
        "clamp",
    );
    assert!(all_proven(&r), "{r:#?}");

    let r = report(
        r#"
    func clamp(x: i64) -> i64
        ensures result >= 0 && result <= 10
    {
        if x < 0 { return 0; }
        if x > 11 { return 10; }
        x
    }
    "#,
        "clamp",
    );
    let cex = refuted(&r, &ObligationKind::Postcondition).expect("x = 11 escapes");
    assert_eq!(input(cex, "x"), 11);
}

#[test]
fn increment_overflows_at_the_maximum() {
    let r = report(
        r#"
    func inc(x: i64) -> i64
        ensures result > x
    {
        x + 1
    }
    "#,
        "inc",
    );
    let cex = refuted(&r, &ObligationKind::Postcondition).expect("must be refuted");
    assert_eq!(input(cex, "x"), i128::from(i64::MAX));
}

#[test]
fn unsigned_arithmetic_wraps_at_its_own_width() {
    let r = report(
        r#"
    func bump(x: u8) -> u8
        ensures result >= x
    {
        x + 1
    }
    "#,
        "bump",
    );
    let cex = refuted(&r, &ObligationKind::Postcondition).expect("must be refuted");
    assert_eq!(input(cex, "x"), 255);
}

#[test]
fn call_site_preconditions_use_the_callers_path_condition() {
    let source = r#"
    func div(a: i64, b: i64) -> i64
        requires b != 0
    {
        a / b
    }
    func careless(x: i64) -> i64 {
        div(10, x)
    }
    func careful(x: i64) -> i64 {
        if x != 0 { div(10, x) } else { 0 }
    }
    "#;
    let careless = report(source, "careless");
    let cex = refuted(
        &careless,
        &ObligationKind::Precondition {
            callee: "div".to_string(),
        },
    )
    .expect("x = 0 violates the precondition");
    assert_eq!(input(cex, "x"), 0);
    assert!(cex
        .at_site
        .iter()
        .any(|(n, v)| n == "b" && v.as_i128() == Some(0)));

    assert!(all_proven(&report(source, "careful")));
    // Inside the callee, `requires b != 0` proves the divisor non-zero, but
    // not the other way `a / b` traps: `i64::MIN / -1` overflows.
    let div = report(source, "div");
    assert!(div
        .obligations
        .iter()
        .any(|o| o.kind == ObligationKind::DivisionByZero && o.verdict == Verdict::Proven));
    assert!(refuted(&div, &ObligationKind::DivisionOverflow).is_some());
}

#[test]
fn callee_postconditions_are_assumed_at_the_call() {
    let r = report(
        r#"
    func div(a: i64, b: i64) -> i64
        requires b != 0
    {
        a / b
    }
    func positive(x: i64) -> i64
        ensures result > 0
    {
        if x > 0 { x } else { 1 }
    }
    func caller(x: i64) -> i64 {
        div(10, positive(x))
    }
    "#,
        "caller",
    );
    assert!(all_proven(&r), "{r:#?}");
}

#[test]
fn refined_parameters_are_checked_at_every_call() {
    let source = r#"
    type Positive = i64 { v | v > 0 };
    func takes_alias(x: Positive) -> i64 { x }
    func takes_inline(x: i64 { v | v >= 10 }) -> i64 { x }
    func unchecked(y: i64) -> i64 { takes_alias(y) }
    func checked(y: i64) -> i64 {
        if y > 0 { takes_alias(y) } else { 0 }
    }
    func inline_bad(y: i64) -> i64 {
        if y > 5 { takes_inline(y) } else { 0 }
    }
    "#;
    let r = report(source, "unchecked");
    let cex = refuted(
        &r,
        &ObligationKind::RefinedArgument {
            callee: "takes_alias".to_string(),
            param: "x".to_string(),
        },
    )
    .expect("y = 0 is not positive");
    assert!(input(cex, "y") <= 0);

    assert!(all_proven(&report(source, "checked")));

    let r = report(source, "inline_bad");
    let cex = refuted(
        &r,
        &ObligationKind::RefinedArgument {
            callee: "takes_inline".to_string(),
            param: "x".to_string(),
        },
    )
    .expect("y in 6..10 escapes");
    let y = input(cex, "y");
    assert!((6..10).contains(&y), "{y}");
}

#[test]
fn a_refined_parameter_is_a_fact_inside_its_function() {
    let r = report(
        r#"
    type NonZero = i64 { v | v != 0 };
    func safe_div(a: i64, b: NonZero) -> i64
        ensures true
    {
        a / b
    }
    "#,
        "safe_div",
    );
    // The division-by-zero obligation is proven from the refinement; the
    // signed-overflow one (MIN / -1) is not.
    assert!(r
        .obligations
        .iter()
        .any(|o| o.kind == ObligationKind::DivisionByZero && o.verdict == Verdict::Proven));
    let cex = refuted(&r, &ObligationKind::DivisionOverflow).expect("MIN / -1");
    assert_eq!(input(cex, "a"), i128::from(i64::MIN));
    assert_eq!(input(cex, "b"), -1);
}

#[test]
fn loop_invariants_and_measures_prove_a_summation() {
    let r = report(
        r#"
    func sum_to(n: i64) -> i64
        requires n >= 0 && n <= 1000
        ensures result >= 0
    {
        var i = 0;
        var s = 0;
        while i < n
            invariant i >= 0 && i <= n
            invariant s >= 0 && s <= i * 1000
            decreases n - i
        {
            i += 1;
            s += i;
        }
        s
    }
    "#,
        "sum_to",
    );
    assert!(all_proven(&r), "{r:#?}");
    for kind in [
        ObligationKind::InvariantOnEntry,
        ObligationKind::InvariantPreserved,
        ObligationKind::MeasureNonNegative,
        ObligationKind::MeasureDecreases,
        ObligationKind::Postcondition,
    ] {
        assert!(
            r.obligations.iter().any(|o| o.kind == kind),
            "missing {kind:?}"
        );
    }
}

#[test]
fn a_wrong_invariant_or_measure_is_refuted() {
    let r = report(
        r#"
    func count(n: i64) -> i64
        requires n >= 0 && n <= 100
    {
        var i = 0;
        while i < n invariant i < n decreases n + i {
            i += 1;
        }
        i
    }
    "#,
        "count",
    );
    let entry = refuted(&r, &ObligationKind::InvariantOnEntry).expect("n = 0 breaks `i < n`");
    assert_eq!(input(entry, "n"), 0);
    assert!(refuted(&r, &ObligationKind::InvariantPreserved).is_some());
    assert!(refuted(&r, &ObligationKind::MeasureDecreases).is_some());
}

#[test]
fn a_loop_without_invariant_forgets_what_it_assigns() {
    // After the loop `i` is havocked, so `result == 0` cannot be proven even
    // though it happens to hold for n <= 0.
    let r = report(
        r#"
    func f(n: i64) -> i64
        ensures result == 0
    {
        var i = 0;
        while i < n { i += 1; }
        i
    }
    "#,
        "f",
    );
    assert!(refuted(&r, &ObligationKind::Postcondition).is_some());
}

#[test]
fn ghost_bindings_feed_invariants_and_are_erased() {
    let r = report(
        r#"
    func countdown(n: i64) -> i64
        requires n >= 0
        ensures result == 0
    {
        ghost let start = n;
        var k = n;
        while k > 0 invariant k >= 0 && k <= start decreases k {
            k -= 1;
        }
        k
    }
    "#,
        "countdown",
    );
    assert!(all_proven(&r), "{r:#?}");
}

#[test]
fn ghost_rules_are_enforced() {
    let diags = diagnostics(
        r#"
    func leak(n: i64) -> i64 {
        ghost let g = n;
        g + 1
    }
    func noisy(n: i64) -> i64
        requires helper(n) > 0
    {
        n
    }
    func helper(n: i64) -> i64 { n }
    "#,
    );
    assert!(
        diags.contains("ghost binding `g` used in executable code"),
        "{diags}"
    );
    assert!(
        diags.contains("a call is not allowed in a contract clause or ghost code"),
        "{diags}"
    );
}

#[test]
fn diagnostics_render_counterexamples_in_codira_syntax() {
    let diags = diagnostics(
        r#"
    func div(a: i64, b: i64) -> i64
        requires b != 0
    {
        a / b
    }
    func caller(x: i64) -> i64 {
        div(x, x - 3)
    }
    "#,
    );
    assert!(
        diags.contains(
            "precondition of `div` might not hold: `b != 0` (counterexample: caller(x: 3); \
             at the failing point: a = 3, b = 0)"
        ),
        "{diags}"
    );
}

#[test]
fn uncheckable_clauses_are_errors_not_silent_passes() {
    let diags = diagnostics(
        r#"
    struct P { x: i64 }
    func get(p: P) -> i64
        ensures result == p.x
    {
        p.x
    }
    "#,
    );
    assert!(
        diags.contains("postcondition might not hold: `result == p.x`"),
        "{diags}"
    );
    assert!(diags.contains("contract clauses do not support"), "{diags}");
}

#[test]
fn functions_without_contracts_and_contracted_calls_cost_nothing() {
    let r = report(
        r#"
    func plain(a: i64, b: i64) -> i64 { a / b }
    "#,
        "plain",
    );
    assert!(r.obligations.is_empty());
    // A function with no contract has no division obligation either.
    assert_eq!(r.params.len(), 2);
}

#[test]
fn value_rendering() {
    let min = Value::Int {
        bits: 0x80,
        signed: true,
        width: 8,
    };
    assert_eq!(min.to_string(), "-128");
    assert_eq!(min.to_source(), "(-127 - 1)");
    let unsigned = Value::Int {
        bits: 0xFF,
        signed: false,
        width: 8,
    };
    assert_eq!(unsigned.to_string(), "255");
    assert_eq!(unsigned.to_source(), "255");
}

/// The example in `agents/USING_CODIRA.md` must verify exactly as the guide
/// says it does.
#[test]
fn the_user_guide_example_verifies() {
    let source = r#"
    type Positive = i64 { v | v > 0 }

    func div(a: i64, b: i64) -> i64
        requires b != 0
        requires a != -9223372036854775807 - 1 || b != -1
    {
        a / b
    }

    func half(x: Positive) -> i64
        ensures result >= 0 && result < x
    {
        x / 2
    }

    func sum_to(n: i64) -> i64
        requires n >= 0 && n <= 1000
        ensures result >= 0
    {
        ghost let limit = n
        var i = 0
        var s = 0
        while i < n
            invariant i >= 0 && i <= limit
            invariant s >= 0 && s <= i * 1000
            decreases n - i
        {
            i += 1
            s += i
        }
        s
    }
    "#;
    assert_eq!(diagnostics(source), "");
    assert!(all_proven(&report(source, "div")));
    assert!(all_proven(&report(source, "half")));
    assert!(all_proven(&report(source, "sum_to")));
}

/// 64-bit nonlinear bit-vector arithmetic is beyond the default budget. The
/// verdict must be "unknown (resource limit)" -- an error -- and the same on
/// every run, because the budget counts Z3's work, not time.
#[test]
fn an_exhausted_budget_is_unknown_and_deterministic() {
    let source = r#"
    func div(a: i64, b: i64) -> i64
        requires b != 0
        requires a != -9223372036854775807 - 1 || b != -1
        ensures result * b + a % b == a
    {
        a / b
    }
    "#;
    for _ in 0..2 {
        let r = report(source, "div");
        let post = r
            .obligations
            .iter()
            .find(|o| o.kind == ObligationKind::Postcondition)
            .unwrap();
        assert_eq!(
            post.verdict,
            Verdict::Unknown(super::UnknownCause::ResourceLimit)
        );
    }
    assert!(diagnostics(source).contains(
        "postcondition might not hold: `result * b + a % b == a`: unknown (resource limit)"
    ));
}

fn generated(source: &str, name: &str) -> Option<super::testgen::GeneratedTest> {
    let (db, _) = MockDatabase::with_single_file(source);
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|def| match def {
            ModuleDef::Function(f) if f.name(&db).to_string() == name => Some(f),
            _ => None,
        })
        .unwrap();
    super::testgen::counterexample_test(&db, func)
}

#[test]
fn a_refuted_postcondition_becomes_a_checking_test() {
    let test = generated(
        r#"
    func abs(x: i64) -> i64
        ensures result >= 0
    {
        if x < 0 { -x } else { x }
    }
    "#,
        "abs",
    )
    .expect("a test is generated");
    assert_eq!(test.file_name, "abs_counterexample.code");
    assert!(test.module_path.is_empty());
    assert!(test.contents.starts_with(super::testgen::GENERATED_MARKER));
    assert!(test.contents.contains(
        "import super.abs;\n\n@test\npublic func abs_counterexample() -> bool {\n    let x = (-9223372036854775807 - 1);\n    let result = abs(x);\n    result >= 0\n}\n"
    ), "{}", test.contents);
    assert!(test
        .contents
        .contains("// Counterexample: abs(x: -9223372036854775808)"));
    assert!(
        test.contents.contains("//   at mod.code:2:13"),
        "{}",
        test.contents
    );
}

#[test]
fn a_refuted_call_site_becomes_a_call_test() {
    let test = generated(
        r#"
    func div(a: i64, b: i64) -> i64
        requires b != 0
    {
        a / b
    }
    func caller(x: i64) -> i64 {
        div(10, x)
    }
    "#,
        "caller",
    )
    .expect("a test is generated");
    assert!(
        test.contents
            .contains("public func caller_counterexample() {\n    let x = 0;\n    caller(x);\n}\n"),
        "{}",
        test.contents
    );
    assert!(test
        .contents
        .contains("// At the failing point: a = 10, b = 0"));
}

#[test]
fn proven_functions_and_unmodelled_signatures_get_no_test() {
    assert!(generated(
        r#"
    func ok(x: i64) -> i64
        requires x > 0
        ensures result > 0
    {
        x
    }
    "#,
        "ok",
    )
    .is_none());
    assert!(generated(
        r#"
    struct P { x: i64 }
    func f(p: P, y: i64) -> i64
        ensures result > y
    {
        y
    }
    "#,
        "f",
    )
    .is_none());
}

#[test]
fn generated_file_names_round_trip_their_module_path() {
    use super::testgen::{file_name, module_path_of_file};
    let path = vec!["math".to_string(), "geometry".to_string()];
    let name = file_name(&path, "area");
    assert_eq!(name, "math.geometry.area_counterexample.code");
    assert_eq!(module_path_of_file(&name), Some(path));
    assert_eq!(module_path_of_file("abs_counterexample.code"), Some(vec![]));
    assert_eq!(module_path_of_file("notes.code"), None);
}

#[test]
fn test_functions_must_have_a_callable_shape() {
    let diags = diagnostics(
        r#"
    @test
    public func good() -> bool { true }
    @test
    public func also_good() { }
    @test
    func private_test() { }
    @test
    public func takes(x: i64) -> bool { x > 0 }
    @test
    public func returns_int() -> i64 { 1 }
    "#,
    );
    assert!(!diags.contains("`good`"), "{diags}");
    assert_eq!(
        diags.matches("invalid `@test` function").count(),
        3,
        "{diags}"
    );
    assert!(diags.contains("must be `public`"));
    assert!(diags.contains("takes parameters"));
    assert!(diags.contains("must return `()` or `bool`"));
}

fn run_query(source: &str, predicate: &str) -> super::query::QueryOutcome {
    let (db, _) = MockDatabase::with_single_file(source);
    super::query::query(&db, predicate).unwrap()
}

#[test]
fn query_finds_functions_that_can_return_negatives_from_their_bodies() {
    let source = r#"
    func abs(x: i64) -> i64 { if x < 0 { -x } else { x } }
    func square_ish(x: i64) -> i64
        requires x >= 0 && x <= 1000
    {
        x * x
    }
    func clamp_low(x: i64) -> i64 { if x < 0 { 0 } else { x } }
    func negate(x: i64) -> i64 { -x }
    func flag(b: bool) -> bool { b }
    "#;
    let outcome = run_query(source, "result < 0");
    let names: Vec<String> = outcome
        .matches
        .iter()
        .map(|m| m.signature.clone())
        .collect();
    // `abs` can: -i64::MIN wraps to itself. `negate` obviously can.
    // `square_ish` and `clamp_low` cannot, and `flag` returns a bool.
    assert_eq!(
        names,
        ["func abs(x: i64) -> i64", "func negate(x: i64) -> i64"],
        "{outcome:#?}"
    );
    let abs = &outcome.matches[0];
    assert_eq!(abs.inputs[0].1.as_i128(), Some(i128::from(i64::MIN)));
    assert_eq!(
        abs.result.and_then(|v| v.as_i128()),
        Some(i128::from(i64::MIN))
    );
    assert_eq!(abs.location, "mod.code:1");
    assert_eq!(
        outcome.not_applicable, 1,
        "only `flag` compares a bool to 0"
    );
}

#[test]
fn query_can_relate_result_and_parameters() {
    let source = r#"
    func inc(x: i64) -> i64 { x + 1 }
    func same(x: i64) -> i64 { x }
    func other(y: i64) -> i64 { y }
    "#;
    let outcome = run_query(source, "result < x");
    let names: Vec<&str> = outcome
        .matches
        .iter()
        .map(|m| m.signature.as_str())
        .collect();
    assert_eq!(names, ["func inc(x: i64) -> i64"], "wrapping at i64::MAX");
    assert_eq!(outcome.not_applicable, 1, "`other` has no `x`");
}

#[test]
fn query_rejects_a_malformed_predicate() {
    let (db, _) = MockDatabase::with_single_file("func f() {}");
    assert!(super::query::query(&db, "result <").is_err());
}

// ---- information flow (`verify::flow`) -------------------------------------

#[test]
fn secret_values_may_not_reach_public_sinks() {
    let diags = diagnostics(
        r#"
    func leak(key: secret u64) -> u64 {
        key + 1
    }
    func ok(key: secret u64) -> secret u64 {
        key + 1
    }
    func revealed(key: secret u64) -> u64 {
        declassify(key & 1)
    }
    func takes_public(x: u64) -> u64 { x }
    func passes(key: secret u64) -> u64 {
        takes_public(declassify(key));
        takes_public(key)
    }
    func printed(key: secret u64) {
        println(key)
    }
    func bound(key: secret u64) {
        let copy = key;
        let public_copy: u64 = copy;
    }
    "#,
    );
    let count = |needle: &str| diags.matches(needle).count();
    assert_eq!(
        count("secret value returned from a function whose result is not `secret`"),
        1,
        "{diags}"
    );
    assert_eq!(
        count("secret value passed to a non-`secret` parameter of `takes_public`"),
        1,
        "{diags}"
    );
    assert_eq!(count("secret value printed"), 1, "{diags}");
    assert_eq!(
        count("secret value bound by a `let` declared without `secret`"),
        1,
        "{diags}"
    );
    assert_eq!(diags.lines().count(), 4, "{diags}");
}

#[test]
fn implicit_flows_through_branches_are_tracked() {
    let diags = diagnostics(
        r#"
    func implicit(key: secret bool) -> i64 {
        var x = 0;
        if key { x = 1; }
        x
    }
    "#,
    );
    assert!(
        diags.contains("secret value returned from a function whose result is not `secret`"),
        "{diags}"
    );
}

#[test]
fn constant_time_functions_may_not_branch_index_or_divide_on_secrets() {
    let diags = diagnostics(
        r#"
    @constant_time
    func branches(key: secret u64) -> secret u64 {
        if key > 5 { 1 } else { 2 }
    }
    @constant_time
    func loops(n: secret u64) -> secret u64 {
        var i: secret u64 = 0;
        while i < n { i += 1; }
        i
    }
    @constant_time
    func divides(a: secret u64, b: u64) -> secret u64 {
        a / b
    }
    @constant_time
    func indexes(table: [u64], i: secret usize) -> secret u64 {
        table[i]
    }
    @constant_time
    func short_circuits(a: secret bool, b: bool) -> secret bool {
        a && b
    }
    func not_constant_time(k: secret u64) -> secret u64 { k }
    @constant_time
    func calls(k: secret u64) -> secret u64 {
        not_constant_time(k)
    }
    oblivious func selects(a: u64, b: u64, pick: bool) -> u64 {
        ct_select(pick, a, b)
    }
    "#,
    );
    let count = |needle: &str| diags.matches(needle).count();
    assert_eq!(
        count("branch on a secret in a constant-time function"),
        2,
        "{diags}"
    );
    assert_eq!(
        count("integer division takes data-dependent time"),
        1,
        "{diags}"
    );
    assert_eq!(
        count("secret index in a constant-time function"),
        1,
        "{diags}"
    );
    assert_eq!(count("short-circuits, which is a branch"), 1, "{diags}");
    assert_eq!(count("which is not `@constant_time`"), 1, "{diags}");
    assert!(!diags.contains("selects"), "{diags}");
    assert_eq!(diags.lines().count(), 6, "{diags}");
}

/// A `match`, `if let` or `while let` on a secret branches on it, and the
/// names its patterns bind are secret.
#[test]
fn matching_on_a_secret_is_a_branch_and_binds_secrets() {
    let diags = diagnostics(
        r#"
    oblivious func matches(k: u64) -> u64 {
        match k { 0 -> 1, _ -> 2 }
    }
    @constant_time
    func if_lets(k: secret u64, o: i64?) -> secret u64 {
        if let v = o { 1 } else { k }
    }
    func takes_public(x: u64) -> u64 { x }
    func binds(k: secret u64) -> secret u64 {
        match k { v -> takes_public(v) }
    }
    "#,
    );
    let count = |needle: &str| diags.matches(needle).count();
    assert_eq!(
        count("`match` (or `if let`/`while let`) on a secret in a constant-time function"),
        1,
        "{diags}"
    );
    assert_eq!(
        count("secret value passed to a non-`secret` parameter of `takes_public`"),
        1,
        "{diags}"
    );
    assert_eq!(diags.lines().count(), 2, "{diags}");
}

/// A `match` on a secret enum is a branch on it: rejected in an
/// `oblivious` function.
#[test]
fn matching_on_a_secret_enum_is_rejected_in_oblivious_code() {
    let diags = diagnostics(
        r#"
    enum Bit { Zero, One }
    oblivious func pick(b: Bit, x: u64) -> u64 {
        match b { Bit.Zero -> x, Bit.One -> 0 }
    }
    "#,
    );
    assert!(
        diags.contains("`match` (or `if let`/`while let`) on a secret in a constant-time function"),
        "{diags}"
    );
}

/// Each `match` arm is checked on every path that may reach it: what holds
/// for all arms is proven, and an arm that breaks the contract is refuted.
#[test]
fn match_arms_are_each_checked_against_the_postcondition() {
    let r = report(
        r#"
    func pick(n: i64) -> i64
        ensures result >= 0
    {
        match n { 0 -> 1, 1 -> 2, m -> if m < 0 { 0 } else { m } }
    }
    "#,
        "pick",
    );
    assert!(all_proven(&r), "{r:#?}");

    let r = report(
        r#"
    func pick(n: i64) -> i64
        ensures result >= 0
    {
        match n { 0 -> 1, _ -> -1 }
    }
    "#,
        "pick",
    );
    assert!(
        refuted(&r, &ObligationKind::Postcondition).is_some(),
        "{r:#?}"
    );

    // An enum's payload is not modelled: a bound payload is a fresh value.
    let r = report(
        r#"
    enum Size { Small, Big(i64) }
    func measure(s: Size) -> i64
        ensures result >= 0
    {
        match s { Size.Small -> 1, Size.Big(n) -> if n < 0 { 0 } else { n } }
    }
    "#,
        "measure",
    );
    assert!(all_proven(&r), "{r:#?}");
}

#[test]
fn ct_select_is_an_ite_to_the_verifier() {
    let r = report(
        r#"
    func max(a: i64, b: i64) -> i64
        ensures result >= a && result >= b
    {
        ct_select(a > b, a, b)
    }
    "#,
        "max",
    );
    assert!(all_proven(&r), "{r:#?}");
}

// ---- synthesis (`verify::synth`)
// ---------------------------------------------

fn synthesized(source: &str, name: &str) -> Result<Vec<String>, String> {
    let (db, _) = MockDatabase::with_single_file(source);
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|def| match def {
            ModuleDef::Function(f) if f.name(&db).to_string() == name => Some(f),
            _ => None,
        })
        .unwrap();
    super::synth::synthesize(&db, func).map(|fills| fills.into_iter().map(|f| f.text).collect())
}

#[test]
fn holes_are_reported_until_filled() {
    let diags = diagnostics(
        r#"
    func f(x: i64) -> i64
        ensures result > x
    {
        ??
    }
    func g(n: i64) -> i64 {
        choose y where y > n
    }
    "#,
    );
    assert!(diags.contains("unfilled synthesis hole `??`"), "{diags}");
    assert!(
        diags.contains("unfilled synthesis hole `choose`"),
        "{diags}"
    );
    assert!(!diags.contains("postcondition"), "{diags}");
}

#[test]
fn a_linear_hole_is_synthesized_against_the_postcondition() {
    let fills = synthesized(
        r#"
    func inc(x: i64) -> i64
        requires x < 1000
        ensures result == x + 1
    {
        ??
    }
    "#,
        "inc",
    )
    .unwrap();
    assert_eq!(fills, ["(x + 1)"]);
}

#[test]
fn a_guarded_hole_synthesizes_max() {
    let fills = synthesized(
        r#"
    func max(a: i64, b: i64) -> i64
        ensures result >= a && result >= b && (result == a || result == b)
    {
        ??
    }
    "#,
        "max",
    )
    .unwrap();
    assert_eq!(fills.len(), 1);
    assert!(fills[0].starts_with("(if a < b {"), "{fills:?}");
}

#[test]
fn a_boolean_hole_synthesizes_a_comparison() {
    let fills = synthesized(
        r#"
    func is_negative(x: i64) -> bool
        ensures result == (x < 0)
    {
        ??
    }
    "#,
        "is_negative",
    )
    .unwrap();
    assert_eq!(fills, ["(x < 0)"]);
}

#[test]
fn choose_finds_a_value_satisfying_its_predicate() {
    let fills = synthesized(
        r#"
    func complement(n: i64) -> i64
        requires n >= 0 && n <= 10
    {
        choose y where y + n == 10
    }
    "#,
        "complement",
    )
    .unwrap();
    assert_eq!(fills, ["(-n + 10)"]);
}

#[test]
fn an_impossible_contract_is_not_synthesized() {
    let err = synthesized(
        r#"
    func impossible(x: i64) -> i64
        ensures result > x && result < x
    {
        ??
    }
    "#,
        "impossible",
    )
    .unwrap_err();
    assert!(err.contains("no candidate"), "{err}");
}

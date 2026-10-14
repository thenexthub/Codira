//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Elaboration tests over real Codira source (parsed, resolved and
//!   type-checked by `codira_hir`, with no diagnostics): `oblivious` and
//!   `@target(fpga)` functions become circuits that compute them; loops, calls,
//!   casts and compile-time arithmetic fold; `if` on an input becomes a
//!   multiplexer for FPGAs; every refusal names its reason and location; the
//!   Circom text; and a Groth16 proof from source.

use codira_hir::{salsa, HirDatabase};
use codira_hir_input::WithFixture;

use super::*;
use crate::{circom, groth16, r1cs};

#[salsa::database(
    codira_hir_input::SourceDatabaseStorage,
    codira_hir::AstDatabaseStorage,
    codira_hir::InternDatabaseStorage,
    codira_hir::DefDatabaseStorage,
    codira_hir::HirDatabaseStorage
)]
struct TestDb {
    storage: salsa::Storage<Self>,
}

impl salsa::Database for TestDb {}

impl Default for TestDb {
    fn default() -> Self {
        let mut db = TestDb {
            storage: salsa::Storage::default(),
        };
        db.set_target(codira_target::spec::Target::host_target().unwrap());
        db
    }
}

fn function(db: &TestDb, name: &str) -> Function {
    all_functions(db, |f| f.name(db).to_string() == name)
        .into_iter()
        .next()
        .unwrap_or_else(|| panic!("no function `{name}`"))
}

fn diagnostics(db: &TestDb) -> Vec<String> {
    let mut messages = Vec::new();
    for package in codira_hir::Package::all(db) {
        for module in package.modules(db) {
            module.diagnostics(
                db,
                &mut codira_hir::DiagnosticSink::new(|d| messages.push(d.message())),
            );
        }
    }
    messages
}

/// Elaborates `name` from `src`, which must have no diagnostics.
fn elab(src: &str, name: &str, mode: Mode) -> Circuit {
    let (db, _file) = TestDb::with_single_file(src);
    let messages = diagnostics(&db);
    assert!(messages.is_empty(), "{messages:?}");
    elaborate(&db, function(&db, name), mode, &ElabOptions::default())
        .unwrap_or_else(|e| panic!("{e}"))
}

/// The refusal for `name` in `src` (which may have other diagnostics).
fn refusal(src: &str, name: &str, mode: Mode) -> ElabError {
    let (db, _file) = TestDb::with_single_file(src);
    elaborate(&db, function(&db, name), mode, &ElabOptions::default())
        .expect_err("elaboration should be refused")
}

fn i64s(v: &[i64]) -> Vec<u64> {
    #[allow(clippy::cast_sign_loss)]
    v.iter().map(|x| *x as u64).collect()
}

#[allow(clippy::cast_possible_wrap)]
fn as_i64(v: u64) -> i64 {
    v as i64
}

const SCORE: &str = r#"
oblivious func score(x: i64, y: i64) -> i64 {
    let base = x * 3 + y
    let bonus = ct_select(x > y, 10i64, -10i64)
    base + bonus
}
"#;

#[test]
fn an_oblivious_function_becomes_a_circuit_that_computes_it() {
    let c = elab(SCORE, "score", Mode::Attest);
    assert_eq!(c.inputs.len(), 2);
    assert_eq!(c.signature(), "score(x: i64, y: i64) -> i64");
    for (x, y) in [
        (4, 2),
        (2, 4),
        (7, 7),
        (-5, 3),
        (i64::MAX, 1),
        (i64::MIN, -1),
        (i64::MIN, i64::MAX),
    ] {
        let expected = x
            .wrapping_mul(3)
            .wrapping_add(y)
            .wrapping_add(if x > y { 10 } else { -10 });
        assert_eq!(as_i64(c.eval(&i64s(&[x, y]))), expected, "score({x}, {y})");
    }
}

#[test]
fn loops_calls_casts_and_compile_time_values_unroll() {
    let src = r#"
oblivious func rotl(v: u32, k: u32) -> u32 {
    (v << k) | (v >> (32 - k))
}

oblivious func mix(a: u32, b: u32) -> u32 {
    var h = a
    for i in 0..4 {
        if i == 2 {
            continue
        }
        h = rotl(h ^ b, 5) + (i as u32) * (100 / 7)
    }
    var j = 0
    while j < 3 {
        h += 1
        j += 1
    }
    h
}
"#;
    let c = elab(src, "mix", Mode::Attest);
    let reference = |a: u32, b: u32| {
        let mut h = a;
        for i in 0..4u32 {
            if i == 2 {
                continue;
            }
            h = (h ^ b).rotate_left(5).wrapping_add(i * 14);
        }
        h.wrapping_add(3)
    };
    let mut rng = crate::r1cs::tests::SplitMix(3);
    for _ in 0..200 {
        let (a, b) = (rng.next() as u32, rng.next() as u32);
        assert_eq!(
            c.eval(&[u64::from(a), u64::from(b)]),
            u64::from(reference(a, b))
        );
    }
    // Nothing is left of the loops and the branch but gates.
    let ops = c.op_counts();
    assert!(ops.iter().all(|(op, _)| *op != "mux"), "{ops:?}");
    assert!(ops.iter().any(|(op, n)| *op == "xor" && *n == 3), "{ops:?}");
}

#[test]
fn declassify_is_the_identity_and_narrow_types_wrap() {
    let src = r#"
oblivious func f(x: u8, y: i16) -> i16 {
    let s: secret u8 = x + 200
    let wide = s as i16
    declassify(wide * y) - (y >> 15)
}
"#;
    let c = elab(src, "f", Mode::Attest);
    for x in 0..=255u8 {
        for y in [0i16, 1, -1, 300, -300, i16::MAX, i16::MIN] {
            let s = x.wrapping_add(200);
            let expected = i16::from(s).wrapping_mul(y).wrapping_sub(y >> 15);
            #[allow(clippy::cast_sign_loss)]
            let got = c.eval(&[u64::from(x), u64::from(y as u16)]);
            #[allow(clippy::cast_sign_loss)]
            let want = u64::from(expected as u16);
            assert_eq!(got, want, "f({x}, {y})");
        }
    }
}

#[test]
fn an_if_on_an_input_becomes_a_multiplexer_in_an_fpga_function() {
    let src = r#"
@target(fpga)
func clamp(x: i32, lo: i32, hi: i32) -> i32 {
    var r = x
    if x < lo {
        r = lo
    }
    if x > hi {
        r = hi
    }
    r
}

@target(fpga)
func magnitude(x: i32) -> i32 {
    if x < 0 { -x } else { x }
}

@target(fpga)
func both(a: bool, b: bool) -> bool {
    a && !b || b && !a
}
"#;
    let c = elab(src, "clamp", Mode::Fpga);
    assert!(c.op_counts().iter().any(|(op, n)| *op == "mux" && *n == 2));
    for x in [-100i32, -5, 0, 5, 100, i32::MIN, i32::MAX] {
        #[allow(clippy::cast_sign_loss)]
        let enc = |v: i32| u64::from(v as u32);
        let expected = x.clamp(-10, 10);
        assert_eq!(c.eval(&[enc(x), enc(-10), enc(10)]), enc(expected));
    }
    let m = elab(src, "magnitude", Mode::Fpga);
    #[allow(clippy::cast_sign_loss)]
    for x in [-7i32, 0, 7, i32::MIN] {
        assert_eq!(
            m.eval(&[u64::from(x as u32)]),
            u64::from(x.wrapping_abs() as u32)
        );
    }
    let b = elab(src, "both", Mode::Fpga);
    for (x, y) in [(0, 0), (0, 1), (1, 0), (1, 1)] {
        assert_eq!(b.eval(&[x, y]), x ^ y);
    }
}

#[test]
fn circom_emission_has_the_same_variables_and_constraints() {
    let c = elab(SCORE, "score", Mode::Attest);
    let r = r1cs::lower(&c);
    let text = circom::emit(&r);
    assert!(text.starts_with("pragma circom 2.0.0;"), "{text}");
    assert!(text.contains("template score() {"), "{text}");
    assert!(text.contains("    signal input x; // i64"), "{text}");
    assert!(text.contains("    signal output reveal; // i64"), "{text}");
    assert!(text.contains("component main = score();"), "{text}");
    assert_eq!(text.matches(" === ").count(), r.num_constraints());
    assert!(text.contains("<-- (t >> i) & 1;"), "{text}");
}

#[test]
fn a_proof_from_source_verifies_and_a_wrong_reveal_does_not() {
    let c = elab(SCORE, "score", Mode::Attest);
    let r = r1cs::lower(&c);
    let mut rng = groth16::seeded_rng(9);
    let (pk, vk) = groth16::setup(&r, &mut rng).unwrap();
    let inputs = i64s(&[-4, 9]);
    let w = r.witness(&inputs).unwrap();
    assert_eq!(as_i64(r.output(&w)), -4 * 3 + 9 - 10);
    assert_eq!(r.output(&w), c.eval(&inputs));
    let proof = groth16::prove(&r, &pk, &w, &mut rng).unwrap();
    assert!(groth16::verify(&vk, &r.public_inputs(&w), &proof).unwrap());
    let wrong = ark_bn254::Fr::from(c.output_ty().from_i128(-12));
    assert!(!groth16::verify(&vk, &[wrong], &proof).unwrap());
}

/// Elaboration refuses with `needle` in the message, at `line` when given.
fn assert_refused(src: &str, name: &str, mode: Mode, needle: &str, line: Option<u32>) {
    let e = refusal(src, name, mode);
    assert!(e.message.contains(needle), "expected `{needle}` in: {e}");
    if let Some(line) = line {
        assert_eq!(e.location.as_ref().map(|l| l.line), Some(line), "{e}");
    }
}

#[test]
fn everything_outside_the_subset_is_refused_with_a_reason() {
    use Mode::{Attest, Fpga};
    assert_refused(
        "func plain(x: i64) -> i64 { x }",
        "plain",
        Attest,
        "is not an `oblivious func`",
        None,
    );
    assert_refused(
        "func plain(x: i64) -> i64 { x }",
        "plain",
        Fpga,
        "is not marked `@target(fpga)`",
        None,
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: f64) -> f64 { x }",
        "f",
        Fpga,
        "floating-point",
        None,
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i128) -> i128 { x }",
        "f",
        Fpga,
        "not supported in circuits yet",
        None,
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) {\n}",
        "f",
        Fpga,
        "returns nothing",
        None,
    );
    assert_refused(
        "@target(fpga)\nfunc f(n: i64) -> i64 {\n    var i = 0\n    while i < n {\n        i += 1\n    }\n    i\n}",
        "f",
        Fpga,
        "loop condition depends on a run-time value",
        Some(4),
    );
    assert_refused(
        "@target(fpga)\nfunc f(n: i64) -> i64 {\n    var s = 0\n    for i in 0..n {\n        s += i\n    }\n    s\n}",
        "f",
        Fpga,
        "bounds depend on a run-time value",
        Some(4),
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) -> i64 {\n    x / 3\n}",
        "f",
        Fpga,
        "division and remainder are supported only on compile-time values",
        Some(3),
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) -> i64 {\n    7 / (x - x + 0)\n}",
        "f",
        Fpga,
        "division",
        Some(3),
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) -> i64 {\n    let a = [x, 1]\n    a[0]\n}",
        "f",
        Fpga,
        "arrays are heap objects",
        Some(3),
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) -> i64 {\n    println(x)\n    x\n}",
        "f",
        Fpga,
        "have effects",
        Some(3),
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) -> i64 {\n    if x > 0 {\n        return 1\n    }\n    0\n}",
        "f",
        Fpga,
        "under a condition known only at run time",
        Some(3),
    );
    assert_refused(
        "@target(fpga)\nfunc f(x: i64) -> i64 { g(x, 0) }\nfunc g(x: i64, n: i64) -> i64 {\n    if n == 1000 { x } else { g(x + 1, n + 1) }\n}",
        "f",
        Fpga,
        "calls nest deeper than",
        None,
    );
    // In an oblivious function, the flow rules hold by construction too.
    assert_refused(
        "oblivious func f(x: i64) -> i64 {\n    if x > 0 { 1 } else { 2 }\n}",
        "f",
        Attest,
        "ct_select",
        Some(2),
    );
    assert_refused(
        "oblivious func f(x: i64) -> i64 {\n    helper(x)\n}\nfunc helper(v: i64) -> i64 { v + 1 }",
        "f",
        Attest,
        "not `oblivious` or `@constant_time`",
        Some(2),
    );
    assert_refused(
        "oblivious func f(x: bool, y: bool) -> bool {\n    x && y\n}",
        "f",
        Attest,
        "short-circuits",
        Some(2),
    );
}

#[test]
fn compile_time_errors_in_folded_code_are_reported() {
    assert_refused(
        "oblivious func f(x: i64) -> i64 {\n    let z = 0\n    x + 5 / z\n}",
        "f",
        Mode::Attest,
        "division by zero",
        Some(3),
    );
    assert_refused(
        "oblivious func f(x: i8) -> i8 {\n    let m: i8 = -128\n    x + m / -1\n}",
        "f",
        Mode::Attest,
        "overflows",
        Some(3),
    );
}

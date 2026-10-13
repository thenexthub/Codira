//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Elaboration tests: real Codira source, parsed, resolved and type-checked
//!   by `codira_hir`, elaborated into circuits that are then checked with the
//!   simulator; plus every diagnostic the elaborator promises (measurement
//!   feedback, missing parameters, fuel, misuse).

use codira_hir::{salsa, HirDatabase};
use codira_hir_input::WithFixture;

use super::*;
use crate::{
    qasm,
    sim::{equal_up_to_global_phase, outcome_probabilities, unitary, Complex},
};

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

/// Parses `body` (with the prelude prepended) and elaborates `kernel`.
fn elab_with(
    body: &str,
    kernel: &str,
    args: &[(&str, &str)],
    options: &ElabOptions,
) -> Result<Circuit, ElabError> {
    let text = format!("{PRELUDE}\n{body}");
    let (db, _file) = TestDb::with_single_file(&text);
    let kernels = find_kernels(&db);
    let func = kernels
        .into_iter()
        .find(|f| f.name(&db).to_string() == kernel)
        .unwrap_or_else(|| panic!("no kernel `{kernel}`"));
    let args: Vec<(String, String)> = args
        .iter()
        .map(|(n, v)| ((*n).to_string(), (*v).to_string()))
        .collect();
    elaborate(&db, func, &args, options)
}

fn elab(body: &str, kernel: &str, args: &[(&str, &str)]) -> Circuit {
    elab_with(body, kernel, args, &ElabOptions::default()).unwrap_or_else(|e| panic!("{e}"))
}

fn elab_err(body: &str, kernel: &str, args: &[(&str, &str)]) -> ElabError {
    elab_with(body, kernel, args, &ElabOptions::default()).expect_err("should fail")
}

#[test]
fn the_prelude_type_checks_cleanly() {
    let text = format!("{PRELUDE}\n@target(qpu)\nfunc k() {{ h(0); measure(0, 0); }}\n");
    let (db, _file) = TestDb::with_single_file(&text);
    let mut messages = Vec::new();
    for package in codira_hir::Package::all(&db) {
        for module in package.modules(&db) {
            module.diagnostics(
                &db,
                &mut codira_hir::DiagnosticSink::new(|d| {
                    messages.push(d.message());
                }),
            );
        }
    }
    assert!(messages.is_empty(), "{messages:?}");
}

#[test]
fn bell_kernel_becomes_the_bell_circuit() {
    let c = elab(
        "@target(qpu)
         public func bell() {
             h(0);
             cx(0, 1);
             measure(0, 0);
             measure(1, 1);
         }",
        "bell",
        &[],
    );
    let mut expected = Circuit::new("bell", 2, 2);
    expected
        .gate(Gate::H, &[0])
        .gate(Gate::Cx, &[0, 1])
        .measure(0, 0)
        .measure(1, 1);
    assert_eq!(c, expected);
    let probs = outcome_probabilities(&c).unwrap();
    assert!((probs["00"] - 0.5).abs() < 1e-12 && (probs["11"] - 0.5).abs() < 1e-12);
}

#[test]
fn loops_and_parameters_unroll_at_compile_time() {
    let src = "@target(qpu)
         func ghz(n: i64) {
             h(0);
             let mut i = 1;
             while i < n {
                 cx(i - 1, i);
                 i += 1;
             }
             let mut j = 0;
             loop {
                 if j == n { break; }
                 measure(j, j);
                 j = j + 1;
             }
         }";
    for n in [2, 4, 7] {
        let c = elab(src, "ghz", &[("n", &n.to_string())]);
        assert_eq!(c.num_qubits(), n);
        assert_eq!(c.ops.len() as u32, 1 + (n - 1) + n);
        let probs = outcome_probabilities(&c).unwrap();
        assert_eq!(probs.len(), 2);
        assert!((probs[&"0".repeat(n as usize)] - 0.5).abs() < 1e-12);
        assert!((probs[&"1".repeat(n as usize)] - 0.5).abs() < 1e-12);
    }
}

#[test]
fn kernels_call_kernels_and_classical_helpers() {
    // The QFT written in Codira: helper function, cast, `pi()`, nested
    // loops, kernel-to-kernel call.
    let src = "func pow2(e: i64) -> i64 { 1 << e }

         @target(qpu)
         func qft(n: i64) {
             let mut j = n - 1;
             while j >= 0 {
                 h(j);
                 let mut k = j - 1;
                 while k >= 0 {
                     cp(pi() / (pow2(j - k) as f64), k, j);
                     k = k - 1;
                 }
                 j = j - 1;
             }
             let mut i = 0;
             while i < n / 2 {
                 swap(i, n - 1 - i);
                 i = i + 1;
             }
         }

         @target(qpu)
         func prepare_and_transform(value: i64, n: i64) {
             alloc_qubits(n);
             let mut q = 0;
             while q < n {
                 if ((value >> q) & 1) == 1 { x(q); }
                 q = q + 1;
             }
             qft(n);
         }";
    let n = 3u32;
    let dim = 1usize << n;
    let u = unitary(&elab(src, "qft", &[("n", "3")])).unwrap();
    let norm = 1.0 / (dim as f64).sqrt();
    let mut expected = vec![vec![Complex::ZERO; dim]; dim];
    for (k, row) in expected.iter_mut().enumerate() {
        for (x, entry) in row.iter_mut().enumerate() {
            let angle = 2.0 * std::f64::consts::PI * (x * k) as f64 / dim as f64;
            *entry = Complex::new(norm * angle.cos(), norm * angle.sin());
        }
    }
    assert!(equal_up_to_global_phase(&u, &expected, 1e-9));

    // QFT|5> has uniform magnitudes and phase e^{2 pi i 5k/8} on |k>.
    let c = elab(src, "prepare_and_transform", &[("value", "5"), ("n", "3")]);
    let state = crate::sim::final_state(&c).unwrap();
    for (k, amp) in state.amplitudes().iter().enumerate() {
        let angle = 2.0 * std::f64::consts::PI * (5 * k) as f64 / 8.0;
        assert!((amp.re - norm * angle.cos()).abs() < 1e-9);
        assert!((amp.im - norm * angle.sin()).abs() < 1e-9);
    }
}

#[test]
fn grover_kernel_matches_the_formula() {
    let src = "@target(qpu)
         func ccz(a: i64, b: i64, c: i64) { h(c); ccx(a, b, c); h(c); }

         @target(qpu)
         func grover(iterations: i64) {
             let mut q = 0;
             while q < 3 { h(q); q += 1; }
             let mut it = 0;
             while it < iterations {
                 x(1); ccz(0, 1, 2); x(1);
                 let mut r = 0;
                 while r < 3 { h(r); x(r); r += 1; }
                 ccz(0, 1, 2);
                 r = 0;
                 while r < 3 { x(r); h(r); r += 1; }
                 it += 1;
             }
             measure(0, 0); measure(1, 1); measure(2, 2);
         }";
    let one = outcome_probabilities(&elab(src, "grover", &[("iterations", "1")])).unwrap();
    assert!((one["101"] - 25.0 / 32.0).abs() < 1e-9);
    let two = outcome_probabilities(&elab(src, "grover", &[("iterations", "2")])).unwrap();
    assert!((two["101"] - 121.0 / 128.0).abs() < 1e-9);
}

#[test]
fn float_parameters_and_bool_parameters_bind() {
    let c = elab(
        "@target(qpu)
         func rot(theta: f64, flip: bool) {
             ry(theta, 0);
             if flip { x(0); }
             measure(0, 0);
         }",
        "rot",
        &[("theta", "1.0471975511965976"), ("flip", "true")],
    );
    // ry(pi/3) then x: P(1) = cos^2(pi/6) = 3/4.
    let probs = outcome_probabilities(&c).unwrap();
    assert!((probs["1"] - 0.75).abs() < 1e-12);
}

#[test]
fn branching_on_a_measurement_is_refused_with_its_location() {
    let body = "@target(qpu)
func feedback() {
    h(0);
    let r = measure(0, 0);
    if r { x(1); }
}";
    let e = elab_err(body, "feedback", &[]);
    assert!(e.message.contains("no measurement feedback"), "{e}");
    let loc = e.location.clone().expect("located");
    let prelude_lines = PRELUDE.lines().count() as u32;
    // The `if` condition is on line 5 of `body`, after the prelude and the
    // blank line `format!` adds.
    assert_eq!(loc.line, prelude_lines + 1 + 5, "{e}");
    assert_eq!(loc.function, "feedback");

    // Computing with it is refused the same way.
    let e = elab_err(
        "@target(qpu)
         func arith() { let r = measure(0, 0); let s = r == true; }",
        "arith",
        &[],
    );
    assert!(e.message.contains("measurement of qubit 0"), "{e}");
}

#[test]
fn parameters_must_be_supplied_and_valid() {
    let src = "@target(qpu) func k(n: i64) { h(n); }";
    let e = elab_err(src, "k", &[]);
    assert!(e.message.contains("--param n=<value>"), "{e}");
    let e = elab_err(src, "k", &[("n", "two")]);
    assert!(e.message.contains("not a valid value"), "{e}");
    let e = elab_err(src, "k", &[("n", "1"), ("m", "2")]);
    assert!(e.message.contains("no parameter `m`"), "{e}");
}

#[test]
fn unbounded_loops_run_out_of_fuel_deterministically() {
    let options = ElabOptions {
        fuel: 1000,
        ..ElabOptions::default()
    };
    let e = elab_with(
        "@target(qpu) func spin() { let mut i = 0; while i >= 0 { i = i + 1; } }",
        "spin",
        &[],
        &options,
    )
    .unwrap_err();
    assert!(e.message.contains("out of fuel"), "{e}");

    let e = elab_err(
        "@target(qpu) func forever(n: i64) { forever(n + 1); }",
        "forever",
        &[("n", "0")],
    );
    assert!(e.message.contains("nest deeper"), "{e}");
}

#[test]
fn misuse_is_reported() {
    // A gate from an ordinary function.
    let e = elab_err(
        "func helper() { h(0); }
         @target(qpu) func k() { helper(); }",
        "k",
        &[],
    );
    assert!(
        e.message.contains("outside a `@target(qpu)` function"),
        "{e}"
    );

    // A negative qubit index.
    let e = elab_err("@target(qpu) func k() { x(0 - 1); }", "k", &[]);
    assert!(e.message.contains("out of range"), "{e}");

    // The same qubit twice.
    let e = elab_err("@target(qpu) func k() { cx(1, 1); }", "k", &[]);
    assert!(e.message.contains("twice"), "{e}");

    // Overflow is an error, not a wrap.
    let e = elab_err(
        "@target(qpu) func k() { let big = 9223372036854775807; x(big + 1); }",
        "k",
        &[],
    );
    assert!(e.message.contains("overflows"), "{e}");

    // A C function's result is not a compile-time value.
    let e = elab_err(
        "extern \"C\" { func rand_index() -> i64; }
         @target(qpu) func k() { x(rand_index()); }",
        "k",
        &[],
    );
    assert!(e.message.contains("external function"), "{e}");
}

#[test]
fn imported_circuits_elaborate_back_to_themselves() {
    let src = "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[3];\nqreg anc[1];\ncreg m[3];\n\
               h q[0];\ncx q[0],q[1];\nrzz(0.25) q[1],q[2];\ncu(0.3,0.2,0.1,0.4) q[0],anc[0];\n\
               u(0.1,-0.2,0.3) q[1];\nbarrier q;\nreset anc[0];\nmeasure q -> m;\n";
    let original = qasm::import(src, "imported").unwrap();
    let code = circuit_to_codira(&original, "imported.qasm");
    assert!(code.contains("qreg anc[1]"));
    let back = elab(code.split_once(PRELUDE).unwrap().1, "imported", &[]);
    // Registers are flattened; the operations are identical except that a
    // barrier comes back as "all qubits".
    assert_eq!(back.num_qubits(), original.num_qubits());
    assert_eq!(back.num_clbits(), original.num_clbits());
    let strip = |c: &Circuit| -> Vec<Op> {
        c.ops
            .iter()
            .map(|op| match op {
                Op::Barrier { .. } => Op::Barrier { qubits: vec![] },
                other => other.clone(),
            })
            .collect()
    };
    assert_eq!(strip(&back), strip(&original));
}

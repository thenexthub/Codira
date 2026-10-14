//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - QIR tests: the emitted module's structure against the Base Profile spec's
//!   required pieces, every gate decomposition against the gate's unitary, text
//!   and bitcode round trips, and import of real Q# 1.31 (typed-pointer, QIR
//!   1.0) output.

use super::*;
use crate::{
    qasm,
    sim::{equal_up_to_global_phase, outcome_probabilities, unitary},
};

fn bell() -> Circuit {
    let mut c = Circuit::new("bell", 2, 2);
    c.gate(Gate::H, &[0]).gate(Gate::Cx, &[0, 1]).measure_all();
    c
}

#[test]
fn bell_module_has_every_piece_the_base_profile_requires() {
    let text = emit_qir_text(&bell(), &QirOptions::default()).unwrap();
    for needle in [
        "define i64 @bell() #0 {",
        "entry:",
        "call void @__quantum__rt__initialize(ptr null)",
        "br label %body",
        "body:",
        "call void @__quantum__qis__h__body(ptr null)",
        "call void @__quantum__qis__cx__body(ptr null, ptr inttoptr (i64 1 to ptr))",
        "br label %measurements",
        "measurements:",
        "call void @__quantum__qis__mz__body(ptr null, ptr null)",
        "call void @__quantum__qis__mz__body(ptr inttoptr (i64 1 to ptr), ptr inttoptr (i64 1 to ptr))",
        "br label %output",
        "output:",
        "call void @__quantum__rt__array_record_output(i64 2, ptr @0)",
        "call void @__quantum__rt__result_record_output(ptr null, ptr @1)",
        "call void @__quantum__rt__result_record_output(ptr inttoptr (i64 1 to ptr), ptr @2)",
        "ret i64 0",
        "@0 = internal constant [4 x i8] c\"0_a\\00\"",
        "@1 = internal constant [6 x i8] c\"1_a0r\\00\"",
        "@2 = internal constant [6 x i8] c\"2_a1r\\00\"",
        "declare void @__quantum__qis__mz__body(ptr, ptr writeonly) #1",
        "\"entry_point\"",
        "\"qir_profiles\"=\"base_profile\"",
        "\"output_labeling_schema\"=\"codira_v1\"",
        "\"required_num_qubits\"=\"2\"",
        "\"required_num_results\"=\"2\"",
        "attributes #1 = { \"irreversible\" }",
        "!{i32 1, !\"qir_major_version\", i32 2}",
        "!{i32 7, !\"qir_minor_version\", i32 0}",
        "!{i32 1, !\"dynamic_qubit_management\", i1 false}",
        "!{i32 1, !\"dynamic_result_management\", i1 false}",
    ] {
        assert!(text.contains(needle), "missing `{needle}` in:\n{text}");
    }
    // Only call/br/ret instructions appear in the entry point.
    let body = text.split("define i64 @bell()").nth(1).unwrap();
    let body = body.split("\n}\n").next().unwrap();
    for line in body.lines().skip(1) {
        let l = line.trim();
        if l.is_empty() || l.ends_with(':') || l.contains("; preds") {
            continue;
        }
        assert!(
            l.starts_with("call void @__quantum__")
                || l.starts_with("br label")
                || l == "ret i64 0",
            "unexpected instruction `{l}`"
        );
    }
}

#[test]
fn bitcode_is_llvm_bitcode_and_reimports() {
    let bc = emit_qir_bitcode(&bell(), &QirOptions::default()).unwrap();
    assert_eq!(&bc[..4], b"BC\xC0\xDE");
    let back = import_qir_bitcode(&bc).unwrap();
    assert_eq!(back.name, "bell");
    assert_eq!(
        outcome_probabilities(&back).unwrap(),
        outcome_probabilities(&bell()).unwrap()
    );
}

#[test]
fn major_version_option_is_honoured() {
    let opts = QirOptions {
        major_version: 1,
        minor_version: 0,
    };
    let text = emit_qir_text(&bell(), &opts).unwrap();
    assert!(text.contains("!{i32 1, !\"qir_major_version\", i32 1}"));
}

/// Rebuilds the IR circuit a decomposition denotes, for unitary comparison.
fn qis_circuit(n: u32, calls: &[QisCall]) -> Circuit {
    let mut c = Circuit::new("decomposed", n, 0);
    for call in calls {
        let angles: Vec<f64> = call.angle.into_iter().collect();
        c.ops
            .extend(qis_to_gates(call.name, &angles, &call.qubits).unwrap());
    }
    c
}

#[test]
fn every_decomposition_equals_its_gate() {
    let gates = [
        Gate::Id,
        Gate::H,
        Gate::X,
        Gate::Y,
        Gate::Z,
        Gate::S,
        Gate::Sdg,
        Gate::T,
        Gate::Tdg,
        Gate::Sx,
        Gate::Rx(0.3),
        Gate::Ry(-0.7),
        Gate::Rz(1.9),
        Gate::P(0.6),
        Gate::U(0.4, -1.2, 2.2),
        Gate::Cx,
        Gate::Cy,
        Gate::Cz,
        Gate::Ch,
        Gate::Cp(0.9),
        Gate::Crx(1.1),
        Gate::Cry(-0.5),
        Gate::Crz(2.7),
        Gate::Swap,
        Gate::Ccx,
        Gate::Cswap,
    ];
    for gate in gates {
        let n = gate.num_qubits() as u32;
        // Reverse the qubit order so "control first" is actually exercised
        // on a non-trivial index.
        let qubits: Vec<u32> = (0..n).rev().collect();
        let mut direct = Circuit::new("direct", n, 0);
        direct.gate(gate, &qubits);
        let decomposed = qis_circuit(n, &decompose(gate, &qubits));
        assert!(
            equal_up_to_global_phase(
                &unitary(&direct).unwrap(),
                &unitary(&decomposed).unwrap(),
                1e-9
            ),
            "{gate:?}"
        );
    }
}

#[test]
fn text_round_trip_preserves_the_distribution() {
    // Exercises decompositions, hex-printed doubles (0.1 is not exact in
    // decimal), several registers and a tuple of arrays.
    let src = "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[3];\ncreg a[1];\ncreg b[2];\n\
               h q[0];\nrx(0.1) q[1];\ncry(0.7) q[0], q[2];\ncu1(1.3) q[1], q[2];\nch q[2], q[0];\n\
               cswap q[0], q[1], q[2];\nu3(0.2, 0.3, 0.4) q[1];\nsx q[2];\n\
               measure q[0] -> b[1];\nmeasure q[1] -> a[0];\nmeasure q[2] -> b[0];\n";
    let original = qasm::import(src, "multi").unwrap();
    let text = emit_qir_text(&original, &QirOptions::default()).unwrap();
    assert!(text.contains("@__quantum__rt__tuple_record_output(i64 2, ptr @0)"));
    assert!(text.contains("c\"1_t0a\\00\""));
    assert!(text.contains("c\"3_t1a\\00\""));
    assert!(text.contains("c\"5_t1a1r\\00\""));
    let back = import_qir_text(&text).unwrap();
    assert_eq!(back.cregs.len(), 2);
    let (p, q) = (
        outcome_probabilities(&original).unwrap(),
        outcome_probabilities(&back).unwrap(),
    );
    assert_eq!(p.keys().collect::<Vec<_>>(), q.keys().collect::<Vec<_>>());
    for (k, v) in &p {
        assert!((v - q[k]).abs() < 1e-12, "{k}");
    }
}

#[test]
fn base_profile_violations_are_refused_before_emission() {
    let mut c = Circuit::new("mid", 1, 2);
    c.gate(Gate::H, &[0])
        .measure(0, 0)
        .gate(Gate::X, &[0])
        .measure(0, 1);
    assert!(matches!(
        emit_qir_text(&c, &QirOptions::default()),
        Err(QirError::Circuit(CircuitError::UseAfterMeasure { .. }))
    ));
    let mut c = Circuit::new("bad-name", 1, 0);
    c.gate(Gate::H, &[0]);
    assert!(matches!(
        emit_qir_text(&c, &QirOptions::default()),
        Err(QirError::BadName(_))
    ));
}

#[test]
fn a_leading_reset_and_barriers_are_dropped() {
    let mut c = Circuit::new("quiet", 1, 1);
    c.reset(0).gate(Gate::H, &[0]).barrier(&[0]).measure(0, 0);
    let text = emit_qir_text(&c, &QirOptions::default()).unwrap();
    assert!(!text.contains("reset"));
    assert!(!text.contains("barrier"));
}

/// Verbatim `qsharp.compile` output (qsharp 1.31, base profile): QIR 1.0,
/// typed pointers, one block, `m__body`, and qubit reuse through fresh
/// qubits after `ResetAll`.
const QSHARP_QIR: &str = r#"%Result = type opaque
%Qubit = type opaque

@0 = internal constant [4 x i8] c"0_a\00"
@1 = internal constant [6 x i8] c"1_a0r\00"
@2 = internal constant [6 x i8] c"2_a1r\00"
@3 = internal constant [6 x i8] c"3_a2r\00"

define i64 @ENTRYPOINT__main() #0 {
block_0:
  call void @__quantum__rt__initialize(i8* null)
  call void @__quantum__qis__h__body(%Qubit* inttoptr (i64 0 to %Qubit*))
  call void @__quantum__qis__cx__body(%Qubit* inttoptr (i64 0 to %Qubit*), %Qubit* inttoptr (i64 1 to %Qubit*))
  call void @__quantum__qis__y__body(%Qubit* inttoptr (i64 2 to %Qubit*))
  call void @__quantum__qis__s__body(%Qubit* inttoptr (i64 2 to %Qubit*))
  call void @__quantum__qis__s__adj(%Qubit* inttoptr (i64 2 to %Qubit*))
  call void @__quantum__qis__rx__body(double 0.5, %Qubit* inttoptr (i64 2 to %Qubit*))
  call void @__quantum__qis__cy__body(%Qubit* inttoptr (i64 0 to %Qubit*), %Qubit* inttoptr (i64 2 to %Qubit*))
  call void @__quantum__qis__ccx__body(%Qubit* inttoptr (i64 0 to %Qubit*), %Qubit* inttoptr (i64 1 to %Qubit*), %Qubit* inttoptr (i64 2 to %Qubit*))
  call void @__quantum__qis__rz__body(double 0.15, %Qubit* inttoptr (i64 1 to %Qubit*))
  call void @__quantum__qis__m__body(%Qubit* inttoptr (i64 0 to %Qubit*), %Result* inttoptr (i64 0 to %Result*))
  call void @__quantum__qis__m__body(%Qubit* inttoptr (i64 1 to %Qubit*), %Result* inttoptr (i64 1 to %Result*))
  call void @__quantum__qis__m__body(%Qubit* inttoptr (i64 2 to %Qubit*), %Result* inttoptr (i64 2 to %Result*))
  call void @__quantum__rt__array_record_output(i64 3, i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i64 0, i64 0))
  call void @__quantum__rt__result_record_output(%Result* inttoptr (i64 0 to %Result*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @1, i64 0, i64 0))
  call void @__quantum__rt__result_record_output(%Result* inttoptr (i64 1 to %Result*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @2, i64 0, i64 0))
  call void @__quantum__rt__result_record_output(%Result* inttoptr (i64 2 to %Result*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @3, i64 0, i64 0))
  ret i64 0
}

declare void @__quantum__rt__initialize(i8*)

attributes #0 = { "entry_point" "output_labeling_schema" "qir_profiles"="base_profile" "required_num_qubits"="6" "required_num_results"="3" }
attributes #1 = { "irreversible" }
"#;

#[test]
fn imports_qsharp_typed_pointer_qir() {
    let c = import_qir_text(QSHARP_QIR).unwrap();
    assert_eq!(c.name, "ENTRYPOINT__main");
    // `required_num_qubits` wins over the highest index used.
    assert_eq!(c.num_qubits(), 6);
    assert_eq!(c.num_clbits(), 3);
    assert_eq!(c.cregs[0].name, "c");
    let gates = c
        .ops
        .iter()
        .filter(|op| matches!(op, Op::Gate { .. }))
        .count();
    assert_eq!(gates, 9);
    let measures: Vec<(u32, u32)> = c
        .ops
        .iter()
        .filter_map(|op| match op {
            Op::Measure { qubit, clbit } => Some((*qubit, *clbit)),
            _ => None,
        })
        .collect();
    assert_eq!(measures, [(0, 0), (1, 1), (2, 2)]);
    // Qubits 0 and 1 form a Bell pair: their bits always agree.
    let probs = outcome_probabilities(&c).unwrap();
    for key in probs.keys() {
        let bits: Vec<char> = key.chars().collect();
        assert_eq!(bits[1], bits[2], "{key}");
    }
}

#[test]
fn importer_rejects_programs_that_are_not_circuits() {
    let branchy = "define i64 @main() #0 {\nentry:\n  br i1 %r, label %a, label %b\n}\n\
                   attributes #0 = { \"entry_point\" }\n";
    assert!(import_qir_text(branchy)
        .unwrap_err()
        .to_string()
        .contains("adaptive profile"));
    let dynamic =
        "define i64 @main() #0 {\nentry:\n  %q = call ptr @__quantum__rt__qubit_allocate()\n}\n\
                   attributes #0 = { \"entry_point\" }\n";
    assert!(import_qir_text(dynamic)
        .unwrap_err()
        .to_string()
        .contains("dynamic"));
}

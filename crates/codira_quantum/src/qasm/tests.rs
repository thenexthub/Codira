//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - OpenQASM emitter/importer tests: exact round trips in both dialects, real
//!   Qiskit 2.5 `qasm2.dumps`/`qasm3.dumps` output, the built-in gate library
//!   checked against independently written matrices, and the rejection messages
//!   for what is not a static circuit.

// Matrix checks read best with explicit row/column indices.
#![allow(clippy::needless_range_loop)]

use std::f64::consts::PI;

use super::*;
use crate::sim::{
    equal_up_to_global_phase, outcome_probabilities, single_qubit_matrix, unitary, Complex,
};

fn every_gate_circuit() -> Circuit {
    let mut c = Circuit {
        name: "all".into(),
        qregs: vec![Register::new("q", 3), Register::new("anc", 1)],
        cregs: vec![Register::new("c", 2), Register::new("flags", 2)],
        ops: vec![],
    };
    let one = [
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
        Gate::Rx(0.125),
        Gate::Ry(-1.0e-7),
        Gate::Rz(PI / 3.0),
        Gate::P(2.5),
        Gate::U(0.1, 0.2, 0.30000000000000004),
    ];
    for (i, g) in one.into_iter().enumerate() {
        c.gate(g, &[(i % 4) as u32]);
    }
    for g in [
        Gate::Cx,
        Gate::Cy,
        Gate::Cz,
        Gate::Ch,
        Gate::Cp(0.7),
        Gate::Crx(0.8),
        Gate::Cry(0.9),
        Gate::Crz(1.0),
        Gate::Swap,
    ] {
        c.gate(g, &[0, 3]);
    }
    c.gate(Gate::Ccx, &[0, 1, 2]).gate(Gate::Cswap, &[3, 1, 2]);
    c.barrier(&[0, 1]).barrier(&[]).reset(2);
    c.measure(0, 0).measure(1, 1).measure(2, 2).measure(3, 3);
    c
}

#[test]
fn round_trip_is_exact_in_both_dialects() {
    let original = every_gate_circuit();
    for version in [QasmVersion::V2, QasmVersion::V3] {
        let text = emit(&original, version).unwrap();
        let mut back = import(&text, "all").unwrap();
        // An empty barrier means "all qubits"; it comes back explicit.
        let all: Vec<u32> = (0..4).collect();
        let mut expected = original.clone();
        for op in &mut expected.ops {
            if let Op::Barrier { qubits } = op {
                if qubits.is_empty() {
                    qubits.clone_from(&all);
                }
            }
        }
        back.name = expected.name.clone();
        assert_eq!(back, expected, "{version:?}:\n{text}");
    }
}

#[test]
fn emitted_text_uses_each_dialects_spelling() {
    let mut c = Circuit::new("bell", 2, 2);
    c.gate(Gate::H, &[0])
        .gate(Gate::Cx, &[0, 1])
        .gate(Gate::U(1.0, 2.0, 3.0), &[1])
        .gate(Gate::P(0.5), &[0]);
    c.measure_all();
    assert_eq!(
        emit_qasm2(&c).unwrap(),
        "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[2];\ncreg c[2];\nh q[0];\ncx q[0], q[1];\n\
         u3(1.0, 2.0, 3.0) q[1];\nu1(0.5) q[0];\nmeasure q[0] -> c[0];\nmeasure q[1] -> c[1];\n"
    );
    assert_eq!(
        emit_qasm3(&c).unwrap(),
        "OPENQASM 3.0;\ninclude \"stdgates.inc\";\nqubit[2] q;\nbit[2] c;\nh q[0];\ncx q[0], q[1];\n\
         U(1.0, 2.0, 3.0) q[1];\np(0.5) q[0];\nc[0] = measure q[0];\nc[1] = measure q[1];\n"
    );
}

/// Verbatim `qiskit.qasm2.dumps` output (Qiskit 2.5.2) for a circuit using
/// gates outside the IR's own set.
const QISKIT_QASM2: &str = r#"OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
qreg anc[1];
creg meas[3];
h q[0];
cx q[0],q[1];
rzz(0.25) q[1],q[2];
cu(0.3,0.2,0.1,0.4) q[0],anc[0];
sxdg q[2];
cp(pi/4) q[2],anc[0];
u(0.1,0.2,0.3) q[1];
barrier q[0],q[1],q[2],anc[0];
ccx q[0],q[1],q[2];
measure q[0] -> meas[0];
measure q[1] -> meas[1];
measure q[2] -> meas[2];
"#;

/// Verbatim `qiskit.qasm3.dumps` output for the same circuit: note the
/// inline definitions of `rzz` and `sxdg`, which override the library's.
const QISKIT_QASM3: &str = r#"OPENQASM 3.0;
include "stdgates.inc";
gate rzz(p0) _gate_q_0, _gate_q_1 {
  cx _gate_q_0, _gate_q_1;
  rz(p0) _gate_q_1;
  cx _gate_q_0, _gate_q_1;
}
gate sxdg _gate_q_0 {
  s _gate_q_0;
  h _gate_q_0;
  s _gate_q_0;
}
bit[3] meas;
qubit[3] q;
qubit[1] anc;
h q[0];
cx q[0], q[1];
rzz(0.25) q[1], q[2];
cu(0.3, 0.2, 0.1, 0.4) q[0], anc[0];
sxdg q[2];
cp(pi/4) q[2], anc[0];
U(0.1, 0.2, 0.3) q[1];
barrier q[0], q[1], q[2], anc[0];
ccx q[0], q[1], q[2];
meas[0] = measure q[0];
meas[1] = measure q[1];
meas[2] = measure q[2];
"#;

#[test]
fn imports_real_qiskit_exports_in_both_dialects() {
    let v2 = import(QISKIT_QASM2, "from_qiskit").unwrap();
    let v3 = import(QISKIT_QASM3, "from_qiskit").unwrap();
    for c in [&v2, &v3] {
        assert_eq!(c.num_qubits(), 4);
        assert_eq!(c.num_clbits(), 3);
        assert_eq!(c.cregs[0].name, "meas");
        assert_eq!(c.qregs[1].name, "anc");
        assert!(c.has_only_terminal_measurements());
    }
    // Both files describe the same circuit; their libraries differ only in
    // how `sxdg` is decomposed, which is a global phase.
    let p2 = outcome_probabilities(&v2).unwrap();
    let p3 = outcome_probabilities(&v3).unwrap();
    assert_eq!(p2.keys().collect::<Vec<_>>(), p3.keys().collect::<Vec<_>>());
    for (k, p) in &p2 {
        assert!((p - p3[k]).abs() < 1e-12, "{k}: {p} vs {}", p3[k]);
    }
    assert!(equal_up_to_global_phase(
        &unitary(&v2.without_measurements()).unwrap(),
        &unitary(&v3.without_measurements()).unwrap(),
        1e-9
    ));
}

#[test]
fn qiskit_qft_export_imports_as_the_qft() {
    let text = "OPENQASM 3.0;\ninclude \"stdgates.inc\";\nqubit[3] q;\nh q[2];\n\
                cp(pi/2) q[2], q[1];\nh q[1];\ncp(pi/4) q[2], q[0];\ncp(pi/2) q[1], q[0];\n\
                h q[0];\nswap q[0], q[2];\n";
    let c = import(text, "qft").unwrap();
    let u = unitary(&c).unwrap();
    let norm = 1.0 / 8f64.sqrt();
    for x in 0..8 {
        for k in 0..8 {
            let angle = 2.0 * PI * (x * k) as f64 / 8.0;
            let expected = Complex::new(norm * angle.cos(), norm * angle.sin());
            assert!((u[k][x] - expected).norm_sqr() < 1e-18);
        }
    }
}

/// `|0><0| (x) I + |1><1| (x) m` with the control on qubit 0 and the
/// target on qubit 1 (basis index = control + 2 * target).
fn controlled(m: [[Complex; 2]; 2]) -> Vec<Vec<Complex>> {
    let mut u = vec![vec![Complex::ZERO; 4]; 4];
    u[0][0] = Complex::ONE;
    u[2][2] = Complex::ONE;
    for (r, row) in m.iter().enumerate() {
        for (c, v) in row.iter().enumerate() {
            u[1 + 2 * r][1 + 2 * c] = *v;
        }
    }
    u
}

fn lib_unitary(src: &str, n: u32) -> Vec<Vec<Complex>> {
    let mut text = format!("OPENQASM 2.0;\nqreg q[{n}];\n");
    text.push_str(src);
    unitary(&import(&text, "lib").unwrap()).unwrap()
}

#[test]
fn library_gates_match_their_matrices() {
    let theta: f64 = 0.37;
    let (c, s) = ((theta / 2.0).cos(), (theta / 2.0).sin());

    // rzz(theta) = exp(-i theta/2 Z(x)Z).
    let mut rzz = vec![vec![Complex::ZERO; 4]; 4];
    for (i, row) in rzz.iter_mut().enumerate() {
        let parity = (i & 1) ^ (i >> 1);
        row[i] = Complex::cis(if parity == 0 {
            -theta / 2.0
        } else {
            theta / 2.0
        });
    }
    assert!(equal_up_to_global_phase(
        &lib_unitary("rzz(0.37) q[0], q[1];", 2),
        &rzz,
        1e-9
    ));

    // rxx(theta) = cos(theta/2) I - i sin(theta/2) X(x)X.
    let mut rxx = vec![vec![Complex::ZERO; 4]; 4];
    for (i, row) in rxx.iter_mut().enumerate() {
        row[i] = Complex::new(c, 0.0);
        row[3 - i] = Complex::new(0.0, -s);
    }
    assert!(equal_up_to_global_phase(
        &lib_unitary("rxx(0.37) q[0], q[1];", 2),
        &rxx,
        1e-9
    ));

    // ryy(theta) = cos(theta/2) I - i sin(theta/2) Y(x)Y; Y(x)Y maps
    // |00> -> -|11>, |11> -> -|00>, |01> -> |10>, |10> -> |01>.
    let mut ryy = vec![vec![Complex::ZERO; 4]; 4];
    for (i, row) in ryy.iter_mut().enumerate() {
        row[i] = Complex::new(c, 0.0);
        let sign = if i == 0 || i == 3 { -1.0 } else { 1.0 };
        row[3 - i] = Complex::new(0.0, -s * sign);
    }
    assert!(equal_up_to_global_phase(
        &lib_unitary("ryy(0.37) q[0], q[1];", 2),
        &ryy,
        1e-9
    ));

    // cu3 is controlled-U exactly (up to one global phase for the whole
    // two-qubit unitary); cu adds e^{i gamma} on the controlled block.
    let u = single_qubit_matrix(Gate::U(0.3, 0.2, 0.1)).unwrap();
    assert!(equal_up_to_global_phase(
        &lib_unitary("cu3(0.3, 0.2, 0.1) q[0], q[1];", 2),
        &controlled(u),
        1e-9
    ));
    let g = Complex::cis(0.4);
    let ug = [[u[0][0] * g, u[0][1] * g], [u[1][0] * g, u[1][1] * g]];
    assert!(equal_up_to_global_phase(
        &lib_unitary("cu(0.3, 0.2, 0.1, 0.4) q[0], q[1];", 2),
        &controlled(ug),
        1e-9
    ));

    // csx = controlled-sx, cs = controlled-s.
    assert!(equal_up_to_global_phase(
        &lib_unitary("csx q[0], q[1];", 2),
        &controlled(single_qubit_matrix(Gate::Sx).unwrap()),
        1e-9
    ));
    assert!(equal_up_to_global_phase(
        &lib_unitary("cs q[0], q[1];", 2),
        &controlled(single_qubit_matrix(Gate::S).unwrap()),
        1e-9
    ));

    // sxdg undoes sx.
    let mut id = vec![vec![Complex::ZERO; 2]; 2];
    id[0][0] = Complex::ONE;
    id[1][1] = Complex::ONE;
    assert!(equal_up_to_global_phase(
        &lib_unitary("sx q[0]; sxdg q[0];", 1),
        &id,
        1e-9
    ));

    // ccz flips the sign of |111> only.
    let mut ccz = vec![vec![Complex::ZERO; 8]; 8];
    for (i, row) in ccz.iter_mut().enumerate() {
        row[i] = if i == 7 { -Complex::ONE } else { Complex::ONE };
    }
    assert!(equal_up_to_global_phase(
        &lib_unitary("ccz q[0], q[1], q[2];", 3),
        &ccz,
        1e-9
    ));

    // iswap: |01> -> i|10>, |10> -> i|01>.
    let mut iswap = vec![vec![Complex::ZERO; 4]; 4];
    iswap[0][0] = Complex::ONE;
    iswap[3][3] = Complex::ONE;
    iswap[2][1] = Complex::I;
    iswap[1][2] = Complex::I;
    assert!(equal_up_to_global_phase(
        &lib_unitary("iswap q[0], q[1];", 2),
        &iswap,
        1e-9
    ));
}

#[test]
fn broadcasting_expressions_and_user_gates() {
    let src = "OPENQASM 2.0;
        include \"qelib1.inc\";
        qreg a[3]; creg m[3];
        gate bell(t) x, y { h x; cx x, y; rz(t / 2 + -0.5) y; }
        h a;                 // broadcast over the register
        bell(1.0) a[0], a[1];
        rx(2 * pi / 4) a[2];
        ry(sqrt(4) ^ 2 - cos(0)) a[1];
        measure a -> m;";
    let c = import(src, "user").unwrap();
    let gates: Vec<String> = c
        .ops
        .iter()
        .filter_map(|op| match op {
            Op::Gate { gate, qubits } => {
                Some(format!("{}{:?}{qubits:?}", gate.name(), gate.params()))
            }
            _ => None,
        })
        .collect();
    assert_eq!(
        gates,
        [
            "h[][0]",
            "h[][1]",
            "h[][2]",
            "h[][0]",
            "cx[][0, 1]",
            "rz[0.0][1]",
            &format!("rx[{}][2]", PI / 2.0),
            "ry[3.0][1]",
        ]
    );
    assert_eq!(
        c.ops
            .iter()
            .filter(|op| matches!(op, Op::Measure { .. }))
            .count(),
        3
    );
}

#[test]
fn rejects_what_is_not_a_static_circuit_with_its_line() {
    let cases = [
        (
            "OPENQASM 2.0;\nqreg q[1];\ncreg c[1];\nif (c==1) x q[0];\n",
            4,
            "measurement feedback",
        ),
        (
            "OPENQASM 3.0;\nqubit[1] q;\nfor int i in [0:2] { x q[0]; }\n",
            3,
            "classical program",
        ),
        (
            "OPENQASM 3.0;\ninput float theta;\n",
            2,
            "classical program",
        ),
        (
            "OPENQASM 3.0;\nqubit[2] q;\nctrl @ x q[0], q[1];\n",
            3,
            "modifier",
        ),
        (
            "OPENQASM 2.0;\nqreg q[1];\nfoo q[0];\n",
            3,
            "unknown gate `foo`",
        ),
        ("OPENQASM 2.0;\nqreg q[1];\nh q[1];\n", 3, "out of range"),
        (
            "OPENQASM 2.0;\nqreg q[1];\nrx q[0];\n",
            3,
            "wrong number of parameters",
        ),
        (
            "OPENQASM 2.0;\nqreg q[2];\ncreg c[1];\nmeasure q -> c;\n",
            4,
            "2 qubits into 1",
        ),
        (
            "OPENQASM 2.0;\ninclude \"mylib.inc\";\n",
            2,
            "cannot include",
        ),
        (
            "OPENQASM 2.0;\nqreg q[1];\nqreg q[2];\n",
            3,
            "declared twice",
        ),
        ("OPENQASM 2.0;\nopaque g q;\n", 2, "opaque"),
    ];
    for (src, line, needle) in cases {
        let e = import(src, "bad").unwrap_err();
        assert_eq!(e.line, line, "{src}: {e}");
        assert!(e.message.contains(needle), "{src}: {e}");
    }
}

#[test]
fn angles_keep_a_decimal_point() {
    assert_eq!(format_angle(2.0), "2.0");
    assert_eq!(format_angle(-0.5), "-0.5");
    assert_eq!(format_angle(1e-7), "0.0000001");
    assert_eq!(format_angle(PI).parse::<f64>().unwrap(), PI);
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Simulator tests against hand-derived amplitudes and probabilities: single
//!   gates, Bell, GHZ, teleportation, Deutsch-Jozsa, Grover on 2 and 3 qubits,
//!   and a QFT checked against its defining formula and round- tripped through
//!   its inverse.

// Matrix checks read best with explicit row/column indices.
#![allow(clippy::needless_range_loop)]

use std::f64::consts::{FRAC_1_SQRT_2, PI};

use super::*;
use crate::circuit::Register;

const EPS: f64 = 1e-12;

fn assert_close(a: f64, b: f64) {
    assert!((a - b).abs() < 1e-9, "{a} != {b}");
}

fn assert_amp(a: Complex, re: f64, im: f64) {
    assert!(
        (a.re - re).abs() < 1e-9 && (a.im - im).abs() < 1e-9,
        "{a:?} != ({re}, {im})"
    );
}

/// The state after the circuit's gates, starting from |0..0>.
fn state(circuit: &Circuit) -> StateVector {
    final_state(circuit).expect("simulates")
}

#[test]
fn single_qubit_gates_match_their_matrices_on_basis_states() {
    let mut c = Circuit::new("h", 1, 0);
    c.gate(Gate::H, &[0]);
    let s = state(&c);
    assert_amp(s.amplitudes()[0], FRAC_1_SQRT_2, 0.0);
    assert_amp(s.amplitudes()[1], FRAC_1_SQRT_2, 0.0);

    // Rx(pi)|0> = -i|1>.
    let mut c = Circuit::new("rx", 1, 0);
    c.gate(Gate::Rx(PI), &[0]);
    let s = state(&c);
    assert_amp(s.amplitudes()[0], 0.0, 0.0);
    assert_amp(s.amplitudes()[1], 0.0, -1.0);

    // Ry(pi/3)|0> = cos(pi/6)|0> + sin(pi/6)|1>: probabilities 3/4, 1/4.
    let mut c = Circuit::new("ry", 1, 0);
    c.gate(Gate::Ry(PI / 3.0), &[0]);
    let p = state(&c).probabilities();
    assert_close(p[0], 0.75);
    assert_close(p[1], 0.25);

    // X then S then T: |1> picks up i * e^{i pi/4} = e^{i 3pi/4}.
    let mut c = Circuit::new("phase", 1, 0);
    c.gate(Gate::X, &[0])
        .gate(Gate::S, &[0])
        .gate(Gate::T, &[0]);
    let s = state(&c);
    assert_amp(
        s.amplitudes()[1],
        (3.0 * PI / 4.0).cos(),
        (3.0 * PI / 4.0).sin(),
    );

    // U(theta, phi, lambda)|0> = cos(theta/2)|0> + e^{i phi} sin(theta/2)|1>.
    let (theta, phi, lambda) = (0.7, 1.3, -0.4);
    let mut c = Circuit::new("u", 1, 0);
    c.gate(Gate::U(theta, phi, lambda), &[0]);
    let s = state(&c);
    assert_amp(s.amplitudes()[0], (theta / 2.0).cos(), 0.0);
    assert_amp(
        s.amplitudes()[1],
        phi.cos() * (theta / 2.0).sin(),
        phi.sin() * (theta / 2.0).sin(),
    );

    // sx twice is x, exactly.
    let mut c = Circuit::new("sx", 1, 0);
    c.gate(Gate::Sx, &[0]).gate(Gate::Sx, &[0]);
    let s = state(&c);
    assert_amp(s.amplitudes()[1], 1.0, 0.0);

    // Every gate's matrix is unitary: M^dagger M = I.
    for gate in [
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
        Gate::Ry(1.1),
        Gate::Rz(-2.0),
        Gate::P(0.9),
        Gate::U(0.2, 0.5, 1.7),
    ] {
        let m = single_qubit_matrix(gate).unwrap();
        for i in 0..2 {
            for j in 0..2 {
                let dot = m[0][i].conj() * m[0][j] + m[1][i].conj() * m[1][j];
                let expected = if i == j { 1.0 } else { 0.0 };
                assert!(
                    (dot.re - expected).abs() < EPS && dot.im.abs() < EPS,
                    "{gate:?} is not unitary"
                );
            }
        }
    }
}

#[test]
fn controls_come_first_and_qubit_zero_is_the_low_bit() {
    // X on qubit 0 makes basis index 1; CX(0 -> 1) then sets qubit 1: index 3.
    let mut c = Circuit::new("cx", 2, 0);
    c.gate(Gate::X, &[0]).gate(Gate::Cx, &[0, 1]);
    assert_close(state(&c).probabilities()[3], 1.0);

    // CX with the control in |0> does nothing.
    let mut c = Circuit::new("cx_off", 2, 0);
    c.gate(Gate::X, &[1]).gate(Gate::Cx, &[0, 1]);
    assert_close(state(&c).probabilities()[2], 1.0);

    // Toffoli truth table.
    for input in 0..8usize {
        let mut c = Circuit::new("ccx", 3, 0);
        for q in 0..3 {
            if input & (1 << q) != 0 {
                c.gate(Gate::X, &[q as u32]);
            }
        }
        c.gate(Gate::Ccx, &[0, 1, 2]);
        let expected = if input & 0b011 == 0b011 {
            input ^ 0b100
        } else {
            input
        };
        assert_close(state(&c).probabilities()[expected], 1.0);
    }

    // SWAP |q1 q0> = |01> -> |10>; CSWAP swaps only when its control is set.
    let mut c = Circuit::new("swap", 2, 0);
    c.gate(Gate::X, &[0]).gate(Gate::Swap, &[0, 1]);
    assert_close(state(&c).probabilities()[2], 1.0);

    let mut c = Circuit::new("cswap", 3, 0);
    c.gate(Gate::X, &[1]).gate(Gate::Cswap, &[0, 1, 2]);
    assert_close(state(&c).probabilities()[0b010], 1.0);
    let mut c = Circuit::new("cswap_on", 3, 0);
    c.gate(Gate::X, &[0])
        .gate(Gate::X, &[1])
        .gate(Gate::Cswap, &[0, 1, 2]);
    assert_close(state(&c).probabilities()[0b101], 1.0);

    // CH on |1>|0>: target becomes |+>.
    let mut c = Circuit::new("ch", 2, 0);
    c.gate(Gate::X, &[0]).gate(Gate::Ch, &[0, 1]);
    let p = state(&c).probabilities();
    assert_close(p[0b01], 0.5);
    assert_close(p[0b11], 0.5);

    // CRY(theta) with control set rotates the target by theta.
    let mut c = Circuit::new("cry", 2, 0);
    c.gate(Gate::X, &[0]).gate(Gate::Cry(PI / 3.0), &[0, 1]);
    let p = state(&c).probabilities();
    assert_close(p[0b01], 0.75);
    assert_close(p[0b11], 0.25);
}

#[test]
fn bell_state_amplitudes_and_counts() {
    let mut c = Circuit::new("bell", 2, 2);
    c.gate(Gate::H, &[0]).gate(Gate::Cx, &[0, 1]).measure_all();

    let s = state(&c);
    assert_amp(s.amplitudes()[0], FRAC_1_SQRT_2, 0.0);
    assert_amp(s.amplitudes()[1], 0.0, 0.0);
    assert_amp(s.amplitudes()[2], 0.0, 0.0);
    assert_amp(s.amplitudes()[3], FRAC_1_SQRT_2, 0.0);

    let probs = outcome_probabilities(&c).unwrap();
    assert_eq!(probs.len(), 2);
    assert_close(probs["00"], 0.5);
    assert_close(probs["11"], 0.5);

    let counts = run(&c, 4000, 42).unwrap();
    assert_eq!(counts.keys().collect::<Vec<_>>(), ["00", "11"]);
    assert_eq!(counts.values().sum::<u64>(), 4000);
    let zeros = counts["00"] as f64 / 4000.0;
    assert!((zeros - 0.5).abs() < 0.05, "{zeros}");
}

#[test]
fn same_seed_same_counts_different_seed_different_counts() {
    let mut c = Circuit::new("coins", 4, 4);
    for q in 0..4 {
        c.gate(Gate::H, &[q]);
    }
    c.measure_all();
    let a = run(&c, 1000, 7).unwrap();
    let b = run(&c, 1000, 7).unwrap();
    let other = run(&c, 1000, 8).unwrap();
    assert_eq!(a, b);
    assert_ne!(a, other);
}

#[test]
fn ghz_state_is_all_zeros_or_all_ones() {
    let n = 5;
    let mut c = Circuit::new("ghz", n, n);
    c.gate(Gate::H, &[0]);
    for q in 1..n {
        c.gate(Gate::Cx, &[q - 1, q]);
    }
    c.measure_all();

    let s = state(&c);
    let last = (1usize << n) - 1;
    for (i, a) in s.amplitudes().iter().enumerate() {
        let expected = if i == 0 || i == last {
            FRAC_1_SQRT_2
        } else {
            0.0
        };
        assert_amp(*a, expected, 0.0);
    }
    let counts = run(&c, 1000, 1).unwrap();
    assert_eq!(counts.keys().collect::<Vec<_>>(), ["00000", "11111"]);
}

/// Teleports `Ry(theta)|0>` from qubit 0 to qubit 2. The corrections are
/// controlled by the measured qubits themselves (the deferred-measurement
/// form), which is what the base profile can express.
fn teleport(theta: f64, measure_first: bool) -> Circuit {
    let mut c = Circuit {
        name: "teleport".into(),
        qregs: vec![Register::new("q", 3)],
        cregs: vec![Register::new("m", 2), Register::new("out", 1)],
        ops: vec![],
    };
    c.gate(Gate::Ry(theta), &[0])
        .gate(Gate::H, &[1])
        .gate(Gate::Cx, &[1, 2])
        .gate(Gate::Cx, &[0, 1])
        .gate(Gate::H, &[0]);
    if measure_first {
        // Measuring before the corrections makes them classically
        // controlled in effect; this exercises the per-shot path.
        c.measure(0, 0).measure(1, 1);
        c.gate(Gate::Cx, &[1, 2]).gate(Gate::Cz, &[0, 2]);
    } else {
        c.gate(Gate::Cx, &[1, 2]).gate(Gate::Cz, &[0, 2]);
        c.measure(0, 0).measure(1, 1);
    }
    c.measure(2, 2);
    c
}

#[test]
fn teleportation_moves_the_state_to_the_last_qubit() {
    let theta = 1.234_f64;
    let expected_one = (theta / 2.0).sin().powi(2);

    // Exact: the probability of reading 1 on the output register is
    // sin^2(theta/2) whatever the two Bell-measurement bits say, and each of
    // their four values is equally likely.
    let probs = outcome_probabilities(&teleport(theta, false)).unwrap();
    let p_one: f64 = probs
        .iter()
        .filter(|(k, _)| k.starts_with('1'))
        .map(|(_, p)| p)
        .sum();
    assert_close(p_one, expected_one);
    for m in ["00", "01", "10", "11"] {
        let p_m: f64 = probs
            .iter()
            .filter(|(k, _)| k.ends_with(m))
            .map(|(_, p)| p)
            .sum();
        assert_close(p_m, 0.25);
    }

    // Sampled through mid-circuit measurement: deterministic for the seed,
    // and statistically consistent with the exact value.
    let circuit = teleport(theta, true);
    assert!(!circuit.has_only_terminal_measurements());
    let counts = run(&circuit, 20000, 99).unwrap();
    let ones: u64 = counts
        .iter()
        .filter(|(k, _)| k.starts_with('1'))
        .map(|(_, n)| n)
        .sum();
    let observed = ones as f64 / 20000.0;
    assert!((observed - expected_one).abs() < 0.02, "{observed}");
    assert!(matches!(
        outcome_probabilities(&circuit),
        Err(SimError::NonTerminalMeasurement)
    ));
}

/// Deutsch-Jozsa on `n` input qubits plus one ancilla (qubit `n`).
fn deutsch_jozsa(n: u32, oracle: impl Fn(&mut Circuit)) -> Circuit {
    let mut c = Circuit::new("dj", n + 1, n);
    c.gate(Gate::X, &[n]);
    for q in 0..=n {
        c.gate(Gate::H, &[q]);
    }
    oracle(&mut c);
    for q in 0..n {
        c.gate(Gate::H, &[q]);
        c.measure(q, q);
    }
    c
}

#[test]
fn deutsch_jozsa_separates_constant_from_balanced() {
    let constant_zero = deutsch_jozsa(3, |_| {});
    let constant_one = deutsch_jozsa(3, |c| {
        c.gate(Gate::X, &[3]);
    });
    // f(x) = x0 ^ x1 ^ x2 is balanced.
    let balanced = deutsch_jozsa(3, |c| {
        for q in 0..3 {
            c.gate(Gate::Cx, &[q, 3]);
        }
    });

    for constant in [&constant_zero, &constant_one] {
        let probs = outcome_probabilities(constant).unwrap();
        assert_eq!(probs.len(), 1);
        assert_close(probs["000"], 1.0);
    }
    let probs = outcome_probabilities(&balanced).unwrap();
    assert!(!probs.contains_key("000"));
    // For the parity function the answer is exactly |111>.
    assert_close(probs["111"], 1.0);
}

fn ccz(c: &mut Circuit, a: u32, b: u32, t: u32) {
    c.gate(Gate::H, &[t])
        .gate(Gate::Ccx, &[a, b, t])
        .gate(Gate::H, &[t]);
}

#[test]
fn grover_two_qubits_finds_the_marked_state_with_certainty() {
    let mut c = Circuit::new("grover2", 2, 2);
    c.gate(Gate::H, &[0]).gate(Gate::H, &[1]);
    // Oracle marking |11>.
    c.gate(Gate::Cz, &[0, 1]);
    // Diffuser.
    c.gate(Gate::H, &[0]).gate(Gate::H, &[1]);
    c.gate(Gate::X, &[0]).gate(Gate::X, &[1]);
    c.gate(Gate::Cz, &[0, 1]);
    c.gate(Gate::X, &[0]).gate(Gate::X, &[1]);
    c.gate(Gate::H, &[0]).gate(Gate::H, &[1]);
    c.measure_all();

    let probs = outcome_probabilities(&c).unwrap();
    assert_eq!(probs.len(), 1);
    assert_close(probs["11"], 1.0);
}

#[test]
fn grover_three_qubits_matches_sin_squared_formula() {
    // Marked state |q2 q1 q0> = |101>. With N = 8 and one marked item,
    // sin(theta) = 1/sqrt(8) and k iterations succeed with probability
    // sin^2((2k+1) theta): 25/32 after one, 121/128 after two.
    let grover = |iterations: usize| {
        let mut c = Circuit::new("grover3", 3, 3);
        for q in 0..3 {
            c.gate(Gate::H, &[q]);
        }
        for _ in 0..iterations {
            c.gate(Gate::X, &[1]);
            ccz(&mut c, 0, 1, 2);
            c.gate(Gate::X, &[1]);
            for q in 0..3 {
                c.gate(Gate::H, &[q]).gate(Gate::X, &[q]);
            }
            ccz(&mut c, 0, 1, 2);
            for q in 0..3 {
                c.gate(Gate::X, &[q]).gate(Gate::H, &[q]);
            }
        }
        c.measure_all();
        c
    };

    let one = outcome_probabilities(&grover(1)).unwrap();
    assert_close(one["101"], 25.0 / 32.0);
    let others: f64 = one
        .iter()
        .filter(|(k, _)| *k != "101")
        .map(|(_, p)| p)
        .sum();
    assert_close(others, 7.0 / 32.0);

    let two = outcome_probabilities(&grover(2)).unwrap();
    assert_close(two["101"], 121.0 / 128.0);
}

/// The quantum Fourier transform on `n` qubits, Qiskit's construction:
/// `QFT|x> = 2^{-n/2} sum_k e^{2 pi i x k / 2^n} |k>`.
pub(crate) fn qft(n: u32) -> Circuit {
    let mut c = Circuit::new("qft", n, 0);
    for j in (0..n).rev() {
        c.gate(Gate::H, &[j]);
        for k in (0..j).rev() {
            c.gate(Gate::Cp(PI / f64::from(1u32 << (j - k))), &[k, j]);
        }
    }
    for i in 0..n / 2 {
        c.gate(Gate::Swap, &[i, n - 1 - i]);
    }
    c
}

#[test]
fn qft_matches_its_definition_on_every_basis_state() {
    let n = 3;
    let dim = 1usize << n;
    let u = unitary(&qft(n)).unwrap();
    let norm = 1.0 / (dim as f64).sqrt();
    for x in 0..dim {
        for k in 0..dim {
            let angle = 2.0 * PI * (x * k) as f64 / dim as f64;
            assert_amp(u[k][x], norm * angle.cos(), norm * angle.sin());
        }
    }
}

#[test]
fn qft_round_trip_returns_every_basis_state() {
    let n = 4;
    let forward = qft(n);
    let inverse = forward.inverse().unwrap();
    for x in 0..(1u32 << n) {
        let mut c = Circuit::new("roundtrip", n, 0);
        for q in 0..n {
            if x & (1 << q) != 0 {
                c.gate(Gate::X, &[q]);
            }
        }
        c.append(&forward);
        c.append(&inverse);
        let s = state(&c);
        for (i, a) in s.amplitudes().iter().enumerate() {
            let expected = if i == x as usize { 1.0 } else { 0.0 };
            assert_amp(*a, expected, 0.0);
        }
    }
}

#[test]
fn reset_and_mid_circuit_measurement_collapse_per_shot() {
    // X then reset: always 0.
    let mut c = Circuit::new("reset", 1, 1);
    c.gate(Gate::X, &[0]).reset(0).measure(0, 0);
    let counts = run(&c, 100, 3).unwrap();
    assert_eq!(counts.get("0"), Some(&100));

    // Measure a |+> qubit twice: both reads always agree.
    let mut c = Circuit::new("twice", 1, 2);
    c.gate(Gate::H, &[0]).measure(0, 0).measure(0, 1);
    let counts = run(&c, 500, 5).unwrap();
    assert!(counts.keys().all(|k| k == "00" || k == "11"));
    assert_eq!(counts.len(), 2);
}

#[test]
fn registers_format_like_qiskit_counts() {
    let mut c = Circuit {
        name: "regs".into(),
        qregs: vec![Register::new("q", 3)],
        cregs: vec![Register::new("a", 1), Register::new("b", 2)],
        ops: vec![],
    };
    // q0 -> a[0] = 1, q1 -> b[0] = 0, q2 -> b[1] = 1.
    c.gate(Gate::X, &[0]).gate(Gate::X, &[2]);
    c.measure(0, 0).measure(1, 1).measure(2, 2);
    let probs = outcome_probabilities(&c).unwrap();
    assert_eq!(probs.keys().collect::<Vec<_>>(), ["10 1"]);
}

#[test]
fn too_many_qubits_is_an_error_not_an_allocation() {
    assert!(matches!(
        StateVector::new(MAX_QUBITS + 1),
        Err(SimError::TooManyQubits { .. })
    ));
}

#[test]
fn global_phase_comparison() {
    // Z = H X H exactly; and Rz(theta) equals P(theta) up to e^{-i theta/2}.
    let mut z = Circuit::new("z", 1, 0);
    z.gate(Gate::Z, &[0]);
    let mut hxh = Circuit::new("hxh", 1, 0);
    hxh.gate(Gate::H, &[0])
        .gate(Gate::X, &[0])
        .gate(Gate::H, &[0]);
    assert!(equal_up_to_global_phase(
        &unitary(&z).unwrap(),
        &unitary(&hxh).unwrap(),
        1e-9
    ));

    let mut rz = Circuit::new("rz", 1, 0);
    rz.gate(Gate::Rz(0.8), &[0]);
    let mut p = Circuit::new("p", 1, 0);
    p.gate(Gate::P(0.8), &[0]);
    assert!(equal_up_to_global_phase(
        &unitary(&rz).unwrap(),
        &unitary(&p).unwrap(),
        1e-9
    ));
    let mut x = Circuit::new("x", 1, 0);
    x.gate(Gate::X, &[0]);
    assert!(!equal_up_to_global_phase(
        &unitary(&z).unwrap(),
        &unitary(&x).unwrap(),
        1e-9
    ));
}

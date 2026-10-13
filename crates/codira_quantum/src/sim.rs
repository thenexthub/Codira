//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - A dense state-vector simulator for [`Circuit`]s: complex `f64` amplitudes,
//!   up to [`MAX_QUBITS`] qubits, and a seeded generator so a given `(circuit,
//!   shots, seed)` always produces the same counts.
//!
//! Conventions match Qiskit's, so results can be compared with it directly:
//! qubit `k` is bit `k` of a basis-state index (little-endian), and a
//! measurement outcome is written with the highest classical bit first,
//! registers separated by a space and the last-declared register leftmost.
//!
//! Two execution paths, chosen per circuit:
//! - every measurement terminal (nothing touches a qubit after it is measured,
//!   no reset): one state vector is computed, and shots are drawn from its
//!   outcome distribution;
//! - otherwise: the state before the first measurement or reset is computed
//!   once, and each shot continues from a copy of it, collapsing on every
//!   measurement and reset.

use std::collections::BTreeMap;

use crate::circuit::{Circuit, CircuitError, Gate, Op};

/// The largest circuit the dense simulator accepts. 2^26 amplitudes of 16
/// bytes each is 1 GiB.
pub const MAX_QUBITS: u32 = 26;

/// A complex number with `f64` parts.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Complex {
    pub re: f64,
    pub im: f64,
}

impl Complex {
    pub const ZERO: Complex = Complex { re: 0.0, im: 0.0 };
    pub const ONE: Complex = Complex { re: 1.0, im: 0.0 };
    pub const I: Complex = Complex { re: 0.0, im: 1.0 };

    pub const fn new(re: f64, im: f64) -> Self {
        Complex { re, im }
    }

    /// `e^{i theta}`.
    pub fn cis(theta: f64) -> Self {
        Complex::new(theta.cos(), theta.sin())
    }

    pub fn conj(self) -> Self {
        Complex::new(self.re, -self.im)
    }

    /// `|z|^2`.
    pub fn norm_sqr(self) -> f64 {
        self.re * self.re + self.im * self.im
    }

    pub fn scale(self, k: f64) -> Self {
        Complex::new(self.re * k, self.im * k)
    }
}

impl std::ops::Add for Complex {
    type Output = Complex;
    fn add(self, rhs: Complex) -> Complex {
        Complex::new(self.re + rhs.re, self.im + rhs.im)
    }
}

impl std::ops::Sub for Complex {
    type Output = Complex;
    fn sub(self, rhs: Complex) -> Complex {
        Complex::new(self.re - rhs.re, self.im - rhs.im)
    }
}

impl std::ops::Mul for Complex {
    type Output = Complex;
    fn mul(self, rhs: Complex) -> Complex {
        Complex::new(
            self.re * rhs.re - self.im * rhs.im,
            self.re * rhs.im + self.im * rhs.re,
        )
    }
}

impl std::ops::Neg for Complex {
    type Output = Complex;
    fn neg(self) -> Complex {
        Complex::new(-self.re, -self.im)
    }
}

/// A 2x2 complex matrix, row-major.
pub type Matrix2 = [[Complex; 2]; 2];

/// The matrix of a single-qubit gate; `None` for multi-qubit gates.
pub fn single_qubit_matrix(gate: Gate) -> Option<Matrix2> {
    use std::f64::consts::FRAC_1_SQRT_2;
    let c = Complex::new;
    let z = Complex::ZERO;
    let one = Complex::ONE;
    let m = match gate {
        Gate::Id => [[one, z], [z, one]],
        Gate::H => [
            [c(FRAC_1_SQRT_2, 0.0), c(FRAC_1_SQRT_2, 0.0)],
            [c(FRAC_1_SQRT_2, 0.0), c(-FRAC_1_SQRT_2, 0.0)],
        ],
        Gate::X => [[z, one], [one, z]],
        Gate::Y => [[z, c(0.0, -1.0)], [c(0.0, 1.0), z]],
        Gate::Z => [[one, z], [z, c(-1.0, 0.0)]],
        Gate::S => [[one, z], [z, Complex::I]],
        Gate::Sdg => [[one, z], [z, c(0.0, -1.0)]],
        Gate::T => [[one, z], [z, Complex::cis(std::f64::consts::FRAC_PI_4)]],
        Gate::Tdg => [[one, z], [z, Complex::cis(-std::f64::consts::FRAC_PI_4)]],
        Gate::Sx => [[c(0.5, 0.5), c(0.5, -0.5)], [c(0.5, -0.5), c(0.5, 0.5)]],
        Gate::Rx(theta) => {
            let (s, co) = (theta / 2.0).sin_cos();
            [[c(co, 0.0), c(0.0, -s)], [c(0.0, -s), c(co, 0.0)]]
        }
        Gate::Ry(theta) => {
            let (s, co) = (theta / 2.0).sin_cos();
            [[c(co, 0.0), c(-s, 0.0)], [c(s, 0.0), c(co, 0.0)]]
        }
        Gate::Rz(theta) => [
            [Complex::cis(-theta / 2.0), z],
            [z, Complex::cis(theta / 2.0)],
        ],
        Gate::P(lambda) => [[one, z], [z, Complex::cis(lambda)]],
        Gate::U(theta, phi, lambda) => {
            let (s, co) = (theta / 2.0).sin_cos();
            [
                [c(co, 0.0), -Complex::cis(lambda).scale(s)],
                [
                    Complex::cis(phi).scale(s),
                    Complex::cis(phi + lambda).scale(co),
                ],
            ]
        }
        Gate::Cx
        | Gate::Cy
        | Gate::Cz
        | Gate::Ch
        | Gate::Cp(_)
        | Gate::Crx(_)
        | Gate::Cry(_)
        | Gate::Crz(_)
        | Gate::Swap
        | Gate::Ccx
        | Gate::Cswap => return None,
    };
    Some(m)
}

/// A failure to simulate.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum SimError {
    #[error(
        "the circuit has {requested} qubits; the dense state-vector simulator handles at most {max}"
    )]
    TooManyQubits { requested: u32, max: u32 },
    #[error("not enough memory for a {0}-qubit state vector")]
    OutOfMemory(u32),
    #[error(transparent)]
    Invalid(#[from] CircuitError),
    #[error(
        "the circuit measures a qubit and then uses it again (or resets one), so it has no single \
         final state; sample it with `run` instead"
    )]
    NonTerminalMeasurement,
}

/// The state of `num_qubits` qubits as `2^num_qubits` complex amplitudes.
#[derive(Debug, Clone, PartialEq)]
pub struct StateVector {
    num_qubits: u32,
    amps: Vec<Complex>,
}

impl StateVector {
    /// |0...0> on `num_qubits` qubits.
    pub fn new(num_qubits: u32) -> Result<Self, SimError> {
        if num_qubits > MAX_QUBITS {
            return Err(SimError::TooManyQubits {
                requested: num_qubits,
                max: MAX_QUBITS,
            });
        }
        let len = 1usize << num_qubits;
        let mut amps = Vec::new();
        amps.try_reserve_exact(len)
            .map_err(|_e| SimError::OutOfMemory(num_qubits))?;
        amps.resize(len, Complex::ZERO);
        amps[0] = Complex::ONE;
        Ok(StateVector { num_qubits, amps })
    }

    pub fn num_qubits(&self) -> u32 {
        self.num_qubits
    }

    pub fn amplitudes(&self) -> &[Complex] {
        &self.amps
    }

    /// `|amplitude|^2` for every basis state, indexed like the amplitudes.
    pub fn probabilities(&self) -> Vec<f64> {
        self.amps.iter().map(|a| a.norm_sqr()).collect()
    }

    /// Applies `gate` to `qubits` (controls first, target last).
    ///
    /// # Panics
    ///
    /// If `qubits` does not have the gate's arity or names a qubit out of
    /// range. [`Circuit::validate`] rules both out, and the circuit-level
    /// entry points call it first.
    pub fn apply(&mut self, gate: Gate, qubits: &[u32]) {
        assert_eq!(qubits.len(), gate.num_qubits(), "gate arity");
        assert!(
            qubits.iter().all(|&q| q < self.num_qubits),
            "qubit out of range"
        );
        if let Some(m) = single_qubit_matrix(gate) {
            self.apply_controlled(&[], qubits[0], &m);
            return;
        }
        let (controls, target, base) = match gate {
            Gate::Cx => (&qubits[..1], qubits[1], Gate::X),
            Gate::Cy => (&qubits[..1], qubits[1], Gate::Y),
            Gate::Cz => (&qubits[..1], qubits[1], Gate::Z),
            Gate::Ch => (&qubits[..1], qubits[1], Gate::H),
            Gate::Cp(l) => (&qubits[..1], qubits[1], Gate::P(l)),
            Gate::Crx(t) => (&qubits[..1], qubits[1], Gate::Rx(t)),
            Gate::Cry(t) => (&qubits[..1], qubits[1], Gate::Ry(t)),
            Gate::Crz(t) => (&qubits[..1], qubits[1], Gate::Rz(t)),
            Gate::Ccx => (&qubits[..2], qubits[2], Gate::X),
            Gate::Swap => {
                self.apply_controlled_swap(&[], qubits[0], qubits[1]);
                return;
            }
            Gate::Cswap => {
                self.apply_controlled_swap(&qubits[..1], qubits[1], qubits[2]);
                return;
            }
            _ => unreachable!("single-qubit gates are handled above"),
        };
        let m = single_qubit_matrix(base).expect("base gate is single-qubit");
        self.apply_controlled(controls, target, &m);
    }

    /// Applies `m` to `target` on the subspace where every control is |1>.
    fn apply_controlled(&mut self, controls: &[u32], target: u32, m: &Matrix2) {
        let mask: usize = controls.iter().fold(0, |acc, &c| acc | (1 << c));
        let t = target as usize;
        let low = (1usize << t) - 1;
        let half = self.amps.len() / 2;
        for k in 0..half {
            // Insert a zero at bit `t` of `k`: every index whose target bit
            // is 0, visited once.
            let i = ((k >> t) << (t + 1)) | (k & low);
            if i & mask != mask {
                continue;
            }
            let j = i | (1 << t);
            let (a, b) = (self.amps[i], self.amps[j]);
            self.amps[i] = m[0][0] * a + m[0][1] * b;
            self.amps[j] = m[1][0] * a + m[1][1] * b;
        }
    }

    fn apply_controlled_swap(&mut self, controls: &[u32], a: u32, b: u32) {
        let mask: usize = controls.iter().fold(0, |acc, &c| acc | (1 << c));
        let (ba, bb) = (1usize << a, 1usize << b);
        for i in 0..self.amps.len() {
            // Visit each (a=1, b=0) index once and swap it with its partner.
            if i & mask == mask && i & ba != 0 && i & bb == 0 {
                self.amps.swap(i, (i & !ba) | bb);
            }
        }
    }

    /// The probability that measuring `qubit` gives 1.
    pub fn probability_of_one(&self, qubit: u32) -> f64 {
        let bit = 1usize << qubit;
        self.amps
            .iter()
            .enumerate()
            .filter(|(i, _)| i & bit != 0)
            .map(|(_, a)| a.norm_sqr())
            .sum()
    }

    /// Measures `qubit`, collapsing the state; `r` in `[0, 1)` decides the
    /// outcome.
    fn measure_with(&mut self, qubit: u32, r: f64) -> bool {
        let p1 = self.probability_of_one(qubit);
        let outcome = r < p1;
        let keep_prob = if outcome { p1 } else { 1.0 - p1 };
        let norm = if keep_prob > 0.0 {
            1.0 / keep_prob.sqrt()
        } else {
            0.0
        };
        let bit = 1usize << qubit;
        for (i, a) in self.amps.iter_mut().enumerate() {
            if (i & bit != 0) == outcome {
                *a = a.scale(norm);
            } else {
                *a = Complex::ZERO;
            }
        }
        outcome
    }
}

/// The state a measurement-terminal circuit leaves its qubits in just before
/// its measurements. Barriers are ignored; a reset before a qubit's first use
/// is a no-op and accepted.
pub fn final_state(circuit: &Circuit) -> Result<StateVector, SimError> {
    circuit.validate()?;
    if !circuit.has_only_terminal_measurements() {
        return Err(SimError::NonTerminalMeasurement);
    }
    let mut state = StateVector::new(circuit.num_qubits())?;
    for op in &circuit.ops {
        if let Op::Gate { gate, qubits } = op {
            state.apply(*gate, qubits);
        }
    }
    Ok(state)
}

/// The exact probability of every outcome of a measurement-terminal
/// circuit, keyed like [`Counts`]. Outcomes with probability below `1e-12`
/// are omitted.
pub fn outcome_probabilities(circuit: &Circuit) -> Result<BTreeMap<String, f64>, SimError> {
    let state = final_state(circuit)?;
    let mapping = measurement_map(circuit);
    let mut out = BTreeMap::new();
    for (index, p) in state.probabilities().into_iter().enumerate() {
        if p < 1e-12 {
            continue;
        }
        let bits = clbits_for_basis_state(circuit.num_clbits(), &mapping, index);
        *out.entry(format_outcome(circuit, &bits)).or_insert(0.0) += p;
    }
    Ok(out)
}

/// Shot counts keyed by outcome string (see the module doc for the format).
pub type Counts = BTreeMap<String, u64>;

/// Samples `shots` executions of `circuit` with a generator seeded by `seed`.
pub fn run(circuit: &Circuit, shots: u64, seed: u64) -> Result<Counts, SimError> {
    circuit.validate()?;
    let mut rng = Rng::new(seed);
    let mut counts = Counts::new();

    if circuit.has_only_terminal_measurements() {
        let state = final_state(circuit)?;
        let mapping = measurement_map(circuit);
        let cumulative: Vec<f64> = state
            .probabilities()
            .into_iter()
            .scan(0.0, |acc, p| {
                *acc += p;
                Some(*acc)
            })
            .collect();
        let total = *cumulative.last().unwrap_or(&1.0);
        for _ in 0..shots {
            let r = rng.next_f64() * total;
            let index = cumulative
                .partition_point(|&c| c <= r)
                .min(cumulative.len() - 1);
            let bits = clbits_for_basis_state(circuit.num_clbits(), &mapping, index);
            *counts.entry(format_outcome(circuit, &bits)).or_insert(0) += 1;
        }
        return Ok(counts);
    }

    // Everything before the first measurement or reset is deterministic and
    // shared by every shot.
    let split = circuit
        .ops
        .iter()
        .position(|op| matches!(op, Op::Measure { .. } | Op::Reset { .. }))
        .unwrap_or(circuit.ops.len());
    let mut prefix = StateVector::new(circuit.num_qubits())?;
    for op in &circuit.ops[..split] {
        if let Op::Gate { gate, qubits } = op {
            prefix.apply(*gate, qubits);
        }
    }

    for _ in 0..shots {
        let mut state = prefix.clone();
        let mut bits = vec![false; circuit.num_clbits() as usize];
        for op in &circuit.ops[split..] {
            match op {
                Op::Gate { gate, qubits } => state.apply(*gate, qubits),
                Op::Measure { qubit, clbit } => {
                    bits[*clbit as usize] = state.measure_with(*qubit, rng.next_f64());
                }
                Op::Reset { qubit } => {
                    if state.measure_with(*qubit, rng.next_f64()) {
                        state.apply(Gate::X, &[*qubit]);
                    }
                }
                Op::Barrier { .. } => {}
            }
        }
        *counts.entry(format_outcome(circuit, &bits)).or_insert(0) += 1;
    }
    Ok(counts)
}

/// For each classical bit, the qubit whose (last) measurement writes it.
fn measurement_map(circuit: &Circuit) -> Vec<Option<u32>> {
    let mut map = vec![None; circuit.num_clbits() as usize];
    for op in &circuit.ops {
        if let Op::Measure { qubit, clbit } = op {
            map[*clbit as usize] = Some(*qubit);
        }
    }
    map
}

fn clbits_for_basis_state(num_clbits: u32, mapping: &[Option<u32>], index: usize) -> Vec<bool> {
    (0..num_clbits as usize)
        .map(|c| mapping[c].is_some_and(|q| index & (1 << q) != 0))
        .collect()
}

/// Formats classical bits the way Qiskit keys its counts: last register
/// first, each register's highest bit first, registers separated by spaces.
pub fn format_outcome(circuit: &Circuit, bits: &[bool]) -> String {
    let mut parts = Vec::with_capacity(circuit.cregs.len());
    let mut start = 0usize;
    for reg in &circuit.cregs {
        let size = reg.size as usize;
        let part: String = (start..start + size)
            .rev()
            .map(|i| if bits[i] { '1' } else { '0' })
            .collect();
        parts.push(part);
        start += size;
    }
    parts.reverse();
    parts.join(" ")
}

/// The full unitary of a measurement-free circuit as a dense matrix
/// (`matrix[row][col]`), for testing gate decompositions. At most 10 qubits.
// Matrix code reads best with explicit row/column indices.
#[allow(clippy::needless_range_loop)]
pub fn unitary(circuit: &Circuit) -> Result<Vec<Vec<Complex>>, SimError> {
    circuit.validate()?;
    let n = circuit.num_qubits();
    if n > 10 {
        return Err(SimError::TooManyQubits {
            requested: n,
            max: 10,
        });
    }
    let dim = 1usize << n;
    let mut matrix = vec![vec![Complex::ZERO; dim]; dim];
    for col in 0..dim {
        let mut state = StateVector::new(n)?;
        state.amps[0] = Complex::ZERO;
        state.amps[col] = Complex::ONE;
        for op in &circuit.ops {
            match op {
                Op::Gate { gate, qubits } => state.apply(*gate, qubits),
                Op::Barrier { .. } => {}
                Op::Measure { .. } | Op::Reset { .. } => {
                    return Err(SimError::NonTerminalMeasurement)
                }
            }
        }
        for (row, amp) in state.amps.iter().enumerate() {
            matrix[row][col] = *amp;
        }
    }
    Ok(matrix)
}

/// Whether two unitaries are equal up to a global phase, within `tol` per
/// entry.
pub fn equal_up_to_global_phase(a: &[Vec<Complex>], b: &[Vec<Complex>], tol: f64) -> bool {
    if a.len() != b.len() {
        return false;
    }
    // Fix the phase on the largest entry of `a`, then compare everything.
    let mut best = (0, 0, 0.0);
    for (r, row) in a.iter().enumerate() {
        for (c, v) in row.iter().enumerate() {
            if v.norm_sqr() > best.2 {
                best = (r, c, v.norm_sqr());
            }
        }
    }
    let (r, c, mag) = best;
    if mag == 0.0 || b[r][c].norm_sqr() == 0.0 {
        return false;
    }
    // phase = b[r][c] / a[r][c], normalised to unit modulus.
    let ratio = b[r][c] * a[r][c].conj();
    let phase = ratio.scale(1.0 / ratio.norm_sqr().sqrt());
    a.iter().zip(b).all(|(ra, rb)| {
        ra.iter()
            .zip(rb)
            .all(|(x, y)| (*x * phase - *y).norm_sqr().sqrt() <= tol)
    })
}

/// xoshiro256** seeded through SplitMix64: small, fast, and identical on
/// every platform, which is the property that matters here -- a seed means
/// the same counts everywhere.
#[derive(Debug, Clone)]
pub struct Rng {
    s: [u64; 4],
}

impl Rng {
    pub fn new(seed: u64) -> Self {
        let mut sm = seed;
        let mut next = || {
            sm = sm.wrapping_add(0x9E37_79B9_7F4A_7C15);
            let mut z = sm;
            z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
            z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
            z ^ (z >> 31)
        };
        Rng {
            s: [next(), next(), next(), next()],
        }
    }

    pub fn next_u64(&mut self) -> u64 {
        let result = self.s[1].wrapping_mul(5).rotate_left(7).wrapping_mul(9);
        let t = self.s[1] << 17;
        self.s[2] ^= self.s[0];
        self.s[3] ^= self.s[1];
        self.s[1] ^= self.s[2];
        self.s[0] ^= self.s[3];
        self.s[2] ^= t;
        self.s[3] = self.s[3].rotate_left(45);
        result
    }

    /// Uniform in `[0, 1)` with 53 bits of precision.
    pub fn next_f64(&mut self) -> f64 {
        (self.next_u64() >> 11) as f64 * (1.0 / (1u64 << 53) as f64)
    }
}

#[cfg(test)]
mod tests;

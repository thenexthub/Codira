//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - The circuit IR every other part of this crate reads or writes: named qubit
//!   and classical-bit registers, a fixed gate set, measurement, reset and
//!   barrier, plus structural validation and the extra restrictions the QIR
//!   base profile imposes.
//!
//! Qubits and classical bits are addressed by *flat* indices. Registers only
//! name contiguous ranges of those indices -- `qreg a[2]; qreg b[3];` makes
//! `b[0]` flat qubit 2 -- which is exactly the numbering QIR uses (one
//! `inttoptr (i64 N to ptr)` per qubit) and the numbering Qiskit uses for
//! `circuit.qubits`. Keeping one numbering everywhere means no emitter has to
//! translate.

use std::fmt;

/// A named, contiguous range of qubits or classical bits.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Register {
    pub name: String,
    pub size: u32,
}

impl Register {
    pub fn new(name: impl Into<String>, size: u32) -> Self {
        Register {
            name: name.into(),
            size,
        }
    }
}

/// The gates the IR represents directly.
///
/// The set is Qiskit's standard-gate vocabulary restricted to what
/// `qelib1.inc` (OpenQASM 2) and `stdgates.inc` (OpenQASM 3) both name, so
/// every gate round-trips through either text format under its own name.
/// Angles are radians. For controlled gates the control qubit(s) come first
/// and the target last, as in Qiskit and OpenQASM.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Gate {
    Id,
    H,
    X,
    Y,
    Z,
    S,
    Sdg,
    T,
    Tdg,
    Sx,
    Rx(f64),
    Ry(f64),
    Rz(f64),
    /// Phase gate `diag(1, e^{i lambda})` (`u1`/`p`).
    P(f64),
    /// The general single-qubit gate `U(theta, phi, lambda)` (`u3`), with
    /// Qiskit's and OpenQASM 3's matrix convention.
    U(f64, f64, f64),
    Cx,
    Cy,
    Cz,
    Ch,
    /// Controlled phase `diag(1, 1, 1, e^{i lambda})` (`cu1`/`cp`).
    Cp(f64),
    Crx(f64),
    Cry(f64),
    Crz(f64),
    Swap,
    Ccx,
    Cswap,
}

impl Gate {
    /// How many qubits the gate acts on.
    pub fn num_qubits(self) -> usize {
        match self {
            Gate::Id
            | Gate::H
            | Gate::X
            | Gate::Y
            | Gate::Z
            | Gate::S
            | Gate::Sdg
            | Gate::T
            | Gate::Tdg
            | Gate::Sx
            | Gate::Rx(_)
            | Gate::Ry(_)
            | Gate::Rz(_)
            | Gate::P(_)
            | Gate::U(..) => 1,
            Gate::Cx
            | Gate::Cy
            | Gate::Cz
            | Gate::Ch
            | Gate::Cp(_)
            | Gate::Crx(_)
            | Gate::Cry(_)
            | Gate::Crz(_)
            | Gate::Swap => 2,
            Gate::Ccx | Gate::Cswap => 3,
        }
    }

    /// The gate's lowercase OpenQASM 3 / Qiskit name.
    pub fn name(self) -> &'static str {
        match self {
            Gate::Id => "id",
            Gate::H => "h",
            Gate::X => "x",
            Gate::Y => "y",
            Gate::Z => "z",
            Gate::S => "s",
            Gate::Sdg => "sdg",
            Gate::T => "t",
            Gate::Tdg => "tdg",
            Gate::Sx => "sx",
            Gate::Rx(_) => "rx",
            Gate::Ry(_) => "ry",
            Gate::Rz(_) => "rz",
            Gate::P(_) => "p",
            Gate::U(..) => "u",
            Gate::Cx => "cx",
            Gate::Cy => "cy",
            Gate::Cz => "cz",
            Gate::Ch => "ch",
            Gate::Cp(_) => "cp",
            Gate::Crx(_) => "crx",
            Gate::Cry(_) => "cry",
            Gate::Crz(_) => "crz",
            Gate::Swap => "swap",
            Gate::Ccx => "ccx",
            Gate::Cswap => "cswap",
        }
    }

    /// The gate's angle parameters, in OpenQASM argument order.
    pub fn params(self) -> Vec<f64> {
        match self {
            Gate::Rx(a)
            | Gate::Ry(a)
            | Gate::Rz(a)
            | Gate::P(a)
            | Gate::Cp(a)
            | Gate::Crx(a)
            | Gate::Cry(a)
            | Gate::Crz(a) => vec![a],
            Gate::U(theta, phi, lambda) => vec![theta, phi, lambda],
            _ => Vec::new(),
        }
    }

    /// Builds a gate from its name and parameters. Accepts the aliases
    /// OpenQASM 2, OpenQASM 3 and Qiskit use for the same matrices (`cnot`,
    /// `CX`, `u3`, `U`, `u1`, `phase`, `cu1`, `cphase`). Returns `None` for an
    /// unknown name or the wrong number of parameters.
    pub fn from_name(name: &str, params: &[f64]) -> Option<Gate> {
        let gate = match (name, params) {
            ("id" | "i", []) => Gate::Id,
            ("h", []) => Gate::H,
            ("x", []) => Gate::X,
            ("y", []) => Gate::Y,
            ("z", []) => Gate::Z,
            ("s", []) => Gate::S,
            ("sdg", []) => Gate::Sdg,
            ("t", []) => Gate::T,
            ("tdg", []) => Gate::Tdg,
            ("sx", []) => Gate::Sx,
            ("rx", [a]) => Gate::Rx(*a),
            ("ry", [a]) => Gate::Ry(*a),
            ("rz", [a]) => Gate::Rz(*a),
            ("p" | "u1" | "phase", [a]) => Gate::P(*a),
            ("u" | "u3" | "U", [theta, phi, lambda]) => Gate::U(*theta, *phi, *lambda),
            ("u2", [phi, lambda]) => Gate::U(std::f64::consts::FRAC_PI_2, *phi, *lambda),
            ("cx" | "CX" | "cnot", []) => Gate::Cx,
            ("cy", []) => Gate::Cy,
            ("cz", []) => Gate::Cz,
            ("ch", []) => Gate::Ch,
            ("cp" | "cu1" | "cphase", [a]) => Gate::Cp(*a),
            ("crx", [a]) => Gate::Crx(*a),
            ("cry", [a]) => Gate::Cry(*a),
            ("crz", [a]) => Gate::Crz(*a),
            ("swap", []) => Gate::Swap,
            ("ccx" | "toffoli", []) => Gate::Ccx,
            ("cswap" | "fredkin", []) => Gate::Cswap,
            _ => return None,
        };
        Some(gate)
    }

    /// The inverse gate. Exact for every gate except `sx`, whose inverse is
    /// returned as a `U` that differs from `sx^dagger` by a global phase
    /// (unobservable, but not "exact").
    pub fn inverse(self) -> Gate {
        match self {
            Gate::S => Gate::Sdg,
            Gate::Sdg => Gate::S,
            Gate::T => Gate::Tdg,
            Gate::Tdg => Gate::T,
            Gate::Rx(a) => Gate::Rx(-a),
            Gate::Ry(a) => Gate::Ry(-a),
            Gate::Rz(a) => Gate::Rz(-a),
            Gate::P(a) => Gate::P(-a),
            Gate::Cp(a) => Gate::Cp(-a),
            Gate::Crx(a) => Gate::Crx(-a),
            Gate::Cry(a) => Gate::Cry(-a),
            Gate::Crz(a) => Gate::Crz(-a),
            // U(theta, phi, lambda)^dagger = U(-theta, -lambda, -phi).
            Gate::U(theta, phi, lambda) => Gate::U(-theta, -lambda, -phi),
            // `sx` is not self-inverse and its inverse (`sxdg`) is not in
            // `stdgates.inc`, so it gets no variant of its own:
            // sx^dagger = e^{-i pi/4} U(-pi/2, -pi/2, pi/2).
            Gate::Sx => Gate::U(
                -std::f64::consts::FRAC_PI_2,
                -std::f64::consts::FRAC_PI_2,
                std::f64::consts::FRAC_PI_2,
            ),
            // Every remaining gate is Hermitian and therefore self-inverse.
            Gate::Id
            | Gate::H
            | Gate::X
            | Gate::Y
            | Gate::Z
            | Gate::Cx
            | Gate::Cy
            | Gate::Cz
            | Gate::Ch
            | Gate::Swap
            | Gate::Ccx
            | Gate::Cswap => self,
        }
    }
}

/// One step of a circuit.
#[derive(Debug, Clone, PartialEq)]
pub enum Op {
    Gate {
        gate: Gate,
        qubits: Vec<u32>,
    },
    /// Measure `qubit` in the computational basis into classical bit `clbit`.
    Measure {
        qubit: u32,
        clbit: u32,
    },
    /// Return `qubit` to |0>.
    Reset {
        qubit: u32,
    },
    /// A scheduling fence with no effect on the state.
    Barrier {
        qubits: Vec<u32>,
    },
}

/// A quantum circuit over flat qubit and classical-bit indices.
#[derive(Debug, Clone, PartialEq, Default)]
pub struct Circuit {
    /// The circuit's name: the QIR entry point and the Codira function name
    /// when the circuit is written out as either.
    pub name: String,
    pub qregs: Vec<Register>,
    pub cregs: Vec<Register>,
    pub ops: Vec<Op>,
}

/// A structural problem found by [`Circuit::validate`] or a profile
/// restriction found by [`Circuit::check_base_profile`].
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum CircuitError {
    #[error("operation {op}: qubit {qubit} is out of range (the circuit has {num_qubits} qubits)")]
    QubitOutOfRange {
        op: usize,
        qubit: u32,
        num_qubits: u32,
    },
    #[error(
        "operation {op}: classical bit {clbit} is out of range (the circuit has {num_clbits} classical bits)"
    )]
    ClbitOutOfRange {
        op: usize,
        clbit: u32,
        num_clbits: u32,
    },
    #[error("operation {op}: `{gate}` acts on {expected} qubits but was given {got}")]
    WrongArity {
        op: usize,
        gate: &'static str,
        expected: usize,
        got: usize,
    },
    #[error("operation {op}: `{gate}` uses qubit {qubit} more than once")]
    RepeatedQubit {
        op: usize,
        gate: &'static str,
        qubit: u32,
    },
    #[error("operation {op}: `{gate}` has a non-finite angle")]
    NonFiniteAngle { op: usize, gate: &'static str },
    #[error("register name `{0}` is used twice")]
    DuplicateRegister(String),
    #[error("register name `{0}` is not a valid identifier")]
    InvalidRegisterName(String),
    #[error(
        "operation {op}: qubit {qubit} is used after it was measured; the QIR base profile \
         allows no operation on a qubit after its measurement (no mid-circuit measurement)"
    )]
    UseAfterMeasure { op: usize, qubit: u32 },
    #[error(
        "operation {op}: reset of qubit {qubit} after it was operated on; the QIR base profile \
         has no reset (qubits start in |0>, so only a reset before any other use is accepted, \
         and it is dropped)"
    )]
    ResetInBaseProfile { op: usize, qubit: u32 },
    #[error(
        "operation {op}: classical bit {clbit} is written twice; base-profile QIR records each \
         measurement result once"
    )]
    ClbitWrittenTwice { op: usize, clbit: u32 },
    #[error(
        "classical bit {clbit} is never measured; base-profile QIR can only record measurement \
         results, so every recorded bit must be the target of exactly one measurement"
    )]
    ClbitNeverMeasured { clbit: u32 },
}

impl Circuit {
    /// A circuit with one qubit register `q` and one classical register `c`
    /// (either omitted when its size is zero) -- the shape Qiskit gives
    /// `QuantumCircuit(n, m)`.
    pub fn new(name: impl Into<String>, num_qubits: u32, num_clbits: u32) -> Self {
        let mut circuit = Circuit {
            name: name.into(),
            ..Circuit::default()
        };
        if num_qubits > 0 {
            circuit.qregs.push(Register::new("q", num_qubits));
        }
        if num_clbits > 0 {
            circuit.cregs.push(Register::new("c", num_clbits));
        }
        circuit
    }

    pub fn num_qubits(&self) -> u32 {
        self.qregs.iter().map(|r| r.size).sum()
    }

    pub fn num_clbits(&self) -> u32 {
        self.cregs.iter().map(|r| r.size).sum()
    }

    /// Appends a gate. Arity and ranges are checked by [`Circuit::validate`],
    /// not here, so a builder can be written straight through.
    pub fn gate(&mut self, gate: Gate, qubits: &[u32]) -> &mut Self {
        self.ops.push(Op::Gate {
            gate,
            qubits: qubits.to_vec(),
        });
        self
    }

    pub fn measure(&mut self, qubit: u32, clbit: u32) -> &mut Self {
        self.ops.push(Op::Measure { qubit, clbit });
        self
    }

    pub fn reset(&mut self, qubit: u32) -> &mut Self {
        self.ops.push(Op::Reset { qubit });
        self
    }

    pub fn barrier(&mut self, qubits: &[u32]) -> &mut Self {
        self.ops.push(Op::Barrier {
            qubits: qubits.to_vec(),
        });
        self
    }

    /// Measures qubit `i` into classical bit `i` for every qubit, the way
    /// Qiskit's `measure_all` does when the classical register already
    /// exists.
    pub fn measure_all(&mut self) -> &mut Self {
        for i in 0..self.num_qubits().min(self.num_clbits()) {
            self.measure(i, i);
        }
        self
    }

    /// Maps a flat qubit index to its `(register, offset)` pair.
    pub fn qubit_location(&self, qubit: u32) -> Option<(&Register, u32)> {
        locate(&self.qregs, qubit)
    }

    /// Maps a flat classical-bit index to its `(register, offset)` pair.
    pub fn clbit_location(&self, clbit: u32) -> Option<(&Register, u32)> {
        locate(&self.cregs, clbit)
    }

    /// Checks that every operation is well formed: register names are valid
    /// and distinct, each gate gets the number of qubits it acts on, no gate
    /// names the same qubit twice, every index is in range, and every angle
    /// is finite.
    pub fn validate(&self) -> Result<(), CircuitError> {
        let mut seen = std::collections::HashSet::new();
        for reg in self.qregs.iter().chain(&self.cregs) {
            if !is_identifier(&reg.name) {
                return Err(CircuitError::InvalidRegisterName(reg.name.clone()));
            }
            if !seen.insert(reg.name.as_str()) {
                return Err(CircuitError::DuplicateRegister(reg.name.clone()));
            }
        }

        let num_qubits = self.num_qubits();
        let num_clbits = self.num_clbits();
        let check_qubit = |op: usize, qubit: u32| {
            if qubit < num_qubits {
                Ok(())
            } else {
                Err(CircuitError::QubitOutOfRange {
                    op,
                    qubit,
                    num_qubits,
                })
            }
        };

        for (index, op) in self.ops.iter().enumerate() {
            match op {
                Op::Gate { gate, qubits } => {
                    if qubits.len() != gate.num_qubits() {
                        return Err(CircuitError::WrongArity {
                            op: index,
                            gate: gate.name(),
                            expected: gate.num_qubits(),
                            got: qubits.len(),
                        });
                    }
                    for (i, &q) in qubits.iter().enumerate() {
                        check_qubit(index, q)?;
                        if qubits[..i].contains(&q) {
                            return Err(CircuitError::RepeatedQubit {
                                op: index,
                                gate: gate.name(),
                                qubit: q,
                            });
                        }
                    }
                    if gate.params().iter().any(|p| !p.is_finite()) {
                        return Err(CircuitError::NonFiniteAngle {
                            op: index,
                            gate: gate.name(),
                        });
                    }
                }
                Op::Measure { qubit, clbit } => {
                    check_qubit(index, *qubit)?;
                    if *clbit >= num_clbits {
                        return Err(CircuitError::ClbitOutOfRange {
                            op: index,
                            clbit: *clbit,
                            num_clbits,
                        });
                    }
                }
                Op::Reset { qubit } => check_qubit(index, *qubit)?,
                Op::Barrier { qubits } => {
                    for &q in qubits {
                        check_qubit(index, q)?;
                    }
                }
            }
        }
        Ok(())
    }

    /// Checks the restrictions the QIR base profile adds on top of
    /// [`Circuit::validate`] (QIR spec, Base Profile, "Bullet 2" and "Qubit
    /// and Result Usage"): nothing touches a qubit after it has been
    /// measured, each classical bit is written by exactly one measurement,
    /// and a reset only appears before a qubit's first use (where it is a
    /// no-op, since the profile starts every qubit in |0>).
    pub fn check_base_profile(&self) -> Result<(), CircuitError> {
        self.validate()?;
        let mut measured = vec![false; self.num_qubits() as usize];
        let mut touched = vec![false; self.num_qubits() as usize];
        let mut written = vec![false; self.num_clbits() as usize];

        for (index, op) in self.ops.iter().enumerate() {
            match op {
                Op::Gate { qubits, .. } => {
                    for &q in qubits {
                        if measured[q as usize] {
                            return Err(CircuitError::UseAfterMeasure {
                                op: index,
                                qubit: q,
                            });
                        }
                        touched[q as usize] = true;
                    }
                }
                Op::Measure { qubit, clbit } => {
                    if measured[*qubit as usize] {
                        return Err(CircuitError::UseAfterMeasure {
                            op: index,
                            qubit: *qubit,
                        });
                    }
                    if written[*clbit as usize] {
                        return Err(CircuitError::ClbitWrittenTwice {
                            op: index,
                            clbit: *clbit,
                        });
                    }
                    measured[*qubit as usize] = true;
                    touched[*qubit as usize] = true;
                    written[*clbit as usize] = true;
                }
                Op::Reset { qubit } => {
                    if touched[*qubit as usize] {
                        return Err(CircuitError::ResetInBaseProfile {
                            op: index,
                            qubit: *qubit,
                        });
                    }
                }
                Op::Barrier { .. } => {}
            }
        }

        if let Some(clbit) = written.iter().position(|w| !w) {
            return Err(CircuitError::ClbitNeverMeasured {
                clbit: clbit as u32,
            });
        }
        Ok(())
    }

    /// Whether every measurement is terminal: no gate, reset or second
    /// measurement touches a qubit after it has been measured, and every
    /// reset comes before its qubit's first use (where it is a no-op). Such a
    /// circuit's outcome distribution can be computed from one final state
    /// vector instead of one simulation per shot.
    ///
    /// Assumes [`Circuit::validate`] passed.
    pub fn has_only_terminal_measurements(&self) -> bool {
        let mut measured = vec![false; self.num_qubits() as usize];
        let mut touched = vec![false; self.num_qubits() as usize];
        for op in &self.ops {
            match op {
                Op::Gate { qubits, .. } => {
                    for &q in qubits {
                        if measured[q as usize] {
                            return false;
                        }
                        touched[q as usize] = true;
                    }
                }
                Op::Measure { qubit, .. } => {
                    if measured[*qubit as usize] {
                        return false;
                    }
                    measured[*qubit as usize] = true;
                    touched[*qubit as usize] = true;
                }
                Op::Reset { qubit } => {
                    if touched[*qubit as usize] {
                        return false;
                    }
                }
                Op::Barrier { .. } => {}
            }
        }
        true
    }

    /// The circuit with every measurement and barrier removed -- its unitary
    /// part, assuming measurements are terminal.
    pub fn without_measurements(&self) -> Circuit {
        Circuit {
            name: self.name.clone(),
            qregs: self.qregs.clone(),
            cregs: self.cregs.clone(),
            ops: self
                .ops
                .iter()
                .filter(|op| matches!(op, Op::Gate { .. } | Op::Reset { .. }))
                .cloned()
                .collect(),
        }
    }

    /// The inverse of a measurement-free circuit: gates in reverse order,
    /// each inverted. `None` if the circuit measures or resets.
    pub fn inverse(&self) -> Option<Circuit> {
        let mut ops = Vec::with_capacity(self.ops.len());
        for op in self.ops.iter().rev() {
            match op {
                Op::Gate { gate, qubits } => ops.push(Op::Gate {
                    gate: gate.inverse(),
                    qubits: qubits.clone(),
                }),
                Op::Barrier { qubits } => ops.push(Op::Barrier {
                    qubits: qubits.clone(),
                }),
                Op::Measure { .. } | Op::Reset { .. } => return None,
            }
        }
        Some(Circuit {
            name: format!("{}_inverse", self.name),
            qregs: self.qregs.clone(),
            cregs: self.cregs.clone(),
            ops,
        })
    }

    /// Appends `other`'s operations to this circuit. Both must have the same
    /// number of qubits and classical bits.
    pub fn append(&mut self, other: &Circuit) {
        self.ops.extend(other.ops.iter().cloned());
    }
}

fn locate(regs: &[Register], index: u32) -> Option<(&Register, u32)> {
    let mut start = 0;
    for reg in regs {
        if index < start + reg.size {
            return Some((reg, index - start));
        }
        start += reg.size;
    }
    None
}

/// Whether `name` is an identifier in the sense OpenQASM, LLVM labels and
/// Codira all accept: an ASCII letter or `_`, then letters, digits or `_`.
pub fn is_identifier(name: &str) -> bool {
    let mut chars = name.chars();
    matches!(chars.next(), Some(c) if c.is_ascii_alphabetic() || c == '_')
        && chars.all(|c| c.is_ascii_alphanumeric() || c == '_')
}

impl fmt::Display for Circuit {
    /// A compact, line-per-operation listing, for diagnostics and tests.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(
            f,
            "circuit {} ({} qubits, {} clbits)",
            self.name,
            self.num_qubits(),
            self.num_clbits()
        )?;
        for op in &self.ops {
            match op {
                Op::Gate { gate, qubits } => {
                    let params = gate.params();
                    if params.is_empty() {
                        writeln!(f, "  {} {qubits:?}", gate.name())?;
                    } else {
                        writeln!(f, "  {}{params:?} {qubits:?}", gate.name())?;
                    }
                }
                Op::Measure { qubit, clbit } => writeln!(f, "  measure {qubit} -> {clbit}")?,
                Op::Reset { qubit } => writeln!(f, "  reset {qubit}")?,
                Op::Barrier { qubits } => writeln!(f, "  barrier {qubits:?}")?,
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn registers_flatten_in_declaration_order() {
        let circuit = Circuit {
            name: "c".into(),
            qregs: vec![Register::new("a", 2), Register::new("b", 3)],
            cregs: vec![Register::new("m", 1)],
            ops: vec![],
        };
        assert_eq!(circuit.num_qubits(), 5);
        let (reg, offset) = circuit.qubit_location(2).unwrap();
        assert_eq!((reg.name.as_str(), offset), ("b", 0));
        assert!(circuit.qubit_location(5).is_none());
    }

    #[test]
    fn validation_catches_each_structural_error() {
        let mut c = Circuit::new("bad", 2, 1);
        c.gate(Gate::Cx, &[0]);
        assert!(matches!(c.validate(), Err(CircuitError::WrongArity { .. })));

        let mut c = Circuit::new("bad", 2, 1);
        c.gate(Gate::Cx, &[1, 1]);
        assert!(matches!(
            c.validate(),
            Err(CircuitError::RepeatedQubit { .. })
        ));

        let mut c = Circuit::new("bad", 2, 1);
        c.gate(Gate::H, &[2]);
        assert!(matches!(
            c.validate(),
            Err(CircuitError::QubitOutOfRange { qubit: 2, .. })
        ));

        let mut c = Circuit::new("bad", 2, 1);
        c.measure(0, 1);
        assert!(matches!(
            c.validate(),
            Err(CircuitError::ClbitOutOfRange { clbit: 1, .. })
        ));

        let mut c = Circuit::new("bad", 1, 0);
        c.gate(Gate::Rx(f64::NAN), &[0]);
        assert!(matches!(
            c.validate(),
            Err(CircuitError::NonFiniteAngle { .. })
        ));

        let mut c = Circuit::new("bad", 1, 0);
        c.qregs.push(Register::new("q", 1));
        assert!(matches!(
            c.validate(),
            Err(CircuitError::DuplicateRegister(_))
        ));
    }

    #[test]
    fn base_profile_rejects_mid_circuit_measurement_and_late_reset() {
        let mut c = Circuit::new("mid", 1, 1);
        c.gate(Gate::H, &[0]).measure(0, 0).gate(Gate::X, &[0]);
        assert!(matches!(
            c.check_base_profile(),
            Err(CircuitError::UseAfterMeasure { op: 2, qubit: 0 })
        ));

        let mut c = Circuit::new("late_reset", 1, 1);
        c.gate(Gate::H, &[0]).reset(0).measure(0, 0);
        assert!(matches!(
            c.check_base_profile(),
            Err(CircuitError::ResetInBaseProfile { .. })
        ));

        let mut c = Circuit::new("early_reset", 1, 1);
        c.reset(0).gate(Gate::H, &[0]).measure(0, 0);
        assert_eq!(c.check_base_profile(), Ok(()));

        let mut c = Circuit::new("twice", 2, 1);
        c.measure(0, 0).measure(1, 0);
        assert!(matches!(
            c.check_base_profile(),
            Err(CircuitError::ClbitWrittenTwice { .. })
        ));

        let mut c = Circuit::new("unmeasured", 1, 2);
        c.measure(0, 0);
        assert!(matches!(
            c.check_base_profile(),
            Err(CircuitError::ClbitNeverMeasured { clbit: 1 })
        ));
    }

    #[test]
    fn gate_names_round_trip_through_from_name() {
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
            Gate::Rx(0.1),
            Gate::Ry(0.2),
            Gate::Rz(0.3),
            Gate::P(0.4),
            Gate::U(0.5, 0.6, 0.7),
            Gate::Cx,
            Gate::Cy,
            Gate::Cz,
            Gate::Ch,
            Gate::Cp(0.8),
            Gate::Crx(0.9),
            Gate::Cry(1.0),
            Gate::Crz(1.1),
            Gate::Swap,
            Gate::Ccx,
            Gate::Cswap,
        ];
        for gate in gates {
            assert_eq!(Gate::from_name(gate.name(), &gate.params()), Some(gate));
        }
        assert_eq!(Gate::from_name("cx", &[1.0]), None);
        assert_eq!(Gate::from_name("frobnicate", &[]), None);
    }
}

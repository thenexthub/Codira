//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Lowers a word-level [`Circuit`] to a rank-1 constraint system over the
//!   BN254 scalar field, with exact fixed-width integer semantics; computes
//!   witnesses; checks them; and writes the iden3 `.r1cs` and `.wtns` binary
//!   formats that circom and snarkjs read.
//!
//! # Exact integers in a prime field
//!
//! The field has a prime modulus `p` of 254 bits, so field addition is not
//! `i64` addition: the field wraps at `p`, the machine at `2^64`. Every
//! value is therefore kept in one of two forms:
//!
//! * **bits**: `w` variables each constrained to `0` or `1` (`b * (b - 1) =
//!   0`), whose weighted sum is the value's unsigned encoding -- exact;
//! * **number**: a linear combination whose value is an integer below a tracked
//!   bound `2^m` (always `m <= 250`, so far below `p`, so the field never
//!   wraps) and congruent to the true value modulo `2^w`.
//!
//! Additions, subtractions, negations, multiplications by constants, `~`,
//! truncations and constant left shifts stay numbers and cost nothing: the
//! modular reduction is deferred. When a consumer needs the exact value or
//! its bits, the number is *reduced*: `m` bit variables are allocated and
//! constrained to sum to it, which proves it lies in `[0, 2^m)`; the low `w`
//! bits are the value modulo `2^w`. Because `2^m < p`, that decomposition is
//! unique, so the reduction is sound. Inputs are decomposed on entry, which
//! range-checks them to their width.
//!
//! # Constraints per operator (`w` = operand width)
//!
//! | operator | constraints |
//! |---|---|
//! | input | `w + 1` (range check: `w` booleans and one sum) |
//! | `+ - ` unary `-`, `~` on a number, `* k` (constant), `as` narrower | 0 (deferred) |
//! | reduction of a number below `2^m` (when bits or the exact value are needed) | `m + 1` |
//! | `*` | 1 (a product of two numbers; reduced later, typically `2w + 1` bits) |
//! | `& | ^` | 1 per bit whose operands are both variables (`w` for words) |
//! | `<<`, `>>` by a constant | 0 on bits (rewiring) |
//! | `<<`, `>>` by a variable | `w * log2(w)` (a barrel shifter of multiplexers), plus `w + 2` when the amount has bits above `log2(w)` (they force the out-of-range result) |
//! | `==`, `!=` | 2 (an is-zero gadget on the difference of exact values) |
//! | `<` unsigned | `w + 2` (bit `w` of `a + 2^w - b`) |
//! | `<` signed | as unsigned, on bits with the sign bits flipped |
//! | `ct_select` / multiplexer | 1 |
//! | `as` widening | 0 (sign extension reads the top bit) |
//! | output | 1 (the public value equals the exact result) |
//!
//! The table in `tests::constraint_counts_per_operator` checks these numbers
//! on 64-bit operands.
//!
//! # Variables
//!
//! Variable 0 is the constant 1, variable 1 the public output (the revealed
//! value: the result's unsigned encoding), then one private variable per
//! input, then internal variables -- the iden3 order (outputs, public
//! inputs, private inputs, internals), so the `.r1cs` file is exactly this
//! system.

use std::{collections::HashMap, fmt};

pub use ark_bn254::Fr;
use ark_ff::{AdditiveGroup, BigInteger, Field, One, PrimeField, Zero};

use crate::ir::{Circuit, Op, Port, Ty, Wire};

/// A variable index. [`ONE`] is the constant 1.
pub type Var = u32;

/// The variable that is always 1.
pub const ONE: Var = 0;

/// The largest bound (in bits) a number may carry; well below the 254-bit
/// modulus, so sums and differences of numbers never wrap the field.
const LIMIT: u32 = 250;

/// A linear combination: `sum(coefficient * variable)`, sorted by variable,
/// with no zero coefficients.
#[derive(Debug, Clone, Default, PartialEq, Eq, Hash)]
pub struct Lc {
    terms: Vec<(Var, Fr)>,
}

impl Lc {
    pub fn zero() -> Lc {
        Lc::default()
    }

    pub fn constant(c: Fr) -> Lc {
        Lc::term(ONE, c)
    }

    pub fn var(v: Var) -> Lc {
        Lc::term(v, Fr::one())
    }

    pub fn term(v: Var, c: Fr) -> Lc {
        if c.is_zero() {
            Lc::zero()
        } else {
            Lc {
                terms: vec![(v, c)],
            }
        }
    }

    pub fn terms(&self) -> &[(Var, Fr)] {
        &self.terms
    }

    /// `self + k * other`.
    pub fn add_scaled(&self, other: &Lc, k: Fr) -> Lc {
        let mut terms = Vec::with_capacity(self.terms.len() + other.terms.len());
        let (mut i, mut j) = (0, 0);
        while i < self.terms.len() || j < other.terms.len() {
            let next = match (self.terms.get(i), other.terms.get(j)) {
                (Some(&(va, ca)), Some(&(vb, cb))) if va == vb => {
                    i += 1;
                    j += 1;
                    (va, ca + cb * k)
                }
                (Some(&(va, ca)), Some(&(vb, _))) if va < vb => {
                    i += 1;
                    (va, ca)
                }
                (Some(_) | None, Some(&(vb, cb))) => {
                    j += 1;
                    (vb, cb * k)
                }
                (Some(&(va, ca)), None) => {
                    i += 1;
                    (va, ca)
                }
                (None, None) => unreachable!("loop condition"),
            };
            if !next.1.is_zero() {
                terms.push(next);
            }
        }
        Lc { terms }
    }

    pub fn add(&self, other: &Lc) -> Lc {
        self.add_scaled(other, Fr::one())
    }

    pub fn sub(&self, other: &Lc) -> Lc {
        self.add_scaled(other, -Fr::one())
    }

    pub fn scale(&self, k: Fr) -> Lc {
        Lc::zero().add_scaled(self, k)
    }

    /// The value if this combination is a constant.
    pub fn as_constant(&self) -> Option<Fr> {
        match self.terms.as_slice() {
            [] => Some(Fr::zero()),
            [(ONE, c)] => Some(*c),
            _ => None,
        }
    }

    pub fn eval(&self, witness: &[Fr]) -> Fr {
        self.terms
            .iter()
            .map(|(v, c)| witness[*v as usize] * c)
            .sum()
    }
}

/// `a * b = c`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Constraint {
    pub a: Lc,
    pub b: Lc,
    pub c: Lc,
}

/// How the prover computes an internal variable. Hints are not constraints:
/// they only say how to find values that satisfy them.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Hint {
    /// Variables `first .. first + count` are bits `0..count` of `value`
    /// (read as an integer below `p`).
    Bits { value: Lc, first: Var, count: u32 },
    /// `out = a * b + c`.
    Quadratic { out: Var, a: Lc, b: Lc, c: Lc },
    /// `out = 1 / value`, or 0 when `value` is 0.
    Inverse { out: Var, value: Lc },
}

/// A rank-1 constraint system for one circuit, with the hints that compute
/// its witness.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct R1cs {
    /// The source function's name.
    pub name: String,
    /// The source function's signature.
    pub signature: String,
    pub inputs: Vec<Port>,
    pub output_ty: Ty,
    /// Including [`ONE`].
    pub num_vars: u32,
    /// The public output (always variable 1).
    pub output_var: Var,
    /// One private variable per input, in order.
    pub input_vars: Vec<Var>,
    pub constraints: Vec<Constraint>,
    pub hints: Vec<Hint>,
    /// Constraints by the circuit operator that caused them, sorted.
    pub cost: Vec<(&'static str, usize)>,
}

/// Why a witness could not be produced or does not satisfy the system.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum WitnessError {
    WrongInputCount { expected: usize, got: usize },
    InputOutOfRange { name: String, ty: Ty, value: u64 },
    Unsatisfied { constraint: usize },
}

impl fmt::Display for WitnessError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            WitnessError::WrongInputCount { expected, got } => {
                write!(f, "expected {expected} inputs, got {got}")
            }
            WitnessError::InputOutOfRange { name, ty, value } => {
                write!(f, "input `{name}` = {value:#x} does not fit `{ty}`")
            }
            WitnessError::Unsatisfied { constraint } => {
                write!(f, "constraint {constraint} is not satisfied")
            }
        }
    }
}

impl std::error::Error for WitnessError {}

/// The integer value of a field element, if it fits in 64 bits.
pub fn fr_to_u64(x: Fr) -> Option<u64> {
    let big = x.into_bigint();
    (big.num_bits() <= 64).then(|| big.0[0])
}

/// The 32 little-endian bytes of `x`'s canonical (non-Montgomery) integer.
fn fr_bytes(x: Fr) -> Vec<u8> {
    let mut bytes = x.into_bigint().to_bytes_le();
    bytes.resize(32, 0);
    bytes
}

/// The field's prime, as 32 little-endian bytes.
fn modulus_bytes() -> Vec<u8> {
    let mut bytes = <Fr as PrimeField>::MODULUS.to_bytes_le();
    bytes.resize(32, 0);
    bytes
}

impl R1cs {
    pub fn num_constraints(&self) -> usize {
        self.constraints.len()
    }

    /// The full assignment for `inputs` (unsigned encodings, which must fit
    /// their types), computed from the hints and then checked against every
    /// constraint.
    pub fn witness(&self, inputs: &[u64]) -> Result<Vec<Fr>, WitnessError> {
        if inputs.len() != self.inputs.len() {
            return Err(WitnessError::WrongInputCount {
                expected: self.inputs.len(),
                got: inputs.len(),
            });
        }
        let mut w = vec![Fr::zero(); self.num_vars as usize];
        w[ONE as usize] = Fr::one();
        for ((port, &value), &var) in self.inputs.iter().zip(inputs).zip(&self.input_vars) {
            if value & !port.ty.mask() != 0 {
                return Err(WitnessError::InputOutOfRange {
                    name: port.name.clone(),
                    ty: port.ty,
                    value,
                });
            }
            w[var as usize] = Fr::from(value);
        }
        for hint in &self.hints {
            match hint {
                Hint::Bits {
                    value,
                    first,
                    count,
                } => {
                    let big = value.eval(&w).into_bigint();
                    for i in 0..*count {
                        w[(first + i) as usize] = Fr::from(u64::from(big.get_bit(i as usize)));
                    }
                }
                Hint::Quadratic { out, a, b, c } => {
                    w[*out as usize] = a.eval(&w) * b.eval(&w) + c.eval(&w);
                }
                Hint::Inverse { out, value } => {
                    w[*out as usize] = value.eval(&w).inverse().unwrap_or_else(Fr::zero);
                }
            }
        }
        self.check(&w)?;
        Ok(w)
    }

    /// Checks `witness` against every constraint.
    pub fn check(&self, witness: &[Fr]) -> Result<(), WitnessError> {
        match self
            .constraints
            .iter()
            .position(|k| k.a.eval(witness) * k.b.eval(witness) != k.c.eval(witness))
        {
            Some(constraint) => Err(WitnessError::Unsatisfied { constraint }),
            None => Ok(()),
        }
    }

    /// The public inputs a verifier sees: the revealed output.
    pub fn public_inputs(&self, witness: &[Fr]) -> Vec<Fr> {
        vec![witness[self.output_var as usize]]
    }

    /// The revealed output's unsigned encoding.
    pub fn output(&self, witness: &[Fr]) -> u64 {
        fr_to_u64(witness[self.output_var as usize]).unwrap_or(u64::MAX)
    }

    /// The system in iden3's binary `.r1cs` format (version 1), as circom
    /// writes it and snarkjs reads it: one public output, no public inputs,
    /// the circuit's inputs as private inputs, and the identity label map.
    pub fn to_iden3_r1cs(&self) -> Vec<u8> {
        let mut header = Vec::new();
        header.extend(32u32.to_le_bytes());
        header.extend(modulus_bytes());
        header.extend(self.num_vars.to_le_bytes());
        header.extend(1u32.to_le_bytes()); // public outputs
        header.extend(0u32.to_le_bytes()); // public inputs
        header.extend(
            u32::try_from(self.inputs.len())
                .unwrap_or(u32::MAX)
                .to_le_bytes(),
        );
        header.extend(u64::from(self.num_vars).to_le_bytes()); // labels
        header.extend(
            u32::try_from(self.constraints.len())
                .unwrap_or(u32::MAX)
                .to_le_bytes(),
        );

        let mut constraints = Vec::new();
        for k in &self.constraints {
            for lc in [&k.a, &k.b, &k.c] {
                constraints.extend(
                    u32::try_from(lc.terms.len())
                        .unwrap_or(u32::MAX)
                        .to_le_bytes(),
                );
                for (var, coeff) in &lc.terms {
                    constraints.extend(var.to_le_bytes());
                    constraints.extend(fr_bytes(*coeff));
                }
            }
        }

        let mut labels = Vec::new();
        for v in 0..u64::from(self.num_vars) {
            labels.extend(v.to_le_bytes());
        }

        let mut out = Vec::new();
        out.extend(b"r1cs");
        out.extend(1u32.to_le_bytes());
        out.extend(3u32.to_le_bytes());
        for (kind, section) in [(1u32, header), (2, constraints), (3, labels)] {
            out.extend(kind.to_le_bytes());
            out.extend((section.len() as u64).to_le_bytes());
            out.extend(section);
        }
        out
    }

    /// `witness` in iden3's binary `.wtns` format (version 2).
    pub fn to_iden3_wtns(witness: &[Fr]) -> Vec<u8> {
        let mut header = Vec::new();
        header.extend(32u32.to_le_bytes());
        header.extend(modulus_bytes());
        header.extend(
            u32::try_from(witness.len())
                .unwrap_or(u32::MAX)
                .to_le_bytes(),
        );
        let mut values = Vec::new();
        for w in witness {
            values.extend(fr_bytes(*w));
        }
        let mut out = Vec::new();
        out.extend(b"wtns");
        out.extend(2u32.to_le_bytes());
        out.extend(2u32.to_le_bytes());
        for (kind, section) in [(1u32, header), (2, values)] {
            out.extend(kind.to_le_bytes());
            out.extend((section.len() as u64).to_le_bytes());
            out.extend(section);
        }
        out
    }
}

/// A value below `2^bound` congruent to the node's value modulo `2^width`.
#[derive(Debug, Clone)]
struct Num {
    lc: Lc,
    bound: u32,
}

struct Lowering<'c> {
    circuit: &'c Circuit,
    num_vars: u32,
    constraints: Vec<Constraint>,
    hints: Vec<Hint>,
    nums: Vec<Option<Num>>,
    bits: Vec<Option<Vec<Lc>>>,
    pow2: Vec<Fr>,
}

/// Lowers `circuit` to R1CS. See the module docs for the encoding and the
/// cost of each operator.
pub fn lower(circuit: &Circuit) -> R1cs {
    let n = circuit.nodes.len();
    let mut pow2 = Vec::with_capacity(LIMIT as usize + 2);
    let mut p = Fr::one();
    for _ in 0..=LIMIT + 1 {
        pow2.push(p);
        p.double_in_place();
    }
    let num_inputs = u32::try_from(circuit.inputs.len()).expect("fewer than 2^32 inputs");
    let mut lowering = Lowering {
        circuit,
        // ONE, the output, the inputs.
        num_vars: 2 + num_inputs,
        constraints: Vec::new(),
        hints: Vec::new(),
        nums: vec![None; n],
        bits: vec![None; n],
        pow2,
    };
    let live = circuit.live();
    let mut cost: HashMap<&'static str, usize> = HashMap::new();
    for (i, node) in circuit.nodes.iter().enumerate() {
        if !live[i] {
            continue;
        }
        let before = lowering.constraints.len();
        lowering.lower_node(Wire(u32::try_from(i).expect("node index fits u32")));
        *cost.entry(node.op.name()).or_default() += lowering.constraints.len() - before;
    }
    // The public output equals the result's exact value.
    let before = lowering.constraints.len();
    let exact = lowering.exact(circuit.output);
    lowering.hints.push(Hint::Quadratic {
        out: 1,
        a: Lc::zero(),
        b: Lc::zero(),
        c: exact.clone(),
    });
    lowering.enforce(Lc::var(ONE), exact, Lc::var(1));
    *cost.entry("output").or_default() += lowering.constraints.len() - before;

    let mut cost: Vec<_> = cost.into_iter().filter(|(_, n)| *n > 0).collect();
    cost.sort_unstable();
    R1cs {
        name: circuit.name.clone(),
        signature: circuit.signature(),
        inputs: circuit.inputs.clone(),
        output_ty: circuit.output_ty(),
        num_vars: lowering.num_vars,
        output_var: 1,
        input_vars: (2..2 + num_inputs).collect(),
        constraints: lowering.constraints,
        hints: lowering.hints,
        cost,
    }
}

impl Lowering<'_> {
    fn alloc(&mut self) -> Var {
        let v = self.num_vars;
        self.num_vars += 1;
        v
    }

    fn enforce(&mut self, a: Lc, b: Lc, c: Lc) {
        self.constraints.push(Constraint { a, b, c });
    }

    fn pow(&self, k: u32) -> Fr {
        self.pow2[k as usize]
    }

    fn width(&self, w: Wire) -> u32 {
        self.circuit.node(w).ty.width()
    }

    /// `sum(2^i * bits[i])`.
    fn weighted(&self, bits: &[Lc]) -> Lc {
        bits.iter().enumerate().fold(Lc::zero(), |acc, (i, b)| {
            acc.add_scaled(b, self.pow(u32::try_from(i).unwrap_or(LIMIT)))
        })
    }

    /// `count` bits of `value`, which must be an integer below `2^count`:
    /// `count` boolean variables and one constraint that they sum to it.
    fn decompose(&mut self, value: &Lc, count: u32) -> Vec<Lc> {
        if let Some(c) = value.as_constant() {
            let big = c.into_bigint();
            return (0..count)
                .map(|i| Lc::constant(Fr::from(u64::from(big.get_bit(i as usize)))))
                .collect();
        }
        let first = self.num_vars;
        self.num_vars += count;
        self.hints.push(Hint::Bits {
            value: value.clone(),
            first,
            count,
        });
        let bits: Vec<Lc> = (first..first + count).map(Lc::var).collect();
        for b in &bits {
            // b * (b - 1) = 0
            self.enforce(b.clone(), b.sub(&Lc::var(ONE)), Lc::zero());
        }
        let sum = self.weighted(&bits);
        self.enforce(Lc::var(ONE), sum, value.clone());
        bits
    }

    fn num(&mut self, w: Wire) -> Num {
        if let Some(n) = &self.nums[w.index()] {
            return n.clone();
        }
        let bits = self.bits[w.index()]
            .clone()
            .expect("every lowered node has a number or bits");
        let n = Num {
            lc: self.weighted(&bits),
            bound: self.width(w),
        };
        self.nums[w.index()] = Some(n.clone());
        n
    }

    fn bits(&mut self, w: Wire) -> Vec<Lc> {
        if let Some(b) = &self.bits[w.index()] {
            return b.clone();
        }
        let width = self.width(w);
        let n = self.num(w);
        let mut all = self.decompose(&n.lc, n.bound.max(1));
        all.resize(width.max(all.len() as u32) as usize, Lc::zero());
        all.truncate(width as usize);
        self.bits[w.index()] = Some(all.clone());
        // The number is exact now.
        self.nums[w.index()] = Some(Num {
            lc: self.weighted(&all),
            bound: width,
        });
        all
    }

    /// The exact value, in `[0, 2^width)`.
    fn exact(&mut self, w: Wire) -> Lc {
        let n = self.num(w);
        if n.bound <= self.width(w) {
            n.lc
        } else {
            let bits = self.bits(w);
            self.weighted(&bits)
        }
    }

    fn exact_num(&mut self, w: Wire) -> Num {
        Num {
            lc: self.exact(w),
            bound: self.width(w),
        }
    }

    fn set_num(&mut self, w: Wire, lc: Lc, bound: u32) {
        debug_assert!(bound <= LIMIT, "number bound {bound} over the limit");
        self.nums[w.index()] = Some(Num { lc, bound });
    }

    fn set_bits(&mut self, w: Wire, bits: Vec<Lc>) {
        self.bits[w.index()] = Some(bits);
    }

    /// A fresh variable `out = a * b + c`, constrained by `a * b = out - c`.
    fn quadratic(&mut self, a: Lc, b: Lc, c: Lc) -> Lc {
        let out = self.alloc();
        self.hints.push(Hint::Quadratic {
            out,
            a: a.clone(),
            b: b.clone(),
            c: c.clone(),
        });
        self.enforce(a, b, Lc::var(out).sub(&c));
        Lc::var(out)
    }

    /// `c ? a : b` for a boolean `c`.
    fn select(&mut self, c: &Lc, a: &Lc, b: &Lc) -> Lc {
        if a == b {
            return a.clone();
        }
        match c.as_constant() {
            Some(k) if k.is_one() => a.clone(),
            Some(_) => b.clone(),
            None => self.quadratic(c.clone(), a.sub(b), b.clone()),
        }
    }

    fn bit_and(&mut self, x: &Lc, y: &Lc) -> Lc {
        match (x.as_constant(), y.as_constant()) {
            (Some(k), _) => {
                if k.is_zero() {
                    Lc::zero()
                } else {
                    y.clone()
                }
            }
            (_, Some(k)) => {
                if k.is_zero() {
                    Lc::zero()
                } else {
                    x.clone()
                }
            }
            _ if x == y => x.clone(),
            _ => self.quadratic(x.clone(), y.clone(), Lc::zero()),
        }
    }

    fn bit_or(&mut self, x: &Lc, y: &Lc) -> Lc {
        match (x.as_constant(), y.as_constant()) {
            (Some(k), _) => {
                if k.is_zero() {
                    y.clone()
                } else {
                    Lc::constant(Fr::one())
                }
            }
            (_, Some(k)) => {
                if k.is_zero() {
                    x.clone()
                } else {
                    Lc::constant(Fr::one())
                }
            }
            _ if x == y => x.clone(),
            // x + y - x*y
            _ => self.quadratic(x.clone(), y.scale(-Fr::one()), x.add(y)),
        }
    }

    fn bit_xor(&mut self, x: &Lc, y: &Lc) -> Lc {
        let one = Lc::constant(Fr::one());
        match (x.as_constant(), y.as_constant()) {
            (Some(k), _) => {
                if k.is_zero() {
                    y.clone()
                } else {
                    one.sub(y)
                }
            }
            (_, Some(k)) => {
                if k.is_zero() {
                    x.clone()
                } else {
                    one.sub(x)
                }
            }
            _ if x == y => Lc::zero(),
            // x + y - 2*x*y
            _ => self.quadratic(x.clone(), y.scale(-Fr::from(2u64)), x.add(y)),
        }
    }

    /// 1 when `d` is 0, else 0: `d * inv = 1 - z` and `d * z = 0`.
    fn is_zero(&mut self, d: &Lc) -> Lc {
        if let Some(k) = d.as_constant() {
            return Lc::constant(Fr::from(u64::from(k.is_zero())));
        }
        let inv = self.alloc();
        self.hints.push(Hint::Inverse {
            out: inv,
            value: d.clone(),
        });
        let z = self.quadratic(
            d.clone(),
            Lc::term(inv, -Fr::one()),
            Lc::constant(Fr::one()),
        );
        self.enforce(d.clone(), z.clone(), Lc::zero());
        z
    }

    /// `x < y` for exact values `x`, `y` in `[0, 2^width)`: bit `width` of
    /// `x + 2^width - y` is 0 exactly then.
    fn less_than(&mut self, x: &Lc, y: &Lc, width: u32) -> Lc {
        let t = x.add(&Lc::constant(self.pow(width))).sub(y);
        let bits = self.decompose(&t, width + 1);
        Lc::constant(Fr::one()).sub(&bits[width as usize])
    }

    /// The exact value with the sign bit flipped: signed order becomes
    /// unsigned order.
    fn sign_flipped(&mut self, w: Wire) -> Lc {
        let mut bits = self.bits(w);
        let top = bits.pop().expect("a width of at least 1");
        bits.push(Lc::constant(Fr::one()).sub(&top));
        self.weighted(&bits)
    }

    /// Two numbers whose bounds, combined by `combine`, stay under the
    /// limit; reduced to exact values when they would not.
    fn fitting(&mut self, a: Wire, b: Wire, combine: impl Fn(u32, u32) -> u32) -> (Num, Num) {
        let (na, nb) = (self.num(a), self.num(b));
        if combine(na.bound, nb.bound) <= LIMIT {
            return (na, nb);
        }
        let (na, nb) = (self.exact_num(a), self.exact_num(b));
        assert!(
            combine(na.bound, nb.bound) <= LIMIT,
            "operands of at most 64 bits always fit the field"
        );
        (na, nb)
    }

    #[allow(clippy::too_many_lines)]
    fn lower_node(&mut self, w: Wire) {
        let node = *self.circuit.node(w);
        let width = node.ty.width();
        match node.op {
            Op::Input(k) => {
                let var = 2 + k;
                let bits = self.decompose(&Lc::var(var), width);
                self.set_bits(w, bits);
                self.set_num(w, Lc::var(var), width);
            }
            Op::Const(c) => {
                let bits: Vec<Lc> = (0..width)
                    .map(|i| Lc::constant(Fr::from((c >> i) & 1)))
                    .collect();
                self.set_bits(w, bits);
                self.set_num(w, Lc::constant(Fr::from(c)), width);
            }
            Op::Not(a) => {
                if let Some(bits) = self.bits[a.index()].clone() {
                    let one = Lc::constant(Fr::one());
                    self.set_bits(w, bits.iter().map(|b| one.sub(b)).collect());
                } else {
                    // (2^c - 1) - a, with 2^c a multiple of 2^width.
                    let n = self.num(a);
                    let c = n.bound.max(width);
                    let lc = Lc::constant(self.pow(c) - Fr::one()).sub(&n.lc);
                    self.set_num(w, lc, c);
                }
            }
            Op::Neg(a) => {
                let mut n = self.num(a);
                if n.bound.max(width) + 1 > LIMIT {
                    n = self.exact_num(a);
                }
                let c = n.bound.max(width);
                let lc = Lc::constant(self.pow(c)).sub(&n.lc);
                self.set_num(w, lc, c + 1);
            }
            Op::Add(a, b) => {
                let (na, nb) = self.fitting(a, b, |x, y| x.max(y) + 1);
                self.set_num(w, na.lc.add(&nb.lc), na.bound.max(nb.bound) + 1);
            }
            Op::Sub(a, b) => {
                let (na, nb) = self.fitting(a, b, |x, y| x.max(y.max(width)) + 1);
                let c = nb.bound.max(width);
                let lc = na.lc.add(&Lc::constant(self.pow(c))).sub(&nb.lc);
                self.set_num(w, lc, na.bound.max(c) + 1);
            }
            Op::Mul(a, b) => {
                let constant = |this: &Self, x: Wire| match this.circuit.node(x).op {
                    Op::Const(k) => Some(k),
                    _ => None,
                };
                if let Some((k, other)) = constant(self, b)
                    .map(|k| (k, a))
                    .or_else(|| constant(self, a).map(|k| (k, b)))
                {
                    let kbits = 64 - k.leading_zeros();
                    let mut n = self.num(other);
                    if n.bound + kbits > LIMIT {
                        n = self.exact_num(other);
                    }
                    self.set_num(w, n.lc.scale(Fr::from(k)), n.bound + kbits);
                } else {
                    let (na, nb) = self.fitting(a, b, |x, y| x + y);
                    let product = self.quadratic(na.lc, nb.lc, Lc::zero());
                    self.set_num(w, product, na.bound + nb.bound);
                }
            }
            Op::And(a, b) | Op::Or(a, b) | Op::Xor(a, b) => {
                let (xa, xb) = (self.bits(a), self.bits(b));
                let out = xa
                    .iter()
                    .zip(&xb)
                    .map(|(x, y)| match node.op {
                        Op::And(..) => self.bit_and(x, y),
                        Op::Or(..) => self.bit_or(x, y),
                        _ => self.bit_xor(x, y),
                    })
                    .collect();
                self.set_bits(w, out);
            }
            Op::Shl(a, s) | Op::LShr(a, s) | Op::AShr(a, s) => self.lower_shift(w, node.op, a, s),
            Op::Eq(a, b) => {
                let d = self.exact(a).sub(&self.exact(b));
                let z = self.is_zero(&d);
                self.set_bits(w, vec![z]);
            }
            Op::Ult(a, b) => {
                let operand_width = self.width(a);
                let (x, y) = (self.exact(a), self.exact(b));
                let lt = self.less_than(&x, &y, operand_width);
                self.set_bits(w, vec![lt]);
            }
            Op::Slt(a, b) => {
                let operand_width = self.width(a);
                let (x, y) = (self.sign_flipped(a), self.sign_flipped(b));
                let lt = self.less_than(&x, &y, operand_width);
                self.set_bits(w, vec![lt]);
            }
            Op::Mux(c, a, b) => {
                let cond = self.bits(c)[0].clone();
                let (na, nb) = (self.num(a), self.num(b));
                let out = self.select(&cond, &na.lc, &nb.lc);
                self.set_num(w, out, na.bound.max(nb.bound));
            }
            Op::Trunc(a) => {
                if let Some(bits) = &self.bits[a.index()] {
                    let low = bits[..width as usize].to_vec();
                    self.set_bits(w, low);
                } else {
                    // Congruent modulo 2^width_a, so modulo 2^width too.
                    let n = self.num(a);
                    self.set_num(w, n.lc, n.bound);
                }
            }
            Op::Zext(a) => {
                let n = self.exact_num(a);
                self.set_num(w, n.lc, n.bound);
            }
            Op::Sext(a) => {
                let mut bits = self.bits(a);
                let top = bits.last().cloned().expect("a width of at least 1");
                bits.resize(width as usize, top);
                self.set_bits(w, bits);
            }
        }
    }

    fn lower_shift(&mut self, w: Wire, op: Op, a: Wire, s: Wire) {
        let width = self.width(w);
        let fill = |this: &mut Self| match op {
            Op::AShr(..) => this.bits(a).last().cloned().expect("a width of at least 1"),
            _ => Lc::zero(),
        };
        if let Op::Const(amount) = self.circuit.node(s).op {
            let amount = amount & self.circuit.node(s).ty.mask();
            if let (Op::Shl(..), true) = (op, amount < u64::from(width)) {
                // A number times 2^amount, while that fits.
                let shift = u32::try_from(amount).expect("below the width");
                let n = self.num(a);
                if n.bound + shift <= LIMIT {
                    self.set_num(w, n.lc.scale(self.pow(shift)), n.bound + shift);
                    return;
                }
            }
            let bits = self.bits(a);
            let fill = fill(self);
            let k = usize::try_from(amount.min(u64::from(width))).expect("small");
            let n = width as usize;
            let out: Vec<Lc> = (0..n)
                .map(|i| match op {
                    Op::Shl(..) => {
                        if i >= k {
                            bits[i - k].clone()
                        } else {
                            Lc::zero()
                        }
                    }
                    _ => bits.get(i + k).cloned().unwrap_or_else(|| fill.clone()),
                })
                .collect();
            self.set_bits(w, out);
            return;
        }

        // A barrel shifter: stage j shifts by 2^j when amount bit j is set.
        let mut cur = self.bits(a);
        let fill = fill(self);
        let amount = self.bits(s);
        let stages = usize::try_from(width.next_power_of_two().trailing_zeros()).expect("small");
        let n = width as usize;
        for (j, sbit) in amount.iter().enumerate().take(stages) {
            let step = 1usize << j;
            let shifted: Vec<Lc> = (0..n)
                .map(|i| match op {
                    Op::Shl(..) => {
                        if i >= step {
                            cur[i - step].clone()
                        } else {
                            Lc::zero()
                        }
                    }
                    _ => cur.get(i + step).cloned().unwrap_or_else(|| fill.clone()),
                })
                .collect();
            cur = (0..n)
                .map(|i| self.select(sbit, &shifted[i], &cur[i]))
                .collect();
        }
        // Any higher amount bit set means the amount is at least the width
        // (widths are powers of two), and the result is all `fill`.
        if amount.len() > stages {
            let high = amount[stages..]
                .iter()
                .fold(Lc::zero(), |acc, b| acc.add(b));
            let in_range = self.is_zero(&high);
            cur = (0..n)
                .map(|i| self.select(&in_range, &cur[i], &fill))
                .collect();
        }
        self.set_bits(w, cur);
    }
}

#[cfg(test)]
pub(crate) mod tests;

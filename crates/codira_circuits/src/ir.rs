//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The word-level circuit IR every circuit back end reads: typed nodes over
//!   fixed-width two's-complement integers and booleans, a builder that folds
//!   constants and shares equal nodes, and the reference evaluator whose
//!   semantics the R1CS lowering and the Verilog emitter must reproduce.
//!
//! # Semantics
//!
//! A value of type [`Ty::Int`] with width `w` is stored as its low `w` bits
//! (the unsigned encoding of a two's-complement value) in a `u64`; a
//! [`Ty::Bool`] is `0` or `1`. Every operator is the machine operation on
//! those bits, exactly as native Codira code computes it:
//!
//! * `Add`, `Sub`, `Mul`, `Neg` wrap modulo `2^w` (signedness does not matter
//!   for the low bits).
//! * `And`, `Or`, `Xor`, `Not` are bitwise (logical on booleans).
//! * `Eq`, `Ult`, `Slt` compare; `Slt` reads both operands as two's complement.
//! * `Shl`, `LShr`, `AShr` shift by the *unsigned* value of the amount operand
//!   (which may have any integer type). An amount of `w` or more gives `0`
//!   (`Shl`, `LShr`) or every bit equal to the sign bit (`AShr`). That is the
//!   rule of SMT-LIB bit-vectors (so of the contract verifier) and of Verilog.
//!   Native code leaves such shifts undefined (LLVM `poison`), so programs
//!   whose behaviour matters must keep amounts below the width; the circuit
//!   back ends all agree on the rule above.
//! * `Mux(c, a, b)` is `c ? a : b` with both operands computed.
//! * `Trunc`, `Zext`, `Sext` are the integer casts `as` performs.
//!
//! Division and remainder are not operators: on compile-time values the
//! elaborator computes them, and on run-time values they are refused (an
//! `oblivious` function may not divide a secret, and a division circuit is a
//! later slice).
//!
//! The same IR is what an FHE back end (`encrypted`) would consume: every
//! operator here has a boolean-circuit reading over the bits, and the
//! integer ones map onto `tfhe`'s radix integer operations.

use std::{collections::HashMap, fmt};

/// The type of a circuit value.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Ty {
    Bool,
    Int { width: u32, signed: bool },
}

impl Ty {
    /// The width in bits (1 for a boolean).
    pub fn width(self) -> u32 {
        match self {
            Ty::Bool => 1,
            Ty::Int { width, .. } => width,
        }
    }

    /// Whether comparisons, right shifts and widening read this type as
    /// two's complement.
    pub fn is_signed(self) -> bool {
        matches!(self, Ty::Int { signed: true, .. })
    }

    /// The mask selecting the value's bits in a `u64`.
    pub fn mask(self) -> u64 {
        mask(self.width())
    }

    /// The value `bits` (an unsigned encoding) read as a signed integer of
    /// this type, or as an unsigned one for unsigned types.
    pub fn to_i128(self, bits: u64) -> i128 {
        let bits = bits & self.mask();
        if self.is_signed() {
            i128::from(sign_extend(bits, self.width()))
        } else {
            i128::from(bits)
        }
    }

    /// The encoding of `value` (wrapping into the type, as `as` does).
    #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
    pub fn from_i128(self, value: i128) -> u64 {
        (value as u64) & self.mask()
    }

    /// Parses a decimal (optionally negative) or `0x` hexadecimal literal,
    /// or `true`/`false` for a boolean, into this type's encoding. Values
    /// out of the type's range are refused rather than wrapped.
    pub fn parse(self, text: &str) -> Option<u64> {
        let text = text.trim();
        if self == Ty::Bool {
            return match text {
                "true" | "1" => Some(1),
                "false" | "0" => Some(0),
                _ => None,
            };
        }
        let (negative, digits) = match text.strip_prefix('-') {
            Some(rest) => (true, rest),
            None => (false, text),
        };
        let magnitude = match digits
            .strip_prefix("0x")
            .or_else(|| digits.strip_prefix("0X"))
        {
            Some(hex) => i128::from_str_radix(&hex.replace('_', ""), 16).ok()?,
            None => digits.replace('_', "").parse::<i128>().ok()?,
        };
        let value = if negative { -magnitude } else { magnitude };
        let width = self.width();
        let (min, max) = if self.is_signed() {
            (-(1i128 << (width - 1)), (1i128 << (width - 1)) - 1)
        } else {
            (0, (1i128 << width) - 1)
        };
        (min..=max).contains(&value).then(|| self.from_i128(value))
    }
}

impl fmt::Display for Ty {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Ty::Bool => f.write_str("bool"),
            Ty::Int { width, signed } => write!(f, "{}{width}", if *signed { 'i' } else { 'u' }),
        }
    }
}

/// The low `width` bits set.
pub fn mask(width: u32) -> u64 {
    if width >= 64 {
        u64::MAX
    } else {
        (1u64 << width) - 1
    }
}

/// `bits` (of the given width) sign-extended to 64 bits.
#[allow(clippy::cast_possible_wrap)]
pub fn sign_extend(bits: u64, width: u32) -> i64 {
    if width >= 64 {
        bits as i64
    } else {
        let shift = 64 - width;
        ((bits << shift) as i64) >> shift
    }
}

/// A node of the circuit, by index. Operands always precede their users.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct Wire(pub u32);

impl Wire {
    pub fn index(self) -> usize {
        self.0 as usize
    }
}

impl fmt::Display for Wire {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "n{}", self.0)
    }
}

/// What a node computes. See the module docs for the exact semantics.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Op {
    /// The circuit's input with this index.
    Input(u32),
    /// A constant (its unsigned encoding).
    Const(u64),
    Not(Wire),
    Neg(Wire),
    Add(Wire, Wire),
    Sub(Wire, Wire),
    Mul(Wire, Wire),
    And(Wire, Wire),
    Or(Wire, Wire),
    Xor(Wire, Wire),
    Shl(Wire, Wire),
    LShr(Wire, Wire),
    AShr(Wire, Wire),
    Eq(Wire, Wire),
    Ult(Wire, Wire),
    Slt(Wire, Wire),
    /// `Mux(c, a, b)` is `c ? a : b`.
    Mux(Wire, Wire, Wire),
    Trunc(Wire),
    Zext(Wire),
    Sext(Wire),
}

impl Op {
    /// The operands, in order.
    pub fn operands(&self) -> Vec<Wire> {
        match *self {
            Op::Input(_) | Op::Const(_) => vec![],
            Op::Not(a) | Op::Neg(a) | Op::Trunc(a) | Op::Zext(a) | Op::Sext(a) => vec![a],
            Op::Add(a, b)
            | Op::Sub(a, b)
            | Op::Mul(a, b)
            | Op::And(a, b)
            | Op::Or(a, b)
            | Op::Xor(a, b)
            | Op::Shl(a, b)
            | Op::LShr(a, b)
            | Op::AShr(a, b)
            | Op::Eq(a, b)
            | Op::Ult(a, b)
            | Op::Slt(a, b) => vec![a, b],
            Op::Mux(c, a, b) => vec![c, a, b],
        }
    }

    /// The operator's name, as the text dump and the statistics print it.
    pub fn name(&self) -> &'static str {
        match self {
            Op::Input(_) => "input",
            Op::Const(_) => "const",
            Op::Not(_) => "not",
            Op::Neg(_) => "neg",
            Op::Add(..) => "add",
            Op::Sub(..) => "sub",
            Op::Mul(..) => "mul",
            Op::And(..) => "and",
            Op::Or(..) => "or",
            Op::Xor(..) => "xor",
            Op::Shl(..) => "shl",
            Op::LShr(..) => "lshr",
            Op::AShr(..) => "ashr",
            Op::Eq(..) => "eq",
            Op::Ult(..) => "ult",
            Op::Slt(..) => "slt",
            Op::Mux(..) => "mux",
            Op::Trunc(_) => "trunc",
            Op::Zext(_) => "zext",
            Op::Sext(_) => "sext",
        }
    }
}

/// One node: an operation and the type of its result.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Node {
    pub op: Op,
    pub ty: Ty,
}

/// A named, typed circuit input (a parameter of the source function).
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Port {
    pub name: String,
    pub ty: Ty,
}

/// A combinational circuit with one output: the elaboration of one Codira
/// function.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Circuit {
    /// The source function's name.
    pub name: String,
    pub inputs: Vec<Port>,
    pub nodes: Vec<Node>,
    pub output: Wire,
}

impl Circuit {
    pub fn node(&self, wire: Wire) -> &Node {
        &self.nodes[wire.index()]
    }

    pub fn output_ty(&self) -> Ty {
        self.node(self.output).ty
    }

    /// Evaluates every node on `inputs` (unsigned encodings, one per input;
    /// bits above an input's width are ignored).
    ///
    /// # Panics
    ///
    /// If `inputs` has the wrong length.
    pub fn eval_all(&self, inputs: &[u64]) -> Vec<u64> {
        assert_eq!(
            inputs.len(),
            self.inputs.len(),
            "`{}` takes {} inputs",
            self.name,
            self.inputs.len()
        );
        let masked: Vec<u64> = inputs
            .iter()
            .zip(&self.inputs)
            .map(|(v, p)| v & p.ty.mask())
            .collect();
        let mut values: Vec<u64> = Vec::with_capacity(self.nodes.len());
        for node in &self.nodes {
            let v = compute(
                node,
                |w| (values[w.index()], self.nodes[w.index()].ty),
                &masked,
            );
            values.push(v);
        }
        values
    }

    /// The output's value on `inputs`.
    pub fn eval(&self, inputs: &[u64]) -> u64 {
        self.eval_all(inputs)[self.output.index()]
    }

    /// Which nodes the output depends on.
    pub fn live(&self) -> Vec<bool> {
        let mut live = vec![false; self.nodes.len()];
        live[self.output.index()] = true;
        for i in (0..self.nodes.len()).rev() {
            if live[i] {
                for w in self.nodes[i].op.operands() {
                    live[w.index()] = true;
                }
            }
        }
        live
    }

    /// Operator counts over the live nodes, by name, sorted.
    pub fn op_counts(&self) -> Vec<(&'static str, usize)> {
        let live = self.live();
        let mut counts: HashMap<&'static str, usize> = HashMap::new();
        for (node, live) in self.nodes.iter().zip(live) {
            if live && !matches!(node.op, Op::Input(_) | Op::Const(_)) {
                *counts.entry(node.op.name()).or_default() += 1;
            }
        }
        let mut counts: Vec<_> = counts.into_iter().collect();
        counts.sort_unstable();
        counts
    }

    /// The signature, as Codira would write it.
    pub fn signature(&self) -> String {
        let params: Vec<String> = self
            .inputs
            .iter()
            .map(|p| format!("{}: {}", p.name, p.ty))
            .collect();
        format!(
            "{}({}) -> {}",
            self.name,
            params.join(", "),
            self.output_ty()
        )
    }
}

impl fmt::Display for Circuit {
    /// A readable listing of the live nodes.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "circuit {}", self.signature())?;
        let live = self.live();
        for (i, node) in self.nodes.iter().enumerate() {
            if !live[i] {
                continue;
            }
            let operands: Vec<String> = node.op.operands().iter().map(Wire::to_string).collect();
            let detail = match node.op {
                Op::Input(k) => self.inputs[k as usize].name.clone(),
                Op::Const(v) => node.ty.to_i128(v).to_string(),
                _ => operands.join(", "),
            };
            writeln!(f, "  n{i}: {} = {} {detail}", node.ty, node.op.name())?;
        }
        writeln!(f, "  output {}", self.output)
    }
}

/// Computes one node from its operands' values and types. This is the single
/// definition of the IR's semantics: the builder folds constants with it and
/// the evaluator runs it.
fn compute(node: &Node, operand: impl Fn(Wire) -> (u64, Ty), inputs: &[u64]) -> u64 {
    let ty = node.ty;
    let width = ty.width();
    let m = ty.mask();
    let v = |w: Wire| operand(w).0;
    let shift_amount = |w: Wire| {
        let (bits, amount_ty) = operand(w);
        bits & amount_ty.mask()
    };
    let r = match node.op {
        Op::Input(k) => inputs[k as usize],
        Op::Const(c) => c,
        Op::Not(a) => !v(a),
        Op::Neg(a) => v(a).wrapping_neg(),
        Op::Add(a, b) => v(a).wrapping_add(v(b)),
        Op::Sub(a, b) => v(a).wrapping_sub(v(b)),
        Op::Mul(a, b) => v(a).wrapping_mul(v(b)),
        Op::And(a, b) => v(a) & v(b),
        Op::Or(a, b) => v(a) | v(b),
        Op::Xor(a, b) => v(a) ^ v(b),
        Op::Shl(a, b) => {
            let s = shift_amount(b);
            if s >= u64::from(width) {
                0
            } else {
                v(a) << s
            }
        }
        Op::LShr(a, b) => {
            let s = shift_amount(b);
            if s >= u64::from(width) {
                0
            } else {
                (v(a) & m) >> s
            }
        }
        #[allow(clippy::cast_sign_loss)]
        Op::AShr(a, b) => {
            let s = shift_amount(b).min(u64::from(width) - 1);
            (sign_extend(v(a) & m, width) >> s) as u64
        }
        Op::Eq(a, b) => {
            let (x, t) = operand(a);
            u64::from((x & t.mask()) == (v(b) & t.mask()))
        }
        Op::Ult(a, b) => {
            let (x, t) = operand(a);
            u64::from((x & t.mask()) < (v(b) & t.mask()))
        }
        Op::Slt(a, b) => {
            let (x, t) = operand(a);
            u64::from(
                sign_extend(x & t.mask(), t.width()) < sign_extend(v(b) & t.mask(), t.width()),
            )
        }
        Op::Mux(c, a, b) => {
            if v(c) & 1 == 1 {
                v(a)
            } else {
                v(b)
            }
        }
        Op::Trunc(a) | Op::Zext(a) => {
            let (x, t) = operand(a);
            x & t.mask()
        }
        #[allow(clippy::cast_sign_loss)]
        Op::Sext(a) => {
            let (x, t) = operand(a);
            sign_extend(x & t.mask(), t.width()) as u64
        }
    };
    r & m
}

/// Builds a [`Circuit`]: folds operations whose operands are all constants
/// (with the evaluator's own `compute`, so folding and evaluation cannot
/// disagree), applies a few identities that hold for every value (`x + 0`,
/// `x * 1`, a `Mux` with a constant condition or equal arms, ...), and gives
/// equal nodes one wire.
#[derive(Debug, Default)]
pub struct Builder {
    nodes: Vec<Node>,
    shared: HashMap<Node, Wire>,
    inputs: Vec<Port>,
}

impl Builder {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn ty(&self, wire: Wire) -> Ty {
        self.nodes[wire.index()].ty
    }

    /// The constant `wire` holds, if it is one.
    pub fn as_const(&self, wire: Wire) -> Option<u64> {
        match self.nodes[wire.index()].op {
            Op::Const(c) => Some(c),
            _ => None,
        }
    }

    /// The number of nodes so far.
    pub fn len(&self) -> usize {
        self.nodes.len()
    }

    pub fn is_empty(&self) -> bool {
        self.nodes.is_empty()
    }

    pub fn input(&mut self, name: impl Into<String>, ty: Ty) -> Wire {
        let index = u32::try_from(self.inputs.len()).expect("fewer than 2^32 inputs");
        self.inputs.push(Port {
            name: name.into(),
            ty,
        });
        self.push(Node {
            op: Op::Input(index),
            ty,
        })
    }

    pub fn constant(&mut self, ty: Ty, value: u64) -> Wire {
        self.push(Node {
            op: Op::Const(value & ty.mask()),
            ty,
        })
    }

    pub fn bool(&mut self, value: bool) -> Wire {
        self.constant(Ty::Bool, u64::from(value))
    }

    fn push(&mut self, node: Node) -> Wire {
        if let Some(&w) = self.shared.get(&node) {
            return w;
        }
        let w = Wire(u32::try_from(self.nodes.len()).expect("fewer than 2^32 nodes"));
        self.nodes.push(node);
        self.shared.insert(node, w);
        w
    }

    /// Adds `op` with result type `ty`, folded and simplified.
    pub fn op(&mut self, op: Op, ty: Ty) -> Wire {
        let node = Node { op, ty };
        let operands = op.operands();
        if !operands.is_empty() && operands.iter().all(|w| self.as_const(*w).is_some()) {
            let value = compute(&node, |w| (self.as_const(w).unwrap_or(0), self.ty(w)), &[]);
            return self.constant(ty, value);
        }
        if let Some(w) = self.simplify(op, ty) {
            return w;
        }
        self.push(node)
    }

    /// Identities that hold for every value of the non-constant operand.
    fn simplify(&mut self, op: Op, ty: Ty) -> Option<Wire> {
        let all_ones = ty.mask();
        let is = |b: &Self, w: Wire, c: u64| b.as_const(w) == Some(c);
        match op {
            Op::Add(a, b) | Op::Or(a, b) | Op::Xor(a, b) if is(self, b, 0) => Some(a),
            Op::Add(a, b) | Op::Or(a, b) | Op::Xor(a, b) if is(self, a, 0) => Some(b),
            Op::Sub(a, b) | Op::Shl(a, b) | Op::LShr(a, b) | Op::AShr(a, b) if is(self, b, 0) => {
                Some(a)
            }
            Op::Mul(_, b) | Op::And(_, b) if is(self, b, 0) => Some(b),
            Op::Mul(a, _) | Op::And(a, _) if is(self, a, 0) => Some(a),
            Op::Mul(a, b) if is(self, b, 1) => Some(a),
            Op::Mul(a, b) if is(self, a, 1) => Some(b),
            Op::And(a, b) if is(self, b, all_ones) => Some(a),
            Op::And(a, b) if is(self, a, all_ones) => Some(b),
            Op::Or(_, b) if is(self, b, all_ones) => Some(b),
            Op::Or(a, _) if is(self, a, all_ones) => Some(a),
            Op::And(a, b) | Op::Or(a, b) if a == b => Some(a),
            Op::Mux(c, a, b) => match self.as_const(c) {
                Some(1) => Some(a),
                Some(_) => Some(b),
                None if a == b => Some(a),
                None => None,
            },
            _ => None,
        }
    }

    /// The finished circuit with `output` as its result.
    pub fn finish(self, name: impl Into<String>, output: Wire) -> Circuit {
        Circuit {
            name: name.into(),
            inputs: self.inputs,
            nodes: self.nodes,
            output,
        }
    }
}

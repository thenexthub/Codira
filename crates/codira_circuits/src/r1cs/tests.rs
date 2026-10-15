//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Every circuit operator, built directly with the IR builder, against an
//!   independent integer reference (exhaustively on 8-bit operands, on edge
//!   cases and random values on 64-bit ones), through both the IR evaluator and
//!   the R1CS witness, which must satisfy every constraint; the constraint cost
//!   of each operator; tampered witnesses; Groth16 proofs that verify and that
//!   fail for a wrong revealed value; and the iden3 file layouts.

use super::*;
use crate::{
    groth16,
    ir::{Builder, Op, Ty},
};

const I8: Ty = Ty::Int {
    width: 8,
    signed: true,
};
const U8: Ty = Ty::Int {
    width: 8,
    signed: false,
};
const I64: Ty = Ty::Int {
    width: 64,
    signed: true,
};
const U64: Ty = Ty::Int {
    width: 64,
    signed: false,
};

/// Operators under test, by name.
const OPS: &[&str] = &[
    "add", "sub", "mul", "and", "or", "xor", "shl", "lshr", "ashr", "eq", "ult", "slt", "neg",
    "not", "mux",
];

/// A two-input circuit `f(a: ty, b: ty)` applying `op`.
fn circuit(op: &str, ty: Ty) -> Circuit {
    let mut b = Builder::new();
    let x = b.input("a", ty);
    let y = b.input("b", ty);
    let out = match op {
        "add" => b.op(Op::Add(x, y), ty),
        "sub" => b.op(Op::Sub(x, y), ty),
        "mul" => b.op(Op::Mul(x, y), ty),
        "and" => b.op(Op::And(x, y), ty),
        "or" => b.op(Op::Or(x, y), ty),
        "xor" => b.op(Op::Xor(x, y), ty),
        "shl" => b.op(Op::Shl(x, y), ty),
        "lshr" => b.op(Op::LShr(x, y), ty),
        "ashr" => b.op(Op::AShr(x, y), ty),
        "eq" => b.op(Op::Eq(x, y), Ty::Bool),
        "ult" => b.op(Op::Ult(x, y), Ty::Bool),
        "slt" => b.op(Op::Slt(x, y), Ty::Bool),
        "neg" => b.op(Op::Neg(x), ty),
        "not" => b.op(Op::Not(x), ty),
        // a < b ? a : b, with the comparison by the type's signedness.
        "mux" => {
            let c = if ty.is_signed() {
                b.op(Op::Slt(x, y), Ty::Bool)
            } else {
                b.op(Op::Ult(x, y), Ty::Bool)
            };
            b.op(Op::Mux(c, x, y), ty)
        }
        other => panic!("unknown op {other}"),
    };
    b.finish(op, out)
}

/// `a` read as two's complement whatever the type's signedness.
fn as_signed(ty: Ty, a: u64) -> i128 {
    Ty::Int {
        width: ty.width(),
        signed: true,
    }
    .to_i128(a)
}

/// The operator computed independently, on mathematical integers.
fn reference(op: &str, ty: Ty, a: u64, b: u64) -> u64 {
    let (sa, sb) = (ty.to_i128(a), ty.to_i128(b));
    let (ua, ub) = (i128::from(a & ty.mask()), i128::from(b & ty.mask()));
    let w = i128::from(ty.width());
    match op {
        "add" => ty.from_i128(sa + sb),
        "sub" => ty.from_i128(sa - sb),
        "mul" => ty.from_i128(sa.wrapping_mul(sb)),
        "and" => ty.from_i128(ua & ub),
        "or" => ty.from_i128(ua | ub),
        "xor" => ty.from_i128(ua ^ ub),
        "shl" | "lshr" if ub >= w => 0,
        "shl" => ty.from_i128(ua << ub),
        "lshr" => ty.from_i128(ua >> ub),
        "ashr" => ty.from_i128(as_signed(ty, a) >> ub.min(w - 1)),
        "eq" => u64::from(ua == ub),
        "ult" => u64::from(ua < ub),
        "slt" => u64::from(as_signed(ty, a) < as_signed(ty, b)),
        "neg" => ty.from_i128(-sa),
        "not" => ty.from_i128(!ua),
        "mux" => {
            if sa < sb {
                a & ty.mask()
            } else {
                b & ty.mask()
            }
        }
        other => panic!("unknown op {other}"),
    }
}

fn check_pair(op: &str, ty: Ty, c: &Circuit, r: &R1cs, a: u64, b: u64) {
    let expected = reference(op, ty, a, b);
    assert_eq!(
        c.eval(&[a, b]),
        expected,
        "IR {op} on {ty}: a={a:#x} b={b:#x}"
    );
    let witness = r
        .witness(&[a, b])
        .unwrap_or_else(|e| panic!("R1CS {op} on {ty}: a={a:#x} b={b:#x}: {e}"));
    assert_eq!(
        r.output(&witness),
        expected,
        "R1CS {op} on {ty}: a={a:#x} b={b:#x}"
    );
}

#[test]
fn every_operator_is_exact_on_every_pair_of_8_bit_values() {
    for ty in [I8, U8] {
        for op in OPS {
            let c = circuit(op, ty);
            let r = lower(&c);
            for a in 0..=255u64 {
                for b in 0..=255u64 {
                    check_pair(op, ty, &c, &r, a, b);
                }
            }
        }
    }
}

/// Edge cases for 64-bit operands, as unsigned encodings.
#[allow(clippy::cast_sign_loss)]
pub(crate) fn edges() -> Vec<u64> {
    vec![
        0,
        1,
        2,
        3,
        7,
        8,
        31,
        32,
        62,
        63,
        64,
        65,
        127,
        128,
        255,
        256,
        i64::MAX as u64,
        i64::MIN as u64,
        (i64::MIN + 1) as u64,
        (i64::MAX - 1) as u64,
        u64::MAX,
        u64::MAX - 1,
        0x5555_5555_5555_5555,
        0xAAAA_AAAA_AAAA_AAAA,
        0x8000_0000,
        0xFFFF_FFFF,
        0x1_0000_0000,
        (-63i64) as u64,
        (-64i64) as u64,
    ]
}

/// `SplitMix64`, so the random cases are the same on every run.
pub(crate) struct SplitMix(pub u64);

impl SplitMix {
    pub fn next(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9E37_79B9_7F4A_7C15);
        let mut z = self.0;
        z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
        z ^ (z >> 31)
    }
}

#[test]
fn every_operator_is_exact_on_64_bit_edge_cases_and_random_values() {
    let mut rng = SplitMix(64);
    for ty in [I64, U64] {
        for op in OPS {
            let c = circuit(op, ty);
            let r = lower(&c);
            for &a in &edges() {
                for &b in &edges() {
                    check_pair(op, ty, &c, &r, a, b);
                }
            }
            for _ in 0..300 {
                let (a, b) = (rng.next(), rng.next());
                // Small shift amounts are the interesting ones.
                let b = if op.contains("sh") { b % 70 } else { b };
                check_pair(op, ty, &c, &r, a, b);
            }
        }
    }
}

#[test]
#[allow(
    clippy::cast_possible_wrap,
    clippy::cast_sign_loss,
    clippy::cast_possible_truncation
)]
fn shifts_by_an_amount_of_another_type_and_casts_are_exact() {
    // ((x >> s) as i32) as i64 + (s as i64), with s: u8.
    let mut b = Builder::new();
    let x = b.input("x", I64);
    let s = b.input("s", U8);
    let shifted = b.op(Op::AShr(x, s), I64);
    let i32_ty = Ty::Int {
        width: 32,
        signed: true,
    };
    let narrow = b.op(Op::Trunc(shifted), i32_ty);
    let wide = b.op(Op::Sext(narrow), I64);
    let s64 = b.op(Op::Zext(s), I64);
    let out = b.op(Op::Add(wide, s64), I64);
    let c = b.finish("casts", out);
    let r = lower(&c);
    let mut rng = SplitMix(7);
    let mut cases: Vec<(u64, u64)> = edges().iter().map(|&x| (x, 63)).collect();
    for _ in 0..500 {
        cases.push((rng.next(), rng.next() & 0xFF));
    }
    for (xv, sv) in cases {
        let shifted = (xv as i64) >> sv.min(63);
        let expected = i64::from(shifted as i32).wrapping_add(sv as i64) as u64;
        assert_eq!(c.eval(&[xv, sv]), expected, "x={xv:#x} s={sv}");
        let w = r.witness(&[xv, sv]).unwrap();
        assert_eq!(r.output(&w), expected, "x={xv:#x} s={sv}");
    }
}

/// The table in the module docs, on 64-bit operands: the cost of `op`
/// alone, excluding inputs (65 each) and the output.
#[test]
fn constraint_counts_per_operator() {
    let cost = |op: &str, ty: Ty| {
        let r = lower(&circuit(op, ty));
        r.cost
            .iter()
            .filter(|(name, _)| *name != "input" && *name != "output")
            .map(|(_, n)| n)
            .sum::<usize>()
    };
    let input = lower(&circuit("add", U64))
        .cost
        .iter()
        .find(|(name, _)| *name == "input")
        .map(|(_, n)| *n);
    assert_eq!(
        input,
        Some(2 * 65),
        "two 64-bit inputs, 64 booleans + 1 sum each"
    );
    // Deferred: the reduction happens at the output (66 bits for a sum).
    assert_eq!(cost("add", U64), 0);
    assert_eq!(cost("sub", U64), 0);
    assert_eq!(cost("neg", U64), 0);
    assert_eq!(cost("mul", U64), 1);
    assert_eq!(cost("and", U64), 64);
    assert_eq!(cost("or", U64), 64);
    assert_eq!(cost("xor", U64), 64);
    assert_eq!(cost("eq", U64), 2);
    assert_eq!(cost("ult", U64), 66);
    assert_eq!(cost("slt", I64), 66);
    // 6 stages of 64 multiplexers, plus is-zero (2) and 64 more
    // multiplexers for amounts of 64 or more.
    assert_eq!(cost("shl", U64), 6 * 64 + 2 + 64);
    // a < b (66) and the multiplexer (1).
    assert_eq!(cost("mux", U64), 67);

    // The output reduction: a 64-bit sum has 65 bits, so 65 booleans and
    // one sum, plus the public-output equation.
    let add = lower(&circuit("add", U64));
    let output = add
        .cost
        .iter()
        .find(|(n, _)| *n == "output")
        .map(|(_, n)| *n);
    assert_eq!(output, Some(65 + 1 + 1));
    // A product of two 64-bit values has 128 bits.
    let mul = lower(&circuit("mul", U64));
    let output = mul
        .cost
        .iter()
        .find(|(n, _)| *n == "output")
        .map(|(_, n)| *n);
    assert_eq!(output, Some(128 + 1 + 1));
    assert_eq!(mul.num_constraints(), 2 * 65 + 1 + 130);
}

#[test]
#[allow(clippy::cast_sign_loss)]
fn a_tampered_witness_is_rejected() {
    let c = circuit("mul", I64);
    let r = lower(&c);
    let good = r.witness(&[(-3i64) as u64, 7]).unwrap();
    assert_eq!(r.output(&good), (-21i64) as u64);

    // A different output.
    let mut bad = good.clone();
    bad[r.output_var as usize] += Fr::one();
    assert!(matches!(
        r.check(&bad),
        Err(WitnessError::Unsatisfied { .. })
    ));
    // A bit that is not a bit.
    let mut bad = good.clone();
    let internal = (r.input_vars.last().unwrap() + 1) as usize;
    bad[internal] = Fr::from(2u64);
    assert!(r.check(&bad).is_err());
    // The product without the modular reduction: what a naive field circuit
    // would reveal for (2^64 - 3) * 7.
    let mut bad = good;
    bad[r.output_var as usize] = Fr::from((-3i64) as u64) * Fr::from(7u64);
    assert!(r.check(&bad).is_err());
}

#[test]
fn inputs_outside_their_type_are_refused() {
    let r = lower(&circuit("add", U8));
    assert!(matches!(
        r.witness(&[256, 0]),
        Err(WitnessError::InputOutOfRange { .. })
    ));
    assert!(matches!(
        r.witness(&[1]),
        Err(WitnessError::WrongInputCount { .. })
    ));
}

#[test]
#[allow(clippy::cast_sign_loss)]
fn groth16_proves_the_output_and_rejects_a_wrong_one() {
    // f(a, b) = a < b ? a : b over i64 (signed).
    let c = circuit("mux", I64);
    let r = lower(&c);
    let mut rng = groth16::seeded_rng(1);
    let (pk, vk) = groth16::setup(&r, &mut rng).unwrap();

    let (a, b) = ((-5i64) as u64, 3u64);
    let witness = r.witness(&[a, b]).unwrap();
    assert_eq!(r.output(&witness), a);
    let proof = groth16::prove(&r, &pk, &witness, &mut rng).unwrap();
    let public = r.public_inputs(&witness);
    assert!(groth16::verify(&vk, &public, &proof).unwrap());

    // A claimed output that is not f(a, b).
    assert!(!groth16::verify(&vk, &[Fr::from(3u64)], &proof).unwrap());
    assert!(!groth16::verify(&vk, &[public[0] + Fr::one()], &proof).unwrap());

    // Keys and proofs survive their byte encoding.
    let vk2: groth16::VerifyingKey = groth16::from_bytes(&groth16::to_bytes(&vk)).unwrap();
    let proof2: groth16::Proof = groth16::from_bytes(&groth16::to_bytes(&proof)).unwrap();
    assert!(groth16::verify(&vk2, &public, &proof2).unwrap());

    // A proof for different inputs proves their output, not this one.
    let other = r.witness(&[9, 4]).unwrap();
    let other_proof = groth16::prove(&r, &pk, &other, &mut rng).unwrap();
    assert!(groth16::verify(&vk, &[Fr::from(4u64)], &other_proof).unwrap());
    assert!(!groth16::verify(&vk, &public, &other_proof).unwrap());

    // A corrupted proof does not decode, or does not verify.
    let mut bytes = groth16::to_bytes(&proof);
    bytes[5] ^= 1;
    if let Ok(p) = groth16::from_bytes::<groth16::Proof>(&bytes) {
        assert!(!groth16::verify(&vk, &public, &p).unwrap());
    }
}

#[test]
fn the_iden3_files_have_the_documented_layout() {
    let r = lower(&circuit("add", U8));
    let bytes = r.to_iden3_r1cs();
    let word = |at: usize| u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap());
    assert_eq!(&bytes[..4], b"r1cs");
    assert_eq!((word(4), word(8)), (1, 3), "version 1, three sections");
    assert_eq!(word(12), 1, "the header section comes first");
    assert_eq!(word(24), 32, "32-byte field elements");
    // BN254's scalar field modulus, little-endian.
    assert_eq!(bytes[28..36], 0x43e1_f593_f000_0001u64.to_le_bytes());
    let header = 60;
    assert_eq!(word(header), r.num_vars);
    assert_eq!(
        (word(header + 4), word(header + 8), word(header + 12)),
        (1, 0, 2),
        "one public output, no public inputs, two private inputs"
    );
    assert_eq!(word(header + 24) as usize, r.num_constraints());

    let w = r.witness(&[200, 100]).unwrap();
    let wtns = R1cs::to_iden3_wtns(&w);
    assert_eq!(&wtns[..4], b"wtns");
    assert_eq!(wtns.len(), 12 + 12 + 40 + 12 + 32 * w.len());
    // Variable 1, the output: 300 mod 256.
    let at = 12 + 12 + 40 + 12 + 32;
    assert_eq!(wtns[at], 44);
}

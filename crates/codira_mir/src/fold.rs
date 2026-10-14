//! Copyright (c) 2026 Omnira CJSC
//!
//! Attribute-level fold hooks: the one place that defines what each pure
//! op *means* on compile-time-known values.
//!
//! KGEN's comptime interpreter deliberately reuses MLIR fold hooks so that
//! constant folding, canonicalization, and comptime execution share a
//! single semantics implementation (`modular/KGEN/docs/overviews/
//! Interpreter.md`). This module is that idea for `codira_mir`: the
//! comptime interpreter (`codira_comptime`), the constant-folding pass
//! (`pass`), and the e-graph optimizer (`codira_egraph`) all call
//! [`fold_op`] rather than re-implementing arithmetic.
//!
//! Semantics reference (also documented on the op kinds themselves):
//! * `Int` is `i64` with wrapping add/sub/mul/neg (comptime integers are
//!   fixed-width two's complement, not bignums -- matching what codegen emits
//!   for the same ops).
//! * `Div`/`Rem` by zero and `i64::MIN / -1` overflow return a [`FoldError`],
//!   never a wrong value or a panic.
//! * Shifts with a count outside `0..64` are errors (LLVM would call this
//!   poison; the comptime layer refuses instead).
//! * `And`/`Or` here are the *non*-short-circuiting bool ops (both operands
//!   already evaluated). Short-circuiting is a control-flow concern and belongs
//!   to the interpreter's `If` handling.
//! * Comparisons and equality work on any two attrs of the same shape; mixed
//!   shapes are a type error.
//! * Float arithmetic is IEEE `f64`; `Eq`/`Ne` on floats are IEEE (so `NaN !=
//!   NaN`), unlike `Attr`'s own bitwise identity.

use crate::{
    op::{Attr, CastKind, OpKind},
    ty::TypeId,
};

/// Everything that can go wrong folding one op over concrete attributes.
/// Mirrored (and wrapped) by `codira_comptime::EvalError`.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum FoldError {
    #[error("type mismatch: expected {expected}, found {found}")]
    TypeMismatch {
        expected: &'static str,
        found: &'static str,
    },
    #[error("division by zero")]
    DivideByZero,
    #[error("integer overflow in division")]
    DivisionOverflow,
    #[error("shift amount out of range 0..64")]
    ShiftOutOfRange,
    #[error("wrong operand count")]
    WrongOperandCount,
    #[error("op is not foldable at the attribute level")]
    NotFoldable,
}

fn type_name(attr: &Attr) -> &'static str {
    match attr {
        Attr::Int(_) => "int",
        Attr::Bool(_) => "bool",
        Attr::Float(_) => "float",
        Attr::Str(_) => "string",
        Attr::Unit => "unit",
        Attr::ParamRef(_) => "unresolved parameter",
    }
}

fn as_int(attr: &Attr) -> Result<i64, FoldError> {
    match attr {
        Attr::Int(v) => Ok(*v),
        other => Err(FoldError::TypeMismatch {
            expected: "int",
            found: type_name(other),
        }),
    }
}

fn as_bool(attr: &Attr) -> Result<bool, FoldError> {
    match attr {
        Attr::Bool(v) => Ok(*v),
        other => Err(FoldError::TypeMismatch {
            expected: "bool",
            found: type_name(other),
        }),
    }
}

/// Folds one pure op applied to fully-concrete operand attributes.
///
/// Returns `Err(FoldError::NotFoldable)` for op kinds that are not pure
/// attribute-level computations (control flow, calls, references) --
/// callers treat that as "leave the op alone", distinct from a genuine
/// evaluation error like division by zero.
// `Cast` and the control-flow catch-all both yield `NotFoldable`, but for
// different reasons the comments below record; merging the arms to satisfy
// clippy::match_same_arms would delete that distinction.
#[allow(clippy::match_same_arms)]
pub fn fold_op(kind: &OpKind, operands: &[Attr]) -> Result<Attr, FoldError> {
    use OpKind::{
        Add, And, Arg, BitAnd, BitNot, BitOr, BitXor, BlockArg, Call, Cast, Const, Div, Eq, For,
        Ge, Gt, If, Le, Lt, Mem, Mul, Ne, Neg, Not, Or, ParamRef, Rem, Shl, Shr, Sub, Tuple,
        TupleGet, While, Yield,
    };

    let unary = || -> Result<&Attr, FoldError> {
        match operands {
            [a] => Ok(a),
            _ => Err(FoldError::WrongOperandCount),
        }
    };
    let binary = || -> Result<(&Attr, &Attr), FoldError> {
        match operands {
            [a, b] => Ok((a, b)),
            _ => Err(FoldError::WrongOperandCount),
        }
    };

    match kind {
        Const(attr) => {
            if operands.is_empty() {
                Ok(attr.clone())
            } else {
                Err(FoldError::WrongOperandCount)
            }
        }

        Add | Sub | Mul => {
            let (a, b) = binary()?;
            match (a, b) {
                (Attr::Float(_), _) | (_, Attr::Float(_)) => {
                    let (x, y) = (float_operand(a)?, float_operand(b)?);
                    Ok(Attr::float(match kind {
                        Add => x + y,
                        Sub => x - y,
                        _ => x * y,
                    }))
                }
                _ => {
                    let (x, y) = (as_int(a)?, as_int(b)?);
                    Ok(Attr::Int(match kind {
                        Add => x.wrapping_add(y),
                        Sub => x.wrapping_sub(y),
                        _ => x.wrapping_mul(y),
                    }))
                }
            }
        }

        Div | Rem => {
            let (a, b) = binary()?;
            match (a, b) {
                (Attr::Float(_), _) | (_, Attr::Float(_)) => {
                    let (x, y) = (float_operand(a)?, float_operand(b)?);
                    // IEEE division by zero is inf/NaN, not an error.
                    Ok(Attr::float(if matches!(kind, Div) { x / y } else { x % y }))
                }
                _ => {
                    let (x, y) = (as_int(a)?, as_int(b)?);
                    if y == 0 {
                        return Err(FoldError::DivideByZero);
                    }
                    if x == i64::MIN && y == -1 {
                        return Err(FoldError::DivisionOverflow);
                    }
                    Ok(Attr::Int(if matches!(kind, Div) { x / y } else { x % y }))
                }
            }
        }

        Neg => match unary()? {
            Attr::Int(v) => Ok(Attr::Int(v.wrapping_neg())),
            Attr::Float(bits) => Ok(Attr::float(-f64::from_bits(*bits))),
            other => Err(FoldError::TypeMismatch {
                expected: "int or float",
                found: type_name(other),
            }),
        },

        Eq | Ne => {
            let (a, b) = binary()?;
            let equal = match (a, b) {
                // IEEE equality for floats (NaN != NaN) -- see module doc.
                (Attr::Float(_), _) | (_, Attr::Float(_)) => float_operand(a)? == float_operand(b)?,
                (Attr::Int(x), Attr::Int(y)) => x == y,
                (Attr::Bool(x), Attr::Bool(y)) => x == y,
                (Attr::Str(x), Attr::Str(y)) => x == y,
                (Attr::Unit, Attr::Unit) => true,
                _ => {
                    return Err(FoldError::TypeMismatch {
                        expected: "operands of the same type",
                        found: "mixed types",
                    })
                }
            };
            Ok(Attr::Bool(if matches!(kind, Eq) { equal } else { !equal }))
        }

        Lt | Le | Gt | Ge => {
            let (a, b) = binary()?;
            let ordering_holds = match (a, b) {
                (Attr::Float(_), _) | (_, Attr::Float(_)) => {
                    let (x, y) = (float_operand(a)?, float_operand(b)?);
                    match kind {
                        Lt => x < y,
                        Le => x <= y,
                        Gt => x > y,
                        _ => x >= y,
                    }
                }
                _ => {
                    let (x, y) = (as_int(a)?, as_int(b)?);
                    match kind {
                        Lt => x < y,
                        Le => x <= y,
                        Gt => x > y,
                        _ => x >= y,
                    }
                }
            };
            Ok(Attr::Bool(ordering_holds))
        }

        And | Or => {
            let (a, b) = binary()?;
            let (x, y) = (as_bool(a)?, as_bool(b)?);
            Ok(Attr::Bool(if matches!(kind, And) {
                x && y
            } else {
                x || y
            }))
        }

        Not => Ok(Attr::Bool(!as_bool(unary()?)?)),

        // Bitwise complement. `!v` on `i64` is the two's-complement
        // complement; callers that need a narrower width re-truncate via
        // a cast, exactly as the source language does.
        BitNot => Ok(Attr::Int(!as_int(unary()?)?)),

        BitAnd | BitOr | BitXor => {
            let (a, b) = binary()?;
            let (x, y) = (as_int(a)?, as_int(b)?);
            Ok(Attr::Int(match kind {
                BitAnd => x & y,
                BitOr => x | y,
                _ => x ^ y,
            }))
        }

        Shl | Shr => {
            let (a, b) = binary()?;
            let (x, y) = (as_int(a)?, as_int(b)?);
            // The conversion can only fail because `y` is negative or
            // exceeds u32, both of which are exactly "shift out of
            // range"; the TryFromIntError carries nothing further.
            let Ok(amount) = u32::try_from(y) else {
                return Err(FoldError::ShiftOutOfRange);
            };
            if amount >= 64 {
                return Err(FoldError::ShiftOutOfRange);
            }
            Ok(Attr::Int(if matches!(kind, Shl) {
                x.wrapping_shl(amount)
            } else {
                x >> amount // arithmetic shift: `Attr::Int` is signed
            }))
        }

        // A cast's result depends on its *target type*, which lives on
        // the `Op` (`Op::ty`), not in the `OpKind` -- so it cannot be
        // folded through this signature. Folding casts is unlocked when
        // `fold_op` becomes type-directed in RFC-001 SS1.8 phase 3; until
        // then a constant cast survives to codegen, which is correct if
        // suboptimal.
        Cast(..) => Err(FoldError::NotFoldable),

        // Everything else is control flow, aggregation, or a reference --
        // not an attribute-level computation.
        Tuple | TupleGet(_) | Call(_) | ParamRef(_) | Arg(_) | BlockArg(_) | If | While | For
        | Yield => Err(FoldError::NotFoldable),

        // Memory is not an attribute: a load's value depends on a memory
        // state, which only an interpreter holds.
        Mem(_) => Err(FoldError::NotFoldable),
    }
}

/// [`fold_op`] with the program's types applied: the entry point for every
/// caller that knows them (the constant-folding pass, the comptime
/// interpreter, the e-graph).
///
/// `fold_op` computes in one domain -- `i64` with wrapping, `f64` -- which
/// is exact only for `i64` and `f64`. A `u8` sum of `200 + 100` is `300`
/// there, not the `44` the generated code computes; `0u64 - 1 > 1` is
/// `-1 > 1`, false, where the machine says true. This wrapper closes that
/// gap:
///
/// * every integer operand is first put in its type's representation (the low
///   `width` bits, sign- or zero-extended; `u64` as its bit pattern), and every
///   integer result is wrapped to the result type the same way, so narrow
///   arithmetic wraps where the hardware does;
/// * the operations that read a `u64`'s top bit as a sign (`/`, `%`, `>>` and
///   the orderings) are computed unsigned;
/// * an `f32` result is rounded through `f32`. One `f64` operation followed by
///   one rounding to `f32` is the correctly rounded `f32` result for `+ - * /`,
///   since `f64` carries more than twice `f32`'s precision;
/// * integers wider than 64 bits decline: `Attr::Int` cannot hold them;
/// * a shift by at least the operand's width, and `MIN / -1` at a narrow signed
///   width, are errors -- LLVM calls both poison or a trap, and the fold must
///   not invent a value for them.
///
/// An operand or result whose type is unknown (`TypeId::UNTYPED`) is left
/// as `fold_op` would treat it, which keeps hand-built untyped bodies
/// meaning what they always meant.
pub fn fold_typed_op(
    kind: &OpKind,
    operands: &[Attr],
    operand_tys: &[TypeId],
    result_ty: TypeId,
) -> Result<Attr, FoldError> {
    use OpKind::{Div, Ge, Gt, Le, Lt, Rem, Shl, Shr};

    if operand_tys.len() != operands.len() {
        return Err(FoldError::WrongOperandCount);
    }
    let wider_than_i64 = |ty: &TypeId| ty.as_int().is_some_and(|(width, _)| width > 64);
    if operand_tys.iter().any(wider_than_i64) || wider_than_i64(&result_ty) {
        return Err(FoldError::NotFoldable);
    }

    let operands: Vec<Attr> = operands
        .iter()
        .zip(operand_tys)
        .map(|(attr, ty)| normalize(attr.clone(), *ty))
        .collect();
    let int_ty = |i: usize| operand_tys.get(i).and_then(|ty| ty.as_int());

    if let (Some((width, _)), [_, Attr::Int(amount)]) = (int_ty(0), operands.as_slice()) {
        if matches!(kind, Shl | Shr) && !(0..i64::from(width)).contains(amount) {
            return Err(FoldError::ShiftOutOfRange);
        }
    }

    let is_u64 = |i: usize| int_ty(i) == Some((64, false));
    if let [Attr::Int(x), Attr::Int(y)] = operands.as_slice() {
        let (x, y) = (x.cast_unsigned(), y.cast_unsigned());
        let unsigned = match kind {
            Div | Rem if is_u64(0) || is_u64(1) => {
                if y == 0 {
                    return Err(FoldError::DivideByZero);
                }
                Some(Attr::Int(
                    if matches!(kind, Div) { x / y } else { x % y }.cast_signed(),
                ))
            }
            Shr if is_u64(0) => Some(Attr::Int((x >> y).cast_signed())),
            Lt | Le | Gt | Ge if is_u64(0) || is_u64(1) => Some(Attr::Bool(match kind {
                Lt => x < y,
                Le => x <= y,
                Gt => x > y,
                _ => x >= y,
            })),
            _ => None,
        };
        if let Some(result) = unsigned {
            return Ok(normalize(result, result_ty));
        }

        if let (Div | Rem, Some((width, true))) = (kind, int_ty(0)) {
            let min = if width >= 64 {
                i64::MIN
            } else {
                -(1i64 << (width - 1))
            };
            if x.cast_signed() == min && y.cast_signed() == -1 {
                return Err(FoldError::DivisionOverflow);
            }
        }
    }

    Ok(normalize(fold_op(kind, &operands)?, result_ty))
}

/// Puts a compile-time value in the representation its type has at run
/// time: an integer reduced to its type's width and signedness, an `f32`
/// rounded through `f32`. Anything else, and anything untyped, is returned
/// unchanged.
pub fn normalize(attr: Attr, ty: TypeId) -> Attr {
    match (&attr, ty.as_int(), ty.as_float()) {
        (Attr::Int(v), Some((width, signed)), _) => Attr::Int(reinterpret(*v, width, signed)),
        (Attr::Float(bits), _, Some(32)) => Attr::float(f64::from(f64::from_bits(*bits) as f32)),
        _ => attr,
    }
}

fn float_operand(attr: &Attr) -> Result<f64, FoldError> {
    match attr {
        Attr::Float(bits) => Ok(f64::from_bits(*bits)),
        // Int-to-float promotion in mixed arithmetic: comptime literals
        // like `2 * 3.14` should fold without requiring the frontend to
        // insert casts the language doesn't have yet.
        Attr::Int(v) => Ok(*v as f64),
        other => Err(FoldError::TypeMismatch {
            expected: "float",
            found: type_name(other),
        }),
    }
}

/// Folds a conversion on a compile-time-known value.
///
/// Kept separate from [`fold_op`] because a cast's result depends on its
/// **target type**, which lives on the `Op` rather than in the `OpKind` --
/// `fold_op`'s `(kind, operands)` signature simply cannot express it. The
/// source type is required too: `zext` and `sext` differ only in how the
/// *source* width's top bit is treated, and `Attr::Int` is a bare `i64`
/// that does not record which width it came from.
///
/// Semantics follow `spec/LANGUAGE_SPEC.md` section 18 exactly:
/// integer conversions wrap, and float-to-integer conversions **saturate**
/// with `NaN` mapping to zero.
///
/// `Err(FoldError::NotFoldable)` means "leave the op alone" (e.g. the
/// operand is not a constant of the right shape), never a wrong answer.
pub fn fold_cast(
    kind: CastKind,
    value: &Attr,
    source: TypeId,
    target: TypeId,
) -> Result<Attr, FoldError> {
    match kind {
        CastKind::Trunc | CastKind::Zext | CastKind::Sext | CastKind::Bitcast => {
            let v = match value {
                Attr::Int(v) => *v,
                // `bool -> int` is a zero-extension in the spec matrix.
                Attr::Bool(b) => i64::from(*b),
                _ => return Err(FoldError::NotFoldable),
            };
            let (target_width, target_signed) = target.as_int().ok_or(FoldError::NotFoldable)?;
            match kind {
                // Truncation and bitcast both keep the low bits of the
                // target width; the result's interpretation comes from
                // the target's signedness.
                CastKind::Trunc | CastKind::Bitcast => {
                    Ok(Attr::Int(reinterpret(v, target_width, target_signed)))
                }
                // Extension reads the low bits of the *source* width and
                // fills according to the direction being requested, then
                // stores the result in the target's representation.
                CastKind::Zext => {
                    let source_width = source_int_width(value, source)?;
                    let extended = reinterpret(v, source_width, false);
                    Ok(Attr::Int(reinterpret(
                        extended,
                        target_width,
                        target_signed,
                    )))
                }
                CastKind::Sext => {
                    let source_width = source_int_width(value, source)?;
                    let extended = reinterpret(v, source_width, true);
                    Ok(Attr::Int(reinterpret(
                        extended,
                        target_width,
                        target_signed,
                    )))
                }
                _ => unreachable!("outer match restricts the kinds"),
            }
        }

        CastKind::FpTrunc | CastKind::FpExt => {
            let v = value.as_float().ok_or(FoldError::NotFoldable)?;
            // An `f32` source holds an `f32` value; if it reached here with
            // more precision (an untyped producer), round it first so the
            // widening cannot invent bits the run-time value lacks.
            let v = if source.as_float() == Some(32) {
                f64::from(v as f32)
            } else {
                v
            };
            match target.as_float() {
                // Narrowing to f32 must actually round through f32, or
                // the folded constant would carry precision the runtime
                // value cannot have.
                Some(32) => Ok(Attr::float(f64::from(v as f32))),
                Some(64) => Ok(Attr::float(v)),
                _ => Err(FoldError::NotFoldable),
            }
        }

        CastKind::SiToFp | CastKind::UiToFp => {
            let v = match value {
                Attr::Int(v) => *v,
                Attr::Bool(b) => i64::from(*b),
                _ => return Err(FoldError::NotFoldable),
            };
            // Both directions read the source width's bits first, with the
            // signedness the cast names: a u64 above i64::MAX converts to
            // the right magnitude, and an i8 holding 0xFF converts to -1.
            let source_width = source_int_width(value, source)?;
            let as_f64 = if matches!(kind, CastKind::SiToFp) {
                reinterpret(v, source_width, true) as f64
            } else {
                let unsigned = reinterpret(v, source_width, false);
                if source_width >= 64 {
                    (unsigned as u64) as f64
                } else {
                    unsigned as f64
                }
            };
            match target.as_float() {
                Some(32) => Ok(Attr::float(f64::from(as_f64 as f32))),
                Some(64) => Ok(Attr::float(as_f64)),
                _ => Err(FoldError::NotFoldable),
            }
        }

        CastKind::FpToSi | CastKind::FpToUi => {
            let v = value.as_float().ok_or(FoldError::NotFoldable)?;
            let (width, _) = target.as_int().ok_or(FoldError::NotFoldable)?;
            let signed = matches!(kind, CastKind::FpToSi);
            Ok(Attr::Int(saturate_to_int(v, width, signed)))
        }
    }
}

/// The bit width a cast reads its integer operand at.
///
/// A `bool` is one bit. An operand with no recorded type has no width to
/// read, and guessing one is how `u32 1000 as u64` once folded to `0`
/// (the guess was one bit): decline instead.
fn source_int_width(value: &Attr, source: TypeId) -> Result<u16, FoldError> {
    if let Some((width, _)) = source.as_int() {
        return Ok(width);
    }
    if matches!(value, Attr::Bool(_)) || source == TypeId::BOOL {
        return Ok(1);
    }
    Err(FoldError::NotFoldable)
}

/// Reinterprets the low `width` bits of `v` as a signed or unsigned value
/// of that width, returned in `i64`'s domain.
fn reinterpret(v: i64, width: u16, signed: bool) -> i64 {
    if width >= 64 {
        return v;
    }
    let shift = 64 - u32::from(width);
    if signed {
        // Shift up then arithmetic-shift down: sign-extends from `width`.
        (v << shift) >> shift
    } else {
        // Mask to `width` bits, leaving a non-negative value.
        let mask = (1i64 << width) - 1;
        v & mask
    }
}

/// Saturating float-to-integer conversion, matching
/// `spec/LANGUAGE_SPEC.md` section 18.3 and LLVM's `llvm.fpto{s,u}i.sat`:
/// out-of-range clamps to the destination bounds and `NaN` maps to zero.
fn saturate_to_int(v: f64, width: u16, signed: bool) -> i64 {
    if v.is_nan() {
        return 0;
    }
    let (min, max) = if signed {
        if width >= 64 {
            (i64::MIN, i64::MAX)
        } else {
            (-(1i64 << (width - 1)), (1i64 << (width - 1)) - 1)
        }
    } else if width >= 64 {
        // u64 lives in `Attr::Int` as its bit pattern (see
        // `int_to_float_respects_source_signedness`), so its range is
        // handled in `u64` and the result stored as those bits.
        let truncated = v.trunc();
        let saturated = if truncated <= 0.0 {
            0
        } else if truncated >= u64::MAX as f64 {
            u64::MAX
        } else {
            truncated as u64
        };
        return saturated.cast_signed();
    } else {
        (0, (1i64 << width) - 1)
    };
    let truncated = v.trunc();
    if truncated <= min as f64 {
        min
    } else if truncated >= max as f64 {
        max
    } else {
        truncated as i64
    }
}
#[cfg(test)]
mod tests {
    use super::*;

    fn int(v: i64) -> Attr {
        Attr::Int(v)
    }

    #[test]
    fn arithmetic_wraps() {
        assert_eq!(
            fold_op(&OpKind::Add, &[int(i64::MAX), int(1)]),
            Ok(int(i64::MIN))
        );
        assert_eq!(fold_op(&OpKind::Neg, &[int(i64::MIN)]), Ok(int(i64::MIN)));
    }

    #[test]
    fn division_guards() {
        assert_eq!(
            fold_op(&OpKind::Div, &[int(1), int(0)]),
            Err(FoldError::DivideByZero)
        );
        assert_eq!(
            fold_op(&OpKind::Div, &[int(i64::MIN), int(-1)]),
            Err(FoldError::DivisionOverflow)
        );
        assert_eq!(fold_op(&OpKind::Rem, &[int(7), int(3)]), Ok(int(1)));
    }

    #[test]
    fn shifts_are_checked() {
        assert_eq!(fold_op(&OpKind::Shl, &[int(1), int(3)]), Ok(int(8)));
        assert_eq!(
            fold_op(&OpKind::Shl, &[int(1), int(64)]),
            Err(FoldError::ShiftOutOfRange)
        );
        assert_eq!(
            fold_op(&OpKind::Shr, &[int(-8), int(1)]),
            Ok(int(-4)),
            "right shift is arithmetic"
        );
    }

    #[test]
    fn float_semantics() {
        assert_eq!(
            fold_op(&OpKind::Add, &[Attr::float(1.5), Attr::float(2.25)]),
            Ok(Attr::float(3.75))
        );
        // Mixed int/float promotes.
        assert_eq!(
            fold_op(&OpKind::Mul, &[int(2), Attr::float(3.5)]),
            Ok(Attr::float(7.0))
        );
        // IEEE: NaN != NaN even though the attrs are bit-identical.
        let nan = Attr::float(f64::NAN);
        assert_eq!(
            fold_op(&OpKind::Eq, &[nan.clone(), nan]),
            Ok(Attr::Bool(false))
        );
    }

    #[test]
    fn control_flow_is_not_foldable() {
        assert_eq!(fold_op(&OpKind::If, &[]), Err(FoldError::NotFoldable));
        assert_eq!(
            fold_op(&OpKind::Call("f".into()), &[]),
            Err(FoldError::NotFoldable)
        );
    }

    #[test]
    fn cast_truncation_wraps() {
        // 300 as u8 == 44 (300 mod 256) -- the spec's worked example.
        assert_eq!(
            fold_cast(CastKind::Trunc, &int(300), TypeId::I64, TypeId::U8),
            Ok(int(44))
        );
        // -1 as u8 == 255
        assert_eq!(
            fold_cast(CastKind::Trunc, &int(-1), TypeId::I64, TypeId::U8),
            Ok(int(255))
        );
        // -1 as i8 == -1
        assert_eq!(
            fold_cast(CastKind::Trunc, &int(-1), TypeId::I64, TypeId::I8),
            Ok(int(-1))
        );
    }

    #[test]
    fn extension_direction_comes_from_the_source() {
        // The classic cast bug: u32 0xFFFF_FFFF widened to i64 must be
        // 4294967295, NOT -1. Zext and sext differ *only* here.
        assert_eq!(
            fold_cast(CastKind::Zext, &int(-1), TypeId::U32, TypeId::I64),
            Ok(int(4294967295))
        );
        assert_eq!(
            fold_cast(CastKind::Sext, &int(-1), TypeId::I32, TypeId::I64),
            Ok(int(-1))
        );
    }

    #[test]
    fn float_to_int_saturates_and_maps_nan_to_zero() {
        assert_eq!(
            fold_cast(
                CastKind::FpToSi,
                &Attr::float(1e20),
                TypeId::F64,
                TypeId::I32
            ),
            Ok(int(i64::from(i32::MAX)))
        );
        assert_eq!(
            fold_cast(
                CastKind::FpToSi,
                &Attr::float(-1e20),
                TypeId::F64,
                TypeId::I32
            ),
            Ok(int(i64::from(i32::MIN)))
        );
        assert_eq!(
            fold_cast(
                CastKind::FpToSi,
                &Attr::float(f64::NAN),
                TypeId::F64,
                TypeId::I32
            ),
            Ok(int(0))
        );
        // Ordinary in-range values truncate toward zero.
        assert_eq!(
            fold_cast(
                CastKind::FpToSi,
                &Attr::float(3.7),
                TypeId::F64,
                TypeId::I32
            ),
            Ok(int(3))
        );
        assert_eq!(
            fold_cast(
                CastKind::FpToSi,
                &Attr::float(-3.7),
                TypeId::F64,
                TypeId::I32
            ),
            Ok(int(-3))
        );
    }

    #[test]
    fn float_narrowing_rounds_through_f32() {
        // A constant that is exact in f64 but not f32 must lose the same
        // precision the runtime conversion would.
        let v = 0.1f64;
        let folded = fold_cast(CastKind::FpTrunc, &Attr::float(v), TypeId::F64, TypeId::F32);
        assert_eq!(folded, Ok(Attr::float(f64::from(v as f32))));
        assert_ne!(folded, Ok(Attr::float(v)), "f32 rounding must be applied");
    }

    #[test]
    fn int_to_float_respects_source_signedness() {
        // u64 bit pattern -1 is 18446744073709551615, not -1.0.
        let unsigned = fold_cast(CastKind::UiToFp, &int(-1), TypeId::U64, TypeId::F64);
        assert_eq!(unsigned, Ok(Attr::float(u64::MAX as f64)));
        let signed = fold_cast(CastKind::SiToFp, &int(-1), TypeId::I64, TypeId::F64);
        assert_eq!(signed, Ok(Attr::float(-1.0)));
    }

    #[test]
    fn casts_decline_without_the_types_they_read() {
        // An untyped operand has no width: guessing one bit is how
        // `1000u32 as u64` once folded to 0.
        for kind in [
            CastKind::Zext,
            CastKind::Sext,
            CastKind::UiToFp,
            CastKind::SiToFp,
        ] {
            let target = if matches!(kind, CastKind::Zext | CastKind::Sext) {
                TypeId::U64
            } else {
                TypeId::F64
            };
            assert_eq!(
                fold_cast(kind, &int(1000), TypeId::UNTYPED, target),
                Err(FoldError::NotFoldable),
                "{kind:?}"
            );
        }
        assert_eq!(
            fold_cast(CastKind::Zext, &int(1000), TypeId::U32, TypeId::U64),
            Ok(int(1000))
        );
        // `bool` is one bit wide whether or not it is typed.
        assert_eq!(
            fold_cast(
                CastKind::Zext,
                &Attr::Bool(true),
                TypeId::UNTYPED,
                TypeId::U8
            ),
            Ok(int(1))
        );
    }

    #[test]
    fn int_to_float_reads_the_source_width() {
        // An i8 holding 0xFF (as a stale, un-normalized 255) is -1.
        assert_eq!(
            fold_cast(CastKind::SiToFp, &int(255), TypeId::I8, TypeId::F64),
            Ok(Attr::float(-1.0))
        );
        assert_eq!(
            fold_cast(CastKind::UiToFp, &int(-1), TypeId::U32, TypeId::F64),
            Ok(Attr::float(4_294_967_295.0))
        );
    }

    #[test]
    fn float_to_u64_saturates_to_the_full_range() {
        // u64 is held as its bit pattern, so u64::MAX is -1 here, not
        // i64::MAX.
        let to_u64 =
            |v: f64| fold_cast(CastKind::FpToUi, &Attr::float(v), TypeId::F64, TypeId::U64);
        assert_eq!(to_u64(f64::INFINITY), Ok(int(-1)));
        assert_eq!(
            to_u64(1e19),
            Ok(int(10_000_000_000_000_000_000u64.cast_signed()))
        );
        assert_eq!(to_u64(-5.0), Ok(int(0)));
        assert_eq!(to_u64(f64::NAN), Ok(int(0)));
    }

    #[test]
    fn widening_an_f32_source_rounds_it_first() {
        assert_eq!(
            fold_cast(CastKind::FpExt, &Attr::float(0.1), TypeId::F32, TypeId::F64),
            Ok(Attr::float(f64::from(0.1f32)))
        );
    }

    #[test]
    fn typed_fold_wraps_at_the_declared_width() {
        let add = |a, b, ty| fold_typed_op(&OpKind::Add, &[int(a), int(b)], &[ty, ty], ty);
        assert_eq!(add(200, 100, TypeId::U8), Ok(int(44)));
        assert_eq!(add(127, 1, TypeId::I8), Ok(int(-128)));
        assert_eq!(add(i64::from(u32::MAX), 1, TypeId::U32), Ok(int(0)));
        // Untyped keeps `fold_op`'s plain i64 meaning.
        assert_eq!(add(200, 100, TypeId::UNTYPED), Ok(int(300)));
        // Operands are normalized before use, too.
        assert_eq!(
            fold_typed_op(
                &OpKind::Eq,
                &[int(300), int(44)],
                &[TypeId::U8, TypeId::U8],
                TypeId::BOOL
            ),
            Ok(Attr::Bool(true))
        );
    }

    #[test]
    fn typed_fold_reads_u64_as_unsigned() {
        let u = [TypeId::U64, TypeId::U64];
        let max = int(-1); // u64::MAX
        assert_eq!(
            fold_typed_op(&OpKind::Gt, &[max.clone(), int(1)], &u, TypeId::BOOL),
            Ok(Attr::Bool(true))
        );
        assert_eq!(
            fold_typed_op(&OpKind::Div, &[max.clone(), int(2)], &u, TypeId::U64),
            Ok(int((u64::MAX / 2).cast_signed()))
        );
        assert_eq!(
            fold_typed_op(&OpKind::Rem, &[max.clone(), int(10)], &u, TypeId::U64),
            Ok(int((u64::MAX % 10).cast_signed()))
        );
        assert_eq!(
            fold_typed_op(&OpKind::Shr, &[max, int(60)], &u, TypeId::U64),
            Ok(int(15))
        );
        assert_eq!(
            fold_typed_op(&OpKind::Div, &[int(1), int(0)], &u, TypeId::U64),
            Err(FoldError::DivideByZero)
        );
    }

    #[test]
    fn typed_fold_refuses_what_has_no_value() {
        let i32s = [TypeId::I32, TypeId::I32];
        assert_eq!(
            fold_typed_op(
                &OpKind::Div,
                &[int(i64::from(i32::MIN)), int(-1)],
                &i32s,
                TypeId::I32
            ),
            Err(FoldError::DivisionOverflow)
        );
        assert_eq!(
            fold_typed_op(&OpKind::Shl, &[int(1), int(32)], &i32s, TypeId::I32),
            Err(FoldError::ShiftOutOfRange)
        );
        assert_eq!(
            fold_typed_op(
                &OpKind::Add,
                &[int(1), int(1)],
                &[TypeId::I128, TypeId::I128],
                TypeId::I128
            ),
            Err(FoldError::NotFoldable)
        );
    }

    #[test]
    fn typed_fold_rounds_f32_results() {
        let f32s = [TypeId::F32, TypeId::F32];
        let sum = fold_typed_op(
            &OpKind::Add,
            &[
                Attr::float(f64::from(0.1f32)),
                Attr::float(f64::from(0.2f32)),
            ],
            &f32s,
            TypeId::F32,
        );
        assert_eq!(sum, Ok(Attr::float(f64::from(0.1f32 + 0.2f32))));
    }

    #[test]
    fn non_constant_shapes_decline() {
        assert_eq!(
            fold_cast(CastKind::Trunc, &Attr::Unit, TypeId::I64, TypeId::I8),
            Err(FoldError::NotFoldable)
        );
        assert_eq!(
            fold_cast(CastKind::FpToSi, &int(1), TypeId::I64, TypeId::I32),
            Err(FoldError::NotFoldable),
            "a float conversion needs a float operand"
        );
    }
}

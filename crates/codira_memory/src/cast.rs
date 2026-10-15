//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Conversions between primitive field types during hot-reload migration.
//!
//! When a field keeps its name but changes its primitive type, the old value
//! is converted, never reinterpreted. Every pair of numeric types (and
//! `bool`) converts, with these semantics:
//!
//! - integer to integer: the value if it fits, otherwise the nearest
//!   representable value (saturation: `300u16` becomes `127i8`, `-1i32` becomes
//!   `0u32`). Never wraps.
//! - float to integer: Rust's `as` -- rounds toward zero, saturates at the
//!   target's bounds, and NaN becomes 0.
//! - integer to float, and `f64` to `f32`: the nearest representable value
//!   (Rust's `as`); a magnitude beyond `f32` becomes infinity.
//! - anything to `bool`: `true` for a non-zero value (NaN is `false`); `bool`
//!   to a number: 0 or 1.
//!
//! Widening conversions are exact.

use std::ptr::NonNull;

use crate::{HasStaticType, Type};

/// A primitive type that participates in conversions.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Scalar {
    I8,
    I16,
    I32,
    I64,
    I128,
    Isize,
    U8,
    U16,
    U32,
    U64,
    U128,
    Usize,
    F32,
    F64,
    Bool,
}

impl Scalar {
    fn of(ty: &Type) -> Option<Scalar> {
        macro_rules! find {
            ($($ty:ty => $scalar:ident),+ $(,)?) => {
                $(if ty == <$ty>::type_info() {
                    return Some(Scalar::$scalar);
                })+
            };
        }
        find!(
            i8 => I8, i16 => I16, i32 => I32, i64 => I64, i128 => I128, isize => Isize,
            u8 => U8, u16 => U16, u32 => U32, u64 => U64, u128 => U128, usize => Usize,
            f32 => F32, f64 => F64, bool => Bool,
        );
        None
    }
}

/// A primitive value, widened to the largest type of its kind.
#[derive(Clone, Copy, Debug)]
enum Value {
    Signed(i128),
    Unsigned(u128),
    Float(f64),
    Bool(bool),
}

/// Reads a value of type `scalar` from `ptr`.
///
/// # Safety
///
/// `ptr` must point to a valid, readable value of the given type.
unsafe fn read(scalar: Scalar, ptr: NonNull<u8>) -> Value {
    let p = ptr.as_ptr();
    match scalar {
        Scalar::I8 => Value::Signed(i128::from(p.cast::<i8>().read_unaligned())),
        Scalar::I16 => Value::Signed(i128::from(p.cast::<i16>().read_unaligned())),
        Scalar::I32 => Value::Signed(i128::from(p.cast::<i32>().read_unaligned())),
        Scalar::I64 => Value::Signed(i128::from(p.cast::<i64>().read_unaligned())),
        Scalar::I128 => Value::Signed(p.cast::<i128>().read_unaligned()),
        Scalar::Isize => Value::Signed(p.cast::<isize>().read_unaligned() as i128),
        Scalar::U8 => Value::Unsigned(u128::from(p.cast::<u8>().read_unaligned())),
        Scalar::U16 => Value::Unsigned(u128::from(p.cast::<u16>().read_unaligned())),
        Scalar::U32 => Value::Unsigned(u128::from(p.cast::<u32>().read_unaligned())),
        Scalar::U64 => Value::Unsigned(u128::from(p.cast::<u64>().read_unaligned())),
        Scalar::U128 => Value::Unsigned(p.cast::<u128>().read_unaligned()),
        Scalar::Usize => Value::Unsigned(p.cast::<usize>().read_unaligned() as u128),
        Scalar::F32 => Value::Float(f64::from(p.cast::<f32>().read_unaligned())),
        Scalar::F64 => Value::Float(p.cast::<f64>().read_unaligned()),
        Scalar::Bool => Value::Bool(p.read() != 0),
    }
}

/// Converts `value` to a signed integer in `[min, max]`.
fn to_signed(value: Value, min: i128, max: i128) -> i128 {
    match value {
        Value::Signed(v) => v.clamp(min, max),
        Value::Unsigned(v) => i128::try_from(v).unwrap_or(i128::MAX).clamp(min, max),
        // `as` saturates and maps NaN to zero.
        Value::Float(v) => (v as i128).clamp(min, max),
        Value::Bool(v) => i128::from(v),
    }
}

/// Converts `value` to an unsigned integer in `[0, max]`.
fn to_unsigned(value: Value, max: u128) -> u128 {
    match value {
        Value::Signed(v) => u128::try_from(v).unwrap_or(0).min(max),
        Value::Unsigned(v) => v.min(max),
        Value::Float(v) => (v as u128).min(max),
        Value::Bool(v) => u128::from(v),
    }
}

fn to_f64(value: Value) -> f64 {
    match value {
        Value::Signed(v) => v as f64,
        Value::Unsigned(v) => v as f64,
        Value::Float(v) => v,
        Value::Bool(v) => f64::from(u8::from(v)),
    }
}

/// Writes `value` converted to `scalar` to `ptr`.
///
/// # Safety
///
/// `ptr` must point to writable memory for a value of the given type.
unsafe fn write(scalar: Scalar, ptr: NonNull<u8>, value: Value) {
    let p = ptr.as_ptr();
    // The clamps above guarantee every `as` below is lossless.
    macro_rules! signed {
        ($ty:ty) => {
            p.cast::<$ty>().write_unaligned(to_signed(
                value,
                i128::from(<$ty>::MIN),
                i128::from(<$ty>::MAX),
            ) as $ty)
        };
    }
    macro_rules! unsigned {
        ($ty:ty) => {
            p.cast::<$ty>()
                .write_unaligned(to_unsigned(value, u128::from(<$ty>::MAX)) as $ty)
        };
    }
    match scalar {
        Scalar::I8 => signed!(i8),
        Scalar::I16 => signed!(i16),
        Scalar::I32 => signed!(i32),
        Scalar::I64 => signed!(i64),
        Scalar::I128 => p
            .cast::<i128>()
            .write_unaligned(to_signed(value, i128::MIN, i128::MAX)),
        Scalar::Isize => p.cast::<isize>().write_unaligned(to_signed(
            value,
            isize::MIN as i128,
            isize::MAX as i128,
        ) as isize),
        Scalar::U8 => unsigned!(u8),
        Scalar::U16 => unsigned!(u16),
        Scalar::U32 => unsigned!(u32),
        Scalar::U64 => unsigned!(u64),
        Scalar::U128 => p
            .cast::<u128>()
            .write_unaligned(to_unsigned(value, u128::MAX)),
        Scalar::Usize => p
            .cast::<usize>()
            .write_unaligned(to_unsigned(value, usize::MAX as u128) as usize),
        Scalar::F32 => p.cast::<f32>().write_unaligned(match value {
            // Convert integers directly, not through `f64`, so the result is
            // the nearest `f32` rather than a double-rounded one.
            Value::Signed(v) => v as f32,
            Value::Unsigned(v) => v as f32,
            other => to_f64(other) as f32,
        }),
        Scalar::F64 => p.cast::<f64>().write_unaligned(to_f64(value)),
        Scalar::Bool => p.write(u8::from(match value {
            Value::Signed(v) => v != 0,
            Value::Unsigned(v) => v != 0,
            Value::Float(v) => v != 0.0 && !v.is_nan(),
            Value::Bool(v) => v,
        })),
    }
}

/// Converts the value of type `old_id` at `src` to type `new_id` at `dest`.
/// Returns `false`, leaving `dest` untouched, if either type is not a
/// primitive numeric type or `bool`.
pub fn try_cast_from_to(old_id: Type, new_id: Type, src: NonNull<u8>, dest: NonNull<u8>) -> bool {
    let (Some(from), Some(to)) = (Scalar::of(&old_id), Scalar::of(&new_id)) else {
        return false;
    };
    // Safety: the caller passes pointers to values of the given types.
    unsafe { write(to, dest, read(from, src)) };
    true
}

#[cfg(test)]
mod tests {
    use std::ptr::NonNull;

    use super::try_cast_from_to;
    use crate::HasStaticType;

    fn assert_cast<A, B>(a: A, mut b: B)
    where
        A: Copy + Into<B> + HasStaticType,
        B: PartialEq + std::fmt::Debug + HasStaticType,
    {
        assert!(try_cast_from_to(
            A::type_info().clone(),
            B::type_info().clone(),
            unsafe { NonNull::new_unchecked(&a as *const _ as *mut _) },
            unsafe { NonNull::new_unchecked(&mut b as *mut _) }.cast::<u8>(),
        ));
        assert_eq!(b, a.into());
    }

    #[test]
    fn cast_f32_to_f64() {
        assert_cast(std::f32::consts::PI, 0f64);
    }

    #[test]
    fn cast_i8_to_i16() {
        assert_cast(-5i8, 0i16);
    }

    #[test]
    fn cast_i8_to_i32() {
        assert_cast(-5i8, 0i32);
    }

    #[test]
    fn cast_i8_to_i64() {
        assert_cast(-5i8, 0i64);
    }

    #[test]
    fn cast_i8_to_i128() {
        assert_cast(-5i8, 0i128);
    }

    #[test]
    fn cast_i16_to_i32() {
        assert_cast(-5i16, 0i32);
    }

    #[test]
    fn cast_i16_to_i64() {
        assert_cast(-5i16, 0i64);
    }

    #[test]
    fn cast_i16_to_i128() {
        assert_cast(-5i16, 0i128);
    }

    #[test]
    fn cast_i32_to_i64() {
        assert_cast(-5i32, 0i64);
    }

    #[test]
    fn cast_i32_to_i128() {
        assert_cast(-5i32, 0i128);
    }

    #[test]
    fn cast_i64_to_i128() {
        assert_cast(-5i64, 0i128);
    }

    #[test]
    fn cast_u8_to_i16() {
        assert_cast(5u8, 0i16);
    }

    #[test]
    fn cast_u8_to_u16() {
        assert_cast(5u8, 0u16);
    }

    #[test]
    fn cast_u8_to_i32() {
        assert_cast(5u8, 0i32);
    }

    #[test]
    fn cast_u8_to_u32() {
        assert_cast(5u8, 0u32);
    }

    #[test]
    fn cast_u8_to_i64() {
        assert_cast(5u8, 0i64);
    }

    #[test]
    fn cast_u8_to_u64() {
        assert_cast(5u8, 0u64);
    }

    #[test]
    fn cast_u8_to_i128() {
        assert_cast(5u8, 0i128);
    }

    #[test]
    fn cast_u8_to_u128() {
        assert_cast(5u8, 0u128);
    }

    #[test]
    fn cast_u16_to_i32() {
        assert_cast(5u16, 0i32);
    }

    #[test]
    fn cast_u16_to_u32() {
        assert_cast(5u16, 0u32);
    }

    #[test]
    fn cast_u16_to_i64() {
        assert_cast(5u16, 0i64);
    }

    #[test]
    fn cast_u16_to_u64() {
        assert_cast(5u16, 0u64);
    }

    #[test]
    fn cast_u16_to_i128() {
        assert_cast(5u16, 0i128);
    }

    #[test]
    fn cast_u16_to_u128() {
        assert_cast(5u16, 0u128);
    }

    #[test]
    fn cast_u32_to_i64() {
        assert_cast(5u32, 0i64);
    }

    #[test]
    fn cast_u32_to_u64() {
        assert_cast(5u32, 0u64);
    }

    #[test]
    fn cast_u32_to_i128() {
        assert_cast(5u32, 0i128);
    }

    #[test]
    fn cast_u32_to_u128() {
        assert_cast(5u32, 0u128);
    }

    #[test]
    fn cast_u64_to_i128() {
        assert_cast(5u64, 0i128);
    }

    #[test]
    fn cast_u64_to_u128() {
        assert_cast(5u64, 0u128);
    }

    fn cast<A: HasStaticType, B: HasStaticType + Default>(a: A) -> B {
        let mut b = B::default();
        assert!(try_cast_from_to(
            A::type_info().clone(),
            B::type_info().clone(),
            unsafe { NonNull::new_unchecked(&a as *const _ as *mut _) },
            unsafe { NonNull::new_unchecked(&mut b as *mut _) }.cast::<u8>(),
        ));
        b
    }

    #[test]
    fn narrowing_integers_saturates() {
        assert_eq!(cast::<i64, i32>(1 << 40), i32::MAX);
        assert_eq!(cast::<i64, i32>(-(1 << 40)), i32::MIN);
        assert_eq!(cast::<i64, i32>(-7), -7);
        assert_eq!(cast::<u16, i8>(300), 127);
        assert_eq!(cast::<i32, u32>(-1), 0);
        assert_eq!(cast::<u128, u8>(u128::MAX), u8::MAX);
        assert_eq!(cast::<i128, usize>(-5), 0);
        assert_eq!(cast::<u64, i64>(u64::MAX), i64::MAX);
    }

    #[test]
    fn floats_convert_like_as() {
        assert_eq!(cast::<f64, i32>(3.9), 3);
        assert_eq!(cast::<f64, i32>(-3.9), -3);
        assert_eq!(cast::<f64, i32>(1e20), i32::MAX);
        assert_eq!(cast::<f64, u8>(-1.0), 0);
        assert_eq!(cast::<f64, i64>(f64::NAN), 0);
        assert_eq!(cast::<f64, f32>(0.1), 0.1f32);
        assert_eq!(cast::<f64, f32>(1e300), f32::INFINITY);
        assert_eq!(cast::<i64, f64>(-42), -42.0);
        assert_eq!(cast::<u64, f32>(u64::MAX), u64::MAX as f32);
    }

    #[test]
    fn bools_convert_to_and_from_numbers() {
        assert_eq!(cast::<bool, i32>(true), 1);
        assert_eq!(cast::<bool, f64>(false), 0.0);
        assert!(cast::<i64, bool>(-3));
        assert!(!cast::<u8, bool>(0));
        assert!(!cast::<f64, bool>(f64::NAN));
    }
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - The runtime half of Codira's builtin functions: the `codira.print_*`
//!   functions a `print`/`println` call lowers to, registered in every
//!   runtime's dispatch table next to `new` and `new_array`.
//! - Output capture, so a host (or a test) can collect what Codira code printed
//!   on the current thread instead of having it go to stdout.
//!
//! The compiler side is `codira_codegen::intrinsics::print_functions`: each
//! name and signature here must match the prototype declared there, or
//! linking an assembly that prints fails with a signature mismatch.

use std::{cell::RefCell, io::Write};

use crate::{FunctionDefinition, IntoFunctionDefinition};

thread_local! {
    /// Output captured by [`capture_output`] on this thread, if capturing.
    static CAPTURE: RefCell<Option<Vec<u8>>> = const { RefCell::new(None) };
}

/// Runs `f`, collecting everything Codira code prints on this thread while
/// it runs instead of writing it to standard output. Returns `f`'s result
/// together with the collected text.
///
/// Capture is per thread because Codira code runs on the thread that
/// invokes it: two tests printing concurrently each see only their own
/// output. Nested captures are not merged -- the inner one takes the output
/// it encloses.
pub fn capture_output<R>(f: impl FnOnce() -> R) -> (R, String) {
    let previous = CAPTURE.with(|c| c.borrow_mut().replace(Vec::new()));
    let result = f();
    let captured = CAPTURE.with(|c| std::mem::replace(&mut *c.borrow_mut(), previous));
    let bytes = captured.unwrap_or_default();
    (result, String::from_utf8_lossy(&bytes).into_owned())
}

/// Writes `bytes` to the capture buffer if one is active, otherwise to
/// standard output.
///
/// Write errors on stdout are ignored deliberately: a closed pipe
/// (`codira run app | head`) must not turn a `println` into a crash in the
/// middle of the program, which is what propagating it would require.
fn emit(bytes: &[u8]) {
    let captured = CAPTURE.with(|c| match c.borrow_mut().as_mut() {
        Some(buffer) => {
            buffer.extend_from_slice(bytes);
            true
        }
        None => false,
    });
    if !captured {
        let _ignored = std::io::stdout().lock().write_all(bytes);
    }
}

extern "C" fn print_str(data: *const u8, len: usize) {
    if len == 0 {
        return;
    }
    // Safety: the compiler only passes the two halves of a `str` value, which
    // describe `len` initialized bytes of a string literal (or, later, a
    // live `String`'s buffer) for the duration of the call.
    let bytes = unsafe { std::slice::from_raw_parts(data, len) };
    emit(bytes);
}

extern "C" fn print_i64(value: i64) {
    emit(value.to_string().as_bytes());
}

extern "C" fn print_u64(value: u64) {
    emit(value.to_string().as_bytes());
}

extern "C" fn print_i128(low: u64, high: u64) {
    let value = ((u128::from(high) << 64) | u128::from(low)) as i128;
    emit(value.to_string().as_bytes());
}

extern "C" fn print_u128(low: u64, high: u64) {
    let value = (u128::from(high) << 64) | u128::from(low);
    emit(value.to_string().as_bytes());
}

/// Floats print in their shortest round-trip form, always with a decimal
/// point or exponent so `1.0` does not read back as an integer.
extern "C" fn print_f32(value: f32) {
    emit(format!("{value:?}").as_bytes());
}

extern "C" fn print_f64(value: f64) {
    emit(format!("{value:?}").as_bytes());
}

extern "C" fn print_bool(value: u8) {
    emit(if value != 0 { b"true" } else { b"false" });
}

/// The builtin functions every runtime registers, under the names the
/// compiler's `runtime_functions!` prototypes use.
pub(crate) fn definitions() -> Vec<FunctionDefinition> {
    vec![
        IntoFunctionDefinition::into(
            print_str as extern "C" fn(*const u8, usize),
            "codira.print_str",
        ),
        IntoFunctionDefinition::into(print_i64 as extern "C" fn(i64), "codira.print_i64"),
        IntoFunctionDefinition::into(print_u64 as extern "C" fn(u64), "codira.print_u64"),
        IntoFunctionDefinition::into(print_i128 as extern "C" fn(u64, u64), "codira.print_i128"),
        IntoFunctionDefinition::into(print_u128 as extern "C" fn(u64, u64), "codira.print_u128"),
        IntoFunctionDefinition::into(print_f32 as extern "C" fn(f32), "codira.print_f32"),
        IntoFunctionDefinition::into(print_f64 as extern "C" fn(f64), "codira.print_f64"),
        IntoFunctionDefinition::into(print_bool as extern "C" fn(u8), "codira.print_bool"),
    ]
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use std::ffi;

use inkwell::{context::Context, targets::TargetData, types::FunctionType};

use crate::ir::dispatch_table::FunctionPrototype;

#[macro_use]
mod macros;

/// Defines the properties of an intrinsic function that can be called from
/// Codira. These functions are mostly used internally.
pub trait Intrinsic: Sync {
    /// Returns the prototype of the intrinsic
    fn prototype(&self) -> FunctionPrototype;

    /// Returns the IR type for the function
    fn ir_type<'ink>(&self, context: &'ink Context, target: &TargetData) -> FunctionType<'ink>;
}

intrinsics! {
    /// Allocates memory for the specified `type` in the allocator referred to by `alloc_handle`.
    pub fn new(type_handle: *const ffi::c_void, alloc_handle: *mut ffi::c_void) -> *const *mut ffi::c_void;

    /// Allocates memory for an array of the specified `type` in the allocator referred to by
    /// `alloc_handle` with at least enough capacity to hold `length` elements.
    ///
    /// Note that the elements in the array are left uninitialized.
    pub fn new_array(type_handle: *const ffi::c_void, length: usize, alloc_handle: *mut ffi::c_void) -> *const *mut ffi::c_void;
}

runtime_functions! {
    /// Writes `len` bytes of UTF-8 starting at `data` to standard output.
    pub fn print_str(data: *const u8, len: usize) -> () = "codira.print_str";

    /// Writes a signed integer in decimal.
    pub fn print_i64(value: i64) -> () = "codira.print_i64";

    /// Writes an unsigned integer in decimal.
    pub fn print_u64(value: u64) -> () = "codira.print_u64";

    /// Writes a 128-bit signed integer, passed as its low and high halves
    /// because a 128-bit integer has no portable C calling convention.
    pub fn print_i128(low: u64, high: u64) -> () = "codira.print_i128";

    /// Writes a 128-bit unsigned integer, passed as its low and high halves.
    pub fn print_u128(low: u64, high: u64) -> () = "codira.print_u128";

    /// Writes an `f32` in its shortest round-trip form.
    pub fn print_f32(value: f32) -> () = "codira.print_f32";

    /// Writes an `f64` in its shortest round-trip form.
    pub fn print_f64(value: f64) -> () = "codira.print_f64";

    /// Writes `true` or `false`. Passed as a byte rather than an `i1`: the C
    /// ABI only defines the low byte of a `bool` argument, and LLVM makes no
    /// promise about the rest of the register for an `i1`.
    pub fn print_bool(value: u8) -> () = "codira.print_bool";

    /// Called by fuel-instrumented code when the fuel counter drops below
    /// zero. The runtime ends the call (see `codira_runtime`); if a host's
    /// handler returns, execution continues with the counter still negative.
    pub fn fuel_exhausted() -> () = "codira.fuel_exhausted";
}

/// Every runtime function a `print`/`println` call may need, collected
/// together so the dispatch table has all of them whenever a module prints
/// anything at all. Which ones a given call uses depends on its argument
/// types; listing them all costs one pointer each and keeps the collection
/// pass from having to repeat code generation's type dispatch.
pub fn print_functions() -> [&'static dyn Intrinsic; 8] {
    [
        &print_str,
        &print_i64,
        &print_u64,
        &print_i128,
        &print_u128,
        &print_f32,
        &print_f64,
        &print_bool,
    ]
}

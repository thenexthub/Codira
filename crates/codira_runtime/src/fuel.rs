//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Fuel for ahead-of-time compiled Codira code: the counter that
//!   fuel-instrumented libraries (`codira build --fuel`) decrement at every
//!   function entry and loop back-edge, and what happens when it runs out.
//!
//! A [`Runtime`](crate::Runtime) owns one counter and hands its address to
//! every instrumented assembly it loads (`set_fuel_counter`). Libraries built
//! without instrumentation have no checks and are not metered at all; a
//! budget set on a runtime only bounds the code that was compiled for it.
//!
//! When the counter drops below zero, instrumented code calls
//! `codira.fuel_exhausted`. The runtime reports the exhaustion on standard
//! error and then raises the [`FUEL_EXHAUSTED_EXCEPTION_CODE`] structured
//! exception on Windows, which `codira_healing::trap::protected` turns into a
//! `FaultKind::FuelExhausted` error with control back at the protected call;
//! nothing else catches it, so outside such a guard (and on other platforms)
//! the process aborts. Unwinding through Codira frames is not defined, so
//! there is no Rust-level error return from the middle of a call.

use std::sync::atomic::{AtomicBool, AtomicI64, Ordering};

/// The structured-exception code the runtime raises when fuel runs out
/// (customer bit set, error severity). `codira_healing::trap` recognizes it.
pub const FUEL_EXHAUSTED_EXCEPTION_CODE: u32 = 0xE0C0_F0E1;

/// The counter instrumented code decrements. Boxed so its address stays
/// valid for as long as the runtime (and the assemblies pointing at it) live.
pub(crate) struct FuelCounter {
    counter: Box<AtomicI64>,
    limited: AtomicBool,
}

impl FuelCounter {
    pub(crate) fn new(fuel: Option<u64>) -> Self {
        let counter = Self {
            counter: Box::new(AtomicI64::new(i64::MAX)),
            limited: AtomicBool::new(false),
        };
        counter.set(fuel);
        counter
    }

    /// The address instrumented code reads and writes.
    pub(crate) fn as_ptr(&self) -> *mut i64 {
        self.counter.as_ptr()
    }

    /// Sets the remaining budget; `None` means unlimited.
    pub(crate) fn set(&self, fuel: Option<u64>) {
        if let Some(fuel) = fuel {
            self.limited.store(true, Ordering::Relaxed);
            self.counter
                .store(i64::try_from(fuel).unwrap_or(i64::MAX), Ordering::Relaxed);
        } else {
            self.limited.store(false, Ordering::Relaxed);
            self.counter.store(i64::MAX, Ordering::Relaxed);
        }
    }

    /// Returns the remaining budget, or `None` if it is unlimited.
    pub(crate) fn remaining(&self) -> Option<u64> {
        self.limited
            .load(Ordering::Relaxed)
            .then(|| u64::try_from(self.counter.load(Ordering::Relaxed)).unwrap_or(0))
    }
}

/// Called by instrumented code when the counter drops below zero.
pub(crate) extern "C" fn fuel_exhausted() {
    eprintln!(
        "codira runtime: fuel exhausted -- the call used up its budget of function entries and \
         loop iterations."
    );
    raise_fuel_exhausted();
    // Only reached if something handled the exception and resumed here.
    std::process::abort();
}

#[cfg(windows)]
fn raise_fuel_exhausted() {
    #[link(name = "kernel32")]
    extern "system" {
        fn RaiseException(code: u32, flags: u32, argument_count: u32, arguments: *const usize);
    }
    // Continuable, so a vectored handler (`codira_healing::trap`) may resume
    // at its checkpoint; with no handler the process terminates with this
    // code.
    unsafe { RaiseException(FUEL_EXHAUSTED_EXCEPTION_CODE, 0, 0, std::ptr::null()) };
}

#[cfg(not(windows))]
fn raise_fuel_exhausted() {}

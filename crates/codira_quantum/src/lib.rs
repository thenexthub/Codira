//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Quantum circuits for Codira. See `README.md` in this crate for the
//!   surface, the profile limits and what is verified.
//!
//! - [`circuit`]: the circuit IR (registers, gates, measurement, reset,
//!   barrier) with validation and the QIR base-profile restrictions.
//! - [`sim`]: a dense state-vector simulator with seeded, deterministic
//!   sampling, so circuits run with no Python and no hardware.
//! - [`qasm`]: OpenQASM 2.0 / 3.0 emitters and an importer for what Qiskit's
//!   `qasm2.dumps` / `qasm3.dumps` produce.
//! - [`qir`]: QIR Base Profile emission as LLVM IR text and bitcode (LLVM 22,
//!   opaque pointers), verified by LLVM, and import of base-profile QIR.
//! - [`elaborate`]: compile-time elaboration of `@target(qpu)` Codira functions
//!   (read from `codira_hir`) into circuits.

// `doc_markdown` reads the language name "OpenQASM" as an unquoted code
// identifier; this crate's docs name it on almost every page.
#![allow(clippy::doc_markdown)]

pub mod circuit;
pub mod elaborate;
pub mod qasm;
pub mod qir;
pub mod sim;

pub use circuit::{Circuit, CircuitError, Gate, Op, Register};

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Circuits from Codira functions: one elaborator from HIR to a word-level
//!   circuit IR, and back ends for zero-knowledge proofs (`codira attest`: R1CS
//!   over BN254, Circom source, Groth16 proofs) and FPGAs (`@target(fpga)`:
//!   synthesizable Verilog-2001).
//!
//! # Pipeline
//!
//! ```text
//! oblivious func / @target(fpga) func
//!        |  elaborate (HIR, interpreted at compile time)
//!        v
//!   ir::Circuit  -- eval: the reference semantics (the "netlist evaluator")
//!     |      \
//!     |       verilog::emit -> <name>.v
//!     |  r1cs::lower (exact integers by bit decomposition)
//!     v
//!   r1cs::R1cs -- witness / check, .r1cs / .wtns (iden3), circom::emit
//!     |
//!   groth16 (arkworks): setup, prove, verify
//! ```
//!
//! A function that checks as `oblivious` never branches on, indexes by, or
//! loops over a secret, which is exactly what makes it a fixed circuit: its
//! gates do not depend on the inputs' values. `@target(fpga)` functions get
//! one more freedom: an `if` on an input becomes a multiplexer.
//!
//! # FHE
//!
//! Fully homomorphic encryption (`encrypted`) is not implemented here, but
//! it needs exactly the input this crate produces: [`ir::Circuit`] is a
//! fixed, data-independent DAG of integer and boolean operations, each with
//! a gate-level reading (see `r1cs` for the bit-level decompositions).
//! Zama's `tfhe` crate is the candidate back end: its radix integers support
//! `+ - * & | ^ << >>` and comparisons, and its `if_then_else` is the
//! multiplexer.

pub mod circom;
pub mod elaborate;
pub mod groth16;
pub mod ir;
pub mod r1cs;
pub mod verilog;

pub use elaborate::{elaborate, ElabError, ElabOptions, Mode};
pub use ir::{Circuit, Ty};
pub use r1cs::R1cs;

/// The Verilog for every `@target(fpga)` function among `functions`, as
/// `(function name, module text)`, or the first elaboration error.
pub fn fpga_modules(
    db: &dyn codira_hir::HirDatabase,
    functions: impl IntoIterator<Item = codira_hir::Function>,
) -> Result<Vec<(String, String)>, ElabError> {
    let options = ElabOptions::default();
    let mut modules = Vec::new();
    for func in functions {
        if !elaborate::is_fpga(db, func) {
            continue;
        }
        let circuit = elaborate(db, func, Mode::Fpga, &options)?;
        modules.push((circuit.name.clone(), verilog::emit(&circuit)));
    }
    Ok(modules)
}

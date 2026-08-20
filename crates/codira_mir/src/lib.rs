//! Copyright (c) 2026 Omnira CJSC
//!
//! `codira_mir` -- Codira's parametric mid-level IR.
//!
//! Sits between `codira_hir` and `codira_codegen`, modeled on (and
//! extending) the pipeline shape of Modular's KGEN (the Mojo compiler).
//! See `spec/KGEN_SUPERSET_ARCHITECTURE.md` for the full design rationale
//! and `spec/KGEN_SUPERSET_STATUS.md` for exactly what is implemented vs.
//! designed as of a given session.
//!
//! This crate defines only the IR data model (this module) and generator
//! declarations (`generator`). Compile-time evaluation/elaboration lives in
//! the separate `codira_comptime` crate, which depends on this one -- kept
//! separate so the IR data model has no evaluation-strategy dependencies.

mod generator;
mod op;

pub use generator::{
    Generator, GeneratorId, GeneratorParam, GeneratorStore, StructGenerator, StructGeneratorId,
};
pub use op::{Attr, Body, Op, OpId, OpKind, Region};

#[cfg(test)]
mod tests;

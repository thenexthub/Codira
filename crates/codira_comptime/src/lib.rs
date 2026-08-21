//! Copyright (c) 2026 Omnira CJSC
//!
//! `codira_comptime` -- the parametric compile-time evaluator for
//! `codira_mir`. Structural analog of `KGEN/lib/Elaborator` +
//! `KGEN/lib/Interpreter`. See `spec/KGEN_SUPERSET_ARCHITECTURE.md` §4.
//!
//! This crate currently provides the interpreter (§4 point 1: evaluating a
//! restricted region to a concrete value). The elaborator (§4 point 2:
//! walking the call graph and producing monomorphized `Op::Func`s, wired as
//! a salsa query for incremental reuse per §4.1) is designed but not yet
//! implemented -- see `spec/KGEN_SUPERSET_STATUS.md`.

mod elaborate;
mod interp;
mod value;

pub use elaborate::elaborate;
pub use interp::eval_body;
pub use value::{EvalError, Env, Value};

#[cfg(test)]
mod tests;

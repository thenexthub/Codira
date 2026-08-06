//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Original module content restored; copyright header moved to top.
//!
//! Adaptive self-healing runtime for Codira.
//!
//! Implements the pieces of `spec/self_healing_programming_language.md`
//! that are genuinely achievable as real, working systems code (see each
//! module's own doc comment for what it does and, just as importantly,
//! what it deliberately does not attempt):
//!
//! - [`trap`]: hardware-fault (access violation / SIGSEGV, floating-point
//!   exception / SIGFPE) interception and routing to a Rust handler,
//!   running on this process's real OS-level exception mechanism.
//! - [`ranker`]: Thompson-sampling ranking of healing strategies.
//! - [`engine`]: ties the above together with `codira_smt` (feasibility
//!   checking) and `codira_runtime`'s dispatch table (the same mechanism
//!   hot-reloading already uses) to actually apply a chosen strategy by
//!   swapping which precompiled implementation a faulting function calls.
//!
//! What this is *not*: a JIT in the strict sense of generating new machine
//! code at runtime. The engine selects among AOT-precompiled candidate
//! implementations rather than synthesizing new code on the fly -- see
//! `engine`'s doc comment for the honest reasoning behind that scope cut.

pub mod contract;
pub mod deficiency;
pub mod engine;
pub mod fingerprint;
pub mod ranker;
pub mod recovery_graph;
pub mod supervisor;
pub mod trap;


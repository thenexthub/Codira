//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Proof-gated live updates, the checking side: the intent an author writes
//!   ([`intent`]), the certificate a prover produces
//!   ([`cert::UpdateCertificate`]), the checker a running process calls
//!   ([`check::check`], [`UpdateAdmission`]), the migration verdicts
//!   ([`migration`]) and the set of functions that must not be active at the
//!   swap ([`affected`]).
//!
//! This crate never sees source code: it depends on `codira_certificate`
//! (query checking, code ranges, hashes) and `codira_smt` (Z3) and on nothing
//! of the parser, type checker, verifier or code generator. The *prover*
//! lives in `codira_hir::update`, next to the symbolic executor it reuses; it
//! produces the types defined here.
//!
//! # What an admission rests on
//!
//! A process that admits an update trusts: Z3; the verification-condition
//! generator and the call-graph fact extractor of the compiler that wrote the
//! certificate (they turned two programs into queries and facts, and no
//! checker can see whether those capture the programs); the SHA-256 binding
//! of queries to bytes; and this crate. It does **not** trust the code
//! generator that produced either library beyond the code-hash binding (and
//! it does not detect a miscompilation: the queries are about the source).
//!
//! The certificate does **not** show the intent is the right one. It shows
//! that, if the author's intent is what was meant, the update does that and
//! nothing else the verifier models: functions the intent does not name keep
//! their results and effects; named ones keep their stated relation.
//!
//! Prior work this builds on, and does not claim to have invented: Hayden et
//! al., *Specifying and verifying the correctness of dynamic software
//! updates* (VSTTE 2012) verify an update by merging old and new programs
//! offline against user-written specifications; Godlin and Strichman's
//! regression verification proves two program versions equivalent
//! with callees as uninterpreted functions and automatically inferred
//! coupling invariants for loops; `VeriBin` (NDSS 2025) proves binary patches
//! equivalent offline. What is new here is only the combination: the check is
//! on the admission path of a running native runtime, it is integrated with
//! the language's contracts, effect summaries and typed heap, and the process
//! re-decides it with Z3 alone.

pub mod admission;
pub mod affected;
pub mod cert;
pub mod check;
pub mod facts;
pub mod intent;
pub mod migration;

pub use admission::{Admission, TestAdmission, UpdateAdmission, UpdateBundle, Z3Admission};
pub use affected::{AffectedNode, AffectedSet, Reason};
pub use cert::{
    query_hash, FunctionEntry, Header, Obligation, ObligationKind, Rejection, RejectionReason,
    UpdateCertificate, VersionRecord, FORMAT,
};
pub use check::{check, CheckOptions, ObligationResult, Report};
pub use facts::{Facts, FnFacts, NodeKind, TypeFacts};
pub use intent::{FieldIntent, FunctionIntent, Intent, MigrationPolicy, Relation};
pub use migration::{FieldChange, MigrationOutcome, MigrationRecord};

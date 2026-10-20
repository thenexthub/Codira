//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The update certificate: what `codira update prove` writes and a running
//!   process re-checks before it admits a new version of a library. Built on
//!   the records of `codira_certificate` (SMT-LIB2 queries, code hashes); the
//!   additions are the two artifacts, the intent, and obligations tagged by
//!   what they prove about the *transition* from old to new.
//!
//! # Shape
//!
//! ```text
//! certificate := header, intent text, functions, facts, migrations,
//!                obligations, rejections, body hash
//! header      := old artifact (file, size, SHA-256), new artifact, intent
//!                SHA-256, solver id, rlimit, compiler id
//! obligation  := id, kind (equivalence | contract | effect | migration),
//!                subject, level, assumes (ids), queries (SMT-LIB2, expected
//!                unsat) with their SHA-256
//! ```
//!
//! A *rejection* is an obligation the prover could not discharge, with why:
//! a counterexample, an unsupported construct (named), or an exhausted
//! solver budget. A certificate with a rejection is still written, so that
//! the reason travels with the update, but no process admits it.
//!
//! # What the hashes bind, and what they do not
//!
//! The artifact hashes bind the certificate to two library files; each
//! function record binds that function's machine code in each file to the
//! hash of its source text. The queries are about the *source* programs (the
//! prover turns the HIR of both versions into them); nothing in the
//! certificate relates a function's machine code to its source except their
//! sitting in one record. `body_sha256` is an integrity check for accidents
//! and casual edits, not a signature: whoever can rewrite every field and
//! recompute it can forge a certificate (see the crate docs).

use std::fmt;

use codira_certificate::{ArtifactRecord, CodeRecord, Query};
use serde_derive::{Deserialize, Serialize};

use crate::{affected::AffectedSet, facts::Facts, intent::Relation, migration::MigrationRecord};

/// The `format` of the certificates this crate writes and reads.
pub const FORMAT: &str = "codira-update-certificate/1";

/// What an obligation proves.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum ObligationKind {
    /// A function computes the same result, performs the same effects and
    /// leaves the same heap as before.
    Equivalence,
    /// A changed function meets its contract (and, for `refines`, the old
    /// contract's relation to the new).
    Contract,
    /// A function still excludes the effects it is annotated to exclude.
    Effect,
    /// The migration of a layout-changed type's live values.
    Migration,
}

impl fmt::Display for ObligationKind {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(match self {
            ObligationKind::Equivalence => "equivalence",
            ObligationKind::Contract => "contract",
            ObligationKind::Effect => "effect",
            ObligationKind::Migration => "migration",
        })
    }
}

/// One proven claim, as the queries that prove it.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct Obligation {
    /// Unique in the certificate: `<kind>:<subject>[:<detail>]`.
    pub id: String,
    pub kind: ObligationKind,
    /// The function or `Type.field` it is about.
    pub subject: String,
    /// For equivalence: the highest level of the relational verifier the
    /// proof needed (1 straight-line and branching integer code, 2 calls,
    /// 3 loops, 4 memory, 5 effect traces). Zero for the other kinds.
    pub level: u8,
    /// What it claims, in words.
    pub what: String,
    /// The obligations whose conclusions this one's premise relies on: an
    /// unchanged callee is assumed to compute what it did, which is its own
    /// equivalence obligation's conclusion.
    #[serde(default)]
    pub assumes: Vec<String>,
    /// Each decided `unsat`; together they prove the claim. Empty for an
    /// attested obligation.
    pub queries: Vec<Query>,
    /// SHA-256 of each query text, in order.
    pub query_sha256: Vec<String>,
    /// A fact the compiler vouches for that Z3 does not re-decide: the
    /// result of an analysis that is not a logical formula (the effect
    /// summaries of `@noalloc`, `@nopanic`, `@noblock`, `@realtime`). The
    /// checker lists it as attested, never as re-proven.
    #[serde(default)]
    pub attested: bool,
}

/// Why an obligation is not proven.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(tag = "why", rename_all = "lowercase")]
pub enum RejectionReason {
    /// The two versions differ on this input.
    Refuted { counterexample: String },
    /// The relational verifier does not model a construct the function uses.
    Unsupported { construct: String },
    /// The solver ran out of its budget or answered `unknown`.
    Unknown { reason: String },
    /// A rule of the update, not a solver verdict: an unnamed addition, a
    /// removed type, a lossy migration the intent did not accept.
    Policy { reason: String },
}

impl fmt::Display for RejectionReason {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RejectionReason::Refuted { counterexample } => {
                write!(f, "refuted: {counterexample}")
            }
            RejectionReason::Unsupported { construct } => {
                write!(f, "unsupported: {construct}")
            }
            RejectionReason::Unknown { reason } => write!(f, "undecided: {reason}"),
            RejectionReason::Policy { reason } => f.write_str(reason),
        }
    }
}

/// An obligation the prover could not discharge.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct Rejection {
    pub kind: ObligationKind,
    pub subject: String,
    pub reason: RejectionReason,
}

impl fmt::Display for Rejection {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} of `{}`: {}", self.kind, self.subject, self.reason)
    }
}

/// Where one version of a function is.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct VersionRecord {
    /// The symbol its machine code carries in the library.
    pub symbol: String,
    /// SHA-256 of its source text.
    pub source_sha256: String,
    /// Its machine code in the library; `None` when it has none of its own
    /// (inlined away, generic) -- see `note`.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub code: Option<CodeRecord>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub note: Option<String>,
}

/// One function of either version.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct FunctionEntry {
    /// The function's path, e.g. `geometry.area`, or `Point.len` for a
    /// method.
    pub name: String,
    /// What the intent says about it; `None` for a function the author left
    /// unchanged (which therefore has an equivalence obligation).
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub relation: Option<Relation>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub old: Option<VersionRecord>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub new: Option<VersionRecord>,
    /// The effect attributes the new version carries (`noalloc`, `nopanic`,
    /// `noblock`, `realtime`): each needs an effect obligation.
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub effect_attrs: Vec<String>,
    /// An `extern` declaration in both versions: it has no code in either
    /// library and is trusted to be the same.
    #[serde(default)]
    pub external: bool,
}

/// The two libraries, the intent and the solver the proofs were made with.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct Header {
    /// The compiler (and so verification-condition generator) that wrote it.
    pub compiler: String,
    /// The Z3 that decided the obligations at proof time.
    pub solver: String,
    /// The `rlimit` budget each query was decided under.
    pub rlimit: u32,
    pub old: ArtifactRecord,
    pub new: ArtifactRecord,
    /// SHA-256 of the intent text embedded in the certificate.
    pub intent_sha256: String,
}

/// A certificate for one update.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct UpdateCertificate {
    /// Always [`FORMAT`].
    pub format: String,
    pub header: Header,
    /// The intent file, verbatim.
    pub intent: String,
    /// The highest level of the relational verifier any equivalence
    /// obligation needed.
    pub level: u8,
    pub functions: Vec<FunctionEntry>,
    /// What the compiler knew about the old program's call graph and use of
    /// types; the affected set is computed from it.
    pub facts: Facts,
    /// Types whose layout differs between the versions.
    pub changed_types: Vec<String>,
    pub affected: AffectedSet,
    pub migrations: Vec<MigrationRecord>,
    pub obligations: Vec<Obligation>,
    /// Empty for a certificate a process can admit.
    pub rejections: Vec<Rejection>,
    /// Things the update does that the certificate does not cover and a
    /// reader should know (not failures).
    #[serde(default)]
    pub notes: Vec<String>,
    /// SHA-256 over the rest of the certificate. An integrity check, not a
    /// signature.
    pub body_sha256: String,
}

impl UpdateCertificate {
    /// SHA-256 of the certificate with `body_sha256` empty.
    pub fn compute_body_hash(&self) -> String {
        let mut copy = self.clone();
        copy.body_sha256 = String::new();
        let bytes = serde_json::to_vec(&copy).expect("a certificate always serializes");
        codira_certificate::sha256_hex(&bytes)
    }

    /// Fills in `body_sha256`. Call last.
    pub fn seal(&mut self) {
        self.body_sha256 = self.compute_body_hash();
    }

    /// Reads a certificate from JSON text.
    pub fn from_json(text: &str) -> Result<UpdateCertificate, String> {
        let certificate: UpdateCertificate =
            serde_json::from_str(text).map_err(|e| format!("not an update certificate: {e}"))?;
        if certificate.format != FORMAT {
            return Err(format!(
                "certificate format `{}` is not `{FORMAT}`",
                certificate.format
            ));
        }
        Ok(certificate)
    }

    /// The certificate as JSON text.
    pub fn to_json(&self) -> String {
        serde_json::to_string_pretty(self).expect("a certificate always serializes")
    }

    /// The obligation with this id.
    pub fn obligation(&self, id: &str) -> Option<&Obligation> {
        self.obligations.iter().find(|o| o.id == id)
    }
}

/// A query's hash, as the certificate records it.
pub fn query_hash(query: &Query) -> String {
    codira_certificate::sha256_hex(query.smtlib2.as_bytes())
}

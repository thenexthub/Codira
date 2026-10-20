//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - What the compiler knew about the old program when it proved an update
//!   safe, in a form the checker can read without the compiler: for every
//!   function, instance of a generic function and closure body, whom it calls
//!   and which types it touches or holds. The affected set
//!   ([`crate::affected`]) is computed from these.
//!
//! The facts are produced by the compiler front end (`codira_hir::update`)
//! and are part of the trusted base exactly like the verification-condition
//! generator: the checker recomputes the affected set from them and compares
//! it with the certificate's, but it cannot tell whether a fact is true.
//! Generic functions have no code of their own; their *instances* are the
//! nodes (`id[i64]`), and a closure body is a node apart from the function
//! that creates it (`f#closure0`), because each is separate machine code with
//! its own stack frames.

use serde_derive::{Deserialize, Serialize};

/// What a node of the call graph is.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum NodeKind {
    /// A non-generic function or method.
    Function,
    /// A generic function at concrete type arguments.
    Instance,
    /// The body of a closure literal.
    Closure,
}

/// One node of the old program's call graph.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct FnFacts {
    /// Unique among the nodes: the function's path, `path[args]` for an
    /// instance, `<creator id>#closure<n>` for a closure body.
    pub id: String,
    pub kind: NodeKind,
    /// The path of the declared function this node belongs to (a generic
    /// function's instances and its closures all belong to it), which is
    /// what an intent names.
    pub function: String,
    /// For a closure body: the node whose body contains the literal.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub parent: Option<String>,
    /// Nodes it calls by name.
    #[serde(default)]
    pub calls: Vec<String>,
    /// It calls through a function value, so it may call any node in
    /// `value_targets` of any node.
    #[serde(default)]
    pub indirect_calls: bool,
    /// Nodes it uses as values (a function name not called, a closure
    /// literal): their code can be reached through any indirect call.
    #[serde(default)]
    pub value_targets: Vec<String>,
    /// Types whose fields, elements or construction it uses.
    #[serde(default)]
    pub touches: Vec<String>,
    /// Every type named by the type of one of its parameters, locals or
    /// intermediate values -- what a stack frame of it can hold.
    #[serde(default)]
    pub mentions: Vec<String>,
}

/// A declared type and the types its fields (and type arguments) name.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct TypeFacts {
    pub name: String,
    #[serde(default)]
    pub mentions: Vec<String>,
}

/// Everything [`crate::affected::compute`] reads.
#[derive(Debug, Clone, PartialEq, Eq, Default, Serialize, Deserialize)]
pub struct Facts {
    pub functions: Vec<FnFacts>,
    pub types: Vec<TypeFacts>,
}

impl Facts {
    /// The node with this id.
    pub fn node(&self, id: &str) -> Option<&FnFacts> {
        self.functions.iter().find(|f| f.id == id)
    }
}

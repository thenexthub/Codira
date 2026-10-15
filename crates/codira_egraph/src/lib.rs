//! Copyright (c) 2026 Omnira CJSC. All Rights Reserved.
//! Author: Tunjay Akbarli
//! Date: September 15, 2026
//!
//! `codira_egraph` -- equality saturation for `codira_mir`.
//!
//! # Why this exists: the phase-ordering problem
//!
//! A conventional optimizer is a *sequence* of destructive rewrites. Each
//! pass looks at the program, decides a rewrite is an improvement, and
//! throws the original away. That makes every pass order a gamble:
//! `simplify` might canonicalize `a*2` into `a<<1`, which is cheaper in
//! isolation but destroys the `a*b + a*c -> a*(b+c)` factoring the next
//! pass would have found. Compilers have lived with this for fifty years,
//! mitigating it with hand-tuned pass orders and fixpoint iteration
//! (`codira_mir::pass::default_pipeline` documents exactly such an order,
//! and its own limitations).
//!
//! Equality saturation dissolves the problem instead of mitigating it.
//! Rather than *replacing* an expression with a rewritten one, it records
//! that the two are **equal** in an e-graph -- a data structure that
//! compactly represents an entire equivalence class of programs -- and
//! keeps applying every rewrite rule until no rule discovers anything new
//! (*saturation*). Only then does it extract a program, choosing the
//! cheapest member of the equivalence class under an explicit cost model.
//! No rewrite is ever regretted, because no rewrite is ever destructive.
//!
//! The technique is due to Tate et al. (*Equality Saturation: A New
//! Approach to Optimization*, POPL 2009); this implementation follows the
//! much faster rebuilding-based design of egg (Willsey, Nandi, Wang,
//! Flatt, Tatlock, Panchekha, *egg: Fast and Extensible Equality
//! Saturation*, POPL 2021), including its deferred-congruence `rebuild`
//! algorithm and e-class analyses.
//!
//! It is also the natural generalization of the vision in KGEN's own
//! design document (`modular/KGEN/docs/DesignOverview.md`, "Dynamic
//! Programming / Caching"), which describes searching a DAG of kernel
//! generator expansions under a cost model. KGEN searches over
//! *instantiations*; this searches over *expressions*, with the same
//! "explore, then pick cheapest" structure.
//!
//! # What enters the e-graph
//!
//! Only **pure** ops: constants, arithmetic, comparison, logic, bitwise,
//! shifts, tuple pack/project, the leaf references (`core.arg`,
//! `cf.block_arg`, `param.ref`), and the memory ops (`mem.*`: a memory
//! state is a value, so these are as pure as an add -- see
//! `codira_mir::mem` for the rules over them). Everything else -- `cf.if`,
//! `cf.while`, `cf.for`, `core.call`, `cf.yield` -- is **opaque**: it enters as
//! a unique leaf node that can never be merged with another, carrying its
//! operand e-classes so extraction can rebuild it. The bodies nested
//! inside an opaque op's regions are optimized *recursively* in their own
//! e-graphs, so `x + 0` inside a loop body still simplifies; what does not
//! happen is reasoning across the boundary. This is the same conservatism
//! [`codira_mir::pass`] applies, for the same reason: a loop or call may
//! have effects, and equality of *values* is not equality of *effects*.
//!
//! # Soundness: the `definitely_int` gate
//!
//! The IR is untyped at this layer ([`codira_mir::Attr`] distinguishes
//! `Int` from `Float`, but a `core.arg` carries no type). That matters
//! because `codira_mir::fold_op` promotes mixed int/float arithmetic, and
//! **floating-point addition and multiplication are not associative**:
//! `(a + b) + c` and `a + (b + c)` can differ by rounding. Applying
//! associativity to a float expression is a miscompilation.
//!
//! So every rule that restructures arithmetic is gated on an e-class
//! analysis, [`Analysis::definitely_int`], which is `true` only when the
//! class provably holds an integer: it contains an `Int` constant, or an
//! integer-only operation (bitwise/shift, whose operands `fold_op` rejects
//! for floats), or every one of its nodes is an int-producing operation
//! over definitely-int children. Leaves of unknown type (`core.arg`,
//! `cf.block_arg`, `param.ref`) and opaque nodes are *not* definitely-int,
//! so nothing restructures around them. Rules that hold for IEEE floats
//! too (commutativity of `+`/`*`, boolean identities, double negation) are
//! ungated.
//!
//! This gate is deliberately conservative: it costs optimization
//! opportunities on integer code whose types the IR does not make
//! evident. A typed MIR would recover them; until then, refusing to
//! optimize is the correct failure mode.

mod cost;
mod egraph;
mod extract;
mod pattern;
mod rules;

use codira_mir::{verify_body, Body};
pub use cost::CostTable;
pub use egraph::{EClassId, EGraph};
pub use pattern::{Pattern, PatternRule, MAX_BINDINGS};
use rustc_hash::FxHashSet;

/// What the caller knows about the types of a body's free references.
///
/// The IR is untyped at this layer, so `core.arg`/`cf.block_arg` carry no
/// type and the [`definitely_int`](egraph::Analysis) analysis has to
/// assume the worst -- which means the arithmetic-restructuring rules
/// almost never fire on a real function body, whose values all flow from
/// its arguments.
///
/// The frontend, however, *does* know: `codira_hir` has inferred a type
/// for every parameter before MIR lowering ever runs. This is the channel
/// for handing that knowledge down. Declaring an argument integer here is
/// a promise; breaking it re-enables float-unsound rewrites, so the
/// constructors are explicit rather than inferred from anything.
///
/// The default ([`TypeEnv::unknown`]) assumes nothing, so an optimizer
/// run without type information is conservative but always correct.
#[derive(Debug, Clone, Default)]
pub struct TypeEnv {
    int_args: FxHashSet<u32>,
    int_block_args: FxHashSet<u32>,
}

impl TypeEnv {
    /// Assumes nothing about any reference: only the IEEE-exact rules
    /// fire. This is the default, and what [`optimize`] uses.
    pub fn unknown() -> Self {
        Self::default()
    }

    /// Declares the given `core.arg` indices integer-typed.
    pub fn with_int_args(indices: impl IntoIterator<Item = u32>) -> Self {
        TypeEnv {
            int_args: indices.into_iter().collect(),
            int_block_args: FxHashSet::default(),
        }
    }

    /// Declares one more `core.arg` index integer-typed.
    pub fn int_arg(mut self, index: u32) -> Self {
        self.int_args.insert(index);
        self
    }

    /// Declares one `cf.block_arg` index integer-typed. Block arguments
    /// are region-local, so this applies to every region at the same
    /// index -- declare it only when every loop in the body carries an
    /// integer there.
    pub fn int_block_arg(mut self, index: u32) -> Self {
        self.int_block_args.insert(index);
        self
    }

    pub(crate) fn is_int_arg(&self, index: u32) -> bool {
        self.int_args.contains(&index)
    }

    pub(crate) fn is_int_block_arg(&self, index: u32) -> bool {
        self.int_block_args.contains(&index)
    }
}

/// What saturation actually achieved -- reported rather than assumed, so
/// a caller can tell "no rule applied" from "we ran out of budget".
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SaturationReport {
    /// Rule-application rounds actually run.
    pub iterations: usize,
    /// E-nodes in the graph when saturation stopped. Never more than the
    /// node cap, unless the body's own graph already was (see
    /// [`EGraphOptimizer::max_nodes`]).
    pub nodes: usize,
    /// E-classes when saturation stopped.
    pub classes: usize,
    /// `true` when a round discovered nothing new (a genuine fixpoint);
    /// `false` when a limit cut the search short -- the output is still
    /// correct, just possibly not the best the rules could find.
    pub saturated: bool,
    /// Which extractor chose the program that was returned.
    pub extraction: Extraction,
}

/// How the extracted program was chosen (see `extract.rs`).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Extraction {
    /// Greedy tree-cost extraction only: the exact search was disabled
    /// (a zero budget) or not applicable.
    Greedy,
    /// The exact search ran to completion: no program in the graph has a
    /// lower DAG cost. `improved` says whether it beat the greedy choice.
    Exact { improved: bool },
    /// The exact search ran out of its step budget. The best program it
    /// had found is used -- never worse than greedy; `improved` says
    /// whether it was better.
    BudgetExhausted { improved: bool },
}

/// Search limits. Equality saturation can grow an e-graph without bound
/// on expressions with rich rewrite structure (associativity plus
/// commutativity alone generate factorially many orderings), so a real
/// implementation is always budgeted.
///
/// Every limit is a count of work, never a duration: the same input and
/// limits give the same program on every machine and every run
/// (`KGEN_SUPERSET_STATUS` §32.5). There is deliberately no timeout.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct EGraphOptimizer {
    /// Maximum rule-application rounds, per region.
    pub max_iterations: usize,
    /// Hard e-node budget, per region: a rewrite that could take the graph
    /// past it is not applied (see `EGraph::saturate`). The graph a body
    /// builds before any rewrite is linear in the body and not bounded by
    /// this; a body whose own graph fills the cap is extracted without
    /// being rewritten.
    pub max_nodes: usize,
    /// Step budget of the exact (branch-and-bound) extractor, per region;
    /// `0` extracts greedily only. See `extract.rs`.
    pub exact_extraction_budget: u64,
    /// Per-operation costs extraction minimizes; see [`CostTable`].
    pub costs: CostTable,
}

impl Default for EGraphOptimizer {
    fn default() -> Self {
        EGraphOptimizer {
            max_iterations: Self::DEFAULT_MAX_ITERATIONS,
            max_nodes: Self::DEFAULT_MAX_NODES,
            exact_extraction_budget: Self::DEFAULT_EXACT_EXTRACTION_BUDGET,
            costs: CostTable::GENERIC,
        }
    }
}

impl EGraphOptimizer {
    /// Default rule-application rounds per region.
    pub const DEFAULT_MAX_ITERATIONS: usize = 8;
    /// Default e-node cap per region.
    pub const DEFAULT_MAX_NODES: usize = 10_000;
    /// Default exact-extraction step budget per region.
    pub const DEFAULT_EXACT_EXTRACTION_BUDGET: u64 = 10_000;

    pub fn new() -> Self {
        Self::default()
    }

    /// Optimizes one body with no type information -- see
    /// [`EGraphOptimizer::optimize_body_with`].
    pub fn optimize_body(&self, body: &Body) -> (Body, SaturationReport) {
        self.optimize_body_with(body, &TypeEnv::unknown())
    }

    /// Optimizes one body: builds an e-graph, saturates it with the
    /// rewrite rules, and extracts the cheapest equivalent program.
    ///
    /// `env` supplies what the caller knows about argument types; the
    /// arithmetic-restructuring rules depend on it (see [`TypeEnv`]).
    ///
    /// The returned body is always structurally valid
    /// ([`codira_mir::verify_body`]) and computes the same result value as
    /// the input. Bodies nested inside opaque ops are optimized
    /// recursively before their parent is added to the graph.
    pub fn optimize_body_with(&self, body: &Body, env: &TypeEnv) -> (Body, SaturationReport) {
        self.optimize_body_with_rules(body, env, &std::sync::Arc::from(Vec::new()))
    }

    /// [`EGraphOptimizer::optimize_body_with`], also applying `rules` --
    /// rewrite rules supplied as data, which the caller vouches for (see
    /// [`PatternRule`]). Bodies nested in opaque ops get the same rules.
    pub fn optimize_body_with_rules(
        &self,
        body: &Body,
        env: &TypeEnv,
        rules: &std::sync::Arc<[PatternRule]>,
    ) -> (Body, SaturationReport) {
        // An empty body has no result to preserve and nothing to rewrite.
        if body.is_empty() {
            return (
                Body::new(),
                SaturationReport {
                    iterations: 0,
                    nodes: 0,
                    classes: 0,
                    saturated: true,
                    extraction: Extraction::Greedy,
                },
            );
        }

        let mut graph = EGraph::with_types(env.clone());
        graph.rules = rules.clone();
        let root = match graph.add_body(body, self) {
            Some(root) => root,
            // A body whose result op cannot be represented (e.g. a bare
            // `cf.yield`, which is only meaningful as a loop terminator)
            // is returned untouched rather than mangled.
            None => {
                return (
                    body.clone(),
                    SaturationReport {
                        iterations: 0,
                        nodes: graph.num_nodes(),
                        classes: graph.num_classes(),
                        saturated: true,
                        extraction: Extraction::Greedy,
                    },
                )
            }
        };

        let mut report = graph.saturate(self);
        let (extracted, extraction) =
            extract::extract(&graph, root, &self.costs, self.exact_extraction_budget);
        report.extraction = extraction;

        // Extraction emits a DAG in post-order, which for memory need not be
        // an executable order (a load can land after the store that ends the
        // state it reads). Find one that is, or keep the input: a memory
        // chain the scheduler cannot linearize is never handed on.
        let extracted = if codira_mir::mem::uses_memory(&extracted) {
            match codira_mir::mem::schedule(&extracted) {
                Some(scheduled) => scheduled,
                None => return (body.clone(), report),
            }
        } else {
            extracted
        };

        // Extraction is the one place a bug could produce malformed IR;
        // verifying here means a defect surfaces as "optimizer declined"
        // rather than as a corrupt body handed to codegen.
        match verify_body(&extracted) {
            Ok(()) => (extracted, report),
            Err(_) => (body.clone(), report),
        }
    }
}

/// Optimizes `body` with default limits. The convenience entry point.
pub fn optimize(body: &Body) -> Body {
    EGraphOptimizer::default().optimize_body(body).0
}

#[cfg(test)]
mod tests;

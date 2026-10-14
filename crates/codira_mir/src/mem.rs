//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Memory SSA for Eidos: the rules that make memory operations sound in a
//!   value-numbered, equality-saturated IR, their verifier, and the scheduler
//!   that turns a memory-state DAG back into an executable order.
//!
//! # Design (Eidos RFC-003: Memory SSA)
//!
//! This is the design of RFC-001 §2 ("heap-aware equality saturation") as
//! built, with the decisions that RFC left open made.
//!
//! ## Memory is a value
//!
//! Memory is an SSA value of type `mem` ([`TypeId::MEM`]). An operation
//! that changes memory takes the state it changes and returns the next one;
//! a load names the state it reads:
//!
//! ```text
//! %m0 = mem.entry                       : mem   -- memory on function entry
//! %m1 = mem.alloc(site 0, i64 x 1) %m0  : mem   -- a fresh slot exists in %m1
//! %p  = mem.addr %m1                    : ptr   -- ... at this address
//! %m2 = mem.store %m1, %p, %v           : mem   -- MemoryDef
//! %x  = mem.load %m2, %p                : i64   -- MemoryUse
//! %q  = mem.offset(i64) %p, %i          : ptr   -- pure address arithmetic
//! ```
//!
//! Because each state is a distinct value, two stores at different program
//! points have different state operands and can never be merged by value
//! numbering, CSE or e-graph congruence. Soundness comes from the shape of
//! the IR, not from special cases in each optimizer. Two loads of the same
//! type from the same address in the same state *are* the same value, and
//! the e-graph merges them -- redundant-load elimination for free. (The CSE
//! pass does not: it does not key on result types yet.)
//!
//! ## Structured control flow needs no `MemoryPhi`
//!
//! Classical `MemorySSA` puts a phi at every CFG merge. Eidos has no CFG at
//! this level: memory crosses a loop as one more loop-carried value (a
//! `cf.for`/`cf.while` operand, a block argument inside, a `cf.yield`
//! operand at the end), and the loop's result carries the final state out.
//!
//! `cf.if` regions take no block arguments and cannot name values of the
//! enclosing body, so memory has no way *into* them. Rather than invent one
//! now, this first version forbids memory in `cf.if` regions (the verifier
//! says so). Conditional memory effects are the next extension; the obvious
//! one is giving `cf.if` carried operands, as the loops have.
//!
//! ## The two invariants
//!
//! Each memory state is **consumed** at most once -- by the store or
//! allocation that produces the next state, or by the loop, yield, tuple or
//! call that hands it on. Loads and `mem.addr` *use* a state without
//! consuming it. That is **linearity**: memory forms a chain, never a tree,
//! which is the only shape that can be executed on one real memory.
//!
//! Every use of a state comes before the op that consumes it, in body order.
//! That is the **schedule** invariant: emitting a body's ops in order is a
//! correct execution, because no load reads a state that a later store has
//! already overwritten. Code generation relies on it and nothing else.
//!
//! [`verify_memory`] checks both (it runs as part of `verify_body`). A
//! transformation that rearranges ops -- e-graph extraction above all, which
//! emits a DAG in post-order -- calls [`schedule`] to find an order that
//! satisfies the schedule invariant, and declines when there is none.
//!
//! ## What a body's memory means
//!
//! A body's *observable* memory is the state its result carries out: a
//! function that writes through a pointer argument returns its final state
//! (for example in a `core.tuple` with its value). A state nothing consumes
//! is unobservable, and dead-code elimination removes the stores that only
//! fed it -- which is correct, and is how stores to a local slot nobody reads
//! disappear.
//!
//! ## Equality saturation over memory
//!
//! Memory ops enter the e-graph as ordinary nodes (`codira_egraph`). The
//! rules are *reducing*: each one shortens a memory chain or leaves it
//! unchanged, never permutes it, which keeps memory classes linear in the
//! size of the program (RFC-001 §2.3):
//!
//! * store-to-load forwarding: `load(store(m, p, v), p) == v`, only when the
//!   load's type is exactly the stored value's type;
//! * load bypass: `load(store(m, q, v), p) == load(m, p)` and `load(alloc(m),
//!   p) == load(m, p)` when `p` and `q` provably do not alias;
//! * dead store: `store(store(m, p, a), p, b) == store(m, p, b)`.
//!
//! Store reordering is not a rule: it is the factorial blow-up. Alias facts
//! come from allocation provenance -- addresses derived from different
//! allocation sites, or from the same site at different constant indices,
//! do not alias. An address of unknown origin (an argument, a loaded
//! pointer) may alias anything. A dead-store rewrite can leave a load
//! reading the state it skipped; extraction may then produce a branching
//! chain, the verifier rejects it, and the optimizer keeps the original
//! body. That costs an optimization, never correctness.
//!
//! ## What this unlocks, and what it does not do yet
//!
//! Time-travel replay needs every write to be an explicit value it can
//! record; lock elision needs to see which states a critical section reads
//! and writes; GPU offload needs loads and stores it can map to global
//! memory. Each still needs its own work on top: there is no frontend
//! lowering from Codira source to these ops yet (`codira_hir::mir_lower`
//! does not model mutation), no volatile or atomic operations (they must be
//! separate ops no rule matches -- RFC-001 §2.5), and calls do not thread
//! memory (a `core.call` is treated as touching none).

use rustc_hash::{FxHashMap, FxHashSet};

use crate::{
    op::{Body, MemOp, Op, OpId, OpKind, Region},
    ty::TypeId,
    verify::VerifyError,
};

/// Whether `op` produces a memory state.
pub fn is_memory_state(op: &Op) -> bool {
    matches!(
        op.kind,
        OpKind::Mem(MemOp::Entry | MemOp::Alloc { .. } | MemOp::Store)
    ) || op.ty == TypeId::MEM
}

/// Whether `body` (or any region nested in it) contains a memory op or a
/// memory-typed value.
pub fn uses_memory(body: &Body) -> bool {
    body.iter().any(|(_, op)| {
        matches!(op.kind, OpKind::Mem(_))
            || op.ty == TypeId::MEM
            || op.regions.iter().any(|r| uses_memory(&r.body))
    })
}

/// How an op relates to one of its memory-state operands.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Access {
    /// Reads the state without ending it (`mem.load`, `mem.addr`).
    Use,
    /// Ends the state: the op produces the next one, or hands it on.
    Consume,
    /// A memory state in a position nothing defines for one.
    Invalid,
}

fn access(op: &Op, operand_index: usize) -> Access {
    match op.kind {
        OpKind::Mem(MemOp::Load | MemOp::Addr) if operand_index == 0 => Access::Use,
        OpKind::Mem(MemOp::Store | MemOp::Alloc { .. }) if operand_index == 0 => Access::Consume,
        OpKind::While | OpKind::For | OpKind::Yield | OpKind::Tuple | OpKind::Call(_) => {
            Access::Consume
        }
        _ => Access::Invalid,
    }
}

/// Checks the memory rules described in the module doc over `body` and
/// every region nested in it. `top_level` is whether `body` is a function
/// body (where `mem.entry` may appear).
pub fn verify_memory(body: &Body, top_level: bool) -> Result<(), VerifyError> {
    verify_in(body, top_level, false)
}

fn verify_in(body: &Body, top_level: bool, in_if: bool) -> Result<(), VerifyError> {
    let fail = |op: OpId, rule: &'static str| Err(VerifyError { op, rule });

    // Position of each op, for the schedule check.
    let position: FxHashMap<OpId, usize> = body
        .iter()
        .enumerate()
        .map(|(i, (id, _))| (id, i))
        .collect();
    // For each memory state: its consumer, and the latest use seen so far.
    let mut consumer: FxHashMap<OpId, OpId> = FxHashMap::default();
    let mut last_use: FxHashMap<OpId, OpId> = FxHashMap::default();
    let mut entries = 0usize;

    for (id, op) in body.iter() {
        let memory_op = matches!(op.kind, OpKind::Mem(_)) || op.ty == TypeId::MEM;
        if in_if && memory_op {
            return fail(
                id,
                "memory in a cf.if region is not supported yet: cf.if takes no carried values, \
                 so a memory state cannot enter or leave it",
            );
        }
        match op.kind {
            OpKind::Mem(MemOp::Entry) => {
                if !top_level {
                    return fail(
                        id,
                        "mem.entry is only valid in a function body; memory enters a loop as a \
                         carried value",
                    );
                }
                entries += 1;
                if entries > 1 {
                    return fail(id, "a body has at most one mem.entry");
                }
            }
            OpKind::Mem(MemOp::Addr) => {
                let source = op.operands.first().map(|&m| &body.get(m).kind);
                if !matches!(source, Some(OpKind::Mem(MemOp::Alloc { .. }))) {
                    return fail(id, "mem.addr must name the state a mem.alloc produced");
                }
            }
            _ => {}
        }

        for (index, &operand) in op.operands.iter().enumerate() {
            if !is_memory_state(body.get(operand)) {
                continue;
            }
            match access(op, index) {
                Access::Use => {
                    if consumer.contains_key(&operand) {
                        return fail(
                            id,
                            "memory state read after the op that consumes it (schedule \
                             invariant)",
                        );
                    }
                    last_use.insert(operand, id);
                }
                Access::Consume => {
                    if consumer.insert(operand, id).is_some() {
                        return fail(
                            id,
                            "memory state consumed twice: memory must form a chain, not a \
                             tree (linearity)",
                        );
                    }
                }
                Access::Invalid => {
                    return fail(id, "memory state used as an ordinary value");
                }
            }
        }

        let nested_if = matches!(op.kind, OpKind::If);
        for region in &op.regions {
            verify_in(&region.body, false, in_if || nested_if)?;
        }
    }

    // Uses recorded before their consumer existed are fine; the check
    // above catches a use that comes after. This is a backstop for the
    // positional reading of the same rule.
    for (state, user) in &last_use {
        if let Some(consumer) = consumer.get(state) {
            if position[user] > position[consumer] {
                return fail(*user, "memory state read after the op that consumes it");
            }
        }
    }
    Ok(())
}

/// Reorders `body` so that the schedule invariant holds, keeping the
/// original order wherever the dependencies allow, and recursing into
/// nested regions. Returns `None` when no order satisfies it -- a load that
/// needs a state after something consumed it, which the IR cannot express
/// on one memory -- or when the result op could not stay last.
///
/// Bodies without memory are returned unchanged (as a copy).
pub fn schedule(body: &Body) -> Option<Body> {
    let ids: Vec<OpId> = body.iter().map(|(id, _)| id).collect();
    let index: FxHashMap<OpId, usize> = ids.iter().enumerate().map(|(i, &id)| (id, i)).collect();
    let n = ids.len();

    // Edges: operand -> op (data), and use-of-m -> consumer-of-m (anti).
    let mut successors: Vec<Vec<usize>> = vec![Vec::new(); n];
    let mut indegree = vec![0usize; n];
    let mut add_edge = |from: usize, to: usize, successors: &mut Vec<Vec<usize>>| {
        successors[from].push(to);
        indegree[to] += 1;
    };
    let mut consumers: FxHashMap<OpId, usize> = FxHashMap::default();
    let mut uses: FxHashMap<OpId, Vec<usize>> = FxHashMap::default();
    for (i, &id) in ids.iter().enumerate() {
        let op = body.get(id);
        let mut seen = FxHashSet::default();
        for (k, &operand) in op.operands.iter().enumerate() {
            if seen.insert(operand) {
                add_edge(index[&operand], i, &mut successors);
            }
            if is_memory_state(body.get(operand)) {
                match access(op, k) {
                    Access::Use => uses.entry(operand).or_default().push(i),
                    Access::Consume => {
                        consumers.insert(operand, i);
                    }
                    Access::Invalid => {}
                }
            }
        }
    }
    for (state, consumer) in &consumers {
        for &user in uses.get(state).map_or(&[][..], Vec::as_slice) {
            if user != *consumer {
                add_edge(user, *consumer, &mut successors);
            }
        }
    }

    // Kahn's algorithm, always taking the earliest ready op: deterministic,
    // and the identity on a body that already satisfies the invariant.
    let mut ready: std::collections::BTreeSet<usize> =
        (0..n).filter(|&i| indegree[i] == 0).collect();
    let mut order = Vec::with_capacity(n);
    while let Some(i) = ready.pop_first() {
        order.push(i);
        for &next in &successors[i] {
            indegree[next] -= 1;
            if indegree[next] == 0 {
                ready.insert(next);
            }
        }
    }
    if order.len() != n {
        return None;
    }
    if n > 0 && *order.last()? != n - 1 {
        return None;
    }

    let mut out = Body::new();
    let mut remap: FxHashMap<OpId, OpId> = FxHashMap::default();
    for i in order {
        let op = body.get(ids[i]);
        let regions = op
            .regions
            .iter()
            .map(|r| schedule(&r.body).map(|b| Region::with_args(r.num_args, b)))
            .collect::<Option<Vec<_>>>()?;
        let operands: Vec<OpId> = op.operands.iter().map(|o| remap[o]).collect();
        let new_id = out.push_with_regions_typed(op.kind.clone(), operands, regions, op.ty);
        remap.insert(ids[i], new_id);
    }
    Some(out)
}

#[cfg(test)]
mod tests;

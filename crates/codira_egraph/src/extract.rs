//! Copyright (c) 2026 Omnira CJSC. All Rights Reserved.
//! Author: Tunjay Akbarli
//! Date: September 15, 2026
//!
//! Extraction: choosing the cheapest program out of the saturated
//! e-graph, then rebuilding it as a [`Body`].
//!
//! Two extractors run, one after the other: a greedy one that is always
//! correct and always fast, and an exact one that may improve on it within
//! a deterministic budget.
//!
//! # Greedy: a fixpoint, not a single bottom-up pass
//!
//! After saturation an e-graph is typically **cyclic**: `x` and `x + 0`
//! and `(x + 0) + 0` all live in one class, so following a class's nodes
//! can lead back to the class itself. A single bottom-up traversal would
//! not terminate, and a naive depth-first walk could pick a node whose
//! cost depends on its own class.
//!
//! So costs are computed by **relaxation**, the way a shortest-path
//! algorithm handles cycles: every class starts at infinite cost, and each
//! round recomputes `cost(class) = min over its nodes of (node cost + sum
//! of children costs)`, using the previous round's values. Costs only ever
//! decrease and are bounded below, so the iteration converges; when a
//! round improves nothing, the assignment is optimal for this cost model.
//! A class that is still infinite afterwards is unreachable-by-any-finite-
//! term (only possible through a malformed graph) and falls back to its
//! first node.
//!
//! That cost model is the *tree* cost: a subterm used twice is paid for
//! twice. The program actually emitted is a DAG -- `build` emits a shared
//! class once -- so the greedy choice can be beaten when sharing matters.
//!
//! # Exact: branch and bound over the DAG cost
//!
//! [`exact`] searches for the choice of one node per reachable class that
//! minimizes the *DAG* cost (each chosen class paid once), which is the
//! problem ILP extractors solve. It is a depth-first branch and bound:
//!
//! * classes are decided in a fixed order (lowest dense index among those still
//!   needed), and each class tries its nodes cheapest first;
//! * the bound is the cost so far plus the cheapest node of every class still
//!   needed -- admissible, since each of them is paid at least once;
//! * a choice that would make the chosen graph cyclic is skipped;
//! * the greedy result is the starting incumbent, so the exact search can only
//!   ever replace it with something strictly cheaper.
//!
//! The search is budgeted in **steps** (alternatives tried), never in
//! time. When the budget runs out, the best program found so far -- at
//! worst the greedy one -- is used. The search order is a pure function of
//! the graph, so the same graph and budget give the same program on every
//! machine (`KGEN_SUPERSET_STATUS` §32.5).
//!
//! # The cost model
//!
//! Per-operation costs come from a [`CostTable`] chosen for the target --
//! see `cost.rs`. Opaque ops (control flow, calls) cost the table's
//! `opaque` plus their operands -- their region bodies were optimized
//! separately, and their *identity* is fixed, so no choice is being made
//! about them here.
//!
//! This is the same "search the space, pick by cost model" structure
//! KGEN's design document describes for kernel-generator expansion
//! (`modular/KGEN/docs/DesignOverview.md`); the cost model is the knob
//! that decides what "optimized" means, and it is deliberately explicit
//! and in one place rather than implicit in the order of a pass pipeline.

use std::collections::BTreeSet;

use codira_mir::{Body, OpId, OpKind};
use rustc_hash::FxHashMap;
use smallvec::SmallVec;

use crate::{
    cost::CostTable,
    egraph::{is_pure, EClassId, EGraph, ENode, NodeKind},
    Extraction,
};

/// Cost of an operator itself, excluding its operands.
#[allow(clippy::match_same_arms)]
pub(crate) fn node_cost(costs: &CostTable, node: &ENode) -> u64 {
    match &node.kind {
        NodeKind::Pure(kind) => match kind {
            OpKind::Const(_) => costs.literal,
            OpKind::Arg(_) | OpKind::BlockArg(_) | OpKind::ParamRef(_) => costs.leaf,
            OpKind::Mul => costs.mul,
            OpKind::Div | OpKind::Rem => costs.div,
            OpKind::Shl | OpKind::Shr => costs.shift,
            _ => costs.alu,
        },
        NodeKind::Opaque(_) => costs.opaque,
    }
}

const INFINITY: u64 = u64::MAX / 4;

/// Per-class best choice: the node to extract and its total cost.
pub(crate) struct Choice {
    node: ENode,
    cost: u64,
}

/// Extracts the cheapest term rooted at `root` as a fresh [`Body`], and
/// says which extractor chose it.
pub(crate) fn extract(
    graph: &EGraph,
    root: EClassId,
    costs: &CostTable,
    exact_budget: u64,
) -> (Body, Extraction) {
    let greedy = compute_costs(graph, costs);
    let root = graph.find_const(root);

    let (best, extraction) = if exact_budget == 0 {
        (greedy, Extraction::Greedy)
    } else {
        match exact(graph, root, costs, &greedy, exact_budget) {
            Some((choices, extraction)) => (choices, extraction),
            None => (greedy, Extraction::Greedy),
        }
    };

    let mut body = Body::new();
    let mut built: FxHashMap<EClassId, OpId> = FxHashMap::default();
    let result = build(graph, &best, root, &mut body, &mut built);

    // A term whose value is an already-built shared subexpression may not
    // be the last op in the arena, but `Body::result()` is positional. Re-
    // materializing the value keeps the body's result correct without a
    // dedicated copy op -- the same identity `codira_mir::pass::rewrite`
    // uses for this, and for the same reason.
    if let Some(result) = result {
        if body.result() != Some(result) {
            // The wrapper is transparent, so it carries the wrapped
            // value's type (the Tuple itself is a one-element aggregate
            // and stays untyped -- nothing consumes it but the projection).
            let ty = body.get(result).ty;
            let tuple = body.push(OpKind::Tuple, [result]);
            body.push_typed(OpKind::TupleGet(0), [tuple], ty);
        }
    }
    (body, extraction)
}

/// Cost relaxation to fixpoint -- see the module doc.
fn compute_costs(graph: &EGraph, costs: &CostTable) -> FxHashMap<EClassId, Choice> {
    let classes = graph.class_ids();
    let mut best: FxHashMap<EClassId, Choice> = FxHashMap::default();

    loop {
        let mut improved = false;
        for &class in &classes {
            let canon = graph.find_const(class);
            for node in graph.nodes(canon) {
                // Sum children using the *previous* round's costs; a child
                // with no cost yet makes this node unusable this round.
                let mut total = node_cost(costs, node);
                let mut usable = true;
                for &child in &node.children {
                    match best.get(&graph.find_const(child)) {
                        Some(c) if c.cost < INFINITY => total = total.saturating_add(c.cost),
                        _ => {
                            usable = false;
                            break;
                        }
                    }
                }
                if !usable {
                    continue;
                }
                let better = match best.get(&canon) {
                    None => true,
                    Some(current) => total < current.cost,
                };
                if better {
                    best.insert(
                        canon,
                        Choice {
                            node: node.clone(),
                            cost: total,
                        },
                    );
                    improved = true;
                }
            }
        }
        if !improved {
            break;
        }
    }

    // Any class still without a choice is only reachable through a cycle
    // with no finite base case -- a malformed graph. Fall back to its
    // first node so extraction is total rather than panicking.
    for &class in &classes {
        let canon = graph.find_const(class);
        if let std::collections::hash_map::Entry::Vacant(e) = best.entry(canon) {
            if let Some(node) = graph.nodes(canon).first() {
                e.insert(Choice {
                    node: node.clone(),
                    cost: INFINITY,
                });
            }
        }
    }
    best
}

/// The DAG cost of a choice map: every class reachable from `root` through
/// chosen nodes, paid once. `None` when the choice is cyclic or a class
/// has no choice (a malformed graph, where exact search is not attempted).
fn dag_cost(
    graph: &EGraph,
    costs: &CostTable,
    choices: &FxHashMap<EClassId, Choice>,
    root: EClassId,
) -> Option<u64> {
    // Iterative DFS with an explicit colour per class, so a cycle is
    // detected rather than looped on.
    #[derive(Clone, Copy, PartialEq, Eq)]
    enum Mark {
        Open,
        Done,
    }
    let mut marks: FxHashMap<EClassId, Mark> = FxHashMap::default();
    let mut total = 0u64;
    let mut stack: Vec<(EClassId, usize)> = vec![(root, 0)];
    marks.insert(root, Mark::Open);
    while let Some(&mut (class, ref mut next)) = stack.last_mut() {
        let choice = choices.get(&class)?;
        if choice.cost >= INFINITY {
            return None;
        }
        if let Some(&child) = choice.node.children.get(*next) {
            *next += 1;
            let child = graph.find_const(child);
            match marks.get(&child) {
                Some(Mark::Open) => return None,
                Some(Mark::Done) => {}
                None => {
                    marks.insert(child, Mark::Open);
                    stack.push((child, 0));
                }
            }
        } else {
            total = total.saturating_add(node_cost(costs, &choice.node));
            marks.insert(class, Mark::Done);
            stack.pop();
        }
    }
    Some(total)
}

/// One class's alternatives, in the dense numbering the search uses.
struct Alternative {
    /// Index of the node within `graph.nodes(class)`.
    node: usize,
    cost: u64,
    children: SmallVec<[usize; 2]>,
}

/// The search state of [`exact`]: which node each class has, which
/// classes are still needed, and what has been paid.
struct Search<'a> {
    alts: &'a [Vec<Alternative>],
    min_cost: &'a [u64],
    decided: Vec<Option<usize>>,
    /// How many decided nodes (plus the root, once) need each class.
    needed_by: Vec<u32>,
    /// Classes that are needed but not decided yet.
    frontier: BTreeSet<usize>,
    frontier_min: u64,
    paid: u64,
    /// Visit marks for [`Search::closes_cycle`], stamped with `generation`
    /// so they never have to be cleared.
    seen: Vec<u64>,
    generation: u64,
}

impl Search<'_> {
    fn apply(&mut self, class: usize, alt: usize) {
        let a = &self.alts[class][alt];
        self.decided[class] = Some(alt);
        self.paid += a.cost;
        self.frontier.remove(&class);
        self.frontier_min -= self.min_cost[class];
        for &child in &a.children {
            self.needed_by[child] += 1;
            if self.needed_by[child] == 1 && self.decided[child].is_none() {
                self.frontier.insert(child);
                self.frontier_min += self.min_cost[child];
            }
        }
    }

    fn undo(&mut self, class: usize, alt: usize) {
        let a = &self.alts[class][alt];
        for &child in a.children.iter().rev() {
            self.needed_by[child] -= 1;
            if self.needed_by[child] == 0 && self.decided[child].is_none() {
                self.frontier.remove(&child);
                self.frontier_min -= self.min_cost[child];
            }
        }
        self.decided[class] = None;
        self.paid -= a.cost;
        self.frontier.insert(class);
        self.frontier_min += self.min_cost[class];
    }

    /// Whether choosing `alt` for `class` would close a cycle through the
    /// nodes already chosen. Only decided classes have outgoing edges, so
    /// any cycle is caught when its last class is decided.
    fn closes_cycle(&mut self, class: usize, alt: usize) -> bool {
        self.generation += 1;
        let generation = self.generation;
        let mut stack: Vec<usize> = self.alts[class][alt].children.to_vec();
        while let Some(c) = stack.pop() {
            if c == class {
                return true;
            }
            if std::mem::replace(&mut self.seen[c], generation) == generation {
                continue;
            }
            if let Some(chosen) = self.decided[c] {
                stack.extend(self.alts[c][chosen].children.iter().copied());
            }
        }
        false
    }
}

/// Exact DAG-cost extraction by branch and bound -- see the module doc.
///
/// Returns the choice map to build from and how it was obtained, or `None`
/// when the greedy choice is unusable as a starting point (a malformed
/// graph), in which case the caller keeps the greedy result.
fn exact(
    graph: &EGraph,
    root: EClassId,
    costs: &CostTable,
    greedy: &FxHashMap<EClassId, Choice>,
    budget: u64,
) -> Option<(FxHashMap<EClassId, Choice>, Extraction)> {
    let greedy_cost = dag_cost(graph, costs, greedy, root)?;

    // Dense numbering of every class reachable from the root through *any*
    // node: the universe the search chooses within. Discovery order is
    // deterministic (node order within a class is fixed by construction).
    let mut index: FxHashMap<EClassId, usize> = FxHashMap::default();
    let mut classes: Vec<EClassId> = vec![root];
    index.insert(root, 0);
    let mut cursor = 0;
    while cursor < classes.len() {
        let class = classes[cursor];
        cursor += 1;
        for node in graph.nodes(class) {
            for &child in &node.children {
                let child = graph.find_const(child);
                if let std::collections::hash_map::Entry::Vacant(e) = index.entry(child) {
                    e.insert(classes.len());
                    classes.push(child);
                }
            }
        }
    }

    // Each class's alternatives, cheapest (by greedy tree cost) first so
    // the first descent reproduces the greedy program and later ones are
    // pruned against it.
    let tree_cost = |node: &ENode| -> u64 {
        node.children
            .iter()
            .fold(node_cost(costs, node), |acc, &c| {
                let c = graph.find_const(c);
                acc.saturating_add(greedy.get(&c).map_or(INFINITY, |choice| choice.cost))
            })
    };
    let mut alts: Vec<Vec<Alternative>> = Vec::with_capacity(classes.len());
    let mut has_choice = false;
    for &class in &classes {
        let nodes = graph.nodes(class);
        let mut list: Vec<(u64, Alternative)> = nodes
            .iter()
            .enumerate()
            .map(|(i, node)| {
                (
                    tree_cost(node),
                    Alternative {
                        node: i,
                        cost: node_cost(costs, node),
                        children: node
                            .children
                            .iter()
                            .map(|&c| index[&graph.find_const(c)])
                            .collect(),
                    },
                )
            })
            .collect();
        list.sort_by_key(|(tree, alt)| (*tree, alt.node));
        has_choice |= list.len() > 1;
        alts.push(list.into_iter().map(|(_, alt)| alt).collect());
    }
    // One node per class: there is nothing to choose, and greedy already
    // chose it.
    if !has_choice {
        return Some((reuse(greedy), Extraction::Exact { improved: false }));
    }
    let min_cost: Vec<u64> = alts
        .iter()
        .map(|list| list.iter().map(|a| a.cost).min().unwrap_or(0))
        .collect();

    let mut search = Search {
        alts: &alts,
        min_cost: &min_cost,
        decided: vec![None; classes.len()],
        needed_by: vec![0; classes.len()],
        frontier: BTreeSet::from([0]),
        frontier_min: min_cost[0],
        paid: 0,
        seen: vec![0; classes.len()],
        generation: 0,
    };
    search.needed_by[0] = 1;

    let mut best_cost = greedy_cost;
    let mut best: Option<Vec<Option<usize>>> = None;
    let mut steps = 0u64;
    let mut exhausted = false;

    // Explicit stack of (class, next alternative to try, applied one):
    // recursion would be as deep as the program is large.
    let mut stack: Vec<(usize, usize, Option<usize>)> = vec![(0, 0, None)];
    'search: while let Some(top) = stack.last_mut() {
        let (class, next, applied) = (top.0, &mut top.1, &mut top.2);
        if let Some(alt) = applied.take() {
            search.undo(class, alt);
        }
        let mut chosen = None;
        while *next < alts[class].len() {
            let alt = *next;
            *next += 1;
            steps += 1;
            if steps > budget {
                exhausted = true;
                break 'search;
            }
            if search.closes_cycle(class, alt) {
                continue;
            }
            search.apply(class, alt);
            if search.paid + search.frontier_min >= best_cost {
                search.undo(class, alt);
                continue;
            }
            chosen = Some(alt);
            break;
        }
        let Some(alt) = chosen else {
            stack.pop();
            continue;
        };
        if let Some(top) = stack.last_mut() {
            top.2 = Some(alt);
        }
        match search.frontier.first().copied() {
            // A complete program, and (by the bound check) a cheaper one.
            None => {
                best_cost = search.paid;
                best = Some(search.decided.clone());
            }
            Some(next_class) => stack.push((next_class, 0, None)),
        }
    }

    let improved = best.is_some();
    let extraction = if exhausted {
        Extraction::BudgetExhausted { improved }
    } else {
        Extraction::Exact { improved }
    };
    let Some(decided) = best else {
        return Some((reuse(greedy), extraction));
    };

    // Translate the dense solution back to e-classes. Only classes the
    // solution reaches matter to `build`; the total cost stored per class
    // is not consulted by it.
    let mut choices: FxHashMap<EClassId, Choice> = FxHashMap::default();
    for (dense, alt) in decided.iter().enumerate() {
        if let Some(alt) = alt {
            let class = classes[dense];
            let node = graph.nodes(class)[alts[dense][*alt].node].clone();
            choices.insert(
                class,
                Choice {
                    node,
                    cost: best_cost,
                },
            );
        }
    }
    Some((choices, extraction))
}

/// A copy of the greedy choice map (choices are small; the graph they
/// point into is what is large).
fn reuse(greedy: &FxHashMap<EClassId, Choice>) -> FxHashMap<EClassId, Choice> {
    greedy
        .iter()
        .map(|(&class, choice)| {
            (
                class,
                Choice {
                    node: choice.node.clone(),
                    cost: choice.cost,
                },
            )
        })
        .collect()
}

/// Emits the chosen term for `class` into `body`, memoizing so a shared
/// subexpression is emitted exactly once (extraction produces a DAG, not
/// a tree).
fn build(
    graph: &EGraph,
    best: &FxHashMap<EClassId, Choice>,
    class: EClassId,
    body: &mut Body,
    built: &mut FxHashMap<EClassId, OpId>,
) -> Option<OpId> {
    let canon = graph.find_const(class);
    if let Some(&id) = built.get(&canon) {
        return Some(id);
    }
    let choice = best.get(&canon)?;

    let mut operands: SmallVec<[OpId; 2]> = SmallVec::new();
    for &child in &choice.node.children {
        operands.push(build(graph, best, child, body, built)?);
    }

    let id = match &choice.node.kind {
        NodeKind::Pure(kind) => {
            debug_assert!(is_pure(kind), "non-pure op reached pure extraction");
            body.push_typed(kind.clone(), operands, choice.node.ty)
        }
        NodeKind::Opaque(serial) => {
            let original = graph.opaque.get(*serial as usize)?;
            body.push_with_regions_typed(
                original.kind.clone(),
                operands,
                original.regions.iter().cloned(),
                original.ty,
            )
        }
    };
    built.insert(canon, id);
    Some(id)
}

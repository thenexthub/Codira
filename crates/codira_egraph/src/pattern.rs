//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Rewrite rules as data: an equation `lhs == rhs` between two patterns over
//!   variables, matched against the e-graph and applied like the built-in rules
//!   in `rules.rs`.
//!
//! The built-in rules are Rust code, written and reviewed with the e-graph.
//! A [`PatternRule`] comes from elsewhere -- a library's `@rewrite`
//! attribute, compiled by `codira_codegen` -- and the e-graph trusts it only
//! because its creator proved it first (`codira_codegen::rewrite` asks
//! `codira_tv` for a proof and rejects the rule without one). Nothing in this
//! module re-checks that; it is the *applier*.
//!
//! # Matching
//!
//! A pattern is a tree: a variable, a literal, or a pure operation over
//! sub-patterns. Matching a class tries every node of the class whose
//! operation is the pattern's, and every match of its children, keeping the
//! bindings consistent (a variable used twice must bind one class). The
//! number of bindings tried per rule and class is capped
//! ([`MAX_BINDINGS`]), counted in matches, never in time.
//!
//! # Types
//!
//! A rule is proven over unbounded integers and booleans. A variable the
//! rule uses as an integer only binds a class known to hold one -- through
//! the `definitely_int` analysis or the class's own integer type -- so an
//! integer identity is never applied to a float.

use codira_mir::{Attr, Body, OpId, OpKind};
use rustc_hash::FxHashMap;

use crate::{
    egraph::{is_pure, EClassId, EGraph, NodeKind},
    rules::{Match, Term},
};

/// The most bindings one rule may produce for one class in one round.
pub const MAX_BINDINGS: usize = 64;

/// One side of a rule.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Pattern {
    /// Pattern variable `i` (the rule's `i`-th parameter).
    Var(u32),
    /// A literal.
    Const(Attr),
    /// A pure operation over sub-patterns.
    Op(OpKind, Vec<Pattern>),
}

/// An equation `lhs == rhs`, applied left to right.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct PatternRule {
    /// Where the rule came from, for reports.
    pub name: String,
    pub lhs: Pattern,
    pub rhs: Pattern,
    /// Variables the rule uses as integers (see the module doc, "Types").
    int_vars: Vec<u32>,
}

impl PatternRule {
    /// Builds a rule from two bodies over `core.arg` variables, as a
    /// frontend lowers the two sides of an equation.
    ///
    /// Refused: a left side that is a bare variable or literal (it would
    /// match every class), an operation the e-graph does not reason about
    /// (control flow, calls, memory), and a right side that uses a variable
    /// the left side does not bind.
    pub fn from_bodies(name: impl Into<String>, lhs: &Body, rhs: &Body) -> Result<Self, String> {
        let lhs = to_pattern(lhs)?;
        let rhs = to_pattern(rhs)?;
        if !matches!(lhs, Pattern::Op(..)) {
            return Err(
                "the left side must be an operation, not a bare variable or literal".into(),
            );
        }
        let mut bound = Vec::new();
        variables(&lhs, &mut bound);
        let mut used = Vec::new();
        variables(&rhs, &mut used);
        if let Some(free) = used.iter().find(|v| !bound.contains(v)) {
            return Err(format!(
                "the right side uses parameter #{free}, which the left side does not bind"
            ));
        }
        let mut int_vars = Vec::new();
        int_uses(&lhs, &mut int_vars);
        int_uses(&rhs, &mut int_vars);
        int_vars.sort_unstable();
        int_vars.dedup();
        Ok(PatternRule {
            name: name.into(),
            lhs,
            rhs,
            int_vars,
        })
    }
}

fn to_pattern(body: &Body) -> Result<Pattern, String> {
    let root = body.result().ok_or("an empty side")?;
    to_pattern_at(body, root)
}

fn to_pattern_at(body: &Body, id: OpId) -> Result<Pattern, String> {
    let op = body.get(id);
    match &op.kind {
        OpKind::Arg(i) => Ok(Pattern::Var(*i)),
        OpKind::Const(attr @ (Attr::Int(_) | Attr::Bool(_))) => Ok(Pattern::Const(attr.clone())),
        kind if is_pure(kind)
            && !matches!(
                kind,
                OpKind::Const(_)
                    | OpKind::BlockArg(_)
                    | OpKind::ParamRef(_)
                    | OpKind::Tuple
                    | OpKind::TupleGet(_)
                    | OpKind::Mem(_)
            ) =>
        {
            let children = op
                .operands
                .iter()
                .map(|&operand| to_pattern_at(body, operand))
                .collect::<Result<Vec<_>, _>>()?;
            Ok(Pattern::Op(kind.clone(), children))
        }
        other => Err(format!("`{other:?}` cannot appear in a rewrite rule")),
    }
}

fn variables(pattern: &Pattern, out: &mut Vec<u32>) {
    match pattern {
        Pattern::Var(v) => {
            if !out.contains(v) {
                out.push(*v);
            }
        }
        Pattern::Const(_) => {}
        Pattern::Op(_, children) => children.iter().for_each(|c| variables(c, out)),
    }
}

/// Variables that appear directly under an integer operation.
fn int_uses(pattern: &Pattern, out: &mut Vec<u32>) {
    if let Pattern::Op(kind, children) = pattern {
        let integer_operands = !matches!(kind, OpKind::And | OpKind::Or | OpKind::Not);
        for child in children {
            if let (true, Pattern::Var(v)) = (integer_operands, child) {
                out.push(*v);
            }
            int_uses(child, out);
        }
    }
}

type Bindings = FxHashMap<u32, EClassId>;

/// Every binding of `pattern`'s variables under which `class` matches it.
fn ematch(graph: &EGraph, pattern: &Pattern, class: EClassId, budget: &mut usize) -> Vec<Bindings> {
    match pattern {
        Pattern::Var(v) => vec![Bindings::from_iter([(*v, graph.find_const(class))])],
        Pattern::Const(attr) => {
            if graph.analysis(class).and_then(|a| a.constant.as_ref()) == Some(attr) {
                vec![Bindings::default()]
            } else {
                Vec::new()
            }
        }
        Pattern::Op(kind, children) => {
            let mut out = Vec::new();
            for node in graph.nodes(class) {
                if *budget == 0 {
                    break;
                }
                let NodeKind::Pure(node_kind) = &node.kind else {
                    continue;
                };
                if node_kind != kind || node.children.len() != children.len() {
                    continue;
                }
                let mut partial = vec![Bindings::default()];
                for (sub, &child) in children.iter().zip(&node.children) {
                    let mut next = Vec::new();
                    for binding in &partial {
                        for extension in ematch(graph, sub, child, budget) {
                            if let Some(merged) = merge(binding, &extension) {
                                next.push(merged);
                            }
                        }
                    }
                    partial = next;
                    if partial.is_empty() {
                        break;
                    }
                }
                for binding in partial {
                    if *budget == 0 {
                        break;
                    }
                    *budget -= 1;
                    out.push(binding);
                }
            }
            out
        }
    }
}

fn merge(a: &Bindings, b: &Bindings) -> Option<Bindings> {
    let mut merged = a.clone();
    for (v, class) in b {
        match merged.get(v) {
            Some(existing) if existing != class => return None,
            _ => {
                merged.insert(*v, *class);
            }
        }
    }
    Some(merged)
}

fn instantiate(pattern: &Pattern, bindings: &Bindings) -> Term {
    match pattern {
        Pattern::Var(v) => Term::Class(bindings[v]),
        Pattern::Const(attr) => Term::Const(attr.clone()),
        Pattern::Op(kind, children) => Term::Node(
            kind.clone(),
            children.iter().map(|c| instantiate(c, bindings)).collect(),
        ),
    }
}

/// The matches of every rule against every class, for one round.
pub(crate) fn collect_rule_matches(graph: &EGraph, rules: &[PatternRule], out: &mut Vec<Match>) {
    for rule in rules {
        for class in graph.class_ids() {
            let mut budget = MAX_BINDINGS;
            for bindings in ematch(graph, &rule.lhs, class, &mut budget) {
                let typed = rule.int_vars.iter().all(|v| {
                    bindings.get(v).is_none_or(|&bound| {
                        graph.analysis(bound).is_some_and(|a| a.definitely_int)
                            || graph.class_ty(bound).is_int()
                    })
                });
                if typed {
                    out.push((class, instantiate(&rule.rhs, &bindings)));
                }
            }
        }
    }
}

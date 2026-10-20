//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The affected set of an update: the functions of the *old* program whose
//!   stack frames must not be live when the new code is swapped in, computed
//!   from the call graph and type-use facts of the compiler, and exposed to the
//!   runtime through [`crate::UpdateAdmission`].
//!
//! # Why a set at all
//!
//! The runtime parks every task at a safe point before it swaps. A frame
//! that is still on a stack when the table is repointed keeps running *old*
//! machine code, which is only harmless if that code does not depend on
//! anything the update changes. Two things it can depend on:
//!
//! * **its own behaviour** -- the function is changed: its remaining
//!   instructions are the ones the author replaced;
//! * **the layout of an object it reads, writes or holds** -- a type whose
//!   layout changed is migrated in place of the old objects; old code that
//!   reads `Counter.n` at the old offset, or holds a pointer to a `Counter`
//!   across the swap, would read the wrong bytes.
//!
//! # What the set contains
//!
//! `functions` (the definitive part) is the least set such that:
//!
//! 1. every node belonging to a function the intent names (changed, removed; an
//!    added function has no old frames) is in it, together with the closure
//!    bodies created in it;
//! 2. every node that *touches* (reads or writes a field or element of,
//!    constructs) or *mentions* (names in the type of a parameter, a local or
//!    any intermediate value) a type in the layout-affected set is in it. The
//!    layout-affected set is the changed types and every type that mentions
//!    one, transitively -- a `Holder` whose field is a changed value `Point`
//!    changes layout with it;
//! 3. a closure body and the node that creates it are in it together: the
//!    creator's frame may hold the closure's value, and the closure's frame
//!    runs code written in the creator.
//!
//! Generic functions are nodes per *instance* (`get[Point]`), closures are
//! nodes apart from their creator, because each is separate machine code with
//! its own frames.
//!
//! `inlining_exposed` lists the transitive callers of those nodes that are
//! not in the first list: when the optimizer inlines a callee into a caller,
//! the caller's frame runs the callee's old instructions. The compiler's
//! inlining decisions are not recorded in the artifact, so the list is
//! offered as it is: a runtime that knows its library was built without
//! inlining (`--mode dev` at `-O0`) may ignore it, and otherwise should treat
//! a live frame of a listed function as unsafe too. An indirect call is an
//! edge to every node used as a value.
//!
//! # What it does not contain
//!
//! A caller that is merely suspended in a call to a changed function is *not*
//! in `functions`: it resumes in its old code with the new callee's result,
//! which the callee's relation (`equal`, `refines`) and the caller's own
//! equivalence proof account for. A caller that cannot be proven unchanged
//! must be named in the intent, and then is in the set by rule 1.

use std::collections::{BTreeMap, BTreeSet};

use serde_derive::{Deserialize, Serialize};

use crate::facts::{Facts, NodeKind};

/// Why a node is in the set.
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Serialize, Deserialize)]
#[serde(tag = "because", rename_all = "snake_case")]
pub enum Reason {
    /// The intent names its function.
    Changed,
    /// It reads, writes or constructs a value of this layout-affected type.
    Touches { ty: String },
    /// A frame of it can hold a value of this layout-affected type.
    Holds { ty: String },
    /// It is a closure body created in this node.
    ClosureOf { creator: String },
    /// This closure body is created in it.
    Creates { closure: String },
}

/// One node of the affected set.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct AffectedNode {
    pub id: String,
    pub reasons: Vec<Reason>,
}

/// The functions that must not be active at the swap. See the module docs.
#[derive(Debug, Clone, PartialEq, Eq, Default, Serialize, Deserialize)]
pub struct AffectedSet {
    pub functions: Vec<AffectedNode>,
    /// Callers that may contain inlined copies of a node in `functions`.
    pub inlining_exposed: Vec<String>,
    /// The layout-affected types the set was computed against.
    pub affected_types: Vec<String>,
}

impl AffectedSet {
    /// Whether a live frame of `id` makes the swap unsafe.
    pub fn contains(&self, id: &str) -> bool {
        self.functions.iter().any(|f| f.id == id)
    }

    /// The ids whose frames must not be live.
    pub fn must_not_be_active(&self) -> impl Iterator<Item = &str> {
        self.functions.iter().map(|f| f.id.as_str())
    }

    /// Which of the functions with live frames (`active`, by id) block the
    /// swap. `assume_inlining` also counts the callers that may contain
    /// inlined copies.
    pub fn blockers<'a>(&self, active: &[&'a str], assume_inlining: bool) -> Vec<&'a str> {
        active
            .iter()
            .copied()
            .filter(|id| {
                self.contains(id)
                    || (assume_inlining && self.inlining_exposed.iter().any(|e| e == id))
            })
            .collect()
    }
}

/// The layout-affected types: `changed` and every type that mentions one,
/// transitively.
pub fn affected_types(facts: &Facts, changed: &BTreeSet<String>) -> BTreeSet<String> {
    let mut affected = changed.clone();
    loop {
        let before = affected.len();
        for ty in &facts.types {
            if !affected.contains(&ty.name) && ty.mentions.iter().any(|m| affected.contains(m)) {
                affected.insert(ty.name.clone());
            }
        }
        if affected.len() == before {
            return affected;
        }
    }
}

/// Records that `id` is affected for `reason`; true when that is new.
fn add(id: &str, reason: Reason, reasons: &mut BTreeMap<String, BTreeSet<Reason>>) -> bool {
    reasons.entry(id.to_string()).or_default().insert(reason)
}

/// Computes the affected set of an update to the program `facts` describes:
/// `changed_functions` are the paths the intent names (except added ones) and
/// `changed_types` the types whose layout differs between the versions.
pub fn compute(
    facts: &Facts,
    changed_functions: &BTreeSet<String>,
    changed_types: &BTreeSet<String>,
) -> AffectedSet {
    let types = affected_types(facts, changed_types);
    let mut reasons: BTreeMap<String, BTreeSet<Reason>> = BTreeMap::new();

    for node in &facts.functions {
        if changed_functions.contains(&node.function) {
            add(&node.id, Reason::Changed, &mut reasons);
        }
        for ty in node.touches.iter().filter(|t| types.contains(*t)) {
            add(&node.id, Reason::Touches { ty: ty.clone() }, &mut reasons);
        }
        for ty in node.mentions.iter().filter(|t| types.contains(*t)) {
            add(&node.id, Reason::Holds { ty: ty.clone() }, &mut reasons);
        }
    }

    // A closure body and its creator go together, whichever started it.
    loop {
        let mut grew = false;
        for node in &facts.functions {
            if node.kind != NodeKind::Closure {
                continue;
            }
            let Some(parent) = &node.parent else { continue };
            if reasons.contains_key(&node.id) {
                grew |= add(
                    parent,
                    Reason::Creates {
                        closure: node.id.clone(),
                    },
                    &mut reasons,
                );
            }
            if reasons.contains_key(parent) {
                grew |= add(
                    &node.id,
                    Reason::ClosureOf {
                        creator: parent.clone(),
                    },
                    &mut reasons,
                );
            }
        }
        if !grew {
            break;
        }
    }

    // Callers that may hold inlined copies.
    let value_targets: BTreeSet<&str> = facts
        .functions
        .iter()
        .flat_map(|f| f.value_targets.iter().map(String::as_str))
        .collect();
    let mut callers: BTreeMap<&str, BTreeSet<&str>> = BTreeMap::new();
    for node in &facts.functions {
        for callee in &node.calls {
            callers.entry(callee).or_default().insert(&node.id);
        }
        if node.indirect_calls {
            for target in &value_targets {
                callers.entry(target).or_default().insert(&node.id);
            }
        }
    }
    let mut exposed: BTreeSet<String> = BTreeSet::new();
    let mut work: Vec<&str> = reasons.keys().map(String::as_str).collect();
    while let Some(id) = work.pop() {
        for caller in callers.get(id).into_iter().flatten() {
            if !reasons.contains_key(*caller) && exposed.insert((*caller).to_string()) {
                work.push(caller);
            }
        }
    }

    AffectedSet {
        functions: reasons
            .into_iter()
            .map(|(id, set)| AffectedNode {
                id,
                reasons: set.into_iter().collect(),
            })
            .collect(),
        inlining_exposed: exposed.into_iter().collect(),
        affected_types: types.into_iter().collect(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::facts::{FnFacts, TypeFacts};

    fn node(id: &str, kind: NodeKind) -> FnFacts {
        FnFacts {
            id: id.to_string(),
            kind,
            function: id.split(['[', '#']).next().unwrap().to_string(),
            parent: None,
            calls: vec![],
            indirect_calls: false,
            value_targets: vec![],
            touches: vec![],
            mentions: vec![],
        }
    }

    fn set(items: &[&str]) -> BTreeSet<String> {
        items.iter().map(ToString::to_string).collect()
    }

    #[test]
    fn a_reader_of_a_retyped_field_is_in_the_set_by_every_route() {
        // main -> middle -> peek (inlined reader of Counter.n);
        // viaclosure creates a closure that reads Counter.n;
        // generic `get[Counter]` reads it through an instance.
        let mut main = node("main", NodeKind::Function);
        main.calls = vec!["middle".into(), "viaclosure".into(), "get[Counter]".into()];
        let mut middle = node("middle", NodeKind::Function);
        middle.calls = vec!["peek".into()];
        let mut peek = node("peek", NodeKind::Function);
        peek.touches = vec!["Counter".into()];
        peek.mentions = vec!["Counter".into()];
        let mut viaclosure = node("viaclosure", NodeKind::Function);
        viaclosure.value_targets = vec!["viaclosure#closure0".into()];
        let mut closure = node("viaclosure#closure0", NodeKind::Closure);
        closure.parent = Some("viaclosure".into());
        closure.touches = vec!["Counter".into()];
        let mut get = node("get[Counter]", NodeKind::Instance);
        get.mentions = vec!["Counter".into()];
        let unrelated = node("unrelated", NodeKind::Function);
        let facts = Facts {
            functions: vec![main, middle, peek, viaclosure, closure, get, unrelated],
            types: vec![
                TypeFacts {
                    name: "Counter".into(),
                    mentions: vec![],
                },
                TypeFacts {
                    name: "Holder".into(),
                    mentions: vec!["Counter".into()],
                },
            ],
        };
        let affected = compute(&facts, &set(&[]), &set(&["Counter"]));
        for id in ["peek", "viaclosure#closure0", "viaclosure", "get[Counter]"] {
            assert!(affected.contains(id), "{id} must be affected");
        }
        assert!(!affected.contains("unrelated"));
        assert!(!affected.contains("main"), "a mere caller is not affected");
        assert!(!affected.contains("middle"));
        // ... but the callers of the reader may hold it inlined.
        assert!(affected.inlining_exposed.contains(&"middle".to_string()));
        assert!(affected.inlining_exposed.contains(&"main".to_string()));
        assert!(!affected.inlining_exposed.contains(&"unrelated".to_string()));
        assert_eq!(affected.affected_types, ["Counter", "Holder"]);
        assert_eq!(
            affected.blockers(&["main", "middle", "unrelated"], false),
            Vec::<&str>::new()
        );
        assert_eq!(
            affected.blockers(&["main", "peek", "unrelated"], false),
            ["peek"]
        );
        assert_eq!(
            affected.blockers(&["main", "unrelated"], true),
            ["main"],
            "with inlining assumed, main may run peek's old code"
        );
    }

    #[test]
    fn a_changed_function_brings_its_closures_and_creator_with_it() {
        let mut outer = node("outer", NodeKind::Function);
        outer.value_targets = vec!["outer#closure0".into()];
        let mut closure = node("outer#closure0", NodeKind::Closure);
        closure.parent = Some("outer".into());
        let facts = Facts {
            functions: vec![outer, closure, node("other", NodeKind::Function)],
            types: vec![],
        };
        let affected = compute(&facts, &set(&["outer"]), &set(&[]));
        assert!(affected.contains("outer"));
        assert!(affected.contains("outer#closure0"));
        assert!(!affected.contains("other"));
        let found = affected
            .functions
            .iter()
            .find(|f| f.id == "outer#closure0")
            .unwrap();
        assert!(found.reasons.contains(&Reason::ClosureOf {
            creator: "outer".into()
        }));
        // A closure that is the only reader pulls its creator in.
        let creator = node("f", NodeKind::Function);
        let mut closure = node("f#closure0", NodeKind::Closure);
        closure.parent = Some("f".into());
        closure.touches = vec!["T".into()];
        let facts = Facts {
            functions: vec![creator, closure],
            types: vec![],
        };
        let affected = compute(&facts, &set(&[]), &set(&["T"]));
        assert!(affected.contains("f"));
        assert!(affected.contains("f#closure0"));
    }

    #[test]
    fn an_indirect_call_reaches_every_value_target() {
        let mut caller = node("caller", NodeKind::Function);
        caller.indirect_calls = true;
        let mut maker = node("maker", NodeKind::Function);
        maker.value_targets = vec!["target".into()];
        let mut target = node("target", NodeKind::Function);
        target.touches = vec!["T".into()];
        let facts = Facts {
            functions: vec![caller, maker, target],
            types: vec![],
        };
        let affected = compute(&facts, &set(&[]), &set(&["T"]));
        assert!(affected.contains("target"));
        assert!(!affected.contains("caller"));
        assert!(affected.inlining_exposed.contains(&"caller".to_string()));
    }
}

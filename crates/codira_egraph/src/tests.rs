//! Copyright (c) 2026 Omnira CJSC. All Rights Reserved.
//! Author: Tunjay Akbarli
//! Date: September 15, 2026
//!
//! Tests for the equality-saturation optimizer.
//!
//! Every test that produces a body checks two things: that the output is
//! structurally valid (`verify_body`), and -- where the body is a pure
//! expression -- that it computes the *same value* as the input on
//! concrete arguments. The latter uses the tiny evaluator below rather
//! than `codira_comptime`, deliberately: an optimizer's test suite should
//! not be able to pass because the interpreter it is checked against
//! shares the same bug.

use codira_mir::{fold_op, print_body, verify_body, Attr, Body, OpId, OpKind, Region, TypeId};
use rustc_hash::FxHashMap;
use smallvec::SmallVec;

use crate::{
    egraph::{EClassId, EGraph, ENode, NodeKind},
    optimize, CostTable, EGraphOptimizer, Extraction, TypeEnv,
};

// ---- a minimal independent evaluator ------------------------------------

/// Evaluates a pure body under concrete `core.arg` values, via `fold_op`
/// (the IR's own semantics reference). Returns `None` for anything
/// outside the pure fragment.
fn eval(body: &Body, args: &[Attr]) -> Option<Attr> {
    let mut values: FxHashMap<OpId, Attr> = FxHashMap::default();
    for (id, op) in body.iter() {
        let value = match &op.kind {
            OpKind::Const(attr) => attr.clone(),
            OpKind::Arg(i) => args.get(*i as usize)?.clone(),
            OpKind::Tuple => continue, // only used as a result wrapper
            OpKind::TupleGet(_) => {
                let inner = op.operands.first()?;
                // The extractor's result wrapper: tuple_get(0)(tuple(v)).
                let tuple_op = body.get(*inner);
                values.get(tuple_op.operands.first()?)?.clone()
            }
            kind => {
                let operands: Option<Vec<Attr>> =
                    op.operands.iter().map(|o| values.get(o).cloned()).collect();
                fold_op(kind, &operands?).ok()?
            }
        };
        values.insert(id, value);
    }
    values.get(&body.result()?).cloned()
}

/// Optimizes with the given argument indices declared integer-typed --
/// the way a frontend that knows its parameter types would call this.
fn optimize_ints(body: &Body, int_args: impl IntoIterator<Item = u32>) -> Body {
    EGraphOptimizer::default()
        .optimize_body_with(body, &TypeEnv::with_int_args(int_args))
        .0
}

/// Asserts the optimized body is valid and agrees with the original on a
/// spread of argument values.
fn check_equivalent(before: &Body, after: &Body, arity: usize) {
    verify_body(after).expect("optimizer output must be structurally valid");
    let samples: [i64; 7] = [0, 1, -1, 2, 7, -13, 1000];
    for &a in &samples {
        for &b in &samples {
            let args = match arity {
                0 => vec![],
                1 => vec![Attr::Int(a)],
                _ => vec![Attr::Int(a), Attr::Int(b)],
            };
            let lhs = eval(before, &args);
            let rhs = eval(after, &args);
            assert_eq!(lhs, rhs, "disagreement on args {args:?}");
            if arity < 2 {
                break;
            }
        }
    }
}

fn arg(body: &mut Body, i: u32) -> OpId {
    body.push(OpKind::Arg(i), [])
}

fn int(body: &mut Body, v: i64) -> OpId {
    body.push(OpKind::Const(Attr::Int(v)), [])
}

// ---- e-graph core -------------------------------------------------------

fn leaf(graph: &mut EGraph, name: &str) -> EClassId {
    graph.add(ENode {
        kind: NodeKind::Pure(OpKind::ParamRef(name.into())),
        children: SmallVec::default(),
        ty: TypeId::UNTYPED,
    })
}

#[test]
fn union_find_merges_and_finds() {
    let mut graph = EGraph::new();
    let a = leaf(&mut graph, "a");
    let b = leaf(&mut graph, "b");
    assert_ne!(graph.find(a), graph.find(b));
    assert!(graph.union(a, b));
    assert_eq!(graph.find(a), graph.find(b));
    // Unioning again is a no-op.
    assert!(!graph.union(a, b));
}

#[test]
fn hashconsing_shares_identical_nodes() {
    let mut graph = EGraph::new();
    let a = leaf(&mut graph, "a");
    let one = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Neg),
        children: [a].into_iter().collect(),
        ty: TypeId::UNTYPED,
    });
    let two = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Neg),
        children: [a].into_iter().collect(),
        ty: TypeId::UNTYPED,
    });
    assert_eq!(one, two, "identical nodes must hashcons to one class");
}

#[test]
fn congruence_rebuild_merges_upward() {
    // The defining property of an e-graph: given f(a) and f(b), unioning
    // a with b must make f(a) and f(b) equal too.
    let mut graph = EGraph::new();
    let a = leaf(&mut graph, "a");
    let b = leaf(&mut graph, "b");
    let fa = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Neg),
        children: [a].into_iter().collect(),
        ty: TypeId::UNTYPED,
    });
    let fb = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Neg),
        children: [b].into_iter().collect(),
        ty: TypeId::UNTYPED,
    });
    assert_ne!(graph.find(fa), graph.find(fb));

    graph.union(a, b);
    graph.rebuild();

    assert_eq!(
        graph.find(fa),
        graph.find(fb),
        "congruence: a == b implies f(a) == f(b)"
    );
}

#[test]
fn constant_analysis_folds_bottom_up() {
    let mut graph = EGraph::new();
    let two = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Const(Attr::Int(2))),
        children: SmallVec::default(),
        ty: TypeId::UNTYPED,
    });
    let three = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Const(Attr::Int(3))),
        children: SmallVec::default(),
        ty: TypeId::UNTYPED,
    });
    let sum = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Add),
        children: [two, three].into_iter().collect(),
        ty: TypeId::UNTYPED,
    });
    assert_eq!(
        graph.analysis(sum).and_then(|a| a.constant.clone()),
        Some(Attr::Int(5))
    );
}

#[test]
fn division_by_zero_is_not_folded_away() {
    // The program is supposed to fail there; folding it to a constant
    // would erase the failure.
    let mut graph = EGraph::new();
    let one = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Const(Attr::Int(1))),
        children: SmallVec::default(),
        ty: TypeId::UNTYPED,
    });
    let zero = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Const(Attr::Int(0))),
        children: SmallVec::default(),
        ty: TypeId::UNTYPED,
    });
    let div = graph.add(ENode {
        kind: NodeKind::Pure(OpKind::Div),
        children: [one, zero].into_iter().collect(),
        ty: TypeId::UNTYPED,
    });
    assert_eq!(graph.analysis(div).and_then(|a| a.constant.clone()), None);
}

// ---- rewrite rules ------------------------------------------------------

#[test]
fn ieee_exact_identities_collapse_on_untyped_args() {
    // ((a * 1) / 1) - 0  ==>  a, even though `a` has unknown type:
    // all three identities are exact for IEEE floats as well as
    // integers, so no gate applies. See `rules`' module doc.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let one = int(&mut body, 1);
    let scaled = body.push(OpKind::Mul, [a, one]);
    let one2 = int(&mut body, 1);
    let divided = body.push(OpKind::Div, [scaled, one2]);
    let zero = int(&mut body, 0);
    body.push(OpKind::Sub, [divided, zero]);

    let out = optimize(&body);
    check_equivalent(&body, &out, 1);
    assert_eq!(
        print_body(&out),
        "\
%0 = core.arg 0
"
    );
}

#[test]
fn add_zero_is_not_folded_on_an_untyped_arg() {
    // `x + 0 == x` is FALSE for `x = -0.0`, so with `a` of unknown type
    // the optimizer must leave it alone. This is the counterpart to the
    // test above: the two identities look symmetric and are not.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let zero = int(&mut body, 0);
    body.push(OpKind::Add, [a, zero]);

    let out = optimize(&body);
    check_equivalent(&body, &out, 1);
    assert!(
        print_body(&out).contains("core.add"),
        "x + 0 must survive on an unknown-typed operand, got:\n{}",
        print_body(&out)
    );
}

#[test]
fn add_zero_does_fold_on_a_provable_integer() {
    // The same rewrite, with the operand provably an integer (it is the
    // result of a bitwise op, which `fold_op` only accepts on ints).
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let mask = int(&mut body, 255);
    let masked = body.push(OpKind::BitAnd, [a, mask]);
    let zero = int(&mut body, 0);
    body.push(OpKind::Add, [masked, zero]);

    let out = optimize(&body);
    check_equivalent(&body, &out, 1);
    let text = print_body(&out);
    assert!(
        !text.contains("core.add"),
        "x + 0 must fold once x is provably an integer, got:\n{text}"
    );
    assert!(
        text.contains("core.bitand"),
        "the mask must survive:\n{text}"
    );
}

#[test]
fn strength_reduction_picks_shift_over_multiply() {
    // a * 8  ==>  a << 3   (multiply costs 3, shift costs 1)
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let eight = int(&mut body, 8);
    body.push(OpKind::Mul, [a, eight]);

    let out = optimize_ints(&body, [0]);
    check_equivalent(&body, &out, 1);
    assert!(
        print_body(&out).contains("core.shl"),
        "expected a shift, got:\n{}",
        print_body(&out)
    );
}

#[test]
fn factoring_wins_on_cost() {
    // a*b + a*c  ==>  a*(b + c):  two multiplies (3+3) plus an add (1) is
    // 7; one multiply plus one add is 4. The cost model, not the rule
    // order, is what decides this.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 1);
    let c = arg(&mut body, 2);
    let ab = body.push(OpKind::Mul, [a, b]);
    let ac = body.push(OpKind::Mul, [a, c]);
    body.push(OpKind::Add, [ab, ac]);

    let out = optimize_ints(&body, [0, 1, 2]);
    verify_body(&out).unwrap();
    let text = print_body(&out);
    let multiplies = text.matches("core.mul").count();
    assert_eq!(
        multiplies, 1,
        "factoring should leave one multiply, got:\n{text}"
    );
}

#[test]
fn constants_reassociate_through_commutativity() {
    // (2 + a) + 3  ==>  a + 5. Requires commutativity AND associativity
    // AND the constant analysis, cooperating -- the canonical example of
    // a rewrite no single pass ordering finds reliably.
    let mut body = Body::new();
    let two = int(&mut body, 2);
    let a = arg(&mut body, 0);
    let sum = body.push(OpKind::Add, [two, a]);
    let three = int(&mut body, 3);
    body.push(OpKind::Add, [sum, three]);

    let out = optimize_ints(&body, [0]);
    check_equivalent(&body, &out, 1);
    let text = print_body(&out);
    assert!(
        text.contains("core.const 5"),
        "expected the constants to combine into 5, got:\n{text}"
    );
    assert_eq!(
        text.matches("core.add").count(),
        1,
        "expected a single add, got:\n{text}"
    );
}

#[test]
fn xor_with_itself_is_zero() {
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 0);
    body.push(OpKind::BitXor, [a, b]);

    let out = optimize(&body);
    check_equivalent(&body, &out, 1);
    assert_eq!(
        print_body(&out),
        "\
%0 = core.const 0
"
    );
}

#[test]
fn comparison_mirroring_is_applied() {
    // a > b and b < a are the same; the cost model settles on one.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 1);
    body.push(OpKind::Gt, [a, b]);

    let out = optimize(&body);
    verify_body(&out).unwrap();
    let text = print_body(&out);
    assert!(
        text.contains("core.gt") || text.contains("core.lt"),
        "expected a comparison, got:\n{text}"
    );
}

// ---- soundness: the float gate ------------------------------------------

#[test]
fn float_arithmetic_is_not_reassociated() {
    // (arg + 1.5) + 2.5. `arg` has unknown type, so `definitely_int` is
    // false and associativity must NOT fire -- reassociating would let
    // the 1.5 and 2.5 combine, which is invalid for IEEE floats (the
    // rounding of (x+1.5)+2.5 differs from x+4.0 in general).
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let p15 = body.push(OpKind::Const(Attr::float(1.5)), []);
    let sum = body.push(OpKind::Add, [a, p15]);
    let p25 = body.push(OpKind::Const(Attr::float(2.5)), []);
    body.push(OpKind::Add, [sum, p25]);

    let out = optimize(&body);
    verify_body(&out).unwrap();
    let text = print_body(&out);
    assert_eq!(
        text.matches("core.add").count(),
        2,
        "float adds must not be reassociated into one, got:\n{text}"
    );
    assert!(
        !text.contains("4.0"),
        "1.5 and 2.5 must not be combined across an unknown-typed operand:\n{text}"
    );
}

#[test]
fn unknown_typed_multiply_by_zero_is_preserved() {
    // arg * 0 is NOT 0 when arg could be a float NaN or infinity. The
    // gate must keep the multiply.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let zero = body.push(OpKind::Const(Attr::float(0.0)), []);
    body.push(OpKind::Mul, [a, zero]);

    let out = optimize(&body);
    verify_body(&out).unwrap();
    assert!(
        print_body(&out).contains("core.mul"),
        "must not fold arg*0.0 to 0, got:\n{}",
        print_body(&out)
    );
}

#[test]
fn integer_multiply_by_zero_does_fold() {
    // The same shape, but the operand is provably an integer (it is the
    // result of a bitwise op), so the rule is sound and must fire.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let mask = int(&mut body, 255);
    let masked = body.push(OpKind::BitAnd, [a, mask]);
    let zero = int(&mut body, 0);
    body.push(OpKind::Mul, [masked, zero]);

    let out = optimize(&body);
    check_equivalent(&body, &out, 1);
    assert_eq!(
        print_body(&out),
        "\
%0 = core.const 0
"
    );
}

// ---- opaque ops ---------------------------------------------------------

#[test]
fn opaque_region_op_survives_with_inner_body_optimized() {
    // `if c { x + 0 } else { x }` -- the If must survive (its identity is
    // opaque), but its branch bodies are optimized recursively.
    let mut then_body = Body::new();
    let x = then_body.push(OpKind::Arg(1), []);
    let z = then_body.push(OpKind::Const(Attr::Int(0)), []);
    then_body.push(OpKind::Add, [x, z]);

    let mut else_body = Body::new();
    else_body.push(OpKind::Arg(1), []);

    let mut body = Body::new();
    let c = arg(&mut body, 0);
    body.push_with_regions(
        OpKind::If,
        [c],
        [Region::new(then_body), Region::new(else_body)],
    );

    let out = optimize_ints(&body, [1]);
    verify_body(&out).unwrap();
    let text = print_body(&out);
    assert!(text.contains("cf.if"), "the If must survive:\n{text}");
    assert!(
        !text.contains("core.add"),
        "the inner `x + 0` should have folded away:\n{text}"
    );
}

#[test]
fn two_distinct_calls_never_merge() {
    // Opaque nodes carry a unique serial precisely so that two calls with
    // identical operands are not assumed equal.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let f = body.push(OpKind::Call("f".into()), [a]);
    let g = body.push(OpKind::Call("g".into()), [a]);
    body.push(OpKind::Add, [f, g]);

    let out = optimize(&body);
    verify_body(&out).unwrap();
    let text = print_body(&out);
    assert!(text.contains("@f"), "call to f must survive:\n{text}");
    assert!(text.contains("@g"), "call to g must survive:\n{text}");
}

// ---- extraction ---------------------------------------------------------

#[test]
fn shared_subexpression_is_emitted_once() {
    // (a*b) + (a*b): extraction produces a DAG, so the product appears
    // once, not twice.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 1);
    let p1 = body.push(OpKind::Mul, [a, b]);
    let p2 = body.push(OpKind::Mul, [a, b]);
    body.push(OpKind::Add, [p1, p2]);

    let out = optimize(&body);
    verify_body(&out).unwrap();
    let text = print_body(&out);
    assert_eq!(
        text.matches("core.mul").count(),
        1,
        "the shared product must be emitted once, got:\n{text}"
    );
}

#[test]
fn empty_body_round_trips() {
    let body = Body::new();
    let (out, report) = EGraphOptimizer::default().optimize_body(&body);
    assert!(out.is_empty());
    assert!(report.saturated);
}

#[test]
fn tight_limits_still_produce_valid_output() {
    // A budget too small to saturate must still yield a correct program --
    // just possibly a less optimized one.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 1);
    let ab = body.push(OpKind::Mul, [a, b]);
    let sum = body.push(OpKind::Add, [ab, a]);
    let zero = int(&mut body, 0);
    body.push(OpKind::Add, [sum, zero]);

    let opt = EGraphOptimizer {
        max_iterations: 1,
        max_nodes: 8,
        ..EGraphOptimizer::default()
    };
    let (out, report) = opt.optimize_body(&body);
    verify_body(&out).unwrap();
    check_equivalent(&body, &out, 2);
    assert_eq!(report.iterations, 1);
}

#[test]
fn saturation_reports_a_genuine_fixpoint() {
    // A body with nothing to rewrite must reach saturation immediately
    // rather than burning the whole iteration budget.
    let mut body = Body::new();
    arg(&mut body, 0);

    let (_, report) = EGraphOptimizer::default().optimize_body(&body);
    assert!(report.saturated, "a trivial body must saturate");
    assert!(report.iterations <= 2, "and do so promptly");
}

#[test]
fn type_env_unlocks_gated_rules() {
    // The same body, optimized twice: without type information the
    // float-unsound rules stay off; declaring the argument integer turns
    // them on. This is the channel a frontend that knows its parameter
    // types uses -- see `TypeEnv`.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 0);
    body.push(OpKind::Sub, [a, b]); // x - x

    let conservative = optimize(&body);
    verify_body(&conservative).unwrap();
    assert!(
        print_body(&conservative).contains("core.sub"),
        "x - x is NaN for float x, so it must survive without type info:\n{}",
        print_body(&conservative)
    );

    let informed = optimize_ints(&body, [0]);
    check_equivalent(&body, &informed, 1);
    assert_eq!(
        print_body(&informed),
        "\
%0 = core.const 0
",
        "with x known integer, x - x folds to 0"
    );
}

// ---- bounds: the node cap is hard -----------------------------------------

/// `a0*a1 + a1*a2 + ... + a(n-1)*a0` with every argument an integer:
/// commutativity, associativity, factoring and expansion all fire, which
/// is the shape that grows an e-graph fastest.
fn sum_of_products(terms: usize) -> Body {
    let mut body = Body::new();
    let args: Vec<OpId> = (0..terms as u32).map(|i| arg(&mut body, i)).collect();
    let mut acc = body.push(OpKind::Mul, [args[0], args[1 % terms]]);
    for i in 1..terms {
        let p = body.push(OpKind::Mul, [args[i], args[(i + 1) % terms]]);
        acc = body.push(OpKind::Add, [acc, p]);
    }
    body
}

#[test]
fn node_cap_is_never_exceeded() {
    // Before the cap was checked per rewrite, a round ran to completion
    // and then compared: on these inputs a 10,000-node cap ended at up to
    // 14,545 nodes and a 1,000-node cap at up to 1,423.
    for terms in [8usize, 16, 48] {
        let body = sum_of_products(terms);
        let env = TypeEnv::with_int_args(0..terms as u32);
        // The body's own graph: what is there before any rewrite.
        let own = EGraphOptimizer {
            max_iterations: 0,
            ..EGraphOptimizer::default()
        }
        .optimize_body_with(&body, &env)
        .1
        .nodes;
        for cap in [100usize, 1_000, 10_000] {
            let opt = EGraphOptimizer {
                max_iterations: 30,
                max_nodes: cap,
                ..EGraphOptimizer::default()
            };
            let (out, report) = opt.optimize_body_with(&body, &env);
            if own >= cap {
                // Nothing may be added to a graph that is already full.
                assert_eq!(report.iterations, 0);
                assert_eq!(report.nodes, own);
            } else {
                assert!(
                    report.nodes <= cap,
                    "{terms} terms, cap {cap}: saturation left {} nodes",
                    report.nodes
                );
            }
            verify_body(&out).unwrap();
        }
    }
}

#[test]
fn a_body_that_fills_the_cap_is_not_rewritten() {
    // The body's own graph is 11 nodes; with a cap of 10 no rewrite may
    // run at all, and the output is still the same program.
    let body = sum_of_products(4);
    let opt = EGraphOptimizer {
        max_nodes: 10,
        ..EGraphOptimizer::default()
    };
    let (out, report) = opt.optimize_body_with(&body, &TypeEnv::with_int_args(0..4));
    assert_eq!(report.iterations, 0);
    assert!(!report.saturated);
    assert!(report.nodes > 10);
    verify_body(&out).unwrap();
    assert_eq!(
        print_body(&out).matches("core.mul").count(),
        4,
        "{}",
        print_body(&out)
    );
}

#[test]
fn capped_saturation_is_deterministic() {
    let body = sum_of_products(16);
    let opt = EGraphOptimizer {
        max_iterations: 30,
        max_nodes: 2_000,
        ..EGraphOptimizer::default()
    };
    let env = TypeEnv::with_int_args(0..16);
    let (first, first_report) = opt.optimize_body_with(&body, &env);
    for _ in 0..3 {
        let (again, report) = opt.optimize_body_with(&body, &env);
        assert_eq!(report, first_report);
        assert_eq!(print_body(&again), print_body(&first));
    }
}

// ---- extraction: exact search and the cost table --------------------------

/// A graph where the greedy (tree-cost) choice is not the cheapest DAG:
///
/// ```text
/// root = tuple(A, B)
/// A    = { neg(P), neg(Q) }      P = x * y  (tree cost 5)
/// B    = not(P)                  Q = x + y  (tree cost 3)
/// ```
///
/// Greedy picks `neg(Q)` for A because it is cheaper *in isolation*, and
/// then pays for both P and Q. The exact extractor sees that B needs P
/// anyway and shares it.
fn sharing_graph() -> (EGraph, EClassId) {
    let mut graph = EGraph::new();
    let x = leaf(&mut graph, "x");
    let y = leaf(&mut graph, "y");
    let node = |kind: OpKind, children: &[EClassId]| ENode {
        kind: NodeKind::Pure(kind),
        children: children.iter().copied().collect(),
        ty: TypeId::UNTYPED,
    };
    let p = graph.add(node(OpKind::Mul, &[x, y]));
    let q = graph.add(node(OpKind::Add, &[x, y]));
    let a1 = graph.add(node(OpKind::Neg, &[p]));
    let a2 = graph.add(node(OpKind::Neg, &[q]));
    graph.union(a1, a2);
    graph.rebuild();
    let b = graph.add(node(OpKind::Not, &[p]));
    let root = graph.add(node(OpKind::Tuple, &[a1, b]));
    graph.rebuild();
    (graph, root)
}

#[test]
fn exact_extraction_shares_what_greedy_duplicates() {
    let (graph, root) = sharing_graph();

    let (greedy, how) = crate::extract::extract(&graph, root, &CostTable::GENERIC, 0);
    assert_eq!(how, Extraction::Greedy);
    let greedy_text = print_body(&greedy);
    assert!(greedy_text.contains("core.add"), "{greedy_text}");

    let (exact, how) = crate::extract::extract(&graph, root, &CostTable::GENERIC, 10_000);
    assert_eq!(how, Extraction::Exact { improved: true });
    let exact_text = print_body(&exact);
    assert!(
        !exact_text.contains("core.add"),
        "the shared product should serve both uses:\n{exact_text}"
    );
    assert_eq!(exact_text.matches("core.mul").count(), 1, "{exact_text}");
    verify_body(&exact).unwrap();
}

#[test]
fn an_exhausted_budget_falls_back_to_greedy_deterministically() {
    let (graph, root) = sharing_graph();
    let (greedy, _) = crate::extract::extract(&graph, root, &CostTable::GENERIC, 0);
    // One step is not enough to complete a single program.
    for _ in 0..3 {
        let (out, how) = crate::extract::extract(&graph, root, &CostTable::GENERIC, 1);
        assert_eq!(how, Extraction::BudgetExhausted { improved: false });
        assert_eq!(print_body(&out), print_body(&greedy));
    }
}

#[test]
fn exact_extraction_never_changes_meaning() {
    // The exact extractor is on by default; greedy-only is a zero budget.
    // Both must produce valid programs that compute what the input does.
    for terms in [3usize, 4, 6] {
        let body = sum_of_products(terms);
        let env = TypeEnv::with_int_args(0..terms as u32);
        let exact = EGraphOptimizer::default().optimize_body_with(&body, &env);
        let greedy = EGraphOptimizer {
            exact_extraction_budget: 0,
            ..EGraphOptimizer::default()
        }
        .optimize_body_with(&body, &env);
        assert_eq!(greedy.1.extraction, Extraction::Greedy);
        assert!(matches!(
            exact.1.extraction,
            Extraction::Exact { .. } | Extraction::BudgetExhausted { .. }
        ));
        verify_body(&exact.0).unwrap();
        verify_body(&greedy.0).unwrap();
    }
    // A two-argument shape with shared structure, checked on values.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let b = arg(&mut body, 1);
    let ab = body.push(OpKind::Mul, [a, b]);
    let ba = body.push(OpKind::Mul, [b, a]);
    let s = body.push(OpKind::Add, [ab, a]);
    let t = body.push(OpKind::Add, [ba, b]);
    body.push(OpKind::Sub, [s, t]);
    let out = EGraphOptimizer::default()
        .optimize_body_with(&body, &TypeEnv::with_int_args([0, 1]))
        .0;
    check_equivalent(&body, &out, 2);
}

#[test]
fn the_cost_table_decides_what_is_extracted() {
    // `a * 8` and `a << 3` are equal; which one comes out is the cost
    // table's call. With the generic table the shift wins (1 < 3); where
    // shifts are dear, the multiply does.
    let mut body = Body::new();
    let a = arg(&mut body, 0);
    let eight = int(&mut body, 8);
    body.push(OpKind::Mul, [a, eight]);
    let env = TypeEnv::with_int_args([0]);

    let generic = EGraphOptimizer::default().optimize_body_with(&body, &env).0;
    assert!(print_body(&generic).contains("core.shl"));

    let dear_shifts = EGraphOptimizer {
        costs: CostTable {
            name: "dear-shifts",
            shift: 10,
            ..CostTable::GENERIC
        },
        ..EGraphOptimizer::default()
    };
    let out = dear_shifts.optimize_body_with(&body, &env).0;
    check_equivalent(&body, &out, 1);
    let text = print_body(&out);
    assert!(
        text.contains("core.mul") && !text.contains("core.shl"),
        "{text}"
    );
}

// ---- memory (codira_mir::mem) --------------------------------------------

mod memory {
    use codira_mir::{print_body, verify_body, Attr, Body, MemOp, OpId, OpKind, TypeId};

    use crate::{EGraphOptimizer, TypeEnv};

    fn int(body: &mut Body, v: i64) -> OpId {
        body.push_typed(OpKind::Const(Attr::Int(v)), [], TypeId::I64)
    }

    fn mem(body: &mut Body, op: MemOp, operands: &[OpId], ty: TypeId) -> OpId {
        body.push_typed(OpKind::Mem(op), operands.iter().copied(), ty)
    }

    fn slot(body: &mut Body, state: OpId, site: u32) -> (OpId, OpId) {
        let m = mem(
            body,
            MemOp::Alloc {
                site,
                elem: TypeId::I64,
                count: 1,
            },
            &[state],
            TypeId::MEM,
        );
        let p = mem(body, MemOp::Addr, &[m], TypeId::PTR);
        (m, p)
    }

    fn optimize(body: &Body) -> Body {
        verify_body(body).expect("test input must verify");
        let out = EGraphOptimizer::default()
            .optimize_body_with(body, &TypeEnv::unknown())
            .0;
        verify_body(&out).expect("optimizer output must verify");
        out
    }

    #[test]
    fn a_stored_value_is_forwarded_to_the_load() {
        let mut body = Body::new();
        let x = body.push_typed(OpKind::Arg(0), [], TypeId::I64);
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let (m1, p) = slot(&mut body, m0, 0);
        let m2 = mem(&mut body, MemOp::Store, &[m1, p, x], TypeId::MEM);
        mem(&mut body, MemOp::Load, &[m2, p], TypeId::I64);
        let text = print_body(&optimize(&body));
        assert!(!text.contains("mem.load"), "{text}");
        assert!(text.contains("core.arg 0"), "{text}");
    }

    #[test]
    fn a_load_bypasses_a_store_to_another_allocation() {
        // *p = 5; *q = 9; return *p   ==>   5
        let mut body = Body::new();
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let (m1, p) = slot(&mut body, m0, 0);
        let (m2, q) = slot(&mut body, m1, 1);
        let five = int(&mut body, 5);
        let nine = int(&mut body, 9);
        let m3 = mem(&mut body, MemOp::Store, &[m2, p, five], TypeId::MEM);
        let m4 = mem(&mut body, MemOp::Store, &[m3, q, nine], TypeId::MEM);
        mem(&mut body, MemOp::Load, &[m4, p], TypeId::I64);
        let text = print_body(&optimize(&body));
        assert_eq!(text, "%0 = core.const 5\n", "{text}");
    }

    #[test]
    fn an_address_of_unknown_origin_is_not_assumed_disjoint() {
        // *a = 5; *b = 9; return *a -- a and b are arguments and may be
        // the same address, so the load must stay.
        let mut body = Body::new();
        let a = body.push_typed(OpKind::Arg(0), [], TypeId::PTR);
        let b = body.push_typed(OpKind::Arg(1), [], TypeId::PTR);
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let five = int(&mut body, 5);
        let nine = int(&mut body, 9);
        let m1 = mem(&mut body, MemOp::Store, &[m0, a, five], TypeId::MEM);
        let m2 = mem(&mut body, MemOp::Store, &[m1, b, nine], TypeId::MEM);
        let v = mem(&mut body, MemOp::Load, &[m2, a], TypeId::I64);
        body.push(OpKind::Tuple, [m2, v]);
        let text = print_body(&optimize(&body));
        assert!(text.contains("mem.load"), "{text}");
        assert_eq!(text.matches("mem.store").count(), 2, "{text}");
    }

    #[test]
    fn an_overwritten_store_is_dropped() {
        // *a = 1; *a = 2; return memory  ==>  one store
        let mut body = Body::new();
        let a = body.push_typed(OpKind::Arg(0), [], TypeId::PTR);
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let one = int(&mut body, 1);
        let two = int(&mut body, 2);
        let m1 = mem(&mut body, MemOp::Store, &[m0, a, one], TypeId::MEM);
        let m2 = mem(&mut body, MemOp::Store, &[m1, a, two], TypeId::MEM);
        let zero = int(&mut body, 0);
        body.push(OpKind::Tuple, [m2, zero]);
        let text = print_body(&optimize(&body));
        assert_eq!(text.matches("mem.store").count(), 1, "{text}");
        assert!(text.contains("core.const 2"), "{text}");
    }

    #[test]
    fn a_differently_typed_load_is_not_forwarded() {
        // Store an i64, load an i32 from the same address: not the same value.
        let mut body = Body::new();
        let a = body.push_typed(OpKind::Arg(0), [], TypeId::PTR);
        let x = body.push_typed(OpKind::Arg(1), [], TypeId::I64);
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let m1 = mem(&mut body, MemOp::Store, &[m0, a, x], TypeId::MEM);
        let narrow = mem(&mut body, MemOp::Load, &[m1, a], TypeId::I32);
        body.push(OpKind::Tuple, [m1, narrow]);
        let text = print_body(&optimize(&body));
        assert!(text.contains("mem.load"), "{text}");
    }

    #[test]
    fn a_reread_of_an_overwritten_state_keeps_the_input_order_valid() {
        // m1 = *a = 1; v = *a (in m1); m2 = *a = 2; return (m2, v).
        // Forwarding makes v = 1; the dead-store rule must not leave a
        // branching chain behind. Whatever comes out has to verify, and v
        // must still be 1.
        let mut body = Body::new();
        let a = body.push_typed(OpKind::Arg(0), [], TypeId::PTR);
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let one = int(&mut body, 1);
        let two = int(&mut body, 2);
        let m1 = mem(&mut body, MemOp::Store, &[m0, a, one], TypeId::MEM);
        let v = mem(&mut body, MemOp::Load, &[m1, a], TypeId::I64);
        let m2 = mem(&mut body, MemOp::Store, &[m1, a, two], TypeId::MEM);
        body.push(OpKind::Tuple, [m2, v]);
        let out = optimize(&body);
        let text = print_body(&out);
        assert_eq!(text.matches("mem.store").count(), 1, "{text}");
        assert!(text.contains("core.const 1"), "{text}");
    }

    #[test]
    fn identical_loads_of_one_state_are_one_value() {
        let mut body = Body::new();
        let a = body.push_typed(OpKind::Arg(0), [], TypeId::PTR);
        let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
        let x = mem(&mut body, MemOp::Load, &[m0, a], TypeId::I64);
        let y = mem(&mut body, MemOp::Load, &[m0, a], TypeId::I64);
        body.push_typed(OpKind::Sub, [x, y], TypeId::I64);
        // x - x == 0 needs both to be integers; a typed load is one.
        let text = print_body(&optimize(&body));
        assert_eq!(text, "%0 = core.const 0\n", "{text}");
    }
}

// ---- rules as data (crate::pattern) ----------------------------------------

mod pattern_rules {
    use std::sync::Arc;

    use codira_mir::{print_body, verify_body, Body, OpKind};

    use crate::{EGraphOptimizer, PatternRule, TypeEnv};

    /// `(a ^ b) ^ b`, which no built-in rule simplifies.
    fn xor_twice() -> Body {
        let mut body = Body::new();
        let a = body.push(OpKind::Arg(0), []);
        let b = body.push(OpKind::Arg(1), []);
        let ab = body.push(OpKind::BitXor, [a, b]);
        body.push(OpKind::BitXor, [ab, b]);
        body
    }

    fn just_a() -> Body {
        let mut body = Body::new();
        body.push(OpKind::Arg(0), []);
        body
    }

    fn rules() -> Arc<[PatternRule]> {
        Arc::from(vec![PatternRule::from_bodies(
            "xor_cancel",
            &xor_twice(),
            &just_a(),
        )
        .unwrap()])
    }

    #[test]
    fn a_supplied_rule_is_applied() {
        let env = TypeEnv::with_int_args([0, 1]);
        let without = EGraphOptimizer::default()
            .optimize_body_with(&xor_twice(), &env)
            .0;
        assert!(print_body(&without).contains("core.bitxor"));

        let with = EGraphOptimizer::default()
            .optimize_body_with_rules(&xor_twice(), &env, &rules())
            .0;
        verify_body(&with).unwrap();
        assert_eq!(print_body(&with), "%0 = core.arg 0\n");
    }

    #[test]
    fn an_integer_rule_does_not_bind_a_value_of_unknown_type() {
        // Nothing says the arguments are integers here, so the rule (proven
        // over integers) must not fire.
        let out = EGraphOptimizer::default()
            .optimize_body_with_rules(&xor_twice(), &TypeEnv::unknown(), &rules())
            .0;
        assert!(print_body(&out).contains("core.bitxor"));
    }

    #[test]
    fn malformed_rules_are_refused() {
        let error = PatternRule::from_bodies("bare", &just_a(), &xor_twice()).unwrap_err();
        assert!(error.contains("must be an operation"), "{error}");

        let mut free = Body::new();
        free.push(OpKind::Arg(5), []);
        let error = PatternRule::from_bodies("free", &xor_twice(), &free).unwrap_err();
        assert!(error.contains("does not bind"), "{error}");
    }
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Tests for the Memory SSA verifier and scheduler.

use super::{schedule, verify_memory};
use crate::{
    check_types, print_body, verify_body, Attr, Body, MemOp, OpId, OpKind, Region, TypeId,
};

fn int(body: &mut Body, v: i64) -> OpId {
    body.push_typed(OpKind::Const(Attr::Int(v)), [], TypeId::I64)
}

fn mem(body: &mut Body, op: MemOp, operands: &[OpId], ty: TypeId) -> OpId {
    body.push_typed(OpKind::Mem(op), operands.iter().copied(), ty)
}

fn alloc_i64(site: u32) -> MemOp {
    MemOp::Alloc {
        site,
        elem: TypeId::I64,
        count: 1,
    }
}

/// `x = 7; *p = x; return (mem, *p)` over a fresh slot.
fn store_then_load() -> Body {
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let p = mem(&mut body, MemOp::Addr, &[m1], TypeId::PTR);
    let seven = int(&mut body, 7);
    let m2 = mem(&mut body, MemOp::Store, &[m1, p, seven], TypeId::MEM);
    let x = mem(&mut body, MemOp::Load, &[m2, p], TypeId::I64);
    body.push(OpKind::Tuple, [m2, x]);
    body
}

#[test]
fn a_memory_chain_verifies_and_prints() {
    let body = store_then_load();
    verify_body(&body).unwrap();
    check_types(&body).unwrap();
    assert_eq!(
        print_body(&body),
        "\
%0 = mem.entry
%1 = mem.alloc(site 0, i64 x 1) %0
%2 = mem.addr %1
%3 = core.const 7
%4 = mem.store %1, %2, %3
%5 = mem.load %4, %2
%6 = core.tuple %4, %5
"
    );
}

#[test]
fn a_state_consumed_twice_is_rejected() {
    // Two stores from one state: a tree of memory, which no single
    // memory can execute.
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let p = mem(&mut body, MemOp::Addr, &[m1], TypeId::PTR);
    let one = int(&mut body, 1);
    let two = int(&mut body, 2);
    let a = mem(&mut body, MemOp::Store, &[m1, p, one], TypeId::MEM);
    let b = mem(&mut body, MemOp::Store, &[m1, p, two], TypeId::MEM);
    body.push(OpKind::Tuple, [a, b]);
    let error = verify_body(&body).unwrap_err();
    assert!(error.rule.contains("linearity"), "{error}");
}

#[test]
fn a_load_after_the_consumer_breaks_the_schedule_and_is_reordered() {
    // m2 = store(m1, p, 1); m3 = store(m2, p, 2); x = load(m2, p)
    // In this order the load would see 2. The scheduler moves it before
    // the second store, where it sees 1.
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let p = mem(&mut body, MemOp::Addr, &[m1], TypeId::PTR);
    let one = int(&mut body, 1);
    let m2 = mem(&mut body, MemOp::Store, &[m1, p, one], TypeId::MEM);
    let two = int(&mut body, 2);
    let m3 = mem(&mut body, MemOp::Store, &[m2, p, two], TypeId::MEM);
    let x = mem(&mut body, MemOp::Load, &[m2, p], TypeId::I64);
    body.push(OpKind::Tuple, [m3, x]);

    let error = verify_body(&body).unwrap_err();
    assert!(error.rule.contains("schedule"), "{error}");

    let fixed = schedule(&body).expect("a valid order exists");
    verify_body(&fixed).unwrap();
    let text = print_body(&fixed);
    let load = text.find("mem.load").unwrap();
    let second_store = text.rfind("mem.store").unwrap();
    assert!(load < second_store, "{text}");
    assert!(text.trim_end().ends_with("core.tuple %7, %6"), "{text}");
}

#[test]
fn a_body_that_already_satisfies_the_schedule_is_unchanged() {
    let body = store_then_load();
    assert_eq!(print_body(&schedule(&body).unwrap()), print_body(&body));
}

#[test]
fn an_impossible_order_is_reported_not_guessed() {
    // The pointer for the load of m1 is itself loaded from m2, the state
    // after m1 was consumed: the load must come both before and after the
    // second store.
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let p = mem(&mut body, MemOp::Addr, &[m1], TypeId::PTR);
    let m2 = mem(&mut body, MemOp::Store, &[m1, p, p], TypeId::MEM);
    let q = mem(&mut body, MemOp::Load, &[m2, p], TypeId::PTR);
    let x = mem(&mut body, MemOp::Load, &[m1, q], TypeId::I64);
    body.push(OpKind::Tuple, [m2, x]);
    assert!(schedule(&body).is_none());
}

#[test]
fn memory_crosses_a_loop_as_a_carried_value() {
    // for i in 0..4 carry(m) { m' = store(m, p, i); yield m' }
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let start = int(&mut body, 0);
    let end = int(&mut body, 4);
    let step = int(&mut body, 1);

    // Regions cannot name the enclosing body's values, and `mem.addr`
    // must name an allocation in its own body, so the loop writes through
    // a pointer argument.
    let mut loop_body = Body::new();
    let iv = loop_body.push_typed(OpKind::BlockArg(0), [], TypeId::I64);
    let m_in = loop_body.push_typed(OpKind::BlockArg(1), [], TypeId::MEM);
    let ptr = loop_body.push_typed(OpKind::Arg(0), [], TypeId::PTR);
    let m_out = mem(&mut loop_body, MemOp::Store, &[m_in, ptr, iv], TypeId::MEM);
    loop_body.push(OpKind::Yield, [m_out]);

    let final_m = body.push_with_regions_typed(
        OpKind::For,
        [start, end, step, m1],
        [Region::with_args(2, loop_body)],
        TypeId::MEM,
    );
    let p = mem(&mut body, MemOp::Addr, &[m1], TypeId::PTR);
    let x = mem(&mut body, MemOp::Load, &[final_m, p], TypeId::I64);
    body.push(OpKind::Tuple, [final_m, x]);

    // `mem.addr %m1` after the loop consumed m1 violates the schedule;
    // the scheduler moves it before the loop.
    assert!(verify_body(&body).is_err());
    let fixed = schedule(&body).unwrap();
    verify_body(&fixed).unwrap();
    check_types(&fixed).unwrap();
}

#[test]
fn memory_in_an_if_region_is_not_supported_yet() {
    let mut then_body = Body::new();
    mem(&mut then_body, MemOp::Entry, &[], TypeId::MEM);
    let mut else_body = Body::new();
    int(&mut else_body, 0);
    let mut body = Body::new();
    let c = body.push_typed(OpKind::Const(Attr::Bool(true)), [], TypeId::BOOL);
    body.push_with_regions(
        OpKind::If,
        [c],
        [Region::new(then_body), Region::new(else_body)],
    );
    let error = verify_memory(&body, true).unwrap_err();
    assert!(error.rule.contains("cf.if"), "{error}");
}

#[test]
fn entry_is_only_valid_in_a_function_body() {
    let mut loop_body = Body::new();
    let m = mem(&mut loop_body, MemOp::Entry, &[], TypeId::MEM);
    loop_body.push(OpKind::Yield, [m]);
    let mut body = Body::new();
    let start = int(&mut body, 0);
    let end = int(&mut body, 1);
    let step = int(&mut body, 1);
    let init = int(&mut body, 0);
    body.push_with_regions(
        OpKind::For,
        [start, end, step, init],
        [Region::with_args(2, loop_body)],
    );
    let error = verify_body(&body).unwrap_err();
    assert!(error.rule.contains("mem.entry"), "{error}");
}

#[test]
fn addr_must_name_an_allocation_and_states_are_not_values() {
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    mem(&mut body, MemOp::Addr, &[m0], TypeId::PTR);
    assert!(verify_body(&body).unwrap_err().rule.contains("mem.alloc"));

    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let one = int(&mut body, 1);
    body.push(OpKind::Add, [m0, one]);
    assert!(verify_body(&body)
        .unwrap_err()
        .rule
        .contains("ordinary value"));
}

#[test]
fn memory_ops_are_type_checked() {
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let not_a_pointer = int(&mut body, 8);
    let one = int(&mut body, 1);
    mem(
        &mut body,
        MemOp::Store,
        &[m1, not_a_pointer, one],
        TypeId::MEM,
    );
    let error = check_types(&body).unwrap_err();
    assert!(error.rule.contains("ptr"), "{error}");
}

#[test]
fn the_pass_pipeline_keeps_memory_bodies_valid() {
    // Every pass of the full pipeline leaves the memory chain linear and
    // in an executable order.
    let mut body = store_then_load();
    crate::pass::full_pipeline()
        .run_to_fixpoint(&mut body, &crate::pass::PassContext::default(), 8)
        .expect("passes keep memory bodies valid");
    verify_body(&body).unwrap();
    assert!(print_body(&body).contains("mem.store"));

    // Two stores of the same value to the same address never merge: their
    // states differ. (The CSE pass keys on kind and operands but not on the
    // result type, so it leaves loads alone -- an i64 and an i32 load of one
    // address are different values. The e-graph, which keys on type, merges
    // identical loads.)
    let mut body = Body::new();
    let m0 = mem(&mut body, MemOp::Entry, &[], TypeId::MEM);
    let m1 = mem(&mut body, alloc_i64(0), &[m0], TypeId::MEM);
    let p = mem(&mut body, MemOp::Addr, &[m1], TypeId::PTR);
    let one = int(&mut body, 1);
    let m2 = mem(&mut body, MemOp::Store, &[m1, p, one], TypeId::MEM);
    let m3 = mem(&mut body, MemOp::Store, &[m2, p, one], TypeId::MEM);
    let a = mem(&mut body, MemOp::Load, &[m3, p], TypeId::I64);
    let b = mem(&mut body, MemOp::Load, &[m3, p], TypeId::I64);
    let sum = body.push(OpKind::Add, [a, b]);
    body.push(OpKind::Tuple, [m3, sum]);
    crate::pass::default_pipeline()
        .run_to_fixpoint(&mut body, &crate::pass::PassContext::default(), 8)
        .unwrap();
    verify_body(&body).unwrap();
    let text = print_body(&body);
    assert_eq!(text.matches("mem.load").count(), 2, "{text}");
    assert_eq!(text.matches("mem.store").count(), 2, "{text}");
}

//! Real end-to-end proof: build a `codira_mir::Body` by hand, lower it to
//! LLVM IR via `lower_mir_body`, JIT-compile the containing module, and
//! actually *call* the resulting native machine code -- not just check
//! that IR was emitted without erroring.

use codira_mir::{Attr, Body, OpKind, Region};
use inkwell::{
    context::Context,
    targets::{InitializationConfig, Target},
    OptimizationLevel,
};

fn init_native_target() {
    Target::initialize_native(&InitializationConfig::default())
        .expect("failed to initialize native target for JIT");
}

#[test]
fn jit_runs_arithmetic_with_a_runtime_argument() {
    // `fn(x: i64) -> i64 { x + 5 }`
    init_native_target();
    let context = Context::create();
    let module = context.create_module("mir_codegen_test");
    let builder = context.create_builder();

    let i64_ty = context.i64_type();
    let fn_type = i64_ty.fn_type(&[i64_ty.into()], false);
    let function = module.add_function("add_five", fn_type, None);
    let entry = context.append_basic_block(function, "entry");
    builder.position_at_end(entry);

    let mut body = Body::new();
    let x = body.push(OpKind::Arg(0), []);
    let five = body.push(OpKind::Const(Attr::Int(5)), []);
    body.push(OpKind::Add, [x, five]);

    let arg = function.get_nth_param(0).unwrap();
    let result = super::lower_mir_body(&context, &builder, function, &body, &[arg])
        .expect("expected a lowered result");
    builder.build_return(Some(&result)).unwrap();

    assert!(function.verify(true), "generated function failed to verify");

    let engine = module
        .create_jit_execution_engine(OptimizationLevel::None)
        .expect("failed to create JIT execution engine");

    type AddFive = unsafe extern "C" fn(i64) -> i64;
    let add_five: inkwell::execution_engine::JitFunction<AddFive> =
        unsafe { engine.get_function("add_five") }.expect("function not found in JIT module");

    assert_eq!(unsafe { add_five.call(10) }, 15);
    assert_eq!(unsafe { add_five.call(-5) }, 0);
    assert_eq!(unsafe { add_five.call(0) }, 5);
}

#[test]
fn jit_runs_real_conditional_branching() {
    // `fn(x: i64) -> i64 { if x < 10 { 1 } else { 0 } }`
    init_native_target();
    let context = Context::create();
    let module = context.create_module("mir_codegen_test_if");
    let builder = context.create_builder();

    let i64_ty = context.i64_type();
    let fn_type = i64_ty.fn_type(&[i64_ty.into()], false);
    let function = module.add_function("is_small", fn_type, None);
    let entry = context.append_basic_block(function, "entry");
    builder.position_at_end(entry);

    let mut body = Body::new();
    let x = body.push(OpKind::Arg(0), []);
    let ten = body.push(OpKind::Const(Attr::Int(10)), []);
    let cond = body.push(OpKind::Lt, [x, ten]);

    let mut then_body = Body::new();
    then_body.push(OpKind::Const(Attr::Int(1)), []);
    let mut else_body = Body::new();
    else_body.push(OpKind::Const(Attr::Int(0)), []);

    body.push_with_regions(
        OpKind::If,
        [cond],
        [Region::new(then_body), Region::new(else_body)],
    );

    let arg = function.get_nth_param(0).unwrap();
    let result = super::lower_mir_body(&context, &builder, function, &body, &[arg])
        .expect("expected a lowered result");
    builder.build_return(Some(&result)).unwrap();

    assert!(function.verify(true), "generated function failed to verify");

    let engine = module
        .create_jit_execution_engine(OptimizationLevel::None)
        .expect("failed to create JIT execution engine");

    type IsSmall = unsafe extern "C" fn(i64) -> i64;
    let is_small: inkwell::execution_engine::JitFunction<IsSmall> =
        unsafe { engine.get_function("is_small") }.expect("function not found in JIT module");

    assert_eq!(unsafe { is_small.call(5) }, 1);
    assert_eq!(unsafe { is_small.call(20) }, 0);
    assert_eq!(unsafe { is_small.call(10) }, 0);
}

#[test]
fn honestly_refuses_unelaborated_param_ref() {
    // A `ParamRef` that survived to codegen (i.e. elaboration didn't bind
    // it) must not be silently miscompiled -- `lower_mir_body` should
    // return `None`, not fabricate a value.
    init_native_target();
    let context = Context::create();
    let module = context.create_module("mir_codegen_test_unresolved");
    let builder = context.create_builder();

    let i64_ty = context.i64_type();
    let fn_type = i64_ty.fn_type(&[], false);
    let function = module.add_function("unresolved", fn_type, None);
    let entry = context.append_basic_block(function, "entry");
    builder.position_at_end(entry);

    let mut body = Body::new();
    body.push(OpKind::ParamRef("N".into()), []);

    assert!(super::lower_mir_body(&context, &builder, function, &body, &[]).is_none());
}

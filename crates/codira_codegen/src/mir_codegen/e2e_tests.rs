//! End-to-end proof (`spec/KGEN_SUPERSET_STATUS.md` roadmap item 6 /
//! session-2 item #20): real Codira source, parsed and lowered through the
//! actual `codira_hir` salsa database -- not a hand-built `codira_mir::Body`
//! like `super::tests` uses -- specialized via the `elaborate_generator`
//! incremental query, lowered to LLVM IR by `lower_mir_body`, JIT-compiled,
//! and executed. This is the full pipeline the architecture doc promises:
//! `codira_hir::mir_lower` -> `codira_comptime::elaborate` (via the salsa
//! query) -> `codira_codegen::mir_codegen` -> native code.

use codira_hir::{HirDatabase, ModuleDef, Package};
use inkwell::{
    context::Context,
    targets::{InitializationConfig, Target},
    OptimizationLevel,
};

use crate::mock::MockDatabase;

fn init_native_target() {
    Target::initialize_native(&InitializationConfig::default())
        .expect("failed to initialize native target for JIT");
}

#[test]
fn end_to_end_specialize_elaborate_codegen_jit_run() {
    // `func double[N]() -> i64 { N * 2 }`, specialized with N = 21.
    init_native_target();

    let (db, _file_id) = MockDatabase::with_single_file("func double[N]() -> i64 { N * 2 }");
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) => Some(f),
            _ => None,
        })
        .expect("no function found in source");

    let bindings = vec![("N".into(), codira_comptime::Value::Int(21))];
    let elaborated = db
        .elaborate_generator(func, bindings)
        .expect("expected an elaborated body");

    // Elaboration should have folded `N * 2` all the way down to `42` --
    // same claim `codira_hir::mir_lower::tests` verifies via
    // `codira_comptime::eval_body` directly; here it's verified by actually
    // running the compiled machine code instead.
    let context = Context::create();
    let module = context.create_module("e2e_test");
    let builder = context.create_builder();

    let i64_ty = context.i64_type();
    let fn_type = i64_ty.fn_type(&[], false);
    let function = module.add_function("double_21", fn_type, None);
    let entry = context.append_basic_block(function, "entry");
    builder.position_at_end(entry);

    let result = super::lower_mir_body(&context, &builder, function, &elaborated, &[])
        .expect("expected a lowered result");
    builder.build_return(Some(&result)).unwrap();

    assert!(function.verify(true), "generated function failed to verify");

    let engine = module
        .create_jit_execution_engine(OptimizationLevel::None)
        .expect("failed to create JIT execution engine");

    type DoubleTwentyOne = unsafe extern "C" fn() -> i64;
    let double_21: inkwell::execution_engine::JitFunction<DoubleTwentyOne> =
        unsafe { engine.get_function("double_21") }.expect("function not found in JIT module");

    assert_eq!(unsafe { double_21.call() }, 42);
}

#[test]
fn end_to_end_is_memoized_across_codegen_calls() {
    // The salsa memoization proof (`codira_hir::mir_lower::tests::
    // elaborate_generator_query_is_memoized`) from the codegen consumer's
    // point of view: two calls into the query with identical `(func,
    // bindings)` from code that looks exactly like what a real codegen
    // driver would do must hit the cache, not recompute.
    let (db, _file_id) = MockDatabase::with_single_file("func triple[N]() -> i64 { N * 3 }");
    let func = Package::all(&db)
        .iter()
        .flat_map(|pkg| pkg.modules(&db))
        .flat_map(|module| module.declarations(&db))
        .find_map(|item| match item {
            ModuleDef::Function(f) => Some(f),
            _ => None,
        })
        .expect("no function found in source");

    let bindings = vec![("N".into(), codira_comptime::Value::Int(7))];
    let first = db
        .elaborate_generator(func, bindings.clone())
        .expect("expected an elaborated body");
    let second = db
        .elaborate_generator(func, bindings)
        .expect("expected an elaborated body");

    assert!(
        std::sync::Arc::ptr_eq(&first, &second),
        "salsa should have served the second call from cache instead of recomputing"
    );
}

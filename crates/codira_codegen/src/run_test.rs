//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Runs real Codira source through the production code generator
//!   (`gen_file_ir`, the path `codira build` takes) and executes the result
//!   in-process, so a test asserts what the code *computes* rather than what IR
//!   it prints.
//!
//! The snapshot tests in `crate::test` pin the shape of the emitted IR; they
//! cannot tell `fcmp one` from `fcmp une` unless someone reads the snapshot
//! with IEEE 754 in mind. These tests call the compiled function and check
//! the answer.
//!
//! Zero-argument functions go through the Eidos constant folder
//! (`crate::eidos_fold`) before the ordinary lowering, so a test of the
//! folder is a zero-argument function here, and a test of the run-time
//! lowering takes its inputs as parameters.
//!
//! Only self-contained functions can run this way: the file module is
//! JIT-compiled on its own, without the group module's dispatch table or a
//! runtime, so a function that calls another function, allocates, or uses a
//! type table will not resolve.

#![allow(clippy::float_cmp, clippy::items_after_statements)]

use std::cell::RefCell;

use codira_hir::{diagnostics::DiagnosticSink, HirDatabase};
use codira_hir_input::WithFixture;
use codira_target::spec::Target;
use inkwell::{
    context::Context,
    execution_engine::{ExecutionEngine, JitFunction, UnsafeFunctionPointer},
    targets::{InitializationConfig, Target as LlvmTarget},
    OptimizationLevel,
};

use crate::{
    code_gen::CodeGenContext,
    ir::{file::gen_file_ir, file_group::gen_file_group_ir},
    mock::MockDatabase,
    CodeGenDatabase,
};

/// Compiles `text` at `opt`, panics with the diagnostics if it does not
/// type-check, and hands the JIT engine for its (single) file module to `f`.
fn with_jit<R>(text: &str, opt: OptimizationLevel, f: impl FnOnce(&ExecutionEngine<'_>) -> R) -> R {
    LlvmTarget::initialize_native(&InitializationConfig::default())
        .expect("failed to initialize native target for JIT");

    let mut db = MockDatabase::with_files(text);
    db.set_optimization_level(opt);
    db.set_target(Target::host_target().unwrap());

    let messages = RefCell::new(Vec::new());
    let mut sink = DiagnosticSink::new(|diag| messages.borrow_mut().push(diag.message()));
    for module in codira_hir::Package::all(&db)
        .into_iter()
        .flat_map(|package| package.modules(&db))
    {
        module.diagnostics(&db, &mut sink);
    }
    drop(sink);
    let messages = messages.into_inner();
    assert!(messages.is_empty(), "source does not compile: {messages:?}");

    let llvm_context = Context::create();
    let code_gen = CodeGenContext::new(&llvm_context, &db);
    let partition = db.module_partition();
    let (_, module_group) = partition.iter().next().expect("source has no module group");
    let group_ir = gen_file_group_ir(&code_gen, module_group);
    let file_ir = gen_file_ir(&code_gen, &group_ir, module_group);
    let engine = file_ir
        .llvm_module
        .create_jit_execution_engine(OptimizationLevel::None)
        .expect("failed to create JIT execution engine");
    f(&engine)
}

fn get<'e, F: UnsafeFunctionPointer>(
    engine: &'e ExecutionEngine<'_>,
    name: &str,
) -> JitFunction<'e, F> {
    unsafe { engine.get_function(name) }
        .unwrap_or_else(|e| panic!("`{name}` not in JIT module: {e:?}"))
}

/// Both optimisation levels: `None` runs the lowering as written, `Default`
/// lets LLVM fold it -- and LLVM folds by the predicate it was given, so a
/// wrong predicate is wrong at both.
const LEVELS: [OptimizationLevel; 2] = [OptimizationLevel::None, OptimizationLevel::Default];

// ---- float comparisons ----------------------------------------------------

#[test]
fn float_comparisons_follow_ieee_754_on_nan() {
    // `!=` is the negation of `==`, so it is the one comparison that is
    // *true* when an operand is NaN. Every ordering and `==` are false.
    let src = r"
    public func eq(a: f64, b: f64) -> bool { a == b }
    public func ne(a: f64, b: f64) -> bool { a != b }
    public func lt(a: f64, b: f64) -> bool { a < b }
    public func le(a: f64, b: f64) -> bool { a <= b }
    public func gt(a: f64, b: f64) -> bool { a > b }
    public func ge(a: f64, b: f64) -> bool { a >= b }
    public func ne32(a: f32, b: f32) -> bool { a != b }
    public func eq32(a: f32, b: f32) -> bool { a == b }
    ";
    type Cmp = unsafe extern "C" fn(f64, f64) -> bool;
    type Cmp32 = unsafe extern "C" fn(f32, f32) -> bool;
    let nan = f64::NAN;
    let pairs = [
        (nan, 1.0),
        (1.0, nan),
        (nan, nan),
        (1.0, 1.0),
        (1.0, 2.0),
        (2.0, 1.0),
    ];
    for opt in LEVELS {
        with_jit(src, opt, |engine| {
            let run = |name: &str| -> Vec<bool> {
                let f = get::<Cmp>(engine, name);
                pairs
                    .iter()
                    .map(|&(a, b)| unsafe { f.call(a, b) })
                    .collect()
            };
            assert_eq!(
                run("eq"),
                [false, false, false, true, false, false],
                "== at {opt:?}"
            );
            assert_eq!(
                run("ne"),
                [true, true, true, false, true, true],
                "!= at {opt:?}"
            );
            assert_eq!(
                run("lt"),
                [false, false, false, false, true, false],
                "< at {opt:?}"
            );
            assert_eq!(
                run("le"),
                [false, false, false, true, true, false],
                "<= at {opt:?}"
            );
            assert_eq!(
                run("gt"),
                [false, false, false, false, false, true],
                "> at {opt:?}"
            );
            assert_eq!(
                run("ge"),
                [false, false, false, true, false, true],
                ">= at {opt:?}"
            );

            let ne32 = get::<Cmp32>(engine, "ne32");
            let eq32 = get::<Cmp32>(engine, "eq32");
            unsafe {
                assert!(ne32.call(f32::NAN, 1.0), "f32 NaN != 1.0 at {opt:?}");
                assert!(ne32.call(f32::NAN, f32::NAN), "f32 NaN != NaN at {opt:?}");
                assert!(!ne32.call(1.0, 1.0), "f32 1.0 != 1.0 at {opt:?}");
                assert!(!eq32.call(f32::NAN, f32::NAN), "f32 NaN == NaN at {opt:?}");
            }
        });
    }
}

#[test]
fn float_not_equal_agrees_with_negated_equal() {
    // `a != b` and `!(a == b)` are the same function; before `!=` was
    // unordered they disagreed exactly on NaN.
    let src = r"
    public func ne(a: f64, b: f64) -> bool { a != b }
    public func not_eq(a: f64, b: f64) -> bool { !(a == b) }
    ";
    type Cmp = unsafe extern "C" fn(f64, f64) -> bool;
    let values = [f64::NAN, 0.0, -0.0, 1.0, f64::INFINITY, f64::NEG_INFINITY];
    for opt in LEVELS {
        with_jit(src, opt, |engine| {
            let ne = get::<Cmp>(engine, "ne");
            let not_eq = get::<Cmp>(engine, "not_eq");
            for a in values {
                for b in values {
                    unsafe {
                        assert_eq!(ne.call(a, b), not_eq.call(a, b), "{a} != {b} at {opt:?}");
                    }
                }
            }
        });
    }
}

#[test]
fn folded_float_not_equal_on_nan_is_true() {
    // Zero-argument: answered by the Eidos folder, which must agree with
    // the run-time lowering above.
    let src = r"
    public func nan_ne_nan() -> bool {
        let n = 0.0 / 0.0;
        n != n
    }
    public func nan_eq_nan() -> bool {
        let n = 0.0 / 0.0;
        n == n
    }
    ";
    type Nullary = unsafe extern "C" fn() -> bool;
    for opt in LEVELS {
        with_jit(src, opt, |engine| unsafe {
            assert!(
                get::<Nullary>(engine, "nan_ne_nan").call(),
                "folded NaN != NaN at {opt:?}"
            );
            assert!(
                !get::<Nullary>(engine, "nan_eq_nan").call(),
                "folded NaN == NaN at {opt:?}"
            );
        });
    }
}

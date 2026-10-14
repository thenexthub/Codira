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

// ---- `as` casts folded at compile time ------------------------------------
//
// Every function below has no parameters, so its body is answered by the
// Eidos folder (`crate::eidos_fold`) when the folder can, and by ordinary
// lowering when it declines. Either way the answer must be Rust's: integer
// conversions wrap (truncate, zero- or sign-extend by the *source* type),
// float-to-integer saturates with NaN mapping to zero.

/// Runs each zero-argument function named in the list and compares its
/// result with the expected value, at both optimisation levels.
macro_rules! assert_nullary {
    ($src:expr, $ret:ty, [$(($name:literal, $expected:expr)),+ $(,)?]) => {
        for opt in LEVELS {
            with_jit($src, opt, |engine| {
                $(
                    let f = get::<unsafe extern "C" fn() -> $ret>(engine, $name);
                    let got = unsafe { f.call() };
                    let expected: $ret = $expected;
                    assert!(
                        got == expected,
                        "{} at {opt:?}: got {got:?}, expected {expected:?}",
                        $name
                    );
                )+
            });
        }
    };
}

#[test]
fn folded_widening_casts_extend_by_the_source_type() {
    assert_nullary!(
        r"
        public func u32_to_u64() -> u64 { let a: u32 = 1000; a as u64 }
        public func u32_max_to_u64() -> u64 { let a: u32 = 4294967295; a as u64 }
        public func u32_to_u64_ret() -> u64 { let a: u32 = 1000; return a as u64; }
        public func u8_to_u64() -> u64 { let a: u8 = 200; a as u64 }
        public func i32_neg_to_u64() -> u64 { let a: i32 = -5; a as u64 }
        ",
        u64,
        [
            ("u32_to_u64", 1000),
            ("u32_max_to_u64", 4_294_967_295),
            ("u32_to_u64_ret", 1000),
            ("u8_to_u64", 200),
            ("i32_neg_to_u64", (-5i64).cast_unsigned()),
        ]
    );
    assert_nullary!(
        r"
        public func i32_to_i64() -> i64 { let a: i32 = -5; a as i64 }
        public func u32_to_i64() -> i64 { let a: u32 = 4294967295; a as i64 }
        public func i8_to_i64() -> i64 { let a: i8 = -100; a as i64 }
        public func u16_to_i64() -> i64 { let a: u16 = 65535; a as i64 }
        ",
        i64,
        [
            ("i32_to_i64", -5),
            ("u32_to_i64", 4_294_967_295),
            ("i8_to_i64", -100),
            ("u16_to_i64", 65535),
        ]
    );
    assert_nullary!(
        r"
        public func u8_to_i32() -> i32 { let a: u8 = 200; a as i32 }
        public func i8_to_i32() -> i32 { let a: i8 = -100; a as i32 }
        ",
        i32,
        [("u8_to_i32", 200), ("i8_to_i32", -100)]
    );
}

#[test]
fn folded_narrowing_casts_keep_the_low_bits() {
    assert_nullary!(
        r"
        public func i64_300_to_u8() -> u8 { let a: i64 = 300; a as u8 }
        public func i64_neg1_to_u8() -> u8 { let a: i64 = -1; a as u8 }
        public func u32_to_u8() -> u8 { let a: u32 = 4294967295; a as u8 }
        ",
        u8,
        [
            ("i64_300_to_u8", 44),
            ("i64_neg1_to_u8", 255),
            ("u32_to_u8", 255)
        ]
    );
    assert_nullary!(
        r"
        public func i64_200_to_i8() -> i8 { let a: i64 = 200; a as i8 }
        public func u16_to_i8() -> i8 { let a: u16 = 65408; a as i8 }
        ",
        i8,
        [("i64_200_to_i8", -56), ("u16_to_i8", -128)]
    );
    assert_nullary!(
        r"
        public func i64_neg1_to_u32() -> u32 { let a: i64 = -1; a as u32 }
        public func i64_big_to_u32() -> u32 { let a: i64 = 4294967396; a as u32 }
        ",
        u32,
        [("i64_neg1_to_u32", u32::MAX), ("i64_big_to_u32", 100)]
    );
    assert_nullary!(
        r"
        public func i64_to_i32_wraps() -> i32 { let a: i64 = 2147483648; a as i32 }
        public func u64_to_i32() -> i32 { let a: u64 = 4294967295; a as i32 }
        ",
        i32,
        [("i64_to_i32_wraps", i32::MIN), ("u64_to_i32", -1)]
    );
}

#[test]
fn folded_int_to_float_casts_read_the_source_signedness() {
    assert_nullary!(
        r"
        public func u32_max_to_f64() -> f64 { let a: u32 = 4294967295; a as f64 }
        public func i32_neg_to_f64() -> f64 { let a: i32 = -7; a as f64 }
        public func u8_to_f64() -> f64 { let a: u8 = 255; a as f64 }
        public func i8_to_f64() -> f64 { let a: i8 = -128; a as f64 }
        ",
        f64,
        [
            ("u32_max_to_f64", 4_294_967_295.0),
            ("i32_neg_to_f64", -7.0),
            ("u8_to_f64", 255.0),
            ("i8_to_f64", -128.0),
        ]
    );
    assert_nullary!(
        r"
        public func i64_to_f32_rounds() -> f32 { let a: i64 = 16777217; a as f32 }
        public func u16_to_f32() -> f32 { let a: u16 = 65535; a as f32 }
        ",
        f32,
        [("i64_to_f32_rounds", 16_777_216.0), ("u16_to_f32", 65535.0)]
    );
}

#[test]
fn folded_float_to_int_casts_saturate() {
    assert_nullary!(
        r"
        public func f_300_7_to_u8() -> u8 { let a: f64 = 300.7; a as u8 }
        public func f_neg_to_u8() -> u8 { let a: f64 = -1.5; a as u8 }
        public func f_nan_to_u8() -> u8 { let a: f64 = 0.0 / 0.0; a as u8 }
        public func f_200_9_to_u8() -> u8 { let a: f64 = 200.9; a as u8 }
        ",
        u8,
        [
            ("f_300_7_to_u8", 255),
            ("f_neg_to_u8", 0),
            ("f_nan_to_u8", 0),
            ("f_200_9_to_u8", 200)
        ]
    );
    assert_nullary!(
        r"
        public func f_big_to_i32() -> i32 { let a: f64 = 1000000000000.0; a as i32 }
        public func f_small_to_i32() -> i32 { let a: f64 = -1000000000000.0; a as i32 }
        public func f_nan_to_i32() -> i32 { let a: f64 = 0.0 / 0.0; a as i32 }
        public func f_neg_trunc_to_i32() -> i32 { let a: f64 = -3.99; a as i32 }
        public func f32_to_i32() -> i32 { let a: f32 = 2.5; a as i32 }
        ",
        i32,
        [
            ("f_big_to_i32", i32::MAX),
            ("f_small_to_i32", i32::MIN),
            ("f_nan_to_i32", 0),
            ("f_neg_trunc_to_i32", -3),
            ("f32_to_i32", 2),
        ]
    );
    assert_nullary!(
        r"
        public func f_inf_to_u64() -> u64 { let a: f64 = 1.0 / 0.0; a as u64 }
        public func f_to_u64() -> u64 { let a: f64 = 18446744073709549568.0; a as u64 }
        ",
        u64,
        [
            ("f_inf_to_u64", u64::MAX),
            ("f_to_u64", 18_446_744_073_709_549_568)
        ]
    );
}

#[test]
fn folded_float_width_casts_round_through_f32() {
    assert_nullary!(
        r"
        public func f64_to_f32() -> f32 { let a: f64 = 0.1; a as f32 }
        public func f64_big_to_f32() -> f32 { let a: f64 = 1e300; a as f32 }
        ",
        f32,
        [("f64_to_f32", 0.1f32), ("f64_big_to_f32", f32::INFINITY)]
    );
    assert_nullary!(
        r"
        public func f32_to_f64() -> f64 { let a: f32 = 0.1; a as f64 }
        public func roundtrip() -> f64 { let a: f64 = 0.1; (a as f32) as f64 }
        ",
        f64,
        [
            ("f32_to_f64", f64::from(0.1f32)),
            ("roundtrip", f64::from(0.1f32))
        ]
    );
}

#[test]
fn folded_narrow_arithmetic_wraps_at_the_declared_width() {
    // The folder evaluates integers in an `i64` domain. Arithmetic on a
    // narrower or unsigned type must still wrap at that type's width
    // before it is compared, divided or converted.
    assert_nullary!(
        r"
        public func u8_sum_wraps() -> bool { let a: u8 = 200; let b: u8 = 100; (a + b) == 44 }
        public func u64_underflow_is_large() -> bool { let a: u64 = 0; (a - 1) > 1 }
        public func u32_underflow_is_large() -> bool { let a: u32 = 0; (a - 1) > 1 }
        public func i8_overflow_is_negative() -> bool { let a: i8 = 127; (a + 1) < 0 }
        ",
        bool,
        [
            ("u8_sum_wraps", true),
            ("u64_underflow_is_large", true),
            ("u32_underflow_is_large", true),
            ("i8_overflow_is_negative", true),
        ]
    );
    assert_nullary!(
        r"
        public func i32_overflow_widened() -> i64 { let a: i32 = 2147483647; (a + 1) as i64 }
        public func u8_product_widened() -> i64 { let a: u8 = 16; (a * a) as i64 }
        public func u32_div_large() -> i64 { let a: u32 = 0; ((a - 2) / 2) as i64 }
        ",
        i64,
        [
            ("i32_overflow_widened", -2_147_483_648),
            ("u8_product_widened", 0),
            ("u32_div_large", 2_147_483_647),
        ]
    );
    assert_nullary!(
        r"
        public func u64_div_large() -> u64 { let a: u64 = 0; (a - 2) / 2 }
        public func u64_shr_large() -> u64 { let a: u64 = 0; (a - 1) >> 60 }
        public func u64_rem_large() -> u64 { let a: u64 = 0; (a - 1) % 10 }
        ",
        u64,
        [
            ("u64_div_large", u64::MAX / 2),
            ("u64_shr_large", 15),
            ("u64_rem_large", u64::MAX % 10),
        ]
    );
}

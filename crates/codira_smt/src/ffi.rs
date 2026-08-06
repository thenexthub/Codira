//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Original module content restored; copyright header moved to top.
//!
//! Raw, hand-written FFI declarations for the small subset of Z3's C API
//! (`z3_api.h`) this crate actually uses. Every opaque handle type
//! (`Z3_context`, `Z3_ast`, ...) is declared in `z3_api.h` via
//! `DEFINE_TYPE(name)`, which expands to an opaque pointer typedef; they're
//! represented here the same way, as newtypes over `*mut c_void` so the
//! Rust type system still distinguishes e.g. a `Z3_sort` from a `Z3_ast`
//! even though both are "just a pointer" at the ABI level.
#![allow(non_camel_case_types, dead_code)]

use std::ffi::{c_char, c_int, c_void};

macro_rules! opaque_handle {
    ($name:ident) => {
        #[repr(transparent)]
        #[derive(Debug, Clone, Copy, PartialEq, Eq)]
        pub struct $name(pub *mut c_void);
    };
}

opaque_handle!(Z3_config);
opaque_handle!(Z3_context);
opaque_handle!(Z3_symbol);
opaque_handle!(Z3_sort);
opaque_handle!(Z3_ast);
opaque_handle!(Z3_solver);
opaque_handle!(Z3_model);

pub type Z3_string = *const c_char;

/// Mirrors the C `Z3_lbool` enum (`z3_api.h`): `Z3_L_FALSE = -1`,
/// `Z3_L_UNDEF = 0`, `Z3_L_TRUE = 1`.
pub type Z3_lbool = c_int;
pub const Z3_L_FALSE: Z3_lbool = -1;
pub const Z3_L_UNDEF: Z3_lbool = 0;
pub const Z3_L_TRUE: Z3_lbool = 1;

extern "C" {
    pub fn Z3_mk_config() -> Z3_config;
    pub fn Z3_del_config(c: Z3_config);
    pub fn Z3_mk_context(c: Z3_config) -> Z3_context;
    pub fn Z3_del_context(c: Z3_context);

    pub fn Z3_mk_bool_sort(c: Z3_context) -> Z3_sort;
    pub fn Z3_mk_int_sort(c: Z3_context) -> Z3_sort;

    pub fn Z3_mk_string_symbol(c: Z3_context, s: Z3_string) -> Z3_symbol;
    pub fn Z3_mk_const(c: Z3_context, s: Z3_symbol, ty: Z3_sort) -> Z3_ast;
    pub fn Z3_mk_numeral(c: Z3_context, numeral: Z3_string, ty: Z3_sort) -> Z3_ast;

    pub fn Z3_mk_true(c: Z3_context) -> Z3_ast;
    pub fn Z3_mk_false(c: Z3_context) -> Z3_ast;
    pub fn Z3_mk_eq(c: Z3_context, l: Z3_ast, r: Z3_ast) -> Z3_ast;
    pub fn Z3_mk_not(c: Z3_context, a: Z3_ast) -> Z3_ast;
    pub fn Z3_mk_implies(c: Z3_context, t1: Z3_ast, t2: Z3_ast) -> Z3_ast;
    pub fn Z3_mk_and(c: Z3_context, num_args: c_int, args: *const Z3_ast) -> Z3_ast;
    pub fn Z3_mk_or(c: Z3_context, num_args: c_int, args: *const Z3_ast) -> Z3_ast;
    pub fn Z3_mk_add(c: Z3_context, num_args: c_int, args: *const Z3_ast) -> Z3_ast;
    pub fn Z3_mk_sub(c: Z3_context, num_args: c_int, args: *const Z3_ast) -> Z3_ast;
    pub fn Z3_mk_mul(c: Z3_context, num_args: c_int, args: *const Z3_ast) -> Z3_ast;
    pub fn Z3_mk_lt(c: Z3_context, t1: Z3_ast, t2: Z3_ast) -> Z3_ast;
    pub fn Z3_mk_le(c: Z3_context, t1: Z3_ast, t2: Z3_ast) -> Z3_ast;
    pub fn Z3_mk_gt(c: Z3_context, t1: Z3_ast, t2: Z3_ast) -> Z3_ast;
    pub fn Z3_mk_ge(c: Z3_context, t1: Z3_ast, t2: Z3_ast) -> Z3_ast;

    pub fn Z3_mk_solver(c: Z3_context) -> Z3_solver;
    pub fn Z3_solver_inc_ref(c: Z3_context, s: Z3_solver);
    pub fn Z3_solver_dec_ref(c: Z3_context, s: Z3_solver);
    pub fn Z3_solver_assert(c: Z3_context, s: Z3_solver, a: Z3_ast);
    pub fn Z3_solver_check(c: Z3_context, s: Z3_solver) -> Z3_lbool;
    pub fn Z3_solver_to_string(c: Z3_context, s: Z3_solver) -> Z3_string;

    pub fn Z3_ast_to_string(c: Z3_context, a: Z3_ast) -> Z3_string;
}


//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Original module doc-comment restored below.
//!
//! SMT-based feasibility/proof checking, backed by a real Z3 solver.
//!
//! This is the "SMT-based feasibility/proof checking" piece described in
//! `spec/self_healing_programming_language.md`: given Codira's refinement
//! types (`Type { binder | predicate }`, see `spec/LANGUAGE_SPEC.md` §7)
//! and healing-strategy preconditions, this crate can actually ask a real
//! SMT solver "is this predicate satisfiable?" and "does this predicate
//! imply that one?" over linear integer arithmetic, rather than faking an
//! answer or hand-rolling a bespoke (and much weaker) decision procedure.
//!
//! It binds directly to Z3's C API (`libz3.dll`) via hand-written FFI
//! declarations rather than a bindgen-based crate: the API surface actually
//! needed here (contexts, integer/boolean expressions, a handful of
//! arithmetic/comparison/boolean operators, and a solver) is small and
//! stable enough that hand-writing it avoids depending on libclang being
//! discoverable at build time. See `build.rs` for how the real Z3 install
//! is located and linked.

mod ffi;

use std::ffi::{c_int, CStr, CString};

/// An owned Z3 context: the arena every expression, sort, and solver
/// created through this crate's API lives in. Expressions and solvers
/// borrow from their `Context` and cannot outlive it.
pub struct Context {
    raw: ffi::Z3_context,
    config: ffi::Z3_config,
}

// Z3 contexts are safe to send between threads as long as they aren't
// accessed concurrently, which the `&`/`&mut` borrows below already
// enforce; there's no thread-local state involved.
unsafe impl Send for Context {}

impl Context {
    /// Creates a fresh Z3 context.
    pub fn new() -> Self {
        // Safety: `Z3_mk_config`/`Z3_mk_context` are always safe to call and
        // never return null on any Z3 build actually shipped.
        unsafe {
            let config = ffi::Z3_mk_config();
            let raw = ffi::Z3_mk_context(config);
            Context { raw, config }
        }
    }

    fn int_sort(&self) -> ffi::Z3_sort {
        unsafe { ffi::Z3_mk_int_sort(self.raw) }
    }

    fn bool_sort(&self) -> ffi::Z3_sort {
        unsafe { ffi::Z3_mk_bool_sort(self.raw) }
    }

    /// Declares a free integer-sorted variable with the given name.
    pub fn int_var(&self, name: &str) -> IntExpr<'_> {
        let c_name = CString::new(name).expect("variable name must not contain a NUL byte");
        // Safety: all arguments are valid, live for the duration of the
        // call, and `self.raw` outlives the returned `Z3_ast` (which is
        // freed only when `self.raw` itself is destroyed).
        let raw = unsafe {
            let symbol = ffi::Z3_mk_string_symbol(self.raw, c_name.as_ptr());
            ffi::Z3_mk_const(self.raw, symbol, self.int_sort())
        };
        IntExpr { ctx: self, raw }
    }

    /// An integer literal.
    pub fn int_lit(&self, value: i64) -> IntExpr<'_> {
        let c_value = CString::new(value.to_string()).unwrap();
        let raw = unsafe { ffi::Z3_mk_numeral(self.raw, c_value.as_ptr(), self.int_sort()) };
        IntExpr { ctx: self, raw }
    }

    /// A free boolean-sorted variable with the given name.
    pub fn bool_var(&self, name: &str) -> BoolExpr<'_> {
        let c_name = CString::new(name).expect("variable name must not contain a NUL byte");
        let raw = unsafe {
            let symbol = ffi::Z3_mk_string_symbol(self.raw, c_name.as_ptr());
            ffi::Z3_mk_const(self.raw, symbol, self.bool_sort())
        };
        BoolExpr { ctx: self, raw }
    }

    /// The boolean literal `true`.
    pub fn bool_true(&self) -> BoolExpr<'_> {
        BoolExpr {
            ctx: self,
            raw: unsafe { ffi::Z3_mk_true(self.raw) },
        }
    }

    /// The boolean literal `false`.
    pub fn bool_false(&self) -> BoolExpr<'_> {
        BoolExpr {
            ctx: self,
            raw: unsafe { ffi::Z3_mk_false(self.raw) },
        }
    }

    /// Creates a new solver bound to this context.
    pub fn solver(&self) -> Solver<'_> {
        let raw = unsafe {
            let solver = ffi::Z3_mk_solver(self.raw);
            ffi::Z3_solver_inc_ref(self.raw, solver);
            solver
        };
        Solver { ctx: self, raw }
    }
}

impl Default for Context {
    fn default() -> Self {
        Self::new()
    }
}

impl Drop for Context {
    fn drop(&mut self) {
        // Safety: every `Z3_ast`/`Z3_sort` created through this context is
        // owned by the context itself (this crate never uses the
        // reference-counted `Z3_mk_context_rc` variant), so it's correct to
        // free them all at once by deleting the context.
        unsafe {
            ffi::Z3_del_context(self.raw);
            ffi::Z3_del_config(self.config);
        }
    }
}

/// An integer-sorted Z3 expression, borrowed from the [`Context`] that
/// created it.
#[derive(Clone, Copy)]
pub struct IntExpr<'ctx> {
    ctx: &'ctx Context,
    raw: ffi::Z3_ast,
}

impl<'ctx> IntExpr<'ctx> {
    fn binop(
        self,
        other: IntExpr<'ctx>,
        f: unsafe extern "C" fn(ffi::Z3_context, ffi::Z3_ast, ffi::Z3_ast) -> ffi::Z3_ast,
    ) -> BoolExpr<'ctx> {
        BoolExpr {
            ctx: self.ctx,
            raw: unsafe { f(self.ctx.raw, self.raw, other.raw) },
        }
    }

    fn variadic(
        self,
        other: IntExpr<'ctx>,
        f: unsafe extern "C" fn(ffi::Z3_context, c_int, *const ffi::Z3_ast) -> ffi::Z3_ast,
    ) -> IntExpr<'ctx> {
        let args = [self.raw, other.raw];
        IntExpr {
            ctx: self.ctx,
            raw: unsafe { f(self.ctx.raw, 2, args.as_ptr()) },
        }
    }

    pub fn lt(self, other: IntExpr<'ctx>) -> BoolExpr<'ctx> {
        self.binop(other, ffi::Z3_mk_lt)
    }

    pub fn le(self, other: IntExpr<'ctx>) -> BoolExpr<'ctx> {
        self.binop(other, ffi::Z3_mk_le)
    }

    pub fn gt(self, other: IntExpr<'ctx>) -> BoolExpr<'ctx> {
        self.binop(other, ffi::Z3_mk_gt)
    }

    pub fn ge(self, other: IntExpr<'ctx>) -> BoolExpr<'ctx> {
        self.binop(other, ffi::Z3_mk_ge)
    }

    pub fn eq(self, other: IntExpr<'ctx>) -> BoolExpr<'ctx> {
        BoolExpr {
            ctx: self.ctx,
            raw: unsafe { ffi::Z3_mk_eq(self.ctx.raw, self.raw, other.raw) },
        }
    }

    pub fn add(self, other: IntExpr<'ctx>) -> IntExpr<'ctx> {
        self.variadic(other, ffi::Z3_mk_add)
    }

    pub fn sub(self, other: IntExpr<'ctx>) -> IntExpr<'ctx> {
        self.variadic(other, ffi::Z3_mk_sub)
    }

    pub fn mul(self, other: IntExpr<'ctx>) -> IntExpr<'ctx> {
        self.variadic(other, ffi::Z3_mk_mul)
    }
}

/// A boolean-sorted Z3 expression, borrowed from the [`Context`] that
/// created it.
#[derive(Clone, Copy)]
pub struct BoolExpr<'ctx> {
    ctx: &'ctx Context,
    raw: ffi::Z3_ast,
}

impl<'ctx> BoolExpr<'ctx> {
    pub fn not(self) -> BoolExpr<'ctx> {
        BoolExpr {
            ctx: self.ctx,
            raw: unsafe { ffi::Z3_mk_not(self.ctx.raw, self.raw) },
        }
    }

    pub fn and(self, other: BoolExpr<'ctx>) -> BoolExpr<'ctx> {
        let args = [self.raw, other.raw];
        BoolExpr {
            ctx: self.ctx,
            raw: unsafe { ffi::Z3_mk_and(self.ctx.raw, 2, args.as_ptr()) },
        }
    }

    pub fn or(self, other: BoolExpr<'ctx>) -> BoolExpr<'ctx> {
        let args = [self.raw, other.raw];
        BoolExpr {
            ctx: self.ctx,
            raw: unsafe { ffi::Z3_mk_or(self.ctx.raw, 2, args.as_ptr()) },
        }
    }

    pub fn implies(self, other: BoolExpr<'ctx>) -> BoolExpr<'ctx> {
        BoolExpr {
            ctx: self.ctx,
            raw: unsafe { ffi::Z3_mk_implies(self.ctx.raw, self.raw, other.raw) },
        }
    }

    /// Combines a slice of boolean expressions with `&&`; `true` for an
    /// empty slice (the identity for conjunction).
    pub fn conjunction(ctx: &'ctx Context, exprs: &[BoolExpr<'ctx>]) -> BoolExpr<'ctx> {
        if exprs.is_empty() {
            return ctx.bool_true();
        }
        let raws: Vec<ffi::Z3_ast> = exprs.iter().map(|e| e.raw).collect();
        BoolExpr {
            ctx,
            raw: unsafe { ffi::Z3_mk_and(ctx.raw, raws.len() as c_int, raws.as_ptr()) },
        }
    }
}

/// The result of asking a [`Solver`] whether its current assertions are
/// satisfiable.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SatResult {
    /// A satisfying assignment exists.
    Sat,
    /// No satisfying assignment exists -- the assertions are contradictory.
    Unsat,
    /// Z3 could not determine satisfiability (e.g. it timed out, or the
    /// formula falls outside the theories it can decide).
    Unknown,
}

/// A Z3 solver: an incremental set of boolean assertions that can be
/// checked for satisfiability.
pub struct Solver<'ctx> {
    ctx: &'ctx Context,
    raw: ffi::Z3_solver,
}

impl<'ctx> Solver<'ctx> {
    /// Adds `expr` to the solver's set of assertions.
    pub fn assert(&self, expr: BoolExpr<'ctx>) {
        unsafe { ffi::Z3_solver_assert(self.ctx.raw, self.raw, expr.raw) };
    }

    /// Checks whether the current set of assertions is satisfiable.
    pub fn check(&self) -> SatResult {
        match unsafe { ffi::Z3_solver_check(self.ctx.raw, self.raw) } {
            ffi::Z3_L_TRUE => SatResult::Sat,
            ffi::Z3_L_FALSE => SatResult::Unsat,
            _ => SatResult::Unknown,
        }
    }

    /// A human-readable dump of the solver's current assertions, useful for
    /// debugging a feasibility check.
    pub fn to_debug_string(&self) -> String {
        unsafe {
            let s = ffi::Z3_solver_to_string(self.ctx.raw, self.raw);
            if s.is_null() {
                String::new()
            } else {
                CStr::from_ptr(s).to_string_lossy().into_owned()
            }
        }
    }
}

impl Drop for Solver<'_> {
    fn drop(&mut self) {
        unsafe { ffi::Z3_solver_dec_ref(self.ctx.raw, self.raw) };
    }
}

/// Checks whether `predicate` is satisfiable: whether there exists *some*
/// assignment to its free variables that makes it true.
///
/// This is exactly the check a refinement type `Type { binder | predicate }`
/// needs to be well-formed in the first place: an *unsatisfiable*
/// refinement (e.g. `i32 { x | x > 0 && x < 0 }`) describes an empty type
/// nothing can ever inhabit, which is almost certainly a mistake at the
/// declaration site rather than an intentional "no valid values" type.
pub fn is_satisfiable<'ctx>(ctx: &'ctx Context, predicate: BoolExpr<'ctx>) -> SatResult {
    let solver = ctx.solver();
    solver.assert(predicate);
    solver.check()
}

/// Checks whether `premise` *implies* `conclusion`: whether every
/// assignment satisfying `premise` also satisfies `conclusion`.
///
/// This is the check a healing strategy's precondition needs: e.g. "is it
/// always safe to retry with `x` halved?" reduces to proving the
/// original refinement predicate implies the retried call's precondition.
/// Implication is checked the standard way for a decision procedure that
/// only directly answers satisfiability queries: `premise => conclusion`
/// is valid exactly when `premise && !conclusion` is unsatisfiable.
pub fn implies<'ctx>(
    ctx: &'ctx Context,
    premise: BoolExpr<'ctx>,
    conclusion: BoolExpr<'ctx>,
) -> bool {
    let solver = ctx.solver();
    solver.assert(premise);
    solver.assert(conclusion.not());
    solver.check() == SatResult::Unsat
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn satisfiable_predicate_is_sat() {
        let ctx = Context::new();
        let x = ctx.int_var("x");
        // x > 0 && x < 100 -- clearly satisfiable (e.g. x = 1).
        let pred = x.gt(ctx.int_lit(0)).and(x.lt(ctx.int_lit(100)));
        assert_eq!(is_satisfiable(&ctx, pred), SatResult::Sat);
    }

    #[test]
    fn contradictory_predicate_is_unsat() {
        let ctx = Context::new();
        let x = ctx.int_var("x");
        // x > 0 && x < 0 -- an empty refinement type, must be UNSAT.
        let pred = x.gt(ctx.int_lit(0)).and(x.lt(ctx.int_lit(0)));
        assert_eq!(is_satisfiable(&ctx, pred), SatResult::Unsat);
    }

    #[test]
    fn tighter_bound_implies_looser_bound() {
        let ctx = Context::new();
        let x = ctx.int_var("x");
        // x > 10 && x < 20  =>  x > 0
        let premise = x.gt(ctx.int_lit(10)).and(x.lt(ctx.int_lit(20)));
        let conclusion = x.gt(ctx.int_lit(0));
        assert!(implies(&ctx, premise, conclusion));
    }

    #[test]
    fn unrelated_bound_does_not_imply() {
        let ctx = Context::new();
        let x = ctx.int_var("x");
        // x > 0  does NOT imply  x > 100 (e.g. x = 1 is a counterexample)
        let premise = x.gt(ctx.int_lit(0));
        let conclusion = x.gt(ctx.int_lit(100));
        assert!(!implies(&ctx, premise, conclusion));
    }

    #[test]
    fn arithmetic_reasoning() {
        let ctx = Context::new();
        let x = ctx.int_var("x");
        let y = ctx.int_var("y");
        // x > 0 && y > 0  =>  x + y > 0
        let premise = x.gt(ctx.int_lit(0)).and(y.gt(ctx.int_lit(0)));
        let conclusion = x.add(y).gt(ctx.int_lit(0));
        assert!(implies(&ctx, premise, conclusion));
    }

    #[test]
    fn division_by_zero_precondition_is_checkable() {
        // Models the exact kind of check a healing strategy would need:
        // "given what we know about `divisor` at the fault site, would a
        // retry with `divisor` clamped to at least 1 actually avoid the
        // division-by-zero that just happened?"
        let ctx = Context::new();
        let divisor = ctx.int_var("divisor");
        let clamped = ctx.int_var("clamped");
        // Precondition of the retry strategy: clamped = max(divisor, 1).
        // Encode `max` as a disjunction rather than needing an `ite`
        // builder in this crate's small API surface.
        let is_divisor = clamped.eq(divisor).and(divisor.ge(ctx.int_lit(1)));
        let is_one = clamped.eq(ctx.int_lit(1)).and(divisor.lt(ctx.int_lit(1)));
        let clamp_def = is_divisor.or(is_one);

        let premise = clamp_def;
        let conclusion = clamped.ge(ctx.int_lit(1));
        assert!(implies(&ctx, premise, conclusion));
    }
}


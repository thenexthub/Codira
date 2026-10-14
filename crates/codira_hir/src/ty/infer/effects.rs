//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Type inference for `perform`, `handle` and `resume` (see
//!   `crate::expr::effects` for the model).
//!
//! The typing rules:
//!
//! * `perform E.op(args)` has the operation's return type; each argument is
//!   checked against the corresponding parameter type.
//! * `handle body { arms }` has one type, `T`: the body's type, which every
//!   arm's value must also have -- an arm that does not resume *is* the value
//!   of the whole `handle`.
//! * An arm's parameters take the operation's parameter types.
//! * `resume(v)` checks `v` against the operation's return type and has type
//!   `never`: control does not continue past it inside the arm.
//!
//! What a path resolves to is recorded in [`EffectResolutions`] for the
//! later passes (the effect validator and code generation). A path that does
//! not resolve is *not* reported here: the effect validator reports it
//! against the path itself, which is where the reader can fix it.
use rustc_hash::FxHashMap;

use super::{error_type, Expectation, InferenceDiagnostic, InferenceResultBuilder};
use crate::{
    code_model::Effect,
    expr::{effects::resolve_effect_op, ExprId, HandlerArm},
    resolve::resolver_for_expr,
    Path, Ty, TyKind,
};

/// What every `perform` and handler arm of a body resolved to.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub(crate) struct EffectResolutions {
    /// For each `perform` expression, the effect and the operation index.
    pub(crate) performs: FxHashMap<ExprId, (Effect, usize)>,
    /// For each handler arm, keyed by its `handle` expression and its
    /// position in that expression's arm list.
    pub(crate) arms: FxHashMap<(ExprId, usize), (Effect, usize)>,
}

impl crate::InferenceResult {
    /// The effect and operation index a `perform` expression invokes.
    pub fn perform_target(&self, expr: ExprId) -> Option<(Effect, usize)> {
        self.effects.performs.get(&expr).copied()
    }

    /// The effect and operation index the `arm`-th arm of the `handle`
    /// expression `handle` implements.
    pub fn handler_arm_target(&self, handle: ExprId, arm: usize) -> Option<(Effect, usize)> {
        self.effects.arms.get(&(handle, arm)).copied()
    }
}

impl InferenceResultBuilder<'_> {
    pub(super) fn infer_perform(&mut self, tgt_expr: ExprId, path: &Path, args: &[ExprId]) -> Ty {
        let resolver = resolver_for_expr(self.db, self.body.owner(), tgt_expr);
        let Ok((effect, op)) = resolve_effect_op(self.db, &resolver, path) else {
            for &arg in args {
                self.infer_expr(arg, &Expectation::none());
            }
            return error_type();
        };
        self.effects.performs.insert(tgt_expr, (effect, op));

        let sig = effect.op_sig(self.db, op);
        if sig.params.len() != args.len() {
            self.diagnostics
                .push(InferenceDiagnostic::ParameterCountMismatch {
                    id: tgt_expr,
                    found: args.len(),
                    expected: sig.params.len(),
                });
        }
        for (idx, &arg) in args.iter().enumerate() {
            match sig.params.get(idx) {
                Some(param) => {
                    self.infer_expr_coerce(arg, &Expectation::has_type(param.clone()));
                }
                None => {
                    self.infer_expr(arg, &Expectation::none());
                }
            }
        }
        sig.ret
    }

    pub(super) fn infer_handle(
        &mut self,
        tgt_expr: ExprId,
        body: ExprId,
        arms: &[HandlerArm],
        expected: &Expectation,
    ) -> Ty {
        let body_ty = self.infer_expr_coerce(body, expected);
        // The type every arm's value must have. A body that never completes
        // says nothing about it, so then the arms decide.
        let result_ty = if !expected.is_none() && !expected.ty.is_unknown() {
            expected.ty.clone()
        } else if body_ty.is_never() {
            self.type_variables.new_type_var()
        } else {
            body_ty
        };

        let resolver = resolver_for_expr(self.db, self.body.owner(), tgt_expr);
        for (idx, arm) in arms.iter().enumerate() {
            let (param_tys, ret_ty) = match resolve_effect_op(self.db, &resolver, &arm.path) {
                Ok((effect, op)) => {
                    self.effects.arms.insert((tgt_expr, idx), (effect, op));
                    let sig = effect.op_sig(self.db, op);
                    (sig.params, sig.ret)
                }
                Err(_) => (Vec::new(), error_type()),
            };
            for (param_idx, &pat) in arm.params.iter().enumerate() {
                let ty = param_tys.get(param_idx).cloned().unwrap_or_else(error_type);
                self.infer_pat(pat, ty);
            }

            // An arm is its own little function: a loop around the `handle`
            // is not one a `break` inside the arm could leave, and `resume`
            // refers to this arm's operation.
            let outer_loop = self.active_loop.take();
            let outer_arm = self.active_arm_ret.replace(ret_ty);
            self.infer_expr_coerce(arm.body, &Expectation::has_type(result_ty.clone()));
            self.active_arm_ret = outer_arm;
            self.active_loop = outer_loop;
        }

        result_ty
    }

    pub(super) fn infer_resume(&mut self, tgt_expr: ExprId, value: Option<ExprId>) -> Ty {
        match (self.active_arm_ret.clone(), value) {
            (Some(ret), Some(value)) => {
                self.infer_expr_coerce(value, &Expectation::has_type(ret));
            }
            (Some(ret), None) => {
                if !ret.is_empty() && !ret.is_unknown() {
                    self.diagnostics.push(InferenceDiagnostic::MismatchedTypes {
                        expected: ret,
                        found: Ty::unit(),
                        id: tgt_expr,
                    });
                }
            }
            // Outside every arm: the effect validator reports the placement;
            // the operand is still checked for its own errors.
            (None, Some(value)) => {
                self.infer_expr(value, &Expectation::none());
            }
            (None, None) => {}
        }
        TyKind::Never.intern()
    }
}

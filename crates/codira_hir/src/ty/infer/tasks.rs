//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Type inference for `scope`, `spawn`, `channel(..)`, `<-` and `ch <- v`
//!   (see `crate::expr::tasks` for the model).
//!
//! * `scope { body }` has the type of its body.
//! * `spawn f(args)` type-checks the call as written and is `()`: the task's
//!   result is discarded, results travel through channels.
//! * `channel(capacity)` takes a `usize` and is a `Channel[T]` whose `T` the
//!   uses decide (a send, a receive, an annotation, a parameter).
//! * `ch <- v` checks `v` against the channel's element type and is `()`;
//!   `<-ch` has the element type.
use super::{error_type, Expectation, InferenceDiagnostic, InferenceResultBuilder};
use crate::{
    expr::ExprId,
    ty::{IntTy, Ty, TyKind},
    HirDisplay,
};

impl InferenceResultBuilder<'_> {
    pub(super) fn infer_spawn(&mut self, call: ExprId) -> Ty {
        self.infer_expr(call, &Expectation::none());
        Ty::unit()
    }

    pub(super) fn infer_channel_new(&mut self, tgt_expr: ExprId, args: &[ExprId]) -> Ty {
        if args.len() != 1 {
            self.diagnostics
                .push(InferenceDiagnostic::ParameterCountMismatch {
                    id: tgt_expr,
                    found: args.len(),
                    expected: 1,
                });
        }
        let usize_ty = TyKind::Int(IntTy::usize()).intern();
        if let Some(&capacity) = args.first() {
            self.infer_expr_coerce(capacity, &Expectation::has_type(usize_ty));
        }
        for &extra in args.iter().skip(1) {
            self.infer_expr(extra, &Expectation::none());
        }
        let element = self.type_variables.new_type_var();
        TyKind::Channel(element).intern()
    }

    /// The element type of the channel `channel` evaluates to, reporting an
    /// operand that is not a channel.
    fn channel_element(&mut self, channel: ExprId, operator: &str) -> Ty {
        let ty = self.infer_expr(channel, &Expectation::none());
        let ty = self.normalize(&ty);
        match ty.interned() {
            TyKind::Channel(element) => element.clone(),
            TyKind::Unknown => error_type(),
            TyKind::InferenceVar(super::InferTy::Type(_)) => {
                let element = self.type_variables.new_type_var();
                let unified = self.unify(&ty, &TyKind::Channel(element.clone()).intern());
                debug_assert!(unified, "a fresh type variable unifies with anything");
                element
            }
            _ => {
                self.diagnostics.push(InferenceDiagnostic::ExprError {
                    id: channel.into(),
                    message: format!(
                        "`{operator}` needs a channel, but this is a `{}`",
                        ty.display(self.db)
                    ),
                });
                error_type()
            }
        }
    }

    pub(super) fn infer_channel_send(
        &mut self,
        _tgt_expr: ExprId,
        channel: ExprId,
        value: ExprId,
    ) -> Ty {
        let element = self.channel_element(channel, "<-");
        self.infer_expr_coerce(value, &Expectation::has_type(element));
        Ty::unit()
    }

    pub(super) fn infer_channel_receive(&mut self, _tgt_expr: ExprId, channel: ExprId) -> Ty {
        self.channel_element(channel, "<-")
    }
}

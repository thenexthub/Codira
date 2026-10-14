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
//! * `select { .. }`: each arm's operation is typed like the operator it is
//!   written with, a receive arm's binding has the channel's element type, and
//!   the arms' bodies share one type, which is the `select`'s.
use super::{error_type, Expectation, InferenceDiagnostic, InferenceResultBuilder};
use crate::{
    code_model::Session,
    expr::{ExprId, SelectArm, SelectOp},
    ty::{IntTy, Substitution, Ty, TyKind},
    HirDisplay, Name,
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
    /// operand that is not a channel. On a session endpoint it is the
    /// payload the session checker found the protocol to expect at `op`,
    /// the send or receive itself (`None` in a `select`, which waits on
    /// channels only).
    fn channel_element(&mut self, op: Option<ExprId>, channel: ExprId, operator: &str) -> Ty {
        let ty = self.infer_expr(channel, &Expectation::none());
        let ty = self.normalize(&ty);
        match ty.interned() {
            TyKind::Channel(element) => element.clone(),
            TyKind::Session(..) => {
                if let Some(op) = op {
                    self.db
                        .session_flow(self.body.owner())
                        .payloads
                        .get(&op)
                        .cloned()
                        .unwrap_or_else(error_type)
                } else {
                    self.diagnostics.push(InferenceDiagnostic::ExprError {
                        id: channel.into(),
                        message: "a `select` waits on channels, not on session endpoints"
                            .to_string(),
                    });
                    error_type()
                }
            }
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
        tgt_expr: ExprId,
        channel: ExprId,
        value: ExprId,
    ) -> Ty {
        let element = self.channel_element(Some(tgt_expr), channel, "<-");
        self.infer_expr_coerce(value, &Expectation::has_type(element));
        Ty::unit()
    }

    pub(super) fn infer_channel_receive(&mut self, tgt_expr: ExprId, channel: ExprId) -> Ty {
        self.channel_element(Some(tgt_expr), channel, "<-")
    }

    pub(super) fn infer_select(&mut self, arms: &[SelectArm], expected: &Expectation) -> Ty {
        for arm in arms {
            match arm.op {
                SelectOp::Receive { channel } => {
                    let element = self.channel_element(None, channel, "<-");
                    if let Some(binding) = arm.binding {
                        self.infer_pat(binding, element);
                    }
                }
                SelectOp::Send { channel, value } => {
                    let element = self.channel_element(None, channel, "<-");
                    self.infer_expr_coerce(value, &Expectation::has_type(element));
                }
                SelectOp::Invalid { expr } => {
                    self.infer_expr(expr, &Expectation::none());
                }
                SelectOp::Default => {}
            }
            // A binding on anything but a receive is reported by the task
            // validator; it still needs a type.
            if let (
                Some(binding),
                SelectOp::Send { .. } | SelectOp::Invalid { .. } | SelectOp::Default,
            ) = (arm.binding, arm.op)
            {
                self.infer_pat(binding, error_type());
            }
        }
        let bodies: Vec<ExprId> = arms.iter().map(|arm| arm.body).collect();
        self.infer_alternatives(&bodies, expected, "`select` arm")
    }

    /// The type of one of several bodies of which exactly one runs: all
    /// must have one type, which is the result's (unit when there are none).
    fn infer_alternatives(&mut self, bodies: &[ExprId], expected: &Expectation, what: &str) -> Ty {
        let mut result: Option<Ty> = None;
        for &body in bodies {
            let ty = self.infer_expr_coerce(body, expected);
            result = Some(match result {
                None => ty,
                Some(previous) => {
                    if let Some(merged) = self.coerce_merge_branch(&previous, &ty) {
                        merged
                    } else {
                        self.diagnostics.push(InferenceDiagnostic::ExprError {
                            id: body.into(),
                            message: format!(
                                "every {what} must have the same type: this one is a `{}`, an \
                                 earlier one a `{}`",
                                ty.display(self.db),
                                previous.display(self.db)
                            ),
                        });
                        previous
                    }
                }
            });
        }
        result.unwrap_or_else(Ty::unit)
    }

    /// `offer endpoint { .. } or { .. }`.
    pub(super) fn infer_offer(
        &mut self,
        endpoint: ExprId,
        branches: &[ExprId],
        expected: &Expectation,
    ) -> Ty {
        let ty = self.infer_expr(endpoint, &Expectation::none());
        let ty = self.normalize(&ty);
        if !matches!(ty.interned(), TyKind::Session(..) | TyKind::Unknown) {
            self.diagnostics.push(InferenceDiagnostic::ExprError {
                id: endpoint.into(),
                message: format!(
                    "`offer` needs a session endpoint, but this is a `{}`",
                    ty.display(self.db)
                ),
            });
        }
        self.infer_alternatives(branches, expected, "`offer` branch")
    }

    /// `S.open()`, which makes the two endpoints of session `S`. Any other
    /// method on a session's name is reported here.
    pub(super) fn infer_session_open(
        &mut self,
        tgt_expr: ExprId,
        session: Session,
        args: &[ExprId],
        method_name: &Name,
    ) -> Ty {
        if method_name.as_str() != Some("open") || !args.is_empty() {
            self.diagnostics.push(InferenceDiagnostic::ExprError {
                id: tgt_expr.into(),
                message: format!(
                    "a session has one operation, `{}.open()`, which creates its two endpoints",
                    session.name(self.db)
                ),
            });
        }
        for &arg in args {
            self.infer_expr(arg, &Expectation::none());
        }
        let endpoints: Substitution = [
            TyKind::Session(session, false).intern(),
            TyKind::Session(session, true).intern(),
        ]
        .into_iter()
        .collect();
        TyKind::Tuple(2, endpoints).intern()
    }

    /// A method call on a session endpoint: only `ep.choose(branch)`, which
    /// the session checker validates. Its value is `()`.
    pub(super) fn infer_endpoint_method(&mut self, args: &[ExprId]) -> Ty {
        for &arg in args {
            self.infer_expr(arg, &Expectation::none());
        }
        Ty::unit()
    }
}

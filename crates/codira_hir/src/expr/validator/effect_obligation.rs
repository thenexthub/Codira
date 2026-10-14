//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 21, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
//!
//! Effect-obligation checking (see `spec/LANGUAGE_SPEC.md` section 6):
//! "a function's `uses Effect, ...` clause is part of its type -- callers
//! must either be inside a matching `handle` block or themselves declare
//! `uses` and propagate the obligation".
//!
//! Both halves are checked, for every way an effect can be needed:
//!
//! * a call (`Expr::Call`, `Expr::MethodCall`) to a function that declares
//!   `uses E`;
//! * a `perform E.op(..)`.
//!
//! Either is satisfied by `uses E` on the enclosing function, or by an
//! enclosing `handle` that has an arm for `E`. Only the `handle`'s *body* is
//! covered: its arms run in the context of the `handle` expression itself,
//! so an arm that performs `E` needs an outer handler (or `uses E`) just
//! like code next to the `handle` would. Effects are compared by identity
//! (the resolved effect), not by name.
use super::ExprValidator;
use crate::{
    code_model::Effect,
    diagnostics::{EffectError, UndeclaredEffect},
    resolve::{resolver_for_expr, ValueNs},
    Expr, ExprId, HasSource,
};

impl ExprValidator<'_> {
    /// Checks every effect the body needs against the function's own `uses`
    /// clause and the `handle` expressions around it.
    pub(super) fn validate_effect_obligations(&self, sink: &mut crate::DiagnosticSink<'_>) {
        if !self.body.exprs().any(|(_, e)| {
            matches!(
                e,
                Expr::Call { .. } | Expr::MethodCall { .. } | Expr::Perform { .. }
            )
        }) {
            return;
        }
        let declared = self.func.effects(self.db);
        let mut handled = Vec::new();
        self.check_obligations(self.body.body_expr(), &declared, &mut handled, sink);
    }

    fn check_obligations(
        &self,
        expr: ExprId,
        declared: &[Effect],
        handled: &mut Vec<Effect>,
        sink: &mut crate::DiagnosticSink<'_>,
    ) {
        match &self.body[expr] {
            Expr::Handle { body, arms } => {
                let depth = handled.len();
                for idx in 0..arms.len() {
                    if let Some((effect, _)) = self.infer.handler_arm_target(expr, idx) {
                        handled.push(effect);
                    }
                }
                self.check_obligations(*body, declared, handled, sink);
                handled.truncate(depth);
                for arm in arms {
                    self.check_obligations(arm.body, declared, handled, sink);
                }
                return;
            }
            Expr::Perform { .. } => {
                if let Some((effect, op)) = self.infer.perform_target(expr) {
                    if !declared.contains(&effect) && !handled.contains(&effect) {
                        if let Some(node) = self.expr_node(expr) {
                            let name = effect.name(self.db);
                            let op = effect.data(self.db).ops[op].name.clone();
                            sink.push(EffectError {
                                file: self.func.source(self.db).file_id,
                                node,
                                message: format!(
                                    "`perform {name}.{op}` needs a handler for `{name}`: wrap it \
                                     in a `handle` with a `{name}` arm, or declare `uses {name}` \
                                     on `{}` to pass the obligation to its callers",
                                    self.func.name(self.db)
                                ),
                            });
                        }
                    }
                }
            }
            Expr::Call { callee, .. } => {
                let callee_fn = match &self.body[*callee] {
                    Expr::Path(path) => resolver_for_expr(self.db, self.body.owner(), *callee)
                        .resolve_path_as_value_fully(self.db, path)
                        .and_then(|(ns, _)| match ns {
                            ValueNs::FunctionId(id) => Some(crate::Function::from(id)),
                            _ => None,
                        }),
                    _ => None,
                };
                if let Some(callee_fn) = callee_fn {
                    self.check_call_effects(expr, callee_fn, declared, handled, sink);
                }
            }
            Expr::MethodCall { .. } => {
                if let Some(id) = self.infer.method_resolutions.get(&expr).copied() {
                    self.check_call_effects(expr, id.into(), declared, handled, sink);
                }
            }
            _ => {}
        }
        self.body[expr].walk_child_exprs(|child| {
            self.check_obligations(child, declared, handled, sink);
        });
    }

    fn check_call_effects(
        &self,
        call: ExprId,
        callee: crate::Function,
        declared: &[Effect],
        handled: &[Effect],
        sink: &mut crate::DiagnosticSink<'_>,
    ) {
        for effect in callee.effects(self.db) {
            if declared.contains(&effect) || handled.contains(&effect) {
                continue;
            }
            let Some(syntax) = self.expr_node(call) else {
                continue;
            };
            sink.push(UndeclaredEffect {
                file: self.func.source(self.db).file_id,
                call: syntax,
                effect: effect.name(self.db),
                callee: callee.data(self.db).name().clone(),
            });
        }
    }

    /// The syntax node an expression was lowered from, if it has one.
    pub(super) fn expr_node(&self, expr: ExprId) -> Option<codira_syntax::SyntaxNodePtr> {
        self.body_source_map.expr_syntax(expr).map(|sp| {
            sp.value
                .either(|it| it.syntax_node_ptr(), |it| it.syntax_node_ptr())
        })
    }
}

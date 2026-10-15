//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The rules closures follow beyond typing (which, together with capture
//!   analysis, is `ty::infer::closures`).
//!
//! # Rules
//!
//! * A closure has no `self` parameter; it captures `self` like any other
//!   binding when its body mentions it.
//! * A closure performs no effects: its body contains no `perform`, no
//!   `handle`, and no call to a function that `uses` an effect. A function
//!   value carries no evidence for handlers (see
//!   `codira_codegen::ir::body::effects`), so there would be nothing to perform
//!   through. A function that uses effects cannot become a function value
//!   either; inference reports that.
use codira_syntax::{ast, AstNode};

use super::ExprValidator;
use crate::{
    diagnostics::{DiagnosticSink, EffectError},
    CallableDef, Expr, ExprId, Function, HasSource,
};

impl ExprValidator<'_> {
    /// Checks the rules described in the module documentation.
    pub(super) fn validate_closures(&self, sink: &mut DiagnosticSink<'_>) {
        for (expr, _) in self.infer.closures() {
            if self.closure_has_self_param(expr) {
                self.push_closure_error(
                    expr,
                    "a closure has no `self` parameter; mention `self` in its body to capture \
                     it"
                    .to_string(),
                    sink,
                );
            }
            let Expr::Closure { body, .. } = &self.body[expr] else {
                continue;
            };
            self.validate_closure_body(*body, sink);
        }
    }

    fn closure_has_self_param(&self, expr: ExprId) -> bool {
        let Some(source) = self.body_source_map.expr_syntax(expr) else {
            return false;
        };
        let Some(ptr) = source.value.left() else {
            return false;
        };
        let root = self.func.source(self.db).value.syntax().ancestors().last();
        let Some(root) = root else {
            return false;
        };
        let node = ptr.to_node(&root);
        let Some(closure) = ast::ClosureExpr::cast(node.syntax().clone()) else {
            return false;
        };
        closure
            .param_list()
            .and_then(|list| list.self_param())
            .is_some()
    }

    /// Reports every effect operation in a closure body. Nested closures are
    /// checked as closures of their own.
    fn validate_closure_body(&self, expr: ExprId, sink: &mut DiagnosticSink<'_>) {
        let message = match &self.body[expr] {
            Expr::Closure { .. } => return,
            Expr::Perform { .. } => Some(
                "a closure cannot `perform` an effect: a function value carries no \
                      effect handlers"
                    .to_string(),
            ),
            Expr::Handle { .. } => Some(
                "a closure cannot contain a `handle` expression yet; handle the effect in a \
                 named function and call that"
                    .to_string(),
            ),
            Expr::Call { callee, .. } => match self.infer[*callee].as_callable_def() {
                Some(CallableDef::Function(function)) => self.uses_effects(function),
                _ => None,
            },
            Expr::MethodCall { .. } => self
                .infer
                .method_resolution(expr)
                .and_then(|function| self.uses_effects(Function::from(function))),
            _ => None,
        };
        if let Some(message) = message {
            self.push_closure_error(expr, message, sink);
        }
        self.body[expr].walk_child_exprs(|child| self.validate_closure_body(child, sink));
    }

    fn uses_effects(&self, function: Function) -> Option<String> {
        if function.effects(self.db).is_empty() {
            return None;
        }
        Some(format!(
            "a closure cannot call `{}`, which uses effects: a function value carries no effect \
             handlers",
            function.name(self.db)
        ))
    }

    fn push_closure_error(&self, expr: ExprId, message: String, sink: &mut DiagnosticSink<'_>) {
        if let Some(node) = self.expr_node(expr) {
            sink.push(EffectError {
                file: self.func.source(self.db).file_id,
                node,
                message,
            });
        }
    }
}

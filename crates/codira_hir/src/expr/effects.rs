//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Lowers the algebraic-effect expressions -- `perform`, `handle` and
//!   `resume` -- from syntax to HIR.
//!
//! # The model
//!
//! An effect (`effect Ask { func ask() -> i32 }`) declares operations. A
//! function that may perform them says so with `uses Ask`. `perform
//! Ask.ask()` invokes the innermost enclosing handler for `Ask`, which is
//! installed by `handle body { Ask.ask() -> resume(42) }`.
//!
//! Version 1 handlers are *one-shot and tail-resumptive or aborting*
//! (`spec/EIDOS_RFC_002.md` section 3.4):
//!
//! * `resume(v)` in tail position of an arm returns `v` to the `perform`, and
//!   the body carries on;
//! * an arm that finishes without resuming makes its own value the value of the
//!   whole `handle` expression, abandoning the rest of the body.
//!
//! That is exactly the expressive power that compiles to plain calls and
//! returns, with no stack capture: see `codira_codegen::ir::effects`.
//!
//! `resume` has no syntax node of its own: `resume(x)` parses as a call
//! whose callee is a path made of the `resume` keyword, which ordinary path
//! lowering cannot represent. It is recognised here instead.
use codira_syntax::{
    ast::{self, ArgListOwner},
    AstNode, AstPtr, SyntaxKind,
};

use super::{Expr, ExprCollector, ExprId, HandlerArm};
use crate::Path;

/// Whether `call` is `resume(..)`.
pub(super) fn is_resume_call(call: &ast::CallExpr) -> bool {
    let Some(ast::ExprKind::PathExpr(path_expr)) = call.expr().map(|e| e.kind()) else {
        return false;
    };
    let Some(path) = path_expr.path() else {
        return false;
    };
    if path.qualifier().is_some() {
        return false;
    }
    path.segment()
        .and_then(|segment| segment.syntax().first_child_or_token())
        .is_some_and(|token| token.kind() == SyntaxKind::RESUME_KW)
}

impl ExprCollector<'_> {
    /// `resume(value)` / `resume()`. More than one argument is a type error
    /// reported by inference (the operation's return type is one value), so
    /// extra arguments are still lowered for their own diagnostics but only
    /// the first is kept as the value.
    pub(super) fn collect_resume(
        &mut self,
        call: &ast::CallExpr,
        syntax_ptr: AstPtr<ast::Expr>,
    ) -> ExprId {
        let mut args = call
            .arg_list()
            .into_iter()
            .flat_map(|list| list.args())
            .map(|arg| self.collect_expr(arg));
        let value = args.next();
        // Extra arguments are lowered so they are still checked, then
        // dropped; the effect validator reports that `resume` takes one.
        args.for_each(drop);
        self.alloc_expr(Expr::Resume { value }, syntax_ptr)
    }

    /// `perform Effect.op(args)`.
    pub(super) fn collect_perform(
        &mut self,
        perform: &ast::PerformExpr,
        syntax_ptr: AstPtr<ast::Expr>,
    ) -> ExprId {
        let path = perform.path().and_then(Path::from_ast);
        let args = perform
            .arg_list()
            .into_iter()
            .flat_map(|list| list.args())
            .map(|arg| self.collect_expr(arg))
            .collect();
        match path {
            Some(path) => self.alloc_expr(Expr::Perform { path, args }, syntax_ptr),
            None => self.alloc_expr(Expr::Missing, syntax_ptr),
        }
    }

    /// `handle body { Effect.op(params) -> arm, .. }`.
    pub(super) fn collect_handle(
        &mut self,
        handle: &ast::HandleExpr,
        syntax_ptr: AstPtr<ast::Expr>,
    ) -> ExprId {
        let body = self.collect_expr_opt(handle.expr());
        let arms = handle
            .handler_arm_list()
            .into_iter()
            .flat_map(|list| list.arms())
            .map(|arm| {
                // An arm without a path is a syntax error the parser has
                // already reported. It is kept, with an empty path that
                // resolves to nothing, so arm indices stay aligned with the
                // syntax for diagnostics.
                let path = arm.path().and_then(Path::from_ast).unwrap_or(Path {
                    kind: crate::PathKind::Plain,
                    segments: Vec::new(),
                });
                let params = arm
                    .param_list()
                    .into_iter()
                    .flat_map(|list| list.params())
                    .map(|param| self.collect_pat_opt(param.pat()))
                    .collect();
                let body = self.collect_expr_opt(arm.body());
                HandlerArm { path, params, body }
            })
            .collect();
        self.alloc_expr(Expr::Handle { body, arms }, syntax_ptr)
    }
}

/// Why an `Effect.op` path did not resolve.
#[derive(Debug, Clone, PartialEq, Eq)]
pub(crate) enum EffectOpError {
    /// The path has a single segment: there is no effect to look the
    /// operation up in.
    NotQualified,
    /// The qualifier does not name an effect.
    UnknownEffect(Path),
    /// The effect has no operation of that name.
    UnknownOp {
        effect: crate::code_model::Effect,
        op: crate::Name,
    },
}

/// Resolves the `Effect.op` path of a `perform` or a handler arm to the
/// effect and the index of the operation within it.
pub(crate) fn resolve_effect_op(
    db: &dyn crate::HirDatabase,
    resolver: &crate::resolve::Resolver,
    path: &Path,
) -> Result<(crate::code_model::Effect, usize), EffectOpError> {
    let Some((op, qualifier)) = path.segments.split_last() else {
        return Err(EffectOpError::NotQualified);
    };
    if qualifier.is_empty() {
        return Err(EffectOpError::NotQualified);
    }
    let effect_path = Path {
        kind: path.kind.clone(),
        segments: qualifier.to_vec(),
    };
    let Some((effect, _)) = resolver.resolve_path_as_effect(db, &effect_path) else {
        return Err(EffectOpError::UnknownEffect(effect_path));
    };
    let effect = crate::code_model::Effect::from(effect);
    effect
        .data(db)
        .op_index(op)
        .map(|index| (effect, index))
        .ok_or_else(|| EffectOpError::UnknownOp {
            effect,
            op: op.clone(),
        })
}

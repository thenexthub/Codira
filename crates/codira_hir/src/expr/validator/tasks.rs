//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The rules that make `scope`, `spawn` and channels memory- and race-safe
//!   (see `crate::expr::tasks` for the model).
//!
//! # Rules
//!
//! * `spawn` appears inside a `scope { .. }` of the same function (a handler
//!   arm is a function of its own), so there is always a scope to join it.
//! * `spawn` takes a direct call `f(args)` to a non-generic function of the
//!   same module that declares no effects (a task has no handler to perform
//!   through), and every parameter of `f` is a scalar -- an integer, a float or
//!   `bool` -- or a `Channel[T]`. Tasks share nothing else, so they cannot race
//!   on memory.
//! * `channel(capacity)` appears only as the initializer of `let name = ..`
//!   inside a `scope`, and carries scalars. The channel belongs to that scope
//!   and is freed when it ends, after every task of the scope has finished.
//! * A channel value is only ever *used*: as the operand of `<-`, or as an
//!   argument passed to a function or to `spawn`. It is never stored, returned,
//!   copied into another binding, or made the value of a block. Bindings are
//!   lexical, so a channel cannot be named after its scope ends, and a function
//!   it is passed to cannot keep it (no return, no field, no array can hold
//!   one); a task it is passed to is joined no later than the scope that owns
//!   the channel.
use rustc_hash::FxHashSet;

use super::ExprValidator;
use crate::{
    diagnostics::{DiagnosticSink, EffectError},
    resolve::{resolver_for_expr, ValueNs},
    BuiltinFn, Expr, ExprId, HasSource, HirDisplay, Pat, Statement, Ty, TyKind,
};

/// Whether values of `ty` may cross between tasks.
fn is_scalar(ty: &Ty) -> bool {
    matches!(
        ty.interned(),
        TyKind::Int(_) | TyKind::Float(_) | TyKind::Bool
    )
}

/// Whether values of `ty` may be passed to a task.
fn is_task_argument(ty: &Ty) -> bool {
    match ty.interned() {
        TyKind::Channel(element) => is_scalar(element),
        _ => is_scalar(ty),
    }
}

impl ExprValidator<'_> {
    /// Checks the rules described in the module documentation.
    pub(super) fn validate_tasks(&self, sink: &mut DiagnosticSink<'_>) {
        let uses_tasks = self.body.exprs().any(|(id, e)| {
            matches!(
                e,
                Expr::Scope { .. }
                    | Expr::Spawn { .. }
                    | Expr::ChannelSend { .. }
                    | Expr::ChannelReceive { .. }
            ) || self.infer.builtin_call(id) == Some(BuiltinFn::Channel)
                || matches!(self.infer[id].interned(), TyKind::Channel(_))
        });
        if !uses_tasks {
            return;
        }
        let mut allowed = FxHashSet::default();
        self.validate_task_exprs(self.body.body_expr(), false, &mut allowed, sink);

        // Every channel-typed expression must be one of the uses collected
        // above.
        for (expr, _) in self.body.exprs() {
            if self.body.contracts().ghost_exprs.contains(&expr) {
                continue;
            }
            if matches!(self.infer[expr].interned(), TyKind::Channel(_)) && !allowed.contains(&expr)
            {
                self.push_task_error(
                    expr,
                    "a channel can only be used with `<-`, passed to a function or to `spawn`, \
                     or created by `let name = channel(capacity)`: it lives only as long as its \
                     `scope`, so it cannot be stored, returned or copied"
                        .to_string(),
                    sink,
                );
            }
        }
    }

    fn push_task_error(&self, expr: ExprId, message: String, sink: &mut DiagnosticSink<'_>) {
        if let Some(node) = self.expr_node(expr) {
            sink.push(EffectError {
                file: self.func.source(self.db).file_id,
                node,
                message,
            });
        }
    }

    /// Walks the executable tree. `in_scope` tells whether a `scope` of this
    /// function encloses `expr`; `allowed` collects the channel-typed
    /// expressions that are legitimate uses.
    fn validate_task_exprs(
        &self,
        expr: ExprId,
        in_scope: bool,
        allowed: &mut FxHashSet<ExprId>,
        sink: &mut DiagnosticSink<'_>,
    ) {
        let mut in_scope = in_scope;
        match &self.body[expr] {
            Expr::Scope { .. } => in_scope = true,
            Expr::Handle { body, arms } => {
                // An arm is a function of its own: a scope around the
                // `handle` cannot join a task the arm spawns.
                self.validate_task_exprs(*body, in_scope, allowed, sink);
                for arm in arms {
                    self.validate_task_exprs(arm.body, false, allowed, sink);
                }
                return;
            }
            Expr::Spawn { call } => {
                if !in_scope {
                    self.push_task_error(
                        expr,
                        "`spawn` must be inside a `scope { .. }` block: the scope is what waits \
                         for the task to finish"
                            .to_string(),
                        sink,
                    );
                }
                self.validate_spawned_call(*call, sink);
            }
            Expr::ChannelSend { channel, .. } | Expr::ChannelReceive { channel } => {
                if matches!(self.body[*channel], Expr::Path(_)) {
                    allowed.insert(*channel);
                }
            }
            Expr::Call { args, .. } | Expr::MethodCall { args, .. } => {
                for &arg in args {
                    if matches!(self.body[arg], Expr::Path(_)) {
                        allowed.insert(arg);
                    }
                }
            }
            Expr::Block { statements, .. } => {
                for statement in statements {
                    let Statement::Let {
                        pat,
                        initializer: Some(init),
                        ..
                    } = statement
                    else {
                        continue;
                    };
                    if self.infer.builtin_call(*init) != Some(BuiltinFn::Channel) {
                        continue;
                    }
                    if !matches!(self.body[*pat], Pat::Bind { .. }) {
                        continue;
                    }
                    allowed.insert(*init);
                    if !in_scope {
                        self.push_task_error(
                            *init,
                            "a channel must be created inside a `scope { .. }` block, which owns \
                             it and frees it when it ends"
                                .to_string(),
                            sink,
                        );
                    }
                    if let TyKind::Channel(element) = self.infer[*init].interned() {
                        if !is_scalar(element) && !element.is_unknown() {
                            self.push_task_error(
                                *init,
                                format!(
                                    "a channel carries integers, floats or `bool`, not `{}`: \
                                     tasks share nothing else",
                                    element.display(self.db)
                                ),
                                sink,
                            );
                        }
                    }
                }
            }
            _ => {}
        }
        self.body[expr]
            .walk_child_exprs(|child| self.validate_task_exprs(child, in_scope, allowed, sink));
    }

    /// `spawn f(args)`: a direct call to a module-local, non-generic,
    /// effect-free function whose parameters can cross between tasks.
    fn validate_spawned_call(&self, call: ExprId, sink: &mut DiagnosticSink<'_>) {
        let function = match &self.body[call] {
            Expr::Call { callee, .. } => match &self.body[*callee] {
                Expr::Path(path) => resolver_for_expr(self.db, self.body.owner(), *callee)
                    .resolve_path_as_value_fully(self.db, path)
                    .and_then(|(ns, _)| match ns {
                        ValueNs::FunctionId(id) => Some(crate::Function::from(id)),
                        _ => None,
                    }),
                _ => None,
            },
            _ => None,
        };
        let Some(function) = function else {
            self.push_task_error(
                call,
                "`spawn` takes a call to a function, `spawn f(args)`".to_string(),
                sink,
            );
            return;
        };
        let name = function.name(self.db);
        if function.module(self.db) != self.func.module(self.db)
            || function.is_extern(self.db)
            || function.data(self.db).self_param().is_some()
        {
            self.push_task_error(
                call,
                format!(
                    "`spawn` runs a function of this module, and `{name}` is not one (or is \
                     `extern` or a method)"
                ),
                sink,
            );
            return;
        }
        if function.is_generic(self.db) {
            // A task starts through a trampoline made per function, and a
            // generic function has one body per instantiation.
            self.push_task_error(
                call,
                format!("`spawn` cannot run the generic function `{name}` yet"),
                sink,
            );
            return;
        }
        if !function.effects(self.db).is_empty() {
            self.push_task_error(
                call,
                format!("`{name}` uses effects, and a task has no handler to perform them through"),
                sink,
            );
        }
        let Some(sig) = function.ty(self.db).callable_sig(self.db) else {
            return;
        };
        for param in sig.params() {
            if !is_task_argument(param) {
                self.push_task_error(
                    call,
                    format!(
                        "a task can only be given integers, floats, `bool` and channels, but \
                         `{name}` takes a `{}`: tasks share nothing else, so they cannot race",
                        param.display(self.db)
                    ),
                    sink,
                );
                break;
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::utils::tests::diagnostics;

    fn assert_clean(source: &str) {
        let diags = diagnostics(source);
        assert!(diags.is_empty(), "expected no diagnostics, got:\n{diags}");
    }

    fn assert_reports(source: &str, needle: &str) {
        let diags = diagnostics(source);
        assert!(
            diags.contains(needle),
            "expected a diagnostic containing `{needle}`, got:\n{diags}"
        );
    }

    #[test]
    fn well_formed_tasks_are_clean() {
        assert_clean(
            r"
    func produce(dest: Channel[i64], n: i64) {
        let i = 0;
        while i < n {
            dest <- i;
            i += 1;
        }
    }
    func main() -> i64 {
        scope {
            let ch = channel(4);
            spawn produce(ch, 3);
            let a = <-ch;
            let b = <-ch;
            let c = <-ch;
            a + b + c
        }
    }
    ",
        );
    }

    #[test]
    fn spawn_outside_a_scope() {
        assert_reports(
            "func work(x: i32) {}\nfunc main() { spawn work(1) }",
            "`spawn` must be inside a `scope { .. }` block",
        );
    }

    #[test]
    fn spawn_needs_a_function_call() {
        assert_reports(
            "func main() { scope { spawn 1 } }",
            "`spawn` takes a call to a function",
        );
    }

    #[test]
    fn spawned_function_takes_only_scalars_and_channels() {
        assert_reports(
            "func work(xs: [i32]) {}\nfunc main() { scope { spawn work([1]) } }",
            "a task can only be given integers, floats, `bool` and channels",
        );
    }

    #[test]
    fn spawned_function_cannot_use_effects() {
        assert_reports(
            r"
    effect Ask { func ask() -> i32 }
    func work() uses Ask { perform Ask.ask(); }
    func main() { scope { spawn work() } }
    ",
            "uses effects, and a task has no handler",
        );
    }

    #[test]
    fn channel_outside_a_scope() {
        assert_reports(
            "func main() { let ch = channel(1); ch <- 1 }",
            "a channel must be created inside a `scope { .. }` block",
        );
    }

    #[test]
    fn channel_of_non_scalars() {
        assert_reports(
            "func main() { scope { let ch = channel(1); ch <- [1] } }",
            "a channel carries integers, floats or `bool`",
        );
    }

    #[test]
    fn channel_cannot_escape() {
        assert_reports(
            "func main() { scope { let ch = channel(1); let copy = ch; copy <- 1 } }",
            "a channel can only be used with `<-`",
        );
        assert_reports(
            "func leak(ch: Channel[i32]) -> Channel[i32] { ch }",
            "a channel can only be used with `<-`",
        );
    }

    #[test]
    fn send_to_a_non_channel() {
        assert_reports("func main() { let x = 1; x <- 2 }", "`<-` needs a channel");
    }
}

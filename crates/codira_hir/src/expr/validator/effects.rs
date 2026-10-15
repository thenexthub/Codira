//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The well-formedness rules of algebraic effects that are not types: names
//!   that must resolve, handlers that must fit their effect, where `resume` may
//!   appear, and the version-1 restrictions on handler arms.
//!
//! # Rules
//!
//! * Every name in a `uses` clause names an effect.
//! * `perform E.op(..)` and every handler arm `E.op(..)` name an existing
//!   effect and one of its operations; an arm binds exactly as many parameters
//!   as the operation has, with irrefutable patterns.
//! * A `handle` implements *every* operation of each effect it handles, once: a
//!   performer reaches an operation through the handler's record, and a
//!   half-filled record would have nothing to call.
//! * `resume` appears only inside a handler arm, in tail position (the last
//!   thing the arm does, through blocks and `if` branches), with at most one
//!   argument. Version-1 handlers resume at most once and only in tail position
//!   (`spec/EIDOS_RFC_002.md` section 3.4): that is what lets an arm compile to
//!   a plain function whose `return` *is* the resumption.
//! * A handler arm does not `return` from the enclosing function: the arm is
//!   compiled as a function of its own, so there is no frame to return from. An
//!   arm that wants to end the computation produces the `handle`'s value
//!   instead.
//! * A handler arm does not use the enclosing function's local variables or
//!   parameters. Arms compile to separate functions and there are no closures
//!   yet to carry such values across; the arm sees its own parameters, its own
//!   `let`s, and module-level items. This restriction lifts when closures land.
//! * A function with a `uses` clause is not `@export("C")`: an external caller
//!   cannot supply the handlers it needs.
//! * A function with a `uses` clause is only called from its own module. Calls
//!   between modules go through the runtime's dispatch table, whose signatures
//!   are the host-visible ones, which have no place for the handlers the callee
//!   needs (`codira_codegen::ir::effects`).
use codira_syntax::{
    ast::{self, ArgListOwner},
    AstNode, SyntaxNodePtr,
};
use rustc_hash::FxHashSet;

use super::ExprValidator;
use crate::{
    code_model::Effect,
    diagnostics::{DiagnosticSink, EffectError, UnsupportedByCodegen},
    expr::effects::{resolve_effect_op, EffectOpError},
    resolve::{resolver_for_expr, ValueNs},
    Expr, ExprId, HasSource, Pat, PatId, Statement,
};

impl ExprValidator<'_> {
    /// Checks the effect rules described in the module documentation.
    pub(super) fn validate_effects(&self, sink: &mut DiagnosticSink<'_>) {
        self.validate_uses_clause(sink);
        self.validate_effect_exprs(self.body.body_expr(), None, sink);
    }

    fn push_effect_error(
        &self,
        node: SyntaxNodePtr,
        message: String,
        sink: &mut DiagnosticSink<'_>,
    ) {
        sink.push(EffectError {
            file: self.func.source(self.db).file_id,
            node,
            message,
        });
    }

    /// Every `uses` name must be an effect; a `uses` function must not be
    /// `@export("C")`.
    fn validate_uses_clause(&self, sink: &mut DiagnosticSink<'_>) {
        let source = self.func.source(self.db);
        let Some(uses) = source.value.uses_clause() else {
            return;
        };
        let resolver = crate::resolve::HasResolver::resolver(self.func.id, self.db);
        for path in uses.effects() {
            let Some(name) = path
                .segment()
                .and_then(|segment| segment.name_ref())
                .map(|name_ref| crate::name::AsName::as_name(&name_ref))
            else {
                continue;
            };
            if resolver
                .resolve_path_as_effect(self.db, &crate::Path::from(name.clone()))
                .is_none()
            {
                self.push_effect_error(
                    SyntaxNodePtr::new(path.syntax()),
                    format!("cannot find effect `{name}` in this scope"),
                    sink,
                );
            }
        }
        if self.func.export_abi(self.db).is_some() {
            self.push_effect_error(
                SyntaxNodePtr::new(uses.syntax()),
                "a function with a `uses` clause cannot be `@export`ed: an external caller has \
                 no way to supply the handlers it needs"
                    .to_string(),
                sink,
            );
        }
    }

    /// Walks the executable tree. `arm` is the arm whose body encloses
    /// `expr`, if any: its tail positions and the patterns bound inside it.
    fn validate_effect_exprs(
        &self,
        expr: ExprId,
        arm: Option<&ArmContext>,
        sink: &mut DiagnosticSink<'_>,
    ) {
        match &self.body[expr] {
            Expr::Perform { path, .. } => {
                if self.infer.perform_target(expr).is_none() {
                    let resolver = resolver_for_expr(self.db, self.body.owner(), expr);
                    if let Err(err) = resolve_effect_op(self.db, &resolver, path) {
                        let node = self.perform_path_node(expr);
                        self.report_effect_op_error(node, &err, sink);
                    }
                }
            }
            Expr::Handle { body, arms } => {
                self.validate_handle(expr, sink);
                self.validate_effect_exprs(*body, arm, sink);
                for arm in arms {
                    let context = ArmContext::new(self, arm.body, &arm.params);
                    self.validate_effect_exprs(arm.body, Some(&context), sink);
                }
                return;
            }
            Expr::Resume { .. } => self.validate_resume(expr, arm, sink),
            // A `return` inside a closure leaves the closure, not the arm.
            Expr::Return { .. } if arm.is_some() && !self.is_inside_closure(expr) => {
                if let Some(node) = self.expr_node(expr) {
                    self.push_effect_error(
                        node,
                        "`return` cannot be used inside a handler arm: an arm either resumes \
                         the `perform` with `resume(..)` or produces the value of the whole \
                         `handle`"
                            .to_string(),
                        sink,
                    );
                }
            }
            Expr::Path(path) => {
                if let Some(arm) = arm {
                    let resolver = resolver_for_expr(self.db, self.body.owner(), expr);
                    if let Some((ValueNs::LocalBinding(pat), _)) =
                        resolver.resolve_path_as_value_fully(self.db, path)
                    {
                        if !arm.bound.contains(&pat) {
                            if let Some(node) = self.expr_node(expr) {
                                self.push_effect_error(
                                    node,
                                    format!(
                                        "a handler arm cannot use `{path}`, a local of the \
                                         enclosing function: arms cannot capture variables \
                                         yet; pass the value through the operation's \
                                         parameters instead"
                                    ),
                                    sink,
                                );
                            }
                        }
                    }
                }
            }
            Expr::Call { callee, .. } => self.validate_effectful_call_site(expr, *callee, sink),
            _ => {}
        }
        self.body[expr].walk_child_exprs(|child| self.validate_effect_exprs(child, arm, sink));
    }

    /// Whether `expr` is part of some closure's body.
    fn is_inside_closure(&self, expr: ExprId) -> bool {
        fn contains(body: &crate::Body, root: ExprId, target: ExprId) -> bool {
            if root == target {
                return true;
            }
            let mut found = false;
            body[root].walk_child_exprs(|child| found |= contains(body, child, target));
            found
        }
        self.infer
            .closures()
            .into_iter()
            .any(|(closure, _)| match &self.body[closure] {
                Expr::Closure { body, .. } => contains(&self.body, *body, expr),
                _ => false,
            })
    }

    /// A call to a function with a `uses` clause must stay within the
    /// caller's module (see the module documentation).
    fn validate_effectful_call_site(
        &self,
        call: ExprId,
        callee: ExprId,
        sink: &mut DiagnosticSink<'_>,
    ) {
        let Expr::Path(path) = &self.body[callee] else {
            return;
        };
        let Some((ValueNs::FunctionId(id), _)) =
            resolver_for_expr(self.db, self.body.owner(), callee)
                .resolve_path_as_value_fully(self.db, path)
        else {
            return;
        };
        let callee_fn = crate::Function::from(id);
        if callee_fn.effects(self.db).is_empty()
            || callee_fn.module(self.db) == self.func.module(self.db)
        {
            return;
        }
        if let Some(node) = self.expr_node(call) {
            sink.push(UnsupportedByCodegen {
                file: self.func.source(self.db).file_id,
                node,
                what: format!(
                    "a call to `{}`, which has a `uses` clause, from another module",
                    callee_fn.name(self.db)
                ),
                because: "calls between modules go through the runtime's dispatch table, whose \
                          host-visible signatures have no place for the handlers the callee \
                          needs; call it from its own module"
                    .to_string(),
            });
        }
    }

    /// Checks that a `handle`'s arms fit the operations they implement and
    /// that every handled effect is implemented in full.
    fn validate_handle(&self, handle: ExprId, sink: &mut DiagnosticSink<'_>) {
        let Expr::Handle { arms, .. } = &self.body[handle] else {
            return;
        };
        let syntax_arms: Vec<ast::HandlerArm> = self
            .handle_syntax(handle)
            .and_then(|h| h.handler_arm_list())
            .map(|list| list.arms().collect())
            .unwrap_or_default();
        let arm_node = |idx: usize| {
            syntax_arms
                .get(idx)
                .map(|arm| SyntaxNodePtr::new(arm.syntax()))
        };
        let arm_path_node = |idx: usize| {
            syntax_arms
                .get(idx)
                .and_then(ast::HandlerArm::path)
                .map(|path| SyntaxNodePtr::new(path.syntax()))
        };

        let resolver = resolver_for_expr(self.db, self.body.owner(), handle);
        let mut seen: Vec<(Effect, usize)> = Vec::new();
        for (idx, arm) in arms.iter().enumerate() {
            let Some((effect, op)) = self.infer.handler_arm_target(handle, idx) else {
                if let (Err(err), Some(node)) = (
                    resolve_effect_op(self.db, &resolver, &arm.path),
                    arm_path_node(idx),
                ) {
                    self.report_effect_op_error(Some(node), &err, sink);
                }
                continue;
            };
            let data = effect.data(self.db);
            let op_name = &data.ops[op].name;
            let effect_name = &data.name;
            if seen.contains(&(effect, op)) {
                if let Some(node) = arm_path_node(idx) {
                    self.push_effect_error(
                        node,
                        format!("`{effect_name}.{op_name}` is already handled by an earlier arm"),
                        sink,
                    );
                }
            } else {
                seen.push((effect, op));
            }
            let expected = data.ops[op].params.len();
            if arm.params.len() != expected {
                if let Some(node) = arm_node(idx) {
                    self.push_effect_error(
                        node,
                        format!(
                            "this arm binds {} parameter(s) but `{effect_name}.{op_name}` has \
                             {expected}",
                            arm.params.len()
                        ),
                        sink,
                    );
                }
            }
            for &param in &arm.params {
                if !self.is_irrefutable(param) {
                    if let Some(node) = arm_node(idx) {
                        self.push_effect_error(
                            node,
                            "a handler arm's parameters must be names, `_` or tuples of them"
                                .to_string(),
                            sink,
                        );
                    }
                    break;
                }
            }
        }

        // Every operation of every handled effect needs an arm.
        let mut effects: Vec<Effect> = Vec::new();
        for (effect, _) in &seen {
            if !effects.contains(effect) {
                effects.push(*effect);
            }
        }
        for effect in effects {
            let data = effect.data(self.db);
            let missing: Vec<String> = data
                .ops
                .iter()
                .enumerate()
                .filter(|(idx, _)| !seen.contains(&(effect, *idx)))
                .map(|(_, op)| format!("`{}.{}`", data.name, op.name))
                .collect();
            if missing.is_empty() {
                continue;
            }
            if let Some(node) = self.expr_node(handle) {
                self.push_effect_error(
                    node,
                    format!(
                        "this `handle` handles `{}` but has no arm for {}: a handler implements \
                         every operation of the effects it handles",
                        data.name,
                        missing.join(", ")
                    ),
                    sink,
                );
            }
        }
    }

    fn validate_resume(
        &self,
        expr: ExprId,
        arm: Option<&ArmContext>,
        sink: &mut DiagnosticSink<'_>,
    ) {
        let Some(node) = self.expr_node(expr) else {
            return;
        };
        let Some(arm) = arm else {
            self.push_effect_error(
                node,
                "`resume` can only be used inside a handler arm".to_string(),
                sink,
            );
            return;
        };
        if !arm.tails.contains(&expr) {
            self.push_effect_error(
                node.clone(),
                "`resume` must be the last thing a handler arm does: handlers resume at most \
                 once, in tail position"
                    .to_string(),
                sink,
            );
        }
        let arg_count = self
            .resume_syntax(expr)
            .and_then(|call| call.arg_list())
            .map_or(0, |list| list.args().count());
        if arg_count > 1 {
            self.push_effect_error(
                node,
                format!("`resume` takes at most one value, but {arg_count} were given"),
                sink,
            );
        }
    }

    fn report_effect_op_error(
        &self,
        node: Option<SyntaxNodePtr>,
        err: &EffectOpError,
        sink: &mut DiagnosticSink<'_>,
    ) {
        let Some(node) = node else {
            return;
        };
        let message = match err {
            EffectOpError::NotQualified => {
                "expected `Effect.operation`: an operation is always named through its effect"
                    .to_string()
            }
            EffectOpError::UnknownEffect(path) => {
                format!("cannot find effect `{path}` in this scope")
            }
            EffectOpError::UnknownOp { effect, op } => {
                format!("effect `{}` has no operation `{op}`", effect.name(self.db))
            }
        };
        self.push_effect_error(node, message, sink);
    }

    /// Whether a parameter pattern can be bound without a test.
    fn is_irrefutable(&self, pat: PatId) -> bool {
        match &self.body[pat] {
            Pat::Bind { .. } | Pat::Wild => true,
            Pat::Tuple(args) => args.iter().all(|&arg| self.is_irrefutable(arg)),
            Pat::Missing | Pat::Path(_) | Pat::Literal(_) | Pat::TupleStruct { .. } => false,
        }
    }

    fn handle_syntax(&self, expr: ExprId) -> Option<ast::HandleExpr> {
        let ptr = self.body_source_map.expr_syntax(expr)?;
        let ptr = ptr.value.left()?;
        let root = self.db.parse(self.func.source(self.db).file_id);
        ast::HandleExpr::cast(ptr.to_node(&root.syntax_node()).syntax().clone())
    }

    fn resume_syntax(&self, expr: ExprId) -> Option<ast::CallExpr> {
        let ptr = self.body_source_map.expr_syntax(expr)?;
        let ptr = ptr.value.left()?;
        let root = self.db.parse(self.func.source(self.db).file_id);
        ast::CallExpr::cast(ptr.to_node(&root.syntax_node()).syntax().clone())
    }

    fn perform_path_node(&self, expr: ExprId) -> Option<SyntaxNodePtr> {
        let ptr = self.body_source_map.expr_syntax(expr)?;
        let ptr = ptr.value.left()?;
        let root = self.db.parse(self.func.source(self.db).file_id);
        let perform = ast::PerformExpr::cast(ptr.to_node(&root.syntax_node()).syntax().clone())?;
        perform
            .path()
            .map(|path| SyntaxNodePtr::new(path.syntax()))
            .or_else(|| Some(SyntaxNodePtr::new(perform.syntax())))
    }
}

/// What the effect rules need to know about the handler arm enclosing an
/// expression.
struct ArmContext {
    /// The arm body's tail positions: where `resume` may appear.
    tails: FxHashSet<ExprId>,
    /// Every pattern bound inside the arm (its parameters and its own
    /// `let`s): the locals the arm may use.
    bound: FxHashSet<PatId>,
}

impl ArmContext {
    fn new(validator: &ExprValidator<'_>, body: ExprId, params: &[PatId]) -> Self {
        let mut context = ArmContext {
            tails: FxHashSet::default(),
            bound: FxHashSet::default(),
        };
        context.collect_tails(validator, body);
        for &param in params {
            context.bind(validator, param);
        }
        context.collect_bound(validator, body);
        context
    }

    fn collect_tails(&mut self, validator: &ExprValidator<'_>, expr: ExprId) {
        self.tails.insert(expr);
        match &validator.body[expr] {
            Expr::Block {
                tail: Some(tail), ..
            } => self.collect_tails(validator, *tail),
            Expr::If {
                then_branch,
                else_branch,
                ..
            } => {
                self.collect_tails(validator, *then_branch);
                if let Some(else_branch) = else_branch {
                    self.collect_tails(validator, *else_branch);
                }
            }
            // A `handle` in tail position: its body's value is the arm's
            // value. (Its own arms are separate arms with their own tails.)
            Expr::Handle { body, .. } => self.collect_tails(validator, *body),
            _ => {}
        }
    }

    fn bind(&mut self, validator: &ExprValidator<'_>, pat: PatId) {
        self.bound.insert(pat);
        validator.body[pat].walk_child_pats(|child| self.bind(validator, child));
    }

    fn collect_bound(&mut self, validator: &ExprValidator<'_>, expr: ExprId) {
        match &validator.body[expr] {
            Expr::Block { statements, .. } => {
                for statement in statements {
                    if let Statement::Let { pat, .. } = statement {
                        self.bind(validator, *pat);
                    }
                }
            }
            Expr::Handle { arms, .. } => {
                for arm in arms {
                    for &param in &arm.params {
                        self.bind(validator, param);
                    }
                }
            }
            Expr::Closure { params, .. } => {
                for &(param, _) in params {
                    self.bind(validator, param);
                }
            }
            Expr::Match { arms, .. } => {
                for arm in arms {
                    self.bind(validator, arm.pat);
                }
            }
            _ => {}
        }
        validator.body[expr].walk_child_exprs(|child| self.collect_bound(validator, child));
    }
}

#[cfg(test)]
mod tests {
    use crate::utils::tests::diagnostics;

    const EFFECTS: &str = r"
    effect Ask {
        func ask(x: i32) -> i32
    }
    effect Log {
        func log(x: i32)
        func flush()
    }
    ";

    fn check(body: &str) -> String {
        diagnostics(&format!("{EFFECTS}\n{body}"))
    }

    fn assert_clean(body: &str) {
        let diags = check(body);
        assert!(diags.is_empty(), "expected no diagnostics, got:\n{diags}");
    }

    fn assert_reports(body: &str, needle: &str) {
        let diags = check(body);
        assert!(
            diags.contains(needle),
            "expected a diagnostic containing `{needle}`, got:\n{diags}"
        );
    }

    #[test]
    fn well_formed_handlers_are_clean() {
        assert_clean(
            r"
    func get(x: i32) uses Ask -> i32 { perform Ask.ask(x) }
    func main() -> i32 {
        handle get(1) {
            Ask.ask(v) -> if v > 0 { resume(v) } else { 0 }
        }
    }
    func logs() -> i32 {
        handle { perform Log.log(1); perform Log.flush(); 3 } {
            Log.log(v) -> { let w = v; resume() },
            Log.flush() -> resume(),
        }
    }
    ",
        );
    }

    #[test]
    fn perform_without_handler_or_uses() {
        assert_reports(
            "func f() -> i32 { perform Ask.ask(1) }",
            "`perform Ask.ask` needs a handler for `Ask`",
        );
    }

    #[test]
    fn call_inside_a_matching_handle_needs_no_uses() {
        assert_clean(
            r"
    func get() uses Ask -> i32 { perform Ask.ask(1) }
    func main() -> i32 { handle get() { Ask.ask(v) -> resume(v) } }
    ",
        );
    }

    #[test]
    fn call_outside_any_handle_still_needs_uses() {
        assert_reports(
            r"
    func get() uses Ask -> i32 { perform Ask.ask(1) }
    func main() -> i32 { get() }
    ",
            "call to `get` requires effect `Ask`",
        );
    }

    #[test]
    fn an_arm_is_not_covered_by_its_own_handle() {
        assert_reports(
            r"
    func main() -> i32 {
        handle 1 { Ask.ask(v) -> resume(perform Ask.ask(v)) }
    }
    ",
            "`perform Ask.ask` needs a handler for `Ask`",
        );
    }

    #[test]
    fn resume_outside_an_arm() {
        assert_reports(
            "func f() -> i32 { resume(1) }",
            "`resume` can only be used inside a handler arm",
        );
    }

    #[test]
    fn resume_not_in_tail_position() {
        assert_reports(
            r"
    func main() -> i32 {
        handle 1 { Ask.ask(v) -> { resume(v); 2 } }
    }
    ",
            "`resume` must be the last thing a handler arm does",
        );
    }

    #[test]
    fn arm_capturing_a_local() {
        assert_reports(
            r"
    func main(n: i32) -> i32 {
        let k = 2;
        handle 1 { Ask.ask(v) -> resume(v + k) }
    }
    ",
            "a handler arm cannot use `k`",
        );
        assert_reports(
            r"
    func main(n: i32) -> i32 {
        handle 1 { Ask.ask(v) -> resume(n) }
    }
    ",
            "a handler arm cannot use `n`",
        );
    }

    #[test]
    fn unknown_effect_in_uses() {
        assert_reports(
            "func f() uses Nope -> i32 { 1 }",
            "cannot find effect `Nope` in this scope",
        );
    }

    #[test]
    fn unknown_effect_and_operation() {
        assert_reports(
            "func f() -> i32 { perform Nope.ask(1) }",
            "cannot find effect `Nope` in this scope",
        );
        assert_reports(
            "func f() uses Ask -> i32 { perform Ask.tell(1) }",
            "effect `Ask` has no operation `tell`",
        );
    }

    #[test]
    fn handler_must_implement_every_operation() {
        assert_reports(
            "func f() -> i32 { handle 1 { Log.log(v) -> resume() } }",
            "has no arm for `Log.flush`",
        );
    }

    #[test]
    fn arm_parameter_count_must_match() {
        assert_reports(
            "func f() -> i32 { handle 1 { Ask.ask(a, b) -> 0 } }",
            "this arm binds 2 parameter(s) but `Ask.ask` has 1",
        );
    }

    #[test]
    fn return_inside_an_arm() {
        assert_reports(
            "func f() -> i32 { handle 1 { Ask.ask(v) -> return 3 } }",
            "`return` cannot be used inside a handler arm",
        );
    }

    #[test]
    fn effect_is_not_a_type() {
        let diags = check("func f(x: Ask) -> i32 { 1 }");
        assert!(
            !diags.is_empty(),
            "an effect used as a type must be rejected"
        );
    }

    #[test]
    fn perform_argument_types_are_checked() {
        let diags = check("func f() uses Ask -> i32 { perform Ask.ask(true) }");
        assert!(diags.contains("mismatched type"), "{diags}");
    }
}

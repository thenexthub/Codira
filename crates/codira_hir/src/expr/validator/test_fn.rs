//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - The shape a `@test` function must have for `codira test` to run it.
//!
//! The runner calls a test from the host with no arguments, through the
//! assembly's exported dispatch table, so a test must:
//!
//! * take no parameters -- there is nothing to pass;
//! * return `()` (passes unless it traps or aborts) or `bool` (passes when it
//!   returns `true`) -- the only results the runner can interpret;
//! * be `public` -- only exported functions are reachable from the host;
//! * not be generic -- there is no instantiation to call.

use codira_syntax::{ast::NameOwner, AstNode, SyntaxNodePtr};

use super::ExprValidator;
use crate::{
    diagnostics::{DiagnosticSink, InvalidTestFunction},
    HasSource, HasVisibility, TyKind,
};

impl ExprValidator<'_> {
    pub(super) fn validate_test_function(&self, sink: &mut DiagnosticSink<'_>) {
        if !self.func.is_test(self.db) {
            return;
        }
        let source = self.func.source(self.db);
        let node = source.value.name().map_or_else(
            || SyntaxNodePtr::new(source.value.syntax()),
            |name| SyntaxNodePtr::new(name.syntax()),
        );
        let mut push = |reason: &str| {
            sink.push(InvalidTestFunction {
                file: source.file_id,
                node: node.clone(),
                reason: reason.to_string(),
            });
        };

        let data = self.func.data(self.db);
        if !data.params().is_empty() || data.self_param().is_some() {
            push("it takes parameters, and the test runner has nothing to pass");
        }
        if !data.generic_params().is_empty() {
            push("it is generic, and there is no instantiation to call");
        }
        let ret = self.func.ret_type(self.db);
        let returns_ok = ret.is_empty() || matches!(ret.interned(), TyKind::Bool);
        if !returns_ok {
            push("it must return `()` or `bool`, the only results the test runner interprets");
        }
        if !self.func.visibility(self.db).is_externally_visible() {
            push("it must be `public`: only exported functions can be called by the test runner");
        }
    }
}

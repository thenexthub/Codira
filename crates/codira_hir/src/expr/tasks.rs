//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Lowers the structured-concurrency expressions to HIR: `scope { .. }`,
//!   `spawn f(args)`, `channel <- value` and `<-channel`.
//!
//! # The model
//!
//! A `scope` block owns tasks: `spawn f(a, b)` evaluates `a` and `b` on the
//! spawning thread and runs `f` with them on the runtime's worker pool, and
//! every task spawned in a scope has finished before control leaves the
//! scope -- by its end, `return`, `break`, or an effect handler abandoning
//! it. Nothing outlives the scope that started it, so no task can observe a
//! frame that is gone.
//!
//! Tasks share nothing but channels: a spawned function takes only scalars
//! (integers, floats, `bool`) and `Channel[T]` handles, and a channel carries
//! only scalars. A channel, created by `channel(capacity)`, belongs to the
//! scope it is created in and is freed when that scope ends, after its tasks
//! have been joined. See `expr::validator::tasks` for the rules that make
//! those lifetimes sound and `codira_runtime::tasks` for the runtime.
use codira_syntax::{ast, AstNode, AstPtr};

use super::{Expr, ExprCollector, ExprId};

impl ExprCollector<'_> {
    /// `channel <- value`. The node has no named children: its two
    /// expression children are the channel and the value, in order.
    pub(super) fn collect_channel_send(
        &mut self,
        send: &ast::ChannelSendExpr,
        syntax_ptr: AstPtr<ast::Expr>,
    ) -> ExprId {
        let mut operands = send.syntax().children().filter_map(ast::Expr::cast);
        let channel = operands.next();
        let value = operands.next();
        let channel = self.collect_expr_opt(channel);
        let value = self.collect_expr_opt(value);
        self.alloc_expr(Expr::ChannelSend { channel, value }, syntax_ptr)
    }
}

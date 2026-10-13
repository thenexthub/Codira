//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - The two rules that make erasing contracts and ghost code sound.
//!
//! Contract clauses and `ghost let` initializers are dropped before code
//! generation (they are not reachable from the body expression). That is
//! only correct if dropping them cannot change what the program does, which
//! takes two rules:
//!
//! 1. **Ghost code has no effects.** A contract clause or a ghost initializer
//!    may not assign, call, return, break or loop. (A call could have an effect
//!    the erased program would then skip; the verifier also has no way to
//!    evaluate one yet.)
//! 2. **Executable code cannot see ghost bindings.** A ghost binding has no
//!    storage after erasure, so naming one from executable code -- reading or
//!    assigning it -- is rejected.

use super::exec::Frame;
use crate::{
    code_model::Function,
    diagnostics::{DiagnosticSink, GhostRuleViolation},
    expr::BinaryOp,
    Expr, HirDatabase,
};

pub(super) fn ghost_diagnostics(
    db: &dyn HirDatabase,
    func: Function,
    sink: &mut DiagnosticSink<'_>,
) {
    let frame = Frame::new(db, func);
    let contracts = frame.body.contracts();
    if contracts.ghost_exprs.is_empty() {
        return;
    }

    let mut push = |expr, message: String| {
        if let Some(site) = frame.site(expr) {
            sink.push(GhostRuleViolation {
                file: site.file_id,
                site: site.value,
                message,
            });
        }
    };

    for (id, expr) in frame.body.exprs() {
        if contracts.ghost_exprs.contains(&id) {
            let what = match expr {
                Expr::BinaryOp {
                    op: Some(BinaryOp::Assignment { .. }),
                    ..
                } => "an assignment",
                Expr::Call { .. } | Expr::MethodCall { .. } => "a call",
                Expr::Return { .. } => "a `return`",
                Expr::Break { .. } => "a `break`",
                Expr::Loop { .. } | Expr::While { .. } => "a loop",
                _ => continue,
            };
            push(
                id,
                format!(
                    "{what} is not allowed in a contract clause or ghost code: it would be erased \
                     before code generation, so it must not have effects"
                ),
            );
        } else if let Expr::Path(path) = expr {
            if let Some(pat) = frame.resolve_local(db, id, path) {
                if contracts.ghost_pats.contains(&pat) {
                    push(
                        id,
                        format!(
                            "ghost binding `{}` used in executable code: ghost bindings exist \
                             only for verification and are erased before code generation",
                            frame.text(db, id)
                        ),
                    );
                }
            }
        }
    }
}

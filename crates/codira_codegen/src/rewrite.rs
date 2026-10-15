//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Library rewrite rules: `@rewrite(from: lhs, to: rhs)` on a function
//!   declares that `lhs == rhs` for every value of the function's parameters,
//!   and offers the left-to-right rewrite to the e-graph -- once `codira_tv`
//!   has proven it.
//!
//! ```codira
//! @rewrite(from: 3 * a + 2 * a, to: 5 * a)
//! func five(a: i64) -> i64 { 5 * a }
//! ```
//!
//! The two sides are expressions over the function's parameters: integer
//! and boolean literals, parameters, arithmetic, comparison, logical,
//! bitwise and shift operators, and parentheses. The function only names
//! the variables and their types; its body is not part of the rule. (The
//! spelling `@rewrite(lhs => rhs)` would need a `=>` token Codira's grammar
//! does not have; `from:`/`to:`, or two positional arguments, say the same.)
//!
//! # Proven before used
//!
//! Every rule is handed to `codira_tv::Validator` before anything uses it,
//! when the library is built:
//!
//! * `Proven` with no overflow caveat -- admitted.
//! * `Proven` modulo overflow -- admitted only when both sides use nothing but
//!   `+`, `-`, unary `-`, `*`, literals and parameters. An identity of the ring
//!   of integers holds in the ring of integers modulo 2^64 too (reduction mod
//!   2^64 is a ring homomorphism), so for those operators "proven over
//!   unbounded integers" is "proven for wrapping `i64`". With division,
//!   comparison or shifts it is not (`(a + b) / 2` and `a + (b - a) / 2` differ
//!   once `a + b` wraps), and such a rule is refused with that reason.
//! * `Refuted` -- refused, with the counterexample the validator confirmed by
//!   running both sides on it.
//! * `Unsupported` / `Unknown` -- refused, with the validator's reason. A rule
//!   nobody could prove is not a rule. The validator decides linear integer
//!   arithmetic, so a true rule that multiplies two parameters (`a * b + a * c
//!   == a * (b + c)`) is refused today for want of a proof, as are bitwise
//!   identities and anything over floats.
//!
//! A refused rule fails the build (`codira_compiler`'s driver reports it).
//!
//! # Where admitted rules apply
//!
//! In the e-graph (`codira_egraph::PatternRule`), alongside the built-in
//! rules, for every function Eidos optimizes in the package. Today the
//! production path uses the e-graph only for whole-function constant
//! folding, whose result the comptime interpreter computes regardless of
//! which equivalent form was extracted -- so an admitted rule cannot change
//! generated code yet. What it changes is what the optimizer can see, which
//! the e-graph's own tests show.

use std::sync::Arc;

use codira_egraph::PatternRule;
use codira_hir::{Function, HasSource, HirDatabase, Pat};
use codira_mir::{Attr, Body, OpId, OpKind};
use codira_syntax::{
    ast::{self, AstToken, AttributeOwner, BinOp, LiteralKind, PrefixOp},
    AstNode,
};

use crate::CodeGenDatabase;

/// The package's rewrite rules: the ones admitted, and why the others were
/// refused.
#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct RewriteRules {
    pub admitted: Vec<PatternRule>,
    pub refused: Vec<String>,
}

/// Salsa query behind `CodeGenDatabase::rewrite_rules`.
pub(crate) fn rewrite_rules_query(db: &dyn CodeGenDatabase) -> Arc<RewriteRules> {
    let mut rules = RewriteRules::default();
    for package in codira_hir::Package::all(db) {
        for module in package.modules(db) {
            for function in module.all_functions(db) {
                for (lhs, rhs) in rewrite_attributes(db, function) {
                    match check_rule(db, function, &lhs, &rhs) {
                        Ok(rule) => rules.admitted.push(rule),
                        Err(reason) => rules.refused.push(format!(
                            "rewrite rule `{} => {}` on `{}` is refused: {reason}",
                            lhs.syntax().text(),
                            rhs.syntax().text(),
                            function.name(db)
                        )),
                    }
                }
            }
        }
    }
    Arc::new(rules)
}

/// The `(from, to)` expression pairs of `function`'s `@rewrite` attributes.
fn rewrite_attributes(db: &dyn HirDatabase, function: Function) -> Vec<(ast::Expr, ast::Expr)> {
    let source = function.source(db);
    let Some(attributes) = source.value.attribute_list() else {
        return Vec::new();
    };
    attributes
        .attributes()
        .filter(|attribute| {
            attribute.path().and_then(|p| p.segment()).is_some_and(|s| {
                matches!(s.kind(), Some(ast::PathSegmentKind::Name(n)) if n.text() == "rewrite")
            })
        })
        .filter_map(|attribute| {
            let mut args = attribute.arg_list()?.args();
            Some((args.next()?, args.next()?))
        })
        .collect()
}

/// Lowers both sides, proves them equal, and builds the e-graph rule.
fn check_rule(
    db: &dyn HirDatabase,
    function: Function,
    lhs: &ast::Expr,
    rhs: &ast::Expr,
) -> Result<PatternRule, String> {
    let body = function.body(db);
    let params: Vec<String> = body
        .params()
        .iter()
        .map(|(pat, _)| match &body[*pat] {
            Pat::Bind { name } => name.to_string(),
            _ => String::new(),
        })
        .collect();
    let lower = |expr: &ast::Expr| {
        let mut out = Body::new();
        lower_expr(expr, &params, &mut out)?;
        Ok::<_, String>(out)
    };
    let lhs_body = lower(lhs)?;
    let rhs_body = lower(rhs)?;

    match codira_tv::Validator::new().validate(&lhs_body, &rhs_body) {
        codira_tv::Verdict::Proven {
            modulo_overflow: false,
        } => {}
        codira_tv::Verdict::Proven {
            modulo_overflow: true,
        } => {
            if !(ring_only(&lhs_body) && ring_only(&rhs_body)) {
                return Err(
                    "it is proven only for inputs whose intermediate values stay within i64, and \
                     with division, comparison or shifts in it that does not make it true for \
                     wrapping i64 arithmetic"
                        .to_owned(),
                );
            }
        }
        codira_tv::Verdict::Refuted { args: Some(values) } => {
            let witness: Vec<String> = params
                .iter()
                .zip(&values)
                .map(|(name, value)| format!("{name} = {value}"))
                .collect();
            return Err(format!(
                "it is false: the two sides differ for {}",
                witness.join(", ")
            ));
        }
        codira_tv::Verdict::Refuted { args: None } => {
            return Err(
                "the solver refuted it over unbounded integers, though no i64 counterexample was \
                 confirmed"
                    .to_owned(),
            )
        }
        codira_tv::Verdict::Unsupported { reason } => {
            return Err(format!("it cannot be proven: {reason}"))
        }
        codira_tv::Verdict::Unknown { reason } => {
            return Err(format!("the solver could not decide it: {reason}"))
        }
    }

    PatternRule::from_bodies(function.name(db).to_string(), &lhs_body, &rhs_body)
}

/// Whether `body` uses only ring operations, literals and parameters.
fn ring_only(body: &Body) -> bool {
    body.iter().all(|(_, op)| {
        matches!(
            op.kind,
            OpKind::Add | OpKind::Sub | OpKind::Mul | OpKind::Neg | OpKind::Arg(_)
        ) || matches!(op.kind, OpKind::Const(Attr::Int(_)))
    })
}

fn lower_expr(expr: &ast::Expr, params: &[String], out: &mut Body) -> Result<OpId, String> {
    let unsupported = || {
        format!(
            "`{}` is outside what a rewrite rule may contain (literals, parameters and \
             operators)",
            expr.syntax().text()
        )
    };
    match expr.kind() {
        ast::ExprKind::ParenExpr(e) => lower_expr(&e.expr().ok_or_else(unsupported)?, params, out),
        ast::ExprKind::Literal(literal) => match literal.kind() {
            LiteralKind::IntNumber(n) => {
                let text: String = n.text().chars().filter(|&c| c != '_').collect();
                let value = text.parse::<i64>().map_err(|_error| unsupported())?;
                Ok(out.push(OpKind::Const(Attr::Int(value)), []))
            }
            LiteralKind::Bool(b) => Ok(out.push(OpKind::Const(Attr::Bool(b)), [])),
            _ => Err(unsupported()),
        },
        ast::ExprKind::PathExpr(path) => {
            let name = path
                .path()
                .and_then(|p| p.segment())
                .and_then(|s| s.name_ref())
                .map(|n| n.text().to_string())
                .ok_or_else(unsupported)?;
            let index = params
                .iter()
                .position(|p| *p == name)
                .ok_or_else(|| format!("`{name}` is not a parameter of the function"))?;
            Ok(out.push(OpKind::Arg(index as u32), []))
        }
        ast::ExprKind::PrefixExpr(prefix) => {
            let operand = lower_expr(&prefix.expr().ok_or_else(unsupported)?, params, out)?;
            let kind = match prefix.op_kind().ok_or_else(unsupported)? {
                PrefixOp::Neg => OpKind::Neg,
                PrefixOp::Not => OpKind::Not,
                PrefixOp::BitNot => OpKind::BitNot,
            };
            Ok(out.push(kind, [operand]))
        }
        ast::ExprKind::BinExpr(bin) => {
            let lhs = lower_expr(&bin.lhs().ok_or_else(unsupported)?, params, out)?;
            let rhs = lower_expr(&bin.rhs().ok_or_else(unsupported)?, params, out)?;
            let kind = match bin.op_kind().ok_or_else(unsupported)? {
                BinOp::Add => OpKind::Add,
                BinOp::Subtract => OpKind::Sub,
                BinOp::Multiply => OpKind::Mul,
                BinOp::Divide => OpKind::Div,
                BinOp::Remainder => OpKind::Rem,
                BinOp::BooleanAnd => OpKind::And,
                BinOp::BooleanOr => OpKind::Or,
                BinOp::LeftShift => OpKind::Shl,
                BinOp::RightShift => OpKind::Shr,
                BinOp::BitwiseOr => OpKind::BitOr,
                BinOp::BitwiseAnd => OpKind::BitAnd,
                BinOp::BitwiseXor => OpKind::BitXor,
                BinOp::Equals => OpKind::Eq,
                BinOp::NotEqual => OpKind::Ne,
                BinOp::Less => OpKind::Lt,
                BinOp::LessEqual => OpKind::Le,
                BinOp::Greater => OpKind::Gt,
                BinOp::GreatEqual => OpKind::Ge,
                _ => return Err(unsupported()),
            };
            Ok(out.push(kind, [lhs, rhs]))
        }
        _ => Err(unsupported()),
    }
}

#[cfg(test)]
mod tests {
    use super::RewriteRules;
    use crate::{mock::MockDatabase, CodeGenDatabase};

    fn rules(source: &str) -> RewriteRules {
        let (db, _) = MockDatabase::with_single_file(source);
        (*db.rewrite_rules()).clone()
    }

    #[test]
    fn a_ring_identity_is_proven_and_admitted() {
        let rules = rules(
            "@rewrite(from: 3 * a + 2 * a, to: 5 * a)
func five(a: i64) -> i64 { 5 * a }
",
        );
        assert!(rules.refused.is_empty(), "{:?}", rules.refused);
        assert_eq!(rules.admitted.len(), 1);
        assert_eq!(rules.admitted[0].name, "five");
    }

    #[test]
    fn what_the_validator_cannot_prove_is_not_admitted() {
        // True, but the validator decides linear integer arithmetic only, and
        // `a * b` is not linear: no proof, so no rule.
        let rules = rules(
            "@rewrite(from: a * b + a * c, to: a * (b + c))
func factor(a: i64, b: i64, c: i64) ->              i64 { a * (b + c) }
",
        );
        assert!(rules.admitted.is_empty());
        assert!(
            rules.refused[0].contains("cannot be proven"),
            "{}",
            rules.refused[0]
        );
    }

    #[test]
    fn positional_arguments_say_the_same() {
        let rules = rules("@rewrite(x - x, 0)\nfunc zero(x: i64) -> i64 { 0 }\n");
        assert!(rules.refused.is_empty(), "{:?}", rules.refused);
        assert_eq!(rules.admitted.len(), 1);
    }

    #[test]
    fn a_false_rule_is_refused_with_a_counterexample() {
        let rules =
            rules("@rewrite(from: a * 2, to: a + 3)\nfunc wrong(a: i64) -> i64 { a * 2 }\n");
        assert!(rules.admitted.is_empty());
        let reason = &rules.refused[0];
        assert!(reason.contains("`a * 2 => a + 3` on `wrong`"), "{reason}");
        assert!(reason.contains("differ for a = "), "{reason}");
    }

    #[test]
    fn a_rule_true_only_without_overflow_is_refused() {
        // `(a * 2) / 2 == a` over unbounded integers, but not once `a * 2`
        // wraps (a = 2^62).
        let rules = rules("@rewrite(from: (a * 2) / 2, to: a)\nfunc halve(a: i64) -> i64 { a }\n");
        assert!(rules.admitted.is_empty());
        assert!(
            rules.refused[0].contains("wrapping i64"),
            "{}",
            rules.refused[0]
        );
    }

    #[test]
    fn rules_outside_the_subset_are_refused_with_the_reason() {
        let rules = rules(
            "@rewrite(from: x * 1.5, to: x)\nfunc f(x: f64) -> f64 { x }\n\n@rewrite(from: y + 0, \
             to: y)\nfunc g(x: i64) -> i64 { x }\n",
        );
        assert!(rules.admitted.is_empty());
        assert_eq!(rules.refused.len(), 2, "{:?}", rules.refused);
        assert!(rules
            .refused
            .iter()
            .any(|r| r.contains("outside what a rewrite rule may contain")));
        assert!(rules
            .refused
            .iter()
            .any(|r| r.contains("`y` is not a parameter")));
    }
}

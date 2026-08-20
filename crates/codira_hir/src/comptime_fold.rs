//! Copyright (c) 2026 Omnira CJSC
//!
//! Bridges a self-contained `comptime { .. }` block's already-lowered HIR
//! into `codira_mir`/`codira_comptime` for real compile-time evaluation.
//!
//! Scope (see `spec/KGEN_SUPERSET_STATUS.md`): only single-tail-expression
//! blocks (no `let`-statements, no expression-statements) built from
//! literals, arithmetic/comparison/logical operators, unary neg/not, and
//! `if`/`else` fold today. Anything referencing a name (`Path`), calling a
//! function, indexing, looping, or containing a statement bails out
//! (`None`) and the caller leaves the block to lower exactly as it did
//! before this module existed -- an honest, additive-only improvement:
//! nothing that used to work stops working, and a real (if narrow) subset
//! of `comptime` now actually evaluates at compile time instead of always
//! running at ordinary runtime.

use la_arena::Arena;

use crate::expr::{
    ArithOp, BinaryOp, CmpOp, Expr, ExprId, Literal, LiteralInt, LiteralIntKind, LogicOp,
    Ordering, UnaryOp,
};

/// Attempts to evaluate `block` (already-lowered HIR) at compile time.
/// `None` means "not foldable with today's restricted subset," not an
/// error -- callers should fall back to leaving the block unevaluated.
pub(crate) fn try_eval(exprs: &Arena<Expr>, block: ExprId) -> Option<codira_comptime::Value> {
    let mut body = codira_mir::Body::new();
    build_op(exprs, block, &mut body)?;
    codira_comptime::eval_body(&body, &codira_comptime::Env::new()).ok()
}

/// Converts a concrete evaluation result back into a source `Expr`,
/// allocating any synthetic sub-expressions (the literal inside a negative
/// result's `UnaryOp::Neg` wrapper) into `exprs`. Returns `None` for
/// `Value::Unit` -- there is no `Expr::Literal` variant that safely stands
/// in for "unit" without risking a type mismatch against the block's
/// inferred type, and it's a low-value case to special-case further today.
pub(crate) fn value_to_expr(
    exprs: &mut Arena<Expr>,
    value: codira_comptime::Value,
) -> Option<Expr> {
    match value {
        codira_comptime::Value::Bool(b) => Some(Expr::Literal(Literal::Bool(b))),
        codira_comptime::Value::Int(v) if v >= 0 => {
            Some(Expr::Literal(Literal::Int(LiteralInt {
                kind: LiteralIntKind::Unsuffixed,
                value: v as u128,
            })))
        }
        codira_comptime::Value::Int(v) => {
            let inner = exprs.alloc(Expr::Literal(Literal::Int(LiteralInt {
                kind: LiteralIntKind::Unsuffixed,
                value: v.unsigned_abs() as u128,
            })));
            Some(Expr::UnaryOp {
                expr: inner,
                op: UnaryOp::Neg,
            })
        }
        codira_comptime::Value::Unit => None,
    }
}

fn build_op(
    exprs: &Arena<Expr>,
    id: ExprId,
    body: &mut codira_mir::Body,
) -> Option<codira_mir::OpId> {
    use codira_mir::{Attr, OpKind, Region};

    match &exprs[id] {
        Expr::Literal(Literal::Bool(b)) => Some(body.push(OpKind::Const(Attr::Bool(*b)), [])),
        Expr::Literal(Literal::Int(LiteralInt { value, .. })) => {
            let v = i64::try_from(*value).ok()?;
            Some(body.push(OpKind::Const(Attr::Int(v)), []))
        }
        Expr::UnaryOp { expr, op } => {
            let inner = build_op(exprs, *expr, body)?;
            let kind = match op {
                UnaryOp::Neg => OpKind::Neg,
                UnaryOp::Not => OpKind::Not,
            };
            Some(body.push(kind, [inner]))
        }
        Expr::BinaryOp {
            lhs,
            rhs,
            op: Some(op),
        } => {
            let lhs_id = build_op(exprs, *lhs, body)?;
            let rhs_id = build_op(exprs, *rhs, body)?;
            let kind = binary_op_kind(*op)?;
            Some(body.push(kind, [lhs_id, rhs_id]))
        }
        Expr::If {
            condition,
            then_branch,
            else_branch,
        } => {
            let cond_id = build_op(exprs, *condition, body)?;

            let mut then_body = codira_mir::Body::new();
            build_op(exprs, *then_branch, &mut then_body)?;

            let mut else_body = codira_mir::Body::new();
            if let Some(else_branch) = else_branch {
                build_op(exprs, *else_branch, &mut else_body)?;
            }

            Some(body.push_with_regions(
                OpKind::If,
                [cond_id],
                [Region::new(then_body), Region::new(else_body)],
            ))
        }
        Expr::Block { statements, tail } if statements.is_empty() => build_op(exprs, (*tail)?, body),
        // `Path`, `Call`, `MethodCall`, `Index`, `Array`, `RecordLit`,
        // `Field`, `Loop`, `While`, `Return`, `Break`, blocks with
        // statements, `Missing`, and string/float literals are all out of
        // scope for this session's restricted interpreter -- see module
        // doc and spec/KGEN_SUPERSET_STATUS.md.
        _ => None,
    }
}

fn binary_op_kind(op: BinaryOp) -> Option<codira_mir::OpKind> {
    use codira_mir::OpKind;
    Some(match op {
        BinaryOp::ArithOp(ArithOp::Add) => OpKind::Add,
        BinaryOp::ArithOp(ArithOp::Subtract) => OpKind::Sub,
        BinaryOp::ArithOp(ArithOp::Multiply) => OpKind::Mul,
        BinaryOp::ArithOp(ArithOp::Divide) => OpKind::Div,
        BinaryOp::ArithOp(ArithOp::Remainder) => OpKind::Rem,
        // Bitwise/shift ops have no codira_mir core.* op yet -- only the
        // arithmetic/comparison/logical subset needed for `comptime { 2 + 2
        // }`-shaped code exists so far. See architecture doc §2.1.
        BinaryOp::ArithOp(
            ArithOp::LeftShift | ArithOp::RightShift | ArithOp::BitAnd | ArithOp::BitOr
            | ArithOp::BitXor,
        ) => return None,
        BinaryOp::LogicOp(LogicOp::And) => OpKind::And,
        BinaryOp::LogicOp(LogicOp::Or) => OpKind::Or,
        BinaryOp::CmpOp(CmpOp::Eq { negated: false }) => OpKind::Eq,
        BinaryOp::CmpOp(CmpOp::Eq { negated: true }) => OpKind::Ne,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Less,
            strict: true,
        }) => OpKind::Lt,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Less,
            strict: false,
        }) => OpKind::Le,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Greater,
            strict: true,
        }) => OpKind::Gt,
        BinaryOp::CmpOp(CmpOp::Ord {
            ordering: Ordering::Greater,
            strict: false,
        }) => OpKind::Ge,
        BinaryOp::Assignment { .. } => return None,
    })
}

#[cfg(test)]
mod tests {
    use la_arena::Arena;

    use super::*;
    use crate::expr::{Literal, LiteralInt, LiteralIntKind};

    #[test]
    fn folds_two_plus_two() {
        let mut exprs = Arena::new();
        let two_a = exprs.alloc(Expr::Literal(Literal::Int(LiteralInt {
            kind: LiteralIntKind::Unsuffixed,
            value: 2,
        })));
        let two_b = exprs.alloc(Expr::Literal(Literal::Int(LiteralInt {
            kind: LiteralIntKind::Unsuffixed,
            value: 2,
        })));
        let sum = exprs.alloc(Expr::BinaryOp {
            lhs: two_a,
            rhs: two_b,
            op: Some(BinaryOp::ArithOp(ArithOp::Add)),
        });
        let block = exprs.alloc(Expr::Block {
            statements: Vec::new(),
            tail: Some(sum),
        });

        let value = try_eval(&exprs, block).unwrap();
        assert_eq!(value, codira_comptime::Value::Int(4));

        let folded = value_to_expr(&mut exprs, value).unwrap();
        assert_eq!(
            folded,
            Expr::Literal(Literal::Int(LiteralInt {
                kind: LiteralIntKind::Unsuffixed,
                value: 4,
            }))
        );
    }

    #[test]
    fn negative_result_wraps_in_unary_neg() {
        let mut exprs = Arena::new();
        let zero = exprs.alloc(Expr::Literal(Literal::Int(LiteralInt {
            kind: LiteralIntKind::Unsuffixed,
            value: 0,
        })));
        let five = exprs.alloc(Expr::Literal(Literal::Int(LiteralInt {
            kind: LiteralIntKind::Unsuffixed,
            value: 5,
        })));
        let diff = exprs.alloc(Expr::BinaryOp {
            lhs: zero,
            rhs: five,
            op: Some(BinaryOp::ArithOp(ArithOp::Subtract)),
        });

        let value = try_eval(&exprs, diff).unwrap();
        assert_eq!(value, codira_comptime::Value::Int(-5));

        let folded = value_to_expr(&mut exprs, value).unwrap();
        match folded {
            Expr::UnaryOp { expr, op: UnaryOp::Neg } => {
                assert_eq!(
                    exprs[expr],
                    Expr::Literal(Literal::Int(LiteralInt {
                        kind: LiteralIntKind::Unsuffixed,
                        value: 5,
                    }))
                );
            }
            other => panic!("expected UnaryOp::Neg, got {other:?}"),
        }
    }

    #[test]
    fn bails_out_on_unsupported_construct() {
        // `Missing` stands in for anything this restricted interpreter
        // doesn't handle (name references, calls, etc. -- see module doc);
        // the important property under test is that an unsupported node
        // anywhere in the tree causes a clean `None`, not a panic or a
        // wrong answer.
        let mut exprs = Arena::new();
        let unsupported = exprs.alloc(Expr::Missing);
        assert!(try_eval(&exprs, unsupported).is_none());
    }
}

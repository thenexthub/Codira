//! The compile-time interpreter.
//!
//! Structural analog of `KGEN/lib/Interpreter` -- evaluates a `codira_mir`
//! [`Body`] to a concrete [`Value`], for both plain `comptime { .. }` blocks
//! and (once wired into elaboration) generator parameter expressions. See
//! `spec/KGEN_SUPERSET_ARCHITECTURE.md` §4.

use codira_mir::{Attr, Body, Op, OpId, OpKind};

use crate::value::{EvalError, Value};
use crate::Env;

/// Evaluates a [`Body`] to its result value (the value of its last op),
/// under the given parameter bindings.
pub fn eval_body(body: &Body, env: &Env) -> Result<Value, EvalError> {
    let result = body.result().ok_or(EvalError::EmptyRegion)?;
    eval_op(body, result, env)
}

fn eval_op(body: &Body, id: OpId, env: &Env) -> Result<Value, EvalError> {
    let op: &Op = body.get(id);
    match &op.kind {
        OpKind::Const(attr) => eval_attr(attr, env),

        OpKind::Add | OpKind::Sub | OpKind::Mul | OpKind::Div | OpKind::Rem => {
            let (lhs, rhs) = binary_int_operands(body, op, env)?;
            match op.kind {
                OpKind::Add => Ok(Value::Int(lhs.wrapping_add(rhs))),
                OpKind::Sub => Ok(Value::Int(lhs.wrapping_sub(rhs))),
                OpKind::Mul => Ok(Value::Int(lhs.wrapping_mul(rhs))),
                OpKind::Div => lhs
                    .checked_div(rhs)
                    .map(Value::Int)
                    .ok_or(EvalError::DivideByZero),
                OpKind::Rem => lhs
                    .checked_rem(rhs)
                    .map(Value::Int)
                    .ok_or(EvalError::DivideByZero),
                _ => unreachable!(),
            }
        }

        OpKind::Neg => {
            let v = unary_int_operand(body, op, env)?;
            Ok(Value::Int(v.wrapping_neg()))
        }

        OpKind::Eq | OpKind::Ne | OpKind::Lt | OpKind::Le | OpKind::Gt | OpKind::Ge => {
            let (lhs, rhs) = binary_int_operands(body, op, env)?;
            Ok(Value::Bool(match op.kind {
                OpKind::Eq => lhs == rhs,
                OpKind::Ne => lhs != rhs,
                OpKind::Lt => lhs < rhs,
                OpKind::Le => lhs <= rhs,
                OpKind::Gt => lhs > rhs,
                OpKind::Ge => lhs >= rhs,
                _ => unreachable!(),
            }))
        }

        OpKind::And | OpKind::Or => {
            let (lhs, rhs) = binary_bool_operands(body, op, env)?;
            Ok(Value::Bool(match op.kind {
                OpKind::And => lhs && rhs,
                OpKind::Or => lhs || rhs,
                _ => unreachable!(),
            }))
        }

        OpKind::Not => {
            let v = unary_bool_operand(body, op, env)?;
            Ok(Value::Bool(!v))
        }

        OpKind::ParamRef(name) => env.lookup(name),

        OpKind::If => {
            let [cond_id] = op.operands.as_slice() else {
                return Err(EvalError::MalformedIf);
            };
            let [then_region, else_region] = op.regions.as_slice() else {
                return Err(EvalError::MalformedIf);
            };
            let cond = eval_op(body, *cond_id, env)?.as_bool()?;
            let region = if cond { then_region } else { else_region };
            if region.body.is_empty() {
                Ok(Value::Unit)
            } else {
                eval_body(&region.body, env)
            }
        }

        OpKind::For => Err(EvalError::UnsupportedOp("cf.for")),
        OpKind::While => Err(EvalError::UnsupportedOp("cf.while")),
    }
}

fn eval_attr(attr: &Attr, env: &Env) -> Result<Value, EvalError> {
    match attr {
        Attr::Int(v) => Ok(Value::Int(*v)),
        Attr::Bool(v) => Ok(Value::Bool(*v)),
        Attr::Unit => Ok(Value::Unit),
        Attr::ParamRef(name) => env.lookup(name),
    }
}

fn binary_int_operands(body: &Body, op: &Op, env: &Env) -> Result<(i64, i64), EvalError> {
    let [lhs_id, rhs_id] = op.operands.as_slice() else {
        return Err(EvalError::TypeMismatch {
            expected: "2 operands",
            found: "different operand count",
        });
    };
    let lhs = eval_op(body, *lhs_id, env)?.as_int()?;
    let rhs = eval_op(body, *rhs_id, env)?.as_int()?;
    Ok((lhs, rhs))
}

fn binary_bool_operands(body: &Body, op: &Op, env: &Env) -> Result<(bool, bool), EvalError> {
    let [lhs_id, rhs_id] = op.operands.as_slice() else {
        return Err(EvalError::TypeMismatch {
            expected: "2 operands",
            found: "different operand count",
        });
    };
    let lhs = eval_op(body, *lhs_id, env)?.as_bool()?;
    let rhs = eval_op(body, *rhs_id, env)?.as_bool()?;
    Ok((lhs, rhs))
}

fn unary_int_operand(body: &Body, op: &Op, env: &Env) -> Result<i64, EvalError> {
    let [id] = op.operands.as_slice() else {
        return Err(EvalError::TypeMismatch {
            expected: "1 operand",
            found: "different operand count",
        });
    };
    eval_op(body, *id, env)?.as_int()
}

fn unary_bool_operand(body: &Body, op: &Op, env: &Env) -> Result<bool, EvalError> {
    let [id] = op.operands.as_slice() else {
        return Err(EvalError::TypeMismatch {
            expected: "1 operand",
            found: "different operand count",
        });
    };
    eval_op(body, *id, env)?.as_bool()
}

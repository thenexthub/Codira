//! Copyright (c) 2026 Omnira CJSC
//!
//! `codira_mir::Body` -> real LLVM IR, via inkwell.
//!
//! This is `spec/KGEN_SUPERSET_STATUS.md` roadmap item 5 / session-2
//! item #19: the piece that makes an elaborated (parameter-free)
//! `codira_mir::Body` -- the output of `codira_comptime::elaborate`,
//! reached via `codira_hir`'s `elaborate_generator` salsa query --
//! actually compilable, not just interpretable. Structural analog of
//! `KGEN/lib/KGENToLLVM`.
//!
//! Scope: mirrors the interpreter's (`codira_comptime::eval_body`) and the
//! elaborator's supported op set exactly -- `Const`/`Arg`/arithmetic/
//! comparison/logic/unary ops/`If`. `cf.for`/`cf.while` are not lowered
//! (same honesty as the interpreter); an unresolved `param.ref` reaching
//! this stage is a hard "not lowerable" (`None`) rather than a guess --
//! elaboration should have removed every one that was going to be
//! resolved, so a surviving one means the caller under-specialized.
//!
//! All integers are treated as signed 64-bit (`codira_mir::Attr::Int` is
//! `i64` with no separate signedness/width tag yet -- see architecture doc
//! §8 non-goals for why that's not modeled further this session).

use inkwell::{
    builder::Builder,
    context::Context,
    values::{BasicValueEnum, FunctionValue, IntValue},
    IntPredicate,
};
use rustc_hash::FxHashMap;

/// Lowers `body` to LLVM IR, emitting instructions at the builder's
/// current insertion point (the caller is responsible for having
/// positioned it inside a real basic block of `function` first, exactly
/// like any other codegen helper in this crate). `args` supplies the
/// concrete LLVM values for the body's `OpKind::Arg(i)` references (the
/// function's actual runtime parameters). Returns the body's result value,
/// or `None` if the body isn't fully lowerable (see module doc).
pub fn lower_mir_body<'ink>(
    context: &'ink Context,
    builder: &Builder<'ink>,
    function: FunctionValue<'ink>,
    body: &codira_mir::Body,
    args: &[BasicValueEnum<'ink>],
) -> Option<BasicValueEnum<'ink>> {
    let mut block_counter = 0u32;
    lower_body(context, builder, function, body, args, &mut block_counter)
}

fn lower_body<'ink>(
    context: &'ink Context,
    builder: &Builder<'ink>,
    function: FunctionValue<'ink>,
    body: &codira_mir::Body,
    args: &[BasicValueEnum<'ink>],
    block_counter: &mut u32,
) -> Option<BasicValueEnum<'ink>> {
    use codira_mir::{Attr, OpKind};

    // Values are keyed by `OpId`, which is only meaningful within this
    // specific `Body`'s arena -- a fresh map per call, matching the same
    // per-arena scoping `codira_hir::mir_lower` and
    // `codira_comptime::elaborate` both use for exactly the same reason
    // (see their doc comments).
    let mut values: FxHashMap<codira_mir::OpId, BasicValueEnum<'ink>> = FxHashMap::default();

    for (id, op) in body.iter() {
        let value: BasicValueEnum<'ink> = match &op.kind {
            OpKind::Const(Attr::Int(v)) => {
                context.i64_type().const_int(*v as u64, true).into()
            }
            OpKind::Const(Attr::Bool(b)) => {
                context.bool_type().const_int(u64::from(*b), false).into()
            }
            // A `Const(Unit)` or a surviving `ParamRef` reaching codegen
            // means the caller handed us a body that either genuinely
            // computes "nothing" or wasn't fully elaborated -- neither is
            // lowerable here. See module doc.
            OpKind::Const(Attr::Unit) | OpKind::Const(Attr::ParamRef(_)) | OpKind::ParamRef(_) => {
                return None
            }

            OpKind::Arg(index) => *args.get(*index as usize)?,

            OpKind::Add | OpKind::Sub | OpKind::Mul | OpKind::Div | OpKind::Rem => {
                let lhs = int_operand(&values, op.operands.first().copied()?)?;
                let rhs = int_operand(&values, op.operands.get(1).copied()?)?;
                let result = match op.kind {
                    OpKind::Add => builder.build_int_add(lhs, rhs, "mir_add"),
                    OpKind::Sub => builder.build_int_sub(lhs, rhs, "mir_sub"),
                    OpKind::Mul => builder.build_int_mul(lhs, rhs, "mir_mul"),
                    OpKind::Div => builder.build_int_signed_div(lhs, rhs, "mir_div"),
                    OpKind::Rem => builder.build_int_signed_rem(lhs, rhs, "mir_rem"),
                    _ => unreachable!(),
                };
                result.ok()?.into()
            }

            OpKind::Neg => {
                let v = int_operand(&values, op.operands.first().copied()?)?;
                builder.build_int_neg(v, "mir_neg").ok()?.into()
            }

            OpKind::Eq | OpKind::Ne | OpKind::Lt | OpKind::Le | OpKind::Gt | OpKind::Ge => {
                let lhs = int_operand(&values, op.operands.first().copied()?)?;
                let rhs = int_operand(&values, op.operands.get(1).copied()?)?;
                let predicate = match op.kind {
                    OpKind::Eq => IntPredicate::EQ,
                    OpKind::Ne => IntPredicate::NE,
                    OpKind::Lt => IntPredicate::SLT,
                    OpKind::Le => IntPredicate::SLE,
                    OpKind::Gt => IntPredicate::SGT,
                    OpKind::Ge => IntPredicate::SGE,
                    _ => unreachable!(),
                };
                builder
                    .build_int_compare(predicate, lhs, rhs, "mir_cmp")
                    .ok()?
                    .into()
            }

            OpKind::And | OpKind::Or => {
                let lhs = int_operand(&values, op.operands.first().copied()?)?;
                let rhs = int_operand(&values, op.operands.get(1).copied()?)?;
                let result = match op.kind {
                    OpKind::And => builder.build_and(lhs, rhs, "mir_and"),
                    OpKind::Or => builder.build_or(lhs, rhs, "mir_or"),
                    _ => unreachable!(),
                };
                result.ok()?.into()
            }

            OpKind::Not => {
                let v = int_operand(&values, op.operands.first().copied()?)?;
                builder.build_not(v, "mir_not").ok()?.into()
            }

            OpKind::If => {
                let cond = int_operand(&values, op.operands.first().copied()?)?;
                let [then_region, else_region] = op.regions.as_slice() else {
                    return None;
                };

                *block_counter += 1;
                let n = *block_counter;
                let then_block = context.append_basic_block(function, &format!("mir_if_then{n}"));
                let else_block = context.append_basic_block(function, &format!("mir_if_else{n}"));
                let merge_block = context.append_basic_block(function, &format!("mir_if_merge{n}"));

                builder
                    .build_conditional_branch(cond, then_block, else_block)
                    .ok()?;

                builder.position_at_end(then_block);
                let then_value =
                    lower_body(context, builder, function, &then_region.body, args, block_counter)?;
                builder.build_unconditional_branch(merge_block).ok()?;
                let then_end_block = builder.get_insert_block()?;

                builder.position_at_end(else_block);
                let else_value =
                    lower_body(context, builder, function, &else_region.body, args, block_counter)?;
                builder.build_unconditional_branch(merge_block).ok()?;
                let else_end_block = builder.get_insert_block()?;

                builder.position_at_end(merge_block);
                let phi = builder
                    .build_phi(then_value.get_type(), "mir_if_result")
                    .ok()?;
                phi.add_incoming(&[
                    (&then_value, then_end_block),
                    (&else_value, else_end_block),
                ]);
                phi.as_basic_value()
            }

            // Not interpreted by `codira_comptime` either -- see its
            // `EvalError::UnsupportedOp` and this module's doc comment.
            OpKind::For | OpKind::While => return None,
        };
        values.insert(id, value);
    }

    let result_id = body.result()?;
    values.get(&result_id).copied()
}

fn int_operand<'ink>(
    values: &FxHashMap<codira_mir::OpId, BasicValueEnum<'ink>>,
    id: codira_mir::OpId,
) -> Option<IntValue<'ink>> {
    Some(values.get(&id).copied()?.into_int_value())
}

#[cfg(test)]
mod tests;
#[cfg(test)]
mod e2e_tests;

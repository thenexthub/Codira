//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Code generation for structured concurrency: `scope`, `spawn`,
//!   `channel(..)`, `ch <- v` and `<-ch`, as calls into the runtime's task pool
//!   (`codira_runtime::tasks`) through the dispatch table.
//!
//! # Scopes are joined on every exit
//!
//! `scope { .. }` calls `codira.scope_begin` on entry and `codira.scope_end`
//! -- which waits for every task the scope spawned, then frees its channels
//! -- on *every* way out of the block, not just its end:
//!
//! * `return` ends every scope of the function before returning;
//! * `break` ends the scopes opened inside the loop it leaves;
//! * an effect handler abandoning the computation (see `effects`) ends the
//!   scopes between the abort check and wherever control lands;
//! * `resume` in a handler arm ends the arm's scopes before returning.
//!
//! Each of those paths is a separate block, and exactly one of them runs for
//! any given entry into the scope, so each scope ends exactly once.
//!
//! # Tasks
//!
//! `spawn f(a, b)` evaluates `a` and `b`, stores them in a stack struct and
//! hands the runtime a pointer to it, its size, and a *trampoline*
//! `f.task(ptr)` that loads them back and calls `f`. The runtime copies the
//! bytes before `scope_spawn` returns, so the struct can live in the
//! spawning frame. The HIR validator guarantees that `f` takes only scalars
//! and channel handles, so the copy shares nothing with the spawner.
//!
//! # `select`
//!
//! A `select` evaluates every arm's channel and sent value in source order,
//! storing each sent value in a stack slot and allocating a slot for each
//! received one, and lays the operations out as an array of
//! `(channel, kind, slot address)` triples for `codira.channel_select`. The
//! index it returns -- or the operation count, for `default` -- switches to
//! the chosen arm, which loads its received value (if it binds one) and runs
//! its body; the arms meet in one block with a phi of their values.
use codira_hir::{Expr, ExprId, SelectArm, SelectOp, TyKind};
use inkwell::{
    module::Linkage,
    types::BasicTypeEnum,
    values::{BasicMetadataValueEnum, BasicValueEnum, FunctionValue, IntValue, PointerValue},
    AddressSpace,
};

use super::BodyIrGenerator;
use crate::intrinsics::{self, Intrinsic};

/// The task-related state of a `BodyIrGenerator`.
#[derive(Default)]
pub(super) struct TaskState<'ink> {
    /// The runtime handles of the `scope`s enclosing the code being
    /// generated, innermost last.
    pub(super) scopes: Vec<IntValue<'ink>>,
    /// For each enclosing loop, innermost last, how many scopes were open
    /// when it started: a `break` ends the ones opened since.
    loop_depths: Vec<usize>,
}

impl<'ink> BodyIrGenerator<'_, 'ink, '_> {
    /// Calls a runtime function through its dispatch table slot and returns
    /// its result, if it has one.
    fn call_runtime(
        &mut self,
        function: &impl Intrinsic,
        args: &[BasicMetadataValueEnum<'ink>],
    ) -> Option<BasicValueEnum<'ink>> {
        let (fn_ty, fn_ptr) = self.dispatch_table.gen_intrinsic_lookup(
            self.external_globals.dispatch_table,
            &self.builder,
            function,
        );
        self.builder
            .build_indirect_call(fn_ty, fn_ptr, args, "")
            .expect("failed to build call to a runtime function")
            .try_as_basic_value()
            .basic()
    }

    fn address_of(&self, ptr: inkwell::values::PointerValue<'ink>) -> IntValue<'ink> {
        self.builder
            .build_ptr_to_int(ptr, self.hir_types.get_usize_type(), "address")
            .expect("failed to take an address")
    }

    /// Notes that a loop starts here, for `break`.
    pub(super) fn enter_loop_scopes(&mut self) {
        self.tasks.loop_depths.push(self.tasks.scopes.len());
    }

    /// Notes that the innermost loop has been generated.
    pub(super) fn leave_loop_scopes(&mut self) {
        self.tasks.loop_depths.pop();
    }

    /// Ends, innermost first, every open scope beyond the first `keep`,
    /// without forgetting them: code generation continues on other paths
    /// that are still inside them.
    pub(super) fn gen_end_scopes(&mut self, keep: usize) {
        let scopes: Vec<IntValue<'ink>> = self.tasks.scopes[keep.min(self.tasks.scopes.len())..]
            .iter()
            .rev()
            .copied()
            .collect();
        for scope in scopes {
            self.call_runtime(&intrinsics::scope_end, &[scope.into()]);
        }
    }

    /// Before a `break`: ends the scopes opened inside the loop it leaves.
    pub(super) fn gen_end_loop_scopes(&mut self) {
        let keep = self.tasks.loop_depths.last().copied().unwrap_or(0);
        self.gen_end_scopes(keep);
    }

    /// `scope { body }`.
    pub(super) fn gen_scope(&mut self, body: ExprId) -> Option<BasicValueEnum<'ink>> {
        let scope = self
            .call_runtime(&intrinsics::scope_begin, &[])
            .expect("`codira.scope_begin` returns a handle")
            .into_int_value();
        self.tasks.scopes.push(scope);
        let value = self.gen_expr(body);
        self.tasks.scopes.pop();
        if value.is_some() {
            self.call_runtime(&intrinsics::scope_end, &[scope.into()]);
        }
        value
    }

    /// `spawn f(args)`.
    pub(super) fn gen_spawn(&mut self, call: ExprId) -> Option<BasicValueEnum<'ink>> {
        let body = self.body.clone();
        let Expr::Call { callee, args } = &body[call] else {
            unreachable!("the task validator rejects a `spawn` of anything but a call");
        };
        let Some(codira_hir::CallableDef::Function(function)) =
            self.infer[*callee].as_callable_def()
        else {
            unreachable!("the task validator rejects a `spawn` of anything but a function");
        };

        let mut values = Vec::with_capacity(args.len());
        for arg in args {
            values.push(self.gen_expr(*arg)?);
        }

        let scope = *self
            .tasks
            .scopes
            .last()
            .expect("the task validator rejects a `spawn` outside a `scope`");
        let trampoline = self.task_trampoline(function);

        let field_types: Vec<BasicTypeEnum<'ink>> =
            values.iter().map(BasicValueEnum::get_type).collect();
        let args_ty = self.context.struct_type(&field_types, false);
        let args_ptr = self
            .new_alloca_builder()
            .build_alloca(args_ty, "task_args")
            .expect("failed to allocate task arguments");
        for (idx, value) in values.into_iter().enumerate() {
            let field = self
                .builder
                .build_struct_gep(args_ty, args_ptr, idx as u32, "task_arg")
                .expect("failed to address task argument");
            self.builder
                .build_store(field, value)
                .expect("failed to store task argument");
        }
        let size = self
            .hir_types
            .get_usize_type()
            .const_int(self.hir_types.target_data().get_abi_size(&args_ty), false);
        let entry = self.address_of(trampoline.as_global_value().as_pointer_value());
        let args_address = self.address_of(args_ptr);
        self.call_runtime(
            &intrinsics::scope_spawn,
            &[scope.into(), entry.into(), args_address.into(), size.into()],
        );
        Some(self.gen_empty())
    }

    /// `f.task(ptr)`: loads `f`'s arguments from the struct `ptr` points at
    /// and calls `f` with them, discarding its result.
    fn task_trampoline(&self, function: codira_hir::Function) -> FunctionValue<'ink> {
        let target = *self
            .function_map
            .get(&function)
            .expect("the task validator only allows functions of this module");
        let name = format!("{}.task", target.get_name().to_string_lossy());
        if let Some(existing) = self.module.get_function(&name) {
            return existing;
        }
        let ptr_ty = self.context.ptr_type(AddressSpace::default());
        let fn_type = self.context.void_type().fn_type(&[ptr_ty.into()], false);
        let trampoline = self
            .module
            .add_function(&name, fn_type, Some(Linkage::Private));

        let builder = self.context.create_builder();
        builder.position_at_end(self.context.append_basic_block(trampoline, "task"));
        let args_ptr = trampoline
            .get_nth_param(0)
            .expect("a trampoline takes its argument pointer")
            .into_pointer_value();
        let param_types: Vec<BasicTypeEnum<'ink>> = function
            .ty(self.db)
            .callable_sig(self.db)
            .expect("a function has a signature")
            .params()
            .iter()
            .map(|ty| {
                self.hir_types
                    .get_basic_type(ty)
                    .expect("a task parameter has a basic type")
            })
            .collect();
        let args_ty = self.context.struct_type(&param_types, false);
        let mut args: Vec<BasicMetadataValueEnum<'ink>> = Vec::with_capacity(param_types.len());
        for (idx, ty) in param_types.iter().enumerate() {
            let field = builder
                .build_struct_gep(args_ty, args_ptr, idx as u32, "arg")
                .expect("failed to address task argument");
            args.push(
                builder
                    .build_load(*ty, field, "arg")
                    .expect("failed to load task argument")
                    .into(),
            );
        }
        builder
            .build_call(target, &args, "")
            .expect("failed to call the task's function");
        builder
            .build_return(None)
            .expect("failed to return from a task");
        trampoline
    }

    /// The `channel(capacity)` builtin: a channel owned by the innermost
    /// scope.
    pub(super) fn gen_channel_new(
        &mut self,
        expr: ExprId,
        args: &[ExprId],
    ) -> Option<BasicValueEnum<'ink>> {
        let capacity = self.gen_expr(args[0])?;
        let element_size = self.channel_element_size(expr);
        let scope = *self
            .tasks
            .scopes
            .last()
            .expect("the task validator rejects a channel created outside a `scope`");
        self.call_runtime(
            &intrinsics::channel_new,
            &[scope.into(), capacity.into(), element_size.into()],
        )
    }

    /// The size, as a `usize` constant, of the elements of the channel `expr`
    /// evaluates to.
    fn channel_element_size(&self, expr: ExprId) -> IntValue<'ink> {
        let size = self
            .hir_types
            .target_data()
            .get_abi_size(&self.channel_element_type(expr));
        self.hir_types.get_usize_type().const_int(size, false)
    }

    fn channel_element_type(&self, expr: ExprId) -> BasicTypeEnum<'ink> {
        let TyKind::Channel(element) = self.infer[expr].interned() else {
            unreachable!("inference types a channel operand as a channel");
        };
        self.hir_types
            .get_basic_type(element)
            .expect("the task validator only allows scalar channel elements")
    }

    /// `channel <- value`.
    pub(super) fn gen_channel_send(
        &mut self,
        channel: ExprId,
        value: ExprId,
    ) -> Option<BasicValueEnum<'ink>> {
        let handle = self.gen_expr(channel)?;
        let value = self.gen_expr(value)?;
        let element_ty = self.channel_element_type(channel);
        let slot = self
            .new_alloca_builder()
            .build_alloca(element_ty, "sent")
            .expect("failed to allocate a channel element");
        self.builder
            .build_store(slot, value)
            .expect("failed to store a channel element");
        let address = self.address_of(slot);
        self.call_runtime(&intrinsics::channel_send, &[handle.into(), address.into()]);
        Some(self.gen_empty())
    }

    /// `select { arm, .. }`.
    pub(super) fn gen_select(
        &mut self,
        _expr: ExprId,
        arms: &[SelectArm],
    ) -> Option<BasicValueEnum<'ink>> {
        let usize_ty = self.hir_types.get_usize_type();

        // The operands, in source order: (arm, channel, kind, slot).
        let mut cases: Vec<(usize, IntValue<'ink>, u64, PointerValue<'ink>)> = Vec::new();
        let mut slots: Vec<Option<(BasicTypeEnum<'ink>, PointerValue<'ink>)>> =
            vec![None; arms.len()];
        let mut default_arm = None;
        for (idx, arm) in arms.iter().enumerate() {
            match arm.op {
                SelectOp::Receive { channel } => {
                    let handle = self.gen_expr(channel)?.into_int_value();
                    let element_ty = self.channel_element_type(channel);
                    let slot = self
                        .new_alloca_builder()
                        .build_alloca(element_ty, "selected")
                        .expect("failed to allocate a channel element");
                    cases.push((idx, handle, 0, slot));
                    slots[idx] = Some((element_ty, slot));
                }
                SelectOp::Send { channel, value } => {
                    let handle = self.gen_expr(channel)?.into_int_value();
                    let value = self.gen_expr(value)?;
                    let element_ty = self.channel_element_type(channel);
                    let slot = self
                        .new_alloca_builder()
                        .build_alloca(element_ty, "offered")
                        .expect("failed to allocate a channel element");
                    self.builder
                        .build_store(slot, value)
                        .expect("failed to store a channel element");
                    cases.push((idx, handle, 1, slot));
                }
                SelectOp::Default => default_arm = Some(idx),
                SelectOp::Invalid { .. } => {
                    unreachable!(
                        "the task validator rejects a `select` arm that is not a channel operation"
                    )
                }
            }
        }

        // The case array: three `usize`s per operation.
        let words = u32::try_from(cases.len() * 3)
            .expect("a `select` has fewer than 2^32 arms")
            .max(1);
        let array = self
            .new_alloca_builder()
            .build_alloca(usize_ty.array_type(words), "select_cases")
            .expect("failed to allocate the select cases");
        let mut word = 0u64;
        for &(_, handle, kind, slot) in &cases {
            let address = self.address_of(slot);
            for value in [handle, usize_ty.const_int(kind, false), address] {
                // SAFETY: `word` is below the array's length.
                let field = unsafe {
                    self.builder
                        .build_gep(usize_ty, array, &[usize_ty.const_int(word, false)], "case")
                        .expect("failed to address a select case")
                };
                self.builder
                    .build_store(field, value)
                    .expect("failed to store a select case");
                word += 1;
            }
        }
        let array_address = self.address_of(array);
        let count = usize_ty.const_int(cases.len() as u64, false);
        let has_default = usize_ty.const_int(u64::from(default_arm.is_some()), false);
        let chosen = self
            .call_runtime(
                &intrinsics::channel_select,
                &[array_address.into(), count.into(), has_default.into()],
            )
            .expect("`codira.channel_select` returns the chosen case")
            .into_int_value();

        // Switch to the chosen arm.
        let arm_blocks: Vec<_> = (0..arms.len())
            .map(|_| self.context.append_basic_block(self.fn_value, "select_arm"))
            .collect();
        let otherwise = if let Some(idx) = default_arm {
            arm_blocks[idx]
        } else {
            // The runtime returns `count` only for a `default` arm.
            let block = self
                .context
                .append_basic_block(self.fn_value, "select_none");
            let current = self.builder.get_insert_block();
            self.builder.position_at_end(block);
            self.builder
                .build_unreachable()
                .expect("failed to build unreachable");
            if let Some(current) = current {
                self.builder.position_at_end(current);
            }
            block
        };
        let switch_cases: Vec<_> = cases
            .iter()
            .enumerate()
            .map(|(case, &(arm, ..))| (usize_ty.const_int(case as u64, false), arm_blocks[arm]))
            .collect();
        self.builder
            .build_switch(chosen, otherwise, &switch_cases)
            .expect("failed to build the select switch");

        let merge = self
            .context
            .append_basic_block(self.fn_value, "select_merge");
        let mut incoming = Vec::new();
        for (idx, arm) in arms.iter().enumerate() {
            self.builder.position_at_end(arm_blocks[idx]);
            if let (Some(binding), Some((element_ty, slot))) = (arm.binding, slots[idx]) {
                let value = self
                    .builder
                    .build_load(element_ty, slot, "received")
                    .expect("failed to load a channel element");
                self.bind_pattern_to_value(binding, value);
            }
            if let Some(value) = self.gen_expr(arm.body) {
                let block = self
                    .builder
                    .get_insert_block()
                    .expect("an arm ends in a block");
                self.builder
                    .build_unconditional_branch(merge)
                    .expect("failed to branch out of a select arm");
                incoming.push((value, block));
            }
        }

        let Some(&(first, _)) = incoming.first() else {
            // Every arm diverges, so the `select` does too.
            merge
                .remove_from_function()
                .expect("the merge block is in the function");
            return None;
        };
        self.builder.position_at_end(merge);
        let phi = self
            .builder
            .build_phi(first.get_type(), "select_value")
            .expect("failed to build phi");
        for (value, block) in &incoming {
            phi.add_incoming(&[(value, *block)]);
        }
        Some(phi.as_basic_value())
    }

    /// `<-channel`.
    pub(super) fn gen_channel_receive(&mut self, channel: ExprId) -> Option<BasicValueEnum<'ink>> {
        let handle = self.gen_expr(channel)?;
        let element_ty = self.channel_element_type(channel);
        let slot = self
            .new_alloca_builder()
            .build_alloca(element_ty, "received")
            .expect("failed to allocate a channel element");
        let address = self.address_of(slot);
        self.call_runtime(
            &intrinsics::channel_receive,
            &[handle.into(), address.into()],
        );
        Some(
            self.builder
                .build_load(element_ty, slot, "received")
                .expect("failed to load a channel element"),
        )
    }
}

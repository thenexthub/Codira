//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Code generation for algebraic effects: `perform`, `handle` and `resume`,
//!   by evidence passing.
//!
//! # Evidence passing
//!
//! A `handle` expression builds one *handler record* on the stack for each
//! effect it handles, and every function that declares `uses E` receives a
//! pointer to the innermost record for `E` as a hidden parameter (one per
//! effect, after the ordinary parameters, in `uses` order). `perform E.op(..)`
//! is an indirect call through that record. Nothing is captured, copied or
//! unwound: an effect operation costs one load and one indirect call.
//!
//! A record is
//!
//! ```text
//! { i8 state, ptr result_slot, ptr op_0, .., ptr op_n-1, ptr env_0, .., ptr env_m-1 }
//! ```
//!
//! * `op_i` is the arm implementing operation `i`, compiled as a function of
//!   its own: `(op params.., ptr record) -> op return`.
//! * `env_j` is the evidence that was in scope at the `handle` expression. An
//!   arm runs "at" the `handle`, so that is the evidence it performs through;
//!   the record carries it to the arm.
//! * `result_slot` points at the `handle`'s result storage in its frame.
//! * `state` is 0 while the body runs normally, 1 once an arm has finished
//!   *without* resuming (its value is in the result slot and the body must be
//!   abandoned), and 2 when an arm itself was abandoned by an outer handler
//!   (the abort is passing *through* this `handle` on its way out).
//!
//! # Resuming and aborting without unwinding
//!
//! Version-1 handlers resume at most once and only in tail position
//! (`spec/EIDOS_RFC_002.md` section 3.4). So `resume(v)` is simply the arm
//! function returning `v` to the `perform` that called it.
//!
//! An arm that finishes without resuming stores its value in the result
//! slot, sets `state = 1` and returns a dummy. There is no `setjmp`/`longjmp`
//! (and raw ones are unusable on MSVC, see `codira_healing::trap`) and no
//! unwinder, so the abort travels by *return-status propagation*: after
//! every `perform`, and after every call that was passed evidence, the
//! caller checks the `state` of every record in scope. A non-zero state on a
//! record created by a `handle` in this function branches to that `handle`'s
//! exit; on any other record the function returns at once (with a zero
//! value nobody reads), and its caller -- which passed it that record --
//! does the same check. Calls that pass no evidence cannot abort and are
//! not checked; code that uses no effects pays nothing.
//!
//! The rule that makes this complete: a record reaches a function only from
//! its caller (as evidence) or from a `handle` inside it, so every frame
//! between an aborting `perform` and its `handle` holds the record and
//! checks it. Arms are the one exception -- an arm is called by whoever
//! performed, not by the `handle` that owns its outer evidence -- which is
//! what `state = 2` is for: an arm that sees an outer abort marks its own
//! record, which the performer and every frame up to the arm's `handle` do
//! hold, and that `handle` then re-checks its own outer evidence.
//!
//! # Calling a `uses` function from the host
//!
//! The host (`Runtime::invoke`, the reflection table, the dispatch table)
//! only ever sees the ordinary signature. Such functions always get a
//! `_wrapper` entry point that passes, for each effect, a static
//! *unhandled* record whose operations trap: the function runs normally
//! until it actually performs an effect nobody handles, which executes
//! `llvm.trap` instead of reading through a null pointer.
use codira_hir::{Effect, ExprId, HandlerArm, HirDatabase, Pat, Ty, TyKind};
use inkwell::{
    basic_block::BasicBlock,
    module::Linkage,
    types::{BasicMetadataTypeEnum, BasicType, BasicTypeEnum, FunctionType, StructType},
    values::{BasicMetadataValueEnum, BasicValueEnum, FunctionValue, PointerValue},
    AddressSpace, IntPredicate,
};

use super::BodyIrGenerator;

/// Record field holding the abort state.
const STATE_FIELD: u32 = 0;
/// Record field holding the pointer to the `handle`'s result storage.
const SLOT_FIELD: u32 = 1;
/// Index of the first operation pointer.
const FIRST_OP_FIELD: u32 = 2;

/// An abort targeting the record's own `handle`.
const STATE_ABORTED: u64 = 1;
/// An abort passing through the record's `handle` to an outer one.
const STATE_PASSING: u64 = 2;

/// The evidence for one effect, in scope at some point of a body.
#[derive(Clone, Copy)]
pub(super) struct Evidence<'ink> {
    effect: Effect,
    record: PointerValue<'ink>,
    /// The index (into `EffectState::frames`) of the `handle` in this body
    /// that created the record, if one did.
    local_frame: Option<usize>,
    /// Whether an abort can ever be signalled through the record. The static
    /// unhandled records cannot: their operations trap.
    can_abort: bool,
}

/// A `handle` expression of this body whose body is being generated.
struct HandleFrame<'ink> {
    /// Entered when an abort reaches one of this frame's records.
    unwind: BasicBlock<'ink>,
    /// Whether anything branches to `unwind`.
    used: bool,
    /// How many `scope`s were open at the `handle` (see `tasks`): an abort
    /// that lands here ends the ones opened since.
    scope_depth: usize,
}

/// The handler arm being generated, when this generator emits one.
struct ArmFrame<'ink> {
    record: PointerValue<'ink>,
    /// The operation's return type: what `resume` returns.
    op_ret: Ty,
    /// The `handle` expression's type: what finishing the arm produces.
    result_ty: Ty,
}

/// The effect-related state of a `BodyIrGenerator`.
#[derive(Default)]
pub(super) struct EffectState<'ink> {
    /// Evidence in scope, innermost last.
    env: Vec<Evidence<'ink>>,
    frames: Vec<HandleFrame<'ink>>,
    arm: Option<ArmFrame<'ink>>,
    /// The shared block that leaves this function because an abort targets a
    /// `handle` outside it. Created on first use.
    early_exit: Option<BasicBlock<'ink>>,
}

/// The LLVM signature of an arm for an operation, which is also the type a
/// `perform` calls through. `void` for a unit or `never` result.
fn op_fn_type<'ink>(
    generator: &BodyIrGenerator<'_, 'ink, '_>,
    sig: &codira_hir::EffectOpSig,
) -> FunctionType<'ink> {
    let ptr = generator.context.ptr_type(AddressSpace::default());
    let mut params: Vec<BasicMetadataTypeEnum<'ink>> = sig
        .params
        .iter()
        .map(|ty| {
            generator
                .hir_types
                .get_basic_type(ty)
                .expect("an effect operation parameter has a basic type")
                .into()
        })
        .collect();
    params.push(ptr.into());
    match value_type(generator, &sig.ret) {
        Some(ret) => ret.fn_type(&params, false),
        None => generator.context.void_type().fn_type(&params, false),
    }
}

/// The basic type values of `ty` are passed as, or `None` for unit and
/// `never`, which pass nothing.
fn value_type<'ink>(
    generator: &BodyIrGenerator<'_, 'ink, '_>,
    ty: &Ty,
) -> Option<BasicTypeEnum<'ink>> {
    match ty.interned() {
        TyKind::Never | TyKind::Tuple(0, _) => None,
        _ => generator.hir_types.get_basic_type(ty),
    }
}

/// The record type of a `handle` site handling an effect with `ops`
/// operations, carrying `env` outer evidence pointers.
fn record_type<'ink>(
    generator: &BodyIrGenerator<'_, 'ink, '_>,
    ops: usize,
    env: usize,
) -> StructType<'ink> {
    let ptr = generator.context.ptr_type(AddressSpace::default());
    let mut fields: Vec<BasicTypeEnum<'ink>> = vec![generator.context.i8_type().into(), ptr.into()];
    fields.extend(std::iter::repeat_n(BasicTypeEnum::from(ptr), ops + env));
    generator.context.struct_type(&fields, false)
}

impl<'ink> BodyIrGenerator<'_, 'ink, '_> {
    /// Binds this function's hidden evidence parameters, which follow its
    /// `first` ordinary LLVM parameters.
    pub(super) fn bind_evidence_params(&mut self, first: usize) {
        for (idx, effect) in self.hir_function.effects(self.db).into_iter().enumerate() {
            let record = self
                .fn_value
                .get_nth_param((first + idx) as u32)
                .expect("a `uses` function has one evidence parameter per effect")
                .into_pointer_value();
            self.effects.env.push(Evidence {
                effect,
                record,
                local_frame: None,
                can_abort: true,
            });
        }
    }

    /// Installs the static unhandled records as this (wrapper) function's
    /// evidence. See the module documentation.
    pub(super) fn bind_unhandled_evidence(&mut self) {
        for effect in self.hir_function.effects(self.db) {
            let record = self.unhandled_record(effect);
            self.effects.env.push(Evidence {
                effect,
                record,
                local_frame: None,
                can_abort: false,
            });
        }
    }

    /// The evidence arguments a call to `function` needs, innermost record
    /// per effect. Empty for a function without a `uses` clause.
    pub(super) fn evidence_args(
        &mut self,
        function: codira_hir::Function,
    ) -> Vec<BasicMetadataValueEnum<'ink>> {
        function
            .effects(self.db)
            .into_iter()
            .map(|effect| self.evidence_for(effect).into())
            .collect()
    }

    /// The innermost record for `effect`. The effect validator guarantees
    /// one is in scope; the unhandled record stands in otherwise, so a
    /// missed diagnostic traps rather than reading through garbage.
    fn evidence_for(&mut self, effect: Effect) -> PointerValue<'ink> {
        match self.effects.env.iter().rev().find(|e| e.effect == effect) {
            Some(evidence) => evidence.record,
            None => self.unhandled_record(effect),
        }
    }

    /// After a `perform` or a call that was passed evidence: if any record in
    /// scope has been aborted, leave for its `handle` (or out of this
    /// function).
    pub(super) fn gen_abort_check(&mut self) {
        if !self.effects.env.iter().any(|evidence| evidence.can_abort) {
            return;
        }
        let env = self.effects.env.clone();
        let continue_block = self.context.append_basic_block(self.fn_value, "no_abort");
        self.gen_abort_check_of(&env, continue_block);
        self.builder.position_at_end(continue_block);
    }

    /// Branches to the right place if any of `env`'s records is aborted,
    /// and to `otherwise` if none is. Innermost records are checked first.
    fn gen_abort_check_of(&mut self, env: &[Evidence<'ink>], otherwise: BasicBlock<'ink>) {
        let mut checked: Vec<PointerValue<'ink>> = Vec::new();
        for evidence in env.iter().rev() {
            if !evidence.can_abort || checked.contains(&evidence.record) {
                continue;
            }
            checked.push(evidence.record);
            let state = self
                .builder
                .build_load(self.context.i8_type(), evidence.record, "abort_state")
                .expect("failed to load handler state")
                .into_int_value();
            let aborted = self
                .builder
                .build_int_compare(
                    IntPredicate::NE,
                    state,
                    self.context.i8_type().const_zero(),
                    "aborted",
                )
                .expect("failed to compare handler state");
            let (target, keep_scopes) = match evidence.local_frame {
                Some(frame) => {
                    self.effects.frames[frame].used = true;
                    (
                        self.effects.frames[frame].unwind,
                        self.effects.frames[frame].scope_depth,
                    )
                }
                None => (self.early_exit_block(), 0),
            };
            let target = self.through_scopes(target, keep_scopes);
            let next = self
                .context
                .append_basic_block(self.fn_value, "not_aborted");
            self.builder
                .build_conditional_branch(aborted, target, next)
                .expect("failed to branch on handler state");
            self.builder.position_at_end(next);
        }
        self.builder
            .build_unconditional_branch(otherwise)
            .expect("failed to branch past abort check");
    }

    /// `target`, or -- when `scope`s opened after the first `keep` are still
    /// open here -- a block that ends them and then goes to `target`, so an
    /// abort never leaves a scope without joining its tasks.
    fn through_scopes(&mut self, target: BasicBlock<'ink>, keep: usize) -> BasicBlock<'ink> {
        if self.tasks.scopes.len() <= keep {
            return target;
        }
        let current = self.builder.get_insert_block();
        let block = self
            .context
            .append_basic_block(self.fn_value, "abort_scopes");
        self.builder.position_at_end(block);
        self.gen_end_scopes(keep);
        self.builder
            .build_unconditional_branch(target)
            .expect("failed to branch after ending scopes");
        if let Some(current) = current {
            self.builder.position_at_end(current);
        }
        block
    }

    /// The block that leaves this function because an abort targets a
    /// `handle` outside it.
    fn early_exit_block(&mut self) -> BasicBlock<'ink> {
        if let Some(block) = self.effects.early_exit {
            return block;
        }
        let current = self.builder.get_insert_block();
        let block = self.context.append_basic_block(self.fn_value, "abort_exit");
        self.builder.position_at_end(block);
        // An arm abandoned by an outer handler marks its own record, so the
        // frames between the `perform` and this arm's `handle` -- which all
        // hold that record -- leave too.
        let arm = self
            .effects
            .arm
            .as_ref()
            .map(|arm| (arm.record, arm.op_ret.clone()));
        if let Some((record, op_ret)) = arm {
            self.builder
                .build_store(
                    record,
                    self.context.i8_type().const_int(STATE_PASSING, false),
                )
                .expect("failed to mark handler record");
            self.build_dummy_return(&op_ret);
        } else {
            let ret = self
                .hir_function
                .ty(self.db)
                .callable_sig(self.db)
                .expect("a function has a signature")
                .ret()
                .clone();
            self.build_dummy_return(&ret);
        }
        if let Some(current) = current {
            self.builder.position_at_end(current);
        }
        self.effects.early_exit = Some(block);
        block
    }

    /// Returns a value of `ty` that nobody will read.
    fn build_dummy_return(&mut self, ty: &Ty) {
        match value_type(self, ty) {
            Some(ty) => self
                .builder
                .build_return(Some(&ty.const_zero()))
                .expect("failed to build return"),
            None => self
                .builder
                .build_return(None)
                .expect("failed to build return"),
        };
    }

    /// `perform Effect.op(args)`.
    pub(super) fn gen_perform(
        &mut self,
        expr: ExprId,
        args: &[ExprId],
    ) -> Option<BasicValueEnum<'ink>> {
        let (effect, op) = self
            .infer
            .perform_target(expr)
            .expect("the effect validator rejects a `perform` that does not resolve");
        let sig = effect.op_sig(self.db, op);

        let mut call_args: Vec<BasicMetadataValueEnum<'ink>> = Vec::with_capacity(args.len() + 1);
        for arg in args {
            call_args.push(self.gen_expr(*arg)?.into());
        }
        let record = self.evidence_for(effect);
        call_args.push(record.into());

        let ops = effect.op_count(self.db);
        let prefix = record_type(self, ops, 0);
        let slot = self
            .builder
            .build_struct_gep(prefix, record, FIRST_OP_FIELD + op as u32, "op_slot")
            .expect("failed to address handler operation");
        let callee = self
            .builder
            .build_load(self.context.ptr_type(AddressSpace::default()), slot, "op")
            .expect("failed to load handler operation")
            .into_pointer_value();
        let fn_type = op_fn_type(self, &sig);
        let call = self
            .builder
            .build_indirect_call(
                fn_type,
                callee,
                &call_args,
                &format!("perform.{}", sig.name),
            )
            .expect("failed to build perform");
        let value = call.try_as_basic_value().basic();

        self.gen_abort_check();

        if sig.ret.is_never() {
            self.builder
                .build_unreachable()
                .expect("failed to build unreachable");
            return None;
        }
        Some(value.unwrap_or_else(|| self.context.const_struct(&[], false).into()))
    }

    /// `resume(value)`: the arm returns `value` to the `perform`.
    pub(super) fn gen_resume(&mut self, value: Option<ExprId>) -> Option<BasicValueEnum<'ink>> {
        let op_ret = self
            .effects
            .arm
            .as_ref()
            .expect("the effect validator rejects `resume` outside a handler arm")
            .op_ret
            .clone();
        let value = match value {
            Some(value) => Some(self.gen_expr(value)?),
            None => None,
        };
        // The arm's own scopes end before it returns to the `perform`.
        self.gen_end_scopes(0);
        match (value_type(self, &op_ret), value) {
            (Some(_), Some(value)) => self
                .builder
                .build_return(Some(&value))
                .expect("failed to build resume"),
            _ => self
                .builder
                .build_return(None)
                .expect("failed to build resume"),
        };
        None
    }

    /// `handle body { arms }`.
    pub(super) fn gen_handle(
        &mut self,
        expr: ExprId,
        body: ExprId,
        arms: &[HandlerArm],
    ) -> Option<BasicValueEnum<'ink>> {
        let result_ty = self.infer[expr].clone();
        let slot_ty = value_type(self, &result_ty)
            .or_else(|| {
                result_ty
                    .is_empty()
                    .then(|| self.context.struct_type(&[], false).into())
            })
            .unwrap_or_else(|| self.context.i8_type().into());
        let slot = self
            .new_alloca_builder()
            .build_alloca(slot_ty, "handle_result")
            .expect("failed to allocate handle result");

        // One record per handled effect, in the order the arms first name
        // them.
        let mut handled: Vec<(Effect, Vec<Option<usize>>)> = Vec::new();
        for idx in 0..arms.len() {
            let (effect, op) = self
                .infer
                .handler_arm_target(expr, idx)
                .expect("the effect validator rejects an arm that does not resolve");
            let entry = handled
                .iter()
                .position(|(e, _)| *e == effect)
                .unwrap_or_else(|| {
                    handled.push((effect, vec![None; effect.op_count(self.db)]));
                    handled.len() - 1
                });
            handled[entry].1[op] = Some(idx);
        }

        let outer_env = self.effects.env.clone();
        let frame = self.effects.frames.len();
        let unwind = self
            .context
            .append_basic_block(self.fn_value, "handle_unwind");
        self.effects.frames.push(HandleFrame {
            unwind,
            used: false,
            scope_depth: self.tasks.scopes.len(),
        });

        let mut records = Vec::with_capacity(handled.len());
        for (effect, arm_for_op) in &handled {
            let record_ty = record_type(self, arm_for_op.len(), outer_env.len());
            let record = self
                .new_alloca_builder()
                .build_alloca(record_ty, &format!("handler.{}", effect.name(self.db)))
                .expect("failed to allocate handler record");
            self.store_field(
                record_ty,
                record,
                STATE_FIELD,
                self.context.i8_type().const_zero().into(),
            );
            self.store_field(record_ty, record, SLOT_FIELD, slot.into());
            for (op, arm_idx) in arm_for_op.iter().enumerate() {
                let arm_idx =
                    arm_idx.expect("the effect validator rejects a handler missing an operation");
                let arm_fn = self.gen_arm_function(
                    expr,
                    &arms[arm_idx],
                    *effect,
                    op,
                    record_ty,
                    &outer_env,
                    &result_ty,
                );
                self.store_field(
                    record_ty,
                    record,
                    FIRST_OP_FIELD + op as u32,
                    arm_fn.as_global_value().as_pointer_value().into(),
                );
            }
            for (idx, evidence) in outer_env.iter().enumerate() {
                self.store_field(
                    record_ty,
                    record,
                    FIRST_OP_FIELD + (arm_for_op.len() + idx) as u32,
                    evidence.record.into(),
                );
            }
            records.push(record);
            self.effects.env.push(Evidence {
                effect: *effect,
                record,
                local_frame: Some(frame),
                can_abort: true,
            });
        }

        let body_value = self.gen_expr(body);
        let body_end = self.builder.get_insert_block();
        self.effects.env.truncate(outer_env.len());
        let used = self.effects.frames.pop().is_some_and(|frame| frame.used);

        let merge = self
            .context
            .append_basic_block(self.fn_value, "handle_merge");
        let mut incoming: Vec<(BasicValueEnum<'ink>, BasicBlock<'ink>)> = Vec::new();
        if let (Some(value), Some(end)) = (body_value, body_end) {
            self.builder
                .build_unconditional_branch(merge)
                .expect("failed to branch out of handle body");
            incoming.push((value, end));
        }

        if used {
            // An abort reached one of this handle's records: either an arm
            // finished without resuming (state 1: its value is the result),
            // or an outer handler abandoned an arm (state 2: keep leaving).
            self.builder.position_at_end(unwind);
            let mut aborted_here = self.context.bool_type().const_zero();
            for record in &records {
                let state = self
                    .builder
                    .build_load(self.context.i8_type(), *record, "handle_state")
                    .expect("failed to load handler state")
                    .into_int_value();
                let is_ours = self
                    .builder
                    .build_int_compare(
                        IntPredicate::EQ,
                        state,
                        self.context.i8_type().const_int(STATE_ABORTED, false),
                        "aborted_here",
                    )
                    .expect("failed to compare handler state");
                aborted_here = self
                    .builder
                    .build_or(aborted_here, is_ours, "aborted_here")
                    .expect("failed to combine handler states");
            }
            let take_result = self
                .context
                .append_basic_block(self.fn_value, "handle_result");
            let pass_through = self
                .context
                .append_basic_block(self.fn_value, "handle_pass");
            self.builder
                .build_conditional_branch(aborted_here, take_result, pass_through)
                .expect("failed to branch on handler state");

            self.builder.position_at_end(pass_through);
            let stuck = self
                .context
                .append_basic_block(self.fn_value, "handle_stuck");
            self.gen_abort_check_of(&outer_env, stuck);
            self.builder.position_at_end(stuck);
            self.builder
                .build_unreachable()
                .expect("failed to build unreachable");

            self.builder.position_at_end(take_result);
            if result_ty.is_never() {
                self.builder
                    .build_unreachable()
                    .expect("failed to build unreachable");
            } else {
                let value = self
                    .builder
                    .build_load(slot_ty, slot, "handle_value")
                    .expect("failed to load handle result");
                self.builder
                    .build_unconditional_branch(merge)
                    .expect("failed to branch out of handle");
                incoming.push((value, take_result));
            }
        } else {
            unwind
                .remove_from_function()
                .expect("the unwind block belongs to this function");
        }

        self.builder.position_at_end(merge);
        match incoming.as_slice() {
            [] => {
                merge
                    .remove_from_function()
                    .expect("the merge block belongs to this function");
                None
            }
            [(value, _)] => Some(*value),
            _ => {
                let phi = self
                    .builder
                    .build_phi(incoming[0].0.get_type(), "handle_value")
                    .expect("failed to build phi");
                for (value, block) in &incoming {
                    phi.add_incoming(&[(value, *block)]);
                }
                Some(phi.as_basic_value())
            }
        }
    }

    fn store_field(
        &self,
        record_ty: StructType<'ink>,
        record: PointerValue<'ink>,
        field: u32,
        value: BasicValueEnum<'ink>,
    ) {
        let ptr = self
            .builder
            .build_struct_gep(record_ty, record, field, "record_field")
            .expect("failed to address handler record field");
        self.builder
            .build_store(ptr, value)
            .expect("failed to initialize handler record field");
    }

    /// Compiles one handler arm to a function of its own (see the module
    /// documentation for its signature and what finishing it means).
    #[allow(clippy::too_many_arguments)]
    fn gen_arm_function(
        &self,
        handle: ExprId,
        arm: &HandlerArm,
        effect: Effect,
        op: usize,
        record_ty: StructType<'ink>,
        outer_env: &[Evidence<'ink>],
        result_ty: &Ty,
    ) -> FunctionValue<'ink> {
        let sig = effect.op_sig(self.db, op);
        let fn_type = op_fn_type(self, &sig);
        let name = format!(
            "{}.handle{}.{}.{}",
            self.fn_value.get_name().to_string_lossy(),
            u32::from(handle.into_raw()),
            effect.name(self.db),
            sig.name
        );
        let arm_fn = self
            .module
            .add_function(&name, fn_type, Some(Linkage::Private));

        let mut generator = BodyIrGenerator::new(
            self.context,
            self.module,
            self.db,
            (self.hir_function, arm_fn),
            self.function_map,
            self.dispatch_table,
            self.type_table,
            self.external_globals.clone(),
            self.hir_types,
            self.module_group,
            None,
        );
        let record = arm_fn
            .get_nth_param(sig.params.len() as u32)
            .expect("an arm's last parameter is its record")
            .into_pointer_value();
        generator.effects.arm = Some(ArmFrame {
            record,
            op_ret: sig.ret.clone(),
            result_ty: result_ty.clone(),
        });

        // The arm performs through the evidence that was in scope at the
        // `handle`, which the record carries after its operations.
        let ops = effect.op_count(self.db);
        for (idx, evidence) in outer_env.iter().enumerate() {
            let field = generator
                .builder
                .build_struct_gep(
                    record_ty,
                    record,
                    FIRST_OP_FIELD + (ops + idx) as u32,
                    "outer_evidence",
                )
                .expect("failed to address outer evidence");
            let outer = generator
                .builder
                .build_load(
                    generator.context.ptr_type(AddressSpace::default()),
                    field,
                    "outer",
                )
                .expect("failed to load outer evidence")
                .into_pointer_value();
            generator.effects.env.push(Evidence {
                effect: evidence.effect,
                record: outer,
                local_frame: None,
                can_abort: evidence.can_abort,
            });
        }

        for (idx, &pat) in arm.params.iter().enumerate() {
            let Some(param) = arm_fn.get_nth_param(idx as u32) else {
                continue;
            };
            generator.bind_arm_param(pat, param);
        }

        generator.gen_arm_body(arm.body);
        arm_fn
    }

    fn bind_arm_param(&mut self, pat: codira_hir::PatId, param: BasicValueEnum<'ink>) {
        let body = self.body.clone();
        match &body[pat] {
            Pat::Bind { name } => {
                let ptr = self
                    .new_alloca_builder()
                    .build_alloca(param.get_type(), &name.to_string())
                    .expect("failed to allocate handler arm parameter");
                self.builder
                    .build_store(ptr, param)
                    .expect("failed to store handler arm parameter");
                self.pat_to_local.insert(pat, ptr);
                self.pat_to_name.insert(pat, name.to_string());
            }
            Pat::Tuple(_) => self.bind_pattern_to_value(pat, param),
            Pat::Wild => {}
            Pat::Missing | Pat::Path(_) | Pat::Literal(_) | Pat::TupleStruct { .. } => {
                unreachable!("the effect validator rejects refutable handler arm parameters")
            }
        }
    }

    /// Generates an arm's body. Finishing it without resuming makes its value
    /// the result of the `handle`: stored in the result slot, `state = 1`.
    fn gen_arm_body(&mut self, body: ExprId) {
        let value = self.gen_expr(body);
        let Some(value) = value else {
            return;
        };
        let arm = self.effects.arm.as_ref().expect("generating an arm");
        let record = arm.record;
        let op_ret = arm.op_ret.clone();
        let result_ty = arm.result_ty.clone();
        if !result_ty.is_never() {
            let ptr_ty = self.context.ptr_type(AddressSpace::default());
            let slot_field = self
                .builder
                .build_struct_gep(record_type(self, 0, 0), record, SLOT_FIELD, "slot_field")
                .expect("failed to address result slot");
            let slot = self
                .builder
                .build_load(ptr_ty, slot_field, "slot")
                .expect("failed to load result slot")
                .into_pointer_value();
            self.builder
                .build_store(slot, value)
                .expect("failed to store handle result");
        }
        self.builder
            .build_store(
                record,
                self.context.i8_type().const_int(STATE_ABORTED, false),
            )
            .expect("failed to mark handler record");
        self.build_dummy_return(&op_ret);
    }

    /// The static record standing in for `effect` when nothing handles it:
    /// every operation traps.
    fn unhandled_record(&self, effect: Effect) -> PointerValue<'ink> {
        let name = format!("codira.unhandled.{}", effect.name(self.db));
        if let Some(global) = self.module.get_global(&name) {
            return global.as_pointer_value();
        }
        let ops = effect.op_count(self.db);
        let record_ty = record_type(self, ops, 0);
        let ptr_ty = self.context.ptr_type(AddressSpace::default());
        let mut fields: Vec<BasicValueEnum<'ink>> = vec![
            self.context.i8_type().const_zero().into(),
            ptr_ty.const_null().into(),
        ];
        for op in 0..ops {
            let stub = self.unhandled_op(effect, op);
            fields.push(stub.as_global_value().as_pointer_value().into());
        }
        let global = self.module.add_global(record_ty, None, &name);
        global.set_linkage(Linkage::Private);
        global.set_constant(true);
        global.set_initializer(&record_ty.const_named_struct(&fields));
        global.as_pointer_value()
    }

    /// An operation implementation that traps: performing an effect nothing
    /// handles.
    fn unhandled_op(&self, effect: Effect, op: usize) -> FunctionValue<'ink> {
        let sig = effect.op_sig(self.db, op);
        let name = format!("codira.unhandled.{}.{}", effect.name(self.db), sig.name);
        if let Some(function) = self.module.get_function(&name) {
            return function;
        }
        let function =
            self.module
                .add_function(&name, op_fn_type(self, &sig), Some(Linkage::Private));
        let builder = self.context.create_builder();
        builder.position_at_end(self.context.append_basic_block(function, "unhandled"));
        let trap = inkwell::intrinsics::Intrinsic::find("llvm.trap")
            .and_then(|intrinsic| intrinsic.get_declaration(self.module, &[]))
            .expect("llvm.trap is always available");
        builder
            .build_call(trap, &[], "")
            .expect("failed to build trap");
        builder
            .build_unreachable()
            .expect("failed to build unreachable");
        function
    }
}

/// Whether a function takes hidden evidence parameters, and so needs a
/// host-facing wrapper.
pub(crate) fn has_evidence(db: &dyn HirDatabase, function: codira_hir::Function) -> bool {
    !function.effects(db).is_empty()
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Code generation for the experimental hardware-safety modes (see
//!   `crate::hardware_safety`): ARM MTE tagging of `stack_alloc` slots under
//!   `--sanitize=memtag`.
//!
//! # `stack_alloc` under `--sanitize=memtag`
//!
//! The slot is allocated once in the entry block, as without the mode, but
//! rounded up to whole 16-byte MTE granules and aligned to at least 16 so no
//! granule is shared with a neighbour. Right after it, in the entry block:
//!
//! ```text
//! %tagged = call ptr @llvm.aarch64.irg(ptr %slot, i64 1)   ; IRG, never tag 0
//! call void @llvm.aarch64.settag(ptr %tagged, i64 <size>)   ; STG/ST2G per granule
//! ```
//!
//! and `stack_alloc` returns `%tagged` as a `usize`. Before every `ret` of the
//! function, `llvm.aarch64.settag(%slot, <size>)` writes tag 0 back (the
//! untagged slot address carries tag 0), so a pointer kept past the frame
//! no longer matches its memory. Excluding tag 0 is what makes an access
//! through an *untagged* pointer -- one forged from an integer, or computed
//! from another frame's stack pointer -- fault as well.
//!
//! A `stack_alloc` whose size is computed at run time is refused: its slot is
//! a dynamic `alloca` that may run any number of times, and untagging every
//! one of them at each return needs bookkeeping this does not do yet.

use codira_hir::ExprId;
use inkwell::{
    types::BasicType,
    values::{BasicMetadataValueEnum, BasicValueEnum, PointerValue},
};

use super::BodyIrGenerator;

/// The MTE granule: tags cover memory in 16-byte units.
const GRANULE: u64 = 16;

/// What a hardware-safety hook did with an expression.
pub(super) enum Lowered<'ink> {
    /// Nothing: the expression is generated as without the mode.
    Unchanged,
    /// The hook generated it; this is its value (`None` for `never`).
    Emitted(Option<BasicValueEnum<'ink>>),
}

/// What a function's hardware-safety code generation has to undo before it
/// returns.
#[derive(Default)]
pub(super) struct SafetyState<'ink> {
    /// `stack_alloc` slots tagged under `--sanitize=memtag`, with their
    /// granule-rounded sizes: untagged again before every return.
    tagged_slots: Vec<(PointerValue<'ink>, u64)>,
}

impl<'ink> BodyIrGenerator<'_, 'ink, '_> {
    /// Emits the intrinsic call `name(args)` the way the hardware-safety mode
    /// needs it, or returns `None` when the mode does not change it.
    pub(super) fn gen_safety_intrinsic(
        &mut self,
        expr: ExprId,
        name: &str,
        args: &[BasicMetadataValueEnum<'ink>],
    ) -> Lowered<'ink> {
        let name = name.strip_prefix("__intrinsic_").unwrap_or(name);
        if self.external_globals.hardware_safety.memtag && name == "stack_alloc" && args.len() == 2
        {
            if let Some(value) = self.gen_memtag_stack_alloc(expr, args) {
                return Lowered::Emitted(Some(value.into()));
            }
        }
        Lowered::Unchanged
    }

    /// `file:line:column` of `expr`, for a refusal.
    fn location_text(&self, expr: ExprId) -> String {
        match self.hir_function.expr_source_range(self.db, expr) {
            Some((file_id, range)) => {
                let line_col = self.db.line_index(file_id).line_col(range.start());
                format!(
                    "{}:{}:{}",
                    self.db.file_relative_path(file_id),
                    line_col.line + 1,
                    line_col.col_utf16 + 1
                )
            }
            None => "<unknown>:0:0".to_owned(),
        }
    }

    /// Records that the mode cannot protect `expr`; the build then fails
    /// with every such place listed.
    fn refuse_under_safety(&self, expr: ExprId, reason: &str) {
        let location = self.location_text(expr);
        self.external_globals
            .safety_refusals
            .borrow_mut()
            .push(format!("{location}: {reason}"));
    }

    /// `stack_alloc(size, align)` with an MTE-tagged slot. Returns `None`
    /// (after recording why) when the size is not a constant, so the caller
    /// emits the plain slot and the build fails on the refusal.
    fn gen_memtag_stack_alloc(
        &mut self,
        expr: ExprId,
        args: &[BasicMetadataValueEnum<'ink>],
    ) -> Option<inkwell::values::IntValue<'ink>> {
        let Some(size) = args[0].into_int_value().get_zero_extended_constant() else {
            self.refuse_under_safety(
                expr,
                "`stack_alloc` with a size computed at run time cannot be tagged by \
                 --sanitize=memtag (experimental) yet; give it a constant size, or allocate \
                 from a region",
            );
            return None;
        };
        let align = args[1]
            .into_int_value()
            .get_zero_extended_constant()
            .filter(|a| a.is_power_of_two() && *a <= 1 << 16)
            .unwrap_or(GRANULE)
            .max(GRANULE);
        let size = size.max(1).next_multiple_of(GRANULE);

        let context = self.context;
        let i64_type = context.i64_type();
        let entry = self.new_alloca_builder();
        let slot = entry
            .build_alloca(
                context
                    .i8_type()
                    .array_type(u32::try_from(size).unwrap_or(u32::MAX))
                    .as_basic_type_enum(),
                "stack_alloc.tagged",
            )
            .expect("could not build the tagged stack slot");
        slot.as_instruction()
            .expect("an alloca is an instruction")
            .set_alignment(u32::try_from(align).unwrap_or(16))
            .expect("alignment is a power of two");

        let irg = self.llvm_intrinsic("llvm.aarch64.irg");
        let tagged = entry
            .build_call(
                irg,
                &[slot.into(), i64_type.const_int(1, false).into()],
                "stack_alloc.irg",
            )
            .expect("could not build irg")
            .try_as_basic_value()
            .basic()
            .expect("irg returns a pointer")
            .into_pointer_value();
        let settag = self.llvm_intrinsic("llvm.aarch64.settag");
        entry
            .build_call(
                settag,
                &[tagged.into(), i64_type.const_int(size, false).into()],
                "",
            )
            .expect("could not build settag");
        self.safety.tagged_slots.push((slot, size));

        let usize_type = context.ptr_sized_int_type(self.hir_types.target_data(), None);
        Some(
            self.builder
                .build_ptr_to_int(tagged, usize_type, "stack_alloc.addr")
                .expect("could not build the slot's address"),
        )
    }

    /// The declaration of a non-overloaded LLVM intrinsic.
    fn llvm_intrinsic(&self, name: &str) -> inkwell::values::FunctionValue<'ink> {
        inkwell::intrinsics::Intrinsic::find(name)
            .and_then(|intrinsic| intrinsic.get_declaration(self.module, &[]))
            .unwrap_or_else(|| panic!("LLVM provides `{name}`"))
    }

    /// Undoes what the hardware-safety mode set up in this function before
    /// each of its returns: tagged stack slots get tag 0 back.
    pub(super) fn finish_safety(&mut self) {
        if self.safety.tagged_slots.is_empty() {
            return;
        }
        let returns: Vec<_> = self
            .fn_value
            .get_basic_block_iter()
            .filter_map(inkwell::basic_block::BasicBlock::get_terminator)
            .filter(|terminator| {
                terminator.get_opcode() == inkwell::values::InstructionOpcode::Return
            })
            .collect();
        let settag = self.llvm_intrinsic("llvm.aarch64.settag");
        let i64_type = self.context.i64_type();
        let builder = self.context.create_builder();
        for ret in returns {
            builder.position_before(&ret);
            for &(slot, size) in &self.safety.tagged_slots {
                builder
                    .build_call(
                        settag,
                        &[slot.into(), i64_type.const_int(size, false).into()],
                        "",
                    )
                    .expect("could not build settag");
            }
        }
    }
}

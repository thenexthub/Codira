//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Machine-level memory intrinsics: stack allocation in the caller's frame,
//!   prefetch hints, non-temporal loads and stores, fences, atomic
//!   read-modify-write on addresses, and the target's cache-line size.
//!
//! Each is one or a few instructions, emitted where it is called, with
//! addresses crossing as `usize` like every other `extern "codira-intrinsic"`
//! memory operation:
//!
//! | intrinsic | emits |
//! |---|---|
//! | `stack_alloc(size, align) -> usize` | `alloca` in the calling function's frame |
//! | `prefetch_read(addr)`, `prefetch_write(addr)`, `prefetch_read_once(addr)` | `llvm.prefetch` |
//! | `load_nt_<t>(addr)`, `store_nt_<t>(addr, v)` | a load/store tagged `!nontemporal` |
//! | `fence_acquire()`, `fence_release()`, `fence_acq_rel()`, `fence_seq_cst()` | `fence` |
//! | `compiler_fence()` | `fence syncscope("singlethread") seq_cst` |
//! | `atomic_load_<t>(addr)`, `atomic_store_<t>(addr, v)` | `seq_cst` load/store |
//! | `atomic_<op>_<t>(addr, v) -> old` for `add sub and or xor swap max min` | `atomicrmw` |
//! | `atomic_cas_<t>(addr, expected, new) -> old` | `cmpxchg` |
//! | `cache_line_size() -> usize` | a constant for the target |
//! | `frame_address() -> usize` | `llvm.frameaddress` |
//!
//! `<t>` is `u8`/`i8` through `u64`/`i64` (and `f32`/`f64` for the
//! non-temporal and atomic load/store). Atomics are sequentially consistent:
//! the strongest ordering, so a program written against them is correct on
//! every target without reasoning about weaker ones.
//!
//! # `stack_alloc`
//!
//! The memory belongs to the function that calls `stack_alloc` and is freed
//! when that function returns, in O(1), with nothing to free by hand; using
//! the address after that is undefined, exactly as in C. When `size` and
//! `align` are constants, as they usually are, the slot is allocated once in
//! the function's entry block, so calling `stack_alloc` in a loop reuses one
//! slot instead of growing the stack each iteration. A size computed at run
//! time allocates where it is called, and in a loop that does grow the stack
//! every iteration -- the same trade-off as C's `alloca`.

use inkwell::{
    builder::Builder,
    context::Context,
    module::Module,
    targets::TargetData,
    types::BasicType,
    values::{BasicMetadataValueEnum, BasicValue, BasicValueEnum, IntValue},
    AddressSpace, AtomicOrdering, AtomicRMWBinOp,
};

use super::{IntrinsicError, Scalar};

/// What a low-level intrinsic produced.
pub(super) enum Emitted<'ink> {
    Value(BasicValueEnum<'ink>),
    Unit,
}

fn arity(args: &[BasicMetadataValueEnum<'_>], expected: usize) -> Result<(), IntrinsicError> {
    if args.len() == expected {
        Ok(())
    } else {
        Err(IntrinsicError::Arity {
            expected,
            found: args.len(),
        })
    }
}

/// Emits `name` if it is one of this module's intrinsics.
pub(super) fn gen_low_level_op<'ink>(
    name: &str,
    args: &[BasicMetadataValueEnum<'ink>],
    context: &'ink Context,
    module: &Module<'ink>,
    builder: &Builder<'ink>,
    target_data: &TargetData,
) -> Result<Option<Emitted<'ink>>, IntrinsicError> {
    let ptr_type = context.ptr_type(AddressSpace::default());
    let usize_type = context.ptr_sized_int_type(target_data, None);
    let address = |index: usize| {
        builder
            .build_int_to_ptr(args[index].into_int_value(), ptr_type, "intrinsic.addr")
            .expect("could not build address cast")
    };

    match name {
        "stack_alloc" => {
            arity(args, 2)?;
            let size = args[0].into_int_value();
            let align = args[1].into_int_value();
            let slot = gen_stack_alloc(context, builder, size, align);
            let address = builder
                .build_ptr_to_int(slot, usize_type, "stack_alloc.addr")
                .expect("could not build address of stack slot");
            return Ok(Some(Emitted::Value(address.into())));
        }
        "prefetch_read" | "prefetch_write" | "prefetch_read_once" => {
            arity(args, 1)?;
            // `llvm.prefetch(ptr, rw, locality, cache type)`: rw 0 = read,
            // 1 = write; locality 0 = no temporal locality (use once) up to
            // 3 = keep in all levels; cache type 1 = data.
            let (rw, locality) = match name {
                "prefetch_read" => (0, 3),
                "prefetch_write" => (1, 3),
                _ => (0, 0),
            };
            let i32_type = context.i32_type();
            let prefetch = inkwell::intrinsics::Intrinsic::find("llvm.prefetch")
                .expect("llvm.prefetch exists")
                .get_declaration(module, &[ptr_type.into()])
                .expect("llvm.prefetch has a declaration for ptr");
            builder
                .build_direct_call(
                    prefetch,
                    &[
                        address(0).into(),
                        i32_type.const_int(rw, false).into(),
                        i32_type.const_int(locality, false).into(),
                        i32_type.const_int(1, false).into(),
                    ],
                    "",
                )
                .expect("could not build prefetch");
            return Ok(Some(Emitted::Unit));
        }
        "fence_acquire" | "fence_release" | "fence_acq_rel" | "fence_seq_cst"
        | "compiler_fence" => {
            arity(args, 0)?;
            let (ordering, single_thread) = match name {
                "fence_acquire" => (AtomicOrdering::Acquire, false),
                "fence_release" => (AtomicOrdering::Release, false),
                "fence_acq_rel" => (AtomicOrdering::AcquireRelease, false),
                "fence_seq_cst" => (AtomicOrdering::SequentiallyConsistent, false),
                _ => (AtomicOrdering::SequentiallyConsistent, true),
            };
            builder
                .build_fence(ordering, single_thread, "")
                .expect("could not build fence");
            return Ok(Some(Emitted::Unit));
        }
        "cache_line_size" => {
            arity(args, 0)?;
            // Apple's AArch64 cores use 128-byte lines; every other target
            // Codira supports uses 64.
            let triple = module.get_triple();
            let triple = triple.as_str().to_string_lossy();
            let is_apple_arm = (triple.starts_with("arm64") || triple.starts_with("aarch64"))
                && triple.contains("apple");
            let size = if is_apple_arm { 128 } else { 64 };
            return Ok(Some(Emitted::Value(
                usize_type.const_int(size, false).into(),
            )));
        }
        "frame_address" => {
            arity(args, 0)?;
            let frame_address = inkwell::intrinsics::Intrinsic::find("llvm.frameaddress")
                .expect("llvm.frameaddress exists")
                .get_declaration(module, &[ptr_type.into()])
                .expect("llvm.frameaddress has a declaration for ptr");
            let frame = builder
                .build_direct_call(
                    frame_address,
                    &[context.i32_type().const_zero().into()],
                    "frame",
                )
                .expect("could not build frame address")
                .try_as_basic_value()
                .basic()
                .expect("llvm.frameaddress returns a pointer")
                .into_pointer_value();
            let frame = builder
                .build_ptr_to_int(frame, usize_type, "frame.addr")
                .expect("could not cast frame address");
            return Ok(Some(Emitted::Value(frame.into())));
        }
        _ => {}
    }

    if let Some(suffix) = name.strip_prefix("load_nt_") {
        let scalar = Scalar::from_suffix(suffix).ok_or(IntrinsicError::Unknown)?;
        arity(args, 1)?;
        let loaded = builder
            .build_load(scalar.ir_type(context), address(0), "intrinsic.load_nt")
            .expect("could not build non-temporal load");
        let instruction = loaded
            .as_instruction_value()
            .expect("a load is an instruction");
        instruction
            .set_alignment(scalar.alignment())
            .expect("alignment is a power of two");
        mark_non_temporal(context, instruction);
        return Ok(Some(Emitted::Value(loaded)));
    }

    if let Some(suffix) = name.strip_prefix("store_nt_") {
        let scalar = Scalar::from_suffix(suffix).ok_or(IntrinsicError::Unknown)?;
        arity(args, 2)?;
        let value: BasicValueEnum<'_> = args[1]
            .try_into()
            .expect("store value must be a basic value");
        let store = builder
            .build_store(address(0), value)
            .expect("could not build non-temporal store");
        store
            .set_alignment(scalar.alignment())
            .expect("alignment is a power of two");
        mark_non_temporal(context, store);
        return Ok(Some(Emitted::Unit));
    }

    let Some(rest) = name.strip_prefix("atomic_") else {
        return Ok(None);
    };
    let Some((op, suffix)) = rest.rsplit_once('_') else {
        return Ok(None);
    };
    let scalar = Scalar::from_suffix(suffix).ok_or(IntrinsicError::Unknown)?;
    let seq_cst = AtomicOrdering::SequentiallyConsistent;

    match op {
        "load" => {
            arity(args, 1)?;
            let loaded = builder
                .build_load(scalar.ir_type(context), address(0), "atomic.load")
                .expect("could not build atomic load");
            let instruction = loaded
                .as_instruction_value()
                .expect("a load is an instruction");
            instruction
                .set_alignment(scalar.alignment())
                .expect("alignment is a power of two");
            instruction
                .set_atomic_ordering(seq_cst)
                .expect("a load takes an atomic ordering");
            Ok(Some(Emitted::Value(loaded)))
        }
        "store" => {
            arity(args, 2)?;
            let value: BasicValueEnum<'_> = args[1]
                .try_into()
                .expect("store value must be a basic value");
            let store = builder
                .build_store(address(0), value)
                .expect("could not build atomic store");
            store
                .set_alignment(scalar.alignment())
                .expect("alignment is a power of two");
            store
                .set_atomic_ordering(seq_cst)
                .expect("a store takes an atomic ordering");
            Ok(Some(Emitted::Unit))
        }
        "cas" => {
            if scalar.is_float() {
                return Err(IntrinsicError::Unknown);
            }
            arity(args, 3)?;
            let expected = args[1].into_int_value();
            let new = args[2].into_int_value();
            let pair = builder
                .build_cmpxchg(address(0), expected, new, seq_cst, seq_cst)
                .expect("could not build cmpxchg");
            let old = builder
                .build_extract_value(pair, 0, "cas.old")
                .expect("cmpxchg yields { value, success }");
            Ok(Some(Emitted::Value(old)))
        }
        _ => {
            let rmw = match op {
                "add" => AtomicRMWBinOp::Add,
                "sub" => AtomicRMWBinOp::Sub,
                "and" => AtomicRMWBinOp::And,
                "or" => AtomicRMWBinOp::Or,
                "xor" => AtomicRMWBinOp::Xor,
                "swap" => AtomicRMWBinOp::Xchg,
                // Signedness comes from the spelling: `atomic_max_i64`
                // compares as signed, `atomic_max_u64` as unsigned.
                "max" if suffix.starts_with('i') => AtomicRMWBinOp::Max,
                "max" => AtomicRMWBinOp::UMax,
                "min" if suffix.starts_with('i') => AtomicRMWBinOp::Min,
                "min" => AtomicRMWBinOp::UMin,
                _ => return Ok(None),
            };
            if scalar.is_float() {
                return Err(IntrinsicError::Unknown);
            }
            arity(args, 2)?;
            let old = builder
                .build_atomicrmw(rmw, address(0), args[1].into_int_value(), seq_cst)
                .expect("could not build atomicrmw");
            Ok(Some(Emitted::Value(old.into())))
        }
    }
}

/// Tags a load or store `!nontemporal`: the data is not expected to be
/// reused soon, so the target may bypass the cache (`MOVNTI`/`MOVNTDQ` on
/// x86, `LDNP`/`STNP` on `AArch64`) instead of evicting lines that are.
fn mark_non_temporal<'ink>(
    context: &'ink Context,
    instruction: inkwell::values::InstructionValue<'ink>,
) {
    let kind = context.get_kind_id("nontemporal");
    let node = context.metadata_node(&[context.i32_type().const_int(1, false).into()]);
    instruction
        .set_metadata(node, kind)
        .expect("a load or store takes metadata");
}

/// Allocates `size` bytes aligned to `align` in the current function's frame.
///
/// Constant operands put a fixed-size slot in the entry block, which LLVM
/// folds into the frame layout; anything else is a dynamic `alloca` at the
/// call site. An alignment that is not a constant power of two is raised to
/// 16, which suits every scalar and SIMD register Codira targets.
fn gen_stack_alloc<'ink>(
    context: &'ink Context,
    builder: &Builder<'ink>,
    size: IntValue<'ink>,
    align: IntValue<'ink>,
) -> inkwell::values::PointerValue<'ink> {
    let align = align
        .get_zero_extended_constant()
        .filter(|a| a.is_power_of_two() && *a <= 1 << 16)
        .map_or(16, |a| u32::try_from(a).unwrap_or(16));

    let byte = context.i8_type();
    let slot = if let Some(constant_size) = size.get_zero_extended_constant() {
        let function = builder
            .get_insert_block()
            .and_then(inkwell::basic_block::BasicBlock::get_parent)
            .expect("an intrinsic is emitted inside a function");
        let entry = function
            .get_first_basic_block()
            .expect("a function being emitted has an entry block");
        let entry_builder = context.create_builder();
        match entry.get_first_instruction() {
            Some(first) => entry_builder.position_before(&first),
            None => entry_builder.position_at_end(entry),
        }
        let array = byte.array_type(u32::try_from(constant_size.max(1)).unwrap_or(u32::MAX));
        entry_builder
            .build_alloca(array.as_basic_type_enum(), "stack_alloc")
            .expect("could not build entry-block alloca")
    } else {
        builder
            .build_array_alloca(byte, size, "stack_alloc.dynamic")
            .expect("could not build dynamic alloca")
    };
    slot.as_instruction()
        .expect("an alloca is an instruction")
        .set_alignment(align)
        .expect("alignment is a power of two");
    slot
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The GPU thread-geometry intrinsics a `@target(gpu)` kernel uses to find
//!   its own work item, and the block-wide barrier.
//!
//! | intrinsic | on NVPTX | on any other target |
//! |---|---|---|
//! | `gpu_thread_id_x/y/z() -> u32` | `%tid.x/y/z` | `0` |
//! | `gpu_block_id_x/y/z() -> u32` | `%ctaid.x/y/z` | `0` |
//! | `gpu_block_dim_x/y/z() -> u32` | `%ntid.x/y/z` | `1` |
//! | `gpu_grid_dim_x/y/z() -> u32` | `%nctaid.x/y/z` | `1` |
//! | `gpu_sync_threads()` | `bar.sync 0` | nothing |
//!
//! On a CPU a `@target(gpu)` function is an ordinary function, and these
//! describe the only launch a single call is: one thread of a one-thread
//! grid. That keeps the host build of a package with kernels in it working,
//! and gives a kernel a sequential meaning a test can compare against.

use inkwell::{
    builder::Builder, context::Context, module::Module, targets::TargetData,
    values::BasicMetadataValueEnum,
};

use super::{low_level::Emitted, IntrinsicError};

/// Emits `name` if it is one of the GPU intrinsics.
pub(super) fn gen_gpu_op<'ink>(
    name: &str,
    args: &[BasicMetadataValueEnum<'ink>],
    context: &'ink Context,
    module: &Module<'ink>,
    builder: &Builder<'ink>,
    target_data: &TargetData,
) -> Result<Option<Emitted<'ink>>, IntrinsicError> {
    let Some(rest) = name.strip_prefix("gpu_") else {
        return Ok(None);
    };
    // The function's module carries no triple while its body is generated
    // (it gets one when linked into the assembly), but the code generator's
    // data layout is the target's, and NVPTX's is its own.
    let on_nvptx = target_data.get_data_layout().as_str().to_bytes()
        == codira_target::spec::Target::gpu_target()
            .data_layout
            .as_bytes();

    if rest == "sync_threads" {
        if !args.is_empty() {
            return Err(IntrinsicError::Arity {
                expected: 0,
                found: args.len(),
            });
        }
        if on_nvptx {
            // `bar.sync 0`. LLVM renamed the intrinsic over time; use the
            // spelling this LLVM has.
            if let Ok(barrier) = declare(context, module, "llvm.nvvm.barrier0", &[]) {
                builder
                    .build_call(barrier, &[], "")
                    .map_err(|_error| IntrinsicError::Unknown)?;
            } else {
                let barrier = declare(
                    context,
                    module,
                    "llvm.nvvm.barrier.cta.sync.aligned.all",
                    &[],
                )?;
                builder
                    .build_call(barrier, &[context.i32_type().const_zero().into()], "")
                    .map_err(|_error| IntrinsicError::Unknown)?;
            }
        }
        return Ok(Some(Emitted::Unit));
    }

    let (register, host_value) = match rest.rsplit_once('_') {
        Some(("thread_id", axis @ ("x" | "y" | "z"))) => (format!("tid.{axis}"), 0),
        Some(("block_id", axis @ ("x" | "y" | "z"))) => (format!("ctaid.{axis}"), 0),
        Some(("block_dim", axis @ ("x" | "y" | "z"))) => (format!("ntid.{axis}"), 1),
        Some(("grid_dim", axis @ ("x" | "y" | "z"))) => (format!("nctaid.{axis}"), 1),
        _ => return Err(IntrinsicError::Unknown),
    };
    if !args.is_empty() {
        return Err(IntrinsicError::Arity {
            expected: 0,
            found: args.len(),
        });
    }
    let u32_type = context.i32_type();
    if !on_nvptx {
        return Ok(Some(Emitted::Value(
            u32_type.const_int(host_value, false).into(),
        )));
    }
    let read = declare(
        context,
        module,
        &format!("llvm.nvvm.read.ptx.sreg.{register}"),
        &[],
    )?;
    let value = builder
        .build_call(read, &[], "gpu_sreg")
        .map_err(|_error| IntrinsicError::Unknown)?
        .try_as_basic_value()
        .basic()
        .ok_or(IntrinsicError::Unknown)?;
    Ok(Some(Emitted::Value(value)))
}

/// Declares an LLVM intrinsic in `module`.
fn declare<'ink>(
    _context: &'ink Context,
    module: &Module<'ink>,
    name: &str,
    overloads: &[inkwell::types::BasicTypeEnum<'ink>],
) -> Result<inkwell::values::FunctionValue<'ink>, IntrinsicError> {
    inkwell::intrinsics::Intrinsic::find(name)
        .and_then(|intrinsic| intrinsic.get_declaration(module, overloads))
        .ok_or(IntrinsicError::Unknown)
}

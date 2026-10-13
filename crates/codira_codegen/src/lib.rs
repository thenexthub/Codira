//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
pub use inkwell::{builder::Builder, context::Context, module::Module, OptimizationLevel};

pub use crate::{
    assembly::{AssemblyIr, TargetAssembly},
    code_gen::AssemblyBuilder,
    db::{CodeGenDatabase, CodeGenDatabaseStorage},
    eidos_options::{BuildMode, EidosOptions},
    linker::{in_process_lld_available, link_mode, LinkMode, LINK_MODE_ENV},
    module_group::ModuleGroup,
    module_partition::{ModuleGroupId, ModulePartition},
    native::NativeInputs,
};

/// This library generates machine code from HIR using inkwell which is a safe
/// wrapper around LLVM.
mod code_gen;
mod db;
mod eidos_fold;
mod eidos_options;
#[macro_use]
mod ir;
mod assembly;

#[cfg(test)]
mod mock;
#[cfg(test)]
mod test;
#[cfg(test)]
mod test_targets;

pub mod value;

mod apple;
pub(crate) mod intrinsics;
mod linker;
pub mod mir_codegen;
mod module_group;
mod module_partition;
mod native;
pub(crate) mod type_info;

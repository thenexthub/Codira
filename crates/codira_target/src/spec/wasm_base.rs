//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Shared options for the 32-bit WebAssembly targets.
//!
//! A WebAssembly module is not a `.codiralib`: there is no Codira runtime on
//! the other side to patch a dispatch table, provide a GC heap or load type
//! information, so these targets only build modules of self-contained
//! `@export("C")` functions (see `codira_codegen`'s `wasm` module). They link
//! with LLD's WebAssembly driver (`wasm-ld`).
use crate::spec::{LinkerFlavor, TargetOptions};

/// LLVM's data layout for 32-bit WebAssembly (the same for every OS).
pub const DATA_LAYOUT: &str =
    "e-m:e-p:32:32-p10:8:8-p20:8:8-i64:64-i128:128-n32:64-S128-ni:1:10:20";

pub fn opts() -> TargetOptions {
    TargetOptions {
        os: "unknown".into(),
        vendor: "unknown".into(),
        linker_flavor: LinkerFlavor::WasmLld,
        dll_prefix: "".into(),
        cpu: "generic".into(),
        ..Default::default()
    }
}

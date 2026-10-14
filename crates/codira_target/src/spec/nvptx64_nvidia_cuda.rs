//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The device side of `@target(gpu)`: 64-bit NVIDIA PTX.
//!
//! Not a `--target`: nothing links for it and nothing runs on it alone. The
//! `@target(gpu)` functions of a host build are compiled for it, to PTX text
//! the CUDA driver JIT-compiles for the GPU it finds (`codira_codegen`'s
//! `gpu` module, `codira_gpu` on the host side).
use crate::spec::{Target, TargetOptions};

pub fn target() -> Target {
    Target {
        llvm_target: "nvptx64-nvidia-cuda".into(),
        pointer_width: 64,
        arch: "nvptx64".into(),
        data_layout: "e-p6:32:32-i64:64-i128:128-i256:256-v16:16-v32:32-n16:32:64".into(),
        options: TargetOptions {
            os: "cuda".into(),
            vendor: "nvidia".into(),
            dll_prefix: "".into(),
            // Turing: the oldest architecture current CUDA drivers still JIT
            // for without deprecation warnings. PTX for it runs on every
            // newer GPU; a build may ask for a newer one with `--target-cpu`.
            cpu: "sm_75".into(),
            ..Default::default()
        },
    }
}

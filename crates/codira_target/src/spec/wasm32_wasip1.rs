//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Target specification for `wasm32-wasip1`: the same freestanding module as
//!   `wasm32-unknown-unknown`, tagged for a WASI preview 1 host. Codira links
//!   no WASI libc, so the module imports nothing from the host either.
use crate::spec::{Target, TargetOptions};

pub fn target() -> Target {
    Target {
        llvm_target: "wasm32-wasip1".into(),
        pointer_width: 32,
        arch: "wasm32".into(),
        data_layout: super::wasm_base::DATA_LAYOUT.into(),
        options: TargetOptions {
            os: "wasi".into(),
            env: "p1".into(),
            ..super::wasm_base::opts()
        },
    }
}
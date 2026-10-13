//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Target specification for `wasm32-unknown-unknown`: a freestanding
//!   WebAssembly module that imports nothing.
use crate::spec::Target;

pub fn target() -> Target {
    Target {
        llvm_target: "wasm32-unknown-unknown".into(),
        pointer_width: 32,
        arch: "wasm32".into(),
        data_layout: super::wasm_base::DATA_LAYOUT.into(),
        options: super::wasm_base::opts(),
    }
}
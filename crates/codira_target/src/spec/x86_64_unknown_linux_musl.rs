//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Target specification for `x86_64-unknown-linux-musl`.
use crate::spec::{Target, TargetOptions};

pub fn target() -> Target {
    Target {
        llvm_target: "x86_64-unknown-linux-musl".into(),
        pointer_width: 64,
        arch: "x86_64".into(),
        data_layout: "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128".into(),
        options: TargetOptions {
            cpu: "x86-64".into(),
            features: "".into(),
            ..super::linux_musl_base::opts()
        },
    }
}

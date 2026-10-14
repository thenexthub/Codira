//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Target specification for `aarch64-unknown-freebsd`.
use crate::spec::{Target, TargetOptions};

pub fn target() -> Target {
    Target {
        llvm_target: "aarch64-unknown-freebsd".into(),
        pointer_width: 64,
        arch: "aarch64".into(),
        data_layout: "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32".into(),
        options: TargetOptions {
            cpu: "generic".into(),
            features: "+v8a".into(),
            ..super::freebsd_base::opts()
        },
    }
}

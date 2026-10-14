//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Target specification for `aarch64-pc-windows-msvc`.
use crate::spec::{Target, TargetOptions};

pub fn target() -> Target {
    Target {
        llvm_target: "aarch64-pc-windows-msvc".into(),
        pointer_width: 64,
        arch: "aarch64".into(),
        data_layout: "e-m:w-p270:32:32-p271:32:32-p272:64:64-p:64:64-i32:32-i64:64-i128:128-n32:64-S128-Fn32".into(),
        options: TargetOptions {
            cpu: "generic".into(),
            features: "+v8a".into(),
            // lld-link infers the machine from the first object, but an
            // ARM64 DLL linked from an x64 host should not depend on that.
            pre_link_args: vec!["/MACHINE:ARM64".into()],
            ..super::windows_msvc_base::opts()
        },
    }
}

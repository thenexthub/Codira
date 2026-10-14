//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Target specification for `riscv64-unknown-linux-gnu`.
use crate::spec::{Target, TargetOptions};

pub fn target() -> Target {
    Target {
        llvm_target: "riscv64-unknown-linux-gnu".into(),
        pointer_width: 64,
        arch: "riscv64".into(),
        data_layout: "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128".into(),
        options: TargetOptions {
            cpu: "generic-rv64".into(),
            features: "+m,+a,+f,+d,+c".into(),
            // LLVM picks the lp64d ABI by itself once `+d` is on, which is the
            // one Linux distributions build everything else with.
            abi: "lp64d".into(),
            ..super::linux_base::opts()
        },
    }
}

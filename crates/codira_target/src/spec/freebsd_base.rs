//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Options shared by every FreeBSD target.
use crate::spec::{LinkerFlavor, TargetOptions};

pub fn opts() -> TargetOptions {
    TargetOptions {
        os: "freebsd".to_string(),
        env: String::new(),
        vendor: "unknown".to_string(),
        linker_flavor: LinkerFlavor::Ld,
        ..Default::default()
    }
}

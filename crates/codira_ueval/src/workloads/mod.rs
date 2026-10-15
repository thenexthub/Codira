//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The host-side drivers of the evaluation workloads, by name.

use crate::workload::Workload;

pub mod kv;

/// The names of all workloads, in the order they are reported.
pub const NAMES: [&str; 1] = ["kv"];

/// The driver of the workload called `name`.
pub fn by_name(name: &str) -> Option<Box<dyn Workload>> {
    match name {
        "kv" => Some(Box::new(kv::Kv)),
        _ => None,
    }
}

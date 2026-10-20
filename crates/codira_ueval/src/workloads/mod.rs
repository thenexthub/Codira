//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The host-side drivers of the evaluation workloads, by name.

use crate::workload::Workload;

pub mod kv;
pub mod ledger;
pub mod stream;

/// The names of all workloads, in the order they are reported.
pub const NAMES: [&str; 3] = ["kv", "ledger", "stream"];

/// The driver of the workload called `name`.
pub fn by_name(name: &str) -> Option<Box<dyn Workload>> {
    match name {
        "kv" => Some(Box::new(kv::Kv)),
        "ledger" => Some(Box::new(ledger::Ledger)),
        "stream" => Some(Box::new(stream::Stream)),
        _ => None,
    }
}

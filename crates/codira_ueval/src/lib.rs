//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Evaluation harness for proof-gated live updates: workload drivers, the
//!   replay harness, the restart baselines, the fault-injection mutator and the
//!   statistics used to report measurements. The workloads themselves are
//!   Codira packages under `eval/workloads`.
#![warn(missing_docs)]

pub mod manifest;
pub mod replay;
pub mod rng;
pub mod session;
pub mod stats;
pub mod workload;
pub mod workloads;

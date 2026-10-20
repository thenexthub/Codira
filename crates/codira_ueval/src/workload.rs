//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The host side of a workload: how a request stream is generated, how a
//!   request is executed through `Runtime::invoke`, and how the logical state
//!   of the service is read back (so that "the update did not corrupt state"
//!   can be tested without trusting the code that was updated).

use codira_runtime::{RootedStruct, Runtime, StructRef};

/// One request of a stream. The meaning of the fields belongs to the
/// workload.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Req {
    /// Operation code.
    pub op: i64,
    /// First argument.
    pub a: i64,
    /// Second argument.
    pub b: i64,
    /// Third argument.
    pub c: i64,
    /// Fourth argument (a logical clock for workloads that need one).
    pub d: i64,
}

/// A service driven through `Runtime::invoke`.
pub trait Workload: Sync {
    /// Short name (`kv`, `ledger`, `stream`).
    fn name(&self) -> &'static str;

    /// `n` requests, a pure function of `seed`.
    fn requests(&self, seed: u64, n: usize) -> Vec<Req>;

    /// Creates the service state inside the runtime and roots it.
    fn init(&self, rt: &Runtime) -> Result<RootedStruct, String>;

    /// Executes one request; the returned integer is the observable output.
    fn exec(&self, rt: &Runtime, state: &RootedStruct, req: &Req) -> Result<i64, String>;

    /// The logical state of the service, in a canonical form that does not
    /// depend on internal layout: equal logical state gives an equal vector.
    /// Read through the inspection module, which no update changes.
    fn dump(&self, rt: &Runtime, state: &RootedStruct) -> Result<Vec<i64>, String>;

    /// Rebuilds a service from a [`Workload::dump`] taken from another
    /// process (the restart baseline). State the dump does not carry starts
    /// fresh.
    fn reingest(&self, rt: &Runtime, state: &RootedStruct, dump: &[i64]) -> Result<(), String>;
}

/// Calls `name` with `args` and reads an `i64` back.
pub fn call_i64<A: codira_runtime::InvokeArgs>(
    rt: &Runtime,
    name: &str,
    args: A,
) -> Result<i64, String> {
    rt.invoke::<i64, _>(name, args).map_err(|e| e.to_string())
}

/// Calls a constructor that returns a class instance and roots it.
pub fn call_new(rt: &Runtime, name: &str) -> Result<RootedStruct, String> {
    let s: StructRef<'_> = rt.invoke(name, ()).map_err(|e| e.to_string())?;
    Ok(s.root())
}

/// Reads a class-typed field of a service object by name.
pub fn field_struct<'r>(obj: &StructRef<'r>, name: &str) -> Result<StructRef<'r>, String> {
    obj.get::<StructRef<'r>>(name)
}

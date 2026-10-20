//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Host side of the key-value store workload (`eval/workloads/kv`): the
//!   request mix, execution through `kv_handle`, and the logical dump.

use codira_runtime::{RootedStruct, Runtime};

use crate::{
    rng::Rng,
    workload::{call_i64, call_new, field_struct, Req, Workload},
};

/// The key-value store workload.
pub struct Kv;

/// Keys are drawn from `0..KEY_SPACE`; the table has 128 slots.
const KEY_SPACE: u64 = 96;

impl Workload for Kv {
    fn name(&self) -> &'static str {
        "kv"
    }

    fn requests(&self, seed: u64, n: usize) -> Vec<Req> {
        let mut rng = Rng::new(seed);
        let mut out = Vec::with_capacity(n);
        for i in 0..n {
            let key = if rng.chance(2) {
                -rng.range(1, 8)
            } else {
                rng.below(KEY_SPACE) as i64
            };
            let val = if rng.chance(3) {
                -rng.range(1, 50)
            } else {
                rng.range(0, 9999)
            };
            let roll = rng.below(100);
            let req = if i < 40 {
                Req {
                    op: 1,
                    a: i as i64,
                    b: val.abs(),
                    c: 0,
                    d: 0,
                }
            } else if roll < 40 {
                Req {
                    op: 0,
                    a: key,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 68 {
                Req {
                    op: 1,
                    a: key,
                    b: val,
                    c: 0,
                    d: 0,
                }
            } else if roll < 80 {
                Req {
                    op: 2,
                    a: key,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 86 {
                let lo = rng.below(KEY_SPACE) as i64;
                let hi = if rng.chance(2) {
                    lo - 1
                } else {
                    lo + rng.below(40) as i64
                };
                Req {
                    op: 3,
                    a: lo,
                    b: hi,
                    c: 0,
                    d: 0,
                }
            } else if roll < 89 {
                Req {
                    op: 4,
                    a: 0,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 94 {
                Req {
                    op: 5,
                    a: key,
                    b: rng.range(-5, 20),
                    c: 0,
                    d: 0,
                }
            } else if roll < 97 {
                Req {
                    op: 6,
                    a: key,
                    b: i as i64 + rng.below(50) as i64,
                    c: 0,
                    d: 0,
                }
            } else if roll < 98 {
                let lo = rng.below(KEY_SPACE) as i64;
                Req {
                    op: 7,
                    a: lo,
                    b: lo + rng.below(6) as i64,
                    c: 0,
                    d: 0,
                }
            } else {
                Req {
                    op: 9,
                    a: key,
                    b: val,
                    c: 0,
                    d: 0,
                }
            };
            out.push(req);
        }
        out
    }

    fn init(&self, rt: &Runtime) -> Result<RootedStruct, String> {
        call_new(rt, "kv_new")
    }

    fn exec(&self, rt: &Runtime, state: &RootedStruct, req: &Req) -> Result<i64, String> {
        call_i64(rt, "kv_handle", (state.as_ref(rt), req.op, req.a, req.b))
    }

    fn dump(&self, rt: &Runtime, state: &RootedStruct) -> Result<Vec<i64>, String> {
        let store = state.as_ref(rt);
        let table = field_struct(&store, "table")?;
        let log = field_struct(&store, "log")?;
        let cap = call_i64(rt, "inspect::table_capacity", (table,))?;
        let table = field_struct(&store, "table")?;
        let mut entries = Vec::new();
        for i in 0..cap {
            let t = field_struct(&store, "table")?;
            if call_i64(rt, "inspect::slot_live", (t, i))? == 1 {
                let k = call_i64(rt, "inspect::slot_key", (field_struct(&store, "table")?, i))?;
                let v = call_i64(rt, "inspect::slot_val", (field_struct(&store, "table")?, i))?;
                entries.push((k, v));
            }
        }
        entries.sort_unstable();
        let mut out = vec![entries.len() as i64];
        for (k, v) in entries {
            out.push(k);
            out.push(v);
        }
        out.push(call_i64(rt, "inspect::table_used", (table,))?);
        out.push(call_i64(
            rt,
            "inspect::log_seq",
            (field_struct(&store, "log")?,),
        )?);
        out.push(call_i64(rt, "inspect::log_sum", (log,))?);
        Ok(out)
    }

    fn reingest(&self, rt: &Runtime, state: &RootedStruct, dump: &[i64]) -> Result<(), String> {
        let n = dump[0] as usize;
        for i in 0..n {
            let (k, v) = (dump[1 + 2 * i], dump[2 + 2 * i]);
            // Through the service's own write path, like a restart that
            // replays its data; versions before the non-negative-key rule
            // accept negative keys, later ones refuse them, which is part of
            // what a restart does to data written under the old rules.
            call_i64(rt, "kv_put", (state.as_ref(rt), k, v))?;
        }
        Ok(())
    }
}

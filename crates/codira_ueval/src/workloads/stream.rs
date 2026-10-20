//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 19, 2026
//!
//! Functionality:
//! - Host side of the stream aggregator workload (`eval/workloads/stream`): the
//!   request mix, execution through `stream_handle`, and the logical dump.

use codira_runtime::{RootedStruct, Runtime};

use crate::{
    rng::Rng,
    workload::{call_i64, call_new, Req, Workload},
};

/// The stream aggregator workload.
pub struct Stream;

impl Workload for Stream {
    fn name(&self) -> &'static str {
        "stream"
    }

    fn requests(&self, seed: u64, n: usize) -> Vec<Req> {
        let mut rng = Rng::new(seed);
        let mut base = 1000i64;
        let mut out = Vec::with_capacity(n);
        for i in 0..n {
            let roll = rng.below(100);
            let req = if i < 3 || roll < 55 {
                let count = rng.range(8, 48);
                let mut ts0 = base;
                if rng.chance(2) {
                    ts0 = -rng.range(1, 5000); // clock behind: negative
                                               // timestamps
                } else if rng.chance(8) {
                    ts0 = base - 300; // late batch
                } else {
                    base += count * 3 + rng.range(0, 40);
                }
                Req {
                    op: 0,
                    a: count,
                    b: rng.range(1, 1 << 20),
                    c: ts0,
                    d: 0,
                }
            } else if roll < 68 {
                let p = [0, 50, 90, 99, 100][rng.below(5) as usize];
                Req {
                    op: 1,
                    a: p,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 80 {
                Req {
                    op: 2,
                    a: base / 64 + rng.range(-3, 1),
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 84 {
                Req {
                    op: 3,
                    a: 0,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 88 {
                Req {
                    op: 4,
                    a: 0,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 92 {
                Req {
                    op: 5,
                    a: 0,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 95 {
                Req {
                    op: 6,
                    a: rng.range(1, 6),
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 97 {
                Req {
                    op: 8,
                    a: rng.below(2) as i64,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else if roll < 98 {
                Req {
                    op: 7,
                    a: 0,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            } else {
                Req {
                    op: 9,
                    a: 0,
                    b: 0,
                    c: 0,
                    d: 0,
                }
            };
            out.push(req);
        }
        out
    }

    fn init(&self, rt: &Runtime) -> Result<RootedStruct, String> {
        call_new(rt, "stream_new")
    }

    fn exec(&self, rt: &Runtime, state: &RootedStruct, r: &Req) -> Result<i64, String> {
        call_i64(rt, "stream_handle", (state.as_ref(rt), r.op, r.a, r.b, r.c))
    }

    fn dump(&self, rt: &Runtime, state: &RootedStruct) -> Result<Vec<i64>, String> {
        let s = || state.as_ref(rt);
        let mut out = Vec::new();
        for i in 0..15i64 {
            out.push(call_i64(rt, "inspect::hist_at", (s(), i))?);
        }
        for i in 0..8i64 {
            for f in [
                "inspect::win_id_at",
                "inspect::win_sum_at",
                "inspect::win_cnt_at",
            ] {
                out.push(call_i64(rt, f, (s(), i))?);
            }
        }
        for f in [
            "inspect::agg_total",
            "inspect::agg_late",
            "inspect::agg_newest",
        ] {
            out.push(call_i64(rt, f, (s(),))?);
        }
        Ok(out)
    }

    fn reingest(&self, _rt: &Runtime, _state: &RootedStruct, _dump: &[i64]) -> Result<(), String> {
        // A histogram and a window ring cannot be rebuilt through the
        // service's API (events are not stored); the restart baseline for this
        // workload starts empty, which is what restarting an aggregator
        // without a snapshot does.
        Ok(())
    }
}

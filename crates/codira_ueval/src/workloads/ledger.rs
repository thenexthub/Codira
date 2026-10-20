//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 19, 2026
//!
//! Functionality:
//! - Host side of the ledger / rate-limiter workload (`eval/workloads/ledger`):
//!   the request mix, execution through `ledger_handle`, and the logical dump.

use codira_runtime::{RootedStruct, Runtime};

use crate::{
    rng::Rng,
    workload::{call_i64, call_new, Req, Workload},
};

/// The ledger workload.
pub struct Ledger;

const ACCOUNTS: u64 = 32;

impl Workload for Ledger {
    fn name(&self) -> &'static str {
        "ledger"
    }

    fn requests(&self, seed: u64, n: usize) -> Vec<Req> {
        let mut rng = Rng::new(seed);
        let mut now = 0i64;
        let mut out = Vec::with_capacity(n);
        for _ in 0..n {
            now += rng.below(3) as i64;
            if rng.chance(2) {
                now -= 5; // a clock that steps backwards
            }
            let a = rng.below(ACCOUNTS) as i64;
            let b = rng.below(ACCOUNTS) as i64;
            let roll = rng.below(100);
            let amt = if rng.chance(3) {
                -rng.range(0, 20)
            } else {
                rng.range(1, 400)
            };
            let (op, x, y, z) = if roll < 50 {
                (0, a, if rng.chance(4) { a } else { b }, amt)
            } else if roll < 62 {
                (1, a, amt, 0)
            } else if roll < 72 {
                (2, a, amt, 0)
            } else if roll < 80 {
                (3, a, 0, 0)
            } else if roll < 84 {
                (4, a, rng.below(2) as i64, 0)
            } else if roll < 88 {
                (5, 0, 0, 0)
            } else if roll < 91 {
                (6, rng.below(2) as i64, 0, 0)
            } else if roll < 93 {
                (7, a, 0, 0)
            } else if roll < 94 {
                (8, a, rng.range(0, 12), 0)
            } else if roll < 97 {
                (9, a, b, amt)
            } else if roll < 98 {
                (10, rng.below(6) as i64, 0, 0)
            } else {
                (12, a, b, amt)
            };
            out.push(Req {
                op,
                a: x,
                b: y,
                c: z,
                d: now,
            });
        }
        out
    }

    fn init(&self, rt: &Runtime) -> Result<RootedStruct, String> {
        call_new(rt, "accounts::ledger_new")
    }

    fn exec(&self, rt: &Runtime, state: &RootedStruct, r: &Req) -> Result<i64, String> {
        call_i64(
            rt,
            "ledger_handle",
            (state.as_ref(rt), r.op, r.a, r.b, r.c, r.d),
        )
    }

    fn dump(&self, rt: &Runtime, state: &RootedStruct) -> Result<Vec<i64>, String> {
        let s = || state.as_ref(rt);
        let n = call_i64(rt, "inspect::ledger_n", (s(),))?;
        let mut out = vec![n];
        for i in 0..n {
            for f in [
                "inspect::acct_bal",
                "inspect::acct_frozen",
                "inspect::acct_tokens",
                "inspect::acct_last",
            ] {
                out.push(call_i64(rt, f, (s(), i))?);
            }
        }
        for f in [
            "inspect::ledger_total",
            "inspect::ledger_ok",
            "inspect::ledger_denied",
        ] {
            out.push(call_i64(rt, f, (s(),))?);
        }
        Ok(out)
    }

    fn reingest(&self, rt: &Runtime, state: &RootedStruct, dump: &[i64]) -> Result<(), String> {
        let n = dump[0];
        for i in 0..n {
            let base = (1 + 4 * i) as usize;
            let (bal, frozen) = (dump[base], dump[base + 1]);
            // Through the service's own API: deposit the difference from the
            // fresh balance, then restore the frozen flag.
            let fresh = 1000;
            let d = bal - fresh;
            if d > 0 {
                call_i64(rt, "accounts::deposit", (state.as_ref(rt), i, d))?;
            } else if d < 0 {
                call_i64(rt, "accounts::withdraw", (state.as_ref(rt), i, -d))?;
            }
            if frozen != 0 {
                call_i64(rt, "accounts::set_frozen", (state.as_ref(rt), i, 1i64))?;
            }
        }
        Ok(())
    }
}

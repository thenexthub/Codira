//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The replay harness: start a workload at one version, serve a request
//!   stream, apply the next version while the stream is running, and record
//!   what happened (pause, outputs, logical state before and after).
//! - A fresh start of the new version on the same stream, which is the ground
//!   truth the updated process is compared with.
//! - Two restart baselines (snapshot restore and re-ingest of the logical
//!   state).
//!
//! The stream is open loop: request `i` arrives at `start + i * interval`
//! whether or not the server is ready, and its latency is measured from its
//! arrival, so a pause shows up as the latency of the requests that arrived
//! during it (no coordinated omission). The server is one thread that polls
//! `Runtime::update` between requests, which is how `Runtime::update` is
//! meant to be called today. Until the runtime has safe points, an update
//! can only be applied at a request boundary of that one thread.

use std::time::{Duration, Instant};

use codira_runtime::Runtime;

use crate::{
    session::{ApplyError, ApplyTiming, Deployment, LibSet},
    workload::{Req, Workload},
};

/// Parameters of one replay.
#[derive(Clone, Copy, Debug)]
pub struct Params {
    /// Seed of the request stream.
    pub seed: u64,
    /// Requests served before the update.
    pub prefix: usize,
    /// Requests served after the update.
    pub suffix: usize,
    /// Spacing of arrivals in nanoseconds; 0 means closed loop.
    pub interval_ns: u64,
}

/// How the new version reaches the running service.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Mode {
    /// Today's hot reload: `Runtime::update` relinks, the heap migrates.
    Hot,
    /// Stop, save the heap, start a new process image on the new libraries,
    /// restore the heap. Refused when a layout differs.
    RestartSnapshot,
    /// Stop, read the logical state out through the inspection API, start on
    /// the new libraries, write it back through the service's own API.
    RestartReingest,
}

impl Mode {
    /// Name used in files and tables.
    pub fn name(self) -> &'static str {
        match self {
            Mode::Hot => "hot",
            Mode::RestartSnapshot => "restart-snapshot",
            Mode::RestartReingest => "restart-reingest",
        }
    }

    /// Parses [`Mode::name`].
    pub fn parse(s: &str) -> Option<Mode> {
        match s {
            "hot" => Some(Mode::Hot),
            "restart-snapshot" => Some(Mode::RestartSnapshot),
            "restart-reingest" => Some(Mode::RestartReingest),
            _ => None,
        }
    }
}

/// Everything one replay observed.
#[derive(Clone, Debug, Default)]
pub struct RunOutput {
    /// Output of every request, prefix then suffix (shorter if the run
    /// stopped early).
    pub outputs: Vec<i64>,
    /// Latency of every request from its arrival, in nanoseconds.
    pub latencies_ns: Vec<u64>,
    /// Pure service time of every request, in nanoseconds.
    pub service_ns: Vec<u64>,
    /// Logical state just before the update.
    pub dump_before: Vec<i64>,
    /// Logical state just after the update.
    pub dump_after: Vec<i64>,
    /// Logical state at the end.
    pub dump_final: Vec<i64>,
    /// Timing of the update (hot mode).
    pub timing: Option<ApplyTiming>,
    /// Downtime of a restart baseline: no request is served from the moment
    /// the old service stops until the new one is ready.
    pub downtime: Option<Duration>,
    /// Why the update did not take effect, if it did not.
    pub apply_error: Option<String>,
    /// The first request that failed to execute: index and message.
    pub exec_error: Option<(usize, String)>,
}

struct Clock {
    start: Instant,
    interval: Duration,
}

impl Clock {
    fn arrival(&self, i: usize) -> Instant {
        self.start + self.interval * i as u32
    }
}

fn serve(
    w: &dyn Workload,
    rt: &Runtime,
    state: &codira_runtime::RootedStruct,
    reqs: &[Req],
    range: std::ops::Range<usize>,
    clock: &Clock,
    out: &mut RunOutput,
) -> bool {
    for i in range {
        let open_loop = !clock.interval.is_zero();
        let arrival = if open_loop {
            let a = clock.arrival(i);
            while Instant::now() < a {
                std::hint::spin_loop();
            }
            a
        } else {
            Instant::now()
        };
        let t0 = Instant::now();
        let r = w.exec(rt, state, &reqs[i]);
        let done = Instant::now();
        match r {
            Ok(v) => {
                out.outputs.push(v);
                out.service_ns.push((done - t0).as_nanos() as u64);
                out.latencies_ns.push((done - arrival).as_nanos() as u64);
            }
            Err(e) => {
                out.exec_error = Some((i, e));
                return false;
            }
        }
    }
    true
}

fn start_service(
    w: &dyn Workload,
    libs: &LibSet,
) -> Result<(Deployment, Runtime, codira_runtime::RootedStruct), String> {
    let dep = Deployment::new(libs).map_err(|e| e.to_string())?;
    let rt = dep.start()?;
    let state = w.init(&rt)?;
    Ok((dep, rt, state))
}

/// Serves the whole stream on `libs` without any update.
pub fn run_fresh(w: &dyn Workload, libs: &LibSet, p: &Params) -> RunOutput {
    let mut out = RunOutput::default();
    let reqs = w.requests(p.seed, p.prefix + p.suffix);
    let (_dep, rt, state) = match start_service(w, libs) {
        Ok(x) => x,
        Err(e) => {
            out.exec_error = Some((0, e));
            return out;
        }
    };
    let clock = Clock {
        start: Instant::now(),
        interval: Duration::from_nanos(p.interval_ns),
    };
    if serve(w, &rt, &state, &reqs, 0..p.prefix, &clock, &mut out) {
        out.dump_before = w.dump(&rt, &state).unwrap_or_default();
        serve(
            w,
            &rt,
            &state,
            &reqs,
            p.prefix..p.prefix + p.suffix,
            &clock,
            &mut out,
        );
    }
    out.dump_final = w.dump(&rt, &state).unwrap_or_default();
    out
}

/// Serves `prefix` requests on `base`, delivers `cand` the way `mode`
/// says, and serves `suffix` more.
pub fn run_update(
    w: &dyn Workload,
    base: &LibSet,
    cand: &LibSet,
    mode: Mode,
    p: &Params,
) -> RunOutput {
    let mut out = RunOutput::default();
    let reqs = w.requests(p.seed, p.prefix + p.suffix);
    let (mut dep, mut rt, mut state) = match start_service(w, base) {
        Ok(x) => x,
        Err(e) => {
            out.exec_error = Some((0, e));
            return out;
        }
    };
    let mut clock = Clock {
        start: Instant::now(),
        interval: Duration::from_nanos(p.interval_ns),
    };
    if !serve(w, &rt, &state, &reqs, 0..p.prefix, &clock, &mut out) {
        return out;
    }
    match w.dump(&rt, &state) {
        Ok(d) => out.dump_before = d,
        Err(e) => {
            out.exec_error = Some((p.prefix, format!("dump before update: {e}")));
            return out;
        }
    }

    match mode {
        Mode::Hot => match dep.apply(&mut rt, cand, Duration::from_secs(20)) {
            Ok(t) => {
                // Arrivals that fell into the install and the wait for the
                // file watcher are modelled as served by the old code on
                // time; only the relink call itself stalls the stream.
                let shift = t.install + t.detect.saturating_sub(t.relink_call);
                clock.start += shift;
                out.timing = Some(t);
            }
            Err(ApplyError::NothingToApply) => {
                out.apply_error = Some("nothing-to-apply".into());
            }
            Err(e) => {
                out.apply_error = Some(format!("{e:?}"));
                return out;
            }
        },
        Mode::RestartSnapshot | Mode::RestartReingest => {
            let t0 = Instant::now();
            let snapshot_path = dep_snapshot_path(&dep);
            let dump = if mode == Mode::RestartReingest {
                Some(out.dump_before.clone())
            } else {
                match rt.save_heap(&snapshot_path) {
                    Ok(_) => None,
                    Err(e) => {
                        out.apply_error = Some(format!("snapshot save: {e}"));
                        return out;
                    }
                }
            };
            drop(state);
            drop(rt);
            let new_dep = match Deployment::new(cand) {
                Ok(d) => d,
                Err(e) => {
                    out.apply_error = Some(e.to_string());
                    return out;
                }
            };
            let new_rt = match new_dep.start() {
                Ok(r) => r,
                Err(e) => {
                    out.apply_error = Some(e);
                    return out;
                }
            };
            let new_state = match &dump {
                Some(d) => w
                    .init(&new_rt)
                    .and_then(|s| w.reingest(&new_rt, &s, d).map(|()| s)),
                None => new_rt
                    .restore_heap(&snapshot_path)
                    .map_err(|e| format!("snapshot restore: {e}"))
                    .and_then(|mut roots| {
                        // The service object is the first rooted struct.
                        if roots.is_empty() {
                            Err("snapshot restore: no rooted struct".to_string())
                        } else {
                            Ok(roots.swap_remove(0))
                        }
                    }),
            };
            match new_state {
                Ok(s) => {
                    state = s;
                    rt = new_rt;
                    dep = new_dep;
                    out.downtime = Some(t0.elapsed());
                    clock.start += t0.elapsed();
                }
                Err(e) => {
                    out.apply_error = Some(e);
                    return out;
                }
            }
        }
    }

    match w.dump(&rt, &state) {
        Ok(d) => out.dump_after = d,
        Err(e) => {
            out.exec_error = Some((p.prefix, format!("dump after update: {e}")));
            return out;
        }
    }
    serve(
        w,
        &rt,
        &state,
        &reqs,
        p.prefix..p.prefix + p.suffix,
        &clock,
        &mut out,
    );
    out.dump_final = w.dump(&rt, &state).unwrap_or_default();
    drop(dep);
    out
}

fn dep_snapshot_path(dep: &Deployment) -> std::path::PathBuf {
    dep.entry().with_file_name("heap.snapshot")
}

/// How an update compares with a fresh start of the new version.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Comparison {
    /// The logical state did not change across the update itself (no code
    /// runs during an update, so any difference is the migration's).
    pub state_preserved: bool,
    /// Every suffix output equals the fresh start's.
    pub suffix_equal_fresh: bool,
    /// Index of the first request whose output differs from the fresh start.
    pub first_divergence: Option<usize>,
    /// The logical state at the end equals the fresh start's.
    pub final_state_equal_fresh: bool,
}

/// Compares an updated run with the fresh run of the new version.
pub fn compare_with_fresh(updated: &RunOutput, fresh: &RunOutput, prefix: usize) -> Comparison {
    let n = updated.outputs.len().min(fresh.outputs.len());
    let first = (prefix..n).find(|&i| updated.outputs[i] != fresh.outputs[i]);
    Comparison {
        state_preserved: updated.dump_before == updated.dump_after
            && !updated.dump_after.is_empty(),
        suffix_equal_fresh: first.is_none() && updated.outputs.len() == fresh.outputs.len(),
        first_divergence: first,
        final_state_equal_fresh: updated.dump_final == fresh.dump_final
            && !updated.dump_final.is_empty(),
    }
}

fn join(v: &[impl ToString]) -> String {
    v.iter()
        .map(ToString::to_string)
        .collect::<Vec<_>>()
        .join(",")
}

fn split(s: &str) -> Vec<i64> {
    s.split(',')
        .filter(|x| !x.is_empty())
        .map(|x| x.parse().unwrap_or(0))
        .collect()
}

impl RunOutput {
    /// A line-oriented text form, so a worker process can hand its result to
    /// the orchestrator.
    pub fn to_text(&self) -> String {
        let mut s = String::new();
        let mut line = |k: &str, v: String| {
            s.push_str(k);
            s.push('=');
            s.push_str(&v);
            s.push('\n');
        };
        line("outputs", join(&self.outputs));
        line("latencies_ns", join(&self.latencies_ns));
        line("service_ns", join(&self.service_ns));
        line("dump_before", join(&self.dump_before));
        line("dump_after", join(&self.dump_after));
        line("dump_final", join(&self.dump_final));
        if let Some(t) = &self.timing {
            line(
                "timing_ns",
                format!(
                    "{},{},{}",
                    t.install.as_nanos(),
                    t.detect.as_nanos(),
                    t.relink_call.as_nanos()
                ),
            );
        }
        if let Some(d) = self.downtime {
            line("downtime_ns", d.as_nanos().to_string());
        }
        if let Some(e) = &self.apply_error {
            line("apply_error", e.replace('\n', " "));
        }
        if let Some((i, e)) = &self.exec_error {
            line("exec_error", format!("{i}:{}", e.replace('\n', " ")));
        }
        s
    }

    /// Parses [`RunOutput::to_text`].
    pub fn from_text(text: &str) -> RunOutput {
        let mut o = RunOutput::default();
        for l in text.lines() {
            let Some((k, v)) = l.split_once('=') else {
                continue;
            };
            match k {
                "outputs" => o.outputs = split(v),
                "latencies_ns" => o.latencies_ns = split(v).into_iter().map(|x| x as u64).collect(),
                "service_ns" => o.service_ns = split(v).into_iter().map(|x| x as u64).collect(),
                "dump_before" => o.dump_before = split(v),
                "dump_after" => o.dump_after = split(v),
                "dump_final" => o.dump_final = split(v),
                "timing_ns" => {
                    let t = split(v);
                    if t.len() == 3 {
                        o.timing = Some(ApplyTiming {
                            install: Duration::from_nanos(t[0] as u64),
                            detect: Duration::from_nanos(t[1] as u64),
                            relink_call: Duration::from_nanos(t[2] as u64),
                        });
                    }
                }
                "downtime_ns" => o.downtime = v.parse().ok().map(Duration::from_nanos),
                "apply_error" => o.apply_error = Some(v.to_owned()),
                "exec_error" => {
                    let (i, e) = v.split_once(':').unwrap_or(("0", v));
                    o.exec_error = Some((i.parse().unwrap_or(0), e.to_owned()));
                }
                _ => {}
            }
        }
        o
    }
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 19, 2026
//!
//! Functionality:
//! - Command line of the evaluation harness: inspect a history, check that
//!   every version builds, replay every update of a history under the
//!   baselines, and run the fault-injection campaign. Each replay runs in a
//!   child process (`ueval worker`) with a time limit, because a faulty update
//!   can panic (which ends the process) or loop forever.

use std::{
    collections::BTreeMap,
    path::{Path, PathBuf},
    process::{Command, Stdio},
    time::{Duration, Instant},
};

use anyhow::{anyhow, Context, Result};
use codira_ueval::{
    manifest::{category_counts, Manifest},
    mutate,
    replay::{compare_with_fresh, run_fresh, run_update, Mode, Params, RunOutput},
    session::{install_relink_logger, write_libs, BuildOptions, Builder, LibSet},
    stats::Summary,
    workloads,
};

fn eval_root() -> PathBuf {
    if let Ok(p) = std::env::var("UEVAL_ROOT") {
        return PathBuf::from(p);
    }
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("..")
        .join("..")
        .join("eval")
}

fn work_root() -> PathBuf {
    std::env::var("UEVAL_WORK")
        .map_or_else(|_| std::env::temp_dir().join("ueval-work"), PathBuf::from)
}

fn load(name: &str) -> Result<Manifest> {
    Manifest::load(&eval_root().join("workloads").join(name))
}

fn usage() -> ! {
    eprintln!(
        "usage:\n  ueval list <workload>\n  ueval build-all <workload>\n  ueval smoke <workload> <from> <to> [seed]\n  \
         ueval history <workload> <out.csv> [seed] [prefix] [suffix]\n  \
         ueval faults <workload> <out.csv> [per-update] [seed]\n  \
         ueval bench <workload> <version> <reps> <requests> <out.csv>\n  \
         ueval worker <args..> (internal)"
    );
    std::process::exit(2)
}

fn arg(args: &[String], i: usize) -> &str {
    args.get(i).map_or_else(|| usage(), String::as_str)
}

fn read_libs_dir(dir: &Path) -> Result<LibSet> {
    codira_ueval::session::read_libs(dir)
        .with_context(|| format!("reading libraries in {}", dir.display()))
}

#[derive(Debug)]
enum ChildEnd {
    Done(Box<RunOutput>),
    Crashed(Option<i32>),
    Hung,
}

fn spawn_worker(
    workload: &str,
    base: Option<&Path>,
    cand: &Path,
    mode: &str,
    p: &Params,
    out: &Path,
    limit: Duration,
) -> Result<ChildEnd> {
    let _ = std::fs::remove_file(out);
    let mut cmd = Command::new(std::env::current_exe()?);
    cmd.arg("worker")
        .args(["--workload", workload])
        .args(["--cand", &cand.to_string_lossy()])
        .args(["--mode", mode])
        .args(["--seed", &p.seed.to_string()])
        .args(["--prefix", &p.prefix.to_string()])
        .args(["--suffix", &p.suffix.to_string()])
        .args(["--interval-ns", &p.interval_ns.to_string()])
        .args(["--out", &out.to_string_lossy()])
        .stdin(Stdio::null())
        .stdout(Stdio::null())
        .stderr(Stdio::null());
    if let Some(b) = base {
        cmd.args(["--base", &b.to_string_lossy()]);
    }
    let mut child = cmd.spawn()?;
    let start = Instant::now();
    loop {
        if let Some(status) = child.try_wait()? {
            let text = std::fs::read_to_string(out).unwrap_or_default();
            return Ok(if status.success() && !text.is_empty() {
                ChildEnd::Done(Box::new(RunOutput::from_text(&text)))
            } else {
                ChildEnd::Crashed(status.code())
            });
        }
        if start.elapsed() > limit {
            let _ = child.kill();
            let _ = child.wait();
            return Ok(ChildEnd::Hung);
        }
        std::thread::sleep(Duration::from_millis(15));
    }
}

fn worker_main(args: &[String]) -> Result<()> {
    install_relink_logger();
    let get = |k: &str| -> Option<String> {
        args.iter()
            .position(|a| a == k)
            .and_then(|i| args.get(i + 1))
            .cloned()
    };
    let w = workloads::by_name(&get("--workload").ok_or_else(|| anyhow!("--workload"))?)
        .ok_or_else(|| anyhow!("unknown workload"))?;
    let cand = read_libs_dir(Path::new(&get("--cand").ok_or_else(|| anyhow!("--cand"))?))?;
    let num = |k: &str| -> Result<u64> { Ok(get(k).ok_or_else(|| anyhow!("{k}"))?.parse()?) };
    let p = Params {
        seed: num("--seed")?,
        prefix: num("--prefix")? as usize,
        suffix: num("--suffix")? as usize,
        interval_ns: num("--interval-ns")?,
    };
    let mode = get("--mode").ok_or_else(|| anyhow!("--mode"))?;
    let out = if mode == "fresh" {
        run_fresh(w.as_ref(), &cand, &p)
    } else {
        let base = read_libs_dir(Path::new(&get("--base").ok_or_else(|| anyhow!("--base"))?))?;
        run_update(
            w.as_ref(),
            &base,
            &cand,
            Mode::parse(&mode).ok_or_else(|| anyhow!("mode"))?,
            &p,
        )
    };
    std::fs::write(get("--out").ok_or_else(|| anyhow!("--out"))?, out.to_text())?;
    // Skip the destructors of the runtime and its file watcher.
    std::process::exit(0);
}

/// Builds every version of a workload once (cached under the work directory).
fn build_versions(m: &Manifest) -> Result<BTreeMap<String, PathBuf>> {
    let mut dirs = BTreeMap::new();
    for n in 1..=m.versions {
        let v = Manifest::version_name(n);
        let dir = work_root().join(&m.name).join("libs").join(&v);
        if !dir.join("mod.codiralib").exists() {
            let files = m.read_version(&v)?;
            let (_, libs) = Builder::new(&files, BuildOptions::default())
                .map_err(|e| anyhow!("{} {v} does not build:\n{e}", m.name))?;
            write_libs(&dir, &libs)?;
        }
        dirs.insert(v, dir);
    }
    Ok(dirs)
}

fn ms(d: Duration) -> f64 {
    d.as_secs_f64() * 1e3
}

fn p99_ms(v: &[u64]) -> f64 {
    if v.is_empty() {
        return f64::NAN;
    }
    let f: Vec<f64> = v.iter().map(|x| *x as f64 / 1e6).collect();
    Summary::of(&f).p99
}

fn end_label(e: &ChildEnd) -> String {
    match e {
        ChildEnd::Done(o) => {
            if let Some(a) = &o.apply_error {
                format!("not-applied:{a}")
            } else if let Some((i, _)) = &o.exec_error {
                format!("exec-error@{i}")
            } else {
                "completed".into()
            }
        }
        ChildEnd::Crashed(code) => {
            format!(
                "crashed:{}",
                code.map_or("signal".into(), |c| c.to_string())
            )
        }
        ChildEnd::Hung => "hung".into(),
    }
}

fn history(m: &Manifest, csv: &Path, seed: u64, prefix: usize, suffix: usize) -> Result<()> {
    let dirs = build_versions(m)?;
    let p = Params {
        seed,
        prefix,
        suffix,
        interval_ns: 0,
    };
    let tmp = work_root().join(&m.name).join("run");
    std::fs::create_dir_all(&tmp)?;
    let limit = Duration::from_secs(120);
    let mut rows = vec![
        "workload,update,category,provenance,layout_change,migrate,behaviour,hot_outcome,state_preserved,suffix_equal_fresh,first_divergence,final_state_equal_fresh,install_ms,detect_ms,relink_ms,hot_p99_ms,restart_snapshot,restart_snapshot_downtime_ms,restart_reingest,restart_reingest_downtime_ms,restart_reingest_suffix_equal_fresh"
            .to_string(),
    ];
    for u in &m.updates {
        let base = &dirs[&u.from];
        let cand = &dirs[&u.to];
        let fresh = spawn_worker(
            &m.name,
            None,
            cand,
            "fresh",
            &p,
            &tmp.join("fresh.txt"),
            limit,
        )?;
        let hot = spawn_worker(
            &m.name,
            Some(base),
            cand,
            "hot",
            &p,
            &tmp.join("hot.txt"),
            limit,
        )?;
        let snap = spawn_worker(
            &m.name,
            Some(base),
            cand,
            "restart-snapshot",
            &p,
            &tmp.join("snap.txt"),
            limit,
        )?;
        let rein = spawn_worker(
            &m.name,
            Some(base),
            cand,
            "restart-reingest",
            &p,
            &tmp.join("rein.txt"),
            limit,
        )?;
        let ChildEnd::Done(fresh_o) = fresh else {
            return Err(anyhow!("fresh start of {} failed", u.to));
        };
        let (mut sp, mut sf, mut fd, mut fs) =
            (String::new(), String::new(), String::new(), String::new());
        let (mut ins, mut det, mut rel, mut p99) =
            (String::new(), String::new(), String::new(), String::new());
        if let ChildEnd::Done(o) = &hot {
            let c = compare_with_fresh(o, &fresh_o, p.prefix);
            sp = c.state_preserved.to_string();
            sf = c.suffix_equal_fresh.to_string();
            fd = c.first_divergence.map_or(String::new(), |x| x.to_string());
            fs = c.final_state_equal_fresh.to_string();
            if let Some(t) = &o.timing {
                ins = format!("{:.3}", ms(t.install));
                det = format!("{:.3}", ms(t.detect));
                rel = format!("{:.3}", ms(t.relink_call));
            }
            p99 = format!("{:.4}", p99_ms(&o.service_ns));
        }
        let down = |e: &ChildEnd| match e {
            ChildEnd::Done(o) => o
                .downtime
                .map_or(String::new(), |d| format!("{:.3}", ms(d))),
            _ => String::new(),
        };
        let rein_eq = match &rein {
            ChildEnd::Done(o) => compare_with_fresh(o, &fresh_o, p.prefix)
                .suffix_equal_fresh
                .to_string(),
            _ => String::new(),
        };
        let row = format!(
            "{},{},{},{},{},{},{},{},{sp},{sf},{fd},{fs},{ins},{det},{rel},{p99},{},{},{},{},{rein_eq}",
            m.name,
            u.id,
            u.category,
            u.provenance,
            u.layout_change,
            u.migrate.split(':').next().unwrap_or(""),
            u.behaviour,
            end_label(&hot),
            end_label(&snap),
            down(&snap),
            end_label(&rein),
            down(&rein),
        );
        println!("{row}");
        rows.push(row);
    }
    std::fs::write(csv, rows.join("\n") + "\n")?;
    Ok(())
}

fn faults(m: &Manifest, csv: &Path, per_update: usize, seed: u64) -> Result<()> {
    let dirs = build_versions(m)?;
    let p = Params {
        seed: 7,
        prefix: 300,
        suffix: 400,
        interval_ns: 0,
    };
    let tmp = work_root().join(&m.name).join("faults");
    let limit = Duration::from_secs(60);
    let mut rows = vec![
        "workload,update,mutant,class,file,line,function,in_diff,build,run_outcome,matches_intended,state_preserved,baseline_verdict".to_string(),
    ];
    let mut valid_total = 0;
    for (ui, u) in m.updates.iter().enumerate() {
        let base = &dirs[&u.from];
        let files = m.read_version(&u.to)?;
        let mut changed = u.changed_functions.clone();
        changed.extend(u.added_functions.iter().cloned());
        changed.extend(u.changed_types.iter().cloned());
        let all = mutate::enumerate(&files, &changed);
        let pool = mutate::sample(&all, per_update * 4, seed + ui as u64);
        let ref_path = tmp.join(&u.id);
        std::fs::create_dir_all(&ref_path)?;
        let reference = spawn_worker(
            &m.name,
            Some(base),
            &dirs[&u.to],
            "hot",
            &p,
            &ref_path.join("ref.txt"),
            limit,
        )?;
        let ChildEnd::Done(ref_o) = reference else {
            return Err(anyhow!("reference run of {} failed", u.id));
        };
        let mut builder = Builder::new(&files, BuildOptions::default())
            .map_err(|e| anyhow!("{e}"))?
            .0;
        let mut kept = 0usize;
        for (mi, mu) in pool.iter().enumerate() {
            if kept >= per_update {
                break;
            }
            let head = format!(
                "{},{},{mi},{},{},{},{},{}",
                m.name,
                u.id,
                mu.class.name(),
                mu.file,
                mu.line,
                mu.function,
                mu.in_diff
            );
            let mut variant = files.clone();
            variant.insert(mu.file.clone(), mu.text.clone());
            let built = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
                builder.build_variant(&variant)
            }));
            let libs = match built {
                Ok(Ok(l)) => l,
                Ok(Err(_)) => {
                    rows.push(format!("{head},rejected-by-build,,,,"));
                    continue;
                }
                Err(_) => {
                    rows.push(format!("{head},compiler-panic,,,,"));
                    builder = Builder::new(&files, BuildOptions::default())
                        .map_err(|e| anyhow!("{e}"))?
                        .0;
                    continue;
                }
            };
            kept += 1;
            valid_total += 1;
            let mdir = ref_path.join(format!("m{mi}"));
            write_libs(&mdir, &libs)?;
            let end = spawn_worker(
                &m.name,
                Some(base),
                &mdir,
                "hot",
                &p,
                &mdir.join("out.txt"),
                limit,
            )?;
            let label = end_label(&end);
            let (matches, preserved, verdict) = match &end {
                ChildEnd::Done(o) if o.apply_error.is_none() && o.exec_error.is_none() => {
                    let same = o.outputs == ref_o.outputs && o.dump_final == ref_o.dump_final;
                    (
                        same.to_string(),
                        (o.dump_before == o.dump_after).to_string(),
                        if same {
                            "admitted-equivalent-on-stream"
                        } else {
                            "admitted-and-wrong"
                        },
                    )
                }
                ChildEnd::Done(_) => (String::new(), String::new(), "not-applied"),
                _ => (
                    String::new(),
                    String::new(),
                    "admitted-then-crashed-or-hung",
                ),
            };
            let row = format!("{head},built,{label},{matches},{preserved},{verdict}");
            println!("{row}");
            rows.push(row);
            std::fs::remove_dir_all(&mdir).ok();
        }
        eprintln!("{}: {kept} built mutants, {valid_total} so far", u.id);
        std::fs::write(csv, rows.join("\n") + "\n")?;
    }
    Ok(())
}

/// Steady-state overhead: the same version built with and without hot reload,
/// serving the same stream, alternating which goes first in every repetition.
fn bench(m: &Manifest, version: &str, reps: usize, requests: usize, csv: &Path) -> Result<()> {
    use codira_ueval::session::Deployment;
    let w = workloads::by_name(&m.name).ok_or_else(|| anyhow!("no driver"))?;
    let files = m.read_version(version)?;
    let hot = Builder::new(
        &files,
        BuildOptions {
            no_hot_reload: false,
        },
    )
    .map_err(|e| anyhow!("{e}"))?
    .1;
    let direct = Builder::new(
        &files,
        BuildOptions {
            no_hot_reload: true,
        },
    )
    .map_err(|e| anyhow!("{e}"))?
    .1;
    println!(
        "{} {version}: hot-reload build has {} libraries, no-hot-reload build has {}",
        m.name,
        hot.len(),
        direct.len()
    );
    let dep_hot = Deployment::new(&hot)?;
    let dep_direct = Deployment::new(&direct)?;
    let rt_hot = dep_hot.start().map_err(|e| anyhow!("{e}"))?;
    let rt_direct = dep_direct.start().map_err(|e| anyhow!("{e}"))?;
    let reqs = w.requests(99, requests);
    let mut rows =
        vec!["workload,version,rep,variant,requests_per_s,p50_us,p99_us,max_us".to_string()];
    let mut thr = [Vec::new(), Vec::new()];
    let mut p99 = [Vec::new(), Vec::new()];
    let mut reference: Option<Vec<i64>> = None;
    for rep in 0..reps {
        let order = if rep % 2 == 0 { [0usize, 1] } else { [1, 0] };
        for v in order {
            let rt = if v == 0 { &rt_hot } else { &rt_direct };
            let state = w.init(rt).map_err(|e| anyhow!("{e}"))?;
            let mut lat = Vec::with_capacity(reqs.len());
            let mut outs = Vec::with_capacity(reqs.len());
            let t0 = Instant::now();
            for r in &reqs {
                let t = Instant::now();
                let o = w.exec(rt, &state, r).map_err(|e| anyhow!("{e}"))?;
                lat.push(t.elapsed().as_nanos() as f64 / 1e3);
                outs.push(o);
            }
            let total = t0.elapsed().as_secs_f64();
            match &reference {
                None => reference = Some(outs),
                Some(r) => {
                    if *r != outs {
                        return Err(anyhow!("the two builds disagree on the outputs"));
                    }
                }
            }
            let s = Summary::of(&lat);
            let name = if v == 0 {
                "hot-reload"
            } else {
                "no-hot-reload"
            };
            let rate = reqs.len() as f64 / total;
            thr[v].push(rate);
            p99[v].push(s.p99);
            rows.push(format!(
                "{},{version},{rep},{name},{rate:.0},{:.3},{:.3},{:.3}",
                m.name, s.median, s.p99, s.max
            ));
        }
    }
    std::fs::write(
        csv,
        rows.join(
            "
",
        ) + "
",
    )?;
    for (v, name) in [(0, "hot-reload"), (1, "no-hot-reload")] {
        let t = Summary::of(&thr[v]);
        let p = Summary::of(&p99[v]);
        println!(
            "{name:<14} throughput req/s: min {:.0} median {:.0} max {:.0} iqr {:.0} | p99 us: min {:.2} median {:.2} max {:.2} iqr {:.2}  (n={})",
            t.min, t.median, t.max, t.iqr(), p.min, p.median, p.max, p.iqr(), t.n
        );
    }
    Ok(())
}

fn main() -> Result<()> {
    install_relink_logger();
    let args: Vec<String> = std::env::args().skip(1).collect();
    let num = |i: usize, d: u64| args.get(i).map_or(d, |s| s.parse().unwrap_or(d));
    match args.first().map(String::as_str) {
        Some("worker") => worker_main(&args[1..]),
        Some("list") => {
            let m = load(arg(&args, 1))?;
            println!(
                "{}: {} versions, {} updates",
                m.name,
                m.versions,
                m.updates.len()
            );
            for (c, n) in category_counts(&m) {
                println!("  {c:<16} {n}");
            }
            Ok(())
        }
        Some("build-all") => {
            let m = load(arg(&args, 1))?;
            let mut failed = 0;
            for n in 1..=m.versions {
                let v = Manifest::version_name(n);
                let files = m.read_version(&v)?;
                let t = Instant::now();
                match Builder::new(&files, BuildOptions::default()) {
                    Ok((_, libs)) => println!(
                        "{} {v}: ok, {} libraries, {:.2}s",
                        m.name,
                        libs.len(),
                        t.elapsed().as_secs_f64()
                    ),
                    Err(e) => {
                        failed += 1;
                        println!("{} {v}: FAILED\n{e}", m.name);
                    }
                }
            }
            if failed > 0 {
                Err(anyhow!("{failed} versions failed to build"))
            } else {
                Ok(())
            }
        }
        Some("smoke") => {
            if args.len() < 4 {
                usage();
            }
            let m = load(&args[1])?;
            let w =
                workloads::by_name(&m.name).ok_or_else(|| anyhow!("no driver for {}", m.name))?;
            let base = Builder::new(&m.read_version(&args[2])?, BuildOptions::default())
                .map_err(|e| anyhow!("{e}"))?
                .1;
            let cand = Builder::new(&m.read_version(&args[3])?, BuildOptions::default())
                .map_err(|e| anyhow!("{e}"))?
                .1;
            let p = Params {
                seed: num(4, 1),
                prefix: 400,
                suffix: 400,
                interval_ns: 0,
            };
            let hot = run_update(w.as_ref(), &base, &cand, Mode::Hot, &p);
            let fresh = run_fresh(w.as_ref(), &cand, &p);
            println!(
                "exec_error: {:?}  apply_error: {:?}",
                hot.exec_error, hot.apply_error
            );
            println!("timing: {:?}", hot.timing);
            println!("{:?}", compare_with_fresh(&hot, &fresh, p.prefix));
            println!(
                "first outputs: {:?}",
                &hot.outputs[..hot.outputs.len().min(12)]
            );
            Ok(())
        }
        Some("history") => {
            let m = load(arg(&args, 1))?;
            history(
                &m,
                Path::new(arg(&args, 2)),
                num(3, 1),
                num(4, 400) as usize,
                num(5, 400) as usize,
            )
        }
        Some("bench") => {
            let m = load(arg(&args, 1))?;
            bench(
                &m,
                arg(&args, 2),
                num(3, 25) as usize,
                num(4, 20000) as usize,
                Path::new(arg(&args, 5)),
            )
        }
        Some("faults") => {
            let m = load(arg(&args, 1))?;
            faults(&m, Path::new(arg(&args, 2)), num(3, 11) as usize, num(4, 1))
        }
        _ => usage(),
    }
}

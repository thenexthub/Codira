//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Command line of the evaluation harness: inspect a history, check that
//!   every version builds, and replay one update.

use std::{path::PathBuf, time::Instant};

use anyhow::{anyhow, Result};
use codira_ueval::{
    manifest::{category_counts, Manifest},
    replay::{compare_with_fresh, run_fresh, run_update, Mode, Params},
    session::{install_relink_logger, BuildOptions, Builder},
    workloads,
};

fn eval_root() -> PathBuf {
    if let Ok(p) = std::env::var("UEVAL_ROOT") {
        return PathBuf::from(p);
    }
    // crates/codira_ueval -> repository root -> eval
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("..")
        .join("..")
        .join("eval")
}

fn load(name: &str) -> Result<Manifest> {
    Manifest::load(&eval_root().join("workloads").join(name))
}

fn usage() -> ! {
    eprintln!(
        "usage:\n  ueval list <workload>\n  ueval build-all <workload>\n  ueval smoke <workload> <from> <to> [seed]"
    );
    std::process::exit(2)
}

fn main() -> Result<()> {
    install_relink_logger();
    let args: Vec<String> = std::env::args().skip(1).collect();
    match args.first().map(String::as_str) {
        Some("list") => {
            let m = load(args.get(1).map_or_else(|| usage(), String::as_str))?;
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
            let m = load(args.get(1).map_or_else(|| usage(), String::as_str))?;
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
            let seed = args.get(4).map_or(Ok(1), |s| s.parse())?;
            let base = Builder::new(&m.read_version(&args[2])?, BuildOptions::default())
                .map_err(|e| anyhow!("{} does not build:\n{e}", args[2]))?
                .1;
            let cand = Builder::new(&m.read_version(&args[3])?, BuildOptions::default())
                .map_err(|e| anyhow!("{} does not build:\n{e}", args[3]))?
                .1;
            let p = Params {
                seed,
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
        _ => usage(),
    }
}

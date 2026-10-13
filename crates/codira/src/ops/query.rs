//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - `codira query "<predicate>"`: semantic search over the package's functions
//!   by what they can return (see `codira_hir::verify::query`).

use std::path::PathBuf;

use codira_compiler::{Config, Driver, Target};

use crate::{ops::build::locate_manifest, ExitStatus};

#[derive(clap::Args)]
pub struct Args {
    /// A boolean Codira expression over `result` and parameter names, e.g.
    /// "result < 0" or "x > 0 && result == 0"
    predicate: String,

    /// Path to the manifest of the project
    #[clap(long, conflicts_with = "path")]
    manifest_path: Option<PathBuf>,

    /// Directory of Codira sources to search instead of a package
    #[clap(long)]
    path: Option<PathBuf>,
}

pub fn query(args: Args) -> anyhow::Result<ExitStatus> {
    let config = Config {
        target: Target::host_target().expect("unable to determine host target"),
        optimization_lvl: codira_compiler::OptimizationLevel::None,
        out_dir: None,
        emit_ir: false,
        mode: codira_compiler::BuildMode::default(),
    };
    let driver = if let Some(path) = &args.path {
        Driver::with_source_directory(path, config)?
    } else {
        let manifest = locate_manifest(args.manifest_path.as_deref())?;
        Driver::with_package_path(&manifest, config)?.1
    };

    let outcome = codira_hir::verify::query::query(driver.db(), &args.predicate)
        .map_err(|e| anyhow::anyhow!(e))?;

    for found in &outcome.matches {
        println!("{}: {}", found.location, found.signature);
        let name = found.function.name(driver.db()).to_string();
        let args: Vec<String> = found
            .inputs
            .iter()
            .map(|(n, v)| format!("{n}: {v}"))
            .collect();
        match &found.result {
            Some(result) => println!("    witness: {name}({}) returns {result}", args.join(", ")),
            None => println!("    witness: {name}({})", args.join(", ")),
        }
    }
    for (location, signature) in &outcome.unknown {
        println!("{location}: {signature}");
        println!("    unknown (resource limit)");
    }
    println!(
        "{} match(es), {} unknown, {} function(s) the predicate does not apply to",
        outcome.matches.len(),
        outcome.unknown.len(),
        outcome.not_applicable
    );
    Ok(ExitStatus::Success)
}

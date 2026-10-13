//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - `codira run`: build the package, then load the library it produced and
//!   invoke its entry point -- `codira build` followed by `codira start`, in
//!   one step.

use std::path::PathBuf;

use codira_compiler::{Config, Target};
use codira_project::Package;

use crate::{
    ops::{
        build::{display_colors, locate_manifest, optimization_level, UseColor},
        start::start_library,
    },
    ExitStatus,
};

#[derive(clap::Args)]
pub struct Args {
    /// Path to the manifest of the project
    #[clap(long)]
    manifest_path: Option<PathBuf>,

    /// Optimization level [0,3]
    #[clap(long, short = 'O', default_value_t = 2)]
    opt_level: u8,

    /// Use color in diagnostics
    #[clap(long, value_enum)]
    color: Option<UseColor>,

    /// Run under the HERACLES self-healing supervisor (see `codira start
    /// --heal`)
    #[clap(long)]
    heal: bool,

    /// The function to invoke
    #[clap(default_value_t = String::from("main"))]
    entry: String,
}

/// Builds the package `args` names and invokes its entry point.
///
/// Diagnostics are printed exactly as `codira build` prints them; if any is
/// an error nothing is run and the exit status is a failure. The library
/// that is run is the package's root module, `target/mod.codiralib`, which is
/// where `codira build` writes it.
pub fn run(args: Args) -> anyhow::Result<ExitStatus> {
    let manifest_path = locate_manifest(args.manifest_path.as_deref())?;
    let colors = display_colors(args.color);

    let config = Config {
        target: Target::host_target().expect("unable to determine host target"),
        optimization_lvl: optimization_level(args.opt_level)?,
        out_dir: None,
        emit_ir: false,
        mode: codira_compiler::BuildMode::default(),
    };

    let package = Package::from_file(&manifest_path)?;
    let library = codira_compiler::ensure_package_output_dir(&package, &config)?
        .join("mod")
        .with_extension("codiralib");

    if !codira_compiler::compile_manifest(&manifest_path, config, colors)? {
        return Ok(ExitStatus::Error);
    }

    start_library(library, args.entry, args.heal)
}

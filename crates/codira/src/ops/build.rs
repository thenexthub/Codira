//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use std::{
    env,
    path::{Path, PathBuf},
};

use anyhow::anyhow;
use codira_compiler::{Config, DisplayColor, Target};
use codira_compiler_daemon::protocol::BuildRequest;
use codira_project::MANIFEST_FILENAME;

use crate::{ops::daemon::build_via_daemon, ExitStatus};

#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, clap::ValueEnum)]
pub enum UseColor {
    Disable,
    Enable,
    Auto,
}

/// `--mode`: whether the Eidos mid-level optimizer runs.
#[derive(Copy, Clone, PartialEq, Eq, clap::ValueEnum)]
pub enum Mode {
    /// Skip Eidos: lower every function straight to LLVM IR. For the
    /// edit-build loop.
    Dev,
    /// Run Eidos (inlining, unrolling, equality saturation, compile-time
    /// evaluation). The default.
    Release,
}

impl Mode {
    fn build_mode(self) -> codira_compiler::BuildMode {
        match self {
            Mode::Dev => codira_compiler::BuildMode::Dev,
            Mode::Release => codira_compiler::BuildMode::Release,
        }
    }
}

#[derive(clap::Args)]
pub struct Args {
    /// Path to the manifest of the project
    #[clap(long)]
    manifest_path: Option<PathBuf>,

    /// Optimization level [0,3]
    #[clap(long, short = 'O', default_value_t = 2)]
    opt_level: u8,

    /// Build mode: `dev` skips the Eidos optimizer for faster builds,
    /// `release` runs it. Independent of `-O`, which drives LLVM.
    #[clap(long, value_enum, default_value_t = Mode::Release)]
    mode: Mode,

    /// Use color in output
    #[clap(long, value_enum)]
    color: Option<UseColor>,

    /// Emits IR instead of a *.codiralib
    #[clap(long)]
    emit_ir: bool,

    /// Run the compiler in watch mode. Watch input files and trigger
    /// recompilation on changes.
    #[clap(long)]
    watch: bool,

    /// Build through the build daemon, starting it if it is not running.
    /// The daemon keeps the compiler warm, so every build after the first
    /// skips process and LLVM startup and recompiles only what changed.
    #[clap(long, conflicts_with = "watch")]
    daemon: bool,

    /// Instrument the generated code with fuel: every function entry and
    /// loop iteration decrements a counter the runtime owns, so a host can
    /// bound how long a call runs (`codira start --fuel <n>`). Costs a few
    /// percent of speed; see `codira_runtime/benches/fuel.rs`.
    #[clap(long, conflicts_with_all = ["watch", "daemon"])]
    fuel: bool,

    /// Target for machine code
    #[clap(long, value_parser=parse_target_triple)]
    target: Option<TargetArg>,

    /// CPU to generate code for: an LLVM CPU name for the target (such as
    /// `znver4` or `apple-m1`), or `native` for the machine running the
    /// compiler. Checked against LLVM's CPU table and recorded with the
    /// features it implies, so the build does not depend on where it runs.
    #[clap(long)]
    target_cpu: Option<String>,

    /// Also write a standalone executable: the Codira runtime with this
    /// package's libraries appended, runnable with nothing else installed.
    #[clap(long, conflicts_with_all = ["watch", "daemon", "emit_ir", "target"])]
    exe: bool,

    /// Where `--exe` writes the executable. Defaults to the package name in
    /// the package's output directory.
    #[clap(long, short = 'o', requires = "exe")]
    output: Option<PathBuf>,
}

/// A `--target` value: the target it resolved to, and the triple as the
/// user wrote it. The triple is kept because it is what a build daemon is
/// sent -- it resolves the name itself, with its own target table.
#[derive(Clone)]
struct TargetArg {
    triple: String,
    target: Target,
}

fn parse_target_triple(target_triple: &str) -> Result<TargetArg, String> {
    Target::search(target_triple)
        .map(|target| TargetArg {
            triple: target_triple.to_owned(),
            target,
        })
        .ok_or_else(|| format!("could not find target for '{target_triple}'"))
}

/// This method is invoked when the executable is run with the `build` argument
/// indicating that a user requested us to build a project in the current
/// directory or one of its parent directories.
pub fn build(args: Args) -> Result<ExitStatus, anyhow::Error> {
    log::trace!("starting build");

    let optimization_lvl = optimization_level(args.opt_level)?;

    let display_colors = display_colors(args.color);
    let manifest_path = locate_manifest(args.manifest_path.as_deref())?;

    log::info!("located build manifest at: {}", manifest_path.display());

    if args.daemon {
        let request = BuildRequest {
            manifest_path: manifest_path.clone(),
            opt_level: args.opt_level,
            mode: args.mode.build_mode().name().to_owned(),
            target: args.target.as_ref().map(|target| target.triple.clone()),
            target_cpu: args.target_cpu.clone(),
            emit_ir: args.emit_ir,
            // Decided here, not by the daemon: this process is the one
            // attached to the terminal the diagnostics will be shown on.
            color: display_colors.should_enable(),
        };
        match build_via_daemon(request) {
            Ok(status) => return Ok(status),
            // The daemon is an optimization. Whatever went wrong with it
            // -- it would not start, it died, it reported an internal
            // error -- the package can still be built right here, and a
            // fresh process gives the answer that does not depend on any
            // cached state.
            Err(error) => eprintln!(
                "warning: could not build through the daemon ({error:#}); building in this \
                 process instead"
            ),
        }
    }

    let target = args.target.map_or_else(
        || Target::host_target().expect("unable to determine host target"),
        |target| target.target,
    );
    let target = match &args.target_cpu {
        Some(cpu) => codira_compiler::resolve_target_cpu(&target, cpu)?,
        None => target,
    };
    log::info!(
        "generating code for {} (cpu {})",
        target.llvm_target,
        target.options.cpu
    );

    let compiler_options = Config {
        target,
        optimization_lvl,
        mode: args.mode.build_mode(),
        out_dir: None,
        emit_ir: args.emit_ir,
    };

    if args.watch {
        return codira_compiler_daemon::compile_and_watch_manifest(
            &manifest_path,
            compiler_options,
            display_colors,
        )
        .map(Into::into);
    }

    let package = codira_project::Package::from_file(&manifest_path)?;
    let output_dir = codira_compiler::ensure_package_output_dir(&package, &compiler_options)?;
    if !codira_compiler::compile_manifest_with(
        &manifest_path,
        compiler_options,
        display_colors,
        |driver| driver.set_fuel_instrumentation(args.fuel),
    )? {
        return Ok(ExitStatus::Error);
    }

    if args.exe {
        let output = args.output.unwrap_or_else(|| {
            output_dir.join(format!(
                "{}{}",
                package.manifest().name(),
                std::env::consts::EXE_SUFFIX
            ))
        });
        crate::ops::exe::bundle(
            &crate::ops::exe::find_launcher()?,
            &output_dir,
            "main",
            &output,
        )?;
        eprintln!("wrote {}", output.display());
    }
    Ok(ExitStatus::Success)
}

/// Decides whether diagnostics are colored: the `--color` flag if given,
/// otherwise `CODIRA_TERMINAL_COLOR`, otherwise auto-detection.
pub(crate) fn display_colors(color: Option<UseColor>) -> DisplayColor {
    color
        .map(|clr| match clr {
            UseColor::Disable => DisplayColor::Disable,
            UseColor::Enable => DisplayColor::Enable,
            UseColor::Auto => DisplayColor::Auto,
        })
        .or_else(|| {
            env::var("CODIRA_TERMINAL_COLOR")
                .map(|value| match value.as_str() {
                    "disable" => DisplayColor::Disable,
                    "enable" => DisplayColor::Enable,
                    _ => DisplayColor::Auto,
                })
                .ok()
        })
        .unwrap_or(DisplayColor::Auto)
}

/// The manifest to build: `manifest_path` if given, otherwise the nearest
/// `codira.toml` in the current directory or one of its parents.
pub(crate) fn locate_manifest(manifest_path: Option<&Path>) -> anyhow::Result<PathBuf> {
    match manifest_path {
        None => {
            let current_dir =
                std::env::current_dir().expect("could not determine current working directory");
            find_manifest(&current_dir).ok_or_else(|| {
                anyhow::anyhow!(
                    "could not find {} in '{}' or a parent directory",
                    MANIFEST_FILENAME,
                    current_dir.display()
                )
            })
        }
        Some(path) => std::fs::canonicalize(path).map_err(|_error| {
            anyhow::anyhow!(
                "'{}' does not refer to a valid manifest path",
                path.display()
            )
        }),
    }
}

/// Maps `-O<n>` to an optimization level.
pub(crate) fn optimization_level(level: u8) -> anyhow::Result<codira_compiler::OptimizationLevel> {
    Ok(match level {
        0 => codira_compiler::OptimizationLevel::None,
        1 => codira_compiler::OptimizationLevel::Less,
        2 => codira_compiler::OptimizationLevel::Default,
        3 => codira_compiler::OptimizationLevel::Aggressive,
        _ => return Err(anyhow!("Only optimization levels 0-3 are supported")),
    })
}

/// Find a Codira manifest file in the specified directory or one of its
/// parents.
pub(crate) fn find_manifest(directory: &Path) -> Option<PathBuf> {
    let mut current_dir = Some(directory);
    while let Some(dir) = current_dir {
        let manifest_path = dir.join(MANIFEST_FILENAME);
        if manifest_path.exists() {
            return Some(manifest_path);
        }
        current_dir = dir.parent();
    }
    None
}

#[cfg(test)]
mod test {
    use codira_project::MANIFEST_FILENAME;

    use super::find_manifest;

    #[test]
    fn test_find_manifest() {
        let dir = tempfile::Builder::new()
            .prefix("test_find_manifest")
            .tempdir()
            .unwrap();
        let path = dir.path();
        let manifest_path = path.join(MANIFEST_FILENAME);

        assert_eq!(find_manifest(path), None);

        std::fs::write(&manifest_path, "").unwrap();
        assert_eq!(find_manifest(path).as_ref(), Some(&manifest_path));

        let subdir_path = path.join("some/random/subdir");
        std::fs::create_dir_all(&subdir_path).unwrap();
        assert_eq!(find_manifest(&subdir_path).as_ref(), Some(&manifest_path));
    }
}

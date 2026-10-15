//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira api-diff <old> <new>`: what changed in a package's public API
//!   between two versions, whether each change is major, minor or patch (see
//!   `codira_hir::verify::api` for the rules), and, with `--check`, whether the
//!   manifests' `version` fields moved far enough.
//!
//! Each side is a package directory (or its `codira.toml`), or a git
//! revision: then it is the current package (`--manifest-path`, or the one
//! around the working directory) as committed at that revision in the
//! repository that contains it, read with `git cat-file` into a temporary
//! directory -- the working tree and the index are not touched.

use std::{
    io::{BufRead, BufReader, Read, Write},
    path::{Path, PathBuf},
    process::{Command, Stdio},
};

use anyhow::{anyhow, bail, Context};
use codira_compiler::{Config, Driver, Target};
use codira_hir::verify::api::{api_diff as diff_apis, Bump};
use codira_project::MANIFEST_FILENAME;

use crate::{ops::build::locate_manifest, ExitStatus};

#[derive(clap::Args)]
pub struct Args {
    /// The old version: a package directory, its manifest, or a git revision
    /// of the current package (e.g. `v1.2.0` or `HEAD~3`)
    old: String,

    /// The new version, in the same forms (e.g. `.`)
    new: String,

    /// The package whose history a git revision refers to (default: the
    /// one around the working directory)
    #[clap(long)]
    manifest_path: Option<PathBuf>,

    /// Fail unless the new manifest's `version` is bumped at least as far as
    /// the changes require (an undecided change requires a major bump)
    #[clap(long)]
    check: bool,
}

/// One side of the comparison, loaded.
struct Side {
    label: String,
    version: semver::Version,
    driver: Driver,
    /// A checkout made for a git revision, removed when done.
    scratch: Option<PathBuf>,
}

impl Drop for Side {
    fn drop(&mut self) {
        if let Some(dir) = &self.scratch {
            let _ = std::fs::remove_dir_all(dir);
        }
    }
}

fn config() -> Config {
    Config {
        target: Target::host_target().expect("unable to determine host target"),
        optimization_lvl: codira_compiler::OptimizationLevel::None,
        out_dir: None,
        emit_ir: false,
        mode: codira_compiler::BuildMode::default(),
    }
}

fn load(manifest: &Path, label: String, scratch: Option<PathBuf>) -> anyhow::Result<Side> {
    let (package, driver) = Driver::with_package_path(manifest, config())
        .with_context(|| format!("could not load {label}"))?;
    Ok(Side {
        label,
        version: package.manifest().version().clone(),
        driver,
        scratch,
    })
}

/// `git -C dir`, finding the repository from `dir` alone. Every `GIT_*`
/// variable is removed first: a git hook exports `GIT_DIR`, `GIT_INDEX_FILE`
/// and others naming the repository the hook runs for, which need not be the
/// package's -- and a command meant for one repository must never act on
/// another.
fn git_command(dir: &Path) -> Command {
    let mut command = Command::new("git");
    for (name, _) in std::env::vars_os() {
        if name.to_string_lossy().starts_with("GIT_") {
            command.env_remove(name);
        }
    }
    command.arg("-C").arg(dir);
    command
}

fn git(dir: &Path, args: &[&str]) -> anyhow::Result<String> {
    let output = git_command(dir)
        .args(args)
        .stdin(Stdio::null())
        .output()
        .context("could not run git")?;
    if !output.status.success() {
        bail!(
            "git {} failed: {}",
            args.join(" "),
            String::from_utf8_lossy(&output.stderr).trim()
        );
    }
    Ok(String::from_utf8_lossy(&output.stdout)
        .trim_end()
        .to_string())
}

/// Writes the package at `package_dir` as committed at `rev` into a fresh
/// temporary directory and returns its manifest.
fn checkout(package_dir: &Path, rev: &str) -> anyhow::Result<(PathBuf, PathBuf)> {
    let top = PathBuf::from(git(package_dir, &["rev-parse", "--show-toplevel"])?);
    let prefix = git(package_dir, &["rev-parse", "--show-prefix"])?;
    let commit = git(
        &top,
        &["rev-parse", "--verify", &format!("{rev}^{{commit}}")],
    )
    .with_context(|| format!("`{rev}` is neither a directory nor a git revision"))?;
    let listing = git(
        &top,
        &[
            "ls-tree",
            "-r",
            "--name-only",
            "--full-tree",
            &commit,
            "--",
            &prefix,
        ],
    )?;
    let files: Vec<&str> = listing.lines().filter(|l| !l.is_empty()).collect();
    if !files
        .iter()
        .any(|f| f.strip_prefix(prefix.as_str()) == Some(MANIFEST_FILENAME))
    {
        bail!("`{rev}` has no {MANIFEST_FILENAME} at `{prefix}`");
    }

    let scratch = std::env::temp_dir().join(format!(
        "codira-api-diff-{}-{}",
        std::process::id(),
        &commit[..12.min(commit.len())]
    ));
    let _ = std::fs::remove_dir_all(&scratch);
    std::fs::create_dir_all(&scratch)?;

    // One `git cat-file --batch` for every file: request and response
    // interleave, `<sha> blob <size>\n<bytes>\n` per object.
    let mut child = git_command(&top)
        .args(["cat-file", "--batch"])
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::null())
        .spawn()
        .context("could not run git")?;
    let mut stdin = child.stdin.take().expect("piped");
    let mut stdout = BufReader::new(child.stdout.take().expect("piped"));
    for file in &files {
        writeln!(stdin, "{commit}:{file}")?;
        stdin.flush()?;
        let mut header = String::new();
        stdout.read_line(&mut header)?;
        let size: usize = header
            .split_whitespace()
            .nth(2)
            .and_then(|s| s.parse().ok())
            .ok_or_else(|| anyhow!("unexpected git cat-file output `{}`", header.trim()))?;
        let mut bytes = vec![0u8; size + 1];
        stdout.read_exact(&mut bytes)?;
        bytes.pop();
        let relative = file.strip_prefix(prefix.as_str()).unwrap_or(file);
        let target = scratch.join(relative);
        if let Some(parent) = target.parent() {
            std::fs::create_dir_all(parent)?;
        }
        std::fs::write(&target, bytes)?;
    }
    drop(stdin);
    child.wait()?;
    Ok((scratch.join(MANIFEST_FILENAME), scratch))
}

fn side(spec: &str, manifest_path: Option<&Path>) -> anyhow::Result<Side> {
    let path = Path::new(spec);
    if path.exists() {
        let manifest = if path.is_dir() {
            path.join(MANIFEST_FILENAME)
        } else {
            path.to_path_buf()
        };
        return load(&manifest, spec.to_string(), None);
    }
    let current = locate_manifest(manifest_path)?;
    let package_dir = current
        .parent()
        .ok_or_else(|| anyhow!("'{}' has no directory", current.display()))?;
    let (manifest, scratch) = checkout(package_dir, spec)?;
    load(&manifest, spec.to_string(), Some(scratch))
}

/// The largest change a version step permits, Cargo's way: below 1.0 the
/// leftmost non-zero component is the breaking one, and every `0.0.z` is
/// incompatible with every other. `None` when `new` is not greater.
pub(crate) fn permitted(old: &semver::Version, new: &semver::Version) -> Option<Bump> {
    if new <= old {
        return None;
    }
    Some(if old.major > 0 || new.major > 0 {
        if new.major != old.major {
            Bump::Major
        } else if new.minor != old.minor {
            Bump::Minor
        } else {
            Bump::Patch
        }
    } else if old.minor > 0 || new.minor > 0 {
        if new.minor == old.minor {
            // `0.y.z` to `0.y.(z+1)`: compatible, additions included.
            Bump::Minor
        } else {
            Bump::Major
        }
    } else {
        Bump::Major
    })
}

pub fn api_diff(args: Args) -> anyhow::Result<ExitStatus> {
    let old = side(&args.old, args.manifest_path.as_deref())?;
    let new = side(&args.new, args.manifest_path.as_deref())?;
    let diff = diff_apis(old.driver.db(), new.driver.db());

    println!(
        "{} ({}) -> {} ({})",
        old.label, old.version, new.label, new.version
    );
    for change in &diff.changes {
        let bump = change
            .bump
            .map_or_else(|| "undecided".to_string(), |b| b.to_string());
        println!("  {bump:>9}: {}", change.summary);
        for detail in &change.details {
            println!("             {detail}");
        }
    }
    println!("{} {}", new.version, diff.sentence());
    let required = diff.required();
    println!("required bump: {required}");

    if !args.check {
        return Ok(ExitStatus::Success);
    }
    match permitted(&old.version, &new.version) {
        _ if diff.changes.is_empty() => Ok(ExitStatus::Success),
        None => {
            println!(
                "error: the version did not increase ({} -> {}) but the API changed",
                old.version, new.version
            );
            Ok(ExitStatus::Error)
        }
        Some(permitted) if permitted < required => {
            println!(
                "error: {} -> {} is a {permitted} bump; these changes need a {required} bump{}",
                old.version,
                new.version,
                if diff.undecided().next().is_some() {
                    " (undecided changes count as major)"
                } else {
                    ""
                }
            );
            Ok(ExitStatus::Error)
        }
        Some(permitted) => {
            println!(
                "ok: {} -> {} is a {permitted} bump",
                old.version, new.version
            );
            Ok(ExitStatus::Success)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{permitted, Bump};

    fn v(text: &str) -> semver::Version {
        semver::Version::parse(text).unwrap()
    }

    #[test]
    fn version_steps_permit_what_cargo_permits() {
        assert_eq!(permitted(&v("1.2.3"), &v("2.0.0")), Some(Bump::Major));
        assert_eq!(permitted(&v("1.2.3"), &v("1.3.0")), Some(Bump::Minor));
        assert_eq!(permitted(&v("1.2.3"), &v("1.2.4")), Some(Bump::Patch));
        assert_eq!(permitted(&v("0.2.3"), &v("0.3.0")), Some(Bump::Major));
        assert_eq!(permitted(&v("0.2.3"), &v("0.2.4")), Some(Bump::Minor));
        assert_eq!(permitted(&v("0.0.3"), &v("0.0.4")), Some(Bump::Major));
        assert_eq!(permitted(&v("1.2.3"), &v("1.2.3")), None);
        assert_eq!(permitted(&v("1.2.3"), &v("1.2.2")), None);
    }
}

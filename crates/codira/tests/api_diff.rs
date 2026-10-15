//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira api-diff --check` through the command line: between two package
//!   directories, and between a git revision and the working tree.

use std::{ffi::OsString, path::Path, process::Command};

use codira::{run_with_args, ExitStatus};

const OLD: &str = r#"
public func parse(x: i64) -> i64
    requires x >= 0
{
    x
}

public func decode(x: i64) -> i64
    ensures result >= 0
{
    0
}
"#;

/// `parse` refuses `x = 0` now: a breaking change.
const NEW: &str = r#"
public func parse(x: i64) -> i64
    requires x > 0
{
    x
}

public func decode(x: i64) -> i64
    ensures result >= 0
{
    0
}

public func extra() -> i64 { 1 }
"#;

fn write_package(dir: &Path, version: &str, source: &str) {
    std::fs::create_dir_all(dir.join("src")).unwrap();
    std::fs::write(
        dir.join("codira.toml"),
        format!("[package]\nname=\"lib\"\nauthors=[]\nversion=\"{version}\"\n"),
    )
    .unwrap();
    std::fs::write(dir.join("src/mod.code"), source).unwrap();
}

fn api_diff(args: &[&str]) -> ExitStatus {
    let args: Vec<OsString> = ["codira", "api-diff"]
        .iter()
        .chain(args)
        .map(OsString::from)
        .collect();
    run_with_args(args).unwrap()
}

#[test]
fn check_refuses_a_minor_bump_for_a_strengthened_precondition() {
    let root = tempfile::tempdir().unwrap();
    let (old, new) = (root.path().join("old"), root.path().join("new"));
    write_package(&old, "1.2.0", OLD);
    write_package(&new, "1.3.0", NEW);
    let (old, new) = (old.to_str().unwrap(), new.to_str().unwrap());
    assert_eq!(api_diff(&[old, new]), ExitStatus::Success, "report only");
    assert_eq!(api_diff(&[old, new, "--check"]), ExitStatus::Error);

    write_package(Path::new(new), "2.0.0", NEW);
    assert_eq!(api_diff(&[old, new, "--check"]), ExitStatus::Success);
}

/// `git -C dir` with every `GIT_*` variable removed. The pre-commit gates
/// run this test inside a git hook, which exports `GIT_DIR`,
/// `GIT_INDEX_FILE` and others naming the real repository; inherited, the
/// scratch repository's `git init` and `git add` act on the real one (an
/// earlier version of this test set the real repository's `core.bare`).
fn scrubbed_git(dir: &Path) -> Command {
    let mut command = Command::new("git");
    for (name, _) in std::env::vars_os() {
        if name.to_string_lossy().starts_with("GIT_") {
            command.env_remove(name);
        }
    }
    command.arg("-C").arg(dir);
    command
}

/// The repository this crate is checked out in is still a normal,
/// non-bare one.
fn assert_real_repository_untouched() {
    let crate_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    let output = scrubbed_git(crate_dir)
        .args(["rev-parse", "--is-bare-repository"])
        .output()
        .unwrap();
    if !output.status.success() {
        // Not built from a git checkout: nothing to protect.
        return;
    }
    assert_eq!(String::from_utf8_lossy(&output.stdout).trim(), "false");
    let bare = scrubbed_git(crate_dir)
        .args(["config", "--get", "core.bare"])
        .output()
        .unwrap();
    assert_ne!(
        String::from_utf8_lossy(&bare.stdout).trim(),
        "true",
        "the real repository's core.bare was set"
    );
}

#[test]
fn a_git_revision_is_compared_without_touching_the_working_tree() {
    assert_real_repository_untouched();
    let repo = tempfile::tempdir().unwrap();
    let git = |args: &[&str]| {
        let mut command = scrubbed_git(repo.path());
        let status = command
            .args([
                "-c",
                "user.name=codira",
                "-c",
                "user.email=codira@example.invalid",
                "-c",
                "commit.gpgsign=false",
            ])
            .args(args)
            .status()
            .unwrap();
        assert!(status.success(), "git {args:?}");
    };
    let package = repo.path().join("pkg");
    write_package(&package, "0.4.0", OLD);
    git(&["init", "-q"]);
    git(&["add", "."]);
    git(&["commit", "-q", "-m", "v0.4.0"]);

    // Adding a function is compatible: 0.4.1 is enough below 1.0.
    let added = format!("{OLD}\npublic func extra() -> i64 {{ 1 }}\n");
    write_package(&package, "0.4.1", &added);
    let manifest = package.join("codira.toml");
    let manifest = manifest.to_str().unwrap();
    assert_eq!(
        api_diff(&[
            "HEAD",
            package.to_str().unwrap(),
            "--check",
            "--manifest-path",
            manifest
        ]),
        ExitStatus::Success
    );

    // The breaking change is not.
    write_package(&package, "0.4.1", NEW);
    assert_eq!(
        api_diff(&[
            "HEAD",
            package.to_str().unwrap(),
            "--check",
            "--manifest-path",
            manifest
        ]),
        ExitStatus::Error
    );
    // The working tree still holds the new source.
    let source = std::fs::read_to_string(package.join("src/mod.code")).unwrap();
    assert!(source.contains("requires x > 0"));
    assert_real_repository_untouched();
}

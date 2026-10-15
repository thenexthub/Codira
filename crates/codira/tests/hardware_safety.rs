//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end tests of the experimental hardware-safety options of the real
//!   `codira` binary: `--sanitize=memtag` builds an aarch64 Linux library with
//!   `+mte` and refuses targets that cannot honour it.

use std::{
    path::Path,
    process::{Command, Output, Stdio},
    time::{Duration, Instant},
};

/// Writes a one-file package whose `src/mod.code` is `source`.
fn package_with(source: &str) -> tempfile::TempDir {
    let dir = tempfile::Builder::new()
        .prefix("codira_hwsafety")
        .tempdir()
        .unwrap();
    std::fs::write(
        dir.path().join("codira.toml"),
        "[package]\nname = \"app\"\nauthors = []\nversion = \"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir(dir.path().join("src")).unwrap();
    std::fs::write(dir.path().join("src/mod.code"), source).unwrap();
    dir
}

/// Runs `codira <args>` on the package in `dir`, killing it after 300 s.
fn codira(dir: &Path, args: &[&str]) -> Output {
    let mut child = Command::new(env!("CARGO_BIN_EXE_codira"))
        .args(args)
        .arg("--manifest-path")
        .arg(dir.join("codira.toml"))
        .arg("--color")
        .arg("disable")
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .expect("failed to spawn codira");
    let deadline = Instant::now() + Duration::from_secs(300);
    loop {
        if child.try_wait().expect("child status").is_some() {
            return child.wait_with_output().expect("child output");
        }
        if Instant::now() > deadline {
            let _ignored = child.kill();
            panic!("`codira {args:?}` did not finish within 300 s");
        }
        std::thread::sleep(Duration::from_millis(20));
    }
}

#[test]
fn memtag_refuses_targets_that_cannot_honour_it() {
    let dir = package_with("public func main() {}\n");
    let refused = |args: &[&str], expected: &str| {
        let output = codira(dir.path(), args);
        let stderr = String::from_utf8_lossy(&output.stderr);
        assert!(!output.status.success(), "{args:?} must be refused");
        assert!(stderr.contains(expected), "{args:?}: {stderr}");
    };
    refused(
        &[
            "build",
            "--sanitize",
            "memtag",
            "--target",
            "x86_64-unknown-linux-gnu",
        ],
        "needs an aarch64 target with the `+mte` feature",
    );
    refused(
        &[
            "build",
            "--sanitize",
            "memtag",
            "--target",
            "aarch64-unknown-linux-gnu",
        ],
        "needs the `+mte` target feature",
    );
    refused(
        &[
            "build",
            "--sanitize",
            "memtag",
            "--target",
            "aarch64-unknown-linux-gnu",
            "--target-feature",
            "mte",
        ],
        "is not a target feature change",
    );
    // A CHERI target is explained, not faked.
    refused(&["build", "--target", "aarch64-morello"], "CHERI LLVM");
    refused(
        &["build", "--target", "aarch64-unknown-freebsd-purecap"],
        "purecap ABI",
    );
}

#[test]
fn memtag_builds_an_aarch64_linux_library_with_mte() {
    let dir = package_with(
        r#"extern "codira-intrinsic" {
    func stack_alloc(size: usize, align: usize) -> usize;
    func store_u64(addr: usize, value: u64);
    func load_u64(addr: usize) -> u64;
}
public func scratch(x: u64) -> u64 {
    let slot = stack_alloc(16, 8);
    store_u64(slot, x);
    load_u64(slot)
}
"#,
    );
    let output = codira(
        dir.path(),
        &[
            "build",
            "--sanitize",
            "memtag",
            "--target",
            "aarch64-unknown-linux-gnu",
            "--target-feature",
            "+mte",
        ],
    );
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end tests of `@noalloc`, `@nopanic`, `@noblock` and `@realtime`: a
//!   package whose annotated functions keep their promises builds and runs,
//!   `codira check` shows the chain of calls behind a broken promise, and a
//!   `--fuel` build refuses `@realtime` (and `@nopanic`).

use std::{
    ffi::OsString,
    path::Path,
    process::{Command, Stdio},
    time::{Duration, Instant},
};

use codira::run_with_args;
use codira_runtime::Runtime;

const KEPT: &str = r#"
struct Sample {
    value: i64,
    weight: i64,
}

@realtime
public func weighted_sum(n: i64) -> i64
    requires n >= 0 && n <= 1000
{
    var i = 0
    var total = 0
    while i < n
        invariant i >= 0 && i <= n
        decreases n - i
    {
        let s = Sample { value: i, weight: 3 }
        total += scaled(s)
        i += 1
    }
    total
}

@realtime
func scaled(s: Sample) -> i64 {
    s.value * s.weight
}

@realtime
public func mean(total: i64, count: i64) -> i64
    requires count > 0
{
    total / count
}

@realtime
public func count_odd(n: i64) -> i64 {
    var acc = 0
    for i in 0..n { acc += i % 2 }
    acc
}
"#;

const BROKEN: &str = r#"
class Particle {
    x: i64
}

@noalloc
public func tick(n: i64) -> i64 {
    step(n)
}

func step(n: i64) -> i64 {
    let p = Particle { x: n }
    p.x
}
"#;

fn write_package(dir: &Path, source: &str) {
    std::fs::write(
        dir.join("codira.toml"),
        "[package]\nname=\"realtime\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir_all(dir.join("src")).unwrap();
    std::fs::write(dir.join("src/mod.code"), source).unwrap();
}

fn build(dir: &Path, extra: &[&str]) -> codira::ExitStatus {
    let mut args: Vec<OsString> = vec![
        "codira".into(),
        "build".into(),
        "--manifest-path".into(),
        dir.join("codira.toml").into(),
    ];
    args.extend(extra.iter().map(OsString::from));
    run_with_args(args).unwrap()
}

/// Runs the `codira` binary with a hard timeout, so a hang cannot hold the
/// build lock.
fn codira(dir: &Path, args: &[&str]) -> (bool, String) {
    let mut child = Command::new(env!("CARGO_BIN_EXE_codira"))
        .args(args)
        .arg("--manifest-path")
        .arg(dir.join("codira.toml"))
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .unwrap();
    let deadline = Instant::now() + Duration::from_secs(300);
    while child.try_wait().unwrap().is_none() {
        if Instant::now() > deadline {
            child.kill().unwrap();
            panic!("`codira {}` did not finish within 300 s", args.join(" "));
        }
        std::thread::sleep(Duration::from_millis(50));
    }
    let output = child.wait_with_output().unwrap();
    let text = format!(
        "{}\n--- stderr ---\n{}",
        String::from_utf8_lossy(&output.stdout),
        String::from_utf8_lossy(&output.stderr)
    );
    (output.status.success(), text)
}

#[test]
fn kept_promises_build_and_run() {
    let project = tempfile::tempdir().unwrap();
    write_package(project.path(), KEPT);
    assert_eq!(build(project.path(), &[]), codira::ExitStatus::Success);

    let library = project.path().join("target/mod.codiralib");
    // Safety: the library was compiled by this test.
    let runtime = unsafe { Runtime::builder(&library).finish() }.unwrap();
    let sum: i64 = runtime.invoke("weighted_sum", (10i64,)).unwrap();
    assert_eq!(sum, 135);
    let mean: i64 = runtime.invoke("mean", (135i64, 10i64)).unwrap();
    assert_eq!(mean, 13);
    let odd: i64 = runtime.invoke("count_odd", (5i64,)).unwrap();
    assert_eq!(odd, 2);
}

#[test]
fn check_shows_the_call_chain_of_a_broken_promise() {
    let project = tempfile::tempdir().unwrap();
    write_package(project.path(), BROKEN);
    let (ok, out) = codira(project.path(), &["check"]);
    assert!(!ok, "{out}");
    assert!(
        out.contains(
            "`tick` is `@noalloc` but calls `step` (line 8), which allocates: class `Particle` \
             constructed at line 12"
        ),
        "{out}"
    );
    assert_eq!(build(project.path(), &[]), codira::ExitStatus::Error);
}

#[test]
fn a_fuel_build_refuses_realtime_functions() {
    let project = tempfile::tempdir().unwrap();
    write_package(project.path(), KEPT);
    assert_eq!(
        build(project.path(), &["--fuel"]),
        codira::ExitStatus::Error
    );
    assert_eq!(build(project.path(), &[]), codira::ExitStatus::Success);
}

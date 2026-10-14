//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Builds a standalone executable with `codira build --exe`, moves it out of
//!   the project, and runs it: its output and its exit code come from the
//!   Codira program inside it.

use std::{ffi::OsString, process::Command};

use codira::run_with_args;

#[test]
fn an_executable_runs_on_its_own() {
    let project = tempfile::tempdir().unwrap();
    std::fs::write(
        project.path().join("codira.toml"),
        "[package]\nname = \"standalone\"\nversion = \"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir(project.path().join("src")).unwrap();
    std::fs::write(
        project.path().join("src/mod.code"),
        "func triple(x: i64) -> i64 { x * 3 }\n\n\
         public func main() -> i32 {\n\
         \x20   println(\"standalone says \", triple(14))\n\
         \x20   7\n\
         }\n",
    )
    .unwrap();

    // The executable is written somewhere else and the project deleted, so
    // nothing but the file itself can be what runs.
    let elsewhere = tempfile::tempdir().unwrap();
    let exe = elsewhere
        .path()
        .join(format!("standalone{}", std::env::consts::EXE_SUFFIX));
    std::env::set_var("CODIRA_LAUNCHER", env!("CARGO_BIN_EXE_codira-launcher"));
    let args: Vec<OsString> = [
        "codira",
        "build",
        "--manifest-path",
        &project.path().join("codira.toml").display().to_string(),
        "--exe",
        "-o",
        &exe.display().to_string(),
    ]
    .into_iter()
    .map(OsString::from)
    .collect();
    assert_eq!(run_with_args(args).unwrap(), codira::ExitStatus::Success);
    drop(project);

    for run in 0..2 {
        // The second run finds the libraries already unpacked.
        let output = Command::new(&exe).output().unwrap();
        assert_eq!(
            String::from_utf8_lossy(&output.stdout).trim(),
            "standalone says 42",
            "run {run}, stderr: {}",
            String::from_utf8_lossy(&output.stderr)
        );
        assert_eq!(output.status.code(), Some(7), "run {run}");
    }
}

#[test]
fn the_bare_launcher_says_what_it_is() {
    let output = Command::new(env!("CARGO_BIN_EXE_codira-launcher"))
        .output()
        .unwrap();
    assert_eq!(output.status.code(), Some(2));
    assert!(String::from_utf8_lossy(&output.stderr).contains("codira build --exe"));
}

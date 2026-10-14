//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Proof certificates end to end: `codira build --certify` writes one,
//!   `codira certify` accepts it, and rejects the library or the certificate
//!   once either is tampered with; an obligation the compiler left to a
//!   run-time check is reported as unproven.

use std::{
    ffi::OsString,
    path::{Path, PathBuf},
};

use codira::{run_with_args, ExitStatus};
use codira_certificate::{check, Certificate, CheckOptions};
use codira_runtime::Runtime;

const VERIFIED: &str = r#"
public func clamp(x: i64) -> i64
    ensures result >= 0 && result <= 100
{
    if x < 0 { return 0 }
    if x > 100 { 100 } else { x }
}

public func sum_to(n: i64) -> i64
    requires n >= 0 && n <= 1000
    ensures result >= 0
{
    var i = 0
    var s = 0
    while i < n
        invariant i >= 0 && i <= n
        invariant s >= 0 && s <= i * 1000
        decreases n - i
    {
        i += 1
        s += i
    }
    s
}

public func call_clamp() -> i64 {
    clamp(250)
}

public func main() -> i32 {
    (call_clamp() - 100) as i32
}
"#;

fn run(args: &[&str]) -> ExitStatus {
    let args: Vec<OsString> = std::iter::once("codira")
        .chain(args.iter().copied())
        .map(OsString::from)
        .collect();
    run_with_args(args).unwrap()
}

fn write_package(dir: &Path, source: &str) -> PathBuf {
    std::fs::write(
        dir.join("codira.toml"),
        "[package]\nname=\"certified\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir_all(dir.join("src")).unwrap();
    std::fs::write(dir.join("src/mod.code"), source).unwrap();
    dir.join("codira.toml")
}

fn build_certified(dir: &Path, source: &str) -> PathBuf {
    let manifest = write_package(dir, source);
    assert_eq!(
        run(&[
            "build",
            "--certify",
            "--manifest-path",
            manifest.to_str().unwrap()
        ]),
        ExitStatus::Success
    );
    dir.join("target/mod.codiralib")
}

fn certificate_of(library: &Path) -> Certificate {
    let text = std::fs::read_to_string(codira_certificate::sidecar_path(library)).unwrap();
    Certificate::from_json(&text).unwrap()
}

#[test]
fn a_certified_library_checks_and_a_changed_code_byte_is_pinned_on_its_function() {
    let project = tempfile::tempdir().unwrap();
    let library = build_certified(project.path(), VERIFIED);
    let certificate = certificate_of(&library);

    // Every function's code is located, and every obligation is recorded as
    // proven with queries to back it.
    let clamp = certificate
        .functions
        .iter()
        .find(|f| f.symbol == "clamp")
        .expect("clamp is certified");
    assert!(clamp.code.is_some(), "{:?}", clamp.code_note);
    assert_eq!(clamp.obligations.len(), 1);
    assert!(clamp.obligations[0].proven);
    assert_eq!(clamp.obligations[0].queries.len(), 2, "one per return");
    let caller = certificate
        .functions
        .iter()
        .find(|f| f.symbol == "call_clamp")
        .unwrap();
    assert!(caller.obligations.is_empty(), "clamp has no `requires`");

    let bytes = std::fs::read(&library).unwrap();
    let options = CheckOptions {
        source_dir: Some(project.path().join("src")),
        ..CheckOptions::default()
    };
    let report = check(&certificate, &bytes, &options);
    assert!(report.passed(), "{report}");
    assert_eq!(report.code_checked, certificate.functions.len());
    assert!(report.obligations_proven >= 5, "{report}");
    assert_eq!(report.sources_checked, certificate.functions.len());
    assert!(report.unproven.is_empty());
    assert_eq!(
        run(&["certify", library.to_str().unwrap()]),
        ExitStatus::Success
    );

    // The certified library is an ordinary one: it loads and runs.
    {
        // Safety: the library was compiled by this test.
        let runtime = unsafe { Runtime::builder(&library).finish() }.unwrap();
        let clamped: i64 = runtime.invoke("clamp", (250i64,)).unwrap();
        assert_eq!(clamped, 100);
    }

    // One byte of `clamp`'s machine code changed.
    let code = clamp.code.as_ref().unwrap();
    let mut tampered = bytes.clone();
    tampered[code.offset as usize + 1] ^= 0x40;
    let report = check(&certificate, &tampered, &CheckOptions::default());
    assert!(!report.passed());
    assert!(
        report
            .failures
            .iter()
            .any(|f| f.contains("`clamp`") && f.contains("machine code")),
        "{report}"
    );
    assert!(
        report.failures.iter().any(|f| f.contains("not the file")),
        "{report}"
    );
    std::fs::write(&library, &tampered).unwrap();
    assert_eq!(
        run(&["certify", library.to_str().unwrap()]),
        ExitStatus::Error
    );

    // A changed source line no longer matches.
    let source = project.path().join("src/mod.code");
    let text = std::fs::read_to_string(&source).unwrap();
    std::fs::write(&source, text.replace("x > 100", "x > 101")).unwrap();
    let report = check(&certificate, &bytes, &options);
    assert!(
        report
            .failures
            .iter()
            .any(|f| f.contains("`clamp`") && f.contains("not the source")),
        "{report}"
    );
}

#[test]
fn a_tampered_query_is_rejected() {
    let project = tempfile::tempdir().unwrap();
    let library = build_certified(project.path(), VERIFIED);
    let bytes = std::fs::read(&library).unwrap();
    let mut certificate = certificate_of(&library);

    // Replace the negated goal of `clamp`'s first query by `true`: the query
    // now asks whether the path is reachable at all, which it is, so Z3
    // answers `sat` and the recorded proof does not hold.
    let clamp = certificate
        .functions
        .iter_mut()
        .find(|f| f.symbol == "clamp")
        .unwrap();
    let query = &mut clamp.obligations[0].queries[0].smtlib2;
    let last_assert = query.rfind("(assert").unwrap();
    let check_sat = query.rfind("(check-sat)").unwrap();
    query.replace_range(last_assert..check_sat, "(assert true)\n");
    let report = check(&certificate, &bytes, &CheckOptions::default());
    assert!(!report.passed());
    assert!(
        report
            .failures
            .iter()
            .any(|f| f.contains("`clamp`") && f.contains("does not hold")),
        "{report}"
    );

    // A query that smuggles in a command is refused, not run.
    let mut certificate = certificate_of(&library);
    let clamp = certificate
        .functions
        .iter_mut()
        .find(|f| f.symbol == "clamp")
        .unwrap();
    clamp.obligations[0].queries[0]
        .smtlib2
        .insert_str(0, "(set-option :rlimit 1)\n");
    let report = check(&certificate, &bytes, &CheckOptions::default());
    assert!(
        report.failures.iter().any(|f| f.contains("`set-option`")),
        "{report}"
    );

    // And through the command line: the edited certificate is rejected.
    std::fs::write(
        codira_certificate::sidecar_path(&library),
        certificate.to_json(),
    )
    .unwrap();
    assert_eq!(
        run(&["certify", library.to_str().unwrap()]),
        ExitStatus::Error
    );
}

#[test]
fn an_obligation_left_to_a_run_time_check_is_reported_as_unproven() {
    let project = tempfile::tempdir().unwrap();
    let library = build_certified(
        project.path(),
        r#"
@runtime_checked
public func scale(x: f64) -> f64
    requires x >= 0.0
{
    x * 2.0
}

public func half(x: i64) -> i64
    requires x >= 0
    ensures result >= 0
{
    x / 2
}
"#,
    );
    let certificate = certificate_of(&library);
    let report = check(
        &certificate,
        &std::fs::read(&library).unwrap(),
        &CheckOptions::default(),
    );
    assert!(report.passed(), "{report}");
    assert!(
        report
            .unproven
            .iter()
            .any(|u| u.contains("`scale`") && u.contains("x >= 0.0") && u.contains("run time")),
        "{report}"
    );
    assert!(report.obligations_proven >= 1, "{report}");

    // Accepted by default, refused on request.
    let library = library.to_str().unwrap();
    assert_eq!(run(&["certify", library]), ExitStatus::Success);
    assert_eq!(
        run(&["certify", "--deny-unproven", library]),
        ExitStatus::Error
    );
}

#[test]
fn an_executable_carries_its_certificates() {
    let project = tempfile::tempdir().unwrap();
    let manifest = write_package(project.path(), VERIFIED);
    let exe = project
        .path()
        .join(format!("certified{}", std::env::consts::EXE_SUFFIX));
    std::env::set_var("CODIRA_LAUNCHER", env!("CARGO_BIN_EXE_codira-launcher"));
    assert_eq!(
        run(&[
            "build",
            "--certify",
            "--manifest-path",
            manifest.to_str().unwrap(),
            "--exe",
            "-o",
            exe.to_str().unwrap(),
        ]),
        ExitStatus::Success
    );
    assert_eq!(
        run(&["certify", exe.to_str().unwrap()]),
        ExitStatus::Success
    );

    // It still runs: `main` returns clamp(250) - 100 = 0.
    let status = std::process::Command::new(&exe).status().unwrap();
    assert_eq!(status.code(), Some(0));

    // A library built without `--certify` has no certificate, and says so.
    let plain = tempfile::tempdir().unwrap();
    let manifest = write_package(plain.path(), VERIFIED);
    assert_eq!(
        run(&["build", "--manifest-path", manifest.to_str().unwrap()]),
        ExitStatus::Success
    );
    assert_eq!(
        run(&["certify", plain.path().join("target").to_str().unwrap()]),
        ExitStatus::Error
    );
}

/// A cross-compiled ELF library is certified the same way; ELF symbols carry
/// their exact sizes.
#[test]
fn a_cross_compiled_elf_library_is_certified_too() {
    let project = tempfile::tempdir().unwrap();
    let manifest = write_package(project.path(), VERIFIED);
    assert_eq!(
        run(&[
            "build",
            "--certify",
            "--target",
            "x86_64-unknown-linux-gnu",
            "--manifest-path",
            manifest.to_str().unwrap()
        ]),
        ExitStatus::Success
    );
    let library = project
        .path()
        .join("target/x86_64-unknown-linux-gnu/mod.codiralib");
    let library = if library.is_file() {
        library
    } else {
        project.path().join("target/mod.codiralib")
    };
    let bytes = std::fs::read(&library).unwrap();
    assert!(bytes.starts_with(b"\x7fELF"), "an ELF shared object");
    let certificate = certificate_of(&library);
    let report = check(&certificate, &bytes, &CheckOptions::default());
    assert!(report.passed(), "{report}");
    assert_eq!(report.code_checked, certificate.functions.len(), "{report}");
    assert_eq!(
        run(&["certify", library.to_str().unwrap()]),
        ExitStatus::Success
    );
}

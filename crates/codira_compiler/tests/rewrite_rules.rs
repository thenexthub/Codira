//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - A package's `@rewrite` rules are proven when it is built: a proven rule
//!   builds, a false one fails the build with the counterexample.

use codira_compiler::{Config, Driver};
use tempfile::TempDir;

fn package(source: &str) -> TempDir {
    let dir = tempfile::tempdir().unwrap();
    std::fs::write(
        dir.path().join("codira.toml"),
        "[package]\nname=\"rules\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir(dir.path().join("src")).unwrap();
    std::fs::write(dir.path().join("src").join("mod.code"), source).unwrap();
    dir
}

fn build(dir: &TempDir) -> anyhow::Result<()> {
    let (_, mut driver) =
        Driver::with_package_path(dir.path().join("codira.toml"), Config::default())?;
    driver.write_all_assemblies(true)
}

#[test]
fn a_proven_rule_builds() {
    let dir = package(
        "@rewrite(from: 3 * a + 2 * a, to: 5 * a)\npublic func five(a: i64) -> i64 { 5 * a }\n",
    );
    build(&dir).unwrap();
}

#[test]
fn a_false_rule_fails_the_build_with_a_counterexample() {
    let dir =
        package("@rewrite(from: a + a, to: a * 3)\npublic func double(a: i64) -> i64 { a + a }\n");
    let error = format!("{:#}", build(&dir).unwrap_err());
    assert!(
        error.contains("`a + a => a * 3` on `double` is refused"),
        "{error}"
    );
    assert!(error.contains("differ for a = "), "{error}");
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - `codira build --target wasm32-unknown-unknown` writes a `.wasm` module
//!   next to where a `.codiralib` would go, and a package that needs the Codira
//!   runtime fails the build with the reason instead of producing a module that
//!   would trap.

use codira_compiler::{Config, Driver, Target};
use tempfile::TempDir;

fn package(source: &str) -> TempDir {
    let dir = tempfile::tempdir().unwrap();
    std::fs::write(
        dir.path().join("codira.toml"),
        "[package]\nname=\"web\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir(dir.path().join("src")).unwrap();
    std::fs::write(dir.path().join("src").join("mod.code"), source).unwrap();
    dir
}

fn build(dir: &TempDir) -> anyhow::Result<()> {
    let config = Config {
        target: Target::search("wasm32-unknown-unknown").unwrap(),
        ..Config::default()
    };
    let (_, mut driver) = Driver::with_package_path(dir.path().join("codira.toml"), config)?;
    driver.write_all_assemblies(true)
}

#[test]
fn a_wasm_build_writes_a_module() {
    let dir = package("@export(\"C\")\npublic func add(a: i32, b: i32) -> i32 { a + b }\n");
    build(&dir).unwrap();
    let module = std::fs::read(dir.path().join("target").join("mod.wasm")).unwrap();
    assert_eq!(&module[..4], b"\0asm");
    assert!(!dir.path().join("target").join("mod.codiralib").exists());
}

#[test]
fn a_package_that_needs_the_runtime_fails_the_build() {
    let dir = package(
        "class Box {\n    var v: i64\n}\n\n@export(\"C\")\npublic func f() -> i64 {\n    let b = Box { v: 1 }\n    b.v\n}\n",
    );
    let error = format!("{:#}", build(&dir).unwrap_err());
    assert!(error.contains("no Codira runtime"), "{error}");
    assert!(!dir.path().join("target").join("mod.wasm").exists());
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - A host build of a package with `@target(gpu)` functions writes their PTX
//!   next to the library, and a kernel that cannot run on a GPU fails the build
//!   with the reason.

use codira_compiler::{Config, Driver};
use tempfile::TempDir;

fn package(source: &str) -> TempDir {
    let dir = tempfile::tempdir().unwrap();
    std::fs::write(
        dir.path().join("codira.toml"),
        "[package]\nname=\"kernels\"\nauthors=[]\nversion=\"0.1.0\"\n",
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

const SCALE: &str = r#"
extern "codira-intrinsic" {
    func gpu_thread_id_x() -> u32;
    func load_f64(addr: usize) -> f64;
    func store_f64(addr: usize, value: f64);
}

@target(gpu)
public func scale(data: usize, factor: f64) {
    let at = data + (gpu_thread_id_x() as usize) * 8
    store_f64(at, load_f64(at) * factor)
}

public func host_side(x: i64) -> i64 { x + 1 }
"#;

#[test]
fn a_host_build_writes_the_kernels_ptx_next_to_the_library() {
    let dir = package(SCALE);
    build(&dir).unwrap();
    let target = dir.path().join("target");
    assert!(target.join("mod.codiralib").is_file());
    let ptx = std::fs::read_to_string(target.join("mod.ptx")).unwrap();
    assert!(ptx.contains(".entry scale"), "{ptx}");
}

#[test]
fn a_package_without_kernels_writes_no_ptx() {
    let dir = package("public func f() -> i64 { 1 }\n");
    build(&dir).unwrap();
    assert!(!dir.path().join("target").join("mod.ptx").exists());
}

#[test]
fn a_kernel_that_needs_the_runtime_fails_the_build() {
    let dir = package(
        "class Box {\n    var v: i64\n}\n\n@target(gpu)\npublic func k(x: i64) {\n    let b = Box { v: x }\n}\n",
    );
    let error = format!("{:#}", build(&dir).unwrap_err());
    assert!(error.contains("does not exist on the GPU"), "{error}");
}

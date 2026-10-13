//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - A package's `[eidos]` table and the build mode reach the salsa input that
//!   codegen reads, and a manifest asking for a wall-clock bound is refused
//!   before anything is built.

use codira_compiler::{BuildMode, Config, Driver, EidosOptions};
use tempfile::TempDir;

fn package(eidos_table: &str) -> TempDir {
    let dir = tempfile::tempdir().unwrap();
    std::fs::write(
        dir.path().join("codira.toml"),
        format!("[package]\nname=\"sample\"\nauthors=[]\nversion=\"0.1.0\"\n\n{eidos_table}\n"),
    )
    .unwrap();
    std::fs::create_dir(dir.path().join("src")).unwrap();
    std::fs::write(
        dir.path().join("src").join("mod.code"),
        "public func main() -> i64 {\n    40 + 2\n}\n",
    )
    .unwrap();
    dir
}

fn driver(dir: &TempDir, mode: BuildMode) -> anyhow::Result<Driver> {
    let config = Config {
        mode,
        ..Config::default()
    };
    Driver::with_package_path(dir.path().join("codira.toml"), config).map(|(_, driver)| driver)
}

#[test]
fn without_a_table_the_defaults_apply() {
    let dir = package("");
    let options = driver(&dir, BuildMode::Release).unwrap().eidos_options();
    assert_eq!(options, EidosOptions::default());
}

#[test]
fn the_table_sets_the_bounds_and_the_mode_comes_from_the_config() {
    let dir = package(
        "[eidos]\nmax_egraph_nodes_per_region = 2500\nmax_egraph_iterations = 3\n\
         exact_extraction_budget = 0",
    );
    let options = driver(&dir, BuildMode::Dev).unwrap().eidos_options();
    assert_eq!(
        options,
        EidosOptions {
            mode: BuildMode::Dev,
            max_egraph_nodes_per_region: 2500,
            max_egraph_iterations: 3,
            exact_extraction_budget: 0,
        }
    );
}

#[test]
fn a_partial_table_keeps_the_other_defaults() {
    let dir = package("[eidos]\nmax_egraph_iterations = 12");
    let options = driver(&dir, BuildMode::Release).unwrap().eidos_options();
    assert_eq!(options.max_egraph_iterations, 12);
    assert_eq!(
        options.max_egraph_nodes_per_region,
        EidosOptions::default().max_egraph_nodes_per_region
    );
}

#[test]
fn a_timeout_is_refused_before_anything_is_built() {
    let dir = package("[eidos]\negraph_timeout_ms = 50");
    let error = format!("{:#}", driver(&dir, BuildMode::Release).err().unwrap());
    assert!(error.contains("egraph_timeout_ms"), "{error}");
    assert!(error.contains("deterministic"), "{error}");
    assert!(!dir.path().join("target").exists());
}

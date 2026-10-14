//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Keeps `tests/generated/` in step with the verifier: one runnable `@test`
//!   per function that has a refuted proof obligation, written from the
//!   counterexample Z3 found (`codira_hir::verify::testgen`), and none for a
//!   function whose obligations are all proven.
//!
//! Only files that start with `testgen::GENERATED_MARKER` are ever rewritten
//! or removed, so anything a person put in the directory is left alone.

use std::{
    collections::BTreeMap,
    path::{Path, PathBuf},
};

use codira_hir::{
    verify::testgen::{self, GENERATED_MARKER},
    HirDatabase, Package,
};

/// What [`sync_generated_tests`] changed on disk.
#[derive(Debug, Default)]
pub(crate) struct SyncReport {
    pub written: Vec<PathBuf>,
    pub removed: Vec<PathBuf>,
}

/// The directory generated tests live in, under a package root.
pub(crate) fn generated_dir(package_root: &Path) -> PathBuf {
    package_root.join("tests").join("generated")
}

fn is_generated(path: &Path) -> bool {
    std::fs::read_to_string(path).is_ok_and(|text| text.lines().next() == Some(GENERATED_MARKER))
}

/// Writes a counterexample test for every function of every package in
/// `db` that has a refuted obligation, and removes generated tests that no
/// longer correspond to one.
pub(crate) fn sync_generated_tests(db: &dyn HirDatabase, dir: &Path) -> anyhow::Result<SyncReport> {
    let mut wanted: BTreeMap<String, String> = BTreeMap::new();
    for package in Package::all(db) {
        for module in package.modules(db) {
            for function in module.all_functions(db) {
                if let Some(test) = testgen::counterexample_test(db, function) {
                    wanted.insert(test.file_name, test.contents);
                }
            }
        }
    }

    let mut report = SyncReport::default();
    if dir.is_dir() {
        for entry in std::fs::read_dir(dir)? {
            let path = entry?.path();
            let name = path.file_name().and_then(|n| n.to_str()).unwrap_or("");
            if !wanted.contains_key(name) && path.is_file() && is_generated(&path) {
                std::fs::remove_file(&path)?;
                report.removed.push(path);
            }
        }
    }
    if !wanted.is_empty() {
        std::fs::create_dir_all(dir)?;
    }
    for (name, contents) in wanted {
        let path = dir.join(&name);
        let unchanged = std::fs::read_to_string(&path).is_ok_and(|old| old == contents);
        if !unchanged {
            // A file of the same name that a person wrote is not ours to
            // overwrite.
            if path.exists() && !is_generated(&path) {
                continue;
            }
            std::fs::write(&path, contents)?;
            report.written.push(path);
        }
    }
    Ok(report)
}

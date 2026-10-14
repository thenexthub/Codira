//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end check that `ct_select` on a secret compiles to branch-free LLVM
//!   IR (a `select` marked `!unpredictable`, no conditional branch), and that
//!   the program computes the right values.

use std::{ffi::OsString, path::Path};

use codira::run_with_args;
use codira_runtime::Runtime;

const SOURCE: &str = r#"
oblivious func ct_max(a: u64, b: u64) -> u64 {
    ct_select(a > b, a, b)
}

@constant_time
func ct_equal(a: secret u64, b: secret u64) -> secret bool {
    let diff: secret u64 = a ^ b;
    diff == 0
}

public func max_of(a: u64, b: u64) -> u64 {
    declassify(ct_max(a, b))
}

public func equal(a: u64, b: u64) -> bool {
    declassify(ct_equal(a, b))
}
"#;

fn write_package(dir: &Path) {
    std::fs::write(
        dir.join("codira.toml"),
        "[package]\nname=\"ct\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir_all(dir.join("src")).unwrap();
    std::fs::write(dir.join("src/mod.code"), SOURCE).unwrap();
}

fn build(dir: &Path, extra: &[&str]) {
    let mut args: Vec<OsString> = vec![
        "codira".into(),
        "build".into(),
        "--manifest-path".into(),
        dir.join("codira.toml").into(),
    ];
    args.extend(extra.iter().map(OsString::from));
    assert_eq!(run_with_args(args).unwrap(), codira::ExitStatus::Success);
}

/// The text of every IR function definition whose `define` line contains
/// `name`.
fn functions_ir<'a>(ir: &'a str, name: &str) -> Vec<&'a str> {
    ir.match_indices("define ")
        .map(|(start, _)| start)
        .filter(|&start| ir[start..].lines().next().is_some_and(|l| l.contains(name)))
        .map(|start| {
            let end = ir[start..].find("\n}").map_or(ir.len(), |e| start + e + 2);
            &ir[start..end]
        })
        .collect()
}

#[test]
fn ct_select_is_a_branch_free_select_in_the_ir_at_every_level() {
    for opt in ["0", "2", "3"] {
        let project = tempfile::tempdir().unwrap();
        write_package(project.path());
        build(project.path(), &["--emit-ir", "-O", opt]);
        let ir = std::fs::read_to_string(project.path().join("target/mod.ll")).unwrap();
        // Optimisation may inline the constant-time functions into their
        // public callers, so every one of them is checked.
        let mut checked = 0;
        for name in ["ct_max", "ct_equal", "max_of", "equal"] {
            for body in functions_ir(&ir, name) {
                checked += 1;
                assert!(
                    !body.contains("br i1"),
                    "-O{opt}: `{name}` branches on a value:\n{body}"
                );
            }
        }
        assert!(checked >= 2, "-O{opt}: no functions found in:\n{ir}");
        if opt == "0" {
            let max = functions_ir(&ir, "ct_max").concat();
            assert!(max.contains("select"), "-O0: no select in:\n{max}");
            assert!(
                max.contains("!unpredictable"),
                "-O0: the select lost `!unpredictable`:\n{max}"
            );
        }
    }
}

#[test]
fn constant_time_code_computes_the_right_values() {
    let project = tempfile::tempdir().unwrap();
    write_package(project.path());
    build(project.path(), &[]);
    let library = project.path().join("target/mod.codiralib");
    // Safety: the library was compiled by this test.
    let runtime = unsafe { Runtime::builder(&library).finish() }.unwrap();
    for (a, b) in [(3u64, 9u64), (9, 3), (7, 7), (0, u64::MAX)] {
        let max: u64 = runtime.invoke("max_of", (a, b)).unwrap();
        assert_eq!(max, a.max(b));
        let equal: bool = runtime.invoke("equal", (a, b)).unwrap();
        assert_eq!(equal, a == b);
    }
}

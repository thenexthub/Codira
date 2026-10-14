//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - End-to-end tests of `codira synth`: holes filled by the built-in CEGIS
//!   search and by an external oracle whose wrong answers are rejected by the
//!   verifier, through the real `codira` binary.

use std::{path::Path, process::Command};

const SOURCE: &str = r#"
func inc(x: i64) -> i64
    requires x < 1000
    ensures result == x + 1
{
    ??
}
"#;

fn write_package(dir: &Path, source: &str) {
    std::fs::write(
        dir.join("codira.toml"),
        "[package]\nname=\"synth\"\nauthors=[]\nversion=\"0.1.0\"\n",
    )
    .unwrap();
    std::fs::create_dir_all(dir.join("src")).unwrap();
    std::fs::write(dir.join("src/mod.code"), source).unwrap();
}

fn codira(dir: &Path, args: &[&str]) -> (bool, String) {
    let output = Command::new(env!("CARGO_BIN_EXE_codira"))
        .args(args)
        .arg("--manifest-path")
        .arg(dir.join("codira.toml"))
        .output()
        .unwrap();
    let text = format!(
        "{}\n--- stderr ---\n{}",
        String::from_utf8_lossy(&output.stdout),
        String::from_utf8_lossy(&output.stderr)
    );
    (output.status.success(), text)
}

#[test]
fn search_fills_a_hole_with_a_proven_expression() {
    let project = tempfile::tempdir().unwrap();
    write_package(project.path(), SOURCE);

    let (ok, out) = codira(project.path(), &["check"]);
    assert!(!ok);
    assert!(out.contains("unfilled synthesis hole `??`"), "{out}");

    let (ok, out) = codira(project.path(), &["synth"]);
    assert!(ok, "{out}");
    assert!(out.contains("filled a hole in `inc` with (x + 1)"), "{out}");
    let source = std::fs::read_to_string(project.path().join("src/mod.code")).unwrap();
    assert!(source.contains("    (x + 1)\n"), "{source}");

    // The rewritten program verifies on its own.
    let (ok, out) = codira(project.path(), &["check"]);
    assert!(ok, "{out}");
}

#[test]
fn an_oracle_answer_is_used_only_once_the_verifier_proves_it() {
    let project = tempfile::tempdir().unwrap();
    write_package(project.path(), SOURCE);

    // A stand-in for a language-model CLI: the first answer is wrong (`x`
    // does not satisfy `result == x + 1`), the second is right. It reads
    // the prompt so the compiler's write to its stdin completes.
    let oracle_dir = tempfile::tempdir().unwrap();
    let script = if cfg!(windows) {
        let path = oracle_dir.path().join("oracle.cmd");
        std::fs::write(
            &path,
            "@echo off\r\nmore > nul\r\nif exist \"%~dp0tried\" (echo x + 1) else (type nul > \"%~dp0tried\" & echo x)\r\n",
        )
        .unwrap();
        path
    } else {
        let path = oracle_dir.path().join("oracle.sh");
        std::fs::write(
            &path,
            "#!/bin/sh\ncat > /dev/null\nif [ -e \"$(dirname \"$0\")/tried\" ]; then echo 'x + 1'; else touch \"$(dirname \"$0\")/tried\"; echo x; fi\n",
        )
        .unwrap();
        path
    };
    let command = if cfg!(windows) {
        script.display().to_string()
    } else {
        format!("sh {}", script.display())
    };

    let (ok, out) = codira(project.path(), &["synth", "--oracle", &command]);
    assert!(ok, "{out}");
    assert!(
        out.contains("oracle candidate 1 for `inc` rejected: postcondition might not hold"),
        "{out}"
    );
    assert!(
        out.contains("oracle candidate 2 for `inc` is proven: x + 1"),
        "{out}"
    );
    let source = std::fs::read_to_string(project.path().join("src/mod.code")).unwrap();
    assert!(source.contains("    (x + 1)\n"), "{source}");
}

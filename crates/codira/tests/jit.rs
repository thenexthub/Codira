//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Runs `examples/hello` with `codira run --jit` and checks that it ran on
//!   the Cranelift backend, not by falling back to LLVM, and printed what the
//!   LLVM backend prints for it.

use std::{path::Path, process::Command};

#[test]
fn hello_runs_on_the_cranelift_backend() {
    let manifest = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../examples/hello/codira.toml");
    let output = Command::new(env!("CARGO_BIN_EXE_codira"))
        .args(["run", "--jit", "--manifest-path"])
        .arg(&manifest)
        .env("CODIRA_JIT_TIMINGS", "1")
        .output()
        .unwrap();
    let stdout = String::from_utf8_lossy(&output.stdout);
    let stderr = String::from_utf8_lossy(&output.stderr);
    assert!(
        output.status.success(),
        "stdout:\n{stdout}\nstderr:\n{stderr}"
    );
    assert!(
        stderr.contains("cranelift: compiled in"),
        "fell back to LLVM:\n{stderr}"
    );
    assert_eq!(
        stdout.lines().collect::<Vec<_>>(),
        vec![
            "Hello, world!",
            "1 squared is 1",
            "2 squared is 4",
            "3 squared is 9",
            "pi is roughly 3.14159, and that is true",
        ]
    );
}

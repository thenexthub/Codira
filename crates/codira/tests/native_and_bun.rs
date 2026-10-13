//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - End-to-end tests through the `codira` CLI for native sources and for
//!   importing a module into Bun with `codira-bun`.

use std::{ffi::OsString, path::Path, process::Command};

use codira::run_with_args;

fn write(root: &Path, files: &[(&str, &str)]) {
    for (path, contents) in files {
        let path = root.join(path);
        std::fs::create_dir_all(path.parent().unwrap()).unwrap();
        std::fs::write(path, contents).unwrap();
    }
}

fn build(project: &Path, extra: &[&str]) {
    let args: Vec<OsString> = ["codira", "build", "--manifest-path"]
        .into_iter()
        .map(OsString::from)
        .chain(std::iter::once(
            project.join("codira.toml").into_os_string(),
        ))
        .chain(extra.iter().map(OsString::from))
        .collect();
    assert_eq!(run_with_args(args).unwrap(), codira::ExitStatus::Success);
}

/// C linked in as bitcode before optimization is inlined into its Codira
/// caller: at `-O2` the optimized IR of `scaled` has no call to `c_scale`
/// left, only the arithmetic.
#[test]
fn c_is_inlined_into_codira_callers() {
    let project = tempfile::tempdir().unwrap();
    write(
        project.path(),
        &[
            (
                "codira.toml",
                "[package]\nname = \"inline\"\nversion = \"0.1.0\"\n\n[native]\nsources = [\"c/scale.c\"]\n",
            ),
            (
                "c/scale.c",
                "#include <stdint.h>\nint64_t c_scale(int64_t x) { return x * 3 + 1; }\n",
            ),
            (
                "src/mod.code",
                "extern \"C\" {\n    func c_scale(x: i64) -> i64;\n}\n\npublic func scaled(x: i64) -> i64 { c_scale(x) }\n",
            ),
        ],
    );
    build(project.path(), &["--emit-ir", "-O", "2"]);

    let ir = std::fs::read_to_string(project.path().join("target/mod.ll")).unwrap();
    let scaled = ir
        .split("\ndefine ")
        .find(|function| function.contains("@scaled("))
        .unwrap_or_else(|| panic!("no definition of `scaled` in:\n{ir}"));
    let body = &scaled[..scaled.find("\n}").unwrap_or(scaled.len())];
    assert!(
        ir.contains("define") && ir.contains("@c_scale("),
        "the C function was not linked into the module:\n{ir}"
    );
    assert!(
        !body
            .lines()
            .any(|line| line.contains("call") && line.contains("@c_scale(")),
        "`c_scale` was not inlined into `scaled`:\n{body}"
    );
}

/// Imports a `.code` file into Bun through `bindings/bun` and calls it.
/// Skipped where Bun is not installed.
#[test]
fn bun_imports_a_codira_module() {
    let Ok(version) = Command::new("bun").arg("--version").output() else {
        eprintln!("skipped: `bun` is not on PATH");
        return;
    };
    assert!(version.status.success());

    let project = tempfile::tempdir().unwrap();
    let plugin = Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../../bindings/bun/plugin.ts")
        .canonicalize()
        .unwrap();
    // `canonicalize` gives Windows' verbatim `\\?\C:\...` form, which Bun
    // cannot open as a module specifier.
    let plugin = plugin.display().to_string();
    let plugin = plugin
        .strip_prefix(r"\\?\")
        .unwrap_or(&plugin)
        .replace('\\', "/");
    write(
        project.path(),
        &[
            (
                "native/codira.toml",
                "[package]\nname = \"bun_math\"\nversion = \"0.1.0\"\n",
            ),
            (
                "native/src/mod.code",
                "@export(\"C\")\npublic func add(a: i32, b: i32) -> i32 { a + b }\n\n\
                 @export(\"C\")\npublic func hypot_sq(x: f64, y: f64) -> f64 { x * x + y * y }\n",
            ),
            (
                "main.ts",
                &format!(
                    "import {{ plugin }} from \"bun\";\n\
                     import {{ codira }} from \"{plugin}\";\n\
                     plugin(codira());\n\
                     const math = await import(\"./native/src/mod.code\");\n\
                     console.log(JSON.stringify([math.add(40, 2), math.hypot_sq(3, 4)]));\n"
                ),
            ),
        ],
    );

    let output = Command::new("bun")
        .arg("run")
        .arg(project.path().join("main.ts"))
        .env("CODIRA", env!("CARGO_BIN_EXE_codira"))
        .current_dir(project.path())
        .output()
        .unwrap();
    let stdout = String::from_utf8_lossy(&output.stdout);
    assert!(
        output.status.success(),
        "bun failed:\n{stdout}\n{}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert_eq!(stdout.trim(), "[42,25]");
}

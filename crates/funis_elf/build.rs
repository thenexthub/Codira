//! Builds the two small C parts of funis.
//!
//! `funis-wrapper.so` is the preload library behind `funis -run`. It
//! interposes the exec family, including the C-variadic `execl` functions
//! that stable Rust cannot define, so it is a C file. The `funis`
//! executable embeds it.
//!
//! `lto-message.c` adapts the LTO plugin's printf-like diagnostics
//! callback, which is C-variadic as well, and is linked into funis.

use std::path::PathBuf;
use std::process::Command;

fn main() {
    println!("cargo:rerun-if-changed=c/funis-wrapper.c");
    println!("cargo:rerun-if-changed=c/lto-message.c");

    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap();
    let target_env = std::env::var("CARGO_CFG_TARGET_ENV").unwrap();
    if target_os == "windows" && target_env == "msvc" {
        return;
    }

    let out_dir = PathBuf::from(std::env::var("OUT_DIR").unwrap());
    let mut build = cc::Build::new();
    build.opt_level(2).pic(true);
    if let Ok(sanitizer) = std::env::var("CARGO_CFG_SANITIZE") {
        build.flag(format!("-fsanitize={sanitizer}"));
    }
    build.file("c/lto-message.c").compile("ltomessage");

    if !matches!(target_os.as_str(), "linux" | "android" | "freebsd") {
        return;
    }

    // The library is loaded into the processes that `funis -run` starts,
    // which are not built with sanitizers, so we leave them out even if
    // CFLAGS asks for them.
    let compiler = cc::Build::new().opt_level(2).pic(true).get_compiler();
    let mut command = Command::new(compiler.path());
    command.args(
        compiler
            .args()
            .iter()
            .filter(|arg| !arg.to_str().is_some_and(|arg| arg.starts_with("-fsanitize"))),
    );

    // The executable embeds the library as plain bytes, which strip and
    // debuginfo extraction tools cannot see, so we omit debug info even if
    // CFLAGS asks for it. -g0 overrides any -g option that comes before it.
    command.arg("-g0");

    let wrapper = out_dir.join("funis-wrapper.so");
    command.args(["-shared", "-o"]);
    command.arg(&wrapper).arg("c/funis-wrapper.c");
    if target_os == "android" || target_os == "linux" {
        command.arg("-ldl");
    }
    let status = command.status();
    if !matches!(status, Ok(s) if s.success()) {
        println!("cargo:warning=could not build funis-wrapper.so; `funis -run` will not work");
        // The executable embeds the file, and `-run` fails if it is empty.
        std::fs::write(&wrapper, b"").unwrap();
    }
}

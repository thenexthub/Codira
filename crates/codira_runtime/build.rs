//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Compiles the runtime's ARM MTE primitives (`native/memtag_aarch64.c`) when
//!   the runtime is built for aarch64 Linux, the only target with an MTE
//!   interface. Every other target builds nothing here; `src/memtag.rs` has
//!   stand-ins for them.

fn main() {
    println!("cargo:rerun-if-changed=native/memtag_aarch64.c");
    let arch = std::env::var("CARGO_CFG_TARGET_ARCH").unwrap_or_default();
    let os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    if arch == "aarch64" && os == "linux" {
        cc::Build::new()
            .file("native/memtag_aarch64.c")
            .flag_if_supported("-ffreestanding")
            .compile("codira_memtag");
    }
}

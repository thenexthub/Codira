//! Funis, a linker forked from mold (https://github.com/rui314/mold, MIT
//! licensed, by Rui Ueyama). See NOTICE for what has changed from mold.
//!
//! This crate holds what the `funis` executable and in-process callers share:
//! the table of targets the linker is built for. Each target is instantiated
//! in a crate of its own so that the compiler can build them in parallel, and
//! a Cargo feature per target decides which of them are built in.
// Vendored from mold (see NOTICE) and kept close to upstream, so that mold's changes keep
// applying. The workspace turns on extra lints (.cargo/config.toml) that mold's code does not
// follow, thousands of times over. They are allowed here by name, and nothing else is.
#![allow(
    elided_lifetimes_in_paths,
    clippy::bool_to_int_with_if,
    clippy::cast_lossless,
    clippy::checked_conversions,
    clippy::debug_assert_with_mut_call,
    clippy::default_trait_access,
    clippy::doc_markdown,
    clippy::enum_glob_use,
    clippy::if_not_else,
    clippy::items_after_statements,
    clippy::manual_assert,
    clippy::map_err_ignore,
    clippy::match_same_arms,
    clippy::match_wildcard_for_single_variants,
    clippy::mem_forget,
    clippy::mut_mut,
    clippy::redundant_closure_for_method_calls,
    clippy::rest_pat_in_fully_bound_structs,
    clippy::semicolon_if_nothing_returned,
    clippy::single_match_else,
    clippy::unnested_or_patterns,
    clippy::unused_self,
    clippy::wildcard_imports
)]

use std::ffi::OsStr;
use std::path::Path;

use funis_elf::driver::Cmdline;

/// Links ELF inputs for one target. Returns the target the inputs are
/// actually for if it is not this one.
pub type ElfLinkFn = fn(Cmdline) -> Option<&'static str>;

/// Links Mach-O inputs for one target.
pub type MachoLinkFn = fn(funis_macho::driver::ParsedArgs);

/// Each ELF target has its own monomorphized link function. The ELF linker
/// starts with the first enabled target and switches to the matching
/// function if the inputs are for another target.
pub const ELF_TARGETS: &[(&str, ElfLinkFn)] = &[
    #[cfg(feature = "x86_64")]
    ("x86_64", funis_elf_x86_64::link),
    #[cfg(feature = "arm64")]
    ("arm64", funis_elf_arm64::link),
    #[cfg(feature = "riscv64")]
    ("riscv64", funis_elf_riscv64::link),
];

/// The Mach-O linker finds the target as it parses the options, and calls
/// the function of that target.
pub const MACHO_TARGETS: &[(&str, MachoLinkFn)] = &[
    #[cfg(feature = "macho-arm64")]
    ("arm64", funis_macho_arm64::link),
    #[cfg(feature = "macho-x86_64")]
    ("x86_64", funis_macho_x86_64::link),
];

/// Looks `target` up in `targets`.
pub fn find_target<F: Copy>(targets: &[(&str, F)], target: &str) -> Option<F> {
    targets.iter().find(|(name, _)| *name == target).map(|&(_, link)| link)
}

/// As with lld's ld64.lld, the name tells funis to act as Apple's linker,
/// ld64. clang on macOS looks for ld64.funis when given -fuse-ld=funis.
pub fn is_ld64(argv0: &OsStr) -> bool {
    let name = Path::new(argv0).file_name().unwrap_or_default().as_encoded_bytes();
    name == b"ld64" || name.starts_with(b"ld64.")
}

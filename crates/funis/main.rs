//! The `funis` executable: runs the ELF linker, or the Mach-O linker if
//! invoked as `ld64.funis`, instantiated for the target the inputs are for.
//! Each target is instantiated in a crate of its own so that the compiler
//! can build them in parallel, and a feature per target decides which of
//! them are built in.
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
// rustc emits asynchronous unwind tables, which compact unwind can't
// encode, so the unwind info of most functions goes to __eh_frame. In a
// debug build, that section exceeds the 16 MiB that compact unwind can
// refer to, and Apple's ld warns about that. The warning concerns only
// the speed of unwinding, so we keep rustc from reporting it.
#![cfg_attr(all(target_vendor = "apple", debug_assertions), allow(linker_messages))]

use funis::{ELF_TARGETS, MACHO_TARGETS, find_target, is_ld64};

// A Rust executable can define only one global allocator, so select mimalloc
// here rather than in the linker library.
#[cfg(feature = "mimalloc")]
#[global_allocator]
static GLOBAL: mimalloc::MiMalloc = mimalloc::MiMalloc;

fn link_fn<F: Copy>(targets: &[(&str, F)], target: &str) -> F {
    find_target(targets, target).unwrap_or_else(|| {
        eprintln!(
            "funis: unsupported target: {target}; rebuild funis with the appropriate target support"
        );
        std::process::exit(1);
    })
}

fn main() {
    let args: Vec<_> = std::env::args_os().collect();
    if args.first().is_some_and(|argv0| is_ld64(argv0)) {
        let link = |target: &str, parsed| link_fn(MACHO_TARGETS, target)(parsed);
        funis_macho::driver::main(args, link);
        std::process::exit(0);
    }

    let Some(&(initial_target, _)) = ELF_TARGETS.first() else {
        eprintln!("funis: no targets enabled; rebuild funis with the appropriate target support");
        std::process::exit(1);
    };
    let link = |target: &str, cmdline| link_fn(ELF_TARGETS, target)(cmdline);
    funis_elf::driver::main(args, initial_target, link);
    std::process::exit(0);
}

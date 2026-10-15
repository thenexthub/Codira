//! A high-performance Mach-O linker.
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

pub mod arch;
mod arm64;
pub(crate) mod bundle_hook;
pub(crate) mod chunks;
pub(crate) mod cmdline;
pub(crate) mod context;
pub(crate) mod dead_strip;
pub(crate) mod delay_init;
pub mod driver;
pub(crate) mod dtrace;
pub(crate) mod dwarf;
pub(crate) mod filetype;
pub(crate) mod icf;
pub(crate) mod input_files;
pub(crate) mod input_sections;
pub(crate) mod lazy_load;
pub(crate) mod lto;
pub mod macho;
mod macho_consts;
pub(crate) mod make_mergeable;
pub(crate) mod mapfile;
pub(crate) mod mapped_file;
pub(crate) mod mergeable;
pub(crate) mod objc;
pub(crate) mod output_file;
pub(crate) mod passes;
pub(crate) mod reader;
pub(crate) mod relocatable;
pub(crate) mod symbol;
pub(crate) mod symbol_moves;
pub(crate) mod tapi;
pub(crate) mod thunks;
pub(crate) mod tls;
mod x86_64;

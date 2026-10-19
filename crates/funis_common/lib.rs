//! Code that funis's linkers share. Nothing here depends on the format of
//! object files.
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

pub mod archive_file;
pub mod bits;
pub mod bytes;
pub mod cityhash;
pub mod compress;
pub mod concurrent_map;
pub mod demangle;
pub mod endian;
pub mod error;
pub mod glob;
pub mod hyperloglog;
pub mod jobs;
pub mod leb128;
pub mod mapped_file;
pub mod mem;
pub mod output_file;
pub mod parallel;
pub mod path;
pub mod perf;
mod prefetch;
pub mod record;
pub mod response_file;
pub mod session;
pub mod siphash;
pub mod subprocess;
pub mod tar;
pub mod worker_local;

pub use prefetch::prefetch;

/// The git commit that funis is built from, or None if the source tree has
/// no git metadata, as in a source tarball.
pub const GIT_HASH: Option<&str> = option_env!("FUNIS_GIT_HASH");

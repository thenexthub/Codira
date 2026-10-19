//! Funis, a linker forked from mold (https://github.com/rui314/mold, MIT
//! licensed, by Rui Ueyama). See NOTICE for what has changed from mold.
//!
//! This crate is the `funis` command line's library and the way to link from
//! inside another program. mold is a one-shot process; [`link`] is the same
//! linker as a function that can be called any number of times:
//!
//! * it never exits the process: `--version` and `--help` come back as an
//!   [`Outcome::Informational`], and the conditions that make the command line
//!   exit come back as a [`LinkError`];
//! * diagnostics are data ([`Diagnostic`]), not text on stderr, unless
//!   [`Options::echo_diagnostics`] asks for both;
//! * each link uses a thread pool of its own, sized to the input, and nothing
//!   of it survives the call: no threads, no mapped files, no handles, no
//!   leaked allocations (a test links a thousand times and checks);
//! * it does not fork, install signal handlers or change the working
//!   directory, and options that would ask it to (`-run`, `-C`) are refused as
//!   [`LinkError::Unsupported`], as is link-time optimisation.
//!
//! The linker keeps process-wide state, so links are serialised: two threads
//! that link at once take turns.
//!
//! The crate also holds the table of targets that the executable is built for.
//! Each target is instantiated in a crate of its own so that the compiler can
//! build them in parallel, and a Cargo feature per target decides which of
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

use std::ffi::{OsStr, OsString};
use std::fmt;
use std::path::{Path, PathBuf};
use std::time::{Duration, Instant};

use funis_common::session::{self, Ended, Session};
use funis_elf::driver::Cmdline;

pub use funis_common::session::{Diagnostic, Severity};

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

/// The object-file format a link produces.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Format {
    Elf,
    MachO,
}

/// How a link is run.
#[derive(Clone, Debug, Default)]
pub struct Options {
    /// Also write each diagnostic to stderr as it happens, as the command
    /// line does. They are collected either way.
    pub echo_diagnostics: bool,
}

/// What a link that did not fail did.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Outcome {
    /// The output was written.
    Linked,
    /// The command line asked for information (`--version`, `--help`) and got
    /// it, in [`LinkReport::stdout`]. No output was written.
    Informational,
}

/// What a link did.
#[derive(Clone, Debug)]
pub struct LinkReport {
    pub outcome: Outcome,
    pub format: Format,
    /// The linker's target, such as "x86_64" or "arm64", when a link ran.
    pub target: Option<&'static str>,
    /// The output file as the command line named it, when a link ran.
    pub output: Option<PathBuf>,
    /// How many threads the link used.
    pub threads: usize,
    /// The warnings and notices the linker reported.
    pub diagnostics: Vec<Diagnostic>,
    /// What the command line would have printed to stdout.
    pub stdout: String,
    /// How long the call took, including starting and stopping its threads.
    pub elapsed: Duration,
}

/// Why a link did not produce an output.
#[derive(Clone, Debug)]
pub enum LinkError {
    /// The link failed: an input or an option is wrong. The diagnostics say
    /// what, as `funis` would have printed it.
    Failed { diagnostics: Vec<Diagnostic> },
    /// Funis does not implement something the command line asks for: an
    /// option, a target, link-time optimisation. Another linker may. Nothing
    /// is wrong with the inputs.
    Unsupported { reason: String, diagnostics: Vec<Diagnostic> },
    /// The linker itself failed (a panic). A bug in Funis, not in the inputs.
    Internal { message: String, diagnostics: Vec<Diagnostic> },
}

impl LinkError {
    /// The diagnostics reported before the link ended.
    pub fn diagnostics(&self) -> &[Diagnostic] {
        match self {
            LinkError::Failed { diagnostics }
            | LinkError::Unsupported { diagnostics, .. }
            | LinkError::Internal { diagnostics, .. } => diagnostics,
        }
    }

    /// Whether Funis does not implement what was asked, as opposed to the
    /// link being wrong.
    pub fn is_unsupported(&self) -> bool {
        matches!(self, LinkError::Unsupported { .. })
    }
}

impl fmt::Display for LinkError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let (what, diagnostics) = match self {
            LinkError::Failed { diagnostics } => ("link failed".to_owned(), diagnostics),
            LinkError::Unsupported { reason, diagnostics } => {
                (format!("unsupported: {reason}"), diagnostics)
            }
            LinkError::Internal { message, diagnostics } => {
                (format!("internal error: {message}"), diagnostics)
            }
        };
        f.write_str(&what)?;
        for d in diagnostics.iter().filter(|d| d.severity != Severity::Notice) {
            // The reason of an unsupported link is also its last diagnostic.
            if matches!(self, LinkError::Unsupported { reason, .. } if *reason == d.message) {
                continue;
            }
            write!(f, "\n{d}")?;
        }
        Ok(())
    }
}

impl std::error::Error for LinkError {}

/// Links as the command line `args` say (`args[0]` is the program name, as in
/// `argv`): an ELF link, or a Mach-O one if the name is `ld64` or `ld64.*`.
pub fn link(args: Vec<OsString>) -> Result<LinkReport, LinkError> {
    let format = match args.first() {
        Some(argv0) if is_ld64(argv0) => Format::MachO,
        _ => Format::Elf,
    };
    link_with(format, args, &Options::default())
}

/// Links ELF inputs. `args[0]` is the program name and is not an input.
pub fn link_elf(args: Vec<OsString>) -> Result<LinkReport, LinkError> {
    link_with(Format::Elf, args, &Options::default())
}

/// Links Mach-O inputs. `args[0]` is the program name and is not an input.
pub fn link_macho(args: Vec<OsString>) -> Result<LinkReport, LinkError> {
    link_with(Format::MachO, args, &Options::default())
}

/// Links in the given format with the given options. See the crate
/// documentation for what that guarantees.
pub fn link_with(
    format: Format,
    args: Vec<OsString>,
    options: &Options,
) -> Result<LinkReport, LinkError> {
    let start = Instant::now();
    if cfg!(panic = "abort") {
        return Err(LinkError::Unsupported {
            reason: "Funis ends a link by unwinding, and this build aborts on panic".to_owned(),
            diagnostics: Vec::new(),
        });
    }

    let session = Session::begin(options.echo_diagnostics);
    let ended = session::catch_termination(|| match format {
        Format::Elf => run_elf(args),
        Format::MachO => run_macho(args),
    });
    let captured = session.finish();
    let session::Captured { mut diagnostics, stdout, unsupported, info } = captured;

    match ended {
        Ok(()) | Err(Ended::Exited(0)) => {
            let outcome =
                if info.output.is_some() { Outcome::Linked } else { Outcome::Informational };
            diagnostics.retain(|d| d.severity != Severity::Fatal);
            Ok(LinkReport {
                outcome,
                format,
                target: info.target,
                output: info.output,
                threads: info.threads,
                diagnostics,
                stdout,
                elapsed: start.elapsed(),
            })
        }
        Err(Ended::Exited(_)) => Err(match unsupported {
            Some(reason) => LinkError::Unsupported { reason, diagnostics },
            None => LinkError::Failed { diagnostics },
        }),
        Err(Ended::Panicked(message)) => Err(LinkError::Internal { message, diagnostics }),
    }
}

fn run_elf(args: Vec<OsString>) {
    let Some(&(initial_target, _)) = ELF_TARGETS.first() else {
        funis_common::unsupported!("no targets enabled; rebuild funis with target support");
    };
    let link = |target: &str, cmdline| match find_target(ELF_TARGETS, target) {
        Some(link) => link(cmdline),
        None => funis_common::unsupported!(
            "unsupported target: {target}; rebuild funis with the appropriate target support"
        ),
    };
    funis_elf::driver::main(args, initial_target, link);
}

fn run_macho(args: Vec<OsString>) {
    let link = |target: &str, parsed| match find_target(MACHO_TARGETS, target) {
        Some(link) => link(parsed),
        None => funis_common::unsupported!(
            "unsupported target: {target}; rebuild funis with the appropriate target support"
        ),
    };
    funis_macho::driver::main(args, link);
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Decides when a `.codiralib` is written by `codira_image` instead of LLD,
//!   runs it, and keeps count of what happened: how many images were written
//!   directly, how many were cross-checked against LLD, and why the writer
//!   declined the rest (those are linked by LLD, exactly as before).
//! - The cross-check (`CODIRA_LINK_VERIFY`, on by default in builds with debug
//!   assertions) links the same object with LLD as well and requires the two
//!   images to be identical but for the header time stamp. Every program the
//!   compiler's own test suites build is therefore a differential test of the
//!   image writer.
use std::{collections::BTreeMap, env, path::Path};

use codira_abi as abi;
use codira_image::{elfinfo, peinfo, DllRequest, Outcome, SoRequest};
use codira_target::spec::{self, LinkerFlavor};
use parking_lot::Mutex;

use crate::linker::direct_image_requested;

/// Name of the environment variable that turns the LLD cross-check on
/// (`1`) or off (`0`).
pub const LINK_VERIFY_ENV: &str = "CODIRA_LINK_VERIFY";

/// Name of the environment variable naming a file that every link outcome is
/// appended to, one line each: `verified`, or `declined` with the reason. It
/// lets a test run over many programs say afterwards what happened to each.
pub const LINK_LOG_ENV: &str = "CODIRA_LINK_LOG";

fn log_outcome(line: &str) {
    use std::io::Write;

    if let Some(path) = env::var_os(LINK_LOG_ENV) {
        if let Ok(mut file) = std::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(path)
        {
            let _ = writeln!(file, "{line}");
        }
    }
}

/// Name of the environment variable naming a directory that every object
/// file handed to the linker is copied to (with the list of symbols it must
/// export beside it). It is how the fixtures and the benchmark inputs of
/// `codira_image` are collected from real programs.
pub const DUMP_OBJECTS_ENV: &str = "CODIRA_DUMP_OBJECTS";

/// Copies `object` into the directory `CODIRA_DUMP_OBJECTS` names, if set.
pub(crate) fn dump_object(object: &[u8], exported_symbols: &[String]) {
    let Some(dir) = env::var_os(DUMP_OBJECTS_ENV) else {
        return;
    };
    let mut hash: u64 = 0xcbf2_9ce4_8422_2325;
    for &byte in object {
        hash = (hash ^ u64::from(byte)).wrapping_mul(0x0000_0100_0000_01b3);
    }
    let stem = Path::new(&dir).join(format!("{hash:016x}"));
    let _ = std::fs::create_dir_all(&dir);
    let _ = std::fs::write(stem.with_extension("obj"), object);
    let _ = std::fs::write(
        stem.with_extension("exports"),
        exported_symbols.join(
            "
",
        ),
    );
}

/// What the direct image writer has done in this process.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct ImageLinkStats {
    /// Images written by `codira_image` and used.
    pub written: u64,
    /// Of those, images also linked by LLD and found identical.
    pub verified: u64,
    /// Objects the writer declined, by reason; LLD linked them.
    pub declined: BTreeMap<&'static str, u64>,
    /// The most recent refusal in full, for diagnostics.
    pub last_decline: Option<String>,
}

static STATS: Mutex<Option<ImageLinkStats>> = Mutex::new(None);

fn with_stats(update: impl FnOnce(&mut ImageLinkStats)) {
    update(STATS.lock().get_or_insert_with(ImageLinkStats::default));
}

/// A snapshot of the counters.
pub fn image_link_stats() -> ImageLinkStats {
    STATS.lock().clone().unwrap_or_default()
}

/// Sets the counters back to zero.
pub fn reset_image_link_stats() {
    *STATS.lock() = None;
}

/// Set to `1` to let the image writer also write x86-64 Linux shared
/// objects. That path is newer than the Windows one: its images are checked
/// by `llvm-readobj` and loaded by a real dynamic loader in the tests, but it
/// has not run under the Codira runtime on Linux, so it is opt-in.
pub const DIRECT_ELF_ENV: &str = "CODIRA_DIRECT_ELF";

/// The kinds of image `codira_image` writes.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) enum ImageFormat {
    /// A PE32+ DLL for `x86_64-pc-windows-msvc`.
    Pe,
    /// An ELF shared object for `x86_64-*-linux-*`.
    Elf,
}

/// The format `codira_image` writes for `target`, if it writes one.
pub(crate) fn image_format(target: &spec::Target) -> Option<ImageFormat> {
    if target.arch != "x86_64" {
        return None;
    }
    let options = &target.options;
    if options.is_like_windows
        && options.is_like_msvc
        && options.linker_flavor == LinkerFlavor::Msvc
    {
        return Some(ImageFormat::Pe);
    }
    let elf_requested = matches!(
        env::var(DIRECT_ELF_ENV).as_deref().map(str::trim),
        Ok("1" | "true" | "on")
    );
    (elf_requested && options.os == "linux" && options.linker_flavor == LinkerFlavor::Ld)
        .then_some(ImageFormat::Elf)
}

/// Whether the cross-check against LLD is on.
pub fn verification_enabled() -> bool {
    match env::var(LINK_VERIFY_ENV).as_deref().map(str::trim) {
        Ok("1" | "true" | "on") => true,
        Ok("0" | "false" | "off") => false,
        _ => cfg!(debug_assertions),
    }
}

/// The symbols the image must export: the three the runtime looks for, and
/// every `@export("C")` function.
fn export_list(exported_symbols: &[String]) -> Vec<&str> {
    let mut exports = vec![
        abi::GET_INFO_FN_NAME,
        abi::GET_VERSION_FN_NAME,
        abi::SET_ALLOCATOR_HANDLE_FN_NAME,
    ];
    exports.extend(exported_symbols.iter().map(String::as_str));
    exports.sort_unstable();
    exports.dedup();
    exports
}

/// Writes the image for `object` directly, or returns `None` when the
/// caller should link with LLD: the mode or target does not call for it, or
/// the writer declined (counted, with the reason).
pub(crate) fn link_directly(
    target: &spec::Target,
    object: &[u8],
    output_path: &Path,
    exported_symbols: &[String],
    has_native_code: bool,
) -> Option<Vec<u8>> {
    if !direct_image_requested() {
        return None;
    }
    let outcome = match image_format(target)? {
        ImageFormat::Pe => {
            let dll_name = output_path
                .file_name()
                .and_then(|name| name.to_str())
                .unwrap_or("");
            let exports = export_list(exported_symbols);
            codira_image::link_windows_dll(&DllRequest {
                object,
                exports: &exports,
                dll_name,
                debug_info: false,
                has_native_code,
            })
        }
        // Every global symbol is exported, as `ld.lld --shared` does.
        ImageFormat::Elf => codira_image::link_linux_so(&SoRequest {
            object,
            debug_info: false,
            has_native_code,
        }),
    };
    match outcome {
        Outcome::Image(image) => {
            with_stats(|stats| stats.written += 1);
            log_outcome(&format!("written	{}", image.len()));
            Some(image)
        }
        Outcome::Declined(decline) => {
            log_outcome(&format!("declined	{decline}"));
            with_stats(|stats| {
                *stats.declined.entry(decline.kind.name()).or_default() += 1;
                stats.last_decline = Some(decline.to_string());
            });
            None
        }
    }
}

/// Requires `ours` to be the image LLD wrote (`lld`): byte for byte for a
/// PE image; for an ELF shared object, which LLD lays out differently, the
/// same exports and the same symbolic relocations, and a library the loader
/// accepts.
pub(crate) fn verify_against(
    ours: &[u8],
    lld: &[u8],
    output_path: &Path,
) -> Result<(), anyhow::Error> {
    if ours.starts_with(b"ELF") {
        let invalid =
            |what: &str, e: String| anyhow::anyhow!("{what} for {}: {e}", output_path.display());
        let a =
            elfinfo::validate(lld).map_err(|e| invalid("LLD's library does not validate", e))?;
        let b = elfinfo::validate(ours)
            .map_err(|e| invalid("codira_image wrote a library that does not validate", e))?;
        elfinfo::compare(&a, &b).map_err(|report| {
            anyhow::anyhow!(
                "codira_image and LLD disagree about {} (set {}=0 to skip this check):
{report}",
                output_path.display(),
                LINK_VERIFY_ENV
            )
        })?;
    } else {
        peinfo::validate(ours).map_err(|e| {
            anyhow::anyhow!(
                "codira_image wrote an image that does not validate for {}: {e}",
                output_path.display()
            )
        })?;
        peinfo::compare(lld, ours).map_err(|report| {
            anyhow::anyhow!(
                "codira_image and LLD disagree about {} (set {}=0 to skip this check):
{report}",
                output_path.display(),
                LINK_VERIFY_ENV
            )
        })?;
    }
    with_stats(|stats| stats.verified += 1);
    log_outcome(&format!("verified	{}", ours.len()));
    Ok(())
}

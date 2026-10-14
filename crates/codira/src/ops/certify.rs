//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira certify <artifact>`: checks the proof certificates `codira build
//!   --certify` wrote, using only `codira_certificate` and Z3 -- none of the
//!   parser, type checker, verifier or code generator runs.
//!
//! The artifact is a `.codiralib` (its certificate is the
//! `<library>.cert.json` beside it), a directory of them, or an executable
//! from `codira build --exe --certify`, whose payload carries both.

use std::path::{Path, PathBuf};

use anyhow::{bail, Context};
use codira_certificate::{Certificate, CheckOptions, Report, SIDECAR_SUFFIX};

use crate::ExitStatus;

#[derive(clap::Args)]
pub struct Args {
    /// A `.codiralib`, a directory of them, or an executable built with
    /// `codira build --exe --certify`
    artifact: PathBuf,

    /// The Z3 resource budget every query is re-decided under
    #[clap(long, default_value_t = default_rlimit())]
    rlimit: u32,

    /// The package's source directory (`src/`): also confirm each
    /// function's source text is the one the certificate was made from
    #[clap(long)]
    source_dir: Option<PathBuf>,

    /// Fail when the certificate records an obligation as not proven (by
    /// default those are reported but are not a failure: the compiler
    /// accepted them because a run-time check enforces them)
    #[clap(long)]
    deny_unproven: bool,
}

fn default_rlimit() -> u32 {
    codira_certificate::CheckOptions::default().rlimit
}

/// A library and the text of its certificate, if it has one.
struct Certified {
    name: String,
    library: Vec<u8>,
    certificate: Option<Vec<u8>>,
}

fn sidecar(library: &Path) -> anyhow::Result<Certified> {
    let certificate_path = codira_certificate::sidecar_path(library);
    Ok(Certified {
        name: library.display().to_string(),
        library: std::fs::read(library)
            .with_context(|| format!("could not read '{}'", library.display()))?,
        certificate: std::fs::read(&certificate_path).ok(),
    })
}

fn collect(artifact: &Path) -> anyhow::Result<Vec<Certified>> {
    if artifact.is_dir() {
        let mut libraries: Vec<PathBuf> = std::fs::read_dir(artifact)
            .with_context(|| format!("could not read '{}'", artifact.display()))?
            .filter_map(Result::ok)
            .map(|entry| entry.path())
            .filter(|path| path.extension().is_some_and(|e| e == "codiralib"))
            .collect();
        libraries.sort();
        if libraries.is_empty() {
            bail!("'{}' contains no .codiralib", artifact.display());
        }
        return libraries.iter().map(|l| sidecar(l)).collect();
    }
    if let Some(payload) = crate::ops::exe::read_payload(artifact)? {
        let mut found = Vec::new();
        for (name, bytes) in &payload.libraries {
            if name.ends_with(SIDECAR_SUFFIX) {
                continue;
            }
            let certificate_name = format!("{name}{SIDECAR_SUFFIX}");
            found.push(Certified {
                name: format!("{} (embedded {name})", artifact.display()),
                library: bytes.clone(),
                certificate: payload
                    .libraries
                    .iter()
                    .find(|(n, _)| *n == certificate_name)
                    .map(|(_, b)| b.clone()),
            });
        }
        return Ok(found);
    }
    Ok(vec![sidecar(artifact)?])
}

pub fn certify(args: Args) -> anyhow::Result<ExitStatus> {
    let options = CheckOptions {
        rlimit: args.rlimit,
        source_dir: args.source_dir.clone(),
    };
    let mut ok = true;
    for item in collect(&args.artifact)? {
        let Some(text) = &item.certificate else {
            println!(
                "{}: no certificate (build with `codira build --certify`)",
                item.name
            );
            ok = false;
            continue;
        };
        let certificate = match std::str::from_utf8(text)
            .map_err(|e| e.to_string())
            .and_then(Certificate::from_json)
        {
            Ok(certificate) => certificate,
            Err(error) => {
                println!("{}: unreadable certificate: {error}", item.name);
                ok = false;
                continue;
            }
        };
        let report: Report = codira_certificate::check(&certificate, &item.library, &options);
        println!(
            "{}
{report}",
            item.name
        );
        ok &= report.passed() && !(args.deny_unproven && !report.unproven.is_empty());
    }
    Ok(ok.into())
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - The program `codira build --exe` turns into a standalone executable by
//!   appending a package's libraries to it. See `codira::exe`.

use std::process::ExitCode;

fn main() -> ExitCode {
    match codira::exe::run_embedded() {
        Ok(codira::exe::Launched::Exited(code)) => {
            ExitCode::from(u8::try_from(code.rem_euclid(256)).unwrap_or(1))
        }
        Ok(codira::exe::Launched::NoPayload) => {
            eprintln!(
                "codira-launcher: this is the Codira runtime launcher, which `codira build \
                 --exe` turns into an executable; it has no program of its own to run"
            );
            ExitCode::from(2)
        }
        Err(error) => {
            eprintln!("error: {error:#}");
            ExitCode::FAILURE
        }
    }
}

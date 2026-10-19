//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The outcome of a link attempt, in which declining to link is a normal
//!   result and not an error: the caller falls back to the general linker.
use std::fmt;

/// Why the image writer refused an input.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum DeclineKind {
    /// The input is not an object file of a format this writer reads.
    UnsupportedFormat,
    /// The object is for a machine this writer does not target.
    UnsupportedMachine,
    /// The object file is damaged: a table or offset points outside it.
    Malformed,
    /// A section this writer does not model: TLS, CRT initializers,
    /// directives, imports, anything outside the five Codira produces.
    UnknownSection,
    /// Debug information is present or was requested.
    DebugInfo,
    /// A symbol is referenced but defined in no input: an import, a runtime
    /// function, or a library.
    UndefinedSymbol,
    /// A symbol kind this writer does not model: weak, common, alias.
    UnknownSymbol,
    /// A relocation type this writer does not model.
    UnknownRelocation,
    /// A COMDAT section with a selection rule that needs more than one input.
    Comdat,
    /// The package has native (C, C++ or assembly) objects.
    NativeCode,
    /// A requested export is not a symbol the object defines.
    MissingExport,
    /// A value does not fit its relocation, or the image would be too big.
    OutOfRange,
    /// Something else this writer does not model.
    Unmodelled,
}

impl DeclineKind {
    /// A short stable name, for statistics and log lines.
    pub fn name(self) -> &'static str {
        match self {
            DeclineKind::UnsupportedFormat => "unsupported-format",
            DeclineKind::UnsupportedMachine => "unsupported-machine",
            DeclineKind::Malformed => "malformed",
            DeclineKind::UnknownSection => "unknown-section",
            DeclineKind::DebugInfo => "debug-info",
            DeclineKind::UndefinedSymbol => "undefined-symbol",
            DeclineKind::UnknownSymbol => "unknown-symbol",
            DeclineKind::UnknownRelocation => "unknown-relocation",
            DeclineKind::Comdat => "comdat",
            DeclineKind::NativeCode => "native-code",
            DeclineKind::MissingExport => "missing-export",
            DeclineKind::OutOfRange => "out-of-range",
            DeclineKind::Unmodelled => "unmodelled",
        }
    }
}

/// A refusal: the kind, and a sentence saying what was met.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Decline {
    pub kind: DeclineKind,
    pub detail: String,
}

impl Decline {
    pub fn new(kind: DeclineKind, detail: impl Into<String>) -> Self {
        Self {
            kind,
            detail: detail.into(),
        }
    }
}

impl fmt::Display for Decline {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}: {}", self.kind.name(), self.detail)
    }
}

impl std::error::Error for Decline {}

/// The result of asking for an image.
#[derive(Debug)]
pub enum Outcome {
    /// The finished image, ready to be written to disk.
    Image(Vec<u8>),
    /// The writer does not handle this input; link it with LLD.
    Declined(Decline),
}

impl Outcome {
    /// The image, or the refusal.
    pub fn into_result(self) -> Result<Vec<u8>, Decline> {
        match self {
            Outcome::Image(bytes) => Ok(bytes),
            Outcome::Declined(decline) => Err(decline),
        }
    }
}

impl From<Result<Vec<u8>, Decline>> for Outcome {
    fn from(result: Result<Vec<u8>, Decline>) -> Self {
        match result {
            Ok(bytes) => Outcome::Image(bytes),
            Err(decline) => Outcome::Declined(decline),
        }
    }
}

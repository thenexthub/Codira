//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - A purpose-built writer for the shared-library images Codira produces
//!   (`.codiralib`), used instead of the general linker when the object is
//!   simple enough to be certain about. A `.codiralib` has a handful of
//!   sections, imports nothing, exports a known list of functions, and needs
//!   only absolute-address base relocations, because calls between Codira
//!   functions go through the dispatch table.
//! - Declining is a normal outcome (`Outcome::Declined`): anything outside the
//!   modelled set -- unknown sections or relocations, TLS, imports, debug
//!   information, native objects -- is handed back to the caller, who links
//!   with LLD as before. The crate does not depend on LLVM; its input is the
//!   neutral `model::Object`, with a COFF reader as the first front end.
pub mod coff;
pub mod decline;
pub mod elf;
pub mod elfinfo;
pub mod model;
pub mod pe;
pub mod peinfo;

pub use decline::{Decline, DeclineKind, Outcome};

/// What the caller wants linked.
#[derive(Clone, Copy, Debug)]
pub struct DllRequest<'a> {
    /// The object file (COFF, AMD64) as LLVM wrote it.
    pub object: &'a [u8],
    /// The functions the image exports, by symbol name.
    pub exports: &'a [&'a str],
    /// The file name of the output; recorded in the export directory.
    pub dll_name: &'a str,
    /// Debug information was requested: not modelled, always declined.
    pub debug_info: bool,
    /// The package has native (C, C++ or assembly) objects: not modelled,
    /// always declined.
    pub has_native_code: bool,
}

/// Links a Windows AMD64 object file into a DLL image, or declines.
pub fn link_windows_dll(request: &DllRequest<'_>) -> Outcome {
    if request.debug_info {
        return Outcome::Declined(Decline::new(
            DeclineKind::DebugInfo,
            "debug information was requested",
        ));
    }
    if request.has_native_code {
        return Outcome::Declined(Decline::new(
            DeclineKind::NativeCode,
            "the package has native objects",
        ));
    }
    let object = match coff::read_coff(request.object) {
        Ok(object) => object,
        Err(decline) => return Outcome::Declined(decline),
    };
    pe::write_pe_dll(
        &object,
        &pe::PeRequest {
            exports: request.exports,
            dll_name: request.dll_name,
        },
    )
    .into()
}

/// What the caller wants linked into a Linux shared object.
#[derive(Clone, Copy, Debug)]
pub struct SoRequest<'a> {
    /// The object file (ELF64, x86-64) as LLVM wrote it.
    pub object: &'a [u8],
    /// Debug information was requested: not modelled, always declined.
    pub debug_info: bool,
    /// The package has native (C, C++ or assembly) objects: not modelled,
    /// always declined.
    pub has_native_code: bool,
}

/// Links an x86-64 Linux object file into a shared object, or declines.
/// Every global symbol the object defines is exported, as `ld.lld --shared`
/// does.
pub fn link_linux_so(request: &SoRequest<'_>) -> Outcome {
    if request.debug_info {
        return Outcome::Declined(Decline::new(
            DeclineKind::DebugInfo,
            "debug information was requested",
        ));
    }
    if request.has_native_code {
        return Outcome::Declined(Decline::new(
            DeclineKind::NativeCode,
            "the package has native objects",
        ));
    }
    let object = match elf::read_elf(request.object) {
        Ok(object) => object,
        Err(decline) => return Outcome::Declined(decline),
    };
    elf::write_elf_so(&object, &elf::ElfRequest::default()).into()
}

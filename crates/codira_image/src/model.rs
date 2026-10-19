//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - The format-neutral description of a relocatable object that the image
//!   writers consume: sections, symbols and relocations. A front end (the COFF
//!   reader today; Cranelift's output or an ELF reader later) produces an
//!   `Object`, and a back end (the PE writer) turns it into an image. The model
//!   carries no reference to LLVM.

/// What a section holds. This decides where the writer puts it and with
/// which protection; sections that fit none of these are declined.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SectionClass {
    /// Executable code.
    Code,
    /// Read-only data.
    ReadOnlyData,
    /// Initialized writable data.
    Data,
    /// Zero-initialized writable data; occupies no space in the file.
    ZeroData,
    /// Unwind information (`.xdata`).
    UnwindInfo,
    /// Data that is written only while the library is being loaded and then
    /// made read-only (`.data.rel.ro`): ELF.
    RelroData,
    /// Call-frame information for the unwinder (`.eh_frame`): ELF.
    EhFrame,
    /// The function table that locates unwind information (`.pdata`).
    ExceptionTable,
}

/// How a relocation computes the value it stores.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum RelocKind {
    /// 64-bit absolute address: `ImageBase + S + A`. Needs a base relocation.
    Abs64,
    /// 32-bit `S + A - P`, with `P` the address of the field.
    PcRel32,
    /// 32-bit image-relative offset: `S + A`.
    Rva32,
    /// 32-bit offset of the target from the start of its section.
    SecRel32,
    /// 16-bit one-based number of the target's section.
    SectionIndex,
    /// 32-bit `G + GOT + A - P`: the address of the symbol's slot in the
    /// global offset table, relative to the field (ELF `GOTPCREL`).
    GotPcRel,
}

/// One relocation. The addend is explicit: a front end whose format keeps it
/// in the section contents (COFF) reads it out when it builds the object.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Reloc {
    /// Offset of the field inside its section.
    pub offset: u32,
    /// Index into `Object::symbols`.
    pub symbol: u32,
    pub kind: RelocKind,
    pub addend: i64,
}

/// Where a symbol is defined.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SymbolSection {
    /// Not defined by this object.
    Undefined,
    /// An absolute value, or a table slot that holds no symbol.
    Absolute,
    /// Defined in `Object::sections[index]`.
    Section(u32),
}

#[derive(Clone, Copy, Debug)]
pub struct Symbol<'a> {
    pub name: &'a [u8],
    pub section: SymbolSection,
    /// Offset from the start of the defining section.
    pub value: u32,
    /// Visible outside the object.
    pub external: bool,
    /// Another library may supply a different definition at run time (ELF
    /// default visibility; always `false` for COFF). A shared library must
    /// reach such a symbol through the global offset table or the procedure
    /// linkage table.
    pub preemptible: bool,
    /// Size in bytes, where the format records one (ELF).
    pub size: u32,
    pub kind: SymbolKind,
}

/// What a symbol names, as far as a dynamic symbol table cares.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SymbolKind {
    NoType,
    Function,
    Object,
}

#[derive(Clone, Debug)]
pub struct Section<'a> {
    pub name: &'a [u8],
    pub class: SectionClass,
    /// Required alignment in bytes; a power of two.
    pub align: u32,
    /// The contents; empty for `SectionClass::ZeroData`.
    pub data: &'a [u8],
    /// Size in bytes; equals `data.len()` except for zero data.
    pub size: u32,
    /// The slice of `Object::relocs` that belongs to this section.
    pub relocs: std::ops::Range<u32>,
    /// The section is a single-definition COMDAT: it is kept only if
    /// something refers to it.
    pub comdat: bool,
}

/// A relocatable object.
#[derive(Clone, Debug, Default)]
pub struct Object<'a> {
    pub sections: Vec<Section<'a>>,
    pub symbols: Vec<Symbol<'a>>,
    pub relocs: Vec<Reloc>,
}

impl Object<'_> {
    /// The relocations of section `index`.
    pub fn relocs_of(&self, index: usize) -> &[Reloc] {
        let range = &self.sections[index].relocs;
        &self.relocs[range.start as usize..range.end as usize]
    }
}

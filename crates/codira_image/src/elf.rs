//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Reads the ELF64 relocatable object LLVM emits for `x86_64-*-linux-*` into
//!   the neutral `Object` model, and writes an x86-64 ELF shared object from
//!   it: `PT_LOAD` segments for read-only data, code and writable data,
//!   `.dynsym`, a System V `.hash`, `.dynstr`, `.rela.dyn`, `.dynamic`, a
//!   `.got`, and relocations resolved the way `ld.lld --shared` resolves them
//!   (`R_X86_64_RELATIVE` for addresses of things in the library, `R_X86_64_64`
//!   and `R_X86_64_GLOB_DAT` for symbols another library may override).
//! - Anything outside that is declined, never guessed: thread-local storage,
//!   section groups, `SHT_REL`, debug sections, an undefined symbol, a call to
//!   a symbol that could be overridden (it would need a PLT), a relocation in
//!   read-only memory (it would need `DT_TEXTREL`).
//! - Strings are not merged and `.eh_frame` is copied as it is, so the image is
//!   functionally the one LLD writes but not byte for byte.
use crate::{
    decline::{Decline, DeclineKind},
    model::{Object, Reloc, RelocKind, Section, SectionClass, Symbol, SymbolKind, SymbolSection},
};

const SHT_PROGBITS: u32 = 1;
const SHT_SYMTAB: u32 = 2;
const SHT_STRTAB: u32 = 3;
const SHT_RELA: u32 = 4;
const SHT_NOBITS: u32 = 8;
const SHT_REL: u32 = 9;
const SHT_X86_64_UNWIND: u32 = 0x7000_0001;
const SHT_GROUP: u32 = 17;

const SHF_WRITE: u64 = 0x1;
const SHF_ALLOC: u64 = 0x2;
const SHF_EXEC: u64 = 0x4;
const SHF_LINK_ORDER: u64 = 0x80;
const SHF_GROUP: u64 = 0x200;
const SHF_TLS: u64 = 0x400;
const SHF_COMPRESSED: u64 = 0x800;

const PAGE: u64 = 0x1000;
const MAX_IMAGE_SIZE: u64 = 0x3000_0000;

fn malformed(what: &str) -> Decline {
    Decline::new(DeclineKind::Malformed, what)
}

fn u16_at(bytes: &[u8], at: usize) -> Option<u16> {
    Some(u16::from_le_bytes(
        bytes.get(at..at.checked_add(2)?)?.try_into().ok()?,
    ))
}

fn u32_at(bytes: &[u8], at: usize) -> Option<u32> {
    Some(u32::from_le_bytes(
        bytes.get(at..at.checked_add(4)?)?.try_into().ok()?,
    ))
}

fn u64_at(bytes: &[u8], at: usize) -> Option<u64> {
    Some(u64::from_le_bytes(
        bytes.get(at..at.checked_add(8)?)?.try_into().ok()?,
    ))
}

/// A section header of the input.
struct Shdr {
    name: u32,
    kind: u32,
    flags: u64,
    offset: u64,
    size: u64,
    link: u32,
    info: u32,
    align: u64,
}

fn c_string(table: &[u8], offset: usize) -> Option<&[u8]> {
    let rest = table.get(offset..)?;
    let end = rest.iter().position(|&b| b == 0)?;
    Some(&rest[..end])
}

fn slice(bytes: &[u8], offset: u64, size: u64) -> Result<&[u8], Decline> {
    let start = usize::try_from(offset).map_err(|_range| malformed("offset"))?;
    let size = usize::try_from(size).map_err(|_range| malformed("size"))?;
    start
        .checked_add(size)
        .and_then(|end| bytes.get(start..end))
        .ok_or_else(|| malformed("section contents outside the file"))
}

/// Parses an ELF64 relocatable object for x86-64. Never panics.
pub fn read_elf(bytes: &[u8]) -> Result<Object<'_>, Decline> {
    if bytes.get(..4) != Some(b"\x7fELF") {
        return Err(Decline::new(
            DeclineKind::UnsupportedFormat,
            "not an ELF file",
        ));
    }
    if bytes.get(4) != Some(&2) || bytes.get(5) != Some(&1) {
        return Err(Decline::new(
            DeclineKind::UnsupportedFormat,
            "not a little-endian ELF64 file",
        ));
    }
    if u16_at(bytes, 16) != Some(1) {
        return Err(Decline::new(
            DeclineKind::UnsupportedFormat,
            "not a relocatable object",
        ));
    }
    if u16_at(bytes, 18) != Some(62) {
        return Err(Decline::new(
            DeclineKind::UnsupportedMachine,
            format!("machine {}", u16_at(bytes, 18).unwrap_or(0)),
        ));
    }
    let shoff = u64_at(bytes, 0x28).ok_or_else(|| malformed("short header"))? as usize;
    let shentsize = usize::from(u16_at(bytes, 0x3A).unwrap_or(0));
    let shnum = usize::from(u16_at(bytes, 0x3C).unwrap_or(0));
    let shstrndx = usize::from(u16_at(bytes, 0x3E).unwrap_or(0));
    if shentsize != 64 || shnum == 0 || shnum >= 0xff00 || shstrndx >= shnum {
        return Err(malformed("section header table"));
    }
    let table = shoff
        .checked_add(shnum * 64)
        .and_then(|end| bytes.get(shoff..end))
        .ok_or_else(|| malformed("section header table outside the file"))?;
    let headers: Vec<Shdr> = table
        .chunks_exact(64)
        .map(|h| Shdr {
            name: u32_at(h, 0).unwrap_or(0),
            kind: u32_at(h, 4).unwrap_or(0),
            flags: u64_at(h, 8).unwrap_or(0),
            offset: u64_at(h, 24).unwrap_or(0),
            size: u64_at(h, 32).unwrap_or(0),
            link: u32_at(h, 40).unwrap_or(0),
            info: u32_at(h, 44).unwrap_or(0),
            align: u64_at(h, 48).unwrap_or(0),
        })
        .collect();

    let contents = |h: &Shdr| slice(bytes, h.offset, h.size);
    let shstrtab = contents(&headers[shstrndx])?;
    let name_of = |h: &Shdr| c_string(shstrtab, h.name as usize).unwrap_or(b"");

    // Which sections become part of the library.
    let mut object = Object::default();
    let mut model_index: Vec<Option<u32>> = vec![None; shnum];
    let mut symtab: Option<usize> = None;
    for (index, h) in headers.iter().enumerate() {
        let name = name_of(h);
        let shown = String::from_utf8_lossy(name).into_owned();
        match h.kind {
            0 | SHT_STRTAB | SHT_RELA => continue,
            SHT_SYMTAB => {
                if symtab.replace(index).is_some() {
                    return Err(malformed("more than one symbol table"));
                }
                continue;
            }
            SHT_REL => {
                return Err(Decline::new(
                    DeclineKind::UnknownRelocation,
                    "SHT_REL relocations",
                ));
            }
            SHT_GROUP => {
                return Err(Decline::new(DeclineKind::Comdat, "a section group"));
            }
            _ => {}
        }
        if h.flags & SHF_TLS != 0 {
            return Err(Decline::new(
                DeclineKind::UnknownSection,
                format!("thread-local section `{shown}`"),
            ));
        }
        if h.flags & (SHF_GROUP | SHF_COMPRESSED | SHF_LINK_ORDER) != 0 {
            return Err(Decline::new(
                DeclineKind::UnknownSection,
                format!("section `{shown}` with flags {:#x}", h.flags),
            ));
        }
        if h.flags & SHF_ALLOC == 0 {
            // Sections that are not loaded: the linker drops these too.
            let droppable = matches!(
                name,
                b".comment" | b".llvm_addrsig" | b".note.GNU-stack" | b".shstrtab"
            );
            if name.starts_with(b".debug") {
                return Err(Decline::new(
                    DeclineKind::DebugInfo,
                    format!("debug section `{shown}`"),
                ));
            }
            if !droppable {
                return Err(Decline::new(
                    DeclineKind::UnknownSection,
                    format!("section `{shown}`"),
                ));
            }
            if name == b".note.GNU-stack" && h.flags & SHF_EXEC != 0 {
                return Err(Decline::new(
                    DeclineKind::UnknownSection,
                    "an object that asks for an executable stack",
                ));
            }
            continue;
        }

        let class = match (h.kind, name) {
            (SHT_PROGBITS, n) if h.flags & SHF_EXEC != 0 && n.starts_with(b".text") => {
                Some(SectionClass::Code)
            }
            (SHT_PROGBITS | SHT_X86_64_UNWIND, b".eh_frame") if h.flags == SHF_ALLOC => {
                Some(SectionClass::EhFrame)
            }
            (SHT_PROGBITS, n)
                if h.flags & SHF_WRITE != 0
                    && h.flags & SHF_EXEC == 0
                    && n.starts_with(b".data.rel.ro") =>
            {
                Some(SectionClass::RelroData)
            }
            (SHT_PROGBITS, n)
                if h.flags & SHF_WRITE != 0
                    && h.flags & SHF_EXEC == 0
                    && (n == b".data" || n.starts_with(b".data.")) =>
            {
                Some(SectionClass::Data)
            }
            (SHT_NOBITS, n)
                if h.flags & SHF_WRITE != 0
                    && h.flags & SHF_EXEC == 0
                    && (n == b".bss" || n.starts_with(b".bss.")) =>
            {
                Some(SectionClass::ZeroData)
            }
            (SHT_PROGBITS, n)
                if h.flags & (SHF_WRITE | SHF_EXEC) == 0 && n.starts_with(b".rodata") =>
            {
                Some(SectionClass::ReadOnlyData)
            }
            _ => None,
        };
        let Some(class) = class else {
            return Err(Decline::new(
                DeclineKind::UnknownSection,
                format!(
                    "section `{shown}` of type {:#x} with flags {:#x}",
                    h.kind, h.flags
                ),
            ));
        };
        let align = h.align.max(1);
        if !align.is_power_of_two() || align > PAGE {
            return Err(Decline::new(
                DeclineKind::Unmodelled,
                format!("section alignment {align}"),
            ));
        }
        let data = if class == SectionClass::ZeroData {
            &[][..]
        } else {
            contents(h)?
        };
        model_index[index] = Some(object.sections.len() as u32);
        object.sections.push(Section {
            name,
            class,
            align: align as u32,
            data,
            size: u32::try_from(h.size).map_err(|_range| malformed("section size"))?,
            relocs: 0..0,
            comdat: false,
        });
    }

    // Symbols.
    let symtab = symtab.ok_or_else(|| malformed("no symbol table"))?;
    let symtab_header = &headers[symtab];
    let strtab = headers
        .get(symtab_header.link as usize)
        .ok_or_else(|| malformed("symbol string table"))
        .and_then(contents)?;
    let symbols = contents(symtab_header)?;
    if symbols.len() % 24 != 0 {
        return Err(malformed("symbol table size"));
    }
    object.symbols.reserve(symbols.len() / 24);
    for s in symbols.chunks_exact(24) {
        let name = c_string(strtab, u32_at(s, 0).unwrap_or(0) as usize).unwrap_or(b"");
        let info = s[4];
        let visibility = s[5] & 3;
        let shndx = u16_at(s, 6).unwrap_or(0);
        let value = u64_at(s, 8).unwrap_or(0);
        let size = u64_at(s, 16).unwrap_or(0);
        let (binding, kind) = (info >> 4, info & 0xF);
        let shown = || String::from_utf8_lossy(name).into_owned();
        let kind = match kind {
            0 | 3 | 4 => SymbolKind::NoType,
            1 => SymbolKind::Object,
            2 => SymbolKind::Function,
            other => {
                return Err(Decline::new(
                    DeclineKind::UnknownSymbol,
                    format!(
                        "symbol `{}` of type {other} (TLS, common or ifunc)",
                        shown()
                    ),
                ));
            }
        };
        if binding > 2 {
            return Err(Decline::new(
                DeclineKind::UnknownSymbol,
                format!("symbol `{}` with binding {binding}", shown()),
            ));
        }
        let placement = match shndx {
            0 => SymbolSection::Undefined,
            0xFFF1 => SymbolSection::Absolute,
            0xFFF2 => {
                return Err(Decline::new(
                    DeclineKind::UnknownSymbol,
                    format!("common symbol `{}`", shown()),
                ));
            }
            n if n >= 0xFF00 => return Err(malformed("reserved section index")),
            n => match model_index.get(usize::from(n)) {
                Some(Some(index)) => SymbolSection::Section(*index),
                // A symbol in a section that is not loaded (the stack note,
                // the comment): nothing may refer to it.
                Some(None) => SymbolSection::Absolute,
                None => return Err(malformed("symbol in a section that does not exist")),
            },
        };
        let external = binding != 0 && visibility != 2 && visibility != 1;
        object.symbols.push(Symbol {
            name,
            section: placement,
            value: u32::try_from(value).map_err(|_range| malformed("symbol value"))?,
            external,
            preemptible: external && visibility == 0,
            size: u32::try_from(size).unwrap_or(u32::MAX),
            kind,
        });
    }

    // Relocations: every SHT_RELA section belongs to the section it names.
    // The model keeps one run of relocations per section, so they are
    // collected per target first.
    let mut per_section: Vec<Vec<Reloc>> = vec![Vec::new(); object.sections.len()];
    for h in headers.iter().filter(|h| h.kind == SHT_RELA) {
        if h.link as usize != symtab {
            return Err(malformed("relocations against another symbol table"));
        }
        let Some(Some(target)) = model_index.get(h.info as usize) else {
            // Relocations of a section that is not loaded.
            continue;
        };
        let data = contents(h)?;
        if data.len() % 24 != 0 {
            return Err(malformed("relocation table size"));
        }
        for r in data.chunks_exact(24) {
            let offset = u64_at(r, 0).unwrap_or(0);
            let info = u64_at(r, 8).unwrap_or(0);
            let addend = i64::from_le_bytes(r[16..24].try_into().unwrap_or([0; 8]));
            let (symbol, kind) = ((info >> 32) as u32, info as u32);
            let kind = match kind {
                0 => continue,
                1 => RelocKind::Abs64,
                // The x86-64 psABI: PLT32 against a symbol that cannot be
                // overridden is a PC32.
                2 | 4 => RelocKind::PcRel32,
                9 | 41 | 42 => RelocKind::GotPcRel,
                other => {
                    return Err(Decline::new(
                        DeclineKind::UnknownRelocation,
                        format!("ELF x86-64 relocation type {other}"),
                    ));
                }
            };
            per_section[*target as usize].push(Reloc {
                offset: u32::try_from(offset).map_err(|_range| malformed("relocation offset"))?,
                symbol,
                kind,
                addend,
            });
        }
    }
    for (section, relocs) in object.sections.iter_mut().zip(per_section) {
        let first = object.relocs.len() as u32;
        object.relocs.extend(relocs);
        section.relocs = first..object.relocs.len() as u32;
    }
    Ok(object)
}

// ---------------------------------------------------------------------------
// Writer
// ---------------------------------------------------------------------------

/// What to build.
#[derive(Clone, Copy, Debug, Default)]
pub struct ElfRequest<'a> {
    /// `DT_SONAME`, if the library should have one.
    pub soname: Option<&'a str>,
}

fn align_up(value: u64, align: u64) -> u64 {
    value.div_ceil(align) * align
}

fn put16(buf: &mut [u8], at: usize, value: u16) {
    buf[at..at + 2].copy_from_slice(&value.to_le_bytes());
}

fn put32(buf: &mut [u8], at: usize, value: u32) {
    buf[at..at + 4].copy_from_slice(&value.to_le_bytes());
}

fn put64(buf: &mut [u8], at: usize, value: u64) {
    buf[at..at + 8].copy_from_slice(&value.to_le_bytes());
}

fn out_of_range(what: &str) -> Decline {
    Decline::new(DeclineKind::OutOfRange, what)
}

/// The System V ELF hash of a symbol name.
fn elf_hash(name: &[u8]) -> u32 {
    let mut h: u32 = 0;
    for &c in name {
        h = (h << 4).wrapping_add(u32::from(c));
        let g = h & 0xF000_0000;
        if g != 0 {
            h ^= g >> 24;
        }
        h &= !g;
    }
    h
}

/// One output section.
#[derive(Clone, Copy, Default)]
struct Out {
    present: bool,
    addr: u64,
    size: u64,
    /// Bytes in the file (less than `size` for `.bss`).
    file_size: u64,
    align: u64,
}

const NAMES: [&str; 10] = [
    ".dynsym",
    ".hash",
    ".dynstr",
    ".rela.dyn",
    ".rodata",
    ".eh_frame",
    ".text",
    ".data.rel.ro",
    ".dynamic",
    ".got",
];
const DYNSYM: usize = 0;
const HASH: usize = 1;
const DYNSTR: usize = 2;
const RELA: usize = 3;
const RODATA: usize = 4;
const EH_FRAME: usize = 5;
const TEXT: usize = 6;
const RELRO: usize = 7;
const DYNAMIC: usize = 8;
const GOT: usize = 9;
const DATA: usize = 10;
const BSS: usize = 11;
const SECTION_SLOTS: usize = 12;

/// Which output section a model section belongs to.
fn slot_of(class: SectionClass) -> Option<usize> {
    match class {
        SectionClass::Code => Some(TEXT),
        SectionClass::ReadOnlyData => Some(RODATA),
        SectionClass::EhFrame => Some(EH_FRAME),
        SectionClass::RelroData => Some(RELRO),
        SectionClass::Data => Some(DATA),
        SectionClass::ZeroData => Some(BSS),
        SectionClass::UnwindInfo | SectionClass::ExceptionTable => None,
    }
}

/// A dynamic relocation to emit.
struct Dyn {
    offset: u64,
    /// `None`: `R_X86_64_RELATIVE`.
    symbol: Option<u32>,
    kind: u32,
    addend: i64,
}

const R_X86_64_64: u32 = 1;
const R_X86_64_GLOB_DAT: u32 = 6;
const R_X86_64_RELATIVE: u32 = 8;

/// Builds an x86-64 ELF shared object for `object`.
pub fn write_elf_so(object: &Object<'_>, request: &ElfRequest<'_>) -> Result<Vec<u8>, Decline> {
    for section in &object.sections {
        if slot_of(section.class).is_none() {
            return Err(Decline::new(
                DeclineKind::Unmodelled,
                "an object that is not an ELF object",
            ));
        }
        if section.align > PAGE as u32 {
            return Err(Decline::new(
                DeclineKind::Unmodelled,
                "section alignment above a page",
            ));
        }
    }
    let symbol_count = object.symbols.len();

    // ---- Which symbols are exported, and the string table. ----
    let mut dynsym_of: Vec<u32> = vec![0; symbol_count];
    let mut exported: Vec<usize> = Vec::new();
    for (index, symbol) in object.symbols.iter().enumerate() {
        if symbol.external && matches!(symbol.section, SymbolSection::Section(_)) {
            exported.push(index);
            dynsym_of[index] = exported.len() as u32;
        }
    }
    let mut dynstr: Vec<u8> = vec![0];
    let mut name_offsets: Vec<u32> = Vec::with_capacity(exported.len());
    for &index in &exported {
        name_offsets.push(dynstr.len() as u32);
        dynstr.extend_from_slice(object.symbols[index].name);
        dynstr.push(0);
    }
    let soname_offset = request.soname.map(|name| {
        let at = dynstr.len() as u32;
        dynstr.extend_from_slice(name.as_bytes());
        dynstr.push(0);
        at
    });
    let nsyms = exported.len() + 1;

    // ---- Dynamic relocations and GOT slots, by what the relocations ask. ----
    // Pass over every relocation once to count; addresses come later.
    let mut got_slot: Vec<u32> = vec![u32::MAX; symbol_count];
    let mut got_count = 0u64;
    let mut abs_count = 0u64;
    for (index, section) in object.sections.iter().enumerate() {
        let class = section.class;
        for reloc in object.relocs_of(index) {
            let symbol = object
                .symbols
                .get(reloc.symbol as usize)
                .ok_or_else(|| malformed("relocation against a missing symbol"))?;
            match reloc.kind {
                RelocKind::Abs64 => {
                    if !matches!(class, SectionClass::RelroData | SectionClass::Data) {
                        return Err(Decline::new(
                            DeclineKind::UnknownRelocation,
                            "an absolute address in read-only memory (it would need DT_TEXTREL)",
                        ));
                    }
                    abs_count += 1;
                }
                RelocKind::GotPcRel => {
                    if got_slot[reloc.symbol as usize] == u32::MAX {
                        got_slot[reloc.symbol as usize] = got_count as u32;
                        got_count += 1;
                    }
                }
                RelocKind::PcRel32 => {
                    if symbol.preemptible {
                        return Err(Decline::new(
                            DeclineKind::UnknownRelocation,
                            format!(
                                "a direct reference to `{}`, which another library could override \
                                 (it would need a PLT)",
                                String::from_utf8_lossy(symbol.name)
                            ),
                        ));
                    }
                }
                RelocKind::Rva32 | RelocKind::SecRel32 | RelocKind::SectionIndex => {
                    return Err(Decline::new(
                        DeclineKind::UnknownRelocation,
                        "a PE relocation in an ELF object",
                    ));
                }
            }
        }
    }
    let dyn_count = abs_count + got_count;

    // ---- Sizes of the synthetic sections. ----
    let phdr_count = 7u64;
    let headers_size = 64 + 56 * phdr_count;
    let nbucket = (nsyms as u64 | 1).min(1 << 20);
    let hash_size = 4 * (2 + nbucket + nsyms as u64);
    let dynamic_entries =
        5 + if dyn_count > 0 { 4 } else { 0 } + u64::from(soname_offset.is_some()) + 1;
    let mut outs = [Out::default(); SECTION_SLOTS];
    outs[DYNSYM] = Out {
        present: true,
        size: 24 * nsyms as u64,
        align: 8,
        ..Out::default()
    };
    outs[HASH] = Out {
        present: true,
        size: hash_size,
        align: 8,
        ..Out::default()
    };
    outs[DYNSTR] = Out {
        present: true,
        size: dynstr.len() as u64,
        align: 1,
        ..Out::default()
    };
    outs[RELA] = Out {
        present: dyn_count > 0,
        size: 24 * dyn_count,
        align: 8,
        ..Out::default()
    };
    outs[DYNAMIC] = Out {
        present: true,
        size: 16 * dynamic_entries,
        align: 8,
        ..Out::default()
    };
    outs[GOT] = Out {
        present: got_count > 0,
        size: 8 * got_count,
        align: 8,
        ..Out::default()
    };

    // Offsets of input sections inside their output sections.
    let mut offset_in_out = vec![0u64; object.sections.len()];
    for (index, section) in object.sections.iter().enumerate() {
        let Some(slot) = slot_of(section.class) else {
            continue;
        };
        let out = &mut outs[slot];
        out.present = true;
        out.align = out.align.max(u64::from(section.align));
        let at = align_up(out.size, u64::from(section.align));
        offset_in_out[index] = at;
        out.size = at + u64::from(section.size);
        if out.size > MAX_IMAGE_SIZE {
            return Err(out_of_range("image larger than 768 MiB"));
        }
    }
    for (slot, out) in outs.iter_mut().enumerate() {
        out.file_size = if slot == BSS { 0 } else { out.size };
    }
    if !outs[TEXT].present {
        return Err(Decline::new(
            DeclineKind::Unmodelled,
            "an object without code",
        ));
    }

    // ---- Addresses. File offsets equal addresses; segments start on pages. ----
    let mut cursor = headers_size;
    let place = |out: &mut Out, cursor: &mut u64| {
        if out.present {
            *cursor = align_up(*cursor, out.align.max(1));
            out.addr = *cursor;
            *cursor += out.size;
        }
    };
    for slot in [DYNSYM, HASH, DYNSTR, RELA, RODATA, EH_FRAME] {
        place(&mut outs[slot], &mut cursor);
    }
    let ro_end = cursor;
    cursor = align_up(cursor, PAGE);
    let rx_start = cursor;
    place(&mut outs[TEXT], &mut cursor);
    let rx_end = cursor;
    cursor = align_up(cursor, PAGE);
    let rw_start = cursor;
    for slot in [RELRO, DYNAMIC, GOT] {
        place(&mut outs[slot], &mut cursor);
    }
    // The part that is made read-only after relocation ends on a page.
    let relro_end = align_up(cursor, PAGE);
    cursor = relro_end;
    place(&mut outs[DATA], &mut cursor);
    let data_file_end = cursor;
    place(&mut outs[BSS], &mut cursor);
    let rw_end = cursor;
    let rw_file_end = data_file_end.max(relro_end);
    if rw_end > MAX_IMAGE_SIZE {
        return Err(out_of_range("image larger than 768 MiB"));
    }

    // ---- Section header table and its names go after the contents. ----
    let present: Vec<usize> = (0..SECTION_SLOTS).filter(|&s| outs[s].present).collect();
    // Section header indices: 0 is null, then the present sections in
    // address order, then `.shstrtab`.
    let mut order: Vec<usize> = present.clone();
    order.sort_by_key(|&s| (outs[s].addr, s));
    let mut shndx_of_slot = [0u16; SECTION_SLOTS];
    for (i, &slot) in order.iter().enumerate() {
        shndx_of_slot[slot] = (i + 1) as u16;
    }
    let all_names: Vec<&str> = (0..SECTION_SLOTS)
        .map(|s| match s {
            DATA => ".data",
            BSS => ".bss",
            _ => NAMES[s],
        })
        .collect();
    let mut shstrtab: Vec<u8> = vec![0];
    let mut sh_name_offset = [0u32; SECTION_SLOTS];
    for &slot in &order {
        sh_name_offset[slot] = shstrtab.len() as u32;
        shstrtab.extend_from_slice(all_names[slot].as_bytes());
        shstrtab.push(0);
    }
    let shstrtab_name = shstrtab.len() as u32;
    shstrtab.extend_from_slice(b".shstrtab\0");
    let shdr_count = order.len() as u64 + 2;
    let shstrtab_at = align_up(rw_file_end, 8);
    let shdr_at = align_up(shstrtab_at + shstrtab.len() as u64, 8);
    let total = shdr_at + 64 * shdr_count;
    if total > MAX_IMAGE_SIZE {
        return Err(out_of_range("image larger than 768 MiB"));
    }
    let mut out = vec![0u8; total as usize];

    // ---- Symbol addresses. ----
    let section_slot = |section: u32| -> usize {
        slot_of(object.sections[section as usize].class).unwrap_or(TEXT)
    };
    let symbol_address = |index: u32| -> Result<u64, Decline> {
        let symbol = object
            .symbols
            .get(index as usize)
            .ok_or_else(|| malformed("relocation against a missing symbol"))?;
        match symbol.section {
            SymbolSection::Section(section) => {
                if symbol.value > object.sections[section as usize].size {
                    return Err(malformed("a symbol outside its section"));
                }
                Ok(outs[section_slot(section)].addr
                    + offset_in_out[section as usize]
                    + u64::from(symbol.value))
            }
            SymbolSection::Undefined => Err(Decline::new(
                DeclineKind::UndefinedSymbol,
                format!("`{}`", String::from_utf8_lossy(symbol.name)),
            )),
            SymbolSection::Absolute => Err(Decline::new(
                DeclineKind::UnknownSymbol,
                format!(
                    "relocation against the absolute symbol `{}`",
                    String::from_utf8_lossy(symbol.name)
                ),
            )),
        }
    };

    // ---- Copy contents, apply relocations, collect dynamic relocations. ----
    let mut dynamic_relocs: Vec<Dyn> = Vec::with_capacity(dyn_count as usize);
    let got_addr = |slot: u32| outs[GOT].addr + 8 * u64::from(slot);
    for (index, section) in object.sections.iter().enumerate() {
        let slot = section_slot(index as u32);
        let base = outs[slot].addr + offset_in_out[index];
        let file_at = base as usize;
        if section.class != SectionClass::ZeroData && !section.data.is_empty() {
            out[file_at..file_at + section.data.len()].copy_from_slice(section.data);
        }
        let mut written_until = 0u64;
        let mut sorted = true;
        for reloc in object.relocs_of(index) {
            let width: u64 = match reloc.kind {
                RelocKind::Abs64 => 8,
                _ => 4,
            };
            if u64::from(reloc.offset) + width > u64::from(section.size) {
                return Err(malformed("relocation outside its section"));
            }
            if u64::from(reloc.offset) < written_until {
                sorted = false;
            }
            written_until = u64::from(reloc.offset) + width;
            let field = file_at + reloc.offset as usize;
            let place_address = base + u64::from(reloc.offset);
            let symbol = &object.symbols[reloc.symbol as usize];
            let target = symbol_address(reloc.symbol)?;
            match reloc.kind {
                RelocKind::Abs64 => {
                    let value = (target as i64)
                        .checked_add(reloc.addend)
                        .ok_or_else(|| out_of_range("64-bit relocation"))?;
                    if symbol.preemptible {
                        dynamic_relocs.push(Dyn {
                            offset: place_address,
                            symbol: Some(dynsym_of[reloc.symbol as usize]),
                            kind: R_X86_64_64,
                            addend: reloc.addend,
                        });
                        put64(&mut out, field, 0);
                    } else {
                        // A relative address must point into the library.
                        let in_a_segment = [(0, ro_end), (rx_start, rx_end), (rw_start, rw_end)]
                            .iter()
                            .any(|&(start, end)| (start as i64..=end as i64).contains(&value));
                        if !in_a_segment {
                            return Err(out_of_range("64-bit relocation outside the image"));
                        }
                        dynamic_relocs.push(Dyn {
                            offset: place_address,
                            symbol: None,
                            kind: R_X86_64_RELATIVE,
                            addend: value,
                        });
                        put64(&mut out, field, value as u64);
                    }
                }
                RelocKind::PcRel32 => {
                    let value = (target as i64)
                        .checked_add(reloc.addend)
                        .and_then(|v| v.checked_sub(place_address as i64))
                        .and_then(|v| i32::try_from(v).ok())
                        .ok_or_else(|| out_of_range("32-bit PC-relative relocation"))?;
                    put32(&mut out, field, value as u32);
                }
                RelocKind::GotPcRel => {
                    let slot_address = got_addr(got_slot[reloc.symbol as usize]);
                    let value = (slot_address as i64)
                        .checked_add(reloc.addend)
                        .and_then(|v| v.checked_sub(place_address as i64))
                        .and_then(|v| i32::try_from(v).ok())
                        .ok_or_else(|| out_of_range("GOT-relative relocation"))?;
                    put32(&mut out, field, value as u32);
                }
                RelocKind::Rva32 | RelocKind::SecRel32 | RelocKind::SectionIndex => {
                    return Err(Decline::new(
                        DeclineKind::UnknownRelocation,
                        "a PE relocation in an ELF object",
                    ));
                }
            }
        }
        if !sorted {
            check_disjoint(object.relocs_of(index))?;
        }
    }
    // The GOT slots: one dynamic relocation each.
    for (symbol_index, &slot) in got_slot.iter().enumerate() {
        if slot == u32::MAX {
            continue;
        }
        let symbol = &object.symbols[symbol_index];
        let target = symbol_address(symbol_index as u32)?;
        if symbol.preemptible {
            dynamic_relocs.push(Dyn {
                offset: got_addr(slot),
                symbol: Some(dynsym_of[symbol_index]),
                kind: R_X86_64_GLOB_DAT,
                addend: 0,
            });
        } else {
            dynamic_relocs.push(Dyn {
                offset: got_addr(slot),
                symbol: None,
                kind: R_X86_64_RELATIVE,
                addend: target as i64,
            });
        }
    }
    // Relative relocations first, in address order: the loader's fast path.
    dynamic_relocs.sort_by_key(|d| (d.symbol.is_some(), d.offset));
    let relative_count = dynamic_relocs
        .iter()
        .take_while(|d| d.symbol.is_none())
        .count();

    // ---- .rela.dyn ----
    let mut at = outs[RELA].addr as usize;
    for d in &dynamic_relocs {
        put64(&mut out, at, d.offset);
        put64(
            &mut out,
            at + 8,
            (u64::from(d.symbol.unwrap_or(0)) << 32) | u64::from(d.kind),
        );
        put64(&mut out, at + 16, d.addend as u64);
        at += 24;
    }

    // ---- .dynsym, .dynstr, .hash ----
    let dynsym_at = outs[DYNSYM].addr as usize;
    for (i, &index) in exported.iter().enumerate() {
        let symbol = &object.symbols[index];
        let at = dynsym_at + 24 * (i + 1);
        put32(&mut out, at, name_offsets[i]);
        let kind = match symbol.kind {
            SymbolKind::NoType => 0u8,
            SymbolKind::Object => 1,
            SymbolKind::Function => 2,
        };
        // STB_GLOBAL; default visibility.
        out[at + 4] = (1 << 4) | kind;
        let SymbolSection::Section(section) = symbol.section else {
            continue;
        };
        put16(&mut out, at + 6, shndx_of_slot[section_slot(section)]);
        put64(&mut out, at + 8, symbol_address(index as u32)?);
        put64(&mut out, at + 16, u64::from(symbol.size));
    }
    let dynstr_at = outs[DYNSTR].addr as usize;
    out[dynstr_at..dynstr_at + dynstr.len()].copy_from_slice(&dynstr);
    let hash_at = outs[HASH].addr as usize;
    put32(&mut out, hash_at, nbucket as u32);
    put32(&mut out, hash_at + 4, nsyms as u32);
    let buckets = hash_at + 8;
    let chains = buckets + 4 * nbucket as usize;
    for (i, &index) in exported.iter().enumerate() {
        let sym = (i + 1) as u32;
        let bucket =
            buckets + 4 * (elf_hash(object.symbols[index].name) as usize % nbucket as usize);
        let previous = u32_at(&out, bucket).unwrap_or(0);
        put32(&mut out, chains + 4 * sym as usize, previous);
        put32(&mut out, bucket, sym);
    }

    // ---- .dynamic ----
    let mut entries: Vec<(u64, u64)> = vec![
        (4, outs[HASH].addr),      // DT_HASH
        (5, outs[DYNSTR].addr),    // DT_STRTAB
        (6, outs[DYNSYM].addr),    // DT_SYMTAB
        (10, dynstr.len() as u64), // DT_STRSZ
        (11, 24),                  // DT_SYMENT
    ];
    if dyn_count > 0 {
        entries.push((7, outs[RELA].addr)); // DT_RELA
        entries.push((8, 24 * dyn_count)); // DT_RELASZ
        entries.push((9, 24)); // DT_RELAENT
        entries.push((0x6fff_fff9, relative_count as u64)); // DT_RELACOUNT
    }
    if let Some(offset) = soname_offset {
        entries.push((14, u64::from(offset))); // DT_SONAME
    }
    entries.push((0, 0));
    // The dynamic section was sized for the maximum; unused entries are
    // `DT_NULL`, which the zeroed memory already is.
    let dynamic_at = outs[DYNAMIC].addr as usize;
    for (i, (tag, value)) in entries.iter().enumerate() {
        put64(&mut out, dynamic_at + 16 * i, *tag);
        put64(&mut out, dynamic_at + 16 * i + 8, *value);
    }

    // ---- Program headers. ----
    let phdrs: Vec<[u64; 7]> = vec![
        // type, flags, offset, vaddr, filesz, memsz, align
        [6, 4, 64, 64, 56 * phdr_count, 56 * phdr_count, 8],
        [1, 4, 0, 0, ro_end, ro_end, PAGE],
        [
            1,
            5,
            rx_start,
            rx_start,
            rx_end - rx_start,
            rx_end - rx_start,
            PAGE,
        ],
        [
            1,
            6,
            rw_start,
            rw_start,
            rw_file_end - rw_start,
            rw_end - rw_start,
            PAGE,
        ],
        [
            2,
            6,
            outs[DYNAMIC].addr,
            outs[DYNAMIC].addr,
            outs[DYNAMIC].size,
            outs[DYNAMIC].size,
            8,
        ],
        [
            0x6474_e552,
            4,
            rw_start,
            rw_start,
            relro_end - rw_start,
            relro_end - rw_start,
            1,
        ],
        [0x6474_e551, 6, 0, 0, 0, 0, 16],
    ];
    debug_assert_eq!(phdrs.len() as u64, phdr_count);
    for (i, p) in phdrs.iter().enumerate() {
        let at = 64 + 56 * i;
        put32(&mut out, at, p[0] as u32);
        put32(&mut out, at + 4, p[1] as u32);
        put64(&mut out, at + 8, p[2]);
        put64(&mut out, at + 16, p[3]);
        put64(&mut out, at + 24, p[3]);
        put64(&mut out, at + 32, p[4]);
        put64(&mut out, at + 40, p[5]);
        put64(&mut out, at + 48, p[6]);
    }

    // ---- Section headers. ----
    out[shstrtab_at as usize..shstrtab_at as usize + shstrtab.len()].copy_from_slice(&shstrtab);
    let section_header = |out: &mut Vec<u8>, index: u64, fields: [u64; 9]| {
        let at = (shdr_at + 64 * index) as usize;
        put32(out, at, fields[0] as u32);
        put32(out, at + 4, fields[1] as u32);
        put64(out, at + 8, fields[2]);
        put64(out, at + 16, fields[3]);
        put64(out, at + 24, fields[3]);
        put64(out, at + 32, fields[4]);
        put32(out, at + 40, fields[5] as u32);
        put32(out, at + 44, fields[6] as u32);
        put64(out, at + 48, fields[7]);
        put64(out, at + 56, fields[8]);
    };
    for &slot in &order {
        let o = outs[slot];
        let (kind, flags, link, info, entsize) = match slot {
            DYNSYM => (
                11u64,
                SHF_ALLOC,
                u64::from(shndx_of_slot[DYNSTR]),
                1u64,
                24u64,
            ),
            HASH => (5, SHF_ALLOC, u64::from(shndx_of_slot[DYNSYM]), 0, 4),
            DYNSTR => (SHT_STRTAB.into(), SHF_ALLOC, 0, 0, 0),
            RELA => (
                SHT_RELA.into(),
                SHF_ALLOC,
                u64::from(shndx_of_slot[DYNSYM]),
                0,
                24,
            ),
            RODATA | EH_FRAME => (SHT_PROGBITS.into(), SHF_ALLOC, 0, 0, 0),
            TEXT => (SHT_PROGBITS.into(), SHF_ALLOC | SHF_EXEC, 0, 0, 0),
            RELRO | DATA => (SHT_PROGBITS.into(), SHF_ALLOC | SHF_WRITE, 0, 0, 0),
            DYNAMIC => (
                6,
                SHF_ALLOC | SHF_WRITE,
                u64::from(shndx_of_slot[DYNSTR]),
                0,
                16,
            ),
            GOT => (SHT_PROGBITS.into(), SHF_ALLOC | SHF_WRITE, 0, 0, 8),
            _ => (SHT_NOBITS.into(), SHF_ALLOC | SHF_WRITE, 0, 0, 0),
        };
        section_header(
            &mut out,
            u64::from(shndx_of_slot[slot]),
            [
                u64::from(sh_name_offset[slot]),
                kind,
                flags,
                o.addr,
                o.size,
                link,
                info,
                o.align.max(1),
                entsize,
            ],
        );
    }
    section_header(
        &mut out,
        shdr_count - 1,
        [
            u64::from(shstrtab_name),
            SHT_STRTAB.into(),
            0,
            0,
            shstrtab.len() as u64,
            0,
            0,
            1,
            0,
        ],
    );
    // `section_header` wrote the file offset from the address; `.shstrtab`
    // is not loaded and lives at its own offset.
    let last = (shdr_at + 64 * (shdr_count - 1)) as usize;
    put64(&mut out, last + 16, 0);
    put64(&mut out, last + 24, shstrtab_at);

    // ---- ELF header. ----
    out[..4].copy_from_slice(b"\x7fELF");
    out[4] = 2;
    out[5] = 1;
    out[6] = 1;
    put16(&mut out, 16, 3);
    put16(&mut out, 18, 62);
    put32(&mut out, 20, 1);
    put64(&mut out, 32, 64);
    put64(&mut out, 40, shdr_at);
    put16(&mut out, 52, 64);
    put16(&mut out, 54, 56);
    put16(&mut out, 56, phdr_count as u16);
    put16(&mut out, 58, 64);
    put16(&mut out, 60, shdr_count as u16);
    put16(&mut out, 62, (shdr_count - 1) as u16);
    Ok(out)
}

fn check_disjoint(relocs: &[Reloc]) -> Result<(), Decline> {
    let mut fields: Vec<(u32, u32)> = relocs
        .iter()
        .map(|r| (r.offset, if r.kind == RelocKind::Abs64 { 8 } else { 4 }))
        .collect();
    fields.sort_unstable();
    if fields
        .windows(2)
        .any(|w| u64::from(w[0].0) + u64::from(w[0].1) > u64::from(w[1].0))
    {
        return Err(malformed("two relocations write the same bytes"));
    }
    Ok(())
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Reads the COFF object file that LLVM emits for `x86_64-pc-windows-*` into
//!   the neutral `Object` model. Everything the reader does not model is
//!   declined, never guessed: unknown sections, relocation types, symbol
//!   classes, debug information, directives, extended relocation counts.
use crate::{
    decline::{Decline, DeclineKind},
    model::{Object, Reloc, RelocKind, Section, SectionClass, Symbol, SymbolKind, SymbolSection},
};

const MACHINE_AMD64: u16 = 0x8664;

const SCN_LNK_NRELOC_OVFL: u32 = 0x0100_0000;
const SCN_LNK_COMDAT: u32 = 0x0000_1000;
const SCN_LNK_REMOVE: u32 = 0x0000_0800;
const SCN_LNK_INFO: u32 = 0x0000_0200;
const SCN_ALIGN_MASK: u32 = 0x00F0_0000;
const SCN_ALIGN_SHIFT: u32 = 20;

const SYM_CLASS_EXTERNAL: u8 = 2;
const SYM_CLASS_STATIC: u8 = 3;
const SYM_CLASS_LABEL: u8 = 6;
const SYM_CLASS_FILE: u8 = 0x67;

const SECTION_HEADER_SIZE: usize = 40;
const SYMBOL_SIZE: usize = 18;
const RELOC_SIZE: usize = 10;

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

/// The section kind for a name and its characteristics (alignment and
/// COMDAT bits already removed), or the reason there is none.
fn classify(name: &[u8], characteristics: u32) -> Result<SectionClass, Decline> {
    let class = match (name, characteristics) {
        (b".text", 0x6000_0020) => Some(SectionClass::Code),
        (b".rdata", 0x4000_0040) => Some(SectionClass::ReadOnlyData),
        (b".data", 0xC000_0040) => Some(SectionClass::Data),
        (b".bss", 0xC000_0080) => Some(SectionClass::ZeroData),
        (b".xdata", 0x4000_0040) => Some(SectionClass::UnwindInfo),
        (b".pdata", 0x4000_0040) => Some(SectionClass::ExceptionTable),
        _ => None,
    };
    if let Some(class) = class {
        return Ok(class);
    }
    let kind = if name.starts_with(b".debug") {
        DeclineKind::DebugInfo
    } else {
        DeclineKind::UnknownSection
    };
    Err(Decline::new(
        kind,
        format!(
            "section `{}` with characteristics {characteristics:#010x}",
            String::from_utf8_lossy(name)
        ),
    ))
}

/// Parses a COFF object file. Never panics, whatever the bytes.
pub fn read_coff(bytes: &[u8]) -> Result<Object<'_>, Decline> {
    let header = bytes.get(..20).ok_or_else(|| {
        Decline::new(DeclineKind::UnsupportedFormat, "shorter than a COFF header")
    })?;
    let machine = u16_at(header, 0).unwrap_or(0);
    if machine == 0 && u16_at(header, 2) == Some(0xFFFF) {
        return Err(Decline::new(
            DeclineKind::UnsupportedFormat,
            "big object (more than 65279 sections)",
        ));
    }
    if machine != MACHINE_AMD64 {
        return Err(Decline::new(
            DeclineKind::UnsupportedMachine,
            format!("machine {machine:#06x}"),
        ));
    }
    let section_count = usize::from(u16_at(header, 2).unwrap_or(0));
    let symtab_ptr = u32_at(header, 8).unwrap_or(0) as usize;
    let symbol_count = u32_at(header, 12).unwrap_or(0) as usize;
    let optional_size = usize::from(u16_at(header, 16).unwrap_or(0));
    if optional_size != 0 {
        return Err(malformed("an object file with an optional header"));
    }

    // The string table follows the symbols; its first word is its own size.
    let symtab_len = symbol_count
        .checked_mul(SYMBOL_SIZE)
        .ok_or_else(|| malformed("symbol count overflows"))?;
    let symtab = if symbol_count == 0 {
        &[][..]
    } else {
        let end = symtab_ptr
            .checked_add(symtab_len)
            .ok_or_else(|| malformed("symbol table offset overflows"))?;
        bytes
            .get(symtab_ptr..end)
            .ok_or_else(|| malformed("symbol table outside the file"))?
    };
    let strings_at = symtab_ptr.saturating_add(symtab_len);
    let strings = if symbol_count == 0 {
        &[][..]
    } else {
        let size = u32_at(bytes, strings_at).unwrap_or(4) as usize;
        bytes
            .get(strings_at..strings_at.saturating_add(size.max(4)))
            .unwrap_or(&[])
    };

    let mut object = Object {
        sections: Vec::with_capacity(section_count),
        symbols: Vec::with_capacity(symbol_count),
        relocs: Vec::new(),
    };

    // Sections.
    for index in 0..section_count {
        let at = 20 + index * SECTION_HEADER_SIZE;
        let h = bytes
            .get(at..at + SECTION_HEADER_SIZE)
            .ok_or_else(|| malformed("section table outside the file"))?;
        let name_field = &h[..8];
        let name_len = name_field.iter().position(|&b| b == 0).unwrap_or(8);
        let name = &name_field[..name_len];
        let virtual_size = u32_at(h, 8).unwrap_or(0);
        let virtual_address = u32_at(h, 12).unwrap_or(0);
        let raw_size = u32_at(h, 16).unwrap_or(0);
        let raw_ptr = u32_at(h, 20).unwrap_or(0) as usize;
        let reloc_ptr = u32_at(h, 24).unwrap_or(0) as usize;
        let line_ptr = u32_at(h, 28).unwrap_or(0);
        let reloc_count = usize::from(u16_at(h, 32).unwrap_or(0));
        let line_count = u16_at(h, 34).unwrap_or(0);
        let characteristics = u32_at(h, 36).unwrap_or(0);

        if name.first() == Some(&b'/') {
            return Err(Decline::new(
                DeclineKind::UnknownSection,
                "a section with a long name",
            ));
        }
        if characteristics & (SCN_LNK_REMOVE | SCN_LNK_INFO) != 0 {
            return Err(Decline::new(
                DeclineKind::UnknownSection,
                format!("directive section `{}`", String::from_utf8_lossy(name)),
            ));
        }
        if characteristics & SCN_LNK_NRELOC_OVFL != 0 {
            return Err(Decline::new(
                DeclineKind::Unmodelled,
                "extended relocation count",
            ));
        }
        if virtual_size != 0 || virtual_address != 0 || line_ptr != 0 || line_count != 0 {
            return Err(malformed("an object section with image fields set"));
        }
        let align_field = (characteristics & SCN_ALIGN_MASK) >> SCN_ALIGN_SHIFT;
        if align_field == 0 {
            return Err(Decline::new(
                DeclineKind::Unmodelled,
                "a section without an alignment",
            ));
        }
        let align = 1u32 << (align_field - 1);
        let class = classify(name, characteristics & !(SCN_ALIGN_MASK | SCN_LNK_COMDAT))?;
        let comdat = characteristics & SCN_LNK_COMDAT != 0;

        let data = if class == SectionClass::ZeroData {
            if raw_ptr != 0 {
                return Err(malformed("a .bss section with file contents"));
            }
            &[][..]
        } else if raw_size == 0 {
            &[][..]
        } else {
            let end = raw_ptr
                .checked_add(raw_size as usize)
                .ok_or_else(|| malformed("section offset overflows"))?;
            bytes
                .get(raw_ptr..end)
                .ok_or_else(|| malformed("section contents outside the file"))?
        };

        let first = object.relocs.len() as u32;
        if reloc_count > 0 {
            let end = reloc_ptr
                .checked_add(reloc_count * RELOC_SIZE)
                .ok_or_else(|| malformed("relocation offset overflows"))?;
            let table = bytes
                .get(reloc_ptr..end)
                .ok_or_else(|| malformed("relocations outside the file"))?;
            object.relocs.reserve(reloc_count);
            for r in table.chunks_exact(RELOC_SIZE) {
                let offset = u32_at(r, 0).unwrap_or(0);
                let symbol = u32_at(r, 4).unwrap_or(0);
                let kind = u16_at(r, 8).unwrap_or(0);
                if let Some(reloc) = decode_reloc(data, offset, symbol, kind)? {
                    object.relocs.push(reloc);
                }
            }
        }
        let last = object.relocs.len() as u32;

        object.sections.push(Section {
            name,
            class,
            align,
            data,
            size: raw_size,
            relocs: first..last,
            comdat,
        });
    }

    // Symbols. Auxiliary records keep their slot, so relocation indices stay
    // valid; the slot reads as a symbol that cannot be referenced.
    let mut comdat_defined = vec![false; section_count];
    let mut slot = 0;
    while slot < symbol_count {
        let s = &symtab[slot * SYMBOL_SIZE..(slot + 1) * SYMBOL_SIZE];
        let name_field = &s[..8];
        let name = if u32_at(name_field, 0) == Some(0) {
            let offset = u32_at(name_field, 4).unwrap_or(0) as usize;
            let rest = strings.get(offset..).unwrap_or(&[]);
            match rest.iter().position(|&b| b == 0) {
                Some(end) if offset >= 4 => &rest[..end],
                _ => return Err(malformed("symbol name outside the string table")),
            }
        } else {
            let len = name_field.iter().position(|&b| b == 0).unwrap_or(8);
            &name_field[..len]
        };
        let value = u32_at(s, 8).unwrap_or(0);
        let section = i16::from_le_bytes([s[12], s[13]]);
        let class = s[16];
        let aux = usize::from(s[17]);
        if slot + 1 + aux > symbol_count {
            return Err(malformed("auxiliary symbol records past the table"));
        }

        let (placement, external) = match class {
            SYM_CLASS_EXTERNAL | SYM_CLASS_STATIC | SYM_CLASS_LABEL => {
                let placement = match section {
                    0 if class == SYM_CLASS_EXTERNAL && value != 0 => {
                        return Err(Decline::new(
                            DeclineKind::UnknownSymbol,
                            format!("common symbol `{}`", String::from_utf8_lossy(name)),
                        ));
                    }
                    0 => SymbolSection::Undefined,
                    -1 => SymbolSection::Absolute,
                    n if n > 0 && (n as usize) <= section_count => {
                        SymbolSection::Section(u32::from(n as u16) - 1)
                    }
                    _ => return Err(malformed("symbol in a section that does not exist")),
                };
                (placement, class == SYM_CLASS_EXTERNAL)
            }
            SYM_CLASS_FILE => (SymbolSection::Absolute, false),
            other => {
                return Err(Decline::new(
                    DeclineKind::UnknownSymbol,
                    format!(
                        "symbol `{}` of storage class {other} (weak, function or debug)",
                        String::from_utf8_lossy(name)
                    ),
                ));
            }
        };

        // A section definition carries the COMDAT selection rule.
        if class == SYM_CLASS_STATIC && aux >= 1 && value == 0 {
            if let SymbolSection::Section(index) = placement {
                let sec = &object.sections[index as usize];
                if sec.comdat && !comdat_defined[index as usize] {
                    let a = &symtab[(slot + 1) * SYMBOL_SIZE..(slot + 2) * SYMBOL_SIZE];
                    let selection = a[14];
                    // 1 no duplicates, 2 any, 3 same size, 4 exact match:
                    // with a single input they all mean "keep it".
                    if !(1..=4).contains(&selection) {
                        return Err(Decline::new(
                            DeclineKind::Comdat,
                            format!(
                                "COMDAT selection {selection} in `{}`",
                                String::from_utf8_lossy(sec.name)
                            ),
                        ));
                    }
                    comdat_defined[index as usize] = true;
                }
            }
        }

        object.symbols.push(Symbol {
            name,
            section: placement,
            value,
            external,
            preemptible: false,
            size: 0,
            kind: SymbolKind::NoType,
        });
        for _ in 0..aux {
            object.symbols.push(Symbol {
                name: &[],
                section: SymbolSection::Absolute,
                value: 0,
                external: false,
                preemptible: false,
                size: 0,
                kind: SymbolKind::NoType,
            });
        }
        slot += 1 + aux;
    }
    for (index, section) in object.sections.iter().enumerate() {
        if section.comdat && !comdat_defined[index] {
            return Err(Decline::new(
                DeclineKind::Comdat,
                "a COMDAT section without a definition record",
            ));
        }
    }
    Ok(object)
}

/// Turns a COFF relocation into the neutral form, reading the addend that
/// COFF keeps in the section contents. `None` for a relocation that does
/// nothing (`IMAGE_REL_AMD64_ABSOLUTE`).
fn decode_reloc(
    data: &[u8],
    offset: u32,
    symbol: u32,
    kind: u16,
) -> Result<Option<Reloc>, Decline> {
    let at = offset as usize;
    let read32 = || {
        u32_at(data, at)
            .map(|v| i64::from(v as i32))
            .ok_or_else(|| malformed("relocation outside its section"))
    };
    let (kind, addend) = match kind {
        0 => return Ok(None),
        1 => {
            let bytes = at
                .checked_add(8)
                .and_then(|end| data.get(at..end))
                .ok_or_else(|| malformed("relocation outside its section"))?;
            let v = i64::from_le_bytes(bytes.try_into().unwrap_or([0; 8]));
            (RelocKind::Abs64, v)
        }
        3 => (RelocKind::Rva32, read32()?),
        // REL32, REL32_1 ... REL32_5: the field is followed by 0 to 5 more
        // bytes of the instruction, and is relative to the end of them.
        4..=9 => (RelocKind::PcRel32, read32()? - 4 - i64::from(kind - 4)),
        10 => {
            u16_at(data, at).ok_or_else(|| malformed("relocation outside its section"))?;
            (RelocKind::SectionIndex, 0)
        }
        11 => (RelocKind::SecRel32, read32()?),
        other => {
            return Err(Decline::new(
                DeclineKind::UnknownRelocation,
                format!("COFF AMD64 relocation type {other}"),
            ));
        }
    };
    Ok(Some(Reloc {
        offset,
        symbol,
        kind,
        addend,
    }))
}

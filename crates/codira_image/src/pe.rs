//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Writes a PE32+ AMD64 DLL from an `Object`: merges the sections into
//!   `.text`/`.rdata`/`.data`/`.pdata` (and `.reloc` when there are absolute
//!   addresses to fix up), applies the AMD64 relocations, builds the export
//!   directory and the base-relocation table, and writes the headers.
//! - The layout deliberately follows what `lld-link /DLL /NOENTRY` produces for
//!   the same object, so the two images can be compared byte for byte
//!   (`peinfo::compare`): chunks keep their input order inside each output
//!   section, `.xdata` is appended to `.rdata` after the export directory,
//!   `.bss` is appended to `.data`, code padding is `int3`.
use crate::{
    decline::{Decline, DeclineKind},
    model::{Object, RelocKind, SectionClass, SymbolSection},
};

/// Where the image asks to be loaded; what `lld-link` picks for a 64-bit DLL.
pub const IMAGE_BASE: u64 = 0x1_8000_0000;
const SECTION_ALIGNMENT: u32 = 0x1000;
const FILE_ALIGNMENT: u32 = 0x200;
const MAX_IMAGE_SIZE: u64 = 0x3000_0000;

/// The MS-DOS header and stub that every PE image starts with; the last four
/// bytes of the header (`e_lfanew`) point at the `PE\0\0` that follows.
const DOS_HEADER_AND_STUB: [u8; 120] = [
    0x4d, 0x5a, 0x78, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00,
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x54, 0x68,
    0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f,
    0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
    0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x24, 0x00, 0x00,
];
const PE_HEADERS_SIZE: usize = 120 + 4 + 20 + 240;
const SECTION_HEADER_SIZE: usize = 40;

/// Offset of the COFF header's `TimeDateStamp`, which a linker may take from
/// the clock and which comparisons therefore skip.
pub const TIME_DATE_STAMP: std::ops::Range<usize> = 128..132;

const CHARS_TEXT: u32 = 0x6000_0020;
const CHARS_RDATA: u32 = 0x4000_0040;
const CHARS_DATA: u32 = 0xC000_0040;
const CHARS_PDATA: u32 = 0x4000_0040;
const CHARS_RELOC: u32 = 0x4200_0040;

/// What to build.
#[derive(Clone, Copy, Debug)]
pub struct PeRequest<'a> {
    /// The symbols to export, by name. All must be defined by the object.
    pub exports: &'a [&'a str],
    /// The name recorded in the export directory; `lld-link` uses the file
    /// name of the output.
    pub dll_name: &'a str,
}

/// The sections the objects are merged into, in image order. The fifth
/// output section, `.reloc`, is built, not merged into.
#[derive(Clone, Copy, PartialEq, Eq)]
enum Group {
    Text = 0,
    Rdata = 1,
    Data = 2,
    Pdata = 3,
}

const RELOC_SLOT: usize = 4;

/// An output section's place in the image and in the file.
#[derive(Clone, Copy, Default)]
struct Placed {
    present: bool,
    virtual_address: u32,
    virtual_size: u32,
    raw_ptr: u32,
    raw_size: u32,
}

fn align_up(value: u64, align: u64) -> u64 {
    value.div_ceil(align) * align
}

fn out_of_range(what: &str) -> Decline {
    Decline::new(DeclineKind::OutOfRange, what)
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

fn group_of(class: SectionClass) -> Group {
    match class {
        SectionClass::Code => Group::Text,
        // `RelroData` and `EhFrame` are refused before layout; the arm only
        // keeps this exhaustive.
        SectionClass::ReadOnlyData
        | SectionClass::UnwindInfo
        | SectionClass::RelroData
        | SectionClass::EhFrame => Group::Rdata,
        SectionClass::Data | SectionClass::ZeroData => Group::Data,
        SectionClass::ExceptionTable => Group::Pdata,
    }
}

/// Which sections survive. Ordinary sections always do; a COMDAT section
/// does only if a live section, or an export, refers to it -- what the
/// linker's `/OPT:REF` does, and the reason an unused constant is dropped.
fn live_sections(object: &Object<'_>, export_symbols: &[usize]) -> Result<Vec<bool>, Decline> {
    let mut live: Vec<bool> = object.sections.iter().map(|s| !s.comdat).collect();
    if live.iter().all(|&l| l) {
        return Ok(live);
    }
    let mut work: Vec<usize> = (0..live.len()).filter(|&i| live[i]).collect();
    for &symbol in export_symbols {
        if let SymbolSection::Section(index) = object.symbols[symbol].section {
            let index = index as usize;
            if !live[index] {
                live[index] = true;
                work.push(index);
            }
        }
    }
    while let Some(index) = work.pop() {
        for reloc in object.relocs_of(index) {
            let symbol = object.symbols.get(reloc.symbol as usize).ok_or_else(|| {
                Decline::new(
                    DeclineKind::Malformed,
                    "relocation against a missing symbol",
                )
            })?;
            if let SymbolSection::Section(target) = symbol.section {
                let target = target as usize;
                if !live[target] {
                    live[target] = true;
                    work.push(target);
                }
            }
        }
    }
    Ok(live)
}

/// The size of the export directory for `names`.
fn export_table_size(dll_name: &str, names: impl Iterator<Item = usize>) -> usize {
    let mut count = 0;
    let mut strings = 0;
    for len in names {
        count += 1;
        strings += len + 1;
    }
    40 + dll_name.len() + 1 + 10 * count + strings
}

/// The export directory for `exports` (sorted by name) as it will sit at
/// `base_rva`. Layout, the same as `lld-link`'s: the directory, the DLL
/// name, the address table, the name pointers, the ordinals, the names.
fn export_table(base_rva: u32, dll_name: &str, exports: &[(&[u8], u32)]) -> Vec<u8> {
    let n = exports.len();
    let dll_at = 40;
    let functions_at = dll_at + dll_name.len() + 1;
    let names_at = functions_at + 4 * n;
    let ordinals_at = names_at + 4 * n;
    let strings_at = ordinals_at + 2 * n;
    let mut out =
        vec![0u8; export_table_size(dll_name, exports.iter().map(|(name, _)| name.len()))];
    let rva = |at: usize| base_rva + at as u32;
    put32(&mut out, 12, rva(dll_at));
    put32(&mut out, 16, 1);
    put32(&mut out, 20, n as u32);
    put32(&mut out, 24, n as u32);
    put32(&mut out, 28, rva(functions_at));
    put32(&mut out, 32, rva(names_at));
    put32(&mut out, 36, rva(ordinals_at));
    out[dll_at..dll_at + dll_name.len()].copy_from_slice(dll_name.as_bytes());
    let mut string_at = strings_at;
    for (i, (name, function_rva)) in exports.iter().enumerate() {
        put32(&mut out, functions_at + 4 * i, *function_rva);
        put32(&mut out, names_at + 4 * i, rva(string_at));
        put16(&mut out, ordinals_at + 2 * i, i as u16);
        out[string_at..string_at + name.len()].copy_from_slice(name);
        string_at += name.len() + 1;
    }
    out
}

/// The base-relocation table for the 64-bit fields at `sites` (ascending).
fn base_relocations(sites: &[u32]) -> Vec<u8> {
    let mut out = Vec::with_capacity(sites.len() * 2 + 16);
    let mut i = 0;
    while i < sites.len() {
        let page = sites[i] & !0xFFF;
        let mut j = i;
        while j < sites.len() && sites[j] & !0xFFF == page {
            j += 1;
        }
        let entries = j - i;
        let padded = entries + (entries & 1);
        out.extend_from_slice(&page.to_le_bytes());
        out.extend_from_slice(&((8 + 2 * padded) as u32).to_le_bytes());
        for &site in &sites[i..j] {
            // IMAGE_REL_BASED_DIR64 is type 10.
            out.extend_from_slice(&((10u16 << 12) | (site & 0xFFF) as u16).to_le_bytes());
        }
        if entries & 1 == 1 {
            out.extend_from_slice(&0u16.to_le_bytes());
        }
        i = j;
    }
    out
}

/// A cheap content hash for the header's time stamp: the image is then the
/// same bytes for the same input, like `/Brepro`.
fn content_hash(buf: &[u8]) -> u32 {
    const PRIME: u64 = 0x0000_0100_0000_01b3;
    let mut h: u64 = 0xcbf2_9ce4_8422_2325;
    let mut words = buf.chunks_exact(8);
    for word in &mut words {
        let word = u64::from_le_bytes(word.try_into().unwrap_or([0; 8]));
        h = (h ^ word).wrapping_mul(PRIME).rotate_left(29);
    }
    for &byte in words.remainder() {
        h = (h ^ u64::from(byte)).wrapping_mul(PRIME);
    }
    ((h >> 32) ^ h) as u32
}

/// How many bytes a relocation of this kind writes.
fn field_width(kind: RelocKind) -> u32 {
    match kind {
        RelocKind::Abs64 => 8,
        RelocKind::SectionIndex => 2,
        RelocKind::PcRel32 | RelocKind::Rva32 | RelocKind::SecRel32 | RelocKind::GotPcRel => 4,
    }
}

/// Two relocations that write the same bytes make the result depend on the
/// order they are applied in, which no compiler output does. Relocations
/// normally arrive sorted by offset, which makes this one comparison each.
fn check_fields_are_disjoint(relocs: &[crate::model::Reloc]) -> Result<(), Decline> {
    let overlap = || {
        Decline::new(
            DeclineKind::Malformed,
            "two relocations write the same bytes",
        )
    };
    let mut end = 0u64;
    let mut sorted = true;
    for reloc in relocs {
        if u64::from(reloc.offset) < end {
            sorted = false;
            break;
        }
        end = u64::from(reloc.offset) + u64::from(field_width(reloc.kind));
    }
    if sorted {
        return Ok(());
    }
    let mut fields: Vec<(u32, u32)> = relocs
        .iter()
        .map(|r| (r.offset, field_width(r.kind)))
        .collect();
    fields.sort_unstable();
    if fields
        .windows(2)
        .any(|w| u64::from(w[0].0) + u64::from(w[0].1) > u64::from(w[1].0))
    {
        return Err(overlap());
    }
    Ok(())
}

/// Gives section `index` its offset inside its output section.
fn place(
    object: &Object<'_>,
    index: usize,
    cursor: &mut u64,
    offset_in_group: &mut [u32],
) -> Result<(), Decline> {
    let section = &object.sections[index];
    if section.align > SECTION_ALIGNMENT {
        return Err(Decline::new(
            DeclineKind::Unmodelled,
            format!("section alignment {}", section.align),
        ));
    }
    let at = align_up(*cursor, u64::from(section.align));
    *cursor = at + u64::from(section.size);
    if *cursor > MAX_IMAGE_SIZE {
        return Err(out_of_range("image larger than 768 MiB"));
    }
    offset_in_group[index] = at as u32;
    Ok(())
}

/// Builds the DLL image for `object`.
pub fn write_pe_dll(object: &Object<'_>, request: &PeRequest<'_>) -> Result<Vec<u8>, Decline> {
    // Sections and relocations of other formats have no place in a PE image.
    if object
        .sections
        .iter()
        .any(|s| matches!(s.class, SectionClass::RelroData | SectionClass::EhFrame))
        || object.relocs.iter().any(|r| r.kind == RelocKind::GotPcRel)
    {
        return Err(Decline::new(
            DeclineKind::Unmodelled,
            "an object that is not a Windows object",
        ));
    }
    // ---- Exports: find the symbols. ----
    let mut wanted: Vec<&str> = request.exports.to_vec();
    wanted.sort_unstable();
    if wanted.windows(2).any(|w| w[0] == w[1]) {
        return Err(Decline::new(
            DeclineKind::MissingExport,
            "a symbol is exported twice",
        ));
    }
    let mut export_symbols: Vec<usize> = vec![usize::MAX; wanted.len()];
    for (index, symbol) in object.symbols.iter().enumerate() {
        if !symbol.external || !matches!(symbol.section, SymbolSection::Section(_)) {
            continue;
        }
        if let Ok(found) = wanted.binary_search_by(|w| w.as_bytes().cmp(symbol.name)) {
            if export_symbols[found] != usize::MAX {
                return Err(Decline::new(
                    DeclineKind::Unmodelled,
                    format!("symbol `{}` defined twice", wanted[found]),
                ));
            }
            export_symbols[found] = index;
        }
    }
    if let Some(missing) = export_symbols.iter().position(|&s| s == usize::MAX) {
        return Err(Decline::new(
            DeclineKind::MissingExport,
            format!("`{}` is not defined by the object", wanted[missing]),
        ));
    }

    let live = live_sections(object, &export_symbols)?;

    // ---- Layout inside the output sections. ----
    let section_count = object.sections.len();
    let mut offset_in_group = vec![0u32; section_count];
    let mut group_size = [0u64; 4];
    let live_ref = &live;
    // Inside an output section the linker lays out the ordinary sections in
    // input order, and after them the COMDAT ones in input order.
    let in_class = |class: SectionClass| {
        (0..section_count)
            .filter(move |&i| {
                object.sections[i].class == class && live_ref[i] && !object.sections[i].comdat
            })
            .chain((0..section_count).filter(move |&i| {
                object.sections[i].class == class && live_ref[i] && object.sections[i].comdat
            }))
    };
    let mut cursor = 0u64;
    for index in in_class(SectionClass::Code) {
        place(object, index, &mut cursor, &mut offset_in_group)?;
    }
    group_size[Group::Text as usize] = cursor;
    cursor = 0;
    for index in in_class(SectionClass::ReadOnlyData) {
        place(object, index, &mut cursor, &mut offset_in_group)?;
    }
    // The export directory follows the read-only data, then the unwind info.
    let export_size = if wanted.is_empty() {
        0
    } else {
        export_table_size(request.dll_name, wanted.iter().map(|w| w.len())) as u64
    };
    let export_offset = cursor;
    cursor += export_size;
    for index in in_class(SectionClass::UnwindInfo) {
        place(object, index, &mut cursor, &mut offset_in_group)?;
    }
    group_size[Group::Rdata as usize] = cursor;
    cursor = 0;
    for index in in_class(SectionClass::Data) {
        place(object, index, &mut cursor, &mut offset_in_group)?;
    }
    let data_raw_end = cursor;
    for index in in_class(SectionClass::ZeroData) {
        place(object, index, &mut cursor, &mut offset_in_group)?;
    }
    group_size[Group::Data as usize] = cursor;
    cursor = 0;
    for index in in_class(SectionClass::ExceptionTable) {
        place(object, index, &mut cursor, &mut offset_in_group)?;
    }
    group_size[Group::Pdata as usize] = cursor;
    if group_size[Group::Data as usize] > 0 && data_raw_end == 0 {
        return Err(Decline::new(
            DeclineKind::Unmodelled,
            "zero-initialized data without initialized data",
        ));
    }
    if group_size[Group::Text as usize] == 0 {
        return Err(Decline::new(
            DeclineKind::Unmodelled,
            "an object without code",
        ));
    }
    let pdata_size = group_size[Group::Pdata as usize];
    if pdata_size % 12 != 0 {
        return Err(Decline::new(
            DeclineKind::Malformed,
            ".pdata is not a whole number of function entries",
        ));
    }

    // ---- Count the base-relocation sites, to know the section count. ----
    let site_count: usize = (0..section_count)
        .filter(|&index| live[index])
        .map(|index| {
            object
                .relocs_of(index)
                .iter()
                .filter(|r| r.kind == RelocKind::Abs64)
                .count()
        })
        .sum();

    // ---- Section table. ----
    let mut placed = [Placed::default(); 5];
    let names: [&[u8; 8]; 5] = [
        b".text\0\0\0",
        b".rdata\0\0",
        b".data\0\0\0",
        b".pdata\0\0",
        b".reloc\0\0",
    ];
    let chars = [
        CHARS_TEXT,
        CHARS_RDATA,
        CHARS_DATA,
        CHARS_PDATA,
        CHARS_RELOC,
    ];
    // `lld-link` sizes the headers for its five output sections (including
    // `.pdata` and `.reloc`, which it drops again when they are empty).
    let headers_size = align_up(
        (PE_HEADERS_SIZE + SECTION_HEADER_SIZE * 5) as u64,
        u64::from(FILE_ALIGNMENT),
    );

    // Virtual addresses do not depend on the relocation table's size, which
    // belongs to the last section.
    let mut next_va = u64::from(SECTION_ALIGNMENT);
    for (slot, &size) in group_size.iter().enumerate() {
        if size == 0 {
            continue;
        }
        placed[slot].present = true;
        placed[slot].virtual_address = next_va as u32;
        placed[slot].virtual_size = size as u32;
        next_va += align_up(size, u64::from(SECTION_ALIGNMENT));
    }
    let group_va: [u32; 4] = [
        placed[0].virtual_address,
        placed[1].virtual_address,
        placed[2].virtual_address,
        placed[3].virtual_address,
    ];
    let rva_of_group = |g: Group| group_va[g as usize];

    // ---- Symbol addresses. ----
    let symbol_rva = |index: u32| -> Result<(u32, Option<Group>), Decline> {
        let symbol = object.symbols.get(index as usize).ok_or_else(|| {
            Decline::new(
                DeclineKind::Malformed,
                "relocation against a missing symbol",
            )
        })?;
        match symbol.section {
            SymbolSection::Section(section) => {
                let section = section as usize;
                // A symbol in a dropped section can only be reached through
                // a bug; refuse rather than point into the wrong place.
                if !live[section] {
                    return Err(Decline::new(
                        DeclineKind::Unmodelled,
                        "symbol in a dropped section",
                    ));
                }
                if symbol.value > object.sections[section].size {
                    return Err(Decline::new(
                        DeclineKind::Malformed,
                        format!(
                            "symbol `{}` lies outside its section",
                            String::from_utf8_lossy(symbol.name)
                        ),
                    ));
                }
                let g = group_of(object.sections[section].class);
                let rva = u64::from(rva_of_group(g))
                    + u64::from(offset_in_group[section])
                    + u64::from(symbol.value);
                u32::try_from(rva)
                    .map(|rva| (rva, Some(g)))
                    .map_err(|_range| out_of_range("symbol address"))
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

    // Export directory contents need the function addresses.
    let mut export_entries: Vec<(&[u8], u32)> = Vec::with_capacity(wanted.len());
    for (name, &symbol) in wanted.iter().zip(&export_symbols) {
        let inside = match object.symbols[symbol].section {
            SymbolSection::Section(section) => {
                object.symbols[symbol].value < object.sections[section as usize].size
            }
            _ => false,
        };
        if !inside {
            return Err(Decline::new(
                DeclineKind::MissingExport,
                format!("`{name}` does not point into the object"),
            ));
        }
        let symbol = u32::try_from(symbol).map_err(|_range| out_of_range("symbol index"))?;
        export_entries.push((name.as_bytes(), symbol_rva(symbol)?.0));
    }

    // Where the sites are is a function of the layout alone, so the table
    // can be built before the file is.
    let mut sites: Vec<u32> = Vec::with_capacity(site_count);
    for index in 0..section_count {
        if !live[index] {
            continue;
        }
        let g = group_of(object.sections[index].class);
        for reloc in object.relocs_of(index) {
            if reloc.kind == RelocKind::Abs64 {
                let site = u64::from(rva_of_group(g))
                    + u64::from(offset_in_group[index])
                    + u64::from(reloc.offset);
                sites.push(u32::try_from(site).map_err(|_range| out_of_range("relocation site"))?);
            }
        }
    }
    sites.sort_unstable();
    sites.dedup();
    let base_reloc_table = base_relocations(&sites);
    if !base_reloc_table.is_empty() {
        placed[RELOC_SLOT].present = true;
        placed[RELOC_SLOT].virtual_address = next_va as u32;
        placed[RELOC_SLOT].virtual_size = base_reloc_table.len() as u32;
        next_va += align_up(base_reloc_table.len() as u64, u64::from(SECTION_ALIGNMENT));
    }
    let size_of_image = align_up(next_va, u64::from(SECTION_ALIGNMENT));

    // ---- File layout. ----
    let mut file_cursor = headers_size;
    for (slot, section) in placed.iter_mut().enumerate() {
        if !section.present {
            continue;
        }
        let raw_content = if slot == Group::Data as usize {
            data_raw_end
        } else {
            u64::from(section.virtual_size)
        };
        section.raw_ptr = file_cursor as u32;
        section.raw_size = align_up(raw_content, u64::from(FILE_ALIGNMENT)) as u32;
        file_cursor += u64::from(section.raw_size);
        if file_cursor > MAX_IMAGE_SIZE {
            return Err(out_of_range("image larger than 768 MiB"));
        }
    }
    let mut out = vec![0u8; file_cursor as usize];
    let text = placed[Group::Text as usize];
    out[text.raw_ptr as usize..(text.raw_ptr + text.raw_size) as usize].fill(0xCC);

    // ---- Copy the sections in and apply the relocations. ----
    for index in 0..section_count {
        if !live[index] {
            continue;
        }
        let section = &object.sections[index];
        let g = group_of(section.class);
        let file_at = placed[g as usize].raw_ptr as usize + offset_in_group[index] as usize;
        if !section.data.is_empty() {
            out[file_at..file_at + section.data.len()].copy_from_slice(section.data);
        }
        check_fields_are_disjoint(object.relocs_of(index))?;
        for reloc in object.relocs_of(index) {
            let (target, target_group) = symbol_rva(reloc.symbol)?;
            let field = file_at + reloc.offset as usize;
            let place_rva = i64::from(rva_of_group(g))
                + i64::from(offset_in_group[index])
                + i64::from(reloc.offset);
            let width = field_width(reloc.kind);
            if u64::from(reloc.offset) + u64::from(width) > u64::from(section.size) {
                return Err(Decline::new(
                    DeclineKind::Malformed,
                    "relocation outside its section",
                ));
            }
            let target = i64::from(target);
            match reloc.kind {
                RelocKind::Abs64 => {
                    // An address that is not inside the image cannot come
                    // from a program this writer should be trusted with.
                    let rva = target
                        .checked_add(reloc.addend)
                        .filter(|rva| (0..=size_of_image as i64).contains(rva))
                        .ok_or_else(|| out_of_range("64-bit relocation outside the image"))?;
                    put64(&mut out, field, IMAGE_BASE.wrapping_add(rva as u64));
                }
                RelocKind::PcRel32 => {
                    let value = target
                        .checked_add(reloc.addend)
                        .and_then(|v| v.checked_sub(place_rva))
                        .and_then(|v| i32::try_from(v).ok())
                        .ok_or_else(|| out_of_range("32-bit PC-relative relocation"))?;
                    put32(&mut out, field, value as u32);
                }
                RelocKind::Rva32 => {
                    let value = target
                        .checked_add(reloc.addend)
                        .and_then(|v| u32::try_from(v).ok())
                        .filter(|&rva| u64::from(rva) <= size_of_image)
                        .ok_or_else(|| out_of_range("image-relative relocation"))?;
                    put32(&mut out, field, value);
                }
                RelocKind::SecRel32 => {
                    let base = target_group.map_or(0, |t| i64::from(rva_of_group(t)));
                    let value = (target - base)
                        .checked_add(reloc.addend)
                        .and_then(|v| u32::try_from(v).ok())
                        .ok_or_else(|| out_of_range("section-relative relocation"))?;
                    put32(&mut out, field, value);
                }
                RelocKind::SectionIndex => {
                    let ordinal = target_group.map_or(0, |t| {
                        1 + placed[..t as usize].iter().filter(|p| p.present).count()
                    });
                    put16(&mut out, field, ordinal as u16);
                }
                // Refused before layout.
                RelocKind::GotPcRel => {
                    return Err(Decline::new(
                        DeclineKind::UnknownRelocation,
                        "a GOT-relative relocation in a PE image",
                    ));
                }
            }
        }
    }

    // ---- Export directory. ----
    let rdata = placed[Group::Rdata as usize];
    let export_rva = rdata.virtual_address + export_offset as u32;
    if !export_entries.is_empty() {
        let table = export_table(export_rva, request.dll_name, &export_entries);
        let at = rdata.raw_ptr as usize + export_offset as usize;
        out[at..at + table.len()].copy_from_slice(&table);
    }

    // ---- The exception table must be sorted by function start. ----
    let pdata = placed[Group::Pdata as usize];
    if pdata.present {
        let start = pdata.raw_ptr as usize;
        let table = &mut out[start..start + pdata_size as usize];
        let begin = |entry: &[u8]| u32::from_le_bytes([entry[0], entry[1], entry[2], entry[3]]);
        // Each entry must describe a function of `.text` and point at unwind
        // data inside the image, or the loader's unwinder would follow it.
        let text_end = u64::from(text.virtual_address) + u64::from(text.virtual_size);
        let in_a_section = |rva: u64| {
            placed.iter().take(4).any(|p| {
                p.present
                    && rva >= u64::from(p.virtual_address)
                    && rva < u64::from(p.virtual_address) + u64::from(p.virtual_size)
            })
        };
        for entry in table.chunks_exact(12) {
            let start = u64::from(begin(entry));
            let end = u64::from(u32::from_le_bytes([entry[4], entry[5], entry[6], entry[7]]));
            let unwind = u64::from(u32::from_le_bytes([
                entry[8], entry[9], entry[10], entry[11],
            ]));
            if start < u64::from(text.virtual_address)
                || start >= end
                || end > text_end
                || !in_a_section(unwind)
            {
                return Err(Decline::new(
                    DeclineKind::Malformed,
                    "an exception table entry that does not describe a function",
                ));
            }
        }
        if !table.chunks_exact(12).map(begin).is_sorted() {
            let mut entries: Vec<[u8; 12]> = table
                .chunks_exact(12)
                .map(|e| e.try_into().unwrap_or([0; 12]))
                .collect();
            entries.sort_by_key(|e| begin(e));
            for (slot, entry) in table.chunks_exact_mut(12).zip(&entries) {
                slot.copy_from_slice(entry);
            }
        }
    }

    // ---- Base relocations. ----
    let reloc = placed[RELOC_SLOT];
    if reloc.present {
        let at = reloc.raw_ptr as usize;
        out[at..at + base_reloc_table.len()].copy_from_slice(&base_reloc_table);
    }

    // ---- Headers. ----
    out[..120].copy_from_slice(&DOS_HEADER_AND_STUB);
    out[120..124].copy_from_slice(b"PE\0\0");
    let coff = 124;
    put16(&mut out, coff, 0x8664);
    put16(
        &mut out,
        coff + 2,
        placed.iter().filter(|p| p.present).count() as u16,
    );
    put16(&mut out, coff + 16, 240);
    // DLL | EXECUTABLE_IMAGE | LARGE_ADDRESS_AWARE
    put16(&mut out, coff + 18, 0x2022);
    let opt = coff + 20;
    put16(&mut out, opt, 0x20B);
    out[opt + 2] = 14;
    let mut size_of_code = 0u64;
    let mut size_of_initialized = 0u64;
    for (p, chars) in placed.iter().zip(chars) {
        if p.present {
            if chars & 0x20 != 0 {
                size_of_code += u64::from(p.raw_size);
            } else {
                size_of_initialized += u64::from(p.raw_size);
            }
        }
    }
    put32(&mut out, opt + 4, size_of_code as u32);
    put32(&mut out, opt + 8, size_of_initialized as u32);
    put32(&mut out, opt + 20, text.virtual_address);
    put64(&mut out, opt + 24, IMAGE_BASE);
    put32(&mut out, opt + 32, SECTION_ALIGNMENT);
    put32(&mut out, opt + 36, FILE_ALIGNMENT);
    put16(&mut out, opt + 40, 6);
    put16(&mut out, opt + 48, 6);
    put32(&mut out, opt + 56, size_of_image as u32);
    put32(&mut out, opt + 60, headers_size as u32);
    // Windows GUI subsystem, as `lld-link` picks when nothing says otherwise.
    put16(&mut out, opt + 68, 2);
    // DYNAMIC_BASE | HIGH_ENTROPY_VA | NX_COMPAT
    put16(&mut out, opt + 70, 0x160);
    put64(&mut out, opt + 72, 0x10_0000);
    put64(&mut out, opt + 80, 0x1000);
    put64(&mut out, opt + 88, 0x10_0000);
    put64(&mut out, opt + 96, 0x1000);
    put32(&mut out, opt + 108, 16);
    if !export_entries.is_empty() {
        put32(&mut out, opt + 112, export_rva);
        put32(&mut out, opt + 116, export_size as u32);
    }
    if pdata.present {
        put32(&mut out, opt + 112 + 3 * 8, pdata.virtual_address);
        put32(&mut out, opt + 112 + 3 * 8 + 4, pdata.virtual_size);
    }
    if reloc.present {
        put32(&mut out, opt + 112 + 5 * 8, reloc.virtual_address);
        put32(&mut out, opt + 112 + 5 * 8 + 4, reloc.virtual_size);
    }
    let mut header_at = opt + 240;
    for (slot, p) in placed.iter().enumerate() {
        if !p.present {
            continue;
        }
        out[header_at..header_at + 8].copy_from_slice(names[slot]);
        put32(&mut out, header_at + 8, p.virtual_size);
        put32(&mut out, header_at + 12, p.virtual_address);
        put32(&mut out, header_at + 16, p.raw_size);
        put32(&mut out, header_at + 20, p.raw_ptr);
        put32(&mut out, header_at + 36, chars[slot]);
        header_at += SECTION_HEADER_SIZE;
    }
    let stamp = content_hash(&out);
    put32(&mut out, coff + 4, stamp);
    Ok(out)
}

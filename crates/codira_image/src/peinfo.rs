//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - A small reader for the PE32+ images this crate writes (and the ones LLD
//!   writes for the same input), used by the differential tests and by the
//!   compiler's self-check: `validate` checks that an image is internally
//!   consistent the way the Windows loader and `llvm-readobj` need it to be,
//!   and `compare` says where two images differ, section by section.
use crate::pe::{IMAGE_BASE, TIME_DATE_STAMP};

fn u16_at(bytes: &[u8], at: usize) -> Result<u16, String> {
    bytes
        .get(at..at + 2)
        .map(|b| u16::from_le_bytes([b[0], b[1]]))
        .ok_or_else(|| format!("read of 2 bytes at {at:#x} is outside the image"))
}

fn u32_at(bytes: &[u8], at: usize) -> Result<u32, String> {
    bytes
        .get(at..at + 4)
        .map(|b| u32::from_le_bytes([b[0], b[1], b[2], b[3]]))
        .ok_or_else(|| format!("read of 4 bytes at {at:#x} is outside the image"))
}

fn u64_at(bytes: &[u8], at: usize) -> Result<u64, String> {
    bytes
        .get(at..at + 8)
        .map(|b| u64::from_le_bytes(b.try_into().unwrap_or([0; 8])))
        .ok_or_else(|| format!("read of 8 bytes at {at:#x} is outside the image"))
}

/// One section header of an image.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SectionInfo {
    pub name: String,
    pub virtual_size: u32,
    pub virtual_address: u32,
    pub raw_size: u32,
    pub raw_ptr: u32,
    pub characteristics: u32,
}

/// The parts of an image the checks need.
#[derive(Clone, Debug)]
pub struct ImageInfo {
    pub sections: Vec<SectionInfo>,
    pub image_base: u64,
    pub size_of_image: u32,
    pub size_of_headers: u32,
    pub file_alignment: u32,
    pub section_alignment: u32,
    /// `(name, rva)`, in export-table order.
    pub exports: Vec<(String, u32)>,
    /// The DLL name recorded in the export directory, if there is one.
    pub dll_name: String,
    pub directories: [(u32, u32); 16],
    optional_at: usize,
}

impl ImageInfo {
    /// Converts an RVA to a file offset, if it lies in the raw data of a
    /// section (headers included).
    pub fn rva_to_offset(&self, rva: u32) -> Option<usize> {
        if rva < self.size_of_headers {
            return Some(rva as usize);
        }
        self.sections.iter().find_map(|s| {
            let within = rva.checked_sub(s.virtual_address)?;
            (within < s.raw_size.min(s.virtual_size.max(1)))
                .then(|| s.raw_ptr as usize + within as usize)
        })
    }

    /// Whether `rva` lies inside the mapped image.
    pub fn rva_in_image(&self, rva: u32) -> bool {
        rva < self.size_of_image
    }
}

/// Parses the headers of `bytes`.
pub fn parse(bytes: &[u8]) -> Result<ImageInfo, String> {
    if bytes.get(..2) != Some(b"MZ") {
        return Err("no MZ signature".into());
    }
    let pe = u32_at(bytes, 0x3C)? as usize;
    if bytes.get(pe..pe + 4) != Some(b"PE\0\0") {
        return Err("no PE signature".into());
    }
    let coff = pe + 4;
    if u16_at(bytes, coff)? != 0x8664 {
        return Err("not an AMD64 image".into());
    }
    let section_count = usize::from(u16_at(bytes, coff + 2)?);
    let optional_size = usize::from(u16_at(bytes, coff + 16)?);
    let optional_at = coff + 20;
    if u16_at(bytes, optional_at)? != 0x20B {
        return Err("not a PE32+ image".into());
    }
    if optional_size < 112 + 16 * 8 {
        return Err("optional header too small for 16 data directories".into());
    }
    let mut directories = [(0, 0); 16];
    for (i, dir) in directories.iter_mut().enumerate() {
        *dir = (
            u32_at(bytes, optional_at + 112 + 8 * i)?,
            u32_at(bytes, optional_at + 116 + 8 * i)?,
        );
    }
    let mut sections = Vec::with_capacity(section_count);
    for i in 0..section_count {
        let at = optional_at + optional_size + 40 * i;
        let name = bytes
            .get(at..at + 8)
            .ok_or("section table outside the image")?;
        let len = name.iter().position(|&b| b == 0).unwrap_or(8);
        sections.push(SectionInfo {
            name: String::from_utf8_lossy(&name[..len]).into_owned(),
            virtual_size: u32_at(bytes, at + 8)?,
            virtual_address: u32_at(bytes, at + 12)?,
            raw_size: u32_at(bytes, at + 16)?,
            raw_ptr: u32_at(bytes, at + 20)?,
            characteristics: u32_at(bytes, at + 36)?,
        });
    }
    let mut info = ImageInfo {
        sections,
        image_base: u64_at(bytes, optional_at + 24)?,
        size_of_image: u32_at(bytes, optional_at + 56)?,
        size_of_headers: u32_at(bytes, optional_at + 60)?,
        file_alignment: u32_at(bytes, optional_at + 36)?,
        section_alignment: u32_at(bytes, optional_at + 32)?,
        exports: Vec::new(),
        dll_name: String::new(),
        directories,
        optional_at,
    };

    // Export directory.
    let (export_rva, export_size) = directories[0];
    if export_rva != 0 {
        let at = info
            .rva_to_offset(export_rva)
            .ok_or("export directory outside the image")?;
        if export_size < 40 {
            return Err("export directory smaller than its header".into());
        }
        let name_at = info
            .rva_to_offset(u32_at(bytes, at + 12)?)
            .ok_or("DLL name outside the image")?;
        let name_len = bytes[name_at..]
            .iter()
            .position(|&b| b == 0)
            .ok_or("unterminated DLL name")?;
        info.dll_name = String::from_utf8_lossy(&bytes[name_at..name_at + name_len]).into_owned();
        let count = u32_at(bytes, at + 24)? as usize;
        let functions = info
            .rva_to_offset(u32_at(bytes, at + 28)?)
            .ok_or("export address table outside the image")?;
        let names = info
            .rva_to_offset(u32_at(bytes, at + 32)?)
            .ok_or("export name table outside the image")?;
        let ordinals = info
            .rva_to_offset(u32_at(bytes, at + 36)?)
            .ok_or("export ordinal table outside the image")?;
        let base = u32_at(bytes, at + 16)?;
        if u32_at(bytes, at + 20)? as usize != count {
            return Err("export tables of different lengths".into());
        }
        for i in 0..count {
            let name_at = info
                .rva_to_offset(u32_at(bytes, names + 4 * i)?)
                .ok_or("export name outside the image")?;
            let end = bytes[name_at..]
                .iter()
                .position(|&b| b == 0)
                .ok_or("unterminated export name")?;
            let name = String::from_utf8_lossy(&bytes[name_at..name_at + end]).into_owned();
            let ordinal = u32::from(u16_at(bytes, ordinals + 2 * i)?) + base;
            let slot = (ordinal - base) as usize;
            if slot >= count {
                return Err(format!("export ordinal {ordinal} out of range"));
            }
            let rva = u32_at(bytes, functions + 4 * slot)?;
            info.exports.push((name, rva));
        }
    }
    Ok(info)
}

/// Checks that `bytes` is a self-consistent image: aligned, non-overlapping
/// sections inside the file, exports inside the image and sorted, base
/// relocations that land on 64-bit fields pointing into the image, and an
/// exception table that is sorted and points at code and unwind data.
pub fn validate(bytes: &[u8]) -> Result<ImageInfo, String> {
    let info = parse(bytes)?;
    if info.section_alignment == 0 || info.file_alignment == 0 {
        return Err("zero alignment".into());
    }
    let mut previous_end = 0u32;
    let mut previous_raw_end = info.size_of_headers;
    if info.size_of_headers as usize > bytes.len() {
        return Err("headers larger than the file".into());
    }
    for s in &info.sections {
        if s.virtual_address % info.section_alignment != 0 {
            return Err(format!("section {} is not section-aligned", s.name));
        }
        if s.virtual_address < previous_end {
            return Err(format!("section {} overlaps the previous one", s.name));
        }
        previous_end = s.virtual_address
            + s.virtual_size.div_ceil(info.section_alignment) * info.section_alignment;
        if s.raw_size > 0 {
            if s.raw_ptr % info.file_alignment != 0 || s.raw_size % info.file_alignment != 0 {
                return Err(format!("section {} is not file-aligned", s.name));
            }
            if s.raw_ptr < previous_raw_end {
                return Err(format!(
                    "section {} overlaps the previous one in the file",
                    s.name
                ));
            }
            previous_raw_end = s.raw_ptr + s.raw_size;
            if previous_raw_end as usize > bytes.len() {
                return Err(format!("section {} runs past the end of the file", s.name));
            }
        }
    }
    if previous_end > info.size_of_image {
        return Err("SizeOfImage smaller than the sections".into());
    }

    // Exports.
    let mut last: Option<&str> = None;
    for (name, rva) in &info.exports {
        if info.rva_to_offset(*rva).is_none() {
            return Err(format!("export {name} points outside the image"));
        }
        if let Some(last) = last {
            if last >= name.as_str() {
                return Err("export names are not sorted".into());
            }
        }
        last = Some(name);
    }

    // Base relocations.
    let (reloc_rva, reloc_size) = info.directories[5];
    if reloc_rva != 0 {
        let start = info
            .rva_to_offset(reloc_rva)
            .ok_or("base relocation table outside the image")?;
        let mut at = start;
        let end = start + reloc_size as usize;
        while at < end {
            let page = u32_at(bytes, at)?;
            let block = u32_at(bytes, at + 4)? as usize;
            if block < 8 || !block.is_multiple_of(4) || at + block > end {
                return Err("malformed base relocation block".into());
            }
            for i in 0..(block - 8) / 2 {
                let entry = u16_at(bytes, at + 8 + 2 * i)?;
                match entry >> 12 {
                    0 => {}
                    10 => {
                        let site = page + u32::from(entry & 0xFFF);
                        let offset = info.rva_to_offset(site).ok_or_else(|| {
                            format!("relocation site {site:#x} outside the image")
                        })?;
                        let value = u64_at(bytes, offset)?;
                        let rva = value.wrapping_sub(info.image_base);
                        if rva > u64::from(info.size_of_image) {
                            return Err(format!(
                                "relocated field at {site:#x} holds {value:#x}, outside the image"
                            ));
                        }
                    }
                    other => return Err(format!("base relocation of unexpected type {other}")),
                }
            }
            at += block;
        }
    }

    // Exception table.
    let (pdata_rva, pdata_size) = info.directories[3];
    if pdata_rva != 0 {
        let start = info
            .rva_to_offset(pdata_rva)
            .ok_or("exception table outside the image")?;
        if pdata_size % 12 != 0 {
            return Err("exception table is not a whole number of entries".into());
        }
        let mut previous = 0u32;
        for i in 0..(pdata_size / 12) as usize {
            let begin = u32_at(bytes, start + 12 * i)?;
            let end = u32_at(bytes, start + 12 * i + 4)?;
            let unwind = u32_at(bytes, start + 12 * i + 8)?;
            if begin >= end || begin < previous {
                return Err("exception table is not sorted, or has an empty function".into());
            }
            previous = begin;
            if !info.rva_in_image(end) || info.rva_to_offset(unwind).is_none() {
                return Err("exception table entry points outside the image".into());
            }
        }
    }
    Ok(info)
}

/// Compares two images; `Ok` when they are the same apart from the header's
/// time stamp. Otherwise says where they first differ.
pub fn compare(expected: &[u8], actual: &[u8]) -> Result<(), String> {
    let mut a = expected.to_vec();
    let mut b = actual.to_vec();
    for image in [&mut a, &mut b] {
        if let Some(stamp) = image.get_mut(TIME_DATE_STAMP) {
            stamp.fill(0);
        }
    }
    if a == b {
        return Ok(());
    }
    let mut report = String::new();
    let (ia, ib) = (parse(&a), parse(&b));
    if let (Ok(ia), Ok(ib)) = (&ia, &ib) {
        if ia.sections != ib.sections {
            report.push_str(&format!(
                "section tables differ:\n  expected {:#x?}\n  actual   {:#x?}\n",
                ia.sections, ib.sections
            ));
        }
        for (what, x, y) in [
            ("ImageBase", ia.image_base, ib.image_base),
            (
                "SizeOfImage",
                u64::from(ia.size_of_image),
                u64::from(ib.size_of_image),
            ),
            (
                "SizeOfHeaders",
                u64::from(ia.size_of_headers),
                u64::from(ib.size_of_headers),
            ),
        ] {
            if x != y {
                report.push_str(&format!("{what}: expected {x:#x}, actual {y:#x}\n"));
            }
        }
        if ia.exports != ib.exports {
            report.push_str(&format!(
                "exports differ:\n  expected {:?}\n  actual   {:?}\n",
                ia.exports, ib.exports
            ));
        }
        if ia.directories != ib.directories {
            report.push_str(&format!(
                "data directories differ:\n  expected {:x?}\n  actual   {:x?}\n",
                ia.directories, ib.directories
            ));
        }
        let first = a.iter().zip(&b).position(|(x, y)| x != y);
        if let Some(first) = first {
            let place = ia
                .sections
                .iter()
                .find(|s| (s.raw_ptr as usize..(s.raw_ptr + s.raw_size) as usize).contains(&first))
                .map_or_else(
                    || format!("header offset {first:#x}"),
                    |s| format!("{} +{:#x}", s.name, first - s.raw_ptr as usize),
                );
            report.push_str(&format!(
                "first differing byte at file offset {first:#x} ({place}): expected {:#04x}, actual {:#04x}\n",
                a[first], b[first]
            ));
        }
        let _ = ia.optional_at;
    }
    if a.len() != b.len() {
        report.push_str(&format!(
            "lengths differ: expected {}, actual {}\n",
            a.len(),
            b.len()
        ));
    }
    if report.is_empty() {
        report.push_str("images differ in bytes the headers do not explain\n");
    }
    Err(report)
}

/// The image base the writer asks for, for callers that apply base
/// relocations to compare two images at a fixed address.
pub const fn default_image_base() -> u64 {
    IMAGE_BASE
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - A reader and consistency check for the ELF64 x86-64 shared objects this
//!   crate writes (and the ones LLD writes for the same input): what the
//!   dynamic loader needs to be true is checked by `validate`, and `compare`
//!   says how two shared objects differ in what a program can observe -- the
//!   symbols they export and the symbolic relocations they ask the loader for.
//!   They are not byte for byte equal: LLD merges strings and lays segments out
//!   differently.
use std::collections::BTreeSet;

fn u16_at(bytes: &[u8], at: usize) -> Result<u16, String> {
    bytes
        .get(at..at + 2)
        .map(|b| u16::from_le_bytes([b[0], b[1]]))
        .ok_or_else(|| format!("read of 2 bytes at {at:#x} is outside the file"))
}

fn u32_at(bytes: &[u8], at: usize) -> Result<u32, String> {
    bytes
        .get(at..at + 4)
        .map(|b| u32::from_le_bytes([b[0], b[1], b[2], b[3]]))
        .ok_or_else(|| format!("read of 4 bytes at {at:#x} is outside the file"))
}

fn u64_at(bytes: &[u8], at: usize) -> Result<u64, String> {
    bytes
        .get(at..at + 8)
        .map(|b| u64::from_le_bytes(b.try_into().unwrap_or([0; 8])))
        .ok_or_else(|| format!("read of 8 bytes at {at:#x} is outside the file"))
}

/// A `PT_LOAD` segment.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Segment {
    pub flags: u32,
    pub offset: u64,
    pub vaddr: u64,
    pub file_size: u64,
    pub mem_size: u64,
}

/// An exported symbol.
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct Export {
    pub name: String,
    pub kind: u8,
    pub size: u64,
}

/// A relocation the loader performs.
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct DynReloc {
    pub offset: u64,
    pub kind: u32,
    /// The symbol's name, for relocations against one.
    pub symbol: Option<String>,
    pub addend: i64,
}

/// What the loader reads from a shared object.
#[derive(Clone, Debug)]
pub struct SharedObject {
    pub segments: Vec<Segment>,
    pub exports: Vec<Export>,
    pub relocs: Vec<DynReloc>,
}

impl SharedObject {
    fn segment_of(&self, address: u64, writable: Option<bool>) -> Option<&Segment> {
        self.segments.iter().find(|s| {
            address >= s.vaddr
                && address < s.vaddr + s.mem_size
                && writable.is_none_or(|w| (s.flags & 2 != 0) == w)
        })
    }
}

/// Checks that `bytes` is a shared object the dynamic loader can load and
/// returns what it found.
pub fn validate(bytes: &[u8]) -> Result<SharedObject, String> {
    if bytes.get(..4) != Some(b"\x7fELF") || bytes.get(4) != Some(&2) || bytes.get(5) != Some(&1) {
        return Err("not a little-endian ELF64 file".into());
    }
    if u16_at(bytes, 16)? != 3 || u16_at(bytes, 18)? != 62 {
        return Err("not an x86-64 shared object".into());
    }
    let phoff = u64_at(bytes, 32)? as usize;
    let phnum = usize::from(u16_at(bytes, 56)?);
    if u16_at(bytes, 54)? != 56 {
        return Err("program header size".into());
    }
    let mut segments = Vec::new();
    let mut dynamic: Option<(u64, u64)> = None;
    let mut saw_stack = false;
    let mut previous_end = 0u64;
    for i in 0..phnum {
        let at = phoff + 56 * i;
        let kind = u32_at(bytes, at)?;
        let flags = u32_at(bytes, at + 4)?;
        let offset = u64_at(bytes, at + 8)?;
        let vaddr = u64_at(bytes, at + 16)?;
        let file_size = u64_at(bytes, at + 32)?;
        let mem_size = u64_at(bytes, at + 40)?;
        let align = u64_at(bytes, at + 48)?;
        match kind {
            1 => {
                if align == 0 || !align.is_power_of_two() || offset % align != vaddr % align {
                    return Err("PT_LOAD offset and address disagree modulo its alignment".into());
                }
                if offset + file_size > bytes.len() as u64 || file_size > mem_size {
                    return Err("PT_LOAD outside the file".into());
                }
                if vaddr < previous_end {
                    return Err("PT_LOAD segments overlap or are out of order".into());
                }
                previous_end = vaddr + mem_size;
                if flags & 1 != 0 && flags & 2 != 0 {
                    return Err("a segment that is writable and executable".into());
                }
                segments.push(Segment {
                    flags,
                    offset,
                    vaddr,
                    file_size,
                    mem_size,
                });
            }
            2 => dynamic = Some((vaddr, file_size)),
            0x6474_e551 => saw_stack = flags & 1 == 0,
            _ => {}
        }
    }
    if !saw_stack {
        return Err("no non-executable PT_GNU_STACK".into());
    }
    let mut object = SharedObject {
        segments,
        exports: Vec::new(),
        relocs: Vec::new(),
    };
    let (dyn_address, dyn_size) = dynamic.ok_or("no PT_DYNAMIC")?;
    if object.segment_of(dyn_address, Some(true)).is_none() {
        return Err("the dynamic section is not in writable memory".into());
    }

    // Offsets equal addresses for the segments this crate writes, but LLD's
    // differ, so translate through the segments.
    let loads = object.segments.clone();
    let file_offset = |address: u64| -> Result<usize, String> {
        let seg = loads
            .iter()
            .find(|s| address >= s.vaddr && address < s.vaddr + s.mem_size)
            .ok_or_else(|| format!("address {address:#x} is in no segment"))?;
        if address - seg.vaddr >= seg.file_size {
            return Err(format!("address {address:#x} is not backed by the file"));
        }
        Ok((seg.offset + address - seg.vaddr) as usize)
    };

    let mut tags: Vec<(u64, u64)> = Vec::new();
    let mut at = file_offset(dyn_address)?;
    let end = at + dyn_size as usize;
    while at + 16 <= end {
        let tag = u64_at(bytes, at)?;
        if tag == 0 {
            break;
        }
        tags.push((tag, u64_at(bytes, at + 8)?));
        at += 16;
    }
    let tag = |wanted: u64| tags.iter().find(|(t, _)| *t == wanted).map(|(_, v)| *v);
    let symtab = tag(6).ok_or("no DT_SYMTAB")?;
    let strtab = tag(5).ok_or("no DT_STRTAB")?;
    let strsz = tag(10).ok_or("no DT_STRSZ")? as usize;
    if tag(11) != Some(24) {
        return Err("DT_SYMENT is not 24".into());
    }
    if tags.iter().any(|(t, _)| matches!(*t, 22 | 20 | 3)) {
        return Err("text relocations or a PLT: not modelled".into());
    }
    let strings = {
        let at = file_offset(strtab)?;
        bytes
            .get(at..at + strsz)
            .ok_or("DT_STRTAB runs past the file")?
    };
    let name_bytes = |offset: u32| -> Result<&[u8], String> {
        let rest = strings
            .get(offset as usize..)
            .ok_or("symbol name outside DT_STRTAB")?;
        let end = rest
            .iter()
            .position(|&b| b == 0)
            .ok_or("unterminated symbol name")?;
        Ok(&rest[..end])
    };
    let name_at = |offset: u32| -> Result<String, String> {
        Ok(String::from_utf8_lossy(name_bytes(offset)?).into_owned())
    };

    // Symbols: the count comes from the hash table.
    let nsyms = if let Some(hash) = tag(4) {
        let at = file_offset(hash)?;
        let nbucket = u32_at(bytes, at)? as usize;
        let nchain = u32_at(bytes, at + 4)? as usize;
        // Every symbol must be found by walking its bucket.
        let symtab_at = file_offset(symtab)?;
        for sym in 1..nchain {
            let raw = name_bytes(u32_at(bytes, symtab_at + 24 * sym)?)?;
            let name = String::from_utf8_lossy(raw);
            let mut h: u32 = 0;
            for &c in raw {
                h = (h << 4).wrapping_add(u32::from(c));
                let g = h & 0xF000_0000;
                if g != 0 {
                    h ^= g >> 24;
                }
                h &= !g;
            }
            let mut walk = u32_at(bytes, at + 8 + 4 * (h as usize % nbucket.max(1)))? as usize;
            let mut steps = 0;
            while walk != 0 && walk != sym {
                walk = u32_at(bytes, at + 8 + 4 * nbucket + 4 * walk)? as usize;
                steps += 1;
                if steps > nchain {
                    return Err("hash chain loops".into());
                }
            }
            if walk != sym {
                return Err(format!("symbol {name} cannot be found through DT_HASH"));
            }
        }
        nchain
    } else {
        return Err("no DT_HASH (this writer emits the SysV hash table)".into());
    };
    let symtab_at = file_offset(symtab)?;
    let mut names: Vec<String> = vec![String::new()];
    for sym in 1..nsyms {
        let at = symtab_at + 24 * sym;
        let name = name_at(u32_at(bytes, at)?)?;
        let info = bytes[at + 4];
        let shndx = u16_at(bytes, at + 6)?;
        let value = u64_at(bytes, at + 8)?;
        let size = u64_at(bytes, at + 16)?;
        if shndx != 0 {
            // A symbol may sit just past the end of its section.
            if !object
                .segments
                .iter()
                .any(|s| (s.vaddr..=s.vaddr + s.mem_size).contains(&value))
            {
                return Err(format!("symbol {name} is outside every segment"));
            }
            object.exports.push(Export {
                name: name.clone(),
                kind: info & 0xF,
                size,
            });
        }
        names.push(name);
    }

    // Relocations.
    if let Some(rela) = tag(7) {
        let size = tag(8).ok_or("DT_RELA without DT_RELASZ")? as usize;
        if tag(9) != Some(24) || !size.is_multiple_of(24) {
            return Err("DT_RELAENT or DT_RELASZ".into());
        }
        let at = file_offset(rela)?;
        for i in 0..size / 24 {
            let r = at + 24 * i;
            let offset = u64_at(bytes, r)?;
            let info = u64_at(bytes, r + 8)?;
            let addend = u64_at(bytes, r + 16)? as i64;
            let (sym, kind) = ((info >> 32) as usize, info as u32);
            if object.segment_of(offset, Some(true)).is_none()
                || object.segment_of(offset + 7, Some(true)).is_none()
            {
                return Err(format!(
                    "relocation at {offset:#x} is not in writable memory"
                ));
            }
            let symbol = match kind {
                8 => {
                    // R_X86_64_RELATIVE: the target is inside the image.
                    if sym != 0
                        || object
                            .segments
                            .iter()
                            .all(|s| !(s.vaddr..=s.vaddr + s.mem_size).contains(&(addend as u64)))
                    {
                        return Err(format!(
                            "R_X86_64_RELATIVE at {offset:#x} with a bad addend {addend:#x}"
                        ));
                    }
                    None
                }
                1 | 6 => {
                    if sym == 0 || sym >= names.len() {
                        return Err(format!("relocation at {offset:#x} names symbol {sym}"));
                    }
                    Some(names[sym].clone())
                }
                other => return Err(format!("relocation type {other} is not modelled")),
            };
            object.relocs.push(DynReloc {
                offset,
                kind,
                symbol,
                addend,
            });
        }
    }
    Ok(object)
}

/// Compares what two shared objects built from the same object file let a
/// program observe: the exported symbols, and the relocations against
/// symbols. `Ok` when equal.
pub fn compare(expected: &SharedObject, actual: &SharedObject) -> Result<(), String> {
    let exports = |o: &SharedObject| o.exports.iter().cloned().collect::<BTreeSet<_>>();
    let (a, b) = (exports(expected), exports(actual));
    if a != b {
        return Err(format!(
            "exports differ: only expected {:?}, only actual {:?}",
            a.difference(&b).collect::<Vec<_>>(),
            b.difference(&a).collect::<Vec<_>>()
        ));
    }
    // Symbolic relocations by (type, symbol, addend); the offsets depend on
    // the layout.
    let symbolic = |o: &SharedObject| {
        let mut v: Vec<(u32, String, i64)> = o
            .relocs
            .iter()
            .filter_map(|r| r.symbol.clone().map(|s| (r.kind, s, r.addend)))
            .collect();
        v.sort();
        v
    };
    let (a, b) = (symbolic(expected), symbolic(actual));
    if a != b {
        return Err(format!(
            "symbolic relocations differ:\n  expected {a:?}\n  actual   {b:?}"
        ));
    }
    Ok(())
}

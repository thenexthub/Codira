//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Mutation fuzzing of the image writer with a fixed seed and a time bound:
//!   objects have relocation types flipped, sections truncated, offsets and
//!   symbol indices corrupted, and the writer must either decline or produce an
//!   image that this crate's validator and `llvm-readobj` both accept. It must
//!   never panic or loop.
//! - `llvm-readobj --unwind` is left out on purpose: a mutated object can carry
//!   unwind data that is garbage, which the writer copies as it copies every
//!   section, and `llvm-readobj` 22 crashes on some such data. The unwind data
//!   of the unmutated fixtures is checked by the golden tests, which compare it
//!   byte for byte against LLD.
mod common;

use std::{
    panic::{catch_unwind, AssertUnwindSafe},
    path::PathBuf,
    process::Command,
    time::{Duration, Instant},
};

use codira_image::{
    elfinfo, link_linux_so, link_windows_dll, peinfo, DllRequest, Outcome, SoRequest,
};

const EXPORTS: [&str; 3] = ["get_info", "get_version", "set_allocator_handle"];

/// How long to fuzz, and the seed: fixed by default so a failure repeats;
/// `CODIRA_FUZZ_SECONDS` and `CODIRA_FUZZ_SEED` let a longer, different run
/// look for more.
fn budget(default_seconds: u64) -> Duration {
    let seconds = std::env::var("CODIRA_FUZZ_SECONDS")
        .ok()
        .and_then(|s| s.parse().ok())
        .unwrap_or(default_seconds);
    Duration::from_secs(seconds)
}

fn seed(default: u64) -> u64 {
    std::env::var("CODIRA_FUZZ_SEED")
        .ok()
        .and_then(|s| s.parse().ok())
        .unwrap_or(default)
}

/// xorshift64*: deterministic, no dependencies.
struct Rng(u64);

impl Rng {
    fn next(&mut self) -> u64 {
        self.0 ^= self.0 >> 12;
        self.0 ^= self.0 << 25;
        self.0 ^= self.0 >> 27;
        self.0.wrapping_mul(0x2545_F491_4F6C_DD1D)
    }

    fn below(&mut self, n: usize) -> usize {
        (self.next() % n.max(1) as u64) as usize
    }
}

fn seeds() -> Vec<Vec<u8>> {
    let dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/fixtures");
    let mut seeds: Vec<Vec<u8>> = ["readme", "p1", "t1"]
        .iter()
        .map(|n| std::fs::read(dir.join(format!("{n}.obj"))).expect("fixture"))
        .collect();
    let (mut b, text) = common::minimal();
    let data = b.section(".data", common::DATA, &[0u8; 64]);
    let slot = b.symbol("slot", 8, data, common::EXTERNAL);
    b.reloc(text, 4, slot, 4);
    b.reloc(data, 16, slot, 1);
    seeds.push(b.build());
    seeds
}

/// Applies one random damage and says what it was.
fn mutate(rng: &mut Rng, object: &mut Vec<u8>) -> String {
    let len = object.len();
    let sections = usize::from(u16::from_le_bytes([object[2], object[3]]));
    match rng.below(9) {
        0 => {
            let at = rng.below(len);
            object[at] ^= 1 << rng.below(8);
            format!("flipped a bit at {at:#x}")
        }
        1 => {
            let at = rng.below(len);
            object[at] = rng.next() as u8;
            format!("random byte at {at:#x}")
        }
        2 => {
            let cut = rng.below(len);
            object.truncate(cut);
            format!("truncated to {cut}")
        }
        3 => {
            // A section header field: sizes, pointers, counts, flags.
            let section = rng.below(sections.max(1));
            let field = [8, 12, 16, 20, 24, 32, 36][rng.below(7)];
            let at = 20 + 40 * section + field;
            if at + 4 <= len {
                let value = match rng.below(4) {
                    0 => 0,
                    1 => u32::MAX,
                    2 => rng.next() as u32,
                    _ => (rng.next() % 4096) as u32,
                };
                object[at..at + 4].copy_from_slice(&value.to_le_bytes());
            }
            format!("section {section} header field {field}")
        }
        4 => {
            // The relocation type of some relocation: find a relocation
            // table through a section header.
            let section = rng.below(sections.max(1));
            let h = 20 + 40 * section;
            if h + 36 <= len {
                let ptr = u32::from_le_bytes(object[h + 24..h + 28].try_into().unwrap()) as usize;
                let count = usize::from(u16::from_le_bytes([object[h + 32], object[h + 33]]));
                if count > 0 && ptr + count * 10 <= len {
                    let r = ptr + 10 * rng.below(count);
                    let kind = (rng.below(20)) as u16;
                    object[r + 8..r + 10].copy_from_slice(&kind.to_le_bytes());
                    return format!("relocation type {kind} in section {section}");
                }
            }
            "no relocation to change".into()
        }
        5 => {
            let section = rng.below(sections.max(1));
            let h = 20 + 40 * section;
            if h + 36 <= len {
                let ptr = u32::from_le_bytes(object[h + 24..h + 28].try_into().unwrap()) as usize;
                let count = usize::from(u16::from_le_bytes([object[h + 32], object[h + 33]]));
                if count > 0 && ptr + count * 10 <= len {
                    let r = ptr + 10 * rng.below(count);
                    let value = if rng.below(2) == 0 {
                        rng.next() as u32
                    } else {
                        (rng.next() % 64) as u32
                    };
                    let field = if rng.below(2) == 0 { 0 } else { 4 };
                    object[r + field..r + field + 4].copy_from_slice(&value.to_le_bytes());
                    return format!("relocation field {field} of section {section} = {value}");
                }
            }
            "no relocation to change".into()
        }
        6 => {
            // The symbol table: values, sections, classes.
            let ptr = u32::from_le_bytes(object[8..12].try_into().unwrap()) as usize;
            let count = u32::from_le_bytes(object[12..16].try_into().unwrap()) as usize;
            if count > 0 && ptr + count * 18 <= len {
                let s = ptr + 18 * rng.below(count);
                let at = s + [8, 12, 16, 17][rng.below(4)];
                if at < len {
                    object[at] = rng.next() as u8;
                }
                return format!("symbol byte at {at:#x}");
            }
            "no symbols".into()
        }
        7 => {
            let n = 1 + rng.below(8);
            for _ in 0..n {
                let at = rng.below(len);
                object[at] = rng.next() as u8;
            }
            format!("{n} random bytes")
        }
        _ => {
            // Swap two ranges of the file.
            let a = rng.below(len);
            let b = rng.below(len);
            let n = rng.below(32).min(len - a.max(b));
            for i in 0..n {
                object.swap(a + i, b + i);
            }
            format!("swapped {n} bytes at {a:#x} and {b:#x}")
        }
    }
}

fn find_readobj() -> Option<PathBuf> {
    if let Ok(path) = std::env::var("LLVM_READOBJ") {
        return Some(path.into());
    }
    let mut candidates: Vec<PathBuf> = Vec::new();
    if let Ok(prefix) = std::env::var("LLVM_SYS_221_PREFIX") {
        candidates.push(PathBuf::from(prefix).join("bin"));
    }
    candidates.push("C:/llvm22/bin".into());
    if let Some(paths) = std::env::var_os("PATH") {
        candidates.extend(std::env::split_paths(&paths));
    }
    candidates.into_iter().find_map(|dir| {
        ["llvm-readobj.exe", "llvm-readobj"]
            .iter()
            .map(|name| dir.join(name))
            .find(|path| path.is_file())
    })
}

#[test]
fn mutated_objects_are_declined_or_give_images_the_tools_accept() {
    let readobj = find_readobj();
    assert!(
        readobj.is_some(),
        "llvm-readobj was not found (set LLVM_READOBJ); this test checks images against it"
    );
    let readobj = readobj.unwrap();
    let scratch = std::env::temp_dir().join(format!("codira_image_fuzz_{}", std::process::id()));
    std::fs::create_dir_all(&scratch).unwrap();

    let seeds = seeds();
    let mut rng = Rng(seed(0x00C0_D1AA_2026_1015));
    let started = Instant::now();
    let budget = budget(5);
    let (mut tried, mut accepted, mut declined, mut checked_by_tool) = (0u32, 0u32, 0u32, 0u32);

    while started.elapsed() < budget
        && (tried < 60_000 || std::env::var_os("CODIRA_FUZZ_SECONDS").is_some())
    {
        tried += 1;
        let mut object = seeds[rng.below(seeds.len())].clone();
        let mut what = Vec::new();
        for _ in 0..1 + rng.below(3) {
            if object.len() < 24 {
                break;
            }
            what.push(mutate(&mut rng, &mut object));
        }
        let outcome = catch_unwind(AssertUnwindSafe(|| {
            link_windows_dll(&DllRequest {
                object: &object,
                exports: &EXPORTS,
                dll_name: "fuzz.codiralib",
                debug_info: false,
                has_native_code: false,
            })
        }));
        let Ok(outcome) = outcome else {
            let path = scratch.join(format!("panic_{tried}.obj"));
            std::fs::write(&path, &object).unwrap();
            panic!("the writer panicked on mutant {tried} ({what:?}); object saved to {path:?}");
        };
        match outcome {
            Outcome::Declined(_) => declined += 1,
            Outcome::Image(image) => {
                accepted += 1;
                if let Err(e) = peinfo::validate(&image) {
                    let path = scratch.join(format!("bad_{tried}.obj"));
                    std::fs::write(&path, &object).unwrap();
                    panic!("mutant {tried} ({what:?}) gave an invalid image: {e}; object saved to {path:?}");
                }
                if accepted % 25 == 1 {
                    let path = scratch.join("image.dll");
                    std::fs::write(&path, &image).unwrap();
                    let out = Command::new(&readobj)
                        .args([
                            "--file-headers",
                            "--sections",
                            "--coff-exports",
                            "--coff-basereloc",
                            "--coff-imports",
                        ])
                        .arg(&path)
                        .output()
                        .expect("llvm-readobj runs");
                    assert!(
                        out.status.success() && out.stderr.is_empty(),
                        "llvm-readobj rejected the image of mutant {tried} ({what:?}):\n{}",
                        String::from_utf8_lossy(&out.stderr)
                    );
                    checked_by_tool += 1;
                }
            }
        }
    }
    let _ = std::fs::remove_dir_all(&scratch);
    eprintln!(
        "fuzz: {tried} mutants, {accepted} images ({checked_by_tool} checked by llvm-readobj), \
         {declined} declined"
    );
    assert!(
        tried >= 2_000,
        "only {tried} mutants fit in the time budget"
    );
    assert!(
        accepted > 100,
        "too few mutants produced an image to test anything"
    );
    assert!(
        declined > 100,
        "too few mutants were declined to test anything"
    );
    assert!(checked_by_tool >= 5);
}

/// Damage aimed at the structures of an ELF object.
fn mutate_elf(rng: &mut Rng, object: &mut Vec<u8>) -> String {
    let len = object.len();
    let shoff = u64::from_le_bytes(object[0x28..0x30].try_into().unwrap_or([0; 8])) as usize;
    let shnum = usize::from(u16::from_le_bytes([object[0x3C], object[0x3D]]));
    match rng.below(7) {
        0 => {
            let at = rng.below(len);
            object[at] ^= 1 << rng.below(8);
            format!("flipped a bit at {at:#x}")
        }
        1 => {
            let at = rng.below(len);
            object[at] = rng.next() as u8;
            format!("random byte at {at:#x}")
        }
        2 => {
            let cut = rng.below(len);
            object.truncate(cut);
            format!("truncated to {cut}")
        }
        3 | 4 => {
            // A field of a section header: type, flags, offset, size, link,
            // info, alignment, entry size.
            let section = rng.below(shnum.max(1));
            let field = [4, 8, 24, 32, 40, 44, 48, 56][rng.below(8)];
            let at = shoff + 64 * section + field;
            if at + 8 <= len {
                let value = match rng.below(4) {
                    0 => 0,
                    1 => u64::MAX,
                    2 => rng.next(),
                    _ => rng.next() % 4096,
                };
                let width = if field == 4 || field == 40 || field == 44 {
                    4
                } else {
                    8
                };
                object[at..at + width].copy_from_slice(&value.to_le_bytes()[..width]);
            }
            format!("section {section} header field {field}")
        }
        5 => {
            // A byte inside the section header table.
            let at = shoff + rng.below((64 * shnum).max(1));
            if at < len {
                object[at] = rng.next() as u8;
            }
            format!("byte {at:#x} of the section header table")
        }
        _ => {
            let n = 1 + rng.below(6);
            for _ in 0..n {
                let at = rng.below(len);
                object[at] = rng.next() as u8;
            }
            format!("{n} random bytes")
        }
    }
}

#[test]
fn mutated_elf_objects_are_declined_or_give_libraries_the_tools_accept() {
    let readobj = find_readobj().expect("llvm-readobj was not found (set LLVM_READOBJ)");
    let scratch =
        std::env::temp_dir().join(format!("codira_image_fuzz_elf_{}", std::process::id()));
    std::fs::create_dir_all(&scratch).unwrap();
    let dir = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/elf_corpus");
    let seeds: Vec<Vec<u8>> = ["p1", "c000", "c006", "c012", "c023"]
        .iter()
        .map(|n| std::fs::read(dir.join(format!("{n}.o"))).expect("fixture"))
        .collect();

    let mut rng = Rng(seed(0x00E1_F0BA_2026_1015));
    let started = Instant::now();
    let budget = budget(6);
    let (mut tried, mut accepted, mut declined, mut checked_by_tool) = (0u32, 0u32, 0u32, 0u32);
    while started.elapsed() < budget
        && (tried < 60_000 || std::env::var_os("CODIRA_FUZZ_SECONDS").is_some())
    {
        tried += 1;
        let mut object = seeds[rng.below(seeds.len())].clone();
        let mut what = Vec::new();
        for _ in 0..1 + rng.below(3) {
            if object.len() < 64 {
                break;
            }
            what.push(mutate_elf(&mut rng, &mut object));
        }
        let outcome = catch_unwind(AssertUnwindSafe(|| {
            link_linux_so(&SoRequest {
                object: &object,
                debug_info: false,
                has_native_code: false,
            })
        }));
        let Ok(outcome) = outcome else {
            let path = scratch.join(format!("panic_{tried}.o"));
            std::fs::write(&path, &object).unwrap();
            panic!("the writer panicked on mutant {tried} ({what:?}); object saved to {path:?}");
        };
        match outcome {
            Outcome::Declined(_) => declined += 1,
            Outcome::Image(image) => {
                accepted += 1;
                if let Err(e) = elfinfo::validate(&image) {
                    let path = scratch.join(format!("bad_{tried}.o"));
                    std::fs::write(&path, &object).unwrap();
                    panic!("mutant {tried} ({what:?}) gave an invalid library: {e}; object saved to {path:?}");
                }
                if accepted % 25 == 1 {
                    let path = scratch.join("image.so");
                    std::fs::write(&path, &image).unwrap();
                    let out = Command::new(&readobj)
                        .args([
                            "--file-headers",
                            "--program-headers",
                            "--sections",
                            "--dyn-symbols",
                            "--dyn-relocations",
                            "--dynamic-table",
                        ])
                        .arg(&path)
                        .output()
                        .expect("llvm-readobj runs");
                    assert!(
                        out.status.success() && out.stderr.is_empty(),
                        "llvm-readobj rejected the library of mutant {tried} ({what:?}):
{}",
                        String::from_utf8_lossy(&out.stderr)
                    );
                    checked_by_tool += 1;
                }
            }
        }
    }
    let _ = std::fs::remove_dir_all(&scratch);
    eprintln!(
        "elf fuzz: {tried} mutants, {accepted} libraries ({checked_by_tool} checked by llvm-readobj),          {declined} declined"
    );
    assert!(tried >= 500, "only {tried} mutants fit in the time budget");
    assert!(
        accepted > 50,
        "too few mutants produced a library to test anything"
    );
    assert!(
        declined > 50,
        "too few mutants were declined to test anything"
    );
    assert!(checked_by_tool >= 3);
}

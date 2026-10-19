//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Builds small relocatable objects for the library-mode tests of Funis, so that the
//!   tests need no compiler, and reads back what the linker made of them.

#![allow(dead_code)]

use std::ffi::OsString;
use std::path::{Path, PathBuf};

use object::write::{Object, Relocation, StandardSection, Symbol, SymbolSection};
use object::{
    Architecture, BinaryFormat, Endianness, RelocationEncoding, RelocationFlags, RelocationKind,
    SymbolFlags, SymbolKind, SymbolScope,
};

/// The machine code of `f`, which returns 42, and `g`, which calls `f`, and the relocation of
/// that call, for each architecture.
struct Code {
    f: &'static [u8],
    g: &'static [u8],
    /// Offset of the call's relocation within `g`.
    call_at: u64,
    /// The relocation type of the call (`R_*` value) and its addend.
    call_reloc: u32,
    call_addend: i64,
    /// The 64-bit absolute relocation type.
    abs64_reloc: u32,
}

fn elf_code(arch: Architecture) -> Code {
    match arch {
        Architecture::X86_64 => Code {
            f: &[0xb8, 0x2a, 0, 0, 0, 0xc3],
            // call f; ret
            g: &[0xe8, 0, 0, 0, 0, 0xc3],
            call_at: 1,
            call_reloc: 4, // R_X86_64_PLT32
            call_addend: -4,
            abs64_reloc: 1, // R_X86_64_64
        },
        Architecture::Aarch64 => Code {
            // mov w0, #42; ret
            f: &[0x40, 0x05, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6],
            // bl f; ret
            g: &[0, 0, 0, 0x94, 0xc0, 0x03, 0x5f, 0xd6],
            call_at: 0,
            call_reloc: 283, // R_AARCH64_CALL26
            call_addend: 0,
            abs64_reloc: 257, // R_AARCH64_ABS64
        },
        Architecture::Riscv64 => Code {
            // li a0, 42; ret
            f: &[0x13, 0x05, 0xa0, 0x02, 0x67, 0x80, 0, 0],
            // auipc ra, 0; jalr ra; ret
            g: &[0x97, 0, 0, 0, 0xe7, 0x80, 0, 0, 0x67, 0x80, 0, 0],
            call_at: 0,
            call_reloc: 19, // R_RISCV_CALL_PLT
            call_addend: 0,
            abs64_reloc: 2, // R_RISCV_64
        },
        _ => unreachable!("no test code for {arch:?}"),
    }
}

/// An ELF relocatable object that defines the exported function `f` (returns 42), the exported
/// function `<prefix>_g` (calls `f`), a pointer to `f` in `.data` (`<prefix>_ptr`) and
/// `ballast` bytes of other data. `ballast` makes inputs big enough to be mapped, not read.
pub fn elf_object(arch: Architecture, prefix: &str, define_f: bool, ballast: usize) -> Vec<u8> {
    let code = elf_code(arch);
    let mut obj = Object::new(BinaryFormat::Elf, arch, Endianness::Little);
    let text = obj.section_id(StandardSection::Text);
    let data = obj.section_id(StandardSection::Data);

    let f = if define_f {
        let offset = obj.append_section_data(text, code.f, 4);
        obj.add_symbol(Symbol {
            name: b"f".to_vec(),
            value: offset,
            size: code.f.len() as u64,
            kind: SymbolKind::Text,
            scope: SymbolScope::Dynamic,
            weak: false,
            section: SymbolSection::Section(text),
            flags: SymbolFlags::None,
        })
    } else {
        obj.add_symbol(Symbol {
            name: b"f".to_vec(),
            value: 0,
            size: 0,
            kind: SymbolKind::Text,
            scope: SymbolScope::Unknown,
            weak: false,
            section: SymbolSection::Undefined,
            flags: SymbolFlags::None,
        })
    };

    let g_offset = obj.append_section_data(text, code.g, 4);
    obj.add_symbol(Symbol {
        name: format!("{prefix}_g").into_bytes(),
        value: g_offset,
        size: code.g.len() as u64,
        kind: SymbolKind::Text,
        scope: SymbolScope::Dynamic,
        weak: false,
        section: SymbolSection::Section(text),
        flags: SymbolFlags::None,
    });
    obj.add_relocation(
        text,
        Relocation {
            offset: g_offset + code.call_at,
            symbol: f,
            addend: code.call_addend,
            flags: RelocationFlags::Elf { r_type: code.call_reloc },
        },
    )
    .unwrap();

    // A pointer to `f` in data: a dynamic relocation in a shared object.
    let ptr_offset = obj.append_section_data(data, &[0; 8], 8);
    obj.add_symbol(Symbol {
        name: format!("{prefix}_ptr").into_bytes(),
        value: ptr_offset,
        size: 8,
        kind: SymbolKind::Data,
        scope: SymbolScope::Dynamic,
        weak: false,
        section: SymbolSection::Section(data),
        flags: SymbolFlags::None,
    });
    obj.add_relocation(
        data,
        Relocation {
            offset: ptr_offset,
            symbol: f,
            addend: 0,
            flags: RelocationFlags::Elf { r_type: code.abs64_reloc },
        },
    )
    .unwrap();

    if ballast > 0 {
        let blob: Vec<u8> = (0..ballast).map(|i| (i * 31 + 7) as u8).collect();
        let off = obj.append_section_data(data, &blob, 16);
        obj.add_symbol(Symbol {
            name: format!("{prefix}_ballast").into_bytes(),
            value: off,
            size: ballast as u64,
            kind: SymbolKind::Data,
            scope: SymbolScope::Dynamic,
            weak: false,
            section: SymbolSection::Section(data),
            flags: SymbolFlags::None,
        });
    }
    obj.write().unwrap()
}

/// A Mach-O relocatable object for `arch` (arm64 or x86-64) that defines `f` (returns 42) and
/// `<prefix>_g`, which calls it, and a pointer to `f` in `__data`.
pub fn macho_object(arch: Architecture, prefix: &str, ballast: usize) -> Vec<u8> {
    let (f_code, g_code, call_at, call_flags, abs_flags): (&[u8], &[u8], u64, RelocationFlags, _) =
        match arch {
            Architecture::Aarch64 => (
                &[0x40, 0x05, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6],
                &[0, 0, 0, 0x94, 0xc0, 0x03, 0x5f, 0xd6],
                0,
                RelocationFlags::MachO { r_type: 2, r_pcrel: true, r_length: 2 }, // ARM64_RELOC_BRANCH26
                RelocationFlags::MachO { r_type: 0, r_pcrel: false, r_length: 3 }, // ARM64_RELOC_UNSIGNED
            ),
            Architecture::X86_64 => (
                &[0xb8, 0x2a, 0, 0, 0, 0xc3],
                &[0xe8, 0, 0, 0, 0, 0xc3],
                1,
                RelocationFlags::MachO { r_type: 2, r_pcrel: true, r_length: 2 }, // X86_64_RELOC_BRANCH
                RelocationFlags::MachO { r_type: 0, r_pcrel: false, r_length: 3 }, // X86_64_RELOC_UNSIGNED
            ),
            _ => unreachable!(),
        };
    let _ = (RelocationKind::Absolute, RelocationEncoding::Generic);
    let mut obj = Object::new(BinaryFormat::MachO, arch, Endianness::Little);
    let text = obj.section_id(StandardSection::Text);
    let data = obj.section_id(StandardSection::Data);
    let f_off = obj.append_section_data(text, f_code, 4);
    let f = obj.add_symbol(Symbol {
        name: b"f".to_vec(),
        value: f_off,
        size: f_code.len() as u64,
        kind: SymbolKind::Text,
        scope: SymbolScope::Dynamic,
        weak: false,
        section: SymbolSection::Section(text),
        flags: SymbolFlags::None,
    });
    let g_off = obj.append_section_data(text, g_code, 4);
    obj.add_symbol(Symbol {
        name: format!("{prefix}_g").into_bytes(),
        value: g_off,
        size: g_code.len() as u64,
        kind: SymbolKind::Text,
        scope: SymbolScope::Dynamic,
        weak: false,
        section: SymbolSection::Section(text),
        flags: SymbolFlags::None,
    });
    obj.add_relocation(
        text,
        Relocation { offset: g_off + call_at, symbol: f, addend: 0, flags: call_flags },
    )
    .unwrap();
    let p_off = obj.append_section_data(data, &[0; 8], 8);
    obj.add_symbol(Symbol {
        name: format!("{prefix}_ptr").into_bytes(),
        value: p_off,
        size: 8,
        kind: SymbolKind::Data,
        scope: SymbolScope::Dynamic,
        weak: false,
        section: SymbolSection::Section(data),
        flags: SymbolFlags::None,
    });
    obj.add_relocation(data, Relocation { offset: p_off, symbol: f, addend: 0, flags: abs_flags })
        .unwrap();
    if ballast > 0 {
        let blob: Vec<u8> = (0..ballast).map(|i| (i * 31 + 7) as u8).collect();
        let off = obj.append_section_data(data, &blob, 16);
        obj.add_symbol(Symbol {
            name: format!("{prefix}_ballast").into_bytes(),
            value: off,
            size: ballast as u64,
            kind: SymbolKind::Data,
            scope: SymbolScope::Dynamic,
            weak: false,
            section: SymbolSection::Section(data),
            flags: SymbolFlags::None,
        });
    }
    obj.write().unwrap()
}

/// A scratch directory that is removed on drop.
pub struct Dir(pub PathBuf);

impl Dir {
    pub fn new(tag: &str) -> Self {
        let path = std::env::temp_dir().join(format!(
            "funis-test-{tag}-{}-{:?}",
            std::process::id(),
            std::thread::current().id()
        ));
        let _ = std::fs::remove_dir_all(&path);
        std::fs::create_dir_all(&path).unwrap();
        Dir(path)
    }

    pub fn file(&self, name: &str) -> PathBuf {
        self.0.join(name)
    }

    pub fn write(&self, name: &str, bytes: &[u8]) -> PathBuf {
        let path = self.file(name);
        std::fs::write(&path, bytes).unwrap();
        path
    }
}

impl Drop for Dir {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

pub fn args<I, S>(items: I) -> Vec<OsString>
where
    I: IntoIterator<Item = S>,
    S: AsRef<std::ffi::OsStr>,
{
    items.into_iter().map(|s| s.as_ref().to_os_string()).collect()
}

/// `funis --shared -o out inputs...`
pub fn shared_args(out: &Path, inputs: &[&Path]) -> Vec<OsString> {
    let mut v = args(["funis", "--shared", "-o"]);
    v.push(out.into());
    v.extend(inputs.iter().map(|p| p.as_os_str().to_os_string()));
    v
}

/// The names and addresses of the dynamic symbols a shared object defines, sorted.
pub fn exported(bytes: &[u8]) -> Vec<(String, u64)> {
    use object::Object as _;
    let file = object::File::parse(bytes).expect("the output is an object file");
    let mut v: Vec<_> = file
        .exports()
        .expect("exports")
        .iter()
        .map(|e| (String::from_utf8_lossy(e.name()).into_owned(), e.address()))
        .collect();
    v.sort();
    v
}

/// Process-wide resource use, to check that repeated links do not grow it.
#[derive(Clone, Copy, Debug)]
pub struct Usage {
    /// Committed private memory, in bytes.
    pub private_bytes: u64,
    /// Open handles (Windows) or file descriptors (Linux).
    pub handles: u32,
    /// Threads of this process.
    pub threads: u32,
}

#[cfg(windows)]
pub fn usage() -> Usage {
    use std::ffi::c_void;
    #[repr(C)]
    struct MemoryCounters {
        cb: u32,
        page_fault_count: u32,
        peak_working_set: usize,
        working_set: usize,
        quota_peak_paged: usize,
        quota_paged: usize,
        quota_peak_non_paged: usize,
        quota_non_paged: usize,
        pagefile_usage: usize,
        peak_pagefile_usage: usize,
        private_usage: usize,
    }
    #[repr(C)]
    struct ThreadEntry {
        size: u32,
        usage: u32,
        thread_id: u32,
        owner_process_id: u32,
        base_pri: i32,
        delta_pri: i32,
        flags: u32,
    }
    #[link(name = "kernel32")]
    unsafe extern "system" {
        fn GetCurrentProcess() -> *mut c_void;
        fn GetCurrentProcessId() -> u32;
        fn K32GetProcessMemoryInfo(h: *mut c_void, counters: *mut MemoryCounters, cb: u32) -> i32;
        fn GetProcessHandleCount(h: *mut c_void, count: *mut u32) -> i32;
        fn CreateToolhelp32Snapshot(flags: u32, pid: u32) -> *mut c_void;
        fn Thread32First(snapshot: *mut c_void, entry: *mut ThreadEntry) -> i32;
        fn Thread32Next(snapshot: *mut c_void, entry: *mut ThreadEntry) -> i32;
        fn CloseHandle(h: *mut c_void) -> i32;
    }
    // SAFETY: plain Win32 queries with correctly sized out-parameters.
    unsafe {
        let process = GetCurrentProcess();
        let mut counters: MemoryCounters = std::mem::zeroed();
        counters.cb = size_of::<MemoryCounters>() as u32;
        assert_ne!(K32GetProcessMemoryInfo(process, &mut counters, counters.cb), 0);
        let mut handles = 0u32;
        assert_ne!(GetProcessHandleCount(process, &mut handles), 0);
        let pid = GetCurrentProcessId();
        let snapshot = CreateToolhelp32Snapshot(4, 0);
        let mut threads = 0u32;
        let mut entry: ThreadEntry = std::mem::zeroed();
        entry.size = size_of::<ThreadEntry>() as u32;
        let mut more = Thread32First(snapshot, &mut entry);
        while more != 0 {
            if entry.owner_process_id == pid {
                threads += 1;
            }
            more = Thread32Next(snapshot, &mut entry);
        }
        CloseHandle(snapshot);
        Usage { private_bytes: counters.private_usage as u64, handles, threads }
    }
}

#[cfg(unix)]
pub fn usage() -> Usage {
    let status = std::fs::read_to_string("/proc/self/status").unwrap_or_default();
    let field = |name: &str| -> u64 {
        status
            .lines()
            .find_map(|l| l.strip_prefix(name))
            .and_then(|v| v.split_whitespace().next())
            .and_then(|v| v.parse().ok())
            .unwrap_or(0)
    };
    let handles = std::fs::read_dir("/proc/self/fd").map_or(0, |d| d.count() as u32);
    Usage {
        // Resident anonymous memory is the closest to "committed private".
        private_bytes: field("RssAnon:") * 1024,
        handles,
        threads: field("Threads:") as u32,
    }
}

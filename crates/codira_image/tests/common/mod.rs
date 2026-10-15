//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - A small builder for COFF object files, so tests can state exactly the
//!   object they mean without LLVM.
#![allow(dead_code)]

pub const TEXT: u32 = 0x6050_0020;
pub const RDATA: u32 = 0x4050_0040;
pub const DATA: u32 = 0xC030_0040;
pub const BSS: u32 = 0xC050_0080;
pub const XDATA: u32 = 0x4030_0040;
pub const PDATA: u32 = 0x4030_0040;
pub const COMDAT: u32 = 0x1000;

pub const EXTERNAL: u8 = 2;
pub const STATIC: u8 = 3;

pub struct Section {
    pub name: String,
    pub characteristics: u32,
    pub data: Vec<u8>,
    /// Size of a section without contents (`.bss`).
    pub bss_size: u32,
    /// `(offset, symbol index, type)`
    pub relocs: Vec<(u32, u32, u16)>,
}

pub struct Symbol {
    pub name: String,
    pub value: u32,
    pub section: i16,
    pub class: u8,
    pub aux: Vec<[u8; 18]>,
}

#[derive(Default)]
pub struct Builder {
    pub machine: u16,
    pub sections: Vec<Section>,
    pub symbols: Vec<Symbol>,
}

impl Builder {
    pub fn new() -> Self {
        Self {
            machine: 0x8664,
            ..Self::default()
        }
    }

    /// Adds a section and returns its one-based number.
    pub fn section(&mut self, name: &str, characteristics: u32, data: &[u8]) -> i16 {
        self.sections.push(Section {
            name: name.into(),
            characteristics,
            data: data.to_vec(),
            bss_size: 0,
            relocs: Vec::new(),
        });
        self.sections.len() as i16
    }

    pub fn bss(&mut self, size: u32) -> i16 {
        let n = self.section(".bss", BSS, &[]);
        self.sections[n as usize - 1].bss_size = size;
        n
    }

    pub fn reloc(&mut self, section: i16, offset: u32, symbol: u32, kind: u16) {
        self.sections[section as usize - 1]
            .relocs
            .push((offset, symbol, kind));
    }

    /// Adds a symbol and returns its table index (counting auxiliary slots).
    pub fn symbol(&mut self, name: &str, value: u32, section: i16, class: u8) -> u32 {
        self.symbol_with_aux(name, value, section, class, Vec::new())
    }

    pub fn symbol_with_aux(
        &mut self,
        name: &str,
        value: u32,
        section: i16,
        class: u8,
        aux: Vec<[u8; 18]>,
    ) -> u32 {
        let index: usize = self.symbols.iter().map(|s| 1 + s.aux.len()).sum();
        self.symbols.push(Symbol {
            name: name.into(),
            value,
            section,
            class,
            aux,
        });
        index as u32
    }

    pub fn build(&self) -> Vec<u8> {
        let mut strings: Vec<u8> = Vec::new();
        let mut symtab: Vec<u8> = Vec::new();
        let mut symbol_slots = 0u32;
        for s in &self.symbols {
            let mut name = [0u8; 8];
            if s.name.len() <= 8 {
                name[..s.name.len()].copy_from_slice(s.name.as_bytes());
            } else {
                let offset = 4 + strings.len() as u32;
                strings.extend_from_slice(s.name.as_bytes());
                strings.push(0);
                name[4..].copy_from_slice(&offset.to_le_bytes());
            }
            symtab.extend_from_slice(&name);
            symtab.extend_from_slice(&s.value.to_le_bytes());
            symtab.extend_from_slice(&s.section.to_le_bytes());
            symtab.extend_from_slice(&0u16.to_le_bytes());
            symtab.push(s.class);
            symtab.push(s.aux.len() as u8);
            for aux in &s.aux {
                symtab.extend_from_slice(aux);
            }
            symbol_slots += 1 + s.aux.len() as u32;
        }

        let headers = 20 + 40 * self.sections.len();
        let mut body: Vec<u8> = Vec::new();
        let mut table: Vec<u8> = Vec::new();
        for s in &self.sections {
            let raw_ptr = if s.data.is_empty() {
                0
            } else {
                (headers + body.len()) as u32
            };
            body.extend_from_slice(&s.data);
            let reloc_ptr = if s.relocs.is_empty() {
                0
            } else {
                (headers + body.len()) as u32
            };
            for &(offset, symbol, kind) in &s.relocs {
                body.extend_from_slice(&offset.to_le_bytes());
                body.extend_from_slice(&symbol.to_le_bytes());
                body.extend_from_slice(&kind.to_le_bytes());
            }
            let mut name = [0u8; 8];
            name[..s.name.len().min(8)].copy_from_slice(&s.name.as_bytes()[..s.name.len().min(8)]);
            table.extend_from_slice(&name);
            table.extend_from_slice(&0u32.to_le_bytes());
            table.extend_from_slice(&0u32.to_le_bytes());
            let raw_size = if s.data.is_empty() {
                s.bss_size
            } else {
                s.data.len() as u32
            };
            table.extend_from_slice(&raw_size.to_le_bytes());
            table.extend_from_slice(&raw_ptr.to_le_bytes());
            table.extend_from_slice(&reloc_ptr.to_le_bytes());
            table.extend_from_slice(&0u32.to_le_bytes());
            table.extend_from_slice(&(s.relocs.len() as u16).to_le_bytes());
            table.extend_from_slice(&0u16.to_le_bytes());
            table.extend_from_slice(&s.characteristics.to_le_bytes());
        }

        let symtab_ptr = if self.symbols.is_empty() {
            0
        } else {
            (headers + body.len()) as u32
        };
        let mut out = Vec::new();
        out.extend_from_slice(&self.machine.to_le_bytes());
        out.extend_from_slice(&(self.sections.len() as u16).to_le_bytes());
        out.extend_from_slice(&0u32.to_le_bytes());
        out.extend_from_slice(&symtab_ptr.to_le_bytes());
        out.extend_from_slice(&symbol_slots.to_le_bytes());
        out.extend_from_slice(&0u16.to_le_bytes());
        out.extend_from_slice(&0u16.to_le_bytes());
        out.extend_from_slice(&table);
        out.extend_from_slice(&body);
        out.extend_from_slice(&symtab);
        out.extend_from_slice(&(4 + strings.len() as u32).to_le_bytes());
        out.extend_from_slice(&strings);
        out
    }
}

/// The auxiliary record of a section-definition symbol.
pub fn section_definition(length: u32, selection: u8) -> [u8; 18] {
    let mut aux = [0u8; 18];
    aux[..4].copy_from_slice(&length.to_le_bytes());
    aux[14] = selection;
    aux
}

/// A module that exports the three names the runtime wants, plus whatever
/// `fill` adds, with zeroed function bodies and one call-site slot.
pub fn minimal() -> (Builder, i16) {
    let mut b = Builder::new();
    let text = b.section(".text", TEXT, &[0u8; 64]);
    b.symbol(".text", 0, text, STATIC);
    b.symbol("get_info", 0, text, EXTERNAL);
    b.symbol("get_version", 16, text, EXTERNAL);
    b.symbol("set_allocator_handle", 32, text, EXTERNAL);
    (b, text)
}

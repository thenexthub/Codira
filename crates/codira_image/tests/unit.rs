//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Tests the image writer on objects built by hand: every relocation it
//!   models is checked against the value worked out independently, and every
//!   kind of input it must refuse is refused with the right reason.
mod common;

use codira_image::{link_windows_dll, peinfo, DeclineKind, DllRequest, Outcome};
use common::{
    minimal, section_definition, Builder, COMDAT, DATA, EXTERNAL, PDATA, RDATA, STATIC, XDATA,
};

const EXPORTS: [&str; 3] = ["get_info", "get_version", "set_allocator_handle"];
const IMAGE_BASE: u64 = 0x1_8000_0000;

fn link_with(object: &[u8], debug_info: bool, has_native_code: bool) -> Outcome {
    link_windows_dll(&DllRequest {
        object,
        exports: &EXPORTS,
        dll_name: "unit.codiralib",
        debug_info,
        has_native_code,
    })
}

fn link(builder: &Builder) -> Vec<u8> {
    match link_with(&builder.build(), false, false) {
        Outcome::Image(bytes) => bytes,
        Outcome::Declined(why) => panic!("declined: {why}"),
    }
}

fn declined(builder: &Builder) -> DeclineKind {
    match link_with(&builder.build(), false, false) {
        Outcome::Image(_) => panic!("expected a refusal"),
        Outcome::Declined(why) => why.kind,
    }
}

fn u32_at(bytes: &[u8], at: usize) -> u32 {
    u32::from_le_bytes(bytes[at..at + 4].try_into().unwrap())
}

fn u64_at(bytes: &[u8], at: usize) -> u64 {
    u64::from_le_bytes(bytes[at..at + 8].try_into().unwrap())
}

/// File offset of `rva` in `image`.
fn at(image: &[u8], rva: u32) -> usize {
    peinfo::parse(image).unwrap().rva_to_offset(rva).unwrap()
}

#[test]
fn every_modelled_relocation_gets_the_value_worked_out_by_hand() {
    let (mut b, text) = minimal();
    let data = b.section(".data", DATA, &[0u8; 64]);
    let data_sym = b.symbol(".data", 0, data, STATIC);
    let slot = b.symbol("slot", 8, data, EXTERNAL);
    let _ = data_sym;
    // Text layout: .text at RVA 0x1000, .rdata 0x2000, .data 0x3000.
    // REL32 at +0x04: S - (P + 4)
    b.reloc(text, 0x04, slot, 4);
    // REL32_1 at +0x10: the field is followed by one more byte.
    b.reloc(text, 0x10, slot, 5);
    // REL32_4 at +0x18.
    b.reloc(text, 0x18, slot, 8);
    // REL32 with an addend of 0x10 in the field, at +0x28.
    b.sections[text as usize - 1].data[0x28..0x2C].copy_from_slice(&0x10u32.to_le_bytes());
    b.reloc(text, 0x28, slot, 4);
    // Data: ADDR64 of get_version, ADDR32NB of it, SECREL, SECTION.
    b.reloc(data, 0, 2, 1);
    b.reloc(data, 0x10, 2, 3);
    b.reloc(data, 0x14, 2, 11);
    b.reloc(data, 0x18, slot, 10);

    let image = link(&b);
    peinfo::validate(&image).expect("a valid image");
    let info = peinfo::parse(&image).unwrap();
    let text_at = at(&image, 0x1000);
    let data_at = at(&image, 0x3000);

    let slot_rva = 0x3008i64;
    let rel32 = |field: i64, extra: i64| (slot_rva - (0x1000 + field + 4 + extra)) as u32;
    assert_eq!(u32_at(&image, text_at + 0x04), rel32(0x04, 0));
    assert_eq!(u32_at(&image, text_at + 0x10), rel32(0x10, 1));
    assert_eq!(u32_at(&image, text_at + 0x18), rel32(0x18, 4));
    assert_eq!(
        u32_at(&image, text_at + 0x28),
        rel32(0x28, 0).wrapping_add(0x10)
    );

    assert_eq!(u64_at(&image, data_at), IMAGE_BASE + 0x1010);
    assert_eq!(u32_at(&image, data_at + 0x10), 0x1010);
    assert_eq!(u32_at(&image, data_at + 0x14), 0x10);
    assert_eq!(
        u16::from_le_bytes([image[data_at + 0x18], image[data_at + 0x19]]),
        3
    );

    // The one 64-bit field is in the base-relocation table, and nothing else.
    let (reloc_rva, reloc_size) = info.directories[5];
    assert_ne!(reloc_rva, 0);
    let table = at(&image, reloc_rva);
    assert_eq!(u32_at(&image, table), 0x3000);
    assert_eq!(u32_at(&image, table + 4), 12);
    assert_eq!(reloc_size, 12);
    assert_eq!(
        u16::from_le_bytes([image[table + 8], image[table + 9]]),
        (10 << 12)
    );
}

#[test]
fn exports_are_sorted_and_carry_the_dll_name() {
    let (mut b, _text) = minimal();
    let text_symbol = b.symbol("a_extra", 48, 1, EXTERNAL);
    let _ = text_symbol;
    let object = b.build();
    let outcome = link_windows_dll(&DllRequest {
        object: &object,
        exports: &["set_allocator_handle", "get_version", "a_extra", "get_info"],
        dll_name: "unit.codiralib",
        debug_info: false,
        has_native_code: false,
    });
    let Outcome::Image(image) = outcome else {
        panic!("declined");
    };
    let info = peinfo::validate(&image).unwrap();
    assert_eq!(info.dll_name, "unit.codiralib");
    let names: Vec<&str> = info.exports.iter().map(|(n, _)| n.as_str()).collect();
    assert_eq!(
        names,
        ["a_extra", "get_info", "get_version", "set_allocator_handle"]
    );
    assert_eq!(info.exports[0].1, 0x1000 + 48);
}

#[test]
fn unreferenced_comdat_sections_are_dropped_and_referenced_ones_kept() {
    let (mut b, text) = minimal();
    let used = b.section(".rdata", RDATA | COMDAT, &[1; 8]);
    b.symbol_with_aux(".rdata", 0, used, STATIC, vec![section_definition(8, 2)]);
    let used_sym = b.symbol("__real@used", 0, used, EXTERNAL);
    let unused = b.section(".rdata", RDATA | COMDAT, &[2; 8]);
    b.symbol_with_aux(".rdata", 0, unused, STATIC, vec![section_definition(8, 2)]);
    b.symbol("__real@unused", 0, unused, EXTERNAL);
    b.reloc(text, 0x04, used_sym, 4);

    let image = link(&b);
    peinfo::validate(&image).unwrap();
    let info = peinfo::parse(&image).unwrap();
    let rdata = info.sections.iter().find(|s| s.name == ".rdata").unwrap();
    // The export directory and one 8-byte constant, not two.
    let export_size = info.directories[0].1;
    assert_eq!(rdata.virtual_size, 8 + export_size);
    let constant_at = at(&image, rdata.virtual_address);
    assert_eq!(&image[constant_at..constant_at + 8], &[1; 8]);
}

#[test]
fn unwind_data_and_a_function_table_are_kept_and_sorted() {
    let (mut b, text) = minimal();
    let xdata = b.section(".xdata", XDATA, &[0x01, 0, 0, 0, 0, 0, 0, 0]);
    let xdata_sym = b.symbol(".xdata", 0, xdata, STATIC);
    // Two functions, deliberately out of order.
    let mut entries = Vec::new();
    for (begin, end) in [(0x20u32, 0x30u32), (0x00, 0x10)] {
        entries.extend_from_slice(&begin.to_le_bytes());
        entries.extend_from_slice(&end.to_le_bytes());
        entries.extend_from_slice(&0u32.to_le_bytes());
    }
    let pdata = b.section(".pdata", PDATA, &entries);
    let text_sym = 0; // `.text`
    for i in 0..2u32 {
        b.reloc(pdata, 12 * i, text_sym, 3);
        b.reloc(pdata, 12 * i + 4, text_sym, 3);
        b.reloc(pdata, 12 * i + 8, xdata_sym, 3);
    }
    let _ = text;
    let image = link(&b);
    let info = peinfo::validate(&image).expect("sorted, in-image function table");
    let (rva, size) = info.directories[3];
    assert_eq!(size, 24);
    let table = at(&image, rva);
    assert_eq!(u32_at(&image, table), 0x1000);
    assert_eq!(u32_at(&image, table + 12), 0x1020);
}

#[test]
fn bss_is_appended_to_data_without_file_space() {
    let (mut b, text) = minimal();
    let data = b.section(".data", DATA, &[7u8; 4]);
    b.symbol(".data", 0, data, STATIC);
    let bss = b.bss(0x1000);
    b.symbol("zeroed", 0, bss, EXTERNAL);
    let zeroed = b.symbol("zeroed2", 0x10, bss, EXTERNAL);
    b.reloc(text, 0x04, zeroed, 4);
    let image = link(&b);
    let info = peinfo::validate(&image).unwrap();
    let data = info.sections.iter().find(|s| s.name == ".data").unwrap();
    assert_eq!(data.raw_size, 0x200);
    // 4 bytes of data, then the 4096-byte zero section at 16-byte alignment.
    assert_eq!(data.virtual_size, 16 + 0x1000);
}

#[test]
fn the_same_input_gives_the_same_image() {
    let (b, _) = minimal();
    assert_eq!(link(&b), link(&b));
}

#[test]
fn refuses_what_it_does_not_model() {
    // An unknown relocation type (ADDR32, which needs a low image base).
    let (mut b, text) = minimal();
    b.reloc(text, 0x04, 1, 2);
    assert_eq!(declined(&b), DeclineKind::UnknownRelocation);

    // Thread-local storage.
    let (mut b, _) = minimal();
    b.section(".tls$", 0xC030_0040, &[0; 8]);
    assert_eq!(declined(&b), DeclineKind::UnknownSection);

    // C runtime initializers.
    let (mut b, _) = minimal();
    b.section(".CRT$XCU", 0x4030_0040, &[0; 8]);
    assert_eq!(declined(&b), DeclineKind::UnknownSection);

    // Debug information.
    let (mut b, _) = minimal();
    b.section(".debug$S", 0x4210_0040, &[0; 8]);
    assert_eq!(declined(&b), DeclineKind::DebugInfo);
    let (b, _) = minimal();
    assert!(matches!(
        link_with(&b.build(), true, false),
        Outcome::Declined(d) if d.kind == DeclineKind::DebugInfo
    ));

    // Linker directives.
    let (mut b, _) = minimal();
    b.section(".drectve", 0x0010_0A00, b"/EXPORT:foo");
    assert_eq!(declined(&b), DeclineKind::UnknownSection);

    // A call to something the object does not define: an import.
    let (mut b, text) = minimal();
    let memcpy = b.symbol("memcpy", 0, 0, EXTERNAL);
    b.reloc(text, 0x04, memcpy, 4);
    assert_eq!(declined(&b), DeclineKind::UndefinedSymbol);

    // Native code in the package.
    let (b, _) = minimal();
    assert!(matches!(
        link_with(&b.build(), false, true),
        Outcome::Declined(d) if d.kind == DeclineKind::NativeCode
    ));

    // A COMDAT that depends on another section.
    let (mut b, _) = minimal();
    let associative = b.section(".rdata", RDATA | COMDAT, &[0; 8]);
    b.symbol_with_aux(
        ".rdata",
        0,
        associative,
        STATIC,
        vec![section_definition(8, 5)],
    );
    assert_eq!(declined(&b), DeclineKind::Comdat);

    // A weak external.
    let (mut b, _) = minimal();
    b.symbol("weak", 0, 0, 105);
    assert_eq!(declined(&b), DeclineKind::UnknownSymbol);

    // A symbol that must be exported but is not defined.
    let (mut b, _) = minimal();
    b.symbols.retain(|s| s.name != "get_version");
    assert_eq!(declined(&b), DeclineKind::MissingExport);

    // Another machine.
    let (mut b, _) = minimal();
    b.machine = 0xAA64;
    assert_eq!(declined(&b), DeclineKind::UnsupportedMachine);

    // A section with characteristics outside the five Codira produces.
    let (mut b, _) = minimal();
    b.section(".text", 0x6050_0020 | 0x0000_0040, &[0; 8]);
    assert_eq!(declined(&b), DeclineKind::UnknownSection);

    // Executable data in `.rdata`.
    let (mut b, _) = minimal();
    b.section(".rdata", 0x6050_0040, &[0; 8]);
    assert_eq!(declined(&b), DeclineKind::UnknownSection);
}

#[test]
fn refuses_damaged_objects_without_panicking() {
    let (mut b, text) = minimal();
    let data = b.section(".data", DATA, &[0u8; 16]);
    let slot = b.symbol("slot", 0, data, EXTERNAL);
    b.reloc(text, 0x04, slot, 4);
    let object = b.build();

    // Truncated at every length.
    for len in 0..object.len() {
        match link_with(&object[..len], false, false) {
            Outcome::Image(image) => {
                peinfo::validate(&image).expect("an accepted prefix still validates");
            }
            Outcome::Declined(_) => {}
        }
    }

    // A relocation at an offset outside its section.
    let (mut b, text) = minimal();
    let data = b.section(".data", DATA, &[0u8; 16]);
    let slot = b.symbol("slot", 0, data, EXTERNAL);
    b.reloc(text, 62, slot, 4);
    assert_eq!(declined(&b), DeclineKind::Malformed);

    // A relocation against a symbol that does not exist.
    let (mut b, text) = minimal();
    b.reloc(text, 0x04, 999, 4);
    assert_eq!(declined(&b), DeclineKind::Malformed);

    // A value that does not fit in 32 bits: a .bss further away than 2 GiB.
    let (mut b, text) = minimal();
    let bss = b.bss(0x7000_0000);
    let far = b.symbol("far", 0x6FFF_FFF0, bss, EXTERNAL);
    b.reloc(text, 0x04, far, 4);
    assert!(matches!(
        link_with(&b.build(), false, false),
        Outcome::Declined(d) if d.kind == DeclineKind::OutOfRange
    ));
}

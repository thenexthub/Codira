//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Golden differential tests: each fixture is an object file that LLVM wrote
//!   for a real Codira program, next to the DLL `lld-link` produced from it.
//!   The image writer must reproduce LLD's image byte for byte (the header time
//!   stamp aside). These need neither LLVM nor LLD, so they run everywhere.
use codira_image::{link_windows_dll, peinfo, DllRequest, Outcome};

const EXPORTS: [&str; 3] = ["get_info", "get_version", "set_allocator_handle"];

fn fixture(name: &str) -> (Vec<u8>, Vec<u8>) {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("tests/fixtures");
    let object = std::fs::read(dir.join(format!("{name}.obj"))).expect("fixture object");
    let dll = std::fs::read(dir.join(format!("{name}.dll"))).expect("fixture dll");
    (object, dll)
}

fn link(object: &[u8], dll_name: &str) -> Vec<u8> {
    match link_windows_dll(&DllRequest {
        object,
        exports: &EXPORTS,
        dll_name,
        debug_info: false,
        has_native_code: false,
    }) {
        Outcome::Image(bytes) => bytes,
        Outcome::Declined(why) => panic!("declined a fixture: {why}"),
    }
}

fn check(name: &str) {
    let (object, lld) = fixture(name);
    let lld_info = peinfo::validate(&lld).expect("LLD's image validates");
    let ours = link(&object, &lld_info.dll_name);
    peinfo::validate(&ours).expect("our image validates");
    if let Err(report) = peinfo::compare(&lld, &ours) {
        panic!("{name}: our image differs from LLD's:\n{report}");
    }
}

#[test]
fn readme_example_matches_lld() {
    check("readme");
}

#[test]
fn floats_comdat_constants_and_arrays_match_lld() {
    check("p1");
}

#[test]
fn smallest_module_without_unwind_or_relocations_matches_lld() {
    check("t1");
}

/// Every program of the runtime's own test suite that builds as a plain
/// Codira package (92 of them: structs, classes, floats, strings, arrays,
/// effects, tasks, generics ...), linked by LLD when the fixtures were made.
#[test]
fn a_corpus_of_real_programs_matches_lld_byte_for_byte() {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("tests/corpus");
    let mut objects: Vec<_> = std::fs::read_dir(&dir)
        .expect("corpus directory")
        .map(|e| e.expect("entry").path())
        .filter(|p| p.extension().is_some_and(|e| e == "obj"))
        .collect();
    objects.sort();
    assert!(
        objects.len() >= 90,
        "the corpus has {} programs",
        objects.len()
    );
    let mut failures = Vec::new();
    for path in &objects {
        let object = std::fs::read(path).unwrap();
        let lld = std::fs::read(path.with_extension("dll")).unwrap();
        let info = peinfo::validate(&lld).expect("LLD's image validates");
        let ours = link(&object, &info.dll_name);
        if let Err(report) = peinfo::compare(&lld, &ours) {
            failures.push(format!("{}:\n{report}", path.display()));
        }
    }
    assert!(failures.is_empty(), "{}", failures.join("\n"));
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
//! - Generates the doc tests that `tests/book.rs` includes from the mdbook in
//!   `book/`.
//!
//! The book is not part of the repository, so a fresh clone does not have it.
//! In that case this writes a test file with no tests in it, so
//! `tests/book.rs` still compiles, and warns that the book's doc tests were
//! skipped.
//!
//! Without the book there is no book path worth watching: cargo treats a
//! `rerun-if-changed` path that does not exist as always changed, so it would
//! rerun this script and rebuild every target of this crate on every build.
//! This script watches only itself then, and a book added later is picked up
//! by `cargo clean -p codira`.

use std::{env, fs, io, path::Path};

/// Root of the mdbook, relative to this crate's manifest directory (the
/// working directory cargo runs build scripts in).
const BOOK_ROOT: &str = "../../book";

/// The book's table of contents. mdbook cannot load a book without it, so its
/// presence is what decides whether there is a book to generate tests from.
const BOOK_SUMMARY: &str = "../../book/src/SUMMARY.md";

/// Name of the generated file, as `tests/book.rs` includes it from `OUT_DIR`.
const TESTS_FILE: &str = "skeptic-tests.rs";

fn main() {
    if Path::new(BOOK_SUMMARY).is_file() {
        // `codira_skeptic` watches the chapters, but not the table of contents
        // that decides which chapters there are.
        println!("cargo:rerun-if-changed={BOOK_SUMMARY}");
        codira_skeptic::generate_doc_tests_from_mdbook(BOOK_ROOT);
    } else {
        println!("cargo:rerun-if-changed=build.rs");
        println!(
            "cargo:warning=book doc-tests skipped: {BOOK_SUMMARY} not found (the mdbook is not \
             part of the repository; after adding it, run `cargo clean -p codira`)"
        );
        write_empty_tests().expect("could not write the empty book doc-test file");
    }
}

/// Writes a test file without tests to `$OUT_DIR/skeptic-tests.rs`.
///
/// An unchanged file is left alone, so its timestamp does not make cargo
/// rebuild `tests/book.rs` for nothing.
fn write_empty_tests() -> io::Result<()> {
    const CONTENTS: &str = "// No mdbook was found in `book/` when this crate was built, so there \
                            are no book doc tests.\n";

    let out_dir = env::var_os("OUT_DIR").expect("cargo sets OUT_DIR for build scripts");
    let path = Path::new(&out_dir).join(TESTS_FILE);
    if fs::read_to_string(&path).is_ok_and(|current| current == CONTENTS) {
        return Ok(());
    }
    fs::write(path, CONTENTS)
}

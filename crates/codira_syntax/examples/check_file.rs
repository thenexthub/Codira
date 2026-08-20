// Parse-only checker for `.code` files: `cargo run -p codira_syntax --example
// check_file -- path/to/file.code [more files...]`. Prints OK/FAIL per file
// and every SyntaxError with its byte offset; exits 1 if any file failed.
//
// Written to verify std/**/*.code syntax migrations against the current
// grammar (see spec/KGEN_SUPERSET_STATUS.md) without needing the LLVM
// toolchain codira_codegen/codira_compiler require -- codira_syntax has no
// such dependency, so this works in any environment that can build Rust.
// Kept as a standing dev tool for the rest of the std/ migration, not a
// one-off script.
use std::{env, fs};

use codira_syntax::SourceFile;

fn main() {
    let mut any_errors = false;
    for path in env::args().skip(1) {
        let text = fs::read_to_string(&path).expect("read file");
        let parse = SourceFile::parse(&text);
        if parse.errors().is_empty() {
            println!("OK   {path}");
        } else {
            any_errors = true;
            println!("FAIL {path}");
            for err in parse.errors() {
                println!("     {err:?}");
            }
        }
    }
    if any_errors {
        std::process::exit(1);
    }
}

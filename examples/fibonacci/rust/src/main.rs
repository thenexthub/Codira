//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
//!
use codira_runtime::Runtime;
use std::env;

fn main() {
    let lib_dir = env::args().nth(1).expect("Expected path to a Codira library.");
    println!("lib: {}", lib_dir);

    // Safety: we assume here that the library passed on the commandline is safe.
    let mut runtime =
        unsafe { Runtime::builder(lib_dir).finish() }.expect("Failed to spawn Runtime");

    loop {
        let n: i64 = runtime
            .invoke("nth", ())
            .unwrap_or_else(|e| e.wait(&mut runtime));
        let result: i64 = runtime
            .invoke("fibonacci", (n,))
            .unwrap_or_else(|e| e.wait(&mut runtime));
        println!("fibonacci({}) = {}", n, result);

        // Safety: we assume the updates are safe.
        unsafe { runtime.update() };
    }
}


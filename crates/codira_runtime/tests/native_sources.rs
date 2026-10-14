//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Builds a package with C, C++ and assembly in its `[native]` table and
//!   calls each of them from Codira.

use codira_test::CompileAndRunTestDriver;

#[macro_use]
mod util;

const FIXTURE: &str = r#"
//- /codira.toml
[package]
name = "native"
version = "0.1.0"

[native]
sources = ["c/kernels.c", "c/table.cpp", "c/ident.S"]
include = ["c/include"]
defines = ["SCALE=3"]

//- /c/include/kernels.h
#pragma once
#include <stdint.h>
int64_t c_mul_add(int64_t a, int64_t b);

//- /c/kernels.c
#include "kernels.h"
#include <stdlib.h>
#include <string.h>

int64_t c_mul_add(int64_t a, int64_t b) { return a * SCALE + b; }

/* Goes through the C runtime: malloc, memset and free. */
uint64_t c_sum_bytes(uint64_t n) {
    unsigned char *bytes = malloc(n);
    if (!bytes) return 0;
    memset(bytes, 2, n);
    uint64_t total = 0;
    for (uint64_t i = 0; i < n; i++) total += bytes[i];
    free(bytes);
    return total;
}

//- /c/table.cpp
#include <cstdint>
#include <vector>

// Filled by a static constructor, which only runs if the assembly's entry
// point does.
static std::vector<std::int64_t> squares = [] {
    std::vector<std::int64_t> v;
    for (std::int64_t i = 0; i < 16; ++i) v.push_back(i * i);
    return v;
}();

extern "C" std::int64_t cpp_square(std::int64_t i) {
    return i >= 0 && i < static_cast<std::int64_t>(squares.size()) ? squares[i] : -1;
}

//- /c/ident.S
#if defined(__APPLE__)
#define SYMBOL(name) _##name
#else
#define SYMBOL(name) name
#endif

    .text
    .globl SYMBOL(asm_identity)
#if defined(_WIN32)
    .def SYMBOL(asm_identity); .scl 2; .type 32; .endef
#endif
SYMBOL(asm_identity):
#if defined(__x86_64__) && defined(_WIN32)
    mov %rcx, %rax
    ret
#elif defined(__x86_64__)
    mov %rdi, %rax
    ret
#elif defined(__aarch64__)
    ret
#else
#error "asm_identity has no implementation for this architecture"
#endif

//- /src/mod.code
extern "C" {
    func c_mul_add(a: i64, b: i64) -> i64;
    func c_sum_bytes(n: u64) -> u64;
    func cpp_square(i: i64) -> i64;
    func asm_identity(x: i64) -> i64;
}

public func mul_add(a: i64, b: i64) -> i64 { c_mul_add(a, b) }
public func sum_bytes(n: u64) -> u64 { c_sum_bytes(n) }
public func square(i: i64) -> i64 { cpp_square(i) }
public func identity(x: i64) -> i64 { asm_identity(x) }
"#;

#[test]
fn c_cpp_and_assembly_sources_are_called_from_codira() {
    let driver = CompileAndRunTestDriver::from_fixture(FIXTURE, |builder| builder)
        .expect("Failed to build test driver");

    // `SCALE` comes from the manifest's `defines`, the header from `include`.
    assert_invoke_eq!(i64, 7 * 3 + 5, driver, "mul_add", 7i64, 5i64);
    assert_invoke_eq!(u64, 2000u64, driver, "sum_bytes", 1000u64);
    assert_invoke_eq!(i64, 81i64, driver, "square", 9i64);
    assert_invoke_eq!(i64, -1i64, driver, "square", 99i64);
    assert_invoke_eq!(i64, 123_456_789i64, driver, "identity", 123_456_789i64);
}

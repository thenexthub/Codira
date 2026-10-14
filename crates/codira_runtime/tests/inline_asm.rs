//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Runs `@asm` functions on the host: each declares an x86-64 and an
//!   `AArch64` variant, so the same source is exercised on either machine.

use codira_test::CompileAndRunTestDriver;

#[macro_use]
mod util;

const SOURCE: &str = r#"
@asm(arch: "x86_64", "lea ($1,$2), $0", constraints: "=r,r,r", pure: true)
@asm(arch: "aarch64", "add $0, $1, $2", constraints: "=r,r,r", pure: true)
extern func asm_add(a: i64, b: i64) -> i64;

@asm(arch: "x86_64", "bswap $0", constraints: "=r,0", pure: true)
@asm(arch: "aarch64", "rev $0, $1", constraints: "=r,r", pure: true)
extern func asm_bswap(x: u64) -> u64;

@asm(arch: "x86_64", "sqrtsd $1, $0", constraints: "=x,x", pure: true)
@asm(arch: "aarch64", "fsqrt ${0:d}, ${1:d}", constraints: "=w,w", pure: true)
extern func asm_sqrt(x: f64) -> f64;

@asm(arch: "x86_64", "lea $0, [$1 + $2*8]", constraints: "=r,r,r", dialect: "intel", pure: true)
@asm(arch: "aarch64", "add $0, $1, $2, lsl #3", constraints: "=r,r,r", pure: true)
extern func asm_index(base: i64, index: i64) -> i64;

@asm(arch: "x86_64", "rdtsc; shl $$32, %rdx; or %rdx, %rax", constraints: "={rax},~{rdx}")
@asm(arch: "aarch64", "mrs $0, cntvct_el0", constraints: "=r")
extern func cycle_counter() -> u64;

public func add(a: i64, b: i64) -> i64 { asm_add(a, b) }
public func bswap(x: u64) -> u64 { asm_bswap(x) }
public func sqrt(x: f64) -> f64 { asm_sqrt(x) }
public func index(base: i64, i: i64) -> i64 { asm_index(base, i) }
public func cycles_advance() -> bool {
    let first = cycle_counter()
    let second = cycle_counter()
    second >= first && first != 0
}
"#;

#[test]
fn inline_assembly_runs_on_the_host() {
    let driver = CompileAndRunTestDriver::new(SOURCE, |builder| builder)
        .expect("Failed to build test driver");

    assert_invoke_eq!(i64, 42i64, driver, "add", 40i64, 2i64);
    assert_invoke_eq!(i64, -7i64, driver, "add", -10i64, 3i64);
    assert_invoke_eq!(
        u64,
        0x0807_0605_0403_0201u64,
        driver,
        "bswap",
        0x0102_0304_0506_0708u64
    );
    assert_invoke_eq!(f64, 4.0f64, driver, "sqrt", 16.0f64);
    assert_invoke_eq!(i64, 1040i64, driver, "index", 1000i64, 5i64);
    assert_invoke_eq!(bool, true, driver, "cycles_advance");
}

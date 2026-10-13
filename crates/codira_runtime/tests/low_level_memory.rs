//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Runs the machine-level memory intrinsics (`stack_alloc`, non-temporal
//!   loads and stores, atomics, fences, prefetch, `cache_line_size`,
//!   `frame_address`) on the host.

use codira_test::CompileAndRunTestDriver;

#[macro_use]
mod util;

const SOURCE: &str = r#"
extern "codira-intrinsic" {
    func stack_alloc(size: usize, align: usize) -> usize;
    func load_u64(addr: usize) -> u64;
    func store_u64(addr: usize, value: u64);
    func load_nt_u64(addr: usize) -> u64;
    func store_nt_u64(addr: usize, value: u64);
    func atomic_load_u64(addr: usize) -> u64;
    func atomic_store_u64(addr: usize, value: u64);
    func atomic_add_u64(addr: usize, value: u64) -> u64;
    func atomic_swap_u64(addr: usize, value: u64) -> u64;
    func atomic_cas_u64(addr: usize, expected: u64, new: u64) -> u64;
    func atomic_max_i64(addr: usize, value: i64) -> i64;
    func atomic_min_u64(addr: usize, value: u64) -> u64;
    func store_i64(addr: usize, value: i64);
    func load_i64(addr: usize) -> i64;
    func prefetch_read(addr: usize);
    func prefetch_write(addr: usize);
    func prefetch_read_once(addr: usize);
    func fence_acquire();
    func fence_release();
    func fence_seq_cst();
    func compiler_fence();
    func cache_line_size() -> usize;
    func frame_address() -> usize;
}

// Sums 0..n through a 64-element buffer on the stack. The constant-size
// `stack_alloc` sits in the entry block, so the loop reuses one slot.
public func stack_sum(n: u64) -> u64 {
    let buffer = stack_alloc(512, 64)
    var i = 0
    while i < 64 {
        store_u64(buffer + (i as usize) * 8, 0)
        i += 1
    }
    var k: u64 = 0
    while k < n {
        let slot = buffer + ((k % 64) as usize) * 8
        prefetch_write(slot)
        store_u64(slot, load_u64(slot) + k)
        k += 1
    }
    var total: u64 = 0
    var j = 0
    while j < 64 {
        prefetch_read(buffer + (j as usize) * 8)
        total += load_u64(buffer + (j as usize) * 8)
        j += 1
    }
    total
}

public func is_aligned(align: usize) -> bool {
    let a = stack_alloc(256, 64)
    let b = stack_alloc(8, 8)
    a % 64 == 0 && b % 8 == 0 && a != b
}

public func non_temporal_round_trip(value: u64) -> u64 {
    let slot = stack_alloc(8, 8)
    store_nt_u64(slot, value)
    fence_seq_cst()
    prefetch_read_once(slot)
    load_nt_u64(slot)
}

public func atomics() -> u64 {
    let slot = stack_alloc(16, 16)
    atomic_store_u64(slot, 10)
    let before_add = atomic_add_u64(slot, 5)        // 10, slot = 15
    fence_release()
    let failed_cas = atomic_cas_u64(slot, 99, 1)    // 15, no change
    let won_cas = atomic_cas_u64(slot, 15, 40)      // 15, slot = 40
    fence_acquire()
    let before_swap = atomic_swap_u64(slot, 7)      // 40, slot = 7
    let before_min = atomic_min_u64(slot, 3)        // 7, slot = 3
    compiler_fence()
    let signed = slot + 8
    store_i64(signed, -5)
    let before_max = atomic_max_i64(signed, -9)     // -5, stays -5
    let after_max = load_i64(signed)
    if before_add == 10 && failed_cas == 15 && won_cas == 15 && before_swap == 40
        && before_min == 7 && before_max == -5 && after_max == -5 {
        atomic_load_u64(slot)
    } else {
        0
    }
}

public func line_size() -> usize {
    cache_line_size()
}

public func has_frame() -> bool {
    frame_address() != 0
}
"#;

#[test]
fn machine_level_memory_intrinsics_run_on_the_host() {
    let driver = CompileAndRunTestDriver::new(SOURCE, |builder| builder)
        .expect("Failed to build test driver");

    assert_invoke_eq!(u64, (0..1000u64).sum::<u64>(), driver, "stack_sum", 1000u64);
    assert_invoke_eq!(bool, true, driver, "is_aligned", 64usize);
    assert_invoke_eq!(
        u64,
        0xDEAD_BEEF_CAFE_F00Du64,
        driver,
        "non_temporal_round_trip",
        0xDEAD_BEEF_CAFE_F00Du64
    );
    assert_invoke_eq!(u64, 3u64, driver, "atomics");
    let expected_line = if cfg!(all(target_arch = "aarch64", target_vendor = "apple")) {
        128usize
    } else {
        64usize
    };
    assert_invoke_eq!(usize, expected_line, driver, "line_size");
    assert_invoke_eq!(bool, true, driver, "has_frame");
}

//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Libraries written by the direct image writer (`codira_image`) work with
//!   the two things that depend on the details of a PE image: hot reloading and
//!   Heracles' fault interception.
//! - The library is loaded through the runtime, an invalid memory access in its
//!   code is caught by a Heracles guard, and the Windows unwinder finds the
//!   faulting instruction in the library's `.pdata` (`RtlLookupFunctionEntry`)
//!   and can unwind it (`RtlVirtualUnwind`). The library is then replaced
//!   through `Runtime::update` and the same is checked for the new image. The
//!   tests fail, rather than skip, if the image writer did not write the
//!   libraries.
#![cfg(windows)]

use std::ffi::c_void;

use codira_compiler::{image_link_stats, DIRECT_LINK_ENV, LINK_VERIFY_ENV};
use codira_healing::trap::{self, FaultKind};
use codira_test::CompileAndRunTestDriver;

#[repr(C)]
struct RuntimeFunction {
    begin: u32,
    end: u32,
    unwind: u32,
}

extern "system" {
    fn RtlLookupFunctionEntry(
        control_pc: u64,
        image_base: *mut u64,
        history_table: *mut c_void,
    ) -> *const RuntimeFunction;
    fn GetProcAddress(module: *mut c_void, name: *const u8) -> *const c_void;
    fn RtlVirtualUnwind(
        handler_type: u32,
        image_base: u64,
        control_pc: u64,
        function_entry: *const RuntimeFunction,
        context: *mut Context,
        handler_data: *mut *mut c_void,
        establisher_frame: *mut u64,
        context_pointers: *mut c_void,
    ) -> *const c_void;
}

/// The AMD64 `CONTEXT`, as much of it as the unwinder needs: 1232 bytes,
/// 16-byte aligned, with `Rsp` at 0x98 and `Rip` at 0xF8.
#[repr(C, align(16))]
struct Context([u8; 1232]);

const SOURCE: &str = r#"
extern "codira-intrinsic" {
    func load_u64(addr: usize) -> u64;
}

public func version() -> i64 { 1 }

// Allocates an array, which calls into the runtime: only a function that
// might unwind gets unwind data (`.pdata`), and this one has to have it.
public func peek(addr: usize) -> u64 {
    let numbers = [1, 2, 3]
    load_u64(addr) + (numbers[1] as u64)
}
"#;

const SOURCE_V2: &str = r#"
extern "codira-intrinsic" {
    func load_u64(addr: usize) -> u64;
}

public func version() -> i64 { 2 }

public func peek(addr: usize) -> u64 {
    let numbers = [1, 2, 3]
    load_u64(addr) + (numbers[2] as u64)
}
"#;

/// Reads a pointer-sized register out of the raw context.
fn register(context: &Context, offset: usize) -> u64 {
    u64::from_le_bytes(context.0[offset..offset + 8].try_into().unwrap())
}

/// Checks that the Windows unwinder knows the instruction at `ip`: it is in
/// a module that exports `get_info` (a Codira library), `.pdata` has an
/// entry for it, and unwinding that entry runs.
fn assert_unwindable(ip: usize) {
    let mut base = 0u64;
    // Safety: plain queries of the loaded module list; `base` is a valid out
    // pointer.
    let entry = unsafe { RtlLookupFunctionEntry(ip as u64, &mut base, std::ptr::null_mut()) };
    assert!(
        !entry.is_null(),
        "the faulting instruction {ip:#x} has no .pdata entry"
    );
    // Safety: the entry points into the loaded image's .pdata.
    let entry_ref = unsafe { &*entry };
    let rva = ip as u64 - base;
    assert!(
        u64::from(entry_ref.begin) <= rva && rva < u64::from(entry_ref.end),
        "the .pdata entry does not cover the faulting instruction"
    );
    // Safety: `base` is the base address of a loaded module.
    let exported = unsafe { GetProcAddress(base as *mut c_void, c"get_info".as_ptr().cast()) };
    assert!(
        !exported.is_null(),
        "the faulting instruction is not in a Codira library"
    );

    // Unwind the frame with a made-up stack: the unwinder reads the return
    // address from where the unwind codes say it is.
    let stack = vec![0x1122_3344_5566_7788u64; 64];
    let mut context = Context([0; 1232]);
    // Safety: the offsets are those of AMD64 `CONTEXT`; `stack` outlives the
    // call and is large enough for any frame of these functions.
    unsafe {
        let rsp = context.0.as_mut_ptr().add(0x98).cast::<u64>();
        rsp.write_unaligned(stack.as_ptr() as u64 + 8 * 32);
        let rip = context.0.as_mut_ptr().add(0xF8).cast::<u64>();
        rip.write_unaligned(ip as u64);
        // ContextFlags: CONTEXT_CONTROL | CONTEXT_INTEGER.
        context
            .0
            .as_mut_ptr()
            .add(0x30)
            .cast::<u32>()
            .write(0x0010_0003);
        let mut handler_data = std::ptr::null_mut();
        let mut frame = 0u64;
        RtlVirtualUnwind(
            0,
            base,
            ip as u64,
            entry,
            &mut context,
            &mut handler_data,
            &mut frame,
            std::ptr::null_mut(),
        );
    }
    assert_ne!(
        register(&context, 0xF8),
        ip as u64,
        "unwinding did not move to the caller"
    );
}

#[test]
fn fault_interception_and_hot_reload_work_in_libraries_from_the_image_writer() {
    std::env::set_var(DIRECT_LINK_ENV, "1");
    std::env::set_var(LINK_VERIFY_ENV, "1");
    trap::install();

    let before = image_link_stats();
    let mut driver = CompileAndRunTestDriver::new(SOURCE, |builder| builder)
        .expect("Failed to build test driver");
    let built = image_link_stats();
    assert!(
        built.written > before.written && built.verified > before.verified,
        "the library was not written by the image writer (declined: {:?})",
        built.declined
    );
    assert_eq!(built.declined, before.declined);

    let version: i64 = driver.runtime.invoke("version", ()).unwrap();
    assert_eq!(version, 1);

    // A Heracles guard catches an access violation in the library's code.
    let fault = trap::protected(|| {
        let value: u64 = driver.runtime.invoke("peek", (0usize,)).unwrap();
        value
    })
    .expect_err("reading address 0 must fault");
    assert_eq!(fault.kind, FaultKind::AccessViolation);
    assert_eq!(fault.faulting_address, Some(0));
    assert_unwindable(fault.instruction_pointer);

    // Hot reload: the compiler writes the new library by rename, the runtime
    // notices and loads it.
    driver.update_file("mod.code", SOURCE_V2);
    let reloaded = image_link_stats();
    assert!(
        reloaded.written > built.written && reloaded.verified > built.verified,
        "the reloaded library was not written by the image writer"
    );
    let version: i64 = driver.runtime.invoke("version", ()).unwrap();
    assert_eq!(version, 2);

    // The new image is as good as the first: faults are still caught, and
    // the unwinder still finds the function.
    let fault = trap::protected(|| {
        let value: u64 = driver.runtime.invoke("peek", (0usize,)).unwrap();
        value
    })
    .expect_err("reading address 0 must fault");
    assert_eq!(fault.kind, FaultKind::AccessViolation);
    assert_unwindable(fault.instruction_pointer);

    // And the runtime keeps working after a caught fault.
    let version: i64 = driver.runtime.invoke("version", ()).unwrap();
    assert_eq!(version, 2);
}

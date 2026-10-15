//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - ARM MTE support for libraries built with `--sanitize=memtag`
//!   (experimental): turning tag checking on when such a library loads, and
//!   tagging the memory the runtime hands out through region arenas and OS
//!   pages (`native_memory`). The instructions live in
//!   `native/memtag_aarch64.c`, compiled only for aarch64 Linux.
//!
//! # What is tagged
//!
//! When a memtag library is loaded, [`activate`] asks the kernel for the
//! tagged-address ABI with synchronous tag-check faults
//! (`prctl(PR_SET_TAGGED_ADDR_CTRL)`) and adds `PROT_MTE` to the loading
//! thread's stack, so the tags the generated code puts on `stack_alloc`
//! slots are kept and checked. From then on:
//!
//! * regions created afterwards take their chunks from `PROT_MTE` mappings;
//!   every allocation is rounded up to whole 16-byte granules and gets a random
//!   tag that is never 0 and never the previous allocation's, so a linear
//!   overflow into the next allocation always faults. Reset sets the reused
//!   chunk back to tag 0 and free unmaps the chunks, so a stale address faults
//!   either way;
//! * pages are mapped with `PROT_MTE` and the whole mapping gets one random
//!   tag;
//! * the address returned carries the tag in bits 56..59, which the hardware
//!   ignores for translation (top-byte ignore) and checks against the memory's
//!   tag on every load and store.
//!
//! Not tagged: the garbage-collected heap and `codira_aligned_alloc` (both
//! come from the C allocator, which is not mapped `PROT_MTE`, so their
//! accesses are not checked and work as before), and threads that existed
//! before activation (they keep running unchecked).
//!
//! # Arithmetic on tagged addresses
//!
//! An address is still a `usize`. Adding or subtracting an offset keeps the
//! tag as long as the result stays below 2^56, which every user-space
//! address plus any in-bounds offset does. Comparing or subtracting two
//! addresses from *different* allocations compares their tags first, so the
//! difference and the ordering are meaningless there (within one allocation
//! they are exact); `addr & 0x00ff_ffff_ffff_ffff` strips a tag. Passing a
//! tagged address to C is fine: the tagged-address ABI lets system calls
//! take them, and C dereferences them like any pointer. A page that will be
//! executed must be called through its untagged address: Linux keeps
//! top-byte-ignore off for instruction addresses.
//!
//! # Verified where
//!
//! On aarch64 Linux only, with an MTE CPU (or `qemu-aarch64 -cpu max`). On
//! any other host this module compiles to [`activate`] returning
//! [`MemtagError::Unsupported`], which makes the runtime refuse to load a
//! memtag library instead of running it unchecked.

use std::{
    ptr::NonNull,
    sync::atomic::{AtomicBool, Ordering},
};

use codira_memory::arena::ChunkSource;

/// Why tag checking could not be turned on.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum MemtagError {
    /// Not aarch64 Linux, so there is no MTE to use.
    #[error(
        "this library was built with --sanitize=memtag (experimental), which needs an \
         aarch64 Linux host with MTE; this runtime runs on {0}"
    )]
    Unsupported(&'static str),
    /// The kernel refused the tagged-address ABI (no MTE in the CPU or kernel).
    #[error(
        "this library was built with --sanitize=memtag (experimental), but the kernel refused \
         to enable tag checking (prctl returned errno {0}): this CPU or kernel has no MTE"
    )]
    Refused(i64),
}

static ACTIVE: AtomicBool = AtomicBool::new(false);

/// Whether tag checking is on and new region and page memory is tagged.
pub fn is_active() -> bool {
    ACTIVE.load(Ordering::Acquire)
}

/// Strips an MTE tag from `addr` (bits 56..63) on aarch64, where they are
/// ignored by translation; elsewhere returns `addr` unchanged.
pub fn untagged(addr: usize) -> usize {
    if cfg!(target_arch = "aarch64") {
        addr & 0x00ff_ffff_ffff_ffff
    } else {
        addr
    }
}

/// The MTE granule: tags cover memory in 16-byte units.
pub const GRANULE: usize = 16;

#[cfg(all(target_arch = "aarch64", target_os = "linux"))]
mod sys {
    extern "C" {
        pub fn codira_mte_enable() -> i64;
        pub fn codira_mte_tag_stack(sp: usize) -> i64;
        pub fn codira_mte_map(size: usize, prot: i32) -> usize;
        pub fn codira_mte_protect(addr: usize, size: usize, prot: i32) -> i64;
        pub fn codira_mte_set_tags(addr: usize, size: usize);
        pub fn codira_mte_tag(addr: usize, size: usize, exclude: u64) -> usize;
    }
    pub const HOST: Option<&str> = None;
}

/// Stand-ins with the same signatures where there is no MTE. Only
/// [`super::activate`] is reached, and it reports [`super::MemtagError`]
/// before anything else could be.
#[cfg(not(all(target_arch = "aarch64", target_os = "linux")))]
#[allow(clippy::missing_safety_doc)]
mod sys {
    pub unsafe fn codira_mte_enable() -> i64 {
        -22
    }
    pub unsafe fn codira_mte_tag_stack(_sp: usize) -> i64 {
        -22
    }
    pub unsafe fn codira_mte_map(_size: usize, _prot: i32) -> usize {
        0
    }
    pub unsafe fn codira_mte_protect(_addr: usize, _size: usize, _prot: i32) -> i64 {
        -22
    }
    pub unsafe fn codira_mte_set_tags(_addr: usize, _size: usize) {}
    pub unsafe fn codira_mte_tag(addr: usize, _size: usize, _exclude: u64) -> usize {
        addr
    }
    pub const HOST: Option<&str> = Some(if cfg!(target_os = "linux") {
        "Linux on a non-aarch64 CPU"
    } else if cfg!(windows) {
        "Windows"
    } else if cfg!(target_os = "macos") {
        "macOS"
    } else {
        "a host without the Linux MTE interface"
    });
}

/// Turns on tag checking for the calling thread (and the threads it creates
/// afterwards) and tags its stack. Idempotent per thread.
pub fn activate() -> Result<(), MemtagError> {
    if let Some(host) = sys::HOST {
        return Err(MemtagError::Unsupported(host));
    }
    // Safety: a system call with no pointer arguments.
    let enabled = unsafe { sys::codira_mte_enable() };
    if enabled < 0 {
        return Err(MemtagError::Refused(-enabled));
    }
    let marker = 0u8;
    // Safety: reads /proc/self/maps and changes the protection of the
    // mapping holding this frame to add PROT_MTE (same access otherwise).
    let stack = unsafe { sys::codira_mte_tag_stack(std::ptr::addr_of!(marker) as usize) };
    if stack < 0 {
        log::warn!(
            "--sanitize=memtag: could not add PROT_MTE to this thread's stack (errno {}); \
             `stack_alloc` slots are not checked",
            -stack
        );
    }
    ACTIVE.store(true, Ordering::Release);
    Ok(())
}

/// Chunks for regions created while tagging is active: `PROT_MTE`
/// mappings, freed with `munmap`.
pub const TAGGED_CHUNKS: ChunkSource = ChunkSource {
    alloc: |size| {
        // Safety: a fresh anonymous mapping.
        NonNull::new(unsafe { sys::codira_mte_map(size, 3) } as *mut u8)
    },
    free: |base, size| {
        // Safety: the caller returns a chunk mapped above with `size`.
        let _ignored = unsafe { codira_memory::pages::free(base, size) };
    },
};

/// Gives the granule-aligned allocation `[addr, addr + size)` a random tag
/// that is neither 0 nor `previous`, and returns `addr` carrying it.
///
/// # Safety
///
/// The range must be writable `PROT_MTE` memory the caller owns; `addr` is
/// 16-aligned and `size` a multiple of 16.
pub unsafe fn tag(addr: usize, size: usize, previous: Option<u8>) -> usize {
    let exclude = previous.map_or(0, |tag| 1u64 << (tag & 0xf));
    sys::codira_mte_tag(untagged(addr), size, exclude)
}

/// The tag `addr` carries.
pub fn tag_of(addr: usize) -> u8 {
    ((addr >> 56) & 0xf) as u8
}

/// Sets every granule of `[addr, addr + size)` back to tag 0.
///
/// # Safety
///
/// As for [`tag`].
pub unsafe fn clear(addr: usize, size: usize) {
    sys::codira_mte_set_tags(untagged(addr), size);
}

/// Maps `size` bytes of tagged memory with protection `prot` (`1` read, `2`
/// write, `4` exec), the whole mapping under one random tag. Returns the
/// tagged address, or `None`.
pub fn map_pages(size: usize, prot: u32) -> Option<usize> {
    let size = size.next_multiple_of(codira_memory::pages::page_size());
    // Mapped writable first: tags are stored like data and need write access.
    // Safety: a fresh anonymous mapping.
    let base = unsafe { sys::codira_mte_map(size, 3) };
    if base == 0 {
        return None;
    }
    // Safety: the whole fresh mapping, page aligned and page sized.
    let tagged = unsafe { tag(base, size, None) };
    if prot != 3 {
        // Safety: the mapping above.
        if unsafe { sys::codira_mte_protect(base, size, prot as i32) } < 0 {
            // Safety: the mapping above, not used again.
            let _ignored =
                unsafe { codira_memory::pages::free(NonNull::new(base as *mut u8)?, size) };
            return None;
        }
    }
    Some(tagged)
}

/// Changes the protection of pages from [`map_pages`], keeping them tagged.
///
/// # Safety
///
/// The range must be one [`map_pages`] returned.
pub unsafe fn protect_pages(addr: usize, size: usize, prot: u32) -> bool {
    sys::codira_mte_protect(untagged(addr), size, prot as i32) == 0
}

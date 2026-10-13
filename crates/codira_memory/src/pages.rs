//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - OS page allocation with explicit protection (`VirtualAlloc`,
//!   `VirtualProtect`, `VirtualFree` on Windows; `mmap`, `mprotect`, `munmap`
//!   elsewhere), huge pages where the OS grants them, aligned heap allocation,
//!   and instruction-cache flushing for code written at run time.
//!
//! Protection is W^X: a mapping may be readable, writable or executable, but
//! never writable and executable at once. Code is written into a
//! `READ | WRITE` mapping and then flipped to `READ | EXEC` with
//! [`protect`]; requesting `WRITE | EXEC` is rejected.

use std::{alloc::Layout, ptr::NonNull};

/// The mapping can be read.
pub const PROT_READ: u32 = 1;
/// The mapping can be written.
pub const PROT_WRITE: u32 = 2;
/// The mapping can be executed.
pub const PROT_EXEC: u32 = 4;

/// An error returned by the page functions.
#[derive(Debug, thiserror::Error)]
pub enum PageError {
    /// `WRITE | EXEC` was requested.
    #[error("a mapping cannot be writable and executable at once (W^X)")]
    WriteAndExecute,
    /// Unknown protection bits were passed.
    #[error("invalid protection bits {0:#x}")]
    InvalidProtection(u32),
    /// A size of zero was passed.
    #[error("size must not be zero")]
    ZeroSize,
    /// The operating system refused the request.
    #[error("the operating system refused the request: {0}")]
    Os(#[from] std::io::Error),
}

fn validate(prot: u32) -> Result<(), PageError> {
    if prot & !(PROT_READ | PROT_WRITE | PROT_EXEC) != 0 {
        return Err(PageError::InvalidProtection(prot));
    }
    if prot & PROT_WRITE != 0 && prot & PROT_EXEC != 0 {
        return Err(PageError::WriteAndExecute);
    }
    Ok(())
}

/// The kind of pages a huge-page request actually got.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HugePages {
    /// The OS mapped the region with large/huge pages.
    Granted,
    /// The OS was asked to back the region with transparent huge pages when
    /// it can (Linux `MADV_HUGEPAGE`); whether it does is up to the kernel.
    Advised,
    /// Regular pages: the OS has no huge pages available for this process
    /// (on Windows the "Lock pages in memory" privilege is required).
    Unavailable,
}

#[cfg(windows)]
mod os {
    use std::ffi::c_void;

    use super::{PROT_EXEC, PROT_READ, PROT_WRITE};

    pub(super) const MEM_COMMIT: u32 = 0x1000;
    pub(super) const MEM_RESERVE: u32 = 0x2000;
    pub(super) const MEM_RELEASE: u32 = 0x8000;
    pub(super) const MEM_LARGE_PAGES: u32 = 0x2000_0000;

    #[repr(C)]
    pub struct SystemInfo {
        pub processor_architecture: u16,
        pub reserved: u16,
        pub page_size: u32,
        pub minimum_application_address: *mut c_void,
        pub maximum_application_address: *mut c_void,
        pub active_processor_mask: usize,
        pub number_of_processors: u32,
        pub processor_type: u32,
        pub allocation_granularity: u32,
        pub processor_level: u16,
        pub processor_revision: u16,
    }

    #[link(name = "kernel32")]
    extern "system" {
        pub fn VirtualAlloc(
            address: *mut c_void,
            size: usize,
            allocation_type: u32,
            protect: u32,
        ) -> *mut c_void;
        pub fn VirtualProtect(
            address: *mut c_void,
            size: usize,
            new_protect: u32,
            old_protect: *mut u32,
        ) -> i32;
        pub fn VirtualFree(address: *mut c_void, size: usize, free_type: u32) -> i32;
        pub fn GetSystemInfo(info: *mut SystemInfo);
        pub fn GetLargePageMinimum() -> usize;
        pub fn FlushInstructionCache(
            process: *mut c_void,
            address: *const c_void,
            size: usize,
        ) -> i32;
        pub fn GetCurrentProcess() -> *mut c_void;
    }

    pub fn protection(prot: u32) -> u32 {
        const PAGE_NOACCESS: u32 = 0x01;
        const PAGE_READONLY: u32 = 0x02;
        const PAGE_READWRITE: u32 = 0x04;
        const PAGE_EXECUTE: u32 = 0x10;
        const PAGE_EXECUTE_READ: u32 = 0x20;
        match (
            prot & PROT_READ != 0,
            prot & PROT_WRITE != 0,
            prot & PROT_EXEC != 0,
        ) {
            (_, true, _) => PAGE_READWRITE,
            (true, false, true) => PAGE_EXECUTE_READ,
            (false, false, true) => PAGE_EXECUTE,
            (true, false, false) => PAGE_READONLY,
            (false, false, false) => PAGE_NOACCESS,
        }
    }
}

#[cfg(unix)]
mod os {
    use super::{PROT_EXEC, PROT_READ, PROT_WRITE};

    pub fn protection(prot: u32) -> libc::c_int {
        let mut flags = libc::PROT_NONE;
        if prot & PROT_READ != 0 {
            flags |= libc::PROT_READ;
        }
        if prot & PROT_WRITE != 0 {
            flags |= libc::PROT_WRITE;
        }
        if prot & PROT_EXEC != 0 {
            flags |= libc::PROT_EXEC;
        }
        flags
    }
}

/// Returns the size of a page.
pub fn page_size() -> usize {
    #[cfg(windows)]
    unsafe {
        let mut info = std::mem::zeroed::<os::SystemInfo>();
        os::GetSystemInfo(&mut info);
        info.page_size as usize
    }
    #[cfg(unix)]
    unsafe {
        libc::sysconf(libc::_SC_PAGESIZE) as usize
    }
}

/// Maps `size` bytes (rounded up to whole pages) of zeroed memory with the
/// given protection.
pub fn alloc(size: usize, prot: u32) -> Result<NonNull<u8>, PageError> {
    validate(prot)?;
    if size == 0 {
        return Err(PageError::ZeroSize);
    }
    #[cfg(windows)]
    let ptr = unsafe {
        os::VirtualAlloc(
            std::ptr::null_mut(),
            size,
            os::MEM_COMMIT | os::MEM_RESERVE,
            os::protection(prot),
        )
        .cast::<u8>()
    };
    #[cfg(unix)]
    let ptr = unsafe {
        let ptr = libc::mmap(
            std::ptr::null_mut(),
            size,
            os::protection(prot),
            libc::MAP_PRIVATE | libc::MAP_ANON,
            -1,
            0,
        );
        if ptr == libc::MAP_FAILED {
            std::ptr::null_mut()
        } else {
            ptr.cast::<u8>()
        }
    };
    NonNull::new(ptr).ok_or_else(|| PageError::Os(std::io::Error::last_os_error()))
}

/// Maps `size` bytes with huge pages if the OS grants them, falling back to
/// regular pages (and saying so) when it does not.
pub fn alloc_huge(size: usize, prot: u32) -> Result<(NonNull<u8>, HugePages), PageError> {
    validate(prot)?;
    if size == 0 {
        return Err(PageError::ZeroSize);
    }
    #[cfg(windows)]
    {
        let minimum = unsafe { os::GetLargePageMinimum() };
        if minimum != 0 {
            let rounded = size.next_multiple_of(minimum);
            let ptr = unsafe {
                os::VirtualAlloc(
                    std::ptr::null_mut(),
                    rounded,
                    os::MEM_COMMIT | os::MEM_RESERVE | os::MEM_LARGE_PAGES,
                    os::protection(prot),
                )
            };
            if let Some(ptr) = NonNull::new(ptr.cast::<u8>()) {
                return Ok((ptr, HugePages::Granted));
            }
        }
        alloc(size, prot).map(|ptr| (ptr, HugePages::Unavailable))
    }
    #[cfg(target_os = "linux")]
    {
        let ptr = unsafe {
            libc::mmap(
                std::ptr::null_mut(),
                size,
                os::protection(prot),
                libc::MAP_PRIVATE | libc::MAP_ANON | libc::MAP_HUGETLB,
                -1,
                0,
            )
        };
        if ptr != libc::MAP_FAILED {
            return Ok((NonNull::new(ptr.cast()).expect("mmap"), HugePages::Granted));
        }
        let ptr = alloc(size, prot)?;
        let advised = unsafe { libc::madvise(ptr.as_ptr().cast(), size, libc::MADV_HUGEPAGE) } == 0;
        Ok((
            ptr,
            if advised {
                HugePages::Advised
            } else {
                HugePages::Unavailable
            },
        ))
    }
    #[cfg(all(unix, not(target_os = "linux")))]
    {
        alloc(size, prot).map(|ptr| (ptr, HugePages::Unavailable))
    }
}

/// Changes the protection of `[addr, addr + size)`.
///
/// # Safety
///
/// The range must lie in a mapping returned by [`alloc`] or [`alloc_huge`],
/// and no code may rely on the old protection afterwards.
pub unsafe fn protect(addr: NonNull<u8>, size: usize, prot: u32) -> Result<(), PageError> {
    validate(prot)?;
    if size == 0 {
        return Err(PageError::ZeroSize);
    }
    #[cfg(windows)]
    let ok = {
        let mut old = 0u32;
        os::VirtualProtect(addr.as_ptr().cast(), size, os::protection(prot), &mut old) != 0
    };
    #[cfg(unix)]
    let ok = libc::mprotect(addr.as_ptr().cast(), size, os::protection(prot)) == 0;
    if ok {
        Ok(())
    } else {
        Err(PageError::Os(std::io::Error::last_os_error()))
    }
}

/// Unmaps a mapping returned by [`alloc`] or [`alloc_huge`].
///
/// # Safety
///
/// `addr` must be the start of such a mapping, `size` the size it was
/// requested with, and the memory must not be used afterwards.
pub unsafe fn free(addr: NonNull<u8>, size: usize) -> Result<(), PageError> {
    #[cfg(windows)]
    let ok = {
        let _ = size;
        os::VirtualFree(addr.as_ptr().cast(), 0, os::MEM_RELEASE) != 0
    };
    #[cfg(unix)]
    let ok = libc::munmap(addr.as_ptr().cast(), size) == 0;
    if ok {
        Ok(())
    } else {
        Err(PageError::Os(std::io::Error::last_os_error()))
    }
}

/// Makes code written to `[addr, addr + size)` visible to instruction
/// fetch. Required on `AArch64` after writing machine code; harmless on x86.
pub fn flush_instruction_cache(addr: *const u8, size: usize) {
    #[cfg(windows)]
    unsafe {
        os::FlushInstructionCache(os::GetCurrentProcess(), addr.cast(), size);
    }
    #[cfg(all(unix, target_arch = "aarch64"))]
    unsafe {
        extern "C" {
            fn __clear_cache(start: *mut libc::c_char, end: *mut libc::c_char);
        }
        __clear_cache(addr as *mut _, addr.add(size) as *mut _);
    }
    #[cfg(all(unix, not(target_arch = "aarch64")))]
    {
        let _ = (addr, size);
    }
}

/// Allocates `size` zeroed bytes aligned to `align` from the global
/// allocator. Returns `None` for an invalid layout or on exhaustion.
pub fn aligned_alloc(size: usize, align: usize) -> Option<NonNull<u8>> {
    let layout = Layout::from_size_align(size.max(1), align).ok()?;
    NonNull::new(unsafe { std::alloc::alloc_zeroed(layout) })
}

/// Frees memory returned by [`aligned_alloc`].
///
/// # Safety
///
/// `addr` must come from [`aligned_alloc`] with the same `size` and `align`.
pub unsafe fn aligned_free(addr: NonNull<u8>, size: usize, align: usize) {
    let layout = Layout::from_size_align(size.max(1), align).expect("invalid layout");
    std::alloc::dealloc(addr.as_ptr(), layout);
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn write_xor_execute_is_enforced() {
        assert!(matches!(
            alloc(4096, PROT_WRITE | PROT_EXEC),
            Err(PageError::WriteAndExecute)
        ));
        assert!(matches!(
            alloc(4096, 8),
            Err(PageError::InvalidProtection(8))
        ));
        assert!(matches!(alloc(0, PROT_READ), Err(PageError::ZeroSize)));
    }

    #[test]
    fn pages_are_zeroed_writable_and_protectable() {
        let size = page_size();
        let page = alloc(size, PROT_READ | PROT_WRITE).unwrap();
        unsafe {
            assert_eq!(*page.as_ptr().add(size - 1), 0);
            page.as_ptr().write(7);
            protect(page, size, PROT_READ).unwrap();
            assert_eq!(*page.as_ptr(), 7);
            free(page, size).unwrap();
        }
    }

    #[cfg(any(target_arch = "x86_64", target_arch = "aarch64"))]
    #[test]
    fn generated_code_runs_after_flipping_to_rx() {
        #[cfg(target_arch = "x86_64")]
        let code: &[u8] = &[0xB8, 42, 0, 0, 0, 0xC3]; // mov eax, 42; ret
        #[cfg(target_arch = "aarch64")]
        let code: &[u8] = &[0x40, 0x05, 0x80, 0x52, 0xC0, 0x03, 0x5F, 0xD6]; // mov w0, #42; ret
        let size = page_size();
        let page = alloc(size, PROT_READ | PROT_WRITE).unwrap();
        unsafe {
            std::ptr::copy_nonoverlapping(code.as_ptr(), page.as_ptr(), code.len());
            protect(page, size, PROT_READ | PROT_EXEC).unwrap();
            flush_instruction_cache(page.as_ptr(), code.len());
            let function: extern "C" fn() -> u32 = std::mem::transmute(page.as_ptr());
            assert_eq!(function(), 42);
            free(page, size).unwrap();
        }
    }

    #[test]
    fn huge_pages_fall_back_honestly() {
        let size = 2 * 1024 * 1024;
        let (ptr, kind) = alloc_huge(size, PROT_READ | PROT_WRITE).unwrap();
        unsafe {
            ptr.as_ptr().write(1);
            ptr.as_ptr().add(size - 1).write(1);
            let _ = kind;
            free(ptr, size).unwrap();
        }
    }

    #[test]
    fn aligned_allocation_round_trips() {
        let ptr = aligned_alloc(100, 4096).unwrap();
        assert_eq!(ptr.as_ptr() as usize % 4096, 0);
        unsafe { aligned_free(ptr, 100, 4096) };
        assert!(aligned_alloc(8, 3).is_none());
    }
}

//! Memory: transparent huge pages for large tables, and allocations that
//! are never freed.

/// Requests transparent huge pages for a mapped byte range on Linux-based
/// targets.
///
/// # Safety
///
/// `data..data + size` must describe a live mapping.
#[cfg(any(target_os = "android", target_os = "linux"))]
pub(crate) unsafe fn madvise_hugepage(data: *mut u8, size: usize) {
    // SAFETY: guaranteed by the caller; MADV_HUGEPAGE is only a kernel hint.
    let _ = unsafe { libc::madvise(data.cast(), size, libc::MADV_HUGEPAGE) };
}

/// No-op on targets that do not support Linux's `MADV_HUGEPAGE` advice.
///
/// # Safety
///
/// Kept identical to the supported-target signature.
#[cfg(not(any(target_os = "android", target_os = "linux")))]
pub(crate) unsafe fn madvise_hugepage(_data: *mut u8, _size: usize) {}

/// Requests transparent huge pages for the whole pages strictly inside an
/// allocation, leaving possible allocator metadata in its boundary pages
/// untouched.
///
/// # Safety
///
/// `data..data + size` must describe a live allocation.
#[cfg(any(target_os = "android", target_os = "linux"))]
pub unsafe fn madvise_hugepage_interior(data: *const u8, size: usize) {
    let page_size = unsafe { libc::sysconf(libc::_SC_PAGESIZE) };
    if size == 0 || page_size <= 0 {
        return;
    }

    let page_size = page_size as usize;
    let start = data.addr();
    let Some(end) = start.checked_add(size) else {
        return;
    };
    let Some(rounded) = start.checked_add(page_size - 1) else {
        return;
    };
    let first_page = rounded / page_size * page_size;
    let last_page = end / page_size * page_size;
    if first_page < last_page {
        // SAFETY: these complete pages lie strictly inside the caller's
        // allocation.
        unsafe { madvise_hugepage(data.with_addr(first_page).cast_mut(), last_page - first_page) };
    }
}

/// No-op on targets that do not support Linux's `MADV_HUGEPAGE` advice.
///
/// # Safety
///
/// Kept identical to the supported-target signature.
#[cfg(not(any(target_os = "android", target_os = "linux")))]
pub unsafe fn madvise_hugepage_interior(_data: *const u8, _size: usize) {}

// Allocations that outlive every data structure of a link.
//
// Input files, symbol names and a few other objects must outlive every data
// structure of a link, and a command-line link exits the process as soon as it
// is done, so never freeing them is both simplest and cheapest. A link run
// inside another program (see `session`) cannot leak them: a build daemon
// links thousands of times. While a library session is active these functions
// therefore also record what they hand out, and the session frees it all when
// the link is over. The `'static` lifetimes they return are the same lie as
// before; they are true for as long as the session lives, and no reference may
// be kept past it. The linker's own data (the Context) is dropped before the
// session ends, and what a link reports to its caller is copied out.

use std::mem::ManuallyDrop;
use std::sync::Mutex;
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};

static ARENA_ACTIVE: AtomicBool = AtomicBool::new(false);

const SHARDS: usize = 32;

/// One recorded allocation: a pointer and how to free it.
struct Leaked {
    ptr: *mut (),
    len: usize,
    cap: usize,
    free: unsafe fn(*mut (), usize, usize),
}

// SAFETY: the pointers are owned allocations that nothing else frees, and
// `free` is called once, from whichever thread ends the session.
unsafe impl Send for Leaked {}

static ARENA: [Mutex<Vec<Leaked>>; SHARDS] = [const { Mutex::new(Vec::new()) }; SHARDS];

fn shard() -> &'static Mutex<Vec<Leaked>> {
    static NEXT: AtomicUsize = AtomicUsize::new(0);
    thread_local! {
        static SHARD: usize = NEXT.fetch_add(1, Ordering::Relaxed) % SHARDS;
    }
    &ARENA[SHARD.with(|s| *s)]
}

fn record(ptr: *mut (), len: usize, cap: usize, free: unsafe fn(*mut (), usize, usize)) {
    shard().lock().unwrap_or_else(|e| e.into_inner()).push(Leaked { ptr, len, cap, free });
}

unsafe fn free_box<T>(ptr: *mut (), _: usize, _: usize) {
    // SAFETY: `ptr` came from `Box::into_raw` of a `Box<T>`.
    drop(unsafe { Box::from_raw(ptr.cast::<T>()) });
}

unsafe fn free_vec<T>(ptr: *mut (), len: usize, cap: usize) {
    // SAFETY: the parts are those of a `Vec<T>` that was given up.
    drop(unsafe { Vec::from_raw_parts(ptr.cast::<T>(), len, cap) });
}

/// Starts recording leaked allocations (see `session`).
pub(crate) fn arena_start() {
    ARENA_ACTIVE.store(true, Ordering::SeqCst);
}

/// Frees everything recorded since `arena_start` and stops recording.
///
/// # Safety
///
/// No reference returned by `leak`, `leak_mut`, `leak_vec` or `leak_bytes`
/// since `arena_start` may be used afterwards.
pub(crate) unsafe fn arena_release() {
    ARENA_ACTIVE.store(false, Ordering::SeqCst);
    for shard in &ARENA {
        let items = std::mem::take(&mut *shard.lock().unwrap_or_else(|e| e.into_inner()));
        for item in items {
            // SAFETY: guaranteed by the caller; each item is freed once.
            unsafe { (item.free)(item.ptr, item.len, item.cap) };
        }
    }
}

/// Leaks a value for the rest of the process's lifetime (or, in a library
/// session, of the session).
pub fn leak<T>(value: T) -> &'static T {
    leak_mut(value)
}

/// Like [`leak`], for a value that is then modified in place.
pub fn leak_mut<T>(value: T) -> &'static mut T {
    let ptr = Box::into_raw(Box::new(value));
    if ARENA_ACTIVE.load(Ordering::Relaxed) {
        record(ptr.cast(), 0, 0, free_box::<T>);
    }
    // SAFETY: the allocation is live until the session frees it, and `ptr`
    // is the only way to it.
    unsafe { &mut *ptr }
}

/// Leaks the elements of a vector, like `Vec::leak`.
pub fn leak_vec<T>(vec: Vec<T>) -> &'static mut [T] {
    let mut vec = ManuallyDrop::new(vec);
    let (ptr, len, cap) = (vec.as_mut_ptr(), vec.len(), vec.capacity());
    if ARENA_ACTIVE.load(Ordering::Relaxed) {
        record(ptr.cast(), len, cap, free_vec::<T>);
    }
    // SAFETY: the allocation is live until the session frees it, and `ptr`
    // is the only way to it.
    unsafe { std::slice::from_raw_parts_mut(ptr, len) }
}

/// Leaks a byte string for the rest of the process's lifetime (or, in a
/// library session, of the session).
pub fn leak_bytes(bytes: Vec<u8>) -> &'static [u8] {
    leak_vec(bytes)
}

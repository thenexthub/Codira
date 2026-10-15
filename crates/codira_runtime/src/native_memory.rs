//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Raw memory services the runtime provides to Codira programs through
//!   `extern "C"` declarations (`std/memory/region.code`,
//!   `std/memory/page.code`): region arenas with byte quotas, OS pages with W^X
//!   protection, huge pages, aligned allocation and instruction-cache flushing.
//!   Every function is registered in each runtime's dispatch table.
//!
//! Addresses and handles cross the boundary as `usize`, like the
//! `__intrinsic_load`/`__intrinsic_store` convention. Every failure is a
//! return value (`0` for an address or handle, `false` for a status), never a
//! panic: these functions are called from generated code, where unwinding is
//! not defined.
//!
//! Region handles are an index into a process-wide table plus a generation,
//! so a freed or forged handle is rejected instead of being dereferenced.
//!
//! Once a `--sanitize=memtag` library has turned MTE on (`crate::memtag`),
//! regions created afterwards and every page mapping hand out tagged
//! addresses; see that module for what is checked.

use std::{cell::Cell, ptr::NonNull};

use codira_memory::{
    arena::Arena,
    pages::{self, HugePages},
};
use parking_lot::Mutex;

use crate::{memtag, FunctionDefinition, IntoFunctionDefinition};

/// Bits of a region handle that hold the slot index (plus one, so that `0`
/// is never a valid handle).
const INDEX_BITS: u32 = 20;
const INDEX_MASK: usize = (1 << INDEX_BITS) - 1;

struct Slot {
    generation: usize,
    arena: Option<Arena>,
    /// Whether the region's chunks are MTE-tagged.
    tagged: bool,
    /// The tag of a tagged region's last allocation (the next one never
    /// repeats it).
    last_tag: Option<u8>,
}

#[derive(Default)]
struct Regions {
    slots: Vec<Slot>,
    free: Vec<usize>,
}

static REGIONS: Mutex<Regions> = Mutex::new(Regions {
    slots: Vec::new(),
    free: Vec::new(),
});

impl Regions {
    fn handle(index: usize, generation: usize) -> usize {
        (generation << INDEX_BITS) | (index + 1)
    }

    fn slot(&mut self, handle: usize) -> Option<&mut Slot> {
        let index = (handle & INDEX_MASK).checked_sub(1)?;
        let generation = handle >> INDEX_BITS;
        let slot = self.slots.get_mut(index)?;
        if slot.generation != generation || slot.arena.is_none() {
            return None;
        }
        Some(slot)
    }

    fn get(&mut self, handle: usize) -> Option<&mut Arena> {
        self.slot(handle)?.arena.as_mut()
    }
}

/// Creates a region. `quota` is the most bytes it may hand out (`0` for no
/// quota). Returns the region handle, or `0` if the region table is full.
pub extern "C" fn codira_region_new(quota: usize) -> usize {
    let mut regions = REGIONS.lock();
    let quota = (quota != 0).then_some(quota);
    let tagged = memtag::is_active();
    let arena = if tagged {
        Arena::with_chunk_source(quota, memtag::TAGGED_CHUNKS)
    } else {
        Arena::new(quota)
    };
    if let Some(index) = regions.free.pop() {
        let slot = &mut regions.slots[index];
        slot.arena = Some(arena);
        slot.tagged = tagged;
        slot.last_tag = None;
        Regions::handle(index, slot.generation)
    } else if regions.slots.len() < INDEX_MASK {
        regions.slots.push(Slot {
            generation: 0,
            arena: Some(arena),
            tagged,
            last_tag: None,
        });
        Regions::handle(regions.slots.len() - 1, 0)
    } else {
        0
    }
}

/// Allocates `size` zeroed bytes aligned to `align` in a region. Returns `0`
/// when the region's quota would be exceeded, the handle is not a live
/// region, or the layout is invalid.
///
/// In a tagged region the allocation is rounded up to whole 16-byte MTE
/// granules (the quota counts the rounded size) and the address carries its
/// tag.
pub extern "C" fn codira_region_alloc(region: usize, size: usize, align: usize) -> usize {
    let mut regions = REGIONS.lock();
    let Some(slot) = regions.slot(region) else {
        return 0;
    };
    let arena = slot.arena.as_mut().expect("`slot` checks it is live");
    if !slot.tagged {
        return arena
            .alloc(size, align)
            .map_or(0, |ptr| ptr.as_ptr() as usize);
    }
    let size = size.max(1).next_multiple_of(memtag::GRANULE);
    let Ok(ptr) = arena.alloc(size, align.max(memtag::GRANULE)) else {
        return 0;
    };
    // Safety: a fresh, granule-aligned allocation in a `PROT_MTE` chunk of
    // this region, which still has tag 0 (chunks are cleared on reset).
    let tagged = unsafe { memtag::tag(ptr.as_ptr() as usize, size, slot.last_tag) };
    slot.last_tag = Some(memtag::tag_of(tagged));
    tagged
}

/// Frees everything allocated in a region at once; the region stays usable.
///
/// A tagged region's chunks get tag 0 back first, so every address it handed
/// out faults until the memory is allocated (and tagged) again.
pub extern "C" fn codira_region_reset(region: usize) -> bool {
    let mut regions = REGIONS.lock();
    let Some(slot) = regions.slot(region) else {
        return false;
    };
    let arena = slot.arena.as_mut().expect("`slot` checks it is live");
    if slot.tagged {
        for (base, size) in arena.chunk_ranges() {
            // Safety: the region's own `PROT_MTE` chunk, whole.
            unsafe { memtag::clear(base.as_ptr() as usize, size) };
        }
        slot.last_tag = None;
    }
    arena.reset();
    true
}

/// Returns the bytes handed out by a region since it was created or reset,
/// or `0` for an invalid handle.
pub extern "C" fn codira_region_used(region: usize) -> usize {
    REGIONS.lock().get(region).map_or(0, |arena| arena.used())
}

/// Frees a region and all its memory. The handle becomes invalid.
pub extern "C" fn codira_region_free(region: usize) -> bool {
    let mut regions = REGIONS.lock();
    if regions.get(region).is_none() {
        return false;
    }
    let index = (region & INDEX_MASK) - 1;
    let slot = &mut regions.slots[index];
    slot.arena = None;
    slot.tagged = false;
    slot.last_tag = None;
    slot.generation = slot.generation.wrapping_add(1) & (usize::MAX >> INDEX_BITS);
    regions.free.push(index);
    true
}

/// Returns the OS page size.
pub extern "C" fn codira_page_size() -> usize {
    pages::page_size()
}

/// Maps zeroed pages with `prot` (`1` read, `2` write, `4` exec; write and
/// exec together are refused). Returns the address or `0`. While MTE is on
/// the mapping is tagged and so is the address.
pub extern "C" fn codira_page_alloc(size: usize, prot: u32) -> usize {
    if memtag::is_active() {
        if size == 0 || pages::validate_protection(prot).is_err() {
            return 0;
        }
        return memtag::map_pages(size, prot).unwrap_or(0);
    }
    pages::alloc(size, prot).map_or(0, |ptr| ptr.as_ptr() as usize)
}

thread_local! {
    static LAST_HUGE_PAGES: Cell<u32> = const { Cell::new(0) };
}

/// Maps pages backed by huge pages where the OS grants them, regular pages
/// otherwise. Returns the address or `0`; [`codira_page_huge_status`] tells
/// what was granted.
///
/// While MTE is on these are regular tagged pages (status `0`): huge pages
/// with `PROT_MTE` are not something every kernel can map.
pub extern "C" fn codira_page_alloc_huge(size: usize, prot: u32) -> usize {
    if memtag::is_active() {
        LAST_HUGE_PAGES.with(|last| last.set(0));
        return codira_page_alloc(size, prot);
    }
    match pages::alloc_huge(size, prot) {
        Ok((ptr, kind)) => {
            let status = match kind {
                HugePages::Unavailable => 0,
                HugePages::Advised => 1,
                HugePages::Granted => 2,
            };
            LAST_HUGE_PAGES.with(|last| last.set(status));
            ptr.as_ptr() as usize
        }
        Err(_) => 0,
    }
}

/// What the last [`codira_page_alloc_huge`] on this thread got: `2` huge
/// pages, `1` transparent huge pages requested (Linux), `0` regular pages.
pub extern "C" fn codira_page_huge_status() -> u32 {
    LAST_HUGE_PAGES.with(Cell::get)
}

/// Changes the protection of pages mapped by `codira_page_alloc*`.
pub extern "C" fn codira_page_protect(addr: usize, size: usize, prot: u32) -> bool {
    if memtag::is_active() && addr != 0 {
        // Safety: as below; the OS rejects ranges that are not mapped.
        return pages::validate_protection(prot).is_ok()
            && unsafe { memtag::protect_pages(addr, size, prot) };
    }
    let Some(addr) = NonNull::new(memtag::untagged(addr) as *mut u8) else {
        return false;
    };
    // Safety: the caller passes a range it mapped; the OS rejects ranges
    // that are not mapped.
    unsafe { pages::protect(addr, size, prot) }.is_ok()
}

/// Unmaps pages mapped by `codira_page_alloc*`.
pub extern "C" fn codira_page_free(addr: usize, size: usize) -> bool {
    let Some(addr) = NonNull::new(memtag::untagged(addr) as *mut u8) else {
        return false;
    };
    // Safety: the caller passes a mapping it owns.
    unsafe { pages::free(addr, size) }.is_ok()
}

/// Makes freshly written machine code visible to instruction fetch.
pub extern "C" fn codira_icache_flush(addr: usize, size: usize) {
    pages::flush_instruction_cache(memtag::untagged(addr) as *const u8, size);
}

/// Allocates `size` zeroed bytes aligned to `align` (a power of two).
/// Returns the address or `0`.
pub extern "C" fn codira_aligned_alloc(size: usize, align: usize) -> usize {
    pages::aligned_alloc(size, align).map_or(0, |ptr| ptr.as_ptr() as usize)
}

/// Frees memory from [`codira_aligned_alloc`]; `size` and `align` must match.
pub extern "C" fn codira_aligned_free(addr: usize, size: usize, align: usize) {
    if let Some(addr) = NonNull::new(addr as *mut u8) {
        if std::alloc::Layout::from_size_align(size.max(1), align).is_ok() {
            // Safety: the caller passes an allocation it owns with its layout.
            unsafe { pages::aligned_free(addr, size, align) };
        }
    }
}

/// Returns the definitions of every function in this module, for the
/// runtime's dispatch table.
pub(crate) fn functions() -> Vec<FunctionDefinition> {
    vec![
        IntoFunctionDefinition::into(
            codira_region_new as extern "C" fn(usize) -> usize,
            "codira_region_new",
        ),
        IntoFunctionDefinition::into(
            codira_region_alloc as extern "C" fn(usize, usize, usize) -> usize,
            "codira_region_alloc",
        ),
        IntoFunctionDefinition::into(
            codira_region_reset as extern "C" fn(usize) -> bool,
            "codira_region_reset",
        ),
        IntoFunctionDefinition::into(
            codira_region_used as extern "C" fn(usize) -> usize,
            "codira_region_used",
        ),
        IntoFunctionDefinition::into(
            codira_region_free as extern "C" fn(usize) -> bool,
            "codira_region_free",
        ),
        IntoFunctionDefinition::into(
            codira_page_size as extern "C" fn() -> usize,
            "codira_page_size",
        ),
        IntoFunctionDefinition::into(
            codira_page_alloc as extern "C" fn(usize, u32) -> usize,
            "codira_page_alloc",
        ),
        IntoFunctionDefinition::into(
            codira_page_alloc_huge as extern "C" fn(usize, u32) -> usize,
            "codira_page_alloc_huge",
        ),
        IntoFunctionDefinition::into(
            codira_page_huge_status as extern "C" fn() -> u32,
            "codira_page_huge_status",
        ),
        IntoFunctionDefinition::into(
            codira_page_protect as extern "C" fn(usize, usize, u32) -> bool,
            "codira_page_protect",
        ),
        IntoFunctionDefinition::into(
            codira_page_free as extern "C" fn(usize, usize) -> bool,
            "codira_page_free",
        ),
        IntoFunctionDefinition::into(
            codira_icache_flush as extern "C" fn(usize, usize),
            "codira_icache_flush",
        ),
        IntoFunctionDefinition::into(
            codira_aligned_alloc as extern "C" fn(usize, usize) -> usize,
            "codira_aligned_alloc",
        ),
        IntoFunctionDefinition::into(
            codira_aligned_free as extern "C" fn(usize, usize, usize),
            "codira_aligned_free",
        ),
    ]
}

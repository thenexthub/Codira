//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - Region arenas: bump allocation with O(1)-per-chunk bulk free and a byte
//!   quota that fails the allocation instead of exhausting the process.
//!
//! An [`Arena`] hands out raw, zeroed memory from a list of chunks. Chunks
//! grow geometrically, so freeing everything ([`Arena::reset`] or dropping
//! the arena) costs one deallocation per chunk, independent of how many
//! allocations were made. The quota counts the bytes handed out (including
//! alignment padding), so a region can be given a hard budget; exceeding it
//! returns [`ArenaError::QuotaExceeded`] and leaves the arena usable.
//!
//! Arena memory is not traced by the garbage collector. Storing a GC handle
//! in arena memory does not keep the object alive, and nothing stops a
//! program from keeping an arena address after the arena was reset; the
//! Codira-facing API exposes addresses as `usize`, like `UnsafePointer`.

use std::{alloc::Layout, ptr::NonNull};

/// The size of the first chunk of an arena.
const FIRST_CHUNK: usize = 4 * 1024;

/// The largest chunk an arena grows to (bigger requests get their own chunk).
const MAX_CHUNK: usize = 1024 * 1024;

/// An error returned by [`Arena::alloc`].
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum ArenaError {
    /// The allocation would exceed the arena's quota.
    #[error(
        "region quota of {quota} bytes exceeded ({used} bytes used, {requested} more requested)"
    )]
    QuotaExceeded {
        quota: usize,
        used: usize,
        requested: usize,
    },
    /// The size/alignment pair does not describe a valid layout.
    #[error("invalid allocation layout (size {size}, align {align})")]
    InvalidLayout { size: usize, align: usize },
    /// The system allocator could not provide a new chunk.
    #[error("the system allocator could not provide {requested} bytes")]
    SystemOutOfMemory { requested: usize },
}

struct Chunk {
    base: NonNull<u8>,
    layout: Layout,
}

/// A bump allocator with bulk free and an optional byte quota.
pub struct Arena {
    chunks: Vec<Chunk>,
    /// Next free address in the last chunk.
    cursor: usize,
    /// End of the last chunk.
    end: usize,
    /// Bytes handed out, including alignment padding.
    used: usize,
    quota: Option<usize>,
}

// Safety: the arena exclusively owns its chunks.
unsafe impl Send for Arena {}

impl Arena {
    /// Creates an empty arena. No memory is requested until the first
    /// allocation.
    pub fn new(quota: Option<usize>) -> Self {
        Self {
            chunks: Vec::new(),
            cursor: 0,
            end: 0,
            used: 0,
            quota,
        }
    }

    /// Returns the bytes handed out since creation or the last reset.
    pub fn used(&self) -> usize {
        self.used
    }

    /// Returns the quota, if any.
    pub fn quota(&self) -> Option<usize> {
        self.quota
    }

    /// Returns the bytes the arena holds from the system.
    pub fn committed(&self) -> usize {
        self.chunks.iter().map(|c| c.layout.size()).sum()
    }

    /// Allocates `size` zeroed bytes aligned to `align`.
    pub fn alloc(&mut self, size: usize, align: usize) -> Result<NonNull<u8>, ArenaError> {
        let layout = Layout::from_size_align(size, align)
            .map_err(|_layout_error| ArenaError::InvalidLayout { size, align })?;

        // Try the current chunk first.
        let start = self.cursor.checked_next_multiple_of(layout.align());
        let fits = start
            .and_then(|start| start.checked_add(layout.size()))
            .filter(|&end| self.cursor != 0 && end <= self.end);
        let (start, new_cursor) = if let (Some(start), Some(end)) = (start, fits) {
            (start, end)
        } else {
            self.check_quota(layout.size() + layout.align() - 1)?;
            self.grow(layout)?;
            let start = self.cursor.next_multiple_of(layout.align());
            (start, start + layout.size())
        };

        let consumed = new_cursor - self.cursor;
        self.check_quota(consumed)?;
        self.used += consumed;
        self.cursor = new_cursor;

        let ptr = start as *mut u8;
        // Reset arenas reuse their first chunk, so zero explicitly.
        unsafe { std::ptr::write_bytes(ptr, 0, layout.size()) };
        Ok(unsafe { NonNull::new_unchecked(ptr) })
    }

    fn check_quota(&self, requested: usize) -> Result<(), ArenaError> {
        match self.quota {
            Some(quota) if self.used.saturating_add(requested) > quota => {
                Err(ArenaError::QuotaExceeded {
                    quota,
                    used: self.used,
                    requested,
                })
            }
            _ => Ok(()),
        }
    }

    fn grow(&mut self, layout: Layout) -> Result<(), ArenaError> {
        let previous = self.chunks.last().map_or(0, |c| c.layout.size());
        let size = (previous * 2)
            .clamp(FIRST_CHUNK, MAX_CHUNK)
            .max(layout.size() + layout.align());
        let chunk_layout = Layout::from_size_align(size, 16).map_err(|_layout_error| {
            ArenaError::InvalidLayout {
                size: layout.size(),
                align: layout.align(),
            }
        })?;
        let base = NonNull::new(unsafe { std::alloc::alloc(chunk_layout) })
            .ok_or(ArenaError::SystemOutOfMemory { requested: size })?;
        self.chunks.push(Chunk {
            base,
            layout: chunk_layout,
        });
        self.cursor = base.as_ptr() as usize;
        self.end = self.cursor + size;
        Ok(())
    }

    /// Frees every allocation at once. The first chunk is kept for reuse;
    /// the others are returned to the system, one deallocation each.
    pub fn reset(&mut self) {
        while self.chunks.len() > 1 {
            let chunk = self.chunks.pop().expect("checked length");
            unsafe { std::alloc::dealloc(chunk.base.as_ptr(), chunk.layout) };
        }
        if let Some(first) = self.chunks.first() {
            self.cursor = first.base.as_ptr() as usize;
            self.end = self.cursor + first.layout.size();
        } else {
            self.cursor = 0;
            self.end = 0;
        }
        self.used = 0;
    }
}

impl Drop for Arena {
    fn drop(&mut self) {
        for chunk in self.chunks.drain(..) {
            unsafe { std::alloc::dealloc(chunk.base.as_ptr(), chunk.layout) };
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn allocations_are_aligned_zeroed_and_distinct() {
        let mut arena = Arena::new(None);
        let a = arena.alloc(3, 1).unwrap();
        let b = arena.alloc(8, 8).unwrap();
        let c = arena.alloc(64, 64).unwrap();
        assert_eq!(b.as_ptr() as usize % 8, 0);
        assert_eq!(c.as_ptr() as usize % 64, 0);
        assert!(b.as_ptr() as usize >= a.as_ptr() as usize + 3);
        unsafe {
            assert_eq!(std::slice::from_raw_parts(c.as_ptr(), 64), &[0u8; 64][..]);
            c.as_ptr().write_bytes(0xAB, 64);
        }
        arena.reset();
        let d = arena.alloc(64, 64).unwrap();
        unsafe { assert_eq!(std::slice::from_raw_parts(d.as_ptr(), 64), &[0u8; 64][..]) };
    }

    #[test]
    fn large_requests_get_their_own_chunk() {
        let mut arena = Arena::new(None);
        arena.alloc(16, 8).unwrap();
        let big = arena.alloc(3 * MAX_CHUNK, 16).unwrap();
        unsafe { big.as_ptr().add(3 * MAX_CHUNK - 1).write(1) };
        assert!(arena.committed() >= 3 * MAX_CHUNK);
        arena.reset();
        assert_eq!(arena.used(), 0);
        assert!(arena.committed() < MAX_CHUNK);
    }

    #[test]
    fn quota_fails_the_allocation_not_the_process() {
        let mut arena = Arena::new(Some(100));
        arena.alloc(60, 1).unwrap();
        let err = arena.alloc(60, 1).unwrap_err();
        assert!(matches!(
            err,
            ArenaError::QuotaExceeded {
                quota: 100,
                used: 60,
                ..
            }
        ));
        // The arena is still usable within its budget
        arena.alloc(40, 1).unwrap();
        assert_eq!(arena.used(), 100);
        arena.reset();
        arena.alloc(100, 1).unwrap();
    }

    #[test]
    fn invalid_layouts_are_errors() {
        let mut arena = Arena::new(None);
        assert!(matches!(
            arena.alloc(8, 3),
            Err(ArenaError::InvalidLayout { size: 8, align: 3 })
        ));
    }
}

//===--- StackAllocator.h - A stack allocator -----------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// A bump-pointer allocator that obeys a stack discipline.
//
//===----------------------------------------------------------------------===//

// Define __STDC_WANT_LIB_EXT1__ to get memset_s on platforms that have it.
// Other files may have included string.h without it already, so we also set
// this with a -D flag when building, but this allows tests to build without
// additional trouble.
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

#include "language/ABI/MetadataValues.h"
#include "language/Runtime/Debug.h"
#include "toolchain/Support/Alignment.h"
#include <cstddef>
#include <new>

// Notes: language::fatalError is not shared between liblanguageCore and liblanguage_Concurrency
// and liblanguage_Concurrency uses language_Concurrency_fatalError instead.
#ifndef LANGUAGE_FATAL_ERROR
#define LANGUAGE_FATAL_ERROR language::fatalError
#endif

namespace language {

/// A bump-pointer allocator that obeys a stack discipline.
///
/// StackAllocator performs fast allocation and deallocation of memory by
/// implementing a bump-pointer allocation strategy.
///
/// This isn't strictly a bump-pointer allocator as it uses backing slabs of
/// memory rather than relying on a boundless contiguous heap. However, it has
/// bump-pointer semantics in that it is a monotonically growing pool of memory
/// where every allocation is found by merely allocating the next N bytes in
/// the slab, or the next N bytes in the next slab.
///
/// In contrast to a pure bump-pointer allocator, it's possible to free memory.
/// Allocations and deallocations must follow a strict stack discipline. In
/// general, slabs which become unused are _not_ freed, but reused for
/// subsequent allocations.
///
/// It's possible to place the first slab into pre-allocated memory.
///
/// The SlabCapacity specifies the capacity for newly allocated slabs.
///
/// SlabMetadataPtr specifies a fake metadata pointer to place at the beginning
/// of slab allocations, so analysis tools can identify them.
template <size_t SlabCapacity, Metadata *SlabMetadataPtr>
class StackAllocator {
private:

  struct Allocation;
  struct Slab;

  /// The last active allocation.
  ///
  /// A deallocate() must free this allocation.
  Allocation *lastAllocation = nullptr;

  /// The first slab.
  Slab *firstSlab;

  /// True if the first slab is pre-allocated.
  uint32_t firstSlabIsPreallocated:1;
  /// Used for unit testing.
  uint32_t numAllocatedSlabs:31;


  /// The minimal alignment of allocated memory.
  static constexpr size_t alignment = MaximumAlignment;

  /// If set to true, memory allocations are checked for buffer overflows and
  /// use-after-free, similar to guard-malloc.
  static constexpr bool guardAllocations =
#ifdef NDEBUG
    false;
#else
    true;
#endif

  static constexpr uintptr_t magicUninitialized =   (uintptr_t)0xcdcdcdcdcdcdcdcdull;
  static constexpr uintptr_t magicEndOfAllocation = (uintptr_t)0xdeadbeafdeadbeafull;

  /// A memory slab holding multiple allocations.
  ///
  /// This struct is actually just the slab header. The slab buffer is tail
  /// allocated after Slab.
  struct Slab {
    /// A fake metadata pointer that analysis tools can use to identify slab
    /// allocations.
    const void *metadata;

    /// A single linked list of all allocated slabs.
    Slab *next = nullptr;

    // Capacity and offset do not include these header fields.
    uint32_t capacity;
    uint32_t currentOffset = 0;

    // Here starts the tail allocated memory buffer of the slab.

    Slab(size_t newCapacity)
        : metadata(SlabMetadataPtr), capacity(newCapacity) {
      assert((size_t)capacity == newCapacity && "capacity overflow");
    }

    /// The size of the slab header.
    static constexpr size_t headerSize() {
      return (sizeof(Slab) + alignment - 1) & ~(alignment - 1);
    }

    /// Return \p size with the added overhead of the slab header.
    static size_t includingHeader(size_t size) {
      return headerSize() + size;
    }

    /// Clear the fake metadata pointer. Call before freeing so that leftover
    /// heap garbage doesn't have slab metadata pointers in it.
    void clearMetadata() {
      // Use memset_s on Apple platforms. Fall back to a plain
      // assignment on other platforms. This is not necessary for
      // correctness, just as an aid to analysis tools, so it's OK if
      // the fallback gets optimized out.
#if defined(__APPLE__)
      memset_s(&metadata, sizeof(metadata), 0, sizeof(metadata));
#else
      metadata = 0;
#endif
    }

    /// Return the payload buffer address at \p atOffset.
    ///
    /// Note: it's valid to call this function on a not-yet-constructed slab.
    char *getAddr(size_t atOffset) {
      return (char *)this + headerSize() + atOffset;
    }

    /// Return true if this slab can fit an allocation of \p size.
    ///
    /// \p size does not include the allocation header, but must include the
    /// overhead for guardAllocations (if enabled).
    inline bool canAllocate(size_t size) const {
      return currentOffset + Allocation::includingHeader(size) <= capacity;
    }

    /// Return true, if no memory is allocated in this slab.
    bool isEmpty() const { return currentOffset == 0; }

    /// Allocate \p alignedSize of bytes in this slab.
    ///
    /// \p alignedSize does not include the allocation header, but must include
    /// the overhead for guardAllocations (if enabled).
    ///
    /// Precondition: \p alignedSize must be aligned up to
    ///               StackAllocator::alignment.
    /// Precondition: there must be enough space in this slab to fit the
    ///               allocation.
    Allocation *allocate(size_t alignedSize, Allocation *lastAllocation) {
      assert(toolchain::isAligned(toolchain::Align(alignment), alignedSize));
      assert(canAllocate(alignedSize));
      void *buffer = getAddr(currentOffset);
      auto *allocation = ::new (buffer) Allocation(lastAllocation, this);
      currentOffset += Allocation::includingHeader(alignedSize);
      if (guardAllocations) {
        uintptr_t *endOfCurrentAllocation = (uintptr_t *)getAddr(currentOffset);
        endOfCurrentAllocation[-1] = magicEndOfAllocation;
      }
      return allocation;
    }

    /// Deallocate \p allocation.
    ///
    /// Precondition: \p allocation must be an allocation in this slab.
    void deallocate(Allocation *allocation) {
      assert(allocation->slab == this);
      if (guardAllocations) {
        auto *endOfAllocation = (uintptr_t *)getAddr(currentOffset);
        if (endOfAllocation[-1] != magicEndOfAllocation)
          LANGUAGE_FATAL_ERROR(0, "Buffer overflow in StackAllocator");
        for (auto *p = (uintptr_t *)allocation; p < endOfAllocation; ++p)
          *p = magicUninitialized;
      }
      currentOffset = (char *)allocation - getAddr(0);
    }
  };

  /// A single memory allocation.
  ///
  /// This struct is actually just the allocation header. The allocated
  /// memory buffer is located after Allocation.
  struct Allocation {
    /// A single linked list of previous allocations.
    Allocation *previous;
    /// The containing slab.
    Slab *slab;

    // Here starts the tail allocated memory.

    Allocation(Allocation *previous, Slab *slab) :
      previous(previous), slab(slab) {}

    void *getAllocatedMemory() {
      return (char *)this + headerSize();
    }

    /// The size of the allocation header.
    static size_t headerSize() {
      return toolchain::alignTo(sizeof(Allocation), toolchain::Align(alignment));
    }

    /// Return \p size with the added overhead of the allocation header.
    static size_t includingHeader(size_t size) {
      return headerSize() + size;
    }
  };

  // Return a slab which is suitable to allocate \p size memory.
  Slab *getSlabForAllocation(size_t size) {
    Slab *slab = (lastAllocation ? lastAllocation->slab : firstSlab);
    if (slab) {
      // Is there enough space in the current slab?
      if (slab->canAllocate(size))
        return slab;

      // Is there a successor slab, which we allocated before (and became free
      // in the meantime)?
      if (Slab *nextSlab = slab->next) {
        assert(nextSlab->isEmpty());
        if (nextSlab->canAllocate(size))
          return nextSlab;

        // No space in the next slab. Although it's empty, the size exceeds its
        // capacity.
        // As we have to allocate a new slab anyway, free all successor slabs
        // and allocate a new one with the accumulated capacity.
        size_t alreadyAllocatedCapacity = freeAllSlabs(slab->next);
        size = std::max(size, alreadyAllocatedCapacity);
      }
    }
    size_t capacity = std::max(SlabCapacity,
                               Allocation::includingHeader(size));
    void *slabBuffer = malloc(Slab::includingHeader(capacity));
    Slab *newSlab = ::new (slabBuffer) Slab(capacity);
    if (slab)
      slab->next = newSlab;
    else
      firstSlab = newSlab;
    numAllocatedSlabs++;
    return newSlab;
  }

  /// Deallocate all slabs after \p first and set \p first to null.
  size_t freeAllSlabs(Slab *&first) {
    size_t freedCapacity = 0;
    Slab *slab = first;
    first = nullptr;
    while (slab) {
      Slab *next = slab->next;
      freedCapacity += slab->capacity;
      slab->clearMetadata();
      free(slab);
      numAllocatedSlabs--;
      slab = next;
    }
    return freedCapacity;
  }

public:
  /// Construct a StackAllocator without a pre-allocated first slab.
  StackAllocator()
      : firstSlab(nullptr), firstSlabIsPreallocated(false),
        numAllocatedSlabs(0) {}

  /// Construct a StackAllocator with a pre-allocated first slab.
  StackAllocator(void *firstSlabBuffer, size_t bufferCapacity) : StackAllocator() {
    // If the pre-allocated buffer can't hold a slab header, ignore it.
    if (bufferCapacity <= Slab::headerSize())
      return;
    char *start = (char *)toolchain::alignAddr(firstSlabBuffer,
                                          toolchain::Align(alignment));
    char *end = (char *)firstSlabBuffer + bufferCapacity;
    assert(start + Slab::headerSize() <= end &&
           "buffer for first slab too small");
    firstSlab = ::new (start) Slab(end - start - Slab::headerSize());
    firstSlabIsPreallocated = true;
    numAllocatedSlabs = 0;
  }

  ~StackAllocator() {
    if (lastAllocation)
      LANGUAGE_FATAL_ERROR(0, "not all allocations are deallocated");
    if (firstSlabIsPreallocated)
      firstSlab->clearMetadata();
    (void)freeAllSlabs(firstSlabIsPreallocated ? firstSlab->next : firstSlab);
    assert(getNumAllocatedSlabs() == 0);
  }

  static constexpr size_t slabHeaderSize() {
    return Slab::headerSize();
  }

  /// Allocate a memory buffer of \p size.
  void *alloc(size_t size) {
    if (guardAllocations)
      size += sizeof(uintptr_t);
    size_t alignedSize = toolchain::alignTo(size, toolchain::Align(alignment));
    Slab *slab = getSlabForAllocation(alignedSize);
    Allocation *allocation = slab->allocate(alignedSize, lastAllocation);
    lastAllocation = allocation;
    assert(toolchain::isAddrAligned(toolchain::Align(alignment),
                               allocation->getAllocatedMemory()));
    return allocation->getAllocatedMemory();
  }

  /// Deallocate memory \p ptr.
  void dealloc(void *ptr) {
    if (!lastAllocation || lastAllocation->getAllocatedMemory() != ptr) {
      LANGUAGE_FATAL_ERROR(0, "freed pointer was not the last allocation");
    }

    Allocation *prev = lastAllocation->previous;
    lastAllocation->slab->deallocate(lastAllocation);
    lastAllocation = prev;
  }

  void deallocThrough(void *ptr) {
    void *current = nullptr;
    do {
      if (!lastAllocation) {
        LANGUAGE_FATAL_ERROR(0, "freed pointer not among allocations");
      }
      current = lastAllocation->getAllocatedMemory();
      dealloc(current);
    } while (current != ptr);
  }

  /// For unit testing.
  int getNumAllocatedSlabs() { return numAllocatedSlabs; }
};

} // namespace language


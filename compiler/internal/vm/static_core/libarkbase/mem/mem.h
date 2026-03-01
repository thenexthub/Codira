/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LIBPANDABASE_MEM_H
#define LIBPANDABASE_MEM_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/math_helpers.h"

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <functional>

namespace ark {

namespace mem {
class GCRoot;

class MemStatsAdditionalInfo;
class MemStatsDefault;
class MemRange;

#ifndef NDEBUG
using MemStatsType = MemStatsAdditionalInfo;
#else
using MemStatsType = MemStatsDefault;
#endif
}  // namespace mem

class ObjectHeader;

#ifdef PANDA_32_BIT_MANAGED_POINTER
using ObjectPointerType = uint32_t;
#else
using ObjectPointerType = uintptr_t;
#endif

constexpr size_t OBJECT_POINTER_SIZE = sizeof(ObjectPointerType);
constexpr size_t OBJECT_POINTER_LOG2_SIZE = Ctz(OBJECT_POINTER_SIZE);

/// @brief Logarithmic/bit alignment
enum Alignment {
    LOG_ALIGN_2 = 2,
    LOG_ALIGN_3 = 3,
    LOG_ALIGN_4 = 4,
    LOG_ALIGN_5 = 5,
    LOG_ALIGN_6 = 6,
    LOG_ALIGN_7 = 7,
    LOG_ALIGN_8 = 8,
    LOG_ALIGN_9 = 9,
    LOG_ALIGN_10 = 10,
    LOG_ALIGN_11 = 11,
    LOG_ALIGN_12 = 12,
    LOG_ALIGN_13 = 13,
    LOG_ALIGN_MIN = LOG_ALIGN_2,
#ifdef __APPLE__
    LOG_ALIGN_14 = 14,
    LOG_ALIGN_MAX = LOG_ALIGN_14,
#else
    LOG_ALIGN_MAX = LOG_ALIGN_13,
#endif
};

/**
 * @param logAlignment - logarithmic alignment
 * @return alingnment in bytes
 */
constexpr size_t GetAlignmentInBytes(const Alignment logAlignment)
{
    return 1U << static_cast<uint32_t>(logAlignment);
}

/**
 * @brief returns log2 for alignment in bytes
 * @param ALIGNMENT_IN_BYTES - should be power of 2
 * @return alignment in bits
 */
constexpr Alignment GetLogAlignment(const uint32_t alignmentInBytes)
{
    using helpers::math::GetIntLog2;
    // check if it is power of 2
    ASSERT((alignmentInBytes != 0) && !(alignmentInBytes & (alignmentInBytes - 1)));
    ASSERT(GetIntLog2(alignmentInBytes) >= Alignment::LOG_ALIGN_MIN);
    ASSERT(GetIntLog2(alignmentInBytes) <= Alignment::LOG_ALIGN_MAX);
    return static_cast<Alignment>(GetIntLog2(alignmentInBytes));
}

template <class T>
constexpr std::enable_if_t<std::is_unsigned_v<T>, T> AlignUp(T value, size_t alignment)
{
    return (value + alignment - 1U) & ~(alignment - 1U);
}

template <class T>
constexpr std::enable_if_t<std::is_unsigned_v<T>, T> AlignDown(T value, size_t alignment)
{
    return value & ~(alignment - 1U);
}

template <class T>
constexpr uintptr_t ToUintPtr(T *val)
{
    return reinterpret_cast<uintptr_t>(val);
}

constexpr uintptr_t ToUintPtr(std::nullptr_t)
{
    return ToUintPtr(static_cast<void *>(nullptr));
}

template <class T>
constexpr T *ToNativePtr(uintptr_t val)
{
    return reinterpret_cast<T *>(val);
}

inline void *ToVoidPtr(uintptr_t val)
{
    return reinterpret_cast<void *>(val);
}

constexpr ObjectPointerType ToObjPtr(const void *ptr)
{
    return static_cast<ObjectPointerType>(ToUintPtr(ptr));
}

// Managed languages may create double, atomic i64, NaN-tagged and similar object fields,
// thus 64-bit object alignment is required
constexpr Alignment DEFAULT_ALIGNMENT = GetLogAlignment(sizeof(uint64_t));
constexpr size_t DEFAULT_ALIGNMENT_IN_BYTES = GetAlignmentInBytes(DEFAULT_ALIGNMENT);

// Internal objects do not all need to be 64 bits aligned
// this alignment helps to reduce memory usage on 32 bits machines
constexpr Alignment DEFAULT_INTERNAL_ALIGNMENT = GetLogAlignment(sizeof(uintptr_t));
constexpr size_t DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES = GetAlignmentInBytes(DEFAULT_INTERNAL_ALIGNMENT);

constexpr size_t GetAlignedObjectSize(size_t size)
{
    return AlignUp(size, DEFAULT_ALIGNMENT_IN_BYTES);
}

template <typename T>
constexpr Alignment GetAlignment()
{
    return GetLogAlignment(std::max(alignof(T), DEFAULT_ALIGNMENT_IN_BYTES));
}

constexpr Alignment GetAlignment(size_t align)
{
    return GetLogAlignment(std::max(align, DEFAULT_ALIGNMENT_IN_BYTES));
}

template <typename T>
constexpr Alignment GetInternalAlignment()
{
    return GetLogAlignment(std::max(alignof(T), DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES));
}

constexpr Alignment GetInternalAlignment(size_t align)
{
    return GetLogAlignment(std::max(align, DEFAULT_INTERNAL_ALIGNMENT_IN_BYTES));
}

/*
    uint64_t return type usage in memory literals for giving
    compile-time error in case of integer overflow
*/

constexpr uint64_t SHIFT_KB = 10ULL;
constexpr uint64_t SHIFT_MB = 20ULL;
constexpr uint64_t SHIFT_GB = 30ULL;

constexpr uint64_t operator"" _KB(long double count)
{
    return static_cast<uint64_t>(count * (1ULL << SHIFT_KB));
}

// NOLINTNEXTLINE(google-runtime-int)
constexpr uint64_t operator"" _KB(unsigned long long count)
{
    return count * (1ULL << SHIFT_KB);
}

constexpr uint64_t operator"" _MB(long double count)
{
    return static_cast<uint64_t>(count * (1ULL << SHIFT_MB));
}

// NOLINTNEXTLINE(google-runtime-int)
constexpr uint64_t operator"" _MB(unsigned long long count)
{
    return count * (1ULL << SHIFT_MB);
}

constexpr uint64_t operator"" _GB(long double count)
{
    return static_cast<uint64_t>(count * (1ULL << SHIFT_GB));
}

// NOLINTNEXTLINE(google-runtime-int)
constexpr uint64_t operator"" _GB(unsigned long long count)
{
    return count * (1ULL << SHIFT_GB);
}

constexpr uint64_t SIZE_1K = 1_KB;
constexpr uint64_t SIZE_1M = 1_MB;
constexpr uint64_t SIZE_1G = 1_GB;

constexpr uint64_t PANDA_MAX_HEAP_SIZE = 4_GB;
constexpr size_t PANDA_POOL_ALIGNMENT_IN_BYTES = 256_KB;

constexpr size_t PANDA_DEFAULT_POOL_SIZE = 1_MB;
constexpr size_t PANDA_DEFAULT_ARENA_SIZE = 1_MB;
constexpr size_t PANDA_DEFAULT_ALLOCATOR_POOL_SIZE = 4_MB;
static_assert(PANDA_DEFAULT_POOL_SIZE % PANDA_POOL_ALIGNMENT_IN_BYTES == 0);
static_assert(PANDA_DEFAULT_ARENA_SIZE % PANDA_POOL_ALIGNMENT_IN_BYTES == 0);
static_assert(PANDA_DEFAULT_ALLOCATOR_POOL_SIZE % PANDA_POOL_ALIGNMENT_IN_BYTES == 0);

constexpr Alignment DEFAULT_FRAME_ALIGNMENT = LOG_ALIGN_6;

constexpr uintptr_t PANDA_32BITS_HEAP_START_ADDRESS = AlignUp(1U, PANDA_POOL_ALIGNMENT_IN_BYTES);
constexpr uint64_t PANDA_32BITS_HEAP_END_OBJECTS_ADDRESS = 4_GB;

constexpr bool IsAddressInObjectsHeap([[maybe_unused]] uintptr_t address)
{
#if defined(PANDA_32_BIT_MANAGED_POINTER) && defined(PANDA_TARGET_64)
    return PANDA_32BITS_HEAP_START_ADDRESS <= address && address < PANDA_32BITS_HEAP_END_OBJECTS_ADDRESS;
#else  // In this case, all 64 bits addresses are valid
    return true;
#endif
}

template <class T>
constexpr bool IsAddressInObjectsHeap(const T *address)
{
    return IsAddressInObjectsHeap(ToUintPtr(address));
}

constexpr bool IsAddressInObjectsHeapOrNull(uintptr_t address)
{
    return address == ToUintPtr(nullptr) || IsAddressInObjectsHeap(address);
}

template <class T>
constexpr bool IsAddressInObjectsHeapOrNull(const T *address)
{
    return IsAddressInObjectsHeapOrNull(ToUintPtr(address));
}

template <class T>
constexpr ObjectPointerType ToObjPtrType(const T *val)
{
    ASSERT(IsAddressInObjectsHeapOrNull(ToUintPtr(val)));
    return static_cast<ObjectPointerType>(ToUintPtr(val));
}

constexpr ObjectPointerType ToObjPtrType(std::nullptr_t)
{
    return static_cast<ObjectPointerType>(ToUintPtr(nullptr));
}

enum class ObjectStatus : bool {
    DEAD_OBJECT,
    ALIVE_OBJECT,
};

using MemVisitor = std::function<void(void *mem, size_t size)>;
using GCObjectVisitor = std::function<ObjectStatus(ObjectHeader *)>;
using ObjectMoveVisitor = std::add_pointer<size_t(void *mem)>::type;
using ObjectVisitor = std::function<void(ObjectHeader *)>;
/// from_object is object from which we found to_object by reference.
using ObjectVisitorEx = std::function<void(ObjectHeader *fromObject, ObjectHeader *toObject)>;
using ObjectChecker = std::function<bool(const ObjectHeader *)>;
using GCRootVisitor = std::function<void(mem::GCRoot)>;
using GCRootUpdater = std::function<bool(ObjectHeader **)>;
using ReferenceUpdater = std::function<ObjectStatus(ObjectHeader **)>;
using MemRangeChecker = std::function<bool(mem::MemRange &)>;

inline bool NoFilterChecker([[maybe_unused]] const ObjectHeader *objectHeader)
{
    return true;
}

inline ObjectStatus GCKillEmAllVisitor([[maybe_unused]] const ObjectHeader *mem)
{
    return ObjectStatus::DEAD_OBJECT;
}

}  // namespace ark

#ifdef PANDA_TARGET_MACOS
// macOS page size is not constexpr, and is defined as vm_page_size
// which is extern  vm_size_t vm_page_size;
#undef PAGE_SIZE
#endif

// If the OS has this macro, do not redefine it.
#ifdef PAGE_SIZE
inline constexpr size_t PANDA_PAGE_SIZE = PAGE_SIZE;
#else
// NB! Keep inline to avoid ODR-violation
inline constexpr size_t PANDA_PAGE_SIZE = 4096U;
inline constexpr size_t PAGE_SIZE = PANDA_PAGE_SIZE;
#endif

#endif  // LIBPANDABASE_MEM_H

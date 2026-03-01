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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_INFO_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_INFO_H

#include <cstdint>
#include <cstddef>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>
#ifdef _WIN64
#include <memoryapi.h>
#include <errhandlingapi.h>
#include <handleapi.h>
#else
#include <sys/mman.h>
#endif
#include "common_components/base/globals.h"
#include "common_components/base/mem_utils.h"
#include "common_components/base/rw_lock.h"
#include "common_components/heap/collector/copy_data_manager.h"
#include "common_components/heap/collector/gc_infos.h"
#include "common_components/heap/collector/region_bitmap.h"
#include "common_components/heap/collector/region_rset.h"
#include "common_components/log/log.h"
#include "common_components/platform/map.h"
#include "securec.h"
#ifdef COMMON_ASAN_SUPPORT
#include "common_components/sanitizer/sanitizer_interface.h"
#endif

namespace common {
template<typename T>
class BitFields {
public:
    // pos: the position where the bit locates. It starts from 0.
    // bitLen: the length that is to be read.
    T AtomicGetValue(size_t pos, size_t bitLen) const
    {
        T value = __atomic_load_n(&fieldVal_, __ATOMIC_ACQUIRE);
        T bitMask = ((1 << bitLen) - 1) << pos;
        return (value & bitMask) >> pos;
    }

    void AtomicSetValue(size_t pos, size_t bitLen, T newValue)
    {
        do {
            T oldValue = fieldVal_;
            T bitMask = ((1 << bitLen) - 1) << pos;
            T unchangedBitMask = ~bitMask;
            T newFieldValue = ((newValue << pos) & bitMask) | (oldValue & unchangedBitMask);
            if (__atomic_compare_exchange_n(&fieldVal_, &oldValue, newFieldValue, false, __ATOMIC_ACQ_REL,
                                            __ATOMIC_ACQUIRE)) {
                return;
            }
        } while (true);
    }

private:
    T fieldVal_;
};
// this class is the metadata of region, it contains all the information needed to manage its corresponding memory.
// Region memory is composed of several Units, described by UnitInfo.
// sizeof(RegionDesc) must be equal to sizeof(UnitInfo). We rely on this fact to calculate region-related address.

// region info is stored in the metadata of its primary unit (i.e. the first unit).
class RegionDesc {
public:
    enum RouteState : uint8_t {
        INITIAL = 0,
        COPYABLE,
        ROUTING,
        ROUTED,
        COMPACTED,
        COPYED,
    };

    // default common region unit size.
    static constexpr size_t UNIT_SIZE = 256 * KB;

    // threshold for object to unique a region
    static constexpr size_t LARGE_OBJECT_DEFAULT_THRESHOLD = UNIT_SIZE * 2 / 3;

    // release a large object when the size is greater than 4096KB.
    static constexpr size_t LARGE_OBJECT_RELEASE_THRESHOLD = 4096 * KB;

    static constexpr size_t DEFAULT_REGION_UNIT_MASK = RegionDesc::UNIT_SIZE - 1;

    RegionDesc()
    {
        metadata.allocPtr = reinterpret_cast<uintptr_t>(nullptr);
        metadata.markingLine = std::numeric_limits<uintptr_t>::max();
        metadata.forwardLine = std::numeric_limits<uintptr_t>::max();
        metadata.freeSlot = nullptr;
        metadata.regionBase = reinterpret_cast<uintptr_t>(nullptr);
        metadata.regionStart = reinterpret_cast<uintptr_t>(nullptr);
        metadata.regionEnd = reinterpret_cast<uintptr_t>(nullptr);
        metadata.regionRSet = nullptr;
    }
    static inline RegionDesc* NullRegion()
    {
        static RegionDesc nullRegion;
        return &nullRegion;
    }

    void SetReadOnly()
    {
        constexpr int pageProtRead = 1;
        DLOG(REPORT, "try to set readonly to %p, size is %ld", GetRegionBase(), GetRegionBaseSize());
        if (PageProtect(reinterpret_cast<void *>(GetRegionBase()), GetRegionBaseSize(), pageProtRead) != 0) {
            DLOG(REPORT, "set read only fail");
        }
    }

    void ClearReadOnly()
    {
        constexpr int pageProtReadWrite = 3;
        DLOG(REPORT, "try to set read & write to %p, size is %ld", GetRegionBase(), GetRegionBaseSize());
        if (PageProtect(reinterpret_cast<void *>(GetRegionBase()), GetRegionBaseSize(), pageProtReadWrite) != 0) {
            DLOG(REPORT, "clear read only fail");
        }
    }

    RegionBitmap *GetMarkBitmap()
    {
        RegionBitmap *bitmap = __atomic_load_n(&metadata.liveInfo_.markBitmap_, std::memory_order_acquire);
        if (reinterpret_cast<HeapAddress>(bitmap) == RegionLiveDesc::TEMPORARY_PTR) {
            return nullptr;
        }
        return bitmap;
    }

    ALWAYS_INLINE_CC RegionBitmap* GetOrAllocMarkBitmap()
    {
        do {
            RegionBitmap *bitmap = __atomic_load_n(&metadata.liveInfo_.markBitmap_, std::memory_order_acquire);
            if (UNLIKELY_CC(reinterpret_cast<uintptr_t>(bitmap) == RegionLiveDesc::TEMPORARY_PTR)) {
                continue;
            }
            if (LIKELY_CC(bitmap != nullptr)) {
                return bitmap;
            }
            RegionBitmap *newValue = reinterpret_cast<RegionBitmap *>(RegionLiveDesc::TEMPORARY_PTR);
            if (__atomic_compare_exchange_n(&metadata.liveInfo_.markBitmap_, &bitmap, newValue, false,
                                            std::memory_order_seq_cst, std::memory_order_relaxed)) {
                RegionBitmap *allocated =
                    HeapBitmapManager::GetHeapBitmapManager().AllocateRegionBitmap(GetRegionBaseSize());
                __atomic_store_n(&metadata.liveInfo_.markBitmap_, allocated, std::memory_order_release);
                DLOG(REGION, "region %p(base=%#zx)@%#zx liveinfo %p alloc markbitmap %p",
                    this, GetRegionBase(), GetRegionStart(), &metadata.liveInfo_, metadata.liveInfo_.markBitmap_);
                return allocated;
            }
        } while (true);

        return nullptr;
    }

    RegionBitmap *GetResurrectBitmap()
    {
        RegionBitmap *bitmap = __atomic_load_n(&metadata.liveInfo_.resurrectBitmap_, std::memory_order_acquire);
        if (reinterpret_cast<HeapAddress>(bitmap) == RegionLiveDesc::TEMPORARY_PTR) {
            return nullptr;
        }
        return bitmap;
    }

    RegionBitmap *GetOrAllocResurrectBitmap()
    {
        do {
            RegionBitmap *bitmap = __atomic_load_n(&metadata.liveInfo_.resurrectBitmap_, std::memory_order_acquire);
            if (UNLIKELY_CC(reinterpret_cast<uintptr_t>(bitmap) == RegionLiveDesc::TEMPORARY_PTR)) {
                continue;
            }
            if (LIKELY_CC(bitmap != nullptr)) {
                return bitmap;
            }
            RegionBitmap *newValue = reinterpret_cast<RegionBitmap *>(RegionLiveDesc::TEMPORARY_PTR);
            if (__atomic_compare_exchange_n(&metadata.liveInfo_.resurrectBitmap_, &bitmap, newValue, false,
                                            std::memory_order_seq_cst, std::memory_order_relaxed)) {
                RegionBitmap *allocated =
                    HeapBitmapManager::GetHeapBitmapManager().AllocateRegionBitmap(GetRegionBaseSize());
                __atomic_store_n(&metadata.liveInfo_.resurrectBitmap_, allocated, std::memory_order_release);
                DLOG(REGION, "region %p(base=%#zx)@%#zx liveinfo %p alloc resurrectbitmap %p",
                     this, GetRegionBase(), GetRegionStart(), &metadata.liveInfo_,
                     metadata.liveInfo_.resurrectBitmap_);
                return allocated;
            }
        } while (true);

        return nullptr;
    }

    RegionBitmap* GetEnqueueBitmap()
    {
        RegionBitmap *bitmap = __atomic_load_n(&metadata.liveInfo_.enqueueBitmap_, std::memory_order_acquire);
        if (reinterpret_cast<HeapAddress>(bitmap) == RegionLiveDesc::TEMPORARY_PTR) {
            return nullptr;
        }
        return bitmap;
    }

    RegionBitmap* GetOrAllocEnqueueBitmap()
    {
        do {
            RegionBitmap *bitmap = __atomic_load_n(&metadata.liveInfo_.enqueueBitmap_, std::memory_order_acquire);
            if (UNLIKELY_CC(reinterpret_cast<uintptr_t>(bitmap) == RegionLiveDesc::TEMPORARY_PTR)) {
                continue;
            }
            if (LIKELY_CC(bitmap != nullptr)) {
                return bitmap;
            }
            RegionBitmap* newValue = reinterpret_cast<RegionBitmap *>(RegionLiveDesc::TEMPORARY_PTR);
            if (__atomic_compare_exchange_n(&metadata.liveInfo_.enqueueBitmap_, &bitmap, newValue, false,
                                            std::memory_order_seq_cst, std::memory_order_relaxed)) {
                RegionBitmap *allocated =
                    HeapBitmapManager::GetHeapBitmapManager().AllocateRegionBitmap(GetRegionBaseSize());
                __atomic_store_n(&metadata.liveInfo_.enqueueBitmap_, allocated, std::memory_order_release);
                DLOG(REGION, "region %p(base=%#zx)@%#zx liveinfo %p alloc enqueuebitmap %p",
                     this, GetRegionBase(), GetRegionStart(), &metadata.liveInfo_, metadata.liveInfo_.enqueueBitmap_);
                return allocated;
            }
        } while (true);

        return nullptr;
    }

    void ResetMarkBit()
    {
        SetMarkedRegionFlag(0);
        SetEnqueuedRegionFlag(0);
        SetResurrectedRegionFlag(0);
    }

    NO_INLINE_CC bool MarkObjectForLargeRegion(const BaseObject* obj)
    {
        if (metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_MARKED_REGION, 1) != 1) {
            SetMarkedRegionFlag(1);
            return false;
        }
        return true;
    }
    ALWAYS_INLINE_CC bool MarkObject(const BaseObject* obj)
    {
        if (IsLargeRegion()) {
            return MarkObjectForLargeRegion(obj);
        }
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        bool marked = GetOrAllocMarkBitmap()->MarkBits(offset);
        DCHECK_CC(IsMarkedObject(obj));
        return marked;
    }

    bool ResurrentObject(const BaseObject* obj)
    {
        if (IsLargeRegion()) {
            if (metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_RESURRECTED_REGION, 1) != 1) {
                SetResurrectedRegionFlag(1);
                return false;
            }
            return true;
        }
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        bool marked = GetOrAllocResurrectBitmap()->MarkBits(offset);
        CHECK_CC(IsResurrectedObject(obj));
        return marked;
    }

    bool EnqueueObject(const BaseObject* obj)
    {
        if (IsLargeRegion()) {
            if (metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_ENQUEUED_REGION, 1) != 1) {
                SetEnqueuedRegionFlag(1);
                return false;
            }
            return true;
        }
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        bool marked = GetOrAllocEnqueueBitmap()->MarkBits(offset);
        CHECK_CC(IsEnqueuedObject(obj));
        return marked;
    }

    bool IsResurrectedObject(const BaseObject* obj)
    {
        if (IsLargeRegion()) {
            return (metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_RESURRECTED_REGION, 1) == 1);
        }
        RegionBitmap* resurrectBitmap = GetResurrectBitmap();
        if (resurrectBitmap == nullptr) {
            return false;
        }
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        return resurrectBitmap->IsMarked(offset);
    }

    bool IsMarkedObject(const BaseObject* obj)
    {
        if (IsLargeRegion()) {
            return (metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_MARKED_REGION, 1) == 1);
        }
        RegionBitmap* markBitmap = GetMarkBitmap();
        if (markBitmap == nullptr) {
            return false;
        }
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        return markBitmap->IsMarked(offset);
    }

    bool IsEnqueuedObject(const BaseObject* obj)
    {
        if (IsLargeRegion()) {
            return (metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_ENQUEUED_REGION, 1) == 1);
        }
        RegionBitmap* enqueBitmap = GetEnqueueBitmap();
        if (enqueBitmap == nullptr) {
            return false;
        }
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        return enqueBitmap->IsMarked(offset);
    }

    RegionRSet* GetRSet()
    {
        return metadata.regionRSet;
    }

    void ClearRSet()
    {
        metadata.regionRSet->ClearCardTable();
    }

    bool MarkRSetCardTable(BaseObject* obj)
    {
        size_t offset = GetAddressOffset(reinterpret_cast<HeapAddress>(obj));
        return GetRSet()->MarkCardTable(offset);
    }

    ALWAYS_INLINE_CC size_t GetAddressOffset(HeapAddress address)
    {
        DCHECK_CC(GetRegionBaseFast() <= address);
        return (address - GetRegionBaseFast());
    }

    enum class UnitRole : uint8_t {
        // for the head unit
        FREE_UNITS = 0,
        SMALL_SIZED_UNITS,
        LARGE_SIZED_UNITS,

        SUBORDINATE_UNIT,
    };

    // region is and must be one of following types during its whole lifecycle.
    // one-to-one mapping to region-lists.

    enum class RegionType : uint8_t {
        FREE_REGION,
        GARBAGE_REGION,

        // ************************boundary of dead region and alive region**************************

        THREAD_LOCAL_REGION,
        RECENT_FULL_REGION,
        FROM_REGION,
        EXEMPTED_FROM_REGION,
        LONE_FROM_REGION,
        TO_REGION,
        OLD_REGION,
        THREAD_LOCAL_OLD_REGION,

        // non movable object will not be forwarded by concurrent copying gc.
        RECENT_POLYSIZE_NONMOVABLE_REGION,
        FULL_POLYSIZE_NONMOVABLE_REGION,
        MONOSIZE_NONMOVABLE_REGION,
        FULL_MONOSIZE_NONMOVABLE_REGION,

        // region for raw-pointer objects which are exposed to runtime thus can not be moved by any gc.
        // raw-pointer region becomes pinned region when none of its member objects are used as raw pointer.
        RAW_POINTER_REGION,

        // allocation context is able and responsible to determine whether it is safe to be collected.
        TL_RAW_POINTER_REGION,

        RECENT_LARGE_REGION,
        LARGE_REGION,

        READ_ONLY_REGION,
        APPSPAWN_REGION,

        END_OF_REGION_TYPE,

        ALIVE_REGION_FIRST = THREAD_LOCAL_REGION,
    };

    static bool IsAliveRegionType(RegionType type)
    {
        return static_cast<uint8_t>(type) >= static_cast<uint8_t>(RegionType::ALIVE_REGION_FIRST);
    }

    static bool IsInRecentSpace(RegionType type)
    {
        return type == RegionType::THREAD_LOCAL_REGION || type == RegionType::RECENT_FULL_REGION;
    }

    static bool IsInYoungSpaceForWB(RegionType type)
    {
        return type == RegionType::THREAD_LOCAL_REGION || type == RegionType::RECENT_FULL_REGION ||
            type == RegionType::FROM_REGION;
    }

    static bool IsInYoungSpace(RegionType type)
    {
        return type == RegionType::THREAD_LOCAL_REGION || type == RegionType::RECENT_FULL_REGION ||
            type == RegionType::FROM_REGION || type == RegionType::EXEMPTED_FROM_REGION;
    }

    static bool IsInFromSpace(RegionType type)
    {
        return type == RegionType::FROM_REGION || type == RegionType::EXEMPTED_FROM_REGION;
    }

    static bool IsInToSpace(RegionType type)
    {
        return type == RegionType::TO_REGION;
    }

    static bool IsInOldSpace(RegionType type)
    {
        return type == RegionType::OLD_REGION;
    }

    static void Initialize(size_t nUnit, uintptr_t regionInfoAddr, uintptr_t heapAddress)
    {
        UnitInfo::totalUnitCount = nUnit;
        UnitInfo::unitInfoStart = regionInfoAddr;
        UnitInfo::heapStartAddress = heapAddress;
    }

    static RegionDesc* GetRegionDesc(uint32_t idx)
    {
        UnitInfo* unit = RegionDesc::UnitInfo::GetUnitInfo(idx);
        DCHECK_CC((reinterpret_cast<uintptr_t>(unit) % 8) == 0);  // 8: Align with 8
        DCHECK_CC(static_cast<UnitRole>(unit->GetMetadata().unitRole) != UnitRole::SUBORDINATE_UNIT);
        return reinterpret_cast<RegionDesc*>(unit);
    }

    static RegionDesc* GetRegionDescAt(uintptr_t allocAddr)
    {
        ASSERT_LOGF(Heap::IsHeapAddress(allocAddr), "Cannot get region info of a non-heap object");
        UnitInfo* unit = reinterpret_cast<UnitInfo*>(UnitInfo::heapStartAddress -
                                                     (((allocAddr - UnitInfo::heapStartAddress) / UNIT_SIZE) + 1) *
                                                         sizeof(RegionDesc));
        DCHECK_CC((reinterpret_cast<uintptr_t>(unit) % 8) == 0);  // 8: Align with 8
        DCHECK_CC(static_cast<UnitRole>(unit->GetMetadata().unitRole) != UnitRole::SUBORDINATE_UNIT);
        return reinterpret_cast<RegionDesc*>(unit);
    }

    // This could only used for surely alive region, such as from interpreter,
    // because ONLY alive region have `InlinedRegionMetaData`
    static RegionDesc* GetAliveRegionDescAt(uintptr_t allocAddr)
    {
        // only alive region have `InlinedRegionMetaData`.
        DCHECK_CC(IsAliveRegionType(GetRegionDescAt(allocAddr)->GetRegionType()));
        InlinedRegionMetaData *metaData = InlinedRegionMetaData::GetInlinedRegionMetaData(allocAddr);
        UnitInfo *unit = reinterpret_cast<UnitInfo*>(metaData->regionDesc_);
        DCHECK_CC(reinterpret_cast<RegionDesc *>(unit) == GetRegionDescAt(allocAddr));
        DCHECK_CC((reinterpret_cast<uintptr_t>(unit) % 8) == 0);  // 8: Align with 8
        DCHECK_CC(static_cast<UnitRole>(unit->GetMetadata().unitRole) != UnitRole::SUBORDINATE_UNIT);
        return reinterpret_cast<RegionDesc*>(unit);
    }

    static void InitFreeRegion(size_t unitIdx, size_t nUnit)
    {
        RegionDesc* region = reinterpret_cast<RegionDesc*>(RegionDesc::UnitInfo::GetUnitInfo(unitIdx));
        region->InitRegionDesc(nUnit, UnitRole::FREE_UNITS);
    }

    static RegionDesc* ResetRegion(size_t unitIdx, size_t nUnit, RegionDesc::UnitRole uclass)
    {
        RegionDesc* region = reinterpret_cast<RegionDesc*>(RegionDesc::UnitInfo::GetUnitInfo(unitIdx));
        region->ResetRegion(nUnit, uclass);
        return region;
    }

    static RegionDesc* InitRegion(size_t unitIdx, size_t nUnit, RegionDesc::UnitRole uclass)
    {
        RegionDesc* region = reinterpret_cast<RegionDesc*>(RegionDesc::UnitInfo::GetUnitInfo(unitIdx));
        region->InitRegion(nUnit, uclass);
        return region;
    }

    static RegionDesc* InitRegionAt(uintptr_t addr, size_t nUnit, RegionDesc::UnitRole uclass)
    {
        size_t idx = RegionDesc::UnitInfo::GetUnitIdxAt(addr);
        return InitRegion(idx, nUnit, uclass);
    }

    static HeapAddress GetUnitAddress(size_t unitIdx) { return UnitInfo::GetUnitAddress(unitIdx); }

    static void ClearUnits(size_t idx, size_t cnt)
    {
        uintptr_t unitAddress = RegionDesc::GetUnitAddress(idx);
        size_t size = cnt * RegionDesc::UNIT_SIZE;
        DLOG(REGION, "clear dirty units[%zu+%zu, %zu) @[%#zx+%zu, %#zx)", idx, cnt, idx + cnt, unitAddress, size,
             RegionDesc::GetUnitAddress(idx + cnt));
        MemorySet(unitAddress, size, 0, size);
    }

    static void ReleaseUnits(size_t idx, size_t cnt)
    {
        void* unitAddress = reinterpret_cast<void*>(RegionDesc::GetUnitAddress(idx));
        size_t size = cnt * RegionDesc::UNIT_SIZE;
        DLOG(REGION, "release physical memory for units [%zu+%zu, %zu) @[%p+%zu, 0x%zx)", idx, cnt, idx + cnt,
             unitAddress, size, RegionDesc::GetUnitAddress(idx + cnt));
#if defined(_WIN64)
        LOGE_IF(UNLIKELY_CC(!VirtualFree(unitAddress, size, MEM_DECOMMIT))) <<
            "VirtualFree failed in ReturnPage, errno: " << GetLastError();

#elif defined(__APPLE__)
        MemorySet(reinterpret_cast<uintptr_t>(unitAddress), size, 0, size);
        (void)madvise(unitAddress, size, MADV_DONTNEED);
#else
        (void)madvise(unitAddress, size, MADV_DONTNEED);
#endif
#ifdef COMMON_ASAN_SUPPORT
        Sanitizer::OnHeapMadvise(unitAddress, size);
#endif
#ifdef USE_HWASAN
        ASAN_POISON_MEMORY_REGION(unitAddress, size);
        const uintptr_t pSize = size;
        LOG_COMMON(DEBUG) << std::hex << "set [" << unitAddress << ", " <<
            (reinterpret_cast<uintptr_t>(unitAddress) + pSize) << ") poisoned\n";
#endif
    }

    size_t GetRegionSize() const
    {
        DCHECK_CC(GetRegionEnd() > GetRegionStart());
        return GetRegionEnd() - GetRegionStart();
    }

    size_t GetRegionBaseSize() const
    {
        DCHECK_CC(GetRegionEnd() > GetRegionBase());
        return GetRegionEnd() - GetRegionBase();
    }

    size_t GetUnitCount() const { return GetRegionBaseSize() / UNIT_SIZE; }

    size_t GetAvailableSize() const
    {
        ASSERT_LOGF(IsSmallRegion(), "wrong region type");
        return GetRegionEnd() - GetRegionAllocPtr();
    }

    size_t GetRegionAllocatedSize() const { return GetRegionAllocPtr() - GetRegionStart(); }

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void DumpRegionDesc(LogType type) const;
    const char* GetTypeName() const;
#endif

    void VisitAllObjects(const std::function<void(BaseObject*)>&& func);
    void VisitAllObjectsBeforeCopy(const std::function<void(BaseObject*)>&& func);
    bool VisitLiveObjectsUntilFalse(const std::function<bool(BaseObject*)>&& func);

    void VisitRememberSetBeforeMarking(const std::function<void(BaseObject*)>& func);
    void VisitRememberSetBeforeCopy(const std::function<void(BaseObject*)>& func);
    void VisitRememberSet(const std::function<void(BaseObject*)>& func);

    // reset so that this region can be reused for allocation
    void InitFreeUnits()
    {
        if (metadata.regionRSet != nullptr) {
            RegionRSet::DestroyRegionRSet(metadata.regionRSet);
            metadata.regionRSet = nullptr;
        }
        size_t nUnit = GetUnitCount();
        UnitInfo* unit = reinterpret_cast<UnitInfo*>(this) - (nUnit - 1);
        for (size_t i = 0; i < nUnit; ++i) {
            unit[i].ToFreeRegion();
        }
    }

    void ClearLiveInfo()
    {
        DCHECK_CC(metadata.liveInfo_.relatedRegion_ == this);
        metadata.liveInfo_.ClearLiveInfo();
        DLOG(REGION, "region %p(base=%#zx)@%#zx+%zu clear liveinfo %p type %u",
             this, GetRegionBase(), GetRegionStart(), GetRegionSize(), &metadata.liveInfo_, GetRegionType());
        metadata.liveByteCount = 0;
    }

    // These interfaces are used to make sure the writing operations of value in C++ Bit Field will be atomic.
    void SetUnitRole(UnitRole role)
    {
        metadata.unitBits.AtomicSetValue(0, BITS_5, static_cast<uint8_t>(role));
    }
    void SetRegionType(RegionType type)
    {
        metadata.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_REGION_TYPE,
            BITS_5, static_cast<uint8_t>(type));
        if (IsAliveRegionType(type)) {
            InlinedRegionMetaData::GetInlinedRegionMetaData(this)->SetRegionType(type);
        }
    }

    void SetMarkedRegionFlag(uint8_t flag)
    {
        metadata.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_MARKED_REGION, 1, flag);
    }
    void SetEnqueuedRegionFlag(uint8_t flag)
    {
        metadata.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_ENQUEUED_REGION, 1, flag);
    }
    void SetResurrectedRegionFlag(uint8_t flag)
    {
        metadata.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_RESURRECTED_REGION, 1, flag);
    }

    void SetRegionCellCount(uint8_t cellCount)
    {
        // 7: region cell count is 7 bits.
        metadata.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_REGION_CELLCOUNT, 7, cellCount);
    }

    uint16_t GetRegionCellCount()
    {
        // 7: region cell count is 7 bits.
        return metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_REGION_CELLCOUNT, 7);
    }

    void SetJitFortAwaitInstallFlag(uint8_t flag)
    {
        metadata.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_IS_JITFORT_AWAIT_INSTALL, 1, flag);
    }

    bool IsJitFortAwaitInstallFlag()
    {
        return metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_IS_JITFORT_AWAIT_INSTALL, 1);
    }

    RegionType GetRegionType() const
    {
        return static_cast<RegionType>(metadata.regionBits.AtomicGetValue(RegionBitOffset::BIT_OFFSET_REGION_TYPE,
                                                                          BITS_5));
    }

    static RegionType GetAliveRegionType(uintptr_t allocAddr)
    {
        // only alive region have `InlinedRegionMetaData`.
        DCHECK_CC(IsAliveRegionType(GetRegionDescAt(allocAddr)->GetRegionType()));
        InlinedRegionMetaData *metaData = InlinedRegionMetaData::GetInlinedRegionMetaData(allocAddr);
        return metaData->GetRegionType();
    }

    UnitRole GetUnitRole() const { return static_cast<UnitRole>(metadata.unitRole); }

    size_t GetUnitIdx() const { return RegionDesc::UnitInfo::GetUnitIdx(reinterpret_cast<const UnitInfo*>(this)); }

    HeapAddress GetRegionBase() const { return RegionDesc::GetUnitAddress(GetUnitIdx()); }

    // This could only used to a `RegionDesc` which has been initialized
    HeapAddress GetRegionBaseFast() const
    {
        ASSERT_LOGF(metadata.regionBase == GetRegionBase(), "");
        return metadata.regionBase;
    }

    HeapAddress GetRegionStart() const { return metadata.regionStart; }

    HeapAddress GetRegionEnd() const { return metadata.regionEnd; }

    void SetRegionAllocPtr(HeapAddress addr) { metadata.allocPtr = addr; }

    HeapAddress GetRegionAllocPtr() const { return metadata.allocPtr; }

    HeapAddress GetMarkingLine() const { return metadata.markingLine; }
    HeapAddress GetCopyLine() const { return metadata.forwardLine; }

    void SetMarkingLine()
    {
        if (metadata.markingLine == std::numeric_limits<uintptr_t>::max()) {
            uintptr_t line = GetRegionAllocPtr();
            metadata.markingLine = line;
            DLOG(REGION, "set region %p(base=%#zx)@%#zx+%zu marking-line %#zx type %u",
                 this, GetRegionBase(), GetRegionStart(), GetRegionSize(), GetMarkingLine(), GetRegionType());
        }
    }

    void SetCopyLine()
    {
        if (metadata.forwardLine == std::numeric_limits<uintptr_t>::max()) {
            uintptr_t line = GetRegionAllocPtr();
            metadata.forwardLine = line;
            DLOG(REGION, "set region %p(base=%#zx)@%#zx+%zu copy-line %#zx type %u",
                 this, GetRegionBase(), GetRegionStart(), GetRegionSize(), GetCopyLine(), GetRegionType());
        }
    }

    void ClearMarkingCopyLine()
    {
        metadata.markingLine = std::numeric_limits<uintptr_t>::max();
        metadata.forwardLine = std::numeric_limits<uintptr_t>::max();
    }

    void ClearFreeSlot()
    {
        metadata.freeSlot = nullptr;
    }

    bool IsNewObjectSinceMarking(const BaseObject* obj)
    {
        return GetMarkingLine() <= reinterpret_cast<uintptr_t>(obj);
    }

    bool IsNewObjectSinceForward(const BaseObject* obj)
    {
        return GetCopyLine() <= reinterpret_cast<uintptr_t>(obj);
    }

    bool IsNewRegionSinceForward() const
    {
        return GetCopyLine() == GetRegionStart();
    }

    bool IsInRecentSpace() const
    {
        RegionType type = GetRegionType();
        return RegionDesc::IsInRecentSpace(type);
    }

    bool IsInYoungSpace() const
    {
        RegionType type = GetRegionType();
        return RegionDesc::IsInYoungSpace(type);
    }

    bool IsInFromSpace() const
    {
        RegionType type = GetRegionType();
        return RegionDesc::IsInFromSpace(type);
    }

    bool IsInToSpace() const
    {
        RegionType type = GetRegionType();
        return RegionDesc::IsInToSpace(type);
    }

    bool IsInOldSpace() const
    {
        RegionType type = GetRegionType();
        return RegionDesc::IsInOldSpace(type);
    }

    int32_t IncRawPointerObjectCount()
    {
        int32_t oldCount = __atomic_fetch_add(&metadata.rawPointerObjectCount, 1, __ATOMIC_SEQ_CST);
        LOGF_CHECK(oldCount >= 0) << "region " << this << " has wrong raw pointer count " << oldCount;
        LOGF_CHECK(oldCount < MAX_RAW_POINTER_COUNT) << "inc raw-pointer-count overflow";
        return oldCount;
    }

    int32_t DecRawPointerObjectCount()
    {
        int32_t oldCount = __atomic_fetch_sub(&metadata.rawPointerObjectCount, 1, __ATOMIC_SEQ_CST);
        LOGF_CHECK(oldCount > 0) << "dec raw-pointer-count underflow, please check whether releaseRawData is overused.";
        return oldCount;
    }

    int32_t GetRawPointerObjectCount() const
    {
        return __atomic_load_n(&metadata.rawPointerObjectCount, __ATOMIC_SEQ_CST);
    }

    bool CompareAndSwapRawPointerObjectCount(int32_t expectVal, int32_t newVal)
    {
        return __atomic_compare_exchange_n(&metadata.rawPointerObjectCount, &expectVal, newVal, false, __ATOMIC_SEQ_CST,
                                           __ATOMIC_ACQUIRE);
    }

    uintptr_t Alloc(size_t size)
    {
        DCHECK_CC(size > 0);
        size_t limit = GetRegionEnd();
        if (metadata.allocPtr + size <= limit) {
            uintptr_t addr = metadata.allocPtr;
            metadata.allocPtr += size;
            return addr;
        } else {
            return 0;
        }
    }

    // for regions shared by multithreads
    uintptr_t AtomicAlloc(size_t size)
    {
        uintptr_t addr = __atomic_fetch_add(&metadata.allocPtr, size, __ATOMIC_ACQ_REL);
        // should not check allocPtr, because it might be shared
        if ((addr < GetRegionEnd()) && (size <= GetRegionEnd() - addr)) {
            return addr;
        }
        if (addr <= GetRegionEnd()) {
            __atomic_store_n(&metadata.allocPtr, addr, __ATOMIC_SEQ_CST);
        }
        return 0;
    }

    // copyable during concurrent copying gc.
    bool IsSmallRegion() const { return static_cast<UnitRole>(metadata.unitRole) == UnitRole::SMALL_SIZED_UNITS; }

    ALWAYS_INLINE_CC bool IsLargeRegion() const
    {
        return static_cast<UnitRole>(metadata.unitRole) == UnitRole::LARGE_SIZED_UNITS;
    }

    bool IsMonoSizeNonMovableRegion() const
    {
        return (GetRegionType()  == RegionType::MONOSIZE_NONMOVABLE_REGION) ||
            (GetRegionType()  == RegionType::FULL_MONOSIZE_NONMOVABLE_REGION);
    }
    
    bool IsThreadLocalRegion() const
    {
        return GetRegionType() == RegionType::THREAD_LOCAL_REGION ||
               GetRegionType() == RegionType::THREAD_LOCAL_OLD_REGION;
    }

    bool IsPolySizeNonMovableRegion() const
    {
        return (GetRegionType()  == RegionType::FULL_POLYSIZE_NONMOVABLE_REGION) ||
               (GetRegionType()  == RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION);
    }

    bool IsReadOnlyRegion() const
    {
        return GetRegionType()  == RegionType::READ_ONLY_REGION;
    }

    RegionDesc* GetPrevRegion() const
    {
        if (UNLIKELY_CC(metadata.prevRegionIdx == NULLPTR_IDX)) {
            return nullptr;
        }
        return reinterpret_cast<RegionDesc*>(UnitInfo::GetUnitInfo(metadata.prevRegionIdx));
    }

    bool CollectNonMovableGarbage(BaseObject* obj, size_t cellCount)
    {
        std::lock_guard<std::mutex> lg(metadata.regionMutex);
        if (IsFreeNonMovableObject(obj)) {
            return false;
        }
        size_t size = (cellCount + 1) * sizeof(uint64_t);
        ObjectSlot* head = reinterpret_cast<ObjectSlot*>(obj);
        head->SetNext(metadata.freeSlot, size);
        metadata.freeSlot = head;
        return true;
    }

    HeapAddress GetFreeSlot()
    {
        if (metadata.freeSlot == nullptr) {
            return 0;
        }
        ObjectSlot* res = metadata.freeSlot;
        metadata.freeSlot = reinterpret_cast<ObjectSlot*>(res->next_);
        res->next_ = 0;
        res->isFree_ = 0;
        return reinterpret_cast<HeapAddress>(res);
    }

    HeapAddress AllocNonMovableFromFreeList()
    {
        std::lock_guard<std::mutex> lg(metadata.regionMutex);
        HeapAddress addr = GetFreeSlot();
        if (addr == 0) {
            RegionDesc* region = GetNextRegion();
            do {
                if (region == nullptr) {
                    break;
                }
                addr = region->GetFreeSlot();
                region = region->GetNextRegion();
            } while (addr == 0);
        }
        return addr;
    }

    bool IsFreeNonMovableObject(BaseObject* object)
    {
        ObjectSlot* slot = reinterpret_cast<ObjectSlot*>(object);
        return slot->isFree_;
    }

    void SetPrevRegion(const RegionDesc* r)
    {
        if (UNLIKELY_CC(r == nullptr)) {
            metadata.prevRegionIdx = NULLPTR_IDX;
            return;
        }
        size_t prevIdx = r->GetUnitIdx();
        ASSERT_LOGF(prevIdx < NULLPTR_IDX, "exceeds the maxinum limit for region info");
        metadata.prevRegionIdx = static_cast<uint32_t>(prevIdx);
    }

    RegionDesc* GetNextRegion() const
    {
        if (UNLIKELY_CC(metadata.nextRegionIdx == NULLPTR_IDX)) {
            return nullptr;
        }
        DCHECK_CC(metadata.nextRegionIdx < UnitInfo::totalUnitCount);
        return reinterpret_cast<RegionDesc*>(UnitInfo::GetUnitInfo(metadata.nextRegionIdx));
    }

    void SetNextRegion(const RegionDesc* r)
    {
        if (UNLIKELY_CC(r == nullptr)) {
            metadata.nextRegionIdx = NULLPTR_IDX;
            return;
        }
        size_t nextIdx = r->GetUnitIdx();
        ASSERT_LOGF(nextIdx < NULLPTR_IDX, "exceeds the maxinum limit for region info");
        metadata.nextRegionIdx = static_cast<uint32_t>(nextIdx);
    }

    bool IsFromRegion() const { return GetRegionType()  == RegionType::FROM_REGION; }

    bool IsAppSpawnRegion() const { return GetRegionType()  == RegionType::APPSPAWN_REGION; }

    bool IsUnmovableFromRegion() const
    {
        return GetRegionType()  == RegionType::EXEMPTED_FROM_REGION ||
            GetRegionType()  == RegionType::RAW_POINTER_REGION;
    }

    bool IsToRegion() const { return GetRegionType()  == RegionType::TO_REGION; }
    bool IsOldRegion() const { return GetRegionType()  == RegionType::OLD_REGION; }

    bool IsGarbageRegion() const { return GetRegionType()  == RegionType::GARBAGE_REGION; }
    bool IsFreeRegion() const { return static_cast<UnitRole>(metadata.unitRole) == UnitRole::FREE_UNITS; }

    bool IsValidRegion() const
    {
        return static_cast<UnitRole>(metadata.unitRole) == UnitRole::SMALL_SIZED_UNITS ||
            static_cast<UnitRole>(metadata.unitRole) == UnitRole::LARGE_SIZED_UNITS;
    }

    uint32_t GetLiveByteCount() const { return metadata.liveByteCount; }

    void ResetLiveByteCount() { metadata.liveByteCount = 0; }

    void AddLiveByteCount(uint32_t count)
    {
        (void)__atomic_fetch_add(&metadata.liveByteCount, count, __ATOMIC_ACQ_REL);
    }

    void RemoveFromList()
    {
        RegionDesc* prev = GetPrevRegion();
        RegionDesc* next = GetNextRegion();
        if (prev != nullptr) {
            prev->SetNextRegion(next);
        }
        if (next != nullptr) {
            next->SetPrevRegion(prev);
        }
        this->SetNextRegion(nullptr);
        this->SetPrevRegion(nullptr);
    }

    static constexpr size_t GetAllocPtrOffset()
    {
        return MEMBER_OFFSET_CC(UnitMetadata, allocPtr);
    }

    static constexpr size_t GetRegionEndOffset()
    {
        return MEMBER_OFFSET_CC(UnitMetadata, regionEnd);
    }

private:
    void VisitAllObjectsBefore(const std::function<void(BaseObject*)>&& func, uintptr_t end);

    static constexpr int32_t MAX_RAW_POINTER_COUNT = std::numeric_limits<int32_t>::max();
    static constexpr int32_t BITS_4 = 4;
    static constexpr int32_t BITS_5 = 5;

    enum RegionBitOffset : uint8_t {
        BIT_OFFSET_REGION_TYPE = 0,
        // use mark-bitmap pointer instead
        BIT_OFFSET_MARKED_REGION = BITS_5,
        BIT_OFFSET_ENQUEUED_REGION = 6,
        BIT_OFFSET_RESURRECTED_REGION = 7,
        BIT_OFFSET_FIXED_REGION = 8,
        BIT_OFFSET_REGION_CELLCOUNT = 9,
        BIT_OFFSET_IS_JITFORT_AWAIT_INSTALL = 16,
    };

    struct ObjectSlot {
        HeapAddress next_ : 48;
        HeapAddress isFree_ : 1;
        HeapAddress padding : 15;

        void SetNext(ObjectSlot* slot, size_t size)
        {
            next_ = reinterpret_cast<HeapAddress>(slot);
            isFree_ = 1;
            size_t extraSize = size - sizeof(ObjectSlot);
            if (extraSize > 0) {
                uintptr_t start = reinterpret_cast<uintptr_t>(this) + sizeof(ObjectSlot);
                LOGE_IF((memset_s(reinterpret_cast<void*>(start), extraSize, 0, extraSize) != EOK)) << "memset_s fail";
            }
        }
    };

    class RegionLiveDesc {
    public:
        static constexpr HeapAddress TEMPORARY_PTR = 0x1234;

        void Init(RegionDesc *region)
        {
            relatedRegion_ = region;
            ClearLiveInfo();
        }

        void Fini()
        {
            relatedRegion_ = nullptr;
            ClearLiveInfo();
        }

        void ClearLiveInfo()
        {
            markBitmap_ = nullptr;
            resurrectBitmap_ = nullptr;
            enqueueBitmap_ = nullptr;
        }
    private:
        RegionDesc *relatedRegion_ {nullptr};
        RegionBitmap *markBitmap_ {nullptr};
        RegionBitmap *resurrectBitmap_ {nullptr};
        RegionBitmap *enqueueBitmap_ {nullptr};

        friend class RegionDesc;
    };

    struct UnitMetadata {
        struct { // basic data for RegionDesc
            // for fast allocation, always at the start.
            uintptr_t allocPtr;
            uintptr_t regionEnd;

            // watermark set when gc phase transitions to pre-marking.
            uintptr_t markingLine;
            uintptr_t forwardLine;
            ObjectSlot* freeSlot;
            // `regionStart` is the header of the data, and `regionBase` is the header of the total region
            /**
             * | *********************************Region*******************************|
             * | InlinedRegionMetaData | *****************Region data******************|
             *   ^                       ^
             *   |                       |
             * regionBase            regionStart
            */
            uintptr_t regionStart;
            uintptr_t regionBase;

            uint32_t nextRegionIdx;
            uint32_t prevRegionIdx; // support fast deletion for region list.

            uint32_t liveByteCount;
            int32_t rawPointerObjectCount;
        };

        RegionLiveDesc liveInfo_ {};

        RegionRSet* regionRSet = nullptr;;

        // the writing operation in C++ Bit-Field feature is not atomic, the we wants to
        // change the value, we must use specific interface implenmented by BitFields.
        union {
            struct {
                uint8_t unitRole : BITS_5;
            };
            BitFields<uint8_t> unitBits;
        };

        // the writing operation in C++ Bit-Field feature is not atomic, the we wants to
        // change the value, we must use specific interface implenmented by BitFields.
        union {
            struct {
                RegionType regionType : BITS_5;

                // true if this unit belongs to a ghost region, which is an unreal region for keeping reclaimed
                // from-region. ghost region is set up to memorize a from-region before from-space is forwarded. this
                // flag is cleared when ghost-from-space is cleared. Note this flag is essentially important for
                // FindToVersion().
                uint8_t isMarked : 1;
                uint8_t isEnqueued : 1;
                uint8_t isResurrected : 1;
                uint8_t isFixed : 1;
                uint8_t cellCount : 7;
                // Only valid in huge region. To mark the JitFort code await for install.
                // An awaiting JitFort does not hold valid data on and no parent reference, but considered as alive.
                uint8_t isJitFortAwaitInstall : 1;
            };
            BitFields<uint32_t> regionBits;
        };

        std::mutex regionMutex;
    };

    class UnitInfo {
    public:
        // propgated from RegionManager
        static uintptr_t heapStartAddress; // the address of the first region space to allocate objects
        static size_t totalUnitCount;
        static uintptr_t unitInfoStart; // the address of the first UnitInfo

        constexpr static uint32_t INVALID_IDX = std::numeric_limits<uint32_t>::max();
        static size_t GetUnitIdxAt(uintptr_t allocAddr)
        {
            if (heapStartAddress <= allocAddr && allocAddr < (heapStartAddress + totalUnitCount * UNIT_SIZE)) {
                return (allocAddr - heapStartAddress) / UNIT_SIZE;
            }

            LOG_COMMON(FATAL) << "Unresolved fatal";
            UNREACHABLE_CC();
        }

        static UnitInfo* GetUnitInfoAt(uintptr_t allocAddr)
        {
            return GetUnitInfo(GetUnitIdxAt(allocAddr));
        }

        // the start address for allocation
        static HeapAddress GetUnitAddress(size_t idx)
        {
            CHECK_CC(idx < totalUnitCount);
            return heapStartAddress + idx * UNIT_SIZE;
        }

        static UnitInfo* GetUnitInfo(size_t idx)
        {
            CHECK_CC(idx < totalUnitCount);
            return reinterpret_cast<UnitInfo*>(heapStartAddress - (idx + 1) * sizeof(UnitInfo));
        }

        static size_t GetUnitIdx(const UnitInfo* unit)
        {
            uintptr_t ptr = reinterpret_cast<uintptr_t>(unit);
            DCHECK_CC(unitInfoStart <= ptr && ptr < heapStartAddress);
            return ((heapStartAddress - ptr) / sizeof(UnitInfo)) - 1;
        }

        UnitInfo() = delete;
        UnitInfo(const UnitInfo&) = delete;
        UnitInfo& operator=(const UnitInfo&) = delete;
        ~UnitInfo() = delete;

        // These interfaces are used to make sure the writing operations of value in C++ Bit Field will be atomic.
        void SetUnitRole(UnitRole role) { metadata_.unitBits.AtomicSetValue(0, BITS_5, static_cast<uint8_t>(role)); }

        void SetMarkedRegionFlag(uint8_t flag)
        {
            metadata_.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_MARKED_REGION, 1, flag);
        }

        void SetEnqueuedRegionFlag(uint8_t flag)
        {
            metadata_.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_ENQUEUED_REGION, 1, flag);
        }

        void SetResurrectedRegionFlag(uint8_t flag)
        {
            metadata_.regionBits.AtomicSetValue(RegionBitOffset::BIT_OFFSET_RESURRECTED_REGION, 1, flag);
        }

        void ToFreeRegion() { InitFreeRegion(GetUnitIdx(this), 1); }

        void ClearUnit() { ClearUnits(GetUnitIdx(this), 1); }

        void ReleaseUnit() { ReleaseUnits(GetUnitIdx(this), 1); }

        UnitMetadata& GetMetadata() { return metadata_; }

        UnitRole GetUnitRole() const { return static_cast<UnitRole>(metadata_.unitRole); }

    private:
        UnitMetadata metadata_;

        friend class RegionDesc;
    };

public:
    // inline copy some data at the begin of the region data, to support fast-path in barrier or smth else.
    // NOTE the data consistency between data in header and that in `RegionDesc`.
    // this could ONLY used in region that is ALIVE.
    class InlinedRegionMetaData {
    public:
        static InlinedRegionMetaData *GetInlinedRegionMetaData(RegionDesc *region)
        {
            InlinedRegionMetaData *data = GetInlinedRegionMetaData(region->GetRegionStart());
            DCHECK_CC(data->regionDesc_ == region);
            return data;
        }
        static InlinedRegionMetaData *GetInlinedRegionMetaData(uintptr_t allocAddr)
        {
            return reinterpret_cast<InlinedRegionMetaData *>(allocAddr & ~DEFAULT_REGION_UNIT_MASK);
        }

        explicit InlinedRegionMetaData(RegionDesc *regionDesc)
            : regionDesc_(regionDesc), regionRSet_(regionDesc->GetRSet()), regionType_(regionDesc->GetRegionType())
        {
            // Since this is a backup copy of `RegionDesc`, create rset at first to guarantee data consistency
            DCHECK_CC(regionRSet_ != nullptr);
            // Not insert to regionList and reset regionType yet
            DCHECK_CC(regionType_ == RegionType::FREE_REGION);
            DCHECK_CC(regionType_ == regionDesc_->GetRegionType());
        }
        ~InlinedRegionMetaData() = default;

        void SetRegionType(RegionType type)
        {
            DCHECK_CC(RegionDesc::IsAliveRegionType(type));
            DCHECK_CC(type == regionDesc_->GetRegionType());
            regionType_ = type;
        }

        RegionDesc *GetRegionDesc() const
        {
            return regionDesc_;
        }

        RegionRSet *GetRegionRSet() const
        {
            return regionRSet_;
        }

        RegionType GetRegionType() const
        {
            DCHECK_CC(RegionDesc::IsAliveRegionType(regionType_));
            return regionType_;
        }
        
        bool IsInRecentSpace() const
        {
            RegionType type = GetRegionType();
            return RegionDesc::IsInRecentSpace(type);
        }

        bool IsInYoungSpace() const
        {
            RegionType type = GetRegionType();
            return RegionDesc::IsInYoungSpace(type);
        }

        bool IsInFromSpace() const
        {
            RegionType type = GetRegionType();
            return RegionDesc::IsInFromSpace(type);
        }

        bool IsInToSpace() const
        {
            RegionType type = GetRegionType();
            return RegionDesc::IsInToSpace(type);
        }

        bool IsInOldSpace() const
        {
            RegionType type = GetRegionType();
            return RegionDesc::IsInOldSpace(type);
        }

        bool IsFromRegion() const
        {
            RegionType type = GetRegionType();
            return type == RegionType::FROM_REGION;
        }

        bool IsInYoungSpaceForWB() const
        {
            RegionType type = GetRegionType();
            return RegionDesc::IsInYoungSpaceForWB(type);
        }

        inline HeapAddress GetRegionStart() const;

        HeapAddress GetRegionBase() const
        {
            uintptr_t base = reinterpret_cast<uintptr_t>(this);
            ASSERT_LOGF(base == regionDesc_->GetRegionBaseFast(), "");
            return static_cast<HeapAddress>(base);
        }

        size_t GetAddressOffset(HeapAddress address) const
        {
            DCHECK_CC(GetRegionBase() <= address);
            return (address - GetRegionBase());
        }

        bool MarkRSetCardTable(BaseObject *obj)
        {
            size_t offset = GetAddressOffset(static_cast<HeapAddress>(reinterpret_cast<uintptr_t>(obj)));
            return GetRegionRSet()->MarkCardTable(offset);
        }
    private:
        RegionDesc *regionDesc_ {nullptr};
        RegionRSet *regionRSet_ {nullptr};
        RegionType regionType_ {};
        // fixme: inline more

        friend class RegionDesc;
    };
    // should keep as same as the align of BaseObject
    static constexpr size_t UNIT_BEGIN_ALIGN = 8;
    // default common region unit header size.
    static constexpr size_t UNIT_HEADER_SIZE = AlignUp<size_t>(sizeof(InlinedRegionMetaData), UNIT_BEGIN_ALIGN);
    // default common region unit available size.
    static constexpr size_t UNIT_AVAILABLE_SIZE = UNIT_SIZE - UNIT_HEADER_SIZE;

private:

    void InitRegionDesc(size_t nUnit, UnitRole uClass)
    {
        DCHECK_CC(uClass != UnitRole::SUBORDINATE_UNIT);
        size_t base = GetRegionBase();
        metadata.regionBase = base;
        metadata.regionStart = base + RegionDesc::UNIT_HEADER_SIZE;
        ASSERT_LOGF(metadata.regionStart % UNIT_BEGIN_ALIGN == 0, "");
        metadata.allocPtr = GetRegionStart();
        metadata.regionEnd = base + nUnit * RegionDesc::UNIT_SIZE;
        DCHECK_CC(GetRegionStart() < GetRegionEnd());
        metadata.prevRegionIdx = NULLPTR_IDX;
        metadata.nextRegionIdx = NULLPTR_IDX;
        metadata.liveByteCount = 0;
        metadata.freeSlot = nullptr;
        SetRegionType(RegionType::FREE_REGION);
        SetUnitRole(uClass);
        ClearMarkingCopyLine();
        SetMarkedRegionFlag(0);
        SetEnqueuedRegionFlag(0);
        SetResurrectedRegionFlag(0);
        __atomic_store_n(&metadata.rawPointerObjectCount, 0, __ATOMIC_SEQ_CST);
#ifdef USE_HWASAN
        ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<const volatile void *>(metadata.regionBase),
            nUnit * RegionDesc::UNIT_SIZE);
        uintptr_t pAddr = metadata.regionBase;
        uintptr_t pSize = nUnit * RegionDesc::UNIT_SIZE;
        LOG_COMMON(DEBUG) << std::hex << "set [" << pAddr <<
            std::hex << ", " << (pAddr + pSize) << ") unpoisoned\n";
#endif
    }

    void ResetRegion(size_t nUnit, UnitRole uClass)
    {
        DCHECK_CC(metadata.regionRSet != nullptr);
        ClearRSet();
        InitRegionDesc(nUnit, uClass);
        InitMetaData(nUnit, uClass);
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }

    void InitRegion(size_t nUnit, UnitRole uClass)
    {
        DCHECK_CC(uClass != UnitRole::FREE_UNITS);   //fixme: remove `UnitRole::SUBORDINATE_UNIT`
        DCHECK_CC(uClass != UnitRole::SUBORDINATE_UNIT);   //fixme: remove `UnitRole::SUBORDINATE_UNIT`
        InitRegionDesc(nUnit, uClass);
        DCHECK_CC(metadata.regionRSet == nullptr);
        metadata.regionRSet = RegionRSet::CreateRegionRSet(GetRegionBaseSize());
        InitMetaData(nUnit, uClass);
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }

    void InitMetaData(size_t nUnit, UnitRole uClass)
    {
        metadata.liveInfo_.Init(this);
        HeapAddress header = GetRegionBase();
        void *ptr = reinterpret_cast<void *>(static_cast<uintptr_t>(header));
        new (ptr) InlinedRegionMetaData(this);

        // initialize region's subordinate units.
        UnitInfo* unit = reinterpret_cast<UnitInfo*>(this) - (nUnit - 1);
        for (size_t i = 0; i < nUnit - 1; i++) {
            DCHECK_CC(uClass == UnitRole::LARGE_SIZED_UNITS);
            unit[i].metadata_.liveInfo_.Fini();
        }
    }

    static constexpr uint32_t NULLPTR_IDX = UnitInfo::INVALID_IDX;
    UnitMetadata metadata;
public:
    friend constexpr size_t GetMetaDataInRegionOffset();
    static constexpr size_t REGION_RSET_IN_INLINED_METADATA_OFFSET
        = MEMBER_OFFSET_CC(InlinedRegionMetaData, regionRSet_);
    static constexpr size_t REGION_TYPE_IN_INLINED_METADATA_OFFSET
        = MEMBER_OFFSET_CC(InlinedRegionMetaData, regionType_);
};

HeapAddress RegionDesc::InlinedRegionMetaData::GetRegionStart() const
{
    HeapAddress addr = static_cast<HeapAddress>(reinterpret_cast<uintptr_t>(this) + RegionDesc::UNIT_HEADER_SIZE);
    DCHECK_CC(addr == regionDesc_->GetRegionStart());
    return addr;
}
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_INFO_H

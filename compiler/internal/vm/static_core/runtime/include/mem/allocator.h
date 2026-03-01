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
#ifndef RUNTIME_MEM_ALLOCATOR_H
#define RUNTIME_MEM_ALLOCATOR_H

#include <functional>

#include "libarkbase/mem/code_allocator.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/pool_map.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/macros.h"
#include "runtime/mem/bump-allocator.h"
#include "runtime/mem/freelist_allocator.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/gc/gc_types.h"
#include "runtime/mem/humongous_obj_allocator.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/runslots_allocator.h"
#include "runtime/mem/pygote_space_allocator.h"
#include "runtime/mem/heap_space.h"

namespace ark {
class ObjectHeader;
}  // namespace ark

namespace ark {
class ManagedThread;
}  // namespace ark

namespace ark {
class BaseClass;
}  // namespace ark

namespace ark::mem {

class ObjectAllocConfigWithCrossingMap;
class ObjectAllocConfig;
class TLAB;

/// AllocatorPurpose and GCCollectMode provide info when we should collect from some allocator or not
enum class AllocatorPurpose {
    ALLOCATOR_PURPOSE_OBJECT,    // Allocator for objects
    ALLOCATOR_PURPOSE_INTERNAL,  // Space for runtime internal needs
};

template <AllocatorType>
class AllocatorTraits {
};

template <>
class AllocatorTraits<AllocatorType::RUNSLOTS_ALLOCATOR> {
    using AllocType = RunSlotsAllocator<ObjectAllocConfig>;
    static constexpr bool HAS_FREE {true};  // indicates allocator can free
};

template <typename T, AllocScope ALLOC_SCOPE_T>
class AllocatorAdapter;

class Allocator {
public:
    template <typename T, AllocScope ALLOC_SCOPE_T = AllocScope::GLOBAL>
    using AdapterType = AllocatorAdapter<T, ALLOC_SCOPE_T>;

    NO_COPY_SEMANTIC(Allocator);
    NO_MOVE_SEMANTIC(Allocator);
    explicit Allocator(MemStatsType *memStats, AllocatorPurpose purpose, GCCollectMode gcCollectMode)
        : memStats_(memStats), allocatorPurpose_(purpose), gcCollectMode_(gcCollectMode)
    {
    }
    virtual ~Allocator() = 0;

    ALWAYS_INLINE AllocatorPurpose GetPurpose() const
    {
        return allocatorPurpose_;
    }

    ALWAYS_INLINE GCCollectMode GetCollectMode() const
    {
        return gcCollectMode_;
    }

    ALWAYS_INLINE MemStatsType *GetMemStats() const
    {
        return memStats_;
    }

    [[nodiscard]] void *Alloc(size_t size)
    {
        return Allocate(size, CalculateAllocatorAlignment(alignof(uintptr_t)), nullptr);
    }

    [[nodiscard]] void *Alloc(size_t size, Alignment align)
    {
        return Allocate(size, align, nullptr);
    }

    [[nodiscard]] void *AllocLocal(size_t size)
    {
        return AllocateLocal(size, CalculateAllocatorAlignment(alignof(uintptr_t)), nullptr);
    }

    [[nodiscard]] void *AllocLocal(size_t size, Alignment align)
    {
        return AllocateLocal(size, align, nullptr);
    }

    [[nodiscard]] virtual void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread) = 0;

    [[nodiscard]] virtual void *AllocateLocal(size_t size, Alignment align,
                                              [[maybe_unused]] ark::ManagedThread *thread) = 0;

    template <class T>
    [[nodiscard]] T *AllocArray(size_t size)
    {
        return static_cast<T *>(this->Allocate(sizeof(T) * size, CalculateAllocatorAlignment(alignof(T)), nullptr));
    }

    template <class T>
    [[nodiscard]] T *AllocArrayLocal(size_t size)
    {
        return static_cast<T *>(
            this->AllocateLocal(sizeof(T) * size, CalculateAllocatorAlignment(alignof(T)), nullptr));
    }

    template <class T>
    void Delete(T *ptr)
    {
        if (ptr == nullptr) {
            return;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (std::is_class_v<T>) {
            ptr->~T();
        }
        Free(ptr);
    }

    template <typename T>
    void DeleteArray(T *data)
    {
        if (data == nullptr) {
            return;
        }
        static constexpr size_t SIZE_BEFORE_DATA_OFFSET =
            AlignUp(sizeof(size_t), GetAlignmentInBytes(GetAlignment<T>()));
        void *p = ToVoidPtr(ToUintPtr(data) - SIZE_BEFORE_DATA_OFFSET);
        size_t size = *static_cast<size_t *>(p);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (std::is_class_v<T>) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (size_t i = 0; i < size; ++i, ++data) {
                data->~T();
            }
        }
        Free(p);
    }

    virtual void Free(void *mem) = 0;

    virtual void VisitAndRemoveAllPools(const MemVisitor &memVisitor) = 0;

    virtual void VisitAndRemoveFreePools(const MemVisitor &memVisitor) = 0;

    virtual void IterateOverYoungObjects([[maybe_unused]] const ObjectVisitor &objectVisitor)
    {
        LOG(FATAL, ALLOC) << "Allocator::IterateOverYoungObjects" << std::endl;
    }

    virtual void IterateOverTenuredObjects([[maybe_unused]] const ObjectVisitor &objectVisitor)
    {
        LOG(FATAL, ALLOC) << "Allocator::IterateOverTenuredObjects" << std::endl;
    }

    /// @brief iterates all objects in object allocator
    virtual void IterateRegularSizeObjects([[maybe_unused]] const ObjectVisitor &objectVisitor)
    {
        LOG(FATAL, ALLOC) << "Allocator::IterateRegularSizeObjects";
    }

    /// @brief iterates objects in all allocators except object allocator
    virtual void IterateNonRegularSizeObjects([[maybe_unused]] const ObjectVisitor &objectVisitor)
    {
        LOG(FATAL, ALLOC) << "Allocator::IterateNonRegularSizeObjects";
    }

    virtual void FreeObjectsMovedToPygoteSpace()
    {
        LOG(FATAL, ALLOC) << "Allocator::FreeObjectsMovedToPygoteSpace";
    }

    virtual void IterateOverObjectsInRange(MemRange memRange, const ObjectVisitor &objectVisitor) = 0;

    virtual void IterateOverObjects(const ObjectVisitor &objectVisitor) = 0;

    virtual void IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor) = 0;

    template <AllocScope ALLOC_SCOPE_T = AllocScope::GLOBAL>
    AllocatorAdapter<void, ALLOC_SCOPE_T> Adapter();

    template <typename T, typename... Args>
    std::enable_if_t<!std::is_array_v<T>, T *> New(Args &&...args)
    {
        void *p = Alloc(sizeof(T), CalculateAllocatorAlignment(alignof(T)));
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        new (p) T(std::forward<Args>(args)...);  // NOLINT(bugprone-throw-keyword-missing)
        return reinterpret_cast<T *>(p);
    }

    template <typename T>
    std::enable_if_t<is_unbounded_array_v<T>, std::remove_extent_t<T> *> New(size_t size)
    {
        static constexpr size_t SIZE_BEFORE_DATA_OFFSET =
            AlignUp(sizeof(size_t), GetAlignmentInBytes(GetAlignment<T>()));
        using ElementType = std::remove_extent_t<T>;
        void *p = Alloc(SIZE_BEFORE_DATA_OFFSET + sizeof(ElementType) * size, CalculateAllocatorAlignment(alignof(T)));
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        *static_cast<size_t *>(p) = size;
        auto *data = ToNativePtr<ElementType>(ToUintPtr(p) + SIZE_BEFORE_DATA_OFFSET);
        ElementType *currentElement = data;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (size_t i = 0; i < size; ++i, ++currentElement) {
            new (currentElement) ElementType();
        }
        return data;
    }

    template <typename T, typename... Args>
    std::enable_if_t<!std::is_array_v<T>, T *> NewLocal(Args &&...args)
    {
        void *p = AllocLocal(sizeof(T), CalculateAllocatorAlignment(alignof(T)));
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        new (p) T(std::forward<Args>(args)...);  // NOLINT(bugprone-throw-keyword-missing)
        return reinterpret_cast<T *>(p);
    }

    template <typename T>
    std::enable_if_t<is_unbounded_array_v<T>, std::remove_extent_t<T> *> NewLocal(size_t size)
    {
        static constexpr size_t SIZE_BEFORE_DATA_OFFSET =
            AlignUp(sizeof(size_t), GetAlignmentInBytes(GetAlignment<T>()));
        using ElementType = std::remove_extent_t<T>;
        void *p =
            AllocLocal(SIZE_BEFORE_DATA_OFFSET + sizeof(ElementType) * size, CalculateAllocatorAlignment(alignof(T)));
        *static_cast<size_t *>(p) = size;
        auto *data = ToNativePtr<ElementType>(ToUintPtr(p) + SIZE_BEFORE_DATA_OFFSET);
        ElementType *currentElement = data;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (size_t i = 0; i < size; ++i, ++currentElement) {
            new (currentElement) ElementType();
        }
        return data;
    }

    virtual void *AllocateInLargeAllocator([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align,
                                           [[maybe_unused]] BaseClass *cls)
    {
        return nullptr;
    }

#if defined(TRACK_INTERNAL_ALLOCATIONS)
    virtual void Dump() {}
#endif

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MemStatsType *memStats_;

private:
    virtual Alignment CalculateAllocatorAlignment(size_t align) = 0;

    AllocatorPurpose allocatorPurpose_;
    GCCollectMode gcCollectMode_;
};

class ObjectAllocatorBase : public Allocator {
protected:
    using PygoteAllocator = PygoteSpaceAllocator<ObjectAllocConfig>;  // Allocator for pygote space

    /// @brief Add new memory pools to object_allocator and allocate memory in them
    template <typename AllocT, bool NEED_LOCK = true>
    inline void *AddPoolsAndAlloc(size_t size, Alignment align, AllocT *objectAllocator, size_t poolSize,
                                  SpaceType spaceType, HeapSpace *heapSpace);

    /**
     * Try to allocate memory for the object and if failed add new memory pools and allocate again
     * @param size - size of the object in bytes
     * @param align - alignment
     * @param object_allocator - allocator for the object
     * @param pool_size - size of a memory pool for specified allocator
     * @param space_type - SpaceType of the object
     * @return pointer to allocated memory or nullptr if failed
     */
    template <typename AllocT, bool NEED_LOCK = true>
    inline void *AllocateSafe(size_t size, Alignment align, AllocT *objectAllocator, size_t poolSize,
                              SpaceType spaceType, HeapSpace *heapSpace);

    /**
     * @brief Initialize an object memory allocated by an allocator.
     *        NOTE: object header should be zero
     * @param mem - pointer to allocated object
     * @param size - size of the object in bytes
     */
    void ObjectMemoryInit(void *mem, size_t size) const;

    /**
     * @brief Initialize memory which will be used for objects.
     * @param mem - pointer to allocated memory
     * @param size - size of the memory in bytes
     */
    void MemoryInitialize(void *mem, size_t size) const;

public:
    enum class ObjMemInitPolicy : bool { NO_INIT, REQUIRE_INIT };
    ObjectAllocatorBase() = delete;
    NO_COPY_SEMANTIC(ObjectAllocatorBase);
    NO_MOVE_SEMANTIC(ObjectAllocatorBase);

    explicit ObjectAllocatorBase(MemStatsType *memStats, GCCollectMode gcCollectMode, bool createPygoteSpaceAllocator);

    ~ObjectAllocatorBase() override;
    void *Allocate([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align,
                   [[maybe_unused]] ark::ManagedThread *thread) final
    {
        LOG(FATAL, ALLOC)
            << "Don't use common Allocate method for object allocation without object initialization argument";
        return nullptr;
    }

    [[nodiscard]] virtual void *Allocate(size_t size, Alignment align, ark::ManagedThread *thread,
                                         ObjMemInitPolicy objInit, bool pinned) = 0;
    [[nodiscard]] virtual void *AllocateNonMovable(size_t size, Alignment align, ark::ManagedThread *thread,
                                                   ObjMemInitPolicy objInit) = 0;

    /**
     * Iterate over all objects and reclaim memory for objects reported as true by gc_object_visitor
     * @param gc_object_visitor - function which return true for ObjectHeader if we can reclaim memory occupied by
     * object
     */
    virtual void Collect(const GCObjectVisitor &gcObjectVisitor, GCCollectMode collectMode) = 0;

    /**
     * Return max size for regular size objects
     * @return max size in bytes for regular size objects
     */
    virtual size_t GetRegularObjectMaxSize() = 0;

    /**
     * Return max size for large objects
     * @return max size in bytes for large objects
     */
    virtual size_t GetLargeObjectMaxSize() = 0;

    /**
     * Checks if object in the young space
     * @param object address
     * @return true if @param object is in young space
     */
    virtual bool IsObjectInYoungSpace(const ObjectHeader *obj) = 0;

    /**
     * Checks if @param mem_range intersect young space
     * @param mem_range
     * @return true if @param mem_range is intersect young space
     */
    virtual bool IsIntersectedWithYoung(const MemRange &memRange) = 0;

    /**
     * Checks if object in the non-movable space
     * @param obj
     * @return true if @param obj is in non-movable space
     */
    virtual bool IsObjectInNonMovableSpace(const ObjectHeader *obj) = 0;

    /**
     * Check GC will never move the object.
     * @param obj - object to check
     * @return true if GC will not move the object, else return false.
     */
    virtual bool IsNonMovable(const ObjectHeader *obj) = 0;

    /// @return true if allocator has an young space
    virtual bool HasYoungSpace() = 0;

    /**
     * Pin object address in heap space. Such object can not be moved to other space
     * @param object object for pinning
     */
    virtual void PinObject(ObjectHeader *object) = 0;

    /**
     * Unpin pinned object in heap space. Such object can be moved after this operation
     * @param object object for unpinning
     */
    virtual void UnpinObject(ObjectHeader *object) = 0;

    /**
     * Get young space memory ranges
     * \note PandaVector can't be used here
     * @return young space memory ranges
     */
    virtual const std::vector<MemRange> &GetYoungSpaceMemRanges() = 0;

    virtual std::vector<MarkBitmap *> &GetYoungSpaceBitmaps() = 0;

    virtual void ResetYoungAllocator() = 0;

    virtual TLAB *CreateNewTLAB(size_t tlabSize) = 0;

    virtual size_t GetTLABMaxAllocSize() = 0;

    virtual bool IsTLABSupported() = 0;

    /// @brief Check if the object allocator contains the object starting at address obj
    virtual bool ContainObject([[maybe_unused]] const ObjectHeader *obj) const = 0;

    /**
     * @brief Check if the object obj is live: obj is allocated already and
     * not collected yet.
     */
    virtual bool IsLive([[maybe_unused]] const ObjectHeader *obj) = 0;

    /// @brief Check if current allocators' allocation state is valid.
    virtual size_t VerifyAllocatorStatus() = 0;

    virtual HeapSpace *GetHeapSpace() = 0;

    PygoteAllocator *GetPygoteSpaceAllocator()
    {
        return pygoteSpaceAllocator_;
    }

    const PygoteAllocator *GetPygoteSpaceAllocator() const
    {
        return pygoteSpaceAllocator_;
    }

    void DisablePygoteAlloc()
    {
        pygoteAllocEnabled_ = false;
    }

    bool IsPygoteAllocEnabled() const
    {
        ASSERT(!pygoteAllocEnabled_ || pygoteSpaceAllocator_ != nullptr);
        return pygoteAllocEnabled_;
    }

    static size_t GetObjectSpaceFreeBytes()
    {
        return PoolManager::GetMmapMemPool()->GetObjectSpaceFreeBytes();
    }

    bool HaveEnoughPoolsInObjectSpace(size_t poolsNum) const;

    void IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor) override;

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    PygoteAllocator *pygoteSpaceAllocator_ = nullptr;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    bool pygoteAllocEnabled_ = false;

private:
    void Free([[maybe_unused]] void *mem) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorBase shouldn't have Free";
    }
};

/**
 * Template wrapper for single underlying allocator
 * @tparam AllocT
 */
template <typename AllocT, AllocatorPurpose ALLOCATOR_PURPOSE>
class AllocatorSingleT final : public Allocator {
public:
    // NOLINTNEXTLINE(readability-magic-numbers)
    explicit AllocatorSingleT(MemStatsType *memStats)
        : Allocator(memStats, ALLOCATOR_PURPOSE, GCCollectMode::GC_NONE), allocator_(memStats)
    {
    }
    ~AllocatorSingleT() final = default;
    NO_COPY_SEMANTIC(AllocatorSingleT);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(AllocatorSingleT);

    [[nodiscard]] void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread) final
    {
        return allocator_.Alloc(size, align);
    }

    [[nodiscard]] void *AllocateLocal(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread) final
    {
        return allocator_.AllocLocal(size, align);
    }

    void Free(void *mem) final
    {
        allocator_.Free(mem);
    }

    void VisitAndRemoveAllPools(const MemVisitor &memVisitor) final
    {
        allocator_.VisitAndRemoveAllPools(memVisitor);
    }

    void VisitAndRemoveFreePools(const MemVisitor &memVisitor) final
    {
        allocator_.VisitAndRemoveFreePools(memVisitor);
    }

    void IterateOverObjectsInRange([[maybe_unused]] MemRange memRange,
                                   [[maybe_unused]] const ObjectVisitor &objectVisitor) final
    {
        LOG(FATAL, ALLOC) << "IterateOverObjectsInRange not implemented for AllocatorSinglet";
    }

    void IterateOverObjects([[maybe_unused]] const ObjectVisitor &objectVisitor) final
    {
        LOG(FATAL, ALLOC) << "IterateOverObjects not implemented for AllocatorSinglet";
    }

    void IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor) final
    {
        LOG(FATAL, ALLOC) << "IterateOverObjectsSafe not implemented for AllocatorSinglet";
    }

#if defined(TRACK_INTERNAL_ALLOCATIONS)
    void Dump() override
    {
        allocator_.Dump();
    }
#endif

private:
    Alignment CalculateAllocatorAlignment(size_t align) final
    {
        if constexpr (ALLOCATOR_PURPOSE == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT) {
            return GetAlignment(align);
        }
        return GetInternalAlignment(align);
    }

    AllocT allocator_;
};

/**
 * Class is pointer wrapper. It checks if type of allocator matches expected.
 * @tparam allocatorType - type of allocator
 */
template <AllocatorPurpose ALLOCATOR_PURPOSE>
class AllocatorPtr {
public:
    AllocatorPtr() = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    AllocatorPtr(std::nullptr_t aNullptr) noexcept : allocatorPtr_(aNullptr) {}

    explicit AllocatorPtr(Allocator *allocator) : allocatorPtr_(allocator) {}

    Allocator *operator->()
    {
        ASSERT((allocatorPtr_ == nullptr) || (allocatorPtr_->GetPurpose() == ALLOCATOR_PURPOSE));
        return allocatorPtr_;
    }

    AllocatorPtr &operator=(std::nullptr_t aNullptr) noexcept
    {
        allocatorPtr_ = aNullptr;
        return *this;
    }

    AllocatorPtr &operator=(Allocator *allocator)
    {
        allocatorPtr_ = allocator;
        return *this;
    }

    explicit operator Allocator *()
    {
        return allocatorPtr_;
    }

    explicit operator ObjectAllocatorBase *()
    {
        ASSERT(allocatorPtr_->GetPurpose() == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
        return static_cast<ObjectAllocatorBase *>(allocatorPtr_);
    }

    ALWAYS_INLINE bool operator==(const AllocatorPtr &other)
    {
        return allocatorPtr_ == static_cast<Allocator *>(other);
    }

    ALWAYS_INLINE bool operator==(std::nullptr_t) noexcept
    {
        return allocatorPtr_ == nullptr;
    }

    ALWAYS_INLINE bool operator!=(std::nullptr_t) noexcept
    {
        return allocatorPtr_ != nullptr;
    }

    ObjectAllocatorBase *AsObjectAllocator()
    {
        ASSERT(ALLOCATOR_PURPOSE == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
        return this->operator ark::mem::ObjectAllocatorBase *();
    }

    ~AllocatorPtr() = default;

    DEFAULT_COPY_SEMANTIC(AllocatorPtr);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(AllocatorPtr);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Allocator *allocatorPtr_ = nullptr;
};

using InternalAllocatorPtr = AllocatorPtr<AllocatorPurpose::ALLOCATOR_PURPOSE_INTERNAL>;
using ObjectAllocatorPtr = AllocatorPtr<AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT>;

template <InternalAllocatorConfig CONFIG>
using InternalAllocatorT = AllocatorSingleT<InternalAllocator<CONFIG>, AllocatorPurpose::ALLOCATOR_PURPOSE_INTERNAL>;

template <MTModeT MT_MODE = MT_MODE_MULTI>
class ObjectAllocatorNoGen : public ObjectAllocatorBase {
    using ObjectAllocator = RunSlotsAllocator<ObjectAllocConfig>;       // Allocator used for middle size allocations
    using LargeObjectAllocator = FreeListAllocator<ObjectAllocConfig>;  // Allocator used for large objects
    using HumongousObjectAllocator = HumongousObjAllocator<ObjectAllocConfig>;  // Allocator used for humongous objects

public:
    NO_MOVE_SEMANTIC(ObjectAllocatorNoGen);
    NO_COPY_SEMANTIC(ObjectAllocatorNoGen);

    explicit ObjectAllocatorNoGen(MemStatsType *memStats, bool createPygoteSpaceAllocator);

    ~ObjectAllocatorNoGen() override;

    [[nodiscard]] void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                 ObjMemInitPolicy objInit, bool pinned) override;

    [[nodiscard]] void *AllocateNonMovable(size_t size, Alignment align, ark::ManagedThread *thread,
                                           ObjMemInitPolicy objInit) override;

    void VisitAndRemoveAllPools(const MemVisitor &memVisitor) final;

    void VisitAndRemoveFreePools(const MemVisitor &memVisitor) final;

    void IterateOverObjects(const ObjectVisitor &objectVisitor) final;

    /// @brief iterates all objects in object allocator
    void IterateRegularSizeObjects(const ObjectVisitor &objectVisitor) final;

    /// @brief iterates objects in all allocators except object allocator
    void IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor) final;

    void FreeObjectsMovedToPygoteSpace() final;

    void Collect(const GCObjectVisitor &gcObjectVisitor, GCCollectMode collectMode) final;

    size_t GetRegularObjectMaxSize() final;

    size_t GetLargeObjectMaxSize() final;

    bool IsObjectInYoungSpace([[maybe_unused]] const ObjectHeader *obj) final
    {
        return false;
    }

    bool IsIntersectedWithYoung([[maybe_unused]] const MemRange &memRange) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorNoGen: IsIntersectedWithYoung not applicable";
        return false;
    }

    bool IsObjectInNonMovableSpace([[maybe_unused]] const ObjectHeader *obj) final
    {
        return true;
    }

    bool IsNonMovable([[maybe_unused]] const ObjectHeader *obj) override
    {
        return true;
    }

    bool HasYoungSpace() final
    {
        return false;
    }

    void PinObject([[maybe_unused]] ObjectHeader *object) final {}

    void UnpinObject([[maybe_unused]] ObjectHeader *object) final {}

    const std::vector<MemRange> &GetYoungSpaceMemRanges() final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorNoGen: GetYoungSpaceMemRanges not applicable";
        UNREACHABLE();
    }

    std::vector<MarkBitmap *> &GetYoungSpaceBitmaps() final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorNoGen: GetYoungBitmaps not applicable";
        UNREACHABLE();
    }

    void ResetYoungAllocator() final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorNoGen: ResetYoungAllocator not applicable";
    }

    TLAB *CreateNewTLAB(size_t tlabSize) final;

    size_t GetTLABMaxAllocSize() final;

    bool IsTLABSupported() final
    {
        return false;
    }

    void IterateOverObjectsInRange([[maybe_unused]] MemRange memRange,
                                   [[maybe_unused]] const ObjectVisitor &objectVisitor) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorNoGen: IterateOverObjectsInRange not implemented";
    }

    bool ContainObject(const ObjectHeader *obj) const final;

    bool IsLive(const ObjectHeader *obj) final;

    size_t VerifyAllocatorStatus() final
    {
        size_t failCount = 0;
        failCount += objectAllocator_->VerifyAllocator();
        // NOTE(yyang): add verify for large/humongous allocator
        return failCount;
    }

    [[nodiscard]] void *AllocateLocal(size_t /* size */, Alignment /* align */, ark::ManagedThread * /* thread */) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorNoGen: AllocateLocal not supported";
        return nullptr;
    }

    HeapSpace *GetHeapSpace() override
    {
        return &heapSpace_;
    }

private:
    Alignment CalculateAllocatorAlignment(size_t align) final;

    ObjectAllocator *objectAllocator_ = nullptr;
    LargeObjectAllocator *largeObjectAllocator_ = nullptr;
    HumongousObjectAllocator *humongousObjectAllocator_ = nullptr;
    HeapSpace heapSpace_;
};

// Base class for all generational GCs
class ObjectAllocatorGenBase : public ObjectAllocatorBase {
public:
    explicit ObjectAllocatorGenBase(MemStatsType *memStats, GCCollectMode gcCollectMode,
                                    bool createPygoteSpaceAllocator);

    GenerationalSpaces *GetHeapSpace() override
    {
        return &heapSpaces_;
    }

    ~ObjectAllocatorGenBase() override = default;

    virtual void *AllocateTenured(size_t size) = 0;
    virtual void *AllocateTenuredWithoutLocks(size_t size) = 0;

    NO_COPY_SEMANTIC(ObjectAllocatorGenBase);
    NO_MOVE_SEMANTIC(ObjectAllocatorGenBase);

    /// Updates young space mem ranges, bitmaps etc
    virtual void UpdateSpaceData() = 0;

    /// Invalidates space mem ranges, bitmaps etc
    virtual void InvalidateSpaceData() final;

protected:
    ALWAYS_INLINE std::vector<MemRange> &GetYoungRanges()
    {
        return ranges_;
    }

    ALWAYS_INLINE std::vector<MarkBitmap *> &GetYoungBitmaps()
    {
        return youngBitmaps_;
    }

    GenerationalSpaces heapSpaces_;  // NOLINT(misc-non-private-member-variables-in-classes)
private:
    std::vector<MemRange> ranges_;            // Ranges for young space
    std::vector<MarkBitmap *> youngBitmaps_;  // Bitmaps for young regions
};

template <MTModeT MT_MODE = MT_MODE_MULTI>
class ObjectAllocatorGen final : public ObjectAllocatorGenBase {
    using YoungGenAllocator = BumpPointerAllocator<ObjectAllocConfigWithCrossingMap,
                                                   BumpPointerAllocatorLockConfig::ParameterizedLock<MT_MODE>, true>;
    using ObjectAllocator =
        RunSlotsAllocator<ObjectAllocConfigWithCrossingMap>;  // Allocator used for middle size allocations
    using LargeObjectAllocator =
        FreeListAllocator<ObjectAllocConfigWithCrossingMap>;  // Allocator used for large objects
    using HumongousObjectAllocator =
        HumongousObjAllocator<ObjectAllocConfigWithCrossingMap>;  // Allocator used for humongous objects

public:
    NO_MOVE_SEMANTIC(ObjectAllocatorGen);
    NO_COPY_SEMANTIC(ObjectAllocatorGen);

    explicit ObjectAllocatorGen(MemStatsType *memStats, bool createPygoteSpaceAllocator);

    ~ObjectAllocatorGen() final;

    [[nodiscard]] void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                 ObjMemInitPolicy objInit, bool pinned) final;

    [[nodiscard]] void *AllocateNonMovable(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                           ObjMemInitPolicy objInit) final;

    void *AllocateTenured(size_t size) final
    {
        return AllocateTenuredImpl<true>(size);
    }

    void *AllocateTenuredWithoutLocks(size_t size) final
    {
        return AllocateTenuredImpl<false>(size);
    }

    void VisitAndRemoveAllPools(const MemVisitor &memVisitor) final;

    void VisitAndRemoveFreePools(const MemVisitor &memVisitor) final;

    void IterateOverYoungObjects(const ObjectVisitor &objectVisitor) final;

    void IterateOverTenuredObjects(const ObjectVisitor &objectVisitor) final;

    void IterateOverObjects(const ObjectVisitor &objectVisitor) final;

    /// @brief iterates all objects in object allocator
    void IterateRegularSizeObjects(const ObjectVisitor &objectVisitor) final;

    /// @brief iterates objects in all allocators except object allocator
    void IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor) final;

    void FreeObjectsMovedToPygoteSpace() final;

    void Collect(const GCObjectVisitor &gcObjectVisitor, GCCollectMode collectMode) final;

    size_t GetRegularObjectMaxSize() final;

    size_t GetLargeObjectMaxSize() final;

    bool IsObjectInYoungSpace(const ObjectHeader *obj) final;

    void PinObject([[maybe_unused]] ObjectHeader *object) final
    {
        ASSERT(!IsObjectInYoungSpace(object));
    }

    void UnpinObject([[maybe_unused]] ObjectHeader *object) final
    {
        ASSERT(!IsObjectInYoungSpace(object));
    }

    bool IsIntersectedWithYoung(const MemRange &memRange) final;

    bool IsObjectInNonMovableSpace(const ObjectHeader *obj) final;
    bool IsNonMovable(const ObjectHeader *obj) final;

    bool HasYoungSpace() final;

    const std::vector<MemRange> &GetYoungSpaceMemRanges() final;

    std::vector<MarkBitmap *> &GetYoungSpaceBitmaps() final;

    void ResetYoungAllocator() final;

    TLAB *CreateNewTLAB(size_t tlabSize) final;

    /**
     * @brief This method should be used carefully, since in case of adaptive TLAB
     * it only shows max possible size (grow limit) of a TLAB
     */
    size_t GetTLABMaxAllocSize() final;

    bool IsTLABSupported() final
    {
        return true;
    }

    void IterateOverObjectsInRange(MemRange memRange, const ObjectVisitor &objectVisitor) final;

    bool ContainObject(const ObjectHeader *obj) const final;

    bool IsLive(const ObjectHeader *obj) final;

    size_t VerifyAllocatorStatus() final
    {
        size_t failCount = 0;
        failCount += objectAllocator_->VerifyAllocator();
        // NOTE(yyang): add verify for large/humongous allocator
        return failCount;
    }

    [[nodiscard]] void *AllocateLocal(size_t /* size */, Alignment /* align */, ark::ManagedThread * /* thread */) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorGen: AllocateLocal not supported";
        return nullptr;
    }

    static size_t GetYoungAllocMaxSize();

    void UpdateSpaceData() final;

private:
    Alignment CalculateAllocatorAlignment(size_t align) final;

    YoungGenAllocator *youngGenAllocator_ = nullptr;
    ObjectAllocator *objectAllocator_ = nullptr;
    LargeObjectAllocator *largeObjectAllocator_ = nullptr;
    HumongousObjectAllocator *humongousObjectAllocator_ = nullptr;
    MemStatsType *memStats_ = nullptr;
    ObjectAllocator *nonMovableObjectAllocator_ = nullptr;
    LargeObjectAllocator *largeNonMovableObjectAllocator_ = nullptr;

    template <bool NEED_LOCK = true>
    void *AllocateTenuredImpl(size_t size);
};

template <GCType GC_TYPE, MTModeT MT_MODE = MT_MODE_MULTI>
class AllocConfig {
};

}  // namespace ark::mem

#endif  // RUNTIME_MEM_ALLOCATOR_H

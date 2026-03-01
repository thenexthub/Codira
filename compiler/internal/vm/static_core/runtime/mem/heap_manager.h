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
#ifndef PANDA_MEM_HEAP_MANAGER_H
#define PANDA_MEM_HEAP_MANAGER_H

#include <cstddef>
#include <memory>

#include "libarkbase/utils/logger.h"
#include "libarkbase/macros.h"
#include "runtime/include/class.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/object_header.h"
#include "runtime/include/thread.h"
#include "runtime/mem/frame_allocator-inl.h"
#include "runtime/mem/gc/heap-space-misc/crossing_map_singleton.h"
#include "runtime/mem/heap_verifier.h"
#include "runtime/mem/lock_config_helper.h"
#include "runtime/mem/tlab.h"

namespace ark {
// Forward declaration
class Runtime;
class PandaVM;
class RuntimeNotificationManager;
}  //  namespace ark

namespace ark::mem {

// Forward declaration
class MemoryManager;

class HeapManager {
public:
    bool Initialize(GCType gcType, MTModeT multithreadingMode, bool useTlab, MemStatsType *memStats,
                    InternalAllocatorPtr internalAllocator, bool createPygoteSpace);

    bool Finalize();

    [[nodiscard]] PANDA_PUBLIC_API ObjectHeader *AllocateObject(
        BaseClass *cls, size_t size, Alignment align = DEFAULT_ALIGNMENT, ManagedThread *thread = nullptr,
        ObjectAllocatorBase::ObjMemInitPolicy objInitType = ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT,
        bool pinned = false);

    template <bool IS_FIRST_CLASS_CLASS = false>
    [[nodiscard]] ObjectHeader *AllocateNonMovableObject(
        BaseClass *cls, size_t size, Alignment align = DEFAULT_ALIGNMENT, ManagedThread *thread = nullptr,
        ObjectAllocatorBase::ObjMemInitPolicy objInitType = ObjectAllocatorBase::ObjMemInitPolicy::REQUIRE_INIT);

    /**
     * @brief Allocates memory for ExtFrame, but do not construct it
     * @param size - size of allocation (ExtFrame) in bytes
     * @param ext_sz - size of frame extension in bytes
     * @return pointer to Frame
     */
    [[nodiscard]] PANDA_PUBLIC_API Frame *AllocateExtFrame(size_t size, size_t extSz);

    /**
     * @brief Frees memory occupied by ExtFrame
     * @param frame - pointer to Frame
     * @param ext_sz - size of frame extension in bytes
     */
    void PANDA_PUBLIC_API FreeExtFrame(Frame *frame, size_t extSz);

    CodeAllocator *GetCodeAllocator() const;

    PANDA_PUBLIC_API InternalAllocatorPtr GetInternalAllocator();

    bool UseTLABForAllocations()
    {
        return useTlabForAllocations_;
    }

    bool CreateNewTLAB(ManagedThread *thread);

    size_t GetTLABMaxAllocSize()
    {
        return UseTLABForAllocations() ? objectAllocator_.AsObjectAllocator()->GetTLABMaxAllocSize() : 0;
    }

    /**
     * Register TLAB information in MemStats during changing TLAB in a thread
     * or during thread destroying.
     */
    void RegisterTLAB(const TLAB *tlab);

    /**
     * Prepare the heap before the fork process, The main function is to compact zygote space for fork subprocess
     *
     * @param
     * @return void
     */
    void PreZygoteFork();

    /**
     *  NOTE :  Not yet implemented
     *  To implement the getTargetHeapUtilization and nativeSetTargetHeapUtilization,
     *  I set two functions and a fixed initial value here. They may need to be rewritten
     */
    float GetTargetHeapUtilization() const;

    void SetTargetHeapUtilization(float target);

    size_t VerifyHeapReferences()
    {
        trace::ScopedTrace scopedTrace(__FUNCTION__);
        size_t failCount = 0;
        HeapObjectVerifier verifier(this, &failCount);
        objectAllocator_->IterateOverObjects(verifier);
        return verifier.GetFailCount();
    }

    // Returns the maximum amount of memory a program can consume.
    size_t GetMaxMemory() const
    {
        return MemConfig::GetHeapSizeLimit();
    }

    // Returns approximate amount of memory currently consumed by an application.
    PANDA_PUBLIC_API size_t GetTotalMemory() const;

    // Returns how much free memory we have until we need to grow the heap to perform an allocation.
    PANDA_PUBLIC_API size_t GetFreeMemory() const;

    /// Clamp current accessable heap size as maximum heap size
    void ClampNewMaxHeapSize();

    // added for VMDebug::countInstancesOfClass and countInstancesOfClasses
    void CountInstances(const PandaVector<Class *> &classes, bool assignable, uint64_t *counts);
    uint64_t CountInstancesOfClass(Class *klass);

    using IsObjectFinalizebleFunc = bool (*)(BaseClass *);
    using RegisterFinalizeReferenceFunc = void (*)(ObjectHeader *, BaseClass *);
    void SetIsFinalizableFunc(IsObjectFinalizebleFunc func);
    void SetRegisterFinalizeReferenceFunc(RegisterFinalizeReferenceFunc func);

    bool IsObjectFinalized(BaseClass *cls);
    void RegisterFinalizedObject(ObjectHeader *object, BaseClass *cls, bool isObjectFinalizable);

    void SetPandaVM(PandaVM *vm);

    PandaVM *GetPandaVM() const
    {
        return vm_;
    }

    mem::GC *GetGC() const
    {
        return gc_;
    }

    RuntimeNotificationManager *GetNotificationManager() const
    {
        return notificationManager_;
    }

    MemStatsType *GetMemStats() const
    {
        return memStats_;
    }

    ALWAYS_INLINE void IterateOverObjects(const ObjectVisitor &objectVisitor)
    {
        GetObjectAllocator()->IterateOverObjects(objectVisitor);
    }

    void IterateOverObjectsWithSuspendAllThread(const ObjectVisitor &objectVisitor);

    ALWAYS_INLINE void PinObject(ObjectHeader *object)
    {
        GetObjectAllocator().AsObjectAllocator()->PinObject(object);
    }

    ALWAYS_INLINE void UnpinObject(ObjectHeader *object)
    {
        GetObjectAllocator().AsObjectAllocator()->UnpinObject(object);
    }

    ALWAYS_INLINE bool IsObjectInYoungSpace(const ObjectHeader *obj)
    {
        return GetObjectAllocator().AsObjectAllocator()->IsObjectInYoungSpace(obj);
    }

    ALWAYS_INLINE bool IsObjectInNonMovableSpace(const ObjectHeader *obj)
    {
        return GetObjectAllocator().AsObjectAllocator()->IsObjectInNonMovableSpace(obj);
    }

    /**
     * Check GC will never move the object.
     * Object may be in non-movable space or it may be a homoungous object in case of G1.
     */
    ALWAYS_INLINE bool IsNonMovable(const ObjectHeader *obj)
    {
        return GetObjectAllocator().AsObjectAllocator()->IsNonMovable(obj);
    }

    ALWAYS_INLINE bool IsLiveObject(const ObjectHeader *obj)
    {
        return GetObjectAllocator().AsObjectAllocator()->IsLive(obj);
    }

    ALWAYS_INLINE bool ContainObject(const ObjectHeader *obj)
    {
        return GetObjectAllocator().AsObjectAllocator()->ContainObject(obj);
    }

    ObjectHeader *RegisterFinalizableIfNeeded(ObjectHeader *obj, BaseClass *cls, size_t size, ManagedThread *thread);

    /**
     * Initialize GC bits and also zeroing memory for the whole Object memory
     * @param cls - class
     * @param mem - pointer to the ObjectHeader
     * @return pointer to the ObjectHeader
     */
    ObjectHeader *InitObjectHeaderAtMem(BaseClass *cls, void *mem);

    HeapManager() : targetUtilization_(DEFAULT_TARGET_UTILIZATION) {}

    ~HeapManager() = default;

    NO_COPY_SEMANTIC(HeapManager);
    NO_MOVE_SEMANTIC(HeapManager);

private:
    template <GCType GC_TYPE, MTModeT MT_MODE = MT_MODE_MULTI>
    bool Initialize(MemStatsType *memStats, bool createPygoteSpace)
    {
        ASSERT(!isInitialized_);
        isInitialized_ = true;

        codeAllocator_ = new (std::nothrow) CodeAllocator(memStats);
        // For now, crossing map is shared by diffrent VMs.
        if (!CrossingMapSingleton::IsCreated()) {
            CrossingMapSingleton::Create();
        }
        objectAllocator_ =
            new (std::nothrow) typename AllocConfig<GC_TYPE, MT_MODE>::ObjectAllocatorType(memStats, createPygoteSpace);
        return (codeAllocator_ != nullptr) && (internalAllocator_ != nullptr) && (objectAllocator_ != nullptr);
    }

    template <GCType GC_TYPE>
    bool Initialize(MemStatsType *memStats, MTModeT multithreadingMode, bool createPygoteSpace)
    {
        switch (multithreadingMode) {
            case MT_MODE_SINGLE: {
                return Initialize<GC_TYPE, MT_MODE_SINGLE>(memStats, createPygoteSpace);
            }
            case MT_MODE_MULTI: {
                return Initialize<GC_TYPE, MT_MODE_MULTI>(memStats, createPygoteSpace);
            }
            case MT_MODE_TASK: {
                return Initialize<GC_TYPE, MT_MODE_TASK>(memStats, createPygoteSpace);
            }
            default: {
                UNREACHABLE();
            }
        }
    }

    /// Triggers GC if needed
    void TriggerGCIfNeeded();

    void *TryGCAndAlloc(size_t size, Alignment align, ManagedThread *thread,
                        ObjectAllocatorBase::ObjMemInitPolicy objInitType, bool pinned = false);

    void *AllocByTLAB(size_t size, ManagedThread *thread);

    void *AllocateMemoryForObject(size_t size, Alignment align, ManagedThread *thread,
                                  ObjectAllocatorBase::ObjMemInitPolicy objInitType, bool pinned = false);

    ObjectAllocatorPtr GetObjectAllocator() const;

    static constexpr float DEFAULT_TARGET_UTILIZATION = 0.5;

    bool isInitialized_ = false;
    bool useRuntimeInternalAllocator_ {true};
    CodeAllocator *codeAllocator_ = nullptr;
    InternalAllocatorPtr internalAllocator_ = nullptr;
    ObjectAllocatorPtr objectAllocator_ = nullptr;

    bool useTlabForAllocations_ = false;
    bool isAdaptiveTlabSize_ = false;

    /// StackFrameAllocator is per thread
    StackFrameAllocator *GetCurrentStackFrameAllocator();

    friend class ::ark::Runtime;

    // Needed to extract object allocator created during HeapManager initialization
    // to pass it to GC creation method;
    friend class ::ark::mem::MemoryManager;

    /**
     * NOTE : Target ideal heap utilization ratio.
     * To implement the getTargetHeapUtilization and nativeSetTargetHeapUtilization, I set a variable here.
     * It may need to be initialized, but now I give it a fixed initial value 0.5
     */
    float targetUtilization_;

    IsObjectFinalizebleFunc isObjectFinalizebleFunc_ = nullptr;
    RegisterFinalizeReferenceFunc registerFinalizeReferenceFunc_ = nullptr;
    PandaVM *vm_ {nullptr};
    MemStatsType *memStats_ {nullptr};
    mem::GC *gc_ = nullptr;
    RuntimeNotificationManager *notificationManager_ = nullptr;
};

}  // namespace ark::mem

#endif  // PANDA_MEM_HEAP_MANAGER_H

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
#ifndef PANDA_RUNTIME_THREAD_H_
#define PANDA_RUNTIME_THREAD_H_

#include <cstdint>
#include <memory>
#include <chrono>
#include <limits>
#include <thread>
#include <atomic>
#include <csignal>

#include "libarkbase/mem/gc_barrier.h"
#include "libarkbase/mem/ringbuf/lock_free_ring_buffer.h"
#include "libarkbase/mem/weighted_adaptive_tlab_average.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/list.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/tsan_interface.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/language_context.h"
#include "runtime/include/thread_proxy.h"
#include "runtime/include/locks.h"
#include "runtime/include/thread_status.h"
#include "runtime/interpreter/cache.h"
#include "runtime/mem/frame_allocator-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/tlab.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "runtime/entrypoints/entrypoints.h"
#include "libarkbase/events/events.h"
#ifdef PANDA_USE_QOS
#include "qos.h"
#endif

enum Priority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_DEFAULT = 2,
    PRIORITY_HIGH = 3,
};

#define ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS()

namespace ark {

template <class TYPE>
class HandleStorage;
template <class TYPE>
class GlobalHandleStorage;
template <class TYPE>
class HandleScope;

namespace test {
class ThreadTest;
}  // namespace test

class ThreadManager;
class Runtime;
class PandaVM;
class MonitorPool;

namespace mem {
class GCBarrierSet;
}  // namespace mem

namespace tooling {
class PtThreadInfo;
}  // namespace tooling

struct CustomTLSData {
    CustomTLSData() = default;
    virtual ~CustomTLSData() = default;

    NO_COPY_SEMANTIC(CustomTLSData);
    NO_MOVE_SEMANTIC(CustomTLSData);
};

class LockedObjectInfo {
public:
    LockedObjectInfo(ObjectHeader *obj, void *fp) : object_(obj), stack_(fp) {}
    inline ObjectHeader *GetObject() const
    {
        return object_;
    }

    inline void SetObject(ObjectHeader *objNew)
    {
        object_ = objNew;
    }

    ALWAYS_INLINE void UpdateObject(const ReferenceUpdater &updater)
    {
        [[maybe_unused]] ObjectStatus status = updater(&object_);
        ASSERT(status == ObjectStatus::ALIVE_OBJECT);
    }

    inline void *GetStack() const
    {
        return stack_;
    }

    inline void SetStack(void *stackNew)
    {
        stack_ = stackNew;
    }

    static constexpr uint32_t GetMonitorOffset()
    {
        return MEMBER_OFFSET(LockedObjectInfo, object_);
    }

    static constexpr uint32_t GetStackOffset()
    {
        return MEMBER_OFFSET(LockedObjectInfo, stack_);
    }

private:
    ObjectHeader *object_;
    void *stack_;
};

template <typename Adapter = mem::AllocatorAdapter<LockedObjectInfo>>
class LockedObjectList {
    static constexpr uint32_t DEFAULT_CAPACITY = 16;

public:
    LockedObjectList() : capacity_(DEFAULT_CAPACITY), allocator_(Adapter())
    {
        storage_ = allocator_.allocate(DEFAULT_CAPACITY);
    }

    ~LockedObjectList()
    {
        allocator_.deallocate(storage_, capacity_);
    }

    NO_COPY_SEMANTIC(LockedObjectList);
    NO_MOVE_SEMANTIC(LockedObjectList);

    void PushBack(LockedObjectInfo data)
    {
        ExtendIfNeeded();
        storage_[size_++] = data;
    }

    template <typename... Args>
    LockedObjectInfo &EmplaceBack(Args &&...args)
    {
        ExtendIfNeeded();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *rawMem = &storage_[size_];
        auto *datum = new (rawMem) LockedObjectInfo(std::forward<Args>(args)...);
        size_++;
        return *datum;
    }

    LockedObjectInfo &Back()
    {
        ASSERT(size_ > 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return storage_[size_ - 1];
    }

    bool Empty() const
    {
        return size_ == 0;
    }

    void PopBack()
    {
        ASSERT(size_ > 0);
        --size_;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        (&storage_[size_])->~LockedObjectInfo();
    }

    Span<LockedObjectInfo> Data()
    {
        return Span<LockedObjectInfo>(storage_, size_);
    }

    static constexpr uint32_t GetCapacityOffset()
    {
        return MEMBER_OFFSET(LockedObjectList, capacity_);
    }

    static constexpr uint32_t GetSizeOffset()
    {
        return MEMBER_OFFSET(LockedObjectList, size_);
    }

    static constexpr uint32_t GetDataOffset()
    {
        return MEMBER_OFFSET(LockedObjectList, storage_);
    }

private:
    void ExtendIfNeeded()
    {
        ASSERT(size_ <= capacity_);
        if (size_ < capacity_) {
            return;
        }
        uint32_t newCapacity = capacity_ * 3U / 2U;  // expand by 1.5
        LockedObjectInfo *newStorage = allocator_.allocate(newCapacity);
        ASSERT(newStorage != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::copy(storage_, storage_ + size_, newStorage);
        allocator_.deallocate(storage_, capacity_);
        storage_ = newStorage;
        capacity_ = newCapacity;
    }

    template <typename T, size_t ALIGNMENT = sizeof(T)>
    using Aligned __attribute__((aligned(ALIGNMENT))) = T;
    // Use uint32_t instead of size_t to guarantee the same size
    // on all platforms and simplify compiler stubs accessing this fields.
    // uint32_t is large enough to fit locked objects list's size.
    Aligned<uint32_t> capacity_;
    Aligned<uint32_t> size_ {0};
    Aligned<LockedObjectInfo *> storage_;
    Adapter allocator_;
};

/**
 *  Hierarchy of thread classes
 *
 *         +--------+
 *         | Thread |
 *         +--------+
 *             |
 *      +---------------+
 *      | ManagedThread |
 *      +---------------+
 *             |
 *     +-----------------+
 *     | MTManagedThread |
 *     +-----------------+
 *
 *
 *  Thread - is the most low-level entity. This class contains pointers to VM which this thread associated.
 *  ManagedThread - stores runtime context to run managed code in single-threaded environment
 *  MTManagedThread - extends ManagedThread to be able to run code in multi-threaded environment
 */

/// @brief Class represents arbitrary runtime thread
// NOLINTNEXTLINE(clang-analyzer-optin.performance.Padding)
class Thread : public ThreadProxy {
public:
    using ThreadId = uint32_t;
    enum class ThreadType {
        THREAD_TYPE_NONE,
        THREAD_TYPE_GC,
        THREAD_TYPE_COMPILER,
        THREAD_TYPE_MANAGED,
        THREAD_TYPE_MT_MANAGED,
        THREAD_TYPE_TASK,
        THREAD_TYPE_WORKER_THREAD,
    };

    Thread(PandaVM *vm, ThreadType threadType);
    virtual ~Thread();
    NO_COPY_SEMANTIC(Thread);
    NO_MOVE_SEMANTIC(Thread);

    PANDA_PUBLIC_API static Thread *GetCurrent();
    PANDA_PUBLIC_API static void SetCurrent(Thread *thread);

    virtual void FreeInternalMemory();

    void FreeAllocatedMemory();

    PandaVM *GetVM() const
    {
        return vm_;
    }

    void SetVM(PandaVM *vm)
    {
        vm_ = vm;
    }

    void *GetPreWrbEntrypoint() const
    {
        // Atomic with relaxed order reason: only atomicity and modification order consistency needed
        return preWrbEntrypoint_.load(std::memory_order_relaxed);
    }

    void SetPreWrbEntrypoint(void *entry)
    {
        preWrbEntrypoint_ = entry;
    }

    ThreadType GetThreadType() const
    {
        return threadType_;
    }

    ALWAYS_INLINE mem::GCBarrierSet *GetBarrierSet() const
    {
        return barrierSet_;
    }

    // pre_buff_ may be destroyed during Detach(), so it should be initialized once more
    void InitPreBuff();

    static constexpr size_t GetVmOffset()
    {
        return MEMBER_OFFSET(Thread, vm_);
    }

    uint64_t *GetThreadRandomState()
    {
        return &threadRandomState_;
    }

private:
    void InitCardTableData(mem::GCBarrierSet *barrier);
    void InitThreadRandomState();

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    bool isCompiledFrame_ {false};
    ThreadId internalId_ {0};

    EntrypointsTable *entrypointsTable_ {nullptr};
    void *object_ {nullptr};
    Frame *frame_ {nullptr};
    ObjectHeader *exception_ {nullptr};
    uintptr_t nativePc_ {};
    mem::TLAB *tlab_ {nullptr};
    void *cardTableAddr_ {nullptr};
    void *cardTableMinAddr_ {nullptr};
    std::atomic<void *> preWrbEntrypoint_ {nullptr};  // if NOT nullptr, stores pointer to PreWrbFunc and indicates we
                                                      // are currently in concurrent marking phase
    // keeps IRtoC GC PostWrb impl for storing one object
    void *postWrbOneObject_ {nullptr};
    // keeps IRtoC GC PostWrb impl for storing two objects
    void *postWrbTwoObjects_ {nullptr};
    void *stringClassPtr_ {nullptr};    // ClassRoot::LINE_STRING
    void *arrayU16ClassPtr_ {nullptr};  // ClassRoot::ARRAY_U16
    void *arrayU8ClassPtr_ {nullptr};   // ClassRoot::ARRAY_U8
    PandaVector<ObjectHeader *> *preBuff_ {nullptr};
    void *languageExtensionData_ {nullptr};
#ifndef NDEBUG
    uintptr_t runtimeCallEnabled_ {1};
#endif
    uint64_t threadRandomState_ {0};
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    PandaVM *vm_ {nullptr};
    ThreadType threadType_ {ThreadType::THREAD_TYPE_NONE};
    mem::GCBarrierSet *barrierSet_ {nullptr};
#ifndef PANDA_TARGET_WINDOWS
    stack_t signalStack_ {};
#endif
};

template <typename ThreadT>
class ScopedCurrentThread {
public:
    explicit ScopedCurrentThread(ThreadT *thread) : thread_(thread)
    {
        ASSERT(Thread::GetCurrent() == nullptr);

        // Set current thread
        Thread::SetCurrent(thread_);
    }

    ~ScopedCurrentThread()
    {
        // Reset current thread
        Thread::SetCurrent(nullptr);
    }

    NO_COPY_SEMANTIC(ScopedCurrentThread);
    NO_MOVE_SEMANTIC(ScopedCurrentThread);

private:
    ThreadT *thread_;
};

class QosHelper {
public:
    static int SetCurrentWorkerPriority([[maybe_unused]] Priority priority)
    {
#ifdef PANDA_USE_QOS
        return SetThreadQos(static_cast<OHOS::QOS::QosLevel>(priority));
#else
        LOG(INFO, RUNTIME) << "QosHelper::SetWorkerPriority is not supported";
        return 0;
#endif
    }
};

}  // namespace ark

#ifdef PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS
#include "platforms/mobile/runtime/thread-inl.cpp"
#endif  // PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS

#endif  // PANDA_RUNTIME_THREAD_H_

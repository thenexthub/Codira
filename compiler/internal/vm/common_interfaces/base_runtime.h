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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_BASE_RUNTIME_H
#define COMMON_INTERFACES_BASE_RUNTIME_H

#include <atomic>
#include <functional>
#include <mutex>

#include "base/runtime_param.h"
#include "heap/heap_visitor.h"
namespace common {
class BaseStringTableImpl;
template <typename Impl>
class BaseStringTableInterface;
using StringTableCleanUpCallback = std::function<void(void)>;
using StringTableProcessCallback = std::function<void(const WeakRefFieldVisitor&)>;
class BaseObject;
class HeapManager;
class Mutator;
class MutatorManager;
class ThreadHolderManager;
class ThreadHolder;
class BaseClassRoots;

// Used by Collector::RequestGC.
// It tells why GC is triggered.
//
// sync: Caller of Collector::RequestGC will wait until GC completes.
// async: Collector::RequestGC returns immediately and caller continues to run.
enum GCReason : uint32_t {
    GC_REASON_BEGIN = 0,
    GC_REASON_USER = GC_REASON_BEGIN,  // Triggered by user explicitly.
    GC_REASON_OOM,                     // Out of memory. Failed to allocate object.
    GC_REASON_BACKUP,                  // backup gc is triggered if no other reason triggers gc for a long time.
    GC_REASON_HEU,                     // Statistics show it is worth doing GC. Does not have to be immediate.
    GC_REASON_YOUNG,                   // Statistics show it is worth doing Young GC. Does not have to be immediate.
    GC_REASON_NATIVE,                  // Native-Allocation-Registry shows it's worth doing GC.
    GC_REASON_HEU_SYNC,                // Just wait one gc request to reduce heap fragmentation.
    GC_REASON_NATIVE_SYNC,             // Just wait one gc request to reduce native heap consumption.
    GC_REASON_FORCE,                   // force gc is triggered when runtime triggers gc actively.
    GC_REASON_APPSPAWN,                // appspawn gc is triggered when prefork.
    GC_REASON_XREF,                    // force gc the whole heap include XRef.
    GC_REASON_BACKGROUND,              // trigger gc caused by switching to background.
    GC_REASON_HINT,                    // trigger gc caused by hint gc.
    GC_REASON_IDLE,                    // When in a low activity state, trigger gc to reduce resicdent memory.
    GC_REASON_END = GC_REASON_IDLE,
    GC_REASON_INVALID = std::numeric_limits<uint32_t>::max(),
};

inline const char* GCREASON_STRING[] = {
    "user",
    "oom",
    "backup",
    "heuristic",
    "young",
    "native_alloc",
    "heuristic_sync",
    "native_alloc_sync",
    "force",
    "appspawn",
    "xref",
    "backgound",
    "hint",
    "idle",
};

inline const char* GCReasonToString(GCReason reason)
{
    if (reason >= GC_REASON_BEGIN && reason <= GC_REASON_END) {
        return GCREASON_STRING[reason];
    }
    return "UNKNOW_GC_REASON";
}

enum GCType : uint32_t {
    GC_TYPE_BEGIN = 0,
    GC_TYPE_FULL = GC_TYPE_BEGIN,
    GC_TYPE_YOUNG,
    GC_TYPE_STW,
    GC_TYPE_END = GC_TYPE_STW,
};

inline const char* GCTYPE_STRING[] = {
    "FULL",
    "YOUNG",
    "STW",
};

inline const char* GCTypeToString(GCType gcType)
{
    if (gcType >= GC_TYPE_BEGIN && gcType <= GC_TYPE_END) {
        return GCTYPE_STRING[gcType];
    }
    return "UNKNOW_GC_TYPE";
}

enum class MemoryReduceDegree : uint8_t {
    LOW = 0,
    HIGH,
};

using HeapVisitor = const std::function<void(BaseObject*)>;

class PUBLIC_API BaseRuntime {
public:
    BaseRuntime() = default;
    ~BaseRuntime() = default;

    static BaseRuntime *GetInstance();
    static void DestroyInstance();

    void PreFork(ThreadHolder *holder);
    void PostFork(bool enableWarmStartup);

    bool HasBeenInitialized();
    void Init(const RuntimeParam &param);   // Support setting custom parameters
    void Init();                            // Use default parameters
    void InitFromDynamic();
    void InitFromDynamic(const RuntimeParam &param);
    void Fini();
    void FiniFromDynamic();

    // Need refactor, move to other file
    static void WriteRoot(void* obj);
    static void WriteBarrier(void* obj, void* field, void* ref, Mutator *mutator = nullptr);
    static void* ReadBarrier(void* obj, void* field);
    static void* ReadBarrier(void* field);
    static void* AtomicReadBarrier(void* obj, void* field, std::memory_order order);
    static void RequestGC(GCReason reason, bool async, GCType gcType);
    static void WaitForGCFinish();
    static void EnterGCCriticalSection();
    static void ExitGCCriticalSection();
    static bool ForEachObj(HeapVisitor& visitor, bool safe);
    static void NotifyNativeAllocation(size_t bytes);
    static void NotifyNativeFree(size_t bytes);
    static void NotifyNativeReset(size_t oldBytes, size_t newBytes);
    static size_t GetNotifiedNativeSize();
    static void ChangeGCParams(bool isBackground);
    static bool CheckAndTriggerHintGC(MemoryReduceDegree degree);
    static void NotifyHighSensitive(bool isStart);
    static void NotifyWarmStart();
    static void FillFreeObject(void *object, size_t size);

    HeapParam &GetHeapParam()
    {
        return param_.heapParam;
    }

    GCParam &GetGCParam()
    {
        return param_.gcParam;
    }

    MutatorManager &GetMutatorManager()
    {
        return *mutatorManager_;
    }

    ThreadHolderManager &GetThreadHolderManager()
    {
        return *threadHolderManager_;
    }

    HeapManager &GetHeapManager()
    {
        return *heapManager_;
    }

    BaseClassRoots &GetBaseClassRoots()
    {
        return *baseClassRoots_;
    }

    BaseStringTableInterface<BaseStringTableImpl> &GetStringTable()
    {
        return *stringTable_;
    }

    void StringTableCleanUp()
    {
        if (stringTableCleanUpCallback_) {
            stringTableCleanUpCallback_();
        }
    }

    void SetStringTableCleanUpCallback(StringTableCleanUpCallback callback)
    {
        stringTableCleanUpCallback_ = callback;
    }

    void ProcessStringTable(const WeakRefFieldVisitor &visitor)
    {
        if (stringTableProcessCallback_) {
            stringTableProcessCallback_(visitor);
        }
    }

    void SetStringTableProcessCallback(StringTableProcessCallback callback)
    {
        stringTableProcessCallback_ = callback;
    }

private:
    RuntimeParam param_ {};

    HeapManager* heapManager_ = nullptr;
    MutatorManager* mutatorManager_ = nullptr;
    ThreadHolderManager* threadHolderManager_  = nullptr;
    BaseClassRoots* baseClassRoots_ = nullptr;
    BaseStringTableInterface<BaseStringTableImpl>* stringTable_ = nullptr;
    StringTableCleanUpCallback stringTableCleanUpCallback_ {};
    StringTableProcessCallback stringTableProcessCallback_ {};
    static std::mutex vmCreationLock_;
    static BaseRuntime *baseRuntimeInstance_;
    static bool initialized_;
};
}  // namespace common
#endif // COMMON_INTERFACES_BASE_RUNTIME_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)

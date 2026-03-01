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

#ifndef COMMON_COMPONENTS_OBJECTS_STRING_TABLE_INTERNAL_H
#define COMMON_COMPONENTS_OBJECTS_STRING_TABLE_INTERNAL_H

#include "common_components/base/globals.h"
#include "common_components/platform/mutex.h"
#include "common_interfaces/thread/thread_holder.h"
#include "common_interfaces/objects/base_string_table.h"
#include "common_components/objects/string_table/hashtriemap.h"
#include "common_components/taskpool/task.h"

namespace common {

class BaseStringTableMutex {
public:
    explicit BaseStringTableMutex(bool isInit = true) : mtx_(isInit) {}

    void LockWithThreadState(ThreadHolder* holder)
    {
        if (mtx_.TryLock()) {
            return;
        }
#ifndef NDEBUG
        BaseRuntime::RequestGC(GC_REASON_USER, true, GC_TYPE_FULL);  // Trigger CMC FULL GC
#endif
        ThreadStateTransitionScope<ThreadHolder, ThreadState::WAIT> ts(holder);
        mtx_.Lock();
    }

    void Lock()
    {
        return mtx_.Lock();
    }

    void Unlock()
    {
        return mtx_.Unlock();
    }

private:
    Mutex mtx_;
};

template<bool ConcurrentSweep>
class BaseStringTableInternal;

class BaseStringTableCleaner {
public:
    using IteratorPtr = std::shared_ptr<std::atomic<uint32_t>>;
    BaseStringTableCleaner(BaseStringTableInternal<true>* stringTable) : stringTable_(stringTable) {}
    ~BaseStringTableCleaner()
    {
        stringTable_ = nullptr;
    }
    void PostSweepWeakRefTask(const WeakRefFieldVisitor &visitor);
    void JoinAndWaitSweepWeakRefTask(const WeakRefFieldVisitor &visitor);

    void CleanUp();
private:
    NO_COPY_SEMANTIC_CC(BaseStringTableCleaner);
    NO_MOVE_SEMANTIC_CC(BaseStringTableCleaner);
    static void ProcessSweepWeakRef(IteratorPtr &iter,
                                    BaseStringTableCleaner *cleaner,
                                    const WeakRefFieldVisitor &visitor);

    void StartSweepWeakRefTask();
    void WaitSweepWeakRefTask();
    void SignalSweepWeakRefTask();

    static inline uint32_t GetNextIndexId(IteratorPtr &iter)
    {
        return iter->fetch_add(1U, std::memory_order_relaxed);
    }

    static inline bool ReduceCountAndCheckFinish(BaseStringTableCleaner *cleaner)
    {
        return (cleaner->PendingTaskCount_.fetch_sub(1U, std::memory_order_relaxed) == 1U);
    }

    class CMCSweepWeakRefTask : public Task {
      public:
        CMCSweepWeakRefTask(IteratorPtr iter, BaseStringTableCleaner *cleaner,
                            const WeakRefFieldVisitor &visitor)
            : Task(0), iter_(iter), cleaner_(cleaner), visitor_(visitor) {}
        ~CMCSweepWeakRefTask() = default;

        bool Run(uint32_t threadIndex) override;

        NO_COPY_SEMANTIC_CC(CMCSweepWeakRefTask);
        NO_MOVE_SEMANTIC_CC(CMCSweepWeakRefTask);

      private:
        IteratorPtr iter_;
        BaseStringTableCleaner *cleaner_;
        const WeakRefFieldVisitor &visitor_;
    };

    IteratorPtr iter_{};
    BaseStringTableInternal<true>* stringTable_;
    std::atomic<uint32_t> PendingTaskCount_{0U};
    std::array<std::vector<HashTrieMapEntry*>, TrieMapConfig::ROOT_SIZE> waitFreeEntries_{};
    Mutex sweepWeakRefMutex_{};
    bool sweepWeakRefFinished_{true};
    ConditionVariable sweepWeakRefCV_{};
};

template<bool ConcurrentSweep>
class BaseStringTableInternal {
public:
    using HandleCreator = BaseStringTableInterface<BaseStringTableImpl>::HandleCreator;
    using HashTrieMapType = std::conditional_t<ConcurrentSweep,
        HashTrieMap<BaseStringTableMutex, ThreadHolder, TrieMapConfig::NeedSlotBarrier>,
        HashTrieMap<BaseStringTableMutex, ThreadHolder, TrieMapConfig::NoSlotBarrier>>;

    template <bool B = ConcurrentSweep, std::enable_if_t<B, int> = 0>
    BaseStringTableInternal(): cleaner_(new BaseStringTableCleaner(this)) {}

    template <bool B = ConcurrentSweep, std::enable_if_t<!B, int> = 0>
    BaseStringTableInternal() {}

    ~BaseStringTableInternal()
    {
        if constexpr (ConcurrentSweep) {
            delete cleaner_;
        }
    }

    BaseString *GetOrInternFlattenString(ThreadHolder *holder, const HandleCreator &handleCreator, BaseString *string);

    BaseString *GetOrInternStringFromCompressedSubString(ThreadHolder *holder, const HandleCreator &handleCreator,
                                                         const ReadOnlyHandle<BaseString> &string, uint32_t offset,
                                                         uint32_t utf8Len);

    BaseString *GetOrInternString(ThreadHolder *holder, const HandleCreator &handleCreator, const uint8_t *utf8Data,
                                  uint32_t utf8Len, bool canBeCompress);

    BaseString *GetOrInternString(ThreadHolder *holder, const HandleCreator &handleCreator, const uint16_t *utf16Data,
                                  uint32_t utf16Len, bool canBeCompress);

    BaseString *TryGetInternString(const ReadOnlyHandle<BaseString> &string);

    HashTrieMapType &GetHashTrieMap()
    {
        return stringTable_;
    }

    BaseStringTableCleaner *GetCleaner()
    {
        return cleaner_;
    }

    template <bool B = ConcurrentSweep, std::enable_if_t<B, int> = 0>
    void SweepWeakRef(const WeakRefFieldVisitor& visitor, uint32_t rootID,
                      std::vector<HashTrieMapEntry*>& waitDeleteEntries);

    template <bool B = ConcurrentSweep, std::enable_if_t<B, int> = 0>
    void CleanUp();

    template <bool B = ConcurrentSweep, std::enable_if_t<!B, int> = 0>
    void SweepWeakRef(const WeakRefFieldVisitor& visitor);
private:

    HashTrieMapType stringTable_{};
    BaseStringTableCleaner* cleaner_ = nullptr;
    static BaseString* AllocateLineStringObject(size_t size);
    static constexpr size_t MAX_REGULAR_HEAP_OBJECT_SIZE = 32 * KB;
};

class BaseStringTableImpl : public BaseStringTableInterface<BaseStringTableImpl> {
public:
    BaseString* GetOrInternFlattenString(ThreadHolder* holder, const HandleCreator& handleCreator, BaseString* string);

    BaseString* GetOrInternStringFromCompressedSubString(ThreadHolder* holder, const HandleCreator& handleCreator,
                                                         const ReadOnlyHandle<BaseString>& string, uint32_t offset,
                                                         uint32_t utf8Len);

    BaseString* GetOrInternString(ThreadHolder* holder, const HandleCreator& handleCreator, const uint8_t* utf8Data,
                                  uint32_t utf8Len, bool canBeCompress);

    BaseString* GetOrInternString(ThreadHolder* holder, const HandleCreator& handleCreator, const uint16_t* utf16Data,
                                  uint32_t utf16Len, bool canBeCompress);

    BaseString* TryGetInternString(const ReadOnlyHandle<BaseString>& string);

    auto* GetInternalTable()
    {
        return &stringTable_;
    }
private:
#ifndef GC_STW_STRINGTABLE
    BaseStringTableInternal<true> stringTable_{};
#else
    BaseStringTableInternal<false> stringTable_{};
#endif
};
}
#endif //COMMON_COMPONENTS_OBJECTS_STRING_TABLE_INTERNAL_H


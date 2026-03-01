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

#ifndef RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_H
#define RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_H

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/include/mem/allocator.h"

namespace ark::mem {
namespace test {
class MemStatsGenGCTest;
}  // namespace test
/// Base class for generational GC
template <class LanguageConfig>
class GenerationalGC : public GCLang<LanguageConfig> {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    CardTable *GetCardTable() const override
    {
        return cardTable_.get();
    }

    bool Trigger(PandaUniquePtr<GCTask> task) override;

protected:
    GenerationalGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
        : GCLang<LanguageConfig>(objectAllocator, settings)
    {
    }
    virtual bool ShouldRunTenuredGC(const GCTask &task);

    void DisableTenuredGC()
    {
        majorPeriod_ = DISABLED_MAJOR_PERIOD;  // Disable tenured GC temporarily.
    }

    void RestoreTenuredGC()
    {
        majorPeriod_ = DEFAULT_MAJOR_PERIOD;
    }

    ALWAYS_INLINE size_t GetMajorPeriod() const
    {
        return majorPeriod_;
    }

    void PostForkCallback([[maybe_unused]] size_t restoreLimit) override
    {
        GenerationalGC<LanguageConfig>::RestoreTenuredGC();
    }

    template <typename Marker>
    NO_THREAD_SAFETY_ANALYSIS void MarkImpl(Marker *marker, GCMarkingStackType *objectsStack,
                                            CardTableVisitFlag visitCardTableRoots,
                                            const ReferenceCheckPredicateT &refPred,
                                            const MemRangeChecker &memRangeChecker,
                                            const GC::MarkPreprocess &markPreprocess = GC::EmptyMarkPreprocess);

    /// Mark all objects in stack recursively
    template <typename Marker, class... ReferenceCheckPredicate>
    void MarkStack(Marker *marker, GCMarkingStackType *stack, const GC::MarkPreprocess &markPreprocess,
                   const ReferenceCheckPredicate &...refPred);

    /**
     * Update statistics in MemStats. Required initialized mem_stats_ field.
     * @param bytes_in_heap_before - bytes in heap before the GC
     * @param update_tenured_stats - if true, we will update tenured moved and tenured deleted memstats too
     * @param record_allocation_for_moved_objects - if true, we will record allocation for all moved objects (young and
     *  tenured)
     */
    void UpdateMemStats(size_t bytesInHeapBefore, bool updateTenuredStats = false,
                        bool recordAllocationForMovedObjects = false);

    class MemStats {
    public:
        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordCountFreedYoung(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&youngFreeObjectCount_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                youngFreeObjectCount_ += count;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordSizeFreedYoung(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&youngFreeObjectSize_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                youngFreeObjectSize_ += size;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordCountMovedYoung(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&youngMoveObjectCount_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                youngMoveObjectCount_ += count;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordSizeMovedYoung(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&youngMoveObjectSize_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                youngMoveObjectSize_ += size;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordYoungStats(size_t youngMoveSize, size_t youngMoveCount, size_t youngDeleteSize,
                                            size_t youngDeleteCount)
        {
            RecordSizeMovedYoung(youngMoveSize);
            RecordCountMovedYoung(youngMoveCount);
            RecordSizeFreedYoung(youngDeleteSize);
            RecordCountFreedYoung(youngDeleteCount);
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordCountMovedTenured(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&tenuredMoveObjectCount_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenuredMoveObjectCount_ += count;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordSizeMovedTenured(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&tenuredMoveObjectSize_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenuredMoveObjectSize_ += size;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordCountFreedTenured(size_t count)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint32_t> *>(&tenuredFreeObjectCount_)
                    ->fetch_add(count, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenuredFreeObjectCount_ += count;
            }
        }

        template <bool ATOMIC = false>
        ALWAYS_INLINE void RecordSizeFreedTenured(size_t size)
        {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (ATOMIC) {
                // Atomic with relaxed order reason: memory accesses from different threads
                reinterpret_cast<std::atomic<uint64_t> *>(&tenuredFreeObjectSize_)
                    ->fetch_add(size, std::memory_order_relaxed);
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                tenuredFreeObjectSize_ += size;
            }
        }

        void Reset()
        {
            youngFreeObjectCount_ = 0U;
            youngFreeObjectSize_ = 0U;
            youngMoveObjectCount_ = 0U;
            youngMoveObjectSize_ = 0U;
            tenuredFreeObjectCount_ = 0U;
            tenuredFreeObjectSize_ = 0U;
            tenuredMoveObjectCount_ = 0U;
            tenuredMoveObjectSize_ = 0U;
        }

        PandaString Dump();

        ALWAYS_INLINE size_t GetCountFreedYoung()
        {
            return youngFreeObjectCount_;
        }

        ALWAYS_INLINE size_t GetSizeFreedYoung()
        {
            return youngFreeObjectSize_;
        }

        ALWAYS_INLINE size_t GetCountMovedYoung()
        {
            return youngMoveObjectCount_;
        }

        ALWAYS_INLINE size_t GetSizeMovedYoung()
        {
            return youngMoveObjectSize_;
        }

        ALWAYS_INLINE size_t GetCountFreedTenured()
        {
            return tenuredFreeObjectCount_;
        }

        ALWAYS_INLINE size_t GetSizeFreedTenured()
        {
            return tenuredFreeObjectSize_;
        }

        ALWAYS_INLINE size_t GetCountMovedTenured()
        {
            return tenuredMoveObjectCount_;
        }

        ALWAYS_INLINE size_t GetSizeMovedTenured()
        {
            return tenuredMoveObjectSize_;
        }

    private:
        uint32_t youngFreeObjectCount_ {0U};
        uint64_t youngFreeObjectSize_ {0U};
        uint32_t youngMoveObjectCount_ {0U};
        uint64_t youngMoveObjectSize_ {0U};
        uint32_t tenuredFreeObjectCount_ {0U};
        uint64_t tenuredFreeObjectSize_ {0U};
        uint32_t tenuredMoveObjectCount_ {0U};
        uint64_t tenuredMoveObjectSize_ {0U};

        friend class GenerationalGC;
        friend class test::MemStatsGenGCTest;
    };

    ALWAYS_INLINE ObjectAllocatorGenBase *GetObjectGenAllocator()
    {
        return static_cast<ObjectAllocatorGenBase *>(this->GetObjectAllocator());
    }

    void CreateCardTable(InternalAllocatorPtr internalAllocatorPtr, uintptr_t minAddress, size_t size);

    template <typename Marker>
    void VisitCardTableConcurrent(Marker *marker, GCMarkingStackType *objectsStack,
                                  const ReferenceCheckPredicateT &refPred, const MemRangeChecker &memRangeChecker,
                                  const GC::MarkPreprocess &markPreprocess);

    void PrintDetailedLog() override
    {
        LOG(INFO, GC) << memStats_.Dump();
        GC::PrintDetailedLog();
    }

    MemStats memStats_;  // NOLINT(misc-non-private-member-variables-in-classes)
private:
    static constexpr size_t DEFAULT_MAJOR_PERIOD = 3;
    static constexpr size_t DISABLED_MAJOR_PERIOD = 65535;
    size_t majorPeriod_ {DEFAULT_MAJOR_PERIOD};
    PandaUniquePtr<CardTable> cardTable_ {nullptr};
    friend class test::MemStatsGenGCTest;
};

}  // namespace ark::mem
#endif  // RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_H

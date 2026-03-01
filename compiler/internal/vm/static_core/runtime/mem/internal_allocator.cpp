/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "include/runtime.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/include/thread.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INTERNAL_ALLOCATOR(level) LOG(level, ALLOC) << "InternalAllocator: "

#if defined(TRACK_INTERNAL_ALLOCATIONS)
static AllocTracker *CreateAllocTracker()
{
    static constexpr int SIMPLE_ALLOC_TRACKER = 1;
    static constexpr int DETAIL_ALLOC_TRACKER = 2;

    if constexpr (TRACK_INTERNAL_ALLOCATIONS == SIMPLE_ALLOC_TRACKER) {
        return new SimpleAllocTracker();
    } else if (TRACK_INTERNAL_ALLOCATIONS == DETAIL_ALLOC_TRACKER) {
        return new DetailAllocTracker();
    } else {
        UNREACHABLE();
    }
}
#endif  // TRACK_INTERNAL_ALLOCATIONS

template <InternalAllocatorConfig CONFIG>
Allocator *InternalAllocator<CONFIG>::allocatorFromRuntime_ = nullptr;

template <InternalAllocatorConfig CONFIG>
InternalAllocator<CONFIG>::InternalAllocator(MemStatsType *memStats)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        runslotsAllocator_ = new RunSlotsAllocatorT(memStats, SpaceType::SPACE_TYPE_INTERNAL);
        freelistAllocator_ = new FreeListAllocatorT(memStats, SpaceType::SPACE_TYPE_INTERNAL);
        humongousAllocator_ = new HumongousObjAllocatorT(memStats, SpaceType::SPACE_TYPE_INTERNAL);
    } else {  // NOLINT(readability-misleading-indentation
        mallocAllocator_ = new MallocProxyAllocatorT(memStats, SpaceType::SPACE_TYPE_INTERNAL);
    }

#if defined(TRACK_INTERNAL_ALLOCATIONS)
    memStats_ = memStats;
    tracker_ = CreateAllocTracker();
#endif  // TRACK_INTERNAL_ALLOCATIONS
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Initializing InternalAllocator finished";
}

template <InternalAllocatorConfig CONFIG>
template <AllocScope ALLOC_SCOPE_T>
[[nodiscard]] void *InternalAllocator<CONFIG>::Alloc(size_t size, Alignment align)
{
#ifdef TRACK_INTERNAL_ALLOCATIONS
    os::memory::LockHolder lock(lock_);
#endif  // TRACK_INTERNAL_ALLOCATIONS
    void *res = nullptr;
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Try to allocate " << size << " bytes";
    if (UNLIKELY(size == 0)) {
        LOG_INTERNAL_ALLOCATOR(DEBUG) << "Failed to allocate - size of object is zero";
        return nullptr;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        res = AllocViaPandaAllocators<ALLOC_SCOPE_T>(size, align);
    } else {  // NOLINT(readability-misleading-indentation
        res = mallocAllocator_->Alloc(size, align);
    }
    if (res == nullptr) {
        return nullptr;
    }
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Allocate " << size << " bytes at address " << std::hex << res;
#ifdef TRACK_INTERNAL_ALLOCATIONS
    tracker_->TrackAlloc(res, AlignUp(size, align), SpaceType::SPACE_TYPE_INTERNAL);
#endif  // TRACK_INTERNAL_ALLOCATIONS
    return res;
}

template <InternalAllocatorConfig CONFIG>
void InternalAllocator<CONFIG>::Free(void *ptr)
{
#ifdef TRACK_INTERNAL_ALLOCATIONS
    os::memory::LockHolder lock(lock_);
#endif  // TRACK_INTERNAL_ALLOCATIONS
    if (ptr == nullptr) {
        return;
    }
#ifdef TRACK_INTERNAL_ALLOCATIONS
    // Do it before actual free even we don't do something with ptr.
    // Clang tidy detects memory at ptr gets unavailable after free
    // and reports errors.
    tracker_->TrackFree(ptr);
#endif  // TRACK_INTERNAL_ALLOCATIONS
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Try to free via InternalAllocator at address " << std::hex << ptr;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        FreeViaPandaAllocators(ptr);
    } else {  // NOLINT(readability-misleading-indentation
        mallocAllocator_->Free(ptr);
    }
}

template <InternalAllocatorConfig CONFIG>
InternalAllocator<CONFIG>::~InternalAllocator()
{
#ifdef TRACK_INTERNAL_ALLOCATIONS
    if (memStats_->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL) != 0) {
        // Memory leaks are detected.
        LOG(ERROR, RUNTIME) << "Memory leaks detected.";
        tracker_->DumpMemLeaks(std::cerr);
    }
    tracker_->Dump();
    delete tracker_;
#endif  // TRACK_INTERNAL_ALLOCATIONS
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Destroying InternalAllocator";
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        delete runslotsAllocator_;
        delete freelistAllocator_;
        delete humongousAllocator_;
    } else {  // NOLINT(readability-misleading-indentation
        delete mallocAllocator_;
    }
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Destroying InternalAllocator finished";
}

template <class AllocatorT>
void *AllocInRunSlots(AllocatorT *runslotsAllocator, size_t size, Alignment align, size_t poolSize)
{
    void *res = runslotsAllocator->Alloc(size, align);
    if (res == nullptr) {
        // Get rid of extra pool adding to the allocator
        static os::memory::Mutex poolLock;
        os::memory::LockHolder lock(poolLock);
        while (true) {
            res = runslotsAllocator->Alloc(size, align);
            if (res != nullptr) {
                break;
            }
            LOG_INTERNAL_ALLOCATOR(DEBUG) << "RunSlotsAllocator didn't allocate memory, try to add new pool";
            auto pool = PoolManager::GetMmapMemPool()->AllocPool(poolSize, SpaceType::SPACE_TYPE_INTERNAL,
                                                                 AllocatorType::RUNSLOTS_ALLOCATOR, runslotsAllocator);
            if (UNLIKELY(pool.GetMem() == nullptr)) {
                return nullptr;
            }
            runslotsAllocator->AddMemoryPool(pool.GetMem(), pool.GetSize());
            LOG_INTERNAL_ALLOCATOR(DEBUG) << "RunSlotsAllocator try to allocate memory again after pool adding";
        }
    }
    return res;
}

template <InternalAllocatorConfig CONFIG>
template <AllocScope ALLOC_SCOPE_T>
void *InternalAllocator<CONFIG>::AllocViaRunSlotsAllocator(size_t size, Alignment align)
{
    void *res = nullptr;
    if constexpr (ALLOC_SCOPE_T == AllocScope::GLOBAL) {
        LOG_INTERNAL_ALLOCATOR(DEBUG) << "Try to use RunSlotsAllocator";
        res = AllocInRunSlots(runslotsAllocator_, size, align, RunSlotsAllocatorT::GetMinPoolSize());
    } else {
        static_assert(ALLOC_SCOPE_T == AllocScope::LOCAL);
        LOG_INTERNAL_ALLOCATOR(DEBUG) << "Try to use thread-local RunSlotsAllocator";
        ASSERT(ark::ManagedThread::GetCurrent()->GetLocalInternalAllocator() != nullptr);
        res = AllocInRunSlots(ark::ManagedThread::GetCurrent()->GetLocalInternalAllocator(), size, align,
                              LocalSmallObjectAllocator::GetMinPoolSize());
    }
    return res;
}

template <InternalAllocatorConfig CONFIG>
void *InternalAllocator<CONFIG>::AllocViaFreeListAllocator(size_t size, Alignment align)
{
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Try to use FreeListAllocator";
    void *res = freelistAllocator_->Alloc(size, align);
    if (res != nullptr) {
        return res;
    }
    // Get rid of extra pool adding to the allocator
    static os::memory::Mutex poolLock;
    os::memory::LockHolder lock(poolLock);
    while (true) {
        res = freelistAllocator_->Alloc(size, align);
        if (res != nullptr) {
            break;
        }
        LOG_INTERNAL_ALLOCATOR(DEBUG) << "FreeListAllocator didn't allocate memory, try to add new pool";
        size_t poolSize = FreeListAllocatorT::GetMinPoolSize();
        auto pool = PoolManager::GetMmapMemPool()->AllocPool(poolSize, SpaceType::SPACE_TYPE_INTERNAL,
                                                             AllocatorType::FREELIST_ALLOCATOR, freelistAllocator_);
        if (UNLIKELY(pool.GetMem() == nullptr)) {
            return nullptr;
        }
        freelistAllocator_->AddMemoryPool(pool.GetMem(), pool.GetSize());
    }
    return res;
}

template <InternalAllocatorConfig CONFIG>
void *InternalAllocator<CONFIG>::AllocViaHumongousAllocator(size_t size, Alignment align)
{
    LOG_INTERNAL_ALLOCATOR(DEBUG) << "Try to use HumongousObjAllocator";
    void *res = humongousAllocator_->Alloc(size, align);
    if (res != nullptr) {
        return res;
    }
    // Get rid of extra pool adding to the allocator
    static os::memory::Mutex poolLock;
    os::memory::LockHolder lock(poolLock);
    while (true) {
        res = humongousAllocator_->Alloc(size, align);
        if (res != nullptr) {
            break;
        }
        LOG_INTERNAL_ALLOCATOR(DEBUG) << "HumongousObjAllocator didn't allocate memory, try to add new pool";
        size_t poolSize = HumongousObjAllocatorT::GetMinPoolSize(size);
        auto pool = PoolManager::GetMmapMemPool()->AllocPool(poolSize, SpaceType::SPACE_TYPE_INTERNAL,
                                                             AllocatorType::HUMONGOUS_ALLOCATOR, humongousAllocator_);
        if (UNLIKELY(pool.GetMem() == nullptr)) {
            return nullptr;
        }
        humongousAllocator_->AddMemoryPool(pool.GetMem(), pool.GetSize());
    }
    return res;
}

template <InternalAllocatorConfig CONFIG>
template <AllocScope ALLOC_SCOPE_T>
void *InternalAllocator<CONFIG>::AllocViaPandaAllocators(size_t size, Alignment align)
{
    void *res = nullptr;
    size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
    static_assert(RunSlotsAllocatorT::GetMaxSize() == LocalSmallObjectAllocator::GetMaxSize());
    if (LIKELY(alignedSize <= RunSlotsAllocatorT::GetMaxSize())) {
        res = this->AllocViaRunSlotsAllocator<ALLOC_SCOPE_T>(size, align);
    } else if (alignedSize <= FreeListAllocatorT::GetMaxSize()) {
        res = this->AllocViaFreeListAllocator(size, align);
    } else {
        res = this->AllocViaHumongousAllocator(size, align);
    }
    return res;
}

template <InternalAllocatorConfig CONFIG>
void InternalAllocator<CONFIG>::FreeViaPandaAllocators(void *ptr)
{
    AllocatorType allocType = PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(ptr).GetType();
    switch (allocType) {
        case AllocatorType::RUNSLOTS_ALLOCATOR:
            if (PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(ptr).GetAllocatorHeaderAddr() ==
                runslotsAllocator_) {
                LOG_INTERNAL_ALLOCATOR(DEBUG) << "free via RunSlotsAllocator";
                runslotsAllocator_->Free(ptr);
            } else {
                LOG_INTERNAL_ALLOCATOR(DEBUG) << "free via thread-local RunSlotsAllocator";
                // It is a thread-local internal allocator instance
                LocalSmallObjectAllocator *localAllocator =
                    ark::ManagedThread::GetCurrent()->GetLocalInternalAllocator();
                ASSERT(PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(ptr).GetAllocatorHeaderAddr() ==
                       localAllocator);
                localAllocator->Free(ptr);
            }
            break;
        case AllocatorType::FREELIST_ALLOCATOR:
            LOG_INTERNAL_ALLOCATOR(DEBUG) << "free via FreeListAllocator";
            ASSERT(PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(ptr).GetAllocatorHeaderAddr() ==
                   freelistAllocator_);
            freelistAllocator_->Free(ptr);
            break;
        case AllocatorType::HUMONGOUS_ALLOCATOR:
            LOG_INTERNAL_ALLOCATOR(DEBUG) << "free via HumongousObjAllocator";
            ASSERT(PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(ptr).GetAllocatorHeaderAddr() ==
                   humongousAllocator_);
            humongousAllocator_->Free(ptr);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

/* static */
template <InternalAllocatorConfig CONFIG>
typename InternalAllocator<CONFIG>::LocalSmallObjectAllocator *InternalAllocator<CONFIG>::SetUpLocalInternalAllocator(
    Allocator *allocator)
{
    (void)allocator;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        auto localAllocator =
            allocator->New<LocalSmallObjectAllocator>(allocator->GetMemStats(), SpaceType::SPACE_TYPE_INTERNAL);
        LOG_INTERNAL_ALLOCATOR(DEBUG) << "Set up local internal allocator at addr " << localAllocator
                                      << " for the thread " << ark::Thread::GetCurrent();
        return localAllocator;
    }
    return nullptr;
}

/* static */
template <InternalAllocatorConfig CONFIG>
void InternalAllocator<CONFIG>::FinalizeLocalInternalAllocator(
    InternalAllocator::LocalSmallObjectAllocator *localAllocator, Allocator *allocator)
{
    (void)localAllocator;
    (void)allocator;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        localAllocator->VisitAndRemoveAllPools(
            [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
        allocator->Delete(localAllocator);
    }
}

/* static */
template <InternalAllocatorConfig CONFIG>
void InternalAllocator<CONFIG>::RemoveFreePoolsForLocalInternalAllocator(LocalSmallObjectAllocator *localAllocator)
{
    (void)localAllocator;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        localAllocator->VisitAndRemoveFreePools(
            [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    }
}

template <InternalAllocatorConfig CONFIG>
void InternalAllocator<CONFIG>::InitInternalAllocatorFromRuntime(Allocator *allocator)
{
    ASSERT(allocatorFromRuntime_ == nullptr);
    allocatorFromRuntime_ = allocator;
}

template <InternalAllocatorConfig CONFIG>
Allocator *InternalAllocator<CONFIG>::GetInternalAllocatorFromRuntime()
{
    return allocatorFromRuntime_;
}

template <InternalAllocatorConfig CONFIG>
void InternalAllocator<CONFIG>::ClearInternalAllocatorFromRuntime()
{
    allocatorFromRuntime_ = nullptr;
}

template class InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>;
template class InternalAllocator<InternalAllocatorConfig::MALLOC_ALLOCATOR>;
template void *InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>::Alloc<AllocScope::GLOBAL>(size_t,
                                                                                                       Alignment);
template void *InternalAllocator<InternalAllocatorConfig::PANDA_ALLOCATORS>::Alloc<AllocScope::LOCAL>(size_t,
                                                                                                      Alignment);
template void *InternalAllocator<InternalAllocatorConfig::MALLOC_ALLOCATOR>::Alloc<AllocScope::GLOBAL>(size_t,
                                                                                                       Alignment);
template void *InternalAllocator<InternalAllocatorConfig::MALLOC_ALLOCATOR>::Alloc<AllocScope::LOCAL>(size_t,
                                                                                                      Alignment);

#undef LOG_INTERNAL_ALLOCATOR

}  // namespace ark::mem

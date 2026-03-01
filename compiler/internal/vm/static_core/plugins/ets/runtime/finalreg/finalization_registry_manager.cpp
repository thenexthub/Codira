/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"
#include "runtime/coroutines/coroutine_worker_group.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"

namespace ark::ets {
void FinalizationRegistryManager::CleanupCoroFinished()
{
    // Atomic with acq_rel order reason: other threads should see correct value
    [[maybe_unused]] uint32_t oldCnt = finRegCleanupCoroCount_.fetch_sub(1, std::memory_order_acq_rel);
    ASSERT(oldCnt > 0);
}

CoroutineWorkerDomain FinalizationRegistryManager::GetCoroDomain(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    CoroutineWorker *worker = coro->GetWorker();
    if (worker->IsMainWorker()) {
        return CoroutineWorkerDomain::MAIN;
    } else if (worker->InExclusiveMode()) {
        return CoroutineWorkerDomain::EA;
    }
    return CoroutineWorkerDomain::GENERAL;
}

void FinalizationRegistryManager::Enqueue(EtsFinalizationRegistry *finReg)
{
    auto workerId = finReg->GetWorkerId();
    auto workerDomain = finReg->GetWorkerDomain();
    auto pred = [workerId, workerDomain](const EtsFinalizationRegistry *fr) {
        if (workerDomain != fr->GetWorkerDomain()) {
            return false;
        }
        if (workerDomain == CoroutineWorkerDomain::EA) {
            return workerId == fr->GetWorkerId();
        }
        return true;
    };
    auto it = std::find_if(finalizationList_.begin(), finalizationList_.end(), pred);
    if (it == finalizationList_.end()) {
        finalizationList_.push_back(finReg);
    } else {
        EtsFinalizationRegistry *head = *it;
        finReg->SetNextFinReg(head);
        *it = finReg;
    }
}

void FinalizationRegistryManager::UpdateAndSweep(const ReferenceUpdater &updater)
{
    auto it = finalizationList_.begin();
    while (it != finalizationList_.end()) {
        EtsFinalizationRegistry *head = UpdateAndSweepFinRegChain(*it, updater);
        if (head == nullptr) {
            finalizationList_.erase(it++);
        } else {
            *it = head;
            ++it;
        }
    }
}

EtsFinalizationRegistry *FinalizationRegistryManager::UpdateAndSweepFinRegChain(EtsFinalizationRegistry *cur,
                                                                                const ReferenceUpdater &updater)
{
    EtsFinalizationRegistry *head = nullptr;
    EtsFinalizationRegistry *prev = nullptr;
    while (cur != nullptr) {
        auto *next = cur->GetNextFinReg();
        ObjectHeader *obj = cur->GetCoreType();
        if (updater(&obj) == ObjectStatus::ALIVE_OBJECT) {
            cur = EtsFinalizationRegistry::FromCoreType(obj);
            if (head == nullptr) {
                head = cur;
            } else {
                prev->SetNextFinReg(cur);
            }
            prev = cur;
        }
        cur = next;
    }
    return head;
}

bool FinalizationRegistryManager::CanCleanup(CoroutineWorker::Id currentId, CoroutineWorkerDomain currentDomain,
                                             CoroutineWorker::Id id, CoroutineWorkerDomain domain)
{
    if (domain == CoroutineWorkerDomain::MAIN) {
        return currentDomain == CoroutineWorkerDomain::MAIN;
    } else if (domain == CoroutineWorkerDomain::GENERAL) {
        return currentDomain == CoroutineWorkerDomain::MAIN || currentDomain == CoroutineWorkerDomain::GENERAL;
    } else if (domain == CoroutineWorkerDomain::EA) {
        return (currentDomain == CoroutineWorkerDomain::EA) && (currentId == id);
    }
    return false;
}

bool FinalizationRegistryManager::UpdateFinRegCoroCountAndCheckIfCleanupNeeded()
{
    // Atomic with acquire order reason: getting correct value
    uint32_t cnt = finRegCleanupCoroCount_.load(std::memory_order_acquire);
    uint32_t oldCnt = cnt;
    // Atomic with acq_rel order reason: sync for counter
    while (cnt < MAX_FINREG_CLEANUP_COROS && !finRegCleanupCoroCount_.compare_exchange_weak(
                                                 // CC-OFFNXT(G.FMT.06-CPP) project code style
                                                 cnt, cnt + 1U, std::memory_order_acq_rel, std::memory_order_acquire)) {
        oldCnt = cnt;
    }
    return oldCnt < MAX_FINREG_CLEANUP_COROS;
}

static CoroutineWorkerGroup::Id GetGroupId(CoroutineManager *coroManager, CoroutineWorkerDomain domain,
                                           CoroutineWorker::Id id)
{
    if (domain == CoroutineWorkerDomain::MAIN) {
        return CoroutineWorkerGroup::FromDomain(coroManager, CoroutineWorkerDomain::MAIN);
    } else if (domain == CoroutineWorkerDomain::EA) {
        return CoroutineWorkerGroup::FromDomain(coroManager, CoroutineWorkerDomain::EA, {id});
    }
    return CoroutineWorkerGroup::FromDomain(coroManager, CoroutineWorkerDomain::GENERAL);
}

static std::pair<EtsFinRegNode *, EtsFinRegNode *> CombineLists(EtsFinalizationRegistry *finReg)
{
    EtsFinRegNode *head = finReg->GetFinalizationQueueHead();
    EtsFinRegNode *tail = finReg->GetFinalizationQueueTail();
    finReg->SetFinalizationQueueHead(nullptr);
    finReg->SetFinalizationQueueTail(nullptr);
    auto *nextFinReg = finReg->GetNextFinReg();
    finReg->SetNextFinReg(nullptr);
    finReg = nextFinReg;
    while (finReg != nullptr) {
        EtsFinRegNode *newHead = finReg->GetFinalizationQueueHead();
        EtsFinRegNode *newTail = finReg->GetFinalizationQueueTail();
        finReg->SetFinalizationQueueHead(nullptr);
        finReg->SetFinalizationQueueTail(nullptr);
        tail->SetNext(newHead);
        tail = newTail;
        nextFinReg = finReg->GetNextFinReg();
        finReg->SetNextFinReg(nullptr);
        finReg = nextFinReg;
    }
    return std::make_pair(head, tail);
}

void FinalizationRegistryManager::StartCleanupCoroIfNeeded(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    auto *coroManager = coro->GetCoroutineManager();
    auto currentWorkerId = coro->GetWorker()->GetId();
    auto currentWorkerDomain = GetCoroDomain(coro);
    // Check are there node that can be finalized by the current worker
    auto pred = [currentWorkerId, currentWorkerDomain](const EtsFinalizationRegistry *finReg) {
        return CanCleanup(currentWorkerId, currentWorkerDomain, finReg->GetWorkerId(), finReg->GetWorkerDomain());
    };
    auto it = std::find_if(finalizationList_.begin(), finalizationList_.end(), pred);
    if (it == finalizationList_.end()) {
        return;
    }
    auto [head, tail] = CombineLists(*it);
    auto toDelete = it;
    ++it;
    finalizationList_.erase(toDelete);
    it = std::find_if(
        it, finalizationList_.end(), [currentWorkerId, currentWorkerDomain](const EtsFinalizationRegistry *finReg) {
            return CanCleanup(currentWorkerId, currentWorkerDomain, finReg->GetWorkerId(), finReg->GetWorkerDomain());
        });
    if (it != finalizationList_.end()) {
        auto [head2, tail2] = CombineLists(*it);
        finalizationList_.erase(it);
        tail->SetNext(head2);
        tail = tail2;
    }

    if (UpdateFinRegCoroCountAndCheckIfCleanupNeeded()) {
        ASSERT(head != nullptr);
        auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, coroManager);
        Method *cleanup = PlatformTypes(vm_)->coreFinalizationRegistryExecCleanup->GetPandaMethod();
        auto workerId = coro->GetWorker()->GetId();
        auto workerDomain = GetCoroDomain(coro);
        auto groupId = GetGroupId(coroManager, workerDomain, workerId);
        auto args = PandaVector<Value> {Value(EtsObject::ToCoreType(head))};
        auto launchResult =
            coroManager->Launch(event, cleanup, std::move(args), groupId, CoroutinePriority::DEFAULT_PRIORITY, false);
        if (UNLIKELY(launchResult == LaunchResult::COROUTINES_LIMIT_EXCEED)) {
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(event);
        }
    }
}
}  // namespace ark::ets

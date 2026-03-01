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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FINALREG_FINALIZATION_REGISTRY_H
#define PANDA_PLUGINS_ETS_RUNTIME_FINALREG_FINALIZATION_REGISTRY_H

#include "runtime/mem/gc/gc.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/mem/root_provider.h"
#include "plugins/ets/runtime/types/ets_finalization_registry.h"

namespace ark::ets {

class PandaEtsVM;

/**
 * The class that manages all created FinalizationRegistry objects
 * and its finalization process together with EtsReferenceProcessor.
 *
 * Each FinalizationRegistry instance has a finalization queue
 * (fields finalizationQueueHead and finalizationQueueTail).
 *
 * When EtsReferenceProcessor clears a FinRegNode (which inherits from WeakRef)
 * it adds the FinRegNode to finalization queue of the same FinalizationRegistry instance.
 * Also reference processor adds the FinalizationRegistry instance to the linked list in
 * FinalizationRegistryManager if it is not in it (the field finalizationList_).
 * This list is organized as a list of lists of FinalizationRegistry instances.
 * Each FinalizationRegistry has a list of FinRegNode which requires finalization.
 *
 * +-------------+    +----------------+    +-----------------+    +-----------------+
 * | domain:main | -> | domain:general | -> | domain:ea, id:5 | -> | domain:ea, id:6 |
 * +-------------+    +----------------+    +-----------------+    +-----------------+
 *        |                   |                      |                      |
 *        V                   V                      V                      V
 *    +--------+          +--------+             +--------+             +--------+
 *    | FinReg |          | FinReg |             | FinReg |             | FinReg |
 *    +--------+          +--------+             +--------+             +--------+
 *        |
 *        V
 *    +--------+    +------------+    +------------+
 *    | FinReg | -> | FinRegNode | -> | FinRegNode |
 *    +--------+    +------------+    +------------+
 * finalizationList_ is not a GC root but GC updates its references and sweeps dead instancies.
 * After GC finished in a managed thread it asks FinalizationRegistryManager to start a clenup coroutine.
 * The manager finds the corresponding FinalizationRegistry list (there are may be several lists) and combine
 * all finalization queues into a single list. This list is passed as an argument to the clenaup coroutine.
 */
class FinalizationRegistryManager final {
public:
    explicit FinalizationRegistryManager(PandaEtsVM *vm) : vm_(vm) {}
    NO_COPY_SEMANTIC(FinalizationRegistryManager);
    NO_MOVE_SEMANTIC(FinalizationRegistryManager);
    ~FinalizationRegistryManager() = default;

    /**
     * @brief Checks the number of called coroutines with cleanup and calls the next one
     * if the number does not exceed MAX_FINREG_CLEANUP_COROS.
     * @param coro Pointer to current coroutine
     */
    void StartCleanupCoroIfNeeded(EtsCoroutine *coro);

    /// @brief Decreases the counter of called cleanup coroutines
    void CleanupCoroFinished();

    static CoroutineWorkerDomain GetCoroDomain(EtsCoroutine *coro);

    void Enqueue(EtsFinalizationRegistry *finReg);

    void UpdateAndSweep(const ReferenceUpdater &updater);

    static bool CanCleanup(CoroutineWorker::Id currentId, CoroutineWorkerDomain currentDomain, CoroutineWorker::Id id,
                           CoroutineWorkerDomain domain);

private:
    /// @brief Increase number of cleanup coroutines and check if not exceeds limit
    bool UpdateFinRegCoroCountAndCheckIfCleanupNeeded();
    EtsFinalizationRegistry *UpdateAndSweepFinRegChain(EtsFinalizationRegistry *head, const ReferenceUpdater &updater);

    // Limit of cleanup coroutines count
    static constexpr uint32_t MAX_FINREG_CLEANUP_COROS = 3;

    PandaEtsVM *vm_ {nullptr};
    std::atomic<uint32_t> finRegCleanupCoroCount_ {0};
    PandaList<EtsFinalizationRegistry *> finalizationList_;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FINALREG_FINALIZATION_REGISTRY_H

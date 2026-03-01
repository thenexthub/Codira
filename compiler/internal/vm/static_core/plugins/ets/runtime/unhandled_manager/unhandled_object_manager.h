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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_UNHANDLED_OBJECT_MANAGER_H
#define PANDA_PLUGINS_ETS_RUNTIME_UNHANDLED_OBJECT_MANAGER_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_handle.h"

namespace ark::ets {

class PandaEtsVM;
class EtsJob;
class EtsPromise;

/// @brief The class manages all unhandled rejected objects
class UnhandledObjectManager final {
public:
    explicit UnhandledObjectManager(PandaEtsVM *vm) : vm_(vm) {}
    NO_COPY_SEMANTIC(UnhandledObjectManager);
    NO_MOVE_SEMANTIC(UnhandledObjectManager);
    ~UnhandledObjectManager() = default;

    /// @brief Adds failed job to an internal storage
    void AddFailedJob(EtsJob *job);

    /// @brief Removes failed job from internal storage
    void RemoveFailedJob(EtsJob *job);

    /// @brief Invokes managed method to apply custom handler on stored failed jobs
    void ListFailedJobs(EtsCoroutine *coro);

    /// @brief Adds rejected promise to an internal storage
    void AddRejectedPromise(EtsPromise *promise, EtsCoroutine *adderCoro);

    /// @brief Removes rejected promise from internal storage
    void RemoveRejectedPromise(EtsPromise *promise, EtsCoroutine *removerCoro);

    /**
     * Invokes managed method to apply custom handler on stored rejected promises
     * @param coro - current coroutine
     * @param listAllObjects - if false, list objects associated with current worker
     */
    void ListRejectedPromises(EtsCoroutine *coro, bool listAllObjects);

    /// @brief Checks if manager has unhandled failed job objects captured
    bool HasFailedJobObjects() const;

    /**
     * Checks if manager has unhandled rejected promise objects captured
     * @param coro - current coroutine
     * @param listAllObjects - if false, checks for the current worker
     */
    bool HasRejectedPromiseObjects(EtsCoroutine *coro, bool listAllObjects) const;

    /// @brief Updates references to objects moved by GC
    void UpdateObjects(const GCRootUpdater &gcRootUpdater);

    /// @brief Visits objects as GC roots
    void VisitObjects(const GCRootVisitor &visitor);

    /// @brief Applies handler to the exception and exits the program
    void InvokeErrorHandler(EtsCoroutine *coro, EtsHandle<EtsObject> exception);

private:
    PandaUnorderedSet<EtsObject *> failedJobs_ GUARDED_BY(mutex_);
    PandaUnorderedMap<CoroutineWorker::Id, PandaUnorderedSet<EtsObject *>> rejectedPromises_ GUARDED_BY(mutex_);
    // NOTE: The current implementation doesn't handle the case where a promise is
    // rejected in one worker and handled in another. This is considered undefined behavior.

    mutable os::memory::Mutex mutex_;
    PandaEtsVM *vm_;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_UNHANDLED_OBJECT_MANAGER_H

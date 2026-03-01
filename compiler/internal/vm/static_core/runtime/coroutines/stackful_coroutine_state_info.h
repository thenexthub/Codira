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

#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_STATE_INFO_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_STATE_INFO_H

#include <cstddef>
#include "coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_context.h"
#include "libarkbase/macros.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/stack_walker.h"

namespace ark {

class StackfulCoroutineWorkerStateInfo;
class StackfulCoroutineStateInfo;
class StackfulCoroutineWorker;
class CoroutineManager;

class StackfulCoroutineStateInfoTable {
public:
    using WorkerStateFilter = std::function<bool(const StackfulCoroutineWorkerStateInfo *)>;
    using CoroutineStateFilter = std::function<bool(const StackfulCoroutineStateInfo *)>;

    StackfulCoroutineStateInfoTable() = default;
    DEFAULT_COPY_SEMANTIC(StackfulCoroutineStateInfoTable);
    DEFAULT_MOVE_SEMANTIC(StackfulCoroutineStateInfoTable);
    ~StackfulCoroutineStateInfoTable() = default;

    void AddWorker(StackfulCoroutineWorker *worker);

    PandaString OutputInfo(const WorkerStateFilter &workerFilter,
                           const CoroutineStateFilter &coroFilter = nullptr) const;
    PandaString OutputInfo(const CoroutineStateFilter &coroFilter) const;
    PandaString OutputInfo(std::nullptr_t coroFilter = nullptr) const;

private:
    PandaVector<StackfulCoroutineWorkerStateInfo> workersInfo_;
};

class StackfulCoroutineWorkerStateInfo {
public:
    using CoroutineStateFilter = StackfulCoroutineStateInfoTable::CoroutineStateFilter;

    explicit StackfulCoroutineWorkerStateInfo(StackfulCoroutineWorker *worker);
    DEFAULT_COPY_SEMANTIC(StackfulCoroutineWorkerStateInfo);
    DEFAULT_MOVE_SEMANTIC(StackfulCoroutineWorkerStateInfo);
    ~StackfulCoroutineWorkerStateInfo() = default;

    void AddCoroutine(Coroutine *co);

    const PandaString &GetWorkerName() const
    {
        return workerName_;
    }

    bool IsMainWorker() const
    {
        return isMainWorker_;
    }

    PandaString OutputInfo(const CoroutineStateFilter &filter = nullptr) const;

private:
    PandaVector<StackfulCoroutineStateInfo> coroutinesInfo_;
    PandaString workerName_;
    bool isMainWorker_;
};

class StackfulCoroutineStateInfo {
public:
    explicit StackfulCoroutineStateInfo(Coroutine *co);
    DEFAULT_COPY_SEMANTIC(StackfulCoroutineStateInfo);
    DEFAULT_MOVE_SEMANTIC(StackfulCoroutineStateInfo);
    ~StackfulCoroutineStateInfo() = default;

    /**
     * @brief Method creates a stackwalker because count of methods works only with rvalue
     * @see StackWalker::Dump(...)
     */
    StackWalker GetStackWalker() const
    {
        return StackWalker::Create(this->coro_);
    }

    const PandaString &GetCoroutineName() const
    {
        return this->coroutineName_;
    }

    Coroutine::Status GetCorutineStatus() const
    {
        return this->coroutineStatus_;
    }

    PandaString OutputInfo() const;

private:
    Coroutine *coro_;
    PandaString coroutineName_;
    Coroutine::Status coroutineStatus_;
    Coroutine::Type coroutineType_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_STATE_INFO_H

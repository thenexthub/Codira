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

#include "runtime/coroutines/stackful_coroutine_state_info.h"
#include <cstddef>
#include "runtime/include/mem/panda_string.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"
#include "runtime/coroutines/coroutine_context.h"

namespace ark {

void StackfulCoroutineStateInfoTable::AddWorker(StackfulCoroutineWorker *worker)
{
    workersInfo_.emplace_back(worker);
}

void StackfulCoroutineWorkerStateInfo::AddCoroutine(Coroutine *co)
{
    coroutinesInfo_.emplace_back(co);
}

StackfulCoroutineWorkerStateInfo::StackfulCoroutineWorkerStateInfo(StackfulCoroutineWorker *worker)
    : workerName_(worker->GetName()), isMainWorker_(worker->IsMainWorker())
{
    worker->GetFullWorkerStateInfo(this);
}

StackfulCoroutineStateInfo::StackfulCoroutineStateInfo(Coroutine *co)
    : coro_(co),
      coroutineName_(co->GetName()),
      coroutineStatus_(co->GetCoroutineStatus()),
      coroutineType_(co->GetType())
{
}

PandaString StackfulCoroutineStateInfo::OutputInfo() const
{
    PandaStringStream sstream;
    // Line with Coroutine Name
    sstream << "Coroutine: " << this->coroutineName_ << "\n";
    // Line with Coroutine Status
    sstream << "Status: " << this->coroutineStatus_ << "\n";
    // Line with Coroutine Type
    sstream << "Type: " << this->coroutineType_ << "\n";
    // Stack Trace
    this->GetStackWalker().Dump(sstream);

    return sstream.str();
}

PandaString StackfulCoroutineWorkerStateInfo::OutputInfo(const CoroutineStateFilter &filter) const
{
    PandaStringStream sstream;

    sstream << "Worker: " << this->workerName_ << "\n";
    sstream << "IsMain: " << (this->isMainWorker_ ? "Yes" : "No") << "\n";

    sstream << "----------\n";
    if (this->coroutinesInfo_.empty()) {
        sstream << "No coroutines on this worker\n";
        sstream << "----------\n";
        return sstream.str();
    }

    size_t coroCount = 0;
    for (auto &coroInfo : this->coroutinesInfo_) {
        if (filter != nullptr && !filter(&coroInfo)) {
            continue;
        }
        coroCount++;
        sstream << coroInfo.OutputInfo();
        sstream << "----------\n";
    }
    if (coroCount == 0) {
        sstream << "No coroutines would pass filter\n";
        sstream << "----------\n";
    }

    return sstream.str();
}
PandaString StackfulCoroutineStateInfoTable::OutputInfo(const WorkerStateFilter &workerFilter,
                                                        const CoroutineStateFilter &coroFilter) const
{
    PandaStringStream sstream;

    sstream << "==========\n";
    sstream << "StackfullCoroutineStateInfoTable\n";
    sstream << "==========\n";

    if (this->workersInfo_.empty()) {
        sstream << "No workers\n";
        sstream << "==========\n";
        return sstream.str();
    }

    size_t workerCount = 0;
    for (const auto &workerInfo : workersInfo_) {
        if (workerFilter != nullptr && workerFilter(&workerInfo)) {
            continue;
        }
        workerCount++;
        sstream << workerInfo.OutputInfo(coroFilter);
        sstream << "==========\n";
    }
    if (workerCount == 0) {
        sstream << "No workeres would pass filter\n";
        sstream << "==========\n";
    }

    return sstream.str();
}

PandaString StackfulCoroutineStateInfoTable::OutputInfo(const CoroutineStateFilter &coroFilter) const
{
    return OutputInfo(nullptr, coroFilter);
}

PandaString StackfulCoroutineStateInfoTable::OutputInfo(std::nullptr_t) const
{
    return OutputInfo(nullptr, nullptr);
}

}  // namespace ark

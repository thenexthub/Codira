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

#include "ArkScheduler.h"
#include "common/BasicHelper.h"

namespace ark {
    ArkScheduler::ArkScheduler(Callbacks *c) : barrier(1), workerThreads(new AsyncTaskRunner()), callback(c),
                                               worker(ArkASTWorker::Create(*workerThreads, barrier, callback)) {}

ArkScheduler::~ArkScheduler() noexcept
{
    if (worker) {
        worker->Stop();
    }
    if (workerThreads) {
        delete workerThreads;
        workerThreads = nullptr;
    }
}

void ArkScheduler::Update(const ParseInputs &inputs, NeedDiagnostics needDiag) const
{
    worker->Update(inputs, needDiag);
}

void ArkScheduler::RunWithAST(const std::string &name, const std::string &file,
                              std::function<void(InputsAndAST)> action) const
{
    worker->RunWithAST(name, file, std::move(action), NeedDiagnostics::YES);
}

void ArkScheduler::RunWithASTCache(
    const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action) const
{
    worker->RunWithASTCache(name, file, pos, std::move(action));
}
} // namespace ark

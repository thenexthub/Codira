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

#ifndef LSPSERVER_ARKSCHEDULER_H
#define LSPSERVER_ARKSCHEDULER_H

#include <map>
#include "ArkThreading.h"
#include "ArkASTWorker.h"

namespace ark {
class ArkScheduler {
public:
    explicit ArkScheduler(Callbacks *c);
    ~ArkScheduler() noexcept;

    void Update(const ParseInputs &inputs, NeedDiagnostics needDiag) const;

    void RunWithAST(const std::string &name, const std::string &file, std::function<void(InputsAndAST)> action) const;

    void RunWithASTCache(
        const std::string &name, const std::string &file, Position pos, std::function<void(InputsAndAST)> action) const;

private:
    Semaphore barrier;
    AsyncTaskRunner *workerThreads = nullptr;
    Callbacks *callback = nullptr;

    // worker will be freed after polling thread exit
    // we call worker->stop to free worker
    ArkASTWorker *worker = nullptr;
};
} // namespace ark

#endif // LSPSERVER_ARKSCHEDULER_H

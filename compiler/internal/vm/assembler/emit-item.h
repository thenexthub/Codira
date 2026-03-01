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

#ifndef ASSEMBLER_EMIT_ITEM_H
#define ASSEMBLER_EMIT_ITEM_H

#include <mutex>

#include "assembly-emitter.h"
#include "libpandabase/utils/workerQueue.h"

namespace panda::pandasm {
class EmitFunctionsQueue : public panda::WorkerQueue {
public:
    explicit EmitFunctionsQueue(size_t threadCount, panda_file::ItemContainer &items,
                                const std::vector<Program *> &progs,
                                AsmEmitter::AsmEntityCollections &entities, bool emit_debug_info)
        : WorkerQueue(threadCount),
          items_(items),
          progs_(progs),
          entities_(entities),
          emit_debug_info_(emit_debug_info) {}

    NO_COPY_SEMANTIC(EmitFunctionsQueue);
    NO_MOVE_SEMANTIC(EmitFunctionsQueue);
    ~EmitFunctionsQueue() override = default;

    void Schedule() override;
private:
    panda_file::ItemContainer &items_;
    const std::vector<Program *> &progs_;
    AsmEmitter::AsmEntityCollections &entities_;
    bool emit_debug_info_;
};

class EmitFunctionsJob : public panda::WorkerJob {
public:
    explicit EmitFunctionsJob(panda_file::ItemContainer &items, const Program &program, bool emit_debug_info,
                              AsmEmitter::AsmEntityCollections &entities)
        : items_(items),
          program_(program),
          emit_debug_info_(emit_debug_info),
          entities_(entities) {};
    NO_COPY_SEMANTIC(EmitFunctionsJob);
    NO_MOVE_SEMANTIC(EmitFunctionsJob);
    ~EmitFunctionsJob() override = default;

    bool Run() override;
    bool EmitFunctions(const AsmEmitter::AsmEntityCollections &entities, bool emit_debug_info);
    void EmitDebugInfo(const std::vector<uint8_t> *bytes, const panda_file::MethodItem *method,
        const Function &func, const std::string &name, bool emit_debug_info);
private:
    static std::mutex mutex_;
    panda_file::ItemContainer &items_;
    const Program &program_;
    bool emit_debug_info_;
    AsmEmitter::AsmEntityCollections &entities_;
};
}
#endif  // ASSEMBLER_EMIT_ITEM_H
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

#ifndef COMPILER_COMPILE_METHOD_H
#define COMPILER_COMPILE_METHOD_H

#include "compiler_options.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/code_allocator.h"
#include "include/method.h"
#include "libarkbase/utils/arch.h"
#include "compiler_task_runner.h"

namespace ark::compiler {
class Graph;
class RuntimeInterface;

class JITStats {
public:
    explicit JITStats(mem::InternalAllocatorPtr internalAllocator)
        : internalAllocator_(internalAllocator), statsList_(internalAllocator->Adapter())
    {
    }
    NO_MOVE_SEMANTIC(JITStats);
    NO_COPY_SEMANTIC(JITStats);
    ~JITStats()
    {
        DumpCsv();
    }
    void SetCompilationStart();
    void EndCompilationWithStats(const std::string &methodName, bool isOsr, size_t bcSize, size_t codeSize);
    void ResetCompilationStart();

private:
    void DumpCsv(char sep = ',');
    struct Entry {
        PandaString methodName;
        bool isOsr;
        size_t bcSize;
        size_t codeSize;
        uint64_t time;
    };

private:
    mem::InternalAllocatorPtr internalAllocator_;
    uint64_t startTime_ {};
    std::vector<Entry, typename mem::AllocatorAdapter<Entry>> statsList_;
};

Arch ChooseArch(Arch arch);

// @tparam RUNNER_MODE=BACKGROUND_MODE means that compilation of method
// is divided into tasks for TaskManager and occurs in its threads.
// Otherwise compilation occurs in-place.
template <TaskRunnerMode RUNNER_MODE>
void JITCompileMethod(RuntimeInterface *runtime, CodeAllocator *codeAllocator, ArenaAllocator *gdbDebugInfoAllocator,
                      JITStats *jitStats, CompilerTaskRunner<RUNNER_MODE> taskRunner);
template <TaskRunnerMode RUNNER_MODE>
void CompileInGraph(RuntimeInterface *runtime, bool isDynamic, Arch arch, CompilerTaskRunner<RUNNER_MODE> taskRunner,
                    JITStats *jitStats = nullptr);
bool CheckMethodInLists(const std::string &methodName);
}  // namespace ark::compiler

#endif  // COMPILER_COMPILE_METHOD_H

/*
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

#ifndef COMPILER_COMPILER_RUN_H
#define COMPILER_COMPILER_RUN_H

#include "optimizer/pipeline.h"
#include "inplace_task_runner.h"
#include "background_task_runner.h"
#include "compiler_task_runner.h"

namespace ark::compiler {
class Graph;

template <TaskRunnerMode RUNNER_MODE>
inline void RunOptimizations(CompilerTaskRunner<RUNNER_MODE> taskRunner)
{
    auto &taskCtx = taskRunner.GetContext();
    auto pipeline = Pipeline::Create(taskCtx.GetGraph());
    if constexpr (RUNNER_MODE == BACKGROUND_MODE) {
        taskCtx.SetPipeline(std::move(pipeline));
    } else {
        taskCtx.SetPipeline(pipeline.get());
    }
    Pipeline::Run<RUNNER_MODE>(std::move(taskRunner));
}

}  // namespace ark::compiler
#endif  // COMPILER_COMPILER_RUN_H

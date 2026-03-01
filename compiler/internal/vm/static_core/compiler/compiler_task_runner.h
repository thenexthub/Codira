/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef COMPILER_COMPILER_TASK_RUNNER_H
#define COMPILER_COMPILER_TASK_RUNNER_H

#include <type_traits>

namespace ark::compiler {
class InPlaceCompilerContext;
class BackgroundCompilerContext;
class InPlaceCompilerTaskRunner;
class BackgroundCompilerTaskRunner;

enum TaskRunnerMode { INPLACE_MODE, BACKGROUND_MODE };

template <TaskRunnerMode RUNNER_MODE>
using CompilerContext =
    std::conditional_t<RUNNER_MODE == BACKGROUND_MODE, BackgroundCompilerContext, InPlaceCompilerContext>;

template <TaskRunnerMode RUNNER_MODE>
using CompilerTaskRunner =
    std::conditional_t<RUNNER_MODE == BACKGROUND_MODE, BackgroundCompilerTaskRunner, InPlaceCompilerTaskRunner>;

}  // namespace ark::compiler

#endif  // COMPILER_COMPILER_TASK_RUNNER_H

/**
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
#ifndef PANDA_DEOPTIMIZATION_H
#define PANDA_DEOPTIMIZATION_H

#include "runtime/include/exceptions.h"
#include "runtime/include/stack_walker.h"

namespace ark {

/**
 * @brief Deoptimize compiled frame
 * @param stack Frame to be deoptimized. Must be CFrame (compiled frame).
 * @param pc    PC from which interpreter starts execution, if nullptr, pc is got from deoptimized CFrame.
 * @param has_exception we use exception from current thread
 */
[[noreturn]] void Deoptimize(StackWalker *stack, const uint8_t *pc, bool hasException = false,
                             Method *destroyMethod = nullptr);

/**
 * @brief Drop given CFrame and return to its caller.
 * @param stack Frame to be dropped. Must be CFrame.
 */
[[noreturn]] void DropCompiledFrame(StackWalker *stack);

void InvalidateCompiledEntryPoint(const PandaSet<Method *> &methods, bool isCha);

}  // namespace ark

#endif  // PANDA_DEOPTIMIZATION_H

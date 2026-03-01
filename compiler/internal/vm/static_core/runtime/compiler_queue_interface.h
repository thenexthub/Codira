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
#ifndef PANDA_RUNTIME_COMPILER_QUEUE_H_
#define PANDA_RUNTIME_COMPILER_QUEUE_H_

#include "runtime/include/compiler_interface-inl.h"
#include "runtime/include/method-inl.h"
#include "runtime/thread_pool_queue.h"

namespace ark {

class Method;

class CompilerQueueInterface : public TaskQueueInterface<CompilerTask> {
protected:
    PandaString GetTaskDescription(const CompilerTask &ctx)
    {
        PandaOStringStream stream;
        // Empty ctx element may only returned if the queue is empty, it must not be stored into the queue
        ASSERT(ctx.GetMethod() != nullptr);
        auto name = reinterpret_cast<const char *>(ctx.GetMethod()->GetName().data);
        stream << "(method: " << name << ", is_osr: " << std::dec << ctx.IsOsr()
               << ", hotness counter: " << ctx.GetMethod()->GetHotnessCounter() << ")";
        return stream.str();
    }
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COMPILER_QUEUE_H_

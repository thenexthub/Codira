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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_JOB_QUEUE_INTERFACE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_JOB_QUEUE_INTERFACE_H_

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

class JobQueue {
public:
    JobQueue() = default;

    virtual ~JobQueue() = default;

    /// @brief Adds a callback (in a form of lambda function) to the JobQueue
    virtual void Post(EtsObject *callback) = 0;
    /**
     * @brief Creates a link between objects, so target's state changes synchronously to source's
     *
     * Example: JS promise --> ETS promise
     */
    virtual void CreateLink(EtsObject *source, EtsObject *target) = 0;

    NO_COPY_SEMANTIC(JobQueue);
    NO_MOVE_SEMANTIC(JobQueue);
};

}  // namespace ark::ets
#endif  // PANDA_PLUGINS_ETS_RUNTIME_JOB_QUEUE_INTERFACE_H_

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

#ifndef LIBPANDABASE_TASKMANAGER_TASK_MANAGER_COMMON_H
#define LIBPANDABASE_TASKMANAGER_TASK_MANAGER_COMMON_H

#include <cstdint>
#include <functional>

#include "libarkbase/macros.h"
#include "libarkbase/taskmanager/utils/wait_list.h"

namespace ark::taskmanager {

// Priority of queue
using QueuePriority = uint8_t;
static constexpr QueuePriority MAX_QUEUE_PRIORITY = 16U;
static constexpr QueuePriority MIN_QUEUE_PRIORITY = 1U;
static constexpr QueuePriority DEFAULT_QUEUE_PRIORITY = 4U;

// CC-OFFNXT(G.PRE.02-CPP): code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TASK_MANAGER_CHECK_PRIORITY_VALUE(priority)               \
    ASSERT((priority) <= ::ark::taskmanager::MAX_QUEUE_PRIORITY); \
    ASSERT((priority) >= ::ark::taskmanager::MIN_QUEUE_PRIORITY)

// Queue id
using QueueId = size_t;
static constexpr QueueId MAX_ID_COUNT = 32U;
static constexpr QueueId INVALID_ID = MAX_ID_COUNT;
static constexpr size_t MAX_COUNT_OF_QUEUE = MAX_ID_COUNT;

// CC-OFFNXT(G.PRE.02-CPP): code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TASK_MANAGER_CHECK_ID_VALUE(id) ASSERT((id) < ::ark::taskmanager::MAX_ID_COUNT)

using RunnerCallback = std::function<void()>;

static constexpr size_t MAX_WORKER_COUNT = 16U;

using TaskWaitListElem = std::pair<RunnerCallback, std::function<void(RunnerCallback)>>;
using TaskWaitList = WaitList<TaskWaitListElem>;

}  // namespace ark::taskmanager

#endif  // LIBPANDABASE_TASKMANAGER_TASK_MANAGER_COMMON_H

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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_MAIN_WORKER_EXTERNAL_SCHEDULER
#define PANDA_PLUGINS_ETS_RUNTIME_MAIN_WORKER_EXTERNAL_SCHEDULER

#include "runtime/include/external_callback_poster.h"

namespace ark::ets {

/// @brief This class is used to schedule coroutines of the main worker from the external scheduler.
class MainWorkerExternalScheduler : public CallbackPoster {
public:
    /**
     * The TaskPosterFunc is an API that should be provided by an external scheduler to post tasks.
     *
     * @param task is a function that will be posted to the external scheduler.
     * @param taskData is the data that will be passed in this task.
     * @param taskName is the name of this task.
     * @param delayMs is the delay after which this task will be ready for execution.
     */
    using TaskPosterFunc = void (*)(void (*task)(void *data), void *taskData, const char *taskName, int64_t delayMs);

    explicit MainWorkerExternalScheduler(TaskPosterFunc poster);
    ~MainWorkerExternalScheduler() = default;
    NO_COPY_SEMANTIC(MainWorkerExternalScheduler);
    NO_MOVE_SEMANTIC(MainWorkerExternalScheduler);

private:
    void PostImpl([[maybe_unused]] WrappedCallback &&callback) override {};

    void PostImpl(int64_t delayMs) override;

    std::atomic<bool> needTriggerScheduler_;
    TaskPosterFunc poster_;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_MAIN_WORKER_EXTERNAL_SCHEDULER
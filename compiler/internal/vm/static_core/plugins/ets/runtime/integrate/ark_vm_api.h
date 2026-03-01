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
#ifndef PLUGINS_ETS_RUNTIME_INTEGRATE_ARK_VM_API
#define PLUGINS_ETS_RUNTIME_INTEGRATE_ARK_VM_API
// NOLINTBEGIN

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifndef PANDA_TARGET_WINDOWS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARKVM_API_EXPORT __attribute__((visibility("default")))
#else
#define ARKVM_API_EXPORT __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { ARKVM_OK, ARKVM_ERROR, ARKVM_INVALID_ARGS, ARKVM_INVALID_CONTEXT } arkvm_status;

typedef enum { ARKVM_SCHEDULER_RUN_ONCE } arkvm_schedule_mode;

/**
 * The arkvm_external_scheduler_poster is an API that should be provided by an external scheduler to post tasks.
 *
 * @param task is a function that will be posted to the external scheduler.
 * @param taskData is the data that will be passed in this task.
 * @param taskName is the name of this task.
 * @param delayMs is the delay after which this task will be ready for execution.
 *
 * NOTE: @param taskName and @param delayMs are optional and can be ignored by the external scheduler
 */
typedef void (*arkvm_external_scheduler_poster)(void (*task)(void *), void *taskData, const char *taskName,
                                                int64_t delayMs);

/**
 * @brief Registers posting function of the external scheduler
 *
 * The posting function is used to trigger the external scheduler and notify
 * that the RUNNABLES coroutines exist in the main worker and it's time to schedule them.
 *
 * NOTE: must be called from the MAIN coroutine
 */
ARKVM_API_EXPORT arkvm_status ARKVM_RegisterExternalScheduler(arkvm_external_scheduler_poster poster);

/**
 * @brief Schedules the RUNNABLE coroutines of the current worker
 *
 * @param mode is a scheduling policy that affects the number of scheduled coroutines.
 */
ARKVM_API_EXPORT arkvm_status ARKVM_RunScheduler(arkvm_schedule_mode mode);

#ifdef __cplusplus
}
#endif

// NOLINTEND
#endif  // PLUGINS_ETS_RUNTIME_INTEGRATE_ARK_VM_API

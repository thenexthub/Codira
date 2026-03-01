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

#ifndef PANDA_RUNTIME_THREAD_STATUS_H_
#define PANDA_RUNTIME_THREAD_STATUS_H_

#include <cstdint>

namespace ark {

enum class ThreadStatus : uint16_t {
    CREATED,
    RUNNING,
    IS_BLOCKED,
    IS_WAITING,
    IS_TIMED_WAITING,
    IS_SUSPENDED,
    IS_COMPILER_WAITING,
    IS_WAITING_INFLATION,
    IS_SLEEPING,
    IS_TERMINATED_LOOP,
    NATIVE,
    TERMINATING,
    FINISHED,
};

enum ThreadFlag { NO_FLAGS = 0, SUSPEND_REQUEST = 2, RUNTIME_TERMINATION_REQUEST = 4, SAFEPOINT_REQUEST = 8 };

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_H_

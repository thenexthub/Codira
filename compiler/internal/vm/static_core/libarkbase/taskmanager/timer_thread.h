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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TIMER_THREAD_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TIMER_THREAD_H

#include "libarkbase/taskmanager/task_manager_common.h"
#include <thread>

namespace ark::taskmanager::internal {

class TimerThread {
public:
    explicit TimerThread(TaskWaitList *list);
    NO_COPY_SEMANTIC(TimerThread);
    NO_MOVE_SEMANTIC(TimerThread);
    ~TimerThread() = default;

    void Start();
    void Finish();
    bool IsEnabled() const;

private:
    static void TimerThreadBody(TimerThread *impl);
    void TimerThreadBodyImpl();

    TaskWaitList *waitList_ = nullptr;
    bool isStarted_ = false;
    std::thread timerThread_;
};

}  // namespace ark::taskmanager::internal
#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TIMER_THREAD_H

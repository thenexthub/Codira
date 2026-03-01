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
#include "libarkbase/taskmanager/timer_thread.h"

namespace ark::taskmanager::internal {

TimerThread::TimerThread(TaskWaitList *list) : waitList_(list)
{
    ASSERT(waitList_ != nullptr);
}

/*static*/
void TimerThread::TimerThreadBody(TimerThread *impl)
{
    ASSERT(impl != nullptr);
    impl->TimerThreadBodyImpl();
}

void TimerThread::Start()
{
    timerThread_ = std::thread(TimerThreadBody, this);
    isStarted_ = true;
}

void TimerThread::Finish()
{
    waitList_->FinishProcessingInLoop();
    timerThread_.join();
    isStarted_ = false;
}

bool TimerThread::IsEnabled() const
{
    return isStarted_;
}

void TimerThread::TimerThreadBodyImpl()
{
    ASSERT(waitList_ != nullptr);
    waitList_->RunProcessingInLoop([](TaskWaitListElem &&waitVal) {
        auto &[task, taskPoster] = waitVal;
        taskPoster(std::move(task));
    });
}

}  // namespace ark::taskmanager::internal

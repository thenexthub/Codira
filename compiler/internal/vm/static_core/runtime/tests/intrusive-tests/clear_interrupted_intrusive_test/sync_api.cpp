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
#include <atomic>
#include <iostream>

#include "sync_api.h"
#include "runtime/include/managed_thread.h"

static std::atomic<bool> g_isThreadInterrupted = false;

void ark::test::IntrusiveClearInterruptedThreadTestAPI::WaitForThreadInterruption(void)
{
    LOG(INFO, RUNTIME) << "Point 1: Waiting for thread interruption";
    while (!g_isThreadInterrupted) {
    }
    LOG(INFO, RUNTIME) << "Point 3: Thread has been interrupted";
}

void ark::test::IntrusiveClearInterruptedThreadTestAPI::NotifyAboutThreadInterruption(void)
{
    LOG(INFO, RUNTIME) << "Point 2: Interrupting thread";
    g_isThreadInterrupted = true;
}

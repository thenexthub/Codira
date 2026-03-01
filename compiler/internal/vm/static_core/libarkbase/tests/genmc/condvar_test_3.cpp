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

#include "common.h"

// The test checks the work of TimedWait-SignalOne
// Thread1 sets g_shared to 1, then waits with timeout,
// Thread2 checks, that there is no race on g_shared,
// then wakes Thread1 and finishes
// Thread1 also checks, that there is no race on g_shared.

extern "C" void __VERIFIER_assume(int) __attribute__((__nothrow__));  // CC-OFF(G.EXP.01-CPP) public API

static struct fmutex g_x;
static struct CondVar g_c;
constexpr int N = 2;
constexpr int TIME_LIMIT = 100;

static void *Thread1(void *arg)
{
    int index = static_cast<int>(reinterpret_cast<intptr_t>(arg));

    MutexLock(&g_x, false);
    g_shared = 1;
    if (TimedWait(&g_c, &g_x, TIME_LIMIT, 0, false)) {
        // Timeout
        return nullptr;
    }

    CheckGlobalVar(index);
    MutexUnlock(&g_x);
    return nullptr;
}

static void *Thread2(void *arg)
{
    int index = static_cast<int>(reinterpret_cast<intptr_t>(arg));

    // To be sure, the first thread is stopped on wait
    MutexLock(&g_x, false);
    // GenMC reports a race here without locks
    __VERIFIER_assume(g_shared == 1);
    MutexUnlock(&g_x);

    MutexLock(&g_x, false);
    CheckGlobalVar(index);
    SignalCount(&g_c, WAKE_ALL);
    MutexUnlock(&g_x);
    return nullptr;
}

int main()
{
    MutexInit(&g_x);
    ConditionVariableInit(&g_c);
    g_shared = 0;
    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, nullptr, Thread1, reinterpret_cast<void *>(3u));
    pthread_create(&t2, nullptr, Thread2, reinterpret_cast<void *>(2u));

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    ConditionVariableDestroy(&g_c);
    MutexDestroy(&g_x);
    return 0;
}

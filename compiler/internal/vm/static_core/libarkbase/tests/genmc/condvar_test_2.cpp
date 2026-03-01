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

// The test checks the work of Wait-SignalAll
// There are two waiters (Thread1) and one waker (Thread2)
// Thread1 sets g_shared to 1, then waits,
// Thread2 checks, that there is no race on g_shared,
// then wakes Thread1 and finishes
// Thread1 also checks, that there is no race on g_shared.

extern "C" void __VERIFIER_assume(int) __attribute__((__nothrow__));  // CC-OFF(G.EXP.01-CPP) public API

static struct fmutex g_x;
static struct CondVar g_c;
constexpr int N = 2;
constexpr int G_SHARED_VAL_2 = 2;
constexpr int G_SHARED_VAL_5 = 5;

static void *Thread1(void *arg)
{
    MutexLock(&g_x, false);
    g_shared = g_shared + 1;
    Wait(&g_c, &g_x);

    CheckGlobalVar(g_shared + 1);
    MutexUnlock(&g_x);
    return nullptr;
}

static void *Thread2(void *arg)
{
    // To be sure, the first thread is stopped on wait
    MutexLock(&g_x, false);
    // GenMC reports a race here without locks
    __VERIFIER_assume(g_shared == G_SHARED_VAL_2);
    MutexUnlock(&g_x);

    MutexLock(&g_x, false);
    CheckGlobalVar(g_shared + 1);
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
    pthread_t t3;

    pthread_create(&t1, nullptr, Thread1, nullptr);
    pthread_create(&t3, nullptr, Thread1, nullptr);
    pthread_create(&t2, nullptr, Thread2, nullptr);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
    pthread_join(t3, nullptr);

    // Check that the threads were really waken
    ASSERT(g_shared == G_SHARED_VAL_5);

    ConditionVariableDestroy(&g_c);
    MutexDestroy(&g_x);
    return 0;
}

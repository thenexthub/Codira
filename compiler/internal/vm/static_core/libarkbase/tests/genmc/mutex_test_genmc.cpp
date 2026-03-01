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

#include <cstdlib>
#include <pthread.h>
#define MC_ON
#include "platforms/unix/libarkbase/futex/fmutex.cpp"

// Copy of mutex storage, after complete implementation should totally replace mutex::current_tid
thread_local pthread_t current_tid;

static struct fmutex g_x;
static int g_shared;

static void *ThreadN(void *arg)
{
    intptr_t index = reinterpret_cast<intptr_t>(arg);

    MutexLock(&g_x, false);
    g_shared = index;
    int r = g_shared;
    ASSERT(r == index);
    MutexUnlock(&g_x);
    return nullptr;
}

int main()
{
    constexpr int N = 2;
    MutexInit(&g_x);
    pthread_t t[N];

    for (long i = 0u; i < N; i++) {
        pthread_create(&t[i], nullptr, ThreadN, reinterpret_cast<void *>(i));
    }

    for (int i = 0u; i < N; i++) {
        pthread_join(t[i], nullptr);
    }

    MutexDestroy(&g_x);
    return 0;
}

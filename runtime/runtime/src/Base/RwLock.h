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


#ifndef MRT_RWLOCK_H
#define MRT_RWLOCK_H
#include <atomic>
#include <pthread.h>
#include "Log.h"

namespace MapleRuntime {
class RwLock {
public:
    void LockRead()
    {
        int count = lockCount.load(std::memory_order_acquire);
        do {
            while (count == WRITE_LOCKED) {
                sched_yield();
                count = lockCount.load(std::memory_order_acquire);
            }
        } while (!lockCount.compare_exchange_weak(count, count + 1, std::memory_order_release));
    }

    void LockWrite()
    {
        for (int count = 0; !lockCount.compare_exchange_weak(count, WRITE_LOCKED, std::memory_order_release);
             count = 0) {
            sched_yield();
        }
    }

    bool TryLockWrite()
    {
        int count = 0;
        if (lockCount.compare_exchange_weak(count, WRITE_LOCKED, std::memory_order_release)) {
            return true;
        }
        return false;
    }

    bool TryLockRead()
    {
        int count = lockCount.load(std::memory_order_acquire);
        do {
            if (count == WRITE_LOCKED) {
                return false;
            }
        } while (!lockCount.compare_exchange_weak(count, count + 1, std::memory_order_release));
        return true;
    }

    void UnlockRead()
    {
        int count = lockCount.fetch_sub(1);
        if (count < 0) {
            std::abort();
        }
    }

    void UnlockWrite()
    {
        CHECK(lockCount.load() == WRITE_LOCKED);
        lockCount.store(0, std::memory_order_release);
    }

private:
    // 0: unlocked
    // -1: write locked
    // inc 1 for each read lock
    static constexpr int WRITE_LOCKED = -1;
    std::atomic<int> lockCount{ 0 };
};
} // namespace MapleRuntime
#endif // MRT_SPINLOCK_H

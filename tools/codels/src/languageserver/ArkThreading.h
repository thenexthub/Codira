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

#ifndef LSPSERVER_ARKTHREADING_H
#define LSPSERVER_ARKTHREADING_H

#include <cassert>
#include <condition_variable>
#include <mutex>

namespace ark {
// Limits the number of threads that can acquire the lock at the same time.
class Semaphore {
public:
    explicit Semaphore(std::size_t maxLocks);
    ~Semaphore() {}
    // try_lock lock unlock is need by std::unique_lock
    bool try_lock();
    void lock();
    void unlock();

private:
    std::mutex mutexSemaphore {};
    std::condition_variable slotsChanged {};
    std::size_t freeSlots = 0;
};

/// A point in time we can wait for.
/// Can be zero (don't wait) or infinity (wait forever).
/// (Not time_point::max(), because many std::chrono implementations overflow).
class Deadline {
public:
    ~Deadline() {}
    static Deadline Zero()
    {
        return Deadline(Type::TYPE_ZEROS);
    }
    static Deadline Infinity()
    {
        return Deadline(Type::TYPE_INFINITE);
    }

    std::chrono::steady_clock::time_point Time() const
    {
        return time;
    }

    bool Expired() const
    {
        return (type == Type::TYPE_ZEROS) ||
               (type == Type::TYPE_FINITE && time < std::chrono::steady_clock::now());
    }
    bool operator==(const Deadline &other) const
    {
        return (type == other.type) && (type != Type::TYPE_FINITE || time == other.time);
    }

private:
    enum class Type { TYPE_ZEROS, TYPE_INFINITE, TYPE_FINITE };
    explicit Deadline(enum Type t) : type(t) {}
    enum Type type;
    std::chrono::steady_clock::time_point time {};
};

/// Wait once on CV for the specified duration.
void Wait(std::unique_lock<std::mutex> &lock, std::condition_variable &cv, const Deadline deadline);

/// Waits on a condition variable until F() is true or D expires.
template <typename Func>
void Wait(std::unique_lock<std::mutex> &lock, std::condition_variable &cv, const Deadline deadline, const Func f)
{
    while (!f()) {
        if (deadline.Expired()) {
            return;
        }
        Wait(lock, cv, deadline);
    }
}
} // namespace ark

#endif // LSPSERVER_ARKTHREADING_H

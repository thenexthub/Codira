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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_WAIT_LIST_H
#define LIBPANDABASE_TASKMANAGER_UTILS_WAIT_LIST_H

#include "libarkbase/utils/time.h"
#include <algorithm>
#include "libarkbase/os/mutex.h"
#include <optional>
#include <list>
#include <limits>

namespace ark::taskmanager {

using WaiterId = size_t;
inline constexpr WaiterId INVALID_WAITER_ID = 0U;

template <class T>
class WaitList {
    static constexpr size_t PER_PROCESSING_NANOSECOND_TIME_DELAY = 50'000U;

    class WaitValue {
    public:
        WaitValue(T &&value, uint64_t targetTime, size_t id)
            : value_(std::move(value)), targetTime_(targetTime), id_(id)
        {
        }
        DEFAULT_MOVE_SEMANTIC(WaitValue);
        NO_COPY_SEMANTIC(WaitValue);
        ~WaitValue() = default;

        T &&GetValue()
        {
            return std::move(value_);
        }

        const T &GetValue() const
        {
            return value_;
        }

        uint64_t GetTargetTime() const
        {
            return targetTime_;
        }

        void SetTargetTime(uint64_t newTargetTime)
        {
            targetTime_ = newTargetTime;
        }

        WaiterId GetId() const
        {
            return id_;
        }

    private:
        T value_;
        uint64_t targetTime_;
        WaiterId id_;
    };

public:
    /**
     * @returns value with targetTime less than or equal to current time. If such a value doesn't exist returns
     * std::nullopt.
     */
    std::optional<T> GetReadyValue();

    template <class Callback>
    size_t ProcessWaitList(Callback callback);
    /// @returns true if there is a value with a targetTime less or equal to current time. Otherwise false.
    bool HaveReadyValue() const;
    /// @brief adds value in waitQueues_ with timeToWait if it's setted.
    WaiterId AddValueToWait(T value, uint64_t timeToWait = std::numeric_limits<uint64_t>().max());
    /// @returns value with specified WaiterId. It it doesn't exists returns std::nullopt.
    std::optional<T> GetValueById(WaiterId id);

    template <class Callback>
    void RunProcessingInLoop(Callback callback);
    void FinishProcessingInLoop();

private:
    std::list<WaitValue> waitList_;
    size_t addCounter_ = 0U;
    os::memory::RWLock mutable lock_;

    os::memory::Mutex loopMutex_;
    os::memory::ConditionVariable loopCondVar_ GUARDED_BY(loopMutex_);
    bool finish_ GUARDED_BY(loopMutex_) {false};
};

template <class T>
template <class Callback>
inline void WaitList<T>::RunProcessingInLoop(Callback callback)
{
    os::memory::LockHolder lh(loopMutex_);
    while (!finish_) {
        ProcessWaitList<Callback>(callback);
        loopCondVar_.TimedWait(&loopMutex_, 0U, PER_PROCESSING_NANOSECOND_TIME_DELAY);
    }
}

template <class T>
inline void WaitList<T>::FinishProcessingInLoop()
{
    os::memory::LockHolder lh(loopMutex_);
    finish_ = true;
    loopCondVar_.SignalAll();
}

template <class T>
inline std::optional<T> WaitList<T>::GetValueById(WaiterId id)
{
    os::memory::WriteLockHolder wlh(lock_);
    auto waitValue =
        std::find_if(waitList_.begin(), waitList_.end(), [id](const auto &value) { return value.GetId() == id; });
    if (waitValue == waitList_.end()) {
        return std::nullopt;
    }
    auto val = waitValue->GetValue();
    waitList_.erase(waitValue);
    return val;
}

template <class T>
// CC-OFFNXT(G.FUD.06): inline is need to meet ODR
inline WaiterId WaitList<T>::AddValueToWait(T value, uint64_t timeToWait)
{
    os::memory::WriteLockHolder wlh(lock_);
    addCounter_++;
    uint64_t currentTime = time::GetCurrentTimeInMillis(true);
    uint64_t targetTime = 0;

    // Check if we need to set max possible time
    if (std::numeric_limits<uint64_t>().max() - timeToWait < currentTime) {
        targetTime = std::numeric_limits<uint64_t>().max();
    } else {
        targetTime = currentTime + timeToWait;
    }

    waitList_.emplace_back(std::move(value), targetTime, addCounter_);
    return waitList_.back().GetId();
}

template <class T>
inline bool WaitList<T>::HaveReadyValue() const
{
    os::memory::ReadLockHolder rlh(lock_);
    auto currentTime = time::GetCurrentTimeInMillis(true);
    for (auto iter = waitList_.begin(); iter != waitList_.end(); iter = next(iter)) {
        if (currentTime >= iter->GetTargetTime()) {
            return true;
        }
    }
    return false;
}

template <class T>
template <class Callback>
// CC-OFFNXT(G.FUD.06): inline is need to meet ODR
inline size_t WaitList<T>::ProcessWaitList(Callback callback)
{
    static constexpr size_t BUFFER_MAX_SIZE = 128U;
    std::array<T, BUFFER_MAX_SIZE> bufferOfElems {};
    size_t sizeOfBuffer = 0;
    {
        os::memory::WriteLockHolder wlh(lock_);
        auto currentTime = time::GetCurrentTimeInMillis(true);
        auto iter = waitList_.begin();
        while (iter != waitList_.end() && sizeOfBuffer < BUFFER_MAX_SIZE) {
            if (currentTime >= iter->GetTargetTime()) {
                bufferOfElems[sizeOfBuffer++] = iter->GetValue();
                iter = waitList_.erase(iter);
            } else {
                iter = next(iter);
            }
        }
    }
    for (size_t i = 0; i < sizeOfBuffer; i++) {
        callback(std::move(bufferOfElems[i]));
    }
    return sizeOfBuffer;
}

template <class T>
std::optional<T> WaitList<T>::GetReadyValue()
{
    os::memory::WriteLockHolder wlh(lock_);
    auto currentTime = time::GetCurrentTimeInMillis(true);
    for (auto iter = waitList_.begin(); iter != waitList_.end(); iter = next(iter)) {
        if (currentTime >= iter->GetTargetTime()) {
            T value = iter->GetValue();
            waitList_.erase(iter);
            return value;
        }
    }
    return std::nullopt;
}

}  // namespace ark::taskmanager

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_WAIT_LIST_H

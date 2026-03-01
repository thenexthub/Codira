/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "intrinsics.h"
#include "libarkbase/os/mutex.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::intrinsics {

using TimerId = int32_t;

class TimerInfo {
public:
    TimerInfo(mem::Reference *callback, int delay, bool isPeriodic, TimerId id)
        : callback_(callback), delay_(delay), isPeriodic_(isPeriodic), id_(id)
    {
    }

    NO_COPY_SEMANTIC(TimerInfo);
    NO_MOVE_SEMANTIC(TimerInfo);

    mem::Reference *GetCallback() const
    {
        return callback_;
    }

    int GetDelay() const
    {
        return delay_;
    }

    bool IsPeriodic() const
    {
        return isPeriodic_;
    }

    TimerId GetId() const
    {
        return id_;
    }

private:
    mem::Reference *callback_;
    const int delay_;
    const bool isPeriodic_;
    const TimerId id_;
};

class TimerTable {
public:
    bool RegisterTimer(TimerId timerId, TimerEvent *event);

    void RetireTimer(TimerId timerId);

    void DisarmTimer(TimerId timerId);

    void RemoveTimer(TimerId timerId);

    bool IsTimerDisarmed(TimerId timerId);

private:
    static constexpr TimerEvent *disarmedTimer_ = nullptr;
    static inline TimerEvent *retiredTimer_ = reinterpret_cast<TimerEvent *>(std::numeric_limits<uintptr_t>::max());

    os::memory::Mutex lock_;
    std::unordered_map<TimerId, TimerEvent *> timers_;
};

static TimerTable g_timerTable = {};
static std::atomic<TimerId> nextTimerId_ = 1;
static std::atomic<TimerId> internalEventId_ = 0;

static void InvokeCallback(mem::Reference *callback)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);
    auto *lambda = coro->GetVM()->GetGlobalObjectStorage()->Get(callback);
    LambdaUtils::InvokeVoid(coro, EtsObject::FromCoreType(lambda));
}

TimerId StdCoreRegisterTimer(EtsObject *callback, int32_t delay, uint8_t periodic)
{
    // Atomic with relaxed order reason: sync is not needed here
    auto timerId = nextTimerId_.fetch_add(1, std::memory_order_relaxed);
    auto *coro = Coroutine::GetCurrent();
    auto *refStorage = coro->GetVM()->GetGlobalObjectStorage();
    auto *callbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    auto *timerInfo =
        Runtime::GetCurrent()->GetInternalAllocator()->New<TimerInfo>(callbackRef, delay, periodic, timerId);

    auto timerEntrypoint = [](void *param) {
        auto *timer = static_cast<TimerInfo *>(param);
        auto *timerCoro = Coroutine::GetCurrent();
        auto *coroMan = timerCoro->GetManager();
        auto usDelay = timer->GetDelay() * 1000U;

        while (true) {
            // Atomic with relaxed order reason: sync is not needed here
            TimerEvent timerEvent(coroMan, internalEventId_.fetch_add(1, std::memory_order_relaxed));

            if (!g_timerTable.RegisterTimer(timer->GetId(), &timerEvent)) {
                break;
            }

            timerEvent.Lock();
            timerEvent.SetExpirationTime(coroMan->GetCurrentTime() + usDelay);
            timerCoro->GetWorker()->TriggerSchedulerExternally(timerCoro, timer->GetDelay());
            coroMan->Await(&timerEvent);

            if (g_timerTable.IsTimerDisarmed(timer->GetId())) {
                break;
            }

            InvokeCallback(timer->GetCallback());

            if (!timer->IsPeriodic()) {
                break;
            }

            g_timerTable.RetireTimer(timer->GetId());
        }

        g_timerTable.RemoveTimer(timer->GetId());
        timerCoro->GetVM()->GetGlobalObjectStorage()->Remove(timer->GetCallback());
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(timer);
    };

    auto coroName = "Timer callback: " + ark::ToPandaString(timerId);
    auto wGroup = ark::CoroutineWorkerGroup::GenerateExactWorkerId(coro->GetWorker()->GetId());
    auto launchRes = coro->GetManager()->LaunchNative(timerEntrypoint, timerInfo, std::move(coroName), wGroup,
                                                      ark::ets::EtsCoroutine::TIMER_CALLBACK, true, true);
    if (launchRes != LaunchResult::OK) {
        refStorage->Remove(callbackRef);
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(timerInfo);
    }
    return timerId;
}

void StdCoreClearTimer(TimerId timerId)
{
    g_timerTable.DisarmTimer(timerId);
}

bool TimerTable::RegisterTimer(TimerId timerId, TimerEvent *event)
{
    ASSERT(event != disarmedTimer_);
    ASSERT(event != retiredTimer_);
    os::memory::LockHolder lh(lock_);
    auto [timerIter, inserted] = timers_.insert({timerId, event});
    if (inserted) {
        return true;
    }
    if (timerIter->second == retiredTimer_) {
        timerIter->second = event;
        return true;
    }
    return false;
}

void TimerTable::RetireTimer(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    ASSERT(timerIter != timers_.end());
    if (timerIter->second != disarmedTimer_) {
        timerIter->second = retiredTimer_;
    }
}

void TimerTable::DisarmTimer(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    if (timerIter != timers_.end()) {
        auto &timerEvent = timerIter->second;
        if (timerEvent != disarmedTimer_ && timerEvent != retiredTimer_) {
            timerEvent->SetExpired();
            timerEvent->Happen();
        }
        timerEvent = disarmedTimer_;
    }
}

void TimerTable::RemoveTimer(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    if (timerIter != timers_.end()) {
        timers_.erase(timerIter);
    }
}

bool TimerTable::IsTimerDisarmed(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    ASSERT(timerIter != timers_.end());
    return timerIter->second == disarmedTimer_;
}

}  // namespace ark::ets::intrinsics

/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include <gtest/gtest.h>
#include "ani.h"
#include "ani_gtest.h"
#include "libani_helpers/concurrency_helpers.h"
#include "include/thread_scopes.h"
#include "coroutines/coroutine_manager.h"

namespace ark::ets::ani_helpers::testing {

using arkts::concurrency_helpers::GetWorkerId;
using arkts::concurrency_helpers::SendEvent;
using arkts::concurrency_helpers::WorkStatus;

namespace {

void OnExecStub([[maybe_unused]] void *data) {}

class SendEventTest : public ark::ets::ani::testing::AniTest {};

class AsyncEvent {
public:
    AsyncEvent() : event_(Coroutine::GetCurrent()->GetManager()) {}

    void Wait()
    {
        event_.Lock();
        Coroutine::GetCurrent()->GetManager()->Await(&event_);
    }

    void Fire()
    {
        event_.Happen();
    }

private:
    GenericEvent event_;
};

}  // namespace

TEST_F(SendEventTest, CreateAsyncWorkInvalidArgs)
{
    ASSERT_EQ(SendEvent(nullptr, -1, OnExecStub, nullptr), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(SendEvent(env_, 0, nullptr, nullptr), WorkStatus::INVALID_ARGS);
}

TEST_F(SendEventTest, SendEvent)
{
    auto event = AsyncEvent();
    auto workerId = GetWorkerId(env_);
    auto status = SendEvent(
        env_, workerId,
        [](void *data) {
            auto *ev = reinterpret_cast<AsyncEvent *>(data);
            ev->Fire();
        },
        reinterpret_cast<void *>(&event));
    ASSERT_EQ(status, WorkStatus::OK);

    event.Wait();
}

TEST_F(SendEventTest, SendComplexEvent)
{
    auto event1 = AsyncEvent();
    auto event2 = AsyncEvent();
    auto workerId = GetWorkerId(env_);
    std::pair<AsyncEvent *, AsyncEvent *> events {&event1, &event2};
    auto status = SendEvent(
        env_, workerId,
        [](void *data) {
            auto [ev1, ev2] = *reinterpret_cast<std::pair<AsyncEvent *, AsyncEvent *> *>(data);
            ev1->Wait();
            ev2->Fire();
        },
        reinterpret_cast<void *>(&events));
    ASSERT_EQ(status, WorkStatus::OK);

    event1.Fire();
    event2.Wait();
}

TEST_F(SendEventTest, SendEventTwoCoro)
{
    auto event1 = AsyncEvent();
    auto event2 = AsyncEvent();
    auto workerId = GetWorkerId(env_);
    auto status1 = SendEvent(
        env_, workerId,
        [](void *data) {
            auto ev1 = reinterpret_cast<AsyncEvent *>(data);
            ev1->Fire();
        },
        reinterpret_cast<void *>(&event1));

    event1.Wait();
    auto status2 = SendEvent(
        env_, workerId,
        [](void *data) {
            auto ev2 = reinterpret_cast<AsyncEvent *>(data);
            ev2->Fire();
        },
        reinterpret_cast<void *>(&event2));
    ASSERT_EQ(status1, WorkStatus::OK);
    ASSERT_EQ(status2, WorkStatus::OK);
    event2.Wait();
}

TEST_F(SendEventTest, SendTwoEventsWaitTwo)
{
    auto event1 = AsyncEvent();
    auto event2 = AsyncEvent();
    auto workerId = GetWorkerId(env_);
    auto status1 = SendEvent(
        env_, workerId,
        [](void *data) {
            auto ev1 = reinterpret_cast<AsyncEvent *>(data);
            ev1->Fire();
        },
        reinterpret_cast<void *>(&event1));

    auto status2 = SendEvent(
        env_, workerId,
        [](void *data) {
            auto ev2 = reinterpret_cast<AsyncEvent *>(data);
            ev2->Fire();
        },
        reinterpret_cast<void *>(&event2));
    ASSERT_EQ(status1, WorkStatus::OK);
    ASSERT_EQ(status2, WorkStatus::OK);
    event1.Wait();
    event2.Wait();
}

static void NestedEvent(void *data)
{
    ani_vm *vm {nullptr};
    ani_env *env {nullptr};

    ani_size nrVMs;
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, 1, &nrVMs), ANI_OK);
    ASSERT_EQ(vm->GetEnv(ANI_VERSION_1, &env), ANI_OK);
    auto workerId = GetWorkerId(env);

    auto nestedEvent = AsyncEvent();
    auto nestedStatus = SendEvent(
        env, workerId,
        [](void *nestedData) {
            auto nestedEv = reinterpret_cast<AsyncEvent *>(nestedData);
            nestedEv->Fire();
        },
        reinterpret_cast<void *>(&nestedEvent));
    ASSERT_EQ(nestedStatus, WorkStatus::OK);
    nestedEvent.Wait();

    auto ev1 = reinterpret_cast<AsyncEvent *>(data);
    ev1->Fire();
}

TEST_F(SendEventTest, SendNestedEvents)
{
    auto event1 = AsyncEvent();
    auto workerId = GetWorkerId(env_);
    auto status1 = SendEvent(env_, workerId, NestedEvent, reinterpret_cast<void *>(&event1));

    ASSERT_EQ(status1, WorkStatus::OK);
    event1.Wait();
}

TEST_F(SendEventTest, SendTimeoutEvent)
{
    auto event = AsyncEvent();
    auto workerId = GetWorkerId(env_);

    constexpr const uint64_t TIMEOUT_MILLIS = 1000U;
    auto startTime = time::GetCurrentTimeInMillis();
    auto status = SendEvent(
        env_, workerId,
        [](void *data) {
            auto *ev = reinterpret_cast<AsyncEvent *>(data);
            ev->Fire();
        },
        reinterpret_cast<void *>(&event), TIMEOUT_MILLIS);
    ASSERT_EQ(status, WorkStatus::OK);

    event.Wait();
    auto endTime = time::GetCurrentTimeInMillis();
    ASSERT_GE(endTime - startTime, TIMEOUT_MILLIS);
}

}  // namespace ark::ets::ani_helpers::testing

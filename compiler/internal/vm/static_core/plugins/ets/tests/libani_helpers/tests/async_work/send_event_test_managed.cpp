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
#include <utility>
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

class SendEventManagedTest : public ark::ets::ani::testing::AniTest {};

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

static void NativeCall(ani_env *env, ani_int id)
{
    auto event = AsyncEvent();

    auto dataSend = std::tie(event, id);
    auto status = SendEvent(
        env, id,
        [](void *data) {
            ani_vm *vm = nullptr;
            ani_size bufferSize = 1;
            ani_size size = 0;
            ASSERT_EQ(ANI_GetCreatedVMs(&vm, bufferSize, &size), ANI_OK);
            ASSERT_EQ(bufferSize, size);
            ani_env *envCurr = nullptr;
            ASSERT_EQ(vm->GetEnv(ANI_VERSION_1, &envCurr), ANI_OK);

            auto workerId = GetWorkerId(envCurr);

            auto *val = reinterpret_cast<std::tuple<AsyncEvent &, int &> *>(data);
            auto idCurr = std::get<1>(*val);

            ASSERT_EQ(idCurr, workerId);

            auto &ev = std::get<0>(*val);
            ev.Fire();
        },
        reinterpret_cast<void *>(&dataSend));
    ASSERT_EQ(status, WorkStatus::OK);

    event.Wait();
}

// NOTE(dslynko, #27940): enable this test after fixing flaky failures
TEST_F(SendEventManagedTest, DISABLED_SendEvent)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("send_event_test_managed", &module), ANI_OK);

    std::array methods {ani_native_function {"NativeCall", "i:", reinterpret_cast<void *>(NativeCall)}};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, methods.data(), methods.size()), ANI_OK);

    ani_function foo = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "TestMain", ":i", &foo), ANI_OK);

    ani_int res = -1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Function_Call_Int(foo, &res), ANI_OK);
}

}  // namespace ark::ets::ani_helpers::testing

/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"
#include "libani_helpers/concurrency_helpers.h"
#include "include/thread_scopes.h"
#include "coroutines/coroutine_manager.h"

namespace ark::ets::ani_helpers::testing {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace arkts::concurrency_helpers;

namespace {

void OnExecStub([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {}
void OnCompleteStub([[maybe_unused]] ani_env *env, [[maybe_unused]] arkts::concurrency_helpers::WorkStatus status,
                    [[maybe_unused]] void *data)
{
}

class AsyncWorkTest : public ark::ets::ani::testing::AniTest {};

class MainWorkerAsyncWorkTest : public ark::ets::ani::testing::AniTest {
    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {ani_option {"--ext:coroutine-workers-count=1", nullptr}};
    }
};

class AsyncWorkEvent {
public:
    AsyncWorkEvent() : event_(Coroutine::GetCurrent()->GetManager()) {}

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

ani_env *GetCurrentEnv()
{
    ani_vm *vm = nullptr;
    ani_size count = 0;
    auto aniStatus = ANI_GetCreatedVMs(&vm, 1, &count);
    if (aniStatus != ANI_OK || count != 1) {
        return nullptr;
    }

    ani_env *aniEnv = nullptr;
    aniStatus = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (aniStatus != ANI_OK || count != 1) {
        return nullptr;
    }
    return aniEnv;
}

struct AddonData {
    AsyncWork *asyncWork = nullptr;
    double args = 0;
    double result = 0;
};

void AddExecuteCB([[maybe_unused]] ani_env *env, void *data)
{
    AddonData *addonData = (AddonData *)data;
    addonData->result = addonData->args;
}

void AddCompleteCB(ani_env *env, [[maybe_unused]] WorkStatus status, void *data)
{
    AddonData *addonData = (AddonData *)data;

    WorkStatus deleteStatus = DeleteAsyncWork(env, addonData->asyncWork);
    ASSERT_EQ(deleteStatus, WorkStatus::OK);

    free(addonData);
    addonData = nullptr;
}
}  // namespace

TEST_F(AsyncWorkTest, CreateAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(nullptr, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CreateAsyncWork(env_, nullptr, OnCompleteStub, nullptr, &work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, nullptr, nullptr, &work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, nullptr), WorkStatus::INVALID_ARGS);
}

TEST_F(AsyncWorkTest, QueueAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(QueueAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(CancelAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CancelAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, DeleteAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(DeleteAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(DeleteAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, QueueAsyncWorkTwiceError)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **>;
    auto testData = TestData {event, &work};
    auto status = CreateAsyncWork(
        env_, [](ani_env *env, [[maybe_unused]] void *data) { ASSERT_EQ(env, GetCurrentEnv()); },
        [](ani_env *env, WorkStatus s, void *data) {
            ASSERT_EQ(env, GetCurrentEnv());
            auto &[e, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            e.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::ERROR);

    event.Wait();
}

TEST_F(AsyncWorkTest, QueueAsyncWork)
{
    auto event = AsyncWorkEvent();
    int testCounter = 0;
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **, int *>;
    auto testData = TestData {event, &work, &testCounter};
    auto status = CreateAsyncWork(
        env_,
        [](ani_env *env, void *data) {
            ASSERT_EQ(env, GetCurrentEnv());
            auto &[_, w, counter] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(*counter, 0);
            (*counter)++;
            ASSERT_EQ(*counter, 1);
        },
        [](ani_env *env, WorkStatus s, void *data) {
            ASSERT_EQ(env, GetCurrentEnv());
            auto &[e, w, counter] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(*counter, 1);
            (*counter)++;
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            e.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();
    ASSERT_EQ(testCounter, 2U);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkTwiceError)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::ERROR);
    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkBeforeQueued)
{
    AsyncWork *work = nullptr;
    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) { ASSERT_TRUE(false); },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, [[maybe_unused]] void *data) {
            ASSERT_TRUE(false);
        },
        nullptr, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::ERROR);
    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

// This test is launched with single main worker to cancel work after it was queued, but before it was started
TEST_F(MainWorkerAsyncWorkTest, CancelAsyncAfterQueued)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **>;
    auto testData = TestData {event, &work};
    auto status = CreateAsyncWork(
        env_, [](ani_env *env, [[maybe_unused]] void *data) { ASSERT_EQ(env, GetCurrentEnv()); },
        [](ani_env *env, WorkStatus s, void *data) {
            ASSERT_EQ(env, GetCurrentEnv());
            auto &[e, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::CANCELED);
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            e.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);

    event.Wait();
}

TEST_F(AsyncWorkTest, CancelAsyncAfterRunning)
{
    auto eventRun = AsyncWorkEvent();
    auto eventCompleted = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWorkEvent &, AsyncWork **>;
    auto testData = TestData {eventRun, eventCompleted, &work};
    auto status = CreateAsyncWork(
        env_,
        [](ani_env *env, [[maybe_unused]] void *data) {
            ASSERT_EQ(env, GetCurrentEnv());
            auto &[eR, eC, w] = *reinterpret_cast<TestData *>(data);
            eR.Fire();
        },
        [](ani_env *env, WorkStatus s, void *data) {
            ASSERT_EQ(env, GetCurrentEnv());
            auto &[eR, eC, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            eC.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    eventRun.Wait();
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::ERROR);
    eventCompleted.Wait();
}

// Migrate from dynamic ArkTS: Start
TEST_F(AsyncWorkTest, QueueAsyncWorkBasic)
{
    struct AsyncWorkContext {
        AsyncWork *work = nullptr;
    };
    AsyncWorkContext *asyncWorkContext = new AsyncWorkContext();
    WorkStatus status = CreateAsyncWork(
        env_, OnExecStub,
        [](ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            AsyncWorkContext *context = (AsyncWorkContext *)data;
            WorkStatus deleteStatus = DeleteAsyncWork(env, context->work);
            ASSERT_EQ(deleteStatus, WorkStatus::OK);
            delete context;
        },
        asyncWorkContext, &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(asyncWorkContext->work, nullptr);

    status = QueueAsyncWork(env_, asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);
}

TEST_F(AsyncWorkTest, QueueAsyncWorkWithExecutionFlag)
{
    struct AsyncWorkContext {
        AsyncWork *work = nullptr;
        bool executed = false;
    };
    AsyncWorkContext *asyncWorkContext = new AsyncWorkContext();
    WorkStatus status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            AsyncWorkContext *asyncWorkContext = (AsyncWorkContext *)data;
            asyncWorkContext->executed = true;
        },
        [](ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            AsyncWorkContext *context = (AsyncWorkContext *)data;
            WorkStatus deleteStatus = DeleteAsyncWork(env, context->work);
            ASSERT_EQ(deleteStatus, WorkStatus::OK);
            bool executed = context->executed;
            if (executed) {
                ASSERT_EQ(s, WorkStatus::OK);
            } else {
                ASSERT_EQ(s, WorkStatus::CANCELED);
            }
            delete context;
        },
        asyncWorkContext, &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(asyncWorkContext->work, nullptr);
    ASSERT_EQ(QueueAsyncWork(env_, asyncWorkContext->work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CreateAsyncWorkInvalidArgsMultiple)
{
    struct AsyncWorkContext {
        AsyncWork *work = nullptr;
    };
    std::unique_ptr<AsyncWorkContext> asyncWorkContext = std::make_unique<AsyncWorkContext>();
    WorkStatus status =
        CreateAsyncWork(nullptr, OnExecStub, OnCompleteStub, asyncWorkContext.get(), &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::INVALID_ARGS);

    status = CreateAsyncWork(env_, nullptr, OnCompleteStub, asyncWorkContext.get(), &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::INVALID_ARGS);

    status = CreateAsyncWork(env_, OnExecStub, nullptr, asyncWorkContext.get(), &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::INVALID_ARGS);

    status = CreateAsyncWork(env_, OnExecStub, OnCompleteStub, asyncWorkContext.get(), nullptr);
    ASSERT_EQ(status, WorkStatus::INVALID_ARGS);

    status = CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);

    WorkStatus deleteStatus = DeleteAsyncWork(env_, asyncWorkContext->work);
    ASSERT_EQ(deleteStatus, WorkStatus::OK);
}

TEST_F(AsyncWorkTest, DeleteAsyncWorkImmediately)
{
    struct AsyncWorkContext {
        AsyncWork *work = nullptr;
    };
    AsyncWorkContext *asyncWorkContext = new AsyncWorkContext();
    WorkStatus status = CreateAsyncWork(
        env_, OnExecStub,
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            AsyncWorkContext *context = (AsyncWorkContext *)data;
            ASSERT_NE(context, nullptr);
            delete context;
        },
        asyncWorkContext, &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(asyncWorkContext->work, nullptr);

    WorkStatus deleteStatus = DeleteAsyncWork(env_, asyncWorkContext->work);
    ASSERT_EQ(deleteStatus, WorkStatus::OK);
    delete asyncWorkContext;
}

TEST_F(AsyncWorkTest, QueueAsyncWorkNullPointerCheck)
{
    struct AsyncWorkContext {
        AsyncWork *work = nullptr;
    };
    AsyncWorkContext *asyncWorkContext = new AsyncWorkContext();
    WorkStatus status = CreateAsyncWork(
        env_, OnExecStub,
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            AsyncWorkContext *context = (AsyncWorkContext *)data;
            ASSERT_NE(context, nullptr);
            WorkStatus deleteStatus = DeleteAsyncWork(env, context->work);
            ASSERT_EQ(deleteStatus, WorkStatus::OK);
            delete context;
        },
        asyncWorkContext, &asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(asyncWorkContext->work, nullptr);

    status = QueueAsyncWork(env_, asyncWorkContext->work);
    ASSERT_EQ(status, WorkStatus::OK);
    status = QueueAsyncWork(env_, nullptr);
    ASSERT_EQ(status, WorkStatus::INVALID_ARGS);
}

TEST_F(AsyncWorkTest, AsyncWorkWithAddonData)
{
    AddonData *addonData = reinterpret_cast<AddonData *>(malloc(sizeof(AddonData)));
    if (addonData == nullptr) {
        return;
    }
    addonData->args = 1;

    WorkStatus status = CreateAsyncWork(env_, AddExecuteCB, AddCompleteCB, (void *)addonData, &addonData->asyncWork);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(addonData->asyncWork, nullptr);

    status = QueueAsyncWork(env_, addonData->asyncWork);
    ASSERT_EQ(status, WorkStatus::OK);
}
// Migrate from dynamic ArkTS: End

TEST_F(AsyncWorkTest, AsyncWorkWithStringData)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    std::string testStr = "Hello";
    using TestData = std::tuple<AsyncWorkEvent *, std::string *>;
    auto testData = TestData {&event, &testStr};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, str] = *reinterpret_cast<TestData *>(data);
            *str += " World!";
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, str] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(*str, "Hello World!");
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_EQ(testStr, "Hello World!");
}

TEST_F(AsyncWorkTest, AsyncWorkWithVectorData)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    std::vector<int> testVec = {1, 2, 3};
    using TestData = std::tuple<AsyncWorkEvent *, std::vector<int> *>;
    auto testData = TestData {&event, &testVec};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, vec] = *reinterpret_cast<TestData *>(data);
            vec->push_back(4);
            vec->push_back(5);
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, vec] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(vec->size(), 5U);
            ASSERT_EQ((*vec)[3], 4);
            ASSERT_EQ((*vec)[4], 5);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_EQ(testVec.size(), 5U);
}

TEST_F(AsyncWorkTest, AsyncWorkWithConditionData)
{
    struct ConditionData {
        int input;
        bool shouldProcess;
        int result;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    ConditionData conditionData {42, true, 0};
    using TestData = std::tuple<AsyncWorkEvent *, ConditionData *>;
    auto testData = TestData {&event, &conditionData};

    int expected = 84;
    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, cond] = *reinterpret_cast<TestData *>(data);
            if (cond->shouldProcess) {
                cond->result = cond->input * 2;
            }
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, cond] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(cond->result, 84);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_EQ(conditionData.result, expected);
}

TEST_F(AsyncWorkTest, AsyncWorkCallbackCalledCheck)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    bool callbackCalled = false;
    using TestData = std::tuple<AsyncWorkEvent *, bool *>;
    auto testData = TestData {&event, &callbackCalled};

    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, called] = *reinterpret_cast<TestData *>(data);
            ASSERT_FALSE(*called);
            *called = true;
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_TRUE(callbackCalled);
}

TEST_F(AsyncWorkTest, AsyncWorkWithSharedState)
{
    struct SharedState {
        int value;
        bool completed;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    SharedState sharedState {0, false};
    using TestData = std::tuple<AsyncWorkEvent *, SharedState *>;
    auto testData = TestData {&event, &sharedState};

    int expected = 100;
    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, state] = *reinterpret_cast<TestData *>(data);
            state->value = 100;
            state->completed = true;
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_EQ(sharedState.value, expected);
    ASSERT_TRUE(sharedState.completed);
}

TEST_F(AsyncWorkTest, AsyncWorkWithNullData)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent *, void *>;
    auto testData = TestData {&event, nullptr};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, p] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(p, nullptr);
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, p] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(p, nullptr);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, AsyncWorkWithNestedStructData)
{
    struct Employee {
        std::string name;
        int id;
        struct {
            float salary;
            std::string department;
        } details;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    Employee employee {"John Doe", 123, {50000.0f, "Engineering"}};
    using TestData = std::tuple<AsyncWorkEvent *, Employee *>;
    auto testData = TestData {&event, &employee};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, emp] = *reinterpret_cast<TestData *>(data);
            emp->details.salary *= 1.1f;
            emp->name += " Jr.";
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, emp] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(emp->name, "John Doe Jr.");
            ASSERT_FLOAT_EQ(emp->details.salary, 55000.0f);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_EQ(employee.name, "John Doe Jr.");
    ASSERT_FLOAT_EQ(employee.details.salary, 55000.0f);
}

TEST_F(AsyncWorkTest, MultipleAsyncWorksInLoop)
{
    std::vector<AsyncWork *> works;

    constexpr int loopCount = 3;
    for (int i = 0; i < loopCount; i++) {
        auto event = AsyncWorkEvent();
        AsyncWork *work = nullptr;
        int *index = new int(i);
        using TestData = std::tuple<AsyncWorkEvent *, int *>;
        auto testData = TestData {&event, index};

        auto status = CreateAsyncWork(
            env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
            []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
                auto &[e, idx] = *reinterpret_cast<TestData *>(data);
                ASSERT_EQ(s, WorkStatus::OK);
                delete idx;
                e->Fire();
            },
            &testData, &work);
        ASSERT_EQ(status, WorkStatus::OK);
        ASSERT_NE(work, nullptr);
        works.push_back(work);

        ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
        event.Wait();
    }

    for (auto work : works) {
        ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
    }
}

TEST_F(AsyncWorkTest, AsyncWorkWithStaticWorkPointer)
{
    auto event = AsyncWorkEvent();
    static AsyncWork *work = nullptr;
    int testCounter = 0;
    using TestData = std::tuple<AsyncWorkEvent *, int *>;
    auto testData = TestData {&event, &testCounter};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, value] = *reinterpret_cast<TestData *>(data);
            *value = 100;
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, value] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(*value, 100);
            ASSERT_EQ(DeleteAsyncWork(env, work), WorkStatus::OK);
            work = nullptr;
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(work, nullptr);
}

TEST_F(AsyncWorkTest, AsyncWorkWithLambdaFunctions)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    bool callbackExecuted = false;
    using TestData = std::tuple<AsyncWorkEvent *, bool *>;
    auto testData = TestData {&event, &callbackExecuted};

    auto createWorkFunc = [this, &testData](AsyncWork **result) {
        return CreateAsyncWork(
            env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
            []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
                auto &[e, executed] = *reinterpret_cast<TestData *>(data);
                *executed = true;
                e->Fire();
            },
            &testData, result);
    };
    auto status = createWorkFunc(&work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    auto queueWorkFunc = [this](AsyncWork *w) { return QueueAsyncWork(env_, w); };
    ASSERT_EQ(queueWorkFunc(work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_TRUE(callbackExecuted);
}

TEST_F(AsyncWorkTest, AsyncWorkWithArrayData)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    constexpr static int arrSize = 5;
    int numbers[arrSize] = {1, 2, 3, 4, 5};
    using TestData = std::tuple<AsyncWorkEvent *, int *>;
    auto testData = TestData {&event, numbers};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, arr] = *reinterpret_cast<TestData *>(data);
            for (int i = 0; i < arrSize; i++) {
                arr[i] *= arr[i];
            }
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, arr] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(arr[0], 1);
            ASSERT_EQ(arr[1], 4);
            ASSERT_EQ(arr[2], 9);
            ASSERT_EQ(arr[3], 16);
            ASSERT_EQ(arr[4], 25);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, AsyncWorkWithStringTransform)
{
    struct TextData {
        std::string original;
        std::string processed;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    TextData textData {"hello world", ""};
    using TestData = std::tuple<AsyncWorkEvent *, TextData *>;
    auto testData = TestData {&event, &textData};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, td] = *reinterpret_cast<TestData *>(data);
            std::transform(td->original.begin(), td->original.end(), std::back_inserter(td->processed), ::toupper);
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, td] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(td->processed, "HELLO WORLD");
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);

    ASSERT_EQ(textData.processed, "HELLO WORLD");
}

TEST_F(AsyncWorkTest, MultipleAsyncWorksWithMap)
{
    std::map<int, AsyncWork *> workMap;

    const static int loopCount = 5;
    for (int i = 0; i < loopCount; i++) {
        auto event = AsyncWorkEvent();
        AsyncWork *work = nullptr;
        int *workId = new int(i);
        using TestData = std::tuple<AsyncWorkEvent *, int *>;
        auto testData = TestData {&event, workId};

        auto status = CreateAsyncWork(
            env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
            []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
                auto &[e, id] = *reinterpret_cast<TestData *>(data);
                ASSERT_EQ(s, WorkStatus::OK);
                delete id;
                e->Fire();
            },
            &testData, &work);
        ASSERT_EQ(status, WorkStatus::OK);
        ASSERT_NE(work, nullptr);
        workMap[i] = work;

        ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
        event.Wait();
    }

    for (auto &pair : workMap) {
        ASSERT_EQ(DeleteAsyncWork(env_, pair.second), WorkStatus::OK);
    }
}

TEST_F(AsyncWorkTest, AsyncWorkCreateNewWorkInCallback)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;

    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
        [](ani_env *env, WorkStatus s, void *data) {
            auto &e = *reinterpret_cast<AsyncWorkEvent *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            AsyncWork *newWork = nullptr;
            auto newStatus = CreateAsyncWork(env, OnExecStub, OnCompleteStub, nullptr, &newWork);
            ASSERT_EQ(newStatus, WorkStatus::OK);
            ASSERT_NE(newWork, nullptr);
            ASSERT_EQ(DeleteAsyncWork(env, newWork), WorkStatus::OK);
            e.Fire();
        },
        &event, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, AsyncWorkLifecycleTracking)
{
    struct LifecycleData {
        AsyncWork *work;
        std::string state;
        int step;
    };

    auto event = AsyncWorkEvent();
    LifecycleData lifecycleData {nullptr, "created", 0};
    using TestData = std::tuple<AsyncWorkEvent *, LifecycleData *>;
    auto testData = TestData {&event, &lifecycleData};

    int expected = 2;
    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {
            auto &[e, ld] = *reinterpret_cast<TestData *>(data);
            ld->state = "executing";
            ld->step = 1;
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, ld] = *reinterpret_cast<TestData *>(data);
            ld->state = "completed";
            ld->step = 2;
            ASSERT_EQ(DeleteAsyncWork(env, ld->work), WorkStatus::OK);
            e->Fire();
        },
        &testData, &lifecycleData.work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(lifecycleData.work, nullptr);
    ASSERT_EQ(lifecycleData.state, "created");
    ASSERT_EQ(lifecycleData.step, 0);

    ASSERT_EQ(QueueAsyncWork(env_, lifecycleData.work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(lifecycleData.state, "completed");
    ASSERT_EQ(lifecycleData.step, expected);
}

TEST_F(AsyncWorkTest, AsyncWorkWithNullPointerCheck)
{
    struct ErrorData {
        int *invalidPtr;
        bool errorOccurred;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    ErrorData errorData {nullptr, false};
    using TestData = std::tuple<AsyncWorkEvent *, ErrorData *>;
    auto testData = TestData {&event, &errorData};

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {
            auto &[e, ed] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(ed->invalidPtr, nullptr);
            ed->errorOccurred = true;
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, ed] = *reinterpret_cast<TestData *>(data);
            ASSERT_TRUE(ed->errorOccurred);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, MultipleAsyncWorksSharedCounter)
{
    struct SharedData {
        std::atomic<int> counter;
        AsyncWorkEvent *event;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work1 = nullptr;
    AsyncWork *work2 = nullptr;
    SharedData sharedData {0, &event};

    auto status1 = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            SharedData *sd = static_cast<SharedData *>(data);
            sd->counter++;
        },
        []([[maybe_unused]] ani_env *env, WorkStatus s, [[maybe_unused]] void *data) { ASSERT_EQ(s, WorkStatus::OK); },
        &sharedData, &work1);

    auto status2 = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            SharedData *sd = static_cast<SharedData *>(data);
            sd->counter++;
        },
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            SharedData *sd = static_cast<SharedData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(sd->counter, 2);
            sd->event->Fire();
        },
        &sharedData, &work2);

    ASSERT_EQ(status1, WorkStatus::OK);
    ASSERT_EQ(status2, WorkStatus::OK);
    ASSERT_NE(work1, nullptr);
    ASSERT_NE(work2, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work1), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work2), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work1), WorkStatus::OK);
    ASSERT_EQ(DeleteAsyncWork(env_, work2), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, AsyncWorkWithClassManager)
{
    class WorkManager {
    public:
        AsyncWork *work = nullptr;
        int processedData = 0;

        void createWork(ani_env *env)
        {
            auto status = CreateAsyncWork(
                env,
                []([[maybe_unused]] ani_env *e, void *data) {
                    WorkManager *wm = reinterpret_cast<WorkManager *>(data);
                    wm->processedData = 100;
                },
                []([[maybe_unused]] ani_env *e, WorkStatus s, void *data) {
                    WorkManager *wm = reinterpret_cast<WorkManager *>(data);
                    ASSERT_EQ(s, WorkStatus::OK);
                    ASSERT_EQ(DeleteAsyncWork(e, wm->work), WorkStatus::OK);
                    wm->work = nullptr;
                    delete wm;
                    wm = nullptr;
                },
                this, &work);
            ASSERT_EQ(status, WorkStatus::OK);
        }
    };

    WorkManager *manager = new WorkManager();
    manager->createWork(env_);
    ASSERT_NE(manager->work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, manager->work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkBeforeQueuedError)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::ERROR);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkAfterCompletedError)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent *, AsyncWork **>;
    auto testData = TestData {&event, &work};
    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            auto &[e, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            e->Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::ERROR);
    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CreateMultipleWorksDifferentPointers)
{
    AsyncWork *work = nullptr;

    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    AsyncWork *firstWork = work;

    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    AsyncWork *secondWork = work;

    ASSERT_NE(firstWork, secondWork);

    ASSERT_EQ(DeleteAsyncWork(env_, firstWork), WorkStatus::OK);
    ASSERT_EQ(DeleteAsyncWork(env_, secondWork), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, DeleteMultipleWorksInCallback)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work1 = nullptr;
    AsyncWork *work2 = nullptr;

    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work1), WorkStatus::OK);
    ASSERT_NE(work1, nullptr);

    using TestData = std::tuple<AsyncWorkEvent *, AsyncWork *, AsyncWork *>;
    auto testData = TestData {&event, work1, work2};

    auto status = CreateAsyncWork(
        env_, OnExecStub,
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            auto &[e, w1, w2] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(DeleteAsyncWork(env, w1), WorkStatus::OK);
            ASSERT_EQ(DeleteAsyncWork(env, w2), WorkStatus::OK);
            e->Fire();
        },
        &testData, &work2);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work2, nullptr);

    constexpr int idx = 2;
    std::get<idx>(testData) = work2;

    ASSERT_EQ(QueueAsyncWork(env_, work2), WorkStatus::OK);
    event.Wait();
}

TEST_F(AsyncWorkTest, CancelAsyncWorkPreventsExecution)
{
    AsyncWork *work = nullptr;
    int executeCount = 0;

    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            int *count = reinterpret_cast<int *>(data);
            (*count)++;
        },
        OnCompleteStub, &executeCount, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::ERROR);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(executeCount, 0);
}

TEST_F(AsyncWorkTest, AsyncWorksExecutionOrder)
{
    auto event1 = AsyncWorkEvent();
    auto event2 = AsyncWorkEvent();

    AsyncWork *work1 = nullptr;
    AsyncWork *work2 = nullptr;

    int executionOrder = 0;

    using TestData = std::tuple<AsyncWorkEvent *, int *>;
    auto testData = TestData {&event1, &executionOrder};
    auto status1 = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, order] = *reinterpret_cast<TestData *>(data);
            *order = 1;
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, order] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(*order, 1);
            e->Fire();
        },
        &testData, &work1);

    struct DependencyData {
        int *order;
        AsyncWorkEvent *event;
    };

    DependencyData depData {&executionOrder, &event2};

    int expected = 2;
    auto status2 = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            DependencyData *dd = reinterpret_cast<DependencyData *>(data);
            *(dd->order) = 2;
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            DependencyData *dd = reinterpret_cast<DependencyData *>(data);
            ASSERT_EQ(*(dd->order), 2);
            dd->event->Fire();
        },
        &depData, &work2);

    ASSERT_EQ(status1, WorkStatus::OK);
    ASSERT_EQ(status2, WorkStatus::OK);

    ASSERT_EQ(QueueAsyncWork(env_, work1), WorkStatus::OK);
    event1.Wait();

    ASSERT_EQ(executionOrder, 1);

    ASSERT_EQ(QueueAsyncWork(env_, work2), WorkStatus::OK);
    event2.Wait();

    ASSERT_EQ(executionOrder, expected);

    ASSERT_EQ(DeleteAsyncWork(env_, work1), WorkStatus::OK);
    ASSERT_EQ(DeleteAsyncWork(env_, work2), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, AsyncWorkWithFunctionPointer)
{
    using ProcessFunc = int (*)(int);

    struct FunctionData {
        ProcessFunc func;
        int input;
        int expected;
        int result;
    };

    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    auto doubleValue = [](int x) -> int { return x * 2; };
    FunctionData functionData {doubleValue, 21, 42, 0};
    using TestData = std::tuple<AsyncWorkEvent *, FunctionData *>;
    auto testData = TestData {&event, &functionData};

    int expected = 42;
    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[e, fd] = *reinterpret_cast<TestData *>(data);
            fd->result = fd->func(fd->input);
        },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, void *data) {
            auto &[e, fd] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(fd->result, fd->expected);
            e->Fire();
        },
        &testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(functionData.result, expected);
}

TEST_F(AsyncWorkTest, QueueAsyncWorkInCompleteCB)
{
    auto outerEvent = AsyncWorkEvent();
    AsyncWork *outerWork = nullptr;
    static int testCounter = 0;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **, int *>;
    auto outerTestData = TestData {outerEvent, &outerWork, &testCounter};
    auto outerStatus = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *outerEnv, void *outerData) {
            auto &[_, outerW, outerCounter] = *reinterpret_cast<TestData *>(outerData);
            (*outerCounter)++;
        },
        [](ani_env *outerEnv, WorkStatus outerS, void *outerData) {
            auto &[outerE, outerW, outerCounter] = *reinterpret_cast<TestData *>(outerData);
            ASSERT_EQ(outerS, WorkStatus::OK);
            (*outerCounter)++;

            auto innerEvent = AsyncWorkEvent();
            AsyncWork *innerWork = nullptr;
            auto innerTestData = TestData {innerEvent, &innerWork, &testCounter};
            auto innerStatus = CreateAsyncWork(
                outerEnv,
                []([[maybe_unused]] ani_env *innerEnv, void *innerData) {
                    auto &[_, innerW, innerCounter] = *reinterpret_cast<TestData *>(innerData);
                    (*innerCounter)++;
                },
                [](ani_env *innerEnv, WorkStatus innerS, void *innerData) {
                    auto &[innerE, innerW, innerCounter] = *reinterpret_cast<TestData *>(innerData);
                    ASSERT_EQ(innerS, WorkStatus::OK);
                    (*innerCounter)++;
                    ASSERT_EQ(DeleteAsyncWork(innerEnv, *innerW), WorkStatus::OK);
                    innerE.Fire();
                },
                (void *)&innerTestData, &innerWork);
            ASSERT_EQ(innerStatus, WorkStatus::OK);
            ASSERT_NE(innerWork, nullptr);

            ASSERT_EQ(QueueAsyncWork(outerEnv, innerWork), WorkStatus::OK);
            innerEvent.Wait();
            ASSERT_EQ(DeleteAsyncWork(outerEnv, *outerW), WorkStatus::OK);
            outerE.Fire();
        },
        (void *)&outerTestData, &outerWork);
    ASSERT_EQ(outerStatus, WorkStatus::OK);
    ASSERT_NE(outerWork, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, outerWork), WorkStatus::OK);
    outerEvent.Wait();
    constexpr int expected = 4;
    ASSERT_EQ(testCounter, expected);
}
}  // namespace ark::ets::ani_helpers::testing

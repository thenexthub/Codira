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

#include "concurrency_helpers.h"

#include <atomic>

namespace arkts::concurrency_helpers {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNLIKELY(exp) (__builtin_expect((exp) != 0, false))

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_RETURN_IF_EQ(a, b, ret) \
    do {                              \
        if (UNLIKELY((a) == (b))) {   \
            return ret;               \
        }                             \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_PTR_ARG(arg) CHECK_RETURN_IF_EQ(arg, nullptr, WorkStatus::INVALID_ARGS)

namespace {

constexpr uint8_t INITIAL = 0;
constexpr uint8_t QUEUED = 1;
constexpr uint8_t RUNNING = 2;
constexpr uint8_t CANCELED = 3;

AsyncWork *CastWork(void *work)
{
    return reinterpret_cast<AsyncWork *>(work);
}

}  // namespace

class AsyncWork {
public:
    ani_env *env;
    ExecuteCallback execute;
    CompleteCallback complete;
    void *data;
    std::atomic<uint8_t> status = INITIAL;
};

namespace {

void QueueCallbackInternal(ani_env *env, void *w)
{
    auto *impl = CastWork(w);
    auto queued = QUEUED;
    if (impl->status.compare_exchange_strong(queued, RUNNING)) {
        impl->execute(env, impl->data);
    }
}

void CompleteCallbackInternal(ani_env *env, void *w)
{
    auto *impl = CastWork(w);
    auto status = WorkStatus::OK;
    if (impl->status == CANCELED) {
        status = WorkStatus::CANCELED;
    }
    impl->complete(env, status, impl->data);
}

}  // namespace

WorkStatus CreateAsyncWork(ani_env *env, ExecuteCallback execute, CompleteCallback complete,
                           [[maybe_unused]] void *data, AsyncWork **result)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(execute);
    CHECK_PTR_ARG(complete);
    CHECK_PTR_ARG(result);

    *result = new AsyncWork {env, execute, complete, data};
    return WorkStatus::OK;
}

WorkStatus QueueAsyncWork(ani_env *env, AsyncWork *work)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(work);

    auto initial = INITIAL;
    if (!work->status.compare_exchange_strong(initial, QUEUED)) {
        return WorkStatus::ERROR;
    }

    ani_class cls = nullptr;
    env->FindClass("std.concurrency.NativeAsyncWorkHelper", &cls);
    ani_static_method queueMethod;
    env->Class_FindStaticMethod(cls, "queue", nullptr, &queueMethod);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    env->Class_CallStaticMethod_Void(cls, queueMethod, reinterpret_cast<int64_t>(QueueCallbackInternal),
                                     reinterpret_cast<int64_t>(CompleteCallbackInternal), work);

    return WorkStatus::OK;
}

WorkStatus CancelAsyncWork(ani_env *env, AsyncWork *work)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(work);

    auto prevState = INITIAL;
    if (!work->status.compare_exchange_strong(prevState, CANCELED)) {
        prevState = QUEUED;
        if (!work->status.compare_exchange_strong(prevState, CANCELED)) {
            return WorkStatus::ERROR;
        }
    }

    return WorkStatus::OK;
}

WorkStatus DeleteAsyncWork(ani_env *env, AsyncWork *work)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(work);

    delete work;

    return WorkStatus::OK;
}

/**
 * @brief ANI helper to execute job on specific worker
 * @param[in] env current coro env
 * @param[in] worker target worker id
 * @param[in] event function to call
 * @param[in] data pointer to additional data to use in event
 * @param[in] timeout time in milliseconds before scheduling event
 * @returns error code
 */
WorkStatus SendEvent(ani_env *env, AniWorkerId worker, EventCallback event, void *data, ani_long timeout)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(event);

    ani_class cls = nullptr;
    env->FindClass("std.concurrency.NativeAsyncWorkHelper", &cls);
    ani_static_method sendEvent {};
    env->Class_FindStaticMethod(cls, "sendEvent", nullptr, &sendEvent);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    env->Class_CallStaticMethod_Void(cls, sendEvent, reinterpret_cast<int32_t>(worker),
                                     reinterpret_cast<int64_t>(event), reinterpret_cast<int64_t>(data), timeout);
    return WorkStatus::OK;
}

AniWorkerId GetWorkerId(ani_env *env)
{
    // NOTE(konstanting): replace with a proper check!
    CHECK_RETURN_IF_EQ(env, nullptr, -1);

    ani_class cls = nullptr;
    env->FindClass("std.debug.concurrency.CoroutineExtras", &cls);
    ani_static_method getWorkerId {};
    env->Class_FindStaticMethod(cls, "getWorkerId", nullptr, &getWorkerId);
    AniWorkerId id;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    env->Class_CallStaticMethod_Int(cls, getWorkerId, &id);
    return id;
}

}  // namespace arkts::concurrency_helpers

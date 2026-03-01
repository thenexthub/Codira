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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_EVENT_LOOP_MODULE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_EVENT_LOOP_MODULE_H

#include <uv.h>
#include <node_api.h>
#include <queue>

#include "runtime/include/external_callback_poster.h"
#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark {
class Coroutine;
}  // namespace ark

namespace ark::ets::interop::js {

class EventLoopCallbackPoster : public CallbackPoster {
    class ThreadSafeCallbackQueue {
        using CallbackQueue = std::queue<WrappedCallback>;

    public:
        ThreadSafeCallbackQueue() = default;
        NO_COPY_SEMANTIC(ThreadSafeCallbackQueue);
        NO_MOVE_SEMANTIC(ThreadSafeCallbackQueue);
        ~ThreadSafeCallbackQueue() = default;

        void PushCallback(WrappedCallback &&callback, uv_async_t *async);
        void ExecuteAllCallbacks();
        bool IsEmpty();

    private:
        os::memory::Mutex lock_;
        CallbackQueue callbackQueue_ GUARDED_BY(lock_);
    };

public:
    static_assert(PANDA_ETS_INTEROP_JS);
    explicit EventLoopCallbackPoster();
    ~EventLoopCallbackPoster() override;
    NO_COPY_SEMANTIC(EventLoopCallbackPoster);
    NO_MOVE_SEMANTIC(EventLoopCallbackPoster);

private:
    void Init();

    void PostImpl(WrappedCallback &&callback) override;

    void PostToEventLoop(WrappedCallback &&callback);

    static void AsyncEventToExecuteCallbacks(uv_async_t *async);

    uv_async_t *async_ = nullptr;
    ThreadSafeCallbackQueue *callbackQueue_ = nullptr;
};

class SingleEventPoster : public CallbackPoster {
public:
    explicit SingleEventPoster(WrappedCallback &&callback);
    ~SingleEventPoster() override;
    NO_COPY_SEMANTIC(SingleEventPoster);
    NO_MOVE_SEMANTIC(SingleEventPoster);

private:
    void PostImpl([[maybe_unused]] WrappedCallback &&callback) override {};

    void PostImpl([[maybe_unused]] int64_t delayMs) override;

    static void CallbackExecutor(uv_async_t *async);

    uv_async_t *async_ = nullptr;
    WrappedCallback callback_;
};

class EventLoopCallbackPosterFactoryImpl : public CallbackPosterFactoryIface {
public:
    EventLoopCallbackPosterFactoryImpl() = default;
    ~EventLoopCallbackPosterFactoryImpl() override = default;
    NO_COPY_SEMANTIC(EventLoopCallbackPosterFactoryImpl);
    NO_MOVE_SEMANTIC(EventLoopCallbackPosterFactoryImpl);

    /**
     * @brief Creates callback poster to perform async work in EventLoop.
     * NOTE: This method can only be called from threads that have napi_env (e.g. Main, Exclusive Workers).
     */
    PandaUniquePtr<CallbackPoster> CreatePoster() override;
};

class EventLoop {
public:
    using AsyncWorkDeleter = void (*)(uv_handle_t *);

    static uv_loop_t *GetEventLoop();

    static bool RunEventLoop(EventLoopRunMode mode = EventLoopRunMode::RUN_DEFAULT);

    static void WalkEventLoop(WalkEventLoopCallback &callback, void *args);

    static uv_timer_t *CreateTimer();

    static void CloseTimer(uv_timer_t *timer);

    static uv_async_t *CreateAsyncWork();

    static void DeleteAsyncWork(uv_async_t *async);

    static void DeleteAsyncWork(uv_async_t *async, AsyncWorkDeleter deleter);

private:
    static void DefaultHandleDeleter(uv_handle_t *handle);

    static void IncrementEventCount();
    static void DecrementEventCount();

    static thread_local uint32_t eventCount_;
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_EVENT_LOOP_MODULE_H

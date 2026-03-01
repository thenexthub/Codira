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

#include "plugins/ets/runtime/interop_js/event_loop_module.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"

// NOTE(konstanting, #23205): A workaround for the hybrid cmake build. Will be removed soon
// using a separate .cpp file with weak symbols.
#if defined(PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS)
extern "C" napi_status __attribute__((weak))  // CC-OFF(G.FMT.10) project code style
napi_get_uv_event_loop([[maybe_unused]] napi_env env, [[maybe_unused]] struct uv_loop_s **loop)
{
    // NOTE: Empty stub. Needed only for the corner case with verifier/aot in the hybrid cmake build
    INTEROP_LOG(ERROR) << "napi_add_env_cleanup_hook is implemented in OHOS since 4.1.0, please update" << std::endl;
    return napi_ok;
}
#endif /* PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS */

namespace ark::ets::interop::js {

thread_local uint32_t EventLoop::eventCount_ {0};

/* static */
void EventLoop::IncrementEventCount()
{
    eventCount_++;
}

/* static */
void EventLoop::DecrementEventCount()
{
    eventCount_--;
}

/* static */
void EventLoop::DefaultHandleDeleter(uv_handle_t *handle)
{
    delete handle;
    DecrementEventCount();
}

/*static*/
uv_loop_t *EventLoop::GetEventLoop()
{
    uv_loop_t *loop = nullptr;
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
    [[maybe_unused]] auto nstatus = napi_get_uv_event_loop(InteropCtx::Current()->GetJSEnv(), &loop);
    ASSERT(nstatus == napi_ok);
#else
    loop = uv_default_loop();
#endif
    return loop;
}

bool EventLoop::RunEventLoop(EventLoopRunMode mode)
{
    if (InteropCtx::Current() == nullptr) {
        return false;
    }

    InteropCtx::Current(EtsCoroutine::GetCurrent())->UpdateInteropStackInfoIfNeeded();
    auto *loop = GetEventLoop();
    switch (mode) {
        case EventLoopRunMode::RUN_DEFAULT:
            uv_run(loop, UV_RUN_DEFAULT);
            break;
        case EventLoopRunMode::RUN_ONCE:
            uv_run(loop, UV_RUN_ONCE);
            break;
        case EventLoopRunMode::RUN_NOWAIT:
            if (eventCount_ == 0) {
                return false;
            }
            uv_run(loop, UV_RUN_NOWAIT);
            break;
        default:
            UNREACHABLE();
    };
    return true;
}

void EventLoop::WalkEventLoop(WalkEventLoopCallback &callback, void *args)
{
    auto *loop = GetEventLoop();

    struct CallbackStruct {
        std::function<void(void *, void *)> *callback;
        void *args;
    };

    CallbackStruct parsedArgs {&callback, args};

    auto uvCalback = []([[maybe_unused]] uv_handle_t *handle, void *rawArgs) {
        auto callbackStruct = reinterpret_cast<CallbackStruct *>(rawArgs);
        (*(callbackStruct->callback))(reinterpret_cast<void *>(handle), callbackStruct->args);
    };
    uv_walk(loop, uvCalback, &parsedArgs);
}

uv_timer_t *EventLoop::CreateTimer()
{
    auto *timer = new uv_timer_t();
    IncrementEventCount();
    return timer;
}

void EventLoop::CloseTimer(uv_timer_t *timer)
{
    uv_timer_stop(timer);
    uv_close(reinterpret_cast<uv_handle_t *>(timer), DefaultHandleDeleter);
}

uv_async_t *EventLoop::CreateAsyncWork()
{
    auto async = new uv_async_t();
    IncrementEventCount();
    return async;
}

void EventLoop::DeleteAsyncWork(uv_async_t *async)
{
    uv_close(reinterpret_cast<uv_handle_t *>(async), DefaultHandleDeleter);
}

void EventLoop::DeleteAsyncWork(uv_async_t *async, AsyncWorkDeleter deleter)
{
    using PackedData = std::pair<void *, AsyncWorkDeleter>;

    auto *data = new PackedData();
    data->first = std::exchange(async->data, data);
    data->second = deleter;
    uv_close(reinterpret_cast<uv_handle_t *>(async), [](uv_handle_t *handle) {
        auto *packedData = static_cast<PackedData *>(handle->data);
        auto [userData, asyncDeleter] = *packedData;
        delete packedData;
        handle->data = userData;
        asyncDeleter(handle);
        DecrementEventCount();
    });
}

EventLoopCallbackPoster::EventLoopCallbackPoster()
{
    Init();
}

void EventLoopCallbackPoster::Init()
{
    auto loop = EventLoop::GetEventLoop();
    // These resources will be deleted in the event loop callback after Runtime destruction,
    // so we need to use a standard allocator
    async_ = EventLoop::CreateAsyncWork();
    callbackQueue_ = new ThreadSafeCallbackQueue();
    [[maybe_unused]] auto uvstatus = uv_async_init(loop, async_, AsyncEventToExecuteCallbacks);
    ASSERT(uvstatus == 0);
    async_->data = callbackQueue_;
}

EventLoopCallbackPoster::~EventLoopCallbackPoster()
{
    ASSERT(async_ != nullptr);
    auto destroyCb = [async = this->async_]() {
        EventLoop::DeleteAsyncWork(async, [](uv_handle_t *handle) {
            auto *poster = reinterpret_cast<ThreadSafeCallbackQueue *>(handle->data);
            delete poster;
            delete handle;
        });
    };
    if (NeedDestroyInPlace()) {
        destroyCb();
        return;
    }
    PostToEventLoop(std::move(destroyCb));
}

void EventLoopCallbackPoster::PostImpl(WrappedCallback &&callback)
{
    ASSERT(callback != nullptr);
    PostToEventLoop(std::move(callback));
}

void EventLoopCallbackPoster::PostToEventLoop(WrappedCallback &&callback)
{
    ASSERT(async_ != nullptr);
    callbackQueue_->PushCallback(std::move(callback), async_);
}

/*static*/
void EventLoopCallbackPoster::AsyncEventToExecuteCallbacks(uv_async_t *async)
{
    auto *callbackQueue = reinterpret_cast<ThreadSafeCallbackQueue *>(async->data);
    ASSERT(callbackQueue != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedInteropCallStackRecord codeScope(coro, __PRETTY_FUNCTION__);
    callbackQueue->ExecuteAllCallbacks();
}

SingleEventPoster::SingleEventPoster(WrappedCallback &&callback) : callback_(std::move(callback))
{
    auto loop = EventLoop::GetEventLoop();
    async_ = EventLoop::CreateAsyncWork();
    [[maybe_unused]] auto uvstatus = uv_async_init(loop, async_, CallbackExecutor);
    ASSERT(uvstatus == 0);
    async_->data = this;
}

SingleEventPoster::~SingleEventPoster()
{
    ASSERT(async_ != nullptr);
    if (NeedDestroyInPlace()) {
        EventLoop::DeleteAsyncWork(async_);
        return;
    }
    async_->data = nullptr;
    uv_async_send(async_);
}

/* static */
void SingleEventPoster::CallbackExecutor(uv_async_t *async)
{
    auto *eventPoster = static_cast<SingleEventPoster *>(async->data);
    if (eventPoster == nullptr) {
        EventLoop::DeleteAsyncWork(async);
        return;
    }
    eventPoster->callback_();
}

void SingleEventPoster::PostImpl([[maybe_unused]] int64_t delayMs)
{
    uv_async_send(async_);
}

void EventLoopCallbackPoster::ThreadSafeCallbackQueue::PushCallback(WrappedCallback &&callback, uv_async_t *async)
{
    {
        os::memory::LockHolder lh(lock_);
        callbackQueue_.push(std::move(callback));
    }
    ASSERT(async != nullptr);
    [[maybe_unused]] auto uvstatus = uv_async_send(async);
    ASSERT(uvstatus == 0);
}

void EventLoopCallbackPoster::ThreadSafeCallbackQueue::ExecuteAllCallbacks()
{
    while (!IsEmpty()) {
        auto localQueue = CallbackQueue {};
        {
            os::memory::LockHolder lh(lock_);
            std::swap(localQueue, callbackQueue_);
        }
        while (!localQueue.empty()) {
            auto callback = std::move(localQueue.front());
            callback();
            localQueue.pop();
        }
    }
}

bool EventLoopCallbackPoster::ThreadSafeCallbackQueue::IsEmpty()
{
    os::memory::LockHolder lh(lock_);
    return callbackQueue_.empty();
}

PandaUniquePtr<CallbackPoster> EventLoopCallbackPosterFactoryImpl::CreatePoster()
{
    [[maybe_unused]] auto *w = Coroutine::GetCurrent()->GetContext<StackfulCoroutineContext>()->GetWorker();
    ASSERT(w->IsMainWorker() || w->InExclusiveMode());
    auto poster = MakePandaUnique<EventLoopCallbackPoster>();
    ASSERT(poster != nullptr);
    return poster;
}

}  // namespace ark::ets::interop::js

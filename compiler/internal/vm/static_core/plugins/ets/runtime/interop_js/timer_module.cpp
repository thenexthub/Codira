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

#include <unistd.h>
#include <string_view>
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/interop_js/timer_module.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/event_loop_module.h"
#include "plugins/ets/runtime/types/ets_type.h"

// NOTE(konstanting, #23205): in case of interop with Node, libuv will be available automatically via
// the load of the ets_vm_plugin module. But since the libuv users are now in the libarkruntime, we should
// define some weak stubs for libuv functions. Actually it's worth to create a separate cpp file for them,
// like it is done for napi.
#if (!defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE)) || \
    defined(PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS)
// NOLINTBEGIN(readability-identifier-naming)
// CC-OFFNXT(G.FMT.10-CPP) project code style
extern "C" {
__attribute__((weak)) int uv_async_send([[maybe_unused]] uv_async_t *async)
{
    return -1;
}
__attribute__((weak)) int uv_async_init([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_async_t *async,
                                        [[maybe_unused]] uv_async_cb async_cb)
{
    return -1;
}

__attribute__((weak)) void uv_update_time([[maybe_unused]] uv_loop_t *loop) {}

__attribute__((weak)) int uv_timer_init([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_timer_t *handle)
{
    return -1;
}
__attribute__((weak)) int uv_timer_start([[maybe_unused]] uv_timer_t *handle, [[maybe_unused]] uv_timer_cb cb,
                                         [[maybe_unused]] uint64_t timeout, [[maybe_unused]] uint64_t repeat)
{
    return -1;
}
__attribute__((weak)) int uv_timer_stop([[maybe_unused]] uv_timer_t *handle)
{
    return -1;
}
__attribute__((weak)) int uv_timer_again([[maybe_unused]] uv_timer_t *handle)
{
    return -1;
}

__attribute__((weak)) void uv_close([[maybe_unused]] uv_handle_t *handle, [[maybe_unused]] uv_close_cb close_cb) {}

__attribute__((weak)) uv_loop_t *uv_default_loop()
{
    return nullptr;
}
__attribute__((weak)) int uv_run([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_run_mode mode)
{
    return -1;
}
__attribute__((weak)) void uv_walk([[maybe_unused]] uv_loop_t *loop, [[maybe_unused]] uv_walk_cb walk_cb,
                                   [[maybe_unused]] void *arg)
{
}
}
// NOLINTEND(readability-identifier-naming)
#endif /* !defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE) */

bool TimerModule::Init(ani_env *env)
{
    ani_module module {};
    auto status = env->FindModule("escompat", &module);
    if (status != ANI_OK) {
        INTEROP_LOG(ERROR) << "Cannot find ESCOMPAT module";
        return false;
    }

    std::array methods = {
        ani_native_function {"startTimerImpl", "C{std.core.Object}iz:i",
                             reinterpret_cast<void *>(TimerModule::StartTimer)},
        ani_native_function {"stopTimerImpl", "i:", reinterpret_cast<void *>(TimerModule::StopTimer)},
    };
    return env->Module_BindNativeFunctions(module, methods.data(), methods.size()) == ANI_OK;
}

ani_int TimerModule::StartTimer(ani_env *env, ani_object funcObject, ani_int delayMs, ani_boolean repeat)
{
    ASSERT(ark::ets::interop::js::InteropCtx::Current() != nullptr);

    auto *timer = ark::ets::interop::js::EventLoop::CreateTimer();
    auto timerTable = GetTimerTable(env);
    ani_ref timerInfoRef {};

    static constexpr auto SIGNATURE = "C{std.core.Object}izl:C{std.concurrency.Timer}";
    [[maybe_unused]] auto status =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Object_CallMethodByName_Ref(timerTable, "createTimer", SIGNATURE, &timerInfoRef, funcObject, delayMs,
                                         repeat, reinterpret_cast<ani_long>(timer));
    ASSERT(status == ANI_OK);
    auto timerInfo = static_cast<ani_object>(timerInfoRef);

    ani_int timerId {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethodByName_Int(timerInfo, "getId", ":i", &timerId);
    ASSERT(status == ANI_OK);

    timer->data = reinterpret_cast<void *>(timerId);

    uv_loop_t *loop = ark::ets::interop::js::EventLoop::GetEventLoop();
    uv_timer_init(loop, timer);
    uv_update_time(loop);
    ani_int intervalMs = static_cast<bool>(repeat)
                             ? (delayMs > TimerModule::MIN_INTERVAL_MS ? delayMs : TimerModule::MIN_INTERVAL_MS)
                             : 0;
    uv_timer_start(timer, TimerCallback, delayMs, intervalMs);
    uv_async_send(&loop->wq_async);
    return timerId;
}

void TimerModule::StopTimer(ani_env *env, ani_int timerId)
{
    ASSERT(ark::ets::interop::js::InteropCtx::Current() != nullptr);
    auto [info, exists] = FindTimerInfo(env, timerId);
    if (exists) {
        TimerInfo timerInfo(info);
        auto *timer = timerInfo.GetLinkedTimer(env);
        if (timer != nullptr) {
            DisarmTimer(timer);
        }
        ClearTimerInfo(env, timerId);
    }
}

void TimerModule::TimerCallback(uv_timer_t *timer)
{
    auto timerFunc = [](void *param) {
        auto *timerHandle = static_cast<uv_timer_t *>(param);
        auto *env = ark::ets::EtsCoroutine::GetCurrent()->GetEtsNapiEnv();
        auto timerId = reinterpret_cast<uint64_t>(timerHandle->data);
        auto [info, exists] = FindTimerInfo(env, timerId);
        if (!exists) {
            return;
        }

        TimerInfo timerInfo(info);
        auto oneShotTimer = timerInfo.IsOneShotTimer(env);
        if (oneShotTimer) {
            DisarmTimer(timerHandle);
            ClearTimerInfo(env, timerId);
        }
        timerInfo.InvokeCallback(env);
        if (!oneShotTimer) {
            RepeatTimer(timerHandle, timerId);
        }
    };
    auto timerId = reinterpret_cast<uint64_t>(timer->data);
    auto coroName = "Timer callback: " + ark::ToPandaString(timerId);
    {
        auto *coro = ark::ets::EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        ark::ScopedManagedCodeThread managedScope(coro);
        coro->GetCoroutineManager()->LaunchNative(timerFunc, timer, std::move(coroName),
                                                  ark::CoroutineWorkerGroup::GenerateExactWorkerId(
                                                      ark::ets::EtsCoroutine::GetCurrent()->GetWorker()->GetId()),
                                                  ark::ets::EtsCoroutine::TIMER_CALLBACK, false, true);
    }
    uv_timer_stop(timer);
}

void TimerModule::RepeatTimer(uv_timer_t *timer, uint64_t timerId)
{
    auto *coro = ark::ets::EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *env = coro->GetEtsNapiEnv();
    auto [_, exists] = FindTimerInfo(env, timerId);
    if (exists) {
        uv_timer_again(timer);
    }
}

void TimerModule::DisarmTimer(uv_timer_t *timer)
{
    ark::ets::interop::js::EventLoop::CloseTimer(timer);
}

ani_object TimerModule::GetTimerTable(ani_env *env)
{
    ani_class loopCls {};
    [[maybe_unused]] auto status = env->FindClass("std.concurrency.EventLoop", &loopCls);
    ASSERT(status == ANI_OK);

    ani_ref timerTable {};
    status = env->Class_GetStaticFieldByName_Ref(loopCls, "timerTable", &timerTable);
    ASSERT(status == ANI_OK);
    return static_cast<ani_object>(timerTable);
}

std::pair<ani_object, bool> TimerModule::FindTimerInfo(ani_env *env, ani_int timerId)
{
    auto timerTable = GetTimerTable(env);
    ani_ref timerInfoRef {};
    static constexpr auto SIGNATURE = "i:C{std.concurrency.Timer}";
    [[maybe_unused]] ani_status status {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethodByName_Ref(timerTable, "findTimer", SIGNATURE, &timerInfoRef, timerId);
    ASSERT(status == ANI_OK);

    auto timerInfo = static_cast<ani_object>(timerInfoRef);
    ani_boolean isUndefined {};
    status = env->Reference_IsUndefined(timerInfo, &isUndefined);
    ASSERT(status == ANI_OK);
    return {timerInfo, !static_cast<bool>(isUndefined)};
}

void TimerModule::ClearTimerInfo(ani_env *env, ani_int timerId)
{
    auto timerTable = GetTimerTable(env);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->Object_CallMethodByName_Void(timerTable, "clearTimer", "i:", timerId);
    ASSERT(status == ANI_OK);
}

bool TimerModule::TimerInfo::IsOneShotTimer(ani_env *env)
{
    ani_boolean repeat {};
    [[maybe_unused]] auto status = env->Object_GetFieldByName_Boolean(info_, "repeat", &repeat);
    ASSERT(status == ANI_OK);
    return !static_cast<bool>(repeat);
}

uv_timer_t *TimerModule::TimerInfo::GetLinkedTimer(ani_env *env)
{
    ani_long linkedTimer;
    [[maybe_unused]] auto status = env->Object_GetFieldByName_Long(info_, "linkedTimer", &linkedTimer);
    ASSERT(status == ANI_OK);
    return reinterpret_cast<uv_timer_t *>(linkedTimer);
}

void TimerModule::TimerInfo::InvokeCallback(ani_env *env)
{
    ani_object callback {};
    [[maybe_unused]] auto status =
        env->Object_GetFieldByName_Ref(info_, "callback", reinterpret_cast<ani_ref *>(&callback));
    ASSERT(status == ANI_OK);

    ani_class callbackCls {};
    status = env->Object_GetType(callback, reinterpret_cast<ani_type *>(&callbackCls));
    ASSERT(status == ANI_OK);

    ani_method invokeMethod {};
    status = env->Class_FindMethod(callbackCls, ark::ets::INVOKE_METHOD_NAME, nullptr, &invokeMethod);
    ASSERT(status == ANI_OK);

    auto *coro = ark::ets::EtsCoroutine::GetCurrent();
    ark::ets::interop::js::INTEROP_CODE_SCOPE_JS_TO_ETS(coro);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Void(callback, invokeMethod);
    ASSERT(status == ANI_OK);
}

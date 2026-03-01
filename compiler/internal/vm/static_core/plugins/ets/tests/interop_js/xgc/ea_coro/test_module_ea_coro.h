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

#ifndef PANDA_PLUGINS_ETS_TESTS_INTEROP_JS_XGC_EA_CORO_TEST_MODULE_EA_CORO_H
#define PANDA_PLUGINS_ETS_TESTS_INTEROP_JS_XGC_EA_CORO_TEST_MODULE_EA_CORO_H

#include <vector>
#include <thread>

#include "libarkbase/os/filesystem.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js::testing {

inline void NapiStatusCheck(napi_status status)
{
    if (UNLIKELY(status != napi_ok)) {
        PrintStack(std::cerr);
        LOG(FATAL, ETS_INTEROP_JS) << "napi_status != napi_ok: " << status;
    }
}

class TestModule {
public:
    NO_COPY_SEMANTIC(TestModule);
    NO_MOVE_SEMANTIC(TestModule);
    TestModule() = delete;
    ~TestModule() = default;

    static std::vector<std::string> GetModuleNames(napi_env env, napi_callback_info info)
    {
        size_t jsArgC;
        NapiStatusCheck(napi_get_cb_info(env, info, &jsArgC, nullptr, nullptr, nullptr));
        if (jsArgC == 0U) {
            NapiStatusCheck(
                napi_throw_error(env, nullptr, "Incorrect arguments number. The arguments count must be > 0"));
            return {};
        }
        std::vector<std::string> jsModuleNames;
        std::vector<napi_value> jsArg(jsArgC);
        jsModuleNames.reserve(jsArgC);
        NapiStatusCheck(napi_get_cb_info(env, info, &jsArgC, jsArg.data(), nullptr, nullptr));
        for (size_t i = 0; i < jsArgC; ++i) {
            std::string moduleName;
            size_t length;
            NapiStatusCheck(napi_get_value_string_utf8(env, jsArg[i], nullptr, 0, &length));
            moduleName.resize(length);
            NapiStatusCheck(napi_get_value_string_utf8(env, jsArg[i], moduleName.data(), length + 1, nullptr));
            jsModuleNames.emplace_back(std::move(moduleName));
        }
        return jsModuleNames;
    }

    static void RunEAWorker(ani_vm *etsVM, std::string moduleName)
    {
        LOG(INFO, ETS_INTEROP_JS) << "Start EA thread: " << moduleName;
        ani_option interopEnabled {"--interop=enable", nullptr};
        ani_options aniArgs {1, &interopEnabled};
        ani_env *etsEnv;
        // Start EA Worker
        etsVM->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv);
        LOG_IF(etsEnv == nullptr, FATAL, ETS_INTEROP_JS) << "Could not get ets env after AttachCurrentThread";
        napi_env jsEnv = InteropCtx::Current()->GetJSEnv();
        auto modulePath = os::GetCurrentWorkingDirectory() + "/" + moduleName;
        // Load js file to the current EA Worker with empty JS runtime
        napi_value jsModule;
        NapiStatusCheck(napi_load_module_with_module_request(jsEnv, modulePath.c_str(), &jsModule));
        // Call "test" function from loaded module
        napi_value calledFn;
        NapiStatusCheck(napi_get_named_property(jsEnv, jsModule, "test", &calledFn));
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(jsEnv, &jsUndefined));
        NapiStatusCheck(napi_call_function(jsEnv, jsUndefined, calledFn, 0U, nullptr, nullptr));
        // Finish EA Worker
        etsVM->DetachCurrentThread();
        LOG(INFO, ETS_INTEROP_JS) << "Finish EA thread: " << moduleName;
    }

    static ani_vm *Setup()
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        mainThreadId_.store(os::thread::GetCurrentThreadId(), std::memory_order_relaxed);
        // Get ETS VM
        ani_vm *etsVM;
        ani_size vmCount;
        if (ANI_GetCreatedVMs(&etsVM, 1, &vmCount) != ANI_OK) {
            return nullptr;
        }
        ASSERT_PRINT(vmCount == 1, "VM count = " << vmCount);
        return etsVM;
    }

    static napi_value SetupAndStartEAWorkers(napi_env env, napi_callback_info info)
    {
        // Setup
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(env, &jsUndefined));
        auto *etsVM = Setup();
        if (etsVM == nullptr) {
            NapiStatusCheck(napi_throw_error(env, nullptr, "Could not setup test module"));
            return jsUndefined;
        }
        // Process arguments for EA workers
        auto jsModuleNames = GetModuleNames(env, info);
        if (jsModuleNames.empty()) {
            return jsUndefined;
        }
        {
            os::memory::LockHolder lh(waitForEaThreadsMutex_);
            eaWaitersCount_ = jsModuleNames.size();
        }
        // Start EA Workers
        LOG(INFO, ETS_INTEROP_JS) << "Need to create EA threads: " << jsModuleNames.size();
        for (size_t i = 0; i < jsModuleNames.size(); ++i) {
            eaThreads_.emplace_back(RunEAWorker, etsVM, jsModuleNames[i]);
            os::thread::SetThreadName(eaThreads_.back().native_handle(),
                                      ("EAWorker_" + std::to_string(i + 1U)).c_str());
        }
        return jsUndefined;
    }

    static napi_value WaitForEAWorkers(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(env, &jsUndefined));
        // Atomic with relaxed order reason: ordering constraints are not required
        if (mainThreadId_.load(std::memory_order_relaxed) != os::thread::GetCurrentThreadId()) {
            NapiStatusCheck(napi_throw_error(env, nullptr, "Call not from main worker"));
            return jsUndefined;
        }
        os::memory::LockHolder lh(waitForEaThreadsMutex_);
        while (eaWaitersCount_ > 0U) {
            waitForEaThreadsCV_.Wait(&waitForEaThreadsMutex_);
        }
        eaWaitersCount_ = eaThreads_.size();
        return jsUndefined;
    }

    static napi_value SignalMainWorker(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(env, &jsUndefined));
        // Atomic with relaxed order reason: ordering constraints are not required
        if (mainThreadId_.load(std::memory_order_relaxed) == os::thread::GetCurrentThreadId()) {
            NapiStatusCheck(napi_throw_error(env, nullptr, "Call from main worker"));
            return jsUndefined;
        }
        os::memory::LockHolder lh(waitForEaThreadsMutex_);
        ASSERT(eaWaitersCount_ > 0U);
        eaWaitersCount_--;
        waitForEaThreadsCV_.Signal();
        return jsUndefined;
    }

    static napi_value JoinEAWorkers(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(env, &jsUndefined));
        // Atomic with relaxed order reason: ordering constraints are not required
        if (mainThreadId_.load(std::memory_order_relaxed) != os::thread::GetCurrentThreadId()) {
            NapiStatusCheck(napi_throw_error(env, nullptr, "Call not from main worker"));
            return jsUndefined;
        }
        for (auto &eaThread : eaThreads_) {
            eaThread.join();
        }
        LOG(INFO, ETS_INTEROP_JS) << "All EA threads are joined";
        return jsUndefined;
    }

    static napi_value FireEventToEaWorkers(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(env, &jsUndefined));
        // Atomic with relaxed order reason: ordering constraints are not required
        if (mainThreadId_.load(std::memory_order_relaxed) != os::thread::GetCurrentThreadId()) {
            NapiStatusCheck(napi_throw_error(env, nullptr, "Call not from main worker"));
            return jsUndefined;
        }
        fromMainEvent_.Fire();
        return jsUndefined;
    }

    static napi_value WaitEventFromMain(napi_env env, [[maybe_unused]] napi_callback_info info)
    {
        napi_value jsUndefined;
        NapiStatusCheck(napi_get_undefined(env, &jsUndefined));
        // Atomic with relaxed order reason: ordering constraints are not required
        if (mainThreadId_.load(std::memory_order_relaxed) == os::thread::GetCurrentThreadId()) {
            NapiStatusCheck(napi_throw_error(env, nullptr, "Call from main worker"));
            return jsUndefined;
        }
        fromMainEvent_.Wait();
        return jsUndefined;
    }

    static napi_value Init(napi_env env, napi_value exports)
    {
        const std::array desc = {
            napi_property_descriptor {"setupAndStartEAWorkers", 0, SetupAndStartEAWorkers, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"waitForEAWorkers", 0, WaitForEAWorkers, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"signalMainWorker", 0, SignalMainWorker, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"fireEventToEAWorkers", 0, FireEventToEaWorkers, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"waitEventFromMainWorker", 0, WaitEventFromMain, 0, 0, 0, napi_enumerable, 0},
            napi_property_descriptor {"joinEAWorkers", 0, JoinEAWorkers, 0, 0, 0, napi_enumerable, 0},
        };
        napi_define_properties(env, exports, desc.size(), desc.data());
        return exports;
    }

protected:
    static inline std::vector<std::thread> eaThreads_;
    static inline os::memory::Mutex waitForEaThreadsMutex_;
    static inline os::memory::ConditionVariable waitForEaThreadsCV_ GUARDED_BY(waitForEaThreadsMutex_);
    static inline size_t eaWaitersCount_ GUARDED_BY(waitForEaThreadsMutex_) {0U};
    static inline os::memory::Event fromMainEvent_;
    static inline std::atomic<os::thread::ThreadId> mainThreadId_ {};
};
}  // namespace ark::ets::interop::js::testing

#endif  // PANDA_PLUGINS_ETS_TESTS_INTEROP_JS_XGC_EA_CORO_TEST_MODULE_EA_CORO_H

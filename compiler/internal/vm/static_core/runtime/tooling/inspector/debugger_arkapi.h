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

#ifndef PANDA_DEBUGGER_ARKAPI_H
#define PANDA_DEBUGGER_ARKAPI_H

#include <functional>
#include <memory>
#include <string>
#include <dlfcn.h>
#include <cstdint>

#ifndef PANDA_TARGET_WINDOWS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDA_DEBUGGER_PUBLIC_API __attribute__((visibility("default")))
#else
#define PANDA_DEBUGGER_PUBLIC_API __declspec(dllexport)
#endif

using DebuggerPostTask = std::function<void(std::function<void()> &&)>;

namespace ark {
class PANDA_DEBUGGER_PUBLIC_API ArkDebugNativeAPI final {
public:
    using DebuggerPostTask = std::function<void(std::function<void()> &&)>;
    static bool StartDebuggerForSocketPair(int tid, int socketfd = -1);
    static bool NotifyDebugMode(int tid, int32_t instanceId, bool debugApp, void *vm,
                                DebuggerPostTask &debuggerPostTask);
    static bool StopDebugger(void *vm);
    static bool IsDebugModeEnabled();

    static bool StartProfiling(const std::string &filePath, uint32_t interval = DEFAULT_SAMPLE_INTERVAL_US);
    static bool StopProfiling();

    ArkDebugNativeAPI() = delete;
    ~ArkDebugNativeAPI() = delete;

    ArkDebugNativeAPI(const ArkDebugNativeAPI &) = delete;
    void operator=(const ArkDebugNativeAPI &) = delete;
    ArkDebugNativeAPI(ArkDebugNativeAPI &&) = delete;
    ArkDebugNativeAPI &operator=(ArkDebugNativeAPI &&) = delete;

private:
    static void *gHybridDebuggerHandle_;
    static constexpr uint32_t DEFAULT_SAMPLE_INTERVAL_US = 500;
};

}  // namespace ark

#endif  // PANDA_DEBUGGER_ARKAPI_H

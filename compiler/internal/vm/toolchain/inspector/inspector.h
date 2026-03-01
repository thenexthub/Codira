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

#ifndef ARKCOMPILER_TOOLCHAIN_INSPECTOR_INSPECTOR_H
#define ARKCOMPILER_TOOLCHAIN_INSPECTOR_INSPECTOR_H

#include "inspector/ws_server.h"

#include <cstddef>
#include <string>

namespace panda::ecmascript {
class EcmaVM;
}  // namespace panda::ecmascript

namespace OHOS::ArkCompiler::Toolchain {
using EcmaVM = panda::ecmascript::EcmaVM;
using DebuggerPostTask = std::function<void(std::function<void()>&&)>;

#if __cplusplus
extern "C" {
#endif

struct DebugResponse {
    size_t size;
    char* response;
};

bool StartDebug(const std::string& componentName, void* vm, bool isDebugMode,
    int32_t instanceId, const DebuggerPostTask& debuggerPostTask, int port);

bool StartDebugForSocketpair(int tid, int socketfd, bool isHybrid = false);

bool InitializeDebuggerForSocketpair(void* vm, bool isHybrid = false);

void StopDebug(void* vm, bool isHybrid = false);

void StopOldDebug(int tid, const std::string& componentName);

void WaitForDebugger(void* vm);

int StartDebugger(uint32_t port, bool breakOnStart);

int StopDebugger();

void StoreDebuggerInfo(int tid, void* vm, const DebuggerPostTask& debuggerPostTask);

// The returned pointer must be released using free() after it is no longer needed.
// Failure to release the memory will result in memory leaks.
const char* GetJsBacktrace();

const char* OperateJsDebugMessage(const char* message);

// To enhance performance and maintain compatibility with older SDK versions,
// new interfaces have been introduced alongside existing ones for lldb invocation.
// These interfaces offer identical functionality, differing only in their return values.
DebugResponse GetJsBacktraceV1();

DebugResponse OperateJsDebugMessageV1(const char* message);
#if __cplusplus
}
#endif

class Inspector {
public:
    Inspector() = default;
    ~Inspector() = default;

    void OnMessage(std::string&& msg, bool isHybrid = false);
#if defined(OHOS_PLATFORM)
    static uint64_t GetThreadOrTaskId();
#endif // defined(OHOS_PLATFORM)

    static constexpr int32_t DELAY_CHECK_DISPATCH_STATUS = 100;

    pthread_t tid_ = 0;
    int tidForSocketPair_ = 0;
    void* vm_ = nullptr;
    std::unique_ptr<WsServer> websocketServer_;
    DebuggerPostTask debuggerPostTask_;
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_INSPECTOR_INSPECTOR_H
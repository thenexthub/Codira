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

#ifndef ECMASCRIPT_TOOLING_CLIENT_TCPSERVER_H
#define ECMASCRIPT_TOOLING_CLIENT_TCPSERVER_H

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <securec.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <uv.h>
#include <vector>

#include "arkcompiler/toolchain/common/log_wrapper.h"

namespace OHOS::ArkCompiler::Toolchain {
extern uv_async_t* g_inputSignal;
extern uv_async_t* g_releaseHandle;
class TcpServer {
public:
    static TcpServer& getInstance()
    {
        static TcpServer instance;
        return instance;
    }

    int CreateTcpServer([[maybe_unused]] void* arg);
    void StartTcpServer([[maybe_unused]] void* arg);
    void TcpServerWrite(std::string msg = "inner");
    bool IsServerActive() const
    {
        return isServerActive;
    }

    void SetServerState(const bool state)
    {
        isServerActive = state;
    }

private:
    TcpServer() = default;
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    void SendCommand(std::string inputStr);
    void CloseServer();
    void ServerConnect();
    bool FindCommand(std::vector<std::string> cmds, std::string cmd);

    uv_thread_t serverTid_ = 0;
    bool isServerActive = false;
    int lfd = -1;
    int cfd = -1;
};
} // namespace OHOS::ArkCompiler::Toolchain
#endif // ECMASCRIPT_TOOLING_CLIENT_TCPSERVER_H
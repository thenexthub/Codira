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

#ifndef ARKCOMPILER_TOOLCHAIN_CONNECT_SERVER_CONNECT_SERVER_H
#define ARKCOMPILER_TOOLCHAIN_CONNECT_SERVER_CONNECT_SERVER_H

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#ifdef WINDOWS_PLATFORM
#include <pthread.h>
#endif
#include <string>

namespace OHOS::ArkCompiler::Toolchain {
class WebSocketServer;

class ConnectServer {
public:
    ConnectServer(int socketfd, std::function<void(std::string&&)> onMessage);
    ConnectServer(const std::string& bundleName, std::function<void(std::string&&)> onMessage);
    ~ConnectServer();
    void RunServer();
    void StopServer();
    void SendMessage(const std::string& message) const;

private:
    std::atomic<bool> terminateExecution_ = false;
    [[maybe_unused]] int socketfd_ {-2};
    [[maybe_unused]] std::string bundleName_;
    pthread_t tid_ {0};
    std::function<void(std::string&&)> wsOnMessage_ {};
    std::unique_ptr<WebSocketServer> webSocket_ { nullptr };
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_CONNECT_SERVER_CONNECT_INSPECTOR_H

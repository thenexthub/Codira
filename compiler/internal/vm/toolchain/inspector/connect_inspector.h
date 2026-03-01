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

#ifndef ARKCOMPILER_TOOLCHAIN_INSPECTOR_CONNECT_INSPECTOR_H
#define ARKCOMPILER_TOOLCHAIN_INSPECTOR_CONNECT_INSPECTOR_H

#include <queue>
#include <string>
#include <unordered_map>
#include "connect_server.h"

namespace OHOS::ArkCompiler::Toolchain {
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// old process.
void StartServer(const std::string& componentName);

// socketpair process.
bool StartServerForSocketPair(int socketfd);

void StopServer([[maybe_unused]] const std::string& componentName);

void SendMessage(const std::string& message);

void StoreMessage(int32_t instanceId, const std::string& message);

void SetConnectCallback(const std::function<void(bool)>& callback);

void RemoveMessage(int32_t instanceId);

bool WaitForConnection();

void SetDebugModeCallBack(const std::function<void()>& setDebugMode);

void SetSwitchCallBack(const std::function<void(int32_t)>& createLayoutInfo, int32_t instanceId);

void SetProfilerCallback(const std::function<void(bool)> &setArkUIStateProfilerStatus);

void SetRecordCallback(const std::function<void(void)> &startRecordFunc,
    const std::function<void(void)> &stopRecordFunc);

/**
 * @brief set connect server message handler callback from ArkUI.
 * @param arkUICallback message handler callback of ArkUI.
 *     @param paramOfCallback message recvived from IDE.
 */
void SetArkUICallback(const std::function<void(const char *)> &arkUICallback);

/**
 * @brief set connect server message handler callback from WMS.
 * @param wMSCallback message handler callback of WMS.
 *     @param paramOfCallback message recvived from IDE.
 */
void SetWMSCallback(const std::function<void(const char *)> &wMSCallback);

/**
 * @brief set connect server message handler callback from cangjie.
 */
using SendMsgCB = const std::function<void(const std::string& message)>;
using CODECallback = const std::function<void(const std::string& message, SendMsgCB)>;
void SetCodiraCallback(CODECallback &cangjieCallback);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

class ConnectInspector {
public:
    ConnectInspector() = default;
    ~ConnectInspector() = default;

    std::string componentName_;
    std::unordered_map<int32_t, std::string> infoBuffer_;
    std::unique_ptr<ConnectServer> connectServer_;
    std::atomic<bool> waitingForDebugger_ = true;
    std::function<void(bool)> setArkUIStateProfilerStatus_;
    std::function<void(int32_t)> createLayoutInfo_;
    std::function<void()> setDebugMode_;
    int32_t instanceId_ = -1;
    std::function<void(void)> startRecord_;
    std::function<void(void)> stopRecord_;
    bool isRecording_ = false;
    std::function<void(const char *)> arkUICallback_;
    std::function<void(const char *)> wMSCallback_;
    std::function<void(const std::string& message, SendMsgCB)> cangjieCallback_;
};

class ConnectRequest {
public:
    explicit ConnectRequest(const std::string &message);
    ~ConnectRequest() = default;

    bool IsValid() const
    {
        return isSuccess_;
    }
    
    const std::string &GetDomain() const
    {
        return domain_;
    }

private:
    std::string domain_ {};
    bool isSuccess_ {false};
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_CONNECT_SERVER_CONNECT_INSPECTOR_H

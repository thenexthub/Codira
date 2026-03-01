/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tooling/dynamic/client/session/session.h"

#include "tooling/dynamic/client/manager/message_manager.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
uv_async_t* g_socketSignal;
void SocketMessageThread(void *arg)
{
    int sessionId = *(uint32_t *)arg;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId);
        return;
    }

    session->SocketMessageLoop();
}

Session::Session(uint32_t sessionId, std::string& sockInfo)
    : sessionId_(sessionId), sockInfo_(sockInfo), domainManager_(sessionId), breakpoint_(sessionId),
      stackManager_(sessionId), variableManager_(sessionId), sourceManager_(sessionId), watchManager_(sessionId)
{
}

void Session::SocketMessageLoop()
{
    while (cliSocket_.IsConnected()) {
        std::string decMessage = cliSocket_.Decode();
        uint32_t len = decMessage.length();
        if (len == 0) {
            continue;
        }
        LOGI("arkdb [%{public}u] message = %{public}s", sessionId_, decMessage.c_str());

        MessageManager::getInstance().MessagePush(sessionId_, decMessage);

        if (uv_is_active(reinterpret_cast<uv_handle_t*>(g_socketSignal))) {
            uv_async_send(g_socketSignal);
        }
    }
}

int Session::CreateSocket()
{
    uint32_t port = 0;
    if (Utils::StrToUInt(sockInfo_.c_str(), &port)) {
        if ((port <= 0) || (port >= 65535)) { // 65535: max port
            LOGE("arkdb:InitToolchainWebSocketForPort the port = %{public}d is wrong.", port);
            return -1;
        }
        if (!cliSocket_.InitToolchainWebSocketForPort(port, 5)) { // 5: five times
            LOGE("arkdb:InitToolchainWebSocketForPort failed");
            return -1;
        }
    } else {
        if (!cliSocket_.InitToolchainWebSocketForSockName(sockInfo_)) {
            LOGE("arkdb:InitToolchainWebSocketForSockName failed");
            return -1;
        }
    }

    if (!cliSocket_.ClientSendWSUpgradeReq()) {
        LOGE("arkdb:ClientSendWSUpgradeReq failed");
        return -1;
    }
    if (!cliSocket_.ClientRecvWSUpgradeRsp()) {
        LOGE("arkdb:ClientRecvWSUpgradeRsp failed");
        return -1;
    }

    return 0;
}

int Session::Start()
{
    if (CreateSocket()) {
        return -1;
    }

    uv_thread_create(&socketTid_, SocketMessageThread, &sessionId_);
    return 0;
}

int Session::Stop()
{
    cliSocket_.Close();
    return 0;
}

int SessionManager::CreateSessionById(uint32_t sessionId, std::string& sockInfo)
{
    sessions_[sessionId] = std::make_unique<Session>(sessionId, sockInfo);
    if (sessions_[sessionId]->Start()) {
        sessions_[sessionId] = nullptr;
        return -1;
    }

    return 0;
}

int SessionManager::CreateNewSession(std::string& sockInfo)
{
    uint32_t sessionId = MAX_SESSION_NUM;
    for (uint32_t i = 0; i < MAX_SESSION_NUM; ++i) {
        if (sessions_[i] == nullptr) {
            if (sessionId == MAX_SESSION_NUM) {
                sessionId = i;
            }
            continue;
        }
        if (sessions_[i]->GetSockInfo() == sockInfo) {
            return -1;
        }
    }

    if (sessionId < MAX_SESSION_NUM) {
        return CreateSessionById(sessionId, sockInfo);
    }

    return -1;
}

int SessionManager::CreateDefaultSession(std::string& sockInfo)
{
    return CreateSessionById(0, sockInfo);
}

int SessionManager::DelSessionById(uint32_t sessionId)
{
    Session *session = GetSessionById(sessionId);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId);
        return -1;
    }
    session->Stop();
    sessions_[sessionId] = nullptr;

    if (sessionId == currentSessionId_) {
        currentSessionId_ = 0;
        std::cout << "session switch to 0" << std::endl;
    }

    return 0;
}

int SessionManager::SessionList()
{
    for (uint32_t i = 0; i < MAX_SESSION_NUM; ++i) {
        if (sessions_[i] != nullptr) {
            std::string flag = (i == currentSessionId_) ? "* " : "  ";
            std::string sockState = sessions_[i]->GetSocketStateString();
            std::cout << flag << i << ": ";
            std::cout << std::setw(32) << std::left << sessions_[i]->GetSockInfo(); // 32: max length of socket info
            std::cout << sockState << std::endl;
        }
    }
    return 0;
}

int SessionManager::SessionSwitch(uint32_t sessionId)
{
    Session *session = GetSessionById(sessionId);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId);
        return -1;
    }
    currentSessionId_ = sessionId;
    return 0;
}

void SessionManager::CmdForAllSessions(CmdForAllCB callback)
{
    for (uint32_t sessionId = 0; sessionId < MAX_SESSION_NUM; ++sessionId) {
        if (sessions_[sessionId] != nullptr) {
            std::cout << "Executing command in session " << sessionId << ":" << std::endl;
            callback(sessionId);
        }
    }
}

int SessionManager::CreateTestSession(std::string& sockInfo)
{
    uint32_t sessionId = 0;
    sessions_[sessionId] = std::make_unique<Session>(sessionId, sockInfo);
    if (sessions_[sessionId]->CreateSocket()) {
        sessions_[sessionId] = nullptr;
        return -1;
    }

    return 0;
}
}
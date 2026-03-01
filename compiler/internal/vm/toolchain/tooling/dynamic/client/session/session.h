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

#ifndef ECMASCRIPT_TOOLING_CLIENT_SESSION_H
#define ECMASCRIPT_TOOLING_CLIENT_SESSION_H

#include <atomic>
#include <iostream>
#include <memory>
#include <map>
#include <pthread.h>
#include <thread>
#include <uv.h>

#include "tooling/dynamic/client/manager/domain_manager.h"
#include "tooling/dynamic/client/manager/breakpoint_manager.h"
#include "tooling/dynamic/client/manager/source_manager.h"
#include "tooling/dynamic/client/manager/stack_manager.h"
#include "tooling/dynamic/client/manager/variable_manager.h"
#include "tooling/dynamic/client/manager/watch_manager.h"
#include "websocket/client/websocket_client.h"

namespace OHOS::ArkCompiler::Toolchain {
using CmdForAllCB = std::function<void(uint32_t sessionId)>;
extern uv_async_t* g_socketSignal;
class Session {
public:
    explicit Session(uint32_t sessionId, std::string& sockInfo);
    ~Session()
    {
        Stop();
    }

    void SocketMessageLoop();
    int CreateSocket();
    int Start();
    int Stop();
    void CmdForAllSession(CmdForAllCB callback);

    uint32_t GetMessageId()
    {
        return messageId_.fetch_add(1);
    }

    std::string& GetSockInfo()
    {
        return sockInfo_;
    }

    bool ClientSendReq(const std::string &message)
    {
        return cliSocket_.SendReply(message);
    }

    DomainManager& GetDomainManager()
    {
        return domainManager_;
    }

    BreakPointManager& GetBreakPointManager()
    {
        return breakpoint_;
    }

    StackManager& GetStackManager()
    {
        return stackManager_;
    }

    VariableManager& GetVariableManager()
    {
        return variableManager_;
    }

    WebSocketClient& GetWebSocketClient()
    {
        return cliSocket_;
    }

    ProfilerSingleton& GetProfilerSingleton()
    {
        return profiler_;
    }

    std::string GetSocketStateString()
    {
        return cliSocket_.GetSocketStateString();
    }

    void ProcSocketMsg(char* msg)
    {
        domainManager_.DispatcherReply(msg);
    }

    SourceManager& GetSourceManager()
    {
        return sourceManager_;
    }

    WatchManager& GetWatchManager()
    {
        return watchManager_;
    }

private:
    uint32_t sessionId_;
    std::string sockInfo_;
    DomainManager domainManager_;
    WebSocketClient cliSocket_;
    uv_thread_t socketTid_ = 0;
    std::atomic<uint32_t> messageId_ {1};
    BreakPointManager breakpoint_;
    StackManager stackManager_;
    VariableManager variableManager_;
    ProfilerSingleton profiler_;
    SourceManager sourceManager_;
    WatchManager watchManager_;
};

constexpr uint32_t MAX_SESSION_NUM = 8;
class SessionManager {
public:
    static SessionManager& getInstance()
    {
        static SessionManager instance;
        return instance;
    }

    uint32_t GetCurrentSessionId()
    {
        return currentSessionId_;
    }

    Session *GetCurrentSession()
    {
        if (currentSessionId_ >= MAX_SESSION_NUM || sessions_[currentSessionId_] == nullptr) {
            return nullptr;
        }
        return sessions_[currentSessionId_].get();
    }

    Session *GetSessionById(uint32_t sessionId)
    {
        if (sessionId >= MAX_SESSION_NUM || sessions_[sessionId] == nullptr) {
            return nullptr;
        }
        return sessions_[sessionId].get();
    }

    int CreateSessionById(uint32_t sessionId, std::string& sockInfo);
    int CreateNewSession(std::string& sockInfo);
    int CreateDefaultSession(std::string& sockInfo);
    int DelSessionById(uint32_t sessionId);
    int SessionList();
    int SessionSwitch(uint32_t sessionId);
    void CmdForAllSessions(CmdForAllCB callback);
    int CreateTestSession(std::string& sockInfo);

private:
    SessionManager() = default;
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    std::unique_ptr<Session> sessions_[MAX_SESSION_NUM];
    uint32_t currentSessionId_ = 0;
};
} // namespace OHOS::ArkCompiler::Toolchain
#endif
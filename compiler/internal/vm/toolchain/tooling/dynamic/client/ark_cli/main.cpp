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

#include "tooling/dynamic/client/utils/cli_command.h"
#include "tooling/dynamic/client/tcpServer/tcp_server.h"
#include "manager/message_manager.h"

namespace OHOS::ArkCompiler::Toolchain {
uv_loop_t* g_loop;

void ReleaseHandle([[maybe_unused]] uv_async_t *releaseHandle)
{
    uv_close(reinterpret_cast<uv_handle_t*>(g_inputSignal), [](uv_handle_t* handle) {
        if (handle != nullptr) {
            uv_async_t* asyncHandle = reinterpret_cast<uv_async_t*>(handle);
            if (asyncHandle->data != nullptr) {
                free(asyncHandle->data);
                asyncHandle->data = nullptr;
            }
            delete asyncHandle;
            asyncHandle = nullptr;
            g_inputSignal = nullptr;
        }
    });

    uv_close(reinterpret_cast<uv_handle_t*>(g_socketSignal), [](uv_handle_t* handle) {
        if (handle != nullptr) {
            delete reinterpret_cast<uv_async_t*>(handle);
            handle = nullptr;
            g_socketSignal = nullptr;
        }
    });

    uv_close(reinterpret_cast<uv_handle_t*>(g_releaseHandle), [](uv_handle_t* handle) {
        if (handle != nullptr) {
            delete reinterpret_cast<uv_async_t*>(handle);
            handle = nullptr;
            g_releaseHandle = nullptr;
        }
    });

    if (g_loop != nullptr) {
        uv_stop(g_loop);
    }
}

void InputMessageInSession(uint32_t sessionId, std::vector<std::string>& cliCmdStr)
{
    CliCommand cmd(cliCmdStr, sessionId);
    cmd.ExecCommand();
    return;
}

void InputOnMessage(uv_async_t *handle)
{
    char* msg = static_cast<char*>(handle->data);
    std::string inputStr = std::string(msg);
    if (msg != nullptr) {
        free(msg);
    }
    std::vector<std::string> cliCmdStr = Utils::SplitString(inputStr, " ");
    if (cliCmdStr[0] == "forall") {
        if (strstr(cliCmdStr[1].c_str(), "session") != nullptr) {
            std::cout << "command " << cliCmdStr[1] << " not support forall" << std::endl;
        } else {
            cliCmdStr.erase(cliCmdStr.begin());
            SessionManager::getInstance().CmdForAllSessions(std::bind(InputMessageInSession, std::placeholders::_1,
                                                                      cliCmdStr));
        }
    } else {
        uint32_t sessionId = SessionManager::getInstance().GetCurrentSessionId();
        InputMessageInSession(sessionId, cliCmdStr);
    }

    if (TcpServer::getInstance().IsServerActive()) {
        TcpServer::getInstance().TcpServerWrite("InputOnMessage");
    } else {
        std::cout << ">>> ";
        fflush(stdout);
    }
}

void GetInputCommand([[maybe_unused]] void *arg)
{
    std::cout << ">>> ";
    std::string inputStr;
    while (getline(std::cin, inputStr)) {
        inputStr.erase(0, inputStr.find_first_not_of(" "));
        if (inputStr.empty()) {
            std::cout << ">>> ";
            continue;
        }
        if ((!strcmp(inputStr.c_str(), "quit")) || (!strcmp(inputStr.c_str(), "q"))) {
            LOGE("arkdb: quit");
            if (uv_is_active(reinterpret_cast<uv_handle_t*>(g_releaseHandle))) {
                uv_async_send(g_releaseHandle);
            }
            break;
        }
        if (uv_is_active(reinterpret_cast<uv_handle_t*>(g_inputSignal))) {
            uint32_t len = inputStr.length();
            char* msg = (char*)malloc(len + 1);
            if (msg == nullptr) {
                continue;
            }
            if (strncpy_s(msg, len + 1, inputStr.c_str(), len) != 0) {
                if (uv_is_active(reinterpret_cast<uv_handle_t*>(g_releaseHandle))) {
                    uv_async_send(g_releaseHandle);
                }
                free(msg);
                break;
            }
            g_inputSignal->data = std::move(msg);
            uv_async_send(g_inputSignal);
        }
    }
}

void SocketOnMessage([[maybe_unused]] uv_async_t *handle)
{
    uint32_t sessionId = 0;
    std::string message;
    while (MessageManager::getInstance().MessagePop(sessionId, message)) {
        Session *session = SessionManager::getInstance().GetSessionById(sessionId);
        if (session == nullptr) {
            LOGE("arkdb get session by id %{public}u failed", sessionId);
            continue;
        }

        session->ProcSocketMsg(const_cast<char *>(message.c_str()));
    }

    if (TcpServer::getInstance().IsServerActive()) {
        TcpServer::getInstance().TcpServerWrite("SocketOnMessage");
    }
}

int Main(const int argc, const char** argv)
{
    if (argc < 2) { // 2: two parameters
        LOGE("arkdb is missing a parameter");
        return -1;
    }
    if (strstr(argv[0], "arkdb") != nullptr) {
        std::string sockInfo(argv[1]);
        if (SessionManager::getInstance().CreateDefaultSession(sockInfo)) {
            LOGE("arkdb create default session failed");
            return -1;
        }

        g_loop = uv_default_loop();

        g_inputSignal = new uv_async_t;
        uv_async_init(g_loop, g_inputSignal, reinterpret_cast<uv_async_cb>(InputOnMessage));

        g_socketSignal = new uv_async_t;
        uv_async_init(g_loop, g_socketSignal, reinterpret_cast<uv_async_cb>(SocketOnMessage));

        g_releaseHandle = new uv_async_t;
        uv_async_init(g_loop, g_releaseHandle, reinterpret_cast<uv_async_cb>(ReleaseHandle));

        if (argc > 2 && strcmp(argv[2], "server") == 0) { // 2: two parameters
            if (TcpServer::getInstance().CreateTcpServer(argv)) {
                LOGE("arkdb create TcpServer failed");
                return -1;
            }
        } else {
            uv_thread_t inputTid;
            uv_thread_create(&inputTid, GetInputCommand, nullptr);
        }

        uv_run(g_loop, UV_RUN_DEFAULT);
    }
    return 0;
}
} // OHOS::ArkCompiler::Toolchain

int main(int argc, const char **argv)
{
    return OHOS::ArkCompiler::Toolchain::Main(argc, argv);
}

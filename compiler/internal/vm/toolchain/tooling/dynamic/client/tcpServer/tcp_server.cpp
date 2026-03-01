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
#include "tooling/dynamic/client/tcpServer/tcp_server.h"

#include "platform/file.h"
#include "tooling/dynamic/utils/utils.h"

namespace OHOS::ArkCompiler::Toolchain {
uv_async_t* g_inputSignal;
uv_async_t* g_releaseHandle;
std::string g_inputStr = "";
std::vector<std::string> g_allCmds = { "allocationtrack", "at", "allocationtrack-stop", "at-stop", "heapdump", "hd",
    "heapprofiler-enable", "hp-enable", "heapprofiler-disable", "hp-disable", "sampling", "sampling", "sampling-stop",
    "sampling-stop", "collectgarbage", "gc", "cpuprofile", "cp", "cpuprofile-stop", "cp-stop", "cpuprofile-enable",
    "cp-enable", "cpuprofile-disable", "cp-disable", "cpuprofile-show", "cp-show", "cpuprofile-setSamplingInterval",
    "cp-ssi", "runtime-enable", "rt-enable", "heapusage", "hu", "break", "b", "backtrack", "bt", "continue", "c",
    "delete", "d", "disable", "disable", "display", "display", "enable", "enable", "finish", "fin", "frame", "f",
    "help", "h", "ignore", "ig", "infobreakpoints", "infob", "infosource", "infos", "jump", "j", "list", "l", "next",
    "n", "print", "p", "ptype", "ptype", "run", "r", "setvar", "sv", "step", "s", "undisplay", "undisplay", "watch",
    "wa", "resume", "resume", "showstack", "ss", "step-into", "si", "step-out", "so", "step-over", "sov",
    "runtime-disable", "rt-disable" };
std::vector<std::string> g_noRecvCmds = { "cpuprofile-enable", "cp-enable", "cpuprofile-disable", "cp-disable",
    "cpuprofile-stop", "cp-stop" };
std::vector<std::string> g_inputOnMessages = { "b", "break", "bt", "backtrack", "d", "delete", "display", "fin",
    "finish", "f", "frame", "h", "help", "ig", "ignore", "infob", "infobreakpoints", "infos", "infosource", "j", "jump",
    "l", "list", "n", "next", "ptype", "s", "step", "ss", "showstack", "watch", "wa" };

void CreateServer(void* arg)
{
    TcpServer::getInstance().StartTcpServer(arg);
}

void TcpServer::CloseServer()
{
    if (uv_is_active(reinterpret_cast<uv_handle_t*>(g_releaseHandle))) {
        uv_async_send(g_releaseHandle);
    }
}

void TcpServer::ServerConnect()
{
    isServerActive = 1;
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(9999); // 9999: tcp bind port

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        std::cout << "socket failed" << std::endl;
        CloseServer();
        return;
    }
    FdsanExchangeOwnerTag(reinterpret_cast<fd_t>(lfd));

    int ret = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        std::cout << "bind failed" << std::endl;
        CloseServer();
        return;
    }

    ret = listen(lfd, 6); // 6: Number of backlogs
    if (ret == -1) {
        std::cout << "listen failed" << std::endl;
        CloseServer();
        return;
    }

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    cfd = accept(lfd, (struct sockaddr*)&clientaddr, &len);
    if (cfd == -1) {
        std::cout << "accept failed" << std::endl;
        CloseServer();
        return;
    }
    FdsanExchangeOwnerTag(reinterpret_cast<fd_t>(cfd));
}

void TcpServer::SendCommand(std::string inputStr)
{
    inputStr.erase(0, inputStr.find_first_not_of(" "));
    if (inputStr.empty()) {
        std::cout << "cmd is empty" << std::endl;
        return;
    }

    if (uv_is_active(reinterpret_cast<uv_handle_t*>(g_inputSignal))) {
        uint32_t len = inputStr.length();
        if (len < 0) {
            CloseServer();
            return;
        }
        char* msg = (char*)malloc(len + 1);
        if (msg == nullptr) {
            return;
        }
        if (strncpy_s(msg, len + 1, inputStr.c_str(), len) != 0) {
            free(msg);
            CloseServer();
            return;
        }
        g_inputSignal->data = std::move(msg);
        uv_async_send(g_inputSignal);
    }
    return;
}

void TcpServer::StartTcpServer([[maybe_unused]] void* arg)
{
    ServerConnect();

    const char* data = "connect success";
    write(cfd, data, strlen(data));
    int num = 0;
    do {
        char recvBuf[1024] = { 0 };
        num = read(cfd, recvBuf, sizeof(recvBuf));
        if (num < 0) {
            std::cout << "read failed" << std::endl;
        } else if (num > 0) {
            g_inputStr = std::string(recvBuf);
            SendCommand(recvBuf);
            TcpServerWrite();
        } else if (num == 0) {
            std::cout << "clinet closed" << std::endl;
        }
    } while (num > 0);

    FdsanClose(reinterpret_cast<fd_t>(cfd));
    FdsanClose(reinterpret_cast<fd_t>(lfd));

    CloseServer();
    return;
}

void TcpServer::TcpServerWrite(std::string msg)
{
    if (g_inputStr.empty()) {
        return;
    }

    std::vector<std::string> cliCmdStr = Utils::SplitString(g_inputStr, " ");
    if (!FindCommand(g_allCmds, cliCmdStr[0])) {
        g_inputStr = "error cmd";
        write(cfd, g_inputStr.c_str(), strlen(g_inputStr.c_str()));
        g_inputStr.clear();
        return;
    }
    if (msg == "inner") {
        if (FindCommand(g_noRecvCmds, cliCmdStr[0])) {
            write(cfd, g_inputStr.c_str(), strlen(g_inputStr.c_str()));
            g_inputStr.clear();
        }
    } else if (msg == "InputOnMessage") {
        if (FindCommand(g_inputOnMessages, cliCmdStr[0])) {
            write(cfd, g_inputStr.c_str(), strlen(g_inputStr.c_str()));
            g_inputStr.clear();
        }
    } else if (msg == "SocketOnMessage") {
        if (FindCommand(g_inputOnMessages, cliCmdStr[0])) {
            return;
        }
        if (g_inputStr.empty()) {
            return;
        }
        write(cfd, g_inputStr.c_str(), strlen(g_inputStr.c_str()));
        g_inputStr.clear();
    }
    return;
}

int TcpServer::CreateTcpServer([[maybe_unused]] void* arg)
{
    uv_thread_create(&serverTid_, CreateServer, nullptr);
    return 0;
}

bool TcpServer::FindCommand(std::vector<std::string> cmds, std::string cmd)
{
    return find(cmds.begin(), cmds.end(), cmd) != cmds.end();
}
} // namespace OHOS::ArkCompiler::Toolchain
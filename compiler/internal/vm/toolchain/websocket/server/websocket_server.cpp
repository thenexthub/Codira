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

#if defined(PANDA_TARGET_WINDOWS)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <fcntl.h>
#include "common/log_wrapper.h"
#include "platform/file.h"
#include "frame_builder.h"
#include "handshake_helper.h"
#include "server/websocket_server.h"
#include "string_utils.h"

#include <mutex>
#include <thread>

namespace OHOS::ArkCompiler::Toolchain {
static bool ValidateHandShakeMessage(const HttpRequest& req)
{
    std::string upgradeHeaderValue = req.upgrade;
    // Switch to lower case in order to support obsolete versions of WebSocket protocol.
    ToLowerCase(upgradeHeaderValue);
    return req.connection.find("Upgrade") != std::string::npos &&
        upgradeHeaderValue.find("websocket") != std::string::npos &&
        req.version.compare("HTTP/1.1") == 0;
}

WebSocketServer::~WebSocketServer() noexcept
{
    if (serverFd_ != -1) {
        LOGW("WebSocket server is closed while destructing the object");
        FdsanClose(reinterpret_cast<fd_t>(serverFd_));
        // Reset directly in order to prevent static analyzer warnings.
        serverFd_ = -1;
    }
}

bool WebSocketServer::DecodeMessage(WebSocketFrame& wsFrame) const
{
    const uint64_t msgLen = wsFrame.payloadLen;
    if (msgLen == 0) {
        // receiving empty data is OK
        return true;
    }
    auto& buffer = wsFrame.payload;
    buffer.resize(msgLen, 0);

    if (!RecvUnderLock(wsFrame.maskingKey, sizeof(wsFrame.maskingKey))) {
        LOGE("DecodeMessage: Recv maskingKey failed");
        return false;
    }

    if (!RecvUnderLock(buffer)) {
        LOGE("DecodeMessage: Recv message with mask failed");
        return false;
    }

    for (uint64_t i = 0; i < msgLen; i++) {
        const auto j = i % WebSocketFrame::MASK_LEN;
        buffer[i] = static_cast<uint8_t>(buffer[i]) ^ wsFrame.maskingKey[j];
    }

    return true;
}

bool WebSocketServer::ProtocolUpgrade(const HttpRequest& req)
{
    unsigned char encodedKey[WebSocketKeyEncoder::ENCODED_KEY_LEN + 1];
    if (!WebSocketKeyEncoder::EncodeKey(req.secWebSocketKey, encodedKey)) {
        LOGE("ProtocolUpgrade: failed to encode WebSocket-Key");
        return false;
    }

    ProtocolUpgradeBuilder requestBuilder(encodedKey);
    if (!SendUnderLock(requestBuilder.GetUpgradeMessage(), requestBuilder.GetLength())) {
        LOGE("ProtocolUpgrade: Send failed");
        return false;
    }
    return true;
}

bool WebSocketServer::ResponseInvalidHandShake() const
{
    const std::string response(BAD_REQUEST_RESPONSE);
    return SendUnderLock(response);
}

bool WebSocketServer::HttpHandShake()
{
    std::string msgBuf(HTTP_HANDSHAKE_MAX_LEN, 0);
    ssize_t msgLen = 0;
    {
        std::shared_lock lock(GetConnectionMutex());
        while ((msgLen = recv(GetConnectionSocket(), msgBuf.data(), HTTP_HANDSHAKE_MAX_LEN, 0)) < 0 &&
            (errno == EINTR || errno == EAGAIN)) {
            LOGW("HttpHandShake recv failed, errno = %{public}d", errno);
        }
    }
    if (msgLen <= 0) {
        LOGE("ReadMsg failed, msgLen = %{public}ld, errno = %{public}d", static_cast<long>(msgLen), errno);
        return false;
    }
    // reduce to received size
    msgBuf.resize(msgLen);

    HttpRequest req;
    if (!HttpRequest::Decode(msgBuf, req)) {
        LOGE("HttpHandShake: Upgrade failed");
        return false;
    }
    if (validateCb_ && !validateCb_(req)) {
        LOGE("HttpHandShake: Validation failed");
        return false;
    }

    if (ValidateHandShakeMessage(req)) {
        return ProtocolUpgrade(req);
    }

    LOGE("HttpHandShake: HTTP upgrade parameters failure");
    if (!ResponseInvalidHandShake()) {
        LOGE("HttpHandShake: failed to send 'bad request' response");
    }
    return false;
}

bool WebSocketServer::MoveToConnectingState()
{
    if (!serverUp_.load()) {
        // Server `Close` happened, must not accept new connections.
        return false;
    }
    auto expected = ConnectionState::CLOSED;
    if (!CompareExchangeConnectionState(expected, ConnectionState::CONNECTING)) {
        switch (expected) {
            case ConnectionState::CLOSING:
                LOGW("MoveToConnectingState during closing connection");
                break;
            case ConnectionState::OPEN:
                LOGW("MoveToConnectingState during already established connection");
                break;
            case ConnectionState::CONNECTING:
                LOGE("MoveToConnectingState during opening connection, which violates WebSocketServer guarantees");
                break;
            default:
                break;
        }
        return false;
    }
    // Must check once again because of `Close` method implementation.
    if (!serverUp_.load()) {
        // Server `Close` happened, `serverFd_` was closed, new connection must not be opened.
        expected = SetConnectionState(ConnectionState::CLOSED);
        if (expected != ConnectionState::CONNECTING) {
            LOGE("AcceptNewConnection: Close guarantees violated");
        }
        return false;
    }
    return true;
}

bool WebSocketServer::AcceptNewConnection()
{
    if (!MoveToConnectingState()) {
        return false;
    }

    const int newConnectionFd = accept(serverFd_, nullptr, nullptr);
    if (newConnectionFd < SOCKET_SUCCESS) {
        LOGI("AcceptNewConnection accept has exited");

        auto expected = SetConnectionState(ConnectionState::CLOSED);
        if (expected != ConnectionState::CONNECTING) {
            LOGE("AcceptNewConnection: violation due to concurrent close and accept: got %{public}d",
                 EnumToNumber(expected));
        }
        return false;
    }
    {
        std::unique_lock lock(GetConnectionMutex());
        SetConnectionSocket(newConnectionFd);
    }

    if (!HttpHandShake()) {
        LOGW("AcceptNewConnection HttpHandShake failed");

        auto expected = SetConnectionState(ConnectionState::CLOSING);
        if (expected != ConnectionState::CONNECTING) {
            LOGE("AcceptNewConnection: violation due to concurrent close and accept: got %{public}d",
                 EnumToNumber(expected));
        }
        CloseConnectionSocket(ConnectionCloseReason::FAIL);
        return false;
    }

    OnNewConnection();
    return true;
}

bool WebSocketServer::InitTcpWebSocket(int port, uint32_t timeoutLimit)
{
    if (port < 0) {
        LOGE("InitTcpWebSocket invalid port");
        return false;
    }
    if (serverUp_.load()) {
        LOGI("InitTcpWebSocket websocket has inited");
        return true;
    }
#if defined(WINDOWS_PLATFORM)
    WORD sockVersion = MAKEWORD(2, 2); // 2: version 2.2
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0) {
        LOGE("InitTcpWebSocket WSA init failed");
        return false;
    }
#endif
    serverFd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverFd_ < SOCKET_SUCCESS) {
        LOGE("InitTcpWebSocket socket init failed, errno = %{public}d", errno);
        return false;
    }
    FdsanExchangeOwnerTag(reinterpret_cast<fd_t>(serverFd_));
    // allow specified port can be used at once and not wait TIME_WAIT status ending
    int sockOptVal = 1;
    if ((setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<char *>(&sockOptVal), sizeof(sockOptVal))) != SOCKET_SUCCESS) {
        LOGE("InitTcpWebSocket setsockopt SO_REUSEADDR failed, errno = %{public}d", errno);
        CloseServerSocket();
        return false;
    }
    if (!SetWebSocketTimeOut(serverFd_, timeoutLimit)) {
        LOGE("InitTcpWebSocket SetWebSocketTimeOut failed");
        CloseServerSocket();
        return false;
    }
    return BindAndListenTcpWebSocket(port);
}

bool WebSocketServer::BindAndListenTcpWebSocket(int port)
{
    sockaddr_in addrSin = {};
    addrSin.sin_family = AF_INET;
    addrSin.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &addrSin.sin_addr) != NET_SUCCESS) {
        LOGE("BindAndListenTcpWebSocket inet_pton failed, error = %{public}d", errno);
        return false;
    }
    if (bind(serverFd_, reinterpret_cast<struct sockaddr*>(&addrSin), sizeof(addrSin)) != SOCKET_SUCCESS ||
        listen(serverFd_, 1) != SOCKET_SUCCESS) {
        LOGE("BindAndListenTcpWebSocket bind/listen failed, errno = %{public}d", errno);
        CloseServerSocket();
        return false;
    }
    serverUp_.store(true);
    return true;
}

#if !defined(WINDOWS_PLATFORM)
bool WebSocketServer::InitUnixWebSocket(const std::string& sockName, uint32_t timeoutLimit)
{
    if (serverUp_.load()) {
        LOGI("InitUnixWebSocket websocket has inited");
        return true;
    }
    serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0); // 0: default protocol
    if (serverFd_ < SOCKET_SUCCESS) {
        LOGE("InitUnixWebSocket socket init failed, errno = %{public}d", errno);
        return false;
    }
    FdsanExchangeOwnerTag(reinterpret_cast<fd_t>(serverFd_));
    // set send and recv timeout
    if (!SetWebSocketTimeOut(serverFd_, timeoutLimit)) {
        LOGE("InitUnixWebSocket SetWebSocketTimeOut failed");
        CloseServerSocket();
        return false;
    }

    struct sockaddr_un un;
    if (memset_s(&un, sizeof(un), 0, sizeof(un)) != EOK) {
        LOGE("InitUnixWebSocket memset_s failed");
        CloseServerSocket();
        return false;
    }
    un.sun_family = AF_UNIX;
    if (strcpy_s(un.sun_path + 1, sizeof(un.sun_path) - 1, sockName.c_str()) != EOK) {
        LOGE("InitUnixWebSocket strcpy_s failed");
        CloseServerSocket();
        return false;
    }
    un.sun_path[0] = '\0';
    uint32_t len = offsetof(struct sockaddr_un, sun_path) + strlen(sockName.c_str()) + 1;
    if (bind(serverFd_, reinterpret_cast<struct sockaddr*>(&un), static_cast<int32_t>(len)) != SOCKET_SUCCESS) {
        LOGE("InitUnixWebSocket bind failed, errno = %{public}d", errno);
        CloseServerSocket();
        return false;
    }
    if (listen(serverFd_, 1) != SOCKET_SUCCESS) { // 1: connection num
        LOGE("InitUnixWebSocket listen failed, errno = %{public}d", errno);
        CloseServerSocket();
        return false;
    }
    serverUp_.store(true);
    return true;
}

bool WebSocketServer::InitUnixWebSocket(int socketfd)
{
    if (serverUp_.load()) {
        LOGI("InitUnixWebSocket websocket has inited");
        return true;
    }
    if (socketfd < SOCKET_SUCCESS) {
        LOGE("InitUnixWebSocket socketfd is invalid");
        return false;
    }
    SetConnectionSocket(socketfd);
    const int flag = fcntl(socketfd, F_GETFL, 0);
    if (flag == -1) {
        LOGE("InitUnixWebSocket get client state is failed, error is %{public}s", strerror(errno));
        return false;
    }
    fcntl(socketfd, F_SETFL, static_cast<size_t>(flag) & ~O_NONBLOCK);
    serverUp_.store(true);
    return true;
}

bool WebSocketServer::ConnectUnixWebSocketBySocketpair()
{
    if (!MoveToConnectingState()) {
        return false;
    }

    if (!HttpHandShake()) {
        LOGE("ConnectUnixWebSocket HttpHandShake failed");

        auto expected = SetConnectionState(ConnectionState::CLOSING);
        if (expected != ConnectionState::CONNECTING) {
            LOGE("ConnectUnixWebSocketBySocketpair: violation due to concurrent close and accept: got %{public}d",
                 EnumToNumber(expected));
        }
        CloseConnectionSocket(ConnectionCloseReason::FAIL);
        return false;
    }

    OnNewConnection();
    return true;
}
#endif  // WINDOWS_PLATFORM

void WebSocketServer::CloseServerSocket()
{
    FdsanClose(reinterpret_cast<fd_t>(serverFd_));
    serverFd_ = -1;
}

void WebSocketServer::OnNewConnection()
{
    LOGI("New client connected");
    if (openCb_) {
        openCb_();
    }

    auto expected = SetConnectionState(ConnectionState::OPEN);
    if (expected != ConnectionState::CONNECTING) {
        LOGE("OnNewConnection violation: expected CONNECTING, but got %{public}d",
             EnumToNumber(expected));
    }
}

void WebSocketServer::SetValidateConnectionCallback(ValidateConnectionCallback cb)
{
    validateCb_ = std::move(cb);
}

void WebSocketServer::SetOpenConnectionCallback(OpenConnectionCallback cb)
{
    openCb_ = std::move(cb);
}

bool WebSocketServer::ValidateIncomingFrame(const WebSocketFrame& wsFrame) const
{
    // "The server MUST close the connection upon receiving a frame that is not masked."
    // https://www.rfc-editor.org/rfc/rfc6455#section-5.1
    return wsFrame.mask == 1;
}

std::string WebSocketServer::CreateFrame(bool isLast, FrameType frameType) const
{
    ServerFrameBuilder builder(isLast, frameType);
    return builder.Build();
}

std::string WebSocketServer::CreateFrame(bool isLast, FrameType frameType, const std::string& payload) const
{
    ServerFrameBuilder builder(isLast, frameType);
    return builder.SetPayload(payload).Build();
}

std::string WebSocketServer::CreateFrame(bool isLast, FrameType frameType, std::string&& payload) const
{
    ServerFrameBuilder builder(isLast, frameType);
    return builder.SetPayload(std::move(payload)).Build();
}

WebSocketServer::ConnectionState WebSocketServer::WaitConnectingStateEnds(ConnectionState connection)
{
    auto shutdownSocketUnderLock = [this]() {
        auto fd = GetConnectionSocket();
        if (fd == -1) {
            return false;
        }
        int err = ShutdownSocket(fd);
        if (err != 0) {
            LOGW("Failed to shutdown client socket, errno = %{public}d", errno);
        }
        return true;
    };

    auto connectionSocketWasShutdown = false;
    while (connection == ConnectionState::CONNECTING) {
        if (!connectionSocketWasShutdown) {
            // Try to shutdown the already accepted connection socket,
            // otherwise thread can infinitely hang on handshake recv.
            std::shared_lock lock(GetConnectionMutex());
            connectionSocketWasShutdown = shutdownSocketUnderLock();
        }

        std::this_thread::yield();
        connection = GetConnectionState();
    }
    return connection;
}

void WebSocketServer::Close()
{
    // Firstly stop accepting new connections.
    if (!serverUp_.exchange(false)) {
        return;
    }

    int err = ShutdownSocket(serverFd_);
    if (err != 0) {
        LOGW("Failed to shutdown server socket, errno = %{public}d", errno);
    }

    // If there is a concurrent call to `CloseConnection`, we can immediately close `serverFd_`.
    // This is because new connections are already prevented, hence no reads of `serverFd_` will be done,
    // and the connection itself will be closed by someone else.
    auto connection = GetConnectionState();
    if (connection == ConnectionState::CLOSING || connection == ConnectionState::CLOSED) {
        CloseServerSocket();
        return;
    }

    connection = WaitConnectingStateEnds(connection);

    // Can safely close server socket, as there will be no more new connections attempts.
    CloseServerSocket();
    // Must check once again after finished `AcceptNewConnection`.
    if (connection == ConnectionState::CLOSING || connection == ConnectionState::CLOSED) {
        return;
    }

    // If we reached this point, connection can be `OPEN`, `CLOSING` or `CLOSED`. Try to close it anyway.
    CloseConnection(CloseStatusCode::SERVER_GO_AWAY);
}
} // namespace OHOS::ArkCompiler::Toolchain

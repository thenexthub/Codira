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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_SERVER_WEBSOCKET_SERVER_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_SERVER_WEBSOCKET_SERVER_H

#include "http.h"
#include "websocket_base.h"

#include <functional>

namespace OHOS::ArkCompiler::Toolchain {
class WebSocketServer final : public WebSocketBase {
public:
    using ValidateConnectionCallback = std::function<bool(const HttpRequest&)>;
    using OpenConnectionCallback = std::function<void()>;

public:
    ~WebSocketServer() noexcept override;

    /**
     * @brief Accept new posix-socket connection.
     * Safe to call concurrently with `Close`.
     * Must not be called concurrently with `SendReply` or `Decode`, as it might lead to race condition.
     */
    bool AcceptNewConnection();

    /**
     * @brief Initialize server posix-socket.
     * Non thread safe.
     * On success, `serverFd_` is initialized and `serverUp_` evaluates true.
     * @param port server TCP socket port number.
     * @param timeoutLimit timeout in seconds for server socket. If zero, timeout is not set.
     * @returns true on success, false otherwise.
     */
    bool InitTcpWebSocket(int port, uint32_t timeoutLimit = 0);

#if !defined(WINDOWS_PLATFORM)
    /**
     * @brief Initialize server unix-socket.
     * Non thread safe.
     * On success, `serverFd_` is initialized and `serverUp_` evaluates true.
     * @param sockName server socket name.
     * @param timeoutLimit timeout in seconds for server socket. If zero, timeout is not set.
     * @returns true on success, false otherwise.
     */
    bool InitUnixWebSocket(const std::string& sockName, uint32_t timeoutLimit = 0);

    /**
     * @brief Initialize connection with unix-socket.
     * Non thread safe.
     * On success, `serverFd_` stays uninitialized, but `serverUp_` evaluates true.
     * Note that this mode supports only a single connection,
     * which must be accepted by calling `ConnectUnixWebSocketBySocketpair`.
     * @param socketfd connection socket file descriptor, must be correctly opened before calling the method.
     * @returns true on success, false otherwise.
     */
    bool InitUnixWebSocket(int socketfd);

    /**
     * @brief Accept new unix-socket connection.
     * Safe to call concurrently with `Close`.
     */
    bool ConnectUnixWebSocketBySocketpair();
#endif  // WINDOWS_PLATFORM

    /**
     * @brief Set callback for calling after received HTTP handshake request.
     * Non thread safe.
     */
    void SetValidateConnectionCallback(ValidateConnectionCallback cb);

    /**
     * @brief Set callback for calling after accepted new connection.
     * Non thread safe.
     */
    void SetOpenConnectionCallback(OpenConnectionCallback cb);

    /**
     * @brief Close server endpoint and connection sockets.
     * Safe to call concurrently with:
     * `AcceptNewConnection`, `ConnectUnixWebSocketBySocketpair` `SendReply`, `Decode`, `CloseConnection`.
     */
    void Close();

private:
    bool BindAndListenTcpWebSocket(int port);

    bool ValidateIncomingFrame(const WebSocketFrame& wsFrame) const override;
    std::string CreateFrame(bool isLast, FrameType frameType) const override;
    std::string CreateFrame(bool isLast, FrameType frameType, const std::string& payload) const override;
    std::string CreateFrame(bool isLast, FrameType frameType, std::string&& payload) const override;
    bool DecodeMessage(WebSocketFrame& wsFrame) const override;

    bool HttpHandShake();
    bool ProtocolUpgrade(const HttpRequest& req);
    bool ResponseInvalidHandShake() const;

    /**
     * @brief Runs user-provided callback and performs transition from `CONNECTING` to `OPEN` state.
     */
    void OnNewConnection();

    void CloseServerSocket();

    /**
     * @brief Performs transition from `CLOSED` to `CONNECTING` state if the server is up.
     * @returns true on success, false otherwise.
     */
    bool MoveToConnectingState();

    /**
     * @brief Wait until concurrent `CONNECTING` state transition ends.
     * There might be a concurrent call to `AcceptNewConnection`;
     * in this case, it must finish and move into either:
     * - `CLOSED` (due to failed `accept` or handshake),
     * - `OPEN` (which can then concurrently transition to either `CLOSING` or `CLOSE`).
     * @param connection previously loaded connection state.
     * @returns updated connection state.
     */
    ConnectionState WaitConnectingStateEnds(ConnectionState connection);

private:
    // Server initialization status.
    // Note that there could be alive connections after `serverUp_` switched to false,
    // but they must terminate soon after. Users must track this with callbacks.
    std::atomic_bool serverUp_ {false};

    int32_t serverFd_ {-1};

    // Callbacks used during different stages of connection lifecycle.
    // E.g. validation callback is executed during handshake
    // and used to indicate whether the incoming connection should be accepted.
    ValidateConnectionCallback validateCb_;
    OpenConnectionCallback openCb_;

    static constexpr std::string_view BAD_REQUEST_RESPONSE = "HTTP/1.1 400 Bad Request\r\n\r\n";
    static constexpr int NET_SUCCESS = 1;
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_SERVER_WEBSOCKET_SERVER_H

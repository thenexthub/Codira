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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_WEBSOCKET_BASE_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_WEBSOCKET_BASE_H

#include "web_socket_frame.h"

#include <atomic>
#include <functional>
#include <shared_mutex>
#include <type_traits>

namespace OHOS::ArkCompiler::Toolchain {
enum CloseStatusCode : uint16_t {
    NO_STATUS_CODE = 0,
    NORMAL = 1000,
    SERVER_GO_AWAY = 1001,
    PROTOCOL_ERROR = 1002,
    UNACCEPTABLE_DATA = 1003,
    INCONSISTENT_DATA = 1007,
    POLICY_VIOLATION = 1008,
    MESSAGE_TOO_BIG = 1009,
    UNEXPECTED_ERROR = 1011,
};

class WebSocketBase {
public:
    using CloseConnectionCallback = std::function<void()>;
    using FailConnectionCallback = std::function<void()>;

public:
    static bool IsDecodeDisconnectMsg(const std::string& message);

    WebSocketBase() = default;
    virtual ~WebSocketBase() noexcept;

    /**
     * @brief Receive and decode a message.
     * Must not be called concurrently on the same connection.
     * Safe to call concurrently with `SendReply` and `Close`.
     * Control frames are handled according to specification with an empty string as returned value,
     * otherwise the method returns the decoded received message.
     * Note that this method closes the connection after receiving invalid data.
     * This event can be checked with `IsDecodeDisconnectMsg`.
     */
    std::string Decode();

    /**
     * @brief Send message on current connection.
     * Safe to call concurrently with: `SendReply`, `Decode`, `Close`.
     * Note that the connection is not closed on transmission failures.
     * @param message text payload.
     * @param frameType frame type, must be either TEXT, BINARY or CONTINUATION.
     * @param isLast flag indicating whether the message is the final.
     * @returns true on success, false otherwise.
     */
    bool SendReply(const std::string& message, FrameType frameType = FrameType::TEXT, bool isLast = true) const;

    /**
     * @brief Check if connection is in `OPEN` state.
     */
    bool IsConnected() const;

    /**
     * @brief Set callback for calling after normal connection close.
     * Non thread safe.
     */
    void SetCloseConnectionCallback(CloseConnectionCallback cb);

    /**
     * @brief Set callback for calling after closing connection on any failure.
     * Non thread safe.
     */
    void SetFailConnectionCallback(FailConnectionCallback cb);

    /**
     * @brief Send `CLOSE` frame and close the connection socket.
     * Does nothing if connection was not in `OPEN` state.
     * @param status close status code specified in sent frame.
     * @returns true if connection was closed, false otherwise.
     */
    bool CloseConnection(CloseStatusCode status);

protected:
    enum class ConnectionState : uint8_t {
        CONNECTING,
        OPEN,
        CLOSING,
        CLOSED,
    };

    enum class ConnectionCloseReason: uint8_t {
        FAIL,
        CLOSE,
    };

protected:
    /**
     * @brief Set `send` and `recv` timeout limits.
     * @param fd socket to set timeout on.
     * @param timeoutLimit timeout in seconds. If zero, function is no-op.
     * @returns true on success, false otherwise.
     */
    static bool SetWebSocketTimeOut(int32_t fd, uint32_t timeoutLimit);

    /**
     * @brief Shutdown socket for sends and receives.
     * Note that the implementation of this function is platform-specific,
     * so there is no unified way to retrieve error code returned from system call.
     * @param fd socket file descriptor.
     * @returns zero on success, `-1` otherwise.
     */
    static int ShutdownSocket(int32_t fd);

    /**
     * @brief Close the connection socket.
     * Must be transition from `CLOSING` to `CLOSED` connection state.
     * @param status close reason, depends which callback to execute.
     */
    void CloseConnectionSocket(ConnectionCloseReason status);

    /**
     * @brief Execute user-provided callbacks before closing the connection socket.
     */
    void OnConnectionClose(ConnectionCloseReason status);

    int GetConnectionSocket() const;
    void SetConnectionSocket(int socketFd);
    std::shared_mutex &GetConnectionMutex();

    ConnectionState GetConnectionState() const;
    ConnectionState SetConnectionState(ConnectionState newState);
    bool CompareExchangeConnectionState(ConnectionState& expected, ConnectionState newState);

    bool HandleDataFrame(WebSocketFrame& wsFrame) const;
    bool HandleControlFrame(WebSocketFrame& wsFrame);
    bool ReadPayload(WebSocketFrame& wsFrame) const;
    void SendPongFrame(std::string payload) const;
    void SendCloseFrame(CloseStatusCode status) const;

    bool SendUnderLock(const std::string& message) const;
    bool SendUnderLock(const char* buf, size_t totalLen) const;
    bool RecvUnderLock(std::string& message) const;
    bool RecvUnderLock(uint8_t* buf, size_t totalLen) const;

    virtual bool ValidateIncomingFrame(const WebSocketFrame& wsFrame) const = 0;
    virtual std::string CreateFrame(bool isLast, FrameType frameType) const = 0;
    virtual std::string CreateFrame(bool isLast, FrameType frameType, const std::string& payload) const = 0;
    virtual std::string CreateFrame(bool isLast, FrameType frameType, std::string&& payload) const = 0;
    virtual bool DecodeMessage(WebSocketFrame& wsFrame) const = 0;

protected:
    static constexpr size_t HTTP_HANDSHAKE_MAX_LEN = 1024;
    static constexpr int SOCKET_SUCCESS = 0;

private:
    std::atomic<ConnectionState> connectionState_ {ConnectionState::CLOSED};

    mutable std::shared_mutex connectionMutex_;
    int connectionFd_ {-1};

    // Callbacks used during different stages of connection lifecycle.
    CloseConnectionCallback closeCb_;
    FailConnectionCallback failCb_;

    static constexpr std::string_view DECODE_DISCONNECT_MSG = "disconnect";
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_WEBSOCKET_BASE_H

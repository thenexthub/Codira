/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "common/log_wrapper.h"
#include "define.h"
#include "platform/file.h"
#include "frame_builder.h"
#include "network.h"
#include "websocket_base.h"

#include <mutex>

namespace OHOS::ArkCompiler::Toolchain {
std::mutex g_sendReplymutex;
static std::string ToString(CloseStatusCode status)
{
    if (status == CloseStatusCode::NO_STATUS_CODE) {
        return "";
    }
    std::string result;
    PushNumberPerByte(result, EnumToNumber(status));
    return result;
}

WebSocketBase::~WebSocketBase() noexcept
{
    if (connectionFd_ != -1) {
        LOGW("WebSocket connection is closed while destructing the object");
        FdsanClose(reinterpret_cast<fd_t>(connectionFd_));
        // Reset directly in order to prevent static analyzer warnings.
        connectionFd_ = -1;
    }
}

// if the data is too large, it will be split into multiple frames, the first frame will be marked as 0x0
// and the last frame will be marked as 0x1.
// we just add the 'isLast' parameter to indicate whether it is the last frame.
bool WebSocketBase::SendReply(const std::string& message, FrameType frameType, bool isLast) const
{
    std::lock_guard<std::mutex> lock(g_sendReplymutex);
    if (connectionState_.load() != ConnectionState::OPEN) {
        LOGE("SendReply failed, websocket not connected");
        return false;
    }

    const auto frame = CreateFrame(isLast, frameType, message);
    if (!SendUnderLock(frame)) {
        LOGE("SendReply: send failed");
        return false;
    }
    return true;
}

/**
  *  The wired format of this data transmission section is described in detail through ABNFRFC5234.
  *  When receive the message, we should decode it according the spec. The structure is as follows:
  *     0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
  *    +-+-+-+-+-------+-+-------------+-------------------------------+
  *    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
  *    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
  *    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
  *    | |1|2|3|       |K|             |                               |
  *    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
  *    |     Extended payload length continued, if payload len == 127  |
  *    + - - - - - - - - - - - - - - - +-------------------------------+
  *    |                               |Masking-key, if MASK set to 1  |
  *    +-------------------------------+-------------------------------+
  *    | Masking-key (continued)       |          Payload Data         |
  *    +-------------------------------- - - - - - - - - - - - - - - - +
  *    :                     Payload Data continued ...                :
  *    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
  *    |                     Payload Data continued ...                |
  *    +---------------------------------------------------------------+
  */

bool WebSocketBase::ReadPayload(WebSocketFrame& wsFrame) const
{
    if (wsFrame.payloadLen == WebSocketFrame::TWO_BYTES_LENTH_ENC) {
        uint8_t recvbuf[WebSocketFrame::TWO_BYTES_LENTH] = {0};
        if (!RecvUnderLock(recvbuf, WebSocketFrame::TWO_BYTES_LENTH)) {
            LOGE("ReadPayload: Recv payloadLen == 126 failed");
            return false;
        }
        wsFrame.payloadLen = NetToHostLongLong(recvbuf, WebSocketFrame::TWO_BYTES_LENTH);
    } else if (wsFrame.payloadLen == WebSocketFrame::EIGHT_BYTES_LENTH_ENC) {
        uint8_t recvbuf[WebSocketFrame::EIGHT_BYTES_LENTH] = {0};
        if (!RecvUnderLock(recvbuf, WebSocketFrame::EIGHT_BYTES_LENTH)) {
            LOGE("ReadPayload: Recv payloadLen == 127 failed");
            return false;
        }
        wsFrame.payloadLen = NetToHostLongLong(recvbuf, WebSocketFrame::EIGHT_BYTES_LENTH);
    }
    return DecodeMessage(wsFrame);
}

bool WebSocketBase::HandleDataFrame(WebSocketFrame& wsFrame) const
{
    if (wsFrame.opcode == EnumToNumber(FrameType::TEXT)) {
        return ReadPayload(wsFrame);
    } else {
        LOGW("Received unsupported data frame, opcode = %{public}d", wsFrame.opcode);
    }
    return true;
}

bool WebSocketBase::HandleControlFrame(WebSocketFrame& wsFrame)
{
    if (wsFrame.opcode == EnumToNumber(FrameType::PING)) {
        // A Pong frame sent in response to a Ping frame must have identical
        // "Application data" as found in the message body of the Ping frame
        // being replied to.
        // https://www.rfc-editor.org/rfc/rfc6455#section-5.5.3
        if (!ReadPayload(wsFrame)) {
            LOGE("Failed to read ping frame payload");
            return false;
        }
        SendPongFrame(wsFrame.payload);
    } else if (wsFrame.opcode == EnumToNumber(FrameType::CLOSE)) {
        // might read payload to response by echoing the status code
        CloseConnection(CloseStatusCode::NO_STATUS_CODE);
    }
    return true;
}

std::string WebSocketBase::Decode()
{
    if (auto state = connectionState_.load(); state != ConnectionState::OPEN) {
        LOGE("Decode failed: websocket not connected, state = %{public}d", EnumToNumber(state));
        return "";
    }

    uint8_t recvbuf[WebSocketFrame::HEADER_LEN] = {0};
    if (!RecvUnderLock(recvbuf, WebSocketFrame::HEADER_LEN)) {
        LOGE("Decode failed, client websocket disconnect");
        CloseConnection(CloseStatusCode::UNEXPECTED_ERROR);
        return std::string(DECODE_DISCONNECT_MSG);
    }
    WebSocketFrame wsFrame(recvbuf);
    if (!ValidateIncomingFrame(wsFrame)) {
        LOGE("Received websocket frame is invalid - header is %02x%02x", recvbuf[0], recvbuf[1]);
        CloseConnection(CloseStatusCode::PROTOCOL_ERROR);
        return std::string(DECODE_DISCONNECT_MSG);
    }

    if (IsControlFrame(wsFrame.opcode)) {
        if (HandleControlFrame(wsFrame)) {
            return wsFrame.payload;
        }
    } else if (HandleDataFrame(wsFrame)) {
        return wsFrame.payload;
    }
    // Unexpected data, must close the connection.
    CloseConnection(CloseStatusCode::PROTOCOL_ERROR);
    return std::string(DECODE_DISCONNECT_MSG);
}

bool WebSocketBase::IsConnected() const
{
    return connectionState_.load() == ConnectionState::OPEN;
}

void WebSocketBase::SetCloseConnectionCallback(CloseConnectionCallback cb)
{
    closeCb_ = std::move(cb);
}

void WebSocketBase::SetFailConnectionCallback(FailConnectionCallback cb)
{
    failCb_ = std::move(cb);
}

void WebSocketBase::OnConnectionClose(ConnectionCloseReason status)
{
    if (status == ConnectionCloseReason::FAIL) {
        if (failCb_) {
            failCb_();
        }
    } else if (status == ConnectionCloseReason::CLOSE) {
        if (closeCb_) {
            closeCb_();
        }
    }
}

void WebSocketBase::CloseConnectionSocket(ConnectionCloseReason status)
{
    OnConnectionClose(status);

    {
        // Shared lock due to other thread possibly hanging on `recv` with acquired shared lock.
        std::shared_lock lock(connectionMutex_);
        int err = ShutdownSocket(connectionFd_);
        if (err != 0) {
            LOGW("Failed to shutdown client socket, errno = %{public}d", errno);
        }
    }
    {
        // Unique lock due to close and write into `connectionFd_`.
        // Note that `close` must be also done in critical section,
        // otherwise the other thread can continue using the outdated and possibly reassigned file descriptor.
        std::unique_lock lock(connectionMutex_);
        FdsanClose(reinterpret_cast<fd_t>(connectionFd_));
        // Reset directly in order to prevent static analyzer warnings.
        connectionFd_ = -1;
    }

    auto expected = ConnectionState::CLOSING;
    if (!connectionState_.compare_exchange_strong(expected, ConnectionState::CLOSED)) {
        LOGE("In connection transition CLOSING->CLOSED got initial state = %{public}d", EnumToNumber(expected));
    }
}

void WebSocketBase::SendPongFrame(std::string payload) const
{
    const auto frame = CreateFrame(true, FrameType::PONG, std::move(payload));
    if (!SendUnderLock(frame)) {
        LOGE("Decode: Send pong frame failed");
    }
}

void WebSocketBase::SendCloseFrame(CloseStatusCode status) const
{
    const auto frame = CreateFrame(true, FrameType::CLOSE, ToString(status));
    if (!SendUnderLock(frame)) {
        LOGE("SendCloseFrame: Send close frame failed");
    }
}

bool WebSocketBase::CloseConnection(CloseStatusCode status)
{
    auto expected = ConnectionState::OPEN;
    if (!connectionState_.compare_exchange_strong(expected, ConnectionState::CLOSING)) {
        // Concurrent connection close detected, do nothing.
        return false;
    }

    LOGI("Close connection, status = %{public}d", static_cast<int>(status));
    SendCloseFrame(status);
    // can close connection right after sending back close frame.
    CloseConnectionSocket(ConnectionCloseReason::CLOSE);
    return true;
}

int WebSocketBase::GetConnectionSocket() const
{
    return connectionFd_;
}

void WebSocketBase::SetConnectionSocket(int socketFd)
{
    FdsanExchangeOwnerTag(reinterpret_cast<fd_t>(socketFd));
    connectionFd_ = socketFd;
}

std::shared_mutex &WebSocketBase::GetConnectionMutex()
{
    return connectionMutex_;
}

WebSocketBase::ConnectionState WebSocketBase::GetConnectionState() const
{
    return connectionState_.load();
}

WebSocketBase::ConnectionState WebSocketBase::SetConnectionState(ConnectionState newState)
{
    return connectionState_.exchange(newState);
}

bool WebSocketBase::CompareExchangeConnectionState(ConnectionState& expected, ConnectionState newState)
{
    return connectionState_.compare_exchange_strong(expected, newState);
}

bool WebSocketBase::SendUnderLock(const std::string& message) const
{
    std::shared_lock lock(connectionMutex_);
    return Send(connectionFd_, message, 0);
}

bool WebSocketBase::SendUnderLock(const char* buf, size_t totalLen) const
{
    std::shared_lock lock(connectionMutex_);
    return Send(connectionFd_, buf, totalLen, 0);
}

bool WebSocketBase::RecvUnderLock(std::string& message) const
{
    std::shared_lock lock(connectionMutex_);
    return Recv(connectionFd_, message, 0);
}

bool WebSocketBase::RecvUnderLock(uint8_t* buf, size_t totalLen) const
{
    std::shared_lock lock(connectionMutex_);
    return Recv(connectionFd_, buf, totalLen, 0);
}

/* static */
bool WebSocketBase::IsDecodeDisconnectMsg(const std::string& message)
{
    return message == DECODE_DISCONNECT_MSG;
}

#if !defined(OHOS_PLATFORM)
/* static */
bool WebSocketBase::SetWebSocketTimeOut(int32_t fd, uint32_t timeoutLimit)
{
    if (timeoutLimit > 0) {
        struct timeval timeout = {static_cast<time_t>(timeoutLimit), 0};
        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<char *>(&timeout), sizeof(timeout)) != SOCKET_SUCCESS) {
            LOGE("SetWebSocketTimeOut setsockopt SO_SNDTIMEO failed, errno = %{public}d", errno);
            return false;
        }
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<char *>(&timeout), sizeof(timeout)) != SOCKET_SUCCESS) {
            LOGE("SetWebSocketTimeOut setsockopt SO_RCVTIMEO failed, errno = %{public}d", errno);
            return false;
        }
    }
    return true;
}
#else
/* static */
bool WebSocketBase::SetWebSocketTimeOut(int32_t fd, uint32_t timeoutLimit)
{
    if (timeoutLimit > 0) {
        struct timeval timeout = {static_cast<time_t>(timeoutLimit), 0};
        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != SOCKET_SUCCESS) {
            LOGE("SetWebSocketTimeOut setsockopt SO_SNDTIMEO failed, errno = %{public}d", errno);
            return false;
        }
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != SOCKET_SUCCESS) {
            LOGE("SetWebSocketTimeOut setsockopt SO_RCVTIMEO failed, errno = %{public}d", errno);
            return false;
        }
    }
    return true;
}
#endif

#if defined(WINDOWS_PLATFORM)
/* static */
int WebSocketBase::ShutdownSocket(int32_t fd)
{
    return shutdown(fd, SD_BOTH);
}
#else
/* static */
int WebSocketBase::ShutdownSocket(int32_t fd)
{
    return shutdown(fd, SHUT_RDWR);
}
#endif
} // namespace OHOS::ArkCompiler::Toolchain

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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_HANDSHAKE_HELPER_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_HANDSHAKE_HELPER_H

#include "define.h"
#include "http.h"
#include "network.h"

#include <array>
#include <string_view>

namespace OHOS::ArkCompiler::Toolchain {
class WebSocketKeyEncoder {
public:
    // WebSocket Globally Unique Identifier
    static constexpr std::string_view WEB_SOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    // The value of |Sec-WebSocket-Key| header field MUST be a nonce consisting of a randomly selected 16-byte value
    static constexpr size_t SEC_WEBSOCKET_KEY_BYTES_LEN = 16;
    static constexpr size_t KEY_LENGTH = GetBase64EncodingLength(SEC_WEBSOCKET_KEY_BYTES_LEN);
    // SHA1 will write SHA_DIGEST_LENGTH == 20 bytes of output
    static constexpr size_t ENCODED_KEY_LEN = GetBase64EncodingLength(SHA_DIGEST_LENGTH);

    static bool GenerateRandomSecWSKey(char (&destination)[KEY_LENGTH + 1]);
    static bool EncodeKey(std::string_view key, unsigned char (&destination)[ENCODED_KEY_LEN + 1]);
    static bool EncodeKey(const unsigned char(&key)[KEY_LENGTH + 1], unsigned char (&destination)[ENCODED_KEY_LEN + 1]);

private:
    static bool EncodeKey(const unsigned char *source, size_t length,
                          unsigned char (&destination)[ENCODED_KEY_LEN + 1]);
};

class ProtocolUpgradeBuilder {
public:
    constexpr size_t CopyStringToBuffer(std::string_view source, size_t startIndex)
    {
        for (size_t i = 0, end = source.size(); i < end; ++i, ++startIndex) {
            upgradeBuffer_[startIndex] = source[i];
        }
        return startIndex;
    }

    template <typename T, size_t LENGTH>
    constexpr size_t CopyStringToBuffer(const T (&source)[LENGTH], size_t startIndex)
    {
        for (size_t i = 0, end = LENGTH - 1; i < end; ++i, ++startIndex) {
            upgradeBuffer_[startIndex] = source[i];
        }
        return startIndex;
    }

    constexpr ProtocolUpgradeBuilder()
    {
        size_t index = CopyStringToBuffer(SWITCHING_PROTOCOLS, 0);
        index = CopyStringToBuffer(HttpBase::EOL, index);
        index = CopyStringToBuffer(CONNECTION_UPGRADE, index);
        index = CopyStringToBuffer(HttpBase::EOL, index);
        index = CopyStringToBuffer(UPGRADE_WEBSOCKET, index);
        index = CopyStringToBuffer(HttpBase::EOL, index);
        index = CopyStringToBuffer(ACCEPT_KEY, index);
        // will copy key without null terminator
        index += WebSocketKeyEncoder::ENCODED_KEY_LEN;
        index = CopyStringToBuffer(HttpBase::EOL, index);
        index = CopyStringToBuffer(HttpBase::EOL, index);
    }

    constexpr explicit ProtocolUpgradeBuilder(
        const unsigned char (&encodedKey)[WebSocketKeyEncoder::ENCODED_KEY_LEN + 1])
        : ProtocolUpgradeBuilder()
    {
        SetKey(encodedKey);
    }

    constexpr void SetKey(const unsigned char (&encodedKey)[WebSocketKeyEncoder::ENCODED_KEY_LEN + 1])
    {
        CopyStringToBuffer(encodedKey, KEY_START);
    }

    constexpr const char *GetUpgradeMessage()
    {
        return upgradeBuffer_.data();
    }

    static constexpr size_t GetLength()
    {
        return MESSAGE_LENGTH;
    }

private:
    static constexpr std::string_view SWITCHING_PROTOCOLS = "HTTP/1.1 101 Switching Protocols";
    static constexpr std::string_view CONNECTION_UPGRADE = "Connection: Upgrade";
    static constexpr std::string_view UPGRADE_WEBSOCKET = "Upgrade: websocket";
    static constexpr std::string_view ACCEPT_KEY = "Sec-WebSocket-Accept: ";
    static constexpr size_t KEY_START = SWITCHING_PROTOCOLS.size()
        + CONNECTION_UPGRADE.size()
        + UPGRADE_WEBSOCKET.size()
        + ACCEPT_KEY.size()
        + 3 * HttpBase::EOL.size();
    static constexpr size_t MESSAGE_LENGTH = KEY_START
        + WebSocketKeyEncoder::ENCODED_KEY_LEN
        + 2 * HttpBase::EOL.size();

private:
    // null-terminated string buffer
    std::array<char, MESSAGE_LENGTH + 1> upgradeBuffer_ {0};
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_HANDSHAKE_HELPER_H
